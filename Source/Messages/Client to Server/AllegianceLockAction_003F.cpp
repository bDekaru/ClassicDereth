#include <StdAfx.h>
#include "AllegianceLockAction_003F.h"
#include "AllegianceManager.h"
#include "World.h"

MAllegianceLockAction_003F::MAllegianceLockAction_003F(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceLockAction_003F::Parse(BinaryReader * reader)
{
	m_dwLockAction = reader->ReadUInt32();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance lock action message (0x003F) from the client.";
		return;
	}

	Process();
}

void MAllegianceLockAction_003F::Process()
{
	g_pAllegianceManager->AllegianceLockAction(m_pPlayer, m_dwLockAction);
}
