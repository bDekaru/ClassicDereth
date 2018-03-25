
#include "StdAfx.h"
#include "PhysicsObj.h"
#include "MovementManager.h"
#include "Physics.h"
#include "Setup.h"
#include "MotionTable.h"
#include "PartArray.h"
#include "Scripts.h"
#include "ObjCell.h"
#include "ChildList.h"
#include "Transition.h"
#include "LandDefs.h"
#include "EnvCell.h"
#include "LandCell.h"
#include "PartArray.h"
#include "PhysicsPart.h"
#include "Particles.h"
#include "ObjectMaint.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

#pragma warning(disable: 4150)

const double HUGE_QUANTUM = 2.0;
const double MIN_FRAMERATE = 5.0;
const double MAX_FRAMERATE = 30.0; // 30.0;
const double MIN_QUANTUM = 1.0 / MAX_FRAMERATE;
const double MAX_QUANTUM = 1.0 / MIN_FRAMERATE;

const float CPhysicsObj::DEFAULT_FRICTION = 0.95f;
const float CPhysicsObj::DEFAULT_TRANSLUCENCY = 0.0f;
const float CPhysicsObj::DEFAULT_ELASTICITY = 0.05f;
const float CPhysicsObj::DEFAULT_MASS = 1.0f;

double max_update_fps = 30.0; // 30.0;
double min_update_delay = 1.0 / max_update_fps;

const float small_velocity = 0.25f;
const float max_velocity = 50.0f;

#if !PHATSDK_IS_SERVER
class CObjectMaint *CPhysicsObj::obj_maint = NULL;
#else
CServerObjectMaint server_obj_maint;
class CServerObjectMaint *CPhysicsObj::obj_maint = &server_obj_maint; // TODO move this
#endif

CPhysicsObj::CPhysicsObj() : shadow_objects(4)
{
	hash_next = NULL;
	id = 0;
	// netblob_list = NULL;
	part_array = NULL;
	player_vector = Vector(0, 0, 1.0f);
	sound_table = NULL;
	m_bExaminationObject = false;
	script_manager = NULL;
	physics_script_table = 0;
	m_DefaultScript = 0;
	m_DefaultScriptIntensity = 0.0f;
	parent = NULL;
	children = NULL;
	player_distance = FLT_MAX;
	CYpt = FLT_MAX;
	cell = NULL;
	num_shadow_objects = 0;
	m_fElasticity = DEFAULT_ELASTICITY;
	transient_state = 0;
	m_fTranslucency = DEFAULT_TRANSLUCENCY;
	m_fFriction = DEFAULT_FRICTION;
	movement_manager = 0;
	position_manager = 0;
	last_move_was_autonomous = 0;
	jumped_this_frame = 0;
	m_PhysicsState = PhysicsState::EDGE_SLIDE_PS | PhysicsState::LIGHTING_ON_PS | PhysicsState::GRAVITY_PS | PhysicsState::REPORT_COLLISIONS_PS; //  0x400C08;
	translucencyOriginal = DEFAULT_TRANSLUCENCY;
	massinv = 1.0 / DEFAULT_MASS;
	m_velocityVector = Vector(0, 0, 0);
	m_Acceleration = Vector(0, 0, 0);
	m_Omega = Vector(0, 0, 0);
	hooks = NULL;
	attack_radius = 0.0f;
	detection_manager = NULL;
	attack_manager = NULL;
	target_manager = NULL;
	particle_manager = NULL;
	weenie_obj = 0;
	contact_plane_cell_id = 0;
	m_scale = 1.0f;
	sliding_normal = Vector(0, 0, 0);
	cached_velocity = Vector(0, 0, 0);
	collision_table = NULL;
	colliding_with_environment = 0;
	memset(update_times, 0, sizeof(update_times));

	// this part is custom
	_position_timestamp = 0; // 0x164
	_movement_timestamp = 0; // 0x166
	_state_timestamp = 0; // 0x168
	_vector_timestamp = 0; // 0x16A
	_teleport_timestamp = 0; // 0x16C
	_server_control_timestamp = 0; // 0x16E
	_force_position_timestamp = 0; // 0x170
	_objdesc_timestamp = 0; // 0x172
	_instance_timestamp = 1; // 0x174 -- setting to 1 even though this is 0
}

CPhysicsObj::~CPhysicsObj()
{
	Destroy();
}


void CPhysicsObj::Destroy()
{
	if (movement_manager)
	{
		delete movement_manager;
		movement_manager = NULL;
	}

	if (position_manager)
	{
		delete position_manager;
		position_manager = NULL;
	}

	if (particle_manager)
	{
		delete particle_manager;
		particle_manager = NULL;
	}

	if (script_manager)
	{
		delete script_manager;
		script_manager = NULL;
	}

	PhysicsObjHook *hook = hooks;
	while (hook)
	{
		PhysicsObjHook *next = hook->next;
		delete hook;
		hook = next;
	}
	hooks = NULL;

	if ((m_PhysicsState & STATIC_PS) && (m_PhysicsState & 0xC0000))
		CPhysics::RemoveStaticAnimatingObject(this);

	if (physics_script_table)
	{
		PhysicsScriptTable::Release(physics_script_table);
		physics_script_table = NULL;
	}

	// netblobs deleted here

	if (sound_table)
	{
		UNFINISHED_LEGACY("CPhysicsObj::Destroy - CSoundTable::Release(m_SoundTable)");
		// CSoundTable::Release(m_SoundTable);
		sound_table = NULL;
	}

	m_Position.objcell_id = 0;

	if (part_array)
	{
		delete part_array;
		part_array = NULL;
	}

	memset(update_times, 0, sizeof(update_times));
	weenie_obj = NULL;

	if (collision_table)
	{
		delete collision_table;
		collision_table = NULL;
	}

	if (detection_manager)
	{
		delete detection_manager;
		detection_manager = NULL;
	}

	if (attack_manager)
	{
		delete attack_manager;
		attack_manager = NULL;
	}

	if (target_manager)
	{
		delete target_manager;
		target_manager = NULL;
	}

	if (children)
	{
		delete children;
		children = NULL;
	}

	transient_state = 0;
	m_PhysicsState = 0x400C08;
}

void CPhysicsObj::MakeMovementManager(BOOL init_motion)
{
	if (!movement_manager)
	{
		movement_manager = MovementManager::Create(this, weenie_obj);

		if (init_motion)
		{
			movement_manager->EnterDefaultState();
		}

		set_active(TRUE);
	}
}

BOOL CPhysicsObj::set_active(BOOL active)
{
	if (active)
	{
		if (m_PhysicsState & STATIC_PS)
			return FALSE;

		if (!(transient_state & ACTIVE_TS))
			update_time = Timer::cur_time;

		transient_state |= ((DWORD)ACTIVE_TS);
	}
	else
	{
		transient_state &= ~((DWORD)ACTIVE_TS);
	}

	return TRUE;
}

void CPhysicsObj::clear_target()
{
	if (target_manager)
		target_manager->ClearTarget();
}

void CPhysicsObj::unstick_from_object()
{
	if (position_manager)
		position_manager->UnStick();
}

void CPhysicsObj::stick_to_object(DWORD target)
{
	MakePositionManager();

	if (obj_maint)
	{
		CPhysicsObj *targetObj = obj_maint->GetObject(target);
		if (targetObj)
		{
			if (targetObj->parent)
				targetObj = targetObj->parent;
			
			position_manager->StickTo(targetObj->id, targetObj->GetRadius(), targetObj->GetHeight());
		}
	}
}

void CPhysicsObj::cancel_moveto()
{
	if (movement_manager)
		movement_manager->CancelMoveTo(0x36);
}

void CPhysicsObj::RemoveLinkAnimations()
{
	if (part_array)
		part_array->HandleEnterWorld();
}

float CPhysicsObj::get_heading()
{
	return m_Position.frame.get_heading();
}

void CPhysicsObj::UpdateChild(CPhysicsObj *child_obj, unsigned int part_index, Frame *child_frame)
{
	Frame new_frame;

	if (part_index >= part_array->num_parts)
		new_frame.combine(&m_Position.frame, child_frame);
	else
		new_frame.combine(&part_array->parts[part_index]->pos.frame, child_frame);

	child_obj->set_frame(new_frame);

#if PHATSDK_RENDER_AVAILABLE
	if (child_obj->particle_manager)
		child_obj->particle_manager->UpdateParticles();
#endif

	if (child_obj->script_manager)
		child_obj->script_manager->UpdateScripts();
}

void CPhysicsObj::UpdateChildrenInternal()
{
	if (part_array && children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
			UpdateChild(children->objects.array_data[i], children->part_numbers.array_data[i], &children->frames.array_data[i]);
	}
}

void CPhysicsObj::set_frame(Frame &frame)
{
	Frame i_frame = frame;

	i_frame = frame;
	if (!i_frame.IsValid())
	{
		if (i_frame.IsValidExceptForHeading())
		{
			i_frame.m_angles = Quaternion(0, 0, 0, 0);
		}
	}

	m_Position.frame = i_frame;

	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
			part_array->SetFrame(&m_Position.frame);
	}

	UpdateChildrenInternal();
}

void CPhysicsObj::set_heading(float degrees, int send_event)
{
	Frame newFrame = m_Position.frame;
	newFrame.set_heading(degrees);
	set_frame(newFrame);
}

BOOL CPhysicsObj::IsInterpolating()
{
	if (position_manager)
	{
		return position_manager->IsInterpolating();
	}

	return FALSE;
}

BOOL CPhysicsObj::motions_pending()
{
	return movement_manager && movement_manager->motions_pending();
}

DWORD CPhysicsObj::StopInterpretedMotion(DWORD motion, class MovementParameters *params)
{
	if (part_array)
		return part_array->StopInterpretedMotion(motion, params);

	return 0x47;
}

DWORD CPhysicsObj::DoInterpretedMotion(DWORD motion, class MovementParameters *params)
{
	if (part_array)
		return part_array->DoInterpretedMotion(motion, params);

	return 0x47;
}

BOOL CPhysicsObj::movement_is_autonomous()
{
	return last_move_was_autonomous;
}

class MovementManager *CPhysicsObj::get_movement_manager(BOOL make) // custom
{
	if (make)
	{
		MakeMovementManager(TRUE);
	}

	return movement_manager;
}

class CMotionInterp *CPhysicsObj::get_minterp()
{
	MakeMovementManager(TRUE);

	return movement_manager->get_minterp();
}

void CPhysicsObj::InitializeMotionTables()
{
	if (part_array)
		return part_array->InitializeMotionTables();
}

void CPhysicsObj::StopCompletely(int send_event)
{
	if (movement_manager)
	{
		MovementStruct ms;
		ms.type = MovementTypes::StopCompletely;
		movement_manager->PerformMovement(ms);
	}
}

void CPhysicsObj::set_local_velocity(const Vector &new_velocity, int send_event)
{
	set_velocity(m_Position.localtoglobalvec(new_velocity), send_event);
}

