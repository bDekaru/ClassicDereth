
#pragma once

class TurbineFile
{
public:
	TurbineFile(uint32_t dwID);
	TurbineFile(uint32_t dwID, BYTE *data, uint32_t length);
	~TurbineFile();

	BYTE *GetData();
	uint32_t GetLength();

private:
	uint32_t m_dwID;
	BYTE *m_pbData;
	uint32_t m_dwLength;
};
