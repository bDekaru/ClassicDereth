
#include <StdAfx.h>
#include "MovementManager.h"
#include "PhysicsObj.h"

const float backwards_factor = 6.4999998e-1f;
const float walk_anim_speed = 3.1199999f;
const float sidestep_anim_speed = 1.25f;
const float sidestep_factor = 0.5;
const float max_sidestep_anim_rate = 3.0f;
const float run_turn_factor = 1.5f;
const float run_anim_speed = 4.0f;

MovementStruct::MovementStruct() : type(MovementTypes::Invalid)
{
}

MovementManager::MovementManager()
{
	motion_interpreter = NULL;
	moveto_manager = NULL;
	physics_obj = NULL;
	weenie_obj = NULL;
}

MovementManager::~MovementManager()
{
	Destroy();
}

MovementManager *MovementManager::Create(CPhysicsObj *pPObject, CWeenieObject *pWObject)
{
	MovementManager *pManager = new MovementManager;
	pManager->SetPhysicsObject(pPObject);
	pManager->SetWeenieObject(pWObject);

	return pManager;
}

void MovementManager::Destroy()
{
	if (motion_interpreter)
		delete motion_interpreter;

	motion_interpreter = NULL;

	if (moveto_manager)
		delete moveto_manager;

	moveto_manager = NULL;
}

void MovementManager::MakeMoveToManager()
{
	if (!moveto_manager)
	{
		moveto_manager = MoveToManager::Create(physics_obj, weenie_obj);
	}
}

void MovementManager::EnterDefaultState()
{
	if (!physics_obj)
		return;

	get_minterp()->enter_default_state();
}

void MovementManager::HandleExitWorld()
{
	if (motion_interpreter)
		motion_interpreter->HandleExitWorld();
}

BOOL MovementManager::IsMovingTo()
{
	if (!moveto_manager)
		return FALSE;

	if (!moveto_manager->is_moving_to())
		return FALSE;

	return TRUE;
}

void MovementManager::HandleUpdateTarget(TargetInfo target_info)
{
	if (moveto_manager)
		moveto_manager->HandleUpdateTarget(&target_info);
}

void MovementManager::CancelMoveTo(uint32_t err)
{
	if (moveto_manager)
		moveto_manager->CancelMoveTo(err);
}

uint32_t MovementManager::PerformMovement(MovementStruct &mvs)
{
	physics_obj->set_active(TRUE);

	switch (mvs.type)
	{
	case RawCommand:
	case InterpretedCommand:
	case StopRawCommand:
	case StopInterpretedCommand:
	case StopCompletely:
		get_minterp();

		return motion_interpreter->PerformMovement(mvs);

	case MoveToObject:
	case MoveToPosition:
	case TurnToObject:
	case TurnToHeading:
		MakeMoveToManager();

		return moveto_manager->PerformMovement(mvs);

	default:
		return WERROR_GENERAL_MOVEMENT_FAILURE;
	}
}

void MovementManager::UseTime()
{
	if (moveto_manager)
		moveto_manager->UseTime();
}

void MovementManager::HandleEnterWorld()
{
	if (motion_interpreter)
		motion_interpreter->HandleEnterWorld();
}

CMotionInterp *MovementManager::get_minterp()
{
	if (!motion_interpreter)
	{
		motion_interpreter = CMotionInterp::Create(physics_obj, weenie_obj);

		if (physics_obj)
			motion_interpreter->enter_default_state();
	}

	return motion_interpreter;
}

BOOL MovementManager::motions_pending()
{
	if (!motion_interpreter)
		return FALSE;
	if (!motion_interpreter->motions_pending())
		return FALSE;

	return TRUE;
}

void MovementManager::SetPhysicsObject(CPhysicsObj *pObject)
{
	physics_obj = pObject;

	if (motion_interpreter)
		motion_interpreter->SetPhysicsObject(pObject);

	if (moveto_manager)
		moveto_manager->SetPhysicsObject(pObject);
}

void MovementManager::SetWeenieObject(CWeenieObject *pObject)
{
	weenie_obj = pObject;

	if (motion_interpreter)
		motion_interpreter->SetWeenieObject(pObject);

	if (moveto_manager)
		moveto_manager->SetWeenieObject(pObject);
}

void MovementManager::HitGround()
{
	if (motion_interpreter)
		motion_interpreter->HitGround();

	if (moveto_manager)
		moveto_manager->HitGround();
}

void MovementManager::LeaveGround()
{
	if (motion_interpreter)
		motion_interpreter->LeaveGround();

	if (moveto_manager)
		moveto_manager->LeaveGround();
}

BOOL MovementManager::unpack_movement(BinaryReader *pReader)
{
	if (!motion_interpreter || !physics_obj)
		return FALSE;

	physics_obj->cancel_moveto();
	physics_obj->unstick_from_object();

	MovementParameters params;
	Position pos;
	InterpretedMotionState state;

	uint32_t pack_word = pReader->Read<WORD>();
	uint32_t style = GetCommandID(pReader->Read<WORD>());

	uint32_t currentStyle = motion_interpreter->InqStyle();

	if (style != currentStyle)
	{
		motion_interpreter->DoMotion(style, &params);
	}

	switch (pack_word & 0xFF)
	{
	case 0:
		{
			InterpretedMotionState interpState;
			interpState.UnPack(pReader);

			uint32_t target = 0;
			if (pack_word & 0x100)
				target = pReader->Read<uint32_t>();
			
			move_to_interpreted_state(interpState);
			if (target)
			{
				physics_obj->stick_to_object(target);
			}

			motion_interpreter->standing_longjump = pack_word & 0x200;
			break;
		}
	default:
		{
			UNFINISHED();
			break;
		}
	}

	return TRUE;
}

BOOL MovementManager::move_to_interpreted_state(const InterpretedMotionState &state)
{
	return get_minterp()->move_to_interpreted_state(state);
}

void MovementManager::MotionDone(uint32_t motion, BOOL success)
{
	if (motion_interpreter)
		motion_interpreter->MotionDone(success);
}

CMotionInterp::CMotionInterp(CPhysicsObj *pPObject, CWeenieObject *pWObject)
{
	initted = 0;
	weenie_obj = NULL;
	physics_obj = NULL;

	current_speed_factor = 1.0;
	my_run_rate = 1.0;

	standing_longjump = 0;
	jump_extent = 0;
	server_action_stamp = 0;

	SetWeenieObject(pWObject);
	SetPhysicsObject(pPObject);
}

CMotionInterp::~CMotionInterp()
{
	Destroy();
}

void CMotionInterp::Destroy()
{
	pending_motions.clear();
}