void CPhysicsObj::set_on_walkable(BOOL is_on_walkable)
{
	DWORD oldState = transient_state & ON_WALKABLE_TS;

	if (is_on_walkable)
		transient_state |= ON_WALKABLE_TS;
	else
		transient_state &= ~((DWORD)ON_WALKABLE_TS);

	if (oldState)
	{
		if (!is_on_walkable)
		{
			if (movement_manager)
				movement_manager->LeaveGround();
		}
	}
	else
	{
		if (is_on_walkable)
		{
			if (movement_manager)
			{
				movement_manager->HitGround();

				// CUSTOM - for fall damage
				if (weenie_obj)
				{
					weenie_obj->HitGround(cached_velocity.z);
				}
				// 
			}
		}
	}

	calc_acceleration();
}

void CPhysicsObj::StopCompletely_Internal()
{
	if (part_array)
		part_array->StopCompletely_Internal();
}

void CPhysicsObj::CheckForCompletedMotions()
{
	if (part_array)
		return part_array->CheckForCompletedMotions();
}

BOOL CPhysicsObj::IsFullyConstrained()
{
	if (position_manager)
		return position_manager->IsFullyConstrained();

	return FALSE;
}

void CPhysicsObj::leave_world()
{
	report_collision_end(TRUE);

	if (obj_maint)
	{
		obj_maint->RemoveFromLostCell(this);
		obj_maint->RemoveObjectToBeDestroyed(id);
	}

	transient_state &= ~((DWORD)TransientState::ACTIVE_TS);
	remove_shadows_from_cells();
	leave_cell(FALSE);
	set_cell_id(0);

	transient_state &= ~((DWORD)TransientState::CONTACT_TS);
	calc_acceleration();

	transient_state &= ~((DWORD)TransientState::WATER_CONTACT_TS);

	BOOL bIsOnWalkable = transient_state & TransientState::ON_WALKABLE_TS;
	transient_state &= ~((DWORD)TransientState::ON_WALKABLE_TS);

	if (bIsOnWalkable && movement_manager)
	{
		movement_manager->LeaveGround();
	}

	calc_acceleration();

	transient_state &= ~((DWORD)0x1F4);
}

void CPhysicsObj::calc_friction(float quantum, float velocity_mag2)
{
	if (transient_state & ON_WALKABLE_TS)
	{
		double the_friction;

		if (m_PhysicsState & SLEDDING_PS)
		{
			float dp = m_velocityVector.dot_product(contact_plane.m_normal);
			if (dp >= 0.25)
				return;

			double frict = 0.2;
			m_velocityVector -= contact_plane.m_normal * dp;

			if (velocity_mag2 >= 1.5625)
			{
				if (velocity_mag2 < 6.25 || cos(0.1745329251994329) <= contact_plane.m_normal.z)
					frict = m_fFriction;

				the_friction = frict;
			}
			else
			{
				the_friction = 1.0;
			}
		}
		else
		{
			float dp = m_velocityVector.dot_product(contact_plane.m_normal);
			if (dp >= 0.25)
				return;

			m_velocityVector -= contact_plane.m_normal * dp;
			the_friction = m_fFriction;
		}

		float someVal = pow(1.0 - the_friction, quantum);
		m_velocityVector *= someVal;
	}
}

void CPhysicsObj::UpdatePhysicsInternal(float quantum, Frame &offset_frame)
{
	float velocity_mag2;
	Vector w;

	velocity_mag2 = m_velocityVector.dot_product(m_velocityVector);
	if (velocity_mag2 <= 0.0)
	{
		if (!movement_manager)
		{
			if (transient_state & ON_WALKABLE_TS)
				transient_state &= ~(ACTIVE_TS);
		}
	}
	else
	{
		float maxVelMag = max_velocity * max_velocity;
		if (velocity_mag2 > maxVelMag)
		{
			Vector normalizedVec = m_velocityVector.normalize();
			velocity_mag2 = maxVelMag;
			m_velocityVector *= max_velocity;
		}

		calc_friction(quantum, velocity_mag2);

		if ((velocity_mag2 - (small_velocity * small_velocity)) < F_EPSILON)
		{
			m_velocityVector = Vector(0, 0, 0);
		}

		Vector change = ((m_Acceleration * 0.5) * quantum * quantum) + (m_velocityVector * quantum);
		offset_frame.m_origin += change;
	}

	m_velocityVector += (m_Acceleration * quantum);
	w = m_Omega * quantum;
	offset_frame.grotate(w);
}

void CPhysicsObj::UpdatePositionInternal(float quantum, Frame &o_newFrame)
{
	// !! modifies o_newFrame

	Frame offset_frame;

	if (!(m_PhysicsState & HIDDEN_PS))
	{
		if (part_array)
			part_array->Update(quantum, &offset_frame);

		if (transient_state & ON_WALKABLE_TS)
			offset_frame.m_origin *= m_scale;
		else
			offset_frame.m_origin *= 0.0f;
	}

	if (position_manager)
	{
		position_manager->adjust_offset(&offset_frame, quantum);
	}

	o_newFrame.combine(&m_Position.frame, &offset_frame);

	if (!(m_PhysicsState & HIDDEN_PS))
		UpdatePhysicsInternal(quantum, o_newFrame);

#if !PHATSDK_IS_SERVER
	process_hooks();
#else
	WORD tts = _teleport_timestamp;
	process_hooks();

	if (tts != _teleport_timestamp)
	{
		o_newFrame = m_Position.frame;
	}
#endif
}

int CPhysicsObj::ethereal_check_for_collisions()
{
	for (DWORD i = 0; i < num_shadow_objects; i++)
	{
		CObjCell *pcell = shadow_objects.data[i].cell;
		if (pcell)
		{
			if (pcell->check_collisions(this))
				return 1;
		}
	}
	return 0;
}

int CPhysicsObj::set_ethereal(int ethereal, int send_event)
{
	if (ethereal)
	{
		m_PhysicsState |= ETHEREAL_PS;
		transient_state &= ~(CHECK_ETHEREAL_TS);
		return 1;
	}

	m_PhysicsState &= ~(ETHEREAL_PS);
	if (parent || !cell || !ethereal_check_for_collisions())
	{
		transient_state &= ~(CHECK_ETHEREAL_TS);
		return 1;
	}

	m_PhysicsState |= ETHEREAL_PS;
	transient_state |= CHECK_ETHEREAL_TS;
	return 0;
}

void CPhysicsObj::UpdateObjectInternal(float quantum)
{
	if (transient_state & ACTIVE_TS)
	{
		if (!cell)
			return;

		if (transient_state & CHECK_ETHEREAL_TS)
		{
			set_ethereal(0, 0);
		}

		jumped_this_frame = 0;

		Position new_pos;
		new_pos.objcell_id = m_Position.objcell_id;
		UpdatePositionInternal(quantum, new_pos.frame);

		if (GetNumSphere())
		{
			if (new_pos.frame.m_origin.is_equal(m_Position.frame.m_origin))
			{
				new_pos.frame.m_origin = m_Position.frame.m_origin;

				set_frame(new_pos.frame); // set_initial_frame(&new_pos.frame);
				// diff = Vector(0, 0, 0);
				cached_velocity = Vector(0, 0, 0);
			}
			else
			{
				if (m_PhysicsState & ALIGNPATH_PS)
				{
					Vector offset = new_pos.frame.m_origin - m_Position.frame.m_origin;
					Vector normal = offset.normalize();

					new_pos.frame.set_vector_heading(normal);
				}
				else if (m_PhysicsState & SLEDDING_PS)
				{
					if (!m_velocityVector.is_zero())
					{
						float degrees = m_velocityVector.get_heading();
						new_pos.frame.set_heading(degrees);
					}
				}

				CTransition *transit = transition(&m_Position, &new_pos, 0);
				if (transit && transit->sphere_path.curr_cell) // CUSTOM PART: transit->sphere_path.curr_cell
				{
					/*
					if (cell && !transit->sphere_path.curr_cell && weenie_obj && weenie_obj->_IsPlayer())
					{
						weenie_obj->SendText("FML", LTT_DEFAULT);
						// DebugBreak();
					}
					*/

					Vector diff = m_Position.get_offset(transit->sphere_path.curr_pos);
					cached_velocity = diff / quantum;
					SetPositionInternal(transit);
				}
				else
				{
					/*
					if (transit && !transit->sphere_path.curr_cell)
					{
						g_pWorld->BroadcastGlobal(ServerText(csprintf("Prevented player %s from being portaled to Holt.", weenie_obj->GetName().c_str()), LTT_DEFAULT), PRIVATE_MSG);
						weenie_obj->SendText(csprintf("Prevented player %s", LTT_DEFAULT);
					}
					*/

					new_pos.frame.m_origin = m_Position.frame.m_origin;
					set_initial_frame(&new_pos.frame);
					cached_velocity = Vector(0, 0, 0);
				}
			}
		}
		else
		{
			if (!movement_manager && (transient_state & ON_WALKABLE_TS))
				transient_state &= ~((DWORD)ACTIVE_TS);

			new_pos.frame.m_origin = m_Position.frame.m_origin;
			set_frame(new_pos.frame);
			cached_velocity = Vector(0, 0, 0);
		}

		if (detection_manager)
		{
			UNFINISHED_LEGACY("detection_manager->CheckDetection();");
		}

		if (target_manager)
			target_manager->HandleTargetting();

		if (movement_manager)
			movement_manager->UseTime();

		if (part_array)
			part_array->HandleMovement();

		if (position_manager)
			position_manager->UseTime();
	}

#if PHATSDK_RENDER_AVAILABLE
	if (particle_manager)
		particle_manager->UpdateParticles();
#endif

	if (script_manager)
		script_manager->UpdateScripts();
}

