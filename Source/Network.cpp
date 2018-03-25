
#include "StdAfx.h"

#include "Network.h"
#include "Client.h"
#include "ClientEvents.h"
#include "World.h"
#include "Database.h"
#include "Database2.h"
#include "DatabaseIO.h"
#include "CRC.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "PacketController.h"
#include "Server.h"
#include "Config.h"

// NOTE: A client can easily perform denial of service attacks by issuing a large number of connection requests if there ever comes a time this matters to fix

CNetwork::CNetwork(CPhatServer *server, SOCKET *sockets, int socketCount)
{
	m_server = server;
	m_sockets = sockets;
	m_socketCount = socketCount;
	m_ServerID = 11; // arbitrary

	m_ClientArray = new CClient *[m_MaxClients];
	memset(m_ClientArray, 0, sizeof(CClient *) * m_MaxClients);

	m_ClientSlots = new CClient *[m_MaxClients + 1];
	memset(m_ClientSlots, 0, sizeof(CClient *) * (m_MaxClients + 1));

	for (DWORD i = 1; i <= m_MaxClients; i++)
	{
		m_OpenSlots.push_back(i);
	}

	LoadBans();

	InitializeCriticalSection(&_incomingLock);
	InitializeCriticalSection(&_outgoingLock);

	m_hNetEvent = new HANDLE[socketCount];
	for (int i = 0; i < m_socketCount; i++)
	{
		m_hNetEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		::WSAEventSelect(m_sockets[i], m_hNetEvent[i], FD_READ|FD_WRITE);
	}

	m_hMakeTick = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hPumpThread = CreateThread(NULL, 0, InternalThreadProcStatic, this, 0, NULL);
}

CNetwork::~CNetwork()
{
	for (int i = 0; i < m_NumClients; i++)
	{
		SafeDelete(m_ClientArray[i]);
	}

	SetEvent(m_hQuitEvent);

	if (m_hPumpThread)
	{
		WaitForSingleObject(m_hPumpThread, 60000);
		CloseHandle(m_hPumpThread);
		m_hPumpThread = NULL;
	}

	if (m_hMakeTick)
	{
		CloseHandle(m_hMakeTick);
		m_hMakeTick = NULL;
	}

	for (int i = 0; i < m_socketCount; i++)
	{
		if (m_hNetEvent[i])
		{
			::WSAEventSelect(m_sockets[i], m_hNetEvent[i], 0);
			CloseHandle(m_hNetEvent[i]);
			m_hNetEvent[i] = NULL;
		}
	}
	SafeDeleteArray(m_hNetEvent);

	if (m_hQuitEvent)
	{
		CloseHandle(m_hQuitEvent);
		m_hQuitEvent = NULL;
	}

	DeleteCriticalSection(&_incomingLock);
	DeleteCriticalSection(&_outgoingLock);

	for (auto &qp : _queuedIncoming)
	{
		delete [] qp.data;
	}
	_queuedIncoming.clear();

	for (auto &qp : _queuedOutgoing)
	{
		delete [] qp.data;
	}
	_queuedOutgoing.clear();

	SafeDeleteArray(m_ClientSlots);
	SafeDeleteArray(m_ClientArray);
}

void CNetwork::Init()
{
}

void CNetwork::Shutdown()
{
}

DWORD WINAPI CNetwork::InternalThreadProcStatic(LPVOID lpThis)
{
	return ((CNetwork *)lpThis)->InternalThreadProc();
}

