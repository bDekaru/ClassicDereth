
#include "StdAfx.h"
#include "PhatSDK.h"
#include "CombatFormulas.h"

double GetImbueMultiplier(double currentSkill, double minEffectivenessSkill, double maxEffectivenessSkill, double maxMultiplier, bool allowNegative)
{
	double multiplier = (currentSkill - minEffectivenessSkill) / (maxEffectivenessSkill - minEffectivenessSkill);
	double value = multiplier * maxMultiplier;
	if (!allowNegative)
		value = max(value, 0.0);
	value = min(value, maxMultiplier);
	return value;
}

void CalculateDamage(DamageEventData *dmgEvent, SpellCastData *spellData)
{
	if (!dmgEvent)
		return;

	dmgEvent->damageBeforeMitigation = dmgEvent->damageAfterMitigation = dmgEvent->baseDamage;

	if (!dmgEvent->source)
		return;

	CalculateAttributeDamageBonus(dmgEvent);
	CalculateSkillDamageBonus(dmgEvent, spellData);
	CalculateCriticalHitData(dmgEvent, spellData);
	CalculateSlayerData(dmgEvent);
	CalculateRendingAndMiscData(dmgEvent);

	double damageCalc = dmgEvent->baseDamage;
	damageCalc += dmgEvent->attributeDamageBonus;
	damageCalc += dmgEvent->skillDamageBonus;
	damageCalc += dmgEvent->slayerDamageBonus;

	dmgEvent->wasCrit = (Random::GenFloat(0.0, 1.0) < dmgEvent->critChance) ? true : false;
	if (dmgEvent->wasCrit)
		damageCalc += damageCalc * dmgEvent->critMultiplier;

	if (dmgEvent->damage_form == DF_MAGIC && !dmgEvent->source->AsPlayer())
		damageCalc /= 2; //creatures do half magic damage. Unconfirmed but feels right. Should this be projectile spells only?

	dmgEvent->damageBeforeMitigation = dmgEvent->damageAfterMitigation = damageCalc;
}

