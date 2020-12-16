#pragma once

#pragma warning(disable: 4503)

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning( disable : 4503 ) //4503 can safely be ignored

#if defined(_WINDOWS)

#define NOMINMAX
#define _USE_MATH_DEFINES

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#define RELEASE_ASSERT(x) if (!(x)) DebugBreak();

#define WIN32_LEAN_AND_MEAN
#define PSAPI_VERSION 2

#include <windows.h>
#include <WinSock2.h>
#include <shellapi.h>
#include <commctrl.h>
#include <psapi.h>

#define stricmp _stricmp
#define strlwr _strlwr

#else	// _WINDOWS
#include <stdlib.h>
#include <inttypes.h>
#include <float.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define RELEASE_ASSERT(x)

typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef int32_t BOOL;
typedef uint32_t ULONG;
typedef int32_t LONG;
typedef int64_t __int64;
typedef uint64_t UINT64;
typedef uint32_t UINT32;
typedef uint32_t UINT;

typedef sockaddr_in SOCKADDR_IN;
typedef int SOCKET;

#define INT64 int64_t
#define QWORD uint64_t
#define HANDLE void*
#define HWND void*
#define LPVOID void*
#define TRUE 1
#define FALSE 0
#define MAX_PATH 255

#define WINAPI
#define __forceinline __attribute__((always_inline))
#define FORCEINLINE __forceinline
#define __cdecl __attribute__((cdecl))

#define HIWORD(x) ((x >> 16) & 0xffff)
#define LOWORD(x) (x & 0xffff)
#define MAKEWORD(low, high) ((high << 8) | low)

#define stricmp strcasecmp

struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
};

struct SIZE
{
	uint32_t cx;
	uint32_t cy;
};

#define _ASSERT assert

#endif	// _WINDOWS

#include <time.h>
#include <math.h>
#include <assert.h>
#include <regex>

#include <zlib/zlib.h>
#include <mysql/include/mysql.h>
#include <mio/mmap.hpp>
#include <xsocket.hpp>
#include <taskex.hpp>

#include <ciso646>

#define me(x) #x
//STL
#include <atomic>
#include <chrono>
#include <algorithm>
#if __has_include(<execution>) && defined(__cpp_lib_execution)
#include <execution>
#endif
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
//#include <hash_map>
#include <string>
#include <locale>
#include <iterator>
#include <stdint.h>
#include <stdio.h>
#include <set>
#include <type_traits>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <thread>
#include <future>
#include <mutex>
#include <memory>
#include <codecvt>

using std::min;
using std::max;
using steady_clock = std::chrono::steady_clock;
using time_point = std::chrono::steady_clock::time_point;
using clock_duration = std::chrono::steady_clock::duration;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

namespace fs = std::filesystem;

// this is evil...
//template<typename vt, typename vtb> vt min(vt a, vtb b) { return std::min<vt>(a, (vt)b); }
//template<typename vt, typename vtb> vt max(vt a, vtb b) { return std::max<vt>(a, (vt)b); }

typedef std::codecvt_utf8_utf16<char16_t> codecvt_16_8_t;
typedef std::wstring_convert<codecvt_16_8_t, char16_t> string16_convert_t;

#include "resource.h"

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

#define SERVER_VERSION_NUMBER_STRING "1.34"
#define SERVER_VERSION_STRING __DATE__ " @ " __TIME__ " (" CONFIGURATION_STRING " " PLATFORM_STRING PUBLIC_STRING ")"

//#define SERVER_VERSION_NUMBER_STRING "1.0.0.9" // "1.0.0.11"
//#define SERVER_VERSION_STRING "Aug 14 2017 @ 21:13:38 (Release 64-bit Public)"
//#define SERVER_VERSION_STRING "(" CONFIGURATION_STRING " " PLATFORM_STRING PUBLIC_STRING ")"

class CWeenieObject;
class CPlayerWeenie;

#include "Common.h"
#include "Util.h"
#include "Globals.h"
#include "Messages/IClientMessage.h"
#include "RecipeFactory.h"

extern class CDatabase *g_pDB;
extern class CMYSQLDatabase *g_pDB2;
extern class CMYSQLDatabase *g_pDB2Async;
extern class CMYSQLDatabase *g_pDBDynamicIDs;
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
extern class CHouseManager *g_pHouseManager;
extern class RecipeFactory *g_pRecipeFactory;

#include "DATDisk.h"
#include "TurbineData.h"
#include "TurbinePortal.h"
#include "TurbineCell.h"

extern TURBINEPORTAL* g_pPortal;
extern TURBINECELL* g_pCell;

extern bool g_bDebugToggle;

#if !defined(_MSC_VER)

inline int _vscprintf(const char *fmt, va_list pargs)
{
	int length = 0;
	va_list args;
	va_copy(args, pargs);
	length = vsnprintf(nullptr, 0, fmt, args);
	va_end(args);
	return length;
}

#endif

template<typename EnumType>
bool flags_check(EnumType flags, EnumType mask)
{
	return (static_cast<int>(flags) & static_cast<int>(mask)) != 0;
}

template<typename EnumType>
bool flags_match(EnumType flags, EnumType mask)
{
	return (static_cast<int>(flags) & static_cast<int>(mask)) == static_cast<int>(mask);
}

template<typename EnumType>
void flags_set(EnumType &flags, EnumType mask)
{
	flags = static_cast<EnumType>(static_cast<int>(flags) | static_cast<int>(mask));
}

template<typename EnumType>
void flags_clear(EnumType &flags, EnumType mask)
{
	flags = static_cast<EnumType>(static_cast<int>(flags) & ~static_cast<int>(mask));
}
