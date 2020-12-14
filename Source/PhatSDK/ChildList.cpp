
#include <StdAfx.h>
#include "PhysicsObj.h"
#include "ChildList.h"

CHILDLIST::CHILDLIST()
{
	num_objects = 0;
}

CHILDLIST::~CHILDLIST()
{
}

BOOL CHILDLIST::FindChildIndex(CPhysicsObj *pObj, WORD *Index)
{
	for (uint32_t i = 0; i < num_objects; i++)
	{
		if (objects.array_data[i] == pObj)
		{
			*Index = (WORD) i;
			return TRUE;
		}
	}

	return FALSE;
}

void CHILDLIST::add_child(CPhysicsObj *pChild, Frame *pFrame, uint32_t part_number, uint32_t location_id)
{
	if (num_objects >= objects.array_size)
	{
		uint32_t new_size = objects.array_size + 4;

		objects.grow(new_size);
		frames.grow(new_size);
		part_numbers.grow(new_size);
		location_ids.grow(new_size);
	}

	objects.array_data[num_objects] = pChild;
	frames.array_data[num_objects] = *pFrame;
	part_numbers.array_data[num_objects] = part_number;
	location_ids.array_data[num_objects] = location_id;

	num_objects++;
}

void CHILDLIST::remove_child(CPhysicsObj *pChild)
{
	uint32_t i;
	for (i = 0; i < num_objects; i++)
	{
		if (objects.array_data[i] == pChild)
			break;
	}

	num_objects--;

	// Shift all children.
	for (; i < num_objects; i++)
	{
		uint32_t j = i + 1;

		objects.array_data[i] = objects.array_data[j];
		frames.array_data[i] = frames.array_data[j];
		part_numbers.array_data[i] = part_numbers.array_data[j];
		location_ids.array_data[i] = location_ids.array_data[j];
	}
}



