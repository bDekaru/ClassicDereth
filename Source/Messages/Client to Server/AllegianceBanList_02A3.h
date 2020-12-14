#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ListAllegianceBans | 02A3
// Shows a list of players banned from the allegiance
class MAllegianceBanList_02A3 : public IClientMessage
{
public:
	MAllegianceBanList_02A3(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};