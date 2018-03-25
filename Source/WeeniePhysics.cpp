
#include "StdAfx.h"
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

#if 0
const float CWeenieObject::DEFAULT_FRICTION = 0.95f;
const float CWeenieObject::DEFAULT_TRANSLUCENCY = 0.0f;
const float CWeenieObject::DEFAULT_ELASTICITY = 0.05f;

double max_update_fps = 30.0;
double min_update_delay = 1.0 / max_update_fps;

void CWeenieObject::InitDefaultPhysics()
{
	m_PhysicsState = PhysicsState::EDGE_SLIDE_PS | PhysicsState::LIGHTING_ON_PS | PhysicsState::GRAVITY_PS | PhysicsState::REPORT_COLLISIONS_PS;
	weenie_obj = this;
}

void CWeenieObject::InitPhysicsObj()
{
	if (!InitObjectBegin(m_dwGUID, FALSE))
		goto bad_object;

	if (!InitPartArrayObject(m_dwModel, TRUE))
		goto bad_object;

	InitObjectEnd();
	return;

bad_object:
	LOG(Temp, Warning, "Failed to initialize physics obj!\n");
}

void CWeenieObject::CleanupPhysics()
{
	SafeDelete(movement_manager);
}

void CWeenieObject::MakeMovementManager(BOOL init_motion)
{
	if (!movement_manager)
	{
		movement_manager = MovementManager::Create(this, this);

		if (init_motion)
		{
			movement_manager->EnterDefaultState();
		}

		set_active(TRUE);
	}
}

BOOL CWeenieObject::set_active(BOOL active)
{
	if (active)
	{
		if (m_PhysicsState & STATIC_PS)
			return FALSE;

		if (!(transient_state & ACTIVE_TS))
			update_time = g_pGlobals->Time();

		transient_state |= ((DWORD)ACTIVE_TS);
	}
	else
	{
		transient_state &= ~((DWORD)ACTIVE_TS);
	}

	return TRUE;
}

void CWeenieObject::clear_target()
{
	UNFINISHED();
}

void CWeenieObject::unstick_from_object()
{
	UNFINISHED();
}

void CWeenieObject::stick_to_object(DWORD target)
{
	UNFINISHED();
}

void CWeenieObject::interrupt_current_movement()
{
	UNFINISHED();
}

void CWeenieObject::RemoveLinkAnimations()
{
	UNFINISHED_UNSAFE();
}

float CWeenieObject::get_heading()
{
	UNFINISHED();
}

void CWeenieObject::UpdateChild(CPhysicsObj *child_obj, unsigned int part_index, Frame *child_frame)
{
	UNFINISHED();
}

void CWeenieObject::UpdateChildrenInternal()
{
	if (part_array && children)
	{
		UNFINISHED() /* Put back in once these types are complete:
		for (DWORD i = 0; i < children->m_ChildCount; i++)
			UpdateChild(children->m_Objects.array_data[i], children->m_14.array_data[i], &children->m_Frames.array_data[i]);
			*/
	}
}

void CWeenieObject::set_frame(Frame &frame)
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

	m_Position.m_Frame = i_frame;

	if (!(m_PhysicsState & PARTICLE_EMITTER_PS))
	{
		if (part_array)
		{
			UNFINISHED();
			// part_array->SetFrame(frame);
		}
	}

	UpdateChildrenInternal();
}

void CWeenieObject::set_heading(float degrees, int send_event)
{
	Frame newFrame = m_Position.m_Frame;
	newFrame.set_heading(degrees);
	set_frame(newFrame);

	UNFINISHED();
}

BOOL CWeenieObject::IsInterpolating()
{
	UNFINISHED();
	return FALSE;
}

BOOL CWeenieObject::motions_pending()
{
	UNFINISHED();
	return FALSE;
}

DWORD CWeenieObject::StopInterpretedMotion(DWORD motion, class MovementParameters *params)
{
	UNFINISHED_UNSAFE();
	return 0;
}

DWORD CWeenieObject::DoInterpretedMotion(DWORD motion, class MovementParameters *params)
{
	UNFINISHED_UNSAFE();
	return 0;
}

BOOL CWeenieObject::movement_is_autonomous()
{
	return last_move_was_autonomous;
}

class CMotionInterp *CWeenieObject::get_minterp()
{
	MakeMovementManager(TRUE);

	return movement_manager->get_minterp();
}

void CWeenieObject::InitializeMotionTables()
{
	// UNFINISHED();
}

