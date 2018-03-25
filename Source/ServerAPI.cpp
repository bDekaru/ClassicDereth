
#include "StdAfx.h"
#include "ServerAPI.h"
#include "Server.h"

extern void InitPhatSDK();
extern void CleanupPhatSDK();

#if defined(WIN32) && defined(_WINDLL)

bool g_bStarted = false;
CPhatServer *g_pPhatServer = NULL;
HINSTANCE g_hInstance = NULL;
HANDLE g_hQuitEvent = NULL;
HANDLE g_hServerThread = NULL;

DWORD WINAPI MainServerThread(LPVOID)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	srand((unsigned int)time(NULL));
	Random::Init();

	g_pGlobals = new CGlobals();
	g_Logger.Open();

	WSADATA	wsaData;
	USHORT wVersionRequested = 0x0202;
	WSAStartup(wVersionRequested, &wsaData);

	InitPhatSDK();

	extern DWORD64 g_RandomAdminPassword;
	g_RandomAdminPassword = ((DWORD64)Random::GenUInt(0, 0xFFFFFFF0) << 32) | Random::GenUInt(0, 0xFFFFFFF0);

	LOG(UserInterface, Normal, "Welcome to PhatAC - Classic Dereth!\n");

	unsigned long serverIP = GetLocalIP();
	int serverPort = 9050;

	g_pPhatServer = new CPhatServer();

	while (WaitForSingleObject(g_hQuitEvent, 0) != WAIT_OBJECT_0)
	{
		g_pPhatServer->Tick();
	}

	SafeDelete(g_pPhatServer);
	CleanupPhatSDK();

	WSACleanup();
	SafeDelete(g_pGlobals);
	g_Logger.Close();

#if !defined(QUICKSTART) && defined(_DEBUG) // check for memory leaks
	if (_CrtDumpMemoryLeaks())
		OutputDebugString("Memory leak found!\n");
	else
		OutputDebugString("No memory leaks found!\n");
#endif

	return 0;
}

extern "C"
{
	int _IsStarted()
	{
		return g_bStarted ? 1 : 0;
	}

	int _StartServer()
	{
		if (g_bStarted)
			return 0;
		
		g_hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (!g_hQuitEvent)
			return -1;

		g_hServerThread = CreateThread(NULL, 0, MainServerThread, NULL, 0, NULL);
		if (!g_hServerThread)
		{
			CloseHandle(g_hQuitEvent);
			return -1;
		}

		g_bStarted = true;
		return 0;
	}

	void _StopServer()
	{
		if (!g_bStarted)
			return;

		SetEvent(g_hQuitEvent);

		if (g_hServerThread)
		{
			WaitForSingleObject(g_hServerThread, 60000);
			CloseHandle(g_hServerThread);
			g_hServerThread = NULL;
		}

		if (g_hQuitEvent)
		{
			CloseHandle(g_hQuitEvent);
			g_hQuitEvent = NULL;
		}

		g_bStarted = false;
	}
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD reason, LPVOID your_mom)
{
	BOOL success = TRUE;

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
		break;

	case DLL_PROCESS_DETACH:
		_StopServer();
		break;
	}

	return success;
}

#endif


