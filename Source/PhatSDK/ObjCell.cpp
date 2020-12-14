
#include <StdAfx.h>
#include "ObjCell.h"
#include "PartArray.h"
#include "PhysicsObj.h"
#include "Transition.h"
#include "Frame.h"
#include "BSPData.h"
#include "LandCell.h"
#include "EnvCell.h"
#include "LandDefs.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

CObjCell::CObjCell() : 
	num_objects(0), object_list(128),
	num_lights(0), light_list(128),
	num_shadow_objects(0), shadow_object_list(4, 128)
{
	restriction_obj = 0;
	clip_planes = 0;
	num_stabs = 0;
	stab_list = NULL;
	seen_outside = 0;
	voyeur_table = 0;
	myLandBlock_ = NULL;
}

CObjCell::~CObjCell()
{
	if (voyeur_table)
	{
		delete voyeur_table;
		voyeur_table = NULL;
	}

	if (clip_planes)
	{
		if (clip_planes[0])
		{
			delete clip_planes[0];
		}
		delete [] clip_planes;
	}
}

CPhysicsObj *CObjCell::get_object(uint32_t iid)
{
	for (uint32_t i = 0; i < num_objects; i++)
	{
		CPhysicsObj *pObject = object_list.array_data[i];

		if (pObject && pObject->GetID() == iid)
			return pObject;
	}

	return NULL;
}

void CObjCell::add_object(CPhysicsObj *pObject)
{
	if (num_objects >= object_list.alloc_size)
		object_list.grow(object_list.alloc_size + 5);

	object_list.array_data[num_objects++] = pObject;

	if (pObject->id && !pObject->parent)
	{
		if (!(pObject->m_PhysicsState & HIDDEN_PS))
		{
			if (voyeur_table)
			{
				LongNIValHashIter<GlobalVoyeurInfo> it(voyeur_table);

				while (!it.EndReached())
				{
					try
					{
						uint32_t voyeur_id = it.GetCurrent()->id;

						if (voyeur_id != pObject->id && voyeur_id && !pObject->parent)
						{
							CPhysicsObj *pVoyeur = CPhysicsObj::GetObject(voyeur_id);

							if (pVoyeur)
							{
								DetectionInfo info;
								info.object_id = pObject->id;
								info.object_status = EnteredDetection;
								pVoyeur->receive_detection_update(&info);
							}
						}

						it.Next();
					}
					catch (...)
					{
						SERVER_ERROR << "Error in Add Object";
					}
				}
			}
		}
	}
}

void CObjCell::remove_object(CPhysicsObj *pObject)
{
	for (uint32_t i = 0; i < num_objects; i++)
	{
		if (pObject == object_list.array_data[i])
		{
			// DEBUGOUT("Removing index %u object (total was %u)\r\n", i, m_ObjectCount);
			object_list.array_data[i] = object_list.array_data[--num_objects];

			// MISSING CODE HERE
			// update_all_voyeur

			if ((num_objects + 10) < object_list.alloc_size)
				object_list.shrink(num_objects + 5);

			break;
		}
	}
}

void CObjCell::add_shadow_object(CShadowObj *_object, unsigned int num_shadow_cells)
{
	shadow_object_list.ensure_space(num_shadow_objects + 1, 5);
	shadow_object_list.data[num_shadow_objects] = _object;
	num_shadow_objects++;
	_object->cell = this;
}

void CObjCell::remove_shadow_object(CShadowObj *_object)
{
	for (uint32_t i = 0; i < num_shadow_objects; i++)
	{
		if (_object == shadow_object_list.array_data[i])
		{
			_object->cell = NULL;
			shadow_object_list.array_data[i] = shadow_object_list.array_data[--num_shadow_objects];

			if ((num_shadow_objects + 10) < shadow_object_list.alloc_size)
			{
				shadow_object_list.shrink(num_shadow_objects + 5);
			}
		}
	}
}

void CObjCell::add_light(LIGHTOBJ *Light)
{
#if PHATSDK_RENDER_AVAILABLE
	// if (m_Position.objcell_id != 0x0120010E)
	//     return;

	if (num_lights >= light_list.alloc_size)
		light_list.grow(light_list.alloc_size + 5);

	light_list.array_data[num_lights++] = Light;

	// DEBUGOUT("Adding light(Mem@%08X) on cell %08X.\r\n", (uint32_t)Light, m_Position.objcell_id);
	Render::pLightManager->AddLight(Light, &pos);
#endif
}

