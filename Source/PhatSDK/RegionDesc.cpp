
#include "StdAfx.h"
#include "GameTime.h"
#include "RegionDesc.h"
#include "LandDefs.h"
#include "SoundDesc.h"

#pragma warning(disable : 4150)

CRegionDesc *CRegionDesc::current_region = NULL;

CRegionDesc::CRegionDesc()
{
	region_number = NULL;
	region_name = "";

	parts_mask = 0;
	file_info = NULL;
	sky_info = NULL;
	sound_info = NULL;
	scene_info = NULL;
	terrain_info = NULL;
	encounter_info = NULL;
	region_misc = NULL;

	version = -1;
	minimize_pal = 0;
}

CRegionDesc::~CRegionDesc()
{
	Destroy();
}

DBObj* CRegionDesc::Allocator()
{
	return((DBObj *)new CRegionDesc());
}

void CRegionDesc::Destroyer(DBObj* pRegionDesc)
{
	delete ((CRegionDesc *)pRegionDesc);
}

CRegionDesc *CRegionDesc::Get(DWORD ID)
{
	return (CRegionDesc *)ObjCaches::RegionDescs->Get(ID);
}

void CRegionDesc::Release(CRegionDesc *pRegionDesc)
{
	if (pRegionDesc)
		ObjCaches::RegionDescs->Release(pRegionDesc->GetID());
}

BOOL CRegionDesc::SetRegion(DWORD Instance, BOOL Hardware)
{
	// Omits the instance checks..
	CRegionDesc *pRegion = CRegionDesc::Get(Hardware ? 0x130F0000 : 0x13000000);

	if (!pRegion)
		return FALSE;

	if (current_region)
		CRegionDesc::Release(current_region);

	current_region = pRegion;
	return TRUE;
}

void CRegionDesc::CalcDayGroup()
{
	if (CRegionDesc::current_region)
	{
		if (CRegionDesc::current_region->sky_info)
			CRegionDesc::current_region->sky_info->CalcPresentDayGroup();
	}
}

int CRegionDesc::GetSky(float time_of_day, SmartArray<CelestialPosition> *sky_pos)
{
	if (CRegionDesc::current_region && CRegionDesc::current_region->sky_info)
		return CRegionDesc::current_region->sky_info->GetSky(time_of_day, sky_pos);

	return 0;
}

CTerrainDesc::~CTerrainDesc()
{
	for (DWORD i = 0; i < terrain_types.num_used; i++)
	{
		SafeDelete(terrain_types.array_data[i]);
	}
	terrain_types.num_used = 0;

	SafeDelete (land_surfaces);
}

unsigned int CTerrainDesc::NumSceneType(unsigned int terrain_id)
{
	if (terrain_id >= terrain_types.num_used)
		return 0;

	return terrain_types.array_data[terrain_id]->scene_types.num_used;
}

unsigned int CRegionDesc::NumSceneType(unsigned int terrain_id)
{
	return terrain_info->NumSceneType(terrain_id);
}

unsigned int CTerrainDesc::SceneCount(unsigned int terrain_id, unsigned int scene_type_id)
{
	if (terrain_id >= terrain_types.num_used)
		return 0;

	if (scene_type_id >= terrain_types.array_data[terrain_id]->scene_types.num_used)
		return 0;

	if (!terrain_types.array_data[terrain_id]->scene_types.array_data[scene_type_id])
		return 0;

	return terrain_types.array_data[terrain_id]->scene_types.array_data[scene_type_id]->scenes.num_used;
}

unsigned int CRegionDesc::SceneCount(unsigned int terrain_id, unsigned int scene_type_id)
{
	return terrain_info->SceneCount(terrain_id, scene_type_id);
}

DWORD CTerrainDesc::GetScene(unsigned int terrain_id, unsigned int scene_type_id, unsigned int scene_index)
{
	CSceneType *pSceneType = terrain_types.array_data[terrain_id]->scene_types.array_data[scene_type_id];
	if (scene_index >= pSceneType->scenes.num_used)
		return 0;

	return pSceneType->scenes.array_data[scene_index];
}

