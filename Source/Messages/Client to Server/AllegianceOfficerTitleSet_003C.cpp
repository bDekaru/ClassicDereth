#include <StdAfx.h>
#include "AllegianceOfficerTitleSet_003C.h"
#include "AllegianceManager.h"
#include "InferredPortalData.h"

MAllegianceOfficerTitleSet_003C::MAllegianceOfficerTitleSet_003C(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficerTitleSet_003C::Parse(BinaryReader * reader)
{
	m_OfficerLevel = reader->ReadUInt32();
	m_OfficerTitle = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a set allegiance officer title message (0x003C) from the client.";
		return;
	}

	Process();
}

void MAllegianceOfficerTitleSet_003C::Process()
{
	g_pAllegianceManager->SetOfficerTitle(m_pPlayer, m_OfficerLevel, m_OfficerTitle);
}