CMotionInterp* CMotionInterp::Create(CPhysicsObj *pPObject, CWeenieObject *pWObject)
{
	CMotionInterp *pInterp = new CMotionInterp(pPObject, pWObject);

	if (!pInterp)
		return NULL;

	return pInterp;
}

void CMotionInterp::HandleExitWorld()
{
	for (std::list<MotionNode>::iterator i = pending_motions.begin(); i != pending_motions.end(); i++)
	{
		if (physics_obj)
		{
			if (i->motion & CM_Action)
			{
				physics_obj->unstick_from_object();
				interpreted_state.RemoveAction();
				raw_state.RemoveAction();
			}
		}
	}
	pending_motions.clear();
}

void CMotionInterp::MotionDone(BOOL success)
{
	if (!physics_obj)
		return;

	std::list<MotionNode>::iterator motionData = pending_motions.begin();
	if (motionData != pending_motions.end())
	{
		if (motionData->motion & CM_Action)
		{
			physics_obj->unstick_from_object();
			interpreted_state.RemoveAction();
			raw_state.RemoveAction();
		}

		motionData = pending_motions.begin();
		if (motionData != pending_motions.end())
		{
			pending_motions.erase(motionData);
		}
	}
}

void CMotionInterp::SetHoldKey(HoldKey key, BOOL cancel_moveto)
{
	if (key != raw_state.current_holdkey)
	{
		switch (key)
		{
		case HoldKey_None:

			if (raw_state.current_holdkey == HoldKey_Run)
			{
				raw_state.current_holdkey = HoldKey_None;
				apply_current_movement(cancel_moveto, FALSE);
			}

			break;
		}
	}
}

uint32_t CMotionInterp::DoMotion(uint32_t MotionID, MovementParameters *Params)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	MovementParameters CurrentParams(*Params);
	uint32_t CurrentMotion = MotionID;

	if (Params->cancel_moveto)
		physics_obj->cancel_moveto();

	if (Params->set_hold_key)
		SetHoldKey(Params->hold_key_to_apply, Params->cancel_moveto);

	adjust_motion(CurrentMotion, CurrentParams.speed, Params->hold_key_to_apply);

	if (interpreted_state.current_style != Motion_NonCombat) //Motion_NonCombat
	{
		switch (MotionID)
		{
		case Motion_Crouch: //Motion_Crouch
			return WERROR_CANT_CROUCH_IN_COMBAT;
		case Motion_Sitting: //Motion_Sitting
			return WERROR_CANT_SIT_IN_COMBAT;
		case Motion_Sleeping: //Motion_Sleeping
			return WERROR_CANT_LIE_DOWN_IN_COMBAT;
		}

		if (MotionID & CM_ChatEmote)
			return WERROR_CANT_CHAT_EMOTE_IN_COMBAT;
	}

	if (MotionID & CM_Action)
	{
		if (interpreted_state.GetNumActions() >= 6)
			return WERROR_TOO_MANY_ACTIONS;
	}

	uint32_t NewMotion = DoInterpretedMotion(CurrentMotion, &CurrentParams);

	if (!NewMotion && Params->modify_raw_state)
		raw_state.ApplyMotion(MotionID, Params);

	return NewMotion;
}

uint32_t CMotionInterp::StopMotion(uint32_t MotionID, MovementParameters *Params)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	if (Params->cancel_moveto)
		physics_obj->cancel_moveto();

	uint32_t                CurrentMotion = MotionID;
	MovementParameters   CurrentParams(*Params);

	adjust_motion(CurrentMotion, CurrentParams.speed, Params->hold_key_to_apply);

	uint32_t NewMotion = StopInterpretedMotion(CurrentMotion, &CurrentParams);

	if (!NewMotion && Params->modify_raw_state)
		raw_state.RemoveMotion(MotionID);

	return NewMotion;
}

uint32_t CMotionInterp::motion_allows_jump(uint32_t mid)
{
	if (((mid >= 0x40000016) && (mid <= 0x40000018)) ||
		((mid >= 0x10000128) && (mid <= 0x10000131)) ||
		((mid >= 0x1000006F) && (mid <= 0x10000078)) ||
		((mid >= 0x41000012) && (mid <= 0x41000014)) ||
		((mid >= 0x4000001E) && (mid <= 0x40000039)) ||
		((mid == Motion_Fallen)))
		return WERROR_CANT_JUMP_POSITION;

	return WERROR_NONE;
}

BOOL CMotionInterp::contact_allows_move(uint32_t mid)
{
	if (!physics_obj)
		return FALSE;

	if (mid == Motion_Dead || mid == Motion_Falling || (mid >= Motion_TurnRight && mid <= Motion_TurnLeft))
		return TRUE;

	if (weenie_obj && !weenie_obj->IsCreature())
	    return TRUE;

	if (!physics_obj)
		return TRUE;
	if (!(physics_obj->m_PhysicsState & GRAVITY_PS))
		return TRUE;
	if (!(physics_obj->transient_state & 0x1))
		return FALSE;
	if (physics_obj->transient_state & 0x2)
		return TRUE;

	return FALSE;
}

BOOL CMotionInterp::move_to_interpreted_state(const InterpretedMotionState &state)
{
	if (!physics_obj)
		return FALSE;

	raw_state.current_style = state.current_style;
	physics_obj->cancel_moveto();

	uint32_t maj = ((motion_allows_jump(interpreted_state.forward_command)) ? TRUE : FALSE);

	interpreted_state.copy_movement_from(state);
	apply_current_movement(1, maj);

	MovementParameters params;

	for (std::list<ActionNode>::const_iterator i = state.actions.cbegin(); i != state.actions.cend(); i++)
	{
		// no idea what is happening here
		WORD stamp1 = i->stamp & 0x7FFF;
		WORD stamp2 = server_action_stamp & 0x7FFF;

		uint32_t val;
		if (stamp1 > stamp2)
			val = stamp1 - stamp2;
		else
			val = stamp2 - stamp1;

		bool val2;
		if (val <= 0x3FFF)
			val2 = stamp2 < stamp1;
		else
			val2 = stamp1 < stamp2;

		if (val2)
		{
			if ((weenie_obj && weenie_obj->IsCreature()) || i->autonomous)
			{
				server_action_stamp = i->stamp;
				params.speed = i->speed;
				params.autonomous = i->autonomous;
				DoInterpretedMotion(i->action, &params);
			}
		}
	}

	return TRUE;
}

