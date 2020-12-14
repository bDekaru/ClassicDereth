
#pragma once

#include "ObjCache.h"
#include "HashData.h"
#include "MathLib.h"
#include "Frame.h"
#include "BSPData.h"
#include "AnimHooks.h"
#include "Palette.h"

class CGfxObj;

class PlacementType : public LongHashData
{
public:
	PlacementType();
	virtual ~PlacementType();

	BOOL UnPack(uint32_t ObjCount, BYTE **ppData, ULONG iSize);

	AnimFrame m_AnimFrame;
};

class LocationType : public LongHashData
{
public:
	LocationType();
	~LocationType();

	BOOL UnPack(BYTE **ppData, ULONG iSize);

	uint32_t part_id; // 0x0C
	Frame frame; // 0x10
};

class LIGHTINFO
{
public:
	LIGHTINFO();
	~LIGHTINFO();

	BOOL UnPack(BYTE **ppData, ULONG iSize);
	Vector GetDirection();

	enum LightType
	{
		INVALID_LIGHT_TYPE = 0xFFFFFFFF,
		POINT_LIGHT = 0x0,
		DISTANT_LIGHT = 0x1,
		SPOT_LIGHT = 0x2,
		FORCE_LightType_32_BIT = 0x7FFFFFFF,
	};

	int type;
	Frame offset;
	Vector viewerspace_location;
	RGBColor color;
	float intensity;
	float falloff;
	float cone_angle;
};

class CSetup : public DBObj
{
public:
	CSetup();
	~CSetup();

	static CSetup *makeSimpleSetup(uint32_t GfxObjID);
	static CSetup *makeParticleSetup(uint32_t ObjCount, CSphere *bounding_sphere);

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static CSetup *Get(uint32_t ID);
	static void Release(CSetup *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	LocationType *GetHoldingLocation(uint32_t location_id);

	uint32_t num_parts; // 0x1C / 0x28
	uint32_t *parts; // 0x20 / 0x2C
	uint32_t *parent_index; // 0x24 / 0x30
	Vector *default_scale; // 0x28 / 0x34 

	uint32_t num_cylsphere; // 0x2C / 0x38
	CCylSphere *cylsphere; // 0x30 / 0x3C

	uint32_t num_sphere; // 0x34 / 0x40
	CSphere *sphere; // 0x38 / 0x44

	BOOL has_physics_bsp; // 0x3C / 0x48
	BOOL allow_free_heading; // 0x40 / 0x4C

	float height; // 0x44 / 0x50
	float radius; // 0x48 / 0x54
	float step_down_height; // 0x4C / 0x58
	float step_up_height; // 0x50 / 0x5C

	CSphere sorting_sphere; // 0x54 / 0x60

#if PHATSDK_RENDER_AVAILABLE
	CSphere selection_sphere; // 0x64 / 0x70
#endif

	uint32_t num_lights; // 0x74 / 0x80
	LIGHTINFO *lights; // 0x78 / 0x84

	Vector anim_scale; // 0x7C / 0x88 // not CSolid?

	LongHash<LocationType> *holding_locations; // 0x88 / 0x94
	LongHash<LocationType> *connection_points; // 0x8C / 0x98
	LongHash<PlacementType> placement_frames; // 0x90 / 0x9C

	uint32_t default_anim_id; // 0xA8 / 0xB4
	uint32_t default_script_id; // 0xAC / 0xB8
	uint32_t default_mtable_id; // 0xB0 / 0xBC
	uint32_t default_stable_id; // 0xB4 / 0xC0
	uint32_t default_phstable_id; // 0xB8 / 0xC4
};





