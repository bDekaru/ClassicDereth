
#include <StdAfx.h>
#include "Polygon.h"
#include "LandDefs.h"
#include "LandCell.h"
#include "LandBlock.h"
#include "LandBlockStruct.h"
#include "RegionDesc.h"

#pragma warning(disable: 4018)

CVertexUV CLandBlockStruct::land_uvs[4];

CLandBlockStruct::CLandBlockStruct()
{
	vertex_lighting = NULL;
	trans_dir = 9;
	side_vertex_count = 0;
	side_polygon_count = 0;
	side_cell_count = 0;
	water_type = LandDefs::WaterType::NOT_WATER;

	polygons = NULL;
	num_surface_strips = 0;
	surface_strips = NULL;
	lcell = NULL;
	SWtoNEcut = NULL;

	int NumMapVertices = LandDefs::side_vertex_count * LandDefs::side_vertex_count;
	height = new BYTE[NumMapVertices];
	terrain = new WORD[NumMapVertices];

	block_surface_index = -1;
}

void CLandBlockStruct::init()
{
	land_uvs[0].u = 0.0f;
	land_uvs[0].v = 1.0f;
	land_uvs[1].u = 1.0f;
	land_uvs[1].v = 1.0f;
	land_uvs[2].u = 1.0f;
	land_uvs[2].v = 0.0f;
	land_uvs[3].u = 0.0f;
	land_uvs[3].v = 0.0f;
}

CLandBlockStruct::~CLandBlockStruct()
{
	Destroy();

	delete[] height;
	delete[] terrain;
}

void CLandBlockStruct::Destroy()
{
	// Set all UV array pointers to NULL (so nothing tries to delete them)
	if (vertex_array.vertex_type == 1)
	{
		for (uint32_t i = 0; i < vertex_array.num_vertices; i++)
			((CVertex *)((BYTE *)vertex_array.vertices + i*CVertexArray::vertex_size))->uvarray = NULL;
	}

	if (lcell)
	{
		RemoveSurfaces();

		delete [] lcell;
		lcell = NULL;
	}

	if (polygons)
	{
		delete[] polygons;
		polygons = NULL;
	}

	vertex_array.DestroyVertex();

	if (SWtoNEcut)
	{
		delete[] SWtoNEcut;
		SWtoNEcut = NULL;
	}

	if (surface_strips)
	{
		UNFINISHED_LEGACY("CSurfaceTriStrips::Free(m_38);");
		surface_strips = NULL;
	}

	if (vertex_lighting)
	{
		delete[] vertex_lighting;
		vertex_lighting = NULL;
	}
}

void CLandBlockStruct::RemoveSurfaces()
{
	// Not done.
}

BOOL CLandBlockStruct::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	int NumMapVertices = LandDefs::side_vertex_count * LandDefs::side_vertex_count;

	// Unpack Terrain Map
	for (int i = 0; i < NumMapVertices; i++)
		UNPACK(WORD, terrain[i]);

	// Unpack Height Map
	for (int i = 0; i < NumMapVertices; i++)
		UNPACK(BYTE, height[i]);

	PACK_ALIGN();

	return TRUE;
}

BOOL CLandBlockStruct::generate(uint32_t LandBlock, uint32_t CellScale, uint32_t TransAdj)
{
	int32_t CellWidth = LandDefs::lblock_side / CellScale;

	if ((CellWidth == side_cell_count) && (trans_dir == TransAdj))
		return FALSE;

	BOOL ReGenerateCells = FALSE;

	if (CellWidth != side_cell_count)
	{
		ReGenerateCells = TRUE;

		if (side_cell_count > 0)
			Destroy();

		side_cell_count = CellWidth;

		side_polygon_count = (side_cell_count * LandDefs::vertex_per_cell);
		side_vertex_count = (side_cell_count * LandDefs::vertex_per_cell) + 1;

		InitPVArrays();
	}

	trans_dir = TransAdj;
	ConstructVertices();

	if (trans_dir && (side_cell_count > 1) && (side_cell_count < LandDefs::lblock_side))
		TransAdjust();

	if (!ReGenerateCells)
	{
		AdjPlanes();
		CalcWater();
		return FALSE;
	}
	else
	{
		ConstructPolygons(LandBlock);
		ConstructUVs(LandBlock);

		//if (RenderOptions::bUseTriangleStrips)
		//  ConstructTriangleStrips();

		CalcWater();

		return TRUE;
	}
}

