#include <StdAfx.h>
#include "AllegianceOfficersList_02A6.h"
#include "AllegianceManager.h"
#include "DatabaseIO.h"

MAllegianceOfficersList_02A6::MAllegianceOfficersList_02A6(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficersList_02A6::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceOfficersList_02A6::Process()
{
	g_pAllegianceManager->ListOfficers(m_pPlayer);
}
