#include <StdAfx.h>
#include "AllegianceBanRemove_02A2.h"
#include "AllegianceManager.h"

MAllegianceBanRemove_02A2::MAllegianceBanRemove_02A2(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceBanRemove_02A2::Parse(BinaryReader * reader)
{
	m_szCharName = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a remove allegiance ban message (0x02A2) from the client.";
		return;
	}

	Process();
}

void MAllegianceBanRemove_02A2::Process()
{
	g_pAllegianceManager->RemoveBan(m_pPlayer, m_szCharName);
}