void CLandBlockStruct::AdjPlanes(void)
{
	for (uint32_t x = 0; x < (unsigned)side_polygon_count; x++) {
		for (uint32_t y = 0; y < (unsigned)side_polygon_count; y++) {
			for (uint32_t poly = 0; poly < 2; poly++) {
				polygons[(((x * side_polygon_count) + y) * 2) + poly].make_plane();
			}
		}
	}
}

void CLandBlockStruct::InitPVArrays(void)
{
	int32_t NumSquares = side_polygon_count * side_polygon_count;
	int32_t NumVertices = side_vertex_count * side_vertex_count;
	int32_t NumCells = side_cell_count * side_cell_count;

	// Use D3DVertex check is here.
	vertex_array.AllocateVertex(NumVertices, 1);

	for (uint32_t x = 0; x < side_vertex_count; x++) {
		for (uint32_t y = 0; y < side_vertex_count; y++) {

			int VertIndex = ((side_vertex_count * x) + y);

			if (vertex_array.vertex_type == 2)
			{
				UNFINISHED_LEGACY("Port me please..");
			}
			else
			{
				CVertex* pVertex = (CVertex *)(((BYTE *)vertex_array.vertices) + (VertIndex * CVertexArray::vertex_size));

				pVertex->index = VertIndex;
				pVertex->uvcount = 4;
				pVertex->uvarray = land_uvs;
			}
		}
	}

	int32_t NumPolygons = NumSquares * 2;
	polygons = new CPolygon[NumPolygons];

	for (int i = 0; i < NumPolygons; i++)
	{
		polygons[i].num_pts = 3;
		polygons[i].poly_id = i;
		polygons[i].vertices = new CVertex*[3];
		polygons[i].vertex_ids = new short[3];
		polygons[i].sides_type = CullCW;

#if PHATSDK_RENDER_AVAILABLE
		if (vertex_array.vertex_type == 1)
		{
			polygons[i].pos_uv_indices = new char[3];
			polygons[i].neg_uv_indices = NULL;
			polygons[i].screen = new CVertex*[3];

			polygons[i].pos_uv_indices[0] = 0;
			polygons[i].pos_uv_indices[1] = 1;
			polygons[i].pos_uv_indices[2] = 2;
		}
#endif
	}

	SWtoNEcut = new uint32_t[NumSquares];
	lcell = new CLandCell[NumCells];
	vertex_lighting = new Vector[NumVertices];
}

void OutputVerticeInfo(CLandBlockStruct *pStruct)
{
	for (ULONG x = 0; x < pStruct->side_vertex_count; x++) {
		for (ULONG y = 0; y < pStruct->side_vertex_count; y++) {

			CVertex* pVertex = VERTEX_NUM(pStruct->vertex_array.vertices, (x * pStruct->side_vertex_count) + y);
			DEBUGOUT("CV %f %f %f\r\n", pVertex->origin.x, pVertex->origin.y, pVertex->origin.z);
		}
	}
}

