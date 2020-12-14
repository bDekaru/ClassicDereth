
#pragma once

#include "Vertex.h"
#include "LandDefs.h"

class CLandCell;
class CSurfaceTriStrips;
class CPolygon;

class CLandBlockStruct
{
public:
	CLandBlockStruct();
	~CLandBlockStruct();

	static void init();

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	CPolygon *AddPolygon(int PolyIndex, int Vertex0, int Vertex1, int Vertex2);
	void AdjPlanes(void);
	void CalcCellWater(int32_t x, int32_t y, BOOL& CellHasWater, BOOL& CellFullyFlooded);
	void CalcWater(void);
	void ConstructPolygons(uint32_t LandBlock);
	void ConstructVertices(void);
	void ConstructUVs(uint32_t LandBlock);
	void GetCellRotation(uint32_t LandBlock, uint32_t x, uint32_t y, int& uvset, int& texidx);
	void InitPVArrays(void);
	void RemoveSurfaces(void);
	void TransAdjust(void);

	void calc_lighting(void);
	BOOL generate(uint32_t LandBlock, uint32_t VertScale, uint32_t TransAdj);
	double calc_water_depth(unsigned int cell_id, Vector *point);

	Vector *vertex_lighting; // 0x00 - LightCache
	uint32_t trans_dir; // 0x04
	int32_t side_vertex_count; // 0x08
	int32_t side_polygon_count; // 0x0C
	int32_t side_cell_count; // 0x10
	LandDefs::WaterType water_type; // 0x14 - 0=none, 1=partial, 2=fully ?

	BYTE *height; // 0x18
	WORD *terrain; // 0x1C
	CVertexArray vertex_array; // 0x20
	CPolygon *polygons; // 0x30
	uint32_t num_surface_strips;
	CSurfaceTriStrips *surface_strips; // 0x38
	uint32_t block_surface_index;
	CLandCell *lcell; // 0x40
	uint32_t *SWtoNEcut; // 0x44

	static CVertexUV land_uvs[4];
};
