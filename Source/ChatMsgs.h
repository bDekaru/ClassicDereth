
#pragma once

class BinaryWriter;
class CWeenieObject;
class CPlayerWeenie;

extern BinaryWriter *ChannelChat(uint32_t dwChannel, const char *name, const char *text);
extern BinaryWriter *LocalChat(const char *text, const char *name, uint32_t sourceID, LogTextType ltt = LTT_SPEECH);
extern BinaryWriter *ActionChat(const char *text, const char *name, uint32_t sourceID);
extern BinaryWriter *EmoteChat(const char *text, const char *name, uint32_t sourceID);
extern BinaryWriter *ServerText(const char *text, int32_t lColor = 0);
extern BinaryWriter *OverlayText(const char *szText);
extern BinaryWriter *ServerBroadcast(const char *szSource, const char *text, int32_t lColor = 0);
extern BinaryWriter *DirectChat(const char* text, const char* name, uint32_t sourceID, uint32_t dwDestID, int32_t lColor);

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

extern BinaryWriter *TurbineChat(uint32_t dwChanId, uint32_t sender, std::string senderName, std::string message);
extern BinaryWriter *SetTurbineChatChannels(uint32_t dwSourceID);
extern BinaryWriter *TurbineInboundAck(uint32_t serial);

extern std::string FilterBadChatCharacters(const char *str);
extern std::u16string FilterBadChatCharacters(const std::u16string &str);