DWORD CRegionDesc::GetScene(unsigned int terrain_id, unsigned int scene_type_id, unsigned int scene_index)
{
	return terrain_info->GetScene(terrain_id, scene_type_id, scene_index);
}

void CRegionDesc::Destroy(void)
{
	version = -1;

	if (file_info)
	{
		UNFINISHED_LEGACY("FileNameDesc");

		delete file_info;
		file_info = NULL;
	}

	if (sky_info)
	{
		delete sky_info;
		sky_info = NULL;
	}

	if (scene_info)
	{
		delete scene_info;
		scene_info = NULL;
	}

	if (terrain_info)
	{
		delete terrain_info;
		terrain_info = NULL;
	}

	if (region_misc)
	{
		delete region_misc;
		region_misc = NULL;
	}

	if (sound_info)
	{
		delete sound_info;
		sound_info = NULL;
	}

	if (encounter_info)
	{
		delete encounter_info;
		encounter_info = NULL;
	}
}

BOOL CRegionDesc::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	if (!UNPACK(DWORD, id))
		goto check_version;
	if (!UNPACK(DWORD, region_number))
		goto check_version;
	if (!UNPACK(DWORD, version))
		goto check_version;

check_version:
	if (version != 3)
	{
		if (version > 3)
		{
			DEBUGOUT("Error: The data files have a more recent version than the executable..\r\n");
			return FALSE;
		}
		else
		{
			DEBUGOUT("Error: The executable has a more recent version than the data files..\r\n");
			return FALSE;
		}
	}

	if (!UNPACK_OBJ(region_name))
		return FALSE;

	{
		// Close your eyes and pretend this isn't here.
		// This was probably a bug on their part, subtracts the pack length of the region
		DWORD NamePackSize = 2 + ((((DWORD)region_name.m_str.length()) >= 0xFFFF) ? 4 : 0) + ((DWORD)region_name.m_str.length());
		if (NamePackSize & 3)
			NamePackSize += 4 - (NamePackSize & 3);
		iSize -= NamePackSize;
	}

	PACK_ALIGN();

	// Excludes size checks. They were worthless anyways.

	long NumBlockLength, NumBlockWidth, LBlockLength, VertexPerCell;
	float SquareLength, MaxObjHeight, SkyHeight, RoadWidth;

	UNPACK(long, NumBlockLength);
	UNPACK(long, NumBlockWidth);
	UNPACK(float, SquareLength);
	UNPACK(long, LBlockLength);
	UNPACK(long, VertexPerCell);
	UNPACK(float, MaxObjHeight);
	UNPACK(float, SkyHeight);
	UNPACK(float, RoadWidth);

	LandDefs::init(NumBlockLength, NumBlockWidth, SquareLength, LBlockLength, VertexPerCell, MaxObjHeight, SkyHeight, RoadWidth);

	float HeightTable[256];
	for (int i = 0; i < 256; i++)
		UNPACK(float, HeightTable[i]);

	LandDefs::set_height_table(HeightTable);

	GameTime *pGameTime = new GameTime;

	if (pGameTime && UNPACK_POBJ(pGameTime))
	{
		if (GameTime::current_game_time)
			delete GameTime::current_game_time;

		GameTime::current_game_time = pGameTime;
	}

	UNPACK(DWORD, parts_mask); // 0x21F

	if (FBitSet(parts_mask, 4)) // 0x10
	{
		sky_info = new SkyDesc;
		UNPACK_OBJ_READER(*sky_info);
	}

	if (FBitSet(parts_mask, 0)) // 0x1
	{
		sound_info = new CSoundDesc;
		UNPACK_OBJ_READER(*sound_info);
	}

	if (FBitSet(parts_mask, 1)) // 0x2
	{
		scene_info = new CSceneDesc;

		BinaryReader reader(*ppData, iSize);

		DWORD num_scene_types = reader.Read<DWORD>();
		scene_info->scene_types.grow(num_scene_types);

		for (DWORD i = 0; i < num_scene_types; i++)
		{
			CSceneType *sceneType = new CSceneType();

			int stb_index = reader.Read<int>();
			sceneType->sound_table_desc = (stb_index != -1) ? sound_info->stb_desc.array_data[stb_index] : NULL;
			sceneType->UnPack(&reader);

			scene_info->scene_types.add(&sceneType);
		}

		DWORD numRead = reader.GetOffset();
		if (numRead > iSize)
			numRead = iSize;

		*ppData = *ppData + numRead;
		iSize = iSize - numRead;
	}

	{
		terrain_info = new CTerrainDesc();

		BinaryReader reader(*ppData, iSize);

		DWORD num_terrain_types = reader.Read<DWORD>();
		terrain_info->terrain_types.grow(num_terrain_types);

		for (DWORD i = 0; i < num_terrain_types; i++)
		{
			CTerrainType *terrainType = new CTerrainType();

			terrainType->terrain_name = reader.ReadString();

			{
				// Close your eyes and pretend this isn't here.
				// This was probably a bug on their part
				DWORD NamePackSize = 2 + (((terrainType->terrain_name.m_str.length()) >= 0xFFFF) ? 4 : 0) + (terrainType->terrain_name.m_str.length());
				if (NamePackSize & 3)
					NamePackSize += 4 - (NamePackSize & 3);
				iSize -= NamePackSize;
			}

			terrainType->terrain_color.color = reader.Read<DWORD>();

			DWORD num_stypes = reader.Read<DWORD>();
			terrainType->scene_types.grow(num_stypes);

			for (DWORD j = 0; j < num_stypes; j++)
			{
				int scene_index = reader.Read<int>();
				CSceneType *scene_type_value = (scene_index != -1) ? scene_info->scene_types.array_data[scene_index] : NULL;
				terrainType->scene_types.add(&scene_type_value);
			}

			terrain_info->terrain_types.add(&terrainType);
		}

		DWORD numRead = reader.GetOffset();
		if (numRead > iSize)
			numRead = iSize;

		*ppData = *ppData + numRead;
		iSize = iSize - numRead;
	}

	LandSurf *landSurf = new LandSurf();
	if (!UNPACK_OBJ_READER(*landSurf))
		return FALSE;

	terrain_info->land_surfaces = landSurf;

	region_misc = new RegionMisc();

	if (FBitSet(parts_mask, 9)) // 0x200
	{
		UNPACK_OBJ_READER(*region_misc);
	}
	else
	{
		UNFINISHED_LEGACY("RegionMisc alternate...");
	}

	// parts_mask |= parts_mask;

	return TRUE;
}

