
#include "StdAfx.h"
#include "BinaryReader.h"

BinaryReader::BinaryReader(void *pData, DWORD dwSize)
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
	DWORD dwOffset = (DWORD)(m_pData - m_pStart);
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
	WORD wLen = ReadWORD();

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
	DWORD length = ReadCompressedUInt32();

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

DWORD BinaryReader::GetDataLen()
{
	return (DWORD)(m_pEnd - m_pStart);
}

DWORD BinaryReader::GetOffset()
{
	return (DWORD)(m_pData - m_pStart);
}

void BinaryReader::SetOffset(DWORD offset)
{
	m_pData = m_pStart + offset;
}

DWORD BinaryReader::GetLastError()
{
	return m_dwErrorCode;
}

DWORD BinaryReader::GetDataRemaining()
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

	wchar_t *wStr = (wchar_t *)ReadArray(charLength * sizeof(wchar_t));
	if (m_dwErrorCode)
		return "";

	char *mbStr = new char[charLength + 1];
	wcstombs(mbStr, wStr, charLength);
	mbStr[charLength] = 0;
	m_stringCache.push_back(mbStr);
	return mbStr;
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

