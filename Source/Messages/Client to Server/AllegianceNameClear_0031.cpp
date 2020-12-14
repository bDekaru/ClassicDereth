#include <StdAfx.h>
#include "AllegianceNameClear_0031.h"
#include "AllegianceManager.h"
MAllegianceNameClear_0031::MAllegianceNameClear_0031(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceNameClear_0031::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceNameClear_0031::Process()
{
	g_pAllegianceManager->ClearAllegianceName(m_pPlayer);
}
