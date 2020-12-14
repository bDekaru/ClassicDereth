#include <StdAfx.h>
#include "AllegianceUpdate_001F.h"
#include "Client.h"
#include "ClientEvents.h"

MAllegianceUpdate_001F::MAllegianceUpdate_001F(CClient * client)
{
	m_pClient = client;
}

void MAllegianceUpdate_001F::Parse(BinaryReader * reader)
{
	m_bOn = reader->Read<bool>();

	if (reader->GetLastError())
	{
		SERVER_ERROR << "Error parsing an allegiance update request message (0x001F) from the client.";
		return;
	}

	Process();
}

void MAllegianceUpdate_001F::Process()
{
	// need to re-implement this
	if (CClientEvents* ce = m_pClient->GetEvents())
	{
		ce->SetRequestAllegianceUpdate(m_bOn);
	}

	// TODO: replace with server to client message
}