void CWeenieObject::StopCompletely(int send_event)
{
	UNFINISHED();
}

void CWeenieObject::set_local_velocity(const Vector &new_velocity, int send_event)
{
	UNFINISHED();
}

void CWeenieObject::set_on_walkable(BOOL is_on_walkable)
{
	UNFINISHED();
}

void CWeenieObject::StopCompletely_Internal()
{
	UNFINISHED();
}

void CWeenieObject::CheckForCompletedMotions()
{
	UNFINISHED();
}

bool CWeenieObject::IsFullyConstrained()
{
	UNFINISHED();
}

void CPhysicsObj::leave_world()
{
	report_collision_end(TRUE);

	UNFINISHED();

	/*
	if (obj_maint)
	{
		obj_maint->RemoveFromLostCell();
		obj_maint->RemoveObjectToBeDestroyed(m_IID);
	}
	*/

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


void CPhysicsObj::remove_shadows_from_cells()
{
	UNFINISHED();
}

void CPhysicsObj::update_object()
{
	UNFINISHED();
}

void CPhysicsObj::calc_cross_cells_static()
{
	UNFINISHED();
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
}

CPhysicsObj *CPhysicsObj::makeObject(DWORD data_did, DWORD object_iid, BOOL bDynamic)
{
	CPhysicsObj *pObject = new CPhysicsObj();

	if (!pObject)
		return NULL;

	if (!pObject->InitObjectBegin(object_iid, bDynamic))
		goto bad_object;

	if (!pObject->InitPartArrayObject(data_did, TRUE))
		goto bad_object;

	pObject->InitObjectEnd(); //  SetPlacementFrameInternal(0x65);

	return pObject;

bad_object:

	delete pObject;
	return NULL;
}

void CPhysicsObj::add_anim_hook(CAnimHook *pHook)
{
	// UNFINISHED
}

void CPhysicsObj::InitDefaults(CSetup *pSetup)
{
	if (pSetup->m_DefaultScript)
	{
		// DEBUGOUT("Playing default script %08X\r\n", pSetup->m_DefaultScript);
		play_script_internal(pSetup->m_DefaultScript);
	}

	if (pSetup->m_DefaultMotionTable)
		SetMotionTableID(pSetup->m_DefaultMotionTable);

	if (pSetup->m_DefaultSoundTable)
		set_stable_id(pSetup->m_DefaultSoundTable);

	if (pSetup->m_DefaultScriptTable)
		set_phstable_id(pSetup->m_DefaultScriptTable);

	if (m_PhysicsState & 1)
	{
		if (pSetup->m_DefaultAnim)
			m_PhysicsState |= 0x40000;

		if (pSetup->m_DefaultScript)
			m_PhysicsState |= 0x80000;

		if (m_PhysicsState & 0xC0000)
			CPhysics::AddStaticAnimatingObject(this);
	}
}

void CWeenieObject::report_collision_end(const int force_end)
{
	UNFINISHED()
}

void CPhysicsObj::calc_acceleration()
{
	if (!(transient_state & 1) && !(transient_state & 2) && !(m_PhysicsState & 0x800000))
	{
		m_Acceleration = Vector(0, 0, 0);
		m_Omega = Vector(0, 0, 0);
	}
	else if (m_PhysicsState & 0x400)
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
	m_Position.m_LandCell = CellID;

	if (!(m_PhysicsState & 0x1000))
	{
		if (part_array)
			part_array->SetCellID(CellID);
	}
}

void CPhysicsObj::leave_cell(BOOL Unknown)
{
	if (!cell)
		return;

	cell->remove_object(this);

	if (children)
	{
		for (DWORD i = 0; i < children->m_ChildCount; i++)
			children->m_Objects.array_data[i]->leave_cell(Unknown);
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
		for (DWORD i = 0; i < children->m_ChildCount; i++)
			children->m_Objects.array_data[i]->enter_cell(pCell);
	}

	m_Position.m_LandCell = pCell->m_Key;
	set_cell_id(pCell->m_Key);

	cell = pCell;

	if (part_array)
		part_array->AddLightsToCell(pCell);
}

void CPhysicsObj::set_initial_frame(Frame *Pos)
{
	m_Position.m_Frame = *Pos;

	if (!(m_PhysicsState & 0x1000))
	{
		if (part_array)
			part_array->SetFrame(&m_Position.m_Frame);
	}

	UpdateChildrenInternal();
}


BOOL CPhysicsObj::InitObjectBegin(DWORD object_iid, BOOL bDynamic)
{
	m_dwGUID = object_iid;

	if (bDynamic)
		m_PhysicsState &= STATIC_PS;
	else
		m_PhysicsState |= STATIC_PS;

	transient_state &= ~(0x00000080UL);
	update_time = Time::GetTimeCurrent();

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
		m_PhysicsState |= 0x00000004UL;
		transient_state &= ~(0x00000100UL);

		SetTranslucencyInternal(0.25f);
		m_PhysicsState |= 0x00000010UL;
	}

	return TRUE;
}

