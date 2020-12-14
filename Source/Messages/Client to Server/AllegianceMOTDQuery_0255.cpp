#include <StdAfx.h>
#include "AllegianceMOTDQuery_0255.h"
#include "AllegianceManager.h"

MAllegianceMOTDQuery_0255::MAllegianceMOTDQuery_0255(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceMOTDQuery_0255::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceMOTDQuery_0255::Process()
{
	g_pAllegianceManager->QueryMOTD(m_pPlayer);
}
