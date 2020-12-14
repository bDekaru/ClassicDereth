
#pragma once

#include "MathLib.h"

class CVec2Duv
{
public:
	float u;
	float v;
};

struct CSWVertex
{
	short vert_id;
	unsigned short num_uvs;
	CVec2Duv *uvs;
	Vector normal;
};

class CVertexArray
{
public:
	CVertexArray();
	~CVertexArray();

	static void SetVertexSize(uint32_t Size);

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	BOOL AllocateVertex(uint32_t VertexCount, uint32_t VertexType);
	void DestroyVertex();

	LPVOID vertex_memory; // 0x00
	uint32_t vertex_type; // 0x04
	uint32_t num_vertices; // 0x08
	CVertex *vertices; // 0x0C

	static uint32_t vertex_size;

#define VERTEX_NUM(vertbuffer, vertindex) ((CVertex *)((BYTE *)vertbuffer + ((CVertexArray::vertex_size) * (vertindex))))
};

class CVertexUV
{
public:
	CVertexUV();

	BOOL UnPack(BYTE **ppData, ULONG iSize);

	float u, v;
};

class CVertex
{
public:
	void Init();
	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	// Origin X Y Z
	Vector origin; // 0x00 0x04 0x08

	// Identifier
	short index; // 0x0C

	// UV Array
	short uvcount; // 0x0E
	CVertexUV* uvarray; // 0x10

	// Normal X Y Z
	Vector normal; // 0x14 0x18 0x1C

	// Unknown
	//float unk20; // 0x20
	//float unk24; // 0x24
};

class BBox
{
public:
	BBox();

	void AdjustBBox(const Vector &Point);
	void LocalToLocal(const BBox *from_box, const Position *from_pos, const Position *to_pos);
	void LocalToGlobal(const BBox *from_box, const Position *from_pos, const Position *to_pos);

	Vector m_Min, m_Max;
};








