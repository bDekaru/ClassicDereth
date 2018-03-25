
#include "StdAfx.h"

#include "Client.h"

//Network access.
#include "crc.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "Network.h"
#include "FragStack.h"
#include "PacketController.h"
#include "World.h"
#include "ClientEvents.h"
#include "Player.h"
#include "ChatMsgs.h"
#include "Config.h"

CPacketController::CPacketController(CClient *client)
{
	m_pClient = client;
	m_pPeer = client->GetHostAddress();
}

CPacketController::~CPacketController()
{
	_outgoingMessageQueue.Lock();
	for (auto entry : _outgoingMessageQueue)
	{
		delete [] entry.data;
	}
	_outgoingMessageQueue.clear();
	_outgoingMessageQueue.Unlock();

	for (auto entry : m_in.fragstack)
	{
		delete entry;
	}
	m_in.fragstack.clear();

	m_out._fragQueue.Lock();
	for (auto entry : m_out._fragQueue)
	{
		delete [] ((BYTE *)entry);
	}
	m_out._fragQueue.clear();
	m_out._fragQueue.Unlock();

	for (auto &entry : m_in.blobstack)
	{
		DELETEBLOB(entry.second);
	}
	m_in.blobstack.clear();

	m_out._blobCache.Lock();
	for (auto &entry : m_out._blobCache)
	{
		DELETEBLOB(entry.second);
	}
	m_out._blobCache.clear();
	m_out._blobCache.Unlock();
}

DWORD CPacketController::GetNextEvent()
{
	return InterlockedIncrement(&m_out.event_counter);
}

void CPacketController::ResetEvent()
{
	m_out.event_counter = 0;
}

DWORD CPacketController::GetNextSequence()
{
	return InterlockedIncrement(&m_out.sequence);
}

BOOL CPacketController::SendNetMessageInternal(void *data, DWORD length, WORD group)
{
	DWORD dwRPC = 0x80000000;

	// Every fragment has a unique sequence.
	DWORD dwFragSequence = InterlockedIncrement(&m_out.fragment_counter);

	//Calculate the number of fragments necessary for the message.
	WORD wFragCount = (WORD)(length / MAX_FRAGMENT_LEN);
	if (length % MAX_FRAGMENT_LEN) wFragCount++;

	WORD wFragIndex = 0;
	WORD wFragLength = 0;
	while (wFragIndex < wFragCount)
	{
		if (wFragCount != (wFragIndex + 1))
			wFragLength = MAX_FRAGMENT_LEN;
		else
		{
			wFragLength = (WORD)(length % MAX_FRAGMENT_LEN);
			if (!wFragLength)
				wFragLength = MAX_FRAGMENT_LEN;
		}

		FragHeader_s header;
		{
			header.id = (DWORD64)dwFragSequence | (((DWORD64)dwRPC) << 32);
			header.wCount = wFragCount;
			header.wSize = sizeof(FragHeader_s) + wFragLength;
			header.wIndex = wFragIndex;
			header.wGroup = group;
		}

		QueueFragmentInternal(&header, &((BYTE *)data)[wFragIndex * MAX_FRAGMENT_LEN]);
		wFragIndex++;
	}

	return TRUE;
}

void CPacketController::QueueFragmentInternal(FragHeader_s *header, BYTE *data)
{
	FragPacket_s *fragment = (FragPacket_s *)new BYTE[header->wSize];

	memcpy(&fragment->header, header, sizeof(FragHeader_s));
	memcpy(&fragment->data, data, header->wSize - sizeof(FragHeader_s));

	m_out._fragQueue.Lock();
	m_out._fragQueue.push_back(fragment);
	m_out._fragQueue.Unlock();
}

void CPacketController::Cleanup()
{
	//Delete and remove whatever has been processed or successful sent.
	//The fragments that are complete have already been processed.
	for (std::list<FragmentStack *>::iterator it = m_in.fragstack.begin(); it != m_in.fragstack.end();)
	{
		FragmentStack *poo = *it;
		if (poo->IsComplete())
		{
			delete poo;
			it = m_in.fragstack.erase(it);
		}
		else
			it++;
	}

	for (std::map<DWORD, BlobPacket_s *>::iterator it = m_in.blobstack.begin(); it != m_in.blobstack.end();)
	{
		if (it->first > m_in.activesequence)
			break;
			
		DELETEBLOB(it->second);
		it = m_in.blobstack.erase(it);
	}

	if (m_out.sequence >= 1000) // keep no more than 1000 cached
	{
		if ((m_out.sequence - 1000) > m_in.flushsequence)
			m_in.flushsequence = m_out.sequence - 1000;		
	}

	m_out._blobCache.Lock();
	for (std::map<DWORD, BlobPacket_s *>::iterator it = m_out._blobCache.begin(); it != m_out._blobCache.end();)
	{
		if (it->first > m_in.flushsequence)
			break;

		DELETEBLOB(it->second);
		it = m_out._blobCache.erase(it);
	}
	m_out._blobCache.Unlock();
}

