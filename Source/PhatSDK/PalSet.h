
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
	static PalSet *Get(DWORD ID);
	static void Release(PalSet *pPalSet);

	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	DWORD GetPaletteID(double v);

	int numPals = 0;
	DWORD *palette_IDs = NULL;
};
