#include <StdAfx.h>
#include "AllegianceOfficerRemove_02A5.h"
#include "AllegianceManager.h"
#include "DatabaseIO.h"
#include "World.h"

MAllegianceOfficerRemove_02A5::MAllegianceOfficerRemove_02A5(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficerRemove_02A5::Parse(BinaryReader * reader)
{
	m_szCharName = reader->ReadString();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing a remove allegiance officer message (0x02A5) from the client.";
		return;
	}

	Process();
}

void MAllegianceOfficerRemove_02A5::Process()
{
	g_pAllegianceManager->RemoveOfficer(m_pPlayer, m_szCharName);
}