void CPacketController::ThinkInbound()
{
	//Check if we have sequences available for processing.
	DWORD dwDesired = m_in.activesequence + 1;

iterate:
	if (dwDesired <= m_in.sequence)
	{
		BlobMap::iterator it = m_in.blobstack.find(dwDesired);

		if (it != m_in.blobstack.end())
		{
			ProcessBlob(it->second);
			m_in.lastactivity = g_pGlobals->Time();

			DELETEBLOB(it->second);
			m_in.blobstack.erase(it);

			m_in.activesequence = dwDesired++;
			goto iterate;
		}
	}

	if ((dwDesired + 2 <= m_in.sequence) && ((m_in.lastrequest + 1) < g_pGlobals->Time()))
	{
		//request lost sequences
		std::vector<DWORD> lostSequences;
		lostSequences.push_back(dwDesired);
		DWORD lowBound = dwDesired + 1;
		DWORD highBound = m_in.sequence;
		for (DWORD check = lowBound; check < highBound; check++)
		{
			if (m_in.blobstack.find(check) == m_in.blobstack.end())
			{
				lostSequences.push_back(check);
			}
		}

		DWORD dwCount = (DWORD)lostSequences.size();
		CREATEBLOB(p, sizeof(DWORD) + dwCount * sizeof(DWORD));

		*((DWORD *)p->data) = (DWORD) lostSequences.size();
		DWORD *requestPos = ((DWORD *)p->data) + 1;

		std::vector<DWORD>::iterator requestIt = lostSequences.begin();
		for (; requestIt != lostSequences.end(); requestIt++) {
			*requestPos = *requestIt;
			requestPos++;
		}

		g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_REQUESTLOST, NULL, GetElapsedTime());

		DELETEBLOB(p);
		m_in.lastrequest = g_pGlobals->Time();

#ifdef _DEBUG
		LOG_PRIVATE(Network, Verbose, "Requesting %d lost packets for %s..\n", lostSequences.size(), m_pClient->GetDescription());
#endif

		m_in.numresendrequests += dwCount;
	}
}

void CPacketController::ThinkOutbound()
{
	double fTime = g_pGlobals->Time();

	if (m_out.nextflush < fTime)
	{
		FlushPeerCache();
	}

	if (m_out.nexttimeupdate < fTime)
	{
		UpdatePeerTime();
	}
	
	FlushQueuedNetMessages();
	FlushFragments();
}

void CPacketController::Think()
{
	if (!IsAlive())
		return;

	// Handle the input first.
	ThinkInbound();

	// Cleanup before we clutter the cache with new stuff.
	Cleanup();
	
	// Handle the output last.
	ThinkOutbound();

	if ((g_pGlobals->Time() - m_in.lastactivity) >= 30.0f)
	{
		Kill(__FILE__, __LINE__);
	}
}

void CPacketController::PerformLoginSync()
{
	/* PUT THIS BACK IN?
	m_out.lastloginsync = g_pGlobals->Time();

	CREATEBLOB( LoginSync, 0xE6 );
	memset( &LoginSync->data[8], 0, 0xDE );
	*((double *)LoginSync->data) = g_pGlobals->Time();

	g_pNetwork->SendConnectlessBlob(m_pPeer, LoginSync, BT_CONNECTIONACK, NULL, GetElapsedTime());
	DELETEBLOB( LoginSync );
	*/
}

void CPacketController::FlushPeerCache()
{
	m_out.nextflush = g_pGlobals->Time() + 3.0f;

	//
	CREATEBLOB(p, sizeof(DWORD));

	*((DWORD *)p->data) = m_in.activesequence;

	g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_ACKSEQUENCE, m_out.sequence, GetElapsedTime());
	DELETEBLOB(p);
}

