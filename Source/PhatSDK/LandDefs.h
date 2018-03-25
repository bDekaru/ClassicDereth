
#pragma once
#include "MathLib.h"

namespace LandDefs
{
	enum TerrainType
	{
		BarrenRock = 0x0,
		Grassland = 0x1,
		Ice = 0x2,
		LushGrass = 0x3,
		MarshSparseSwamp = 0x4,
		MudRichDirt = 0x5,
		ObsidianPlain = 0x6,
		PackedDirt = 0x7,
		PatchyDirt = 0x8,
		PatchyGrassland = 0x9,
		SandYellow = 0xA,
		SandGrey = 0xB,
		SandRockStrewn = 0xC,
		SedimentaryRock = 0xD,
		SemiBarrenRock = 0xE,
		Snow = 0xF,
		WaterRunning = 0x10,
		WaterStandingFresh = 0x11,
		WaterShallowSea = 0x12,
		WaterShallowStillSea = 0x13,
		WaterDeepSea = 0x14,
		Reserved21 = 0x15,
		Reserved22 = 0x16,
		Reserved23 = 0x17,
		Reserved24 = 0x18,
		Reserved25 = 0x19,
		Reserved26 = 0x1A,
		Reserved27 = 0x1B,
		Reserved28 = 0x1C,
		Reserved29 = 0x1D,
		Reserved30 = 0x1E,
		Reserved31 = 0x1F,
		RoadType = 0x20,
		FORCE_TerrainType_32_BIT = 0x7FFFFFFF,
	};

	enum PalType
	{
		SWTerrain = 0x0,
		SETerrain = 0x1,
		NETerrain = 0x2,
		NWTerrain = 0x3,
		Road = 0x4,
		FORCE_PalType_32_BIT = 0x7FFFFFFF,
	};

	enum WaterType
	{
		NOT_WATER = 0x0,
		PARTIALLY_WATER = 0x1,
		ENTIRELY_WATER = 0x2,
		FORCE_WaterType_32_BIT = 0x7FFFFFFF,
	};

	enum Rotation
	{
		ROT_0 = 0x0,
		ROT_90 = 0x1,
		ROT_180 = 0x2,
		ROT_270 = 0x3,
		FORCE_Rotation_32_BIT = 0x7FFFFFFF,
	};

	enum Direction
	{
		IN_VIEWER_BLOCK = 0x0,
		NORTH_OF_VIEWER = 0x1,
		SOUTH_OF_VIEWER = 0x2,
		EAST_OF_VIEWER = 0x3,
		WEST_OF_VIEWER = 0x4,
		NORTHWEST_OF_VIEWER = 0x5,
		SOUTHWEST_OF_VIEWER = 0x6,
		NORTHEAST_OF_VIEWER = 0x7,
		SOUTHEAST_OF_VIEWER = 0x8,
		UNKNOWN = 0x9,
		FORCE_Direction_32_BIT = 0x7FFFFFFF,
	};

	extern DWORD blockid_mask;
	extern DWORD lbi_cell_id;
	extern DWORD block_cell_id;
	extern DWORD first_envcell_id;
	extern DWORD last_envcell_id;
	extern DWORD first_lcell_id;
	extern DWORD last_lcell_id;
	extern long max_block_width;
	extern DWORD max_block_shift;
	extern DWORD blockx_mask;
	extern DWORD blocky_mask;
	extern DWORD block_part_shift;
	extern DWORD cellid_mask;
	extern DWORD terrain_byte_offset;

	/*
	extern float Land_Height_Table[256];
	extern float inside_val;
	extern long num_block_length;
	extern long num_block_width;
	extern long num_blocks;
	extern float square_length;
	extern float half_square_length;
	extern long lblock_side;
	extern long lblock_shift;
	extern long lblock_mask;
	extern long land_width;
	extern long land_height;
	extern float block_length;
	extern long last_lcell_id;
	*/

	extern long side_vertex_count;
	extern float half_square_length;
	extern float square_length;
	extern float block_length;
	extern long lblock_shift;
	extern long lblock_side;
	extern long lblock_mask;
	extern long land_width;
	extern long land_length;
	extern long num_block_length;
	extern long num_block_width;
	extern long num_blocks;
	extern float inside_val;
	extern float outside_val;
	extern float max_object_height;
	extern float road_width;
	extern float sky_height;
	extern long vertex_per_cell;
	extern long polys_per_landcell;
	extern float Land_Height_Table[256];

	extern Vector get_block_offset(DWORD LandBlock1, DWORD LandBlock2);
	extern DWORD get_block_gid(long X, long Y);
	extern BOOL blockid_to_lcoord(DWORD LandBlock, long& X, long& Y);
	extern BOOL gid_to_lcoord(DWORD LandCell, long& X, long& Y);
	extern DWORD lcoord_to_gid(long X, long Y);
	extern BOOL inbound_valid_cellid(DWORD LandCell);
	extern BOOL in_bounds(long GidX, long GidY);
	extern BOOL set_height_table(const float *HeightTable);
	extern BOOL adjust_to_outside(DWORD *cellid, Vector *offset);
	extern BOOL get_outside_lcoord(DWORD _cellid, Vector *_offset, long *x, long *y);

	extern BOOL init(long NumBlockLength, long NumBlockWidth, float SquareLength, long LBlockLength, long VertexPerCell, float MaxObjHeight, float SkyHeight, float RoadWidth);
};

