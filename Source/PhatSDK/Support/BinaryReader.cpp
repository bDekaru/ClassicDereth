
#include <StdAfx.h>
#include "BinaryReader.h"

BinaryReader::BinaryReader(void *pData, uint32_t dwSize)
{
	m_dwErrorCode = 0;

	m_pData = (BYTE *)pData;
	m_pStart = m_pData;
	m_pEnd = m_pStart + dwSize;
}

BinaryReader::~BinaryReader()
{
	for (auto entry : m_stringCache)
		delete [] entry;
	m_stringCache.clear();

	for (auto entry : m_wstringCache)
		delete[] entry;
	m_wstringCache.clear();
}

void BinaryReader::ReadAlign()
{
	uint32_t dwOffset = (uint32_t)(m_pData - m_pStart);
	if ((dwOffset % 4) != 0)
		m_pData += (4 - (dwOffset % 4));
}

void *BinaryReader::ReadArray(size_t size)
{
	static BYTE dummyData[100000];

	void *retval = m_pData;
	m_pData += size;

	if (m_pData > m_pEnd)
	{
		m_dwErrorCode = 2;
		return dummyData;
	}

	return retval;
}

char *BinaryReader::ReadString()
{
	WORD wLen = ReadUInt16();

	if (m_dwErrorCode || wLen > MAX_BINARYREADER_STRING_LENGTH)
	{
		m_dwErrorCode = 1;
		return "";
	}

	char *szArray = (char *)ReadArray(wLen);
	if (!szArray)
	{
		m_dwErrorCode = 1;
		return "";
	}

	char *szString = new char[wLen + 1];
	szString[wLen] = 0;
	memcpy(szString, szArray, wLen);
	m_stringCache.push_back(szString);

	ReadAlign();

	return szString;
}

char *BinaryReader::ReadSerializedString()
{
	uint32_t length = ReadCompressedUInt32();

	if (m_dwErrorCode || length > MAX_BINARYREADER_STRING_LENGTH)
	{
		// DEBUG_BREAK();
		m_dwErrorCode = 1;
		return "";
	}

	char *szArray = (char *)ReadArray(length);
	if (!szArray)
	{
		// DEBUG_BREAK();
		m_dwErrorCode = 1;
		return "";
	}

	char *szString = new char[length + 1];
	szString[length] = 0;
	memcpy(szString, szArray, length);
	m_stringCache.push_back(szString);

	return szString;
}

BYTE *BinaryReader::GetDataStart()
{
	return m_pStart;
}

BYTE *BinaryReader::GetDataPtr()
{
	return m_pData;
}

BYTE *BinaryReader::GetDataEnd()
{
	return m_pEnd;
}

uint32_t BinaryReader::GetDataLen()
{
	return (uint32_t)(m_pEnd - m_pStart);
}

uint32_t BinaryReader::GetOffset()
{
	return (uint32_t)(m_pData - m_pStart);
}

void BinaryReader::SetOffset(uint32_t offset)
{
	m_pData = m_pStart + offset;
}

uint32_t BinaryReader::GetLastError()
{
	return m_dwErrorCode;
}

uint32_t BinaryReader::GetDataRemaining()
{
	return m_pEnd - m_pData;
}

const char *BinaryReader::ReadWStringToString()
{
	unsigned int charLength = ReadCompressedUInt32();

	if (m_dwErrorCode || charLength > MAX_BINARYREADER_STRING_LENGTH)
	{
		m_dwErrorCode = 1;
		return "";
	}

	char16_t *wStr = (char16_t *)ReadArray(charLength * sizeof(char16_t));
	if (m_dwErrorCode)
		return "";

	char *mbStr = new char[charLength + 1];

	// the codecvt bits are broken in vs 2015/2017
	// it is supposed to be fixed in 2019-16.2 when it is released
#if _MSC_VER >= 1900 && _MSC_VER < 1925
	wcstombs(mbStr, (wchar_t*)wStr, charLength);
#else
	std::u16string s16(wStr);
	std::string s8 = string16_convert_t{}.to_bytes(s16);

	s8.copy(mbStr, s8.length());
#endif

	mbStr[charLength] = 0;
	m_stringCache.push_back(mbStr);
	return mbStr;
}

std::u16string BinaryReader::ReadString16(void)
{
	unsigned int charLength = ReadCompressedUInt32();

	if (m_dwErrorCode || charLength > MAX_BINARYREADER_STRING_LENGTH)
	{
		m_dwErrorCode = 1;
		return u"";
	}

	char16_t *wStr = (char16_t *)ReadArray(charLength * sizeof(char16_t));
	if (m_dwErrorCode)
		return u"";

	std::u16string s16(wStr, charLength);
	return s16;
}

const char *BinaryReader::ReadNewString()
{
	unsigned int charLength = ReadCompressedUInt32();

	if (m_dwErrorCode || charLength > MAX_BINARYREADER_STRING_LENGTH)
	{
		m_dwErrorCode = 1;
		return "";
	}

	char *str = (char *)ReadArray(charLength * sizeof(char));
	if (m_dwErrorCode)
		return "";

	char *buffer = new char[charLength + 1];
	buffer[charLength] = 0;
	memcpy(buffer, str, sizeof(char *) * charLength);
	m_stringCache.push_back(buffer);

	return buffer;
}

const wchar_t *BinaryReader::ReadNewWString()
{
	unsigned int charLength = ReadCompressedUInt32();

	if (m_dwErrorCode || charLength > MAX_BINARYREADER_STRING_LENGTH)
	{
		m_dwErrorCode = 1;
		return L"";
	}

	wchar_t *str = (wchar_t *)ReadArray(charLength * sizeof(wchar_t));
	if (m_dwErrorCode)
		return L"";

	wchar_t *buffer = new wchar_t[charLength + 1];
	buffer[charLength] = 0;
	memcpy(buffer, str, sizeof(wchar_t *) * charLength);
	m_wstringCache.push_back(buffer);

	return buffer;
}

