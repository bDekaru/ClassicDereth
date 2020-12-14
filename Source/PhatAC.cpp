#include <StdAfx.h>
#include "Server.h"
#include "World.h"
#include "PacketCaptureCredits.h"
#include "Client.h"
#include "Logging.h"
#include "easylogging++.h"
#include "cxxopts.hpp"

INITIALIZE_EASYLOGGINGPP

CPhatServer *g_pPhatServer = 0;
UINT64 framerecord = 0;
std::string g_configFile = DEFAULT_CONFIG_FILE;
bool g_autoStart;

extern void InitPhatSDK();
extern void CleanupPhatSDK();

void InitBasic(int argc, char** argv)
{
	cxxopts::Options options("GDLE - Classic Dereth", "Game Server");
	options.add_options()
		("c,config", "configuration file", cxxopts::value<std::string>(g_configFile))
		("s,start", "start the service automatically", cxxopts::value<bool>(g_autoStart));

	cxxopts::ParseResult result = options.parse(argc, argv);

	el::Loggers::configureFromGlobal("./GDLELogging.conf");
	el::Loggers::addFlag(el::LoggingFlag::AutoSpacing);

	srand((unsigned int)time(NULL));
	Random::Init();
	InitPhatSDK();

	g_pGlobals = new CGlobals();
	g_Logger.Open();

	extern uint64_t g_RandomAdminPassword;
	std::chrono::duration ticks = steady_clock::now().time_since_epoch();
	g_RandomAdminPassword = ((uint64_t)Random::GenUInt(0, 0xFFFFFFF0) << 32) | Random::GenUInt(0, 0xFFFFFFF0) + (uint64_t)ticks.count();

}


#if defined(_WIN32) && !defined(_WINDLL) && !defined(_CONSOLE)

HINSTANCE g_hInstance = 0;
HWND g_hWndLauncher = 0;
HWND g_hWndStatus = 0;
HWND g_hWndMain = 0;

void ToggleServerState(HWND hDlg);

// const char *PACKET_CAPTURE_CREDITS = PACKET_CAPTURE_CREDITS_STRING;

LRESULT CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_CREDITS), "GDLE - Classic Dereth " SERVER_VERSION_NUMBER_STRING "\n" SERVER_VERSION_STRING "\n\nClassic Dereth created by Dekaru (dekaru@protonmail.com)\nhttps://github.com/bDekaru/ClassicDereth\n\nWith updates from the GDLEnhanced project that is maintained by Scribble and ChosenOne.\n\nBased on the GamesDeadLol Server project.\nhttps://github.com/GamesDeadLol/GDL\n\nPhatAC originally created by Pea and halted in September 2017.\n\nPhatAC was created in C++ and compiled with Visual Studio. Third-party libraries used include zlib, SHA512 and ISAAC algorithms.");
		// SetWindowText(GetDlgItem(hDlg, IDC_PACKETCREDITS), PACKET_CAPTURE_CREDITS);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		case IDC_WEBSITE:
			ShellExecute(0, "open", "https://github.com/bDekaru/ClassicDereth", NULL, NULL, SW_SHOW);
			break;
		case IDC_WEBSITE2:
			ShellExecute(0, "open", "https://github.com/GamesDeadLol/GDL", NULL, NULL, SW_SHOW);
			break;
		case IDC_WEBSITE3:
			ShellExecute(0, "open", "https://gdleac.com/", NULL, NULL, SW_SHOW);
			break;
		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

