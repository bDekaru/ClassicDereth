#include <StdAfx.h>
#include "AllegianceNameSet_0033.h"
#include "AllegianceManager.h"
#include "InferredPortalData.h"

MAllegianceNameSet_0033::MAllegianceNameSet_0033(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceNameSet_0033::Parse(BinaryReader * reader)
{
	m_szNewName = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a set allegiance name message (0x0033) from the client.";
		return;
	}

	Process();
}

void MAllegianceNameSet_0033::Process()
{
	g_pAllegianceManager->SetAllegianceName(m_pPlayer, m_szNewName);
}
