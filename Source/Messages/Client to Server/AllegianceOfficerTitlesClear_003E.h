#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ClearAllegianceOfficerTitles | 003E
// Clears the allegiance custom officer titles
class MAllegianceOfficerTitlesClear_003E : public IClientMessage
{
public:
	MAllegianceOfficerTitlesClear_003E(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};