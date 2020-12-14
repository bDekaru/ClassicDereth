
#include <StdAfx.h>

#include "Network.h"
#include "Client.h"
#include "ClientEvents.h"
#include "World.h"
#include "crc.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "PacketController.h"
#include "Server.h"
#include "Config.h"
#include "SHA512.h"

// NOTE: A client can easily perform denial of service attacks by issuing a large number of connection requests if there ever comes a time this matters to fix

CNetwork::CNetwork(class CPhatServer *server, in_addr address, WORD port)
{
	m_server = server;
	m_port = port;
	m_addr = address;
	m_ServerID = 11; // arbitrary

	LoadBans();

	Init();

	m_running = true;
	m_incomingThread = std::thread([&]() { IncomingThreadProc(); });
	m_outgoingThread = std::thread([&]() { OutgoingThreadProc(); });
}

CNetwork::~CNetwork()
{
	m_running = false;

	if (m_outgoingThread.joinable())
		m_outgoingThread.join();

	if (m_incomingThread.joinable())
		m_incomingThread.join();

	Shutdown();

	_queuedIncoming.clear();
	_queuedOutgoing.clear();
}

void CNetwork::Init()
{
	srand((unsigned int)time(NULL));

	xsocket::socket_init();

	WINLOG(Temp, Normal, "Binding to addr %s\n", inet_ntoa(m_addr));
	if (m_read_sock.bind(m_addr, m_port))
	{
		WINLOG(Temp, Normal, "Failed bind on recv port %u!\n", m_port);
		SERVER_ERROR << "Failed bind on recv port:" << m_port;		
	}

	if (m_write_sock.bind(m_addr, m_port + 1))
	{
		WINLOG(Temp, Normal, "Failed bind on send port %u!\n",  m_port + 1);
		SERVER_ERROR << "Failed bind on send port:" <<  m_port + 1;		
	}

	// configure for non-blocking
	int buffsize = g_pConfig->NetBufferSize();

	m_read_sock.blocking(false);
	m_read_sock.buffer_size(&buffsize, &buffsize);
	m_write_sock.blocking(false);
	m_write_sock.buffer_size(&buffsize, &buffsize);
}

void CNetwork::Shutdown()
{
	SaveBans();

	m_read_sock.close();
	m_write_sock.close();

	xsocket::socket_shutdown();	
}

void CNetwork::IncomingThreadProc()
{
	xsocket::socket_wait wait;

	while (m_running)
	{
		wait.reset();
		wait.set(m_read_sock);
		wait.set(m_write_sock);

		int result = wait.wait();

		if (result > 0)
		{
			if (wait.isset(m_read_sock))
				QueueIncomingOnSocket(m_read_sock);
			if (wait.isset(m_write_sock))
				QueueIncomingOnSocket(m_write_sock);
		}
		std::this_thread::yield();
	}
}

void CNetwork::OutgoingThreadProc()
{
	while (m_running)
	{
		{
			std::unique_lock sigLock(m_sigOutgoingLock);
			m_sigOutgoing.wait_for(sigLock, std::chrono::seconds(1));
		}

		while (!_queuedOutgoing.empty())
		{
			std::scoped_lock lock(m_outgoingLock);

			auto entry = _queuedOutgoing.begin();
			if (entry != _queuedOutgoing.end())
			{
				CQueuedPacket& queued = *entry;
				//if (SendPacket(queued.useReadStream ? m_read_sock : m_write_sock, &queued.addr, queued.data.get(), queued.len))
				SendPacket(queued.useReadStream ? m_read_sock : m_read_sock, &queued.addr, queued.data.get(), queued.len);
				_queuedOutgoing.erase(entry);

				std::this_thread::yield();
			}
		}

		std::this_thread::yield();
	}
}

WORD CNetwork::GetServerID(void)
{
	return m_ServerID;
}

