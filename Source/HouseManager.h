#pragma once

#include "WeenieObject.h"
#include "House.h"
#include "Player.h"
#include "World.h"
#include "Server.h"
#include "Network.h"

class CHouseManager
{
public:
	CHouseManager();
	~CHouseManager();

	void Load();
	void Save();

	uint32_t _currentHouseMaintenancePeriod = 0;
	uint32_t _nextHouseMaintenancePeriod = 0;
	bool _freeHouseMaintenancePeriod = false;

	CHouseData *GetHouseData(uint32_t houseId);
	void SaveHouseData(uint32_t houseId);
	void SendHouseData(CPlayerWeenie *player, uint32_t houseId);
	void RemoveHouseFromAccount(uint32_t characterId);
	void ReleaseDeletedCharacterHousing();
	void ReleaseBannedCharacterHousing();
	void LoadHousingMap();
	std::tuple<int, std::list<uint32_t>> GetAvailableHouses(HouseType houseType);

private:
	PackableHashTable<uint32_t, CHouseData> _houseDataMap;

	std::mutex m_housingLock;


};