LRESULT CALLBACK StatusProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			HWND hWndPriority = GetDlgItem(hDlg, IDC_PRIORITY);
			SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Lowest");
			SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Below Normal");
			SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Normal");
			SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Above Normal");
			SendMessage(hWndPriority, CB_ADDSTRING, 0, (LPARAM)"Highest");

			int tp = GetThreadPriority(GetCurrentThread());
			int index;
			switch (tp)
			{
			case THREAD_PRIORITY_LOWEST:		index = 0; break;
			case THREAD_PRIORITY_BELOW_NORMAL:	index = 1; break;
			case THREAD_PRIORITY_NORMAL:		index = 2; break;
			case THREAD_PRIORITY_ABOVE_NORMAL:	index = 3; break;
			case THREAD_PRIORITY_HIGHEST:		index = 4; break;
			default:							index = 2; break;
			};
			SendMessage(hWndPriority, CB_SETCURSEL, index, 0);

			return TRUE;
		}
	case WM_COMMAND:
		{
			WORD wEvent = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch (wID)
			{
			case IDC_PRIORITY:
				if (wEvent == CBN_SELCHANGE)
				{
					int index = SendMessage(GetDlgItem(hDlg, IDC_PRIORITY), CB_GETCURSEL, 0, 0);
					int tp;
					switch (index)
					{
					case 0: tp = THREAD_PRIORITY_LOWEST; break;
					case 1: tp = THREAD_PRIORITY_BELOW_NORMAL; break;
					case 3: tp = THREAD_PRIORITY_ABOVE_NORMAL; break;
					case 4:	tp = THREAD_PRIORITY_HIGHEST; break;
					case 2:
					default: tp = THREAD_PRIORITY_NORMAL; break;
					}

					SetThreadPriority(GetCurrentThread(), tp);
				}
			case IDOK:
				if (wEvent == BN_CLICKED)
				{
					g_hWndStatus = 0;
					DestroyWindow(hDlg);
				}
				break;
			}
			break;
		}
	case WM_CLOSE:
		g_hWndStatus = 0;
		DestroyWindow(hDlg);
		break;
	}

	return FALSE;
}

