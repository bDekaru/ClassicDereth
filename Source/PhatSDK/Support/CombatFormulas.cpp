#include <StdAfx.h>
#include "PhatSDK.h"
#include "CombatFormulas.h"
#include "Config.h"

double GetImbueMultiplier(double currentSkill, double minEffectivenessSkill, double maxEffectivenessSkill, double minMultiplier, double maxMultiplier, bool allowNegative)
{
	if (currentSkill < minEffectivenessSkill)
		return minMultiplier;
	if (currentSkill > maxEffectivenessSkill)
		return maxMultiplier;

	double multiplier = (currentSkill - minEffectivenessSkill) / (maxEffectivenessSkill - minEffectivenessSkill);
	double value = multiplier * maxMultiplier;
	if (!allowNegative)
	{
		value = max(value, 0.0);
	}
	value = min(value, maxMultiplier);
	return value;
}

void CalculateDamage(DamageEventData *dmgEvent, SpellCastData *spellData)
{
	if (!dmgEvent)
	{
		return;
	}	

	dmgEvent->damageBeforeMitigation = dmgEvent->damageAfterMitigation = dmgEvent->baseDamage;

	if (!dmgEvent->source)
		return;

	if (dmgEvent->source->_IsPlayer() && dmgEvent->target->_IsPlayer())
		dmgEvent->isPvP = true;

	CalculateRendingAndMiscData(dmgEvent);
	CalculateAttributeDamageBonus(dmgEvent);
	CalculateSkillDamageBonus(dmgEvent, spellData);
	CalculateSlayerData(dmgEvent);
	//CalculateRatingData(dmgEvent);

	double damageCalc = dmgEvent->baseDamage;
	damageCalc += dmgEvent->attributeDamageBonus;
	damageCalc += dmgEvent->skillDamageBonus;
	damageCalc += dmgEvent->slayerDamageBonus;

	if (dmgEvent->wasCrit)
	{
		if (!dmgEvent->critDefended)
			damageCalc += damageCalc * dmgEvent->critMultiplier; //Leave the old formula for Melee/Missile crits.

		if (dmgEvent->damage_form == DF_MAGIC) //Multiply base spell damage by the critMultiplier before adding skill and slayer damage bonuses for Magic.
		{
			damageCalc = dmgEvent->baseDamage;
			if (!dmgEvent->critDefended)
			{
				if(dmgEvent->isPvP)
					damageCalc += ((damageCalc / 2) * dmgEvent->critMultiplier);
				else
					damageCalc += damageCalc * dmgEvent->critMultiplier;
			}
			damageCalc += dmgEvent->skillDamageBonus;
			damageCalc += dmgEvent->slayerDamageBonus;
		}
	}

	if (dmgEvent->damage_form == DF_MAGIC && !dmgEvent->source->AsPlayer())
		damageCalc /= 2; //creatures do half magic damage. Unconfirmed but feels right. Should this be projectile spells only?

	//if (dmgEvent->damageRatingMod)
	//	damageCalc *= dmgEvent->damageRatingMod;

	dmgEvent->damageBeforeMitigation = dmgEvent->damageAfterMitigation = damageCalc;
}

