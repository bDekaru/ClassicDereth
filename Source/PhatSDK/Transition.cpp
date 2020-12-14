
#include <StdAfx.h>
#include "Transition.h"
#include "PhysicsObj.h"
#include "LandDefs.h"
#include "Polygon.h"

OBJECTINFO::OBJECTINFO()
{
}

SPHEREPATH::SPHEREPATH()
{
	local_sphere = new CSphere[2];
	global_sphere = new CSphere[2];
	localspace_sphere = new CSphere[2];
	localspace_curr_center = new Vector[2];
	global_curr_center = new Vector[2];

	init();
}

SPHEREPATH::~SPHEREPATH()
{
	SafeDeleteArray(local_sphere);
	SafeDeleteArray(global_sphere);
	SafeDeleteArray(localspace_sphere);
	delete[] localspace_curr_center;
	delete[] global_curr_center;
}

void SPHEREPATH::init()
{
	num_sphere = 0;
	begin_cell = NULL;
	begin_pos = NULL;
	curr_cell = NULL;
	check_cell = NULL;
	insert_type = TRANSITION_INSERT;
	step_down = 0;
	step_up = 0;
	collide = 0;
	hits_interior_cell = 0;
	bldg_check = 0;
	obstruction_ethereal = 0;
	backup_cell = NULL;
	walkable_allowance = 0.0f;
	walkable = NULL;
	check_walkable = 0;
	cell_array_valid = 0;
	neg_step_up = 0;
	neg_poly_hit = 0;
	placement_allows_sliding = 1;
}

COLLISIONINFO::COLLISIONINFO() : collide_object(80)
{
}

void COLLISIONINFO::init()
{
	last_known_contact_plane_valid = 0;
	contact_plane_valid = 0;
	sliding_normal_valid = 0;
	collision_normal_valid = 0;
	num_collide_object = 0;
	last_collided_object = 0;
	collided_with_environment = 0;
	contact_plane_cell_id = 0;
	frames_stationary_fall = 0;
}

void COLLISIONINFO::add_object(CPhysicsObj *object, TransitionState ts)
{
	for (uint32_t i = 0; i < num_collide_object; i++)
	{
		if (object == collide_object.data[i])
			return;
	}

	collide_object.ensure_space(num_collide_object + 1, 10);
	collide_object.data[num_collide_object++] = object;

	if (ts != OK_TS)
		last_collided_object = object;
}

CELLARRAY::CELLARRAY() : cells(8)
{
}

void CELLARRAY::remove_cell(int index)
{
	cells.data[index] = cells.data[--num_cells];
}

void CELLARRAY::add_cell(const unsigned int cell_id, CObjCell *cell)
{
	for (uint32_t i = 0; i < num_cells; i++)
	{
		if (cells.data[i].cell_id == cell_id)
			return;
	}

	cells.ensure_space(num_cells + 1, 8);	
	cells.data[num_cells].cell_id = cell_id;
	cells.data[num_cells++].cell = cell;
}

int CTransition::transition_level = 0;

CTransition *CTransition::makeTransition()
{
	static CTransition transit[10];

	if (transition_level >= 10)
		return NULL;

	CTransition *currentTransition = &transit[transition_level];

	currentTransition->init();
	transition_level++;

	return currentTransition;
}

void CTransition::cleanupTransition(CTransition *)
{
	transition_level--;
}

CTransition::CTransition()
{
	object_info.object = NULL;
	object_info.state = 0;
	object_info.targetID = 0;

	init();
}

void CTransition::init()
{
	object_info.object = NULL;
	object_info.state = 0;
	object_info.targetID = 0;

	sphere_path.init();

	collision_info.last_known_contact_plane_valid = 0;
	collision_info.contact_plane_valid = 0;
	collision_info.sliding_normal_valid = 0;
	collision_info.collision_normal_valid = 0;
	collision_info.num_collide_object = 0;
	collision_info.last_collided_object = NULL;
	collision_info.collided_with_environment = 0;
	collision_info.contact_plane_cell_id = 0;
	collision_info.frames_stationary_fall = 0;
	cell_array.num_cells = 0;
	cell_array.added_outside = 0;
	cell_array.do_not_load_cells = 0;
}

int OBJECTINFO::missile_ignore(CPhysicsObj *collideobject)
{
	int result;

	if (collideobject->m_PhysicsState & MISSILE_PS)
	{
		result = 1;
	}
	else
	{
		result = 0;
		if (object->m_PhysicsState & MISSILE_PS)
		{
			if (collideobject->id != targetID)
			{
				if ((collideobject->m_PhysicsState & ETHEREAL_PS) && collideobject->weenie_obj
					|| targetID && collideobject->weenie_obj && collideobject->weenie_obj->IsCreature())
				{
					result = 1;
				}
			}
		}
	}
	return result;
}

void OBJECTINFO::init(CPhysicsObj *_object, int object_state)
{
	object = _object;
	state = object_state;
	scale = _object->m_scale;
	step_up_height = _object->GetStepUpHeight();
	step_down_height = _object->GetStepDownHeight();
	ethereal = object->m_PhysicsState & ETHEREAL_PS;
	step_down = ~(uint8_t)(object->m_PhysicsState >> 6) & 1; // if not a missile MISSILE_PS
	CWeenieObject *pWeenie = object->weenie_obj;
	if (pWeenie)
	{
		if (pWeenie->IsImpenetrable()) // 28 IsImpenetrable
			state |= IS_IMPENETRABLE;
		if (pWeenie->_IsPlayer()) // 10 IsPlayer
			state |= IS_PLAYER;
		if (pWeenie->IsPK()) // 20 IsPK
			state |= IS_PK;
		if (pWeenie->IsPKLite()) // 24 IsPKLite
			state |= IS_PKLITE;

		targetID = pWeenie->GetPhysicsTargetID();
	}
}

void CTransition::init_object(CPhysicsObj *object, int object_state)
{
	object_info.init(object, object_state);
}

BOOL SPHEREPATH::is_walkable_allowable(float zval)
{
	return zval > (double)walkable_allowance;
}

