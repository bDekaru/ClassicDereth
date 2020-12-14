#include <StdAfx.h>
#include "AllegianceBoot_0277.h"
#include "AllegianceManager.h"
#include "Player.h"

MAllegianceBoot_0277::MAllegianceBoot_0277(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceBoot_0277::Parse(BinaryReader * reader)
{
	m_szBooteeName = reader->ReadString();
	m_bAccountBoot = reader->Read<bool>();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance boot message (0x0277) from the client.";
		return;
	}

	Process();
}

void MAllegianceBoot_0277::Process()
{
	g_pAllegianceManager->BootPlayer(m_pPlayer, m_szBooteeName, m_bAccountBoot);
}
