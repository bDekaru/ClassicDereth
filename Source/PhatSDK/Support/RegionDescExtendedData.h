
#pragma once

class CRegionDescExtendedDataTable : public PackObj // , public PackableJson
{
public:
	CRegionDescExtendedDataTable();
	virtual ~CRegionDescExtendedDataTable();

	DECLARE_PACKABLE()
	// DECLARE_PACKABLE_JSON()

	void Destroy();

	DWORD GetEncounter(long x, long y, int index);

	DWORD _encounterTableSize = 0;
	DWORD _numTableEntries = 0;
	DWORD *_encounterTable = NULL;
	BYTE *_encounterMap = NULL;
};
