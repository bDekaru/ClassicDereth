#include <StdAfx.h>
#include "AllegianceOfficerTitlesList_003D.h"
#include "AllegianceManager.h"

MAllegianceOfficerTitlesList_003D::MAllegianceOfficerTitlesList_003D(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceOfficerTitlesList_003D::Parse(BinaryReader * reader)
{
	Process();
}

void MAllegianceOfficerTitlesList_003D::Process()
{
	g_pAllegianceManager->ListOfficerTitles(m_pPlayer);
}
