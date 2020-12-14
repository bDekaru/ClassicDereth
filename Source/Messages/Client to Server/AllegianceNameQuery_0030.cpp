#include <StdAfx.h>
#include "AllegianceNameQuery_0030.h"
#include "AllegianceManager.h"

MAllegianceNameQuery_0030::MAllegianceNameQuery_0030(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceNameQuery_0030::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceNameQuery_0030::Process()
{
	g_pAllegianceManager->QueryAllegianceName(m_pPlayer);
}
