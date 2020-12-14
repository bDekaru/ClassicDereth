
#include <StdAfx.h>
#include "GfxObj.h"
#include "Setup.h"

PlacementType::PlacementType()
{
}

PlacementType::~PlacementType()
{
}

BOOL PlacementType::UnPack(uint32_t ObjCount, BYTE **ppData, ULONG iSize)
{
	UNPACK(uint32_t, id);
	m_AnimFrame.UnPack(ObjCount, ppData, iSize);

	return TRUE;
}

LocationType::LocationType()
{
	part_id = 0;
}

LocationType::~LocationType()
{
}

BOOL LocationType::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK(uint32_t, id);
	UNPACK(uint32_t, part_id);
	UNPACK_OBJ(frame);

	return TRUE;
}

LIGHTINFO::LIGHTINFO()
{
	type = 0;
}

LIGHTINFO::~LIGHTINFO()
{
}

BOOL LIGHTINFO::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK(int, type);

	UNPACK_OBJ(offset);
	UNPACK_OBJ(color);

	UNPACK(float, intensity);
	UNPACK(float, falloff);
	UNPACK(float, cone_angle);

	return TRUE;
}

Vector LIGHTINFO::GetDirection()
{
	if (type == 1 || type == 2)
		return offset.get_vector_heading();

	return Vector(0, 0, 0);
}

CSetup::CSetup() : placement_frames(2)
{
	num_parts = 0;
	parts = NULL;
	parent_index = NULL;
	default_scale = NULL;

	num_cylsphere = 0;
	cylsphere = NULL;

	num_sphere = 0;
	sphere = NULL;

	has_physics_bsp = FALSE;
	allow_free_heading = TRUE;

	height = 0.0f;
	radius = 0.0f;
	step_down_height = 0;
	step_up_height = 0;

	sorting_sphere = CSphere(Vector(0.0f, 0.0f, 0.0f), 0.0f); // sorting_sphere = null sphere
#if PHATSDK_RENDER_AVAILABLE
	selection_sphere = CSphere(Vector(0.0f, 0.0f, 0.0f), 0.0f); // selection_sphere = null sphere
#endif

	num_lights = 0;
	lights = NULL;

	anim_scale = Vector(1.0f, 1.0f, 1.0f);

	holding_locations = NULL;
	connection_points = NULL;
}

CSetup::~CSetup()
{
	Destroy();
}

DBObj *CSetup::Allocator()
{
	return((DBObj *)new CSetup());
}

void CSetup::Destroyer(DBObj *pSetup)
{
	delete ((CSetup *)pSetup);
}

CSetup *CSetup::Get(uint32_t ID)
{
	return (CSetup *)ObjCaches::Setups->Get(ID);
}

void CSetup::Release(CSetup *pSetup)
{
	if (pSetup)
		ObjCaches::Setups->Release(pSetup->GetID());
}

void CSetup::Destroy()
{
	if (default_scale)
	{
		delete[] default_scale;
		default_scale = NULL;
	}

	if (parent_index)
	{
		delete[] parent_index;
		parent_index = NULL;
	}

	if (parts)
	{
		delete[] parts;
		parts = NULL;
	}

	num_parts = 0;

	if (cylsphere)
	{
		delete[] cylsphere;
		cylsphere = NULL;
	}

	num_cylsphere = 0;

	if (sphere)
	{
		delete[] sphere;
		sphere = NULL;
	}

	num_sphere = 0;

	if (lights)
	{
		delete[] lights;
		lights = NULL;
	}

	num_lights = 0;

	if (holding_locations)
	{
		holding_locations->destroy_contents();
		delete holding_locations;
		holding_locations = NULL;
	}

	if (connection_points)
	{
		connection_points->destroy_contents();
		delete connection_points;
		connection_points = NULL;
	}

	placement_frames.destroy_contents();

	default_anim_id = 0;
	default_script_id = 0;
	default_mtable_id = 0;
	default_stable_id = 0;
	default_phstable_id = 0;
}

CSetup *CSetup::makeParticleSetup(uint32_t ObjCount, CSphere *bounding_sphere)
{
	CSetup *pSetup = new CSetup();

	if (!pSetup)
		return NULL;

	pSetup->id = 0;
	pSetup->m_lLinks = 0;
	pSetup->num_parts = ObjCount;
	pSetup->parts = NULL;

	return pSetup;
}

