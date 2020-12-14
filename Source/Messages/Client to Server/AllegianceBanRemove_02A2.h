#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_RemoveAllegianceBan | 02A2
// Removes a player from the allegiance ban list
class MAllegianceBanRemove_02A2 : public IClientMessage
{
public:
	MAllegianceBanRemove_02A2(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
};