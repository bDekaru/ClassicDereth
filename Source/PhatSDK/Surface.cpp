
#include "StdAfx.h"
#include "Texture.h"
#include "Palette.h"
#include "Surface.h"

#pragma warning(disable: 4311) // these are valid warnings, but, yolo
#pragma warning(disable: 4302) // these are valid warnings, but, yolo

LongNIValHash<CSurface *> CSurface::custom_surface_table(128);

CSurface::CSurface()
{
	type = 0; // 0x28
	handler = SH_UNKNOWN; // 0x2C
	color_value = 0xFFFFFFFF; // 0x30
	solid_index = 0xFFFFFFFF; // 0x34
	indexed_texture_id = 0; // 0x38
	base1map = NULL; // 0x3C
	base1pal = NULL; // 0x40
	translucency = 0.0f; // 0x44
	luminosity = 0.0f; // 0x48
	diffuse = 1.0f; // 0x4C
	orig_texture_id = 0; // 0x50
	orig_palette_id = 0; // 0x54
	orig_luminosity = 0.0f; // 0x58
	orig_diffuse = 1.0f; // 0x5C
}

CSurface::CSurface(CSurface *pSurface)
{
	if (this != pSurface)
	{
		type = pSurface->type;

		if (pSurface->handler == SH_DATABASE)
			handler = SH_CUSTOMDB;
		else
			handler = pSurface->handler;

		indexed_texture_id = pSurface->indexed_texture_id;

		base1map = NULL;
		base1pal = NULL;

		color_value = pSurface->color_value;
		solid_index = pSurface->solid_index;

		translucency = pSurface->translucency;
		luminosity = pSurface->luminosity;
		diffuse = pSurface->diffuse;

		orig_texture_id = pSurface->orig_texture_id;
		orig_palette_id = pSurface->orig_palette_id;
		orig_luminosity = pSurface->orig_luminosity;
		orig_diffuse = pSurface->orig_diffuse;

		if (pSurface->base1map)
			SetTexture(pSurface->base1map);

		if (pSurface->base1pal)
			SetPalette(pSurface->base1pal);
	}
	else
	{
		DebugBreak();
	}
}

CSurface::~CSurface()
{
	Destroy();
}

DBObj *CSurface::Allocator()
{
	return((DBObj *)new CSurface());
}

void CSurface::Destroyer(DBObj *pSurface)
{
	delete ((CSurface *)pSurface);
}

CSurface *CSurface::Get(DWORD ID)
{
#if PHATSDK_LOAD_SURFACES
	return (CSurface *)ObjCaches::Surfaces->Get(ID);
#else
	return NULL;
#endif
}

void CSurface::Release(CSurface *pSurface)
{
	if (pSurface)
		ObjCaches::Surfaces->Release(pSurface->GetID());
}

CSurface *CSurface::makeCustomSurface(CSurface *pSurface)
{
	if (!pSurface)
		return NULL;

	CSurface *pCustom = new CSurface(pSurface);

	pCustom->SetID(0);
	pCustom->m_lLinks = 0;
	custom_surface_table.add(pCustom, (DWORD)pCustom);

	return pCustom;
}

void CSurface::releaseCustomSurface(CSurface *pSurface)
{
	/*
	// Don't ask.
	HashBaseData<unsigned long> *pRemoved =
	((HashBase<unsigned long> *)&custom_surface_table)->remove((DWORD)pSurface);

	if (pRemoved)
	{
	delete pRemoved;

	if (pSurface)
	delete pSurface;
	}
	*/
	if (custom_surface_table.remove((DWORD)pSurface, &pSurface))
	{
		if (pSurface)
			delete pSurface;
	}
}

void CSurface::releaseSurfaces()
{
	ImgTex::releaseAllSurfaces();
}

void CSurface::ReleaseTexture()
{
	if (base1map)
	{
		ImgTex::releaseTexture(base1map);
		base1map = NULL;
	}
}