void CLandBlockStruct::TransAdjust(void)
{
	if ((trans_dir == 1) || (trans_dir == 5) || (trans_dir == 7))
	{
		for (uint32_t i = 1; i < side_polygon_count; i += 2)
		{
			CVertex *pV1, *pV2, *pV3;

			pV1 = VERTEX_NUM(vertex_array.vertices, ((i - 1) * side_vertex_count) + side_polygon_count);
			pV2 = VERTEX_NUM(vertex_array.vertices, ((i + 1) * side_vertex_count) + side_polygon_count);
			pV3 = VERTEX_NUM(vertex_array.vertices, ((i)* side_vertex_count) + side_polygon_count);

			pV3->origin.z = (pV1->origin.z + pV2->origin.z) / 2;
		}
	}

	if ((trans_dir == 4) || (trans_dir == 5) || (trans_dir == 6))
	{
		for (unsigned int i = 1; i < side_polygon_count; i += 2)
		{
			CVertex *pV1, *pV2, *pV3;

			pV1 = VERTEX_NUM(vertex_array.vertices, (i - 1));
			pV2 = VERTEX_NUM(vertex_array.vertices, (i + 1));
			pV3 = VERTEX_NUM(vertex_array.vertices, (i));

			pV3->origin.z = (pV1->origin.z + pV2->origin.z) / 2;
		}
	}

	if ((trans_dir == 2) || (trans_dir == 6) || (trans_dir == 8))
	{
		for (unsigned int i = 1; i < side_polygon_count; i += 2)
		{
			CVertex *pV1, *pV2, *pV3;

			pV1 = VERTEX_NUM(vertex_array.vertices, (i - 1) * side_vertex_count);
			pV2 = VERTEX_NUM(vertex_array.vertices, (i + 1) * side_vertex_count);
			pV3 = VERTEX_NUM(vertex_array.vertices, (i)* side_vertex_count);

			pV3->origin.z = (pV1->origin.z + pV2->origin.z) / 2;
		}
	}

	if ((trans_dir == 3) || (trans_dir == 7) || (trans_dir == 8))
	{
		for (unsigned int i = 1; i < side_polygon_count; i += 2)
		{
			CVertex *pV1, *pV2, *pV3;

			uint32_t dwWhatever = (side_vertex_count * side_polygon_count) + i;

			pV1 = VERTEX_NUM(vertex_array.vertices, dwWhatever - 1);
			pV2 = VERTEX_NUM(vertex_array.vertices, dwWhatever + 1);
			pV3 = VERTEX_NUM(vertex_array.vertices, dwWhatever);

			pV3->origin.z = (pV1->origin.z + pV2->origin.z) / 2;
		}
	}

	if (side_cell_count != (LandDefs::lblock_side / 2))
		return;

	if (trans_dir == 1)
	{
		for (unsigned int i = 1, j = 4; i < side_polygon_count; i += 2, j += 4)
		{
			BYTE    bHeight;
			float*    OldZ = &VERTEX_NUM(vertex_array.vertices, i * side_vertex_count)->origin.z;

			bHeight = height[(j - 1) * LandDefs::side_vertex_count];
			float fHeight1 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(j)* LandDefs::side_vertex_count];
			float fHeight2 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(j - 3) * LandDefs::side_vertex_count];
			float fHeight3 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(j - 4) * LandDefs::side_vertex_count];
			float fHeight4 = LandDefs::Land_Height_Table[bHeight];

			float fVariantZ;

			fVariantZ = (fHeight3 + fHeight3) - fHeight4;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;

			fVariantZ = (fHeight1 + fHeight1) - fHeight2;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;
		}
	}

	if (trans_dir == 2)
	{
		for (unsigned int i = 1, j = 4; i < side_polygon_count; i += 2, j += 4)
		{
			BYTE    bHeight;
			float*    OldZ = &VERTEX_NUM(vertex_array.vertices, (i * side_vertex_count) + side_polygon_count)->origin.z;

			bHeight = height[((j - 1) * LandDefs::side_vertex_count) + LandDefs::side_vertex_count - 1];
			float fHeight1 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[((j)* LandDefs::side_vertex_count) + LandDefs::side_vertex_count - 1];
			float fHeight2 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[((j - 3) * LandDefs::side_vertex_count) + LandDefs::side_vertex_count - 1];
			float fHeight3 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[((j - 4) * LandDefs::side_vertex_count) + LandDefs::side_vertex_count - 1];
			float fHeight4 = LandDefs::Land_Height_Table[bHeight];

			float fVariantZ1 = (fHeight3 + fHeight3) - fHeight4;
			if (fVariantZ1 < (*OldZ))
				*OldZ = fVariantZ1;

			float fVariantZ2 = (fHeight1 + fHeight1) - fHeight2;
			if (fVariantZ2 < (*OldZ))
				*OldZ = fVariantZ2;
		}
	}

	if (trans_dir == 3)
	{
		for (unsigned int i = 1; i < side_polygon_count; i += 2)
		{
			BYTE    bHeight;
			float*    OldZ = &VERTEX_NUM(vertex_array.vertices, i)->origin.z;

			bHeight = height[(i * 2) + 1];
			float fHeight1 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(i * 2) + 2];
			float fHeight2 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(i * 2) - 1];
			float fHeight3 = LandDefs::Land_Height_Table[bHeight];

			bHeight = height[(i * 2) - 2];
			float fHeight4 = LandDefs::Land_Height_Table[bHeight];

			float fVariantZ;
			fVariantZ = (fHeight3 + fHeight3) - fHeight4;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;

			fVariantZ = (fHeight1 + fHeight1) - fHeight2;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;
		}
	}

	if (trans_dir == 4)
	{
		for (unsigned int i = 1; i < side_polygon_count; i += 2)
		{
			float *OldZ = &VERTEX_NUM(vertex_array.vertices, (side_vertex_count * side_polygon_count) + i)->origin.z;
			BYTE *bTrans = &height[((LandDefs::side_vertex_count - 1) * LandDefs::side_vertex_count) + (i * 2)];

			float fHeight1 = LandDefs::Land_Height_Table[bTrans[1]];
			float fHeight2 = LandDefs::Land_Height_Table[bTrans[2]];
			float fHeight3 = LandDefs::Land_Height_Table[bTrans[-1]];
			float fHeight4 = LandDefs::Land_Height_Table[bTrans[-2]];

			float fVariantZ;

			fVariantZ = (fHeight3 + fHeight3) - fHeight4;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;

			fVariantZ = (fHeight1 + fHeight1) - fHeight2;
			if (fVariantZ < (*OldZ))
				*OldZ = fVariantZ;
		}
	}
}

