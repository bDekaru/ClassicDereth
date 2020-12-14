#include <StdAfx.h>
#include "AllegianceMOTDClear_0256.h"
#include "AllegianceManager.h"

MAllegianceMOTDClear_0256::MAllegianceMOTDClear_0256(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceMOTDClear_0256::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceMOTDClear_0256::Process()
{
	g_pAllegianceManager->ClearMOTD(m_pPlayer);
}