void SPHEREPATH::init_sphere(unsigned int _num_sphere, CSphere *_sphere, const float _scale)
{
	if (_num_sphere <= 2)
		num_sphere = _num_sphere;
	else
		num_sphere = 2;

	uint32_t index = 0;
	while (index < num_sphere)
	{
		local_sphere[index].center = _sphere[index].center * _scale;
		local_sphere[index].radius = _sphere[index].radius * _scale;
		index++;
	}

	local_low_point.x = local_sphere[0].center.x;
	local_low_point.y = local_sphere[0].center.y;
	local_low_point.z = (local_sphere[0].center.z - local_sphere[0].radius);
}

void CTransition::init_sphere(unsigned int num_sphere, CSphere *sphere, const float scale)
{
	sphere_path.init_sphere(num_sphere, sphere, scale);
}

void SPHEREPATH::cache_global_curr_center()
{
	for (uint32_t i = 0; i < num_sphere; i++)
	{
		global_curr_center[i] = curr_pos.localtoglobal(local_sphere[i].center);
	}
}

void SPHEREPATH::cache_global_sphere(Vector *offset)
{
	if (offset)
	{
		for (uint32_t i = 0; i < num_sphere; i++)
		{
			global_sphere[i].center += *offset;
		}

		global_low_point += *offset;
	}
	else
	{
		for (uint32_t i = 0; i < num_sphere; i++)
		{
			global_sphere[i].radius = local_sphere[i].radius;
			global_sphere[i].center = check_pos.localtoglobal(local_sphere[i].center);
		}

		global_low_point = check_pos.localtoglobal(local_low_point);
	}
}

void SPHEREPATH::init_path(CObjCell *_begin_cell, Position *_begin_pos, Position *_end_pos)
{
	begin_cell = _begin_cell;
	begin_pos = _begin_pos;
	end_pos = _end_pos;
	if (_begin_pos)
	{
		curr_pos = *_begin_pos;
		curr_cell = _begin_cell;
		cache_global_curr_center();

		insert_type = TRANSITION_INSERT;
	}
	else
	{
		curr_pos = *_end_pos;
		curr_cell = _begin_cell;
		cache_global_curr_center();

		insert_type = PLACEMENT_INSERT;
	}
}

void CTransition::init_path(CObjCell *begin_cell, Position *begin_pos, Position *end_pos)
{
	sphere_path.init_path(begin_cell, begin_pos, end_pos);
}

void CTransition::init_contact_plane(unsigned int cell_id, Plane *plane, int is_water)
{
	collision_info.last_known_contact_plane_valid = 1;
	collision_info.last_known_contact_plane = *plane;
	collision_info.last_known_contact_plane_is_water = is_water;
	collision_info.last_known_contact_plane_cell_id = cell_id;
	collision_info.contact_plane_valid = 1;
	collision_info.contact_plane = *plane;
	collision_info.contact_plane_is_water = is_water;
	collision_info.contact_plane_cell_id = cell_id;
}

void CTransition::init_last_known_contact_plane(unsigned int cell_id, Plane *plane, int is_water)
{
	collision_info.last_known_contact_plane_valid = 1;
	collision_info.last_known_contact_plane = *plane;
	collision_info.last_known_contact_plane_is_water = is_water;
	collision_info.contact_plane_cell_id = cell_id;
}

void CTransition::init_sliding_normal(Vector *normal)
{
	collision_info.sliding_normal_valid = 1;
	collision_info.sliding_normal = *normal;

	if (collision_info.sliding_normal.normalize_check_small())
	{
		collision_info.sliding_normal = Vector(0, 0, 0);
	}
}

BOOL CTransition::check_collisions(CPhysicsObj *object)
{
	sphere_path.insert_type = SPHEREPATH::PLACEMENT_INSERT;
	sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);

	return object->FindObjCollisions(this) != 1;
}

void COLLISIONINFO::set_contact_plane(Plane *plane, int is_water)
{
	contact_plane_valid = 1;
	contact_plane = *plane;
	contact_plane_is_water = is_water;
}

Vector SPHEREPATH::get_curr_pos_check_pos_block_offset()
{
	Vector result = LandDefs::get_block_offset(curr_pos.objcell_id, check_pos.objcell_id);
	return result;
}

void SPHEREPATH::add_offset_to_check_pos(Vector *offset)
{
	cell_array_valid = 0;
	check_pos.frame.m_origin += *offset;

	cache_global_sphere(offset);
}

void SPHEREPATH::add_offset_to_check_pos(Vector *offset, const float radius)
{
	cell_array_valid = 0;
	check_pos.frame.m_origin += *offset;

	cache_global_sphere(offset);
}

void COLLISIONINFO::set_collision_normal(const Vector &normal)
{
	collision_normal_valid = 1;
	collision_normal = normal;

	if (collision_normal.normalize_check_small())
	{
		collision_normal = Vector(0, 0, 0);
	}
}

void SPHEREPATH::set_collide(const Vector &collision_normal)
{
	collide = 1;
	backup_cell = check_cell;
	backup_check_pos = check_pos;

	step_up_normal = collision_normal;
	walk_interp = 1.0f;
}

TransitionState SPHEREPATH::step_up_slide(OBJECTINFO *object, COLLISIONINFO *collisions)
{
	collisions->contact_plane_valid = 0;
	collisions->contact_plane_is_water = 0;
	step_up = 0;

	return global_sphere->slide_sphere(this, collisions, &step_up_normal, global_curr_center);
}

float OBJECTINFO::get_walkable_z()
{
	return object->get_walkable_z();
}

const float z_for_landing = 0.0871557f;

BOOL CTransition::step_up(Vector *collision_normal)
{
	collision_info.contact_plane_valid = 0;
	collision_info.contact_plane_is_water = 0;
	sphere_path.step_up = 1;
	sphere_path.step_up_normal = *collision_normal;

	float step_down_ht = 0.039999999f;

	float zLandingValue = z_for_landing;

	if (object_info.state & OBJECTINFO::ON_WALKABLE_OI)
	{
		zLandingValue = object_info.get_walkable_z();
		step_down_ht = object_info.step_up_height;
	}

	sphere_path.walkable_allowance = zLandingValue;
	sphere_path.backup_cell = sphere_path.check_cell;
	sphere_path.backup_check_pos = sphere_path.check_pos;

	BOOL bStepDown = step_down(step_down_ht, zLandingValue);

	sphere_path.step_up = 0;
	sphere_path.walkable = 0;

	if (bStepDown)
	{
		return TRUE;
	}
	else
	{
		sphere_path.restore_check_pos();
		return FALSE;
	}
}

