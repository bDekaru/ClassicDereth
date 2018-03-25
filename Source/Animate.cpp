
#include "StdAfx.h"
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "World.h"
#include "AnimationPackage.h"
#include "Movement.h"
#include "MovementManager.h"
#include "ChatMsgs.h"

AnimationPackage::AnimationPackage(WORD wStance, WORD wIndex, float fSpeed)
{
	m_wStance = wStance;
	m_wIndex = wIndex;
	m_fSpeed = fSpeed;

	m_dwTarget = 0;
	m_dwAction = 0;
}

bool AnimationPackage::Initialize()
{
#if _TODO // Should be looking up the animation/s from the set.
#else
	m_fStartTime = g_pGlobals->Time();
	m_dwCurrentFrame = 0x80000000; //Magic number for "null frame"
	m_dwStartFrame = 0;
	m_dwEndFrame = -1;

	if (!m_fSpeed)
		m_fSpeed = 30.0f;
#endif

	return true;
}

SequencedAnimation::SequencedAnimation(WORD wSequence, WORD wStance, WORD wIndex, float fSpeed)
	: AnimationPackage(wStance, wIndex, fSpeed)
{
	m_wSequence = wSequence;
}

void CPhysicsObj::Animation_Init()
{
	m_wAnimSequence = 0;
}

void CPhysicsObj::Animation_Shutdown()
{
}

BinaryWriter *CPhysicsObj::Animation_GetAnimationInfo(bool bMoveToUpdate)
{
	BinaryWriter *AnimInfo = new BinaryWriter;

	WORD movementType = MovementTypes::Invalid;

	MoveToManager *moveToManager = movement_manager ? movement_manager->moveto_manager : NULL;

	if (moveToManager)
	{
		switch (moveToManager->movement_type)
		{
			// TODO add others
		case MovementTypes::MoveToPosition:
		case MovementTypes::MoveToObject:
		case MovementTypes::TurnToHeading:
		case MovementTypes::TurnToObject:
			movementType = (WORD)moveToManager->movement_type;
			break;
		}
	}

	WORD movementFlags = 0;
	
	if (position_manager && position_manager->GetStickyObjectID() != 0)
		movementFlags |= 0x100; // sticky

	if (get_minterp()->standing_longjump)
		movementFlags |= 0x200;

	WORD pack_word = movementFlags | movementType;
	AnimInfo->Write<WORD>(pack_word);
	AnimInfo->Write<WORD>(get_minterp()->interpreted_state.current_style & 0xFFFF);

	switch (movementType)
	{
	case MovementTypes::Invalid:
		{
			get_minterp()->interpreted_state.Pack(AnimInfo);

			if (position_manager && position_manager->GetStickyObjectID() != 0)
				AnimInfo->Write<DWORD>(position_manager->GetStickyObjectID()); // sticky

			break;
		}
	case MovementTypes::MoveToPosition:
		{
			moveToManager->sought_position.PackOrigin(AnimInfo);
			moveToManager->movement_params.PackNet(MovementTypes::MoveToPosition, AnimInfo);
			
			/*
			g_pWorld->BroadcastGlobal(ServerText(csprintf("my_run_rate: %f", get_minterp()->my_run_rate), LTT_DEFAULT), PRIVATE_MSG);
			g_pWorld->BroadcastGlobal(ServerText(csprintf("my_run_rate * 1.9: %f", get_minterp()->my_run_rate * 1.9), LTT_DEFAULT), PRIVATE_MSG);
			g_pWorld->BroadcastGlobal(ServerText(csprintf("max_speed: %f", get_minterp()->get_max_speed()), LTT_DEFAULT), PRIVATE_MSG);
			g_pWorld->BroadcastGlobal(ServerText(csprintf("adjusted_max_speed: %f", get_minterp()->get_adjusted_max_speed()), LTT_DEFAULT), PRIVATE_MSG);
			*/

			AnimInfo->Write<float>(get_minterp()->my_run_rate * 1.9);
			break;
		}

	case MovementTypes::MoveToObject:
		{
			// revisit this...
			AnimInfo->Write<DWORD>(moveToManager->sought_object_id);

			// _position_timestamp++;
			CPhysicsObj *pTarget = CPhysicsObj::GetObject(moveToManager->sought_object_id);
			if (pTarget && !pTarget->parent && pTarget->m_Position.objcell_id != 0)
				pTarget->m_Position.PackOrigin(AnimInfo);
			else
				m_Position.PackOrigin(AnimInfo);

			moveToManager->movement_params.PackNet(MovementTypes::MoveToObject, AnimInfo);

			AnimInfo->Write<float>(get_minterp()->my_run_rate * 1.9);
			break;
		}
	case MovementTypes::TurnToObject:
		{
			AnimInfo->Write<DWORD>(moveToManager->sought_object_id);

			float heading = 0.0f;

			CPhysicsObj *pTarget = CPhysicsObj::GetObject(moveToManager->sought_object_id);
			if (pTarget && !pTarget->parent)
				m_Position.heading(pTarget->m_Position);

			AnimInfo->Write<float>(heading);
			moveToManager->movement_params.PackNet(MovementTypes::TurnToObject, AnimInfo);
			break;
		}
	case MovementTypes::TurnToHeading:
		{
			moveToManager->movement_params.PackNet(MovementTypes::TurnToHeading, AnimInfo);
			break;
		}
	}

	AnimInfo->Align();

	return AnimInfo;
}

void CPhysicsObj::Animation_Update()
{
	m_bAnimUpdate = FALSE;

	if (parent)
		return;
	
	BinaryWriter AnimUpdate;
	AnimUpdate.Write<DWORD>(0xF74C);
	AnimUpdate.Write<DWORD>(id);
	AnimUpdate.Write<WORD>(_instance_timestamp);
	AnimUpdate.Write<WORD>(++_movement_timestamp);
	AnimUpdate.Write<WORD>(_server_control_timestamp);
	AnimUpdate.Write<BYTE>((weenie_obj && weenie_obj->AsPlayer()) ? last_move_was_autonomous : 0);
	AnimUpdate.Align();

	last_move_was_autonomous = true;

	BinaryWriter *AnimInfo = Animation_GetAnimationInfo();
	AnimUpdate.Write(AnimInfo);
	delete AnimInfo;

	g_pWorld->BroadcastPVS(this, AnimUpdate.GetData(), AnimUpdate.GetSize(), OBJECT_MSG);
}

void CPhysicsObj::Animation_MoveToUpdate()
{
	if (parent)
		return;
	
	last_move_was_autonomous = false;

	BinaryWriter AnimUpdate;
	AnimUpdate.Write<DWORD>(0xF74C);
	AnimUpdate.Write<DWORD>(id);
	AnimUpdate.Write<WORD>(_instance_timestamp);
	AnimUpdate.Write<WORD>(++_movement_timestamp);
	AnimUpdate.Write<WORD>(++_server_control_timestamp);
	AnimUpdate.Write<BYTE>(last_move_was_autonomous);
	AnimUpdate.Align();

	BinaryWriter *AnimInfo = Animation_GetAnimationInfo(true);
	AnimUpdate.Write(AnimInfo);
	delete AnimInfo;

	g_pWorld->BroadcastPVS(this, AnimUpdate.GetData(), AnimUpdate.GetSize(), OBJECT_MSG);

	last_move_was_autonomous = false;
}
