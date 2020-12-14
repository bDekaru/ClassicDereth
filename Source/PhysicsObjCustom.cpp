
#include <StdAfx.h>
#include "PhysicsObj.h"
#include "WeenieObject.h"
#include "MovementManager.h"
#include "Physics.h"
#include "Setup.h"
#include "MotionTable.h"
#include "PartArray.h"
#include "Scripts.h"
#include "ObjCell.h"
#include "ChildList.h"
#include "Transition.h"
#include "World.h"
#include "ObjectMsgs.h"

void CPhysicsObj::Send_StateChangeEvent()
{
	_state_timestamp++;

	if (InValidCell())
	{
		uint32_t UVF[4];

		UVF[0] = 0xF74B;
		UVF[1] = GetID();
		UVF[2] = m_PhysicsState;
		UVF[3] = (_state_timestamp << 16) | _instance_timestamp;

		g_pWorld->BroadcastPVS(GetLandcell(), UVF, sizeof(UVF), OBJECT_MSG, false, false, true);
	}
}

void CPhysicsObj::EmitSound(uint32_t sound_id, float speed, bool bLocalClientOnly)
{
	if (!InValidCell())
	{
		return;
	}

	BinaryWriter SoundMsg;
	SoundMsg.Write<uint32_t>(0xF750);
	SoundMsg.Write<uint32_t>(id);
	SoundMsg.Write<uint32_t>(sound_id);
	SoundMsg.Write<float>(speed);

	if (bLocalClientOnly)
	{
		SendNetMessage(&SoundMsg, OBJECT_MSG, FALSE, FALSE);
	}
	else
	{
		g_pWorld->BroadcastPVS(this, SoundMsg.GetData(), SoundMsg.GetSize(), OBJECT_MSG);
	}
}

void CPhysicsObj::EmitEffect(uint32_t dwIndex, float flScale)
{
	if (!InValidCell())
		return;

	BinaryWriter EffectMsg;
	EffectMsg.Write<uint32_t>(0xF755);
	EffectMsg.Write<uint32_t>(GetID());
	EffectMsg.Write<uint32_t>(dwIndex);
	EffectMsg.Write<float>(flScale);

	g_pWorld->BroadcastPVS(this, EffectMsg.GetData(), EffectMsg.GetSize(), OBJECT_MSG, 0);
}

void CPhysicsObj::InitPhysicsTemporary()
{
	Movement_Init();
	Animation_Init();
}

void CPhysicsObj::CleanupPhysicsTemporary()
{
	Animation_Shutdown();
	Movement_Shutdown();
}

float CPhysicsObj::DistanceTo(CPhysicsObj *pOther)
{
	return m_Position.distance(pOther->m_Position);
}

float CPhysicsObj::DistanceSquared(CPhysicsObj *pOther)
{
	return m_Position.distance_squared(pOther->m_Position);
}

void CPhysicsObj::EnterPortal(uint32_t old_cell_id)
{
	BinaryWriter EnterPortal;
	EnterPortal.Write<uint32_t>(0xF751);
	EnterPortal.Write<uint32_t>(_teleport_timestamp);
	SendNetMessage(EnterPortal.GetData(), EnterPortal.GetSize(), OBJECT_MSG);

	set_state(PhysicsState::HIDDEN_PS | PhysicsState::IGNORE_COLLISIONS_PS | PhysicsState::EDGE_SLIDE_PS | GRAVITY_PS, TRUE);
}

void CPhysicsObj::ExitPortal()
{
	set_state(REPORT_COLLISIONS_PS | PhysicsState::EDGE_SLIDE_PS | GRAVITY_PS, TRUE);
}

void CPhysicsObj::SendNetMessage(void *_data, uint32_t _len, WORD _group, BOOL _event)
{
	if (weenie_obj)
		weenie_obj->SendNetMessage(_data, _len, _group, _event);
}

void CPhysicsObj::SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event, BOOL del)
{
	if (weenie_obj)
		weenie_obj->SendNetMessage(_food, _group, _event, del);
}

void CPhysicsObj::Tick()
{
}

CWeenieObject *CPhysicsObj::GetWeenie()
{
	return weenie_obj;
}

uint32_t CPhysicsObj::GetLandcell()
{
	return m_Position.objcell_id;
}

int CPhysicsObj::GetPlacementFrameID() // custom
{
	return part_array ? part_array->GetPlacementFrameID() : 0;
}

int CPhysicsObj::GetActivePlacementFrameID() // custom
{
	return part_array ? part_array->GetActivePlacementFrameID() : 0;
}