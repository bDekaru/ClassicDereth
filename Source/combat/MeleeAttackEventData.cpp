#include <StdAfx.h>
#include "AttackManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "Player.h"
#include "WeenieFactory.h"
#include "Ammunition.h"
#include "CombatFormulas.h"
#include "combat/MeleeAttackEventData.h"

// TODO: Move these up to AttackEventData?
void CMeleeAttackEvent::CalculateAtt(CWeenieObject *weapon, STypeSkill& weaponSkill, uint32_t& weaponSkillLevel)
{
	float offenseMod = weapon->GetOffenseMod();
	weaponSkill = (STypeSkill)weapon->InqIntQuality(WEAPON_SKILL_INT, UNARMED_COMBAT_SKILL, TRUE);
	weaponSkillLevel = 0;

	if (_weenie->InqSkill(weaponSkill, weaponSkillLevel, FALSE))
	{
		weaponSkillLevel = (uint32_t)(weaponSkillLevel * offenseMod);
	}
}

//float CMeleeAttackEvent::CalculateDef()
//{
//	CWeenieObject *weapon = _weenie->GetWieldedCombat(_combat_style);
//	if (weapon)
//	{
//		float defenseMod = weapon->GetMeleeDefenseMod();
//		return defenseMod;
//	}
//
//	return CAttackEventData::CalculateDef();
//}

