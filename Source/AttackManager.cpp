#include <StdAfx.h>
#include "AttackManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "Player.h"
#include "WeenieFactory.h"
#include "Ammunition.h"
#include "CombatFormulas.h"

#include "combat/MeleeAttackEventData.h"
#include "combat/DualWieldAttackEventData.h"
#include "combat/MissileAttackEventData.h"

// TODO fix memory leak with attack data

AttackManager::AttackManager(CWeenieObject *weenie)
{
	_weenie = weenie;
}

AttackManager::~AttackManager()
{
	SafeDelete(_attackData);
	SafeDelete(_cleanupData);
	SafeDelete(_queuedAttackData);
}

void AttackManager::MarkForCleanup(CAttackEventData *data)
{
	if (_cleanupData && _cleanupData != data)
	{
		delete _cleanupData;
	}

	_cleanupData = data;
}

void AttackManager::Cancel()
{
	if (_attackData)
		_attackData->Cancel();
}

void AttackManager::OnAttackCancelled(uint32_t error)
{
	if (_attackData)
	{
		_weenie->NotifyAttackDone();
		_weenie->DoForcedMotion(_weenie->get_minterp()->InqStyle());
		_weenie->unstick_from_object();

		MarkForCleanup(_attackData);
		_attackData = NULL;
	}
}

bool AttackManager::RepeatAttacks()
{
	if (_weenie->AsPlayer())
	{
		CPlayerWeenie *player = (CPlayerWeenie *)_weenie;
		return player->ShouldRepeatAttacks();
	}

	return false;
}

void AttackManager::OnAttackDone(uint32_t error)
{
	if (_weenie->AsPlayer())
	{
		_weenie->AsPlayer()->m_Qualities.SetFloat(ATTACK_TIMESTAMP_FLOAT, Timer::cur_time);
	}
	//if(_weenie->_blockNewAttacksUntil < Timer::cur_time) //fix for cancelling reload animation making attacking faster 
		//_weenie->_blockNewAttacksUntil = Timer::cur_time + 1.0;
	if (_attackData)
	{
		if (RepeatAttacks() && _attackData->IsValidTarget())
		{

			if (_queuedAttackData != NULL)
			{
				//we have a queued attack, change to that.
				SafeDelete(_attackData);
				_attackData = _queuedAttackData;
				_queuedAttackData = NULL;
			}

			if (_attackData->ShouldNotifyAttackDone())
			{
				_weenie->NotifyAttackDone();
				_weenie->NotifyCommenceAttack();
			}

			_attackData->_attack_charge_time = Timer::cur_time + (_attackData->_attack_power * _attackData->AttackTimeMod());
			_attackData->_attack_speed *= _attackData->AttackSpeedMod();
			_attackData->Begin();
		}
		else
		{
			_weenie->NotifyAttackDone();

			if (_weenie->_IsPlayer())
				_weenie->AsPlayer()->m_bCancelAttack = true;

			MarkForCleanup(_attackData);
			_attackData = NULL;
		}
	}
}

void AttackManager::Update()
{
	if (_attackData)
	{
		_attackData->Update();
	}

	SafeDelete(_cleanupData);
}

void AttackManager::OnDeath(uint32_t killer_id)
{
	Cancel();
}

void AttackManager::HandleMoveToDone(uint32_t error)
{
	if (_attackData)
	{
		_attackData->HandleMoveToDone(error);
	}
}

void AttackManager::HandleAttackHook(const AttackCone &cone)
{
	if (_attackData)
	{
		_attackData->HandleAttackHook(cone);
	}
}

void AttackManager::OnMotionDone(uint32_t motion, BOOL success)
{
	if (_attackData)
	{
		_attackData->OnMotionDone(motion, success);

		if (motion == Motion_Reload)
		{
			_weenie->NotifyAttackDone();
			_weenie->NotifyCommenceAttack();
		}
	}
}

bool AttackManager::IsAttacking()
{
	return _attackData != NULL ? true : false;
}

void AttackManager::BeginAttack(CAttackEventData *data)
{
	if (_attackData != NULL)
	{
		//we're already attacking, queue this, or change current queued attack to this.
		SafeDelete(_queuedAttackData);
		_queuedAttackData = data;
	}
	else
	{
		_attackData = data;
		_attackData->Begin();
	}
}

void AttackManager::BeginMeleeAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, float chase_distance, uint32_t motion)
{
	CMeleeAttackEvent *attackEvent = nullptr;

	CWeenieObject *left = _weenie->GetWieldedCombat(COMBAT_USE_OFFHAND);

	if (_weenie->GetWieldedCombat(COMBAT_USE_TWO_HANDED))
		attackEvent = new CTwoHandAttackEvent();
	else if (left && left->InqIntQuality(LOCATIONS_INT, 0) != SHIELD_LOC)
		attackEvent = new CDualWieldAttackEvent();
	else
		attackEvent = new CMeleeAttackEvent();

	attackEvent->_weenie = _weenie;
	attackEvent->_manager = this;
	attackEvent->_target_id = target_id;
	attackEvent->_attack_height = height;
	attackEvent->_attack_power = power;
	attackEvent->_do_attack_animation = motion;
	attackEvent->_fail_distance = chase_distance;

	BeginAttack(attackEvent);
}

void AttackManager::BeginMissileAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion)
{
	CMissileAttackEvent *attackEvent = new CMissileAttackEvent();

	attackEvent->_weenie = _weenie;
	attackEvent->_manager = this;
	attackEvent->_target_id = target_id;
	attackEvent->_attack_height = height;
	attackEvent->_attack_power = power;
	attackEvent->_do_attack_animation = motion;

	BeginAttack(attackEvent);
}
