
#include "StdAfx.h"
#include "House.h"
#include "World.h"
#include "WeenieFactory.h"
#include "Player.h"
#include "InferredPortalData.h"
#include "AllegianceManager.h"
#include "Client.h"
#include "EmoteManager.h"

#define HOUSE_SAVE_INTERVAL 300.0

CHouseManager::CHouseManager()
{
}

CHouseManager::~CHouseManager()
{
}

void CHouseManager::Load()
{
	void *data = NULL;
	DWORD length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_HOUSING_DATA, &data, &length))
	{
		BinaryReader reader(data, length);
		_currentHouseMaintenancePeriod = reader.Read<DWORD>();
		_nextHouseMaintenancePeriod = reader.Read<DWORD>();
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
	writer.Write<DWORD>(_currentHouseMaintenancePeriod);
	writer.Write<DWORD>(_nextHouseMaintenancePeriod);
	writer.Write<int>((int)_freeHouseMaintenancePeriod);
	g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_HOUSING_DATA, writer.GetData(), writer.GetSize());

	for (auto houseData : _houseDataMap)
		houseData.second.Save();
}

CHouseData *CHouseManager::GetHouseData(DWORD houseId)
{
	if (!houseId)
		return NULL;

	CHouseData *houseData = _houseDataMap.lookup(houseId);
	if (!houseData)
	{
		//let's try the database
		void *data = NULL;
		DWORD length = 0;
		if (g_pDBIO->GetHouseData(houseId, &data, &length))
		{
			BinaryReader reader(data, length);
			houseData = new CHouseData();
			houseData->UnPack(&reader);
			houseData->_houseId = houseId;

			_houseDataMap.add(houseId, houseData);
		}
	}

	if (!houseData)
	{
		houseData = new CHouseData(); //just make a new one.
		houseData->_houseId = houseId;

		_houseDataMap.add(houseId, houseData);
	}

	return houseData;
}

void CHouseManager::SaveHouseData(DWORD houseId)
{
	if (!houseId)
		return;

	CHouseData *houseData = _houseDataMap.lookup(houseId);
	if (!houseData)
		return;

	houseData->Save();
}

void CHouseManager::SendHouseData(CPlayerWeenie *player, DWORD houseId)
{
	if (!player)
		return;

	CHouseData *houseData = GetHouseData(houseId);

	BinaryWriter houseDataPacket;

	if (!houseData || houseData->_ownerId != player->GetID())
	{
		houseDataPacket.Write<DWORD>(0x0226);
		houseDataPacket.Write<DWORD>(0);
		player->SendNetMessage(&houseDataPacket, PRIVATE_MSG, TRUE, FALSE);
	}
	else
	{
		houseDataPacket.Write<DWORD>(0x0225);
		houseDataPacket.Write<DWORD>(houseData->_purchaseTimestamp);
		houseDataPacket.Write<DWORD>(g_pHouseManager->_currentHouseMaintenancePeriod);
		houseDataPacket.Write<DWORD>(houseData->_houseType);
		houseDataPacket.Write<int>((int)g_pHouseManager->_freeHouseMaintenancePeriod);
		houseData->_buy.Pack(&houseDataPacket);
		houseData->_rent.Pack(&houseDataPacket);
		houseData->_position.Pack(&houseDataPacket);
		player->SendNetMessage(&houseDataPacket, PRIVATE_MSG, TRUE, FALSE);
	}
}

DEFINE_PACK(CHouseData)
{
	pWriter->Write<DWORD>(_slumLordId);
	pWriter->Write<DWORD>(_ownerId);
	pWriter->Write<DWORD>(_ownerAccount);
	pWriter->Write<DWORD>(_purchaseTimestamp);
	pWriter->Write<DWORD>(_currentMaintenancePeriod);
	pWriter->Write<DWORD>(_houseType);
	_position.Pack(pWriter);
	_buy.Pack(pWriter);
	_rent.Pack(pWriter);
	_accessList.Pack(pWriter);
	_storageAccessList.Pack(pWriter);
	pWriter->Write<bool>(_allegianceAccess);
	pWriter->Write<bool>(_allegianceStorageAccess);
	pWriter->Write<bool>(_everyoneAccess);
	pWriter->Write<bool>(_everyoneStorageAccess);
	pWriter->Write<bool>(_hooksVisible);
}

DEFINE_UNPACK(CHouseData)
{
	_slumLordId = pReader->Read<DWORD>();
	_ownerId = pReader->Read<DWORD>();
	_ownerAccount = pReader->Read<DWORD>();
	_purchaseTimestamp = pReader->Read<DWORD>();
	_currentMaintenancePeriod = pReader->Read<DWORD>();
	_houseType = pReader->Read<DWORD>();
	_position.UnPack(pReader);
	_buy.UnPack(pReader);
	_rent.UnPack(pReader);
	_accessList.UnPack(pReader);
	_storageAccessList.UnPack(pReader);
	_allegianceAccess = pReader->Read<bool>();
	_allegianceStorageAccess = pReader->Read<bool>();
	_everyoneAccess = pReader->Read<bool>();
	_everyoneStorageAccess = pReader->Read<bool>();
	_hooksVisible = pReader->Read<bool>();

	return true;
}

void CHouseData::ClearOwnershipData()
{
	_ownerId = 0;
	_ownerAccount = 0;
	_purchaseTimestamp = 0;
	_currentMaintenancePeriod = 0;
	_allegianceAccess = false;
	_allegianceStorageAccess = false;
	_everyoneAccess = false;
	_everyoneStorageAccess = false;
	_hooksVisible = true;
	_rent.clear();
	_buy.clear();
}

