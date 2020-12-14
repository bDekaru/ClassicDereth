
#include <StdAfx.h>
#include "BinaryWriter.h"
#include "Packable.h"

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

		unsigned int growSize = min(maxGrowSize, max(minGrowSize, (unsigned int)m_dwDataSize));
		unsigned int newSize = (uint32_t) (len + growSize);

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
	uint32_t length = (uint32_t) value.size();

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

void BinaryWriter::WriteString16(const std::u16string &value)
{
	if (!value.empty())
	{
		size_t len = value.length();
		size_t byte_len = len * sizeof(std::u16string::value_type);

		WriteCompressedUInt32(len);
		Write(value.data(), byte_len);
	}
	else
		Write<WORD>(0);
}

void BinaryWriter::Write(const void *pData, unsigned int length)
{
	ExpandBuffer(m_dwSize + length);

	memcpy(m_pbDataPos, pData, length);
	m_pbDataPos += length;
	m_dwSize += (uint32_t)length;
}

void BinaryWriter::Align()
{
	uint32_t offset = uint32_t(m_pbDataPos - m_pbData);

	if ((offset % 4) != 0)
	{
		uint32_t len = 4 - (offset % 4);

		ExpandBuffer(m_dwSize + len);

		memset(m_pbDataPos, 0, len);
		m_pbDataPos += len;
		m_dwSize += len;
	}
}

void BinaryWriter::Write(PackObj* packable)
{
	packable->Pack(this);
}


BYTE* BinaryWriter::GetData(void)
{
	return m_pbData;
}

uint32_t BinaryWriter::GetSize(void)
{
	return m_dwSize;
}

void BinaryWriter::AppendWStringFromString(const std::string &str)
{
	if (!str.empty())
	{
		// the codecvt bits are broken in vs 2015/2017
		// it is supposed to be fixed in 2019-16.2 when it is released
#if _MSC_VER >= 1900 && _MSC_VER < 1925
		wchar_t *wideCharStr = new wchar_t[str.size()];			
		uint32_t len = (uint32_t) mbstowcs(wideCharStr, str.c_str(), str.size());
		WriteCompressedUInt32(len);
		Write(wideCharStr, len * sizeof(wchar_t));
		delete [] wideCharStr;
#else
		std::u16string s16 = string16_convert_t{}.from_bytes(str);

		size_t len = s16.length();
		size_t byte_len = len * sizeof(std::u16string::value_type);

		WriteCompressedUInt32(len);
		Write(s16.data(), byte_len);
#endif
	}
	else
		Write<WORD>(0);
}

void BinaryWriter::WriteNewString(const std::string &value)
{
	uint32_t len = (uint32_t) value.length();
	WriteCompressedUInt32(len);
	Write(value.c_str(), len);
}

void BinaryWriter::WriteNewWString(const std::wstring &value)
{
	uint32_t len = (uint32_t)value.length();
	WriteCompressedUInt32(len);
	Write(value.c_str(), len);
}

