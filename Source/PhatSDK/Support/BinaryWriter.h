
#pragma once

#include "Packable.h"

class BinaryWriter
{
public:
	BinaryWriter();
	~BinaryWriter();

	void ExpandBuffer(size_t length);
	
	void WriteString(const std::string &value);
	void WriteString(const char *value);
	void Write(const void *data, unsigned int length);
	void Align();

	template <typename ValueType, typename InputType>
	void Write(InputType inputType)
	{
		ValueType valueType = (ValueType)inputType;
		Write(&valueType, sizeof(ValueType));
	}

	template <>
	void Write<std::string>(std::string inputType)
	{
		WriteString(inputType.c_str());
	}

	void Write(PackObj *packable) {
		packable->Pack(this);
	}

	void Write(BinaryWriter *otherWriter)
	{
		Write(otherWriter->GetData(), otherWriter->GetSize());
	}

	void WritePackedDWORD(DWORD value)
	{
		if (value < 0x8000)
			Write<WORD>(value);
		else
			Write<DWORD>((value << 16) | ((value >> 16) | 0x8000));
	}

	void WriteCompressedUInt32(DWORD v)
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

	void Pack_AsWClassIDCompressed(DWORD value)
	{
		if (value < 0x8000)
			Write<WORD>(value);
		else
			Write<DWORD>((value << 16) | ((value >> 16) | 0x8000));
	}

	void Pack_AsDataIDOfKnownType(DWORD i_FirstID, DWORD i_toPack)
	{
		DWORD dataID = i_toPack ? i_toPack : i_FirstID;
		DWORD offset = dataID - i_FirstID;

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
		for (std::map<A, B>::iterator keyValuePair = table.begin(); keyValuePair != table.end(); keyValuePair++)
		{
			AppendData((A)keyValuePair->first);
			AppendData((B)keyValuePair->second);
		}
	}

	template<typename A> void WriteMap(std::map<A, std::string> &table, unsigned int bucketSize = 0x40)
	{
		Write<WORD>((WORD) table.size());
		Write<WORD>(bucketSize);
		for (std::map<A, std::string>::iterator keyValuePair = table.begin(); keyValuePair != table.end(); keyValuePair++)
		{
			AppendData((A)keyValuePair->first);
			AppendString(keyValuePair->second.c_str());
		}
	}

	void AppendWStringFromString(const std::string &str);

	void WriteNewString(const std::string &value);
	void WriteNewWString(const std::wstring &value);

	BYTE *GetData();
	DWORD GetSize();

protected:
	// BYTE m_pbData[0x800];

	BYTE m_LocalBuffer[512];

	BYTE *m_pbData;
	DWORD m_dwDataSize;
	BYTE *m_pbDataPos;
	DWORD m_dwSize;
};