void CHouseData::SetHookVisibility(bool newSetting)
{
	for (auto hook_id : _hookList)
	{
		CWeenieObject *hookWeenie = g_pWorld->FindObject(hook_id, true);

		if (!hookWeenie)
			continue;

		CHookWeenie *hook = hookWeenie->AsHook();

		if (!hook)
			continue;

		hook->SetHookVisibility(newSetting);
	}
}

void CHouseData::AbandonHouse()
{
		CWeenieObject *owner = g_pWorld->FindObject(_ownerId);
		if (owner)
		{
			//if the player is offline we won't find him but that's okay, we will just update his housing data when he logins in UpdateHouseData()
			owner->m_Qualities.RemoveDataID(HOUSEID_DID);
			owner->NotifyDIDStatUpdated(HOUSEID_DID);
		}

		ClearOwnershipData();
		SetHookVisibility(_hooksVisible);

		if(CWeenieObject *slumLord = g_pWorld->FindObject(_slumLordId))
			slumLord->DoForcedMotion(Motion_Off);

		if (owner)
			g_pHouseManager->SendHouseData(owner->AsPlayer(), _houseId);
}

void CHouseData::Save()
{
	BinaryWriter writer;
	Pack(&writer);
	g_pDBIO->CreateOrUpdateHouseData(_houseId, writer.GetData(), writer.GetSize());
}


CHouseWeenie::CHouseWeenie()
{
	m_bDontClear = true;
}

void CHouseWeenie::EnsureLink(CWeenieObject *source)
{
	source->m_Qualities.SetInstanceID(HOUSE_IID, GetID());

	if (source->AsSlumLord())
	{
		m_Qualities.SetInstanceID(SLUMLORD_IID, source->GetID());
	}

	if (source->AsHook())
	{
		CHouseData *houseData = source->AsHook()->GetHouseData();
		if(houseData)
			houseData->_hookList.insert(source->GetID());
	}

	if (source->AsBootSpot())
	{
		m_Qualities.SetPosition(HOUSE_BOOT_POSITION, source->GetPosition());
	}



	// Deed_WeenieType
	// BootSpot_WeenieType
	// HousePortal_WeenieType
	// Storage_WeenieType
}

CHouseData *CHouseWeenie::GetHouseData()
{
	return g_pHouseManager->GetHouseData(GetHouseDID());
}

std::string CHouseWeenie::GetHouseOwnerName()
{
	DWORD owner = GetHouseOwner();

	if (!owner)
		return "";

	return g_pWorld->GetPlayerName(owner, true);
	//return InqStringQuality(HOUSE_OWNER_NAME_STRING, "");
}

DWORD CHouseWeenie::GetHouseOwner()
{
	return GetHouseData()->_ownerId;
	//return InqIIDQuality(HOUSE_OWNER_IID, 0);
}

DWORD CHouseWeenie::GetHouseDID()
{
	return InqDIDQuality(HOUSEID_DID, 0);
}

int CHouseWeenie::GetHouseType()
{
	return InqIntQuality(HOUSE_TYPE_INT, 0);
}

CSlumLordWeenie *CHouseWeenie::GetSlumLord()
{
	if (CWeenieObject *slumlord = g_pWorld->FindObject(InqIIDQuality(SLUMLORD_IID, 0), true))
	{
		return slumlord->AsSlumLord();
	}

	return NULL;
}

bool CHouseWeenie::HasAccess(CPlayerWeenie *requester)
{
	if (!requester)
		return false;

	CHouseData *houseData = GetHouseData();

	DWORD requesterId = requester->GetID();
	DWORD houseOwnerId = GetHouseOwner();

	if (requesterId == houseData->_ownerId)
		return true;

	if (houseData->_everyoneAccess)
		return true;

	if (requester->GetClient()->GetAccountInfo().id == houseData->_ownerAccount)
		return true;

	if (std::find(houseData->_accessList.begin(), houseData->_accessList.end(), requesterId) != houseData->_accessList.end())
		return true;

	if (houseData->_allegianceAccess)
	{
		AllegianceTreeNode *ownerAllegianceNode = g_pAllegianceManager->GetTreeNode(houseOwnerId);
		AllegianceTreeNode *requesterAllegianceNode = g_pAllegianceManager->GetTreeNode(requesterId);

		if (ownerAllegianceNode->_monarchID == requesterAllegianceNode->_monarchID)
			return true;
	}

	return HasStorageAccess(requester); //storage access automatically grants access.
}

bool CHouseWeenie::HasStorageAccess(CPlayerWeenie *requester)
{
	if (!requester)
		return false;

	CHouseData *houseData = GetHouseData();

	DWORD requesterId = requester->GetID();
	DWORD houseOwnerId = GetHouseOwner();

	if (requesterId == houseData->_ownerId)
		return true;

	if (houseData->_everyoneStorageAccess)
		return true;

	if (requester->GetClient()->GetAccountInfo().id == houseData->_ownerAccount)
		return true;

	if (std::find(houseData->_storageAccessList.begin(), houseData->_storageAccessList.end(), requesterId) != houseData->_storageAccessList.end())
		return true;

	if (houseData->_allegianceStorageAccess)
	{
		AllegianceTreeNode *ownerAllegianceNode = g_pAllegianceManager->GetTreeNode(houseOwnerId);
		AllegianceTreeNode *requesterAllegianceNode = g_pAllegianceManager->GetTreeNode(requesterId);

		if (ownerAllegianceNode->_monarchID == requesterAllegianceNode->_monarchID)
			return true;
	}

	return false;
}

CSlumLordWeenie::CSlumLordWeenie()
{
	m_bDontClear = true;
	_nextHeartBeat = Timer::cur_time; //let's tick as soon as possible to establish if we have an owner.
}