void CalculateAttributeDamageBonus(DamageEventData *dmgEvent)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	/*if (!dmgEvent->source->GetWielded(MELEE_WEAPON_LOC) && !dmgEvent->source->AsPlayer())
		return;*/

	double attributeBonus = 6.75;
	if(dmgEvent->damage_form == DF_MISSILE)
		attributeBonus += g_pConfig->MissileAttributeAdjust();

	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
	case DF_MISSILE:
	{
		uint32_t attrib = 0;
		if (dmgEvent->attackSkill == DAGGER_SKILL || dmgEvent->attackSkill == BOW_SKILL || dmgEvent->attackSkill == CROSSBOW_SKILL)
			dmgEvent->source->m_Qualities.InqAttribute(COORDINATION_ATTRIBUTE, attrib, FALSE);
		else if (dmgEvent->attackSkill == DUAL_WIELD_SKILL)
		{
			//get skill from weapon (source)
			int sourceSkill = 0;
			if (dmgEvent->weapon->m_Qualities.InqInt(WEAPON_SKILL_INT, sourceSkill) && (STypeSkill)sourceSkill == DAGGER_SKILL)
			{
				dmgEvent->source->m_Qualities.InqAttribute(COORDINATION_ATTRIBUTE, attrib, FALSE);
			}
			else
			{
				dmgEvent->source->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, attrib, FALSE);
			}
		}
		else
			dmgEvent->source->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, attrib, FALSE);

		double attribDamageMod;
		if (attrib >= 1000000) //this makes /godly characters use the old formula(huge damage!)
			attribDamageMod = ((int)attrib - 55.0) / 33.0;
		else
			attribDamageMod = attributeBonus*(1.0 - exp(-0.005*((int)attrib - 55)));

		if (attribDamageMod < 0 || dmgEvent->ignoreMagicArmor || dmgEvent->ignoreMagicResist) //half attribute bonus for hollow weapons.
			dmgEvent->attributeDamageBonus = dmgEvent->baseDamage * (attribDamageMod / 2.0);
		else
			dmgEvent->attributeDamageBonus = dmgEvent->baseDamage * (attribDamageMod - 1.0);

		if (dmgEvent->attributeDamageBonus < 0 && !dmgEvent->source->AsPlayer())
			dmgEvent->attributeDamageBonus = 0; //no damage penalties from attributes for monsters. This improves very early level monster's damage without doing anything for later levels.
		break;
	}
	case DF_MAGIC:
		break;
	}
}

void CalculateSkillDamageBonus(DamageEventData *dmgEvent, SpellCastData *spellData)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	if (!dmgEvent->source->AsPlayer())
		return;

	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
	case DF_MISSILE:
		return;
	case DF_MAGIC:
		if (!spellData)
			return;

		if (dmgEvent->attackSkill == WAR_MAGIC_SKILL)
		{
			ProjectileSpellEx *meta = (ProjectileSpellEx *)spellData->spellEx->_meta_spell._spell;
			//Skill based damage bonus: This additional damage will be a constant percentage of the minimum damage value.
			//The percentage is determined by comparing the level of the spell against the buffed war magic skill of the character.
			//Note that creatures do not receive this bonus.
			if (dmgEvent->source->AsPlayer())
			{
				float minDamage = (float)meta->_baseIntensity;
				if (dmgEvent->isPvP)
					minDamage *= 0.5f;
				
				float skillDamageMod = 0;

				auto spellLevel = spellData->spell_formula.GetPowerLevelOfPowerComponent();
				int difficulty = 100 + (50 * spellLevel);
				if(spellData->current_skill > difficulty)
				{
					skillDamageMod = 1.0;
				}
				else if (spellData->current_skill > (difficulty - 75))
				{
					skillDamageMod = (spellData->current_skill - (difficulty - 75)) / (difficulty - (difficulty - 75));
				}

				if (dmgEvent->isPvP)
					skillDamageMod *= 0.5;

				if (skillDamageMod > 0)
					dmgEvent->skillDamageBonus = minDamage * skillDamageMod;
			}
		}
		return;
	}
}