uint32_t CMotionInterp::StopInterpretedMotion(uint32_t mid, MovementParameters *params)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	uint32_t var_ebp;

	if (contact_allows_move(mid))
	{
		if (standing_longjump && ((mid == Motion_WalkForward) || (mid == Motion_RunForward) || (mid == Motion_SideStepRight)))
		{
			if (params->modify_interpreted_state)
				interpreted_state.RemoveMotion(mid);

			var_ebp = WERROR_NONE;
		}
		else
		{
			var_ebp = physics_obj->StopInterpretedMotion(mid, params);

			if (!var_ebp)
			{
				add_to_queue(params->context_id, Motion_Ready, WERROR_NONE);

				if (params->modify_interpreted_state)
					interpreted_state.RemoveMotion(mid);
			}
		}
	}
	else
	{
		if (params->modify_interpreted_state)
			interpreted_state.RemoveMotion(mid);

		var_ebp = 0;
	}

	// Inlined.
	if (physics_obj)
	{
		if (!physics_obj->cell)
			physics_obj->RemoveLinkAnimations();
	}

	return var_ebp;
}

uint32_t CMotionInterp::DoInterpretedMotion(uint32_t motion, MovementParameters *params)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	uint32_t var_ebp;

	if (contact_allows_move(motion))
	{
		if (standing_longjump && ((motion == Motion_WalkForward) || (motion == Motion_RunForward) || (motion == Motion_SideStepRight)))
		{
			if (params->modify_interpreted_state)
				interpreted_state.ApplyMotion(motion, params);

			var_ebp = WERROR_NONE;
		}
		else
		{
			if (motion == Motion_Dead)
				physics_obj->RemoveLinkAnimations();

			var_ebp = physics_obj->DoInterpretedMotion(motion, params);

			if (!var_ebp)
			{
				uint32_t jump_error_code;

				if (params->disable_jump_during_link)
					jump_error_code = WERROR_CANT_JUMP_POSITION;
				else
				{
					jump_error_code = motion_allows_jump(motion);

					if (!jump_error_code && !(motion & CM_Action))
						jump_error_code = motion_allows_jump(interpreted_state.forward_command);
				}

				add_to_queue(params->context_id, motion, jump_error_code);

				if (params->modify_interpreted_state)
					interpreted_state.ApplyMotion(motion, params);
			}
		}
	}
	else
	{
		if (motion & CM_Action)
			var_ebp = WERROR_NO_CONTACT;
		else
		{
			if (params->modify_interpreted_state)
				interpreted_state.ApplyMotion(motion, params);

			var_ebp = WERROR_NONE;
		}
	}

	// Inlined.
	if (physics_obj)
	{
		if (!physics_obj->cell)
			physics_obj->RemoveLinkAnimations();
	}

	return var_ebp;
}

void CMotionInterp::adjust_motion(uint32_t &motion, float &speed, HoldKey key)
{
	if (weenie_obj && !weenie_obj->IsCreature())
		return;

	switch (motion)
	{
	case Motion_RunForward:
		return;

	case Motion_WalkBackwards:
		motion = Motion_WalkForward;
		speed *= -backwards_factor;
		break;

	case Motion_TurnLeft:
		motion = Motion_TurnRight;
		speed *= -1.0;
		break;

	case Motion_SideStepLeft:
		motion = Motion_SideStepRight;
		speed *= -1.0;

	case Motion_SideStepRight:
		speed = speed * sidestep_factor * (walk_anim_speed / sidestep_anim_speed);
		break;
	}

	BOOL currentHoldKey = key;

	if (!currentHoldKey)
		currentHoldKey = raw_state.current_holdkey;

	if (currentHoldKey == HoldKey_Run)
		apply_run_to_command(motion, speed);
}

void CMotionInterp::apply_run_to_command(uint32_t &motion, float &speed)
{
	float speedMod;

	if (weenie_obj)
	{
		float run_factor;
		if (weenie_obj->InqRunRate(run_factor))
			speedMod = run_factor;
		else
			speedMod = my_run_rate;
	}
	else
		speedMod = 1.0f;

	switch (motion)
	{
	case Motion_WalkForward:
		if (speed > 0)
			motion = Motion_RunForward;

		speed *= speedMod;
		break;

	case Motion_TurnRight:
		speed *= run_turn_factor;
		break;

	case Motion_SideStepRight:
		speed *= speedMod;

		if (max_sidestep_anim_rate < abs(speed))
		{
			if (speed > 0)
				speed = max_sidestep_anim_rate * 1.0f;
			else
				speed = max_sidestep_anim_rate * -1.0f;
		}
		break;
	}
}

void CMotionInterp::apply_current_movement(BOOL cancel_moveto, BOOL disallow_jump)
{
	if (!physics_obj)
		return;
	if (initted == 0)
		return;

	if ((weenie_obj && !weenie_obj->IsCreature()) || !physics_obj->movement_is_autonomous())
		apply_interpreted_movement(cancel_moveto, disallow_jump);
	else
		apply_raw_movement(cancel_moveto, disallow_jump);
}

void CMotionInterp::apply_raw_movement(BOOL cancel_moveto, BOOL disallow_jump)
{
	if (!physics_obj)
		return;

	interpreted_state.current_style = raw_state.current_style;
	interpreted_state.forward_command = raw_state.forward_command;
	interpreted_state.forward_speed = raw_state.forward_speed;
	interpreted_state.sidestep_command = raw_state.sidestep_command;
	interpreted_state.sidestep_speed = raw_state.sidestep_speed;
	interpreted_state.turn_command = raw_state.turn_command;
	interpreted_state.turn_speed = raw_state.turn_speed;

	adjust_motion(interpreted_state.forward_command, interpreted_state.forward_speed, raw_state.forward_holdkey);
	adjust_motion(interpreted_state.sidestep_command, interpreted_state.sidestep_speed, raw_state.sidestep_holdkey);
	adjust_motion(interpreted_state.turn_command, interpreted_state.turn_speed, raw_state.turn_holdkey);

	apply_interpreted_movement(cancel_moveto, disallow_jump);
}

