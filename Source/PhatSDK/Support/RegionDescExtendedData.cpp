
#include "stdafx.h"
#include "PhatSDK.h"

CRegionDescExtendedDataTable::CRegionDescExtendedDataTable()
{
}

CRegionDescExtendedDataTable::~CRegionDescExtendedDataTable()
{
	Destroy();
}

void CRegionDescExtendedDataTable::Destroy()
{
	_encounterTableSize = 0;
	_numTableEntries = 0;
	SafeDeleteArray(_encounterTable);
	SafeDeleteArray(_encounterMap);
}

DWORD CRegionDescExtendedDataTable::GetEncounter(long x, long y, int index)
{
	if (!_encounterTableSize)
		return 0;

	DWORD *encounterTableData = &_encounterTable[_encounterMap[(x * 255) + y] * 16];
	
	DWORD wcid = encounterTableData[index];

	//assert(wcid != 0xFFFFFFFF);
	if (wcid == 0xFFFFFFFF)
		wcid = 0;

	return wcid;
}

DEFINE_PACK(CRegionDescExtendedDataTable)
{
	pWriter->Write<DWORD>(_encounterTableSize);
	if (_encounterTableSize)
	{
		pWriter->Write<DWORD>(_numTableEntries);

		DWORD numWritten = 0;
		for (DWORD i = 0; i < _encounterTableSize; i++)
		{
			if (_encounterTable[i * 16] != 0xFFFFFFFF)
			{
				pWriter->Write<DWORD>(i);
				pWriter->Write(&_encounterTable[i * 16], sizeof(DWORD) * 16);
				numWritten++;
			}
		}

		assert(numWritten == _numTableEntries);

		pWriter->Write(_encounterMap, 255 * 255);
	}
}

DEFINE_UNPACK(CRegionDescExtendedDataTable)
{
	Destroy();
	_encounterTableSize = pReader->Read<DWORD>();
	if (_encounterTableSize)
	{
		_encounterTable = new DWORD[_encounterTableSize * 16];
		memset(_encounterTable, 0xFF, sizeof(DWORD) * _encounterTableSize * 16);

		DWORD numTableEntries = pReader->Read<DWORD>();
		for (DWORD i = 0; i < numTableEntries; i++)
		{
			DWORD index = pReader->Read<DWORD>();
			memcpy(&_encounterTable[index * 16], pReader->ReadArray(sizeof(DWORD) * 16), sizeof(DWORD) * 16);
		}

		_encounterMap = new BYTE[255 * 255];
		memcpy(_encounterMap, pReader->ReadArray(sizeof(BYTE) * 255 * 255), sizeof(BYTE) * 255 * 255);
	}
	return true;
}


/*
DEFINE_PACK_JSON(CRegionDescExtendedDataTable)
{
}

DEFINE_UNPACK_JSON(CRegionDescExtendedDataTable)
{
	return true;
}
*/
