#include <StdAfx.h>
#include "AllegianceRecallHometown_02AB.h"
#include "AllegianceManager.h"

MAllegianceRecallHometown_02AB::MAllegianceRecallHometown_02AB(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceRecallHometown_02AB::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceRecallHometown_02AB::Process()
{
	g_pAllegianceManager->RecallHometown(m_pPlayer);
}
