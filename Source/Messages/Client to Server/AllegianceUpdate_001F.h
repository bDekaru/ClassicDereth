#pragma once

#include "Messages/IClientMessage.h"
#include "Client.h"

// Allegiance_UpdateRequest | 001F
// Request for updated allegiance information
class MAllegianceUpdate_001F : public IClientMessage
{
public:
	MAllegianceUpdate_001F(CClient * client);
	void Parse(BinaryReader *reader);

private:
	void Process();
	CClient* m_pClient;
	bool m_bOn;
};