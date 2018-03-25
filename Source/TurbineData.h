
#pragma once

class TurbineData;
class TurbineFile;
class TurbineObject;

typedef TurbineData	TURBINEDATA;
typedef TurbineFile	TURBINEFILE;

typedef struct FileInfo
{
	DWORD dwPosition;
	DWORD dwLength;
} FILEINFO;

typedef	std::map<DWORD, FILEINFO> FILEMAP;

#include "TurbineFile.h"

class TurbineData
{
public:
	TurbineData();
	virtual ~TurbineData();
	
	DWORD GetVersion();
	DWORD GetFileCount();

	FILEMAP *GetFiles();
	TURBINEFILE *GetFile(DWORD dwID);

	BOOL FileExists(DWORD dwID);

protected:
	void LoadFile(const char* szFile);
	void CloseFile();

protected:
	std::string	m_strPath;
	std::string	m_strFile;

	static void FileFoundCallback(void *This, DWORD dwFileID, BTEntry *pEntry);
	void FileFoundCallbackInternal(DWORD dwFileID, BTEntry *pEntry);

	DATDisk *m_pDATDisk;
	DWORD m_dwVersion;
	FILEMAP	m_mFileInfo;
};