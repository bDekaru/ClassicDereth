
#include <StdAfx.h>

#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "Door.h"
#include "Player.h"

#define DOOR_OPEN			0x0B
#define DOOR_CLOSED			0x0C

CBaseDoor::CBaseDoor()
{
	m_Qualities.m_WeenieType = Door_WeenieType;
}

CBaseDoor::~CBaseDoor()
{
}

void CBaseDoor::PostSpawn()
{
	m_bInitiallyLocked = IsLocked();

	CWeenieObject::PostSpawn();
}

void CBaseDoor::ResetToInitialState()
{
	if (m_bOpen)
	{
		// TODO check if the door was initially open or not, but for now we assume it wasn't
		CloseDoor();
	}

	SetLocked(m_bInitiallyLocked ? TRUE : FALSE);
}

void CBaseDoor::OpenDoor()
{
	// anim already in progress
	if (_phys_obj->get_minterp()->interpreted_state.GetNumActions() > 0)
		return;

	last_move_was_autonomous = false;

	_server_control_timestamp++;

	MovementStruct mvs;
	MovementParameters params;
	mvs.type = RawCommand;
	mvs.motion = Motion_On;
	mvs.params = &params;
	params.autonomous = 0;
	params.action_stamp = ++m_wAnimSequence;
	get_movement_manager()->PerformMovement(mvs);
	Animation_Update();

	m_bOpen = true;

	double resetInterval;
	if (m_Qualities.InqFloat(RESET_INTERVAL_FLOAT, resetInterval))
	{
		_nextReset = Timer::cur_time + resetInterval;
	}
}

void CBaseDoor::CloseDoor()
{
	// anim already in progress
	if (_phys_obj->get_minterp()->interpreted_state.GetNumActions() > 0)
		return;

	last_move_was_autonomous = false;

	_server_control_timestamp++;

	MovementStruct mvs;
	MovementParameters params;
	mvs.type = RawCommand;
	mvs.motion = Motion_Off;
	mvs.params = &params;
	params.autonomous = 0;
	params.action_stamp = ++m_wAnimSequence;
	_phys_obj->movement_manager->PerformMovement(mvs);
	Animation_Update();

	m_bOpen = false;
}

bool CBaseDoor::IsClosed()
{
	return get_minterp()->interpreted_state.forward_command != Motion_On;
}

int CBaseDoor::Activate(uint32_t activator_id)
{
	if (!m_bOpen)
	{
		if (IsLocked())
		{
			EmitSound(Sound_OpenFailDueToLock, 1.0f);
			return WERROR_NONE;
		}

		OpenDoor();
	}
	else
	{
		CloseDoor();
	}

	CWeenieObject::Activate(activator_id);

	return WERROR_NONE;
}

int CBaseDoor::Use(CPlayerWeenie *pOther)
{
	CActivationUseEvent *useEvent = new CActivationUseEvent();
	useEvent->_target_id = GetID();
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}
