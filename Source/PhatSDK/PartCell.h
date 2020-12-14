
#pragma once
#include "DArray.h"

struct ClipPlaneList;
class Frame;
class CPhysicsPart;

class CShadowPart
{
public:
	CShadowPart(unsigned int nump, ClipPlaneList **planes_, Frame *frame_, CPhysicsPart *part_);
	CShadowPart(unsigned int nump, Frame *_frame, CPhysicsPart *_part);
	~CShadowPart();

	void draw(uint32_t ClipPlaneIndex);

	uint32_t num_planes; // 0x00
	ClipPlaneList** planes; // 0x04
	Frame* frame; // 0x08
	CPhysicsPart* part; // 0x0C
};

class CPartCell
{
public:
	CPartCell();
	virtual ~CPartCell();

	virtual void add_part(CPhysicsPart *part, ClipPlaneList **planes, Frame *frame, unsigned int num_shadow_parts);
	virtual void remove_part(CPhysicsPart *part);

	uint32_t num_shadow_parts;
	DArray<CShadowPart *> shadow_part_list;
};