void CalculateCriticalHitData(DamageEventData *dmgEvent, SpellCastData *spellData)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	if (!dmgEvent->target)
		return;

	bool isPvP = dmgEvent->source->AsPlayer() && dmgEvent->target->AsPlayer();

	uint32_t imbueEffects;

	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
		dmgEvent->critChance = 0.1;
		dmgEvent->critMultiplier = 1.0;

		if (!dmgEvent->weapon)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		if (dmgEvent->weapon->GetBitingStrikeFrequency())
			dmgEvent->critChance = dmgEvent->weapon->GetBitingStrikeFrequency();

		if (dmgEvent->weapon->GetCrushingBlowMultiplier())
			dmgEvent->critMultiplier += dmgEvent->weapon->GetCrushingBlowMultiplier();

		if (imbueEffects & CriticalStrike_ImbuedEffectType)
			dmgEvent->critChance = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCSMeleeMinSkill() : 150, dmgEvent->isPvP ? g_pConfig->GetPkCSMeleeMaxSkill() : 400,
				dmgEvent->isPvP ? g_pConfig->GetPkCSMeleeBaseChance() : dmgEvent->critChance, dmgEvent->isPvP ? g_pConfig->GetPkCSMeleeMaxChance() : 0.5);

		if (imbueEffects & CripplingBlow_ImbuedEffectType)
			dmgEvent->critMultiplier = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCBMeleeMinSkill() : 150, dmgEvent->isPvP ? g_pConfig->GetPkCBMeleeMaxSkill() : 400,
				dmgEvent->isPvP ? g_pConfig->GetPkCBMeleeBaseMult() : dmgEvent->critMultiplier, 7);

		return;
	case DF_MISSILE:
		dmgEvent->critChance = 0.1;
		dmgEvent->critMultiplier = 1.0;

		if (!dmgEvent->weapon)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		if (dmgEvent->weapon->GetBitingStrikeFrequency())
			dmgEvent->critChance = dmgEvent->weapon->GetBitingStrikeFrequency();

		if (dmgEvent->weapon->GetCrushingBlowMultiplier())
			dmgEvent->critMultiplier += dmgEvent->weapon->GetCrushingBlowMultiplier();

		if (imbueEffects & CriticalStrike_ImbuedEffectType)
			dmgEvent->critChance = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCSMissileMinSkill() : 125, dmgEvent->isPvP ? g_pConfig->GetPkCSMissileMaxSkill() : 360,
				dmgEvent->isPvP ? g_pConfig->GetPkCSMissileBaseChance() : dmgEvent->critChance, dmgEvent->isPvP ? g_pConfig->GetPkCSMissileMaxChance() : 0.5);

		if (imbueEffects & CripplingBlow_ImbuedEffectType)
			dmgEvent->critMultiplier = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCBMissileMinSkill() : 125, dmgEvent->isPvP ? g_pConfig->GetPkCBMissileMaxSkill() : 360,
				dmgEvent->isPvP ? g_pConfig->GetPkCBMissileBaseMult() : dmgEvent->critMultiplier, 7);

		return;
	case DF_MAGIC:
		dmgEvent->critChance = 0.05;
		dmgEvent->critMultiplier = 0.5;

		if (!dmgEvent->weapon)
			return;
		if (!spellData)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		if(dmgEvent->weapon->GetBitingStrikeFrequency())
		dmgEvent->critChance = dmgEvent->weapon->GetBitingStrikeFrequency();

		if(dmgEvent->weapon->GetCrushingBlowMultiplier())
		dmgEvent->critMultiplier += dmgEvent->weapon->GetCrushingBlowMultiplier();

		ProjectileSpellEx *meta = (ProjectileSpellEx *)spellData->spellEx->_meta_spell._spell;
		if (dmgEvent->attackSkill == WAR_MAGIC_SKILL || (dmgEvent->attackSkill == VOID_MAGIC_SKILL && meta->AsProjectileSpell()))
		{
			//Imbue and slayer effects for War Magic now scale from minimum effectiveness at 125 to 
			//maximum effectiveness at 360 skill instead of from 150 to 400 skill(PvM only).

			//Critical Strike for War Magic scales from 5% critical hit chance to 50% critical hit chance at maximum effectiveness.
			//PvP: Critical Strike for War Magic scales from 5% critical hit chance to 25% critical hit chance at maximum effectiveness.
			if (imbueEffects & CriticalStrike_ImbuedEffectType)
				dmgEvent->critChance = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCSMagicMinSkill() : 125, dmgEvent->isPvP ? g_pConfig->GetPkCSMagicMaxSkill() : 360,
					dmgEvent->isPvP ? g_pConfig->GetPkCSMagicBaseChance() : dmgEvent->critChance, dmgEvent->isPvP ? g_pConfig->GetPkCSMagicMaxChance() : 0.5);

			//Crippling Blow for War Magic currently scales from adding 50% of the spells damage
			//on critical hits to adding 500% at maximum effectiveness.
			//PvP: Crippling Blow for War Magic currently scales from adding 50 % of the spells damage on critical hits 
			//to adding 100 % at maximum effectiveness
			if (imbueEffects & CripplingBlow_ImbuedEffectType)
				dmgEvent->critMultiplier = GetImbueMultiplier(dmgEvent->attackSkillLevel, dmgEvent->isPvP ? g_pConfig->GetPkCBMagicMinSkill() : 125, dmgEvent->isPvP ? g_pConfig->GetPkCBMagicMaxSkill() : 360,
					dmgEvent->isPvP ? g_pConfig->GetPkCBMagicBaseMult() : dmgEvent->critMultiplier, 5);
		}

		return;
	}
}