void CLandBlockStruct::ConstructVertices(void)
{
	uint32_t CellScale = LandDefs::lblock_side / side_cell_count;
	float PolyGridWidth = LandDefs::block_length / side_polygon_count;

	for (ULONG x = 0; x < side_vertex_count; x++) {
		float PolyGridX = (float)x * PolyGridWidth;

		for (ULONG y = 0; y < side_vertex_count; y++) {
			float PolyGridY = (float)y * PolyGridWidth;

			CVertex* pVertex = VERTEX_NUM(vertex_array.vertices, (x * side_vertex_count) + y);

			BYTE HeightTableIndex
				= height[((x * LandDefs::side_vertex_count) + y) * CellScale];

			float ZHeight = LandDefs::Land_Height_Table[HeightTableIndex];

			pVertex->origin = Vector(PolyGridX, PolyGridY, ZHeight);
		}
	}
}

int FSplitNESW(int32_t x, int32_t y)
{
	double v = ((x * y) * 0xCCAC033) - (x * 0x421BE3BD) + ((y * 0x6C1AC587) - 0x519B8F25);
	int w = (uint32_t)v / 2147483648;
	int p = ((w % 2) ? 1 : 0);
	return p;
}

void CLandBlockStruct::ConstructPolygons(uint32_t LandBlock)
{
	// if (LandBlock == 0x0000FFFF)
	//    __asm int 3;

	int32_t LCoordX, LCoordY;
	LandDefs::blockid_to_lcoord(LandBlock, LCoordX, LCoordY);

	int32_t var_4 = 214614067 * LandDefs::vertex_per_cell;
	int32_t var_8 = 1109124029 * LandDefs::vertex_per_cell;
	int32_t var_20 = 1;
	int32_t var_24 = 214614067 * (LCoordX * LandDefs::vertex_per_cell);
	int32_t var_28 = 1109124029 * (LCoordX * LandDefs::vertex_per_cell);
	int32_t var_2C = 0;
	int32_t var_10 = (LCoordY * LandDefs::vertex_per_cell);
	int32_t var_3C = LandDefs::vertex_per_cell;

	for (int x = 0; x < side_cell_count; x++) // x = var_58
	{
		int32_t var_38 = LCoordX + x;
		int32_t var_30 = 1;
		int32_t var_34 = 0;
		float var_C = var_20;

		for (int y = 0; y < side_cell_count; y++) // y = arg_0
		{
			int32_t var_14 = LCoordY + y;
			int32_t var_40 = var_28;
			int32_t var_44 = var_24 + 1813693831;

			for (int i = 0; i < LandDefs::vertex_per_cell; i++) // i = var_4C
			{
				int32_t var_ebx = var_2C + i;
				int32_t var_edi = var_34;

				for (int j = 0; j < LandDefs::vertex_per_cell; j++) // j = var_48
				{
					uint32_t var_54 = (((var_10 + var_edi) * var_44) - var_40) - 1369149221;
					uint32_t var_50 = ((LandDefs::vertex_per_cell * i) + j) * 2;

#if 0
					if (side_cell_count == 8)
					{
						if (FSplitNESW(LCoordX + x, LCoordY + y) == 0)
						{
							SWtoNEcut[(side_polygon_count * var_ebx) + var_edi] = 0;

							int32_t unk = (side_vertex_count * var_ebx) + var_edi;
							var_54 = var_ebx + 1;

							lcell[(side_cell_count * x) + y].polygons[var_50] =
								AddPolygon(
								((side_polygon_count * var_ebx) + var_edi) * 2, unk,
									(side_vertex_count * var_54) + var_edi, unk + 1);

							int32_t unk2 = (side_vertex_count * var_54) + var_edi;

							lcell[(side_cell_count * x) + y].polygons[var_50 + 1] =
								AddPolygon(
								(((side_polygon_count * var_ebx) + var_edi) * 2) + 1, unk2 + 1,
									(side_vertex_count * var_ebx) + var_edi + 1, unk2);
						}
						else
						{
							SWtoNEcut[(side_polygon_count * var_ebx) + var_edi] = 1;

							int32_t unk = (side_vertex_count * (var_ebx + 1)) + var_edi;
							var_54 = var_ebx + 1;

							lcell[(side_cell_count * x) + y].polygons[var_50] =
								AddPolygon(
								((side_polygon_count * var_ebx) + var_edi) * 2,
									(side_vertex_count * var_ebx) + var_edi,
									unk, unk + 1);

							int32_t unk2 = (side_vertex_count * var_ebx) + var_edi;

							lcell[(side_cell_count * x) + y].polygons[var_50 + 1] =
								AddPolygon(
								(((side_polygon_count * var_ebx) + var_edi) * 2) + 1, unk2,
									(side_vertex_count * var_54) + var_edi + 1, unk2 + 1);
						}
					}
					else
#endif
						if (((float)var_54 * 2.3283064e-10) < 0.5)
						{
							SWtoNEcut[(side_polygon_count * var_ebx) + var_edi] = 0;

							int32_t unk = (side_vertex_count * var_ebx) + var_edi;
							var_54 = var_ebx + 1;

							lcell[(side_cell_count * x) + y].polygons[var_50] =
								AddPolygon(
								((side_polygon_count * var_ebx) + var_edi) * 2, unk,
									(side_vertex_count * var_54) + var_edi, unk + 1);

							int32_t unk2 = (side_vertex_count * var_54) + var_edi;

							lcell[(side_cell_count * x) + y].polygons[var_50 + 1] =
								AddPolygon(
								(((side_polygon_count * var_ebx) + var_edi) * 2) + 1, unk2 + 1,
									(side_vertex_count * var_ebx) + var_edi + 1, unk2);
						}
						else
						{
							SWtoNEcut[(side_polygon_count * var_ebx) + var_edi] = 1;

							int32_t unk = (side_vertex_count * (var_ebx + 1)) + var_edi;
							var_54 = var_ebx + 1;

							lcell[(side_cell_count * x) + y].polygons[var_50] =
								AddPolygon(
								((side_polygon_count * var_ebx) + var_edi) * 2,
									(side_vertex_count * var_ebx) + var_edi,
									unk, unk + 1);

							int32_t unk2 = (side_vertex_count * var_ebx) + var_edi;

							lcell[(side_cell_count * x) + y].polygons[var_50 + 1] =
								AddPolygon(
								(((side_polygon_count * var_ebx) + var_edi) * 2) + 1, unk2,
									(side_vertex_count * var_54) + var_edi + 1, unk2 + 1);
						}

					if (FSplitNESW(LCoordX + x, LCoordY + y) == SWtoNEcut[(side_polygon_count * var_ebx) + var_edi])
					{
						// OutputDebug("Works\r\n");
					}
					else
						// This shouldn't happen?
						UNFINISHED(); //  OutputDebug("Doesn't work\r\n");

					var_edi++;
				}

				var_44 += 214614067;
				var_40 += 1109124029;
			}

			uint32_t CellID = LandDefs::lcoord_to_gid(var_38, var_14); // LandDefs::get_block_gid(var_38, var_14);

			lcell[(side_cell_count * x) + y].id = CellID;
			lcell[(side_cell_count * x) + y].pos.objcell_id = CellID;

			lcell[(side_cell_count * x) + y].pos.frame.m_origin.x =
				(var_C * LandDefs::half_square_length);
			lcell[(side_cell_count * x) + y].pos.frame.m_origin.y =
				((float)var_30 * LandDefs::half_square_length);

			var_30 += 2;
			var_34 += LandDefs::vertex_per_cell; // double check this
		}

		var_2C += LandDefs::vertex_per_cell;
		var_28 += var_8;
		var_24 += var_4;
		var_20 += 2;
	}
}

