
#pragma once

class CInferredPortalData
{
public:
	CInferredPortalData();
	~CInferredPortalData();

	void Init();

	DWORD GetWCIDForTerrain(long x, long y, int index);
	CSpellTableEx *GetSpellTableEx();
	class CCraftOperation *GetCraftOperation(DWORD source_wcid, DWORD dest_wcid);
	Position *GetHousePortalDest(DWORD house_id, DWORD ignore_cell_id);
	CMutationFilter *GetMutationFilter(DWORD id);

	CRegionDescExtendedDataTable _regionData;
	CSpellTableExtendedDataTable _spellTableData;
	TreasureTable _treasureTableData;
	CCraftTable _craftTableData;
	PackableHashTable<DWORD, PackableList<Position>> _housePortalDests;
	CQuestDefDB _questDefDB;
	std::unordered_map<DWORD, CMutationFilter *> _mutationFilters;
	GameEventDefDB _gameEvents;
};


