#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_SetAllegianceOfficer | 003B
// Sets a player as an allegiance officer at the given permission level
class MAllegianceOfficerSet_003B : public IClientMessage
{
public:
	MAllegianceOfficerSet_003B(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szOfficerName;
	uint32_t m_dwOfficerLevel;
};