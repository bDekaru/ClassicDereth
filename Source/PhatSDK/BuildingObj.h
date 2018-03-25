
#pragma once

#include "PhysicsObj.h"

class CBuildingObj : public CPhysicsObj
{
public:
	CBuildingObj();
	virtual ~CBuildingObj();

	static CBuildingObj *makeBuilding(DWORD data_id, unsigned int _num_portals, class CBldPortal **_portals, unsigned int _num_leaves);

	CPhysicsObj *get_object(DWORD obj_iid);
	void remove();

	void add_to_cell(class CSortCell *new_cell);
	void add_to_stablist(DWORD * &block_stab_list, DWORD &max_size, DWORD &stab_num);
	TransitionState find_building_collisions(CTransition *transition);
	void find_building_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path);
	void find_building_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array);

	unsigned int num_portals = 0;
	class CBldPortal **portals = NULL;
	unsigned int num_leaves = 0;
	class CPartCell **leaf_cells = NULL;
	// unsigned int num_shadow = 0; unused
	// DArray<class CShadowPart *> shadow_list;  unused
};