void CNetwork::SendConnectlessBlob(SOCKADDR_IN *peer, BlobPacket_s *blob, uint32_t dwFlags, uint32_t dwSequence, WORD wTime, bool useReadStream)
{
	BlobHeader_s *header = &blob->header;

	header->dwSequence = dwSequence;
	header->dwFlags = dwFlags;
	header->dwCRC = 0;
	header->wRecID = GetServerID();
	header->wTime = wTime;
	header->wTable = 0x01;

	GenericCRC(blob);

	QueuePacket(peer, blob, BLOBLEN(blob), useReadStream);
}

bool CNetwork::SendPacket(xsocket::socket_udp socket, sockaddr_in *peer, void *data, uint32_t len)
{
	if (!socket.valid())
	{
		NETWORK_LOG << "Invalid socket:" << inet_ntoa(peer->sin_addr) << ":" << peer->sin_port;
		return false;
	}

	//WINLOG(Temp, Normal, "Sending to %s:%d\n", inet_ntoa(peer->sin_addr), ntohs(peer->sin_port));

	int bytesSent = socket.sendto((uint8_t*)data, len, peer);

	bool success = (bytesSent > 0);

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
	else
	{
		NETWORK_LOG << "Failed send:" << inet_ntoa(peer->sin_addr) << ":" << peer->sin_port << " - " << bytesSent << "bytes sent" << " - " << xsocket::socket_error_text(xsocket::socket_error());
	}

	return success;
}

void CNetwork::QueuePacket(SOCKADDR_IN *peer, void *data, uint32_t len, bool useReadStream)
{
	CQueuedPacket qp;
	qp.addr = *peer;
	qp.data = std::make_unique<BYTE[]>(len);
	memcpy(qp.data.get(), data, len);
	qp.len = len;
	qp.useReadStream = useReadStream;

	{
		std::scoped_lock lock(m_outgoingLock);
		_queuedOutgoing.push_back(std::move(qp));
	}
	m_sigOutgoing.notify_all();
}

