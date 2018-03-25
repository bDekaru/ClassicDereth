
#include "StdAfx.h"
#include "Polygon.h"
#include "Surface.h"
#include "GfxObj.h"
#include "Transition.h"

CGfxObj::CGfxObj()
{
#if PHATSDK_RENDER_AVAILABLE
	sort_center = Vector(0, 0, 0);
	material = NULL;
#endif

	num_surfaces = 0;
	m_rgSurfaces = NULL;

	num_physics_polygons = 0;
	physics_polygons = NULL;

#if PHATSDK_RENDER_AVAILABLE
	constructed_mesh = 0;
	use_built_mesh = 0;
#endif

	physics_sphere = NULL;
	physics_bsp = NULL;

#ifdef PRE_TOD // surface triangle fans
	m_dw60 = 0;
	m_dw64 = 0;
#endif

	num_polygons = 0;
	polygons = NULL;
	drawing_sphere = NULL;
	drawing_bsp = NULL;

	m_didDegrade = 0;
}

CGfxObj::~CGfxObj()
{
	Destroy();
}

DBObj* CGfxObj::Allocator()
{
	return((DBObj *)new CGfxObj());
}

void CGfxObj::Destroyer(DBObj* pGfxObj)
{
	delete ((CGfxObj *)pGfxObj);
}

CGfxObj *CGfxObj::Get(DWORD ID)
{
	return (CGfxObj *)ObjCaches::GfxObjs->Get(ID);
}

void CGfxObj::Release(CGfxObj *pGfxObj)
{
	if (pGfxObj)
		ObjCaches::GfxObjs->Release(pGfxObj->GetID());
}

void CGfxObj::Destroy()
{
	if (drawing_bsp)
	{
		delete drawing_bsp;
		drawing_bsp = NULL;
	}
	drawing_sphere = NULL;

	if (polygons)
	{
		delete[] polygons;
		polygons = NULL;
	}
	num_polygons = 0;

#ifdef PRE_TOD
	if (m_dw64)
	{
		UNFINISHED_LEGACY("CSurfaceTriStrips::Free call");

		// CSurfaceTriStrips::Free(m_dw64);
		m_dw64 = NULL;

		if (drawing_sphere)
			delete drawing_sphere;
	}
	m_dw60 = 0;
#endif

	if (physics_polygons)
	{
		delete[] physics_polygons;
		physics_polygons = NULL;
	}

	if (physics_bsp)
	{
		delete physics_bsp;
		physics_bsp = NULL;
	}

	if (physics_sphere)
	{
		// delete physics_sphere;
		physics_sphere = NULL;
	}

	vertex_array.DestroyVertex();

	if (m_rgSurfaces)
	{
		for (DWORD i = 0; i < num_surfaces; i++)
		{
			CSurface::Release(m_rgSurfaces[i]);
		}

		delete[] m_rgSurfaces;
		m_rgSurfaces = NULL;
	}
	num_surfaces = 0;

	m_didDegrade = 0;
}

BOOL CGfxObj::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	UNPACK(DWORD, id);

	DWORD Fields;
	UNPACK(DWORD, Fields);

#ifdef PRE_TOD
	UNPACK(DWORD, num_surfaces);
#else
	UNPACK_COMPRESSED32(num_surfaces);
#endif

	m_rgSurfaces = new CSurface*[num_surfaces];

	for (DWORD i = 0; i < num_surfaces; i++)
	{
		DWORD TextureID;
		UNPACK(DWORD, TextureID);

		m_rgSurfaces[i] = CSurface::Get(TextureID);
	}

	if (!UNPACK_OBJ(vertex_array))
		return FALSE;

	CPolygon::SetPackVerts(&vertex_array);

	if (Fields & 1)
	{
		// Collision data
#ifdef PRE_TOD
		UNPACK(DWORD, num_physics_polygons);
#else
		UNPACK_COMPRESSED32(num_physics_polygons);
#endif

		physics_polygons = new CPolygon[num_physics_polygons];

		for (DWORD i = 0; i < num_physics_polygons; i++)
			UNPACK_OBJ(physics_polygons[i]);

		BSPNODE::pack_poly = physics_polygons;
		BSPNODE::pack_tree_type = 1;

		physics_bsp = new BSPTREE();
		UNPACK_POBJ(physics_bsp);

		physics_sphere = physics_bsp->GetSphere();
	}

#if PHATSDK_RENDER_AVAILABLE
	UNPACK_OBJ(sort_center);
#else
	Vector dummy;
	dummy.UnPack(ppData, iSize);
#endif

	if (Fields & 2)
	{
		// Rendering data.
#ifdef PRE_TOD
		UNPACK(DWORD, num_polygons);
#else
		UNPACK_COMPRESSED32(num_polygons);
#endif

		polygons = new CPolygon[num_polygons];

		for (DWORD i = 0; i < num_polygons; i++)
			UNPACK_OBJ(polygons[i]);

		BSPNODE::pack_poly = polygons;
		BSPNODE::pack_tree_type = 0;

		drawing_bsp = new BSPTREE();
		UNPACK_POBJ(drawing_bsp);

		drawing_sphere = drawing_bsp->GetSphere();

#if !PHATSDK_RENDER_AVAILABLE
		delete [] polygons;
		polygons = NULL;
#endif
	}