void CPhysicsObj::update_object()
{
	if (parent || !cell || m_PhysicsState & FROZEN_PS)
	{
		transient_state &= ~((DWORD)ACTIVE_TS);
	}
	else
	{
		/*
		if (CPhysicsObj::player_object)
		{
			v2 = Position::get_offset(&CPhysicsObj::player_object->m_position, &quantum, &this->m_position);
			v3 = (int)&v1->player_vector;
			*(float *)v3 = v2->x;
			*(float *)(v3 + 4) = v2->y;
			*(float *)(v3 + 8) = v2->z;
			v4 = sqrt(v1->player_vector.x * v1->player_vector.x + v1->player_vector.y * v1->player_vector.y + v1->player_vector.z * v1->player_vector.z);
			v1->player_distance = *(float *)&v4;
			if (v4 > 96.0 && CPhysicsObj::obj_maint->is_active)
				v1->transient_state &= 0xFFFFFF7F;
			else
				CPhysicsObj::set_active(v1, 1);
		}
		*/

#if the_real_code
		PhysicsTimer::curr_time = update_time;
		double timeElapsed = Timer::cur_time - update_time;
		if (timeElapsed < F_EPSILON)
		{
			// update_time = Timer::cur_time;
			return;
		}
		else
		{
			if (timeElapsed <= HUGE_QUANTUM)
			{
				while (timeElapsed > MAX_QUANTUM)
				{
					PhysicsTimer::curr_time += MAX_QUANTUM;
					UpdateObjectInternal(MAX_QUANTUM);
					timeElapsed -= MAX_QUANTUM;
				}

				if (timeElapsed > MIN_QUANTUM)
				{
					PhysicsTimer::curr_time += timeElapsed;
					UpdateObjectInternal(timeElapsed);
				}

				//update_time = Timer::cur_time;
			}
			else
			{
				update_time = Timer::cur_time;
				return;
			}
		}
#else

		PhysicsTimer::curr_time = update_time;
		double timeElapsed = Timer::cur_time - update_time;
		if (timeElapsed <= MIN_QUANTUM)
			return;

		if (timeElapsed > HUGE_QUANTUM)
		{
			update_time = Timer::cur_time;
			return;
		}

		while (timeElapsed > MAX_QUANTUM)
		{
			PhysicsTimer::curr_time += MAX_QUANTUM;
			UpdateObjectInternal(MAX_QUANTUM);
			timeElapsed -= MAX_QUANTUM;
		}

		if (timeElapsed > MIN_QUANTUM)
		{
			PhysicsTimer::curr_time += timeElapsed;
			UpdateObjectInternal(timeElapsed);
		}

		update_time = Timer::cur_time;
#endif
	}
}

void CPhysicsObj::calc_cross_cells_static()
{
	static CELLARRAY cell_array;

	cell_array.num_cells = 0;
	cell_array.added_outside = 0;
	cell_array.do_not_load_cells = 1;

	if (!(m_PhysicsState & HAS_PHYSICS_BSP_PS) && GetNumCylsphere())
	{
		CObjCell::find_cell_list(&m_Position, GetNumCylsphere(), GetCylsphere(), &cell_array, NULL);
	}
	else
	{
		find_bbox_cell_list(&cell_array);
	}

	remove_shadows_from_cells();
	add_shadows_to_cells(&cell_array);
}

void CPhysicsObj::add_obj_to_cell(CObjCell *pCell, Frame *pFrame)
{
	enter_cell(pCell);
	set_initial_frame(pFrame);
	calc_cross_cells_static();
}

void CPhysicsObj::MotionDone(DWORD motion, BOOL success)
{
	if (movement_manager)
		movement_manager->MotionDone(motion, success);

	/*
#if PHATSDK_IS_SERVER
	if (weenie_obj)
		weenie_obj->OnMotionDone(motion, success);
#endif
*/
}

CPhysicsObj *CPhysicsObj::makeObject(DWORD data_did, DWORD object_iid, BOOL bDynamic)
{
	CPhysicsObj *pObject = new CPhysicsObj();

	if (!pObject)
		return NULL;

	if (!pObject->InitObjectBegin(object_iid, bDynamic))
		goto bad_object;

	
	BOOL bCreateAtAll = FALSE;
	BOOL bCreateParts = FALSE;

#if PHATSDK_IS_SERVER
	DWORD dataType = data_did & 0xFF000000;

	if (dataType == 0x01000000)
	{
		CGfxObj *object = CGfxObj::Get(data_did);

		if (object)
		{
			bCreateAtAll = TRUE;

			if (object->physics_bsp)
			{
				bCreateParts = TRUE;
			}

			CGfxObj::Release(object);
		}
	}
	else if (dataType == 0x02000000)
	{
		CSetup *object = CSetup::Get(data_did);

		if (object)
		{
			bCreateAtAll = TRUE;

			if (object->has_physics_bsp)
			{
				bCreateParts = TRUE;
			}
			else if (object->num_parts >= 1 && object->parts && object->parts[0])
			{
				CGfxObj *gfxobject = CGfxObj::Get(object->parts[0]);

				if (gfxobject)
				{
					if (gfxobject->physics_bsp)
					{
						bCreateParts = TRUE;
					}

					CGfxObj::Release(gfxobject);
				}
			}

			CSetup::Release(object);
		}
	}
#else
	bCreateParts = TRUE;
#endif

	if (!pObject->InitPartArrayObject(data_did, bCreateParts))
		goto bad_object;

	pObject->InitObjectEnd(); //  SetPlacementFrameInternal(0x65);

	return pObject;

bad_object:

	delete pObject;
	return NULL;
}

void CPhysicsObj::add_anim_hook(CAnimHook *hook)
{
	anim_hooks.add(&hook);
}

void CPhysicsObj::InitDefaults(CSetup *pSetup)
{
	if (pSetup->default_script_id)
	{
		// DEBUGOUT("Playing default script %08X\r\n", pSetup->m_DefaultScript);
		play_script_internal(pSetup->default_script_id);
	}

	if (pSetup->default_mtable_id)
		SetMotionTableID(pSetup->default_mtable_id);

	if (pSetup->default_stable_id)
		set_stable_id(pSetup->default_stable_id);

	if (pSetup->default_phstable_id)
		set_phstable_id(pSetup->default_phstable_id);

	if (m_PhysicsState & STATIC_PS)
	{
		if (pSetup->default_anim_id)
			m_PhysicsState |= HAS_DEFAULT_ANIM_PS;

		if (pSetup->default_script_id)
			m_PhysicsState |= HAS_DEFAULT_SCRIPT_PS;

		if (m_PhysicsState & (HAS_DEFAULT_ANIM_PS| HAS_DEFAULT_SCRIPT_PS))
			CPhysics::AddStaticAnimatingObject(this);
	}
}

int CPhysicsObj::report_object_collision_end(const unsigned int object_id)
{
	CPhysicsObj *collidedObject; // v3

	if (CPhysicsObj::obj_maint && (collidedObject = CPhysicsObj::obj_maint->GetObjectA(object_id)) != 0)
	{
		if (!(collidedObject->m_PhysicsState & REPORT_COLLISIONS_AS_ENVIRONMENT_PS))
		{
			if (m_PhysicsState & REPORT_COLLISIONS_PS)
			{
				if (weenie_obj)
					weenie_obj->DoCollisionEnd(object_id);
			}

			if (collidedObject->m_PhysicsState & REPORT_COLLISIONS_PS)
			{
				if (collidedObject->weenie_obj)
					collidedObject->weenie_obj->DoCollisionEnd(id);
			}
		}

		return TRUE;
	}

	if (m_PhysicsState & REPORT_COLLISIONS_PS)
	{
		if (weenie_obj)
			weenie_obj->DoCollisionEnd(object_id);
	}

	return FALSE;
}

void CPhysicsObj::report_collision_end(const int force_end)
{
	if (collision_table)
	{
		// this code isn't perfect but should be same behavior
		LongNIValHashIter<CPhysicsObj::CollisionRecord> iter(collision_table);
		SmartArray<DWORD> end_array(10);

		while (!iter.EndReached() && iter.GetCurrent())
		{
			auto current = iter.GetCurrent();

			DWORD currentID = current->id;
			CPhysicsObj::CollisionRecord entry = current->m_Data;

			if (((PhysicsTimer::curr_time - entry.touched_time) > 1.0)
				|| (entry.ethereal && (PhysicsTimer::curr_time - entry.touched_time) > 0.0)
				|| (force_end))
			{
				iter.DeleteCurrent();
				end_array.add(&currentID);
			}
			else
			{
				iter.Next();
			}
		}

		for (DWORD i = 0; i < end_array.num_used; i++)
		{
			report_object_collision_end(end_array.array_data[i]);
		}
	}
}

void CPhysicsObj::calc_acceleration()
{
	if ((transient_state & 1) && (transient_state & 2) && !(m_PhysicsState & SLEDDING_PS))
	{
		m_Acceleration = Vector(0, 0, 0);
		m_Omega = Vector(0, 0, 0);
	}
	else if (m_PhysicsState & GRAVITY_PS)
	{
		m_Acceleration = Vector(0, 0, (float)PhysicsGlobals::gravity);
	}
	else
	{
		m_Acceleration = Vector(0, 0, 0);
	}
}

void CPhysicsObj::set_cell_id(DWORD CellID)
{
	m_Position.objcell_id = CellID;

	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
			part_array->SetCellID(CellID);
	}
}

void CPhysicsObj::leave_cell(BOOL is_changing_cell)
{
	if (!cell)
		return;

	cell->remove_object(this);

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
			children->objects.array_data[i]->leave_cell(is_changing_cell);
	}

	if (part_array)
		part_array->RemoveLightsFromCell(cell);

	cell = NULL;
}


void CPhysicsObj::enter_cell(CObjCell *pCell)
{
	if (!part_array)
		return;

	pCell->add_object(this);

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
			children->objects.array_data[i]->enter_cell(pCell);
	}

	m_Position.objcell_id = pCell->id;
	set_cell_id(pCell->id);

	cell = pCell;

	if (part_array)
		part_array->AddLightsToCell(pCell);
}

void CPhysicsObj::set_initial_frame(Frame *Pos)
{
	m_Position.frame = *Pos;

	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
			part_array->SetFrame(&m_Position.frame);
	}

	UpdateChildrenInternal();
}


BOOL CPhysicsObj::InitObjectBegin(DWORD object_iid, BOOL bDynamic)
{
	id = object_iid;

	if (bDynamic)
		m_PhysicsState &= ~STATIC_PS;
	else
		m_PhysicsState |= STATIC_PS;

	transient_state &= ~((DWORD)ACTIVE_TS);
	update_time = Timer::cur_time;

	return TRUE;
}

BOOL CPhysicsObj::InitPartArrayObject(DWORD data_did, BOOL bCreateParts)
{
	if (!data_did)
		return FALSE;

	DWORD dataType = data_did & 0xFF000000;
	BOOL MakeMeTranslucent = FALSE;

	if (dataType == 0x01000000)
	{
		part_array = CPartArray::CreateMesh(this, data_did);

		if (!part_array)
			return FALSE;
	}
	else if (dataType == 0x02000000)
	{
		part_array = CPartArray::CreateSetup(this, data_did, bCreateParts);

		if (!part_array)
			return FALSE;
	}
	else
	{
		if (dataType)
			return FALSE;

		MakeMeTranslucent = TRUE;

		if (!makeAnimObject(0x02000000 | data_did, bCreateParts))
			return FALSE;
	}

	CacheHasPhysicsBSP();

	if (MakeMeTranslucent)
	{
		m_PhysicsState |= ETHEREAL_PS;
		transient_state &= ~(0x00000100UL);

		SetTranslucencyInternal(0.25f);
		m_PhysicsState |= IGNORE_COLLISIONS_PS;
	}

	return TRUE;
}

BOOL CPhysicsObj::CacheHasPhysicsBSP()
{
	if (part_array)
	{
		if (part_array->CacheHasPhysicsBSP())
		{
			m_PhysicsState |= HAS_PHYSICS_BSP_PS;
			return TRUE;
		}
	}

	m_PhysicsState &= ~HAS_PHYSICS_BSP_PS;
	return FALSE;
}

