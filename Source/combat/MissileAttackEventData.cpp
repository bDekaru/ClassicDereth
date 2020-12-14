#include <StdAfx.h>
#include "AttackManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "Player.h"
#include "WeenieFactory.h"
#include "Ammunition.h"
#include "CombatFormulas.h"
#include "combat/MissileAttackEventData.h"

float CMissileAttackEvent::CalculateDef()
{
	CWeenieObject *weapon = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_MISSILE);
	if (weapon)
	{
		float defenseMod = weapon->GetMeleeDefenseMod();
		return defenseMod;
	}

	return CAttackEventData::CalculateDef();
}

void CMissileAttackEvent::Setup()
{
	if (!_weenie)
	{
		return;
	}

	if (_attack_charge_time >= 0.0)
	{
		_attack_charge_time += 1; // for reload animation
	}

	uint32_t attack_motion = 0;
	uint32_t weapon_id = 0;

	if (!_do_attack_animation)
	{
		switch (_attack_height)
		{
		case LOW_ATTACK_HEIGHT: attack_motion = Motion_AimLevel; break;
		case MEDIUM_ATTACK_HEIGHT: attack_motion = Motion_AimLevel; break;
		case HIGH_ATTACK_HEIGHT: attack_motion = Motion_AimLevel; break;
		default:
		{
			Cancel();
			return;
		}
		}

		if (_attack_power < 0.0f || _attack_power > 1.0f)
		{
			Cancel();
			return;
		}
		_do_attack_animation = attack_motion;
	}

	uint32_t quickness = 0;
	_weenie->m_Qualities.InqAttribute(QUICKNESS_ATTRIBUTE, quickness, FALSE);

	int weaponAttackTime = _weenie->GetAttackTimeUsingWielded();
	int creatureAttackTime = max(0, 120 - (((int)quickness - 60) / 2)); //we reach 0 attack speed at 300 quickness

	int attackTime = (creatureAttackTime + weaponAttackTime) / 2; //our attack time is the average between our speed and the speed of our weapon.
	attackTime = max(0, min(120, attackTime));

	_attack_speed = 2.5f - (attackTime * (1.0 / 70.0));
	_attack_speed = max(min(_attack_speed, 2.5f), 0.8f);

	_max_attack_distance = 80.0;
	_max_attack_angle = MAX_MISSILE_ATTACK_CONE_ANGLE;
	_timeout = Timer::cur_time + 15.0;
	m_bTurned = false;
}

void CMissileAttackEvent::PostCharge()
{
	_attack_charge_time = -1.0;

	if ((_max_attack_distance + F_EPSILON) < DistanceToTarget())
	{
		// out of range so just stop
		if (auto pPlayer = _weenie->AsPlayer())
		{
			pPlayer->NotifyAttackDone();
			pPlayer->NotifyWeenieError(0x550);
		}
	}

	if (m_bTurned || InAttackCone())
	{
		OnReadyToAttack();
	}
	else
	{
		TurnToAttack();
		m_bTurned = true;
	}

}

void CMissileAttackEvent::OnReadyToAttack()
{
	if (_weenie->AsPlayer())
		_weenie->AsPlayer()->CancelLifestoneProtection();

	if (_do_attack_animation)
	{
		MovementParameters params;
		params.modify_interpreted_state = 1;
		params.speed = _attack_speed;
		params.autonomous = 0;

		CalculateAttackMotion();

		ExecuteAnimation(_do_attack_animation, &params);
	}
	else
	{
		Finish();
	}
}

void CMissileAttackEvent::CalculateAttackMotion()
{
	CWeenieObject *weapon = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_MISSILE);

	if (!weapon)
	{
		_weenie->DoForcedStopCompletely();
		return;
	}

	CWeenieObject *equippedAmmo;

	bool isThrownWeapon = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == ThrownWeapon_CombatStyle);
	bool isAtlatl = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == Atlatl_CombatStyle);

	if (isThrownWeapon)
		equippedAmmo = weapon;
	else
		equippedAmmo = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_AMMO);

	if (!equippedAmmo)
	{
		_weenie->DoForcedStopCompletely();
		_weenie->NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
		return;
	}

	CalculateTargetPosition();
	CalculateSpawnPosition(equippedAmmo->GetRadius());

	bool bTrack = false;
	float fSpeed = weapon->InqFloatQuality(MAXIMUM_VELOCITY_FLOAT, 20.0);
	if (CPlayerWeenie *pPlayer = _weenie->AsPlayer())
	{
		bTrack = pPlayer->GetCharacterOptions2() & LeadMissileTargets_CharacterOptions2;
		fSpeed *= pPlayer->GetCharacterOptions2() & UseFastMissiles_CharacterOptions2 ? MISSILE_FAST_SPEED : MISSILE_SLOW_SPEED;
	}
	else
	{
		fSpeed *= MISSILE_SLOW_SPEED;
	}

	CalculateMissileVelocity(bTrack, true, fSpeed);

	float fVertAngle = RAD2DEG(asin(_missile_velocity.z / _missile_velocity.magnitude()));

	int motions[] = { Motion_AimLevel, Motion_AimHigh15, Motion_AimHigh30, Motion_AimHigh45, Motion_AimHigh60, Motion_AimHigh75, Motion_AimHigh90, Motion_AimLow15, Motion_AimLow30, Motion_AimLow45, Motion_AimLow60, Motion_AimLow75, Motion_AimLow90 };
	int iMotionIndex = 0;

	if (fVertAngle > 7.5)
	{
		iMotionIndex = min((int)floor((fVertAngle + 7.55f) / 15.0f), 6);
	}
	else if (fVertAngle < -7.5)
	{
		iMotionIndex = -min((int)ceil((fVertAngle - 7.55f) / 15.0f), 6) + 6;
	}

	_do_attack_animation = motions[min(max(0, iMotionIndex), 12)];
}