void CalculateSlayerData(DamageEventData *dmgEvent)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	if (!dmgEvent->target)
		return;
	if (!dmgEvent->weapon)
		return;

	if (dmgEvent->damage_form == DF_MAGIC && !dmgEvent->isProjectileSpell)
		return; //non projectile spells do not benefit from the slayer property.

	double slayerDamageMod = 0.0;
	int slayerType = dmgEvent->weapon->InqIntQuality(SLAYER_CREATURE_TYPE_INT, 0, TRUE);
	if (slayerType && slayerType == dmgEvent->target->InqIntQuality(CREATURE_TYPE_INT, 0, TRUE))
		slayerDamageMod = dmgEvent->weapon->InqFloatQuality(SLAYER_DAMAGE_BONUS_FLOAT, 0.0, FALSE);

	double slayerSkillMod = 1.0;
	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
		slayerSkillMod = GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 1.0, slayerDamageMod);
		break;
	case DF_MISSILE:
	case DF_MAGIC:
		slayerSkillMod = GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 1.0, slayerDamageMod);
		break;
	}

	if (slayerSkillMod > 0.0)
		dmgEvent->slayerDamageBonus = dmgEvent->baseDamage * (slayerSkillMod - 1.0);
}

void CalculateRendingAndMiscData(DamageEventData *dmgEvent)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;

	dmgEvent->ignoreMagicResist = dmgEvent->source->InqBoolQuality(IGNORE_MAGIC_RESIST_BOOL, FALSE);
	dmgEvent->ignoreMagicArmor = dmgEvent->source->InqBoolQuality(IGNORE_MAGIC_ARMOR_BOOL, FALSE);

	if (!dmgEvent->weapon)
		return;

	if (!dmgEvent->ignoreMagicResist)
		dmgEvent->ignoreMagicResist = dmgEvent->weapon->InqBoolQuality(IGNORE_MAGIC_RESIST_BOOL, FALSE);

	if (!dmgEvent->ignoreMagicArmor)
		dmgEvent->ignoreMagicArmor =dmgEvent->weapon->InqBoolQuality(IGNORE_MAGIC_ARMOR_BOOL, FALSE);

	uint32_t imbueEffects = dmgEvent->weapon->GetImbueEffects();

	if (imbueEffects & IgnoreAllArmor_ImbuedEffectType)
		dmgEvent->ignoreArmorEntirely = true;

	if (imbueEffects & ArmorRending_ImbuedEffectType)
		dmgEvent->isArmorRending = true;

	switch (dmgEvent->damage_type)
	{
	case SLASH_DAMAGE_TYPE:
		if (imbueEffects & SlashRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case PIERCE_DAMAGE_TYPE:
		if (imbueEffects & PierceRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case BLUDGEON_DAMAGE_TYPE:
		if (imbueEffects & BludgeonRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case COLD_DAMAGE_TYPE:
		if (imbueEffects & ColdRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case FIRE_DAMAGE_TYPE:
		if (imbueEffects & FireRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case ACID_DAMAGE_TYPE:
		if (imbueEffects & AcidRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	case ELECTRIC_DAMAGE_TYPE:
		if (imbueEffects & ElectricRending_ImbuedEffectType)
			dmgEvent->isElementalRending = true;
		break;
	}

	if (dmgEvent->isElementalRending)
	{
		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			dmgEvent->rendingMultiplier = max(GetImbueMultiplier(dmgEvent->attackSkillLevel, 0, 400, 1.0, 2.5), 1.0);
			break;
		case DF_MISSILE:
			dmgEvent->rendingMultiplier = max(0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 0, 360, 1.0, 2.25), 1.0);
			break;
		case DF_MAGIC:
			dmgEvent->rendingMultiplier = max(0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 0, 360, 1.0, 2.25), 1.0);
			break;
		default:
			return;
		}
	}

	int resistanceMod;

	if (dmgEvent->weapon->m_Qualities.InqInt(RESISTANCE_MODIFIER_TYPE_INT, resistanceMod))
		dmgEvent->isResistanceCleaving = TRUE;

	if (dmgEvent->isResistanceCleaving && !dmgEvent->isElementalRending)
	{
		if (resistanceMod == dmgEvent->damage_type)
			dmgEvent->isElementalRending = true;

		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			dmgEvent->rendingMultiplier = 2.5;
			break;
		case DF_MISSILE:
			dmgEvent->rendingMultiplier = 2.25;
			break;
		case DF_MAGIC:
			dmgEvent->rendingMultiplier = 2.25;
			break;
		default:
			return;
		}
	}

	if (dmgEvent->isArmorRending)
	{
		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			dmgEvent->armorRendingMultiplier = 1.0 / max(GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 1.0, 2.5), 1.0);
			break;
		case DF_MISSILE:
			dmgEvent->armorRendingMultiplier = 1.0 / max(0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 1.0, 2.25), 1.0);
			break;
		case DF_MAGIC:
		default:
			return;
		}
	}

	if (dmgEvent->weapon->InqFloatQuality(IGNORE_ARMOR_FLOAT, 0, FALSE))
		dmgEvent->isArmorCleaving = TRUE;

	if (dmgEvent->isArmorCleaving && !dmgEvent->isArmorRending)
	{
		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			dmgEvent->armorRendingMultiplier = 1.0 / 2.5;
		case DF_MISSILE:
			dmgEvent->armorRendingMultiplier = 1.0 / 2.25;
			break;
		case DF_MAGIC:
		default:
			return;
		}
	}

}

