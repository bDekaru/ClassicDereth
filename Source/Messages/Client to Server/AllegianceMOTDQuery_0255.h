#pragma once
#include "Messages/IClientMessage.h"

// Allegiance_QueryMotd | 0255
// Request to see the allegiance MOTD ( /allegiance motd )
class MAllegianceMOTDQuery_0255 : public IClientMessage
{
public:
	MAllegianceMOTDQuery_0255(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};