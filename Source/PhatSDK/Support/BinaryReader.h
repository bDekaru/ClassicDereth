
#pragma once

#define MAX_BINARYREADER_STRING_LENGTH 0x1000

class BinaryReader
{
public:
	BinaryReader(void *pData, uint32_t dwSize);
	~BinaryReader();

#define BOUND_CHECK(x) \
	if ( !(x) ) \
	{ \
		m_dwErrorCode = 1; \
		memset(&returnValue, 0, sizeof(returnValue)); \
		return returnValue; \
	}

	template <typename vt>
	typename std::enable_if_t<
		!(std::is_same_v<std::string, vt> || std::is_same_v<std::u16string, vt>), vt>
	Read()
	{
		vt returnValue = vt();
		if (m_pData + sizeof(vt) <= m_pEnd)
		{
			//BOUND_CHECK((m_pData + sizeof(vt)) <= m_pEnd);
			returnValue = *((vt *)m_pData);
			m_pData += sizeof(vt);
		}
		return returnValue;
	}

	template<typename vt>
	typename std::enable_if_t<std::is_same_v<std::string, vt>, vt>
	Read()
	{
		return ReadString();
	};

	template<typename vt>
	typename std::enable_if_t<std::is_same_v<std::u16string, vt>, vt>
		Read()
	{
		return ReadString16();
	};

#define STREAM_OUT(func, type) type func() { return Read<type>(); }
	
	STREAM_OUT(ReadChar, BYTE);
	STREAM_OUT(ReadShort, BYTE);
	STREAM_OUT(ReadLong, BYTE);

	STREAM_OUT(ReadBYTE, BYTE);
	//STREAM_OUT(ReadUInt16, WORD);
	//STREAM_OUT(ReadUInt32, uint32_t);
	STREAM_OUT(ReadFloat, float);
	STREAM_OUT(ReadDouble, double);

	STREAM_OUT(ReadByte, BYTE);
	STREAM_OUT(ReadInt8, char);
	STREAM_OUT(ReadUInt8, BYTE);
	STREAM_OUT(ReadInt16, short);
	STREAM_OUT(ReadUInt16, WORD);
	STREAM_OUT(ReadInt32, int);
	STREAM_OUT(ReadUInt32, uint32_t);

	STREAM_OUT(ReadSingle, float);

	uint32_t ReadPackeduint32_t()
	{
		uint32_t returnValue;
		BOUND_CHECK((m_pData + sizeof(WORD)) <= m_pEnd);
		returnValue = *((WORD *)m_pData);
		if (returnValue & 0x8000)
		{
			BOUND_CHECK((m_pData + sizeof(uint32_t)) <= m_pEnd);
			uint32_t src = *((uint32_t *)m_pData);
			returnValue = (((src & 0x3FFF) << 16) | (src >> 16));
			m_pData += sizeof(uint32_t);
		}
		else
		{
			m_pData += sizeof(WORD);
		}
		return returnValue;
	}

	uint32_t ReadCompressedUInt16()
	{
		BYTE b0 = Read<BYTE>();
		if ((b0 & 0x80) == 0)
			return (uint16_t)b0;

		BYTE b1 = Read<BYTE>();

		return (uint16_t)(((b0 & 0x7f) << 8) | b1);
	}

	uint32_t ReadCompressedUInt32()
	{
		BYTE b0 = Read<BYTE>();
		if ((b0 & 0x80) == 0)
			return (uint32_t)b0;

		BYTE b1 = Read<BYTE>();
		if ((b0 & 0x40) == 0)
			return (uint32_t)((((WORD)b0 & 0x7F) << 8) | b1);

		WORD s = Read<WORD>();
		return (uint32_t)((((((uint32_t)b0 & 0x3F) << 8) | b1) << 16) | s);
	}

	uint32_t Unpack_AsWClassIDCompressed()
	{
		uint32_t returnValue;
		BOUND_CHECK((m_pData + sizeof(WORD)) <= m_pEnd);
		returnValue = *((WORD *)m_pData);
		if (returnValue & 0x8000)
		{
			BOUND_CHECK((m_pData + sizeof(uint32_t)) <= m_pEnd);
			uint32_t src = *((uint32_t *)m_pData);
			returnValue = (((src & 0x7FFF) << 16) | (src >> 16));
			m_pData += sizeof(uint32_t);
		}
		else
		{
			m_pData += sizeof(WORD);
		}
		return returnValue;
	}

	uint32_t Unpack_AsDataIDOfKnownType(uint32_t knownType)
	{
		WORD val = Read<WORD>();

		if (val & 0x8000)
		{
			uint32_t lower = Read<WORD>();
			uint32_t higher = (val & 0x3FFF) << 16;
			return (knownType + (higher | lower));
		}

		return (knownType + val);
	}

	template<typename A, typename B> std::map<A, B> ReadMap()
	{
		std::map<A, B> table;

		WORD count = ReadUInt16();
		ReadUInt16();

		while (count > 0 && !m_dwErrorCode)
		{
			A theKey = Read<A>();
			B theValue = Read<B>();
			table.insert(std::pair<A, B>(theKey, theValue));
			count--;
		}

		return table;
	}

	template<typename A> std::map<A, std::string> ReadMap()
	{
		std::map<A, std::string> table;

		WORD count = ReadUInt16();
		ReadUInt16();

		while (count > 0 && !m_dwErrorCode)
		{
			A theKey = Read<A>();
			std::string theValue = ReadString();
			table.insert(std::pair<A, std::string>(theKey, theValue));
			count--;
		}

		return table;
	}

	void ReadAlign(void);
	void *ReadArray(size_t size);
	char *ReadString(void);
	char *ReadSerializedString(void);

	BYTE *GetDataStart(void);
	BYTE *GetDataPtr(void);
	BYTE *GetDataEnd(void);
	uint32_t GetDataLen(void);
	uint32_t GetOffset(void);
	uint32_t GetLastError(void);
	uint32_t GetDataRemaining(void);

	void SetOffset(uint32_t offset);

	const char *ReadWStringToString(void);
	std::u16string ReadString16(void);

	const char *ReadNewString();
	const wchar_t *ReadNewWString();

private:
	uint32_t m_dwErrorCode;

	BYTE *m_pData;
	BYTE *m_pStart;
	BYTE *m_pEnd;
	std::list<char *> m_stringCache;
	std::list<wchar_t *> m_wstringCache;
};

// template<>
// std::string BinaryReader::Read<std::string>()
// {
//     return ReadString();
// };
