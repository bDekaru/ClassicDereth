
#include "StdAfx.h"
#include "BinaryWriter.h"

// Expandable buffer with AC style data IO

BinaryWriter::BinaryWriter()
{
	m_pbData = m_LocalBuffer; // new BYTE[0x200];
	m_dwDataSize = sizeof(m_LocalBuffer); // 0x200;

	m_pbDataPos = m_pbData;
	m_dwSize = 0;
}

BinaryWriter::~BinaryWriter()
{
	if (m_pbData)
	{
		if (m_pbData != m_LocalBuffer) {
			delete[] m_pbData;
		}

		m_pbData = NULL;
	}
}

void BinaryWriter::ExpandBuffer(size_t len)
{
	if (m_dwDataSize < len)
	{
		unsigned int minGrowSize = 0x200;
		unsigned int maxGrowSize = 0x100000;

		unsigned int growSize = min(maxGrowSize, max(minGrowSize, m_dwDataSize));
		unsigned int newSize = (DWORD) (len + growSize);

		BYTE *pbExpandedBuffer = new BYTE[newSize];
		memcpy(pbExpandedBuffer, m_pbData, m_dwSize);

		if (m_pbData != m_LocalBuffer) {
			delete[] m_pbData;
		}

		m_pbData = pbExpandedBuffer;
		m_dwDataSize = newSize;
		m_pbDataPos = m_pbData + m_dwSize;
	}
}

void BinaryWriter::WriteString(const std::string &value)
{
	DWORD length = (DWORD) value.size();

	// TODO handle larger strings correctly
	if (length)
	{
		Write<WORD>(length);
		Write(value.c_str(), length);
	}
	else
	{
		Write<WORD>(0);
	}

	Align();
}

void BinaryWriter::WriteString(const char *value)
{
	// TODO handle larger strings correctly
	if (value)
	{
		WORD length = (WORD)strlen(value);
		Write<WORD>(length);
		Write(value, length);
	}
	else
	{
		Write<WORD>(0);
	}

	Align();
}

void BinaryWriter::Write(const void *pData, unsigned int length)
{
	ExpandBuffer(m_dwSize + length);

	memcpy(m_pbDataPos, pData, length);
	m_pbDataPos += length;
	m_dwSize += (DWORD)length;
}

void BinaryWriter::Align()
{
	DWORD offset = DWORD(m_pbDataPos - m_pbData);

	if ((offset % 4) != 0)
	{
		DWORD len = 4 - (offset % 4);

		ExpandBuffer(m_dwSize + len);

		memset(m_pbDataPos, 0, len);
		m_pbDataPos += len;
		m_dwSize += len;
	}
}

BYTE* BinaryWriter::GetData(void)
{
	return m_pbData;
}

DWORD BinaryWriter::GetSize(void)
{
	return m_dwSize;
}

void BinaryWriter::AppendWStringFromString(const std::string &str)
{
	if (!str.empty())
	{
		wchar_t *wideCharStr = new wchar_t[str.size()];			
		DWORD len = (DWORD) mbstowcs(wideCharStr, str.c_str(), str.size());
		WriteCompressedUInt32(len);
		Write(wideCharStr, len * sizeof(wchar_t));
		delete [] wideCharStr;
	}
	else
		Write<WORD>(0);
}

void BinaryWriter::WriteNewString(const std::string &value)
{
	DWORD len = (DWORD) value.length();
	WriteCompressedUInt32(len);
	Write(value.c_str(), len);
}

void BinaryWriter::WriteNewWString(const std::wstring &value)
{
	DWORD len = (DWORD)value.length();
	WriteCompressedUInt32(len);
	Write(value.c_str(), len);
}

