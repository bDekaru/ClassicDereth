#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_RemoveAllegianceOfficer | 02A5
// Removes a player from the allegiance officers list
class MAllegianceOfficerRemove_02A5 : public IClientMessage
{
public:
	MAllegianceOfficerRemove_02A5(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
};