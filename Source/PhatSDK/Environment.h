
#pragma once
#include "ObjCache.h"
#include "Vertex.h"

class CSurfaceTriStrips;
class CPolygon;
class BSPTREE;

class CCellStruct
{
public:
	CCellStruct();
	~CCellStruct();

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	CPolygon* get_portal(WORD index);
	BOOL point_in_cell(const Vector& point);	
	BoundingType sphere_intersects_cell(CSphere *sphere);
	int box_intersects_cell(BBox *box);

	uint32_t cellstruct_id; // 0x00
	CVertexArray vertex_array; // 0x04
	uint32_t num_portals; // 0x14
	CPolygon **portals; // 0x18
	uint32_t num_surface_strips; // 0x1C
	CSurfaceTriStrips *surface_strips; // 0x20
	uint32_t num_polygons; // 0x24
	CPolygon *polygons; // 0x28
	BSPTREE *drawing_bsp; // 0x2C - Used for drawing
	uint32_t num_physics_polygons; // 0x30
	CPolygon *physics_polygons; // 0x34
	BSPTREE *physics_bsp; // 0x38
	BSPTREE *cell_bsp; // 0x3C
};

class CEnvironment : public DBObj
{
public:
	CEnvironment();
	~CEnvironment();

	static DBObj* Allocator();
	static void Destroyer(DBObj*);
	static CEnvironment* Get(uint32_t ID);
	static void Release(CEnvironment *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	CCellStruct* get_cellstruct(uint32_t index);

private:

	uint32_t num_cells; // 0x1C / 0x28
	CCellStruct *cells; // 0x20 / 0x2C
};