DEFINE_PACK(RegionMisc)
{
	UNFINISHED();
}

DEFINE_UNPACK(RegionMisc)
{
	version = pReader->Read<DWORD>();
	game_map = pReader->Read<DWORD>();
	autotest_map = pReader->Read<DWORD>();
	autotest_map_size = pReader->Read<DWORD>();
	clear_cell = pReader->Read<DWORD>();
	clear_monster = pReader->Read<DWORD>();

	return true;
}

CSceneDesc::CSceneDesc()
{
}

CSceneDesc::~CSceneDesc()
{
	Destroy();
}

void CSceneDesc::Destroy()
{
	for (DWORD i = 0; i < scene_types.num_used; i++)
		delete scene_types.array_data[i];
	scene_types.num_used = 0;
}

DEFINE_PACK(CSceneType)
{
	UNFINISHED();
}

DEFINE_UNPACK(CSceneType)
{
	DWORD num_scenes = pReader->Read<DWORD>();
	scenes.grow(num_scenes);

	for (DWORD i = 0; i < num_scenes; i++)
	{
		DWORD scene_type_value = pReader->Read<DWORD>();
		scenes.add(&scene_type_value);
	}

	return true;
}

LandSurf::LandSurf()
{
	pal_shift = 0;
	tex_merge = 0;
	num_lsurf = 0;
	lsurf = 0;
	num_unique_surfaces = 0;

	surf_info = new LongNIValHash<SurfInfo *>();
	curr_tex = 0;
	num_block_surfs = 0;
}

LandSurf::~LandSurf()
{
	Destroy();

	SafeDelete(surf_info);
}

