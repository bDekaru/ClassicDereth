#include <StdAfx.h>
#include "AllegianceMOTDSet_0254.h"
#include "AllegianceManager.h"

MAllegianceMOTDSet_0254::MAllegianceMOTDSet_0254(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceMOTDSet_0254::Parse(BinaryReader * reader)
{
	m_szMessage = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a set allegiance MOTD message (0x0254) from the client.";
		return;
	}

	Process();
}

void MAllegianceMOTDSet_0254::Process()
{
	g_pAllegianceManager->SetMOTD(m_pPlayer, m_szMessage);
}
