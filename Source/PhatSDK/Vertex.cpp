
#include "StdAfx.h"
#include "ObjCache.h"
#include "Vertex.h"

DWORD CVertexArray::vertex_size = sizeof(CVertex); // 0x24;

CVertexArray::CVertexArray()
{
    vertex_memory = NULL;
    vertex_type = 0;
    num_vertices = 0;
    vertices = NULL;
}

CVertexArray::~CVertexArray()
{
}

void CVertexArray::DestroyVertex()
{
    if (vertex_type == 1)
    {
        for (DWORD i = 0; i < num_vertices; i++)
        {
            ((CVertex *)((BYTE *)vertices + i*vertex_size))->Destroy();
        }
    }
    
	delete [] ((BYTE *)vertex_memory);

    vertex_memory        = NULL;
    vertex_type    = 0;
    num_vertices    = 0;
    vertices    = NULL;
}

BOOL CVertexArray::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(DWORD, vertex_type);
    UNPACK(DWORD, num_vertices);
    
    if (!AllocateVertex(num_vertices, vertex_type))
        return FALSE;

    switch (vertex_type)
    {
    case 1:
        for (DWORD i = 0; i < num_vertices; i++)
            ((CVertex *)((BYTE *)vertices + i*vertex_size))->UnPack(ppData, iSize);

        break;
    case 2:
    case 3:
        memcpy(vertices, *ppData, num_vertices * 32);
        *ppData = *ppData + (num_vertices * 32);

        DEBUGOUT("UnPacking type %i Vertices!\r\n", vertex_type);

        break;
    }

    return TRUE;
}

BOOL CVertexArray::AllocateVertex(DWORD VertexCount, DWORD VertexType)
{
    LPVOID VertexBuffer;

	assert(sizeof(CVertex) == vertex_size);

    vertex_memory = new BYTE[ (VertexCount + 1) * vertex_size ];

    if (!vertex_memory)
        return FALSE;

    VertexBuffer = vertex_memory;

    if (vertex_size == 0x20)
    {
        // Align to 32-byte boundary.
        size_t Alignment = (size_t)VertexBuffer & 0x1F;

        if (Alignment)
        {
            VertexBuffer = (LPVOID)((size_t)VertexBuffer - Alignment);
            VertexBuffer = (LPVOID)((size_t)VertexBuffer + 0x20);
        }
    }

    vertices = (CVertex *)VertexBuffer;
    vertex_type = VertexType;
    num_vertices = VertexCount;

    if (VertexType == 1)
    {
        for (DWORD i = 0; i < num_vertices; i++)
            ((CVertex *)((BYTE *)vertices + i*vertex_size))->Init();
    }

    return TRUE;
}

void CVertex::Init()
{
    index        = 0;
    uvcount        = 0;
    uvarray        = NULL;

	/*
    if (CVertexArray::vertex_size == 0x28)
    {
        unk20 = 0;
        unk24 = 0;
    }
	*/
}

void CVertex::Destroy()
{
    if (uvarray)
    {
        delete [] uvarray;
        uvarray = NULL;
    }
}

BOOL CVertex::UnPack(BYTE **ppData, ULONG iSize)
{
    Destroy();

#if !PHATSDK_USE_EXTENDED_CELL_DATA
    UNPACK(short, index);
    UNPACK(short, uvcount);

    origin.UnPack(ppData, iSize);
    normal.UnPack(ppData, iSize);

    if (uvcount)
    {
        uvarray = new CVertexUV[ uvcount ];

        for (int i = 0; i < uvcount; i++)
            uvarray[i].UnPack(ppData, iSize);
    }
#else
	origin.UnPack(ppData, iSize);
#endif

#ifdef PRE_TOD
   PACK_ALIGN();
#else
    // CFTOD: PACK_ALIGN();
#endif

    return TRUE;
}

CVertexUV::CVertexUV()
{
}

BOOL CVertexUV::UnPack(BYTE **ppData, ULONG iSize)
{
    UNPACK(float, u);
    UNPACK(float, v);

    return TRUE;
}

BBox::BBox()
{
    m_Min = Vector(0, 0, 0);
    m_Max = Vector(0, 0, 0);
}

void BBox::AdjustBBox(const Vector& Point)
{
    if (Point.x < m_Min.x)
        m_Min.x = Point.x;
    if (Point.x > m_Max.x)
        m_Max.x = Point.x;

    if (Point.y < m_Min.y)
        m_Min.y = Point.y;
    if (Point.y > m_Max.y)
        m_Max.y = Point.y;

    if (Point.z < m_Min.z)
        m_Min.z = Point.z;
    if (Point.z > m_Max.z)
        m_Max.z = Point.z;
}

void BBox::LocalToLocal(const BBox *from_box, const Position *from_pos, const Position *to_pos)
{
	Vector v, result;
	
	result = to_pos->localtolocal(*from_pos, from_box->m_Min);
	m_Min = result;
	m_Max = result;

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);

	// 
	v.x = from_box->m_Max.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);
	//

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtolocal(*from_pos, v);
	AdjustBBox(result);
}

void BBox::LocalToGlobal(const BBox *from_box, const Position *from_pos, const Position *to_pos)
{
	Vector v, result;

	result = to_pos->localtoglobal(*from_pos, from_box->m_Min);
	m_Min = result;
	m_Max = result;

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);

	// 
	v.x = from_box->m_Max.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);
	//

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Max.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Max.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Min.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);

	v.x = from_box->m_Min.x;
	v.y = from_box->m_Min.y;
	v.z = from_box->m_Max.z;
	result = to_pos->localtoglobal(*from_pos, v);
	AdjustBBox(result);
}














