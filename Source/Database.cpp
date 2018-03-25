
#include "StdAfx.h"
#include "Database.h"

CDatabase::CDatabase()
{
	LOG(Temp, Normal, "Initializing Database..\n");

	m_hSearchHandle = INVALID_HANDLE_VALUE;

	Initialize();
}

CDatabase::~CDatabase()
{
	Shutdown();

	if (m_hSearchHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hSearchHandle);
		m_hSearchHandle = INVALID_HANDLE_VALUE;
	}
}

void CDatabase::Initialize()
{
	if (!CreateDirectory("Data", NULL))
	{
		int dwError = GetLastError();

		if (dwError != ERROR_ALREADY_EXISTS)
		{
			MsgBoxError(dwError, "creating data folder");
		}
	}
}

void CDatabase::Shutdown()
{
}

FILE* CDatabase::DataFileOpen(const char* filename, const char* mode)
{
	std::string filepath = "Data\\";
	filepath += filename;
	FILE *fp = fopen(filepath.c_str(), mode);
	return fp;
}

FILE* CDatabase::DataFileCreate(const char* filename, const char* mode)
{
	std::string filepath = "Data\\";
	filepath += filename;
	FILE *fp = fopen(filepath.c_str(), mode);
	return fp;
}

BOOL CDatabase::DataFileFindFirst(const char* filemask, WIN32_FIND_DATA* data)
{
	std::string filepath = "Data\\";
	filepath += filemask;

	m_hSearchHandle = FindFirstFile(filepath.c_str(), data);

	return (m_hSearchHandle != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
}

BOOL CDatabase::DataFileFindNext(WIN32_FIND_DATA* data)
{
	if (m_hSearchHandle == INVALID_HANDLE_VALUE)
		return FALSE;

	return FindNextFile(m_hSearchHandle, data);
}

void CDatabase::DataFileFindClose()
{
	if (m_hSearchHandle != INVALID_HANDLE_VALUE)
	{
		FindClose(m_hSearchHandle);
		m_hSearchHandle = INVALID_HANDLE_VALUE;
	}
}