void CSlumLordWeenie::Tick()
{
	if (_initialized && _nextSave != -1.0 && _nextSave < Timer::cur_time)
	{
		CHouseWeenie *house = GetHouse();
		if (!house)
			return;

		if(house->ShouldSave())
			house->Save();

		CHouseData *houseData = house->GetHouseData();

		if (houseData)
		{
			houseData->Save();

			for (auto hook_id : houseData->_hookList)
			{
				CWeenieObject *hook = g_pWorld->FindObject(hook_id);

				if (!hook)
					continue;

				if (hook->ShouldSave())
					hook->Save();
			}
		}

		_nextSave = Timer::cur_time + HOUSE_SAVE_INTERVAL;
	}

	if (_nextHeartBeat != -1.0 && _nextHeartBeat <= Timer::cur_time)
	{
		if (!_initialized)
		{
			_nextHeartBeat = Timer::cur_time + Random::GenUInt(1, 10);

			CHouseWeenie *house = GetHouse();
			if (!house)
				return;

			CHouseData *houseData = house->GetHouseData();
			houseData->_slumLordId = GetID();
			houseData->_position = m_Position;

			if (houseData->_ownerId)
				DoForcedMotion(Motion_On);

			for (auto hook_id : houseData->_hookList)
			{
				CWeenieObject *hook = g_pWorld->FindObject(hook_id, true);

				if (!hook)
					continue;

				if(hook->AsHook())
					hook->AsHook()->UpdateHookedObject();
			}

			_initialized = true;

			_nextSave = Timer::cur_time + HOUSE_SAVE_INTERVAL;
		}
		else
		{
			//check global data and update if necessary
			DWORD now = g_pPhatSDK->GetCurrTimeStamp();
			if (g_pHouseManager->_nextHouseMaintenancePeriod < now)
			{
				if (!g_pHouseManager->_freeHouseMaintenancePeriod)
					g_pHouseManager->_currentHouseMaintenancePeriod = now;
				g_pHouseManager->_freeHouseMaintenancePeriod = false;
				g_pHouseManager->_nextHouseMaintenancePeriod = now + (30 * 24 * 60 * 60); //in 30 days.
			}

			CheckRentPeriod();

			_nextHeartBeat = Timer::cur_time + Random::GenUInt(30 * 60, 60 * 60); //check again in 30 to 60 minutes.
		}
	}
}

CHouseWeenie *CSlumLordWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}

void CSlumLordWeenie::GetHouseProfile(HouseProfile &prof)
{
	prof._slumlord = GetID();
	prof._maintenance_free = 0;
	prof._bitmask = Active_HouseBitmask;
	prof._min_level = InqIntQuality(MIN_LEVEL_INT, -1);
	prof._max_level = InqIntQuality(MAX_LEVEL_INT, -1);
	prof._min_alleg_rank = 0;

	if (m_Qualities._create_list)
	{
		for (auto &entry : *m_Qualities._create_list)
		{
			if (entry.regen_algorithm == HouseBuy_DestinationType)
			{
				HousePayment pay;
				pay.wcid = entry.wcid;
				pay.num = entry.amount;
				pay.paid = 0;

				CWeenieDefaults *weenieDef = g_pWeenieFactory->GetWeenieDefaults(entry.wcid);
				if (weenieDef)
				{
					weenieDef->m_Qualities.InqString(NAME_STRING, pay.name);
					weenieDef->m_Qualities.InqString(PLURAL_NAME_STRING, pay.pname);
				}
				prof._buy.push_back(pay);
			}
			else if (entry.regen_algorithm == HouseRent_DestinationType)
			{
				HousePayment pay;
				pay.wcid = entry.wcid;
				pay.num = entry.amount;
				pay.paid = 0;

				CWeenieDefaults *weenieDef = g_pWeenieFactory->GetWeenieDefaults(entry.wcid);
				if (weenieDef)
				{
					weenieDef->m_Qualities.InqString(NAME_STRING, pay.name);
					weenieDef->m_Qualities.InqString(PLURAL_NAME_STRING, pay.pname);
				}

				prof._rent.push_back(pay);
			}
		}
	}
	
	if (CHouseWeenie *house = GetHouse())
	{
		prof._name = house->GetHouseOwnerName();
		prof._owner = house->GetHouseOwner();
		prof._id = house->GetHouseDID();
		prof._type = house->GetHouseType();
	}
}

int CSlumLordWeenie::DoUseResponse(CWeenieObject *other)
{
	HouseProfile prof;
	GetHouseProfile(prof);

	if (CHouseWeenie *house = GetHouse())
	{
		CHouseData *houseData = house->GetHouseData();
		if (houseData && houseData->_ownerId)
			prof._rent = houseData->_rent; //get the real house rent information that may contain payments
	}
	
	BinaryWriter profMsg;
	profMsg.Write<DWORD>(0x21D);
	prof.Pack(&profMsg);
	other->SendNetMessage(&profMsg, PRIVATE_MSG, TRUE, FALSE);

	return WERROR_NONE;
}