#ifdef PRE_TOD // surface triangle fans
	if (Fields & 4)
	{
		// Not done.
		UNFINISHED();

		drawing_sphere = new CSphere;
		UNPACK_POBJ(drawing_sphere);
	}
#endif

#ifdef PRE_TOD
	PACK_ALIGN();
#else
	// CFTOD PACK_ALIGN();
	// CFTOD Another word here?

	if (Fields & 8)
	{
		UNPACK(DWORD, m_didDegrade);
		assert((m_didDegrade & 0xFF000000) == 0x11000000);
	}
#endif

	init_end();

	InitLoad(); // CUSTOM - NOT ACTUALLY PLACED HERE!

	return TRUE;
}

void CGfxObj::init_end()
{
	if (vertex_array.vertices)
	{
		gfx_bound_box.m_Min = ((CVertex *)vertex_array.vertices)[0].origin;
		gfx_bound_box.m_Max = ((CVertex *)vertex_array.vertices)[0].origin;

		for (DWORD i = 1; i < vertex_array.num_vertices; i++)
			gfx_bound_box.AdjustBBox(((CVertex *)((BYTE *)vertex_array.vertices + i*CVertexArray::vertex_size))->origin);
	}
	else
	{
		gfx_bound_box.m_Min = Vector(0, 0, 0);
		gfx_bound_box.m_Max = Vector(0, 0, 0);
	}
}

GfxObjDegradeInfo::GfxObjDegradeInfo()
{
	num_degrades = 0;
	degrades = NULL;
}

GfxObjDegradeInfo::~GfxObjDegradeInfo()
{
	Destroy();
}

DBObj* GfxObjDegradeInfo::Allocator()
{
	return((DBObj *)new GfxObjDegradeInfo());
}

void GfxObjDegradeInfo::Destroyer(DBObj* pGfxObjDegradeInfo)
{
	delete ((GfxObjDegradeInfo *)pGfxObjDegradeInfo);
}

GfxObjDegradeInfo *GfxObjDegradeInfo::Get(DWORD ID)
{
	return (GfxObjDegradeInfo *)ObjCaches::GfxObjDegradeInfos->Get(ID);
}

void GfxObjDegradeInfo::Release(GfxObjDegradeInfo *pGfxObjDegradeInfo)
{
	if (pGfxObjDegradeInfo)
		ObjCaches::GfxObjDegradeInfos->Release(pGfxObjDegradeInfo->GetID());
}

void GfxObjDegradeInfo::Destroy()
{
	if (degrades)
	{
		delete[] degrades;
		degrades = NULL;
	}
	num_degrades = 0;
}

BOOL GfxObjDegradeInfo::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	UNPACK(DWORD, id);
	UNPACK(DWORD, num_degrades);

	degrades = new GfxObjDegradeLevel[num_degrades];

	for (DWORD i = 0; i < num_degrades; i++)
	{
		UNPACK(DWORD, degrades[i].gfxobj_id);
		UNPACK(DWORD, degrades[i].degrade_mode);
		UNPACK(DWORD, degrades[i].min_dist);
		UNPACK(DWORD, degrades[i].ideal_dist);
		UNPACK(float, degrades[i].max_dist);
	}

	return TRUE;
}

float GfxObjDegradeInfo::get_max_degrade_distance(void) const
{
	if (num_degrades > 2)
		return degrades[num_degrades - 2].max_dist;
	else
		return degrades[0].max_dist;
}

void GfxObjDegradeInfo::get_degrade(float ViewerDist, DWORD *GfxIndex, DWORD *GfxFrameMod) const
{
	if (TRUE) // if (degrades_disabled)
	{
		*GfxIndex = 0;
		*GfxFrameMod = degrades[0].degrade_mode;
		return;
	}
}

TransitionState CGfxObj::find_obj_collisions(CTransition *transition, float scale)
{
	for (DWORD i = 0; i < transition->sphere_path.num_sphere; i++)
	{
		Vector offset = physics_sphere->center - transition->sphere_path.localspace_sphere[i].center;
		float radSum = physics_sphere->radius + transition->sphere_path.localspace_sphere[i].radius;

		if ((offset.sum_of_square() - (radSum*radSum)) < F_EPSILON)
		{
			if (transition->sphere_path.insert_type == SPHEREPATH::INITIAL_PLACEMENT_INSERT)
				return physics_bsp->placement_insert(transition);
			else
				return physics_bsp->find_collisions(transition, scale);
		}
	}

	return OK_TS;
}

bool CGfxObj::InitLoad()
{
	if (true) // DBCache::IsRunTime())
	{
#if !PHATSDK_RENDER_AVAILABLE
		if (drawing_bsp)
			drawing_bsp->RemoveNonPortalNodes();
#else
		use_built_mesh = 1;
		if (constructed_mesh || !D3DPolyRender::ConstructMesh(this, &constructed_mesh))
			use_built_mesh = 0;
#endif
	}

	return 1;
}


