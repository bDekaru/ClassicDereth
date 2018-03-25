
#pragma once

class BinaryWriter;
class CWeenieObject;
class CPlayerWeenie;

extern BinaryWriter *ChannelChat(DWORD dwChannel, const char *name, const char *text);
extern BinaryWriter *LocalChat(const char *text, const char *name, DWORD sourceID, LogTextType ltt = LTT_SPEECH);
extern BinaryWriter *ActionChat(const char *text, const char *name, DWORD sourceID);
extern BinaryWriter *EmoteChat(const char *text, const char *name, DWORD sourceID);
extern BinaryWriter *ServerText(const char *text, long lColor = 0);
extern BinaryWriter *ServerBroadcast(const char *szSource, const char *text, long lColor = 0);
extern BinaryWriter *DirectChat(const char* text, const char* name, DWORD sourceID, DWORD dwDestID, long lColor);

enum ChatChannel {
	Allegiance_ChatChannel = 1,
	General_ChatChannel,
	Trade_ChatChannel,
	LFG_ChatChannel,
	Roleplay_ChatChannel,
	Olthoi_ChatChannel,
	Society_ChatChannel,
	SocietyCelHan_ChatChannel,
	SocietyEldWeb_ChatChannel,
	SocietyRadBlo_ChatChannel //,
	// Count_ChatChannels
};

extern BinaryWriter *TurbineChat(DWORD dwChanId, DWORD sender, std::string senderName, std::string message);
extern BinaryWriter *SetTurbineChatChannels(DWORD dwSourceID);
extern BinaryWriter *TurbineInboundAck(DWORD serial);

extern std::string FilterBadChatCharacters(const char *str);