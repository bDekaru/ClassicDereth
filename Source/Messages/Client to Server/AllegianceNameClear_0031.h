#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_ClearAllegianceName | 0031
// Clears the name of the allegiance
class MAllegianceNameClear_0031 : public IClientMessage
{
public:
	MAllegianceNameClear_0031(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
};