void CPacketController::UpdatePeerTime()
{
	m_out.nexttimeupdate = g_pGlobals->Time() + 3.0;

	CREATEBLOB(tupdate, sizeof(double));
	*((double *)tupdate->data) = g_pGlobals->Time();

	BlobHeader_s *header = &tupdate->header;

	header->dwSequence = GetNextSequence();
	header->dwFlags = BT_TIMEUPDATE | BT_USES_CRC;
	header->dwCRC = 0;
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	DWORD dwXOR = GetNextXORVal(m_out.crypto);
	header->dwCRC = BlobCRC(tupdate, dwXOR);

	//Off you go.
	g_pNetwork->QueuePacket(m_pPeer, tupdate, BLOBLEN(tupdate));
	// g_pNetwork->SendConnectlessBlob(m_pPeer, tupdate, BT_TIMEUPDATE, m_out.sequence, GetElapsedTime());

	//Cache for later use.
	m_out._blobCache.insert(std::pair<DWORD, BlobPacket_s *>(header->dwSequence, tupdate));

	m_out._sentBytes += header->wSize + sizeof(BlobHeader_s);
}

WORD CPacketController::GetElapsedTime()
{
	return (WORD)((g_pGlobals->Time() - m_in.logintime) * 2);
}

//This is a generic handler for malicious clients.
void CPacketController::EvilClient(const char* szSource, DWORD dwLine, BOOL bKill)
{
#ifdef _DEBUG
	if (szSource)
		LOG(Temp, Normal, "Evil client @ %u of %s!!!\n", dwLine, szSource);
#endif

	if (bKill && IsAlive())
	{
		Kill(szSource, dwLine);
	}
}

