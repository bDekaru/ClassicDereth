
#include <StdAfx.h>
#include "PhatSDK.h"
#include "TargetManager.h"

TargetManager::TargetManager(CPhysicsObj *object)
{
	physobj = object;
	target_info = 0;
	voyeur_table = NULL;
	last_update_time = 0;
}

TargetManager::~TargetManager()
{
	if (target_info)
	{
		delete target_info;
		target_info = NULL;
	}

	if (voyeur_table)
	{
		delete voyeur_table;
	}
}

void TargetManager::SetTargetQuantum(double new_quantum)
{
	float quantum;

	if (target_info)
	{
		quantum = new_quantum;

		CPhysicsObj *ptarget = CPhysicsObj::GetObject(target_info->object_id);

		if (ptarget)
		{
			target_info->quantum = new_quantum;
			ptarget->add_voyeur(physobj->id, target_info->radius, quantum);
		}
	}
}

void TargetManager::AddVoyeur(uint32_t object_id, float radius, double quantum)
{
	if (voyeur_table)
	{
		TargettedVoyeurInfo *existing_info = voyeur_table->lookup(object_id);

		if (existing_info)
		{
			existing_info->radius = radius;
			existing_info->quantum = quantum;
			return;
		}
	}
	else
	{
		voyeur_table = new LongNIHash<TargettedVoyeurInfo>(4);
	}

	TargettedVoyeurInfo *info = new TargettedVoyeurInfo;
	info->object_id = object_id;
	info->radius = radius;
	info->quantum = quantum;
	voyeur_table->add(info, object_id);

	SendVoyeurUpdate(info, &physobj->m_Position, Ok_TargetStatus);
}

void TargetManager::SendVoyeurUpdate(TargettedVoyeurInfo *voyeur, Position *p, TargetStatus status)
{
	voyeur->last_sent_position = *p;

	TargetInfo info;
	info.context_id = 0;
	info.object_id = physobj->id;
	info.quantum = voyeur->quantum;
	info.radius = voyeur->radius;
	info.target_position = physobj->m_Position;
	info.interpolated_position = *p;	
	info.velocity = physobj->get_velocity();
	info.status = status;

	CPhysicsObj *voyObj = CPhysicsObj::GetObject(voyeur->object_id);
	if (voyObj)
		voyObj->receive_target_update(&info);
}

BOOL TargetManager::RemoveVoyeur(uint32_t object_id)
{
	if (voyeur_table)
	{
		TargettedVoyeurInfo *info = voyeur_table->remove(object_id);

		if (info)
		{
			delete info;
			return TRUE;
		}
	}

	return FALSE;
}

void TargetManager::ReceiveUpdate(TargetInfo *target_update)
{
	if (target_info)
	{
		if (target_update->object_id == target_info->object_id)
		{
			*target_info = *target_update;
			target_info->last_update_time = Timer::cur_time;

			target_info->interpolated_heading = physobj->m_Position.get_offset(target_info->interpolated_position);
			

			if (target_info->interpolated_heading.normalize_check_small())
				target_info->interpolated_heading = Vector(0, 0, 1.0f);

			physobj->HandleUpdateTarget(TargetInfo(*target_info));

			if (target_update->status == ExitWorld_TargetStatus)
				ClearTarget();
		}
	}
}

void TargetManager::ClearTarget()
{
	if (target_info)
	{
		CPhysicsObj *targetObj = CPhysicsObj::GetObject(target_info->object_id);
		if (targetObj)
			targetObj->remove_voyeur(physobj->id);

		if (target_info)
			delete target_info;
	
		target_info = NULL;
	}
}

void TargetManager::SetTarget(uint32_t context_id, uint32_t object_id, float radius, double quantum)
{
	ClearTarget();

	if (object_id)
	{
		target_info = new TargetInfo();

		target_info->context_id = context_id;
		target_info->object_id = object_id;
		target_info->radius = radius;
		target_info->quantum = quantum;
		target_info->last_update_time = Timer::cur_time;

		CPhysicsObj *ptarget = CPhysicsObj::GetObject(target_info->object_id);
		if (ptarget)
			ptarget->add_voyeur(physobj->id, target_info->radius, target_info->quantum);
	}
	else
	{
		TargetInfo failed_target_info;
		failed_target_info.context_id = context_id;
		failed_target_info.object_id = 0;
		failed_target_info.status = TimedOut_TargetStatus;
		physobj->HandleUpdateTarget(TargetInfo(failed_target_info));
	}
}

void TargetManager::HandleTargetting()
{
	TargetInfo v5;

	if ((PhysicsTimer::curr_time - last_update_time) >= 0.5)
	{
		if (target_info && target_info->status == Undef_TargetStatus && target_info->last_update_time + 10.0 < Timer::cur_time)
		{
			target_info->status = TimedOut_TargetStatus;
			physobj->HandleUpdateTarget(TargetInfo(*target_info));
		}

		if (voyeur_table)
		{
			LongNIHashIter<TargettedVoyeurInfo> iter(voyeur_table);

			while (!iter.EndReached())
			{
				try
				{

					TargettedVoyeurInfo *pVoyeurInfo = iter.GetCurrentData();

					iter.Next();

					CheckAndUpdateVoyeur(pVoyeurInfo);
				}
				catch (...)
				{
					SERVER_ERROR << "Error in targetting";
				}
			}
		}

		last_update_time = PhysicsTimer::curr_time;
	}
}

void TargetManager::CheckAndUpdateVoyeur(TargettedVoyeurInfo *voyeur)
{
	Position p;
	GetInterpolatedPosition(voyeur->quantum, &p);

	if (p.distance(voyeur->last_sent_position) > voyeur->radius)
		SendVoyeurUpdate(voyeur, &p, Ok_TargetStatus);
}

void TargetManager::GetInterpolatedPosition(double quantum, Position *p)
{
	*p = physobj->m_Position;
	p->frame.m_origin += physobj->get_velocity() * quantum;
}

void TargetManager::NotifyVoyeurOfEvent(TargetStatus _event)
{
	if (voyeur_table)
	{
		LongNIHashIter<TargettedVoyeurInfo> iter(voyeur_table);

		while (!iter.EndReached())
		{
			try
			{
				TargettedVoyeurInfo *pInfo = iter.GetCurrentData();
				iter.Next();

				SendVoyeurUpdate(pInfo, &physobj->m_Position, _event);
			}
			catch(...)
			{
				SERVER_ERROR << "Error in TargetManager.";
			}
		}
	}
}