void CSlumLordWeenie::BuyHouse(CPlayerWeenie *player, const PackableList<DWORD> &items)
{
	if (CHouseWeenie *house = GetHouse())
	{
		if(player->GetAccountHouseId())
		{
			player->SendText("You may only own one dwelling at a time.", LTT_DEFAULT); //made up message.
			return;
		}

		if (house->GetHouseOwner())
		{
			player->SendText("That dwelling already has an owner.", LTT_DEFAULT); //made up message.
			return;
		}

		if (int minLevel = InqIntQuality(MIN_LEVEL_INT, 0))
		{
			if (player->InqIntQuality(LEVEL_INT, 1) < minLevel)
			{
				player->SendText(csprintf("You must be at least level %d to purchase this dwelling.", minLevel), LTT_DEFAULT); //made up message.
				return;
			}
		}

		AllegianceTreeNode *allegianceNode = g_pAllegianceManager->GetTreeNode(player->GetID());
		if (InqBoolQuality(HOUSE_REQUIRES_MONARCH_BOOL, false))
		{
			if (!allegianceNode || allegianceNode->_patronID != 0)
			{
				player->SendText("You must be a monarch to purchase this dwelling.", LTT_DEFAULT); //made up message.
				return;
			}
		}

		if (int minAllegianceRank = InqIntQuality(ALLEGIANCE_MIN_LEVEL_INT, 0))
		{
			if (!allegianceNode || allegianceNode->_rank < minAllegianceRank)
			{
				player->SendText(csprintf("Your allegiance rank must be at least %d to purchase this dwelling.", minAllegianceRank), LTT_DEFAULT); //made up message.
				return;
			}
		}

		if (house->GetHouseType() != 4 && player->InqIntQuality(HOUSE_PURCHASE_TIMESTAMP_INT, 0) + (30 * 24 * 60) > g_pPhatSDK->GetCurrTimeStamp())
		{
			player->SendText("You cannot buy another landscape house yet. This restriction does not apply to apartments.", LTT_DEFAULT); //made up message.
			return;
		}

		HouseProfile prof;
		GetHouseProfile(prof);

		std::map<CWeenieObject *, int> consumeList;

		for (HousePayment &payment : prof._buy)
		{
			for (auto &itemID : items)
			{
				CWeenieObject *item = g_pWorld->FindObject(itemID);

				if (!item || !player->FindContainedItem(itemID))
					continue;

				if (payment.wcid == W_COINSTACK_CLASS)
				{
					if (payment.paid < payment.num)
					{
						if (item->m_Qualities.id == payment.wcid || item->InqIntQuality(ITEM_TYPE_INT, 0) == ITEM_TYPE::TYPE_PROMISSORY_NOTE)
						{
							payment.paid += item->InqIntQuality(VALUE_INT, 0);
							consumeList[item] = item->InqIntQuality(STACK_SIZE_INT, 1); //add the complete stack, we will sort values later.
						}
					}
				}
				else
				{
					if (item->m_Qualities.id == payment.wcid && payment.paid < payment.num)
					{
						int amountToConsume = min(item->InqIntQuality(STACK_SIZE_INT, 1), payment.num - payment.paid);
						payment.paid += amountToConsume;
						consumeList[item] = amountToConsume;
					}
				}
			}
		}

		for (HousePayment &payment : prof._buy)
		{
			if (payment.paid < payment.num)
			{
				player->SendText("That's not everything you need to buy this dwelling.", LTT_DEFAULT); //made up message.
				return;
			}
		}

		for (auto entry : consumeList)
		{
			CWeenieObject *item = entry.first;
			int amount = entry.second;

			if (item->m_Qualities.id == W_COINSTACK_CLASS || item->InqIntQuality(ITEM_TYPE_INT, 0) == ITEM_TYPE::TYPE_PROMISSORY_NOTE)
				item->Remove(); //we remove the entire stack of coins/trade notes and then give back the appropriate amount.
			else
			{
				int stackSize = item->InqIntQuality(STACK_SIZE_INT, 1);
				item->DecrementStackNum(amount);
			}
		}

		for (HousePayment &payment : prof._buy)
		{
			if (payment.wcid == W_COINSTACK_CLASS)
			{
				if (payment.paid > payment.num) //we have to return some money
				{
					int amountToReturn = payment.paid - payment.num;
					payment.paid = payment.num;
					while (amountToReturn > 0)
					{
						if (amountToReturn >= 250000)
						{
							player->SpawnInContainer(W_TRADENOTE250000_CLASS);
							amountToReturn -= 250000;
						}
						else if (amountToReturn >= 200000)
						{
							player->SpawnInContainer(W_TRADENOTE200000_CLASS);
							amountToReturn -= 200000;
						}
						else if (amountToReturn >= 150000)
						{
							player->SpawnInContainer(W_TRADENOTE150000_CLASS);
							amountToReturn -= 150000;
						}
						else if (amountToReturn >= 100000)
						{
							player->SpawnInContainer(W_TRADENOTE100000_CLASS);
							amountToReturn -= 100000;
						}
						else if (amountToReturn >= 75000)
						{
							player->SpawnInContainer(W_TRADENOTE75000_CLASS);
							amountToReturn -= 75000;
						}
						else if (amountToReturn >= 50000)
						{
							player->SpawnInContainer(W_TRADENOTE50000_CLASS);
							amountToReturn -= 50000;
						}
						else if (amountToReturn >= 25000)
						{
							player->SpawnInContainer(W_TRADENOTE25000_CLASS);
							amountToReturn -= 25000;
						}
						else if (amountToReturn >= 20000)
						{
							player->SpawnInContainer(W_TRADENOTE200000_CLASS);
							amountToReturn -= 20000;
						}
						else if (amountToReturn >= 15000)
						{
							player->SpawnInContainer(W_TRADENOTE15000_CLASS);
							amountToReturn -= 15000;
						}
						else if (amountToReturn >= 10000)
						{
							player->SpawnInContainer(W_TRADENOTE10000_CLASS);
							amountToReturn -= 10000;
						}
						else if (amountToReturn >= 5000)
						{
							player->SpawnInContainer(W_TRADENOTE5000_CLASS);
							amountToReturn -= 5000;
						}
						else if (amountToReturn >= 1000)
						{
							player->SpawnInContainer(W_TRADENOTE1000_CLASS);
							amountToReturn -= 1000;
						}
						else if (amountToReturn >= 500)
						{
							player->SpawnInContainer(W_TRADENOTE500_CLASS);
							amountToReturn -= 500;
						}
						else if (amountToReturn >= 100)
						{
							player->SpawnInContainer(W_TRADENOTE100_CLASS);
							amountToReturn -= 100;
						}
						else
						{
							player->SpawnInContainer(W_COINSTACK_CLASS, amountToReturn);
							amountToReturn = 0;
						}
					}
				}
				break;
			}
		}

		int timeStamp = g_pPhatSDK->GetCurrTimeStamp();
		player->SendText("Congratulations!  You now own this dwelling.", LTT_DEFAULT);

		//Disabled the cooldown on purchasing houses
		//if (house->GetHouseType() != 4)//apartments are not affected nor do the affect the house purchase frequency limitation
		//{
		//	player->m_Qualities.SetInt(HOUSE_PURCHASE_TIMESTAMP_INT, timeStamp);
		//	player->NotifyIntStatUpdated(HOUSE_PURCHASE_TIMESTAMP_INT);
		//}

		player->m_Qualities.SetDataID(HOUSEID_DID, house->GetHouseDID());
		player->NotifyDIDStatUpdated(HOUSEID_DID);

		CHouseData *houseData = house->GetHouseData();
		houseData->ClearOwnershipData();
		houseData->_ownerId = player->GetID();
		houseData->_ownerAccount = player->GetClient()->GetAccountInfo().id;
		houseData->_purchaseTimestamp = timeStamp;
		houseData->_currentMaintenancePeriod = g_pHouseManager->_currentHouseMaintenancePeriod;
		houseData->_rent = prof._rent;
		houseData->_buy = prof._buy;
		houseData->_houseType = house->GetHouseType();
		houseData->_position = GetPosition();

		for (auto hook_id : houseData->_hookList)
		{
			CWeenieObject *hook = g_pWorld->FindObject(hook_id, true);

			if (!hook)
				continue;

			hook->m_Qualities.SetInstanceID(HOUSE_OWNER_IID, player->GetID());
			hook->NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);

			if (hook->ShouldSave())
				hook->Save();
		}
		houseData->SetHookVisibility(true);

		DoForcedMotion(Motion_On);
		g_pHouseManager->SendHouseData(player, house->GetHouseDID());
		DoUseResponse(player);
		player->RecalculateCoinAmount();
		player->Save();
		if(house->ShouldSave())
			house->Save();
		houseData->Save();
	}
}

