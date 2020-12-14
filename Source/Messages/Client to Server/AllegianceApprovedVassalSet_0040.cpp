#include <StdAfx.h>
#include "AllegianceApprovedVassalSet_0040.h"
#include "AllegianceManager.h"
#include "World.h"

MAllegianceApprovedVassalSet_0040::MAllegianceApprovedVassalSet_0040(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceApprovedVassalSet_0040::Parse(BinaryReader * reader)
{
	m_szCharName = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance set approved vassal message (0x0040) from the client.";
		return;
	}

	Process();
}

void MAllegianceApprovedVassalSet_0040::Process()
{
	g_pAllegianceManager->ApproveVassal(m_pPlayer, m_szCharName);
}
