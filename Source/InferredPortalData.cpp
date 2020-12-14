#include <StdAfx.h>
#include "PhatSDK.h"
#include "InferredPortalData.h"
#include "GameEventManager.h"

CInferredPortalData::CInferredPortalData()
{
}

CInferredPortalData::~CInferredPortalData()
{
	//for (auto &entry : _mutationFilters)
	//{
	//	delete entry.second;
	//}
	_mutationFilters.clear();
}

void CInferredPortalData::Init()
{
#ifndef PUBLIC_BUILD
	SERVER_INFO << "Loading inferred portal data...";
#endif

	fs::path dataPath(g_pGlobals->GetGameData("Data", "json"));

	fs::create_directories(dataPath);

	_regionData.Destroy();

	if (!LoadJsonData(dataPath / "region.json", _regionData))
	{
		LoadCacheData(1, 0xe8b00434, 0x82092270, _regionData);
	}

	if (!LoadJsonData(dataPath / "spells.json", _spellTableData))
	{
		LoadCacheData(2, 0x5D97BAEC, 0x41675123, _spellTableData);
	}

	// Treasure Factory
	if (!LoadJsonData(dataPath / "treasureTables.json", _treasureTableData))
	{
		LoadCacheData(3, 0x7DC126EB, 0x5F41B9AD, _treasureTableData);
		//SaveJsonData(dataPath / "treasureTables.json", _treasureTableData);
	}
	
	// WieldedTreasureType DIDs
	LoadJsonData(dataPath / "wieldedTreasure.json", _treasureTableData._treasureEquipment);

	// Recipe Factory
	LoadCacheData(4, 0x5F41B9AD, 0x7DC126EB, _craftTableData);

	if (!LoadJsonData(dataPath / "housePortalDestinations.json", _housePortalDests))
	{
		LoadCacheData(5, 0x887aef9c, 0xa92ec9ac, _housePortalDests);
	}

	if (!LoadJsonData(dataPath / "quests.json", _questDefDB))
	{
		LoadCacheData(8, 0xE80D81CA, 0x8ECA9786, _questDefDB);
	}

	// Mutations
	if (!LoadJsonData(dataPath / "mutationFilters.json", _mutationFilters))
	{
		BYTE *data = NULL;
		uint32_t length = 0;
		if (LoadDataFromPhatDataBin(10, &data, &length, 0x5f1fa913, 0xe345c74c))
		{
			BinaryReader reader(data, length);

			uint32_t numEntries = reader.Read<uint32_t>();
			for (uint32_t i = 0; i < numEntries; i++)
			{
				uint32_t key = reader.Read<uint32_t>();
				uint32_t dataLength = reader.Read<uint32_t>();

				BinaryReader entryReader(reader.ReadArray(dataLength), dataLength);
				_mutationFilters[key].UnPack(&entryReader);
			}

			delete[] data;

			//SaveJsonData(dataPath / "mutationFilters.json", _mutationFilters);
		}
	}

	if (!LoadJsonData(dataPath / "events.json", _gameEvents))
	{
		LoadCacheData(11, 0x812a7823, 0x8b28e107, _gameEvents);
	}

	std::vector<std::string> &words = _bannedwords;
	LoadJsonData(dataPath / "bannedwords.json", [&words](json &data)
	{
		words = data.at("badwords").get<std::vector<std::string>>();
	});

	//std::set<uint32_t> &restrictions = _restrictedLBData;
	//LoadJsonData(dataPath / "restrictedlandblocks.json", [&restrictions](json &data)
	//{
	//	restrictions = data.at("restrictedlandblocks").get<std::set<uint32_t>>();
	//});


#ifndef PUBLIC_BUILD
	SERVER_INFO << "Finished loading inferred cell data.";
#endif
}

uint32_t CInferredPortalData::GetWCIDForTerrain(int32_t x, int32_t y, int index)
{
	return _regionData.GetEncounter(x, y, index);
}

CSpellTableEx *CInferredPortalData::GetSpellTableEx()
{
	return &_spellTableData._table;
}

CCraftOperation *CInferredPortalData::GetCraftOperation(uint32_t source_wcid, uint32_t dest_wcid)
{
	uint64_t toolComboKey = ((uint64_t)source_wcid << 32) | dest_wcid;
	const uint32_t *opKey = g_pPortalDataEx->_craftTableData._precursorMap.lookup(toolComboKey);

	if (!opKey)
		return NULL;

	return g_pPortalDataEx->_craftTableData._operations.lookup(*opKey);
}

Position *CInferredPortalData::GetHousePortalDest(uint32_t house_id, uint32_t ignore_cell_id)
{
	house_portal_table_t::mapped_type *dests = _housePortalDests.lookup(house_id);

	if (dests)
	{
		for (auto &entry : *dests)
		{
			if (entry.objcell_id != ignore_cell_id)
				return &entry;
		}
	}

	return NULL;
}

CMutationFilter *CInferredPortalData::GetMutationFilter(uint32_t id)
{
	mutation_table_t::iterator entry = _mutationFilters.find(id & 0xFFFFFF);
	
	if (entry != _mutationFilters.end())
	{
		return &(entry->second);
	}

	return NULL;
}

 

std::vector<std::string> CInferredPortalData::GetBannedwords()
{
	return _bannedwords;
}

// std::set<uint32_t> CInferredPortalData::GetRestrictedLandblocks()
//{
//	return _restrictedLBData;
//}

 bool CInferredPortalData::ReloadQuestData()
 {
	 fs::path dataPath(g_pGlobals->GetGameData("Data", "json"));
	 if (LoadJsonData(dataPath / "quests.json", g_pPortalDataEx->_questDefDB))
		 return true;
	 return false;
 }

 bool CInferredPortalData::ReloadEventData()
 {
	 fs::path dataPath(g_pGlobals->GetGameData("Data", "json"));
	 if (LoadJsonData(dataPath / "events.json", g_pGameEventManager->_gameEvents))
		 return true;
	 return false;
 }