void CPhysicsObj::SetTranslucencyInternal(float Amount)
{
	if (Amount < translucencyOriginal)
		Amount = translucencyOriginal;

	m_fTranslucency = Amount;

	if (part_array)
		part_array->SetTranslucencyInternal(Amount);
}

BOOL CPhysicsObj::InitObjectEnd()
{
	SetPlacementFrameInternal(0x65);
	return TRUE;
}

BOOL CPhysicsObj::SetPlacementFrameInternal(DWORD frame_id)
{
	BOOL result = FALSE;

	if (part_array)
	{
		result = part_array->SetPlacementFrame(frame_id);
	}

	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
		{
			part_array->SetFrame(&m_Position.frame);
		}
	}

	return result;
}


BOOL CPhysicsObj::SetPlacementFrame(DWORD frame_id, BOOL send_event)
{
#if PHATSDK_IS_SERVER
	if (weenie_obj) // custom
	{
		weenie_obj->m_Qualities.SetInt(PLACEMENT_POSITION_INT, frame_id);
	}
#endif

	return SetPlacementFrameInternal(frame_id);
}

BOOL CPhysicsObj::play_script_internal(DWORD ScriptID)
{
#if PHATSDK_RENDER_AVAILABLE
	if (!ScriptID)
		return FALSE;

	if (!script_manager)
	{
		script_manager = new ScriptManager(this);
	}

	if (script_manager)
		return script_manager->AddScript(ScriptID);
	else
		return FALSE;
#else
	return FALSE;
#endif
}

DWORD CPhysicsObj::SetMotionTableID(DWORD ID)
{
	// DEBUGOUT("Omitted motion table code here\r\n");

	DWORD unknown = 0;

	if (part_array)
	{
		unknown = part_array->SetMotionTableID(ID);

		if (unknown)
		{
			if (movement_manager)
			{
				delete movement_manager;
				movement_manager = NULL;
			}

			if (ID)
				MakeMovementManager(1);
		}
	}

	return unknown;
}

void CPhysicsObj::set_stable_id(DWORD ID)
{
	// DEBUGOUT("Omitted sound table code here\r\n");
	// CSoundTable::Release(m_PhysicsScriptTable);
	// m_SoundTable = CSoundTable::Get(ID);
}

void CPhysicsObj::set_phstable_id(DWORD ID)
{
	PhysicsScriptTable::Release(physics_script_table);
	physics_script_table = PhysicsScriptTable::Get(ID);
}

BOOL CPhysicsObj::makeAnimObject(DWORD setup_id, BOOL bCreateParts)
{
	part_array = CPartArray::CreateSetup(this, setup_id, bCreateParts);

	return part_array ? TRUE : FALSE;
}

void CPhysicsObj::Hook_AnimDone()
{
	if (part_array)
		part_array->AnimationDone(1);
}


void CPhysicsObj::set_nodraw(BOOL NoDraw, BOOL Unused)
{
	if (NoDraw)
	{
		m_PhysicsState |= NODRAW_PS;

		if (part_array)
			part_array->SetNoDrawInternal(1);
	}
	else
	{
		m_PhysicsState &= ~(NODRAW_PS);

		if (part_array)
			part_array->SetNoDrawInternal(0);
	}
}

BOOL CPhysicsObj::is_completely_visible()
{
	if (!cell)
		return FALSE;
	if (!num_shadow_objects)
		return FALSE;

	for (DWORD i = 0; i < num_shadow_objects; i++)
	{
		if (!shadow_objects.data[i].cell)
			return FALSE;
	}

	return TRUE;
}


void CPhysicsObj::animate_static_object()
{
	if (!cell)
		return;

	double CurrentTime = Timer::cur_time; // Timer__m_timeCurrent

												 // Update physics timer.
	PhysicsTimer::curr_time = CurrentTime;

	// Time between last update.
	double FrameTime = CurrentTime - update_time;

	/*
	if (FrameTime < F_EPSILON)
	{
		// update_time = CurrentTime; seems bad
	}
	else if (FrameTime < min_update_delay)
	{
		// Wait to update.. not quite there yet
	}
	else if (FrameTime > HUGE_QUANTUM)
	{
		update_time = CurrentTime;
	}
	else
	*/
	{
		if (part_array)
		{
			if (m_PhysicsState & HAS_DEFAULT_ANIM_PS)
			{
				part_array->Update((float)FrameTime, NULL);
				m_Position.frame.grotate(m_Omega);
				UpdatePartsInternal();
				UpdateChildrenInternal();
			}

			if (m_PhysicsState & HAS_DEFAULT_SCRIPT_PS)
			{
				if (script_manager)
					script_manager->UpdateScripts();
			}

#if PHATSDK_RENDER_AVAILABLE
			if (particle_manager)
				particle_manager->UpdateParticles();
#endif

			process_hooks();
		}

		update_time = CurrentTime;
	}
}

void CPhysicsObj::UpdatePartsInternal()
{
	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
			part_array->SetFrame(&m_Position.frame);
	}
}

void CPhysicsObj::process_hooks()
{
	// UNFINISHED hooks
	
	/*
	v1 = this;
	v2 = this->hooks;
	if (v2)
	{
		do
		{
			v3 = v2->next;
			if (v2->vfptr->Execute(v2, v1))
			{
				v4 = v2->next;
				if (v4)
					v4->prev = v2->prev;
				v5 = v2->prev;
				if (v5)
					v5->next = v2->next;
				else
					v1->hooks = v2->next;
				v2->prev = 0;
				v2->next = 0;
				v2->vfptr = (PhysicsObjHookVtbl *)&PhysicsObjHook::`vftable';
					operator delete(v2);
			}
			v2 = v3;
		} while (v3);
	}
	*/

	for (DWORD i = 0; i < anim_hooks.num_used; i++)
	{
		CAnimHook *pHook = anim_hooks.array_data[i];
		pHook->Execute(this);
	}
	
	// shrink.... missing
	anim_hooks.num_used = 0;
}

DWORD CPhysicsObj::GetNumSphere() // inlined
{
	if (part_array)
		return part_array->GetNumSphere();

	return NULL;
}

CSphere *CPhysicsObj::GetSphere() // inlined
{
	if (part_array)
		return part_array->GetSphere();

	return NULL;
}

DWORD CPhysicsObj::GetNumCylsphere() // inlined
{
	if (part_array)
		return part_array->GetNumCylsphere();

	return NULL;
}

CCylSphere *CPhysicsObj::GetCylsphere() // inlined
{
	if (part_array)
		return part_array->GetCylsphere();

	return NULL;
}


int CPhysicsObj::check_collision(CPhysicsObj *object)
{
	if (m_PhysicsState & STATIC_PS)
		return FALSE;

	CTransition *transit = CTransition::makeTransition();

	if (!transit)
		return FALSE;

	get_object_info(transit, 0);
	transit->init_object(this, get_object_info(transit, 0));

	if (GetNumSphere())
	{
		transit->init_sphere(GetNumSphere(), GetSphere(), m_scale);
	}
	else
	{
		static CSphere dummy_sphere(Vector(0, 0, 0.1f), 0.1f);
		transit->init_sphere(1, &dummy_sphere, 1.0f);
	}

	transit->init_path(cell, &m_Position, &m_Position);
	int collisions = transit->check_collisions(object);

	CTransition::cleanupTransition(transit);

	return collisions;
}

int CPhysicsObj::get_object_info(CTransition *transit, int admin_move)
{
	DWORD object_info = 0;
	if (m_PhysicsState & EDGE_SLIDE_PS)
		object_info = OBJECTINFO::EDGE_SLIDE;

	if (!admin_move)
	{
		if (transient_state & CONTACT_TS)
		{
			BOOL isWater = transient_state & WATER_CONTACT_TS;

			if (check_contact(1))
			{
				transit->init_contact_plane(contact_plane_cell_id, &contact_plane, isWater);

				object_info |= OBJECTINFO::CONTACT_OI;
				if (transient_state & ON_WALKABLE_TS)
					object_info |= OBJECTINFO::ON_WALKABLE_OI;
			}
			else
			{
				transit->init_last_known_contact_plane(contact_plane_cell_id, &contact_plane, isWater);
			}
		}
		if (transient_state & SLIDING_TS)
			transit->init_sliding_normal(&sliding_normal);
	}

	if (part_array && part_array->AllowsFreeHeading())
		object_info |= OBJECTINFO::FREE_ROTATE_OI;

	if (m_PhysicsState & MISSILE_PS)
		object_info |= OBJECTINFO::PATH_CLIPPED_OI;

	return object_info;
}

float CPhysicsObj::GetStepDownHeight()
{
	if (part_array)
		return part_array->GetStepDownHeight();

	return 0.0f;
}

float CPhysicsObj::GetStepUpHeight()
{
	if (part_array)
		return part_array->GetStepUpHeight();

	return 0.0f;
}

int CPhysicsObj::check_contact(int contact)
{
	int result;

	if (transient_state & CONTACT_TS && (m_velocityVector.dot_product(contact_plane.m_normal) > F_EPSILON))
	{
		result = 0;
	}
	else
	{
		result = contact;
	}

	return result;
}