DWORD CNetwork::InternalThreadProc()
{
	WSADATA	wsaData;
	USHORT wVersionRequested = 0x0202;
	WSAStartup(wVersionRequested, &wsaData);

	srand((unsigned int)time(NULL));

	DWORD numEvents = (DWORD)(m_socketCount + 2);
	HANDLE *hEvents = new HANDLE[numEvents];

	hEvents[0] = m_hMakeTick;
	hEvents[1] = m_hQuitEvent;
	memcpy(&hEvents[2], m_hNetEvent, sizeof(HANDLE) * m_socketCount);

	bool bRun = true;

	while (bRun)
	{
		EnterCriticalSection(&_outgoingLock);
		bool bIsSendQueueEmpty = _queuedOutgoing.empty();
		LeaveCriticalSection(&_outgoingLock);
		
		switch (::WaitForMultipleObjects(numEvents, hEvents, FALSE, bIsSendQueueEmpty ? 500 : 0))
		{
		case (WAIT_OBJECT_0 + 0): // Make Think Event
			break;

		case (WAIT_OBJECT_0 + 1): // Quit Event
			bRun = false;
			break;

		default:
		case (WAIT_OBJECT_0 + 2): // Net Event or Timeout
			break;
		}

		for (int i = 0; i < m_socketCount; i++)
		{
			QueueIncomingOnSocket(m_sockets[i]);
		}
		SendQueuedOutgoing();
	}
	
	delete [] hEvents;

	Shutdown();

	WSACleanup();

	return 0;
}

WORD CNetwork::GetServerID(void)
{
	return m_ServerID;
}

void CNetwork::SendConnectlessBlob(SOCKADDR_IN *peer, BlobPacket_s *blob, DWORD dwFlags, DWORD dwSequence = 0, WORD wTime = 0)
{
	BlobHeader_s *header = &blob->header;

	header->dwSequence = dwSequence;
	header->dwFlags = dwFlags;
	header->dwCRC = 0;
	header->wRecID = GetServerID();
	header->wTime = wTime;
	header->wTable = 0x01;

	GenericCRC(blob);

	QueuePacket(peer, blob, BLOBLEN(blob));
}

bool CNetwork::SendPacket(SOCKADDR_IN *peer, void *data, DWORD len)
{
	SOCKET socket = m_sockets[0];

	if (socket == INVALID_SOCKET)
	{
		return false;
	}

	int bytesSent = sendto(socket, (char *)data, len, 0, (sockaddr *)peer, sizeof(SOCKADDR_IN));
	bool success = (SOCKET_ERROR != bytesSent);

	if (success)
	{
		/*
#ifdef _DEBUG
		if (false) // g_bDebugToggle)
		{
			LOG(Network, Normal, "Sent:\n");
			LOG_BYTES(Network, Normal, data, len);
		}
		else
		{
			LOG(Network, Verbose, "Sent to %s:\n", inet_ntoa(peer->sin_addr));
			LOG_BYTES(Network, Verbose, data, len);
		}
#endif

		g_pGlobals->PacketSent(len);
		*/
	}

	return success;
}

void CNetwork::QueuePacket(SOCKADDR_IN *peer, void *data, DWORD len)
{
	CQueuedPacket qp;
	qp.addr = *peer;
	qp.data = new BYTE[len];
	memcpy(qp.data, data, len);
	qp.len = len;

	EnterCriticalSection(&_outgoingLock);
	_queuedOutgoing.push_back(qp);
	LeaveCriticalSection(&_outgoingLock);

	SetEvent(m_hMakeTick);
}

void CNetwork::SendQueuedOutgoing()
{
	EnterCriticalSection(&_outgoingLock);

	auto entry = _queuedOutgoing.begin();

	while (entry != _queuedOutgoing.end())
	{
		// copy it locally
		CQueuedPacket queued = *entry;
		// remove
		_queuedOutgoing.erase(entry);

		// unlock while we send it
		LeaveCriticalSection(&_outgoingLock);

		if (!SendPacket(&queued.addr, queued.data, queued.len))
		{
			// failed to send
			EnterCriticalSection(&_outgoingLock);

			// queue it back
			_queuedOutgoing.push_front(queued);
			break;
		}

		// delete now that the packet is sent
		delete[] queued.data;

		EnterCriticalSection(&_outgoingLock);
		entry = _queuedOutgoing.begin();
	}

	LeaveCriticalSection(&_outgoingLock);
}

