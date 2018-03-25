
#pragma once

#include "Json.h"

class PackableJson
{
public:
	PackableJson() { }
	virtual ~PackableJson() { }

	virtual void PackJson(json& writer) { }
	virtual bool UnPackJson(const json& reader) { return true; }

	std::string GetJsonSafeText(const char *text, bool allowNewLines = true)
	{
		std::string filteredText;

		const char *ptr = text;
		while (*ptr)
		{
			char c = *ptr;
			if (c > 0x7F || (c < 0x20 && (!allowNewLines || (c != '\r' && c != '\n' && c == '\t'))))
			{
				if (c == 0x92)
					c = '\'';
				else
					c = ' ';
			}
			else
			{
				if (c == '\'')
					c = ' '; // causes trouble
			}
			
			filteredText += c;

			ptr++;
		}

		return filteredText;
	}
};


#define DECLARE_PACKABLE_JSON() \
	virtual void PackJson(json& writer) override; \
	virtual bool UnPackJson(const json& reader) override;
#define DEFINE_PACK_JSON(className) \
	void className::PackJson(json& writer)
#define DEFINE_UNPACK_JSON(className) \
	bool className::UnPackJson(const json& reader)
#define DEFINE_LOCAL_PACK_JSON() \
	void PackJson(json& writer) override
#define DEFINE_LOCAL_UNPACK_JSON() \
	bool UnPackJson(const json& reader) override
