
#pragma once

#include "DArray.h"
#include "ObjCache.h"

class Palette;

#define MAX_IMAGE_TYPES 12

class ImgTexChunk
{
public:
	BYTE* m_IntPixelMap[0x4000];
	long m_iWidthPow; // 0x10000
	long m_iHeightPow; // 0x10004
	BYTE *m_pPixelData; // 0x10008
	DWORD m_iUnknown; // 0x1000C

	ImgTexChunk();
	void SetImageSize(long Width, long Height);
};

class ImgTexChunkInfo
{
public:
	ImgTexChunkInfo();
	~ImgTexChunkInfo();

	void calc_chunk_offset();

	BYTE *m_pPixelData; // m_dw4C;
	ImgTexChunk *m_pChunk; // m_dw50;
	float m_f08; // m_dw54;
	float m_f0C; // m_dw58;
};

class ImgTexManager
{
public:
	static BOOL AllocateTexture(long Width, long Height, ImgTexChunkInfo *pChunkInfo);
	static BOOL FreeTexture(ImgTexChunkInfo *pChunkInfo);

	static DWORD  num_chunks;
	static DArray<ImgTexChunk *> chunk_array;
};

class ImgTex : public DBObj
{
public:
	ImgTex();
	~ImgTex();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static ImgTex *Get(DWORD ID);

	static void releaseTexture(ImgTex* pTexture);
	static ImgTex *getCustomTexture(DWORD custom_texture_ID);
	static ImgTex *makeCustomTexture(DWORD custom_texture_ID);
	static ImgTex *makeTempTexture();

	static ImgTex *AllocateTempBuffer(long Width, long Height, long Type);
	static ImgTex *GetTempBuffer(long Width, long Height, long Type);
	static long GetPow2(long Value);

	static BOOL DoChunkification();
	static BOOL CopyIntoData(LPVOID lpData, long Width, long Height, long Pitch, DWORD arg_10, ImgTex *pSourceImg, Palette *pPalette, BOOL bUnknown);
	static BOOL CopySrcIntoDst(long sWidth, long sHeight, long sPitch, LPVOID sData, int sPSize, long dWidth, long dHeight, long dPitch, LPVOID dData, int dPSize);
	static void SetTextureLossCallback(void(*)(DWORD));

	static BOOL InitTemporaryBuffer();
	static void ReleaseTemporaryBuffer();
	static void CleanupTemporaryBuffer();

	static void releaseAllSurfaces();

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	BOOL CopyIntoTexture(long Height, long Width, long Pitch, BYTE *Data);
	BOOL Combine(ImgTex* pTex, Palette* pPal, BOOL bUnknown, long Type);
	BOOL Load(ImgTex* pSource);

	BOOL Init(long Width, long Height, long Type, BOOL InCache);
	BOOL InitIndex8(long Width, long Height, long Type);
	BOOL InitAlpha(long Width, long Height, long Type);
	BOOL InitCSI(long Width, long Height, long Type);
	BOOL InitOther(long Width, long Height, long Type, DWORD Flags, BOOL bCached);


	LPVOID GetData();


	// DX Upgrade related
	void UnlockTexture9();

#ifdef DEPRECATED_CODE
	// Deprecated (Old DX)
	void UnlockDDSurf();
	BOOL QueryTextureInterface();
#endif

	long m_lWidth; // 0x28
	long m_lHeight; // 0x2C
	long m_lPitch; // 0x30
	long m_lType; // 0x34
	BYTE *m_pPixels; // 0x38
	Palette* m_pPalette; // 0x3C
	// LPDIRECTDRAWSURFACE4 m_pDDSurf; // 0x40
#if PHATSDK_RENDER_AVAILABLE
	LPDIRECT3DTEXTURE9 m_pD3DTexture9; // 0x44
#endif
	// D3DSurfaceChunk* m_pSurfaceChunk; // 0x48
	ImgTexChunkInfo m_ChunkInfo; // 0x4C
	DWORD m_bInCache; // m_dw5C;

	static void(*textureloss_callback)(DWORD dwLoss);

	static long min_tex_size;
	static LongNIValHash<ImgTex *> texture_table; // Keyed by custom texture id
	static LongNIValHash<ImgTex *> custom_texture_table; // Keyed by texture pointer
	static LongNIValHash<ImgTex *> tiled_image_table; // Keyed by texture ID
	static LongNIValHash<ImgTex *>* temp_buffer_table[MAX_IMAGE_TYPES]; // Indexed by image type.

	static long image_use_type;
	static long RowHeights[6];
	static BOOL bSquareTexturesOnly;
};

class ImgColor : public DBObj
{
public:
	ImgColor();
	~ImgColor();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static ImgColor *Get(DWORD ID);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	DWORD m_Width;
	DWORD m_Height;
	DWORD m_Format;
	DWORD m_Length;
	BYTE *m_Data;

#if PHATSDK_RENDER_AVAILABLE
	LPDIRECT3DTEXTURE9 m_pD3DTexture9;
#endif
};