bool CMissileAttackEvent::CalculateTargetPosition()
{
	CWeenieObject *target = GetTarget();

	if (!target || !target->InValidCell())
	{
		return false;
	}

	_missile_target_position = target->GetPosition();

	switch (_attack_height)
	{
	case ATTACK_HEIGHT::LOW_ATTACK_HEIGHT:
		_missile_target_position.frame.m_origin.z += target->GetHeight() * (1.0 / 6.0); // 0.25;
		break;

	default:
	case ATTACK_HEIGHT::MEDIUM_ATTACK_HEIGHT:
		_missile_target_position.frame.m_origin.z += target->GetHeight() * 0.5;
		break;

	case ATTACK_HEIGHT::HIGH_ATTACK_HEIGHT:
		_missile_target_position.frame.m_origin.z += target->GetHeight() * (5.0 / 6.0); // 0.75;
		break;
	}

	return true;
}

bool CMissileAttackEvent::CalculateSpawnPosition(float missileRadius)
{
	if (!_weenie->InValidCell())
	{
		return false;
	}

	_missile_spawn_position = _weenie->GetPosition();
	_missile_spawn_position = _missile_spawn_position.add_offset(Vector(0, 0, _weenie->GetHeight() * 0.75)); //(2.0 / 3.0))); // 0.75f));

	Vector targetOffset = _missile_spawn_position.get_offset(_missile_target_position);
	Vector targetDir = targetOffset;

	if (targetDir.normalize_check_small())
	{
		targetDir = _missile_spawn_position.frame.get_vector_heading();

		// spawnPosition.frame.m_origin += targetDir * minSpawnDist;
		_missile_spawn_position.frame.set_vector_heading(targetDir);
		_missile_dist_to_target = 0.0;
	}
	else
	{
		float minSpawnDist = (_weenie->GetRadius() + missileRadius) + 0.1f;

		_missile_spawn_position.frame.m_origin += targetDir * minSpawnDist;
		_missile_spawn_position.frame.set_vector_heading(targetDir);

		_missile_dist_to_target = targetOffset.magnitude();
	}

	return true;
}

bool CMissileAttackEvent::CalculateMissileVelocity(bool track, bool gravity, float speed)
{
	CWeenieObject *target = GetTarget();

	if (!target)
	{
		return false;
	}

	Vector targetOffset = _missile_spawn_position.get_offset(_missile_target_position);
	float targetDist = targetOffset.magnitude();

	if (!track)
	{
		float t = targetDist / speed;
		Vector v = targetOffset / t;

		if (gravity)
		{
			v.z += (9.8f * t) / 2.0f;
		}
		//Vector targetDir = v;
		//targetDir.normalize();

		_missile_velocity = v;

		return true;
	}

	Vector P0 = targetOffset;
	Vector P1(0, 0, 0);

	float s0 = target->get_velocity().magnitude();
	Vector V0 = target->get_velocity();
	if (V0.normalize_check_small())
	{
		V0 = Vector(0, 0, 0);
	}

	float s1 = speed;

	float a = (V0.x * V0.x) + (V0.y * V0.y) - (s1 * s1);
	float b = 2.0f * ((P0.x * V0.x) + (P0.y * V0.y) - (P1.x * V0.x) - (P1.y * V0.y));
	float c = (P0.x * P0.x) + (P0.y * P0.y) + (P1.x * P1.x) + (P1.y * P1.y) - (2.0f * P1.x * P0.x) - (2.0f * P1.y * P0.y);

	float t1 = (-b + sqrt((b * b) - (4.0f * a * c))) / (2.0f * a);
	float t2 = (-b - sqrt((b * b) - (4.0f * a * c))) / (2.0f * a);

	if (t1 < 0)
	{
		t1 = FLT_MAX;
	}

	if (t2 < 0)
	{
		t2 = FLT_MAX;
	}

	float t = min(t1, t2);
	if (t >= 100.0)
	{
		return CalculateMissileVelocity(false, true, speed);
	}

	Vector ts0(t * s0);

	Vector v = Vector::fma(ts0, V0, P0) / t;

	//v.x = (P0.x + (ts0 * V0.x)) / (t); // * s1);
	//v.y = (P0.y + (ts0 * V0.y)) / (t); // * s1);
	//v.z = (P0.z + (ts0 * V0.z)) / (t); // * s1);

	if (gravity)
	{
		// add z to velocity for gravity
		v.z += (9.8f * t) / 2.0f;
	}

	_missile_velocity = v;

	return true;
}

