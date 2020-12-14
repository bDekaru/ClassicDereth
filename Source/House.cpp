#include <StdAfx.h>
#include "House.h"
#include "World.h"
#include "WeenieFactory.h"
#include "Player.h"
#include "InferredPortalData.h"
#include "AllegianceManager.h"
#include "Client.h"
#include "EmoteManager.h"
#include "HouseManager.h"

#define HOUSE_SAVE_INTERVAL 300.0


DEFINE_PACK(CHouseData)
{
	pWriter->Write<uint32_t>(_slumLordId);
	pWriter->Write<uint32_t>(_ownerId);
	pWriter->Write<uint32_t>(_ownerAccount);
	pWriter->Write<uint32_t>(_purchaseTimestamp);
	pWriter->Write<uint32_t>(_currentMaintenancePeriod);
	pWriter->Write<uint32_t>(_houseType);
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
	_slumLordId = pReader->Read<uint32_t>();
	_ownerId = pReader->Read<uint32_t>();
	_ownerAccount = pReader->Read<uint32_t>();
	_purchaseTimestamp = pReader->Read<uint32_t>();
	_currentMaintenancePeriod = pReader->Read<uint32_t>();
	_houseType = pReader->Read<uint32_t>();
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
	_accessList.clear();
	_storageAccessList.clear();
}

void CHouseData::SetHookVisibility(bool newSetting)
{
	_hooksVisible = newSetting;

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
	Save();
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
	SetHookVisibility(true);

	if (CWeenieObject *slumLord = g_pWorld->FindObject(_slumLordId))
	{
		slumLord->DoForcedMotion(Motion_Off);
		CHouseWeenie* house = slumLord->AsSlumLord()->GetHouse();
		if (house)
			house->m_Qualities.SetString(HOUSE_OWNER_NAME_STRING, "");
	}

	if (owner)
		g_pHouseManager->SendHouseData(owner->AsPlayer(), _houseId);

	Save();
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
	source->m_Qualities.SetDataID(HOUSEID_DID, GetHouseDID());

	if (source->AsSlumLord())
	{
		m_Qualities.SetInstanceID(SLUMLORD_IID, source->GetID());
	}

	if (source->AsHook())
	{
		CHouseData *houseData = source->AsHook()->GetHouseData();
		if (houseData)
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
	uint32_t owner = GetHouseOwner();

	if (!owner)
		return "";

	std::string ownerName;
	ownerName = g_pWorld->GetPlayerName(owner, true);
	if (ownerName.empty())
	{
		if (!m_Qualities.InqString(HOUSE_OWNER_NAME_STRING, ownerName))
		{
			ownerName = g_pDBIO->GetPlayerCharacterName(GetHouseData()->_ownerId);
			m_Qualities.SetString(HOUSE_OWNER_NAME_STRING, ownerName);
		}
	}

	return ownerName;
}

uint32_t CHouseWeenie::GetHouseOwner()
{
	return GetHouseData()->_ownerId;
	//return InqIIDQuality(HOUSE_OWNER_IID, 0);
}

uint32_t CHouseWeenie::GetHouseDID()
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

	uint32_t requesterId = requester->GetID();
	uint32_t houseOwnerId = GetHouseOwner();

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
		std::string alleg;
		if (requester->m_Qualities.InqString(ALLEGIANCE_NAME_STRING, alleg)) {

			AllegianceTreeNode *ownerAllegianceNode = g_pAllegianceManager->GetTreeNode(houseOwnerId);
			AllegianceTreeNode *requesterAllegianceNode = g_pAllegianceManager->GetTreeNode(requesterId);

			if (ownerAllegianceNode && requesterAllegianceNode)
			{
				if (ownerAllegianceNode->_monarchID == requesterAllegianceNode->_monarchID)
					return true;
				else
					return false;
			}
			else
			{
				if (!ownerAllegianceNode)
					SERVER_ERROR << "House Owner:" << houseOwnerId << " may not exist.  Check DB";
				else
					SERVER_ERROR << "Requestor:" << requesterId << " may not have or be in allegiance.";
			}

		}
	}

	return HasStorageAccess(requester); //storage access automatically grants access.
}

