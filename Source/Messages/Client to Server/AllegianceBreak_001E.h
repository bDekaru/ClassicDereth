#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_BreakAllegiance | 001E
// Player has broken their allegiance
class MAllegianceBreak_001E : public IClientMessage
{
public:
	MAllegianceBreak_001E(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	uint32_t m_dwPatronID;
};