void CPacketController::ProcessBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	DWORD dwSequence = header->dwSequence;
	DWORD dwFlags = header->dwFlags;
	DWORD dwCRC = header->dwCRC;
	DWORD dwSize = header->wSize;
	BYTE* pbData = blob->data;

	if (dwFlags & BT_REQUESTLOST) //0x00000002
	{
		if (dwSize >= 4)
		{
			//LOG(Temp, Normal, "Client requesting lost packets.\n");
			DWORD dwLostPackets = *((DWORD *)pbData);

			pbData += sizeof(DWORD);
			dwSize -= sizeof(DWORD);

			if (dwSize >= (dwLostPackets * sizeof(DWORD)))
			{
				DWORD dwDenySize = 0;
				DWORD dwDenyBlobs[0x50];

				DWORD lowest_sequence = 0;

				for (unsigned int i = 0; i < dwLostPackets; i++)
				{
					DWORD dwRequested = ((DWORD *)pbData)[i];

					if (dwRequested < 2 || dwRequested > m_out.sequence)
					{
						// Sequence out of range.
						FlagEvilClient();
						continue;
					}

					if (!lowest_sequence || dwRequested < lowest_sequence)
					{
						lowest_sequence = dwRequested;
					}

					//Find the requested sequence in the cache.
					m_out._blobCache.Lock();
					BlobMap::iterator it = m_out._blobCache.find(dwRequested);

					if (it != m_out._blobCache.end())
					{
						ResendBlob(it->second);
						m_out._blobCache.Unlock();
						m_out.numretransmit++;
					}
					else
					{
						m_out._blobCache.Unlock();

						if (dwDenySize < 0x50)
						{
							dwDenyBlobs[dwDenySize] = dwRequested;
							dwDenySize++;
						}
						else
						{
							// Too many
							FlagEvilClient();
						}
					}
				}

				if (dwDenySize > 0)
				{
					DWORD dwLength = dwDenySize * sizeof(DWORD);
					CREATEBLOB(p, sizeof(DWORD) + dwLength);

					memcpy(p->data, &dwDenySize, sizeof(DWORD));
					memcpy(p->data + sizeof(DWORD), dwDenyBlobs, dwLength);

					g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_DENY, NULL, GetElapsedTime());

					DELETEBLOB(p);
					
					m_out.numdenied += dwDenySize;
				}

				if (lowest_sequence)
				{
					m_in.flushsequence = lowest_sequence;
				}

				pbData += sizeof(DWORD) * dwLostPackets;
				dwSize -= sizeof(DWORD) * dwLostPackets;
			}
			else
			{
				FlagEvilClient();
				return;
			}
		}
		else
		{
			FlagEvilClient();
			return;
		}
	}

	if (dwFlags & BT_DENY) //0x00000008
	{
		if (dwSize >= 4)
		{
			DWORD dwDeniedCount = *((DWORD *)pbData);

			pbData += sizeof(DWORD);
			dwSize -= sizeof(DWORD);

			if (dwSize == (dwDeniedCount * sizeof(DWORD)))
			{
				unsigned int i = 0;
				while (i < dwSize)
				{
					DWORD dwDenied = ((DWORD *)pbData)[i];

					if (dwDenied <= m_in.sequence)
					{
						if (dwDenied == (m_in.activesequence + 1))
						{
							m_in.activesequence++;
							i = 0;
						}
					}
					//else
					//	FlagEvilClient(__FILE__, __LINE__);

					i++;
				}
			}
			else
				FlagEvilClient();
		}
		else
			FlagEvilClient();

		return;
	}

	if (dwFlags & BT_ACKSEQUENCE)
	{
		if (dwSize < 4)
		{
			FlagEvilClient();
			return;
		}
		
		m_in.flushsequence = *((DWORD *)pbData);

		dwSize -= 4;
		pbData += 4;
	}

	/*
	if (dwFlags & BT_LOGIN)
	{
		if (m_out.loginsyncs < 0)
			m_out.loginsyncs = 0;

		return;
	}
	*/

	/*
	if (dwFlags & BT_CONNECTIONACK) //0x00000040
	{
		if (m_out.loginsyncs >= 0)
		{
			m_out.loginsyncs++;
			if (m_out.loginsyncs < 2)
			{
				m_in.lastactivity = g_pGlobals->Time();
				// PerformLoginSync();
			}
		}

		return;
	}
	*/

	if (dwFlags & BT_TIMEUPDATE) //0x00100000
	{
#ifdef _DEBUG
		//if (dwFlags & BT_FRAGMENTS)
		//	LOG(Temp, Normal, "Fragments? %s %u\n", __FILE__, __LINE__);
#endif
		if (dwSize < 8)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			//Time critical.
			/*
			const double gear_tolerance = 1.0f; // Cheater!! =)

			if ( (g_pGlobals->Time() + gear_tolerance) < *((double *)pbData) )
			{
				const char* account = m_pClient->GetAccount( );
				SOCKADDR_IN *ip = m_pClient->GetHostAddress( );

				LOG(Temp, Normal, "Detected client(%s @ %s) using gear. Killing!\n", (account ? account : "???"), (ip ? inet_ntoa(ip->sin_addr) : "???") );

				g_pNetwork->KickClient( m_pClient->GetIndex() );
			}

			return;
			*/
		}

		dwSize -= 8;
		pbData += 8;
	}

	if (dwFlags & BT_ECHOREQUEST)
	{
		if (dwSize < 4)
		{
			FlagEvilClient();
			return;
		}

		float echoValue = *((float *)pbData);

		/*
		CREATEBLOB(tupdate, sizeof(DWORD));
		*((DWORD *)tupdate->data) = echoValue;

		BlobHeader_s *header = &tupdate->header;

		header->dwSequence = m_out.sequence;
		header->dwFlags = BT_ECHORESPONSE | BT_USES_CRC;
		header->dwCRC = 0;
		header->wRecID = g_pNetwork->GetServerID();
		header->wTime = GetElapsedTime();
		header->wTable = 0x01;

		DWORD dwXOR = GetSendXORVal(m_out.servercrc);
		header->dwCRC = BlobCRC(tupdate, dwXOR);

		//Off you go.
		g_pNetwork->QueuePacket(m_pPeer, tupdate, BLOBLEN(tupdate));

		//Cache for later use.
		header->dwCRC = dwXOR;
		m_out.blobcache.push_back(tupdate);
		*/

		CREATEBLOB(tupdate, sizeof(float)*2);
		((float *)tupdate->data)[0] = echoValue;
		((float *)tupdate->data)[1] = echoValue;

		g_pNetwork->SendConnectlessBlob(m_pPeer, tupdate, BT_ECHORESPONSE, m_out.sequence, GetElapsedTime());

		DELETEBLOB(tupdate);

		pbData += sizeof(DWORD);
		dwSize -= sizeof(DWORD);
	}

	if (dwFlags & BT_ECHORESPONSE)
	{
		if (dwSize < 8)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			pbData += sizeof(float);
			pbData += sizeof(float);
		}

		dwSize -= 8;
	}

	if (dwFlags & BT_FLOW)
	{
		if (dwSize < 6)
		{
			FlagEvilClient();
			return;
		}
		else
		{
			DWORD dwBytes = *((DWORD *)pbData);
			pbData += sizeof(DWORD);
			WORD wElapsed = *((WORD *)pbData);
			pbData += sizeof(WORD);

			//LOG(Temp, Normal, "Flow: %u bytes over %u seconds\n", dwBytes, wElapsed );
		}

		dwSize -= 6;
	}

	if (dwFlags & BT_FRAGMENTS)
	{
		while (dwSize >= sizeof(FragHeader_s))
		{
			FragPacket_s *frag = (FragPacket_s *)pbData;

			if (dwSize < frag->header.wSize)
				break;

			pbData += frag->header.wSize;
			dwSize -= frag->header.wSize;

			BYTE* data = frag->data;
			DWORD length = frag->header.wSize - sizeof(FragHeader_s);

			if (frag->header.wCount == 1)
			{
				ProcessMessage(data, length, frag->header.wGroup);
			}
			else
			{
				FragmentList::iterator it;
				DWORD fragID = frag->header.id;

				bool bCompleted = false;
				for (it = m_in.fragstack.begin(); it != m_in.fragstack.end(); it++)
				{
					FragmentStack *s = *it;
					if (s->_id == fragID)
					{
						s->AddFragment(frag);

						if (s->IsComplete())
						{
							ProcessMessage(s->GetData(), s->GetLength(), frag->header.wGroup);
							delete s;

							m_in.fragstack.erase(it);
							bCompleted = true;
						}
						break;
					}
				}

				
				if (!bCompleted)
				{					
					if (it == m_in.fragstack.end())
					{
						// No existing group matches, create one
						FragmentStack *s = new FragmentStack(frag);
						m_in.fragstack.push_back(s);
					}
				}
			}
		}
	}
}

