
#include <StdAfx.h>
#include "PhatSDK.h"
#include "PositionManager.h"

PositionManager::PositionManager(CPhysicsObj *_physics_obj)
{
	interpolation_manager = 0;
	sticky_manager = 0;
	constraint_manager = 0;
	physics_obj = 0;

	SetPhysicsObject(_physics_obj);
}

PositionManager::~PositionManager()
{
	Destroy();
}

void PositionManager::Destroy()
{
	if (interpolation_manager)
		delete interpolation_manager;

	if (sticky_manager)
		delete sticky_manager;

	if (constraint_manager)
		delete constraint_manager;
}

PositionManager *PositionManager::Create(CPhysicsObj *_physics_obj)
{
	return new PositionManager(_physics_obj);
}

void PositionManager::SetPhysicsObject(CPhysicsObj *_physics_obj)
{
	physics_obj = _physics_obj;
	if (interpolation_manager)
		interpolation_manager->SetPhysicsObject(_physics_obj);

	if (sticky_manager)
		sticky_manager->SetPhysicsObject(_physics_obj);

	if (constraint_manager)
		constraint_manager->SetPhysicsObject(_physics_obj);
}

BOOL PositionManager::IsInterpolating()
{
	return (interpolation_manager ? interpolation_manager->IsInterpolating() : FALSE);
}

void PositionManager::StickTo(uint32_t object_id, float radius, float height)
{
	if (!sticky_manager)
		sticky_manager = StickyManager::Create(physics_obj);

	sticky_manager->StickTo(object_id, radius, height);
}

void PositionManager::UnStick()
{
	if (sticky_manager)
		sticky_manager->HandleExitWorld();
}

void PositionManager::HandleUpdateTarget(TargetInfo target_info)
{
	if (sticky_manager)
		sticky_manager->HandleUpdateTarget(TargetInfo(target_info));
}

uint32_t PositionManager::GetStickyObjectID()
{
	if (sticky_manager)
		return sticky_manager->target_id;

	return 0;
}

void PositionManager::UseTime()
{
	if (interpolation_manager)
		interpolation_manager->UseTime();

	if (constraint_manager)
		constraint_manager->UseTime();

	if (sticky_manager)
		sticky_manager->UseTime();
}

void PositionManager::adjust_offset(Frame *offset, double quantum)
{
	if (interpolation_manager)
		interpolation_manager->adjust_offset(offset, quantum);

	if (sticky_manager)
		sticky_manager->adjust_offset(offset, quantum);

	if (constraint_manager)
		constraint_manager->adjust_offset(offset, quantum);
}

void PositionManager::ConstrainTo(Position *p, float start_distance, float max_distance)
{
	if (!constraint_manager)
		constraint_manager = ConstraintManager::Create(physics_obj);

	if (constraint_manager)
		constraint_manager->ConstrainTo(p, start_distance, max_distance);
}

void PositionManager::UnConstrain()
{
	if (constraint_manager)
		constraint_manager->UnConstrain();
}

BOOL PositionManager::IsFullyConstrained()
{
	if (constraint_manager)
		return constraint_manager->IsFullyConstrained();

	return FALSE;
}

void PositionManager::InterpolateTo(Position *p, int keep_heading)
{
	if (!interpolation_manager)
		interpolation_manager = InterpolationManager::Create(physics_obj);

	interpolation_manager->InterpolateTo(p, keep_heading);
}

const float BIG_DISTANCE = 999999.0;

InterpolationManager::InterpolationManager(CPhysicsObj *new_physobj)
{
	original_distance = BIG_DISTANCE;
	frame_counter = 0;
	progress_quantum = 0;
	node_fail_counter = 0;
	physics_obj = new_physobj;
}

InterpolationManager::~InterpolationManager()
{
	Destroy();
}

void InterpolationManager::Destroy()
{
	position_queue.clear();
}

InterpolationManager *InterpolationManager::Create(CPhysicsObj *_physics_obj)
{
	return new InterpolationManager(_physics_obj);
}

void InterpolationManager::StopInterpolating()
{
	position_queue.clear();

	frame_counter = 0;
	progress_quantum = 0;
	node_fail_counter = 0;
	original_distance = BIG_DISTANCE;
}

