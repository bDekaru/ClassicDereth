#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_AddAllegianceBan | 02A1
// Bans a player from the allegiance
class MAllegianceBanAdd_02A1 : public IClientMessage
{
public:
	MAllegianceBanAdd_02A1(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
};