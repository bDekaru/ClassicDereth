#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_DoAllegianceLockAction | 003F
// Performs an AllegianceLockAction (Locked on/off, toggle, check, display/clear bypass)
class MAllegianceLockAction_003F : public IClientMessage
{
public:
	MAllegianceLockAction_003F(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	uint32_t m_dwLockAction;
};