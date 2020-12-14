#include <StdAfx.h>
#include "AttackManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "Player.h"
#include "WeenieFactory.h"
#include "Ammunition.h"
#include "CombatFormulas.h"

// TODO fix memory leak with attack data

CAttackEventData::CAttackEventData() noexcept
{
}

void CAttackEventData::Update()
{
	if (_attack_charge_time >= 0.0 && Timer::cur_time >= _attack_charge_time)
	{
		PostCharge();
	}

	CheckTimeout();
}

void CAttackEventData::Setup()
{
	_max_attack_distance = DISTANCE_REQUIRED_FOR_MELEE_ATTACK;
	_max_attack_angle = MAX_MELEE_ATTACK_CONE_ANGLE;
	_timeout = Timer::cur_time + 15.0;
}

void CAttackEventData::PostCharge()
{
	_attack_charge_time = -1.0;

	if (InAttackCone())
	{
		OnReadyToAttack();
	}
	else
	{
		MoveToAttack();
	}
}

void CAttackEventData::Begin()
{
	Setup();

	CWeenieObject *target = GetTarget();
	if (!target)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (target->HasOwner())
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!target->IsAttackable() && !_weenie->CanTarget(target))
	{
		Cancel();
		return;
	}

	if (_attack_charge_time < 0.0)
	{
		PostCharge();
	}
}

void CAttackEventData::MoveToAttack()
{
	_move_to = true;

	MovementParameters params;
	params.can_walk = 0;
	params.can_run = 0;
	params.can_sidestep = 0;
	params.can_walk_backwards = 0;
	params.move_away = 1;
	params.can_charge = m_bCanCharge || !_weenie->AsPlayer() ? 1 : 0;
	params.fail_walk = 1;
	params.use_final_heading = 1;
	params.sticky = _use_sticky;

	params.min_distance = 0.1f;
	params.distance_to_object = _max_attack_distance - 0.5f; // 0.5
	params.fail_distance = _fail_distance;
	params.speed = 1.5f;
	params.action_stamp = ++_weenie->m_wAnimSequence;
	_weenie->last_move_was_autonomous = false;
	_weenie->MoveToObject(_target_id, &params);
}

void CAttackEventData::TurnToAttack()
{
	_move_to = true;

	MovementParameters params;

	_weenie->last_move_was_autonomous = false;

	_weenie->TurnToObject(_target_id, &params);
}

void CAttackEventData::CheckTimeout()
{
	if (Timer::cur_time > _timeout)
	{
		if (_move_to)
			Cancel(WERROR_MOVED_TOO_FAR);
		else
			Cancel(0);
	}
}

void CAttackEventData::Cancel(uint32_t error)
{
	CancelMoveTo();

	_manager->OnAttackCancelled(error);
}

void CAttackEventData::CancelMoveTo()
{
	if (_move_to)
	{
		_weenie->cancel_moveto();
		_weenie->Animation_MoveToUpdate();

		_move_to = false;
	}
}

double CAttackEventData::DistanceToTarget()
{
	if (!_target_id || _target_id == _weenie->GetID())
		return 0.0;

	CWeenieObject *target = GetTarget();
	if (!target)
		return FLT_MAX;

	return _weenie->DistanceTo(target, true);
}

double CAttackEventData::HeadingToTarget(bool relative)
{
	if (!_target_id || _target_id == _weenie->GetID())
		return 0.0;

	CWeenieObject *target = GetTarget();
	if (!target)
		return 0.0;

	return _weenie->HeadingTo(target, relative);
}

bool CAttackEventData::InAttackRange()
{
	CWeenieObject *target = GetTarget();
	if (!target || target->HasOwner())
		return true;

	if ((_max_attack_distance + F_EPSILON) < DistanceToTarget())
		return false;

	return true;
}

bool CAttackEventData::InAttackCone()
{
	CWeenieObject *target = GetTarget();
	if (!target || target->HasOwner())
		return true;

	if ((_max_attack_distance + F_EPSILON) < DistanceToTarget())
		return false;
	if ((_max_attack_angle + F_EPSILON) < HeadingToTarget())
		return false;

	return true;
}

CWeenieObject *CAttackEventData::GetTarget()
{
	return g_pWorld->FindObject(_target_id);
}

void CAttackEventData::HandleMoveToDone(uint32_t error)
{
	_move_to = false;

	if (error)
	{
		Cancel(error);
		return;
	}

	if (!InAttackRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	OnReadyToAttack();
}

void CAttackEventData::OnMotionDone(uint32_t motion, BOOL success)
{
	if (_move_to || _turn_to || !_active_attack_anim)
		return;

	if (motion == _active_attack_anim)
	{
		_active_attack_anim = 0;

		if (success)
		{
			OnAttackAnimSuccess(motion);
		}
		else
		{
			Cancel();
		}
	}
}

void CAttackEventData::OnAttackAnimSuccess(uint32_t motion)
{
	Done();
}

void CAttackEventData::Done(uint32_t error)
{
	_manager->OnAttackDone(error);
}

bool CAttackEventData::IsValidTarget()
{
	CWeenieObject *target = GetTarget();

	if (!target || (!target->IsAttackable() && !_weenie->CanTarget(target)) || target->IsDead() || target->IsInPortalSpace() || target->ImmuneToDamage(_weenie))
	{
		return false;
	}

	return true;
}

void CAttackEventData::ExecuteAnimation(uint32_t motion, MovementParameters *params)
{
	assert(!_move_to);
	assert(!_turn_to);
	assert(!_active_attack_anim);

	if (_weenie->IsDead() || _weenie->IsInPortalSpace())
	{
		Cancel(WERROR_ACTIONS_LOCKED);
		return;
	}

	_active_attack_anim = motion;

	uint32_t error = _weenie->DoForcedMotion(motion, params);

	if (error)
	{
		Cancel(error);
	}
}
