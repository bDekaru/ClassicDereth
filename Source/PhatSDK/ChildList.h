
#pragma once

#include "SArray.h"

class CHILDLIST
{
public:
	CHILDLIST();
	~CHILDLIST();

	void add_child(CPhysicsObj *pObj, Frame *pFrame, uint32_t part_number, uint32_t location_id);
	void remove_child(CPhysicsObj *pChild);

	BOOL FindChildIndex(CPhysicsObj *pObj, WORD *Index);

	WORD num_objects;
	SArray<CPhysicsObj *> objects;
	SArray<Frame> frames;
	SArray<uint32_t> part_numbers;
	SArray<uint32_t> location_ids;
};