TransitionState CPhysicsObj::FindObjCollisions(CTransition *transition)
{
	int ethereal;

	if (m_PhysicsState & ETHEREAL_PS && m_PhysicsState & IGNORE_COLLISIONS_PS)
		return TransitionState::OK_TS;

	if (weenie_obj)
	{
		if (transition->object_info.state & OBJECTINFO::IS_VIEWER_OI && weenie_obj->IsCreature())
			return TransitionState::OK_TS;
	}

	if (m_PhysicsState & ETHEREAL_PS || transition->object_info.ethereal && !(m_PhysicsState & STATIC_PS))
	{
		if (transition->sphere_path.step_down)
			return TransitionState::OK_TS;

		ethereal = 1;
	}
	else
	{
		ethereal = 0;
	}

	transition->sphere_path.obstruction_ethereal = ethereal;

	DWORD v10 = 1;

	if (!weenie_obj
		|| !weenie_obj->_IsPlayer()
		|| !(transition->object_info.state & OBJECTINFO::IS_PLAYER)
		|| (transition->object_info.state & OBJECTINFO::IS_IMPENETRABLE)
		|| weenie_obj->IsImpenetrable()
		|| (transition->object_info.state & OBJECTINFO::IS_PK) && weenie_obj->IsPK()
		|| (transition->object_info.state & OBJECTINFO::IS_PKLITE) && weenie_obj->IsPKLite())
	{
		v10 = 0;
	}

	TransitionState result = TransitionState::OK_TS;

	int is_creature = 0;
	if (m_PhysicsState & MISSILE_PS || (weenie_obj && weenie_obj->IsCreature()))
		is_creature = 1;

	if (!(m_PhysicsState & HAS_PHYSICS_BSP_PS) || v10 || transition->object_info.missile_ignore(this))
	{
		if (!part_array || !part_array->GetNumCylsphere() || v10 || transition->object_info.missile_ignore(this))
		{
			if (part_array && part_array->GetNumSphere() && !v10 && !transition->object_info.missile_ignore(this))
			{
				DWORD transitionIndex = 0;

				while (transitionIndex < GetNumSphere())
				{
					CSphere *pSphere = GetSphere();

					result = pSphere[transitionIndex].intersects_sphere(&m_Position, m_scale, transition, is_creature);

					if (result != OK_TS)
					{
						goto transition_finish;
					}

					transitionIndex++;
				}
			}
		}
		else
		{
			DWORD transitionIndex = 0;

			while (transitionIndex < GetNumCylsphere())
			{
				CCylSphere *pSphere = GetCylsphere();

				result = pSphere[transitionIndex].intersects_sphere(&m_Position, m_scale, transition);
				if (result != OK_TS)
				{
					goto transition_finish;
				}

				transitionIndex++;
			}
		}
	}
	else
	{
		if (part_array)
		{
			result = part_array->FindObjCollisions(transition);
			goto transition_finish;
		}
	}

	transition->sphere_path.obstruction_ethereal = 0;
	return result;

transition_finish:

	if (result != OK_TS)
	{
		if (!transition->sphere_path.step_down)
		{
			if (m_PhysicsState & STATIC_PS)
			{
				if (!(transition->object_info.state & OBJECTINFO::CONTACT_OI))
					transition->collision_info.collided_with_environment = 1;
			}
			else if (ethereal || is_creature && (transition->object_info.state & OBJECTINFO::IGNORE_CREATURES))
			{
				result = OK_TS;
				transition->collision_info.collision_normal_valid = 0;
				transition->collision_info.add_object(this, OK_TS);
			}
			else
			{
				transition->collision_info.add_object(this, result);
			}
		}
	}

	transition->sphere_path.obstruction_ethereal = 0;
	return result;
}

float CPhysicsObj::get_walkable_z()
{
	return (float)PhysicsGlobals::floor_z;
}

void CPhysicsObj::set_velocity(const Vector &new_velocity, int send_event)
{
	if (new_velocity != m_velocityVector)
	{
		m_velocityVector = new_velocity;

		if (m_velocityVector.magnitude() > max_velocity)
		{
			m_velocityVector.normalize();
			m_velocityVector *= max_velocity;
		}
		jumped_this_frame = 1;
	}

	if (!(m_PhysicsState & STATIC_PS))
	{
		if (!(transient_state & ACTIVE_TS))
		{
			update_time = Timer::cur_time;
		}

		transient_state |= ACTIVE_TS;
	}
}

CSphere dummy_sphere;

CTransition *CPhysicsObj::transition(Position *old_pos, Position *new_pos, int admin_move)
{
	CTransition *transit = CTransition::makeTransition();
	if (!transit)
		return NULL;

	transit->init_object(this, get_object_info(transit, admin_move));

	if (GetNumSphere())
		transit->init_sphere(GetNumSphere(), GetSphere(), m_scale);
	else
		transit->init_sphere(1, &dummy_sphere, 1.0);

	transit->init_path(cell, old_pos, new_pos);

	if (transient_state & STATIONARY_STUCK_TS)
		transit->collision_info.frames_stationary_fall = 3;
	else if (transient_state & STATIONARY_STOP_TS)
		transit->collision_info.frames_stationary_fall = 2;
	else if (transient_state & STATIONARY_FALL_TS)
		transit->collision_info.frames_stationary_fall = 1;

	int result = transit->find_valid_position();

	CTransition::cleanupTransition(transit);

	return result ? transit : NULL;
}

DWORD CPhysicsObj::DoMotion(DWORD motion, MovementParameters *params, int send_event)
{
	last_move_was_autonomous = 1;

	if (movement_manager)
	{
		MovementStruct mvs;
		mvs.params = params;
		mvs.type = MovementTypes::RawCommand;
		mvs.motion = motion;

		return movement_manager->PerformMovement(mvs);
	}

	return 7;
}

DWORD CPhysicsObj::GetSetupID()
{
	if (part_array)
		return part_array->GetSetupID();

	return 0;
}

void CPhysicsObj::store_position(Position *position)
{
	Position temp = *position;

	if ((temp.objcell_id & LandDefs::cellid_mask) < LandDefs::first_envcell_id)
		LandDefs::adjust_to_outside(&temp.objcell_id, &temp.frame.m_origin);

	set_cell_id(temp.objcell_id);
	set_frame(temp.frame);
}

void CPhysicsObj::add_particle_shadow_to_cell()
{
	num_shadow_objects = 1;
	if (shadow_objects.alloc_size < 1)
		shadow_objects.grow(1);

	shadow_objects.data[0].set_physobj(this);
	shadow_objects.data[0].m_CellID = cell->id;
	cell->add_shadow_object(&shadow_objects.data[0], 1);
	if (part_array)
		part_array->AddPartsShadow(cell, 1);
}

void CPhysicsObj::add_shadows_to_cells(CELLARRAY *cell_array)
{
	if (m_PhysicsState & PARTICLE_EMITTER_PS)
	{
		add_particle_shadow_to_cell();
	}
	else
	{
		num_shadow_objects = cell_array->num_cells;
		if (num_shadow_objects > shadow_objects.alloc_size)
			shadow_objects.grow(num_shadow_objects);

		for (DWORD i = 0; i < num_shadow_objects; i++)
		{
			shadow_objects.array_data[i].set_physobj(this);
			shadow_objects.array_data[i].m_CellID = cell_array->cells.array_data[i].cell_id;
		}

		for (DWORD i = 0; i < num_shadow_objects; i++)
		{
			CObjCell *pObjCell = cell_array->cells.array_data[i].cell;
			if (pObjCell)
			{
				pObjCell->add_shadow_object(&shadow_objects.array_data[i], cell_array->num_cells);
				if (part_array)
					part_array->AddPartsShadow(pObjCell, num_shadow_objects);
			}
			else
			{
				shadow_objects.array_data[i].cell = NULL;
			}

			shadow_objects.array_data[i].set_physobj(this);
			shadow_objects.array_data[i].m_CellID = cell_array->cells.array_data[i].cell_id;
		}
	}

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
		{
			children->objects.data[i]->add_shadows_to_cells(cell_array);
		}
	}
}

void CPhysicsObj::remove_shadows_from_cells()
{
	for (DWORD i = 0; i < num_shadow_objects; i++)
	{
		CObjCell *cell_ = shadow_objects.array_data[i].cell;

		if (cell_)
		{
			cell_->remove_shadow_object(&shadow_objects.array_data[i]);
			if (part_array)
				part_array->RemoveParts(cell_);
		}
	}
	num_shadow_objects = 0;

	DWORD num_children = children ? children->num_objects : 0;

	for (DWORD i = 0; i < num_children; i++)
	{
		children->objects.array_data[i]->remove_shadows_from_cells();
	}
}

void CPhysicsObj::leave_visibility()
{
	prepare_to_leave_visibility();
	store_position(&m_Position);

	if (obj_maint)
		obj_maint->GotoLostCell(this, m_Position.objcell_id);

	transient_state &= ~(ACTIVE_TS);
}

int CPhysicsObj::prepare_to_leave_visibility()
{
	remove_shadows_from_cells();
	obj_maint->RemoveFromLostCell(this);
	leave_cell(0);
	obj_maint->AddObjectToBeDestroyed(id);

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
		{
			obj_maint->AddObjectToBeDestroyed(children->objects.data[i]->id);
		}
	}

	return 1;
}

void CPhysicsObj::prepare_to_enter_world()
{
	update_time = Timer::cur_time;

	obj_maint->RemoveFromLostCell(this);
	obj_maint->RemoveObjectToBeDestroyed(id);

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
		{
			obj_maint->RemoveObjectToBeDestroyed(children->objects.array_data[i]->id);
		}
	}

	set_active(TRUE);
}

void CPhysicsObj::find_bbox_cell_list(CELLARRAY *cell_array)
{
	cell_array->num_cells = 0;
	cell_array->added_outside = 0;
	cell_array->add_cell(cell->id, cell);

	if (cell)
	{
		if (part_array)
		{
			for (DWORD i = 0; i < cell_array->num_cells; i++)
			{
				CObjCell *pCell = cell_array->cells.data[i].cell;
				if (pCell)
				{
					part_array->calc_cross_cells_static(pCell, cell_array);
				}
			}
		}
	}
}

void CPhysicsObj::calc_cross_cells()
{
	static CELLARRAY cell_array_0;
	cell_array_0.num_cells = 0;
	cell_array_0.added_outside = 0;
	cell_array_0.do_not_load_cells = 0;

	if (m_PhysicsState & HAS_PHYSICS_BSP_PS)
	{
		find_bbox_cell_list(&cell_array_0);
	}
	else
	{
		if (GetNumCylsphere())
		{
			CObjCell::find_cell_list(&m_Position, GetNumCylsphere(), GetCylsphere(), &cell_array_0, 0);
		}
		else
		{
			CSphere *sphere;
			if (part_array)
				sphere = part_array->GetSortingSphere();
			else
				sphere = &dummy_sphere;

			CObjCell::find_cell_list(&m_Position, sphere, &cell_array_0, 0);
		}
	}

	remove_shadows_from_cells();
	add_shadows_to_cells(&cell_array_0);
}

void CPhysicsObj::change_cell(CObjCell *new_cell)
{
	if (cell)
		leave_cell(1);

	if (new_cell)
	{
		enter_cell(new_cell);
	}
	else
	{
		m_Position.objcell_id = 0;
		if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
		{
			if (part_array)
				part_array->SetCellID(0);
		}

		cell = NULL;
	}
}

SetPositionError CPhysicsObj::ForceIntoCell(CObjCell *_pNewCell, Position *_pos)
{
	if (_pNewCell)
	{
		set_frame(_pos->frame);
		if (cell != _pNewCell)
		{
			change_cell(_pNewCell);
			calc_cross_cells();
		}

		return SetPositionError::OK_SPE;
	}

	return SetPositionError::NO_CELL_SPE;
}

int CPhysicsObj::CheckPositionInternal(CObjCell *new_cell, Position *new_pos, CTransition *transit, const SetPositionStruct &sps)
{
	// !! modifies new_pos

	transit->init_path(new_cell, 0, new_pos);

	if (!(sps.flags & SLIDE_SPF))
		transit->sphere_path.placement_allows_sliding = 0;

	if (transit->find_valid_position())
	{
		if (sps.flags & SLIDE_SPF)
			return 1;

		if (transit->sphere_path.curr_pos.frame.m_origin.x - new_pos->frame.m_origin.x < 0.050000001
			&& transit->sphere_path.curr_pos.frame.m_origin.y - new_pos->frame.m_origin.y < 0.050000001
			&& transit->sphere_path.curr_pos.objcell_id == new_cell->id)
		{
			new_pos->frame.m_origin = transit->sphere_path.curr_pos.frame.m_origin;
			return 1;
		}
	}
	return 0;
}

