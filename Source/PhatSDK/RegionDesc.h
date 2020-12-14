
#pragma once

#include "ObjCache.h"
#include "PString.h"
#include "Packable.h"
#include "SkyDesc.h"
#include "SoundDesc.h"
#include "SmartArray.h"
#include "LandDefs.h"
#include "Texture.h"

class CEncounterDesc;
class FileNameDesc;

class LandSurf : public PackObj
{
public:
	LandSurf();
	virtual ~LandSurf();

	void Destroy();
	DECLARE_PACKABLE();

	class PalShift *pal_shift;
	class TexMerge *tex_merge;
	LongNIValHash<class SurfInfo *> *surf_info;
	unsigned int num_lsurf;
	class CSurface **lsurf;
	unsigned int num_unique_surfaces;
	unsigned int num_block_surfs;
	SmartArray<class CSurface *> block_surf_array;
	char *curr_tex;
};

class PalShiftSubPal
{
public:
	unsigned int sub_pal_index;
	unsigned int sub_pal_length;
};

class PalShiftRoadCode
{
public:
	unsigned int road_code;
	LandDefs::PalType *sub_pal_type;
};

class PalShiftTerrainPal
{
public:
	LandDefs::TerrainType terrain_index;
	uint32_t pal_id;
};

class PalShiftTex : public PackObj
{
public:
	PalShiftTex();
	virtual ~PalShiftTex();

	void Destroy();
	DECLARE_PACKABLE();

	uint32_t tex_gid = 0; // 0
	SmartArray<class PalShiftSubPal *> sub_pal; // 4
	SmartArray<class PalShiftRoadCode *> road_code; // 0x10
	SmartArray<class PalShiftTerrainPal *> terrain_pal; // 0x1C
};

class PalShift : public PackObj
{
public:
	PalShift();
	virtual ~PalShift();

	void Destroy();
	DECLARE_PACKABLE();
	
	unsigned int cur_tex;
	SmartArray<class PalShiftTex *> land_tex;
	class Subpalette *sub_pals = NULL;
	unsigned int maxsubs;
};

class CTerrainDesc
{
public:
	~CTerrainDesc();

	unsigned int NumSceneType(unsigned int terrain_id);
	unsigned int SceneCount(unsigned int terrain_id, unsigned int scene_type_id);
	uint32_t GetScene(unsigned int terrain_id, unsigned int scene_type_id, unsigned int scene_index);

	class LandSurf *land_surfaces = NULL;
	SmartArray<class CTerrainType *> terrain_types;
};

class CTerrainType : public PackObj
{
public:
	PString terrain_name;
	RGBAUnion terrain_color;
	SmartArray<class CSceneType *> scene_types;
};

class CSceneType : public PackObj
{
public:
	DECLARE_PACKABLE();

	std::string scene_name;
	SmartArray<uint32_t> scenes;
	AmbientSTBDesc *sound_table_desc = NULL;
};

class CSceneDesc : public PackObj
{
public:
	CSceneDesc();
	virtual ~CSceneDesc();

	void Destroy();

	SmartArray<CSceneType *> scene_types;
};

class RegionMisc : public PackObj
{
public:
	DECLARE_PACKABLE();

	unsigned int version = 0;
	uint32_t game_map = 0x600127D;
	uint32_t autotest_map = 0x6000261;
	unsigned int autotest_map_size = 4;
	uint32_t clear_cell = 0;
	uint32_t clear_monster = 0;
};

class TerrainAlphaMap : public PackObj
{
public:
	TerrainAlphaMap();
	virtual ~TerrainAlphaMap();

	void Destroy();

	unsigned int tcode = 0;
	uint32_t tex_gid = 0;
	ImgTex *texture = NULL;
};

class RoadAlphaMap : public PackObj
{
public:
	RoadAlphaMap();
	virtual ~RoadAlphaMap();

	void Destroy();

	unsigned int rcode = 0;
	uint32_t road_tex_gid = 0;
	ImgTex *texture = NULL;
};

class TerrainTex : public PackObj
{
public:
	TerrainTex();
	virtual ~TerrainTex();

	void Destroy();
	DECLARE_PACKABLE();

	uint32_t tex_gid = 0;
	ImgTex *base_texture = NULL;
	float min_slope = 0.0f;
	unsigned int tex_tiling = 0;
	unsigned int max_vert_bright = 0;
	unsigned int min_vert_bright = 0;
	unsigned int max_vert_saturate = 0;
	unsigned int min_vert_saturate = 0;
	unsigned int max_vert_hue = 0;
	unsigned int min_vert_hue = 0;
	unsigned int detail_tex_tiling = 0;
	uint32_t detail_tex_gid = 0;
};

class TMTerrainDesc : public PackObj
{
public:
	TMTerrainDesc();
	virtual ~TMTerrainDesc();

	void Destroy();
	DECLARE_PACKABLE();

	LandDefs::TerrainType terrain_type = LandDefs::TerrainType::Reserved31;
	SmartArray<TerrainTex *> terrain_tex;
};

class TexMerge : public PackObj
{
public:
	TexMerge();
	virtual ~TexMerge();

	void Destroy();
	DECLARE_PACKABLE();

	unsigned int base_tex_size;
	SmartArray<TerrainAlphaMap *> corner_terrain_maps;
	SmartArray<TerrainAlphaMap *> side_terrain_maps;
	SmartArray<RoadAlphaMap *> road_maps;
	SmartArray<TMTerrainDesc *> terrain_desc;
};


class CRegionDesc : public DBObj
{
public:
	CRegionDesc();
	~CRegionDesc();

	static DBObj* Allocator();
	static void Destroyer(DBObj*);
	static CRegionDesc* Get(uint32_t ID);
	static void Release(CRegionDesc *);

	void Destroy(void);

	BOOL UnPack(BYTE **ppData, ULONG iSize);

	unsigned int NumSceneType(unsigned int terrain_id);
	unsigned int SceneCount(unsigned int terrain_id, unsigned int scene_type_id);
	uint32_t GetScene(unsigned int terrain_id, unsigned int scene_type_id, unsigned int scene_index);

	uint32_t region_number;
	PString region_name;
	uint32_t version;
	uint32_t minimize_pal;
	uint32_t parts_mask;
	FileNameDesc *file_info;
	SkyDesc *sky_info;
	CSoundDesc *sound_info;
	CSceneDesc *scene_info;
	CTerrainDesc *terrain_info;
	CEncounterDesc *encounter_info;
	RegionMisc *region_misc;

	static void CalcDayGroup();
	static int GetSky(float time_of_day, SmartArray<CelestialPosition> *sky_pos);
	static BOOL SetRegion(uint32_t Instance, BOOL Hardware);
	static CRegionDesc *current_region;
};




