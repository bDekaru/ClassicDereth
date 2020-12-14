
#pragma once

class CInferredPortalData : public InferredData
{
public:
	CInferredPortalData();
	~CInferredPortalData();

	void Init();

	uint32_t GetWCIDForTerrain(int32_t x, int32_t y, int index);
	CSpellTableEx *GetSpellTableEx();
	class CCraftOperation *GetCraftOperation(uint32_t source_wcid, uint32_t dest_wcid);
	Position *GetHousePortalDest(uint32_t house_id, uint32_t ignore_cell_id);
	CMutationFilter *GetMutationFilter(uint32_t id);
	std::vector<std::string> GetBannedwords();
	//std::set<uint32_t> GetRestrictedLandblocks();

	bool ReloadQuestData();
	bool ReloadEventData();

public:

	using position_list_t = PackableListWithJson<Position>;
	using house_portal_table_t = PackableHashTableWithJson<uint32_t, position_list_t>;
	using mutation_table_t = PackableHashTableWithJson<uint32_t, CMutationFilter>;
	//using mutation_table_t = std::unordered_map<uint32_t, CMutationFilter *>;

	CRegionDescExtendedDataTable _regionData;
	CSpellTableExtendedDataTable _spellTableData;
	TreasureTable _treasureTableData;
	CCraftTable _craftTableData;
	house_portal_table_t _housePortalDests;
	CQuestDefDB _questDefDB;
	mutation_table_t _mutationFilters;
	GameEventDefDB _gameEvents;
	std::vector<std::string> _bannedwords;
	//std::set<uint32_t> _restrictedLBData;
};