int CPhysicsObj::track_object_collision(const CPhysicsObj *object, int prev_has_contact)
{
	if (object->m_PhysicsState & STATIC_PS)
		return report_environment_collision(prev_has_contact);

	if (!collision_table)
		collision_table = new LongNIValHash<CPhysicsObj::CollisionRecord>(4);

	if (!collision_table)
		return 0;

	CPhysicsObj::CollisionRecord record;
	record.touched_time = PhysicsTimer::curr_time;
	record.ethereal = object->m_PhysicsState & ETHEREAL_PS;
	if (collision_table->clobber(&record, object->id))
		return 0;
	
	return report_object_collision((CPhysicsObj *)object, prev_has_contact);
}

void ObjCollisionProfile::SetMissile(const int isMissile)
{
	if (isMissile)
		_bitfield |= 8;
	else
		_bitfield &= 0xFFFFFFF7;
}

void ObjCollisionProfile::SetInContact(const int hasContact)
{
	if (hasContact)
		_bitfield |= 0x10;
	else
		_bitfield &= 0xFFFFFFEF;
}

void ObjCollisionProfile::SetMeInContact(const int hasContact)
{
	if (hasContact)
		_bitfield |= 0x20;
	else
		_bitfield &= 0xFFFFFFDF;
}

void ObjCollisionProfile::SetCreature(const int isCreature)
{
	if (isCreature)
		_bitfield |= 0x1;
	else
		_bitfield &= 0xFFFFFFFE;
}

void ObjCollisionProfile::SetPlayer(const int isPlayer)
{
	if (isPlayer)
		_bitfield |= 0x2;
	else
		_bitfield &= 0xFFFFFFFD;
}

void ObjCollisionProfile::SetAttackable(const int isAttackable)
{
	if (isAttackable)
		_bitfield |= 0x4;
	else
		_bitfield &= 0xFFFFFFFB;
}

void ObjCollisionProfile::SetDoor(const int isDoor)
{
	if (isDoor)
		_bitfield |= 0x40;
	else
		_bitfield &= 0xFFFFFFBF;
}

bool ObjCollisionProfile::IsDoor() const
{
	return (_bitfield & 0x40) ? true : false;
}

int CPhysicsObj::build_collision_profile(ObjCollisionProfile *prof, CPhysicsObj *obj, Vector *vel, const int amIInContact, const int objIsMissile, const int objHasContact) const
{
	if (obj->weenie_obj && obj->weenie_obj->InqCollisionProfile(*prof))
	{
		prof->id = obj->id;
		prof->velocity = *vel;
		prof->SetMissile(objIsMissile);
		prof->SetInContact(objHasContact);
		prof->SetMeInContact(amIInContact);
		return TRUE;
	}

	return FALSE;
}

int CPhysicsObj::report_object_collision(CPhysicsObj *object, int prev_has_contact)
{
	if (object->m_PhysicsState & REPORT_COLLISIONS_AS_ENVIRONMENT_PS)
		return report_environment_collision(prev_has_contact);

	Vector collision_velocity = m_velocityVector - object->m_velocityVector;

	BOOL bReportedCollided = FALSE;

	if (!(object->m_PhysicsState & IGNORE_COLLISIONS_PS))
	{
		if (m_PhysicsState & REPORT_COLLISIONS_PS && weenie_obj)
		{
			if (m_PhysicsState & MISSILE_PS)
			{
				AtkCollisionProfile prof;
				prof.id = object->id;
				prof.part = -1;
				prof.location = object->m_Position.determine_quadrant(object->GetHeight(), &m_Position);
				weenie_obj->DoCollision(prof);
			}
			else
			{
				ObjCollisionProfile prof;

				build_collision_profile(
					&prof,
					object,
					&collision_velocity,
					prev_has_contact,
					object->m_PhysicsState & MISSILE_PS,
					object->transient_state & CONTACT_TS);
				weenie_obj->DoCollision(prof);
			}

			bReportedCollided = TRUE;
		}

		if (m_PhysicsState & MISSILE_PS)
			m_PhysicsState &= ~(MISSILE_PS|ALIGNPATH_PS|PATHCLIPPED_PS);
	}

	if (object->m_PhysicsState & REPORT_COLLISIONS_PS && !(m_PhysicsState & IGNORE_COLLISIONS_PS) && object->weenie_obj)
	{
		if (object->m_PhysicsState & MISSILE_PS)
		{
			AtkCollisionProfile prof;
			prof.id = id;
			prof.part = -1;
			prof.location = m_Position.determine_quadrant(GetHeight(), &object->m_Position);
			object->weenie_obj->DoCollision(prof);
		}
		else
		{
			ObjCollisionProfile prof;
			
			object->build_collision_profile(
				&prof,
				this,
				&collision_velocity,
				object->transient_state & 1,
				prev_has_contact,
				m_PhysicsState & MISSILE_PS);
			object->weenie_obj->DoCollision(prof);
		}

		bReportedCollided = TRUE;
	}

	return bReportedCollided;
}

int CPhysicsObj::play_default_script()
{
	if (cell)
	{
		if (physics_script_table)
		{
			DWORD scriptID = physics_script_table->GetScript(m_DefaultScript, m_DefaultScriptIntensity);
			return play_script_internal(scriptID);
		}

		return 0;
	}

	return 1;
}

int CPhysicsObj::report_environment_collision(int prev_has_contact)
{
	int result = 0;

	if (!colliding_with_environment)
	{
		if (m_PhysicsState & REPORT_COLLISIONS_PS && weenie_obj)
		{
			EnvCollisionProfile prof;
			prof.velocity = m_velocityVector;
			prof.SetMeInContact(prev_has_contact);

			weenie_obj->DoCollision(prof);
			result = 1;
		}

		colliding_with_environment = 1;

		if (m_PhysicsState & MISSILE_PS)
			m_PhysicsState &= ~((DWORD)(MISSILE_PS | ALIGNPATH_PS | PATHCLIPPED_PS)); // 0xFFFFFCBF
	}

	return result;
}

int CPhysicsObj::handle_all_collisions(COLLISIONINFO *collisions, int prev_has_contact, int prev_on_walkable)
{
	int retval = 0;
	int apply_bounce = 1;

	if (prev_on_walkable && (transient_state & ON_WALKABLE_TS) && !(m_PhysicsState & SLEDDING_PS))
		apply_bounce = 0;

	for (DWORD i = 0; i < collisions->num_collide_object; i++)
	{
		const CPhysicsObj *pObject = collisions->collide_object.data[i];

		if (pObject && track_object_collision(pObject, prev_has_contact))
			retval = 1;
	}

	report_collision_end(0);

	if (colliding_with_environment)
	{
		colliding_with_environment = collisions->collided_with_environment != 0;
	}
	else
	{
		if (collisions->collided_with_environment || (!prev_on_walkable && (transient_state & ON_WALKABLE_TS)))
		{
			report_environment_collision(prev_has_contact);
			retval = 1;
		}
	}

	if (collisions->frames_stationary_fall > 1)
	{
		m_velocityVector = Vector(0, 0, 0);

		if (collisions->frames_stationary_fall == 3)
		{
			transient_state &= 0xFFFFFF8F;
			return retval;
		}
	}
	else
	{
		if (apply_bounce && collisions->collision_normal_valid)
		{
			if (m_PhysicsState & INELASTIC_PS)
			{
				m_velocityVector = Vector(0, 0, 0);
			}
			else
			{
				float dp = m_velocityVector.dot_product(collisions->collision_normal);

				if (dp < 0.0)
				{
					float someVal = -(dp * (m_fElasticity + 1.0));

					m_velocityVector += (collisions->collision_normal * someVal);
				}
			}
		}
	}

	if (!collisions->frames_stationary_fall)
	{
		transient_state &= ~((DWORD)(STATIONARY_FALL_TS | STATIONARY_STOP_TS | STATIONARY_STUCK_TS)); // 0xFFFFFF8F;
		return retval;
	}

	if (collisions->frames_stationary_fall == 1)
	{
		transient_state |= STATIONARY_FALL_TS;
		return retval;
	}
	else if (collisions->frames_stationary_fall == 2)
	{
		transient_state |= STATIONARY_STOP_TS;
		return retval;
	}

	transient_state |= STATIONARY_STUCK_TS;
	return retval;
}

int CPhysicsObj::SetPositionInternal(CTransition *transit)
{
	unsigned int prev_on_walkable = transient_state & ON_WALKABLE_TS;
	CObjCell *transitCell = transit->sphere_path.curr_cell;
	unsigned int prev_contact = transient_state & CONTACT_TS;
	if (transitCell)
	{
		if (cell == transitCell)
		{
			m_Position.objcell_id = transit->sphere_path.curr_pos.objcell_id;
			if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
			{
				if (part_array)
					part_array->SetCellID(transit->sphere_path.curr_pos.objcell_id);
			}

			if (children)
			{
				for (DWORD i = 0; i < children->num_objects; i++)
				{
					CPhysicsObj *child = children->objects.data[i];

					child->m_Position.objcell_id = transit->sphere_path.curr_pos.objcell_id;

					if (!(child->m_PhysicsState & PARTICLE_EMITTER_PS))
					{
						if (child->part_array)
							child->part_array->SetCellID(transit->sphere_path.curr_pos.objcell_id);
					}
				}
			}
		}
		else
		{
			change_cell(transitCell);
		}

		set_frame(transit->sphere_path.curr_pos.frame);

		int is_water = transit->collision_info.contact_plane_is_water;

		contact_plane_cell_id = transit->collision_info.contact_plane_cell_id;
		contact_plane = transit->collision_info.contact_plane;

		if (transit->collision_info.contact_plane_valid)
			transient_state |= CONTACT_TS;
		else
			transient_state &= ~((DWORD)CONTACT_TS);

		calc_acceleration();

		if (is_water)
			transient_state |= WATER_CONTACT_TS;
		else
			transient_state &= ~((DWORD)WATER_CONTACT_TS);

		if (transient_state & CONTACT_TS)
		{
			if (contact_plane.m_normal.z < (double)PhysicsGlobals::floor_z)
				set_on_walkable(0);
			else
				set_on_walkable(1);
		}
		else
		{
			BOOL bWasOnWalkable = transient_state & ON_WALKABLE_TS;
			transient_state &= ~((DWORD)ON_WALKABLE_TS);

			if (bWasOnWalkable)
			{
				if (movement_manager)
					movement_manager->LeaveGround();
			}

			calc_acceleration();
		}

		sliding_normal = transit->collision_info.sliding_normal;

		if (transit->collision_info.sliding_normal_valid)
			transient_state |= SLIDING_TS;
		else
			transient_state &= ~((DWORD)SLIDING_TS);

		handle_all_collisions(&transit->collision_info, prev_contact, prev_on_walkable);

		if (cell)
		{
			if (m_PhysicsState & HAS_PHYSICS_BSP_PS)
			{
				calc_cross_cells();
				return 1;
			}

			if (transit->cell_array.num_cells)
			{
				remove_shadows_from_cells();
				add_shadows_to_cells(&transit->cell_array);
				return 1;
			}
		}
	}
	else
	{
		prepare_to_leave_visibility();
		store_position(&transit->sphere_path.curr_pos);

		obj_maint->GotoLostCell(this, m_Position.objcell_id);

		transient_state &= ~((DWORD)ACTIVE_TS);
	}

	return 1;
}

