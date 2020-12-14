
#include <StdAfx.h>
#include "LandDefs.h"

namespace LandDefs
{
	// Initialized from beginning.
	uint32_t blockid_mask = 0xFFFF0000;
	uint32_t lbi_cell_id = 0x0000FFFE;
	uint32_t block_cell_id = 0x0000FFFF;
	uint32_t first_envcell_id = 0x100;
	uint32_t last_envcell_id = 0xFFFD;
	uint32_t first_lcell_id = 1;
	uint32_t last_lcell_id = 64;
	int32_t max_block_width = 0xFF;
	uint32_t max_block_shift = 8;
	uint32_t blockx_mask = 0xFF00;
	uint32_t blocky_mask = 0x00FF;
	uint32_t block_part_shift = 16;
	uint32_t cellid_mask = 0x0000FFFF;
	uint32_t terrain_byte_offset = 2;

	// Initialized later on.
	int32_t side_vertex_count = 9;
	float half_square_length = 12.0f;
	float square_length = 24.0f;
	float block_length = 192.0f;
	int32_t lblock_shift = 3;
	int32_t lblock_side = 8;
	int32_t lblock_mask = 7;
	int32_t land_width = 0x7F8;
	int32_t land_length = 0x7F8;
	int32_t num_block_length = 0xFF;
	int32_t num_block_width = 0xFF;
	int32_t num_blocks = 0xFF * 0xFF;
	float inside_val; // ? ....
	float outside_val;
	float max_object_height;
	float road_width;
	float sky_height;
	int32_t vertex_per_cell = 1;
	int32_t polys_per_landcell = 2;
	float Land_Height_Table[256];

	class FillHeightTable
	{
	public:
		FillHeightTable();
	};

	FillHeightTable IM_A_QUICKFIX;

	FillHeightTable::FillHeightTable()
	{
		for (int i = 0; i < 256; i++)
			Land_Height_Table[i] = (float)(i * 2);
	}

	Vector get_block_offset(uint32_t LandBlock1, uint32_t LandBlock2)
	{
		if ((LandBlock1 >> block_part_shift) == (LandBlock2 >> block_part_shift))
			return Vector(0, 0, 0);

		int32_t X1, Y1;
		int32_t X2, Y2;
		blockid_to_lcoord(LandBlock1, X1, Y1);
		blockid_to_lcoord(LandBlock2, X2, Y2);

		return Vector(
			(X2 - X1) * square_length,
			(Y2 - Y1) * square_length,
			0.0);
	}

	BOOL blockid_to_lcoord(uint32_t LandBlock, int32_t& X, int32_t& Y)
	{
		if (!LandBlock)
		{
			// PEAFIX //
			X = 0;
			Y = 0;
			// END //

			return FALSE;
		}

		X = (((LandBlock >> block_part_shift) & blockx_mask) >> max_block_shift) << lblock_shift;
		Y = (((LandBlock >> block_part_shift) & blocky_mask) << lblock_shift);

		if (X < 0 || Y < 0 || X >= land_width || Y >= land_length)
			return FALSE;

		return TRUE;
	}

	BOOL gid_to_lcoord(uint32_t LandCell, int32_t& X, int32_t& Y)
	{
		if (!inbound_valid_cellid(LandCell))
			return FALSE;

		if ((LandCell & cellid_mask) >= first_envcell_id)
			return FALSE;

		X = ((((LandCell >> block_part_shift) & blockx_mask) >> max_block_shift) << lblock_shift);
		Y = (((LandCell >> block_part_shift) & blocky_mask) << lblock_shift);

		X += ((LandCell & cellid_mask) - first_lcell_id) >> lblock_shift;
		Y += ((LandCell & cellid_mask) - first_lcell_id) & lblock_mask;

		if (X < 0 || Y < 0 || X >= land_width || Y >= land_length)
			return FALSE;

		return TRUE;
	}

	BOOL inbound_valid_cellid(uint32_t LandCell)
	{
		uint32_t CellID = LandCell & cellid_mask;

		if (((CellID >= first_lcell_id) && (CellID <= last_lcell_id)) ||
			((CellID >= first_envcell_id) && (CellID <= last_envcell_id)) ||
			((CellID == block_cell_id)))
		{
			int32_t blockx;

			blockx = ((LandCell >> block_part_shift) & blockx_mask) >> max_block_shift;
			blockx = blockx << lblock_shift;

			if (blockx >= 0 && blockx < land_width && blockx < land_length)
				return TRUE;

			return FALSE;
		}

		return FALSE;
	}

	uint32_t lcoord_to_gid(int32_t X, int32_t Y)
	{
		if (X < 0 || Y < 0 || X >= land_width || Y >= land_length)
			return 0;

		uint32_t Block = ((X >> lblock_shift) << max_block_shift) | (Y >> lblock_shift);
		uint32_t Cell = first_lcell_id + ((X & lblock_mask) << lblock_shift) + (Y & lblock_mask);

		return (uint32_t)((Block << block_part_shift) | Cell);
	}

