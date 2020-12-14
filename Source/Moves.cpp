
#include <StdAfx.h>
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "World.h"

#include "ObjectMsgs.h"

void CPhysicsObj::Movement_Init()
{
	m_fMoveThink = g_pGlobals->Time();
}

void CPhysicsObj::Movement_Shutdown()
{
}

void CPhysicsObj::Movement_Think()
{
	if (parent)
		return;

	if ((m_fMoveThink + 1.0f) < Timer::cur_time)
	{
		if (m_Position.objcell_id != m_LastMovePosition.objcell_id || !m_Position.frame.is_vector_equal(m_LastMovePosition.frame))
		{
			Movement_UpdatePos();
			// Animation_Update();
		}
		else if (!m_Position.frame.is_quaternion_equal(m_LastMovePosition.frame))
		{
			Movement_UpdatePos();
		}

		m_fMoveThink = Timer::cur_time;
	}
}

void CPhysicsObj::Movement_SendUpdate(uint32_t dwCell)
{
	if (CWeenieObject *pWeenie = GetWeenie())
	{
		BinaryWriter* poo = MoveUpdate(pWeenie);
		g_pWorld->BroadcastPVS(dwCell, poo->GetData(), poo->GetSize(), OBJECT_MSG, false, false, true);
		delete poo;
	}
}

void CPhysicsObj::Movement_UpdatePos()
{
	if (parent)
		return;

	//QUICKFIX: Broadcast to the old landblock that we've moved from.
	//This sends duplicates if the block is near the other.

	_position_timestamp++;
	_last_update_pos = Timer::cur_time;

	uint32_t dwNewCell = GetLandcell();
	uint32_t dwOldCell = m_LastMovePosition.objcell_id;

	if (BLOCK_WORD(dwOldCell) != BLOCK_WORD(dwNewCell))
	{
		Movement_SendUpdate(dwOldCell);
	}

	Movement_SendUpdate(dwNewCell);

	m_LastMovePosition = m_Position;

	/*
	GetWeenie()->EmoteLocal(csprintf("Sending position update. Pos: %.1f %.1f %.1f v: %.1f %.1f %.1f", 
		m_Position.frame.m_origin.x, m_Position.frame.m_origin.y, m_Position.frame.m_origin.z,
		m_velocityVector.x, m_velocityVector.y, m_velocityVector.z));
		*/
}


void CPhysicsObj::Movement_UpdateVector()
{
	if (parent)
		return;

	BinaryWriter moveMsg;
	moveMsg.Write<uint32_t>(0xF74E);
	moveMsg.Write<uint32_t>(id);

	// velocity
	Vector localVel = m_velocityVector;
	localVel.Pack(&moveMsg);

	// omega
	m_Omega.Pack(&moveMsg);

	moveMsg.Write<WORD>(_instance_timestamp);
	moveMsg.Write<WORD>(++_vector_timestamp);

	g_pWorld->BroadcastPVS(this, moveMsg.GetData(), moveMsg.GetSize(), OBJECT_MSG, false, false, true);
}