void CSurface::ReleasePalette()
{
	if (base1pal)
	{
		Palette::releasePalette(base1pal);
		base1pal = NULL;
	}
}

void CSurface::Destroy()
{
	ReleaseTexture();
	ReleasePalette();

	color_value = 0xFFFFFFFF; // 0x30
	solid_index = 0xFFFFFFFF; // 0x34
	type = 0; // 0x28
	indexed_texture_id = 0; // 0x38
	orig_texture_id = 0; // 0x50
	orig_palette_id = 0; // 0x54
	translucency = 0.0f; // 0x44
	luminosity = 0.0f; // 0x48
	orig_luminosity = 0; // 0x58
	handler = SH_UNKNOWN; // 0x2C
	diffuse = 1.0f; // 0x4C
	orig_diffuse = 1.0f; // 0x5C
}

BOOL CSurface::ClearSurface()
{
	if (handler != 1)
		return FALSE;

	ReleaseTexture();
	ReleasePalette();

	return TRUE;
}

DWORD CSurface::GetFlags()
{
	return type;
}

ImgTex *CSurface::GetTexture()
{
	return base1map;
}

BOOL CSurface::RestorePalette()
{
	if (handler == 1)
		return FALSE;

	if (!base1map)
		return FALSE;

	SetPalette(base1map->m_pPalette);
	return TRUE;
}

void CSurface::SetPalette(Palette *pPalette)
{
	ReleasePalette();

	base1pal = Palette::copyRef(pPalette);
}

BOOL CSurface::UsePalette(Palette *pPalette)
{
	if (handler == SH_DATABASE)
		return FALSE;

	if (handler == SH_CUSTOMDB)
		handler = SH_PALSHIFT;

	if (handler != SH_PALSHIFT)
		return FALSE;

	if (!pPalette)
		return FALSE;

	SetPalette(pPalette);

	return TRUE;
}

BOOL CSurface::UseTextureMap(ImgTex *pTexture, DWORD dw2CNum)
{
	if (handler == SH_DATABASE)
		return FALSE;

	if (handler != dw2CNum)
		return FALSE;

	if (!pTexture || base1map)
		return FALSE;

	base1map = pTexture;
	type = SH_PALSHIFT;

	if (!orig_texture_id)
		orig_texture_id = base1map->GetID();

	ReleasePalette();

	return TRUE;
}

BOOL CSurface::SetDiffuse(float Value)
{
	if (handler == SH_DATABASE)
		return FALSE;

	if (Value < luminosity || Value > 1.0)
		return FALSE;

	diffuse = Value;

	return TRUE;
}

BOOL CSurface::SetLuminosity(float Value)
{
	if (handler == SH_DATABASE)
		return FALSE;

	if (Value < 0.0 || Value > 1.0)
		return FALSE;

	luminosity = Value;

	return TRUE;
}

BOOL CSurface::SetTranslucency(float Value)
{
	if (handler == SH_DATABASE)
		return FALSE;

	if (Value < 0.0 || Value > 1.0)
		return FALSE;

	translucency = Value;

	if (translucency > F_EPSILON)
		type |= ST_TRANSLUCENT;
	else
		type &= ~ST_TRANSLUCENT;

	return TRUE;
}

DWORD CSurface::GetOriginalTextureMapID()
{
	return orig_texture_id;
}

DWORD CSurface::GetOriginalPaletteID()
{
	return orig_palette_id;
}

DWORD CSurface::GetTextureCode(ImgTex *pTexture, Palette *pPalette)
{
	if (pPalette->m_bInCache)
		return ((pTexture->GetID() << 16) | (pTexture->GetID() & 0xFFFF));
	else
		return 0;
}