void CMotionInterp::apply_interpreted_movement(BOOL cancel_moveto, BOOL disallow_jump)
{
	if (!physics_obj)
		return;

	MovementParameters params;

	params.set_hold_key = 0;
	params.modify_interpreted_state = 0;
	params.cancel_moveto = 0;
	params.disable_jump_during_link = 0;

	if (cancel_moveto)
		params.cancel_moveto = 1;
	if (disallow_jump)
		params.disable_jump_during_link = 1;

	if (interpreted_state.forward_command == Motion_RunForward)
		my_run_rate = interpreted_state.forward_speed;

	DoInterpretedMotion(interpreted_state.current_style, &params);

	if (contact_allows_move(interpreted_state.forward_command))
	{
		if (!standing_longjump)
		{
			params.speed = interpreted_state.forward_speed;
			DoInterpretedMotion(interpreted_state.forward_command, &params);

			if (interpreted_state.sidestep_command)
			{
				params.speed = interpreted_state.sidestep_speed;
				DoInterpretedMotion(interpreted_state.sidestep_command, &params);
			}
			else
				StopInterpretedMotion(Motion_SideStepRight, &params);
		}
		else
		{
			params.speed = 1.0f;
			DoInterpretedMotion(Motion_Ready, &params);
			StopInterpretedMotion(Motion_SideStepRight, &params);
		}
	}
	else
	{
		params.speed = 1.0f;
		DoInterpretedMotion(Motion_Falling, &params);
	}

	if (interpreted_state.turn_command)
	{
		params.speed = interpreted_state.turn_speed;
		DoInterpretedMotion(interpreted_state.turn_command, &params);
	}
	else
	{
		// Inlined from somewhere.
		if (!physics_obj)
			return;

		uint32_t x = physics_obj->StopInterpretedMotion(Motion_TurnRight, &params);

		if (!x)
		{
			add_to_queue(params.context_id, Motion_Ready, 0 /*x*/);

			if (params.modify_interpreted_state)
				interpreted_state.RemoveMotion(Motion_TurnRight);
		}

		// Inlined from somewhere.
		if (!physics_obj)
			return;

		if (!physics_obj->cell)
			physics_obj->RemoveLinkAnimations();
	}
}

void CMotionInterp::enter_default_state()
{
	raw_state = RawMotionState();
	interpreted_state = InterpretedMotionState();

	physics_obj->InitializeMotionTables();

	add_to_queue(0, Motion_Ready, WERROR_NONE);

	initted = 1;

	LeaveGround();
}

void CMotionInterp::LeaveGround()
{
	if (!physics_obj)
		return;

	if (weenie_obj)
	{
		if (!weenie_obj->IsCreature())
			return;
	}

	if (!physics_obj || !(physics_obj->m_PhysicsState & GRAVITY_PS))
		return;

	Vector velocity;
	get_leave_ground_velocity(velocity);

	physics_obj->set_local_velocity(velocity, 1);

	standing_longjump = 0;
	jump_extent = 0;

	physics_obj->RemoveLinkAnimations();
	apply_current_movement(0, 0);
}

void CMotionInterp::get_leave_ground_velocity(Vector& v)
{
	v = get_state_velocity();
	v.z = get_jump_v_z();

	if (!v.is_zero())
	{
		v = physics_obj->m_Position.globaltolocalvec(v);
	}
}

Vector CMotionInterp::get_state_velocity()
{
	Vector sv;

	if (interpreted_state.sidestep_command == Motion_SideStepRight)
		sv.x = sidestep_anim_speed * interpreted_state.sidestep_speed;
	else
		sv.x = 0.0f;

	if (interpreted_state.forward_command == Motion_WalkForward)
		sv.y = walk_anim_speed * interpreted_state.forward_speed;
	else if (interpreted_state.forward_command == Motion_RunForward)
		sv.y = run_anim_speed * interpreted_state.forward_speed;
	else
		sv.y = 0.0f;

	sv.z = 0.0f;

	float runRate = 1.0f;

	if (weenie_obj)
	{
		float _runRate;
		if (weenie_obj->InqRunRate(_runRate))
			runRate = _runRate;
		else
			runRate = my_run_rate;
	}

	float constraint = runRate * run_anim_speed;

	if (sv.magnitude() > constraint)
	{
		sv.normalize();
		sv *= constraint;
	}

	return sv;
}

float CMotionInterp::get_jump_v_z()
{
	float extent = jump_extent;

	if (extent < F_EPSILON)
		return 0.0f;

	if (extent > 1.0f)
		extent = 1.0f;

	if (!weenie_obj)
		return 10.0f;

	float vz = extent;
	if (weenie_obj->InqJumpVelocity(extent, vz))
		return vz;
	
	return 0.0f;
}

double CMotionInterp::get_adjusted_max_speed()
{
	double rate;

	if (weenie_obj)
	{
		float run_factor;
		if (weenie_obj->InqRunRate(run_factor))
			rate = run_factor;
		else
			rate = my_run_rate;
	}
	else
	{
		rate = 1.0;
	}

	if (interpreted_state.forward_command == Motion_RunForward)
	{
		rate = interpreted_state.forward_speed / current_speed_factor;
	}

	return rate * 4.0;
}

float CMotionInterp::get_max_speed(void)
{
	float speed = 0.0f;

	if (weenie_obj)
	{
		if (!weenie_obj->InqRunRate(speed))
			speed = my_run_rate;
	}
	else
		speed = 1.0f;

	return (speed * run_anim_speed);
}

uint32_t CMotionInterp::jump(float extent, int32_t &stamina_adjustment)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	physics_obj->cancel_moveto();

	uint32_t allowed = jump_is_allowed(extent, stamina_adjustment);

	if (allowed)
		standing_longjump = 0;
	else
	{
		jump_extent = extent;
		physics_obj->set_on_walkable(allowed);
	}

	return allowed;
}

uint32_t CMotionInterp::jump_charge_is_allowed()
{
	if (!weenie_obj || weenie_obj->CanJump(jump_extent))
	{
		uint32_t fwd_command = interpreted_state.forward_command;

		if (fwd_command == Motion_Fallen || (fwd_command > 0x41000011 && fwd_command <= 0x41000014))
			return WERROR_CANT_JUMP_POSITION;
		
		return 0;
	}

	return WERROR_CANT_JUMP_LOAD;
}

uint32_t CMotionInterp::jump_is_allowed(float extent, int32_t &stamina_cost)
{
	if (physics_obj && ((weenie_obj && !weenie_obj->IsCreature()) || !physics_obj || !(physics_obj->m_PhysicsState & GRAVITY_PS) || ((physics_obj->transient_state & CONTACT_TS) && (physics_obj->transient_state & ON_WALKABLE_TS))))
	{
		if (physics_obj->IsFullyConstrained())
			return WERROR_GENERAL_MOVEMENT_FAILURE;

		auto pendingMotionHead = pending_motions.begin();
		if (pendingMotionHead != pending_motions.end() && pendingMotionHead->jump_error_code)
			return pendingMotionHead->jump_error_code;

		uint32_t jumpError = jump_charge_is_allowed();

		if (!jumpError)
		{
			jumpError = motion_allows_jump(interpreted_state.forward_command);

			if (!jumpError)
			{
				if (weenie_obj && !(weenie_obj->JumpStaminaCost(extent, stamina_cost)))
					jumpError = WERROR_GENERAL_MOVEMENT_FAILURE;
			}
		}

		return jumpError;
	}

	return WERROR_NO_CONTACT;
}