void LandSurf::Destroy()
{
	if (pal_shift)
	{
		delete pal_shift;
		pal_shift = NULL;
	}

	if (tex_merge)
	{
		delete tex_merge;
		tex_merge = NULL;
	}
}

DEFINE_PACK(LandSurf)
{
	UNFINISHED();
}

DEFINE_UNPACK(LandSurf)
{
	int type = pReader->Read<int>();

	if (type)
	{
		pal_shift = new PalShift();
		pal_shift->UnPack(pReader);
	}
	else
	{
		tex_merge = new TexMerge();
		tex_merge->UnPack(pReader);
	}

	return true;
}

PalShift::PalShift()
{
}

PalShift::~PalShift()
{
	Destroy();
}

void PalShift::Destroy()
{
	for (DWORD i = 0; i < land_tex.num_used; i++)
		delete land_tex.array_data[i];
	land_tex.num_used = 0;
}

DEFINE_PACK(PalShift)
{
	UNFINISHED();
}

DEFINE_UNPACK(PalShift)
{
	DWORD numTex = pReader->Read<DWORD>();
	land_tex.grow(numTex);

	Destroy();
	for (DWORD i = 0; i < numTex; i++)
	{
		PalShiftTex *tex = new PalShiftTex();
		tex->tex_gid = 0;
		tex->UnPack(pReader);
		land_tex.add(&tex);
	}

	return true;
}

PalShiftTex::PalShiftTex()
{
}

PalShiftTex::~PalShiftTex()
{
	Destroy();
}

void PalShiftTex::Destroy()
{
	for (DWORD i = 0; i < sub_pal.num_used; i++)
		delete sub_pal.array_data[i];
	sub_pal.num_used = 0;

	for (DWORD i = 0; i < road_code.num_used; i++)
		delete road_code.array_data[i];
	road_code.num_used = 0;

	for (DWORD i = 0; i < terrain_pal.num_used; i++)
		delete terrain_pal.array_data[i];
	terrain_pal.num_used = 0;
}

DEFINE_PACK(PalShiftTex)
{
	UNFINISHED();
}

DEFINE_UNPACK(PalShiftTex)
{
	DWORD num_sub_pal = pReader->Read<DWORD>();
	sub_pal.grow(num_sub_pal);

	for (DWORD i = 0; i < num_sub_pal; i++)
	{
		PalShiftSubPal *entry = new PalShiftSubPal;
		entry->sub_pal_index = pReader->Read<DWORD>();
		entry->sub_pal_length = pReader->Read<DWORD>();
		pReader->ReadAlign();

		sub_pal.add(&entry);
	}

	DWORD num_road_code = pReader->Read<DWORD>();
	road_code.grow(num_road_code);

	for (DWORD i = 0; i < num_road_code; i++)
	{
		PalShiftRoadCode *entry = new PalShiftRoadCode;
		entry->road_code = pReader->Read<DWORD>();
		entry->sub_pal_type = new LandDefs::PalType[sub_pal.num_used];

		for (DWORD j = 0; j < sub_pal.num_used; j++)
			entry->sub_pal_type[j] = (LandDefs::PalType) pReader->Read<int>();

		road_code.add(&entry);
	}

	DWORD num_terrain_pal = pReader->Read<DWORD>();
	terrain_pal.grow(num_terrain_pal);

	for (DWORD i = 0; i < num_terrain_pal; i++)
	{
		PalShiftTerrainPal *entry = new PalShiftTerrainPal;
		entry->terrain_index = (LandDefs::TerrainType)pReader->Read<DWORD>();
		entry->pal_id = pReader->Read<DWORD>();
		pReader->ReadAlign();

		terrain_pal.add(&entry);
	}

	return true;
}

TerrainAlphaMap::TerrainAlphaMap()
{
}

TerrainAlphaMap::~TerrainAlphaMap()
{
	Destroy();
}

void TerrainAlphaMap::Destroy()
{
	// missing code here
}

RoadAlphaMap::RoadAlphaMap()
{
}

RoadAlphaMap::~RoadAlphaMap()
{
	Destroy();
}

void RoadAlphaMap::Destroy()
{
	// missing code here
}

TerrainTex::TerrainTex()
{
}

