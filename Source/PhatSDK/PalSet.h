
#pragma once

#include "Packable.h"
#include "ObjCache.h"

class PalSet : public DBObj, public PackObj
{
public:
	PalSet();
	virtual ~PalSet();

	static DBObj *Allocator();
	static void Destroyer(DBObj *pObject);
	static PalSet *Get(uint32_t ID);
	static void Release(PalSet *pPalSet);

	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	uint32_t GetPaletteID(double v);

	int numPals = 0;
	uint32_t *palette_IDs = NULL;
};
