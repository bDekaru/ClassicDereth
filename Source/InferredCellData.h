
#pragma once

using encounter_list_t = PackableListWithJson<uint16_t>;
using encounter_map_t = PackableHashTableWithJson<uint32_t, encounter_list_t>;
using block_flags_map_t = PackableHashTableWithJson<uint32_t, uint32_t>;

enum LandBlockFlags
{
	AlwaysAlive = 1,
	NoLog = 2,
	NoDrop = 4
};

class CInferredCellData : public InferredData, public ThreadedFileLoader
{
public:
	void Init();
	
	CLandBlockExtendedData *GetLandBlockData(uint32_t landblock);

	//uint16_t GetTerrain(uint32_t landcell)
	//{
	//	int32_t x, y;
	//	return LandDefs::gid_to_lcoord(landcell, x, y) ? GetTerrain(landcell, x, y) : 0;
	//}

	//uint16_t GetTerrain(uint32_t landblock, int32_t cellx, int32_t celly)
	//{

	//}

	uint16_t GetEncounterIndex(uint32_t landcell)
	{
		int32_t x, y;
		return LandDefs::gid_to_lcoord(landcell, x, y) ? GetEncounterIndex(landcell >> 16, x, y) : 0;
	}
	uint16_t GetEncounterIndex(uint32_t landblock, int32_t cellx, int32_t celly)
	{
		encounter_map_t::iterator itr = _encounters.find(landblock);
		if (itr != _encounters.end())
		{
			int index = (cellx * 9) + celly;
			return *(itr->second.GetAt(index));
		}

		uint16_t terrain = _data.get_terrain(landblock << 16, cellx, celly);
		return (terrain >> 7) & 0xF;
	}

	bool RefreshLocalSpawnMapStorage();
	bool RefreshLocalSpawnMap(uint32_t landblock);
	bool FindSpawnMap(uint32_t landblock);

	bool BlockHasFlags(uint32_t blockId, LandBlockFlags flags)
	{
		block_flags_map_t::iterator itr = _blockFlags.find(blockId);
		if (itr != _blockFlags.end())
			return (itr->second & flags) == flags;

		return false;
	}

	std::list<uint32_t> GetAlwaysLoaded()
	{
		std::list<uint32_t> result;
		for (auto &entry : _blockFlags)
			if ((entry.second & LandBlockFlags::AlwaysAlive) != 0)
				result.push_back(entry.first);

		return result;
	}

protected:

	CLandBlockExtendedDataTable _data;
	CLandBlockExtendedDataTable _jsonData;

	encounter_map_t _encounters;

	block_flags_map_t _blockFlags;

	bool LoadLocalSpawnMapStorage(bool refresh = false);
	bool LoadLocalSpawnMap(uint32_t landblock);
};