bool CHouseWeenie::HasStorageAccess(CPlayerWeenie *requester)
{
	if (!requester)
		return false;

	CHouseData *houseData = GetHouseData();

	uint32_t requesterId = requester->GetID();
	uint32_t houseOwnerId = GetHouseOwner();

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
		std::string alleg;
		if (requester->m_Qualities.InqString(ALLEGIANCE_NAME_STRING, alleg)) {
			AllegianceTreeNode *ownerAllegianceNode = g_pAllegianceManager->GetTreeNode(houseOwnerId);
			AllegianceTreeNode *requesterAllegianceNode = g_pAllegianceManager->GetTreeNode(requesterId);

			if (ownerAllegianceNode && requesterAllegianceNode)
			{
				if (ownerAllegianceNode->_monarchID == requesterAllegianceNode->_monarchID)
					return true;
			}
		}
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
			else
				DoForcedMotion(Motion_Off);

			//for (auto hook_id : houseData->_hookList)
			//{
			//	CWeenieObject *hook = g_pWorld->FindObject(hook_id, true);

			//	if (!hook)
			//		continue;

			//	if(hook->AsHook())
			//		hook->AsHook()->UpdateHookedObject();
			//}

			_initialized = true;

			_nextSave = Timer::cur_time + HOUSE_SAVE_INTERVAL;
		}
		else
		{
			CHouseWeenie *house = GetHouse();
			if (!house)
				return;
			CHouseData *houseData = house->GetHouseData();
			if (houseData->_ownerId)
				DoForcedMotion(Motion_On);
			else
				DoForcedMotion(Motion_Off);

			//check global data and update if necessary
			uint32_t now = g_pPhatSDK->GetCurrTimeStamp();
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

		uint32_t houseDID = 0;
		if (other->GetID() == houseData->_ownerId && (!other->m_Qualities.InqDataID(HOUSEID_DID, houseDID) || houseDID == 0)) // player lost link
		{
			other->m_Qualities.SetDataID(HOUSEID_DID, houseData->_houseId);
		}
	}

	BinaryWriter profMsg;
	profMsg.Write<uint32_t>(0x21D);
	prof.Pack(&profMsg);
	other->SendNetMessage(&profMsg, PRIVATE_MSG, TRUE, FALSE);

	return CWeenieObject::DoUseResponse(other);
}

void CSlumLordWeenie::BuyHouse(CPlayerWeenie *player, const PackableList<uint32_t> &items)
{
	if (CHouseWeenie *house = GetHouse())
	{
		if (player->GetAccountHouseId())
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

		if (house->GetHouseType() != Apartment_HouseType && player->InqIntQuality(HOUSE_PURCHASE_TIMESTAMP_INT, 0) + (30 * 24 * 60) > g_pPhatSDK->GetCurrTimeStamp())
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

			hook->Save();
		}
		houseData->SetHookVisibility(true);

		DoForcedMotion(Motion_On);
		g_pHouseManager->SendHouseData(player, house->GetHouseDID());
		DoUseResponse(player);
		player->RecalculateCoinAmount(W_COINSTACK_CLASS);
		player->Save();
		house->m_Qualities.SetString(HOUSE_OWNER_NAME_STRING, player->GetName());
		house->Save();
		houseData->Save();

		HOUSE_LOG << "Player:" << player->GetName() << "bought house" << houseData->_houseId;
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
				houseData->Save();

				if (CWeenieObject *owner = g_pWorld->FindObject(houseData->_ownerId))
					g_pHouseManager->SendHouseData(owner->AsPlayer(), house->GetHouseDID()); //update house's owner panel if the owner is online.
			}
			else
			{
				//houseData->AbandonHouse(); temporarily disable abandon house on rent not paid
			}
		}
	}
}