void CMeleeAttackEvent::Setup()
{
	uint32_t attack_motion = 0;
	uint32_t weapon_id = 0;
	uint32_t style = _weenie->get_minterp()->InqStyle();

	bool exhausted = _weenie->GetStamina() == 0;
	float effective_attack_power = (exhausted? 0.0f : _attack_power );

	//Recalculate animation if exhausted because we have an effective attack power much lower than our actual attack power
	if (exhausted || !_do_attack_animation)
	{
		if (_weenie->_combatTable)
		{
			CWeenieObject *weapon = _weenie->GetWieldedCombat(_combat_style);

			if (weapon)
			{
				//_max_attack_distance = weapon->InqFloatQuality(WEAPON_LENGTH_FLOAT, 0.5); //todo: this would be interesting but something makes the character still move next to the target anyway. Is it the client?

				weapon_id = weapon->GetID();

				AttackType weapon_attack_type = (AttackType)weapon->InqIntQuality(ATTACK_TYPE_INT, 0);

				//Need high power for multistrike.
				const uint32_t anyMultistrike = DoubleThrust_AttackType | DoubleSlash_AttackType |TripleThrust_AttackType | TripleSlash_AttackType;
				if(weapon_attack_type & anyMultistrike && effective_attack_power < 0.75f) {
					weapon_attack_type = (AttackType)(weapon_attack_type & ~anyMultistrike);
				}

				AttackType attack_type = weapon_attack_type;

				if (attack_type == (Thrust_AttackType | Slash_AttackType)) {
					if (effective_attack_power >= 0.25f)
						attack_type = Slash_AttackType;
					else
						attack_type = Thrust_AttackType;
				}
				
				// Different rules for Dual Wield vs Onehand and shield.
				if (style == Motion_DualWieldCombat)
				{
					// Dual Wield can use both slash and thrust animations based on power.
					switch (attack_type)
					{
					case DoubleThrust_AttackType | DoubleSlash_AttackType:
						if (effective_attack_power >= 0.25f)
							attack_type = DoubleSlash_AttackType;
						else
							attack_type = DoubleThrust_AttackType;
						break;

					case TripleThrust_AttackType | TripleSlash_AttackType:
						if (effective_attack_power >= 0.25f)
							attack_type = TripleSlash_AttackType;
						else
							attack_type = TripleThrust_AttackType;
						break;
					}
				}
				else if (style == Motion_SwordShieldCombat)
				{
					// Force Thrust animation when use a shield with a multi-strike weapon.
					switch (attack_type)
					{
					case DoubleThrust_AttackType | DoubleSlash_AttackType:
						attack_type = DoubleThrust_AttackType;
						break;

					case TripleThrust_AttackType | TripleSlash_AttackType:
						attack_type = TripleThrust_AttackType;
						break;
					}
				}



				if (CombatManeuver *combat_maneuver = _weenie->_combatTable->TryGetCombatManuever(style, attack_type, _attack_height))
				{
					attack_motion = combat_maneuver->motion;
					CombatStyle cs = (CombatStyle)weapon->m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, 0);

					// UA full speed attacks (Low/Medium only) need 3 removed to get the correct animation.
					if (cs == Unarmed_CombatStyle && effective_attack_power <= 0.25f && (_attack_height == MEDIUM_ATTACK_HEIGHT || _attack_height == LOW_ATTACK_HEIGHT))
						attack_motion -= 3;

					//Slash_AttackType -> Axes, mace, jitte: use a backhand when using lowish power.
					//Thrust_AttackType|Slash_AttackType -> staff, dagger, sword: use a backhand when using lowish power but not super low (pierce)
					//Adding 3 to Motion_Slash[High|Medium|Low] yields Motion_Backhand[High|Medium|Low].
					//Note that some weapons like sing dagger are (Thrust|Slash|DoubleSlash|DoubleThrust), so use & instead of ==
					if((weapon_attack_type == Slash_AttackType && effective_attack_power <= 0.33f) ||
						((weapon_attack_type == (Thrust_AttackType | Slash_AttackType)) && effective_attack_power <= 0.50f && effective_attack_power >= 0.25f)) {
						attack_motion += 3;
					}
				}
			
			}
		}

		if (!attack_motion)
		{
			switch (_attack_height)
			{
			case LOW_ATTACK_HEIGHT: attack_motion = Motion_AttackLow1; break;
			case MEDIUM_ATTACK_HEIGHT: attack_motion = Motion_AttackMed1; break;
			case HIGH_ATTACK_HEIGHT: attack_motion = Motion_AttackHigh1; break;
			default:
			{
				Cancel();
				return;
			}
			}

			if (effective_attack_power >= 0.25f)
				attack_motion += 3;
			if (effective_attack_power >= 0.75f)
				attack_motion += 3;

			if (_attack_power < 0.0f || _attack_power > 1.0f)
			{
				Cancel();
				return;
			}
		}

		// melee attacks can charge!
		m_bCanCharge = true;
		_do_attack_animation = attack_motion;
	}

	uint32_t quickness = 0;
	_weenie->m_Qualities.InqAttribute(QUICKNESS_ATTRIBUTE, quickness, FALSE);

	int weaponAttackTime = _weenie->GetAttackTimeUsingWielded();
	int creatureAttackTime = max(0, 120 - (((int)quickness - 60) / 2)); //we reach 0 attack speed at 300 quickness

	int attackTime = (creatureAttackTime + weaponAttackTime) / 2; //our attack time is the average between our speed and the speed of our weapon.
	attackTime = max(0, min(120, attackTime));

	_attack_speed = 2.25f - (attackTime * (1.0 / 70.0));
	_attack_speed = max(min(_attack_speed, 2.25f), 0.8f);

	//old formula:
	//int attackTime = max(0, min(120, _weenie->GetAttackTimeUsingWielded()));
	//_attack_speed = 1.0 / (1.0 / (1.0 + ((120 - attackTime) * (0.005))));

	CAttackEventData::Setup();
}

void CMeleeAttackEvent::OnReadyToAttack()
{
	if (_do_attack_animation)
	{
		MovementParameters params;
		params.sticky = 1;
		params.can_charge = 1;
		params.modify_interpreted_state = 1;
		params.speed = _attack_speed;
		params.action_stamp = ++_weenie->m_wAnimSequence;
		params.autonomous = 0;
		_weenie->stick_to_object(_target_id);

		ExecuteAnimation(_do_attack_animation, &params);
	}
	else
	{
		Finish();
	}
}

void CMeleeAttackEvent::OnAttackAnimSuccess(uint32_t motion)
{
	Finish();
}

void CMeleeAttackEvent::Finish()
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	Done();
}