CPolygon* CLandBlockStruct::AddPolygon(int PolyIndex, int Vertex0, int Vertex1, int Vertex2)
{
	polygons[PolyIndex].vertices[0] = VERTEX_NUM(vertex_array.vertices, Vertex0);
	polygons[PolyIndex].vertex_ids[0] = Vertex0;

	polygons[PolyIndex].vertices[1] = VERTEX_NUM(vertex_array.vertices, Vertex1);
	polygons[PolyIndex].vertex_ids[1] = Vertex1;

	polygons[PolyIndex].vertices[2] = VERTEX_NUM(vertex_array.vertices, Vertex2);
	polygons[PolyIndex].vertex_ids[2] = Vertex2;

	polygons[PolyIndex].make_plane();

	polygons[PolyIndex].pos_surface = 0;

	if (polygons[PolyIndex].vertices[0]->origin.z == 0 &&
		polygons[PolyIndex].vertices[1]->origin.z == 0 &&
		polygons[PolyIndex].vertices[2]->origin.z == 0)
	{
		polygons[PolyIndex].pos_surface = 0;
	}
	else
		polygons[PolyIndex].pos_surface = 1;

	return &polygons[PolyIndex];
}

void CLandBlockStruct::ConstructUVs(uint32_t LandBlock)
{
	// Missing RenderOptions::bSingleSurfaceLScape code

	for (int x = 0; x < side_polygon_count; x++) {
		for (int y = 0; y < side_polygon_count; y++) {

			int uvset, texidx;
			GetCellRotation(LandBlock, x, y, uvset, texidx);

			if (SWtoNEcut[(x * side_polygon_count) + y] != 0)
			{
				if (vertex_array.vertex_type == 1)
				{
				}
			}
			else
			{
				if (vertex_array.vertex_type == 1)
				{
				}
			}
		}
	}

	// Missing RenderOptions::bSingleSurfaceLScape code
}