void CalculateRatingData(DamageEventData *dmgEvent)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	if (!dmgEvent->target)
		return;

	Skill skill;
	int ratingtotal = 0;

	// From attacker
	// Damage
	ratingtotal += dmgEvent->source->GetRating(DAMAGE_RATING_INT);
	// Recklessness
	if (dmgEvent->isReckless)
		ratingtotal += dmgEvent->source->GetRating(RECKLESSNESS_RATING_INT);
	// Sneak Attack
	if (dmgEvent->isSneakAttack)
		ratingtotal += dmgEvent->source->GetRating(SNEAK_ATTACK_RATING_INT);
	// Weakness
	ratingtotal -= dmgEvent->source->GetRating(WEAKNESS_RATING_INT);
	// Crit
	if (dmgEvent->wasCrit)
	{
		ratingtotal += dmgEvent->source->GetRating(CRIT_DAMAGE_RATING_INT);
	}
	// PvP
	if (dmgEvent->isPvP)
	{
		// PK Damage
		ratingtotal += dmgEvent->source->GetRating(PK_DAMAGE_RATING_INT);
	}

	// From defender
	// Damage Resistance - Defender
	ratingtotal -= dmgEvent->target->GetRating(DAMAGE_RESIST_RATING_INT);
	// Recklessness - Defender
	ratingtotal += dmgEvent->target->GetRating(RECKLESSNESS_RATING_INT);
	// Criticals
	if (dmgEvent->wasCrit)
	{
		// Critical Damage Resistance - Defender
		ratingtotal -= dmgEvent->target->GetRating(CRIT_DAMAGE_RESIST_RATING_INT);
	}
	// PvP
	if (dmgEvent->isPvP)
	{
		// PK Damage Resistance - Defender
		ratingtotal -= dmgEvent->target->GetRating(PK_DAMAGE_RESIST_RATING_INT);
	}

	/*Specialized Defense skill damage resist bonuses from Balance of Power patch.
	Per release notes:
	New bonus added to specialized defenses against damage of their respective attack type. (Applied in both PvE & PvP)
	Specialized Melee Defense skill now adds 1 Damage Rating Resist for every 60 pts against melee attacks
	Specialized Missile Defense skill now adds 1 Damage Rating Resist for every 50 pts against missile attacks
	Specialized Magic Defense skill now adds 1 Damage Rating Resist for every 50 pts against magic attacks*/
	uint32_t skillLevel = 0;
	STypeSkill specSkill = UNDEF_SKILL;

	if (dmgEvent->target->_IsPlayer() && dmgEvent->damage_form > 0 && dmgEvent->damage_form <= DF_MAGIC)
	{
		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			specSkill = MELEE_DEFENSE_SKILL;
			break;
		case DF_MISSILE:
			specSkill = MISSILE_DEFENSE_SKILL;
			break;
		case DF_MAGIC:
			specSkill = MAGIC_DEFENSE_SKILL;
			break;
		}

		if (specSkill > UNDEF_SKILL)
		{
			if (dmgEvent->target->m_Qualities.InqSkill(specSkill, skill))
			{
				if (skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
				{
					dmgEvent->target->InqSkill(specSkill, skillLevel, false);

					ratingtotal -= (int)((float)skillLevel / (dmgEvent->damage_form == DF_MELEE ? 60 : 50));
				}
			}
		}
	}

	dmgEvent->damageRatingMod = GetRatingMod(ratingtotal);
}

