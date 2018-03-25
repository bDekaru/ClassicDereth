
#pragma once

class CDatabase : public CKillable
{
public:
	CDatabase();
	~CDatabase();

	FILE *DataFileOpen(const char*, const char* mode = "rb");
	FILE *DataFileCreate(const char*, const char* mode = "wb");
	BOOL DataFileFindFirst(const char* filemask, WIN32_FIND_DATA* data);
	BOOL DataFileFindNext(WIN32_FIND_DATA* data);
	void DataFileFindClose();

private:

	void Initialize();
	void Shutdown();

	HANDLE m_hSearchHandle;
};