void CMeleeAttackEvent::HandleAttackHook(const AttackCone &cone)
{
	CWeenieObject *target = GetTarget();

	if (!target || !IsValidTarget())
	{
		return;
	}

	int preVarianceDamage = 0;
	float variance = 0.0f;

	DAMAGE_TYPE damageType = DAMAGE_TYPE::UNDEF_DAMAGE_TYPE;

	bool isBodyPart = false;
	CWeenieObject *weapon = _weenie->GetWieldedCombat(_combat_style);

	if (!weapon || weapon->GetAttackDamage() == 0) //if we still don't have a weapon use our body parts
	{
		weapon = _weenie;
		if (_weenie->m_Qualities._body)
		{
			BodyPart *part = _weenie->m_Qualities._body->_body_part_table.lookup(cone.part_index);
			if (part)
			{
				isBodyPart = true;
				damageType = part->_dtype;
				preVarianceDamage = part->_dval;
				variance = part->_dvar;
			}
		}

		CWeenieObject *gloverOrBoots;
		if (_attack_power >= 0.75f) //this is a kick
			gloverOrBoots = _weenie->GetWielded(FOOT_WEAR_LOC);
		else //this is a punch
			gloverOrBoots = _weenie->GetWielded(HAND_WEAR_LOC);

		if (gloverOrBoots)
		{
			damageType = (DAMAGE_TYPE)gloverOrBoots->InqIntQuality(DAMAGE_TYPE_INT, damageType);
			preVarianceDamage += gloverOrBoots->GetAttackDamage();
			variance = gloverOrBoots->InqFloatQuality(DAMAGE_VARIANCE_FLOAT, variance);
		}
	}

	if (!isBodyPart)
	{
		preVarianceDamage = weapon->GetAttackDamage();
		variance = weapon->InqFloatQuality(DAMAGE_VARIANCE_FLOAT, 0.0f);
		damageType = weapon->InqDamageType();
	}

	//todo: maybe handle this differently as to integrate all possible damage type combos
	if (damageType == (DAMAGE_TYPE::SLASH_DAMAGE_TYPE | DAMAGE_TYPE::PIERCE_DAMAGE_TYPE))
	{
		//UA weapons (not hands!) have a quirk that high attack heights use the slashing version, always. Reason: The punching animation
		//uses the left hand, but the katar/cestus/etc. is in the _RIGHT_ hand. It looks silly, so fix it up here by letting lowest power
		//attacks use slashing.
		const CombatStyle cs = (CombatStyle)weapon->m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, 0);

		// Damage type should always be Pierce for multi-strike Thrust animations, not slashing.
		if ((cs == Unarmed_CombatStyle && _attack_height == HIGH_ATTACK_HEIGHT) ||
			_attack_power >= 0.25f && !(_do_attack_animation >= Motion_DoubleThrustLow && _do_attack_animation <= Motion_TripleThrustHigh))
			damageType = DAMAGE_TYPE::SLASH_DAMAGE_TYPE;
		else
			damageType = DAMAGE_TYPE::PIERCE_DAMAGE_TYPE;
	}
	else if (damageType == (DAMAGE_TYPE::SLASH_DAMAGE_TYPE | DAMAGE_TYPE::FIRE_DAMAGE_TYPE))
	{
		//todo: as far as I know only the Mattekar Claw had this, figure out what it did exactly, was it like this? or was it a bit of both damages?
		//or even a chance for fire damage?
		if (_attack_power >= 0.25f)
			damageType = DAMAGE_TYPE::SLASH_DAMAGE_TYPE;
		else
			damageType = DAMAGE_TYPE::FIRE_DAMAGE_TYPE;
	}

	CWeenieObject *shield = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_SHIELD);

	int burden = 0;
	if (weapon != NULL && weapon != _weenie)
		burden += weapon->InqIntQuality(ENCUMB_VAL_INT, 0);

	if (shield != NULL)
		burden += shield->InqIntQuality(ENCUMB_VAL_INT, 0);

	float necessaryStaminaFloat;
	if (_attack_power < 0.33)
		necessaryStaminaFloat = max(burden / 900.0f, 1.0f);
	else if (_attack_power < 0.66)
		necessaryStaminaFloat = max(burden / 600.0f, 1.0f);
	else
		necessaryStaminaFloat = max(burden / 300.0f, 1.0f);

	if (_weenie->AsPlayer())
	{
		//the higher a player's Endurance, the less stamina one uses while attacking. 
		//This benefit is tied to Endurance only, and it caps out at around 50% less stamina used per attack. 
		//The minimum stamina used per attack remains one. 
		uint32_t endurance = 0;
		float necStamMod = 1.0f;
		_weenie->m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);

		if (endurance >= 50)
		{
			necStamMod = ((float)(endurance * endurance) * -0.000003175f) - ((float)endurance * 0.0008889f) + 1.052f;
			necStamMod = min(max(necStamMod, 0.5f), 1.0f);
			necessaryStaminaFloat = necessaryStaminaFloat * necStamMod + Random::RollDice(0.0f, 1.0f); // little sprinkle of luck 
		}
	}
	int necessaryStam = max(1, (int)round(necessaryStaminaFloat));

	bool hadEnoughStamina = true;
	if (_weenie->GetStamina() < necessaryStam)
	{
		hadEnoughStamina = false;
		_attack_power = 0.00f;
		_weenie->SetStamina(0, true); // you lose all current stam
	}
	else
		_weenie->AdjustStamina(-necessaryStam);

	STypeSkill weaponSkill = STypeSkill::UNDEF_SKILL;
	uint32_t weaponSkillLevel = 0;
	CalculateAtt(weapon, weaponSkill, weaponSkillLevel);

	if (!hadEnoughStamina)
	{
		if (CPlayerWeenie *pPlayer = _weenie->AsPlayer())
			pPlayer->SendText("You're exhausted!", LTT_ERROR);
		weaponSkillLevel *= 0.5; //50% penalty to our attack skill when we don't have enough to perform it.
	}

	DamageEventData dmgEvent;
	dmgEvent.source = _weenie;
	dmgEvent.weapon = weapon;
	dmgEvent.damage_form = DF_MELEE;
	dmgEvent.damage_type = damageType;
	dmgEvent.attackSkill = weaponSkill;
	dmgEvent.attackSkillLevel = weaponSkillLevel;
	dmgEvent.preVarianceDamage = preVarianceDamage;
	dmgEvent.variance = variance;

	HandlePerformAttack(target, dmgEvent);

	int cleaveTargets = weapon->InqIntQuality(CLEAVING_INT, 1) - 1;
	if (cleaveTargets)
	{
		std::list<CWeenieObject *> lpNearby;
		g_pWorld->EnumNearby(dmgEvent.source, _max_attack_distance, &lpNearby);

		int numTargets = cleaveTargets;
		for (auto tg : lpNearby)
		{
			if (!numTargets)
				break;

			if (tg == target)
				continue;

			if (_weenie->m_Qualities.id != 1 && tg->m_Qualities.id != 1) // Don't cleave mobs if we are a mob. Where 1 is the WCID for a player (always 1).
				continue;

			if (!tg->IsAttackable() || (tg->_IsPlayer() && _weenie->_IsPlayer() && ((!_weenie->IsPK() || !tg->IsPK()) && (!_weenie->IsPKLite() || !tg->IsPKLite()))))
				continue;

			if (tg->m_Qualities.m_WeenieType == CombatPet_WeenieType)
				continue;

			if (tg->HeadingFrom(_weenie, true) < CLEAVING_ATTACK_ANGLE)
			{
				HandlePerformAttack(tg, dmgEvent);
				dmgEvent.killingBlow = false;
				numTargets--;
			}
		}
	}
}

