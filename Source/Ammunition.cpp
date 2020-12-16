#include <StdAfx.h>
#include "WeenieObject.h"
#include "Ammunition.h"
#include "Player.h"
#include "World.h"
#include "CombatFormulas.h"

CAmmunitionWeenie::CAmmunitionWeenie()
{
}

CAmmunitionWeenie::~CAmmunitionWeenie()
{
}

void CAmmunitionWeenie::ApplyQualityOverrides()
{
	CWeenieObject::ApplyQualityOverrides();
}

void CAmmunitionWeenie::MakeIntoMissile()
{
	int stackSize = 0;
	if (m_Qualities.InqInt(STACK_SIZE_INT, stackSize))
	{
		SetStackSize(1);
	}

	m_Qualities.SetInstanceID(WIELDER_IID, 0);
	m_Qualities.SetInstanceID(CONTAINER_IID, 0);
	_cachedHasOwner = false;

	m_Qualities.SetInt(CURRENT_WIELDED_LOCATION_INT, 0);
	m_Qualities.SetInt(PARENT_LOCATION_INT, 0);

	m_PhysicsState = INELASTIC_PS | GRAVITY_PS | PATHCLIPPED_PS | ALIGNPATH_PS | MISSILE_PS | REPORT_COLLISIONS_PS;
	SetInitialPhysicsState(m_PhysicsState);

	SetPlacementFrame(Placement::MissileFlight, FALSE);
}

void CAmmunitionWeenie::MakeIntoAmmo()
{
	set_state(INELASTIC_PS | GRAVITY_PS | IGNORE_COLLISIONS_PS | ETHEREAL_PS, m_bWorldIsAware ? TRUE : FALSE);
	SetInitialPhysicsState(m_PhysicsState);

	SetPlacementFrame(Placement::Resting, TRUE);

	_timeToRot = Timer::cur_time + 60.0;
	_beganRot = false;
	m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _timeToRot);
}

void CAmmunitionWeenie::PostSpawn()
{
	CWeenieObject::PostSpawn();

	if (m_PhysicsState & MISSILE_PS)
	{
		EmitEffect(PS_Launch, 0.0f);
	}
}

void CAmmunitionWeenie::HandleNonTargetCollision()
{
	Movement_UpdatePos();
	MakeIntoAmmo();
	_targetID = 0;

	if (CWeenieObject *source = g_pWorld->FindObject(_sourceID))
	{
		if (source->AsPlayer())
			source->SendText("Your missile attack hit the environment.", LTT_DEFAULT);
		else
			MarkForDestroy();

		EmitSound(Sound_Collision, 1.0f);
	}

	// set the collision angle to be the heading of the object?
}

void CAmmunitionWeenie::HandleTargetCollision()
{
	MarkForDestroy();
}

BOOL CAmmunitionWeenie::DoCollision(const class EnvCollisionProfile &prof)
{
	HandleNonTargetCollision();
	return CWeenieObject::DoCollision(prof);
}

