
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

	extern uint32_t blockid_mask;
	extern uint32_t lbi_cell_id;
	extern uint32_t block_cell_id;
	extern uint32_t first_envcell_id;
	extern uint32_t last_envcell_id;
	extern uint32_t first_lcell_id;
	extern uint32_t last_lcell_id;
	extern int32_t max_block_width;
	extern uint32_t max_block_shift;
	extern uint32_t blockx_mask;
	extern uint32_t blocky_mask;
	extern uint32_t block_part_shift;
	extern uint32_t cellid_mask;
	extern uint32_t terrain_byte_offset;

	/*
	extern float Land_Height_Table[256];
	extern float inside_val;
	extern int32_t num_block_length;
	extern int32_t num_block_width;
	extern int32_t num_blocks;
	extern float square_length;
	extern float half_square_length;
	extern int32_t lblock_side;
	extern int32_t lblock_shift;
	extern int32_t lblock_mask;
	extern int32_t land_width;
	extern int32_t land_height;
	extern float block_length;
	extern int32_t last_lcell_id;
	*/

	extern int32_t side_vertex_count;
	extern float half_square_length;
	extern float square_length;
	extern float block_length;
	extern int32_t lblock_shift;
	extern int32_t lblock_side;
	extern int32_t lblock_mask;
	extern int32_t land_width;
	extern int32_t land_length;
	extern int32_t num_block_length;
	extern int32_t num_block_width;
	extern int32_t num_blocks;
	extern float inside_val;
	extern float outside_val;
	extern float max_object_height;
	extern float road_width;
	extern float sky_height;
	extern int32_t vertex_per_cell;
	extern int32_t polys_per_landcell;
	extern float Land_Height_Table[256];

	extern Vector get_block_offset(uint32_t LandBlock1, uint32_t LandBlock2);
	extern uint32_t get_block_gid(int32_t X, int32_t Y);
	extern BOOL blockid_to_lcoord(uint32_t LandBlock, int32_t& X, int32_t& Y);
	extern BOOL gid_to_lcoord(uint32_t LandCell, int32_t& X, int32_t& Y);
	extern uint32_t lcoord_to_gid(int32_t X, int32_t Y);
	extern BOOL inbound_valid_cellid(uint32_t LandCell);
	extern BOOL in_bounds(int32_t GidX, int32_t GidY);
	extern BOOL set_height_table(const float *HeightTable);
	extern BOOL adjust_to_outside(uint32_t *cellid, Vector *offset);
	extern BOOL get_outside_lcoord(uint32_t _cellid, Vector *_offset, int32_t *x, int32_t *y);

	extern BOOL init(int32_t NumBlockLength, int32_t NumBlockWidth, float SquareLength, int32_t LBlockLength, int32_t VertexPerCell, float MaxObjHeight, float SkyHeight, float RoadWidth);
};