//LRESULT CALLBACK LauncherProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	switch (message)
//	{
//	case WM_INITDIALOG:
//		{
//			char account[256];
//			char password[256];
//			char remoteport[256];
//			uint32_t remoteip;
//			if (ReadConfigKey("account", account, 256))
//				SetWindowText(GetDlgItem(hDlg, IDC_ACCOUNT), account);
//			if (ReadConfigKey("password", password, 256))
//				SetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), password);
//			if (ReadConfigKey("remoteport", remoteport, 256))
//				SetWindowText(GetDlgItem(hDlg, IDC_REMOTEPORT), remoteport);
//			if (ReadConfigKey("remoteip", &remoteip))
//				SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_SETADDRESS, 0, remoteip);
//
//			return TRUE;
//		}
//	case WM_COMMAND:
//		{
//			WORD wEvent = HIWORD(wParam);
//			WORD wID = LOWORD(wParam);
//
//			switch (wID)
//			{
//			case IDC_LAUNCHCLIENT:
//				if (wEvent == BN_CLICKED)
//				{
//					char szRemotePort[10];
//					char szAccount[64]; *szAccount = 0;
//					char szPassword[64]; *szPassword = 0;
//					uint32_t dwRemoteIP;
//					uint32_t dwFields = (uint32_t)SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_GETADDRESS, 0, (LPARAM)&dwRemoteIP);
//					SendMessage(GetDlgItem(hDlg, IDC_REMOTEPORT), WM_GETTEXT, 10, (LPARAM)&szRemotePort);
//					SendMessage(GetDlgItem(hDlg, IDC_ACCOUNT), WM_GETTEXT, 64, (LPARAM)&szAccount);
//					SendMessage(GetDlgItem(hDlg, IDC_PASSWORD), WM_GETTEXT, 64, (LPARAM)&szPassword);
//
//					if (!strlen(szAccount) || !strlen(szPassword))
//					{
//						MsgBox("Please specify the account name/password to connect with.");
//						break;
//					}
//
//					if (dwFields == 4)
//					{
//						dwRemoteIP = htonl(dwRemoteIP);
//						char szLaunch[256];
//						char szLaunchDir[MAX_PATH + 1];
//						sprintf(szLaunch, "-h %s -p %u -rodat off -a %s:%s", dwRemoteIP != 0 ? inet_ntoa(*((in_addr *)&dwRemoteIP)) : "127.0.0.1", atol(szRemotePort), szAccount, szPassword);
//
//						uint32_t	dwLen = MAX_PATH + 1;
//						memset(szLaunchDir, 0, dwLen);
//						dwLen -= 1;
//
//						/*
//						HKEY hKey;
//						if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Microsoft Games\\Asheron's Call\\1.00", NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
//						{
//							RegQueryValueEx(hKey, "Portal Dat", NULL, NULL, (BYTE*)szLaunchDir, &dwLen);
//							RegCloseKey(hKey);
//						}
//						else
//						{
//							MsgBox("Couldn't find installation directory. Is AC installed?\r\n");
//							break;
//						}
//						*/
//
//						char dir[MAX_PATH];
//						GetCurrentDirectory(MAX_PATH, dir);
//						dir[MAX_PATH - 1] = '\0';
//
//						sprintf(szLaunchDir, "%s\\Client", dir);
//
//						if (szLaunchDir[strlen(szLaunchDir) - 1] != '\\')
//						{
//							char *end = &szLaunchDir[strlen(szLaunchDir)];
//							end[0] = '\\';
//							end[1] = '\0';
//						}
//
//						if (!FileExists(((std::string)szLaunchDir + "\\acclient.exe").c_str()))
//						{
//							MsgBox("Please copy your Asheron's Call client to the Client folder.\r\n");
//						}
//						else
//						{
//							//LOG(Temp, Normal, "Launching %s %s\n", szLaunch, szLaunchDir);
//							ShellExecute(0, "open", "acclient.exe", szLaunch, szLaunchDir, SW_SHOW);
//						}
//					}
//					else
//						MsgBox("Please specify the remote IP to connect to.");
//				}
//				break;
//			}
//			break;
//		}
//	case WM_CLOSE:
//		char account[256]; *account = 0;
//		char password[256]; *password = 0;
//		char remoteport[256]; *remoteport = 0;
//		uint32_t remoteip;
//		if (GetWindowText(GetDlgItem(hDlg, IDC_ACCOUNT), account, 256))
//			SaveConfigKey("account", account);
//		if (GetWindowText(GetDlgItem(hDlg, IDC_PASSWORD), password, 256))
//			SaveConfigKey("password", password);
//		if (GetWindowText(GetDlgItem(hDlg, IDC_REMOTEPORT), remoteport, 256))
//			SaveConfigKey("remoteport", remoteport);
//		if (4 == SendMessage(GetDlgItem(hDlg, IDC_REMOTEIP), IPM_GETADDRESS, 0, (LPARAM)&remoteip))
//			SaveConfigKey("remoteip", remoteip);
//
//		g_hWndLauncher = 0;
//		DestroyWindow(hDlg);
//		break;
//	}
//
//	return FALSE;
//}

bool g_bOutputConsoleDisabled = false;

void OutputConsole(int category, int level, const char *text)
{
	if (level < LOGLEVEL_Normal)
	{
		return;
	}

	if (g_bOutputConsoleDisabled)
	{
		return;
	}

	HWND hWndConsole = g_pGlobals->GetConsoleWindowHandle();

	if (!hWndConsole)
		return;

	std::string _fixed_text = text;

	std::string::size_type pos = 0u;
	while ((pos = _fixed_text.find("\n", pos)) != std::string::npos) {
		_fixed_text.replace(pos, 1, "\r\n");
		pos += 2;
	}

	int len = (int)SendMessage(hWndConsole, WM_GETTEXTLENGTH, 0, 0);
	uint32_t start, end;
	SendMessage(hWndConsole, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
	SendMessage(hWndConsole, EM_SETSEL, len, len);
	SendMessage(hWndConsole, EM_REPLACESEL, FALSE, (LPARAM)_fixed_text.c_str());
	SendMessage(hWndConsole, EM_SETSEL, start, end);
}

void SetWindowStringText(HWND hWnd, const char *text)
{
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)text);
}