void SPHEREPATH::restore_check_pos()
{
	set_check_pos(&backup_check_pos, backup_cell);
}

BOOL CTransition::step_down(float step_down_ht, float z_val)
{
	sphere_path.neg_poly_hit = 0;
	sphere_path.step_down = 1;

	sphere_path.step_down_amt = step_down_ht;
	sphere_path.walk_interp = 1.0f;

	if (!sphere_path.step_up)
	{
		sphere_path.cell_array_valid = 0;

		Vector offset;

		offset.x = 0;
		offset.y = 0;
		offset.z = -step_down_ht;

		sphere_path.check_pos.frame.m_origin.x = sphere_path.check_pos.frame.m_origin.x;
		sphere_path.check_pos.frame.m_origin.y = sphere_path.check_pos.frame.m_origin.y;
		sphere_path.check_pos.frame.m_origin.z = sphere_path.check_pos.frame.m_origin.z - step_down_ht;

		sphere_path.cache_global_sphere(&offset);
	}

	TransitionState ts = transitional_insert(5);

	sphere_path.step_down = 0;

	if (ts == 1
		&& collision_info.contact_plane_valid
		&& collision_info.contact_plane.m_normal.z >= (double)z_val
		&& (!(object_info.state & OBJECTINFO::EDGE_SLIDE)
			|| sphere_path.step_up
			|| check_walkable(z_val)))
	{
		sphere_path.backup = sphere_path.insert_type;
		sphere_path.insert_type = SPHEREPATH::PLACEMENT_INSERT;

		ts = transitional_insert(1);

		sphere_path.insert_type = sphere_path.backup;
		return (ts == TransitionState::OK_TS);
	}
	else
	{
		return FALSE;
	}
}

int CTransition::check_walkable(float z_chk)
{
	if (!(object_info.state & OBJECTINFO::ON_WALKABLE_OI) || sphere_path.check_walkables())
		return 1;

	Position backup_check_pos(sphere_path.check_pos);
	CObjCell *backup_check_cell = sphere_path.check_cell;

	float stepHeight = object_info.step_down_height;

	sphere_path.walkable_allowance = z_chk;
	sphere_path.check_walkable = 1;

	if (sphere_path.num_sphere < 2)
	{
		CSphere *v8 = sphere_path.global_sphere;
		if (stepHeight > v8->radius + v8->radius)
			stepHeight = v8->radius * 0.5f;
	}
	if (stepHeight > (sphere_path.global_sphere->radius + sphere_path.global_sphere->radius))
		stepHeight = stepHeight * 0.5f;

	Vector offset(0, 0, -stepHeight);
	sphere_path.add_offset_to_check_pos(&offset);

	TransitionState ts = transitional_insert(1);

	sphere_path.check_walkable = 0;
	sphere_path.set_check_pos(&backup_check_pos, backup_check_cell);

	return (ts != TransitionState::OK_TS);
}

int SPHEREPATH::check_walkables()
{
	if (walkable)
	{
		walkable_check_pos.radius = walkable_check_pos.radius * 0.5f;
		return walkable->check_walkable(&walkable_check_pos, &walkable_up);
	}

	return 1;
}

void SPHEREPATH::set_check_pos(Position *p, CObjCell *cell)
{
	check_pos = *p;
	check_cell = cell;
	cell_array_valid = 0;
	cache_global_sphere(0);
}

TransitionState CTransition::transitional_insert(int num_insertion_attempts)
{
	if (!sphere_path.check_cell)
		return TransitionState::OK_TS;

	if (num_insertion_attempts <= 0)
		return TransitionState::INVALID_TS;

	TransitionState ts = INVALID_TS;

	for (int i = 0; i < num_insertion_attempts; i++)
	{
		ts = insert_into_cell(sphere_path.check_cell, num_insertion_attempts);

		switch (ts)
		{
		case TransitionState::OK_TS:
			ts = check_other_cells(sphere_path.check_cell);

			if (ts != TransitionState::OK_TS)
				sphere_path.neg_poly_hit = 0;

			if (ts == TransitionState::COLLIDED_TS)
				return ts;

			break;
		case TransitionState::COLLIDED_TS:
			return ts;

		case TransitionState::ADJUSTED_TS:
			sphere_path.neg_poly_hit = 0;
			break;

		case TransitionState::SLID_TS:
			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			sphere_path.neg_poly_hit = 0;
			break;
		}

		if (ts == OK_TS)
		{
			if (!sphere_path.collide)
			{
				if (sphere_path.neg_poly_hit && !sphere_path.step_down && !sphere_path.step_up)
				{
					sphere_path.neg_poly_hit = 0;

					if (sphere_path.neg_step_up)
					{
						if (!step_up(&sphere_path.neg_collision_normal))
						{
							ts = sphere_path.step_up_slide(&object_info, &collision_info);
						}
					}
					else
					{
						ts = sphere_path.global_sphere->slide_sphere(&sphere_path, &collision_info, &sphere_path.neg_collision_normal, sphere_path.global_curr_center);
					}
				}
				else
				{
					if (collision_info.contact_plane_valid || !(object_info.state & OBJECTINFO::CONTACT_OI) || sphere_path.step_down || !sphere_path.check_cell || !object_info.step_down)
					{
						return TransitionState::OK_TS;
					}

					float z_val = z_for_landing;
					float step_down_ht = 0.04f;

					if (object_info.state & OBJECTINFO::ON_WALKABLE_OI)
					{
						z_val = object_info.get_walkable_z();
						step_down_ht = object_info.step_down_height;
					}

					sphere_path.walkable_allowance = z_val;
					sphere_path.save_check_pos();

					if (sphere_path.num_sphere < 2)
					{
						CSphere *psphere = sphere_path.global_sphere;
						if ((psphere->radius + psphere->radius) < step_down_ht)
							step_down_ht = psphere->radius * 0.5f;
					}

					if ((sphere_path.global_sphere->radius + sphere_path.global_sphere->radius) < step_down_ht)
					{
						step_down_ht *= 0.5f;
						if (step_down(step_down_ht, z_val) || step_down(step_down_ht, z_val))
						{
							sphere_path.walkable = 0;
							return TransitionState::OK_TS;
						}
					}

					if (step_down(step_down_ht, z_val))
					{
						sphere_path.walkable = 0;
						return TransitionState::OK_TS;
					}

					if (edge_slide(&ts, step_down_ht, z_val))
					{
						return ts;
					}
				}
			}
			else
			{
				BOOL someBool = FALSE;

				sphere_path.collide = 0;

				if (collision_info.contact_plane_valid && check_walkable(z_for_landing))
				{
					sphere_path.backup = sphere_path.insert_type;
					sphere_path.insert_type = SPHEREPATH::PLACEMENT_INSERT;
					ts = (TransitionState)transitional_insert(num_insertion_attempts);

					sphere_path.insert_type = sphere_path.backup;

					if (ts != TransitionState::OK_TS)
					{
						ts = TransitionState::OK_TS;
						someBool = TRUE;
					}
				}
				else
				{
					someBool = TRUE;
				}

				sphere_path.walkable = NULL;

				if (!someBool)
				{
					return ts;
				}

				sphere_path.restore_check_pos();

				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;

				if (collision_info.last_known_contact_plane_valid)
				{
					object_info.kill_velocity();
					collision_info.last_known_contact_plane_valid = 0;
				}
				else
				{
					collision_info.set_collision_normal(sphere_path.step_up_normal);
				}
				return TransitionState::COLLIDED_TS;
			}
		}
	}

	return ts;
}