void CPacketController::ProcessMessage(BYTE *data, DWORD length, WORD group)
{
	if (m_pClient->IsAlive())
		m_pClient->ProcessMessage(data, length, group);
}

void CPacketController::IncomingBlob(BlobPacket_s *blob, double recvTime)
{
	//No CRC checks as of yet.
	BlobHeader_s *header = &blob->header;

	DWORD dwSequence = header->dwSequence;
	DWORD dwFlags = header->dwFlags;
	DWORD dwCRC = header->dwCRC;
	DWORD dwSize = header->wSize;
	BYTE* pbData = blob->data;

	m_in.receivedbytes += header->wSize + sizeof(BlobHeader_s);

	if (dwSequence > m_in.sequence)
		m_in.sequence = dwSequence;

#if 1
	if (g_pConfig->SpeedHackKicking())
	{
		if (!m_in._clientIntervalBase)
		{
			m_in._clientIntervalBase = header->wTime;
			m_in._clientIntervalBaseStart = recvTime;
		}
		else
		{
			short intervalDiff = (short)(header->wTime - m_in._clientIntervalBase);

			if (intervalDiff > 120 || intervalDiff < 0)
			{
				m_in._clientIntervalBase = header->wTime;
				m_in._clientIntervalBaseStart = recvTime;
			}
			else if (intervalDiff > 30)
			{
				DWORD expectedIntervals = (recvTime - m_in._clientIntervalBaseStart) * 2.0;
				DWORD actualIntervals = intervalDiff;

				if ((float)actualIntervals > (float)(expectedIntervals * 2.0))
				{
					LOG(Temp, Normal, "Possible speed hack on user: %s ([rate: %f]) Disconnecting.", m_pClient->GetDescription(), (double)actualIntervals / (double)expectedIntervals);

					Kill(__FILE__, __LINE__);

					/*
					if (m_pClient && m_pClient->GetEvents() && m_pClient->GetEvents()->GetPlayer())
					{
						if (g_pWorld)
							g_pWorld->BroadcastGlobal(ServerText(csprintf("%s has been detected using a possible speed hack and has been kicked.", m_pClient->GetEvents()->GetPlayer()->GetName().c_str()), 1), PRIVATE_MSG);
					}
					*/
				}
			}
		}
	}
#endif

	if (dwFlags & BT_DISCONNECT)
	{
		Kill(__FILE__, __LINE__);
		return;
	}
	
	if (dwFlags & BT_REQUESTLOST)
	{
		ProcessBlob(blob);
		return;
	}
	
	if (!dwSequence)
	{
		ProcessBlob(blob);
		return;
	}
	if (dwSequence <= m_in.activesequence) return;

	// Cache this blob, for later use.
	if (m_in.blobstack.find(dwSequence) != m_in.blobstack.end())
	{
		// LOG(Temp, Normal, "Already had blob sequence!\n");
	}
	else
	{
		DUPEBLOB(p, blob);
		m_in.blobstack.insert(std::pair<DWORD, BlobPacket_s *>(dwSequence, p));
	}
}

