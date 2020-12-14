
#include <StdAfx.h>
#include "Globals.h"

bool g_bDebugToggle = false;
CGlobals *g_pGlobals = NULL;

double g_TimeAdjustment = time(0) - 936144000; // 24000.0 + 2700.0 + (35.0 * 60.0) + (105 * 60.0);

CGlobals::CGlobals()
{
	m_hWnd = NULL;
	m_hConsoleWnd = NULL;

	m_last = m_start = steady_clock::now();
	m_fTime = g_TimeAdjustment;

	m_GameDir = fs::current_path();

	// int GameDirLen = GetCurrentDirectory(MAX_PATH, m_GameDir);
	// if (GameDirLen > 0)
	// {
	// 	if (m_GameDir[GameDirLen - 1] == '\\')
	// 	{
	// 		m_GameDir[GameDirLen - 1] = 0;
	// 	}
	// }

	ResetPackets();
	UpdateTime();
}

CGlobals::~CGlobals()
{
}

const char *CGlobals::GetGameDirectory()
{
	//return (char *)m_GameDir;
	return m_GameDir.string().c_str();
}

std::string CGlobals::GetGameFile(const char *filename)
{
	fs::path filepath = m_GameDir / filename;
	return filepath.string();
	//std::string filepath = GetGameDirectory();
	//filepath += "\\";
	//filepath += filename;
	//return filepath;
}

std::string CGlobals::GetGameData(const char *type, const char *filename)
{
	fs::path filepath = m_GameDir / type / filename;
	return filepath.string();
}

void CGlobals::SetWindowHandle(HWND hWnd)
{
	m_hWnd = hWnd;
}

HWND CGlobals::GetWindowHandle()
{
	return (HWND)m_hWnd;
}

void CGlobals::SetConsoleWindowHandle(HWND hConsoleWnd)
{
	m_hConsoleWnd = hConsoleWnd;
}

HWND CGlobals::GetConsoleWindowHandle()
{
	return (HWND)m_hConsoleWnd;
}

void CGlobals::SetClientCount(WORD wCount)
{
	m_wClientCount = wCount;
}

WORD CGlobals::GetClientCount()
{
	return (WORD)m_wClientCount;
}

void CGlobals::PacketSent(uint32_t dwLength)
{
	m_cPacketSendCount++;
	m_cPacketSendSize += dwLength;
}

void CGlobals::PacketRecv(uint32_t dwLength)
{
	m_cPacketRecvCount++;
	m_cPacketRecvSize += dwLength;
}

UINT64 CGlobals::GetPacketSendCount()
{
	return m_cPacketSendCount;
}

UINT64 CGlobals::GetPacketRecvCount()
{
	return m_cPacketRecvCount;
}

UINT64 CGlobals::GetPacketSendSize()
{
	return m_cPacketSendSize;
}

UINT64 CGlobals::GetPacketRecvSize()
{
	return m_cPacketRecvSize;
}

void CGlobals::ResetPackets()
{
	m_cPacketSendCount = 0;
	m_cPacketRecvCount = 0;

	m_cPacketSendSize = 0;
	m_cPacketRecvSize = 0;
}

double CGlobals::Time()
{
	return m_fTime;
}

int CGlobals::Timestamp()
{
	return time(0);
}

double CGlobals::UpdateTime()
{
	time_point current = steady_clock::now();

	std::chrono::duration<double> elapsed = current - m_last;
	m_last = current;

	m_fTime += elapsed.count();
	return m_fTime;
	//QueryPerformanceCounter((LARGE_INTEGER *)&m_CounterTime);
	//double fTime = (m_CounterTime - m_CounterStart) / (double)m_CounterFreq;
	//fTime += g_TimeAdjustment;
	//m_fTime = fTime;

	//return fTime;
}