void InterpolationManager::UseTime()
{
	if (!physics_obj)
		return;

	if (node_fail_counter > 3 || position_queue.empty())
	{
		if (node_fail_counter <= 0)
			return;

		uint32_t v7 = 0;
		uint32_t v8 = 0;
		Position pos;
		Vector v;
		Position v14;

		auto &backNode = position_queue.back();
		if (backNode.type != 2 && backNode.type != 3)
		{
			Position pos = backNode.p;
			if (physics_obj->SetPositionSimple(&pos, TRUE))
			{
				return;
			}

			StopInterpolating();
			return;
		}
		
		if (position_queue.size() > 1)
		{
			auto nodeIterator = position_queue.begin();

			auto nextNode = nodeIterator;
			nextNode++;

			v = position_queue.back().v;

			do
			{
				if (nodeIterator->type == 1)
				{
					pos = nodeIterator->p;
					v7 = 1;
				}

				nodeIterator++;
				nextNode++;

			} while (nextNode != position_queue.end());

			if (v7)
			{
				Position pos = backNode.p;
				if (physics_obj->SetPositionSimple(&pos, TRUE))
				{
					return;
				}

				physics_obj->set_velocity(v, TRUE);
				StopInterpolating();
				return;
			}
		}

		if (physics_obj->SetPositionSimple(&blipto_position, TRUE))
			return;

		StopInterpolating();
		return;
	}

	auto &currentNode = position_queue.front();
	switch (currentNode.type)
	{
	case 2:
		NodeCompleted(TRUE);
		break;
	case 3:
		physics_obj->set_velocity(currentNode.v, TRUE);
		NodeCompleted(TRUE);
		break;
	}
}

void InterpolationManager::NodeCompleted(BOOL success)
{
	if (!physics_obj)
	{
		return;
	}

	frame_counter = 0;
	progress_quantum = 0;

	InterpolationNode *headNode = NULL;
	if (position_queue.size() >= 1)
	{
		headNode = &*(position_queue.begin());
	}

	InterpolationNode *nextNode = NULL;
	if (position_queue.size() >= 2)
	{
		nextNode = &*(++position_queue.begin());

		if (nextNode->type == 1)
		{
			original_distance = physics_obj->m_Position.distance(nextNode->p);
		}
		else if (!success)
		{
			if (!headNode)
				return;

			blipto_position = headNode->p;
		}
	}
	else
	{
		original_distance = BIG_DISTANCE;

		if (success)
		{
			StopInterpolating();
		}
		else
		{
			if (!headNode)
				return;

			blipto_position = headNode->p;
		}
	}

	if (position_queue.size() >= 1)
		position_queue.pop_front();
}

BOOL InterpolationManager::fUseAdjustedSpeed_ = TRUE;
double MAX_INTERPOLATED_VELOCITY = 7.5;

void InterpolationManager::adjust_offset(Frame *offset, double quantum)
{
	if (position_queue.empty() || !physics_obj || !(physics_obj->transient_state & CONTACT_TS))
	{
		return;
	}

	InterpolationNode *headNode = &position_queue.front();
	if (headNode->type == 2 || headNode->type == 3)
	{
		return;
	}
	
	float curr_distance = physics_obj->m_Position.distance(headNode->p);
	if (curr_distance < 0.05)
	{
		NodeCompleted(TRUE);
		return;
	}

	float my_max_speed = 0.0;
	if (physics_obj->get_minterp())
	{
		double max_speed;

		if (InterpolationManager::fUseAdjustedSpeed_)
		{
			max_speed = physics_obj->get_minterp()->get_adjusted_max_speed();
		}
		else
		{
			max_speed = physics_obj->get_minterp()->get_max_speed();
		}
		my_max_speed = max_speed * 2.0;
	}

	if (my_max_speed < F_EPSILON)
		my_max_speed = MAX_INTERPOLATED_VELOCITY;

	progress_quantum += quantum;
	frame_counter++;
	float progress_made = original_distance - curr_distance;

	if (frame_counter < 5 ||
		(physics_obj->get_sticky_object_id() ||
		  (progress_made >= F_EPSILON && ((progress_made / progress_quantum) / my_max_speed) >= 0.3)))
	{
		if (frame_counter >= 5)
		{
			frame_counter = 0;
			progress_quantum = 0;
			original_distance = curr_distance;
		}

		Frame adjustment = headNode->p.subtract2(&physics_obj->m_Position);

		float progress_madea = my_max_speed * quantum;

		float adjustment_distance = adjustment.m_origin.magnitude();
		if (adjustment_distance <= 0.05)
		{
			NodeCompleted(TRUE);
		}

		if (adjustment_distance > (double)progress_madea)
		{
			adjustment.m_origin *= progress_madea / adjustment_distance;
		}

		if (keep_heading)
		{
			adjustment.set_heading(0.0);
		}

		*offset = adjustment;
		return;
	}

	if (curr_distance < 0.2)
	{
		NodeCompleted(TRUE);
		return;
	}

	node_fail_counter++;
	NodeCompleted(FALSE);
}