void CNetwork::QueueIncomingOnSocket(xsocket::socket_udp socket)
{
	if (!socket.valid())
	{
		return;
	}
	
	static BYTE	buffer[0x1E4];

	while (TRUE)
	{
		int clientaddrlen = sizeof(sockaddr_in);
		sockaddr_in clientaddr;

		// Doing it similar to AC..
		int bloblen = socket.recvfrom(buffer, sizeof(buffer), &clientaddr);

		if (bloblen <= 0)
		{
			// uint32_t dwCode = WSAGetLastError();

			// if (dwCode != 10035)
			// {
			// 	// LOG(Temp, Normal, "Winsock Error %lu\n", dwCode);
			// }

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

		if (wRecID == 0)
		{
			ProcessConnectionless(&clientaddr, blob);
			continue;
		}

		CQueuedPacket qp;
		memcpy(&qp.addr, &clientaddr, sizeof(sockaddr_in));
		qp.data = std::make_unique<BYTE[]>(bloblen);
		memcpy(qp.data.get(), buffer, bloblen);
		qp.len = bloblen;
		qp.recvTime = g_pGlobals->Time();

		{
			std::scoped_lock lock(m_incomingLock);
			_queuedIncoming.push_back(std::move(qp));
		}

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
	}

}

void CNetwork::ProcessQueuedIncoming()
{
	while (!_queuedIncoming.empty())
	{
		CQueuedPacket qp;
		{
			std::scoped_lock lock(m_incomingLock);
			qp = std::move(_queuedIncoming.front());
			_queuedIncoming.pop_front();
		}
			
		BlobPacket_s *blob = (BlobPacket_s *)qp.data.get();
		blob->header.dwCRC -= CalcTransportCRC((uint32_t *)blob);

		if (!blob->header.wRecID)
		{
			ProcessConnectionless(&qp.addr, blob);
		}
		else if (CClient *client = ValidateClient(blob->header.wRecID, &qp.addr))
		{
			client->IncomingBlob(blob, qp.recvTime);
		}
	}
}

void CNetwork::LogoutAll()
{
	for (auto entry : m_clients)
	{
		CClient *client = entry.second;
		if (client && client->IsAlive() && client->GetEvents())
			client->GetEvents()->ForceLogout();
	}
}

void CNetwork::CompleteLogoutAll()
{
	for (auto entry : m_clients)
	{
		CClient *client = entry.second;
		if (client && client->IsAlive() && client->GetEvents())
		{
			BinaryWriter EnterPortal;
			EnterPortal.Write<uint32_t>(0xF751);
			EnterPortal.Write<uint32_t>(0);
			client->SendNetMessage(EnterPortal.GetData(), EnterPortal.GetSize(), OBJECT_MSG);

			BinaryWriter popupString;
			popupString.Write<uint32_t>(4);
			popupString.WriteString("The server has shutdown.");
			client->SendNetMessage(&popupString, PRIVATE_MSG, FALSE, FALSE);
		}
	}
}

void CNetwork::Think()
{
	ProcessQueuedIncoming();

	auto itr = m_clients.begin();
	while (itr != m_clients.end())
	{
		CClient *client = itr->second;
		if (!client || !client->IsAlive())
		{
			// nuke it
			SERVER_INFO << "Client" << client->GetAccount() << "(" << inet_ntoa(client->GetHostAddress()->sin_addr) << ") disconnected";
			delete client;

			{
				std::scoped_lock lock(m_clientsLock);
				itr = m_clients.erase(itr);
			}

			continue;
		}

		client->Think();
		itr++;
	}

#if defined(__cpp_lib_execution) && defined(__cpp_lib_parallel_algorithm)
	std::for_each(std::execution::par, m_clients.begin(), m_clients.end(), [](client_list_value_t &client)
#else
	std::for_each(m_clients.begin(), m_clients.end(), [](client_list_value_t &client)
#endif
	{
		client.second->ThinkOutbound();
	});

	if (m_lastBanSave + 30.0 < g_pGlobals->Time())
	{
		SaveBans();
		m_lastBanSave = g_pGlobals->Time();
	}
}

CClient* CNetwork::GetClient(WORD slot)
{
	auto itr = m_clients.find(slot);

	if (itr != m_clients.end())
	{
		return itr->second;
	}
	return nullptr;
}

void CNetwork::KickClient(CClient *pClient)
{
	if (!pClient)
		return;

	SERVER_INFO << "Client" << pClient->GetSlot() << "(" << pClient->GetAccount() << ") is being kicked.";
	BinaryWriter KC;
	KC.Write<int32_t>(0xF7DC);
	KC.Write<int32_t>(0);

	pClient->SendNetMessage(KC.GetData(), KC.GetSize(), PRIVATE_MSG);
	pClient->ThinkOutbound();
	pClient->Kill(NULL, NULL);
}

void CNetwork::KickBannedClient(CClient *pClient, int32_t duration)
{
	if (!pClient)
		return;

	// newlines to push turbine support url out of the window.
	std::string msg;
	msg += "\n\n";
	msg += g_pConfig->GetBanString();
	msg += "\n\n\n\n";

	BinaryWriter KBC;
	KBC.Write<int32_t>(0xF7C1);
	KBC.Write<int32_t>(duration); // ban duration in seconds
	KBC.WriteString(msg);

	pClient->SendNetMessage(KBC.GetData(), KBC.GetSize(), PRIVATE_MSG);
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
	//if (CClient *pClient = GetClient(slot))
	//{		
	//	SERVER_INFO << "Client" << pClient->GetAccount() << "(" << inet_ntoa(pClient->GetHostAddress()->sin_addr) << ") disconnected";

	//	uint32_t arrayPos = pClient->GetArrayPos();

	//	delete pClient;

	//	m_NumClients--;
	//	m_ClientArray[arrayPos] = m_ClientArray[m_NumClients];
	//	m_ClientArray[m_NumClients] = NULL;
	//	m_ClientSlots[slot] = NULL;
	//	m_OpenSlots.push_front(slot);

	//	if (m_ClientArray[arrayPos])
	//	{
	//		m_ClientArray[arrayPos]->SetArrayPos(arrayPos);
	//	}

	//	// m_server->Stats().UpdateClientList(NULL, 0);
	//}
}

CClient* CNetwork::FindClientByAccount(const char* account)
{
	for (auto entry : m_clients)
	{
		if (entry.second && entry.second->CheckAccount(account))
			return entry.second;
	}
	return nullptr;
}

void CNetwork::SendConnectLoginFailure(sockaddr_in *addr, int error, const char *accountname, const char *password)
{
	//Bad login.
	CREATEBLOB(BadLogin, sizeof(uint32_t) * 2);
	((uint32_t *)BadLogin->data)[0] = (uint32_t)error;
	((uint32_t *)BadLogin->data)[1] = ST_SERVER_ERRORS;

	SendConnectlessBlob(addr, BadLogin, BT_NETERROR, 0, 0, true);
	//SendConnectlessBlob(addr, BadLogin, BT_ERROR, 0, 0, true);

	DELETEBLOB(BadLogin);

	if (accountname)
	{
		SERVER_INFO << "Invalid login from " << inet_ntoa(addr->sin_addr) << ", used account name '" << accountname << "'";
	}
}

void CNetwork::ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p)
{
	g_pDB2->QueueAsyncQuery(new AuthenticateUserQuery(*addr, p));
}

void CNetwork::ProcessConnectionless(sockaddr_in *peer, BlobPacket_s *blob)
{
	uint32_t dwFlags = blob->header.dwFlags;

	if (dwFlags == BT_LOGIN)
	{
		ConnectionRequest(peer, blob);

		return;
	}

	// LOG(Network, Verbose, "Unhandled connectionless packet received: 0x%08X Look into this\n", dwFlags);
}

void CNetwork::AcceptClientLogin(sockaddr_in *addr, AccountInformation_t account, uint32_t timestamp, uint32_t portal_stamp, uint32_t cell_stamp)
{
	// this isn't happening on the same threads as before
	// make sure everything is protected
	CClient *client = FindClientByAccount(account.username.c_str());

	if (client)
	{
		if (stricmp(client->GetAccount(), "admin") && addr->sin_port == 9000)
		{
			SendConnectLoginFailure(addr, STR_LOGIN_ACCOUNT_ONLINE, nullptr, nullptr);
			return;
		}
		else
		{
			// TODO: make sure this client is using the same connection, otherwise send an error
			return;
		}
	}

	if (isFull())
	{
		if (account.access == 5)
		{
			// setup to allow admins access
		}
		else
		{
			SendConnectLoginFailure(addr, STR_LOGIN_SERVER_FULL, nullptr, nullptr);
			return;
		}
	}

	uint16_t slot = GetClientId();

	SERVER_INFO << "Client" << account.username << "(" << inet_ntoa(addr->sin_addr) << ":" << ntohs(addr->sin_port) << ") connected on slot" << slot;

	client = new CClient(addr, slot, account);
	client->SetLoginData(timestamp, portal_stamp, cell_stamp);

	{
		std::scoped_lock lock(m_clientsLock);
		m_clients.insert({ slot, client });
	}


	// Add the client to the HUD
	// m_server->Stats().UpdateClientList(m_clients, m_slotrange);

	BinaryWriter AcceptConnect;

	// Some server variables
	AcceptConnect.Write<double>(g_pGlobals->Time());

	BYTE cookie[] = {
		0xbe, 0xc8, 0x8a, 0x58, 0x0b, 0x1e, 0x99, 0x43
	};
	AcceptConnect.Write(cookie, sizeof(cookie));

	AcceptConnect.Write<uint32_t>(slot);
	AcceptConnect.Write<uint32_t>(client->GetPacketController()->GetServerCryptoSeed());
	AcceptConnect.Write<uint32_t>(client->GetPacketController()->GetClientCryptoSeed());
	AcceptConnect.Write<uint32_t>(2);

	uint32_t dwLength = AcceptConnect.GetSize();

	if (dwLength <= 0x1D0)
	{
		CREATEBLOB(Woot, (WORD)dwLength);
		memcpy(Woot->data, AcceptConnect.GetData(), dwLength);

		SendConnectlessBlob(addr, Woot, BT_LOGINREPLY, 0x00000000, 0, true);

		DELETEBLOB(Woot);

		// Check the account in the database to prevent users from changing their ip
		if (IsBannedIP(addr->sin_addr) || account.banned == true)
		{
			int32_t duration = GetBanDuration(addr->sin_addr);

			if (duration >= 0)
				KickBannedClient(client, duration);
			else
			{
				RemoveBan(addr->sin_addr);
				g_pDBIO->UpdateBan(account.id, false);
			}

			return;
		}
	}
	else
	{
		SERVER_INFO << "AcceptConnect.GetSize() > 0x1D0";
	}
}

DEFINE_PACK(CBanDescription)
{
	pWriter->Write<uint32_t>(2); // version
	pWriter->WriteString(m_AdminName);
	pWriter->WriteString(m_Reason);
	pWriter->Write<uint32_t>(m_Timestamp);
	pWriter->Write<int32_t>(m_BanDuration);
	pWriter->Write<uint32_t>(m_AccountID);
}

DEFINE_UNPACK(CBanDescription)
{
	uint32_t version = pReader->Read<uint32_t>();
	m_AdminName = pReader->ReadString();
	m_Reason = pReader->ReadString();
	m_Timestamp = pReader->Read<uint32_t>();
	if (version > 1)
	{
		m_BanDuration = pReader->Read<int32_t>();
		m_AccountID = pReader->Read<uint32_t>();
	}
	return true;
}

DEFINE_PACK(CNetworkBanList)
{
	pWriter->Write<uint32_t>(1); // version
	m_BanTable.Pack(pWriter);
}

DEFINE_UNPACK(CNetworkBanList)
{
	m_BanTable.clear();

	uint32_t version = pReader->Read<uint32_t>();
	m_BanTable.UnPack(pReader);
	return true;
}

bool CNetwork::IsBannedIP(in_addr ipaddr)
{
	return m_Bans.m_BanTable.lookup(ipaddr.s_addr) ? true : false;
}

int32_t CNetwork::GetBanDuration(in_addr ipaddr)
{
	CBanDescription* ban = m_Bans.m_BanTable.lookup(ipaddr.s_addr);

	int32_t timestamp, duration = 0;

	if (ban)
	{
		timestamp = ban->m_Timestamp;
		duration = ban->m_BanDuration;
	}

	if (!duration)
		return 0;

	return (timestamp + duration) - time(0);
}

uint32_t CNetwork::GetBanID(in_addr ipaddr)
{
	uint32_t id = m_Bans.m_BanTable.lookup(ipaddr.s_addr)->m_AccountID;

	if (!id)
		return 0;

	return id;
}

void CNetwork::LoadBans()
{
	void *data = NULL;
	unsigned long length = 0;
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

void CNetwork::AddBan(in_addr ipaddr, const char *admin, const char *reason, uint32_t account_id)
{
	CBanDescription *pBan = &m_Bans.m_BanTable[ipaddr.s_addr];

	pBan->m_AdminName = admin;
	pBan->m_Reason = reason;
	pBan->m_Timestamp = time(0);
	pBan->m_BanDuration = 0;
	pBan->m_AccountID = account_id;
}

void CNetwork::AddBan(in_addr ipaddr, const char *admin, const char *reason, int32_t duration, uint32_t account_id)
{
	CBanDescription *pBan = &m_Bans.m_BanTable[ipaddr.s_addr];

	pBan->m_AdminName = admin;
	pBan->m_Reason = reason;
	pBan->m_Timestamp = time(0);
	pBan->m_BanDuration = duration;
	pBan->m_AccountID = account_id;
}

bool CNetwork::RemoveBan(in_addr ipaddr)
{
	if (m_Bans.m_BanTable.lookup(ipaddr.s_addr))
	{
		m_Bans.m_BanTable.erase(ipaddr.s_addr);

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
		banList += csprintf("\n%s - Admin: %s @ %s Reason: %s Duration: %d\n", inet_ntoa(*(in_addr *)&entry.first), pBan->m_AdminName.c_str(), timestampDateString(pBan->m_Timestamp), pBan->m_Reason.c_str(), GetBanDuration(*(in_addr *)&entry.first));
	}

	return banList;
}

uint32_t CNetwork::GetUniques()
{
	if (m_clients.size() == 0)
		return 0;

	std::list<std::string> addresses;
	std::string addr = "";

	for (auto entry : m_clients)
	{
		if (entry.second)
		{
			addr = inet_ntoa(entry.second->GetHostAddress()->sin_addr);
			if (addr != "")
				addresses.push_back(addr);
		}
	}

	addresses.unique();

	uint32_t count = addresses.size();

	return count;
}

AuthenticateUserQuery::AuthenticateUserQuery(sockaddr_in connection, BlobPacket_s *blob)
	: m_connection(connection)
{
	BinaryReader loginRequest(blob->data, blob->header.wSize);

	m_version = loginRequest.ReadString();
	loginRequest.ReadUInt32(); // 0x20 ?

	m_auth_type = loginRequest.ReadUInt32();
	loginRequest.ReadUInt32(); // 0x0 ?

	m_timestamp = loginRequest.ReadUInt32(); // client unix timestamp

	switch (m_auth_type)
	{
	case 1:
	{
		// -a username:password
		char *login_credentials;
		login_credentials = loginRequest.ReadString();

		char *szPassword = strstr(login_credentials, ":");
		if (!szPassword) return;

		*(szPassword) = '\0';
		szPassword++;

		m_username = login_credentials;
		m_password = szPassword;

		// there are two empty ints
		loginRequest.ReadUInt32();
		loginRequest.ReadUInt32();
		break;
	}

	case 2:
	{
		// -a username -v password
		char *login_credentials;
		login_credentials = loginRequest.ReadString();
		m_username = login_credentials;

		// nothing
		loginRequest.ReadUInt32();

		// length of encoded data following
		uint32_t dataLen = loginRequest.ReadUInt32();

		// data is here packed_short,char[packed_short]
		uint16_t len = loginRequest.ReadCompressedUInt16();
		login_credentials = (char*)loginRequest.ReadArray(len);
		m_password = login_credentials;
		break;
	}

	default:
		m_auth_type = 0;
		break;
	}
}

bool AuthenticateUserQuery::PerformQuery(MYSQL *c)
{
	if (!c)
		return false;

	if (m_username.compare("acservertracker") == 0)
	{
		g_pNetwork->SendConnectLoginFailure(&m_connection, STR_LOGIN_FAILED, nullptr, nullptr);
		return true;
	}

	if (m_version.compare("1802") != 0)
	{
		g_pNetwork->SendConnectLoginFailure(&m_connection, STR_LOGIN_CLIENT_VERSION, nullptr, nullptr);
		return true;
	}

	if (m_auth_type == 0) // case 3 was turbine ticket method, took that out
	{
		g_pNetwork->SendConnectLoginFailure(&m_connection, STR_LOGIN_INVALID_AUTH, nullptr, nullptr);
		return true;
	}

	AccountInformation_t account = { 0 };
	int err = 0;

	if (!g_pDBIO->VerifyAccount(c, m_username.c_str(), m_password.c_str(), &account, &err))
	{
		if (err == VERIFYACCOUNT_ERROR_DOESNT_EXIST && g_pConfig->AutoCreateAccounts())
		{
			char *ipaddr = inet_ntoa(m_connection.sin_addr);
			if (g_pDBIO->CreateAccount(c, m_username.c_str(), m_password.c_str(), &err, ipaddr))
			{
				g_pDBIO->VerifyAccount(c, m_username.c_str(), m_password.c_str(), &account, &err);
			}
		}

		if (err != DBIO_ERROR_NONE)
		{
			g_pNetwork->SendConnectLoginFailure(&m_connection, STR_LOGIN_FAILED, m_username.c_str(), nullptr);
			return true;
		}
	}

	g_pNetwork->AcceptClientLogin(&m_connection, account, m_timestamp, 0, 0);

	// if we get this far, go ahead and bail the attempt
	return true;
}