void CSlumLordWeenie::RentHouse(CPlayerWeenie *player, const PackableList<uint32_t> &items)
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

		DoUseResponse(player);
		player->RecalculateCoinAmount(W_COINSTACK_CLASS);
		player->Save();
		house->Save();
		houseData->Save();


		if (CWeenieObject *owner = g_pWorld->FindObject(houseData->_ownerId))
			g_pHouseManager->SendHouseData(owner->AsPlayer(), house->GetHouseDID()); //update house's owner panel if the owner is online.

		HOUSE_LOG << "Player:" << player->GetName() << "paid rent" << houseData->_houseId;
	}
}


CHookWeenie::CHookWeenie()
{
	m_bDontClear = true;
	_nextInitCheck = Timer::cur_time;
}

void CHookWeenie::Tick()
{
	//if (!_initialized && _nextInitCheck != -1.0 && _nextInitCheck <= Timer::cur_time)
	//{
	//	_nextInitCheck = Timer::cur_time + Random::GenUInt(1, 10);

	//	CHouseData *houseData = GetHouseData();

	//	if (!houseData)
	//		return;

	//	if (houseData->_ownerId)
	//	{
	//		SetHookVisibility(houseData->_hooksVisible);
	//		m_Qualities.SetInstanceID(HOUSE_OWNER_IID, houseData->_ownerId);
	//		NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
	//	}
	//	else
	//	{
	//		m_Qualities.RemoveInstanceID(HOUSE_OWNER_IID);
	//		NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
	//	}

	//	UpdateHookedObject(NULL);
	//	SetHookVisibility(houseData->_hooksVisible);

	//	_nextInitCheck = -1.0;
	//	_initialized = true;
	//}

	CContainerWeenie::Tick();
}

void CHookWeenie::SaveEx(CWeenieSave & save)
{
	//if we don't clear before saving some hooked item's position and rotation changes, not sure what is causing that.
	ClearHookedObject(false);
	CContainerWeenie::SaveEx(save);

	m_bSaveMe = false;

	UpdateHookedObject(NULL, false);
}

void CHookWeenie::LoadEx(CWeenieSave & save)
{
	CContainerWeenie::LoadEx(save);

	_nextInitCheck = Timer::cur_time;
}

void CHookWeenie::PreSpawnCreate()
{
	CHouseData *houseData = GetHouseData();

	if (!houseData)
		return;

	if (houseData->_ownerId)
	{
		//SetHookVisibility(houseData->_hooksVisible);
		m_Qualities.SetInstanceID(HOUSE_OWNER_IID, houseData->_ownerId);
		//NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
	}
	else
	{
		m_Qualities.RemoveInstanceID(HOUSE_OWNER_IID);
		//NotifyIIDStatUpdated(HOUSE_OWNER_IID, false);
	}

	if (houseData->_hooksVisible)
	{
		m_Qualities.RemoveBool(VISIBILITY_BOOL);
	}
	else if (m_Items.empty())
	{
		m_Qualities.SetBool(VISIBILITY_BOOL, false);
	}

	UpdateHookedObject(NULL, false);
	//SetHookVisibility(houseData->_hooksVisible);
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
		{
			int useResponse = CContainerWeenie::DoUseResponse(other);
			m_bSaveMe = true;
			Save();
			return useResponse;
		}
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

			if (hookedItem->InqIntQuality(HOOK_GROUP_INT, 0) == 4 && house->GetHouseType() != Mansion_HouseType)
			{
				other->SendText("That can only be activated when hooked in a mansion.", LTT_DEFAULT); //made up message.
				return WERROR_OBJECT_GONE;
			}

			// Prevent hooked items from being usable if recently involved in PK Activity.
			if (hookedItem->InqIntQuality(HOOK_GROUP_INT, 0) && other->_IsPlayer() && other->AsPlayer()->CheckPKActivity())
			{
				return WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY;
			}
		}
		else
		{
			other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
			return WERROR_HOOK_NOT_PERMITED_TO_USE_HOOK;
		}
	}

	return CWeenieObject::DoUseResponse(other);
}

