
#include "StdAfx.h"
#include "FragStack.h"

FragmentStack::FragmentStack(FragPacket_s *frag)
{
	FragHeader_s *header = &frag->header;
	_id = header->id;
	m_wGroup = header->wGroup;
	m_wCount = header->wCount;

	m_pbData = new BYTE[m_wCount * 0x1C0];
	m_pbReceived = new bool[m_wCount];

	memset(m_pbReceived, 0, sizeof(bool) * m_wCount);
	m_wSize = 0;

	AddFragment(frag);
}

FragmentStack::~FragmentStack()
{
	SafeDeleteArray(m_pbData);
	SafeDeleteArray(m_pbReceived);
}

void FragmentStack::AddFragment(FragPacket_s *frag)
{
	BYTE* data = frag->data;
	DWORD datalen = frag->header.wSize - sizeof(FragHeader_s);

	WORD index = frag->header.wIndex;
	WORD count = frag->header.wCount;

	memcpy(&m_pbData[index * 0x1C0], data, datalen);
	m_pbReceived[index] = true;

	if (index == (count - 1))
	{
		m_wSize = (WORD) (((count - 1) * 0x1C0) + datalen);
	}
	else if (datalen < 0x1C0)
	{
		//Size should never be less than 0x1C0 if it isn't the last chunk
		LOG(Temp, Normal, "This should never happen\n");
	}
}

bool FragmentStack::IsComplete(void)
{
	for (int i = 0; i < m_wCount; i++)
	{
		if (m_pbReceived[i] != true) { return false; }
	}

	return true;
}

int FragmentStack::GetLength(void)
{
	return m_wSize;
}

BYTE *FragmentStack::GetData(void)
{
	return m_pbData;
}

