#include <StdAfx.h>
#include "AllegianceOfficerTitlesClear_003E.h"
#include "AllegianceManager.h"

MAllegianceOfficerTitlesClear_003E::MAllegianceOfficerTitlesClear_003E(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficerTitlesClear_003E::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceOfficerTitlesClear_003E::Process()
{
	g_pAllegianceManager->ClearOfficerTitles(m_pPlayer);
}
