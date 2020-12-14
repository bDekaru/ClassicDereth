
#pragma once

#include "DArray.h"
#include "ObjCache.h"

class Palette;

#define MAX_IMAGE_TYPES 12

class ImgTexChunk
{
public:
	BYTE* m_IntPixelMap[0x4000];
	int32_t m_iWidthPow; // 0x10000
	int32_t m_iHeightPow; // 0x10004
	BYTE *m_pPixelData; // 0x10008
	uint32_t m_iUnknown; // 0x1000C

	ImgTexChunk();
	void SetImageSize(int32_t Width, int32_t Height);
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
	static BOOL AllocateTexture(int32_t Width, int32_t Height, ImgTexChunkInfo *pChunkInfo);
	static BOOL FreeTexture(ImgTexChunkInfo *pChunkInfo);

	static uint32_t  num_chunks;
	static DArray<ImgTexChunk *> chunk_array;
};

class ImgTex : public DBObj
{
public:
	ImgTex();
	~ImgTex();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static ImgTex *Get(uint32_t ID);

	static void releaseTexture(ImgTex* pTexture);
	static ImgTex *getCustomTexture(uint32_t custom_texture_ID);
	static ImgTex *makeCustomTexture(uint32_t custom_texture_ID);
	static ImgTex *makeTempTexture();

	static ImgTex *AllocateTempBuffer(int32_t Width, int32_t Height, int32_t Type);
	static ImgTex *GetTempBuffer(int32_t Width, int32_t Height, int32_t Type);
	static int32_t GetPow2(int32_t Value);

	static BOOL DoChunkification();
	static BOOL CopyIntoData(LPVOID lpData, int32_t Width, int32_t Height, int32_t Pitch, uint32_t arg_10, ImgTex *pSourceImg, Palette *pPalette, BOOL bUnknown);
	static BOOL CopySrcIntoDst(int32_t sWidth, int32_t sHeight, int32_t sPitch, LPVOID sData, int sPSize, int32_t dWidth, int32_t dHeight, int32_t dPitch, LPVOID dData, int dPSize);
	static void SetTextureLossCallback(void(*)(uint32_t));

	static BOOL InitTemporaryBuffer();
	static void ReleaseTemporaryBuffer();
	static void CleanupTemporaryBuffer();

	static void releaseAllSurfaces();

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	BOOL CopyIntoTexture(int32_t Height, int32_t Width, int32_t Pitch, BYTE *Data);
	BOOL Combine(ImgTex* pTex, Palette* pPal, BOOL bUnknown, int32_t Type);
	BOOL Load(ImgTex* pSource);

	BOOL Init(int32_t Width, int32_t Height, int32_t Type, BOOL InCache);
	BOOL InitIndex8(int32_t Width, int32_t Height, int32_t Type);
	BOOL InitAlpha(int32_t Width, int32_t Height, int32_t Type);
	BOOL InitCSI(int32_t Width, int32_t Height, int32_t Type);
	BOOL InitOther(int32_t Width, int32_t Height, int32_t Type, uint32_t Flags, BOOL bCached);


	LPVOID GetData();


	// DX Upgrade related
	void UnlockTexture9();

#ifdef DEPRECATED_CODE
	// Deprecated (Old DX)
	void UnlockDDSurf();
	BOOL QueryTextureInterface();
#endif

	int32_t m_lWidth; // 0x28
	int32_t m_lHeight; // 0x2C
	int32_t m_lPitch; // 0x30
	int32_t m_lType; // 0x34
	BYTE *m_pPixels; // 0x38
	Palette* m_pPalette; // 0x3C
	// LPDIRECTDRAWSURFACE4 m_pDDSurf; // 0x40
#if PHATSDK_RENDER_AVAILABLE
	LPDIRECT3DTEXTURE9 m_pD3DTexture9; // 0x44
#endif
	// D3DSurfaceChunk* m_pSurfaceChunk; // 0x48
	ImgTexChunkInfo m_ChunkInfo; // 0x4C
	uint32_t m_bInCache; // m_dw5C;

	static void(*textureloss_callback)(uint32_t dwLoss);

	static int32_t min_tex_size;
	static LongNIValHash<ImgTex *> texture_table; // Keyed by custom texture id
	static LongNIValHash<ImgTex *> custom_texture_table; // Keyed by texture pointer
	static LongNIValHash<ImgTex *> tiled_image_table; // Keyed by texture ID
	static LongNIValHash<ImgTex *>* temp_buffer_table[MAX_IMAGE_TYPES]; // Indexed by image type.

	static int32_t image_use_type;
	static int32_t RowHeights[6];
	static BOOL bSquareTexturesOnly;
};

class ImgColor : public DBObj
{
public:
	ImgColor();
	~ImgColor();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static ImgColor *Get(uint32_t ID);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	uint32_t m_Width;
	uint32_t m_Height;
	uint32_t m_Format;
	uint32_t m_Length;
	BYTE *m_Data;

#if PHATSDK_RENDER_AVAILABLE
	LPDIRECT3DTEXTURE9 m_pD3DTexture9;
#endif
};