void CObjCell::remove_light(LIGHTOBJ *Light)
{
#if PHATSDK_RENDER_AVAILABLE
	for (uint32_t i = 0; i < num_lights; i++)
	{
		if (Light == light_list.array_data[i])
		{
			// DEBUGOUT("Removing index %u light (total was %u)\r\n", i, num_lights);
			light_list.array_data[i] = light_list.array_data[--num_lights];

			if ((num_lights + 10) < light_list.alloc_size)
				light_list.shrink(num_lights + 5);

			if (Render::pLightManager)
				Render::pLightManager->RemoveLight(Light);
			break;
		}
	}
#endif
}

CShadowObj::CShadowObj()
{
	physobj = NULL;
	m_CellID = 0;
	cell = NULL;
}

CShadowObj::~CShadowObj()
{
}

void CShadowObj::set_physobj(CPhysicsObj *pObject)
{
	physobj = pObject;
	id = pObject->GetID();
}

TransitionState CObjCell::find_collisions(class CTransition *)
{
	return TransitionState::INVALID_TS;
}

TransitionState CObjCell::find_obj_collisions(CTransition *transition)
{
	TransitionState ts = OK_TS;

	if (transition->sphere_path.insert_type != SPHEREPATH::INITIAL_PLACEMENT_INSERT)
	{
		for (uint32_t i = 0; i < num_shadow_objects; i++)
		{
			CPhysicsObj *pobj = shadow_object_list.array_data[i]->physobj;

			if (!pobj->parent && pobj != transition->object_info.object)
			{
				ts = pobj->FindObjCollisions(transition);
				if (ts != OK_TS)
					break;
			}
		}
	}

	return ts;
}

TransitionState CObjCell::find_env_collisions(CTransition *transition)
{
	return TransitionState::INVALID_TS;
}

TransitionState CObjCell::check_entry_restrictions(CTransition *transition)
{
	if (!transition->object_info.object)
		return COLLIDED_TS;

	/*
	if (transition->object_info.object->weenie_obj)
	{
		v4 = ((int(__thiscall *)(CWeenieObject *))v3->vfptr[18].__vecDelDtor)(transition->object_info.object->weenie_obj);
		v5 = transition->object_info.state;
		if (BYTE1(v5) & 1)
		{
			if (v2->restriction_obj && !v4)
			{
				v6 = CPhysicsObj::GetObjectA(v2->restriction_obj);
				if (!v6)
					return 2;
				v7 = v6->weenie_obj;
				if (!v7)
					return 2;
				if (!((int(__stdcall *)(CWeenieObject *))v7->vfptr[17].__vecDelDtor)(v3))
				{
					((void(__thiscall *)(CObjCell *, CTransition *))v2->vfptr[6].IUnknown_QueryInterface)(v2, transition);
					return 2;
				}
			}
		}
	}
	*/
	// TODO
	//UNFINISHED();

	return OK_TS;
}

void CObjCell::find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	// should be overridden
	assert(0);
}

void CObjCell::find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	// should be overridden
	assert(0);
}

