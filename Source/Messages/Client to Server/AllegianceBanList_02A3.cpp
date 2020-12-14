#include <StdAfx.h>
#include "AllegianceBanList_02A3.h"
#include "AllegianceManager.h"

MAllegianceBanList_02A3::MAllegianceBanList_02A3(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceBanList_02A3::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceBanList_02A3::Process()
{
	g_pAllegianceManager->GetBanList(m_pPlayer);
}
