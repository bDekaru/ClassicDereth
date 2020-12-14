
#pragma once

class FragmentStack
{
public:
	FragmentStack();
	FragmentStack(FragPacket_s *);
	~FragmentStack();

	void AddFragment(FragPacket_s *);

	BYTE *GetData();
	int	GetLength();
	bool IsComplete();

	// Used for forming groups together
    uint64_t _id;

private:
	BYTE *m_pbData;
	bool *m_pbReceived;

	// Used for reading data
	BYTE *m_pbDataPtr;

	WORD m_wCount;
	WORD m_wGroup;
	WORD m_wSize;
};