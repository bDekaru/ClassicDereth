
#pragma once

#include "crcwheel.h"

class FragmentStack;

typedef std::list<BlobPacket_s *> BlobList;
typedef std::list<FragmentStack *> FragmentList;
typedef std::map<DWORD, BlobPacket_s *> BlobMap;

#ifdef _DEBUG
#define FlagEvilClient() EvilClient( __FILE__, __LINE__, FALSE )
#else
#define FlagEvilClient() EvilClient( NULL, NULL, TRUE )
#endif

#pragma pack(push, 4)
struct EventHeader
{
	DWORD dwF7B0;
	DWORD dwPlayer;
	DWORD dwSequence;
};
#pragma pack(pop)

class COutgoingNetMessage
{
public:
	BYTE *data;
	DWORD length;
	WORD group_id;
};

class CNetwork;

class CPacketController : public CKillable
{
public:
	CPacketController(CClient *);
	~CPacketController();

	void Think(); //Generic work cycle.
	void ThinkInbound();
	void ThinkOutbound();
	
	void IncomingBlob(BlobPacket_s *blob, double recvTime);

	DWORD GetNextSequence();;
	DWORD GetNextEvent();
	void ResetEvent();

	DWORD GetClientCryptoSeed() { return m_in.crypto_seed; }
	DWORD GetServerCryptoSeed() { return m_out.crypto_seed; }

// private:
	WORD GetElapsedTime();

	void Cleanup();
	void FlushFragments();
	void FlushPeerCache();
	void PerformLoginSync();
	void UpdatePeerTime();
	void UpdateCharacters();

	void ProcessMessage(BYTE *data, DWORD length, WORD);
	void ProcessBlob(BlobPacket_s *);

	void EvilClient(const char* szSource, DWORD dwLine, BOOL bKill);
	void ResendBlob(BlobPacket_s *);
	void SendGenericBlob(BlobPacket_s *, DWORD dwFlags, DWORD dwSequence);
	void SendFragmentBlob(BlobPacket_s *);

	CClient	*m_pClient;
	SOCKADDR_IN	*m_pPeer;

	struct InputPCVars_t
	{
		InputPCVars_t()
		{
			logintime = g_pGlobals->Time();
			lastactivity = g_pGlobals->Time();
			lastrequest = g_pGlobals->Time();

			sequence = 0;
			activesequence = 1;
			flushsequence = 0;

			crypto_seed = Random::GenUInt(0x10000000, 0x70000000);
			crypto = AllocCrypto(crypto_seed);
		}

		~InputPCVars_t()
		{
			FreeCrypto(crypto);
		}

		double logintime; // When the client connected. Used for time subtraction.
		double lastactivity; // When the last SUCCESSFUL packet received.
		double lastrequest;	//When the last lost packets request was made.

		//Sequencing.
		DWORD sequence; //The blob counter. The last one we know the client sent.
		DWORD activesequence; //The blob counter. The last sequence we processed.
		DWORD flushsequence;	//The last sequence the client processed.

		//CRC
		void *crypto;
		DWORD crypto_seed;

		std::list<FragmentStack *> fragstack; // All incomplete messages stack here.
		std::map<DWORD, BlobPacket_s *> blobstack; // All queued blobs wait here. (for sequencing)

		WORD _clientIntervalBase = 0;
		double _clientIntervalBaseStart = 0.0;

		DWORD64 receivedbytes = 0;
		DWORD numresendrequests = 0;
	} m_in;

	struct OutputPCVars_t
	{
		//Network variables for outgoing data.

		OutputPCVars_t()
		{
			nextflush = g_pGlobals->Time() + 3.0;
			nexttimeupdate = 0.0;

			lastloginsync = 0;
			loginsyncs = 0; // -1;

			sequence = 1;
			fragment_counter = 1;
			event_counter = 0;

			initchars = FALSE;

			crypto_seed = Random::GenUInt(0x10000000, 0x70000000);
			crypto = AllocCrypto(crypto_seed);
		}

		~OutputPCVars_t()
		{
			FreeCrypto(crypto);
		}

		double nextflush; // Used to clean the peer's blob cache.
		double nexttimeupdate; // Used to correct the client's time.

		double lastloginsync; // Used to sync the time at login.
		long loginsyncs; // The number of login syncs performed.

		BOOL initchars;

		//Sequencing.
		DWORD sequence;	//The blob counter.
		DWORD fragment_counter;	//The fragment counter.
		DWORD event_counter; //The game event counter.

		//CRC
		void *crypto;
		DWORD crypto_seed;

		TLockable<std::list<FragPacket_s *>> _fragQueue; // All fragments to be sent will wait here.
		TLockable<std::map<DWORD, BlobPacket_s *>> _blobCache; // Remember blobs until the client receives them.

		DWORD numretransmit = 0;
		DWORD numdenied = 0;

		DWORD64 _sentBytes = 0;

	} m_out;

	void QueueNetMessage(void *data, DWORD length, WORD group, DWORD weenie_id);
	void FlushQueuedNetMessages();

	TLockable<std::list<COutgoingNetMessage>> _outgoingMessageQueue;

private:
	BOOL SendNetMessageInternal(void *data, DWORD length, WORD group);
	void QueueFragmentInternal(FragHeader_s *header, BYTE *data);
};
