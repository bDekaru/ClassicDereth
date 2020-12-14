#include <StdAfx.h>
#include "AllegianceHouseAction_0042.h"
#include "AllegianceManager.h"
#include "House.h"

MAllegianceHouseAction_0042::MAllegianceHouseAction_0042(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceHouseAction_0042::Parse(BinaryReader * reader)
{
	m_dwHouseAction = reader->ReadUInt32();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance house action message (0x0042) from the client.";
		return;
	}

	Process();
}

void MAllegianceHouseAction_0042::Process()
{
	/* TODO: this should move to house manager
	uint32_t playerID = m_pPlayer->GetID();
	if (g_pAllegianceManager->IsMonarch(playerID) || g_pAllegianceManager->IsOfficerWithLevel(playerID, Seneschal_AllegianceOfficerLevel))
	{
		if (m_dwHouseAction == CheckStatus_AllegianceHouseAction)
		{
			// TODO is there a server message to reply to this?
		}
		else if (CHouseData* houseData = g_pHouseManager->GetHouseData(m_pPlayer->GetAccountHouseId()))
		{
			switch (m_dwHouseAction)
			{
				case GuestOpen_AllegianceHouseAction:
				{
					houseData->_everyoneAccess = true;
				}
				case GuestClose_AllegianceHouseAction:
				{
					houseData->_everyoneAccess = false;
				}
				case StorageOpen_AllegianceHouseAction:
				{
					houseData->_everyoneStorageAccess = true;
				}
				case StorageClose_AllegianceHouseAction:
				{
					houseData->_everyoneStorageAccess = false;
				}
			}
		}
	}*/
}
