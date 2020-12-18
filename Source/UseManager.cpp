
#include <StdAfx.h>
#include "UseManager.h"
#include "WeenieObject.h"
#include "Animate.h"
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

	if (_move_to && InUseRange())
	{
		_weenie->DoForcedMotion(Motion_Ready);
		HandleMoveToDone(WERROR_NONE);
	}
}

void CUseEventData::SetupUse()
{	
	CWeenieObject *target = GetTarget();
	if (target && _max_use_distance == FLT_MAX) //change max use distance only if still the initialized value. Otherwise this has already been defined.)
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
	//if (CWeenieObject *target = GetTarget())
	//{
	//	if (target->AsPortal())
	//	{
	//		params.use_spheres = 0; // if we're using a portal we want to get to the center of it, not collide with it's sphere
	//	}
	//}


	_weenie->MoveToObject(_target_id, &params);
}

void CUseEventData::CheckTimeout()
{
	if (Timer::cur_time > _timeout)
	{
		if (_move_to)
			Cancel(WERROR_MOVED_TOO_FAR);
		else if (_recall_event)
		{
			// If you're in the max use range during a recall event, then the recall should still trigger. Otherwise you've moved too far.
			if (InMoveRange())
				OnUseAnimSuccess(_active_use_anim);
			else
				Cancel(WERROR_MOVED_TOO_FAR);
		}
		else
			Cancel(0);
	}
}

bool CUseEventData::InMoveRange()
{
	return _weenie->m_Position.distance(_initial_use_position) <= _max_use_distance ? true : false;
}

void CUseEventData::SetupRecall()
{
	_max_use_distance = 5.0f;
	_initial_use_position = _weenie->m_Position;
	_recall_event = true;
}

void CUseEventData::Cancel(uint32_t error)
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

	//if (target->AsPortal())
	//{
	//	return _weenie->DistanceTo(target, false);
	//}


	return _weenie->DistanceTo(target, _max_use_distance >= 0);
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

CWeenieObject *CUseEventData::GetSourceItem()
{
	return g_pWorld->FindObject(_source_item_id);
}

void CUseEventData::HandleMoveToDone(uint32_t error)
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

void CUseEventData::OnMotionDone(uint32_t motion, BOOL success)
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
			if(!_recall_event) // Don't cancel recall events on motion interrupts, such as jumping.
				Cancel();
		}
	}	
}

void CUseEventData::OnUseAnimSuccess(uint32_t motion)
{
	Done();
}

void CUseEventData::Done(uint32_t error)
{
	Done(error, false);
}

void CUseEventData::Done(uint32_t error, bool silent)
{
	_manager->OnUseDone(error, silent);
}

void CUseEventData::ExecuteUseAnimation(uint32_t motion, MovementParameters *params)
{
	assert (!_move_to);
	assert (!_active_use_anim);

	if (_weenie->IsDead() || _weenie->IsInPortalSpace())
	{
		Cancel(WERROR_ACTIONS_LOCKED);
		return;
	}

	_active_use_anim = motion;

	uint32_t error = _weenie->DoForcedMotion(motion, params);

	if (error)
	{
		Cancel(error);
	}
}