uint32_t CMotionInterp::StopCompletely()
{
	if (physics_obj == NULL)
		return WERROR_NO_PHYSICS_OBJECT;

	physics_obj->cancel_moveto();

	uint32_t maj = motion_allows_jump(interpreted_state.forward_command);

	raw_state.forward_command = Motion_Ready;
	raw_state.forward_speed = 1.0f;
	raw_state.sidestep_command = 0;
	raw_state.turn_command = 0;
	interpreted_state.forward_command = Motion_Ready;
	interpreted_state.forward_speed = 1.0f;
	interpreted_state.sidestep_command = 0;
	interpreted_state.turn_command = 0;

	physics_obj->StopCompletely_Internal();

	add_to_queue(0, Motion_Ready, maj);

	if (physics_obj && !physics_obj->cell)
		physics_obj->RemoveLinkAnimations();

	return 0;
}

void CMotionInterp::HandleEnterWorld()
{
}

void CMotionInterp::add_to_queue(uint32_t context_id, uint32_t motion, uint32_t jump_error_code)
{
	pending_motions.push_back(MotionNode(context_id, motion, jump_error_code));
}

BOOL CMotionInterp::motions_pending()
{
	return !!pending_motions.size();
}

void CMotionInterp::SetPhysicsObject(CPhysicsObj *pObject)
{
	physics_obj = pObject;

	apply_current_movement(TRUE, 0);
}

void CMotionInterp::SetWeenieObject(CWeenieObject *pObject)
{
	weenie_obj = pObject;

	apply_current_movement(TRUE, 0);
}

uint32_t CMotionInterp::PerformMovement(MovementStruct& cmd)
{
	uint32_t x;

	switch (cmd.type)
	{
	case MovementTypes::RawCommand:
		x = DoMotion(cmd.motion, cmd.params);
		break;

	case MovementTypes::InterpretedCommand:
		x = DoInterpretedMotion(cmd.motion, cmd.params);
		break;

	case MovementTypes::StopRawCommand:
		x = StopMotion(cmd.motion, cmd.params);
		break;

	case MovementTypes::StopInterpretedCommand:
		x = StopInterpretedMotion(cmd.motion, cmd.params);;
		break;

	case MovementTypes::StopCompletely:
		x = StopCompletely();
		break;

	default:
		return WERROR_GENERAL_MOVEMENT_FAILURE; // 0x47;
	}

	physics_obj->CheckForCompletedMotions();
	return x;
}

void CMotionInterp::HitGround()
{
	if (!physics_obj)
		return;

	if (weenie_obj)
	{
		if (!weenie_obj->IsCreature())
			return;
	}

	if (!physics_obj || !(physics_obj->m_PhysicsState & GRAVITY_PS))
		return;

	physics_obj->RemoveLinkAnimations();
	apply_current_movement(0, 0);
}

MoveToManager::MoveToManager()
{
	physics_obj = NULL;
	weenie_obj = NULL;

	InitializeLocalVariables();
}

MoveToManager::~MoveToManager()
{
	Destroy();
}

void MoveToManager::Destroy()
{
	// Finish me.
}

void MoveToManager::CleanUp()
{
	MovementParameters Params;

	Params.hold_key_to_apply = movement_params.hold_key_to_apply;
	Params.cancel_moveto = 0;

	if (physics_obj)
	{
		if (current_command)
			_StopMotion(current_command, &Params);

		if (aux_command)
			_StopMotion(aux_command, &Params);

		if (top_level_object_id && movement_type)
			physics_obj->clear_target();
	}

	InitializeLocalVariables();
}

MoveToManager* MoveToManager::Create(CPhysicsObj *pPhysicsObj, CWeenieObject *pWeenieObj)
{
	MoveToManager *pManager = new MoveToManager();

	if (!pManager)
		return NULL;

	pManager->SetPhysicsObject(pPhysicsObj);
	pManager->SetWeenieObject(pWeenieObj);

	return pManager;
}

void MoveToManager::SetPhysicsObject(CPhysicsObj *pPhysicsObj)
{
	physics_obj = pPhysicsObj;
}

void MoveToManager::SetWeenieObject(CWeenieObject *pWeenieObj)
{
	weenie_obj = pWeenieObj;
}

uint32_t MoveToManager::_StopMotion(uint32_t motion, MovementParameters *Params)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	if (!physics_obj->get_minterp())
		return WERROR_NO_MOTION_INTERPRETER;

	physics_obj->get_minterp()->adjust_motion(motion, Params->speed, Params->hold_key_to_apply);
	return physics_obj->get_minterp()->StopInterpretedMotion(motion, Params);
}

void MoveToManager::InitializeLocalVariables()
{
	movement_type = MovementTypes::Invalid;

	movement_params.distance_to_object = 0;
	movement_params.context_id = 0;

	previous_distance_time = Timer::cur_time;
	original_distance_time = Timer::cur_time;

	previous_heading = 0.0f;
	
	fail_progress_count = 0;
	current_command = 0;
	aux_command = 0;
	moving_away = 0;
	initialized = 0;

	sought_position = Position();
	current_target_position = Position();

	sought_object_id = 0;
	top_level_object_id = 0;
	sought_object_radius = 0;
	sought_object_height = 0;
}

void MoveToManager::HandleUpdateTarget(TargetInfo *target_info)
{
	if (physics_obj)
	{
		if (top_level_object_id == target_info->object_id)
		{
			if (initialized)
			{
				if (target_info->status == Ok_TargetStatus)
				{
					if (movement_type == MovementTypes::MoveToObject)
					{
						sought_position = target_info->interpolated_position;
						current_target_position = target_info->target_position;
						previous_distance = FLT_MAX;
						previous_distance_time = Timer::cur_time;
						original_distance = FLT_MAX;
						original_distance_time = Timer::cur_time;
					}
				}
				else
				{
					CancelMoveTo(WERROR_OBJECT_GONE);
				}
			}
			else if (top_level_object_id == physics_obj->id)
			{
				sought_position = physics_obj->m_Position;
				current_target_position = physics_obj->m_Position;
				CleanUpAndCallWeenie(WERROR_NONE);
			}
			else if (target_info->status == Ok_TargetStatus)
			{
				if (movement_type == MovementTypes::MoveToObject)
				{
					MoveToObject_Internal(&target_info->target_position, &target_info->interpolated_position);
				}
				else if (movement_type == MovementTypes::TurnToObject)
				{
					TurnToObject_Internal(&target_info->target_position);
				}
			}
			else
			{
				CancelMoveTo(WERROR_NO_OBJECT);
			}
		}
	}
	else
	{
		CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
	}
}