void CMeleeAttackEvent::HandlePerformAttack(CWeenieObject *target, DamageEventData dmgEvent)
{
	CPlayerWeenie* playerAttacker = _weenie->AsPlayer();
	CPlayerWeenie* playerDefender = target->AsPlayer();

	// okay, we're attacking. check for pvp interactions
	if (playerDefender && playerAttacker)
	{
		playerDefender->UpdatePKActivity();
		playerAttacker->UpdatePKActivity();
	}

	target->m_Qualities.SetInstanceID(CURRENT_ATTACKER_IID, _weenie->GetID());

	uint32_t currentEnemy;
	if (target->m_Qualities.InqInstanceID(CURRENT_ENEMY_IID, currentEnemy))
	{
		if (currentEnemy == 0)
			target->m_Qualities.SetInstanceID(CURRENT_ENEMY_IID, _weenie->GetID());
	}

	if (playerAttacker)
		playerAttacker->CancelLifestoneProtection();

	uint32_t meleeDefense = 0;
	uint32_t evasionDifficulty = 0;
	if (target->InqSkill(MELEE_DEFENSE_SKILL, meleeDefense, FALSE) && meleeDefense > 0)
	{
		if (target->TryAttackEvade(dmgEvent.attackSkillLevel, STypeSkill::MELEE_DEFENSE_SKILL, &evasionDifficulty))
		{
			target->OnEvadeAttack(_weenie);

			// send evasion message
			BinaryWriter attackerEvadeEvent;
			attackerEvadeEvent.Write<uint32_t>(0x01B3);
			attackerEvadeEvent.WriteString(target->GetName());
			_weenie->SendNetMessage(&attackerEvadeEvent, PRIVATE_MSG, TRUE, FALSE);

			BinaryWriter attackedEvadeEvent;
			attackedEvadeEvent.Write<uint32_t>(0x01B4);
			attackedEvadeEvent.WriteString(_weenie->GetName());
			target->SendNetMessage(&attackedEvadeEvent, PRIVATE_MSG, TRUE, FALSE);

			if (_weenie->m_Qualities.m_WeenieType == WeenieType::CombatPet_WeenieType && _weenie->AsMonster()->ShowCombatDamage())
			{
				CPlayerWeenie* owner = g_pWorld->FindPlayer(_weenie->m_Qualities.GetIID(PET_OWNER_IID, 0));
				if (owner && owner->AsPlayer())
				{
					owner->AsPlayer()->SendText(csprintf("Your pet missed %s.", target->GetName().c_str()), LTT_COMBAT);
				}
			}
			if (_weenie && _weenie->AsPlayer())
				_weenie->AsPlayer()->HandleAetheriaProc(0);
			return;
		}
	}

	if(playerAttacker) //Grant attacker some XP in their skill
		playerAttacker->MaybeGiveSkillUsageXP((STypeSkill)dmgEvent.attackSkill, evasionDifficulty);


	DAMAGE_QUADRANT hitQuadrant = DAMAGE_QUADRANT::DQ_UNDEF;
	switch (_attack_height)
	{
	case HIGH_ATTACK_HEIGHT:
		hitQuadrant = DAMAGE_QUADRANT::DQ_HIGH;
		break;
	default:
	case MEDIUM_ATTACK_HEIGHT:
		hitQuadrant = DAMAGE_QUADRANT::DQ_MEDIUM;
		break;
	case LOW_ATTACK_HEIGHT:
		hitQuadrant = DAMAGE_QUADRANT::DQ_LOW;
		break;
	}

	double angle = _weenie->HeadingFrom(_target_id, false);
	if (angle >= 0 && angle < 180)
		hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_RIGHT);
	else
		hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_LEFT);

	if (angle >= 90 && angle < 270)
		hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_BACK);
	else
		hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_FRONT);

	dmgEvent.hit_quadrant = hitQuadrant;

	dmgEvent.target = target;

	CalculateCriticalHitData(&dmgEvent, NULL);
	dmgEvent.wasCrit = (Random::GenFloat(0.0, 1.0) < dmgEvent.critChance) ? true : false;

	if (dmgEvent.wasCrit)
		dmgEvent.baseDamage = dmgEvent.preVarianceDamage * (0.5 + _attack_power);//Calculate baseDamage with no variance (uses max dmg on weapon)

	else
		dmgEvent.baseDamage = dmgEvent.preVarianceDamage * (1.0f - Random::GenFloat(0.0f, dmgEvent.variance)) * (0.5 + _attack_power); // not a crit so include variance in base damage

	//cast on strike
	if (dmgEvent.weapon->InqDIDQuality(PROC_SPELL_DID, 0))
	{
		double procChance = dmgEvent.weapon->InqFloatQuality(PROC_SPELL_RATE_FLOAT, 0.0f);

		bool proc = (Random::GenFloat(0.0, 1.0) < procChance) ? true : false;

		if (proc && target)
		{
			uint32_t targetid = target->GetID();
			uint32_t procspell = dmgEvent.weapon->InqDIDQuality(PROC_SPELL_DID, 0);

			dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, procspell);
		}
	}

	if (_weenie && _weenie->AsPlayer())
		_weenie->AsPlayer()->HandleAetheriaProc(target->GetID());

	//uint32_t dirtyFighting = 0;
	//if (_weenie->AsPlayer() && _weenie->InqSkill(DIRTY_FIGHTING_SKILL, dirtyFighting, FALSE) && dirtyFighting > 0)
	//{
	//	double skillCalc = (double)dirtyFighting / (double)dmgEvent.attackSkillLevel;
	//	double chanceOfDfEffect = .25 * min(1.0, skillCalc);
	//	double procRoll = Random::GenFloat(0.0, 1.0);
	//	bool proc = (procRoll < chanceOfDfEffect) ? true : false;
	//	SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
	//	bool hasQuality = _weenie->m_Qualities.InqSkillAdvancementClass((STypeSkill)DIRTY_FIGHTING_SKILL, sac);
	//	uint32_t targetid = target->GetID();
	//	//_weenie->AsPlayer()->SendText(csprintf("Attack Skill: %u  DF Skill: %u  Skill Calc: %f Proc %f", dmgEvent.attackSkillLevel, dirtyFighting, skillCalc, procRoll), LTT_COMBAT);
	//	if (proc && sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
	//	{
	//		if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_HIGH)
	//		{
	//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	//			{
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5938); //Blinding Assault
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5941); //Traumatic Assault
	//			}
	//			else
	//			{
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5942); //Blinding Blow
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5945); //Traumatic Blow
	//			}
	//		}
	//		else if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_MEDIUM)
	//		{
	//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	//			{
	//				dmgEvent.source->MakeSpellcastingManager()->CastSpellInstant(targetid, 5939);
	//				//dmgEvent.weapon->TryCastSpell(targetid, 5939); //Bleeding Assault
	//			}
	//			else
	//			{
	//				dmgEvent.source->MakeSpellcastingManager()->CastSpellInstant(targetid, 5943);
	//				//dmgEvent.weapon->TryCastSpell(targetid, 5943); //Bleeding Blow
	//			}
	//		}
	//		else if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_LOW)
	//		{
	//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
	//			{
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5940); //Unbalancing Assault
	//			}
	//			else
	//			{
	//				dmgEvent.weapon->MakeSpellcastingManager()->CastSpellInstant(targetid, 5944); //Unbalancing Blow
	//			}
	//		}
	//		else
	//		{
	//			SERVER_ERROR << "Unknown error on Dirty Fighting";
	//		}
	//	}
	//}

	CalculateAttackConditions(&dmgEvent, _attack_power, angle);
	CalculateDamage(&dmgEvent);
	//if (dmgEvent.source->m_Qualities.m_WeenieType == CombatPet_WeenieType)
	//{
	//	CWeenieObject* owner = g_pWorld->FindPlayer(dmgEvent.source->m_Qualities.GetIID(PET_OWNER_IID, 0));
	//	if (owner)
	//	{
	//		owner->AsPlayer()->SendText(csprintf("Your Pet does %u damage to %s.", (uint32_t)ceil(dmgEvent.damageAfterMitigation), dmgEvent.target->GetName().c_str()), LTT_COMBAT);
	//	}

	//}
	_weenie->TryToDealDamage(dmgEvent);
}
