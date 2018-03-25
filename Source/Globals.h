
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

	void SetWindowHandle(HWND);
	HWND GetWindowHandle();
	void SetConsoleWindowHandle(HWND);
	HWND GetConsoleWindowHandle();

	void SetClientCount(WORD wCount);
	WORD GetClientCount();

	void PacketSent(DWORD dwLength);
	void PacketRecv(DWORD dwLength);
	UINT64 GetPacketSendCount();
	UINT64 GetPacketRecvCount();
	UINT64 GetPacketSendSize();
	UINT64 GetPacketRecvSize();
	void ResetPackets();

	UINT64 m_CounterStart;
	UINT64 m_CounterTime;
	UINT64 m_CounterFreq;

private:
	HWND m_hWnd;
	HWND m_hConsoleWnd;
	char m_GameDir[MAX_PATH + 2];

	WORD m_wClientCount;
	UINT64 m_cPacketSendCount;
	UINT64 m_cPacketRecvCount;
	UINT64 m_cPacketSendSize;
	UINT64 m_cPacketRecvSize;

	double m_fTime;
};

extern CGlobals *g_pGlobals;

class CStopWatch
{
public:
	CStopWatch()
	{
		m_fStartTime = g_pGlobals->UpdateTime();
	}

	double GetElapsed()
	{
		return (g_pGlobals->UpdateTime() - m_fStartTime);
	}

private:
	double m_fStartTime;
};

namespace Time
{
	inline double GetTimeCurrent()
	{
		return g_pGlobals->Time();
	}
};


