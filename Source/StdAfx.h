
#pragma once

#pragma warning(disable: 4503)

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning( disable : 4503 ) //4503 can safely be ignored

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#define WIN32_LEAN_AND_MEAN
#define PSAPI_VERSION 2

#include <windows.h>
#include <WinSock2.h>
#include <shellapi.h>
#include <commctrl.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <regex>
#include <psapi.h>

#include "zlib/zlib.h"
#include "mysql/mysql.h"

//STL
#include <vector>
#include <list>
#include <map>
#include <hash_map>
#include <string>
#include <iterator>
#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <set>
#include <type_traits>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>

#include "resource.h"

#define stricmp _stricmp
#define strlwr _strlwr

#ifdef _DEBUG
#define DEFAULT_CONFIG_FILE "debug_server.cfg"
#define CONFIGURATION_STRING "Debug"
#else
#define DEFAULT_CONFIG_FILE "server.cfg"
#define CONFIGURATION_STRING "Release"
#endif

#ifdef _WIN64
#define PLATFORM_STRING "64-bit"
#else
#define PLATFORM_STRING "32-bit"
#endif

#ifdef PUBLIC_BUILD
#define PUBLIC_STRING " Public"
#else
#define PUBLIC_STRING ""
#endif

#define SERVER_VERSION_NUMBER_STRING "1.07"
#define SERVER_VERSION_STRING __DATE__ " @ " __TIME__ " (" CONFIGURATION_STRING " " PLATFORM_STRING PUBLIC_STRING ")"

//#define SERVER_VERSION_NUMBER_STRING "1.0.0.9" // "1.0.0.11"
//#define SERVER_VERSION_STRING "Aug 14 2017 @ 21:13:38 (Release 64-bit Public)"
//#define SERVER_VERSION_STRING "(" CONFIGURATION_STRING " " PLATFORM_STRING PUBLIC_STRING ")"

class CWeenieObject;
class CPlayerWeenie;

#include "Common.h"
#include "Util.h"
#include "Globals.h"

extern class CDatabase *g_pDB;
extern class CMYSQLDatabase *g_pDB2;
extern class CMYSQLDatabase *g_pDB2Async;
extern class CDatabaseIO *g_pDBIO;
extern class CGameDatabase *g_pGameDatabase;
extern class ServerCellManager *g_pCellManager;
extern class AllegianceManager *g_pAllegianceManager;
extern class FellowshipManager *g_pFellowshipManager;
extern class GameEventManager *g_pGameEventManager;
extern class CWorld *g_pWorld;
extern class CNetwork *g_pNetwork;
extern class CObjectIDGenerator *g_pObjectIDGen;
extern class CWeenieFactory *g_pWeenieFactory;
extern class CTreasureFactory *g_pTreasureFactory;
extern class WeenieLandCoverage *g_pWeenieLandCoverage;
extern class CInferredPortalData *g_pPortalDataEx;
extern class CInferredCellData *g_pCellDataEx;
extern class CPhatDataBin *g_pPhatDataBin;

#include "DATDisk.h"
#include "TurbineData.h"
#include "TurbinePortal.h"
#include "TurbineCell.h"

extern TURBINEPORTAL* g_pPortal;
extern TURBINECELL* g_pCell;

extern bool g_bDebugToggle;

extern DWORD g_CurrentHouseMaintenancePeriod;
extern DWORD g_NextHouseMaintenancePeriod;
extern bool g_FreeHouseMaintenancePeriod;