void CSlumLordWeenie::CheckRentPeriod()
{
	if (CHouseWeenie *house = GetHouse())
	{
		CHouseData *houseData = house->GetHouseData();

		if (!houseData->_ownerId)
			return; //no owner no rent

		if (houseData->_currentMaintenancePeriod < g_pHouseManager->_currentHouseMaintenancePeriod)
		{
			//check if we still meet the allegiance requirements to own this house.
			AllegianceTreeNode *allegianceNode = g_pAllegianceManager->GetTreeNode(houseData->_ownerId);
			if (InqBoolQuality(HOUSE_REQUIRES_MONARCH_BOOL, false))
			{
				if (!allegianceNode || allegianceNode->_patronID)
				{
					if (CWeenieObject *player = g_pWorld->FindObject(houseData->_ownerId)) //todo: a way to inform the allegiance and the player if he is offline.
						player->SendText("You no longer meet the monarch requirement for your dwelling so it has been abandoned.", LTT_DEFAULT); //made up message.
					houseData->AbandonHouse();
					return;
				}
			}

			if (int minAllegianceRank = InqIntQuality(ALLEGIANCE_MIN_LEVEL_INT, 0))
			{
				if (!allegianceNode || allegianceNode->_rank < minAllegianceRank)
				{
					if (CWeenieObject *player = g_pWorld->FindObject(houseData->_ownerId)) //todo: a way to inform the allegiance and the player if he is offline.
						player->SendText(csprintf("You no longer meet the allegiance rank requirement for your dwelling so it has been abandoned.", minAllegianceRank), LTT_DEFAULT); //made up message.
					houseData->AbandonHouse();
					return;
				}
			}

			bool fullyPaid = true;
			for (HousePayment &payment : houseData->_rent)
			{
				if (payment.paid < payment.num)
				{
					fullyPaid = false;
					break;
				}
			}

			if (fullyPaid)
			{
				HouseProfile prof;
				GetHouseProfile(prof);

				//update rent and buy values from house profile, maybe the prices have changed!
				houseData->_rent = prof._rent;
				houseData->_buy = prof._buy;

				if (CWeenieObject *owner = g_pWorld->FindObject(houseData->_ownerId))
					g_pHouseManager->SendHouseData(owner->AsPlayer(), house->GetHouseDID()); //update house's owner panel if the owner is online.
			}
			else
			{
				houseData->AbandonHouse();
			}
		}
	}
}

