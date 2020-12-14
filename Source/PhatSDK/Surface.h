
#pragma once

#include "ObjCache.h"

class Palette;
class ImgTex;

enum SurfaceHandlerEnum
{
	SH_UNKNOWN = 0x0,
	SH_DATABASE = 0x1,
	SH_PALSHIFT = 0x2,
	SH_TEXMERGE = 0x3,
	SH_CUSTOMDB = 0x4,
	NUM_SURFACE_HANDLER = 0x5,
	FORCE_SurfaceHandlerEnum_32_BIT = 0x7FFFFFFF,
};

enum SurfaceInitType
{
	SurfaceInitObjDescChange = 0x0,
	SurfaceInitLoading = 0x1,
	SurfaceInitCadding = 0x2,
	SurfaceInitRestoring = 0x4,
	FORCE_SurfaceInitType_32_BIT = 0x7FFFFFFF,
};

#define ST_TRANSLUCENT (0x00000010UL)

class CSurface : public DBObj
{
public:
	CSurface(CSurface *pSurface);
	CSurface();
	~CSurface();

	friend class SurfaceHUD;
	friend class Render;

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static CSurface *Get(uint32_t ID);
	static void Release(CSurface*);

	static CSurface *makeCustomSurface(CSurface *pSurface);
	static void releaseCustomSurface(CSurface *pSurface);
	static void releaseSurfaces();

	BOOL UnPack(BYTE* *ppData, ULONG iSize);

	uint32_t GetFlags();

	void Destroy();
	BOOL ClearSurface();
	void ReleaseTexture();
	void ReleasePalette();
	BOOL RestorePalette();
	uint32_t GetOriginalTextureMapID();
	uint32_t GetOriginalPaletteID();
	uint32_t GetTextureCode(ImgTex *pTexture, Palette *pPalette);
	void SetPalette(Palette *pPalette);
	void SetTexture(ImgTex *pTexture);
	BOOL SetTextureAndPalette(ImgTex *pTexture, Palette *pPalette);
	BOOL SetDiffuse(float Value);
	BOOL SetLuminosity(float Value);
	BOOL SetTranslucency(float Value);
	BOOL UsePalette(Palette *pPalette);
	BOOL UseTextureMap(ImgTex *pTexture, uint32_t dw2CNum);

	ImgTex *GetTexture();

	void InitEnd(SurfaceInitType init_type);

public:

	uint32_t type; // 0x1C / 0x28
	SurfaceHandlerEnum handler; // 0x20 / 0x2C
	uint32_t color_value; // 0x24 / 0x30
	uint32_t solid_index; // 0x28 / 0x34
	uint32_t indexed_texture_id; // 0x2C / 0x38
	ImgTex *base1map; // 0x30 / 0x3C
	Palette *base1pal; // 0x34 / 0x40
	float translucency; // 0x38 / 0x44
	float luminosity; // 0x3C / 0x48
	float diffuse; // 0x40 / 0x4C
	uint32_t orig_texture_id; // 0x44 / 0x50
	uint32_t orig_palette_id; // 0x48 / 0x54
	float orig_luminosity; // 0x4C / 0x58
	float orig_diffuse; // 0x50 / 0x5C

	static LongNIValHash<CSurface *> custom_surface_table;
};
