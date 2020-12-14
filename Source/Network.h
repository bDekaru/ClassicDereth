#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Database.h"
#include "Database2.h"
#include "DatabaseIO.h"

#define MAX_CONNECTED_CLIENTS 50000

class AuthenticateUserQuery;

class CBanDescription : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string m_AdminName;
	std::string m_Reason;
	uint32_t m_Timestamp;
	int32_t m_BanDuration;
	uint32_t m_AccountID;
};

class CNetworkBanList : public PackObj
{
public:
	DECLARE_PACKABLE()

	PackableHashTable<uint32_t, CBanDescription> m_BanTable;
};

class CQueuedPacket
{
public:
	CQueuedPacket() = default;
	CQueuedPacket(CQueuedPacket&&) = default;
	//CQueuedPacket(CQueuedPacket&) = default;
	CQueuedPacket& operator=(CQueuedPacket&&) = default;

	SOCKADDR_IN addr;
	//BYTE *data = NULL;
	std::unique_ptr<BYTE[]> data;
	uint32_t len = 0;
	double recvTime;
	bool useReadStream;
};

class CNetwork
{
public:
	CNetwork(class CPhatServer *server, in_addr address, WORD port);
	~CNetwork();

	void Think();
	CClient *GetClient(WORD index);
	WORD GetServerID();

	void LogoutAll();
	void CompleteLogoutAll();

	void KickClient(class CClient* pClient);
	void KickBannedClient(class CClient* pClient, int32_t duration);
	void KickClient(WORD slot);
	void KillClient(WORD slot);
	void QueuePacket(SOCKADDR_IN *peer, void *data, uint32_t len) { QueuePacket(peer, data, len, false); };
	void QueuePacket(SOCKADDR_IN *peer, void *data, uint32_t len, bool useReadStream);
	void SendConnectlessBlob(SOCKADDR_IN *peer, BlobPacket_s *blob, uint32_t dwFlags, uint32_t dwSequence, WORD wTime)
	{
		SendConnectlessBlob(peer, blob, dwFlags, dwSequence, wTime, false);
	};
	void SendConnectlessBlob(SOCKADDR_IN *peer, BlobPacket_s *blob, uint32_t dwFlags, uint32_t dwSequence, WORD wTime, bool useReadStream);

	void AddBan(in_addr ipaddr, const char *admin, const char *reason, uint32_t account_id);
	void AddBan(in_addr ipaddr, const char *admin, const char *reason, int32_t duration, uint32_t account_id);
	bool RemoveBan(in_addr ipaddr);
	std::string GetBanList();
	uint32_t GetBanID(in_addr ipaddr);

	uint32_t GetUniques();

private:

	bool IsBannedIP(in_addr ipaddr);
	void LoadBans();
	void SaveBans();
	int32_t GetBanDuration(in_addr ipaddr);

	CClient *ValidateClient(WORD, sockaddr_in *);
	CClient *FindClientByAccount(const char *);

	void SendConnectLoginFailure(sockaddr_in *addr, int error, const char *accountname, const char *password);
	void ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p);
	void ProcessConnectionless(sockaddr_in *, BlobPacket_s *);
	void AcceptClientLogin(sockaddr_in *addr, AccountInformation_t account, uint32_t timestamp, uint32_t portal_stamp, uint32_t cell_stamp);

	class CPhatServer *m_server;
	int m_socketCount;

	WORD m_ServerID;

	/*
	WORD m_freeslot;
	WORD m_slotrange;
	CClient *m_clients[400];
	*/

	using client_list_t = std::unordered_map<uint16_t, CClient*>;
	using client_list_iterator_t = client_list_t::iterator;
	using client_list_value_t = client_list_t::value_type;

	std::mutex m_clientsLock;
	client_list_t m_clients;
	uint16_t m_nextClientId = 0;

	bool isFull()
	{
		return !(m_clients.size() < MAX_CONNECTED_CLIENTS);
	}

	uint16_t GetClientId()
	{
		uint16_t result = ++m_nextClientId;
		while (m_clients.find(result) != m_clients.end())
			result = ++m_nextClientId;
		return result;
	}

	uint32_t m_MaxClients = MAX_CONNECTED_CLIENTS;

	CNetworkBanList m_Bans;
	double m_lastBanSave = 0.0;

	std::list<CQueuedPacket *> m_PacketQueue;

	void Init();
	void Shutdown();

	void IncomingThreadProc();
	void OutgoingThreadProc();

	void QueueIncomingOnSocket(xsocket::socket_udp socket);
	void ProcessQueuedIncoming();
	//bool SendPacket(SOCKET socket, SOCKADDR_IN *peer, void *data, uint32_t len);
	bool SendPacket(xsocket::socket_udp socket, sockaddr_in *peer, void *data, uint32_t len);

	bool m_running;

	WORD m_port;
	in_addr m_addr;

	xsocket::socket_udp m_read_sock;
	xsocket::socket_udp m_write_sock;

	std::thread m_incomingThread;
	std::thread m_outgoingThread;

	std::mutex m_incomingLock;
	std::mutex m_outgoingLock;

	std::mutex m_sigIncomingLock;
	std::condition_variable m_sigIncoming;

	std::mutex m_sigOutgoingLock;
	std::condition_variable m_sigOutgoing;

	std::list<CQueuedPacket> _queuedIncoming;
	std::list<CQueuedPacket> _queuedOutgoing;

	friend class AuthenticateUserQuery;
};

class AuthenticateUserQuery : public CMYSQLQuery
{
public:
	AuthenticateUserQuery(sockaddr_in connection, BlobPacket_s *blob);

	virtual ~AuthenticateUserQuery() override { };

	virtual bool PerformQuery(MYSQL *c) override;

protected:
	std::string m_username;
	std::string m_password;
	sockaddr_in m_connection;

	std::string m_version;

	uint32_t m_auth_type;
	uint32_t m_timestamp;
};
