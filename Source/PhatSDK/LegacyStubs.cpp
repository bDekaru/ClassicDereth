
#include <StdAfx.h>
#include "LegacyStubs.h"

#define DEFINE_LEGACY_STUB(className) \
DBObj *className::Allocator() { return NULL; } \
void className::Destroyer(DBObj *) { } \
className *className::Get(uint32_t ID) {	return NULL; } \
void className::Release(className *) { }

// DEFINE_LEGACY_STUB(CMaterial);
// DEFINE_LEGACY_STUB(ImgTex);
// DEFINE_LEGACY_STUB(Palette);
// DEFINE_LEGACY_STUB(CSurface);
// DEFINE_LEGACY_STUB(ParticleEmitterInfo);
// DEFINE_LEGACY_STUB(ImgColor);