SetPositionError CPhysicsObj::SetPositionInternal(Position *p, const SetPositionStruct &sps, CTransition *transit)
{
	if (!cell)
		prepare_to_enter_world();

	CObjCell *newCell = NULL;
	AdjustPosition(p, &transit->sphere_path.local_sphere->center, (CObjCell **)&newCell, (sps.flags & DONOTCREATECELLS_SPF) ? 1 : 0, 1);

	if (newCell)
	{
		if (weenie_obj && (weenie_obj->IsStorage() || weenie_obj->IsCorpse()))
		{
			return ForceIntoCell(newCell, p);
		}

		if (sps.flags & DONOTCREATECELLS_SPF)
			transit->cell_array.do_not_load_cells = 1;

		if (!CheckPositionInternal(newCell, p, transit, sps))
			return handle_all_collisions(&transit->collision_info, 0, 0) != 0 ? COLLIDED_SPE : NO_VALID_POSITION_SPE;

		if (!transit->sphere_path.curr_cell)
			return NO_CELL_SPE;

		if (!SetPositionInternal(transit))
			return GENERAL_FAILURE_SPE;
	}
	else
	{
		prepare_to_leave_visibility();
		store_position(p);

		obj_maint->GotoLostCell(this, this->m_Position.objcell_id);

		set_active(FALSE);
	}

	return SetPositionError::OK_SPE;
}

SetPositionError CPhysicsObj::SetScatterPositionInternal(const SetPositionStruct &sps, CTransition *transit)
{
	SetPositionError result = SetPositionError::GENERAL_FAILURE_SPE;

	for (DWORD i = 0; i < sps.num_tries; i++)
	{
		Position new_p;
		Vector *new_p_origin = new_p.get_origin();

		new_p = sps.pos;

		new_p_origin->x += Random::RollDice(-1.0, 1.0) * sps.xrad;
		new_p_origin->y += Random::RollDice(-1.0, 1.0) * sps.yrad;

		result = SetPositionInternal(&new_p, sps, transit);
		if (result == SetPositionError::OK_SPE)
			break;
	}

	return result;
}

SetPositionError CPhysicsObj::SetPositionInternal(const SetPositionStruct &sps, CTransition *transit)
{
	SetPositionError result;

	if (sps.flags & RANDOMSCATTER_SPF)
	{
		result = SetScatterPositionInternal(sps, transit);
	}
	else
	{
		Position p = sps.pos;
		result = SetPositionInternal(&p, sps, transit);

		if (result != OK_SPE)
		{
			if (sps.flags & SCATTER_SPF)
				result = SetScatterPositionInternal(sps, transit);
		}
	}

	return result;
}

SetPositionError CPhysicsObj::SetPosition(const SetPositionStruct &sps)
{
	SetPositionError result;
	CTransition *transit = CTransition::makeTransition();

	if (transit)
	{
		transit->init_object(this, 0);

		if (GetNumSphere())
		{
			transit->init_sphere(GetNumSphere(), GetSphere(), m_scale);
		}
		else
		{
			transit->init_sphere(1, &dummy_sphere, 1.0f);
		}

		result = SetPositionInternal(sps, transit);
		CTransition::cleanupTransition(transit);
	}
	else
	{
		result = SetPositionError::GENERAL_FAILURE_SPE;
	}

	return result;
}

BOOL CPhysicsObj::enter_world(Position *position)
{
	store_position(position);

	return enter_world(TRUE);
}

BOOL CPhysicsObj::enter_world(BOOL slide)
{
	if (parent)
		return FALSE;

	update_time = Timer::cur_time;

	DWORD flags = PLACEMENT_SPF;
	if (slide)
		flags |= SLIDE_SPF;

#if PHATSDK_IS_SERVER
	// flags |= SCATTER_SPF;
#endif

	SetPositionStruct sps;
	sps.pos = m_Position;
	sps.flags = flags;
	if (SetPosition(sps))
		return FALSE;

	set_active(TRUE);

	if (part_array)
		part_array->HandleEnterWorld();

	if (movement_manager)
		movement_manager->HandleEnterWorld();

	return TRUE;
}

BOOL CPhysicsObj::set_state(DWORD new_state, BOOL send_event)
{
	DWORD stateChange = new_state ^ m_PhysicsState;
	m_PhysicsState = new_state;

	if (stateChange & LIGHTING_ON_PS)
	{
		if (new_state & LIGHTING_ON_PS)
		{
			m_PhysicsState = new_state | LIGHTING_ON_PS;
			if (part_array)
				part_array->InitLights();
		}
		else
		{
			m_PhysicsState = new_state & ~(LIGHTING_ON_PS);
			if (part_array)
				part_array->DestroyLights();
		}
	}

	if (stateChange & NODRAW_PS)
		set_nodraw(m_PhysicsState & NODRAW_PS, 0);

	if (stateChange & HIDDEN_PS)
		set_hidden(m_PhysicsState & HIDDEN_PS, 0);

#if PHATSDK_IS_SERVER
	if (send_event)
	{
		Send_StateChangeEvent();
	}
#endif

	return 1;
}

void CPhysicsObj::set_hidden(BOOL hidden, BOOL send_event)
{
	// MISSING CODE HERE
	// UNFINISHED();
}

BOOL CPhysicsObj::AdjustPosition(Position *p, Vector *low_pt, CObjCell **new_cell, int bDontCreateCells, int bSearchCells)
{
	Vector pt;

	WORD lowCell = p->objcell_id & 0xFFFF;

	if ((lowCell < 1 || lowCell > 0x40) && (lowCell < 0x100 || lowCell > 0xFFFD) && lowCell != 0xFFFF)
		return 0;

	*new_cell = NULL;
	if (((WORD)p->objcell_id) >= 0x100u)
	{
		Position baseP = *p; // custom
		CEnvCell *pEnvCell = (CEnvCell *)CObjCell::GetVisible(p->objcell_id);

		if (pEnvCell)
		{
			pt = p->localtoglobal(*p, *low_pt);
			CEnvCell *pChildCell = pEnvCell->find_visible_child_cell(&pt, bSearchCells);

			if (pChildCell)
			{
				p->objcell_id = pChildCell->id; // pChildCell->m_DID.id;
				*new_cell = (CObjCell *)pChildCell;
				return TRUE;
			}

			if (pEnvCell->seen_outside)
			{
				p->adjust_to_outside();

				if (p->objcell_id)
				{
					*new_cell = CObjCell::GetVisible(p->objcell_id);
					return TRUE;
				}
			}
		}

		// custom	
		unsigned int i = 0;
		while (pEnvCell = (CEnvCell *)CObjCell::GetVisible((baseP.objcell_id & 0xFFFF0000) + 0x100 + i))
		{
			pt = baseP.localtoglobal(baseP, *low_pt);
			if (pEnvCell->point_in_cell(pt))
			{
				p->objcell_id = pEnvCell->id;
				*new_cell = (CObjCell *)pEnvCell;
				return TRUE;
			}

			i++;
		}
	}
	else
	{
		p->adjust_to_outside();

		if (p->objcell_id)
		{
			*new_cell = CObjCell::GetVisible(p->objcell_id);
			return TRUE;
		}
	}
	return FALSE;
}

void CPhysicsObj::SetNoDraw(int no_draw)
{
	if (part_array)
		part_array->SetNoDrawInternal(no_draw);
}

void CPhysicsObj::unset_parent()
{
	if (!parent)
		return;

	if (parent->children)
		parent->children->remove_child(this);

	if (parent->m_PhysicsState & HIDDEN_PS)
	{
		m_PhysicsState &= ~NODRAW_PS;

		if (part_array)
			part_array->SetNoDrawInternal(0);
	}

	parent = NULL;
	update_time = Timer::cur_time;

	clear_transient_states();
}

void CPhysicsObj::unparent_children()
{
	while (children && children->num_objects)
	{
		children->objects.data[0]->unset_parent();
	}
}

void CPhysicsObj::clear_transient_states()
{
	transient_state &= 0xFFFFFFFE;
	calc_acceleration();

	BOOL bWasOnWalkable = transient_state & ON_WALKABLE_TS;
	transient_state &= 0xFFFFFFF5;

	if (bWasOnWalkable)
	{
		if (movement_manager)
			movement_manager->LeaveGround();
	}

	calc_acceleration();

	transient_state &= 0xFFFFFE0B;
}