void CalculateAttributeDamageBonus(DamageEventData *dmgEvent)
{
	if (!dmgEvent)
		return;
	if (!dmgEvent->source)
		return;

	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
	case DF_MISSILE:
	{
		DWORD attrib = 0;
		if (dmgEvent->attackSkill == DAGGER_SKILL || dmgEvent->attackSkill == BOW_SKILL || dmgEvent->attackSkill == CROSSBOW_SKILL)
			dmgEvent->source->m_Qualities.InqAttribute(COORDINATION_ATTRIBUTE, attrib, FALSE);
		else
			dmgEvent->source->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, attrib, FALSE);

		//double attribDamageMod = ((int)attrib - 55.0) / 33.0;
		double attribDamageMod = 6.75*(1.0 - exp(-0.005*((int)attrib - 55)));
		if (attribDamageMod < 0)
			dmgEvent->attributeDamageBonus = dmgEvent->baseDamage * (attribDamageMod / 2.0);
		else
			dmgEvent->attributeDamageBonus = dmgEvent->baseDamage * (attribDamageMod - 1.0);
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

				float skillDamageMod = ((int)spellData->current_skill - (spellData->spell->_power + 50)) / 250.0; //made up formula.
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

	DWORD imbueEffects;

	switch (dmgEvent->damage_form)
	{
	case DF_MELEE:
		dmgEvent->critChance = 0.1;
		dmgEvent->critMultiplier = 1.0;

		if (!dmgEvent->weapon)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		dmgEvent->critChance += (dmgEvent->critChance * dmgEvent->weapon->GetBitingStrikeFrequency());
		dmgEvent->critMultiplier += dmgEvent->weapon->GetCrushingBlowMultiplier();

		if (imbueEffects & CriticalStrike_ImbuedEffectType)
			dmgEvent->critChance += GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 0.5);

		if (imbueEffects & CripplingBlow_ImbuedEffectType)
			dmgEvent->critMultiplier += GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 6);

		dmgEvent->critMultiplier = min(max(dmgEvent->critMultiplier, 0.0), 7.0);
		dmgEvent->critChance = min(max(dmgEvent->critChance, 0.0), 1.0);
		return;
	case DF_MISSILE:
		dmgEvent->critChance = 0.1;
		dmgEvent->critMultiplier = 1.0;

		if (!dmgEvent->weapon)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		dmgEvent->critChance += (dmgEvent->critChance * dmgEvent->weapon->GetBitingStrikeFrequency());
		dmgEvent->critMultiplier += dmgEvent->weapon->GetCrushingBlowMultiplier();

		if (imbueEffects & CriticalStrike_ImbuedEffectType)
			dmgEvent->critChance += GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 0.5);

		if (imbueEffects & CripplingBlow_ImbuedEffectType)
			dmgEvent->critMultiplier += GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 6);

		dmgEvent->critMultiplier = min(max(dmgEvent->critMultiplier, 0.0), 7.0);
		dmgEvent->critChance = min(max(dmgEvent->critChance, 0.0), 1.0);
		return;
	case DF_MAGIC:
		dmgEvent->critChance = 0.05;
		dmgEvent->critMultiplier = 0.5;

		if (!dmgEvent->weapon)
			return;
		if (!spellData)
			return;

		imbueEffects = dmgEvent->weapon->GetImbueEffects();

		if (dmgEvent->attackSkill == WAR_MAGIC_SKILL)
		{
			ProjectileSpellEx *meta = (ProjectileSpellEx *)spellData->spellEx->_meta_spell._spell;
			//Imbue and slayer effects for War Magic now scale from minimum effectiveness at 125 to 
			//maximum effectiveness at 360 skill instead of from 150 to 400 skill(PvM only).

			bool isPvP = dmgEvent->source->AsPlayer() && dmgEvent->target->AsPlayer();

			if (imbueEffects & CriticalStrike_ImbuedEffectType)
			{
				//Critical Strike for War Magic scales from 5% critical hit chance to 50% critical hit chance at maximum effectiveness.
				//PvP: Critical Strike for War Magic scales from 5% critical hit chance to 25% critical hit chance at maximum effectiveness.
				if (isPvP)
					dmgEvent->critChance += GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 0.25);
				else
					dmgEvent->critChance += GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 0.5);
			}

			if (imbueEffects & CripplingBlow_ImbuedEffectType)
			{
				//Crippling Blow for War Magic currently scales from adding 50% of the spells damage
				//on critical hits to adding 500% at maximum effectiveness.
				//PvP: Crippling Blow for War Magic currently scales from adding 50 % of the spells damage on critical hits 
				//to adding 100 % at maximum effectiveness
				if (isPvP)
					dmgEvent->critMultiplier += GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 1.0);
				else
					dmgEvent->critMultiplier += GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 5.0);
			}
		}
		dmgEvent->critMultiplier = min(max(dmgEvent->critMultiplier, 0.0), 7.0);
		dmgEvent->critChance = min(max(dmgEvent->critChance, 0.0), 1.0);
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

	if (slayerDamageMod > 0.0)
		dmgEvent->slayerDamageBonus = dmgEvent->baseDamage * (slayerDamageMod - 1.0);
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

	DWORD imbueEffects = dmgEvent->weapon->GetImbueEffects();

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

	if (dmgEvent->isArmorRending || dmgEvent->isElementalRending)
	{
		switch (dmgEvent->damage_form)
		{
		case DF_MELEE:
			dmgEvent->rendingMultiplier = 0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 0.5); //made up formula.
			break;
		case DF_MISSILE:
			dmgEvent->rendingMultiplier = 0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 0.5); //made up formula.
			break;
		case DF_MAGIC:
		{
			bool isPvP = dmgEvent->source->AsPlayer() && dmgEvent->target->AsPlayer();
			if (isPvP)
				dmgEvent->rendingMultiplier = 0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 125, 360, 0.5); //made up formula.
			else
				dmgEvent->rendingMultiplier = 0.25 + GetImbueMultiplier(dmgEvent->attackSkillLevel, 150, 400, 0.5); //made up formula.
			break;
		}
		default:
			return;
		}
	}

	//prepare rendingMultiplier for use.
	if (dmgEvent->isElementalRending)
		dmgEvent->rendingMultiplier += 1.0; //positive multiplier
	else if (dmgEvent->isArmorRending)
		dmgEvent->rendingMultiplier = 1.0 - dmgEvent->rendingMultiplier; //negative multiplier
}