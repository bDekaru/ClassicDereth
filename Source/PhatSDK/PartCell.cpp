
#include "StdAfx.h"
#include "PartCell.h"
#include "BSPData.h"
#include "PhysicsPart.h"

CPartCell::CPartCell() : num_shadow_parts(0), shadow_part_list(128)
{
}

CPartCell::~CPartCell()
{
	assert(num_shadow_parts == 0);
}

void CPartCell::add_part(CPhysicsPart *part, ClipPlaneList **planes, Frame *frame, unsigned int num_shadow_parts_) // TODO fix this variable name its colliding
{
	CShadowPart *newPart;	
	if (planes)
	{
		newPart = new CShadowPart(1, planes, frame, part);
	}
	else
	{
		newPart = new CShadowPart(NULL, NULL, part);
	}
	
	if (num_shadow_parts >= shadow_part_list.alloc_size)
		shadow_part_list.grow(shadow_part_list.alloc_size + 100);

	shadow_part_list.data[num_shadow_parts++] = newPart;
}

void CPartCell::remove_part(CPhysicsPart *part)
{
	for (DWORD i = 0; i < num_shadow_parts; i++)
	{
		CShadowPart *shadowPart = shadow_part_list.array_data[i];
		if (shadowPart->part == part)
		{
			shadowPart->planes = NULL;
			shadowPart->num_planes = 0;
			delete shadowPart;

			shadow_part_list.array_data[i] = shadow_part_list.array_data[--num_shadow_parts];

			if ((num_shadow_parts + 200) < shadow_part_list.alloc_size)
				shadow_part_list.shrink(num_shadow_parts + 100);

			break;
		}
	}
}

CShadowPart::CShadowPart(unsigned int nump, ClipPlaneList **planes_, Frame *frame_, CPhysicsPart *part_)
{
	num_planes = nump;
	planes = planes_;
	frame = frame_;
	part = part_;
}

CShadowPart::CShadowPart(unsigned int nump, Frame *frame_, CPhysicsPart *part_)
{
	part = part_;
	frame = frame_;
	num_planes = nump;

	if (nump)
	{
		planes = new ClipPlaneList*[nump];
		for (DWORD i = 0; i < nump; i++)
			planes[i] = NULL;
	}
	else
	{
		planes = NULL;
	}
}

CShadowPart::~CShadowPart()
{
	if (planes)
	{
		for (DWORD i = 0; i < num_planes; i++)
		{
			if (planes[i])
			{
				delete planes[i];
			}
		}

		delete[] planes;
	}
}

void CShadowPart::draw(DWORD ClipPlaneIndex)
{
#if PHATSDK_RENDER_AVAILABLE
	part->Draw(FALSE);
#endif
}