void CNetwork::QueueIncomingOnSocket(SOCKET socket)
{
	if (socket == INVALID_SOCKET)
	{
		return;
	}
	
	static BYTE	buffer[0x1E4];

	while (TRUE)
	{
		int clientaddrlen = sizeof(sockaddr_in);
		sockaddr_in clientaddr;

		// Doing it similar to AC..
		int bloblen = recvfrom(socket, (char *)buffer, 0x1E4, NULL, (sockaddr *)&clientaddr, &clientaddrlen);

		if (bloblen == SOCKET_ERROR)
		{
			DWORD dwCode = WSAGetLastError();

			if (dwCode != 10035)
			{
				// LOG(Temp, Normal, "Winsock Error %lu\n", dwCode);
			}

			break;
		}
		else if (!bloblen)
		{
			break;
		}
		else if (bloblen < sizeof(BlobHeader_s))
		{
			continue;
		}

		g_pGlobals->PacketRecv(bloblen);

		BlobPacket_s *blob = reinterpret_cast<BlobPacket_s*>(buffer);

		WORD wSize = blob->header.wSize;
		WORD wRecID = blob->header.wRecID;

		if ((bloblen - sizeof(BlobHeader_s)) != wSize)
			continue;

		CQueuedPacket qp;
		memcpy(&qp.addr, &clientaddr, sizeof(sockaddr_in));
		qp.data = new BYTE[bloblen];
		memcpy(qp.data, buffer, bloblen);
		qp.len = bloblen;
		qp.recvTime = g_pGlobals->UpdateTime();

		EnterCriticalSection(&_incomingLock);
		_queuedIncoming.push_back(qp);
		LeaveCriticalSection(&_incomingLock);

		// blob->header.dwCRC -= CalcTransportCRC((DWORD *)blob);
		
		/*
#ifdef _DEBUG
		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(addr));
		int namelen = sizeof(addr);
		getsockname(socket, (sockaddr *)&addr, &namelen);

		if (false) // g_bDebugToggle)
		{
			LOG(Network, Normal, "Received on port %d:\n", ntohs(addr.sin_port));
			LOG_BYTES(Network, Normal, &blob->header, blob->header.wSize + sizeof(blob->header));
		}
		else
		{
			LOG(Network, Verbose, "Received on port %d:\n", ntohs(addr.sin_port));
			LOG_BYTES(Network, Verbose, &blob->header, blob->header.wSize + sizeof(blob->header));
		}
#endif
*/

		/*
		if (!wRecID)
		{
			ProcessConnectionless(&clientaddr, blob);
		}
		else
		{
			CClient *client = ValidateClient(wRecID, &clientaddr);
			if (client)
				client->IncomingBlob(blob);
		}
		*/
	}

}

void CNetwork::ProcessQueuedIncoming()
{
	CQueuedPacket qp;
	bool bHasQueued;

	do 
	{
		bHasQueued = false;

		EnterCriticalSection(&_incomingLock);
		if (!_queuedIncoming.empty())
		{
			qp = _queuedIncoming.front();
			_queuedIncoming.pop_front();
			bHasQueued = true;
		}
		LeaveCriticalSection(&_incomingLock);

		if (bHasQueued)
		{
			BlobPacket_s *blob = (BlobPacket_s *) qp.data;
			blob->header.dwCRC -= CalcTransportCRC((DWORD *)blob);
		
			if (!blob->header.wRecID)
			{
				ProcessConnectionless(&qp.addr, blob);
			}
			else if (CClient *client = ValidateClient(blob->header.wRecID, &qp.addr))
			{
				client->IncomingBlob(blob, qp.recvTime);
			}

			delete [] qp.data;
		}

	} while (bHasQueued);
}

void CNetwork::LogoutAll()
{
	for (DWORD i = 0; i < m_NumClients; i++)
	{
		if (CClient *client = m_ClientArray[i])
		{
			if (client->IsAlive() && client->GetEvents())
			{
				client->GetEvents()->BeginLogout();
			}
		}
	}
}