void CPacketController::ResendBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	header->dwFlags |= BT_RESENT;

	DWORD dwXOR = header->dwCRC;
	header->dwCRC = BlobCRC(blob, dwXOR);

	//Off you go.
	g_pNetwork->QueuePacket(m_pPeer, blob, BLOBLEN(blob));

	//Keep in cache.
	header->dwCRC = dwXOR;

	m_out._sentBytes += header->wSize + sizeof(BlobHeader_s);
}

void CPacketController::SendFragmentBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	header->dwSequence = GetNextSequence();
	header->dwFlags = BT_FRAGMENTS | BT_USES_CRC;
	header->dwCRC = 0;
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	DWORD dwXOR = GetNextXORVal(m_out.crypto);
	header->dwCRC = BlobCRC(blob, dwXOR);

	//Off you go.
	g_pNetwork->QueuePacket(m_pPeer, blob, BLOBLEN(blob));

	//Cache for later use.
	header->dwCRC = dwXOR;

	m_out._blobCache.Lock();
	m_out._blobCache.insert(std::pair<DWORD, BlobPacket_s *>(header->dwSequence, blob));
	m_out._blobCache.Unlock();

	m_out._sentBytes += header->wSize + sizeof(BlobHeader_s);
}

// Sends all the queued fragments
void CPacketController::FlushFragments()
{
	m_out._fragQueue.Lock();

	auto entry = m_out._fragQueue.begin();
	if (entry == m_out._fragQueue.end())
	{
		m_out._fragQueue.Unlock();
		return;
	}
	m_out._fragQueue.size();

	BlobPacket_s *blob = (BlobPacket_s *)new BYTE[MAX_BLOB_LEN];
	blob->header.wSize = 0;

	do
	{
		// copy it locally
		FragPacket_s *edible = *entry;

		// remove
		m_out._fragQueue.erase(entry);
		m_out._fragQueue.Unlock();

		if ((blob->header.wSize + edible->header.wSize) < (MAX_BLOB_LEN - sizeof(BlobHeader_s)))
		{
			BYTE *dataPos = &blob->data[blob->header.wSize];
			WORD wSize = edible->header.wSize;
			memcpy(dataPos, edible, wSize);
			blob->header.wSize += wSize;
		}
		else
		{
			// this one is full, let's send it and make a new one.
			SendFragmentBlob(blob);

			blob = (BlobPacket_s *)new BYTE[MAX_BLOB_LEN];

			WORD wSize = blob->header.wSize = edible->header.wSize;
			memcpy(blob->data, edible, wSize);
		}

		delete [] ((BYTE *)edible);

		m_out._fragQueue.Lock();
		entry = m_out._fragQueue.begin();
	} while (entry != m_out._fragQueue.end());

	m_out._fragQueue.Unlock();

	SendFragmentBlob(blob);
}

void CPacketController::QueueNetMessage(void *data, DWORD length, WORD group, DWORD weenie_id)
{
	if (!IsAlive())
	{
		return;
	}

	BYTE *messageData;
	DWORD messageLength;

	if (weenie_id)
	{
		messageLength = sizeof(EventHeader) + length;
		messageData = new BYTE[messageLength];

		EventHeader *gameEvent = (EventHeader *)messageData;
		gameEvent->dwF7B0 = 0xF7B0;
		gameEvent->dwPlayer = weenie_id;
		gameEvent->dwSequence = GetNextEvent();
		memcpy((BYTE *)messageData + sizeof(EventHeader), data, length);
	}
	else
	{
		messageLength = length;
		messageData = new BYTE[messageLength];
		memcpy(messageData, data, length);
	}
	
	COutgoingNetMessage message;
	message.data = messageData;
	message.length = messageLength;
	message.group_id = group;

	_outgoingMessageQueue.Lock();
	_outgoingMessageQueue.push_back(message);
	_outgoingMessageQueue.Unlock();
}

void CPacketController::FlushQueuedNetMessages()
{
	_outgoingMessageQueue.Lock();

	auto entry = _outgoingMessageQueue.begin();
	while (entry != _outgoingMessageQueue.end())
	{
		// copy it locally
		COutgoingNetMessage queued = *entry;
		// remove
		_outgoingMessageQueue.erase(entry);

		// unlock while we send it
		_outgoingMessageQueue.Unlock();

		SendNetMessageInternal(queued.data, queued.length, queued.group_id);
		delete[] queued.data;

		_outgoingMessageQueue.Lock();
		entry = _outgoingMessageQueue.begin();
	}

	_outgoingMessageQueue.Unlock();
}
