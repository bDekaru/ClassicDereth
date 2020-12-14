#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_ClearMotd | 0256
// Clears the allegiance MOTD (/allegiance motd clear)
class MAllegianceMOTDClear_0256 : public IClientMessage
{
public:
	MAllegianceMOTDClear_0256(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};