bool CUseEventData::QuestRestrictions(CWeenieObject *target)
{
	std::string restriction;
	if (target->m_Qualities.InqString(QUEST_RESTRICTION_STRING, restriction) && !restriction.empty()) //Allows for restrition of pickup if you are NOT flagged. (IE must have flag for use)
	{
		if (CPlayerWeenie *player = _weenie->AsPlayer())
		{
			if (!player->InqQuest(restriction.c_str()))
			{
				_weenie->DoForcedStopCompletely();
				Cancel(WERROR_QUEST_RESRICTION_UNSOLVED); // Sends -> This item requires you to complete a specific quest before you can pick it up!
				return true;
			}
		}
	}

	std::string questString;
	if (target->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty()) //Used for restriction of pickup if quest is already solved. Common use is for timer restrictions.
	{
		if (_weenie->InqQuest(questString.c_str()))
		{
			int timeTilOkay = _weenie->InqTimeUntilOkayToComplete(questString.c_str());

			if (timeTilOkay > 0)
			{
				std::string cdTime = ("You cannot complete this quest for another " + TimeToString(timeTilOkay) + ".");
				_weenie->SendText(cdTime.c_str(), LTT_DEFAULT);
			}

			_weenie->DoForcedStopCompletely();
			Cancel(WERROR_QUEST_SOLVED_TOO_RECENTLY);
			return true;
		}

		_weenie->StampQuest(questString.c_str());
		target->m_Qualities.SetString(QUEST_STRING, "");
		
	}
	return false;
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

void CGenericUseEvent::OnUseAnimSuccess(uint32_t motion)
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

		// Prevent items like Head of the Homunculus, Wallbound Niffis, Tursh Totem, etc. from being used in packs.
		if (target->InqIntQuality(HOOK_GROUP_INT, 0))
		{
			Cancel(WERROR_HOOKER_NOT_USEABLE_OFF_HOOK);
			return;
		}

		uint32_t createWcid;
		if (target->m_Qualities.InqDataID(USE_CREATE_ITEM_DID, createWcid))
		{
			CPlayerWeenie *player = g_pWorld->FindPlayer(_weenie->GetID());
			if (player)
			{
				if (!player->SpawnInContainer(createWcid, target->InqIntQuality(USE_CREATE_QUANTITY_INT, 1)))
				{
					player->SendText("Not enough pack space!", LTT_ERROR);
					Done(error);
					return;
				}
			}
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
		target->DoUseEmote(_weenie);
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
	CWeenieObject *target = GetTarget();

	if (target->HasOwner()) {
		
		if (this->_target_container_id == _weenie->GetID())
			target = target->GetWorldTopLevelOwner();
		else
			target = g_pWorld->FindObject(this->_target_container_id);
	}

	float z1 = target->m_Position.frame.m_origin.z;
	float z2 = _weenie->m_Position.frame.m_origin.z;

	if (z1 - z2 >= 1.9)
		ExecuteUseAnimation(Motion_Pickup20);
	else if (z1 - z2 >= 1.4)
		ExecuteUseAnimation(Motion_Pickup15);
	else if (z1 - z2 >= 0.9)
		ExecuteUseAnimation(Motion_Pickup10);
	else
		ExecuteUseAnimation(Motion_Pickup);
}

void CPickupInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
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

void CDropInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

void CMoveToWieldInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
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

void CStackMergeInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
	}

	//disabled the following code as it bugs picking up stackable items(such as pyreals) while having some of them in your inventory.
	//uint32_t owner = 0;
	//if (target->m_Qualities.InqInstanceID(OWNER_IID, owner) && owner != _weenie->GetID())
	//{
	//	_weenie->NotifyInventoryFailedEvent(_sourceItemId, WERROR_OBJECT_GONE);
	//	_weenie->DoForcedStopCompletely();
	//	Cancel();
	//}
	//else
	//{
		bool success = _weenie->AsPlayer()->MergeItem(_sourceItemId, _targetItemId, _amountToTransfer, true);

		_weenie->DoForcedStopCompletely();

		if (!success)
			Cancel();
		else
			Done();
	//}
}

//-------------------------------------------------------------------------------------

void CStackSplitToContainerInventoryUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Pickup);
}

void CStackSplitToContainerInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
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

void CStackSplitTo3DInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
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

void CStackSplitToWieldInventoryUseEvent::OnUseAnimSuccess(uint32_t motion)
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

	if (QuestRestrictions(target))
	{
		return;
	}

	bool success = _weenie->AsPlayer()->SplitItemToWield(_sourceItemId, _targetLoc, _amountToTransfer, true);

	_weenie->DoForcedStopCompletely();

	if (!success)
		Cancel();
	else
		Done();
}

//-------------------------------------------------------------------------------------

void CGiveEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Ready);
}

void CGiveEvent::OnUseAnimSuccess(uint32_t motion)
{
	CWeenieObject *giveTarget = GetTarget();

	if (giveTarget)
	{
		CContainerWeenie *target = giveTarget->AsContainer();
		CWeenieObject *source = GetSourceItem();
		if (!target || !source)
		{
			_weenie->NotifyInventoryFailedEvent(_source_item_id, WERROR_NONE);
			return Done(WERROR_OBJECT_GONE);
		}
		_weenie->AsMonster()->FinishGiveItem(target, source, _transfer_amount);
		Done();
	}
	else
	{
		_weenie->NotifyInventoryFailedEvent(_source_item_id, WERROR_NONE);
		return Done(WERROR_OBJECT_GONE);
	}
}

void CGiveEvent::Cancel(uint32_t error) //This override will capture all Use_Manger/Movement Errors and tack on an inventory failed event so we don't get stuck in a busy state if our give action is cancelled.
{
	CancelMoveTo();
	_weenie->NotifyInventoryFailedEvent(_source_item_id, WERROR_NONE);
	_manager->OnUseCancelled(error);
}

//-------------------------------------------------------------------------------------

void CTradeUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Ready);
}

void CTradeUseEvent::OnUseAnimSuccess(uint32_t motion)
{
	CPlayerWeenie *target = GetTarget()->AsPlayer();
	if (!target)
		return Done(WERROR_OBJECT_GONE);
	
	TradeManager *tm = TradeManager::RegisterTrade(_weenie->AsPlayer(), target);

	_weenie->AsPlayer()->SetTradeManager(tm);
	target->SetTradeManager(tm);
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

void UseManager::OnUseCancelled(uint32_t error)
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

void UseManager::OnUseDone(uint32_t error)
{
	OnUseDone(error, false);
}

void UseManager::OnUseDone(uint32_t error, bool silent)
{
	if (_useData)
	{
		if (!_useData->IsInventoryEvent() && !silent)
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

void UseManager::OnDeath(uint32_t killer_id)
{
	Cancel();
}

void UseManager::HandleMoveToDone(uint32_t error)
{
	if (_useData)
	{
		_useData->HandleMoveToDone(error);
	}
}

void UseManager::OnMotionDone(uint32_t motion, BOOL success)
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

bool UseManager::IsMoving()
{
	return IsUsing() && _useData->_move_to != 0 ? true : false;
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

