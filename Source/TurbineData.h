
#pragma once

class TurbineData;
class TurbineFile;
class TurbineObject;

typedef TurbineData	TURBINEDATA;
typedef TurbineFile	TURBINEFILE;

typedef struct FileInfo
{
	uint32_t dwPosition;
	uint32_t dwLength;
} FILEINFO;

typedef	std::map<uint32_t, FILEINFO> FILEMAP;

#include "TurbineFile.h"

class TurbineData
{
public:
	TurbineData();
	virtual ~TurbineData();
	
	uint32_t GetVersion();
	uint32_t GetFileCount();

	FILEMAP *GetFiles();
	TURBINEFILE *GetFile(uint32_t dwID);

	BOOL FileExists(uint32_t dwID);

	int CompareIteration(BYTE *data);

protected:
	void LoadFile(const char* szFile);
	void CloseFile();

protected:
	std::string	m_strPath;
	std::string	m_strFile;

	static void FileFoundCallback(void *This, uint32_t dwFileID, BTEntry *pEntry);
	void FileFoundCallbackInternal(uint32_t dwFileID, BTEntry *pEntry);

	DATDisk *m_pDATDisk;
	uint32_t m_dwVersion;
	FILEMAP	m_mFileInfo;

	std::unique_ptr<TurbineFile> m_iterFile;
};