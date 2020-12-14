#pragma once
#include "BinaryReader.h"
#include "Messages/IClientMessage.h"
#include "Player.h"

// Allegiance_AllegianceInfoRequest | 027B
// Request for the allegiance info of a given player
class MAllegianceInfoRequest_027B : public IClientMessage
{
public:
	MAllegianceInfoRequest_027B(CPlayerWeenie * player);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CPlayerWeenie* m_pPlayer;
	std::string m_szTargetPlayer;
};