
#include <StdAfx.h>

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

uint32_t CPacketController::GetNextEvent()
{
	// return InterlockedIncrement(&m_out.event_counter);
	return ++m_out.event_counter;
}

void CPacketController::ResetEvent()
{
	m_out.event_counter = 0;
}

uint32_t CPacketController::GetNextSequence()
{
	// return InterlockedIncrement(&m_out.sequence);
	return ++m_out.sequence;
}

BOOL CPacketController::SendNetMessageInternal(void *data, uint32_t length, WORD group, bool ephemeral)
{
	uint32_t dwRPC = 0x80000000;

	// Every fragment has a unique sequence.
	// uint32_t dwFragSequence = InterlockedIncrement(&m_out.fragment_counter);
	uint32_t dwFragSequence = ++m_out.fragment_counter;

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
			if (ephemeral)
				header.id = (uint64_t)dwFragSequence | (((uint64_t)dwRPC) << 32);
			else
				header.id = (uint64_t)dwFragSequence;
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

	for (std::map<uint32_t, BlobPacket_s *>::iterator it = m_in.blobstack.begin(); it != m_in.blobstack.end();)
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
	for (std::map<uint32_t, BlobPacket_s *>::iterator it = m_out._blobCache.begin(); it != m_out._blobCache.end();)
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
	uint32_t dwDesired = m_in.activesequence + 1;

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
		std::vector<uint32_t> lostSequences;
		lostSequences.push_back(dwDesired);
		uint32_t lowBound = dwDesired + 1;
		uint32_t highBound = m_in.sequence;
		for (uint32_t check = lowBound; check < highBound; check++)
		{
			if (m_in.blobstack.find(check) == m_in.blobstack.end())
			{
				lostSequences.push_back(check);
			}
		}

		uint32_t dwCount = (uint32_t)lostSequences.size();
		CREATEBLOB(p, sizeof(uint32_t) + dwCount * sizeof(uint32_t));

		*((uint32_t *)p->data) = (uint32_t) lostSequences.size();
		uint32_t *requestPos = ((uint32_t *)p->data) + 1;

		std::vector<uint32_t>::iterator requestIt = lostSequences.begin();
		for (; requestIt != lostSequences.end(); requestIt++) {
			*requestPos = *requestIt;
			requestPos++;
		}

		g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_REQUESTLOST, m_out.sequence, GetElapsedTime());

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
	//double fTime = g_pGlobals->Time();
	if (!m_in.alive) return;

	FlushQueuedNetMessages();
	FlushFragments();

//	if (m_out.nextflush < fTime)
//	{
//		FlushPeerCache();
//	}

	//if (m_out.nexttimeupdate < fTime)
	//{
		UpdatePeerTime();
	//}

	//m_in.alive = true;
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
	//ThinkOutbound();

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
	CREATEBLOB(p, sizeof(uint32_t));

	*((uint32_t *)p->data) = m_in.activesequence;

	g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_ACKSEQUENCE, m_out.sequence, GetElapsedTime());
	DELETEBLOB(p);
}

void CPacketController::UpdatePeerTime()
{
	size_t blobLen = 0;
	double fTime = GetElapsedTime();

	BYTE blob[sizeof(BlobHeader_s) + 20] = { 0 };
	BYTE *pBlob = blob + sizeof(BlobHeader_s);

	BlobHeader_s *header = (BlobHeader_s*)&blob[0];
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	// ack, sync, echo, flow

	// ack every 4 ticks
	// sync every 20
	// echo as needed
	// flow at the start/end of every "burst" ???

	if (m_out.nextflush <= fTime)
	{
		m_out.nextflush = fTime + DelayAck;
		header->dwFlags |= BT_ACKSEQUENCE;

		*((uint32_t *)pBlob) = m_in.activesequence;
		pBlob += sizeof(uint32_t);
		blobLen += sizeof(uint32_t);
	}

	if (m_out.nexttimeupdate <= fTime)
	{
		m_out.nexttimeupdate = fTime + DelayTime;
		header->dwFlags |= BT_TIMEUPDATE | BT_USES_CRC;

		*((double *)pBlob) = g_pGlobals->Time();
		pBlob += sizeof(double);
		blobLen += sizeof(double);
	}

	if (m_out.echoData)
	{
		// echo response was never alone in retail
		header->dwFlags |= BT_ECHORESPONSE | BT_USES_CRC;

		*((uint32_t *)pBlob) = m_out.echoData;

		pBlob += sizeof(uint32_t);
		blobLen += sizeof(uint32_t);

		*((uint32_t *)pBlob) = m_out.echoData;

		pBlob += sizeof(uint32_t);
		blobLen += sizeof(uint32_t);

		m_out.echoData = 0;
	}

	if (blobLen == 0)
		return;

	header->dwCRC = 0;
	header->wSize = blobLen;

	uint32_t dwXOR = 0;

	if (header->dwFlags & BT_USES_CRC)
	{
		CREATEBLOB(tupdate, blobLen);
		memcpy(tupdate, blob, blobLen + sizeof(BlobHeader_s));

		header = &tupdate->header;

		header->dwSequence = GetNextSequence();
		dwXOR = GetNextXORVal(m_out.crypto);
		header->dwCRC = BlobCRC(tupdate, dwXOR);

		//Cache for later use.
		m_out._blobCache.Lock();
		m_out._blobCache.insert(std::pair<uint32_t, BlobPacket_s *>(header->dwSequence, tupdate));
		m_out._blobCache.Unlock();

		pBlob = (BYTE*)tupdate;
	}
	else
	{
		header->dwSequence = m_out.sequence;
		header->dwCRC = GenericCRC((BlobPacket_s*)blob);

		pBlob = blob;
	}

	//Off you go.
	g_pNetwork->QueuePacket(m_pPeer, pBlob, blobLen + sizeof(BlobHeader_s));
	// g_pNetwork->SendConnectlessBlob(m_pPeer, tupdate, BT_TIMEUPDATE, m_out.sequence, GetElapsedTime());

	// save crc seed
	header->dwCRC = dwXOR;

	m_out._sentBytes += header->wSize + sizeof(BlobHeader_s);
}

