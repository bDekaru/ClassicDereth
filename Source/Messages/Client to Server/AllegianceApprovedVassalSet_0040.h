#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_SetAllegianceApprovedVassal | 0040
// Sets a character as being approved to join the allegiance
class MAllegianceApprovedVassalSet_0040 : public IClientMessage
{
public:
	MAllegianceApprovedVassalSet_0040(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szCharName;
};