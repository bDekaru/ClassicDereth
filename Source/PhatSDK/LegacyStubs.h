
#pragma once

class DBObj;

#define DECLARE_LEGACY_STUB(className) \
class className \
{ \
public: \
	static DBObj *Allocator(); \
	static void Destroyer(DBObj *); \
	static className *Get(uint32_t ID); \
	static void Release(className *); \
};

// DECLARE_LEGACY_STUB(CMaterial);
// DECLARE_LEGACY_STUB(ImgTex);
// DECLARE_LEGACY_STUB(Palette);
// DECLARE_LEGACY_STUB(CSurface);
// DECLARE_LEGACY_STUB(ParticleEmitterInfo);
// DECLARE_LEGACY_STUB(ImgColor);

