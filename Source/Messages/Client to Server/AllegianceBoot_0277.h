#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_BreakAllegianceBoot | 0277
// Boots a player out of the allegiance and, optionally, the rest of the characters from that account
class MAllegianceBoot_0277 : public IClientMessage
{
public:
	MAllegianceBoot_0277(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szBooteeName;
	bool m_bAccountBoot;
};