TerrainTex::~TerrainTex()
{
	Destroy();
}

void TerrainTex::Destroy()
{
	// missing code here
}

DEFINE_PACK(TerrainTex)
{
}

DEFINE_UNPACK(TerrainTex)
{
	tex_gid = pReader->Read<DWORD>();
	tex_tiling = pReader->Read<DWORD>();
	max_vert_bright = pReader->Read<DWORD>();
	min_vert_bright = pReader->Read<DWORD>();
	max_vert_saturate = pReader->Read<DWORD>();
	min_vert_saturate = pReader->Read<DWORD>();
	max_vert_hue = pReader->Read<DWORD>();
	min_vert_hue= pReader->Read<DWORD>();
	detail_tex_tiling = pReader->Read<DWORD>();
	detail_tex_gid = pReader->Read<DWORD>();
	return true;
}

TMTerrainDesc::TMTerrainDesc()
{
}

TMTerrainDesc::~TMTerrainDesc()
{
	Destroy();
}

void TMTerrainDesc::Destroy()
{
	// missing code here
	for (DWORD i = 0; i < terrain_tex.num_used; i++)
	{
		delete terrain_tex.array_data[i];
	}

	terrain_tex.num_used = 0;
}

DEFINE_PACK(TMTerrainDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(TMTerrainDesc)
{
	terrain_type = (LandDefs::TerrainType) pReader->Read<int>();

	TerrainTex *entry = new TerrainTex();
	entry->UnPack(pReader);
	terrain_tex.add(&entry);
	return true;
}

TexMerge::TexMerge()
{
}

TexMerge::~TexMerge()
{
	Destroy();
}

void TexMerge::Destroy()
{
	for (DWORD i = 0; i < corner_terrain_maps.num_used; i++)
	{
		delete corner_terrain_maps.array_data[i];
	}
	corner_terrain_maps.num_used = 0;

	for (DWORD i = 0; i < side_terrain_maps.num_used; i++)
	{
		delete side_terrain_maps.array_data[i];
	}
	side_terrain_maps.num_used = 0;

	for (DWORD i = 0; i < road_maps.num_used; i++)
	{
		delete road_maps.array_data[i];
	}
	road_maps.num_used = 0;

	for (DWORD i = 0; i < terrain_desc.num_used; i++)
	{
		delete terrain_desc.array_data[i];
	}
	terrain_desc.num_used = 0;
}

DEFINE_PACK(TexMerge)
{
}

DEFINE_UNPACK(TexMerge)
{
	base_tex_size = pReader->Read<DWORD>();

	{
		DWORD num_entries = pReader->Read<DWORD>();
		corner_terrain_maps.grow(num_entries);

		for (DWORD i = 0; i < num_entries; i++)
		{
			TerrainAlphaMap *entry = new TerrainAlphaMap();
			entry->tcode = pReader->Read<DWORD>();
			entry->tex_gid = pReader->Read<DWORD>();
			corner_terrain_maps.add(&entry);
		}
	}

	{
		DWORD num_entries = pReader->Read<DWORD>();
		side_terrain_maps.grow(num_entries);

		for (DWORD i = 0; i < num_entries; i++)
		{
			TerrainAlphaMap *entry = new TerrainAlphaMap();
			entry->tcode = pReader->Read<DWORD>();
			entry->tex_gid = pReader->Read<DWORD>();
			side_terrain_maps.add(&entry);
		}
	}

	{
		DWORD num_entries = pReader->Read<DWORD>();
		road_maps.grow(num_entries);

		for (DWORD i = 0; i < num_entries; i++)
		{
			RoadAlphaMap *entry = new RoadAlphaMap();
			entry->rcode = pReader->Read<DWORD>();
			entry->road_tex_gid = pReader->Read<DWORD>();
			road_maps.add(&entry);
		}
	}

	{
		DWORD num_entries = pReader->Read<DWORD>();
		terrain_desc.grow(num_entries);

		for (DWORD i = 0; i < num_entries; i++)
		{
			TMTerrainDesc *entry = new TMTerrainDesc();
			entry->UnPack(pReader);
			terrain_desc.add(&entry);
		}
	}

	return true;
}




