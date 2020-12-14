
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
			unsigned char c = *ptr;
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

protected:

	template<typename value_type>
	bool UnPackValue(const json& reader, std::string name, value_type &value)
	{
		json::const_iterator itr = reader.end();
		json::const_iterator end = reader.end();

		itr = reader.find(name);
		if (itr != end)
		{
			value = itr->get<value_type>();
			return true;
		}
		return false;
	}

	template<typename value_type>
	bool UnPackValue(const json& reader, std::string name, value_type &value, const value_type default_value)
	{
		bool result = UnPackValue(reader, name, value);
		if (!result) {
			value = default_value;
		}
		return result;
	}

	virtual bool UnPackObjJson(const json& reader, std::string name, PackableJson &obj)
	{
		//if (reader.is_null())
		//	return true;

		json::const_iterator itr = reader.end();
		json::const_iterator end = reader.end();

		itr = reader.find(name);
		if (itr != end)
		{
			obj.UnPackJson(*itr);
			return true;
		}
		return false;
	}

	virtual void PackObjJson(json& writer, std::string name, PackableJson &obj)
	{
		json tmp;
		obj.PackJson(tmp);
		writer[name] = tmp;
	}

	template<typename container_type>
	bool UnPackObjList(const json& reader, std::string name, std::back_insert_iterator<container_type> outItr)
	{
		json::const_iterator end = reader.end();
		json::const_iterator itr = reader.find(name);

		if (itr != end && itr->is_array())
		{
			for (auto iitr = itr->begin(); iitr != itr->end(); iitr++)
			{
				typename container_type::value_type value;
				value.UnPackJson(*iitr);
				outItr = value;
			}

			return true;
		}

		return false;
	}

	template<typename value_type>
	bool UnPackObjArrayJson(const json& reader, std::string name, value_type obj[], int &length)
	{
		json::const_iterator end = reader.end();
		json::const_iterator itr = reader.find(name);
		length = 0;

		if (itr != end && itr->is_array())
		{
			for (auto iitr = itr->begin(); iitr != itr->end(); iitr++)
				obj[length++].UnPackJson(*iitr);

			return true;
		}

		return false;
	}

	template<typename itr_type>
	void PackObjArrayJson(json& writer, std::string name, itr_type begin, itr_type end)
	{
		json items;
		for (itr_type i = begin; i != end; i++)
		{
			json tmp;
			i->PackJson(tmp);
			items.push_back(tmp);
		}
		writer[name] = items;
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
