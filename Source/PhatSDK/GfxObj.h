
#pragma once

#include "ObjCache.h"
#include "Vertex.h"
#include "BSPData.h"

class CMaterial;
class CPolygon;
class CSurface;

class CGfxObj : public DBObj
{
public:
	CGfxObj();
	~CGfxObj();

	static DBObj *Allocator();
	static void Destroyer(DBObj *);
	static CGfxObj *Get(uint32_t ID);
	static void Release(CGfxObj *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);
	void init_end();
	bool InitLoad();

	TransitionState find_obj_collisions(class CTransition *transition, float scale);

#if PHATSDK_RENDER_AVAILABLE
	class CMaterial *material; // 0x1C / 0x28
#endif

	uint32_t num_surfaces; // 0x20 / 0x2C
	CSurface **m_rgSurfaces; // 0x24 / 0x30
	CVertexArray vertex_array; // 0x28 / 0x34 - size: 0x10

	uint32_t num_physics_polygons; // 0x38 / 0x44
	CPolygon *physics_polygons; // 0x3C / 0x48

#if PHATSDK_RENDER_AVAILABLE
	MeshBuffer *constructed_mesh;
	int use_built_mesh;
#endif

	CSphere *physics_sphere; // 0x40 / 0x4C
	BSPTREE *physics_bsp; // 0x44 / 0x50

#if PHATSDK_RENDER_AVAILABLE
	Vector sort_center; // 0x48 / 0x54 - size: 0x0C
#endif

#ifdef PRE_TOD // surface triangle fans
	uint32_t m_dw60;
	uint32_t m_dw64;
#endif

	uint32_t num_polygons; // 0x5C / 0x68
	CPolygon *polygons; // 0x60 / 0x6C
	CSphere *drawing_sphere; // 0x64 / 0x70
	BSPTREE *drawing_bsp; // 0x68 / 0x74

	BBox gfx_bound_box; // 0x6C / 0x78 - size: 0x18?

	uint32_t m_didDegrade;
};

struct GfxObjDegradeLevel
{
	uint32_t gfxobj_id;
	uint32_t degrade_mode;
	float min_dist;
	float ideal_dist;
	float max_dist;
};

class GfxObjDegradeInfo : public DBObj
{
public:
	GfxObjDegradeInfo();
	~GfxObjDegradeInfo();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static GfxObjDegradeInfo *Get(uint32_t ID);
	static void Release(GfxObjDegradeInfo *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	void get_degrade(float ViewerDist, uint32_t *GfxIndex, uint32_t *GfxFrameMod) const;
	float get_max_degrade_distance(void) const;

	uint32_t num_degrades; // 0x1C / 0x28
	GfxObjDegradeLevel *degrades; // 0x20 / 0x2C
};