void CMissileAttackEvent::FireMissile()
{
	CWeenieObject *weapon = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_MISSILE);

	if (!weapon)
	{
		_weenie->DoForcedStopCompletely();
		return;
	}

	CWeenieObject *equippedAmmo;
	bool isThrownWeapon = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == ThrownWeapon_CombatStyle);
	bool isAtlatl = (weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) == Atlatl_CombatStyle);
	if (isThrownWeapon)
		equippedAmmo = weapon;
	else
		equippedAmmo = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_AMMO);

	if (!equippedAmmo)
	{
		_weenie->DoForcedStopCompletely();
		_weenie->NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
		return;
	}

	//This converts Generic_WeenieType items (plates, mugs, etc.) to Missile_WeenieType so that they can be cast as Ammunition below.
	if (isThrownWeapon && equippedAmmo->m_Qualities.m_WeenieType == Generic_WeenieType)
		equippedAmmo->m_Qualities.m_WeenieType = Missile_WeenieType;

	CWeenieObject *missileAsWeenieObject = g_pWeenieFactory->CloneWeenie(equippedAmmo);

	if (!missileAsWeenieObject)
	{
		_weenie->DoForcedStopCompletely();
		_weenie->NotifyWeenieError(WERROR_COMBAT_MISFIRE);
		return;
	}

	CAmmunitionWeenie *missile = missileAsWeenieObject->AsAmmunition();

	if (!missile)
	{
		_weenie->DoForcedStopCompletely();
		_weenie->NotifyWeenieError(WERROR_COMBAT_MISFIRE);
		SafeDelete(missileAsWeenieObject);
		return;
	}

	int stackSize = 0;
	if (missile->m_Qualities.InqInt(STACK_SIZE_INT, stackSize))
	{
		missile->SetStackSize(1);
	}

	missile->m_Qualities.SetInstanceID(WIELDER_IID, 0);
	missile->m_Qualities.SetInstanceID(CONTAINER_IID, 0);
	missile->_cachedHasOwner = false;

	missile->m_Qualities.SetInt(CURRENT_WIELDED_LOCATION_INT, 0);
	missile->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);

	missile->SetInitialPhysicsState(INELASTIC_PS | GRAVITY_PS | PATHCLIPPED_PS | ALIGNPATH_PS | MISSILE_PS | REPORT_COLLISIONS_PS);
	missile->SetInitialPosition(_weenie->GetPosition());
	missile->SetPlacementFrame(Placement::MissileFlight, FALSE);
	missile->InitPhysicsObj();

	CalculateTargetPosition();
	CalculateSpawnPosition(missile->GetRadius());

	//Set to non-tracking by default so mobs don't track
	bool bTrack = false;
	float fSpeed = weapon->InqFloatQuality(MAXIMUM_VELOCITY_FLOAT, 20.0);
	if (CPlayerWeenie *pPlayer = _weenie->AsPlayer())
	{
		bTrack = pPlayer->GetCharacterOptions2() & LeadMissileTargets_CharacterOptions2;
		fSpeed *= pPlayer->GetCharacterOptions2() & UseFastMissiles_CharacterOptions2 ? MISSILE_FAST_SPEED : MISSILE_SLOW_SPEED;
	}
	else
	{
		fSpeed *= MISSILE_SLOW_SPEED;
	}

	CalculateMissileVelocity(bTrack, true, fSpeed);

	missile->m_Position = _missile_spawn_position;
	missile->set_velocity(_missile_velocity, FALSE);

	CWeenieObject *launcher = _weenie->GetWieldedCombat(COMBAT_USE_MISSILE);
	CWeenieObject *target = GetTarget();
	missile->_sourceID = _weenie->GetID();
	missile->_launcherID = launcher ? launcher->GetID() : 0;
	missile->_targetID = target ? target->GetID() : 0;
	missile->_attackPower = _attack_power;
	missile->_timeToRot = Timer::cur_time + 5.0;

	CWeenieObject *shield = _weenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_SHIELD); //thrown weapons users can have a shield

	int burden = weapon->InqIntQuality(ENCUMB_VAL_INT, 0);
	if (shield != NULL)
		burden += shield->InqIntQuality(ENCUMB_VAL_INT, 0);

	int necessaryStamina;
	if (_attack_power < 0.33)
		necessaryStamina = max((int)round(burden / 900.0f), 1);
	else if (_attack_power < 0.66)
		necessaryStamina = max((int)round(burden / 600.0f), 1);
	else
		necessaryStamina = max((int)round(burden / 300.0f), 1);

	if (_weenie->AsPlayer())
	{
		//the higher a player's Endurance, the less stamina one uses while attacking. 
		//This benefit is tied to Endurance only, and it caps out at around 50% less stamina used per attack. 
		//The minimum stamina used per attack remains one. 

		uint32_t endurance = 0;
		_weenie->m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
		float necessaryStaminaMod = 1.0 - ((float)endurance - 100.0) / 600.0; //made up formula: 50% reduction at 400 endurance.
		necessaryStaminaMod = min(max(necessaryStaminaMod, 0.5f), 1.0f);
		necessaryStamina = round((float)necessaryStamina * necessaryStaminaMod);
	}
	necessaryStamina = max(necessaryStamina, 1);

	bool hadEnoughStamina = true;
	if (_weenie->GetStamina() < necessaryStamina)
		hadEnoughStamina = false;

	_weenie->AdjustStamina(-necessaryStamina);

	missile->_weaponSkill = (STypeSkill)weapon->InqIntQuality(WEAPON_SKILL_INT, UNDEF_SKILL, false);
	if (_weenie->InqSkill(missile->_weaponSkill, missile->_weaponSkillLevel, false))
	{
		//Missile weapons don't get offense mods.
		//double offenseMod = weapon->GetOffenseMod();
		//missile->_weaponSkillLevel = (uint32_t)(missile->_weaponSkillLevel * offenseMod);

		if (!hadEnoughStamina)
		{
			_weenie->NotifyWeenieError(WERROR_STAMINA_TOO_LOW);
			missile->_weaponSkillLevel *= 0.5; //50% penalty to our attack skill when we don't have enough to perform it.
		}
	}

	if (!g_pWorld->CreateEntity(missile))
	{
		_weenie->DoForcedStopCompletely();
		_weenie->NotifyWeenieError(WERROR_COMBAT_MISFIRE);
		return;
	}

	if (!isAtlatl)
	{
		int ammoType = weapon->InqIntQuality(AMMO_TYPE_INT, 0);

		switch (ammoType)
		{
		case AMMO_TYPE::AMMO_ARROW:
		case AMMO_TYPE::AMMO_ARROW_CHORIZITE:
		case AMMO_TYPE::AMMO_ARROW_CRYSTAL:
			_weenie->EmitSound(Sound_BowRelease, 1.0f);
			break;

		case AMMO_TYPE::AMMO_BOLT:
		case AMMO_TYPE::AMMO_BOLT_CHORIZITE:
		case AMMO_TYPE::AMMO_BOLT_CRYSTAL:
			_weenie->EmitSound(Sound_CrossbowRelease, 1.0f);
			break;
		}
	}

	if (equippedAmmo->IsWorldAware())
	{
		equippedAmmo->_position_timestamp++;

		BinaryWriter removeFrom3D;
		removeFrom3D.Write<uint32_t>(0xF74A);
		removeFrom3D.Write<uint32_t>(equippedAmmo->GetID());
		removeFrom3D.Write<WORD>(equippedAmmo->_instance_timestamp);
		removeFrom3D.Write<WORD>(equippedAmmo->_position_timestamp);
		g_pWorld->BroadcastPVS(equippedAmmo->GetWorldTopLevelOwner()->GetLandcell(), removeFrom3D.GetData(), removeFrom3D.GetSize());

		equippedAmmo->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
		equippedAmmo->unset_parent();
	}

	if (_weenie->AsPlayer()) //only players use ammo.
	{
		if (equippedAmmo->InqIntQuality(STACK_SIZE_INT, 1) <= 1)
		{
			//we're about to run out of ammo, exit combat mode instead of trying to reload.
			_weenie->ChangeCombatMode(NONCOMBAT_COMBAT_MODE, false);
			_weenie->NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
		}

		//and consume the ammo.
		equippedAmmo->DecrementStackNum(1, true);
	}
}

void CMissileAttackEvent::OnAttackAnimSuccess(uint32_t motion)
{
	FireMissile();
	Finish();
}

void CMissileAttackEvent::Finish()
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	Done();
}

void CMissileAttackEvent::HandleAttackHook(const AttackCone &cone)
{
}
