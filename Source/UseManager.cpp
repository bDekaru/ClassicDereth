
#include "StdAfx.h"
#include "UseManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "Container.h"
#include "Player.h"

// TODO fix memory leak with use data

CUseEventData::CUseEventData()
{
}

void CUseEventData::Update()
{
	CheckTimeout();
}

void CUseEventData::SetupUse()
{	
	CWeenieObject *target = GetTarget();
	if (target)
	{
		_max_use_distance = target->InqFloatQuality(USE_RADIUS_FLOAT, 0.0);
	}

	_timeout = Timer::cur_time + 15.0;
}

void CUseEventData::Begin()
{
	SetupUse();

	if (_target_id)
	{
		CWeenieObject *target = GetTarget();
		if (!target)
		{
			Cancel(WERROR_OBJECT_GONE);
			return;
		}

		if (target->IsContained())
		{
			bool bInViewedContainer = false;

			if (!_weenie->FindContainedItem(target->GetID()))
			{
				bool bInViewedContainer = false;
				if (CContainerWeenie *externalContainer = target->GetWorldTopLevelContainer())
				{
					if (externalContainer->_openedById == _weenie->GetID())
					{
						bInViewedContainer = true;
					}
				}

				if (!bInViewedContainer)
				{
					Cancel(WERROR_OBJECT_GONE);
					return;
				}
			}

			OnReadyToUse();
		}
		else
		{
			if (InUseRange())
			{
				OnReadyToUse();
			}
			else
			{
				MoveToUse();
			}
		}
	}
	else
	{
		OnReadyToUse();
	}
}

void CUseEventData::MoveToUse()
{
	_move_to = true;

	MovementParameters params;
	params.min_distance = _max_use_distance;
	params.action_stamp = ++_weenie->m_wAnimSequence;
	_weenie->last_move_was_autonomous = false;
	_weenie->MoveToObject(_target_id, &params);
}

void CUseEventData::CheckTimeout()
{
	if (Timer::cur_time > _timeout)
	{
		if (_move_to)
			Cancel(WERROR_MOVED_TOO_FAR);
		else
			Cancel(0);
	}
}

void CUseEventData::Cancel(DWORD error)
{
	CancelMoveTo();

	_manager->OnUseCancelled(error);
}

void CUseEventData::CancelMoveTo()
{
	if (_move_to)
	{
		_weenie->cancel_moveto();
		_weenie->Animation_MoveToUpdate();

		_move_to = false;
	}
}

double CUseEventData::DistanceToTarget()
{
	if (!_target_id || _target_id == _weenie->GetID())
		return 0.0;

	CWeenieObject *target = GetTarget();
	if (!target)
		return FLT_MAX;

	return _weenie->DistanceTo(target, true);
}

double CUseEventData::HeadingDifferenceToTarget()
{
	if (!_target_id || _target_id == _weenie->GetID())
		return 0.0;

	CWeenieObject *target = GetTarget();
	if (!target)
		return 0.0;

	return _weenie->get_heading();
}

bool CUseEventData::InUseRange()
{
	CWeenieObject *target = GetTarget();
	if (target && (_weenie->IsContainedWithinViewable(target->GetID())))
		return true;

	if ((_max_use_distance + F_EPSILON) < DistanceToTarget())
		return false;

	return true;
}

CWeenieObject *CUseEventData::GetTarget()
{
	return g_pWorld->FindObject(_target_id);
}

CWeenieObject *CUseEventData::GetTool()
{
	return g_pWorld->FindObject(_tool_id);
}

