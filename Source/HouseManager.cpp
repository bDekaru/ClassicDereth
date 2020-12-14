#include <StdAfx.h>
#include "HouseManager.h"

CHouseManager::CHouseManager()
{
	WINLOG(Data, Warning, "Initializing Housing..\n");
	//LoadHousingMap();
}

CHouseManager::~CHouseManager()
{
	_houseDataMap.clear();
}

void CHouseManager::LoadHousingMap()
{
	_houseDataMap.clear();
	std::list<HouseInfo_t> houses = g_pDBIO->GetHousingData();

	for (auto hd : houses)
	{
		CHouseData houseData;
		BinaryReader reader(hd.houseBlob, hd.houseBlobLen);
		houseData.UnPack(&reader);
		houseData._houseId = hd.house_id;

		if (CHouseData* existingHouse = _houseDataMap.lookup(hd.house_id))
		{
			_houseDataMap.remove(hd.house_id);
		}

		_houseDataMap.add(hd.house_id, &houseData);
		delete[] hd.houseBlob;
	}
}

void CHouseManager::ReleaseDeletedCharacterHousing()
{
	std::list<unsigned int> allCharacters = g_pDBIO->GetAllCharacterIDs();

	std::list<unsigned int>::iterator it;
	for (auto hd : _houseDataMap)
	{
		it = std::find(allCharacters.begin(), allCharacters.end(), hd.second._ownerId);
		if (it == allCharacters.end() && hd.second._ownerId != 0)
		{
			// char is deleted remove account from house
			hd.second.AbandonHouse();
		}
	}
}

void CHouseManager::ReleaseBannedCharacterHousing()
{
	// get banned accounts
	std::list<unsigned int> bannedAccts = g_pDBIO->GetBannedAccountIDs();
	// get bans
	CNetworkBanList m_Bans;
	void *data = NULL;
	unsigned long length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_BAN_DATA, &data, &length))
	{
		BinaryReader reader(data, length);
		m_Bans.UnPack(&reader);
	}
	
	std::list<unsigned int>::iterator it;
	for (auto hd : _houseDataMap)
	{
		break;
		it = std::find(bannedAccts.begin(), bannedAccts.end(), hd.second._ownerAccount);
		if (it != bannedAccts.end())
		{
			for (auto ba : m_Bans.m_BanTable)
			{
				if (hd.second._ownerAccount == ba.second.m_AccountID && ba.second.m_BanDuration == 0)
				{
					// char is perma banned remove account from house
					hd.second.AbandonHouse();
				}
			}
		}
	}
}

std::tuple<int, std::list<uint32_t>> CHouseManager::GetAvailableHouses(HouseType houseType)
{
	std::list<uint32_t> results;
	int countOfAvailable = 0;
	for (auto ht : _houseDataMap)
	{
		if (ht.second._houseType == houseType && ht.second._ownerId == 0)
		{
			countOfAvailable++;
			if(results.size() <= 400)
				results.push_back(ht.second._position.objcell_id);
		}
	}
	return std::make_tuple(countOfAvailable, results);
}


void CHouseManager::Load()
{
	void *data = NULL;
	unsigned long length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_HOUSING_DATA, &data, &length))
	{
		BinaryReader reader(data, length);
		_currentHouseMaintenancePeriod = reader.Read<uint32_t>();
		_nextHouseMaintenancePeriod = reader.Read<uint32_t>();
		_freeHouseMaintenancePeriod = (reader.Read<int>() > 0);
	}
	else
	{
		_currentHouseMaintenancePeriod = g_pPhatSDK->GetCurrTimeStamp();
		_nextHouseMaintenancePeriod = _currentHouseMaintenancePeriod + (30 * 24 * 60 * 60); //in 30 days.
		_freeHouseMaintenancePeriod = false;
	}
}

void CHouseManager::Save()
{
	BinaryWriter writer;
	writer.Write<uint32_t>(_currentHouseMaintenancePeriod);
	writer.Write<uint32_t>(_nextHouseMaintenancePeriod);
	writer.Write<int>((int)_freeHouseMaintenancePeriod);
	g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_HOUSING_DATA, writer.GetData(), writer.GetSize());
}


CHouseData *CHouseManager::GetHouseData(uint32_t houseId)
{
	if (!houseId)
		return NULL;

	CHouseData *houseData = _houseDataMap.lookup(houseId);
	if (!houseData)
	{
		//let's try the database
		void *data = NULL;
		unsigned long length = 0;
		if (g_pDBIO->GetHouseData(houseId, &data, &length))
		{
			BinaryReader reader(data, length);
			houseData = new CHouseData();
			houseData->UnPack(&reader);
			houseData->_houseId = houseId;

			_houseDataMap.add(houseId, houseData);
			delete houseData;
			houseData = _houseDataMap.lookup(houseId);
		}
	}

	if (!houseData)
	{
		houseData = new CHouseData(); //just make a new one.
		houseData->_houseId = houseId;

		_houseDataMap.add(houseId, houseData);
		delete houseData;
		houseData = _houseDataMap.lookup(houseId);
	}

	return houseData;
}

void CHouseManager::RemoveHouseFromAccount(uint32_t characterId)
{
	if (!characterId)
		return;

	CWeenieObject* player = g_pWorld->FindObject(characterId, true);
	if (!player)
		return;

	if (uint32_t houseId = player->m_Qualities.GetIID(HOUSE_IID, 0) &&
		characterId == player->m_Qualities.GetIID(HOUSE_OWNER_IID, 0))
	{
		CHouseData *houseData = _houseDataMap.lookup(houseId);
		if (houseData)
		{
			houseData->AbandonHouse();
			_houseDataMap.remove(houseId);
		}
	}
}

void CHouseManager::SaveHouseData(uint32_t houseId)
{
	if (!houseId)
		return;

	CHouseData *houseData = _houseDataMap.lookup(houseId);
	if (!houseData)
		return;

	houseData->Save();
}

void CHouseManager::SendHouseData(CPlayerWeenie *player, uint32_t houseId)
{
	if (!player)
		return;

	CHouseData *houseData = GetHouseData(houseId);

	BinaryWriter houseDataPacket;

	if (!houseData || houseData->_ownerId != player->GetID())
	{
		houseDataPacket.Write<uint32_t>(0x0226);
		houseDataPacket.Write<uint32_t>(0);
		player->SendNetMessage(&houseDataPacket, PRIVATE_MSG, TRUE, FALSE);
	}
	else
	{
		houseDataPacket.Write<uint32_t>(0x0225);
		houseDataPacket.Write<uint32_t>(houseData->_purchaseTimestamp);
		houseDataPacket.Write<uint32_t>(g_pHouseManager->_currentHouseMaintenancePeriod);
		houseDataPacket.Write<uint32_t>(houseData->_houseType);
		houseDataPacket.Write<int>((int)g_pHouseManager->_freeHouseMaintenancePeriod);
		houseData->_buy.Pack(&houseDataPacket);
		houseData->_rent.Pack(&houseDataPacket);
		houseData->_position.Pack(&houseDataPacket);
		player->SendNetMessage(&houseDataPacket, PRIVATE_MSG, TRUE, FALSE);
	}
}
