
#include <StdAfx.h>
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"

//Network access.
#include "Client.h"
#include "BinaryWriter.h"
#include "ChatMsgs.h"

#define MESSAGE_BEGIN(x) BinaryWriter *x = new BinaryWriter
#define MESSAGE_END(x) return x

BinaryWriter *LocalChat(const char *text, const char *name, uint32_t sourceID, LogTextType ltt)
{
	MESSAGE_BEGIN(LocalChat);

	LocalChat->Write<uint32_t>(0x02BB);
	LocalChat->WriteString(text);
	LocalChat->WriteString(name);
	LocalChat->Write<uint32_t>(sourceID);
	LocalChat->Write<int>(ltt);

	MESSAGE_END(LocalChat);
}

BinaryWriter *EmoteChat(const char* szText, const char* szName, uint32_t dwSourceID)
{
	MESSAGE_BEGIN(EmoteChat);

	EmoteChat->Write<uint32_t>(0x1E0);
	EmoteChat->Write<uint32_t>(dwSourceID);
	EmoteChat->WriteString(szName);
	EmoteChat->WriteString(szText);

	MESSAGE_END(EmoteChat);
}

//0x5719F5DB

BinaryWriter *DirectChat(const char* szText, const char* szName, uint32_t dwSourceID, uint32_t dwDestID, int32_t lColor)
{
	//Fake-tells: bit 10 must be set.
	//Envoy: bit 21 must be set.

	//Example:
	//Step 1. Take a uint32_t of any value. (entirely random if you wish, it doesn't matter.)
	//Step 2. Remove bits 10 and 21 from that uint32_t.
	//Step 3. If FAKE-tell: set bit 10 (other don't.) These tells will never be drawn to the chat.
	//Step 4. If Envoy: set bit 21 (otherwise don't.) These tells will be prefixed with '+Envoy'.
	//Step 5. Now take the uint32_t and XOR it against the source player's GUID.
	//The final result will be the magic number.

#define RANDOM_LONG() ((rand() << 30) | (rand()<<15) | rand()) // RAND_MAX=7fff

	BOOL bFakeTell = FALSE;
	BOOL bEnvoy = FALSE;

	uint32_t dwMagicValue;
	dwMagicValue = RANDOM_LONG();				//Step 1
	dwMagicValue &= ~((1 << 10) | (1 << 21));	//Step 2

	if (bFakeTell)							//Step 3
		dwMagicValue |= 1 << 10;
	if (bEnvoy)								//Step 4
		dwMagicValue |= 1 << 21;

	dwMagicValue ^= dwSourceID;					//Step 5

	MESSAGE_BEGIN(DirectChat);
	DirectChat->Write<uint32_t>(0x2BD);
	DirectChat->WriteString(szText);
	DirectChat->WriteString(szName);
	DirectChat->Write<uint32_t>(dwSourceID);
	DirectChat->Write<uint32_t>(dwDestID);
	DirectChat->Write<int32_t>(lColor);
	DirectChat->Write<uint32_t>(dwMagicValue);
	MESSAGE_END(DirectChat);
}

BinaryWriter *ActionChat(const char* szText, const char* szName, uint32_t dwSourceID)
{
	MESSAGE_BEGIN(EmoteChat);

	EmoteChat->Write<uint32_t>(0x1E2);
	EmoteChat->Write<uint32_t>(dwSourceID);
	EmoteChat->WriteString(szName);
	EmoteChat->WriteString(szText);

	MESSAGE_END(EmoteChat);
}

BinaryWriter *ServerText(const char *szText, int32_t lColor)
{
	MESSAGE_BEGIN(ServerText);

	ServerText->Write<uint32_t>(0xF7E0);
	ServerText->WriteString(szText);
	ServerText->Write<int32_t>(lColor);

	MESSAGE_END(ServerText);
}

BinaryWriter *OverlayText(const char *szText)
{
	MESSAGE_BEGIN(OverlayText);

	OverlayText->Write<uint32_t>(0x02EB);
	OverlayText->WriteString(szText);

	MESSAGE_END(OverlayText);
}

BinaryWriter *ServerBroadcast(const char *szSource, const char *szText, int32_t lColor)
{
	// Using string class to prevent from static buffer collisions.
	std::string strBroadcast;
	strBroadcast = "Broadcast from ";
	strBroadcast += szSource;
	strBroadcast += "> ";
	strBroadcast += szText;

	MESSAGE_END(ServerText(strBroadcast.c_str(), lColor));
}

BinaryWriter *ChannelChat(uint32_t dwChannel, const char* szName, const char* szText)
{
	MESSAGE_BEGIN(ChannelChat);

	ChannelChat->Write<uint32_t>(0x147);
	ChannelChat->Write<uint32_t>(dwChannel);
	ChannelChat->WriteString(szName);
	ChannelChat->WriteString(szText);

	MESSAGE_END(ChannelChat);
}

BinaryWriter *TurbineInboundAck(uint32_t serial)
{
	MESSAGE_BEGIN(msg);
	msg->Write<uint32_t>(0xF7DE);
	msg->Write<uint32_t>(0x30); // Size
	msg->Write<uint32_t>(0x05); // Type 05 - ACK
	msg->Write<uint32_t>(0x02); // ??
	msg->Write<uint32_t>(0x01); // ??
	msg->Write<uint32_t>(0xB0045); // bitfield?
	msg->Write<uint32_t>(0x01);
	msg->Write<uint32_t>(0xB0045); // bitfield?
	msg->Write<uint32_t>(0x00);
	msg->Write<uint32_t>(0x10);   // Payload size
	msg->Write<uint32_t>(serial); // serial
	msg->Write<uint32_t>(0x02);
	msg->Write<uint32_t>(0x02);
	msg->Write<uint32_t>(0x00);
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

extern std::u16string FilterBadChatCharacters(const std::u16string &str)
{
	std::u16string newMessage;

	std::u16string::const_iterator ptr = str.begin();
	while (ptr != str.end())
	{
		const char16_t c = *ptr;
		if (c < 0x20 || c > 0x7F)
		{
			newMessage += u" ";
		}
		else
		{
			newMessage += c;
		}
		ptr++;
	}

	return newMessage;
}
