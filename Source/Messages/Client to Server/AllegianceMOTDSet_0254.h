#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"

// Allegiance_SetMotd | 0254
// Sets the allegiance MOTD ( /allegiance motd set )
class MAllegianceMOTDSet_0254 : public IClientMessage
{
public:
	MAllegianceMOTDSet_0254(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szMessage;
};