CSetup *CSetup::makeSimpleSetup(uint32_t GfxObjID)
{
	CSetup *pSetup = new CSetup();

	if (!pSetup)
		return NULL;

	pSetup->id = 0;
	pSetup->m_lLinks = 0;

	if (!(pSetup->parts = new uint32_t[1]))
	{
		delete pSetup;
		return NULL;
	}

	pSetup->num_parts = 1;
	pSetup->parts[0] = GfxObjID;

	CGfxObj *pGfxObj = CGfxObj::Get(GfxObjID);
	if (pGfxObj)
	{
		if (pGfxObj->physics_sphere)
		{
			pSetup->sorting_sphere = *pGfxObj->physics_sphere;
		}
		else if (pGfxObj->drawing_sphere)
		{
			pSetup->sorting_sphere = *pGfxObj->drawing_sphere;
		}

		CGfxObj::Release(pGfxObj);
	}

	// This isn't right, since hash's aren't declared like this, but we'll use it for now.
	PlacementType *pPT = new PlacementType;

	if (!pPT)
	{
		delete pSetup;
		return NULL;
	}

	pPT->id = 0;
	pPT->m_AnimFrame.num_parts = 1;
	pPT->m_AnimFrame.frame = new AFrame[1];
	pSetup->placement_frames.add(pPT);

	return pSetup;
}

BOOL CSetup::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	BYTE bTypeID;

	UNPACK(uint32_t, id);
	UNPACK(uint32_t, bTypeID);

	allow_free_heading = (bTypeID & 4) ? TRUE : FALSE;
	has_physics_bsp = (bTypeID & 8) ? TRUE : FALSE;

	UNPACK(uint32_t, num_parts);

	if (num_parts)
	{
		parts = new uint32_t[num_parts];
		for (uint32_t i = 0; i < num_parts; i++)
			UNPACK(uint32_t, parts[i]);
	}

	if (bTypeID & 1)
	{
		parent_index = new uint32_t[num_parts];
		for (uint32_t i = 0; i < num_parts; i++)
			UNPACK(uint32_t, parent_index[i]);
	}

	if (bTypeID & 2)
	{
		default_scale = new Vector[num_parts];
		for (unsigned int i = 0; i < num_parts; i++)
			UNPACK_OBJ(default_scale[i]);
	}

	uint32_t LT94Count;
	UNPACK(uint32_t, LT94Count);

	if (LT94Count > 0)
	{
		holding_locations = new LongHash< LocationType >(4);

		for (uint32_t i = 0; i < LT94Count; i++)
		{
			LocationType *pLocation = new LocationType;
			UNPACK_POBJ(pLocation);

			holding_locations->add(pLocation);
		}
	}

	uint32_t LT98Count;
	UNPACK(uint32_t, LT98Count);

	if (LT98Count > 0)
	{
		connection_points = new LongHash< LocationType >(4);

		for (uint32_t i = 0; i < LT98Count; i++)
		{
			LocationType *pLocation = new LocationType;
			UNPACK_POBJ(pLocation);

			connection_points->add(pLocation);
		}
	}

	uint32_t PT9CCount;
	UNPACK(uint32_t, PT9CCount);

	if (PT9CCount > 0)
	{
		for (uint32_t i = 0; i < PT9CCount; i++)
		{
			PlacementType *pPlacement = new PlacementType;
			pPlacement->UnPack(num_parts, ppData, iSize);

			placement_frames.add(pPlacement);
		}
	}

	UNPACK(uint32_t, num_cylsphere);

	if (num_cylsphere > 0)
	{
		cylsphere = new CCylSphere[num_cylsphere];

		for (unsigned int i = 0; i < num_cylsphere; i++)
			UNPACK_OBJ(cylsphere[i]);
	}

	UNPACK(uint32_t, num_sphere);

	if (num_sphere > 0)
	{
		sphere = new CSphere[num_sphere];

		for (unsigned int i = 0; i < num_sphere; i++)
			sphere[i].UnPack(ppData, iSize);
	}

	UNPACK(float, height);
	UNPACK(float, radius);
	UNPACK(float, step_up_height);
	UNPACK(float, step_down_height);

	sorting_sphere.UnPack(ppData, iSize);

#if PHATSDK_RENDER_AVAILABLE
	selection_sphere.UnPack(ppData, iSize);
#else
	CSphere dummy;
	dummy.UnPack(ppData, iSize);
#endif

	UNPACK(uint32_t, num_lights);

	if (num_lights > 0)
	{
		lights = new LIGHTINFO[num_lights];

		for (unsigned int i = 0; i < num_lights; i++)
			UNPACK_OBJ(lights[i]);
	}

	UNPACK(uint32_t, default_anim_id);
	UNPACK(uint32_t, default_script_id);
	UNPACK(uint32_t, default_mtable_id);
	UNPACK(uint32_t, default_stable_id);
	UNPACK(uint32_t, default_phstable_id);

#ifdef PRE_TOD
	PACK_ALIGN();
#else
	// CFTOD: PACK_ALIGN();
#endif

	return TRUE;
}

LocationType *CSetup::GetHoldingLocation(uint32_t location_id)
{
	if (!holding_locations)
		return NULL;

	return holding_locations->lookup(location_id);
}





