BOOL CPhysicsObj::CacheHasPhysicsBSP()
{
	if (part_array)
	{
		if (part_array->CacheHasPhysicsBSP())
		{
			m_PhysicsState |= (0x00010000UL);
			return TRUE;
		}
	}

	m_PhysicsState &= ~(0x00010000UL);
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

BOOL CPhysicsObj::SetPlacementFrameInternal(DWORD PlacementID)
{
	BOOL result = FALSE;

	if (part_array)
	{
		result = part_array->SetPlacementFrame(PlacementID);
	}

	if (!(m_PhysicsState & 0x1000))
	{
		if (part_array)
		{
			part_array->SetFrame(&m_Position.m_Frame);
		}
	}

	return result;
}

BOOL CPhysicsObj::play_script_internal(DWORD ScriptID)
{
#if HAS_RENDER
	if (!ScriptID)
		return FALSE;

	if (!m_ScriptManager)
	{
		m_ScriptManager = new ScriptManager(this);
	}

	if (m_ScriptManager)
		return m_ScriptManager->AddScript(ScriptID);
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
		m_PhysicsState |= ~(0x00000020UL);

		if (part_array)
			part_array->SetNoDrawInternal(1);
	}
	else
	{
		m_PhysicsState &= ~(0x00000020UL);

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
		// m_ShadowObjs shit
	}

	return TRUE;
}


void CPhysicsObj::animate_static_object()
{
	if (!cell)
		return;

	double CurrentTime = Time::GetTimeCurrent(); // Timer__m_timeCurrent

												 // Update physics timer.
	PhysicsTimer::curr_time = CurrentTime;

	// Time between last update.
	double FrameTime = CurrentTime - update_time;

	if (FrameTime < F_EPSILON)
	{
		update_time = CurrentTime;
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
	{
		if (part_array)
		{
			if (m_PhysicsState & 0x40000)
			{
				part_array->Update((float) FrameTime, NULL);
				m_Position.m_Frame.grotate(m_Omega);
				UpdatePartsInternal();
				UpdateChildrenInternal();
			}

			if (m_PhysicsState & 0x80000)
			{
				if (script_manager)
					script_manager->UpdateScripts();
			}

#if HAS_RENDER
			if (m_ParticleManager)
				m_ParticleManager->UpdateParticles();
#endif

			process_hooks();
		}

		update_time = CurrentTime;

		/*
		// where did this come from??

		Frame NewPosition;
		UpdatePositionInternal(FrameTime, &NewPosition);

		set_initial_frame(&NewPosition);

		if (m_ParticleManager)
		m_ParticleManager->UpdateParticles();

		if (m_ScriptManager)
		m_ScriptManager->UpdateScripts();

		update_time = CurrentTime;
		*/
	}
}

void CPhysicsObj::UpdatePartsInternal()
{
	if (!(m_PhysicsState & 0x1000))
	{
		if (part_array)
			part_array->SetFrame(&m_Position.m_Frame);
	}
}

void CPhysicsObj::process_hooks()
{
	// UNFINISHED
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


int CPhysicsObj::check_collisions(CPhysicsObj *object)
{
	if (m_PhysicsState & STATIC_PS)
		return FALSE;

	CTransition *transit = CTransition::makeTransition();

	if (transit)
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
	if (m_PhysicsState & 0x400000)
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

	if (transient_state & CONTACT_TS &&
		(m_velocityVector.z * contact_plane.m_normal.z +
			m_velocityVector.y * contact_plane.m_normal.y +
			m_velocityVector.x * contact_plane.m_normal.x) > 0.00019999999)
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
		|| !weenie_obj->IsPlayer()
		|| !(transition->object_info.state & OBJECTINFO::IS_PLAYER)
		|| !(transition->object_info.state & OBJECTINFO::IS_IMPENETRABLE)
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
	return (float) PhysicsGlobals::floor_z;
}

const float max_velocity = 50.0f;

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
			update_time = Time::GetTimeCurrent();
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
#endif