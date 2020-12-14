
#include <StdAfx.h>
#include "ObjCache.h"
#include "Palette.h"

LongNIValHash< Palette* > Palette::custom_palette_table;
Palette* Palette::solid_color_palette = NULL;
uint32_t Palette::curr_solid_index = -1;

BOOL RGBColor::UnPack(BYTE **ppData, ULONG iSize)
{
	uint32_t Color;
	UNPACK(uint32_t, Color);

	SetColor32(Color);

	return TRUE;
}

void RGBColor::SetColor32(uint32_t Color)
{
	m_fRed = ((Color >> 16) & 0xFF) / 255.0f;
	m_fGreen = ((Color >> 8) & 0xFF) / 255.0f;
	m_fBlue = ((Color >> 0) & 0xFF) / 255.0f;
}

void RGBAUnion::Get(uint32_t RGBA, float *R, float *G, float *B)
{
	*R = BYTE(RGBA >> 16) / 255.0f;
	*G = BYTE(RGBA >> 8) / 255.0f;
	*B = BYTE(RGBA >> 0) / 255.0f;
}

Palette::Palette()
{
	m_dwNumPaletteColors = 0;
	m_dwNumScreenColors = 0;
	m_pScreenColors = NULL;
	m_bInCache = FALSE;
	m_pPaletteColors = NULL;

	m_dwNumScreenColors = 1;
	m_fMinimumAlpha = 0.1f;

	m_dwNumPaletteColors = 2048;

	m_pScreenColors32 = NULL; // this var was made up

	InitEnd();
}

Palette::~Palette()
{
	if (m_pPaletteColors)
	{
		delete[] m_pPaletteColors;
		m_pPaletteColors = NULL;
	}

	if (m_pScreenColors)
	{
		delete[] m_pScreenColors;
		m_pScreenColors = NULL;
	}

	if (m_pScreenColors32)
	{
		delete[] m_pScreenColors32;
		m_pScreenColors32 = NULL;
	}
}

DBObj* Palette::Allocator()
{
	return((DBObj *)new Palette());
}

void Palette::Destroyer(DBObj* pPalette)
{
	delete ((Palette *)pPalette);
}

Palette *Palette::Get(uint32_t ID)
{
	// added by pea
	if (ID == 0x04001071)
		ID = 0x04001072;

	return (Palette *)ObjCaches::Palettes->Get(ID);
}

Palette *Palette::copyRef(Palette *pPalette)
{
	if (!pPalette)
		return NULL;

	if (pPalette->m_bInCache)
	{
		if (!pPalette->GetID())
			return NULL;

		return (Palette *)ObjCaches::Palettes->Get(pPalette->GetID());
	}
	else
	{
		pPalette->Link();
		return pPalette;
	}
}

void Palette::releasePalette(Palette *pPalette)
{
	if (!pPalette)
		return;

	if (pPalette == solid_color_palette)
	{
		if (!solid_color_palette->Unlink())
		{
			// No more links exist.

			// Inlined? destroy_solid_color_palette ?
			if (solid_color_palette)
				delete solid_color_palette;

			solid_color_palette = NULL;
			curr_solid_index = -1;
		}
	}
	else
	{
		if (!pPalette->m_bInCache)
		{
			if (!pPalette->Unlink())
			{
				// No more links exist.
				Palette *pEntry = NULL;

				if (custom_palette_table.remove((uint32_t)pPalette->GetID(), &pEntry))
				{
					if (pEntry)
						delete pEntry;
				}
			}
		}
		else
			ObjCaches::Palettes->Release(pPalette->GetID());
	}
}

BOOL Palette::InitEnd()
{
	m_pPaletteColors = new uint32_t[m_dwNumPaletteColors];
	m_pScreenColors = new WORD[m_dwNumPaletteColors * m_dwNumScreenColors];
	m_pScreenColors32 = new uint32_t[m_dwNumPaletteColors * m_dwNumScreenColors];

	return TRUE;
}

Palette* Palette::get_solid_color_palette()
{
	if (!solid_color_palette)
	{
		solid_color_palette = new Palette();

		if (!solid_color_palette)
			return NULL;

		solid_color_palette->SetID(0);
		solid_color_palette->m_lLinks = 0;
		solid_color_palette->m_bInCache = FALSE;
	}

	solid_color_palette->Link();

	return solid_color_palette;
}

BOOL Palette::UnPack(BYTE **ppData, ULONG iSize)
{
	UNPACK(uint32_t, id);
	UNPACK(uint32_t, m_dwNumPaletteColors);

	for (uint32_t Index = 0; Index < m_dwNumPaletteColors; Index++)
	{
		// Load all the colors.
		uint32_t Color;
		UNPACK(uint32_t, Color);
		set_color_index(Index, Color);
	}

	m_bInCache = TRUE;

	return TRUE;
}

void Palette::set_color_index(uint32_t Index, uint32_t Color)
{
	m_pPaletteColors[Index] = Color;

	if (!m_pScreenColors)
		return;

	BYTE Red = (BYTE)(Color >> 16);
	BYTE Green = (BYTE)(Color >> 8);
	BYTE Blue = (BYTE)(Color >> 0);

	WORD ScreenColor = ((((Red & 0xF8) << 5) | (Green & 0xF8)) << 3) | (Blue >> 3);

	if (ScreenColor == 0)
		ScreenColor = 1;

	m_pScreenColors[Index] = ScreenColor;
	m_pScreenColors32[Index] = Color;
}

uint32_t Palette::get_solid_color_index()
{
	uint32_t index = ++curr_solid_index;

	if (index > 256)
		return 0;

	return index;
}

WORD Palette::get_color(uint32_t Index)
{
	// The last entries should be solid colors.
	uint32_t ScreenIndex = (m_dwNumPaletteColors * (m_dwNumScreenColors - 1)) + Index;
	return m_pScreenColors[ScreenIndex];
}

uint32_t Palette::get_color_32(uint32_t Index)
{
	// this function was made up
	uint32_t ScreenIndex = (m_dwNumPaletteColors * (m_dwNumScreenColors - 1)) + Index;
	return m_pScreenColors32[ScreenIndex];
}