BOOL CPhysicsObj::set_parent(CPhysicsObj *obj, unsigned int part_index, Frame *frame)
{
	if (obj)
	{
		if (obj->add_child(this, part_index, frame))
		{
			m_bExaminationObject = obj->m_bExaminationObject;

			unset_parent();
			leave_world();

			parent = obj;

			if (obj->cell)
			{
				change_cell(obj->cell);
				obj->UpdateChild(this, part_index, frame);
				recalc_cross_cells();
			}			

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CPhysicsObj::add_child(CPhysicsObj *obj, DWORD location_id)
{
	if (obj == this)
		return FALSE;

	LocationType *holdingLocation = NULL;
	if (part_array && (holdingLocation = part_array->setup->GetHoldingLocation(location_id)) != 0)
	{
		if (!children)
			children = new CHILDLIST();

		children->add_child(obj, &holdingLocation->frame, holdingLocation->part_id, location_id);
		return TRUE;
	}

	return FALSE;
}

BOOL CPhysicsObj::set_parent(CPhysicsObj *obj, DWORD location_id)
{
	if (obj && obj->add_child(this, location_id))
	{
		unset_parent();
		leave_world();

		parent = obj;

		if (obj->cell)
		{
			change_cell(obj->cell);

			if (obj->children)
			{
				WORD index;
				if (obj->children->FindChildIndex(this, &index))
				{
					obj->UpdateChild(this, obj->children->part_numbers.data[index], &obj->children->frames.data[index]);
					recalc_cross_cells();
				}
			}
		}

		if (parent->m_PhysicsState & HIDDEN_PS)
		{
			m_PhysicsState |= NODRAW_PS;

			if (part_array)
				part_array->SetNoDrawInternal(1);
		}

		return TRUE;
	}

	return FALSE;
}

void CPhysicsObj::recalc_cross_cells()
{
	if (part_array)
	{
		if (m_Position.objcell_id)
		{
			calc_cross_cells();
		}
		else
		{
			if (!m_bExaminationObject)
				return;

			if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
				return;

			add_particle_shadow_to_cell();
		}

		if (children)
		{
			for (DWORD i = 0; i < children->num_objects; i++)
			{
				children->objects.data[i]->recalc_cross_cells();
			}
		}
	}
}

BOOL CPhysicsObj::add_child(CPhysicsObj *obj, unsigned int part_index, Frame *frame)
{
	if (obj == this)
		return FALSE;
		
	if (part_index == -1 || part_index < part_array->num_parts)
	{
		if (!children)
			children = new CHILDLIST();

		children->add_child(obj, frame, part_index, 0);
		return TRUE;
	}
	
	return FALSE;
}

void CPhysicsObj::RemovePartFromShadowCells(CPhysicsPart *part)
{
	if (cell)
		part->pos.objcell_id = cell->GetID();
	
	for (DWORD i = 0; i < num_shadow_objects; i++)
	{
		if (shadow_objects.data[i].cell)
			shadow_objects.data[i].cell->remove_part(part);
	}
}

void CPhysicsObj::AddPartToShadowCells(CPhysicsPart *part)
{
	if (cell)
		part->pos.objcell_id = cell->GetID();

	for (DWORD i = 0; i < num_shadow_objects; i++)
	{
		if (shadow_objects.data[i].cell)
			shadow_objects.data[i].cell->add_part(part, 0, &shadow_objects.data[i].cell->pos.frame, num_shadow_objects);
	}
}

CPhysicsObj *CPhysicsObj::makeParticleObject(unsigned int num_parts, CSphere *sorting_sphere)
{
	CPhysicsObj *obj = new CPhysicsObj();

	obj->id = 0;
	obj->m_PhysicsState |= PARTICLE_EMITTER_PS| STATIC_PS;
	obj->part_array = CPartArray::CreateParticle(obj, num_parts, sorting_sphere);

	if (!obj->part_array)
	{
		delete obj;
		return NULL;
	}

	return obj;
}

#if PHATSDK_RENDER_AVAILABLE
void CPhysicsObj::DrawRecursive()
{
	if (part_array)
		part_array->Draw(&m_Position);

	if (children)
	{
		for (DWORD i = 0; i < children->num_objects; i++)
			children->objects.array_data[i]->DrawRecursive();
	}
}
#endif

void CPhysicsObj::UpdateViewerDistance()
{
#if PHATSDK_RENDER_AVAILABLE
	const float particle_distance_2dsq = FLT_MAX; // Render::*
	const float object_distance_2dsq = FLT_MAX;    // Render::*

												   // The distance at which we should be degraded.
	float degrade_dist = (m_PhysicsState & PARTICLE_EMITTER_PS) ? particle_distance_2dsq : object_distance_2dsq;

	// Our offset relative to the viewer.
	Vector offset = Render::ViewerPos.get_offset(m_Position);

	// 3D distance
	float distance3D = CYpt = offset.magnitude();

	// 2D distance (squared)
	float distance2Dsq = offset.x*offset.x + offset.y*offset.y;

	if (TRUE) // !! never degrade (substitutes for "if (distance2Dsq < degrade_dist)")
	{
		if (part_array)
			part_array->UpdateViewerDistance();
	}
	else
	{
		// Do degrading..
	}
#endif
}

void CPhysicsObj::set_sequence_animation(DWORD AnimationID, BOOL ClearAnimations, long StartFrame, float FrameRate)
{
	if (!part_array)
		return;

	AnimData adata;
	adata.anim_id = AnimationID;
	adata.low_frame = StartFrame;
	adata.framerate = FrameRate;

	if (ClearAnimations)
		part_array->sequence.clear();

	part_array->sequence.append_animation(&adata);
}

DWORD CPhysicsObj::create_particle_emitter(DWORD emitter_info_id, unsigned int part_index, Frame *offset, unsigned int emitter_id)
{
	if (!particle_manager)
		particle_manager = new ParticleManager();

	return particle_manager->CreateParticleEmitter(this, emitter_info_id, part_index, offset, emitter_id);
}

void CPhysicsObj::remove_parts(CObjCell *obj_cell)
{
	if (part_array)
		part_array->RemoveParts(obj_cell);
}

BOOL CPhysicsObj::is_valid_walkable(Vector *normal)
{
	return normal->z >= (double)PhysicsGlobals::floor_z;
}

void CPhysicsObj::set_target(unsigned int context_id, unsigned int object_id, float radius, long double quantum)
{
	if (!target_manager)
		target_manager = new TargetManager(this);

	target_manager->SetTarget(context_id, object_id, radius, quantum);
}

float CPhysicsObj::GetHeight() const
{
	if (part_array)
		return part_array->GetHeight();

	return 0.0;
}

float CPhysicsObj::GetRadius() const
{
	if (part_array)
		return part_array->GetRadius();

	return 0.0;
}

CPhysicsObj *CPhysicsObj::GetObject(DWORD object_id)
{
	if (obj_maint)
		return obj_maint->GetObject(object_id);

	return NULL;
}

void CPhysicsObj::MakePositionManager()
{
	if (!position_manager)
		position_manager = PositionManager::Create(this);

	set_active(TRUE);
}

void CPhysicsObj::ConstrainTo(Position *p, float start_distance, float max_distance)
{
	MakePositionManager();

	if (position_manager)
		position_manager->ConstrainTo(p, start_distance, max_distance);
}

void CPhysicsObj::UnConstrain()
{
	if (position_manager)
		position_manager->UnConstrain();
}

void CPhysicsObj::TurnToObject(DWORD object_id, MovementParameters *params)
{
	if (obj_maint)
	{
		CPhysicsObj *pTarget = obj_maint->GetObject(object_id);
		if (pTarget)
		{
			if (pTarget->parent)
				pTarget = pTarget->parent;

			TurnToObject_Internal(object_id, pTarget->id, params);
		}
	}
}

void CPhysicsObj::TurnToObject_Internal(DWORD object_id, DWORD top_level_id, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	MovementStruct mvs;
	mvs.type = MovementTypes::TurnToObject;
	mvs.object_id = object_id;
	mvs.top_level_id = top_level_id;
	mvs.params = params;
	movement_manager->PerformMovement(mvs);
}

PositionManager *CPhysicsObj::get_position_manager()
{
	MakePositionManager();
	return position_manager;
}

DWORD CPhysicsObj::PerformMovement(MovementStruct &mvs)
{
	MakeMovementManager(TRUE);
	return movement_manager->PerformMovement(mvs);
}

void CPhysicsObj::TurnToHeading(MovementParameters *params)
{
	MakeMovementManager(TRUE);

	MovementStruct mvs;
	mvs.type = MovementTypes::TurnToHeading;
	mvs.params = params;
	movement_manager->PerformMovement(mvs);
}

Vector CPhysicsObj::get_velocity()
{
	return cached_velocity;
}

double CPhysicsObj::get_target_quantum()
{
	if (target_manager && target_manager->target_info != 0)
		return target_manager->target_info->quantum;
	
	return 0.0;
}

void CPhysicsObj::set_target_quantum(double new_quantum)
{
	if (target_manager)
		target_manager->SetTargetQuantum(new_quantum);
}

void CPhysicsObj::add_voyeur(DWORD object_id, float radius, float quantum)
{
	if (!target_manager)
		target_manager = new TargetManager(this);
	
	target_manager->AddVoyeur(object_id, radius, quantum);
}

int CPhysicsObj::remove_voyeur(DWORD object_id)
{
	if (target_manager)
		return target_manager->RemoveVoyeur(object_id);

	return 0;
}

void CPhysicsObj::receive_detection_update(DetectionInfo *info)
{
	if (detection_manager)
	{
		UNFINISHED();
		// detection_manager->ReceiveDetectionUpdate(info);
		set_active(TRUE);
	}
}

void CPhysicsObj::receive_target_update(TargetInfo *info)
{
	if (target_manager)
		target_manager->ReceiveUpdate(info);
}

void CPhysicsObj::HandleUpdateTarget(TargetInfo target_info)
{
	if (!target_info.context_id)
	{
		if (movement_manager)
			movement_manager->HandleUpdateTarget(TargetInfo(target_info));

		if (position_manager)
			position_manager->HandleUpdateTarget(TargetInfo(target_info));
	}
}

void CPhysicsObj::MoveToObject(DWORD object_id, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	CPhysicsObj *target = GetObject(object_id);

	if (target)
	{
		MoveToObject_Internal(object_id, target->parent ? target->parent->id : target->id, target->GetRadius(), target->GetHeight(), params);
	}
}

void CPhysicsObj::MoveToObject_Internal(DWORD object_id, DWORD top_level_id, float object_radius, float object_height, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	MovementStruct mvs;

	mvs.top_level_id = top_level_id;
	mvs.radius = object_radius;
	mvs.object_id = object_id;
	mvs.params = params;
	mvs.type = MovementTypes::MoveToObject;
	mvs.height = object_height;
	movement_manager->PerformMovement(mvs);
}

void CPhysicsObj::exit_world()
{
	if (part_array)
		part_array->HandleExitWorld();

	if (movement_manager)
		movement_manager->HandleExitWorld();

	if (position_manager)
		position_manager->UnStick();

	if (target_manager)
	{
		target_manager->ClearTarget();
		target_manager->NotifyVoyeurOfEvent(ExitWorld_TargetStatus);
	}

	if (detection_manager)
	{
		// detection_manager->DestroyDetectionCylsphere(0);
		UNFINISHED();
	}

	report_collision_end(1);
}

BOOL CPhysicsObj::HasAnims()
{
	if (part_array)
		return part_array->HasAnims();
	
	return FALSE;
}

Vector CPhysicsObj::get_local_physics_velocity()
{
	Vector localVel;
	localVel = m_Position.globaltolocalvec(m_velocityVector);
	return localVel;
}

void CPhysicsObj::SetScaleStatic(float new_scale)
{
	m_scale = new_scale;

	if (part_array)
		part_array->SetScaleInternal(Vector(new_scale, new_scale, new_scale));
}

SetPositionError CPhysicsObj::SetPositionSimple(Position *p, int sliding)
{
	int flags = 4098;
	if (sliding)
		flags = 4114;

	SetPositionStruct sps;
	sps.SetPosition(p);
	sps.SetFlags(flags);
	return SetPosition(sps);
}

DWORD CPhysicsObj::get_sticky_object_id()
{
	if (position_manager)
		return position_manager->GetStickyObjectID();

	return 0;
}

void CPhysicsObj::InterpolateTo(Position *p, int keep_heading)
{
	MakePositionManager();
	position_manager->InterpolateTo(p, keep_heading);
}

double CPhysicsObj::GetAutonomyBlipDistance()
{
	/*
	if (CPhysicsObj::player_object)
	{
		if ((this->m_position.objcell_id & 0xFFFF) >= 0x100)
			result = 25.0;
		else
			result = 100.0;
	}
	else if ((this->m_position.objcell_id & 0xFFFF) >= 0x100)
	{
		result = 20.0;
	}
	else
	{
		result = 100.0;
	}
	return result;
	*/

	if ((m_Position.objcell_id & 0xFFFF) >= 0x100)
	{
		return 20.0;
	}

	return 100.0;
}
