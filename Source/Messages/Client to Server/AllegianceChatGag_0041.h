#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_AllegianceChatGag | 0041
// Gags a player in the allegiance chat channel
class MAllegianceChatGag_0041 : public IClientMessage
{
public:
	MAllegianceChatGag_0041(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
	bool m_bOn;
};