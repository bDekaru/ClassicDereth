
#include <StdAfx.h>
#include "Polygon.h"
#include "BSPData.h"
#include "Environment.h"

CEnvironment::CEnvironment()
{
    num_cells = 0;
    cells = NULL;
}

CEnvironment::~CEnvironment()
{
    Destroy();
}

DBObj* CEnvironment::Allocator()
{
    return((DBObj *)new CEnvironment());
}

void CEnvironment::Destroyer(DBObj* pEnvironment)
{
    delete ((CEnvironment *)pEnvironment);
}

CEnvironment *CEnvironment::Get(uint32_t ID)
{
    return (CEnvironment *)ObjCaches::Environments->Get(ID);
}

void CEnvironment::Release(CEnvironment *pEnvironment)
{
    if (pEnvironment)
        ObjCaches::Environments->Release(pEnvironment->GetID());
}

void CEnvironment::Destroy()
{
    if (cells)
    {
        delete [] cells;
        cells = NULL;
    }

    num_cells = 0;
}

BOOL CEnvironment::UnPack(BYTE **ppData, ULONG iSize)
{
    UNPACK(uint32_t, id);
    UNPACK(uint32_t, num_cells);

    cells = new CCellStruct[ num_cells ];

    for (uint32_t i = 0; i < num_cells; i++)
        UNPACK_OBJ(cells[i]);

    return TRUE;
}

CCellStruct *CEnvironment::get_cellstruct(uint32_t index)
{
    return &cells[index];
}

CCellStruct::CCellStruct()
{
    cellstruct_id = 0;

    num_portals = 0;
    portals = NULL;
    num_surface_strips = 0;
    surface_strips = NULL;
    num_polygons = 0;
    polygons = NULL;
    drawing_bsp = NULL;
    num_physics_polygons = 0;
    physics_polygons = NULL;
    physics_bsp = NULL;
    cell_bsp = NULL;
}

CCellStruct::~CCellStruct()
{
    Destroy();
}

void CCellStruct::Destroy()
{
    if (cell_bsp)
    {
        delete cell_bsp;
        cell_bsp = NULL;
    }

    if (portals)
    {
        delete [] portals;
        portals = NULL;
    }
    num_portals = 0;

    if (physics_bsp)
    {
        delete physics_bsp;
        physics_bsp = NULL;
    }

    if (physics_polygons)
    {
        delete [] physics_polygons;
        physics_polygons = NULL;
    }
    num_physics_polygons = 0;

    if (drawing_bsp)
    {
        delete drawing_bsp;
        drawing_bsp = NULL;
    }

    if (polygons)
    {
        delete [] polygons;
        polygons = NULL;
    }
    num_polygons = 0;

    if (surface_strips)
    {
		UNFINISHED();
        // delete [] surface_strips;
        surface_strips = NULL;
    }
    num_surface_strips = 0;

    vertex_array.DestroyVertex();
}

CPolygon *CCellStruct::get_portal(WORD index)
{
    for (uint32_t i = 0; i < num_portals; i++)
    {
        if (index == portals[i]->poly_id)
            return portals[i];
    }

    return NULL;
}

BOOL CCellStruct::point_in_cell(const Vector& point)
{
    return cell_bsp->point_inside_cell_bsp(point);
}

BOOL CCellStruct::UnPack(BYTE **ppData, ULONG iSize)
{
    UNPACK(uint32_t, cellstruct_id);

    UNPACK(uint32_t, num_polygons);
    UNPACK(uint32_t, num_physics_polygons);
    UNPACK(uint32_t, num_portals);

    if (!UNPACK_OBJ(vertex_array))
        return FALSE;

    CPolygon::SetPackVerts(&vertex_array);

    polygons = new CPolygon[ num_polygons ];
    for (uint32_t i = 0; i < num_polygons; i++)
        UNPACK_OBJ(polygons[i]);

    portals = new CPolygon*[ num_portals ];
    for (uint32_t i = 0; i < num_portals; i++)
    {
        uint32_t Index;
        UNPACK(WORD, Index);

        portals[i] = &polygons[Index];
    }

    PACK_ALIGN();

    BSPNODE::pack_poly = polygons;
    BSPNODE::pack_tree_type = 2;
    cell_bsp = new BSPTREE;
    UNPACK_POBJ(cell_bsp);

    physics_polygons = new CPolygon[ num_physics_polygons ];
    for (uint32_t i = 0; i < num_physics_polygons; i++)
        UNPACK_OBJ(physics_polygons[i]);

    BSPNODE::pack_poly = physics_polygons;
    BSPNODE::pack_tree_type = 1;
    physics_bsp = new BSPTREE;
    UNPACK_POBJ(physics_bsp);
    BSPNODE::pack_poly = polygons;
    
#if !PHATSDK_USE_EXTENDED_CELL_DATA
    BOOL LastField;
    UNPACK(uint32_t, LastField);

    if (LastField)
    {
        BSPNODE::pack_tree_type = 0;
        drawing_bsp = new BSPTREE;
        UNPACK_POBJ(drawing_bsp);
    }
#endif

    PACK_ALIGN();
    return TRUE;
}

BoundingType CCellStruct::sphere_intersects_cell(CSphere *sphere)
{
	return cell_bsp->sphere_intersects_cell_bsp(sphere);
}

int CCellStruct::box_intersects_cell(BBox *box)
{
	return cell_bsp->box_intersects_cell_bsp(box);
}











