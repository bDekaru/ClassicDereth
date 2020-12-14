#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_QueryAllegianceName | 0030
// Request for the current name of the allegiance
class MAllegianceNameQuery_0030 : public IClientMessage
{
public:
	MAllegianceNameQuery_0030(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};