WORD CPacketController::GetElapsedTime()
{
	return (WORD)((g_pGlobals->Time() - m_in.logintime) * 2);
}

//This is a generic handler for malicious clients.
void CPacketController::EvilClient(const char* szSource, uint32_t dwLine, BOOL bKill)
{
#ifdef _DEBUG
	if (szSource)
		SERVER_INFO << "Evil client @"<< dwLine << "of" << szSource << "!";
#endif

	if (bKill && IsAlive())
	{
		Kill(szSource, dwLine);
	}
}

void CPacketController::ProcessBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	uint32_t dwSequence = header->dwSequence;
	uint32_t dwFlags = header->dwFlags;
	uint32_t dwCRC = header->dwCRC;
	uint32_t dwSize = header->wSize;
	BYTE* pbData = blob->data;

	if (dwFlags & BT_CONNECTIONACK)
	{
		// TODO: check cookie
		UpdatePeerTime();
		m_in.alive = true;
	}

	if (dwFlags & BT_REQUESTLOST) //0x00000002
	{
		if (dwSize >= 4)
		{
			//LOG(Temp, Normal, "Client requesting lost packets.\n");
			uint32_t dwLostPackets = *((uint32_t *)pbData);

			pbData += sizeof(uint32_t);
			dwSize -= sizeof(uint32_t);

			if (dwSize >= (dwLostPackets * sizeof(uint32_t)))
			{
				uint32_t dwDenySize = 0;
				uint32_t dwDenyBlobs[0x50];

				uint32_t lowest_sequence = 0;

				for (unsigned int i = 0; i < dwLostPackets; i++)
				{
					uint32_t dwRequested = ((uint32_t *)pbData)[i];

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
					uint32_t dwLength = dwDenySize * sizeof(uint32_t);
					CREATEBLOB(p, sizeof(uint32_t) + dwLength);

					memcpy(p->data, &dwDenySize, sizeof(uint32_t));
					memcpy(p->data + sizeof(uint32_t), dwDenyBlobs, dwLength);

					g_pNetwork->SendConnectlessBlob(m_pPeer, p, BT_DENY, NULL, GetElapsedTime());

					DELETEBLOB(p);

					m_out.numdenied += dwDenySize;
				}

				//if (lowest_sequence)
				//{
				//	m_in.flushsequence = lowest_sequence;
				//}

				pbData += sizeof(uint32_t) * dwLostPackets;
				dwSize -= sizeof(uint32_t) * dwLostPackets;
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
			uint32_t dwDeniedCount = *((uint32_t *)pbData);

			pbData += sizeof(uint32_t);
			dwSize -= sizeof(uint32_t);

			if (dwSize >= (dwDeniedCount * sizeof(uint32_t)))
			{
				unsigned int i = 0;
				while (i < dwDeniedCount)
				{
					uint32_t dwDenied = *((uint32_t *)pbData);

					if (dwDenied <= m_in.sequence)
					{
						if (dwDenied == (m_in.activesequence + 1))
						{
							m_in.activesequence++;
						}
					}
					//else
					//	FlagEvilClient(__FILE__, __LINE__);

					i++;
					pbData += sizeof(uint32_t);
					dwSize -= sizeof(uint32_t);
				}
			}
			else
				FlagEvilClient();
		}
		else
			FlagEvilClient();

		//return;
	}

	if (dwFlags & BT_ACKSEQUENCE)
	{
		if (dwSize < 4)
		{
			FlagEvilClient();
			return;
		}

		m_in.flushsequence = *((uint32_t *)pbData);

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

        m_out.echoData = *(uint32_t*)pbData;
		//float echoValue = *((float *)pbData);

		//CREATEBLOB(tupdate, sizeof(float)*2);
		//((float *)tupdate->data)[0] = echoValue;
		//((float *)tupdate->data)[1] = echoValue;

		//g_pNetwork->SendConnectlessBlob(m_pPeer, tupdate, BT_ECHORESPONSE, m_out.sequence, GetElapsedTime());

		//DELETEBLOB(tupdate);

		pbData += sizeof(uint32_t);
		dwSize -= sizeof(uint32_t);
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
			uint32_t dwBytes = *((uint32_t *)pbData);
			pbData += sizeof(uint32_t);
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
			uint32_t length = frag->header.wSize - sizeof(FragHeader_s);

			if (frag->header.wCount == 1)
			{
				ProcessMessage(data, length, frag->header.wGroup);
			}
			else
			{
				FragmentList::iterator it;
				uint64_t fragID = frag->header.id;

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

void CPacketController::ProcessMessage(BYTE *data, uint32_t length, WORD group)
{
	if (m_pClient->IsAlive())
		m_pClient->ProcessMessage(data, length, group);
}

void CPacketController::IncomingBlob(BlobPacket_s *blob, double recvTime)
{
	//No CRC checks as of yet.
	BlobHeader_s *header = &blob->header;

	uint32_t dwSequence = header->dwSequence;
	uint32_t dwFlags = header->dwFlags;
	uint32_t dwCRC = header->dwCRC;
	uint32_t dwSize = header->wSize;
	BYTE* pbData = blob->data;

	m_in.receivedbytes += header->wSize + sizeof(BlobHeader_s);

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
				uint32_t expectedIntervals = (recvTime - m_in._clientIntervalBaseStart) * 2.0;
				uint32_t actualIntervals = intervalDiff;

				double rate = (double)actualIntervals / (double)expectedIntervals;

				if (rate > g_pConfig->SpeedHackKickThreshold())
				{
					SERVER_ERROR << "Possible speed hack on user:" << m_pClient->GetDescription() << "([rate:" << rate << "]) Disconnecting.";

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

	uint32_t highest = m_in.sequence;

	if (dwSequence > 0 && dwSequence < m_in.activesequence) return;

	if (!dwSequence || dwSequence == highest)
	{
		ProcessBlob(blob);
		return;
	}

	if (dwSequence > highest)
		m_in.sequence = dwSequence;

	// Cache this blob, for later use.
	if (m_in.blobstack.find(dwSequence) != m_in.blobstack.end())
	{
		// LOG(Temp, Normal, "Already had blob sequence!\n");
	}
	else
	{
		DUPEBLOB(p, blob);
		m_in.blobstack.insert(std::pair<uint32_t, BlobPacket_s *>(dwSequence, p));
	}
}

void CPacketController::ResendBlob(BlobPacket_s *blob)
{
	BlobHeader_s *header = &blob->header;

	header->dwFlags |= BT_RESENT;
	header->wTime = GetElapsedTime();

	uint32_t dwXOR = header->dwCRC;
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

	if (header->wSize == 0)
	{
		delete[] (BYTE*)blob;
		return;
	}

	header->dwSequence = GetNextSequence();
	header->dwFlags = BT_FRAGMENTS | BT_USES_CRC;
	header->dwCRC = 0;
	header->wRecID = g_pNetwork->GetServerID();
	header->wTime = GetElapsedTime();
	header->wTable = 0x01;

	uint32_t dwXOR = GetNextXORVal(m_out.crypto);
	header->dwCRC = BlobCRC(blob, dwXOR);

	//Off you go.
	g_pNetwork->QueuePacket(m_pPeer, blob, BLOBLEN(blob));

	//Cache for later use.
	header->dwCRC = dwXOR;

	m_out._blobCache.Lock();
	m_out._blobCache.insert(std::pair<uint32_t, BlobPacket_s *>(header->dwSequence, blob));
	m_out._blobCache.Unlock();

	m_out._sentBytes += header->wSize + sizeof(BlobHeader_s);
}

void CPacketController::SendCriticalError(uint32_t table, uint32_t stringId)
{
	//Bad login.
	CREATEBLOB(critErr, sizeof(uint32_t) * 2);
	((uint32_t *)critErr->data)[0] = stringId;
	((uint32_t *)critErr->data)[1] = ST_SERVER_ERRORS;

	g_pNetwork->SendConnectlessBlob(m_pPeer, critErr, BT_NETERROR, 0, 0, true);

	DELETEBLOB(critErr);

	Kill(__FILE__, __LINE__);
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
	//m_out._fragQueue.size();

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

void CPacketController::QueueNetMessage(void *data, uint32_t length, WORD group, uint32_t weenie_id, bool ephemeral)
{
	if (!IsAlive())
	{
		return;
	}

	BYTE *messageData;
	uint32_t messageLength;

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
	message.ephemeral = ephemeral;

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

		SendNetMessageInternal(queued.data, queued.length, queued.group_id, queued.ephemeral);
		delete[] queued.data;

		_outgoingMessageQueue.Lock();
		entry = _outgoingMessageQueue.begin();
	}

	_outgoingMessageQueue.Unlock();
}