void CUseEventData::HandleMoveToDone(DWORD error)
{
	_move_to = false;

	if (error)
	{
		Cancel(error);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	OnReadyToUse();
}

void CUseEventData::OnMotionDone(DWORD motion, BOOL success)
{
	if (_move_to || !_active_use_anim)
		return;

	if (motion == _active_use_anim)
	{
		_active_use_anim = 0;

		if (success)
		{
			OnUseAnimSuccess(motion);
		}
		else
		{
			Cancel();
		}
	}	
}

void CUseEventData::OnUseAnimSuccess(DWORD motion)
{
	Done();
}

void CUseEventData::Done(DWORD error)
{
	_manager->OnUseDone(error);
}

void CUseEventData::ExecuteUseAnimation(DWORD motion, MovementParameters *params)
{
	assert (!_move_to);
	assert (!_active_use_anim);

	if (_weenie->IsDead() || _weenie->IsInPortalSpace())
	{
		Cancel(WERROR_ACTIONS_LOCKED);
		return;
	}

	_active_use_anim = motion;

	DWORD error = _weenie->DoForcedMotion(motion, params);

	if (error)
	{
		Cancel(error);
	}
}

void CGenericUseEvent::OnReadyToUse()
{
	if (_do_use_animation)
	{
		ExecuteUseAnimation(_do_use_animation);
	}
	else
	{
		Finish();
	}
}

void CGenericUseEvent::OnUseAnimSuccess(DWORD motion)
{
	Finish();
}

void CGenericUseEvent::Finish()
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	CWeenieObject *tool = GetTool();
	if (!tool && _tool_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	int error = WERROR_NONE;

	if (target)
	{
		if (!InUseRange())
		{
			Cancel(WERROR_TOO_FAR);
			return;
		}
		
		if (_do_use_response)
		{
			if (_tool_id)
			{
				error = tool->DoUseWithResponse(_weenie, target);
			}
			else
			{
				error = target->DoUseResponse(_weenie);
			}
		}

		if (error == WERROR_NONE && _do_use_emote)
		{
			target->DoUseEmote(_weenie);
		}

		if (_do_use_message)
		{
			if (error == WERROR_NONE)
			{
				std::string useMessage;
				if (target->m_Qualities.InqString(USE_MESSAGE_STRING, useMessage))
				{
					_weenie->SendText(useMessage.c_str(), LTT_MAGIC);
				}
			}
			else
			{
				std::string failMessage;
				if (target->m_Qualities.InqString(ACTIVATION_FAILURE_STRING, failMessage))
				{
					_weenie->SendText(failMessage.c_str(), LTT_MAGIC);
				}
			}
		}
	}

	Done(error);
}

void CActivationUseEvent::OnReadyToUse()
{
	CWeenieObject *target = GetTarget();
	if (target)
	{
		target->Activate(_weenie->GetID());
	}

	Done();
}

void CInventoryUseEvent::SetupUse()
{
	_max_use_distance = 1.0;
	_timeout = Timer::cur_time + 15.0;
}

//-------------------------------------------------------------------------------------

void CPickupInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CPickupInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days= timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->MoveItemToContainer(_source_item_id, _target_container_id, _target_slot, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CDropInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CDropInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	if (target->IsAttunedOrContainsAttuned())
	{
		Cancel(WERROR_ATTUNED_ITEM);
		return;
	}

	bool success = _weenie->AsPlayer()->MoveItemTo3D(_target_id, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CMoveToWieldInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CMoveToWieldInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days = timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->MoveItemToWield(_sourceItemId, _targetLoc, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CStackMergeInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CStackMergeInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days = timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->MergeItem(_sourceItemId, _targetItemId, _amountToTransfer, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CStackSplitToContainerInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CStackSplitToContainerInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days = timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->SplitItemToContainer(_sourceItemId, _targetContainerId, _targetSlot, _amountToTransfer, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------


void CStackSplitTo3DInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CStackSplitTo3DInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days = timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->SplitItemto3D(_sourceItemId, _amountToTransfer, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CStackSplitToWieldInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CStackSplitToWieldInventoryUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	if (!InUseRange())
	{
		Cancel(WERROR_TOO_FAR);
		return;
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				int secs = timeTilOkay % 60;
				timeTilOkay /= 60;

				int mins = timeTilOkay % 60;
				timeTilOkay /= 60;

				int hours = timeTilOkay % 24;
				timeTilOkay /= 24;

				int days = timeTilOkay;

				_weenie->SendText(csprintf("You cannot complete this quest for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
	}

	bool success = _weenie->AsPlayer()->SplitItemToWield(_sourceItemId, _targetLoc, _amountToTransfer, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

UseManager::UseManager(CWeenieObject *weenie)
{
	_weenie = weenie;
}

UseManager::~UseManager()
{
	SafeDelete(_useData);
	SafeDelete(_cleanupData);
}

void UseManager::MarkForCleanup(CUseEventData *data)
{
	if (_cleanupData && _cleanupData != data)
	{
		delete _cleanupData;
	}

	_cleanupData = data;
}

void UseManager::Cancel()
{
	if (_useData)
	{
		_useData->Cancel();
	}
}

void UseManager::OnUseCancelled(DWORD error)
{
	if (_useData)
	{
		if (!_useData->IsInventoryEvent())
		{
			_weenie->NotifyUseDone(error);
		}
		else
		{
			_weenie->NotifyInventoryFailedEvent(_useData->_target_id, error);
		}

		MarkForCleanup(_useData);
		_useData = NULL;
	}
}

void UseManager::OnUseDone(DWORD error)
{
	if (_useData)
	{
		if (!_useData->IsInventoryEvent())
		{
			_weenie->NotifyUseDone(error);
		}
		else
		{
		}

		MarkForCleanup(_useData);
		_useData = NULL;
	}
}

void UseManager::Update()
{
	if (_useData)
	{
		_useData->Update();
	}

	SafeDelete(_cleanupData);
}

void UseManager::OnDeath(DWORD killer_id)
{
	Cancel();
}

void UseManager::HandleMoveToDone(DWORD error)
{
	if (_useData)
	{
		_useData->HandleMoveToDone(error);
	}
}

void UseManager::OnMotionDone(DWORD motion, BOOL success)
{
	if (_useData)
	{
		_useData->OnMotionDone(motion, success);
	}
}

bool UseManager::IsUsing()
{
	return _useData != NULL ? true : false;
}

void UseManager::BeginUse(CUseEventData *data)
{
	if (_useData)
	{
		// already busy
		if (!data->IsInventoryEvent())
		{
			_weenie->NotifyWeenieError(WERROR_ACTIONS_LOCKED);
			_weenie->NotifyUseDone(0);
		}
		else
		{
			_weenie->NotifyInventoryFailedEvent(_useData->_target_id, WERROR_ACTIONS_LOCKED);
		}

		delete data;
		return;
	}

	_useData = data;
	_useData->Begin();
}