void OBJECTINFO::kill_velocity()
{
	object->set_velocity(Vector(0, 0, 0), 0);
}

void SPHEREPATH::save_check_pos()
{
	backup_cell = check_cell;
	backup_check_pos = check_pos;
}

int CTransition::find_valid_position()
{
	if (sphere_path.insert_type == SPHEREPATH::TRANSITION_INSERT)
		return find_transitional_position();
	else
		return find_placement_position();
}

void CTransition::calc_num_steps(Vector *offset, Vector *offset_per_step, unsigned int *num_steps)
{
	if (!sphere_path.begin_pos)
	{
		*offset = Vector(0, 0, 0);
		*offset_per_step = Vector(0, 0, 0);
		*num_steps = 1;
		return;
	}

	Vector result = sphere_path.begin_pos->get_offset(*sphere_path.end_pos);

	*offset = result;
	float offsetMag = offset->magnitude();
	if (object_info.state & OBJECTINFO::IS_VIEWER_OI)
	{
		if (offsetMag <= F_EPSILON)
		{
			*offset_per_step = Vector(0, 0, 0);
			*num_steps = 0;
		}
		else
		{
			float fNumSteps = offsetMag / sphere_path.local_sphere->radius;

			*offset_per_step = *offset * (1.0 / fNumSteps);
			*num_steps = (unsigned int)floor(fNumSteps) + 1;
		}
	}
	else
	{
		float fNumSteps = offsetMag / sphere_path.local_sphere->radius;
		if (fNumSteps > 1.0)
		{
			fNumSteps = ceil(fNumSteps);
			*offset_per_step = *offset * (1.0 / fNumSteps);
			*num_steps = (unsigned int)fNumSteps;
		}
		else if (offset->x != 0.0 || offset->y != 0.0 || offset->z != 0.0)
		{
			*offset_per_step = *offset;
			*num_steps = 1;
		}
		else
		{
			*offset_per_step = Vector(0, 0, 0);
			*num_steps = 0;
		}
	}
}

void COLLISIONINFO::set_sliding_normal(Vector *normal)
{
	sliding_normal_valid = 1;
	sliding_normal = Vector(normal->x, normal->y, 0);

	if (sliding_normal.normalize_check_small())
	{
		sliding_normal = Vector(0, 0, 0);
	}
}

void CTransition::build_cell_array(CObjCell **new_cell_p)
{
	sphere_path.cell_array_valid = 1;
	sphere_path.hits_interior_cell = 0;
	CObjCell::find_cell_list(&cell_array, new_cell_p, &sphere_path);
}

BOOL OBJECTINFO::is_valid_walkable(Vector *normal)
{
	return object->is_valid_walkable(normal);
}

TransitionState OBJECTINFO::validate_walkable(CSphere *check_pos, Plane *contact_plane, const int is_water, const float water_depth, SPHEREPATH *path, COLLISIONINFO *collisions, unsigned int land_cell_id)
{
	TransitionState result;
	TransitionState ts = OK_TS;

	if (state & 4)
	{
		float sphereDist = contact_plane->dot_product(check_pos->center) - check_pos->radius;
		if ((sphereDist > -F_EPSILON) && path->begin_pos != 0 && (path->begin_pos->objcell_id & 0xFFFF) >= 0x100)
			return OK_TS;

		Vector offset = check_pos->center - *path->global_curr_center;
		float someDp = sphereDist / offset.dot_product(contact_plane->m_normal);
		if ((someDp <= 0.0 || someDp > 1.0) && path->begin_pos != 0 && (path->begin_pos->objcell_id & 0xFFFF) >= 0x100)
			return OK_TS;
		
		offset *= -someDp;
		path->add_offset_to_check_pos(&offset);
		collisions->set_collision_normal(contact_plane->m_normal);
		collisions->collided_with_environment = 1;
		return ADJUSTED_TS;		
	}
	else
	{
		float someDist = contact_plane->dot_product(check_pos->center - Vector(0, 0, check_pos->radius)) + water_depth;

		if (someDist >= -F_EPSILON)
		{
			if (someDist > F_EPSILON)
				return OK_TS;

			if (path->step_down || !(state & 2) || CPhysicsObj::is_valid_walkable(&contact_plane->m_normal))
			{
				collisions->set_contact_plane(contact_plane, is_water);
				collisions->contact_plane_cell_id = land_cell_id;
			}
			if (!(state & 1) && !path->step_down)
			{
				collisions->set_collision_normal(contact_plane->m_normal);
				collisions->collided_with_environment = 1;
			}
			ts = OK_TS;
		}
		else
		{
			if (path->check_walkable)
				return COLLIDED_TS;
			
			float someVal = someDist / contact_plane->m_normal.z;

			if (path->step_down || !(state & 2) || CPhysicsObj::is_valid_walkable(&contact_plane->m_normal))
			{
				collisions->set_contact_plane(contact_plane, is_water);
				collisions->contact_plane_cell_id = land_cell_id;
				if (path->step_down)
				{
					double v21 = (1.0 - -1.0 / (path->step_down_amt * path->walk_interp) * someVal) * path->walk_interp;

					if (v21 >= path->walk_interp || v21 < -0.1)
						return COLLIDED_TS;

					path->walk_interp = v21;
				}

				Vector offset(0, 0, -someVal);
				path->add_offset_to_check_pos(&offset);

				ts = ADJUSTED_TS;
			}

			if (!(state & 1) && !path->step_down)
			{
				collisions->set_collision_normal(contact_plane->m_normal);
				result = ts;
				collisions->collided_with_environment = 1;
				return result;
			}
		}
		result = ts;
	}
	return result;
}

