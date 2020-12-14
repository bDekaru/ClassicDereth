#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ListAllegianceOfficers | 02A6
// Shows a list of characters that are officers of the allegiance
class MAllegianceOfficersList_02A6 : public IClientMessage
{
public:
	MAllegianceOfficersList_02A6(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};