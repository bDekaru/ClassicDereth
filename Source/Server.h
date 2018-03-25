
#pragma once

#include "StatTracker.h"
#include "Config.h"

class CPhatServer
{
public:
	CPhatServer(const char *configFileName = DEFAULT_CONFIG_FILE);
	~CPhatServer();

	void Tick(void);
	void KickClient(WORD);
	void BanClient(WORD);

	void SystemBroadcast(char *text);

	u_short	GetPort();

	CStatTracker &Stats();
	CPhatACServerConfig &Config();
	double GetStartupTime() { return m_fStartupTime; }

private:

	bool Init();
	void Shutdown();

	void InitializeSocket(unsigned short port, in_addr address);

	static DWORD WINAPI InternalThreadProcStatic(LPVOID lpThis);
	DWORD InternalThreadProc();

	SOCKET m_sockets[10];
	int m_socketCount;

	in_addr	m_hostaddr;
	u_short	m_hostport;

	CStatTracker m_Stats;
	CPhatACServerConfig m_Config;

	HANDLE m_hQuitEvent = NULL;
	HANDLE m_hServerThread = NULL;

	double m_fStartupTime;
};


