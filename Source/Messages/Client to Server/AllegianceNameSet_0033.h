#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_SetAllegianceName | 0033
// Sets the name of the allegiance
class MAllegianceNameSet_0033 : public IClientMessage
{
public:
	MAllegianceNameSet_0033(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szNewName;
};