
#pragma once

#include "PhysicsObj.h"

class CBuildingObj : public CPhysicsObj
{
public:
	CBuildingObj();
	virtual ~CBuildingObj();

	static CBuildingObj *makeBuilding(uint32_t data_id, unsigned int _num_portals, class CBldPortal **_portals, unsigned int _num_leaves);

	CPhysicsObj *get_object(uint32_t obj_iid);
	void remove();

	void add_to_cell(class CSortCell *new_cell);
	void add_to_stablist(uint32_t * &block_stab_list, uint32_t &max_size, uint32_t &stab_num);
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