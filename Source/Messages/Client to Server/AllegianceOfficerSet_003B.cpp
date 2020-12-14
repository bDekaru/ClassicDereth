#include <StdAfx.h>
#include "AllegianceOfficerSet_003B.h"
#include "AllegianceManager.h"

MAllegianceOfficerSet_003B::MAllegianceOfficerSet_003B(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficerSet_003B::Parse(BinaryReader * reader)
{
	m_szOfficerName = reader->ReadString();
	if (reader->GetLastError()) return;
	m_dwOfficerLevel = reader->ReadUInt32();
	
	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a set allegiance officer message (0x003B) from the client.";
		return;
	}

	Process();
}

void MAllegianceOfficerSet_003B::Process()
{
	g_pAllegianceManager->SetOfficer(m_pPlayer, m_szOfficerName, static_cast<eAllegianceOfficerLevel>(m_dwOfficerLevel));
}