TransitionState CTransition::validate_transition(TransitionState ts, int *redo)
{
	Plane contact_plane;

	*redo = 0;

	int redoa = 1;

	if (ts != 1 || (sphere_path.check_pos == sphere_path.curr_pos))
	{
		redoa = 0;
		if (ts != 1)
		{
			if (ts > 1 && ts <= 4)
			{
				if (collision_info.last_known_contact_plane_valid)
				{
					object_info.kill_velocity();

					if ((sphere_path.global_sphere->radius + F_EPSILON) > fabs(collision_info.last_known_contact_plane.dot_product(*sphere_path.global_curr_center)))
					{
						collision_info.set_contact_plane(
							&collision_info.last_known_contact_plane,
							collision_info.last_known_contact_plane_is_water);

						collision_info.contact_plane_cell_id = collision_info.last_known_contact_plane_cell_id;
						if (object_info.state & OBJECTINFO::ON_WALKABLE_OI)
							redoa = 1;
					}
				}

				if (!collision_info.collision_normal_valid)
				{
					contact_plane.m_normal = Vector(0, 0, 1.0f);
					collision_info.set_collision_normal(contact_plane.m_normal);
				}

				sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);
				build_cell_array(NULL);
				ts = OK_TS;
			}
		}
		else
		{
			//assert(sphere_path.check_cell != NULL);

			sphere_path.curr_pos = sphere_path.check_pos;
			sphere_path.curr_cell = sphere_path.check_cell;
			sphere_path.cache_global_curr_center();

			sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);
		}
	}
	else
	{
		//assert(sphere_path.check_cell != NULL);

		sphere_path.curr_pos = sphere_path.check_pos;
		sphere_path.curr_cell = sphere_path.check_cell;
		sphere_path.cache_global_curr_center();

		sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);
	}

	if (collision_info.collision_normal_valid)
		collision_info.set_sliding_normal(&collision_info.collision_normal);
	
	if (!(object_info.state & OBJECTINFO::IS_VIEWER_OI))
	{
		if (object_info.object->m_PhysicsState & GRAVITY_PS)
		{
			if (redoa)
			{
				collision_info.frames_stationary_fall = 0;
			}
			else
			{
				if (collision_info.frames_stationary_fall)
				{
					if (collision_info.frames_stationary_fall == 1)
					{
						collision_info.frames_stationary_fall = 2;
					}
					else
					{
						collision_info.frames_stationary_fall = 3;
						contact_plane.m_normal = Vector(0, 0, 1.0f);
						contact_plane.m_dist = sphere_path.global_sphere->radius - sphere_path.global_sphere->center.z;

						collision_info.set_contact_plane(&contact_plane, 0);
						collision_info.contact_plane_cell_id = sphere_path.check_pos.objcell_id;

						if (!(object_info.state & OBJECTINFO::CONTACT_OI))
						{
							collision_info.set_collision_normal(contact_plane.m_normal);
							collision_info.collided_with_environment = 1;
						}
					}
				}
				else
				{
					collision_info.frames_stationary_fall = 1;
				}
			}
		}
	}

	collision_info.last_known_contact_plane_valid = collision_info.contact_plane_valid;
	if (collision_info.contact_plane_valid)
	{
		collision_info.last_known_contact_plane = collision_info.contact_plane;
		collision_info.last_known_contact_plane_cell_id = collision_info.contact_plane_cell_id;
		collision_info.last_known_contact_plane_is_water = collision_info.contact_plane_is_water;
	}

	if (collision_info.contact_plane_valid)
	{
		object_info.state |= OBJECTINFO::CONTACT_OI;

		if (!object_info.is_valid_walkable(&collision_info.contact_plane.m_normal))
			object_info.state &= 0xFFFFFFFD;
		else
			object_info.state |= 2;
	}
	else
	{
		object_info.state &= 0xFFFFFFFC;
	}

	return ts;
}