void InterpolationManager::InterpolateTo(Position *p, int _keep_heading)
{
	if (!physics_obj)
	{
		return;
	}

	Position *destPos;
	if (!position_queue.empty() && position_queue.back().type == 1)
		destPos = &position_queue.back().p;
	else
		destPos = &physics_obj->m_Position;

	float dist = destPos->distance(*p);

	if (physics_obj->GetAutonomyBlipDistance() >= dist)
	{
		if (physics_obj->m_Position.distance(*p) > 0.05)
		{
			while (!position_queue.empty())
			{
				auto &lastNode = position_queue.back();
				if (lastNode.type != 1 || lastNode.p.distance(*p) >= 0.05)
					break;

				position_queue.pop_back();
			}
			
			while (position_queue.size() >= 20)
			{
				position_queue.pop_front();
			}

			keep_heading = _keep_heading;

			InterpolationNode newNode;
			newNode.type = 1;
			newNode.p = *p;
			if (keep_heading)
			{
				newNode.p.frame.set_heading(physics_obj->get_heading());
			}

			position_queue.push_back(newNode);
		}
		else
		{
			if (!_keep_heading)
			{
				physics_obj->set_heading(p->frame.get_heading(), TRUE);
			}

			StopInterpolating();
		}
	}
	else
	{
		InterpolationNode newNode;
		newNode.type = 1;
		newNode.p = *p;
		if (keep_heading)
		{
			newNode.p.frame.set_heading(physics_obj->get_heading());
		}

		position_queue.push_back(newNode);
		node_fail_counter = 4;
	}
}

BOOL InterpolationManager::IsInterpolating()
{
	return !position_queue.empty();
}

void InterpolationManager::SetPhysicsObject(CPhysicsObj *new_physobj)
{
	physics_obj = new_physobj;
}

StickyManager::StickyManager(CPhysicsObj *_physics_obj)
{
	target_id = 0;
	target_radius = 0;
	physics_obj = 0;
	initialized = 0;

	SetPhysicsObject(_physics_obj);
}

StickyManager::~StickyManager()
{
	Destroy();
}

void StickyManager::Destroy()
{
	if (target_id)
		physics_obj->clear_target();

	target_id = 0;
	target_position = Position();
	initialized = 0;
}

StickyManager *StickyManager::Create(CPhysicsObj *_physics_obj)
{
	return new StickyManager(_physics_obj);
}

void StickyManager::SetPhysicsObject(CPhysicsObj *new_physobj)
{
	if (physics_obj)
	{
		Destroy();
		physics_obj = new_physobj;
	}
	else
	{
		physics_obj = new_physobj;
	}
}

void StickyManager::HandleExitWorld()
{
	if (target_id)
	{
		target_id = 0;
		initialized = 0;
		physics_obj->clear_target();
		physics_obj->cancel_moveto();
	}
}

void StickyManager::HandleUpdateTarget(TargetInfo target_info)
{
	if (target_info.object_id == target_id)
	{
		if (target_info.status == Ok_TargetStatus)
		{
			initialized = 1;
			target_position = target_info.target_position;
		}
		else if (target_id)
		{
			target_id = 0;
			initialized = 0;
			physics_obj->clear_target();
			physics_obj->cancel_moveto();
		}
	}
}

void StickyManager::StickTo(unsigned int _target_id, float _target_radius, float _target_height)
{
	if (target_id)
	{
		target_id = 0;
		initialized = 0;
		physics_obj->clear_target();
		physics_obj->cancel_moveto();
	}

	target_radius = _target_radius;
	target_id = _target_id;
	initialized = 0;
	sticky_timeout_time = Timer::cur_time + 1.0;
	physics_obj->set_target(0, _target_id, 0.5, 0.5);
}

