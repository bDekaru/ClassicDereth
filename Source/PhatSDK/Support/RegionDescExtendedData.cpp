
#include <StdAfx.h>
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
	//SafeDeleteArray(_encounterTable);
	//SafeDeleteArray(_encounterMap);
}

uint32_t CRegionDescExtendedDataTable::GetEncounter(int32_t x, int32_t y, int index)
{
	if (!_encounterTableSize)
		return 0;

	//uint32_t *encounterTableData = &_encounterTable[_encounterMap[(x * 255) + y] * 16];
	encounter_list_t &table = _encounterTable[_encounterMap[(x * 255) + y]];
	
	//uint32_t wcid = encounterTableData[index];
	uint32_t wcid = table[index];

	//assert(wcid != 0xFFFFFFFF);
	if (wcid == 0xFFFFFFFF)
		wcid = 0;

	return wcid;
}

DEFINE_PACK(CRegionDescExtendedDataTable)
{
	pWriter->Write<uint32_t>(_encounterTableSize);
	if (_encounterTableSize)
	{
		pWriter->Write<uint32_t>(_numTableEntries);

		uint32_t numWritten = 0;
		for (uint32_t i = 0; i < _encounterTableSize; i++)
		{
			encounter_list_t& table = _encounterTable[i];
			if (table[0] != 0xFFFFFFFF)
			{
				pWriter->Write<uint32_t>(i);
				pWriter->Write(table.data(), sizeof(uint32_t) * 16);
				numWritten++;
			}
		}

		assert(numWritten == _numTableEntries);

		pWriter->Write(_encounterMap.data(), 255 * 255);
	}
}

DEFINE_UNPACK(CRegionDescExtendedDataTable)
{
	Destroy();

	_encounterTableSize = pReader->Read<uint32_t>();
	if (_encounterTableSize)
	{
		//_encounterTable = new uint32_t[_encounterTableSize * 16];
		//memset(_encounterTable, 0xFF, sizeof(uint32_t) * _encounterTableSize * 16);

		_encounterTable.resize(_encounterTableSize);
		for (encounter_list_t &table : _encounterTable)
		{
			table.fill(0xffffffff);
		}

		_numTableEntries = pReader->Read<uint32_t>();
		for (uint32_t i = 0; i < _numTableEntries; i++)
		{
			uint32_t index = pReader->Read<uint32_t>();
			encounter_list_t &table = _encounterTable[index];

			memcpy(table.data(), pReader->ReadArray(sizeof(uint32_t) * 16), sizeof(uint32_t) * 16);
		}
		//_encounterMap = new BYTE[255 * 255];
		memcpy(_encounterMap.data(), pReader->ReadArray(sizeof(BYTE) * 255 * 255), sizeof(BYTE) * 255 * 255);
	}
	return true;
}

DEFINE_PACK_JSON(CRegionDescExtendedDataTable)
{
	writer["tableSize"] = _encounterTableSize;
	writer["tableCount"] = _numTableEntries;

	if (_encounterTableSize > 0)
	{
		json tableWriter;
		for (int i = 0; i < _encounterTableSize; i++)
		{
			encounter_list_t &table = _encounterTable[i];
			if (table[0] != 0xffffffff)
			{
				json entry;
				entry["key"] = i;
				entry["value"] = table;

				tableWriter.push_back(entry);
			}
		}
		writer["encounters"] = tableWriter;
		writer["encounterMap"] = _encounterMap;
	}
}

DEFINE_UNPACK_JSON(CRegionDescExtendedDataTable)
{
	_encounterTableSize = reader.value<uint32_t>("tableSize", 0);
	_numTableEntries = reader.value<uint32_t>("tableCount", 0);

	if (_encounterTableSize > 0)
	{
		_encounterTable.resize(_encounterTableSize);
		for (encounter_list_t &table : _encounterTable)
		{
			table.fill(0xffffffff);
		}

		json::const_iterator end = reader.end();
		json::const_iterator itr = reader.find("encounters");
		if (itr != end)
		{
			for (auto &entry : *itr)
			{
				int i = entry["key"];
				json value = entry["value"];

				encounter_list_t &table = _encounterTable[i];
				table = value;
			}
		}

		itr = reader.find("encounterMap");
		if (itr != end)
		{
			const json &encounterMap = *itr;
			_encounterMap = encounterMap;
		}
	}
	return true;
}