void CHookWeenie::Identify(CWeenieObject *other, uint32_t overrideId)
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

uint32_t CHookWeenie::Container_InsertInventoryItem(uint32_t dwCell, CWeenieObject *pItem, uint32_t slot)
{
	UpdateHookedObject(pItem);

	return CContainerWeenie::Container_InsertInventoryItem(dwCell, pItem, slot);
}

void CHookWeenie::ReleaseContainedItemRecursive(CWeenieObject *item)
{
	ClearHookedObject();

	CContainerWeenie::ReleaseContainedItemRecursive(item);
}

void CHookWeenie::OnContainerClosed(CWeenieObject *requestedBy)
{
	CContainerWeenie::OnContainerClosed(requestedBy);

	m_bSaveMe = true;
	Save();
}

void CHookWeenie::UpdateHookedObject(CWeenieObject *hookedItem, bool sendUpdate)
{
	if (!hookedItem && !m_Items.empty())
		hookedItem = m_Items[0];

	if (!hookedItem)
		return;

	uint32_t value;

	if (hookedItem->m_Qualities.InqDataID(SETUP_DID, value))
	{
		m_Qualities.SetDataID(SETUP_DID, value);
		_phys_obj->part_array->SetSetupID(value, TRUE);
		//NotifyDIDStatUpdated(SETUP_DID, false);
		//SetPlacementFrame(Placement::Hook, FALSE);
	}
	else
	{
		m_Qualities.RemoveDataID(SETUP_DID);
		//NotifyDIDStatUpdated(SETUP_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(MOTION_TABLE_DID, value))
	{
		m_Qualities.SetDataID(MOTION_TABLE_DID, value);
		//NotifyDIDStatUpdated(MOTION_TABLE_DID, false);

		_phys_obj->SetMotionTableID(value);
	}
	else
	{
		m_Qualities.RemoveDataID(MOTION_TABLE_DID);
		//NotifyDIDStatUpdated(MOTION_TABLE_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(SOUND_TABLE_DID, value))
	{
		m_Qualities.SetDataID(SOUND_TABLE_DID, value);
		//NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SOUND_TABLE_DID);
		//NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}

	if (hookedItem->m_Qualities.InqDataID(PHYSICS_EFFECT_TABLE_DID, value))
	{
		m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, value);
		//NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		set_phstable_id(value);
	}
	else
	{
		m_Qualities.RemoveDataID(PHYSICS_EFFECT_TABLE_DID);
		//NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

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
		//NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(floatValue);
	}
	else
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, 1.0);
		//NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(1.0);
	}

	if (hookedItem->m_Qualities.InqFloat(TRANSLUCENCY_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, floatValue);
		//NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(floatValue);
	}
	else
	{
		m_Qualities.RemoveFloat(TRANSLUCENCY_FLOAT);
		//NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(0.0);
	}

	set_state(PhysicsState::LIGHTING_ON_PS | PhysicsState::REPORT_COLLISIONS_PS, TRUE);

	m_bObjDescOverride = true;
	hookedItem->GetObjDesc(m_ObjDescOverride);

	int32_t placement = hookedItem->InqIntQuality(HOOK_PLACEMENT_INT, Placement::Resting, TRUE);
	if (placement != Placement::Resting)
		SetPlacementFrame(placement, FALSE);

	if (sendUpdate && InqBoolQuality(VISIBILITY_BOOL, true))
		NotifyObjectUpdated(false);
}