void CLandBlockStruct::GetCellRotation(uint32_t LandBlock, uint32_t x, uint32_t y, int& uvset, int& texidx)
{
	return;
#if 0
	int32_t lcx, lcy;
	LandDefs::blockid_to_lcoord(LandBlock, lcx, lcy);
	lcx += x;
	lcy += y;

	int32_t var_18 = LandDefs::lblock_side / side_cell_count;

	int PalShifted;

	if (CRegionDesc::current_region->IsPalShifted())
		PalShifted = 1;
	else
		PalShifted = ((var_18 > 1) ? 4 : 0);

	int32_t var_1C = (LandDefs::side_vertex_count * x) + y;
	uint32_t Terrain = terrain[var_1C * var_18];
	int32_t var_esi = (Terrain & 0x7F) >> LandDefs::terrain_byte_offset;
	int32_t var_30 = Terrain & (LandDefs::num_road - 1);
	int32_t var_28 = (LandDefs::side_vertex_count * (x + 1)) + y;
	uint32_t Terrain2 = terrain[var_28 * var_18];
	int32_t var_edi = (Terrain2 & 0x7F) >> LandDefs::terrain_byte_offset;
	int32_t var_34 = Terrain2 & (LandDefs::num_road - 1);
	uint32_t Terrain3 = terrain[(var_28 + 1) * var_18];
	int32_t var_ebx = (Terrain3 & 0x7F) >> LandDefs::terrain_byte_offset;
	int32_t var_38 = Terrain3 & (LandDefs::num_road - 1);
	uint32_t Terrain4 = terrain[(var_1C + 1) * var_18];
	int32_t var_ebp = (Terrain4 & 0x7F) >> LandDefs::terrain_byte_offset;
	int32_t var_28 = Terrain4 & (LandDefs::num_road - 1);

	uint32_t Palettes[4]; // var_10, var_C, var_8, var_4

	Palettes[0] = GetPalCode(PalShifted, var_30, var_34, var_38, var_28, var_esi, var_edi, var_ebx, var_ebp);
	Palettes[1] = GetPalCode(PalShifted, var_34, var_38, var_28, var_30, var_edi, var_ebx, var_ebp, var_esi);
	Palettes[2] = GetPalCode(PalShifted, var_38, var_28, var_30, var_34, var_ebx, var_ebp, var_esi, var_edi);
	Palettes[3] = GetPalCode(PalShifted, var_28, var_30, var_34, var_38, var_ebp, var_esi, var_edi, var_ebx);

	// Missing RenderOptions::bSingleSurfaceLScape code

	CRegionDesc::current_region->terrain_info->GetLandSurf()->
		SelectTerrain(lcx, lcy, uvset, texid, Palettes, PalShifted, CRegionDesc::current_region->minimize_pal);
#endif
}