void MoveToManager::MoveToObject_Internal(Position *_target_position, Position *interpolated_position)
{
	float distance;
	unsigned int command; 
	int move_away;
	HoldKey hold_key;

	if (physics_obj)
	{
		sought_position = *interpolated_position;
		current_target_position = *_target_position;
		
		float h1 = physics_obj->m_Position.heading(*interpolated_position);
		float heading_to_object = h1 - physics_obj->get_heading();

		distance = GetCurrentDistance();

		if (fabs(heading_to_object) < F_EPSILON)
			heading_to_object = 0.0;
		if (-F_EPSILON > heading_to_object)
			heading_to_object = heading_to_object + 360.0;

		movement_params.get_command(distance, heading_to_object, &command, &hold_key, &move_away);
		if (command)
		{
			AddTurnToHeadingNode(h1);
			AddMoveToPositionNode();
		}

		if (movement_params.use_final_heading)
		{
			float h2 = h1 + movement_params.desired_heading;
			if (h2 >= 360.0)
				h2 -= 360.0;
			AddTurnToHeadingNode(h2);
		}

		initialized = 1;
		BeginNextNode();
	}
	else
	{
		CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
	}
}

void MoveToManager::TurnToObject_Internal(Position *_target_position)
{
	if (physics_obj)
	{
		current_target_position = *_target_position;

		float headingGoal = fmod(physics_obj->m_Position.heading(current_target_position) + sought_position.frame.get_heading(), 360.0);
		sought_position.frame.set_heading(headingGoal);

		MovementNode node;
		node.type = MovementTypes::TurnToHeading;
		node.heading = headingGoal;
		pending_actions.push_back(node);

		initialized = 1;
		BeginNextNode();
	}
	else
	{
		MoveToManager::CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
	}
}

void MoveToManager::CancelMoveTo(uint32_t retval)
{
	if (!movement_type)
		return;

	pending_actions.clear();
	CleanUpAndCallWeenie(retval);
}

void MoveToManager::CleanUpAndCallWeenie(uint32_t retval)
{
	CleanUp();

	if (physics_obj)
		physics_obj->StopCompletely(WERROR_NONE);

	// custom
	if (weenie_obj)
		weenie_obj->HandleMoveToDone(retval);
	//
}

BOOL MoveToManager::is_moving_to()
{
	return (movement_type ? TRUE : FALSE);
}

void MoveToManager::MoveToObject(uint32_t object_id, uint32_t top_level_id, float object_radius, float object_height, MovementParameters *params)
{
	if (physics_obj)
	{
		physics_obj->StopCompletely(WERROR_NONE);

		starting_position = physics_obj->m_Position;
		sought_object_id = object_id;
		sought_object_radius = object_radius;
		sought_object_height = object_height;
		movement_type = MovementTypes::MoveToObject;
		top_level_object_id = top_level_id;

		movement_params.bitfield = params->bitfield;
		movement_params.distance_to_object = params->distance_to_object;
		movement_params.min_distance = params->min_distance;
		movement_params.desired_heading = params->desired_heading;
		movement_params.speed = params->speed;
		movement_params.fail_distance = params->fail_distance;
		movement_params.walk_run_threshhold = params->walk_run_threshhold;
		movement_params.context_id = params->context_id;
		movement_params.hold_key_to_apply = params->hold_key_to_apply;
		movement_params.action_stamp = params->action_stamp;

		initialized = 0;
		if (top_level_id != physics_obj->id)
		{
			physics_obj->set_target(0, top_level_object_id, 0.5, 0.0);
			return;
		}

		CleanUp();
	}

	if (physics_obj)
		physics_obj->StopCompletely(WERROR_NONE);
}

uint32_t MoveToManager::PerformMovement(MovementStruct &mvs)
{
	CancelMoveTo(WERROR_INTERRUPTED);
	physics_obj->unstick_from_object();

	switch (mvs.type)
	{
	case MovementTypes::MoveToObject:
		MoveToObject(mvs.object_id, mvs.top_level_id, mvs.radius, mvs.height, mvs.params);
		break;

	case MovementTypes::MoveToPosition:
		MoveToPosition(&mvs.pos, mvs.params);
		break;

	case MovementTypes::TurnToObject:
		TurnToObject(mvs.object_id, mvs.top_level_id, mvs.params);
		break;

	case MovementTypes::TurnToHeading:
		TurnToHeading(mvs.params);
		break;
	}

#if PHATSDK_IS_SERVER
	// send current movement type
	if (physics_obj)
	{
		physics_obj->Movement_UpdatePos();
		physics_obj->Animation_MoveToUpdate();
	}
#endif

	return 0;
}

void MoveToManager::MoveToPosition(Position *p, MovementParameters *params)
{
	if (physics_obj)
	{
		physics_obj->StopCompletely(WERROR_NONE);

		current_target_position = *p;
		sought_object_radius = 0;

		float distance = GetCurrentDistance();

		float headingDiff = physics_obj->m_Position.heading(*p) - physics_obj->get_heading();
		if (fabs(headingDiff) < F_EPSILON)
			headingDiff = 0.0;
		if (-F_EPSILON > headingDiff)
			headingDiff += 360.0;

		unsigned int command;
		int move_away;
		HoldKey hold_key;
		params->get_command(distance, headingDiff, &command, &hold_key, &move_away);

		if (command)
		{
			AddTurnToHeadingNode(physics_obj->m_Position.heading(*p));
			AddMoveToPositionNode();
		}

		if (params->use_final_heading)
			AddTurnToHeadingNode(params->desired_heading);

		sought_position = *p;
		starting_position = physics_obj->m_Position;

		movement_type = MovementTypes::MoveToPosition;
		movement_params.bitfield = params->bitfield;
		movement_params.distance_to_object = params->distance_to_object;
		movement_params.min_distance = params->min_distance;
		movement_params.desired_heading = params->desired_heading;
		movement_params.speed = params->speed;
		movement_params.fail_distance = params->fail_distance;
		movement_params.walk_run_threshhold = params->walk_run_threshhold;
		movement_params.context_id = params->context_id;
		movement_params.hold_key_to_apply = params->hold_key_to_apply;
		movement_params.action_stamp = params->action_stamp;
		movement_params.bitfield &= 0xFFFFFF7F;
		MoveToManager::BeginNextNode();
	}
}

void MoveToManager::AddTurnToHeadingNode(float global_heading)
{
	MovementNode node;
	node.type = MovementTypes::TurnToHeading;
	node.heading = global_heading;
	pending_actions.push_back(node);
}

