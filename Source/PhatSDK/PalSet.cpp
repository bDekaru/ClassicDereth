
#include "StdAfx.h"
#include "PalSet.h"

PalSet::PalSet()
{
}

PalSet::~PalSet()
{
	SafeDeleteArray(palette_IDs);
}

DBObj *PalSet::Allocator()
{
	return((DBObj *)new PalSet());
}

void PalSet::Destroyer(DBObj *pPalSet)
{
	delete ((PalSet *)pPalSet);
}

PalSet *PalSet::Get(DWORD ID)
{
	return (PalSet *)ObjCaches::PalSets->Get(ID);
}

void PalSet::Release(PalSet *pPalSet)
{
	if (pPalSet)
		ObjCaches::PalSets->Release(pPalSet->GetID());
}

DEFINE_PACK(PalSet)
{
	UNFINISHED();
}

DEFINE_UNPACK(PalSet)
{
	pReader->ReadDWORD(); // file ID

	numPals = pReader->Read<DWORD>();
	palette_IDs = new DWORD[numPals];
	for (int i = 0; i < numPals; i++)
		palette_IDs[i] = pReader->Read<DWORD>();

	return true;
}

DEFINE_LEGACY_PACK_MIGRATOR(PalSet);

DWORD PalSet::GetPaletteID(double v)
{
	if (!numPals)
		return 0;
	
	if (v > 1.0)
		v = 1.0;
	if (v < 0.0)
		v = 0.0;

	return palette_IDs[(int)(((double)numPals - 0.000001) * v)];
}