void CHookWeenie::ClearHookedObject(bool sendUpdate)
{
	CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(m_Qualities.id);

	uint32_t value;

	if (defaults->m_Qualities.InqDataID(SETUP_DID, value))
	{
		m_Qualities.SetDataID(SETUP_DID, value);
		//NotifyDIDStatUpdated(SETUP_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SETUP_DID);
		//NotifyDIDStatUpdated(SETUP_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(MOTION_TABLE_DID, value))
	{
		m_Qualities.SetDataID(MOTION_TABLE_DID, value);
		//NotifyDIDStatUpdated(MOTION_TABLE_DID, false);

		_phys_obj->SetMotionTableID(value);
	}
	else
	{
		m_Qualities.RemoveDataID(MOTION_TABLE_DID);
		//NotifyDIDStatUpdated(MOTION_TABLE_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(SOUND_TABLE_DID, value))
	{
		m_Qualities.SetDataID(SOUND_TABLE_DID, value);
		//NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}
	else
	{
		m_Qualities.RemoveDataID(SOUND_TABLE_DID);
		//NotifyDIDStatUpdated(SOUND_TABLE_DID, false);
	}

	if (defaults->m_Qualities.InqDataID(PHYSICS_EFFECT_TABLE_DID, value))
	{
		m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, value);
		//NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

		set_phstable_id(value);
	}
	else
	{
		m_Qualities.RemoveDataID(PHYSICS_EFFECT_TABLE_DID);
		//NotifyDIDStatUpdated(PHYSICS_EFFECT_TABLE_DID, false);

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
		//NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(floatValue);
	}
	else
	{
		m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, 1.0);
		//NotifyFloatStatUpdated(DEFAULT_SCALE_FLOAT, false);

		_phys_obj->SetScaleStatic(1.0);
	}

	if (defaults->m_Qualities.InqFloat(TRANSLUCENCY_FLOAT, floatValue))
	{
		m_Qualities.SetFloat(TRANSLUCENCY_FLOAT, floatValue);
		//NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(floatValue);
	}
	else
	{
		m_Qualities.RemoveFloat(TRANSLUCENCY_FLOAT);
		//NotifyFloatStatUpdated(TRANSLUCENCY_FLOAT, false);

		_phys_obj->SetTranslucencyInternal(0.0);
	}

	set_state(ETHEREAL_PS | IGNORE_COLLISIONS_PS, TRUE);

	m_bObjDescOverride = false;
	m_ObjDescOverride.Clear();

	SetPlacementFrame(Placement::Resting, FALSE);

	if (sendUpdate && InqBoolQuality(VISIBILITY_BOOL, true))
		NotifyObjectUpdated(false);
}

void CHookWeenie::SetHookVisibility(bool newSetting)
{
	if (!m_Items.empty())
	{
		CWeenieObject *hookedItem = m_Items[0];

		if (!hookedItem)
			return;

		m_Qualities.RemoveBool(VISIBILITY_BOOL); //if it's visible we remove the bool altogether.
		//NotifyBoolStatUpdated(VISIBILITY_BOOL, false);

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
			//NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
			NotifyObjectCreated(false);
		}
		else
		{
			m_Qualities.SetBool(VISIBILITY_BOOL, false);
			//NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
			NotifyObjectRemoved();
		}
	}
	Save();
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
	if (uint32_t hid = InqDIDQuality(HOUSEID_DID, 0))
		return g_pHouseManager->GetHouseData(hid);

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

	if (house->HasStorageAccess(other->AsPlayer()))
	{
		int storageUseResponse = CContainerWeenie::DoUseResponse(other);
		return storageUseResponse;
	}
	else
	{
		other->SendText("You do not have permission to access that.", LTT_DEFAULT); //made up message.
	}

	return CWeenieObject::DoUseResponse(other);
}

void CStorageWeenie::OnContainerClosed(CWeenieObject *requestedBy)
{
	CChestWeenie::OnContainerClosed(requestedBy);

	m_bSaveMe = true;
	Save();
}

CHouseWeenie *CStorageWeenie::GetHouse()
{
	if (CWeenieObject *house = g_pWorld->FindObject(InqIIDQuality(HOUSE_IID, 0), true))
	{
		return house->AsHouse();
	}

	return NULL;
}