int CTransition::find_transitional_position()
{
	if (!sphere_path.begin_cell)
		return 0;

	TransitionState ts = OK_TS;

	Vector offset;
	Vector offset_per_step;
	unsigned int num_steps = 0;
	calc_num_steps(&offset, &offset_per_step, &num_steps);

	if (object_info.state & OBJECTINFO::FREE_ROTATE_OI)
		sphere_path.curr_pos.frame.set_rotate(sphere_path.end_pos->frame.m_angles);

	sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);

	int redo;

	if (num_steps <= 0)
	{
		if (!(object_info.state & OBJECTINFO::FREE_ROTATE_OI))
			sphere_path.curr_pos.frame.set_rotate(sphere_path.end_pos->frame.m_angles);

		sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);
		sphere_path.cell_array_valid = 1;
		sphere_path.hits_interior_cell = 0;
		CObjCell::find_cell_list(&cell_array, 0, &sphere_path);
		return 1;
	}
	
	for (uint32_t i = 0; i < num_steps; i++)
	{
		if (object_info.state & OBJECTINFO::IS_VIEWER_OI)
		{
			int lastStep = num_steps - 1;
			if (i == lastStep)
			{
				float offsetMag = offset.magnitude();
				if (offsetMag > F_EPSILON)
				{
					redo = lastStep;
					offset_per_step = (offset * (offsetMag - (sphere_path.local_sphere->radius * lastStep))) / offsetMag;
				}
			}
		}

		sphere_path.global_offset = adjust_offset(&offset_per_step);

		if (!(object_info.state & OBJECTINFO::IS_VIEWER_OI))
		{
			if ((F_EPSILON*F_EPSILON) > sphere_path.global_offset.mag_squared())
			{
				if (i != 0 && ts == OK_TS)
					return TRUE;
				
				return FALSE;
			}
		}

		if (!(object_info.state & OBJECTINFO::FREE_ROTATE_OI))
		{
			redo = i + 1;

			double t = (double)redo / (double)num_steps;
			sphere_path.check_pos.frame.interpolate_rotation(sphere_path.begin_pos->frame, sphere_path.end_pos->frame, t);
		}

		collision_info.sliding_normal_valid = 0;
		collision_info.contact_plane_valid = 0;
		collision_info.contact_plane_is_water = 0;

		if (sphere_path.insert_type)
		{
			ts = validate_placement_transition(transitional_insert(3), &redo);
			
			if (ts == OK_TS)
			{
				return TRUE;
			}

			if (!sphere_path.placement_allows_sliding)
			{
				return FALSE;
			}

			sphere_path.add_offset_to_check_pos(&sphere_path.global_offset);
		}
		else
		{
			sphere_path.cell_array_valid = 0;

			sphere_path.check_pos.frame.m_origin += sphere_path.global_offset;
			sphere_path.cache_global_sphere(&sphere_path.global_offset);

			ts = validate_transition(transitional_insert(3), &redo);

			if (collision_info.frames_stationary_fall)
			{
				break;
			}
		}

		if (collision_info.collision_normal_valid && (object_info.state & OBJECTINFO::PATH_CLIPPED_OI))
		{
			break;
		}
	}

	return (ts == OK_TS) ? TRUE : FALSE;
}

int CTransition::find_placement_position()
{
	sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);

	sphere_path.insert_type = SPHEREPATH::INITIAL_PLACEMENT_INSERT;

	TransitionState ts;

	if (sphere_path.check_cell)
	{
		ts = insert_into_cell(sphere_path.check_cell, 3);

		if (ts == TransitionState::OK_TS)
			ts = check_other_cells(sphere_path.check_cell);
	}
	else
	{
		ts = TransitionState::COLLIDED_TS;
	}

	if (validate_placement(ts, 1) != 1)
		return 0;

	sphere_path.insert_type = SPHEREPATH::PLACEMENT_INSERT;
	if (!find_placement_pos())
		return 0;

	if (object_info.step_down)
	{
		float z_val = z_for_landing;
		float step_down_ht = object_info.step_down_height;
		sphere_path.walkable_allowance = z_for_landing;
		sphere_path.save_check_pos();

		sphere_path.backup = sphere_path.insert_type;
		sphere_path.insert_type = SPHEREPATH::TRANSITION_INSERT;

		if (sphere_path.num_sphere < 2)
		{
			if ((sphere_path.global_sphere->radius + sphere_path.global_sphere->radius) < step_down_ht)
				step_down_ht = sphere_path.global_sphere->radius * 0.5f;
		}

		if ((sphere_path.global_sphere->radius + sphere_path.global_sphere->radius) >= step_down_ht)
		{
			if (!step_down(step_down_ht, z_for_landing))
			{
				sphere_path.restore_check_pos();
				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;
			}
		}
		else
		{
			step_down_ht *= 0.5f;
			if (!step_down(step_down_ht, z_for_landing) && !step_down(step_down_ht, z_val))
			{
				sphere_path.restore_check_pos();
				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;
			}
		}

		sphere_path.insert_type = sphere_path.backup;
		sphere_path.walkable = NULL;
	}

	return validate_placement(ts, 1) == OK_TS;
}

TransitionState CTransition::validate_placement(TransitionState ts, int adjust)
{
	if (!sphere_path.check_cell)
		return TransitionState::COLLIDED_TS;

	switch (ts)
	{
	case TransitionState::OK_TS:
		{
			assert(sphere_path.check_cell != NULL);

			sphere_path.curr_pos = sphere_path.check_pos;
			sphere_path.curr_cell = sphere_path.check_cell;
			sphere_path.cache_global_curr_center();
			break;
		}
	case TransitionState::ADJUSTED_TS:
	case TransitionState::SLID_TS:
		{
			if (adjust)
			{
				return validate_placement(placement_insert(), 0);
			}
		}
	}

	return ts;
}

TransitionState CTransition::placement_insert()
{
	if (sphere_path.check_cell)
	{
		TransitionState ts = insert_into_cell(sphere_path.check_cell, 3);

		if (ts == TransitionState::OK_TS)
		{
			ts = check_other_cells(sphere_path.check_cell);
		}

		return ts;
	}
	else
	{
		return TransitionState::COLLIDED_TS;
	}
}

TransitionState CTransition::insert_into_cell(CObjCell *cell, int num_insertion_attempts)
{
	if (!cell)
	{
		return TransitionState::COLLIDED_TS;
	}

	TransitionState ts = OK_TS;
	
	for (int i = 0; i < num_insertion_attempts; i++)
	{
		ts = cell->find_collisions(this);
		
		switch (ts)
		{
		case TransitionState::OK_TS:
		case TransitionState::COLLIDED_TS:
			return ts;

		case TransitionState::SLID_TS:
			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			break;
		}
	}

	return ts;
}

void SPHEREPATH::adjust_check_pos(uint32_t cell_id)
{
	if ((WORD)cell_id < 0x100u)
	{
		Vector offset = LandDefs::get_block_offset(cell_id, check_pos.objcell_id);
		cache_global_sphere(&offset);
		check_pos.frame.m_origin += offset;
	}
	check_pos.objcell_id = cell_id;
}

