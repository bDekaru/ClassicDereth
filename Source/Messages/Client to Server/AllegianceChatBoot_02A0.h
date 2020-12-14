#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_AllegianceChatBoot | 02A0
// Boots a player from the allegiance chat channel
class MAllegianceChatBoot_02A0 : public IClientMessage
{
public:
	MAllegianceChatBoot_02A0(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
	std::string m_szReason;
};