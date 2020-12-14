
#include <StdAfx.h>
#include "Logging.h"

CLogger g_Logger;

CLogFile::CLogFile()
{
}

CLogFile::~CLogFile()
{
	Close();
}

bool CLogFile::Open(const char *filepath)
{
	SCOPE_LOCK

	Close();
	m_File = fopen(filepath, "wt");

	return m_File != NULL;
}

void CLogFile::Close()
{
	SCOPE_LOCK

	if (m_File)
	{
		fclose(m_File);
		m_File = NULL;
	}
}

void CLogFile::Write(const char *text)
{
	SCOPE_LOCK

	if (m_File)
	{
		fwrite(text, sizeof(char), strlen(text), m_File);
	}
}

CLogger::CLogger()
{
}

CLogger::~CLogger()
{
	Close();
}

bool CLogger::Open()
{
	return m_Log.Open(g_pGlobals->GetGameData("Logs", csprintf("console_%s.txt", timestampDateStringForFileName(time(NULL)))).c_str());
}

void CLogger::Close()
{
	m_Log.Close();
}

void CLogger::Write(int category, int level, const char *format, ...)
{

	va_list args;
	va_start(args, format);

	int charcount = _vscprintf(format, args) + 1;
	char *charbuffer = new char[charcount];
	vsnprintf(charbuffer, charcount, format, args);

	va_end(args);

#if defined(_WINDOWS) && defined(_DEBUG)
	OutputDebugStringA(charbuffer);
#endif

	m_Log.Write(charbuffer);

	for (auto& callback : m_LogCallbacks)
	{
		callback(category, level, charbuffer);
	}

	delete[] charbuffer;
}