void CObjCell::find_cell_list(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, CObjCell **curr_cell, SPHEREPATH *path)
{
	CObjCell *pCell = NULL;

	cell_array->num_cells = 0;
	cell_array->added_outside = 0;
	if (p->objcell_id)
	{
		if (((WORD)p->objcell_id) >= 0x100)
			pCell = CEnvCell::GetVisible(p->objcell_id);
		else
			pCell = CLandCell::Get(p->objcell_id);
	}

	if (((WORD)p->objcell_id) >= 0x100)
	{
		if (path)
			path->hits_interior_cell = 1;

		cell_array->add_cell(p->objcell_id, pCell);
	}
	else
	{
		CLandCell::add_all_outside_cells(p, num_sphere, sphere, cell_array);
	}
	
	if (pCell && num_sphere)
	{
		for (uint32_t i = 0; i < cell_array->num_cells; i++)
		{
			CObjCell *otherCell = cell_array->cells.data[i].cell;
			if (otherCell)
				otherCell->find_transit_cells(p, num_sphere, sphere, cell_array, path);
		}

		if (curr_cell)
		{
			*curr_cell = NULL;

			for (uint32_t i = 0; i < cell_array->num_cells; i++)
			{
				CObjCell *otherCell = cell_array->cells.data[i].cell;
				if (otherCell)
				{
					Vector blockOffset = LandDefs::get_block_offset(p->objcell_id, otherCell->id);
					Vector localpoint = sphere->center - blockOffset;

					if (otherCell->point_in_cell(localpoint))
					{
						*curr_cell = otherCell;
						if ((otherCell->id & 0xFFFF) >= 0x100)
						{
							if (path)
								path->hits_interior_cell = 1;
							return;
						}
					}
				}
			}

			// assert(*curr_cell);
		}

		if (cell_array->do_not_load_cells)
		{
			if ((p->objcell_id & 0xFFFF) >= 0x100)
			{
				for (uint32_t i = 0; i < cell_array->num_cells; i++)
				{
					uint32_t cellID = cell_array->cells.data[i].cell_id;

					if (pCell->id != cellID)
						continue;

					bool found = false;
					for (uint32_t j = 0; j < pCell->num_stabs; j++)
					{
						if (cellID == pCell->stab_list[j])
						{
							found = true;
							break;
						}
					}
					if (!found)
						cell_array->remove_cell(i);
				}
			}
		}
	}
}

BOOL CObjCell::point_in_cell(const Vector &pt)
{
	return FALSE;
}

void CObjCell::find_cell_list(Position *p, unsigned int num_cylsphere, CCylSphere *cylsphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	static CSphere sphere[10];

	if (num_cylsphere > 10)
		num_cylsphere = 10;

	for (uint32_t i = 0; i < num_cylsphere; i++)
	{
		sphere[i].center = p->localtoglobal(*p, cylsphere[i].low_pt);
		sphere[i].radius = cylsphere[i].radius;
	}

	CObjCell::find_cell_list(p, num_cylsphere, sphere, cell_array, 0, path);
}

void CObjCell::find_cell_list(Position *p, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	CSphere global_sphere;

	global_sphere.center = p->localtoglobal(*p, sphere->center);
	global_sphere.radius = sphere->radius;

	CObjCell::find_cell_list(p, 1, &global_sphere, cell_array, 0, path);
}

void CObjCell::find_cell_list(CELLARRAY *cell_array, CObjCell **check_cell, SPHEREPATH *path)
{
	CObjCell::find_cell_list(&path->check_pos, path->num_sphere, path->global_sphere, cell_array, check_cell, path);
}

CObjCell *CObjCell::GetVisible(uint32_t cell_id, bool bDoPostLoad)
{
	if (cell_id)
	{
		if ((WORD)cell_id >= 0x100)
			return CEnvCell::GetVisible(cell_id, bDoPostLoad);
		else
			return CLandCell::Get(cell_id);
	}

	return NULL;
}

int CObjCell::check_collisions(CPhysicsObj *object)
{
	for (uint32_t i = 0; i < num_shadow_objects; i++)
	{
		CPhysicsObj *pobj = shadow_object_list.data[i]->physobj;
		if (!pobj->parent && pobj != object && pobj->check_collision(object))
			return 1;
	}

	return 0;
}

void CObjCell::release_objects()
{
	while (num_shadow_objects)
	{
		CShadowObj *obj = shadow_object_list.data[0];
		remove_shadow_object(obj);

		obj->physobj->remove_parts(this);
	}

	while (num_objects)
	{
		CPhysicsObj *obj = object_list.data[0];
		obj->leave_world();
		if (!obj->weenie_obj)
			delete obj;
	}
}

LandDefs::WaterType CObjCell::get_block_water_type()
{
	if (myLandBlock_)
		return myLandBlock_->water_type;
	
	return LandDefs::WaterType::NOT_WATER;
}

double CObjCell::get_water_depth(Vector *point)
{
	if (water_type == LandDefs::WaterType::NOT_WATER)
		return 0.0;

	if (water_type != LandDefs::WaterType::PARTIALLY_WATER)
	{
		if (water_type == LandDefs::WaterType::ENTIRELY_WATER)
			return 0.89999998;

		return 0.0;
	}

	if (myLandBlock_)
		return myLandBlock_->calc_water_depth(GetID(), point);

	return 0.1;
}
