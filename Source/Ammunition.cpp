
#include "StdAfx.h"
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

		if (!pHit->ImmuneToDamage(pSource))
		{
			targetCollision = true;
			
			int preVarianceDamage;
			float variance;

			CWeenieObject *weapon = g_pWorld->FindObject(_launcherID);
			if (weapon)
			{		
				bool bEvaded = false;

				DWORD missileDefense = 0;
				if (pHit->InqSkill(MISSILE_DEFENSE_SKILL, missileDefense, FALSE) && missileDefense > 0)
				{
					if (pHit->TryMissileEvade((DWORD)(_weaponSkillLevel * (_attackPower + 0.5f))))
					{
						pHit->OnEvadeAttack(pSource);

						// send evasion message
						BinaryWriter attackerEvadeEvent;
						attackerEvadeEvent.Write<DWORD>(0x01B3);
						attackerEvadeEvent.WriteString(pHit->GetName());
						pSource->SendNetMessage(&attackerEvadeEvent, PRIVATE_MSG, TRUE, FALSE);

						BinaryWriter attackedEvadeEvent;
						attackedEvadeEvent.Write<DWORD>(0x01B4);
						attackedEvadeEvent.WriteString(pSource->GetName());
						pHit->SendNetMessage(&attackedEvadeEvent, PRIVATE_MSG, TRUE, FALSE);
						bEvaded = true;
					}
				}

				if (!bEvaded)
				{
					EmitSound(Sound_Collision, 1.0f);

					// todo: do this in a better way?
					// 50% medium, 30% low, 20% high
					DAMAGE_QUADRANT hitQuadrant = DAMAGE_QUADRANT::DQ_UNDEF;
					double roll = Random::RollDice(0.0, 1.0);
					if(roll < 50.0)
						hitQuadrant = DQ_MEDIUM;
					else if (roll < 80.0)
						hitQuadrant = DQ_LOW;
					else
						hitQuadrant = DQ_HIGH;

					double angle = HeadingFrom(pHit, false);
					if (angle <= 45)
						hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_FRONT);
					else if (angle > 45 && angle <= 135)
						hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_RIGHT);
					else if (angle > 135 && angle <= 225)
						hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_BACK);
					else if (angle > 225 && angle <= 315)
						hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_LEFT);
					else
						hitQuadrant = (DAMAGE_QUADRANT)(hitQuadrant | DAMAGE_QUADRANT::DQ_FRONT);

					preVarianceDamage = GetAttackDamage();
					variance = InqFloatQuality(DAMAGE_VARIANCE_FLOAT, 0.0f);

					bool isThrownWeapon = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == ThrownWeapon_CombatStyle);
					int weaponDamage = !isThrownWeapon ? weapon->GetAttackDamage() : 0;
					int elementalDamageBonus = weapon->InqDamageType() == InqDamageType() ? weapon->InqIntQuality(ELEMENTAL_DAMAGE_BONUS_INT, 0) : 0;
					double damageMod = weapon->InqFloatQuality(DAMAGE_MOD_FLOAT, 1.0);

					preVarianceDamage += weaponDamage + elementalDamageBonus;
					preVarianceDamage *= damageMod;

					DamageEventData dmgEvent;
					dmgEvent.source = pSource;
					dmgEvent.target = pHit;
					dmgEvent.weapon = weapon;
					dmgEvent.damage_form = DF_MISSILE;
					dmgEvent.damage_type = InqDamageType();
					dmgEvent.hit_quadrant = hitQuadrant;
					dmgEvent.attackSkill = _weaponSkill;
					dmgEvent.attackSkillLevel = _weaponSkillLevel;
					dmgEvent.preVarianceDamage = preVarianceDamage;
					dmgEvent.baseDamage = preVarianceDamage * (1.0f - Random::GenFloat(0.0f, variance));

					CalculateDamage(&dmgEvent);

					pSource->TryToDealDamage(dmgEvent);
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
	HandleNonTargetCollision();
	return CWeenieObject::DoCollision(prof);
}

void CAmmunitionWeenie::DoCollisionEnd(DWORD object_id)
{
	CWeenieObject::DoCollisionEnd(object_id);
}