void CNetwork::CompleteLogoutAll()
{
	for (DWORD i = 0; i < m_NumClients; i++)
	{
		if (CClient *client = m_ClientArray[i])
		{
			if (client->IsAlive() && client->GetEvents())
			{
				//todo: send the user to the appropriate server connection lost screen.
				BinaryWriter EnterPortal;
				EnterPortal.Write<DWORD>(0xF751);
				EnterPortal.Write<DWORD>(0);
				client->SendNetMessage(EnterPortal.GetData(), EnterPortal.GetSize(), OBJECT_MSG);

				BinaryWriter popupString;
				popupString.Write<DWORD>(4);
				popupString.WriteString("The server has shutdown.");
				client->SendNetMessage(&popupString, PRIVATE_MSG, FALSE, FALSE);
			}
		}
	}
}

void CNetwork::Think()
{
	ProcessQueuedIncoming();

	for (DWORD i = 0; i < m_NumClients; i++)
	{
		if (CClient *client = m_ClientArray[i])
		{
			client->Think();

			if (!client->IsAlive())
			{
				KillClient(client->GetSlot());
			}
		}
	}
}

CClient* CNetwork::GetClient(WORD slot)
{
	if (!slot || slot > m_MaxClients)
		return NULL;

	return m_ClientSlots[slot];
}

void CNetwork::KickClient(CClient *pClient)
{
	if (!pClient)
		return;

	LOG(Temp, Normal, "Client #%u (%s) is being kicked.\n", pClient->GetSlot(), pClient->GetAccount());
	BinaryWriter KC;
	KC.Write<long>(0xF7DC);
	KC.Write<long>(0);

	pClient->SendNetMessage(KC.GetData(), KC.GetSize(), PRIVATE_MSG);
	pClient->ThinkOutbound();
	pClient->Kill(NULL, NULL);
}

void CNetwork::KickClient(WORD slot)
{
	KickClient(GetClient(slot));
}

CClient* CNetwork::ValidateClient(WORD index, sockaddr_in *peer)
{
	CClient* pClient = GetClient(index);

	if (!pClient)
		return NULL;

	if (!pClient->CheckAddress(peer))
		return NULL;

	return pClient;
}

void CNetwork::KillClient(WORD slot)
{
	if (CClient *pClient = GetClient(slot))
	{		
		LOG(Temp, Normal, "Client(%s, %s) disconnected. (%s)\n", pClient->GetAccount(), inet_ntoa(pClient->GetHostAddress()->sin_addr), timestamp());

		DWORD arrayPos = pClient->GetArrayPos();

		delete pClient;

		m_NumClients--;
		m_ClientArray[arrayPos] = m_ClientArray[m_NumClients];
		m_ClientArray[m_NumClients] = NULL;
		m_ClientSlots[slot] = NULL;
		m_OpenSlots.push_front(slot);

		if (m_ClientArray[arrayPos])
		{
			m_ClientArray[arrayPos]->SetArrayPos(arrayPos);
		}

		// m_server->Stats().UpdateClientList(NULL, 0);
	}
}

CClient* CNetwork::FindClientByAccount(const char* account)
{
	for (DWORD i = 0; i < m_NumClients; i++)
	{
		if (CClient *client = m_ClientArray[i])
		{
			if (client->CheckAccount(account))
				return client;
		}
	}

	return NULL;
}

void CNetwork::SendConnectLoginFailure(sockaddr_in *addr, int error, const char *accountname, const char *password)
{
	//Bad login.
	CREATEBLOB(BadLogin, sizeof(DWORD));
	*((DWORD *)BadLogin->data) = 0x00000000;

	SendConnectlessBlob(addr, BadLogin, BT_ERROR, NULL);

	DELETEBLOB(BadLogin);

	if (strcmp(accountname, "acservertracker"))
	{
		LOG(Temp, Normal, "Invalid login from %s, used account name '%s'\n", inet_ntoa(addr->sin_addr), accountname);
	}
}