void CSlumLordWeenie::RentHouse(CPlayerWeenie *player, const PackableList<DWORD> &items)
{
	if (CHouseWeenie *house = GetHouse())
	{
		CHouseData *houseData = house->GetHouseData();

		if (!houseData->_ownerId)
		{
			player->SendText("That dwelling has no owner.", LTT_DEFAULT); //made up message.
			return;
		}

		bool requiresRent = false;
		for (HousePayment &payment : houseData->_rent)
		{
			if (payment.paid < payment.num)
			{
				requiresRent = true;
				break;
			}
		}

		if (!requiresRent)
		{
			player->SendText("The maintenance has already been paid for this period. You may not prepay the next period's maintenance.", LTT_DEFAULT); //made up message.
			return;
		}

		std::map<CWeenieObject *, int> consumeList;

		for (HousePayment &payment : houseData->_rent)
		{
			for (auto &itemID : items)
			{
				CWeenieObject *item = g_pWorld->FindObject(itemID);

				if (!item || !player->FindContainedItem(itemID))
					continue;

				if (payment.wcid == W_COINSTACK_CLASS)
				{
					if (payment.paid < payment.num)
					{
						if (item->m_Qualities.id == payment.wcid || item->InqIntQuality(ITEM_TYPE_INT, 0) == ITEM_TYPE::TYPE_PROMISSORY_NOTE)
						{
							payment.paid += item->InqIntQuality(VALUE_INT, 0);
							consumeList[item] = item->InqIntQuality(STACK_SIZE_INT, 1); //add the complete stack, we will sort values later.
						}
					}
				}
				else
				{
					if (item->m_Qualities.id == payment.wcid && payment.paid < payment.num)
					{
						int amountToConsume = min(item->InqIntQuality(STACK_SIZE_INT, 1), payment.num - payment.paid);
						payment.paid += amountToConsume;
						consumeList[item] = amountToConsume;
					}
				}
			}
		}

		for (auto entry : consumeList)
		{
			CWeenieObject *item = entry.first;
			int amount = entry.second;

			if (item->m_Qualities.id == W_COINSTACK_CLASS || item->InqIntQuality(ITEM_TYPE_INT, 0) == ITEM_TYPE::TYPE_PROMISSORY_NOTE)
				item->Remove(); //we remove the entire stack of coins/trade notes and then give back the appropriate amount.
			else
			{
				int stackSize = item->InqIntQuality(STACK_SIZE_INT, 1);
				item->DecrementStackNum(amount);
			}
		}

		for (HousePayment &payment : houseData->_rent)
		{
			if (payment.wcid == W_COINSTACK_CLASS)
			{
				if (payment.paid > payment.num) //we have to return some money
				{
					int amountToReturn = payment.paid - payment.num;
					payment.paid = payment.num;
					while (amountToReturn > 0)
					{
						if (amountToReturn >= 250000)
						{
							player->SpawnInContainer(W_TRADENOTE250000_CLASS);
							amountToReturn -= 250000;
						}
						else if (amountToReturn >= 200000)
						{
							player->SpawnInContainer(W_TRADENOTE200000_CLASS);
							amountToReturn -= 200000;
						}
						else if (amountToReturn >= 150000)
						{
							player->SpawnInContainer(W_TRADENOTE150000_CLASS);
							amountToReturn -= 150000;
						}
						else if (amountToReturn >= 100000)
						{
							player->SpawnInContainer(W_TRADENOTE100000_CLASS);
							amountToReturn -= 100000;
						}
						else if (amountToReturn >= 75000)
						{
							player->SpawnInContainer(W_TRADENOTE75000_CLASS);
							amountToReturn -= 75000;
						}
						else if (amountToReturn >= 50000)
						{
							player->SpawnInContainer(W_TRADENOTE50000_CLASS);
							amountToReturn -= 50000;
						}
						else if (amountToReturn >= 25000)
						{
							player->SpawnInContainer(W_TRADENOTE25000_CLASS);
							amountToReturn -= 25000;
						}
						else if (amountToReturn >= 20000)
						{
							player->SpawnInContainer(W_TRADENOTE200000_CLASS);
							amountToReturn -= 20000;
						}
						else if (amountToReturn >= 15000)
						{
							player->SpawnInContainer(W_TRADENOTE15000_CLASS);
							amountToReturn -= 15000;
						}
						else if (amountToReturn >= 10000)
						{
							player->SpawnInContainer(W_TRADENOTE10000_CLASS);
							amountToReturn -= 10000;
						}
						else if (amountToReturn >= 5000)
						{
							player->SpawnInContainer(W_TRADENOTE5000_CLASS);
							amountToReturn -= 5000;
						}
						else if (amountToReturn >= 1000)
						{
							player->SpawnInContainer(W_TRADENOTE1000_CLASS);
							amountToReturn -= 1000;
						}
						else if (amountToReturn >= 500)
						{
							player->SpawnInContainer(W_TRADENOTE500_CLASS);
							amountToReturn -= 500;
						}
						else if (amountToReturn >= 100)
						{
							player->SpawnInContainer(W_TRADENOTE100_CLASS);
							amountToReturn -= 100;
						}
						else
						{
							player->SpawnInContainer(W_COINSTACK_CLASS, amountToReturn);
							amountToReturn = 0;
						}
					}
				}
				break;
			}
		}
		DoUseResponse(player);
		player->RecalculateCoinAmount();

		if (CWeenieObject *owner = g_pWorld->FindObject(houseData->_ownerId))
			g_pHouseManager->SendHouseData(owner->AsPlayer(), house->GetHouseDID()); //update house's owner panel if the owner is online.
	}
}


CHookWeenie::CHookWeenie()
{
	m_bDontClear = true;
	_nextInitCheck = Timer::cur_time;
}

void CHookWeenie::Tick()
{
	if (!_initialized && _nextInitCheck != -1.0 && _nextInitCheck <= Timer::cur_time)
	{
		_nextInitCheck = Timer::cur_time + Random::GenUInt(1, 10);

		CHouseData *houseData = GetHouseData();

		if (!houseData)
			return;

		if (houseData->_ownerId)
		{
			SetHookVisibility(houseData->_hooksVisible);
			m_Qualities.SetInstanceID(HOUSE_OWNER_IID, houseData->_ownerId);
			NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
		}
		else
		{
			m_Qualities.RemoveInstanceID(HOUSE_OWNER_IID);
			NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
		}

		UpdateHookedObject(NULL);
		SetHookVisibility(houseData->_hooksVisible);

		_nextInitCheck = -1.0;
		_initialized = true;
	}

	CContainerWeenie::Tick();
}

void CHookWeenie::SaveEx(CWeenieSave & save)
{
	//if we don't clear before saving some hooked item's position and rotation changes, not sure what is causing that.
	ClearHookedObject(false);

	CContainerWeenie::SaveEx(save);

	UpdateHookedObject(NULL, false);
}

void CHookWeenie::LoadEx(CWeenieSave & save)
{
	CContainerWeenie::LoadEx(save);

	_nextInitCheck = Timer::cur_time;
}