TransitionState CTransition::check_other_cells(CObjCell *curr_cell)
{
	TransitionState ts = OK_TS;

	sphere_path.cell_array_valid = 1;
	sphere_path.hits_interior_cell = 0;

	CObjCell *new_cell2 = NULL;
	CObjCell::find_cell_list(&cell_array, &new_cell2, &sphere_path);

	for (uint32_t i = 0; i < cell_array.num_cells; i++)
	{
		CObjCell *other = cell_array.cells.data[i].cell;

		if (other && other != curr_cell)
		{
			ts = other->find_collisions(this);

			switch (ts)
			{
			case SLID_TS:
				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;
				return ts;

			case COLLIDED_TS:
			case ADJUSTED_TS:
				return ts;
			}
		}
	}

	sphere_path.check_cell = new_cell2;
	if (new_cell2)
	{
		sphere_path.adjust_check_pos(new_cell2->id);
		return ts;
	}

	if (sphere_path.step_down)
		return COLLIDED_TS;

	Position p;
	p = sphere_path.check_pos;

	if (((WORD)p.objcell_id) < 0x100)
	{
		LandDefs::adjust_to_outside(&p.objcell_id, &p.frame.m_origin);
	}

	if (p.objcell_id)
	{
		sphere_path.adjust_check_pos(p.objcell_id);

		sphere_path.set_check_pos(&p, NULL);

		sphere_path.cell_array_valid = 1;
		return ts;
	}

	return COLLIDED_TS;
}

void SPHEREPATH::cache_localspace_sphere(Position *p, const float scale)
{		
	double invScale = 1.0 / scale;

	for (uint32_t i = 0; i < num_sphere; i++)
	{
		localspace_sphere[i].radius = invScale * local_sphere[i].radius;
		localspace_sphere[i].center = p->localtolocal(check_pos, local_sphere[i].center) * invScale;
		localspace_curr_center[i] = p->localtolocal(curr_pos, local_sphere[i].center) * invScale;
	}

	localspace_pos = *p;
	localspace_z = p->globaltolocalvec(Vector(0, 0, 1));
	localspace_low_point = localspace_sphere[0].center - (localspace_z * localspace_sphere[0].radius);
}

Vector CTransition::adjust_offset(Vector *offset)
{
	Vector coffset;
	Vector new_offset = *offset;
	int v7 = 0;

	if (collision_info.sliding_normal_valid)
	{
		if (new_offset.dot_product(collision_info.sliding_normal) < 0.0)
			v7 = 1;
		else
			collision_info.sliding_normal_valid = 0;
	}

	if (collision_info.contact_plane_valid)
	{
		float dp = new_offset.dot_product(collision_info.contact_plane.m_normal);

		if (v7)
		{
			Vector coffset = cross_product(collision_info.contact_plane.m_normal, collision_info.sliding_normal);

			if (coffset.normalize_check_small())
			{
				coffset = Vector(0, 0, 0);
				new_offset = Vector(0, 0, 0);
			}
			else
			{
				float v9 = coffset.dot_product(new_offset);
				coffset *= v9;
				new_offset = coffset;
			}
		}
		else if (dp <= 0.0)
		{
			coffset = new_offset - (collision_info.contact_plane.m_normal * dp);
			new_offset = coffset;
		}
		else
		{
			collision_info.contact_plane.snap_to_plane(&new_offset);
		}
		
		if (!collision_info.contact_plane_is_water)
		{
			if (collision_info.contact_plane_cell_id)
			{
				Vector coffset = LandDefs::get_block_offset(
					sphere_path.check_pos.objcell_id,
					collision_info.contact_plane_cell_id);

				float v13 = collision_info.contact_plane.dot_product(sphere_path.global_sphere->center - coffset);

				if (v13 < (sphere_path.global_sphere->radius - F_EPSILON))
				{
					float v14 = (sphere_path.global_sphere->radius - v13) / collision_info.contact_plane.m_normal.z;
					if (sphere_path.global_sphere->radius > fabs(v14))
					{
						coffset = Vector(0, 0, v14);
						sphere_path.add_offset_to_check_pos(&coffset);
					}
				}
			}
		}
	}
	else if (v7)
	{
		float dp = new_offset.dot_product(collision_info.sliding_normal);
		new_offset -= (collision_info.sliding_normal * dp);
	}

	return new_offset;
}

TransitionState CTransition::validate_placement_transition(TransitionState ts, int *redo)
{
	*redo = 0;

	if (!sphere_path.check_cell)
		return COLLIDED_TS;

	switch (ts)
	{
	case OK_TS:
		assert(sphere_path.check_cell != NULL);

		sphere_path.curr_pos = sphere_path.check_pos;
		sphere_path.curr_cell = sphere_path.check_cell;
		sphere_path.cache_global_curr_center();
		break;

	case COLLIDED_TS:
	case ADJUSTED_TS:
	case SLID_TS:
		if (sphere_path.placement_allows_sliding)
			collision_info.init();

		break;
	}

	return ts;
}

const float TRANSITIONAL_PERCENT_OF_RADIUS = 1.0f;

int CTransition::find_placement_pos()
{
	// LOG(Temp, Normal, "CTransition::find_placement_pos()\n");
	sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);

	collision_info.sliding_normal_valid = 0;
	collision_info.contact_plane_valid = 0;
	collision_info.contact_plane_is_water = 0;

	TransitionState ts = transitional_insert(3);

	BOOL redo;
	ts = validate_placement_transition(ts, &redo);

	if (ts == OK_TS)
	{
		// LOG(Temp, Normal, "validate_placement_transition == OK_TS\n");
		return 1;
	}

	if (!sphere_path.placement_allows_sliding)
	{
		// LOG(Temp, Normal, "!sphere_path.placement_allows_sliding\n");
		return 0;
	}

	float adjustDist = 4.0f;
	float adjustRad = adjustDist;
	float sphereRad = sphere_path.local_sphere->radius;

	bool fakeSphere = false;
	if (sphereRad < 0.125f)
	{
		fakeSphere = true;
		adjustRad = 2.0f;
	}
	else
	{
		// >= 0.125f
		if (sphereRad < 0.48f)
			sphereRad = 0.48f;
	}

	float movementDelta = sphereRad;
	movementDelta *= TRANSITIONAL_PERCENT_OF_RADIUS;

	float dist_per_step;
	uint32_t num_steps;

	double numStepsF = 4.0 / movementDelta;

	if (fakeSphere)
		numStepsF *= 0.5;

	if (numStepsF <= 1.0f)
		return false;

	numStepsF = ceil(numStepsF);
	dist_per_step = adjustRad / numStepsF;
	num_steps = (uint32_t) numStepsF;

	float distance = 0;
	float numRadial = 0;
	float dNumRadial = PI * dist_per_step / sphereRad;

	// LOG(Temp, Normal, "Checking...\n");

	for (uint32_t i = 0; i < num_steps; ++i)
	{
		distance += dist_per_step;
		numRadial += dNumRadial;
		uint32_t numRad = (uint32_t) ceil(numRadial);
		numRad += numRad;
		float angleOffset = 360.0f / (float)numRad;

		Frame offsetFrame;

		for (uint32_t j = 0; j < numRad; ++j)
		{
			sphere_path.set_check_pos(&sphere_path.curr_pos, sphere_path.curr_cell);

			offsetFrame.set_heading((float)j * angleOffset);

			Vector offset = offsetFrame.get_vector_heading() * distance;

			sphere_path.global_offset = adjust_offset(&offset);

			if (sphere_path.global_offset.dot_product(sphere_path.global_offset) >= (F_EPSILON * F_EPSILON))
			{
				sphere_path.add_offset_to_check_pos(&sphere_path.global_offset);

				collision_info.sliding_normal_valid = 0;
				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;

				ts = transitional_insert(3);
				ts = validate_placement_transition(ts, &redo);

				if (ts == OK_TS)
					return true;
			}
		}
	}

	return false;
}

