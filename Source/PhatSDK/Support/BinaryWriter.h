
#pragma once

class PackObj;

class BinaryWriter
{
public:
	BinaryWriter();
	~BinaryWriter();

	void ExpandBuffer(size_t length);
	
	void WriteString(const std::string &value);
	void WriteString(const char *value);
	void WriteString(const std::u16string &value) { WriteString16(value); }
	void WriteString16(const std::u16string &value);
	void Write(const void *data, unsigned int length);
	void Align();

	template <typename ValueType, typename InputType>
	typename std::enable_if_t<!(std::is_same_v<std::string, InputType> || std::is_same_v<std::u16string, InputType>)>
	Write(InputType inputType)
	{
		ValueType valueType = (ValueType)inputType;
		Write(&valueType, sizeof(ValueType));
	}

	template <typename ValueType, typename InputType>
	typename std::enable_if_t<std::is_same_v<std::string, InputType>>
	Write(InputType inputType)
	{
		WriteString(inputType);
	}

	template <typename ValueType, typename InputType>
	typename std::enable_if_t<std::is_same_v<std::u16string, InputType>>
		Write(InputType inputType)
	{
		WriteString16(inputType);
	}

	void Write(PackObj* packable);

	void Write(BinaryWriter *otherWriter)
	{
		Write(otherWriter->GetData(), otherWriter->GetSize());
	}

	void WritePackedUInt32(uint32_t value)
	{
		if (value < 0x8000)
			Write<WORD>(value);
		else
			Write<uint32_t>((value << 16) | ((value >> 16) | 0x8000));
	}

	void WriteCompressedUInt32(uint32_t v)
	{
		BYTE *b = (BYTE *)&v;

		if (v > 0x7F)
		{
			if (v > 0x3FFF)
			{
				Write<BYTE>((BYTE)(b[3] | 0xC0));
				Write<BYTE>(b[2]);
				Write<BYTE>(b[0]);
				Write<BYTE>(b[1]);
			}
			else
			{
				Write<BYTE>((BYTE)(b[1] | 0x80));
				Write<BYTE>(b[0]);
			}
		}
		else
			Write<BYTE>(b[0]);
	}

	void Pack_AsWClassIDCompressed(uint32_t value)
	{
		if (value < 0x8000)
			Write<WORD>(value);
		else
			Write<uint32_t>((value << 16) | ((value >> 16) | 0x8000));
	}

	void Pack_AsDataIDOfKnownType(uint32_t i_FirstID, uint32_t i_toPack)
	{
		uint32_t dataID = i_toPack ? i_toPack : i_FirstID;
		uint32_t offset = dataID - i_FirstID;

		if (offset > 0x3FFF)
		{
			if (offset > 0x3FFFFFFF)
			{
				// this is bad!
			}
			else
			{
				WORD low = HIWORD(offset) | 0x8000;
				Write<WORD>(low);

				WORD high = LOWORD(offset);
				Write<WORD>(high);
			}
		}
		else
		{
			Write<WORD>(offset);
		}
	}

	template<typename A, typename B> void WriteMap(std::map<A, B> &table, unsigned int bucketSize = 0x40)
	{
		Write<WORD>((WORD) table.size());
		Write<WORD>(bucketSize);
		for (typename std::map<A, B>::iterator keyValuePair = table.begin(); keyValuePair != table.end(); keyValuePair++)
		{
			AppendData((A)keyValuePair->first);
			AppendData((B)keyValuePair->second);
		}
	}

	template<typename A> void WriteMap(std::map<A, std::string> &table, unsigned int bucketSize = 0x40)
	{
		Write<WORD>((WORD) table.size());
		Write<WORD>(bucketSize);
		for (typename std::map<A, std::string>::iterator keyValuePair = table.begin(); keyValuePair != table.end(); keyValuePair++)
		{
			AppendData((A)keyValuePair->first);
			AppendString(keyValuePair->second.c_str());
		}
	}

	void AppendWStringFromString(const std::string &str);

	void WriteNewString(const std::string &value);
	void WriteNewWString(const std::wstring &value);

	BYTE *GetData();
	uint32_t GetSize();

protected:
	// BYTE m_pbData[0x800];

	BYTE m_LocalBuffer[512];

	BYTE *m_pbData;
	uint32_t m_dwDataSize;
	BYTE *m_pbDataPos;
	uint32_t m_dwSize;
};
