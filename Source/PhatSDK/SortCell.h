
#pragma once

#include "ObjCell.h"

class CBuildingObj;

class CSortCell : public CObjCell
{
public:
	CSortCell();

	void add_building(CBuildingObj *_object);
	void remove_building(CBuildingObj *_object);
	BOOL has_building();

	CPhysicsObj *get_object(uint32_t obj_iid);

	virtual TransitionState find_collisions(class CTransition *transit) override;
	virtual void find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array) override;
	virtual void find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path) override;

	CBuildingObj *building; // 0xE8
};