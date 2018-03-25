
#include "StdAfx.h"
#include "BuildingObj.h"

CBuildingObj::CBuildingObj() //  : shadow_list(128) unused
{
}

CBuildingObj::~CBuildingObj()
{
	SafeDeleteArray(portals);
	
	if (leaf_cells)
	{
		for (DWORD i = 0; i < num_leaves; i++)
		{
			if (leaf_cells[i])
				delete leaf_cells[i];
		}
		delete [] leaf_cells;
		leaf_cells = NULL;
	}

	num_leaves = 0;
	// num_shadow = 0;  unused
}

CBuildingObj *CBuildingObj::makeBuilding(DWORD data_id, unsigned int _num_portals, CBldPortal **_portals, unsigned int _num_leaves)
{
	CBuildingObj *building = new CBuildingObj();

	if (!building->InitObjectBegin(0, FALSE) || !building->InitPartArrayObject(data_id, TRUE))
	{
		delete building;
		return NULL;
	}

	building->num_leaves = _num_leaves;
	building->leaf_cells = new CPartCell *[_num_leaves];
	for (DWORD i = 0; i < building->num_leaves; i++)
		building->leaf_cells[i] = NULL;

	building->num_portals = _num_portals;
	building->portals = new CBldPortal *[_num_portals];
	for (DWORD i = 0; i < building->num_portals; i++)
		building->portals[i] = _portals[i];
	
	if (!building->InitObjectEnd())
	{
		delete building;
		return NULL;
	}

	return building;
}

void CBuildingObj::remove()
{
	((CSortCell *)cell)->remove_building(this);

	set_cell_id(0);
	cell = NULL;
}

CPhysicsObj *CBuildingObj::get_object(DWORD obj_iid)
{
	PackableHashTable<unsigned long, int> visited_cells;

	if (num_portals)
	{
		for (DWORD i = 0; i < num_portals; i++)
		{
			if (portals[i])
			{
				CEnvCell *pOtherCell = portals[i]->GetOtherCell();
				if (pOtherCell)
				{
					CPhysicsObj *pObject = pOtherCell->recursively_get_object(obj_iid, &visited_cells);
					if (pObject)
					{
						return pObject;
					}
				}
			}
		}
	}

	return NULL;
}

void CBuildingObj::add_to_cell(CSortCell *new_cell)
{
	new_cell->add_building(this);

	set_cell_id(new_cell->GetID());
	cell = new_cell;
}

void CBuildingObj::add_to_stablist(DWORD * &block_stab_list, DWORD &max_size, DWORD &stab_num)
{
	for (DWORD i = 0; i < num_portals; i++)
		portals[i]->add_to_stablist(block_stab_list, max_size, stab_num);
}

TransitionState CBuildingObj::find_building_collisions(CTransition *transition)
{
	if (!part_array)
		return OK_TS;

	transition->sphere_path.bldg_check = 1;
	TransitionState ts = part_array->parts[0]->find_obj_collisions(transition);
	transition->sphere_path.bldg_check = 0;

	if (ts != OK_TS && !(transition->object_info.state & 1))
		transition->collision_info.collided_with_environment = 1;

	return ts;
}

void CBuildingObj::find_building_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	for (DWORD i = 0; i < num_portals; i++)
	{
		CEnvCell *pOtherCell = portals[i]->GetOtherCell();
		if (pOtherCell)
			pOtherCell->check_building_transit(portals[i]->other_portal_id, p, num_sphere, sphere, cell_array, path);
	}
}

void CBuildingObj::find_building_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	for (DWORD i = 0; i < num_portals; i++)
	{
		CEnvCell *pOtherCell = portals[i]->GetOtherCell();
		if (pOtherCell)
			pOtherCell->check_building_transit(portals[i]->other_portal_id, num_parts, parts, cell_array);
	}
}

