#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ClearAllegianceOfficers | 02A7
// Clears the allegiance officers list
class MAllegianceOfficersClear_02A7 : public IClientMessage
{
public:
	MAllegianceOfficersClear_02A7(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};