void MoveToManager::AddMoveToPositionNode()
{
	MovementNode node;
	node.type = MovementTypes::MoveToPosition;
	pending_actions.push_back(node);
}

void MoveToManager::TurnToObject(uint32_t object_id, uint32_t top_level_id, MovementParameters *params)
{
	if (physics_obj)
	{
		if (params->stop_completely)
			physics_obj->StopCompletely(WERROR_NONE);

		movement_type = MovementTypes::TurnToObject;
		sought_object_id = object_id;

		current_target_position.frame.set_heading(params->desired_heading);

		top_level_object_id = top_level_id;
		movement_params.bitfield = params->bitfield;
		movement_params.distance_to_object = params->distance_to_object;
		movement_params.min_distance = params->min_distance;
		movement_params.desired_heading = params->desired_heading;
		movement_params.speed = params->speed;
		movement_params.fail_distance = params->fail_distance;
		movement_params.walk_run_threshhold = params->walk_run_threshhold;
		movement_params.context_id = params->context_id;
		movement_params.hold_key_to_apply = params->hold_key_to_apply;
		movement_params.action_stamp = params->action_stamp;

		if (top_level_id != physics_obj->id)
		{
			initialized = 0;
			physics_obj->set_target(0, top_level_id, 0.5, 0.0);
			return;
		}

		CleanUp();
	}
	else
	{
		movement_params.context_id = params->context_id;
	}

	if (physics_obj)
		physics_obj->StopCompletely(WERROR_NONE);
}

void MoveToManager::TurnToHeading(MovementParameters *pparams)
{
	if (physics_obj == NULL)
	{
		movement_params.context_id = pparams->context_id;

		// this must be inlined, can never happen..
		if (physics_obj)
			physics_obj->StopCompletely(WERROR_NONE);
	}
	else
	{
		if (pparams->stop_completely)
			physics_obj->StopCompletely(WERROR_NONE);

		movement_params = *pparams;
		movement_params.sticky = 0;

		sought_position.frame.set_heading(pparams->desired_heading);
		movement_type = MovementTypes::TurnToHeading;

		MovementNode node;
		node.type = MovementTypes::TurnToHeading;
		node.heading = pparams->desired_heading;
		pending_actions.push_back(node);
	}
}

void MoveToManager::HitGround()
{
	if (movement_type)
		BeginNextNode();
}

void MoveToManager::LeaveGround()
{
	// This function is really empty (null sub).
}

void MoveToManager::BeginNextNode()
{
	std::list<MovementNode>::iterator i = pending_actions.begin();

	if (i != pending_actions.end())
	{
		switch (i->type)
		{
		case MovementTypes::MoveToPosition:
			BeginMoveForward();
			break;

		case MovementTypes::TurnToHeading:
			BeginTurnToHeading();
			break;
		}
	}
	else
	{
		if (movement_params.sticky)
		{
			float old_object_radius = sought_object_radius;
			float old_object_height = sought_object_height;
			uint32_t old_object_ID = top_level_object_id;

			/*
			CleanUp();

			if (physics_obj) // obviously inlined
				physics_obj->StopCompletely(0);
				*/
			CleanUpAndCallWeenie(WERROR_NONE);
			
			physics_obj->get_position_manager()->StickTo(old_object_ID, old_object_radius, old_object_height);
		}
		else
		{
			/*
			CleanUp();

			if (physics_obj)
				physics_obj->StopCompletely(0);
				*/
			CleanUpAndCallWeenie(WERROR_NONE);
		}
	}
}

void MoveToManager::UseTime()
{
	if (!physics_obj)
		return;
	if (!(physics_obj->transient_state & CONTACT_TS))
		return;

	std::list<MovementNode>::iterator i = pending_actions.begin();

	if (i == pending_actions.end())
		return;

	if (!top_level_object_id || !movement_type || initialized)
	{
		switch (i->type)
		{
		case MovementTypes::MoveToPosition:
			HandleMoveToPosition();
			break;

		case MovementTypes::TurnToHeading:
			HandleTurnToHeading();
			break;
		}
	}
}

void MoveToManager::HandleMoveToPosition()
{	
	if (!physics_obj)
	{
		MoveToManager::CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
		return;
	}

	Position curr_pos = physics_obj->m_Position;

	MovementParameters params;
	params.speed = movement_params.speed;
	params.bitfield &= 0xFFFF7FFF;
	params.hold_key_to_apply = movement_params.hold_key_to_apply;

	if (physics_obj->motions_pending())
	{
		if (aux_command)
		{
			_StopMotion(aux_command, &params);
			aux_command = 0;
		}
	}
	else
	{
		float someHeading = movement_params.get_desired_heading(current_command, moving_away) + curr_pos.heading(current_target_position);
		
		if (someHeading >= 360.0)
			someHeading -= 360.0;

		float headingDiff = someHeading - physics_obj->get_heading();

		if (fabs(headingDiff) < F_EPSILON)
			headingDiff = 0.0;
		if (headingDiff < -F_EPSILON)
			headingDiff += 360.0;

		if (headingDiff <= 20.0 || headingDiff >= (360.0 - 20.0))
		{
			if (aux_command)
			{
				_StopMotion(aux_command, &params);
				aux_command = 0;
			}
		}
		else
		{
			uint32_t cmd = Motion_TurnRight;
			if (headingDiff >= 180.0)
				cmd = Motion_TurnLeft;

			if (cmd != aux_command)
			{
				_DoMotion(cmd, &params);
				aux_command = cmd;
			}
		}
	}

	float currDistance = GetCurrentDistance();

	if (!CheckProgressMade(currDistance))
	{
		if (!physics_obj->IsInterpolating() && !physics_obj->motions_pending())
			++fail_progress_count;
	}
	else
	{
		fail_progress_count = 0;
		if ((moving_away && currDistance >= movement_params.min_distance) || (!moving_away && currDistance <= movement_params.distance_to_object))
		{
			pending_actions.pop_front();

			MoveToManager::_StopMotion(current_command, &params);
			current_command = 0;

			if (aux_command)
			{
				_StopMotion(aux_command, &params);
				aux_command = 0;
			}

			BeginNextNode();
		}
		else
		{
			if (starting_position.distance(physics_obj->m_Position) > movement_params.fail_distance)
				CancelMoveTo(WERROR_TOO_FAR);
		}
	}

	if (top_level_object_id)
	{
		if (movement_type)
		{
			Vector vel = physics_obj->get_velocity();
			float velMag = vel.magnitude();

			if (velMag > 0.1)
			{
				float approx_time_to_get_there = currDistance / velMag;
				if (fabs(approx_time_to_get_there - physics_obj->get_target_quantum()) > 1.0)
					physics_obj->set_target_quantum(approx_time_to_get_there);
			}
		}
	}
}

