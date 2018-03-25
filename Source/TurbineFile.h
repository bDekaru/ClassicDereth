
#pragma once

class TurbineFile
{
public:
	TurbineFile(DWORD dwID);
	TurbineFile(DWORD dwID, BYTE *data, DWORD length);
	~TurbineFile();

	BYTE *GetData();
	DWORD GetLength();

private:
	DWORD m_dwID;
	BYTE *m_pbData;
	DWORD m_dwLength;
};
