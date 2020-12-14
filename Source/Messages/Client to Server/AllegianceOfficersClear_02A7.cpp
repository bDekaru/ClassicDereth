#include <StdAfx.h>
#include "AllegianceOfficersClear_02A7.h"
#include "AllegianceManager.h"

MAllegianceOfficersClear_02A7::MAllegianceOfficersClear_02A7(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficersClear_02A7::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceOfficersClear_02A7::Process()
{
	g_pAllegianceManager->ClearOfficers(m_pPlayer);
}
