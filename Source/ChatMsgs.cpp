
#include "StdAfx.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"

//Network access.
#include "Client.h"
#include "BinaryWriter.h"
#include "ChatMsgs.h"

#define MESSAGE_BEGIN(x) BinaryWriter *x = new BinaryWriter
#define MESSAGE_END(x) return x

BinaryWriter *LocalChat(const char *text, const char *name, DWORD sourceID, LogTextType ltt)
{
	MESSAGE_BEGIN(LocalChat);

	LocalChat->Write<DWORD>(0x02BB);
	LocalChat->WriteString(text);
	LocalChat->WriteString(name);
	LocalChat->Write<DWORD>(sourceID);
	LocalChat->Write<int>(ltt);

	MESSAGE_END(LocalChat);
}

BinaryWriter *EmoteChat(const char* szText, const char* szName, DWORD dwSourceID)
{
	MESSAGE_BEGIN(EmoteChat);

	EmoteChat->Write<DWORD>(0x1E0);
	EmoteChat->Write<DWORD>(dwSourceID);
	EmoteChat->WriteString(szName);
	EmoteChat->WriteString(szText);

	MESSAGE_END(EmoteChat);
}

//0x5719F5DB

BinaryWriter *DirectChat(const char* szText, const char* szName, DWORD dwSourceID, DWORD dwDestID, long lColor)
{
	//Fake-tells: bit 10 must be set.
	//Envoy: bit 21 must be set.

	//Example:
	//Step 1. Take a DWORD of any value. (entirely random if you wish, it doesn't matter.)
	//Step 2. Remove bits 10 and 21 from that DWORD.
	//Step 3. If FAKE-tell: set bit 10 (other don't.) These tells will never be drawn to the chat.
	//Step 4. If Envoy: set bit 21 (otherwise don't.) These tells will be prefixed with '+Envoy'.
	//Step 5. Now take the DWORD and XOR it against the source player's GUID.
	//The final result will be the magic number.

#define RANDOM_LONG() ((rand() << 30) | (rand()<<15) | rand()) // RAND_MAX=7fff

	BOOL bFakeTell = FALSE;
	BOOL bEnvoy = FALSE;

	DWORD dwMagicValue;
	dwMagicValue = RANDOM_LONG();				//Step 1
	dwMagicValue &= ~((1 << 10) | (1 << 21));	//Step 2

	if (bFakeTell)							//Step 3
		dwMagicValue |= 1 << 10;
	if (bEnvoy)								//Step 4
		dwMagicValue |= 1 << 21;

	dwMagicValue ^= dwSourceID;					//Step 5

	MESSAGE_BEGIN(DirectChat);
	DirectChat->Write<DWORD>(0x2BD);
	DirectChat->WriteString(szText);
	DirectChat->WriteString(szName);
	DirectChat->Write<DWORD>(dwSourceID);
	DirectChat->Write<DWORD>(dwDestID);
	DirectChat->Write<long>(lColor);
	DirectChat->Write<DWORD>(dwMagicValue);
	MESSAGE_END(DirectChat);
}

BinaryWriter *ActionChat(const char* szText, const char* szName, DWORD dwSourceID)
{
	MESSAGE_BEGIN(EmoteChat);

	EmoteChat->Write<DWORD>(0x1E2);
	EmoteChat->Write<DWORD>(dwSourceID);
	EmoteChat->WriteString(szName);
	EmoteChat->WriteString(szText);

	MESSAGE_END(EmoteChat);
}

BinaryWriter *ServerText(const char *szText, long lColor)
{
	MESSAGE_BEGIN(ServerText);

	ServerText->Write<DWORD>(0xF7E0);
	ServerText->WriteString(szText);
	ServerText->Write<long>(lColor);

	MESSAGE_END(ServerText);
}

BinaryWriter *ServerBroadcast(const char *szSource, const char *szText, long lColor)
{
	// Using string class to prevent from static buffer collisions.
	std::string strBroadcast;
	strBroadcast = "Broadcast from ";
	strBroadcast += szSource;
	strBroadcast += "> ";
	strBroadcast += szText;

	MESSAGE_END(ServerText(strBroadcast.c_str(), lColor));
}

BinaryWriter *ChannelChat(DWORD dwChannel, const char* szName, const char* szText)
{
	MESSAGE_BEGIN(ChannelChat);

	ChannelChat->Write<DWORD>(0x14A);
	ChannelChat->Write<DWORD>(dwChannel);
	ChannelChat->WriteString(szName);
	ChannelChat->WriteString(szText);

	MESSAGE_END(ChannelChat);
}

BinaryWriter *TurbineInboundAck(DWORD serial)
{
	MESSAGE_BEGIN(msg);
	msg->Write<DWORD>(0xF7DE);
	msg->Write<DWORD>(0x30); // Size
	msg->Write<DWORD>(0x05); // Type 05 - ACK
	msg->Write<DWORD>(0x02); // ??
	msg->Write<DWORD>(0x01); // ??
	msg->Write<DWORD>(0xB0045); // bitfield?
	msg->Write<DWORD>(0x01);
	msg->Write<DWORD>(0xB0045); // bitfield?
	msg->Write<DWORD>(0x00);
	msg->Write<DWORD>(0x10);   // Payload size
	msg->Write<DWORD>(serial); // serial
	msg->Write<DWORD>(0x02);
	msg->Write<DWORD>(0x02);
	msg->Write<DWORD>(0x00);
	MESSAGE_END(msg);
}

std::string FilterBadChatCharacters(const char *str)
{
	std::string newMessage;

	const char *ptr = str;
	while (*ptr)
	{
		char c = *ptr;
		if (c < 0x20 || c > 0x7F)
		{
			newMessage += " ";
		}
		else
		{
			newMessage += c;
		}
		ptr++;
	}

	return newMessage;
}