int CHookWeenie::DoUseResponse(CWeenieObject *other)
{
	CHouseWeenie *house = GetHouse();
	if (!house)
		return WERROR_NO_HOUSE;

	CHouseData *houseData = house->GetHouseData();
	if (!houseData)
		return WERROR_NO_HOUSE;

	if (houseData->_hooksVisible)
	{
		if (house->HasStorageAccess(other->AsPlayer()))
			return CContainerWeenie::DoUseResponse(other);
		else
		{
			other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
			return WERROR_ACTIVATION_NOT_ALLOWED_NO_NAME;
		}
	}
	else
	{
		//if house hooks are off this means we're using the hooked item.
		if (house->HasAccess(other->AsPlayer()))
		{
			CWeenieObject *hookedItem;
			if (m_Items.empty())
				return WERROR_OBJECT_GONE;

			hookedItem = m_Items[0];

			if (!hookedItem)
				return WERROR_OBJECT_GONE;

			if (hookedItem->InqIntQuality(HOOK_GROUP_INT, 0) >= 4 && house->GetHouseType() != 3) //todo: figure out what each value of this flag means, known values are 1, 2, 4, 8, 16, 32.
			{
				other->SendText("That can only be activated when hooked in a mansion.", LTT_DEFAULT); //made up message.
				return WERROR_OBJECT_GONE;
			}

			return WERROR_NONE;
		}
		else
		{
			other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
			return WERROR_HOOK_NOT_PERMITED_TO_USE_HOOK;
		}
	}
}

void CHookWeenie::Identify(CWeenieObject *other, DWORD overrideId)
{
	CHouseData *houseData = GetHouseData();

	if (!houseData || houseData->_hooksVisible)
		CContainerWeenie::Identify(other, overrideId);
	else
	{
		//if house hooks are off this means we're identifying the hooked item.
		CWeenieObject *hookedItem;
		if (m_Items.empty())
			return;

		hookedItem = m_Items[0];

		if (!hookedItem)
			return;

		hookedItem->Identify(other, GetID());
	}
}

DWORD CHookWeenie::Container_InsertInventoryItem(DWORD dwCell, CWeenieObject *pItem, DWORD slot)
{
	UpdateHookedObject(pItem);
	
	return CContainerWeenie::Container_InsertInventoryItem(dwCell, pItem, slot);
}

void CHookWeenie::ReleaseContainedItemRecursive(CWeenieObject *item)
{
	ClearHookedObject();

	CContainerWeenie::ReleaseContainedItemRecursive(item);
}

void CHookWeenie::UpdateHookedObject(CWeenieObject *hookedItem, bool sendUpdate)
{
	if (!hookedItem && !m_Items.empty())
		hookedItem = m_Items[0];
	
	if (!hookedItem)
		return;

	DWORD value;
	if (hookedItem->m_Qualities.InqDataID(SETUP_DID, value))
	{
		m_Qualities.SetDataID(SETUP_DID, value);
		NotifyDIDStatUpdated(SETUP_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SETUP_DID);
		NotifyDIDStatUpdated(SETUP_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(MOTION_TABLE_DID, value))
	{
		m_Qualities.SetDataID(MOTION_TABLE_DID, value);
		NotifyDIDStatUpdated(MOTION_TABLE_DID, false);

		_phys_obj->SetMotionTableID(value);
	}
	else
	{
		m_Qualities.RemoveDataID(MOTION_TABLE_DID);
		NotifyDIDStatUpdated(MOTION_TABLE_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(SOUND_TABLE_DID, value))
	{
		m_Qualities.SetDataID(SOUND_TABLE_DID, value);
		NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SOUND_TABLE_DID);
		NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(PHYSICS_EFFECT_TABLE_DID, value))
	{
		m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, value);
		NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		set_phstable_id(value);
	}
	else
	{
		m_Qualities.RemoveDataID(PHYSICS_EFFECT_TABLE_DID);
		NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		if (physics_script_table)
		{
			PhysicsScriptTable::Release(physics_script_table);
			physics_script_table = NULL;
		}
	}

	double floatValue;
	if (hookedItem->m_Qualities.InqFloat(DEFAULT_SCALE_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, floatValue);
		NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(floatValue);
	}
	else
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, 1.0);
		NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(1.0);
	}

	if (hookedItem->m_Qualities.InqFloat(TRANSLUCENCY_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, floatValue);
		NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(floatValue);
	}
	else
	{
		m_Qualities.RemoveFloat(TRANSLUCENCY_FLOAT);
		NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(0.0);
	}

	set_state(PhysicsState::LIGHTING_ON_PS | PhysicsState::REPORT_COLLISIONS_PS, TRUE);

	m_bObjDescOverride = true;
	hookedItem->GetObjDesc(m_ObjDescOverride);
	if (sendUpdate && InqBoolQuality(VISIBILITY_BOOL, true))
		NotifyObjectUpdated(false);
}

