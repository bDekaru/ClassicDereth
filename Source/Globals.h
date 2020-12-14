
#pragma once

class CGlobals
{
public:
	CGlobals();
	~CGlobals();

	double Time();
	double UpdateTime();

	int Timestamp();

	const char *GetGameDirectory();
	std::string GetGameFile(const char *filename);
	std::string GetGameData(const char *type, const char *filename);

	void SetWindowHandle(HWND);
	HWND GetWindowHandle();
	void SetConsoleWindowHandle(HWND);
	HWND GetConsoleWindowHandle();

	void SetClientCount(WORD wCount);
	WORD GetClientCount();

	void PacketSent(uint32_t dwLength);
	void PacketRecv(uint32_t dwLength);
	UINT64 GetPacketSendCount();
	UINT64 GetPacketRecvCount();
	UINT64 GetPacketSendSize();
	UINT64 GetPacketRecvSize();
	void ResetPackets();

	time_point m_start;
	time_point m_last;

private:
	HWND m_hWnd;
	HWND m_hConsoleWnd;
	//char m_GameDir[MAX_PATH + 2];
	fs::path m_GameDir;

	WORD m_wClientCount;
	UINT64 m_cPacketSendCount;
	UINT64 m_cPacketRecvCount;
	UINT64 m_cPacketSendSize;
	UINT64 m_cPacketRecvSize;

	double m_fTime = 0.0;
};

extern CGlobals *g_pGlobals;

class CStopWatch
{
public:
	CStopWatch() : m_start(steady_clock::now())
	{
		//m_fStartTime = g_pGlobals->UpdateTime();
	}

	void Reset()
	{
		m_start = steady_clock::now();
	}

	double GetElapsed()
	{
		std::chrono::duration<double> elapsed = steady_clock::now() - m_start;
		return elapsed.count();
		//return (g_pGlobals->UpdateTime() - m_fStartTime);
	}

private:
	time_point m_start;

	//double m_fStartTime;
};

namespace Time
{
	inline double GetTimeCurrent()
	{
		return g_pGlobals->Time();
	}
};


