

#pragma once

#define MAX_CONNECTED_CLIENTS 600

class CBanDescription : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string m_AdminName;
	std::string m_Reason;
	DWORD m_Timestamp;
};

class CNetworkBanList : public PackObj
{
public:
	DECLARE_PACKABLE()

	PackableHashTable<DWORD, CBanDescription> m_BanTable;
};

class CQueuedPacket
{
public:
	SOCKADDR_IN addr;
	BYTE *data = NULL;
	DWORD len = 0;
	double recvTime;
};

class CNetwork
{
public:
	CNetwork(class CPhatServer *server, SOCKET *sockets, int socketCount);
	~CNetwork();

	void Think();
	CClient *GetClient(WORD index);
	WORD GetServerID();

	void LogoutAll();
	void CompleteLogoutAll();

	void KickClient(class CClient* pClient);
	void KickClient(WORD slot);
	void KillClient(WORD slot);
	void QueuePacket(SOCKADDR_IN *, void *data, DWORD len);
	void SendConnectlessBlob(SOCKADDR_IN *, BlobPacket_s *, DWORD dwFlags, DWORD dwSequence, WORD wTime);

	void AddBan(in_addr ipaddr, const char *admin, const char *reason);
	bool RemoveBan(in_addr ipaddr);
	std::string GetBanList();

	DWORD GetNumClients();

	SOCKET *m_sockets;

private:

	bool IsBannedIP(in_addr ipaddr);
	void LoadBans();
	void SaveBans();

	WORD AllocOpenClientSlot();

	CClient *ValidateClient(WORD, sockaddr_in *);
	CClient *FindClientByAccount(const char *);

	void SendConnectLoginFailure(sockaddr_in *addr, int error, const char *accountname, const char *password);
	void ConnectionRequest(sockaddr_in *addr, BlobPacket_s *p);
	void ProcessConnectionless(sockaddr_in *, BlobPacket_s *);

	class CPhatServer *m_server;
	int m_socketCount;

	WORD m_ServerID;

	/*
	WORD m_freeslot;
	WORD m_slotrange;
	CClient *m_clients[400];
	*/

	CClient **m_ClientArray = NULL;
	CClient **m_ClientSlots = NULL;
	std::list<WORD> m_OpenSlots;
	DWORD m_NumClients = 0;
	DWORD m_MaxClients = MAX_CONNECTED_CLIENTS;

	CNetworkBanList m_Bans;

	std::list<CQueuedPacket *> m_PacketQueue;

	void Init();
	void Shutdown();

	static DWORD WINAPI InternalThreadProcStatic(LPVOID lpThis);
	DWORD InternalThreadProc();

	void QueueIncomingOnSocket(SOCKET socket);
	void ProcessQueuedIncoming();
	void SendQueuedOutgoing();
	bool SendPacket(SOCKADDR_IN *peer, void *data, DWORD len);

	HANDLE m_hQuitEvent = NULL;
	HANDLE *m_hNetEvent = NULL;
	HANDLE m_hMakeTick = NULL;

	HANDLE m_hPumpThread = NULL;

	CRITICAL_SECTION _incomingLock;
	std::list<CQueuedPacket> _queuedIncoming;

	CRITICAL_SECTION _outgoingLock;
	std::list<CQueuedPacket> _queuedOutgoing;
};