std::string GetWindowStringText(HWND hWnd)
{
	char text[300];
	text[0] = '\0';
	SendMessage(hWnd, WM_GETTEXT, 300, (LPARAM)text);
	text[299] = '\0';

	return text;
}

INT_PTR CALLBACK MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			g_pGlobals->SetWindowHandle(hDlg);
			g_pGlobals->SetConsoleWindowHandle(GetDlgItem(hDlg, IDC_CONSOLE));

			g_Logger.AddLogCallback(OutputConsole);

			HWND hVersion = GetDlgItem(hDlg, IDC_VERSION);
			SetWindowText(hVersion, "GDLE - Classic Dereth " SERVER_VERSION_NUMBER_STRING " " SERVER_VERSION_STRING); // "PhatAC compiled " __TIMESTAMP__);

			HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PHATAC));
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			DeleteObject(hIcon);

			SetWindowStringText(GetDlgItem(hDlg, IDC_SERVERCONFIG), g_configFile.c_str());

#ifdef PUBLIC_BUILD
			ShowWindow(GetDlgItem(hDlg, IDC_DEBUGCHECK), SW_HIDE);
#endif

			return TRUE;
		}
	case WM_COMMAND:
		{
			WORD wEvent = HIWORD(wParam);
			WORD wID = LOWORD(wParam);

			switch (wID)
			{
			case IDM_EXIT:
				PostQuitMessage(0);
				DestroyWindow(hDlg);
				break;
			case IDM_ABOUT:
				DialogBox(g_hInstance, (LPCTSTR)IDD_ABOUT, hDlg, (DLGPROC)AboutProc);
				break;

			//case IDM_LAUNCHER:
			//	if (!g_hWndLauncher)
			//	{
			//		g_hWndLauncher = CreateDialog(g_hInstance, (LPCTSTR)IDD_LAUNCHER, hDlg, (DLGPROC)LauncherProc);
			//		ShowWindow(g_hWndLauncher, SW_SHOW);
			//	}
			//	break;
			case IDM_STATUS:
				if (!g_hWndStatus)
				{
					g_hWndStatus = CreateDialog(g_hInstance, (LPCTSTR)IDD_STATUS, hDlg, (DLGPROC)StatusProc);
					ShowWindow(g_hWndStatus, SW_SHOW);
				}
				break;
			case IDC_DEBUGCHECK:
				{
#ifndef PUBLIC_BUILD
					if (wEvent == BN_CLICKED)
					{
						g_bDebugToggle = BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_DEBUGCHECK), BM_GETCHECK, 0, 0);
					}
#endif
					break;
				}
			case IDC_CLEARLOG:
				{
					if (wEvent == BN_CLICKED)
					{
						HWND hWndConsole = GetDlgItem(hDlg, IDC_CONSOLE);

						SetWindowText(hWndConsole, "");
						SERVER_INFO << "Console cleared.";
						WINLOG(Data, Warning, "Console cleared.\n");
					}
				}
				break;
			case IDC_BROADCAST:
				if (wEvent == BN_CLICKED)
				{
					if (!g_pPhatServer)
					{
						SERVER_INFO << "You must be running a server to broadcast a system message.";
						break;
					}

					char text[400];
					memset(text, 0, 400);
					SendMessage(GetDlgItem(hDlg, IDC_BROADCASTTEXT), WM_GETTEXT, 399, (LPARAM)text);

					g_pPhatServer->SystemBroadcast(text);
				}
				break;

			case IDC_LAUNCH:
				if (wEvent == BN_CLICKED)
				{
					if (!g_pPhatServer)
					{
						//MsgBox("You might want to start the server first. ;)");
						::MessageBox(hDlg, "You need to start the server first.", "GDLE - Classic Dereth", MB_ICONEXCLAMATION);
						break;
					}

					::MessageBox(hDlg, "You need to use a launcher", "GDLE - Classic Dereth", MB_ICONEXCLAMATION);

					//char szLaunch[256];
					//char szLaunchDir[MAX_PATH + 10];
					//extern uint64_t g_RandomAdminPassword;
					//sprintf(szLaunch, "-h %s -p %s -rodat off -a admin:%I64u",
					//	(!strcmp(g_pPhatServer->Config().GetValue("bind_ip"), "0.0.0.0") ? "127.0.0.1" : g_pPhatServer->Config().GetValue("bind_ip")),
					//	g_pPhatServer->Config().GetValue("bind_port", "9050"),
					//	g_RandomAdminPassword);

					//uint32_t dwLen = MAX_PATH + 10;
					//memset(szLaunchDir, 0, dwLen);
					//dwLen -= 1;

					//char dir[MAX_PATH];
					//GetCurrentDirectory(MAX_PATH, dir);
					//dir[MAX_PATH - 1] = '\0';

					//sprintf(szLaunchDir, "%s\\Client", dir);

					//if (szLaunchDir[strlen(szLaunchDir) - 1] != '\\')
					//{
					//	char *end = &szLaunchDir[strlen(szLaunchDir)];
					//	end[0] = '\\';
					//	end[1] = '\0';
					//}

					//if (!FileExists(((std::string)szLaunchDir + "\\acclient.exe").c_str()))
					//{
					//	MsgBox("Please copy your Asheron's Call client to the Client folder.\r\n");
					//}
					//else
					//{
					//	//LOG(Temp, Normal, "Launching %s %s\n", szLaunch, szLaunchDir);
					//	ShellExecute(0, "open", "acclient.exe", szLaunch, szLaunchDir, SW_SHOW);
					//}
				}
				break;

			case IDC_EDIT:
				if (wEvent == BN_CLICKED)
				{
					std::string serverConfig = GetWindowStringText(GetDlgItem(hDlg, IDC_SERVERCONFIG));

					if (serverConfig.empty())
					{
						serverConfig = DEFAULT_CONFIG_FILE;
					}

					std::string configFilePath = g_pGlobals->GetGameFile(serverConfig.c_str());

					if (FileExists(configFilePath.c_str()))
					{
						ShellExecute(NULL, "open", configFilePath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
					}

					break;
				}

			case IDC_TOGGLE:
				if (wEvent == BN_CLICKED)
				{
					ToggleServerState(hDlg);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		if (g_hWndLauncher)	SendMessage(g_hWndLauncher, WM_CLOSE, 0, 0);
		if (g_hWndStatus)		SendMessage(g_hWndStatus, WM_CLOSE, 0, 0);

		PostQuitMessage(0);
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
#if !defined(QUICKSTART) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	InitBasic(__argc, __argv);
	
	INITCOMMONCONTROLSEX iccex;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_INTERNET_CLASSES;
	InitCommonControlsEx(&iccex);

	WSADATA	wsaData;
	USHORT wVersionRequested = 0x0202;
	WSAStartup(wVersionRequested, &wsaData);

	g_hInstance = hInstance;
	g_hWndMain = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainProc);

	if (!g_hWndMain)
	{
		::MessageBox(NULL, "There was an error creating the window", "GDLE - Classic Dereth", MB_ICONERROR);
		SERVER_ERROR << "Error creating dialog" << GetLastError();
		//MsgBoxError(GetLastError(), "creating main dialog box");
		return 0;
	}

	WINLOG(Temp, Normal, "Welcome to GDLE - Classic Dereth!\n");
	SERVER_INFO << "Welcome to GDLE - Classic Dereth!";

	ShowWindow(g_hWndMain, nCmdShow);

	if (g_autoStart)
	{
		ToggleServerState(g_hWndMain);
	}

	MSG msg;
	msg.message = WM_NULL;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (IsDialogMessage(g_hWndMain, &msg))
				continue;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Sleep(1);

			extern bool g_bRestartOnNextTick;

			if (g_bRestartOnNextTick)
			{
				g_bRestartOnNextTick = false;
				
				if (g_pPhatServer)
				{
					std::string configPath = g_pPhatServer->Config().GetConfigPath();
					SafeDelete(g_pPhatServer);

					g_pPhatServer = new CPhatServer(configPath.c_str());
				}
			}

			//if (g_pPhatServer)
			//	g_pPhatServer->Tick();
		}
	}

	SafeDelete(g_pPhatServer);
	SafeDelete(g_pGlobals);
	WSACleanup();

	CleanupPhatSDK();