void CalculateAttackConditions(DamageEventData *dmgEvent, float attackPower, double angle)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;
	if (!dmgEvent->target)
		return;

	Skill skill;
	int rating = 0;
	uint32_t skillLevel = 0;
	float skillMod = 0.0f;

	if (dmgEvent->wasCrit)
	{
		if (bool critDefense = dmgEvent->target->m_Qualities.GetInt(AUGMENTATION_CRITICAL_DEFENSE_INT, 0))
		{
			if (Random::GenFloat(0.0, 1.0) < (dmgEvent->source->_IsPlayer() ? 0.05 : 0.25))
			{
				// do not apply crit multiplier - treat this as a normal hit.
				dmgEvent->critDefended = true;
				dmgEvent->attackConditions |= 1;
			}
		}

		// Recklessness does not affect critical damage.
		dmgEvent->source->m_Qualities.SetInt(RECKLESSNESS_RATING_INT, 0);
	}

	//if (!dmgEvent->wasCrit)
	//{
	//	if (dmgEvent->source->m_Qualities.InqSkill(RECKLESSNESS_SKILL, skill))
	//	{
	//		// Wiki states that Recklessness only procs between 20% and 80% power but the client seems to show 10% and 90%
	//		if (skill._sac >= TRAINED_SKILL_ADVANCEMENT_CLASS && attackPower > 0.1f && attackPower < 0.9f)
	//		{
	//			rating = skill._sac >= SPECIALIZED_SKILL_ADVANCEMENT_CLASS ? 20 : 10;
	//			dmgEvent->source->InqSkill(RECKLESSNESS_SKILL, skillLevel, false);

	//			if (skillLevel < dmgEvent->attackSkillLevel)
	//			{
	//				skillMod = (float)skillLevel / (float)dmgEvent->attackSkillLevel;
	//				rating *= skillMod;
	//			}

	//			dmgEvent->source->m_Qualities.SetInt(RECKLESSNESS_RATING_INT, rating);
	//			dmgEvent->isReckless = true;
	//			dmgEvent->attackConditions |= 2;
	//		}
	//		else
	//		{
	//			// Not Reckless so set the rating int to zero so you no longer take increased damage.
	//			dmgEvent->source->m_Qualities.SetInt(RECKLESSNESS_RATING_INT, 0);
	//		}
	//	}
	//}

	//if (dmgEvent->source->m_Qualities.InqSkill(SNEAK_ATTACK_SKILL, skill))
	//{
	//	if (skill._sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
	//	{
	//		rating = skill._sac >= SPECIALIZED_SKILL_ADVANCEMENT_CLASS ? 20 : 10;
	//		dmgEvent->source->InqSkill(SNEAK_ATTACK_SKILL, skillLevel, false);

	//		if (skillLevel < dmgEvent->attackSkillLevel)
	//		{
	//			skillMod = (float)skillLevel / (float)dmgEvent->attackSkillLevel;
	//			rating *= skillMod;
	//		}

	//		// If behind the target then this is a sneak attack.
	//		if (angle >= 90 && angle <= 270)
	//		{
	//			dmgEvent->source->m_Qualities.SetInt(SNEAK_ATTACK_RATING_INT, rating);
	//			dmgEvent->isSneakAttack = true;
	//			dmgEvent->attackConditions |= 4;
	//		}
	//		else
	//		{
	//			dmgEvent->source->m_Qualities.InqSkill(DECEPTION_SKILL, skill);

	//			// Max chance is 10% for trained and 15% for Specialized Deception.
	//			if (skill._sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
	//			{
	//				float chance = 0.0f;
	//				float roll = Random::GenFloat(0.0, 1.0);

	//				chance = skill._sac >= SPECIALIZED_SKILL_ADVANCEMENT_CLASS ? 0.15f : 0.1f;

	//				dmgEvent->source->InqSkill(DECEPTION_SKILL, skillLevel, false);

	//				// If Deception is below 306 your chance is reduced proportionately.
	//				if (skillLevel < 306)
	//				{
	//					chance *= min(((float)skillLevel / 306.0f), 1.0f);
	//				}

	//				if (roll < chance)
	//				{
	//					dmgEvent->target->m_Qualities.InqSkill(PERSONAL_APPRAISAL_SKILL, skill);

	//					// Assess Person can reduce the additional sneak attack damage from the front by up to 100% of the sneak attack bonus.
	//					if (skill._sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
	//					{
	//						dmgEvent->target->InqSkill(PERSONAL_APPRAISAL_SKILL, skillLevel, false);

	//						// If Assess Person is 306 or above the damage rating is nullified. Otherwise it is reduced proportionately.
	//						if (skillLevel < 306)
	//						{
	//							rating *= min(((float)skillLevel / 306.0f), 1.0f);
	//						}
	//						else
	//							rating = 0;
	//					}

	//					if (rating > 0)
	//						dmgEvent->source->m_Qualities.SetInt(SNEAK_ATTACK_RATING_INT, rating);

	//					dmgEvent->isSneakAttack = true;
	//					dmgEvent->attackConditions |= 4;
	//				}
	//				else
	//				{
	//					// Not a Sneak Attack so set rating to 0
	//					dmgEvent->source->m_Qualities.SetInt(SNEAK_ATTACK_RATING_INT, 0);
	//				}
	//			}
	//		}
	//	}
	//}

}
