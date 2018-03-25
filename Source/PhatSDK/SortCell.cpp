
#include "StdAfx.h"
#include "SortCell.h"

CSortCell::CSortCell()
{
	building = NULL;
}

void CSortCell::add_building(CBuildingObj *_object)
{
	if (!building)
		building = _object;
}

void CSortCell::remove_building(CBuildingObj *_object)
{
	building = NULL;
}

BOOL CSortCell::has_building()
{
	return (building != NULL);
}

CPhysicsObj *CSortCell::get_object(DWORD obj_iid)
{
	CPhysicsObj *result = CObjCell::get_object(obj_iid);

	if (!result)
	{
		if (building)
			result = building->get_object(obj_iid);
	}

	return result;
}

TransitionState CSortCell::find_collisions(CTransition *transit)
{
	if (building)
	{
		return building->find_building_collisions(transit);
	}

	return OK_TS;
}

void CSortCell::find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	if (building)
	{
		building->find_building_transit_cells(num_parts, parts, cell_array);
	}
}

void CSortCell::find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	if (building)
	{
		building->find_building_transit_cells(p, num_sphere, sphere, cell_array, path);
	}
}