
#pragma once

#include "Frame.h"
#include "ObjCache.h"
#include "Packable.h"

class ObjectDesc : public PackObj
{
public:
	ObjectDesc();
	virtual ~ObjectDesc();

	DECLARE_PACKABLE();

	double ScaleObj(unsigned int x, unsigned int y, unsigned int k);
	void ObjAlign(Plane *plane, Vector *loc, Frame *obj_frame);
	BOOL CheckSlope(float z_val);
	void GetObjFrame(unsigned int x, unsigned int y, unsigned int k, Vector *loc, Frame *obj_frame);
	void Place(unsigned int ix, unsigned int iy, unsigned int iq, Vector *obj_loc);

	uint32_t obj_id = 0;
	Frame base_loc;
	float freq = 1.0f;
	float displace_x = 0.0f;
	float displace_y = 0.0f;
	float min_scale = 1.0f;
	float max_scale = 1.0f;
	float max_rot = 0.0f;
	float min_slope = 0.0f;
	float max_slope = 90.0f;
	int align = 0;
	int orient = 0;
	int weenie_obj = NULL;
};

class Scene : public DBObj, public PackObj
{
public:
	Scene();
	~Scene();

	static DBObj *Allocator();
	static void Destroyer(DBObj *);
	static Scene *Get(uint32_t ID);
	static void Release(Scene *);

	void Destroy();

	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	unsigned int version = 0;
	unsigned int num_objects = 0;
	ObjectDesc *objects = NULL;
};