void CNetwork::ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p)
{
	BinaryReader loginRequest(p->data, p->header.wSize);

	char *version_string = loginRequest.ReadString();
	loginRequest.ReadDWORD(); // 0x20 ?

	DWORD auth_method = loginRequest.ReadDWORD();
	loginRequest.ReadDWORD(); // 0x0 ?
	DWORD client_unix_timestamp = loginRequest.ReadDWORD(); // client unix timestamp

	char *login_credentials;

	if (auth_method != 1) // case 3 was turbine ticket method, took that out
		return; 

	login_credentials = loginRequest.ReadString();

	DWORD portal_stamp = loginRequest.ReadDWORD();
	DWORD cell_stamp = loginRequest.ReadDWORD();

	if (loginRequest.GetLastError()) 
		return;

	char *szPassword = strstr(login_credentials, ":");
	if (!szPassword) return;

	*(szPassword) = '\0';
	szPassword++;

	if (!strcmp(login_credentials, "acservertracker"))
	{
		SendConnectLoginFailure(addr, 0, login_credentials, szPassword);
		return;
	}

	// Try to login to the login_credentials
	AccountInformation_t accountInfo;
	int error = 0;
	if (!g_pDBIO->VerifyAccount(login_credentials, szPassword, &accountInfo, &error))
	{
		if (error == VERIFYACCOUNT_ERROR_DOESNT_EXIST && g_pConfig->AutoCreateAccounts())
		{
			std::string ipaddress = inet_ntoa(addr->sin_addr);

			// try to create the login_credentials
			if (g_pDBIO->CreateAccount(login_credentials, szPassword, &error, ipaddress.c_str()))
			{
				// now try to verify the newly created login_credentials
				if (!g_pDBIO->VerifyAccount(login_credentials, szPassword, &accountInfo, &error))
				{
					// fail
					SendConnectLoginFailure(addr, error, login_credentials, szPassword);
					return;
				}
			}
			else
			{
				// fail
				SendConnectLoginFailure(addr, error, login_credentials, szPassword);
				return;
			}
		}
		else
		{
			// fail
			SendConnectLoginFailure(addr, error, login_credentials, szPassword);
			return;
		}
	}
	
	CClient *client = FindClientByAccount(accountInfo.username.c_str());

	if (client)
	{
		if (_stricmp(client->GetAccount(), "admin"))
		{
			KickClient(client);
			// TODO don't allow this player to login for a few seconds while the world handles the other player
		}
		else
		{
			return;
		}
	}

	WORD slot = AllocOpenClientSlot();

	if (!slot)
	{
		// Server unavailable.
		CREATEBLOB(ServerFull, sizeof(DWORD));
		*((DWORD *)ServerFull->data) = 0x00000005;

		SendConnectlessBlob(addr, ServerFull, BT_ERROR, NULL);

		DELETEBLOB(ServerFull);
		return;
	}

	LOG(Temp, Normal, "Client(%s, %s) connected on slot #%u (%s)\n", login_credentials, inet_ntoa(addr->sin_addr), slot, timestamp());

	client = m_ClientSlots[slot] = new CClient(addr, slot, accountInfo);
	client->SetLoginData(client_unix_timestamp, portal_stamp, cell_stamp);

	m_ClientArray[m_NumClients] = client;
	client->SetArrayPos(m_NumClients);
	m_NumClients++;

	// Add the client to the HUD
	// m_server->Stats().UpdateClientList(m_clients, m_slotrange);

	BinaryWriter AcceptConnect;

	// Some server variables
	AcceptConnect.Write<double>(g_pGlobals->Time());

	BYTE cookie[] = {
		0xbe, 0xc8, 0x8a, 0x58, 0x0b, 0x1e, 0x99, 0x43
	};
	AcceptConnect.Write(cookie, sizeof(cookie));

	AcceptConnect.Write<DWORD>(slot);
	AcceptConnect.Write<DWORD>(client->GetPacketController()->GetServerCryptoSeed());
	AcceptConnect.Write<DWORD>(client->GetPacketController()->GetClientCryptoSeed());
	AcceptConnect.Write<DWORD>(2);

	DWORD dwLength = AcceptConnect.GetSize();

	if (dwLength <= 0x1D0)
	{
		CREATEBLOB(Woot, (WORD)dwLength);
		memcpy(Woot->data, AcceptConnect.GetData(), dwLength);

		SendConnectlessBlob(addr, Woot, BT_LOGINREPLY, 0x00000000);

		DELETEBLOB(Woot);
	}
	else
	{
		LOG(Temp, Normal, "AcceptConnect.GetSize() > 0x1D0");
	}
}

