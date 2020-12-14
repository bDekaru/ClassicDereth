#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "House.h"
#include "HouseManager.h"
#include "ChatMsgs.h"

#include "ClientCommands.h"

CLIENT_COMMAND(cleanhousing, "", "Clears housing from delete chars and perma banned accounts", ADMIN_ACCESS, SERVER_CATEGORY)
{
	g_pHouseManager->ReleaseDeletedCharacterHousing();
	g_pHouseManager->ReleaseBannedCharacterHousing();
	g_pHouseManager->LoadHousingMap();
	return false;
}

CLIENT_COMMAND(waivenextrent, "<on/off>", "Toggles this rent period rent.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		g_pWorld->BroadcastGlobal(ServerBroadcast("System", "This rent period has been waived, rents do not need to be paid for this period.", LTT_DEFAULT), PRIVATE_MSG);
		g_pHouseManager->_freeHouseMaintenancePeriod = true;
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		g_pWorld->BroadcastGlobal(ServerBroadcast("System", "Rent is back in effect for this rent period.", LTT_DEFAULT), PRIVATE_MSG);
		g_pHouseManager->_freeHouseMaintenancePeriod = false;
	}
	else
		return true;

	//update everyone's house panel
	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		CPlayerWeenie *player = i->second;
		uint32_t houseId = player->InqDIDQuality(HOUSEID_DID, 0);
		if (houseId)
			g_pHouseManager->SendHouseData(player, houseId);
		else
		{
			//if we can't get the data send the "no house" packet
			BinaryWriter noHouseData;
			noHouseData.Write<uint32_t>(0x0226);
			noHouseData.Write<uint32_t>(0);
			player->SendNetMessage(&noHouseData, PRIVATE_MSG, TRUE, FALSE);
		}
	}

	return false;
}

CLIENT_COMMAND(resethousepurchasetimestamp, "", "Resets the house purchase timestamp allowing the purchase of another house instantly.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	pPlayer->m_Qualities.RemoveInt(HOUSE_PURCHASE_TIMESTAMP_INT);
	pPlayer->NotifyIntStatUpdated(HOUSE_PURCHASE_TIMESTAMP_INT);
	pPlayer->SendText("Your house purchase timestamp has been reset.", LTT_DEFAULT);

	uint32_t houseId = pPlayer->InqDIDQuality(HOUSEID_DID, 0);
	if (houseId)
		g_pHouseManager->SendHouseData(pPlayer, houseId);
	else
	{
		//if we can't get the data send the "no house" packet
		BinaryWriter noHouseData;
		noHouseData.Write<uint32_t>(0x0226);
		noHouseData.Write<uint32_t>(0);
		pPlayer->SendNetMessage(&noHouseData, PRIVATE_MSG, TRUE, FALSE);
	}

	return false;
}

CLIENT_COMMAND(evict, "", "Force abandons a house either last assessed or by house id", ADMIN_ACCESS, SERVER_CATEGORY)
{
	CWeenieObject *pObject = g_pWorld->FindWithinPVS(pPlayer, pPlayer->m_LastAssessed);

	if (pObject && pObject->AsSlumLord())
	{
		CSlumLordWeenie* slumlord = static_cast<CSlumLordWeenie*>(pObject);
		auto house = slumlord->GetHouse();
		if (house)
		{
			auto hd = house->GetHouseData();
			if (hd)
				hd->AbandonHouse();
		}
		return false;
	}

	uint32_t houseId = 0;
	if (argc >= 1)
	{
		houseId = strtoul(argv[0], NULL, 10);

		if (houseId == 0)
		{
			return false;
		}
		auto house = g_pHouseManager->GetHouseData((uint32_t)houseId);
		house->AbandonHouse();
	}

	return false;
}