void CHookWeenie::ClearHookedObject(bool sendUpdate)
{
	CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(m_Qualities.id);

	DWORD value;
	if (defaults->m_Qualities.InqDataID(SETUP_DID, value))
	{
		m_Qualities.SetDataID(SETUP_DID, value);
		NotifyDIDStatUpdated(SETUP_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SETUP_DID);
		NotifyDIDStatUpdated(SETUP_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(MOTION_TABLE_DID, value))
	{
		m_Qualities.SetDataID(MOTION_TABLE_DID, value);
		NotifyDIDStatUpdated(MOTION_TABLE_DID, false);

		_phys_obj->SetMotionTableID(value);
	}
	else
	{
		m_Qualities.RemoveDataID(MOTION_TABLE_DID);
		NotifyDIDStatUpdated(MOTION_TABLE_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(SOUND_TABLE_DID, value))
	{
		m_Qualities.SetDataID(SOUND_TABLE_DID, value);
		NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SOUND_TABLE_DID);
		NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(PHYSICS_EFFECT_TABLE_DID, value))
	{
		m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, value);
		NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		set_phstable_id(value);
	}
	else
	{
		m_Qualities.RemoveDataID(PHYSICS_EFFECT_TABLE_DID);
		NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		if (physics_script_table)
		{
			PhysicsScriptTable::Release(physics_script_table);
			physics_script_table = NULL;
		}
	}

	double floatValue;
	if (defaults->m_Qualities.InqFloat(DEFAULT_SCALE_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, floatValue);
		NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(floatValue);
	}
	else
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, 1.0);
		NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(1.0);
	}

	if (defaults->m_Qualities.InqFloat(TRANSLUCENCY_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, floatValue);
		NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(floatValue);
	}
	else
	{
		m_Qualities.RemoveFloat(TRANSLUCENCY_FLOAT);
		NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(0.0);
	}

	set_state(ETHEREAL_PS | IGNORE_COLLISIONS_PS, TRUE);

	m_bObjDescOverride = false;
	m_ObjDescOverride.Clear();

	if(sendUpdate && InqBoolQuality(VISIBILITY_BOOL, true))
		NotifyObjectUpdated(false);
}

void CHookWeenie::SetHookVisibility(bool newSetting)
{
	CHouseData *houseData = GetHouseData();

	if(houseData)
		houseData->_hooksVisible = newSetting;

	if (!m_Items.empty())
	{
		CWeenieObject *hookedItem = m_Items[0];

		if (!hookedItem)
			return;

		m_Qualities.RemoveBool(VISIBILITY_BOOL); //if it's visible we remove the bool altogether.
		NotifyBoolStatUpdated(VISIBILITY_BOOL, false);

		if (newSetting)
		{
			//revert to hook's info.
			CWeenieDefaults *weenieDef = g_pWeenieFactory->GetWeenieDefaults(m_Qualities.id);
			if (weenieDef)
			{
				if (m_Qualities._emote_table)
					SafeDelete(m_Qualities._emote_table);

				m_Qualities.SetString(NAME_STRING, weenieDef->m_Qualities.GetString(NAME_STRING, ""));
				m_Qualities.SetInt(ITEMS_CAPACITY_INT, weenieDef->m_Qualities.GetInt(ITEMS_CAPACITY_INT, 0));
			}

			NotifyObjectUpdated(false);
		}
		else
		{
			//overwrite hook with hooked item's info.
			if (hookedItem->m_Qualities._emote_table)
			{
				SafeDelete(m_Qualities._emote_table);
				m_Qualities._emote_table = new CEmoteTable();
				BinaryWriter w;
				hookedItem->m_Qualities._emote_table->Pack(&w);
				BinaryReader r(w.GetData(), w.GetSize());
				m_Qualities._emote_table->UnPack(&r);
			}
			else
				SafeDelete(m_Qualities._emote_table);

			if (CWeenieObject *hookedItem = m_Items[0])
			{
				m_Qualities.SetString(NAME_STRING, hookedItem->GetName());
				m_Qualities.SetInt(ITEMS_CAPACITY_INT, hookedItem->m_Qualities.GetInt(ITEMS_CAPACITY_INT, 0));
			}

			NotifyObjectUpdated(false);
		}
	}
	else
	{
		if (newSetting)
		{
			m_Qualities.RemoveBool(VISIBILITY_BOOL); //if it's visible we remove the bool altogether.
			NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
			NotifyObjectCreated(false);
		}
		else
		{
			m_Qualities.SetBool(VISIBILITY_BOOL, false);
			NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
			NotifyObjectRemoved();
		}
	}
}


CHouseWeenie *CHookWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}

CHouseData *CHookWeenie::GetHouseData()
{
	CHouseWeenie *house = GetHouse();
	if (house)
		return house->GetHouseData();
	else
		return NULL;
}

CDeedWeenie::CDeedWeenie()
{
}

CHouseWeenie *CDeedWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}

CBootSpotWeenie::CBootSpotWeenie()
{
	m_bDontClear = true;
}

CHouseWeenie *CBootSpotWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}

CHousePortalWeenie::CHousePortalWeenie()
{
	m_bDontClear = true;
}

void CHousePortalWeenie::ApplyQualityOverrides()
{
	SetRadarBlipColor(Portal_RadarBlipEnum);
}

CHouseWeenie *CHousePortalWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}

int CHousePortalWeenie::Use(CPlayerWeenie *other)
{
	CHouseWeenie *house = GetHouse();
	if (!house)
	{
		other->NotifyUseDone();
		return WERROR_NO_HOUSE;
	}

	if (house->HasAccess(other->AsPlayer()))
		return CPortal::Use(other);
	else
	{
		other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
		other->NotifyUseDone();
		return WERROR_NONE;
	}
}


bool CHousePortalWeenie::GetDestination(Position &dest)
{
	if (CHouseWeenie *house = GetHouse())
	{
		if (Position *pos = g_pPortalDataEx->GetHousePortalDest(house->GetHouseDID(), m_Position.objcell_id))
		{
			dest = *pos;
			return true;
		}
	}

	return false;
}

CStorageWeenie::CStorageWeenie()
{
	m_bDontClear = true;
}

int CStorageWeenie::DoUseResponse(CWeenieObject *other)
{
	CHouseWeenie *house = GetHouse();
	if (!house)
		return WERROR_NO_HOUSE;

	if(house->HasStorageAccess(other->AsPlayer()))
		return CContainerWeenie::DoUseResponse(other);
	else
	{
		other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
		return WERROR_NONE;
	}
}

CHouseWeenie *CStorageWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}
