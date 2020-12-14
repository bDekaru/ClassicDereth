#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_RecallAllegianceHometown | 02AB
// Recalls the player to their allegiance hometown
class MAllegianceRecallHometown_02AB : public IClientMessage
{
public:
	MAllegianceRecallHometown_02AB(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};