	BOOL in_bounds(int32_t GidX, int32_t GidY)
	{
		if (GidX < 0 || GidY < 0)
			return FALSE;
		if (GidX >= land_width || GidY >= land_length)
			return FALSE;

		return TRUE;
	}

	uint32_t get_block_gid(int32_t X, int32_t Y)
	{
		if (!in_bounds(X, Y))
			return 0;

		return ((uint32_t)(((X >> lblock_shift) << max_block_shift) | (Y >> lblock_shift)) << block_part_shift) | block_cell_id;
	}

	BOOL init(int32_t NumBlockLength, int32_t NumBlockWidth, float SquareLength, int32_t LBlockSide,
		int32_t VertexPerCell, float MaxObjHeight, float SkyHeight, float RoadWidth)
	{
		if (NumBlockLength <= 0 || NumBlockLength > max_block_width)
			return FALSE;

		num_block_length = NumBlockLength;

		if (NumBlockWidth <= 0 || NumBlockWidth > max_block_width)
			return FALSE;

		num_block_width = NumBlockWidth;
		num_blocks = NumBlockWidth * NumBlockLength;

		if (!(SquareLength >= 0.1f) || !(SquareLength <= 1000.0))
			return FALSE;

		square_length = SquareLength;
		half_square_length = SquareLength * 0.5f;

		if (LBlockSide == 2)
			lblock_shift = 1;
		else
			if (LBlockSide == 4)
				lblock_shift = 2;
			else
				if (LBlockSide == 8)
					lblock_shift = 3;
				else
					if (LBlockSide == 16)
						lblock_shift = 4;
					else
						return FALSE;

		lblock_mask = LBlockSide - 1;
		lblock_side = LBlockSide;
		land_length = LBlockSide * NumBlockLength;
		land_width = LBlockSide * NumBlockWidth;
		last_lcell_id = (LBlockSide * LBlockSide) + first_lcell_id - 1;
		block_length = LBlockSide * SquareLength;

		if (VertexPerCell < 0 || VertexPerCell > 64)
			return FALSE;

		side_vertex_count = (LBlockSide * VertexPerCell) + 1;
		polys_per_landcell = (VertexPerCell * VertexPerCell) * 2;
		vertex_per_cell = VertexPerCell;

		if (!(MaxObjHeight >= 10.0f) || !(MaxObjHeight < 100000.0f))
			return FALSE;

		max_object_height = MaxObjHeight;

		if (!(SkyHeight >= 10.0f) || !(SkyHeight < 1000000.0f))
			return FALSE;

		sky_height = SkyHeight;
		outside_val = SkyHeight + 1.0f;

		if (!(RoadWidth >= 0.0) || !(RoadWidth < SquareLength))
			return FALSE;

		road_width = RoadWidth;
		return TRUE;
	}

	BOOL set_height_table(const float *HeightTable)
	{
		for (int i = 0; i < 256; i++)
		{
			float Height = HeightTable[i];

			if ((Height >= 0.0) && ((sky_height - max_object_height) >= Height))
				Land_Height_Table[i] = Height;
			else
				return FALSE;
		}

		return TRUE;
	}

	BOOL get_outside_lcoord(uint32_t cellid, Vector *_offset, int32_t *x, int32_t *y)
	{
		uint32_t cell = cellid & cellid_mask;

		if ((cell >= first_lcell_id && cell <= last_lcell_id) ||
			(cell >= first_envcell_id && cell <= last_envcell_id) ||
			(cell == block_cell_id))
		{
			blockid_to_lcoord(cellid, *x, *y);

			*x = *x + (int32_t)floor(_offset->x / square_length);
			*y = *y + (int32_t)floor(_offset->y / square_length);

			if (*x < 0 || *y < 0 || *x >= land_width || *y >= land_length)
				return FALSE;

			return TRUE;
		}
		else
			return FALSE;
	}

	BOOL adjust_to_outside(uint32_t *_cellid, Vector *_offset)
	{
		uint32_t cellid = *_cellid;

		uint32_t cell = cellid & cellid_mask;

		if ((cell >= first_lcell_id && cell <= last_lcell_id) ||
			(cell >= first_envcell_id && cell <= last_envcell_id) ||
			(cell == block_cell_id))
		{
			if (fabs(_offset->x) < F_EPSILON)
				_offset->x = 0;
			if (fabs(_offset->y) < F_EPSILON)
				_offset->y = 0;

			int32_t x, y;
			if (get_outside_lcoord(*_cellid, _offset, &x, &y))
			{
				*_cellid = lcoord_to_gid(x, y);

				_offset->x -= (floor(_offset->x / block_length) * block_length);
				_offset->y -= (floor(_offset->y / block_length) * block_length);
				return TRUE;
			}
			else
			{
				*_cellid = 0;
				return FALSE;
			}
		}
		else
		{
			*_cellid = 0;
			return FALSE;
		}
	}
}
