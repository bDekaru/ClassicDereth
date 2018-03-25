
#pragma once

#include "SArray.h"

class CHILDLIST
{
public:
	CHILDLIST();
	~CHILDLIST();

	void add_child(CPhysicsObj *pObj, Frame *pFrame, DWORD part_number, DWORD location_id);
	void remove_child(CPhysicsObj *pChild);

	BOOL FindChildIndex(CPhysicsObj *pObj, WORD *Index);

	WORD num_objects;
	SArray<CPhysicsObj *> objects;
	SArray<Frame> frames;
	SArray<DWORD> part_numbers;
	SArray<DWORD> location_ids;
};

