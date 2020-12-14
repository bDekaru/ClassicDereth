#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ListAllegianceOfficerTitles | 003D
// Lists the current officer titles for the allegiance
class MAllegianceOfficerTitlesList_003D : public IClientMessage
{
public:
	MAllegianceOfficerTitlesList_003D(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};