#include <StdAfx.h>
#include "AllegianceChatGag_0041.h"

MAllegianceChatGag_0041::MAllegianceChatGag_0041(CPlayerWeenie * player)
{
	m_pPlayer = player;
}

void MAllegianceChatGag_0041::Parse(BinaryReader * reader)
{
	m_szCharName = reader->ReadString();
	m_bOn = reader->Read<bool>();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance chat gag message (0x0041) from the client.";
		return;
	}

	Process();
}

void MAllegianceChatGag_0041::Process()
{
	// TODO Allegiance Manager needs chat ban/gag functionality finished
}