Position SPHEREPATH::get_walkable_pos()
{
	return walkable_pos;
}

void SPHEREPATH::set_walkable_check_pos(CSphere *sphere)
{
	walkable_check_pos = *sphere;
}

TransitionState SPHEREPATH::precipice_slide(COLLISIONINFO *collisions)
{
	Vector normal;
	if (walkable->find_crossed_edge(&walkable_check_pos, &walkable_up, &normal))
	{
		walkable = 0;
		step_up = 0;
		normal = walkable_pos.frame.localtoglobalvec(normal);

		Vector blockOffset = LandDefs::get_block_offset(curr_pos.objcell_id, check_pos.objcell_id);
		Vector someOffset = global_sphere->center - *global_curr_center;

		if (normal.dot_product(blockOffset + someOffset) > 0.0)
		{
			normal = normal * -1.0f;
		}

		return global_sphere->slide_sphere(this, collisions, &normal, global_curr_center);
	}
	else
	{
		walkable = 0;
		return COLLIDED_TS;
	}
}

TransitionState CTransition::cliff_slide(Plane *contact_plane)
{
	Vector cross = cross_product(contact_plane->m_normal, collision_info.last_known_contact_plane.m_normal);
	cross.z = 0;

	Vector collision_normal;
	collision_normal.x = cross.z - cross.y;
	collision_normal.y = cross.x - cross.z;
	collision_normal.z = 0.0f;

	if (collision_normal.normalize_check_small())
		return OK_TS;

	Vector result = LandDefs::get_block_offset(sphere_path.curr_pos.objcell_id, sphere_path.check_pos.objcell_id);
	Vector v9 = sphere_path.global_sphere->center - *sphere_path.global_curr_center;
	
	float v13 = collision_normal.dot_product(result + v9);
	if (v13 <= 0.0)
	{
		result = collision_normal * v13;
		sphere_path.add_offset_to_check_pos(&result);
		collision_info.set_collision_normal(collision_normal);
	}
	else
	{
		result = collision_normal * -v13;
		sphere_path.add_offset_to_check_pos(&result);

		result = collision_normal * -1.0f;
		collision_info.set_collision_normal(result);
	}

	return ADJUSTED_TS;
}

int CTransition::edge_slide(TransitionState *ts, float step_down_ht, float z_val)
{
	if (object_info.state & OBJECTINFO::ON_WALKABLE_OI && object_info.state & OBJECTINFO::EDGE_SLIDE)
	{
		if (collision_info.contact_plane_valid && collision_info.contact_plane.m_normal.z < (double)z_val)
		{
			sphere_path.walkable = 0;
			sphere_path.restore_check_pos();
			*ts = cliff_slide(&collision_info.contact_plane);

			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			return FALSE;
		}
		else if (sphere_path.walkable)
		{
			sphere_path.restore_check_pos();
			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			*ts = sphere_path.precipice_slide(&collision_info);

			return (*ts == COLLIDED_TS);
		}
		else if (collision_info.contact_plane_valid)
		{
			sphere_path.walkable = 0;
			sphere_path.restore_check_pos();
			sphere_path.cell_array_valid = 1;
			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			*ts = OK_TS;
			return TRUE;
		}
		else
		{
			Vector move = *sphere_path.global_curr_center - sphere_path.global_sphere->center;
			sphere_path.add_offset_to_check_pos(&move);
			
			step_down(step_down_ht, z_val);

			collision_info.contact_plane_valid = 0;
			collision_info.contact_plane_is_water = 0;
			sphere_path.restore_check_pos();

			if (sphere_path.walkable)
			{
				collision_info.contact_plane_valid = 0;
				collision_info.contact_plane_is_water = 0;
								
				float v13 = sphere_path.walkable_scale;
				Position v14 = sphere_path.get_walkable_pos();
				sphere_path.cache_localspace_sphere(&v14, v13);

				sphere_path.set_walkable_check_pos(sphere_path.localspace_sphere);
				*ts = sphere_path.precipice_slide(&collision_info);

				return (*ts == 2);
			}
			else
			{
				sphere_path.walkable = 0;
				sphere_path.cell_array_valid = 1;
				*ts = COLLIDED_TS;
				return TRUE;
			}
		}
	}

	sphere_path.walkable = 0;

	sphere_path.restore_check_pos();

	collision_info.contact_plane_valid = 0;
	collision_info.contact_plane_is_water = 0;
	sphere_path.cell_array_valid = 1;
	*ts = OK_TS;	
	return TRUE;
}

void SPHEREPATH::set_walkable(CSphere *sphere, CPolygon *poly, Vector *zaxis, Position *local_pos, float scale)
{
	walkable_check_pos = *sphere;
	walkable = poly;
	walkable_up = *zaxis;
	walkable_pos = *local_pos;
	walkable_scale = scale;
}

void SPHEREPATH::set_neg_poly_hit(int step_up, Vector *collision_normal)
{
	neg_step_up = step_up;
	neg_poly_hit = 1;

	neg_collision_normal.x = -collision_normal->x;
	neg_collision_normal.y = -collision_normal->y;
	neg_collision_normal.z = -collision_normal->z;
}