void CLandBlockStruct::CalcWater()
{
	BOOL WaterBlock = TRUE; // ebp
	BOOL HasWater = FALSE; // var_C

	if (LandDefs::lblock_side == side_cell_count)
	{
		for (int x = 0; x < side_cell_count; x++) {
			for (int y = 0; y < side_cell_count; y++) {

				BOOL CellHasWater, CellFullyFlooded;
				CalcCellWater(x, y, CellHasWater, CellFullyFlooded);

				if (CellHasWater)
				{
					HasWater = TRUE;

					if (CellFullyFlooded)
						lcell[(side_cell_count * x) + y].water_type = 2;
					else
					{
						lcell[(side_cell_count * x) + y].water_type = 1;
						WaterBlock = FALSE;
					}
				}
				else
				{
					lcell[(side_cell_count * x) + y].water_type = 0;
					WaterBlock = FALSE;
				}
			}
		}
	}

	water_type = (LandDefs::WaterType)(HasWater ? (WaterBlock ? 2 : 1) : 0);
}

void CLandBlockStruct::CalcCellWater(int32_t x, int32_t y, BOOL& CellHasWater, BOOL& CellFullyFlooded)
{
	CellHasWater = FALSE;
	CellFullyFlooded = TRUE;

	for (int vx = (x * LandDefs::vertex_per_cell); vx <= ((x + 1) * LandDefs::vertex_per_cell); vx++) {
		for (int vy = (y * LandDefs::vertex_per_cell); vy <= ((y + 1) * LandDefs::vertex_per_cell); vy++)
		{
			static BOOL terrain_is_water[32] = {
				0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0
			};

			if (1 == terrain_is_water[((terrain[(LandDefs::side_vertex_count * vx) + vy] & 0x7F) >> LandDefs::terrain_byte_offset)])
				CellHasWater = TRUE;
			else
				CellFullyFlooded = FALSE;
		}
	}
}

void CLandBlockStruct::calc_lighting(void)
{
	// Not done.
	UNFINISHED();
}

SURFCHAR TERRAIN_SURF_CHAR[32] = {
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, // 16 solids
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID,
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID,
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID,

	SURFCHAR::WATER, SURFCHAR::WATER, SURFCHAR::WATER, SURFCHAR::WATER, SURFCHAR::WATER, // 5 waters

	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID,
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID,
	SURFCHAR::SOLID, SURFCHAR::SOLID, SURFCHAR::SOLID // 11 solids
};

double CLandBlockStruct::calc_water_depth(unsigned int cell_id, Vector *point)
{
	int cellX = ((cell_id - 1) & 0xFFFF) / 8;
	int cellY = ((cell_id - 1) & 0xFFFF) % 8;

	int terrainIdx = cellX * 9 + cellY;

	if (fmod(point->x, 24.0f) > 12.0f)
		terrainIdx += 9;

	if(fmod(point->y, 24.0f) > 12.0f)
		terrainIdx++;

	WORD cell_terrain = terrain[terrainIdx];
	int surfCharIdx = cell_terrain >> 2 & 0x1F;

	bool has_water = TERRAIN_SURF_CHAR[surfCharIdx];

	if (has_water)
		return 0.44999999;
	else
		return 0.1;
}








