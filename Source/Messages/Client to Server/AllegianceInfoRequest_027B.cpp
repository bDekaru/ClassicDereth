#include <StdAfx.h>
#include "AllegianceInfoRequest_027B.h"
#include "AllegianceManager.h"

MAllegianceInfoRequest_027B::MAllegianceInfoRequest_027B(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceInfoRequest_027B::Parse(BinaryReader * reader)
{
	m_szTargetPlayer = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance info request message (0x027B) from the client.";
		return;
	}

	Process();
}

void MAllegianceInfoRequest_027B::Process()
{
	g_pAllegianceManager->AllegianceInfoRequest(m_pPlayer, m_szTargetPlayer);
}
