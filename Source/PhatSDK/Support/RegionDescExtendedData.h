
#pragma once

class CRegionDescExtendedDataTable : public PackObj, public PackableJson
{
public:
	CRegionDescExtendedDataTable();
	virtual ~CRegionDescExtendedDataTable();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy();

	uint32_t GetEncounter(int32_t x, int32_t y, int index);

	uint32_t _encounterTableSize = 0;
	uint32_t _numTableEntries = 0;

	using encounter_list_t = std::array<uint32_t, 16>;
	using encounter_table_t = std::vector<encounter_list_t>;
	using encounter_map_t = std::array<BYTE, 255 * 255>;

	encounter_table_t _encounterTable;
	encounter_map_t _encounterMap;
	//uint32_t *_encounterTable = NULL;
	//BYTE *_encounterMap = NULL;
};
