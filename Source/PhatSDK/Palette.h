
#pragma once

#include "ObjCache.h"

class RGBAUnion
{
public:
	RGBAUnion() {
		r = 255;
		g = 255;
		b = 255;
		a = 255;
	}

	static void Get(uint32_t RGBA, float *R, float *G, float *B);

	union {
		uint32_t color;
		struct {
			BYTE b;
			BYTE g;
			BYTE r;
			BYTE a;
		};
	};
};

class RGBColor
{
public:
	RGBColor() { }
	RGBColor(float r, float g, float b) { m_fRed = r; m_fGreen = g; m_fBlue = b; }

	BOOL UnPack(BYTE **ppData, ULONG iSize);
	void SetColor32(uint32_t Color);

	inline RGBColor operator*(const float amount) const {
		return RGBColor(m_fRed * amount, m_fGreen * amount, m_fBlue * amount);
	}

	float m_fRed = 0.0f;
	float m_fGreen = 0.0f;
	float m_fBlue = 0.0f;
};

class Palette : public DBObj
{
public:
	Palette();
	~Palette();

	static DBObj* Allocator();
	static void Destroyer(DBObj*);
	static Palette* Get(uint32_t ID);

	static void releasePalette(Palette *pPalette);
	static Palette *copyRef(Palette *pPalette);
	static Palette *get_solid_color_palette();
	static uint32_t get_solid_color_index();

	virtual BOOL UnPack(BYTE** ppData, ULONG iSize);

	BOOL InitEnd();

	WORD get_color(uint32_t Index);
	uint32_t get_color_32(uint32_t Index); // this was made up
	void set_color_index(uint32_t Index, uint32_t Color);

	static LongNIValHash<Palette*> custom_palette_table; // 0x005E1288
	static Palette* solid_color_palette; // 0x005F3BD4;
	static uint32_t curr_solid_index;

	uint32_t m_dwNumPaletteColors; // 0x1C / 0x28
	uint32_t m_dwNumScreenColors; // 0x20 / 0x2C
	WORD* m_pScreenColors; // 0x24 / 0x30
	float m_fMinimumAlpha; // 0x28 / 0x34
	BOOL m_bInCache; // 0x2C / 0x38
	uint32_t* m_pPaletteColors; // 0x30 / 0x3C

	// this was made up:
	uint32_t* m_pScreenColors32;
};