const float STICKY_RADIUS = 0.3f;

void StickyManager::adjust_offset(Frame *offset, double quantum)
{
	if (target_id && initialized)
	{
		CPhysicsObj *target = CPhysicsObj::GetObject(target_id);
		Position *targetPosition = target ? &target->m_Position : &target_position;

		offset->m_origin = physics_obj->m_Position.get_offset(*targetPosition);
		offset->m_origin = physics_obj->m_Position.globaltolocalvec(offset->m_origin);
		offset->m_origin.z = 0;

		float r1 = physics_obj->GetRadius();
		float mag = Position::cylinder_distance_no_z(r1, physics_obj->m_Position, target_radius, *targetPosition) - STICKY_RADIUS;

		// -- inlined
		if (offset->m_origin.normalize_check_small())
			offset->m_origin = Vector(0, 0, 0);
		// --

		// -- surely inlined
		float adjSpeed;
		if (physics_obj->get_minterp())
			adjSpeed = physics_obj->get_minterp()->get_max_speed() * 5.0;
		else
			adjSpeed = 0.0;
		// --

		if (adjSpeed < F_EPSILON)
			adjSpeed = 15.0;

		float deltaSpeed = adjSpeed * quantum;
		if (deltaSpeed >= fabs(mag))
			deltaSpeed = mag;

		offset->m_origin *= deltaSpeed;

		float sought_heading = physics_obj->m_Position.heading(*targetPosition) - physics_obj->m_Position.frame.get_heading();
		if (fabs(sought_heading) < F_EPSILON)
			sought_heading = 0.0;
		if (-F_EPSILON > sought_heading)
			sought_heading = sought_heading + 360.0;

		offset->set_heading(sought_heading);
	}
}

void StickyManager::UseTime()
{
	if (target_id)
	{
		if (Timer::cur_time > sticky_timeout_time)
		{
			target_id = 0;
			initialized = 0;
			physics_obj->clear_target();
			physics_obj->cancel_moveto();
		}
	}
}

ConstraintManager::ConstraintManager(CPhysicsObj *physobj)
{
	physics_obj = 0;
	is_constrained = 0;
	constraint_pos_offset = 0;
	constraint_distance_start = 0;
	constraint_distance_max = 0;

	SetPhysicsObject(physobj);
}

ConstraintManager::~ConstraintManager()
{
}

ConstraintManager *ConstraintManager::Create(CPhysicsObj *physobj)
{
	return new ConstraintManager(physobj);
}

void ConstraintManager::SetPhysicsObject(CPhysicsObj *new_physobj)
{
	if (physics_obj)
	{
		physics_obj = NULL;
		is_constrained = 0;
		constraint_pos_offset = 0;
		physics_obj = new_physobj;
	}
	else
	{
		physics_obj = new_physobj;
	}
}

void ConstraintManager::ConstrainTo(Position *p, float start_distance, float max_distance)
{
	is_constrained = 1;

	constraint_pos = *p;
	constraint_distance_start = start_distance;
	constraint_distance_max = max_distance;
	constraint_pos_offset = p->distance(physics_obj->m_Position);
}

BOOL ConstraintManager::IsFullyConstrained()
{
	return ((constraint_distance_max * 0.9) < constraint_pos_offset);
}

void ConstraintManager::UseTime()
{
	// doesn't do anything
}

void ConstraintManager::UnConstrain()
{
	is_constrained = 0;
}

void ConstraintManager::adjust_offset(Frame *offset, double quantum)
{
	float _rhs;

	if (physics_obj && is_constrained)
	{
		if (physics_obj->transient_state & CONTACT_TS)
		{
			if (constraint_pos_offset < (double)constraint_distance_max)
			{
				if (constraint_pos_offset >(double)constraint_distance_start)
				{
					_rhs = (constraint_distance_max - constraint_pos_offset) / (constraint_distance_max - constraint_distance_start);
					offset->m_origin *= _rhs;
				}
			}
			else
			{
				offset->m_origin = Vector(0, 0, 0);
			}
		}

		constraint_pos_offset += offset->m_origin.magnitude();
	}
}