BOOL CAmmunitionWeenie::DoCollision(const class AtkCollisionProfile &prof)
{
	bool targetCollision = false;

	CWeenieObject *pHit = g_pWorld->FindWithinPVS(this, prof.id);
	if (pHit && (!_targetID || _targetID == pHit->GetID()) && (pHit->GetID() != _sourceID) && (pHit->GetID() != _launcherID))
	{
		CWeenieObject *pSource = g_pWorld->FindObject(_sourceID);

		pHit->m_Qualities.SetInstanceID(CURRENT_ATTACKER_IID, _sourceID);

		uint32_t currentEnemy;
		if (pHit->m_Qualities.InqInstanceID(CURRENT_ENEMY_IID, currentEnemy))
		{
			if(currentEnemy == 0)
				pHit->m_Qualities.SetInstanceID(CURRENT_ENEMY_IID, _sourceID);
		}

		if (!pHit->ImmuneToDamage(pSource))
		{
			targetCollision = true;
			
			int preVarianceDamage;
			float variance;

			CWeenieObject *weapon = g_pWorld->FindObject(_launcherID);

			// For thrown weaps, the weapon no longer exists on the last missile in a stack. So for these we will just use the cloned ammo weenie itself.
			if (!weapon && m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, 0) == ThrownWeapon_CombatStyle)
				weapon = g_pWorld->FindObject(id);

			if (weapon)
			{		
				bool bEvaded = false;

				uint32_t missileDefense = 0;
				uint32_t skill_level = 0;
				if (pHit->InqSkill(MISSILE_DEFENSE_SKILL, missileDefense, FALSE) && missileDefense > 0)
				{
					if (pHit->TryAttackEvade((uint32_t)(_weaponSkillLevel * (_attackPower + 0.5f)), STypeSkill::MISSILE_DEFENSE_SKILL, &skill_level))
					{
						pHit->OnEvadeAttack(pSource);

						// send evasion message
						BinaryWriter attackerEvadeEvent;
						attackerEvadeEvent.Write<uint32_t>(0x01B3);
						attackerEvadeEvent.WriteString(pHit->GetName());
						if (pSource != nullptr)
							pSource->SendNetMessage(&attackerEvadeEvent, PRIVATE_MSG, TRUE, FALSE);


						BinaryWriter attackedEvadeEvent;
						attackedEvadeEvent.Write<uint32_t>(0x01B4);
						if (pSource != nullptr)
							attackedEvadeEvent.WriteString(pSource->GetName());
						else
							attackedEvadeEvent.WriteString("Unknown");
						pHit->SendNetMessage(&attackedEvadeEvent, PRIVATE_MSG, TRUE, FALSE);
						bEvaded = true;
					}
				}

				if (!bEvaded)
				{
					EmitSound(Sound_Collision, 1.0f);

					if (pSource != nullptr && pSource && pHit && pSource->AsPlayer() && pHit->AsPlayer())
					{
						pSource->AsPlayer()->UpdatePKActivity();
						pHit->AsPlayer()->UpdatePKActivity();
					}

					bool isThrownWeapon = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == ThrownWeapon_CombatStyle);

					preVarianceDamage = isThrownWeapon ? weapon->GetAttackDamage() : GetAttackDamage();
					variance = InqFloatQuality(DAMAGE_VARIANCE_FLOAT, 0.0f);

					//int elementalDamageBonus = weapon->InqDamageType() == InqDamageType() || InqDamageType() == BASE_DAMAGE_TYPE ? weapon->InqIntQuality(ELEMENTAL_DAMAGE_BONUS_INT, 0) : 0;
					double damageMod = weapon->InqFloatQuality(DAMAGE_MOD_FLOAT, 1.0);

					//preVarianceDamage += elementalDamageBonus;
					preVarianceDamage *= damageMod;

					DamageEventData dmgEvent;
					dmgEvent.source = pSource;
					dmgEvent.target = pHit;
					dmgEvent.weapon = weapon;
					dmgEvent.damage_form = DF_MISSILE;

					if (InqDamageType() != BASE_DAMAGE_TYPE)
						dmgEvent.damage_type = InqDamageType();
					else if (!weapon->InqDamageType())
						dmgEvent.damage_type = PIERCE_DAMAGE_TYPE;
					else
						dmgEvent.damage_type = weapon->InqDamageType();

					dmgEvent.hit_quadrant = (DAMAGE_QUADRANT)prof.location;
					dmgEvent.attackSkill = _weaponSkill;
					dmgEvent.attackSkillLevel = _weaponSkillLevel;
					dmgEvent.preVarianceDamage = preVarianceDamage;
					dmgEvent.baseDamage = preVarianceDamage * (1.0f - Random::GenFloat(0.0f, variance));

					CalculateCriticalHitData(&dmgEvent, NULL);
					dmgEvent.wasCrit = (Random::GenFloat(0.0, 1.0) < dmgEvent.critChance) ? true : false;
					if (dmgEvent.wasCrit)
					{
						dmgEvent.baseDamage = dmgEvent.preVarianceDamage;//Recalculate baseDamage with no variance (uses max dmg on weapon)
					}
                    
					CalculateAttackConditions(&dmgEvent, _attackPower, HeadingFrom(pHit, false));

					//cast on strike
					if (weapon->InqDIDQuality(PROC_SPELL_DID, 0))
					{
						double procChance = weapon->InqFloatQuality(PROC_SPELL_RATE_FLOAT, 0.0f);

						bool proc = (Random::GenFloat(0.0, 1.0) < procChance) ? true : false;

						if (proc && _targetID)
						{
							uint32_t targetid = _targetID;
							uint32_t procspell = weapon->InqDIDQuality(PROC_SPELL_DID, 0);

							weapon->TryCastSpell(targetid, procspell);
						}
					}

					if (pSource && pSource->AsPlayer())
						pSource->AsPlayer()->HandleAetheriaProc(pHit->GetID());

					//uint32_t dirtyFighting = 0;
					//if (pSource && pSource->AsPlayer() && pSource->InqSkill(DIRTY_FIGHTING_SKILL, dirtyFighting, FALSE) && dirtyFighting > 0)
					//{
					//	double skillCalc = (double)dirtyFighting / (double)dmgEvent.attackSkillLevel;
					//	double chanceOfDfEffect = .25 * min(1.0, skillCalc);
					//	double procRoll = Random::GenFloat(0.0, 1.0);
					//	bool proc = (procRoll < chanceOfDfEffect) ? true : false;
					//	SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
					//	bool hasQuality = pSource->m_Qualities.InqSkillAdvancementClass((STypeSkill)DIRTY_FIGHTING_SKILL, sac);
					//	uint32_t targetid = pHit->GetID();
					//	//_weenie->AsPlayer()->SendText(csprintf("Attack Skill: %u  DF Skill: %u  Skill Calc: %f Proc %f", dmgEvent.attackSkillLevel, dirtyFighting, skillCalc, procRoll), LTT_COMBAT);
					//	if (proc)
					//	{
					//		if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_HIGH)
					//		{
					//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5938); //Blinding Assault
					//				dmgEvent.weapon->TryCastSpell(targetid, 5941); //Traumatic Assault
					//			}
					//			else
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5942); //Blinding Blow
					//				dmgEvent.weapon->TryCastSpell(targetid, 5945); //Traumatic Blow
					//			}
					//		}
					//		else if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_MEDIUM)
					//		{
					//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5939); //Bleeding Assault
					//			}
					//			else
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5943); //Bleeding Blow
					//			}
					//		}
					//		else if (dmgEvent.hit_quadrant & DAMAGE_QUADRANT::DQ_LOW)
					//		{
					//			if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5940); //Unbalancing Assault
					//			}
					//			else
					//			{
					//				dmgEvent.weapon->TryCastSpell(targetid, 5944); //Unbalancing Blow
					//			}
					//		}
					//		else
					//		{
					//			SERVER_ERROR << "Unknown error on Dirty Fighting";
					//		}
					//	}
					//}


					CalculateDamage(&dmgEvent);
					if (pSource != nullptr)
					{
						pSource->TryToDealDamage(dmgEvent);
						if(pSource->AsPlayer())
							pSource->AsPlayer()->MaybeGiveSkillUsageXP((STypeSkill)dmgEvent.attackSkill, skill_level);
					}
				}
			}
		}
	}

	if (targetCollision)
		HandleTargetCollision();
	else
		HandleNonTargetCollision();

	return CWeenieObject::DoCollision(prof);
}

BOOL CAmmunitionWeenie::DoCollision(const class ObjCollisionProfile &prof)
{

	//Sometimes instead of arrow colliding with monster, we see monster colliding with arrow.
	//This produces TWO collisions. Ignore object collision when obj == target.
	if(prof.id == this->_targetID) {
		return FALSE;
	}

	HandleNonTargetCollision();
	return CWeenieObject::DoCollision(prof);
}

void CAmmunitionWeenie::DoCollisionEnd(uint32_t object_id)
{
	CWeenieObject::DoCollisionEnd(object_id);
}