int MoveToManager::CheckProgressMade(float curr_distance)
{
	double timeDelta = Timer::cur_time - previous_distance_time;

	if (timeDelta > 1.0)
	{
		float currDistanceToGo = moving_away ? curr_distance - previous_distance : previous_distance - curr_distance;

		if ((currDistanceToGo / timeDelta) < 0.25)
		{
			return FALSE;
		}

		previous_distance = curr_distance;
		previous_distance_time = Timer::cur_time;

		float origDistanceToGo = !moving_away ? original_distance - curr_distance : curr_distance - original_distance;
		if ((origDistanceToGo / (Timer::cur_time - original_distance_time)) < 0.25)
		{
			return FALSE;
		}
	}
	return TRUE;
}

float heading_diff(float heading1, float heading2, uint32_t motionid)
{
	double diff = heading1 - heading2;

	if (F_EPSILON > abs(diff))
		diff = 0.0f;

	if (diff < -F_EPSILON)
		diff += 360.0f;

	if (diff > F_EPSILON && motionid != Motion_TurnRight)
		diff = 360.0 - diff;

	return (float) diff;
}

BOOL heading_greater(float x, float y, uint32_t motionid)
{
	double diff = abs(x - y);

	float value1, value2;

	if (diff <= 180.0)
	{
		value1 = y;
		value2 = x;
	}
	else
	{
		value1 = x;
		value2 = y;
	}

	BOOL result = (value2 > value1) ? TRUE : FALSE;

	if (motionid != Motion_TurnRight)
		result = !result;

	return result;
}

void MoveToManager::BeginTurnToHeading()
{
	if (!pending_actions.size() || !physics_obj)
	{
		CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
		return;
	}

	if (physics_obj->motions_pending())
		return;

	std::list<MovementNode>::iterator i = pending_actions.begin();

	float diff = heading_diff(i->heading, physics_obj->get_heading(), Motion_TurnRight);
	uint32_t motionid;

	if (diff <= 180.0f)
	{
		if (diff > F_EPSILON)
		{
			motionid = Motion_TurnRight;
		}
		else
		{
			// Reached target?
			RemovePendingActionsHead();
			BeginNextNode();
			return;
		}
	}
	else
	{
		if ((diff + F_EPSILON) <= 360.0f)
		{
			motionid = Motion_TurnLeft;
		}
		else
		{
			// Reached target?
			RemovePendingActionsHead();
			BeginNextNode();
			return;
		}
	}

	MovementParameters params;

	params.cancel_moveto = 0;
	params.speed = movement_params.speed;
	params.hold_key_to_apply = movement_params.hold_key_to_apply;

	uint32_t err = _DoMotion(motionid, &params);

	if (err)
	{
		CancelMoveTo(err);
		return;
	}

	current_command = motionid;
	previous_heading = diff;
}

double MoveToManager::GetCurrentDistance()
{
	if (!physics_obj)
		return FLT_MAX;

	if (movement_params.bitfield & 0x400)
	{
		return Position::cylinder_distance(
			physics_obj->GetRadius(),
			physics_obj->GetHeight(),
			physics_obj->m_Position,
			sought_object_radius, 
			sought_object_height,
			current_target_position);
	}
	else
	{
		return physics_obj->m_Position.distance(current_target_position);
	}	
}

void MoveToManager::BeginMoveForward()
{
	if (!physics_obj)
	{
		CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
		return;
	}

	float curr_heading;
	float curr_distance;
	unsigned int motion; 
	HoldKey hold_key;
	int move_away;
	MovementParameters params;

	curr_distance = GetCurrentDistance();
	curr_heading = physics_obj->m_Position.heading(current_target_position) - physics_obj->get_heading();
	if (fabs(curr_heading) < F_EPSILON)
		curr_heading = 0.0;

	if (-F_EPSILON > curr_heading)
		curr_heading = curr_heading + 360.0;

	movement_params.get_command(curr_distance, curr_heading, &motion, &hold_key, &move_away);

	if (motion)
	{
		params = MovementParameters();
		params.hold_key_to_apply = hold_key;
		params.cancel_moveto = 0;
		params.speed = movement_params.speed;

		uint32_t motionResult = _DoMotion(motion, &params);
		if (motionResult)
		{
			CancelMoveTo(motionResult);
		}
		else
		{
			current_command = motion;
			moving_away = move_away;
			movement_params.hold_key_to_apply = hold_key;
			previous_distance = curr_distance;
			previous_distance_time = Timer::cur_time;
			original_distance = curr_distance;
			original_distance_time = Timer::cur_time;
		}
	}
	else
	{
		std::list<MovementNode>::iterator actionNode = pending_actions.begin();

		if (actionNode != pending_actions.end())
			pending_actions.erase(actionNode);

		BeginNextNode();
	}
}

uint32_t MoveToManager::_DoMotion(uint32_t motionid, MovementParameters *pparams)
{
	if (!physics_obj)
		return WERROR_NO_PHYSICS_OBJECT;

	if (!physics_obj->get_minterp())
		return WERROR_NO_MOTION_INTERPRETER;

	physics_obj->get_minterp()->adjust_motion(motionid, pparams->speed, pparams->hold_key_to_apply);
	return physics_obj->get_minterp()->DoInterpretedMotion(motionid, pparams);
}

void MoveToManager::RemovePendingActionsHead()
{
	pending_actions.pop_front();
}

void MoveToManager::HandleTurnToHeading()
{
	if (!physics_obj)
	{
		CancelMoveTo(WERROR_NO_PHYSICS_OBJECT);
		return;
	}

	if (current_command != Motion_TurnRight && current_command != Motion_TurnLeft)
	{
		BeginTurnToHeading();
		return;
	}

	std::list<MovementNode>::iterator i = pending_actions.begin();

	float curheading = physics_obj->get_heading();

	if (heading_greater(curheading, i->heading, current_command))
	{
		fail_progress_count = 0;
		physics_obj->set_heading(i->heading, 1);

		RemovePendingActionsHead();

		MovementParameters params;
		params.cancel_moveto = 0;
		params.hold_key_to_apply = movement_params.hold_key_to_apply;

		_StopMotion(current_command, &params);

		current_command = 0;
		BeginNextNode();
	}
	else
	{
		float diff = heading_diff(curheading, previous_heading, current_command);

		if (diff < 180 && diff > F_EPSILON)
		{
			fail_progress_count = 0;
			previous_heading = curheading;
		}
		else
		{
			previous_heading = curheading;

			if (!physics_obj->IsInterpolating() && !physics_obj->motions_pending())
				fail_progress_count++;
		}
	}
}