WORD CNetwork::AllocOpenClientSlot()
{
	// Allocate an available slot for a connecting client

	if (m_OpenSlots.empty())
		return 0;

	DWORD slot = *m_OpenSlots.begin();
	m_OpenSlots.pop_front();

	return slot;
}

void CNetwork::ProcessConnectionless(sockaddr_in *peer, BlobPacket_s *blob)
{
	DWORD dwFlags = blob->header.dwFlags;

	if (dwFlags == BT_LOGIN)
	{
		if (!IsBannedIP(peer->sin_addr))
		{
			ConnectionRequest(peer, blob);
		}

		return;
	}

	// LOG(Network, Verbose, "Unhandled connectionless packet received: 0x%08X Look into this\n", dwFlags);
}

DEFINE_PACK(CBanDescription)
{
	pWriter->Write<DWORD>(1); // version
	pWriter->WriteString(m_AdminName);
	pWriter->WriteString(m_Reason);
	pWriter->Write<DWORD>(m_Timestamp);
}

DEFINE_UNPACK(CBanDescription)
{
	DWORD version = pReader->Read<DWORD>();
	m_AdminName = pReader->ReadString();
	m_Reason = pReader->ReadString();
	m_Timestamp = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(CNetworkBanList)
{
	pWriter->Write<DWORD>(1); // version
	m_BanTable.Pack(pWriter);
}

DEFINE_UNPACK(CNetworkBanList)
{
	m_BanTable.clear();

	DWORD version = pReader->Read<DWORD>();
	m_BanTable.UnPack(pReader);
	return true;
}

bool CNetwork::IsBannedIP(in_addr ipaddr)
{
	return m_Bans.m_BanTable.lookup(ipaddr.s_addr) ? true : false;
}

void CNetwork::LoadBans()
{
	void *data = NULL;
	DWORD length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_BAN_DATA, &data, &length))
	{
		BinaryReader reader(data, length);
		m_Bans.UnPack(&reader);
	}
}

void CNetwork::SaveBans()
{
	BinaryWriter banData;
	m_Bans.Pack(&banData);
	g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_BAN_DATA, banData.GetData(), banData.GetSize());
}

void CNetwork::AddBan(in_addr ipaddr, const char *admin, const char *reason)
{
	CBanDescription *pBan = &m_Bans.m_BanTable[ipaddr.s_addr];

	pBan->m_AdminName = admin;
	pBan->m_Reason = reason;
	pBan->m_Timestamp = time(0);

	SaveBans();
}

bool CNetwork::RemoveBan(in_addr ipaddr)
{
	if (m_Bans.m_BanTable.lookup(ipaddr.s_addr))
	{
		m_Bans.m_BanTable.erase(ipaddr.s_addr);

		SaveBans();
		return true;
	}

	return false;
}

std::string CNetwork::GetBanList()
{
	std::string banList = csprintf("Ban List (%d entries):", m_Bans.m_BanTable.size());
	for (auto &entry : m_Bans.m_BanTable)
	{
		CBanDescription *pBan = &entry.second;
		banList += csprintf("\n%s - Admin: %s @ %s Reason: %s\n", inet_ntoa(*(in_addr *)&entry.first), pBan->m_AdminName.c_str(), timestampDateString(pBan->m_Timestamp), pBan->m_Reason.c_str());
	}

	return banList;
}