#if !defined(QUICKSTART) && defined(_DEBUG)
	/*
	if (_CrtDumpMemoryLeaks())
		OutputDebugString("Memory leak found!\n");
	else
		OutputDebugString("No memory leaks found!\n");
		*/
#endif

	return 0;
}

void ToggleServerState(HWND hDlg)
{
	if (!g_pPhatServer)
	{
		std::string serverConfig = GetWindowStringText(GetDlgItem(hDlg, IDC_SERVERCONFIG));

		if (serverConfig.empty())
		{
			serverConfig = DEFAULT_CONFIG_FILE;
		}

		std::string configFilePath = g_pGlobals->GetGameFile(serverConfig.c_str());

		if (FileExists(configFilePath.c_str()))
		{
			g_pPhatServer = new CPhatServer(configFilePath.c_str());
			SetWindowText(GetDlgItem(hDlg, IDC_TOGGLE), "Stop");
			SetWindowText(GetDlgItem(hDlg, IDC_CONNECTLINK),
				csprintf("acclient.exe -h %s -p %s -a username:password -rodat off",
				(!strcmp(g_pPhatServer->Config().GetValue("bind_ip"), "0.0.0.0") ? "127.0.0.1" : g_pPhatServer->Config().GetValue("bind_ip")),
					g_pPhatServer->Config().GetValue("bind_port")));
		}
		else
		{
			//MsgBox("Please specify a valid config filename.");
			::MessageBox(hDlg, "Please specify a valid config filename", "GDLE - Classic Dereth", MB_ICONEXCLAMATION);
		}
	}
	else
	{
		g_bOutputConsoleDisabled = true;
		SafeDelete(g_pPhatServer);
		g_bOutputConsoleDisabled = false;


		SetWindowText(GetDlgItem(hDlg, IDC_TOGGLE), "Start");
		SetWindowText(GetDlgItem(hDlg, IDC_CONNECTLINK), "");

		WINLOG(Temp, Normal, "Server shutdown.\n");
		SERVER_INFO << "Server shutdown.";
	}
}

#endif

#if defined(_CONSOLE)

#include <csignal>

volatile std::sig_atomic_t signal_status = 0;

void signal_handler(int signal)
{
	signal_status = signal;
}

int main(int argc, char** argv)
{
	InitBasic(argc, argv);

	std::string configFilePath = g_pGlobals->GetGameFile(g_configFile.c_str());

	if (FileExists(configFilePath.c_str()))
	{
		g_pPhatServer = new CPhatServer(configFilePath.c_str());
	}
	else
	{
		printf("Please specify a valid config filename.");
		return 1;
	}

	std::signal(SIGINT, signal_handler);

	// wait for server to fully start
	while (!g_pPhatServer->IsRunning())
		std::this_thread::sleep_for(std::chrono::seconds(5));

	while (signal_status == 0)
		std::this_thread::sleep_for(std::chrono::seconds(5));

	SERVER_INFO << "Shutting down from signal" << signal_status;

	std::this_thread::sleep_for(std::chrono::seconds(5));

	delete g_pPhatServer;
	return 0;
}

#endif
