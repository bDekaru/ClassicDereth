
#pragma once

#include "crcwheel.h"

const double DelayAck = 4.0;
const double DelayTime = 20.0;

class FragmentStack;

typedef std::list<BlobPacket_s *> BlobList;
typedef std::list<FragmentStack *> FragmentList;
typedef std::map<uint32_t, BlobPacket_s *> BlobMap;

#ifdef _DEBUG
#define FlagEvilClient() EvilClient( __FILE__, __LINE__, FALSE )
#else
#define FlagEvilClient() EvilClient( NULL, NULL, TRUE )
#endif

#pragma pack(push, 4)
struct EventHeader
{
	uint32_t dwF7B0;
	uint32_t dwPlayer;
	uint32_t dwSequence;
};
#pragma pack(pop)

class COutgoingNetMessage
{
public:
	BYTE *data;
	uint32_t length;
	WORD group_id;
	bool ephemeral = false;
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

	uint32_t GetNextSequence();;
	uint32_t GetNextEvent();
	void ResetEvent();

	uint32_t GetClientCryptoSeed() { return m_in.crypto_seed; }
	uint32_t GetServerCryptoSeed() { return m_out.crypto_seed; }

// private:
	WORD GetElapsedTime();

	void Cleanup();
	void FlushFragments();
	void FlushPeerCache();
	void PerformLoginSync();
	void UpdatePeerTime();
	void UpdateCharacters();

	void ProcessMessage(BYTE *data, uint32_t length, WORD);
	void ProcessBlob(BlobPacket_s *);

	void EvilClient(const char* szSource, uint32_t dwLine, BOOL bKill);
	void ResendBlob(BlobPacket_s *);
	void SendGenericBlob(BlobPacket_s *, uint32_t dwFlags, uint32_t dwSequence);
	void SendFragmentBlob(BlobPacket_s *);

	void SendCriticalError(uint32_t table, uint32_t stringId);

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
		uint32_t sequence; //The blob counter. The last one we know the client sent.
		uint32_t activesequence; //The blob counter. The last sequence we processed.
		uint32_t flushsequence;	//The last sequence the client processed.

		//CRC
		void *crypto;
		uint32_t crypto_seed;

		std::list<FragmentStack *> fragstack; // All incomplete messages stack here.
		std::map<uint32_t, BlobPacket_s *> blobstack; // All queued blobs wait here. (for sequencing)

		WORD _clientIntervalBase = 0;
		double _clientIntervalBaseStart = 0.0;

		uint64_t receivedbytes = 0;
		uint32_t numresendrequests = 0;

		bool alive = false;
	} m_in;

	struct OutputPCVars_t
	{
		//Network variables for outgoing data.

		OutputPCVars_t()
		{
			//nextflush = g_pGlobals->Time() + 3.0;
			nextflush = DelayAck;
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
		int32_t loginsyncs; // The number of login syncs performed.

		BOOL initchars;

		//Sequencing.
		// uint32_t sequence;	//The blob counter.
		// uint32_t fragment_counter;	//The fragment counter.
		// uint32_t event_counter; //The game event counter.

		std::atomic_uint32_t sequence;
		std::atomic_uint32_t fragment_counter;
		std::atomic_uint32_t event_counter;

		//CRC
		void *crypto;
		uint32_t crypto_seed;

		TLockable<std::list<FragPacket_s *>> _fragQueue; // All fragments to be sent will wait here.
		TLockable<std::map<uint32_t, BlobPacket_s *>> _blobCache; // Remember blobs until the client receives them.

		uint32_t numretransmit = 0;
		uint32_t numdenied = 0;

		uint64_t _sentBytes = 0;

		uint32_t echoData = 0;

	} m_out;

	void QueueNetMessage(void *data, uint32_t length, WORD group, uint32_t weenie_id, bool ephemeral = false);
	void FlushQueuedNetMessages();

	TLockable<std::list<COutgoingNetMessage>> _outgoingMessageQueue;

private:
	BOOL SendNetMessageInternal(void *data, uint32_t length, WORD group, bool ephemeral);
	void QueueFragmentInternal(FragHeader_s *header, BYTE *data);
};
