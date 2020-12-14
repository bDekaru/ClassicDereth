#include <StdAfx.h>
#include "AllegianceSwear_001D.h"
#include "AllegianceManager.h"
#include "World.h"

MAllegianceSwear_001D::MAllegianceSwear_001D(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceSwear_001D::Parse(BinaryReader * reader)
{
	m_dwTarget = reader->ReadUInt32();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a swear allegiance message (0x001D) from the client.";
		return;
	}

	Process();
}

void MAllegianceSwear_001D::Process()
{
	if(CPlayerWeenie *targetWeenie = g_pWorld->FindPlayer(m_dwTarget))
	{
		g_pAllegianceManager->TrySwearAllegiance(m_pPlayer, targetWeenie);
		g_pAllegianceManager->SendAllegianceProfile(m_pPlayer);
	}
}