BOOL CSurface::UnPack(BYTE* *ppData, ULONG iSize)
{
#ifdef PRE_TOD
	UNPACK(DWORD, id);
#else
	// CFTOD: UNPACK(DWORD, m_Key)
#endif
	UNPACK(DWORD, type);

	if (GetFlags() & 6)
	{
		UNPACK(DWORD, orig_texture_id);
		UNPACK(DWORD, orig_palette_id);
	}
	else
		UNPACK(DWORD, color_value);

	UNPACK(float, translucency);
	UNPACK(float, luminosity);
	UNPACK(float, diffuse);

	orig_luminosity = luminosity;
	orig_diffuse = diffuse;
	handler = SH_DATABASE;

	InitEnd(SurfaceInitLoading);

	return TRUE;
}

void CSurface::InitEnd(SurfaceInitType init_type)
{
	DWORD TextureID = indexed_texture_id ? indexed_texture_id : orig_texture_id;

	if (!TextureID)
		ReleaseTexture();
	else
	{
		if (GetFlags() & 6)
		{
			long old_image_type = ImgTex::image_use_type;

			// Force type on this texture.
			if (GetFlags() & 4)
				ImgTex::image_use_type = 1;

			ImgTex *pTexture = ImgTex::Get(TextureID);

			// Return to old setting.
			ImgTex::image_use_type = old_image_type;

			if (pTexture)
			{
				// Replace old texture.
				ReleaseTexture();
				base1map = pTexture;

				if (pTexture->m_lType == 2)
				{
					indexed_texture_id = pTexture->GetID();

					// No surface palette? Inherit from the texture!
					if (!base1pal)
					{
						Palette *pPalette = base1map->m_pPalette;

						if (pPalette)
						{
							SetPalette(pPalette);

							if (init_type == SurfaceInitLoading || init_type == SurfaceInitCadding)
							{
								// Go ahead and init the palette ID.
								orig_palette_id = base1pal->GetID();
							}
						}
					}
				}
			}
			else
			{
				// Couldn't get ImgTex by that ID!
				if (init_type == SurfaceInitRestoring)
					ReleaseTexture();
			}
		}
	}

	if (base1map && (base1map->m_lType == 2) && (init_type != SurfaceInitCadding))
	{
		// If we have no palette, Release!
		if (!base1pal)
		{
			ReleaseTexture();
		}
		else if (!SetTextureAndPalette(base1map, base1pal))
		{
			DEBUGOUT("Failed STAP\r\n");
			ReleaseTexture();
		}
	}

	if (translucency > F_EPSILON)
		type |= ST_TRANSLUCENT;
}


void CSurface::SetTexture(ImgTex *pTexture)
{
	ReleaseTexture();

	if (pTexture)
	{
		if (pTexture->m_bInCache)
			base1map = ImgTex::Get(pTexture->GetID());
		else
			base1map = ImgTex::getCustomTexture(pTexture->GetID());
	}
}

// Textures may desire a surface palette instead.
// If that is the case, a custom texture gets created here.
BOOL CSurface::SetTextureAndPalette(ImgTex *pTexture, Palette *pPalette)
{
	handler = SH_PALSHIFT;

	ImgTex *custom_texture;
	DWORD custom_texture_ID;

	if (pPalette->m_bInCache)
		custom_texture_ID = (pTexture->GetID() << 16) | (pPalette->GetID() & 0xFFFF);
	else
		custom_texture_ID = 0;

	// Does a custom texture already exist?
	custom_texture = ImgTex::getCustomTexture(custom_texture_ID);

	if (!custom_texture)
	{
		// Ok, make one then..
		custom_texture = ImgTex::makeCustomTexture(custom_texture_ID);

		if (!custom_texture)
		{
			// Failed making texture!
			return FALSE;
		}

		// Combine.
		if (!custom_texture->Combine(pTexture, pPalette, /*Flag 4*/(type >> 2) & 1, 7))
		{
			// Failed to combine!
			ReleaseTexture();
			return FALSE;
		}
	}

	ReleaseTexture();
	base1map = custom_texture;

	return TRUE;
}
