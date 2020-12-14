
#include <StdAfx.h>
#include "TurbineFormats.h"
#include <random>

#if defined(_WIN32)
#pragma comment(lib, "psapi.lib")
#else
#include <sys/sysinfo.h>
#endif

namespace Random
{
	static std::mt19937 _generator;
	static std::uniform_real_distribution<double> _range_f(0.0f, 1.0f);

	int m_iSeed;

	void Init()
	{
		std::random_device rd;
		currentSeed = rd();
		_generator.seed(currentSeed);

		m_iSeed = (signed)((unsigned int)(time(0)*12345) << 2) - 144810491;
	}

	uint32_t GenRandom15()
	{
		m_iSeed = (m_iSeed * 214013) + 2531011;

		uint32_t dwRandom15 = (unsigned)m_iSeed;
		dwRandom15 >>= 16;
		dwRandom15 &= 0x7FFF;

		return dwRandom15;
	}

	uint32_t currentSeed;
	void SetSeed(uint32_t newSeed)
	{
		currentSeed = newSeed;
		_generator.seed(newSeed);
	}

	uint32_t GetSeed()
	{
		return currentSeed;
	}

	uint32_t GenRandom32()
	{
		uint32_t dwRandom32 = 0;
		dwRandom32 |= GenRandom15();
		dwRandom32 <<= 15;
		dwRandom32 |= GenRandom15();
		dwRandom32 <<= 15;
		dwRandom32 |= GenRandom15();

		return dwRandom32;
	}

	uint32_t GenUInt(uint32_t min, uint32_t max)
	{
		if (max <= min)
			return min;

		std::uniform_int_distribution<uint32_t> range(min, max);
		return range(_generator);

		//unsigned int range = max - min;
		//return (min + (GenRandom32() % (range + 1)));
	}

	int32_t GenInt(int32_t min, int32_t max)
	{
		if (max <= min)
			return min;

		std::uniform_int_distribution<int32_t> range(min, max);
		return range(_generator);

		//unsigned int range = (unsigned)(max - min);
		//return (min + (int)(GenRandom32() % (range + 1)));
	}
	
	double GenFloat()
	{
		return _range_f(_generator);
	}

	double GenFloat(double min, double max)
	{
		if (max <= min)
			return min;

		double range = max - min;		
		//double value = min + (range * ((double)GenRandom32() / 0xFFFFFFFF));
		double value = min + (range * _range_f(_generator));

		return value;
	}

	float GenFloat(float min, float max)
	{
		if (max <= min)
			return min;

		double range = max - min;
		//double value = min + (range * ((double)GenRandom32() / 0xFFFFFFFF));
		double value = min + (range * _range_f(_generator));

		return (float)value;
	}

	float RollDice(float min, float max)
	{
		return GenFloat(min, max);
	}
}

//void EnumerateFolderFilePaths(std::list<std::string> &results, const char *basePath, const char *fileMask, bool bSubFolders)
//{
//	std::string basePathStr(basePath);
//	basePathStr += "\\";
//
//	WIN32_FIND_DATA data;
//	HANDLE hFind = FindFirstFile((basePathStr + fileMask).c_str(), &data);
//
//	if (hFind != INVALID_HANDLE_VALUE)
//	{
//		do
//		{
//			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
//			{
//				continue;
//			}
//
//			results.push_back(basePathStr + data.cFileName);
//
//		} while (FindNextFile(hFind, &data));
//
//		CloseHandle(INVALID_HANDLE_VALUE);
//	}
//
//	if (bSubFolders)
//	{
//		hFind = FindFirstFile((basePathStr + "*").c_str(), &data);
//		if (hFind != INVALID_HANDLE_VALUE)
//		{
//			do
//			{
//				if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
//				{
//					if (!strcmp(data.cFileName, ".") || !strcmp(data.cFileName, ".."))
//						continue;
//
//					std::string subFolderStr;
//					subFolderStr = basePathStr + data.cFileName;
//					EnumerateFolderFilePaths(results, subFolderStr.c_str(), fileMask, true);
//					continue;
//				}
//			} while (FindNextFile(hFind, &data));
//			CloseHandle(INVALID_HANDLE_VALUE);
//		}
//	}
//}

char NibbleToChar(int n)
{
	if (n < 10)
		return '0' + n;
	else
		return 'a' + (n - 10);
}

char NibbleToCharUCase(int n)
{
	if (n < 10)
		return '0' + n;
	else
		return 'A' + (n - 10);
}

char *DataToHexString(const void *input, unsigned int inputlen, char *output)
{
	uint8_t *data = (uint8_t *)input;
	for (unsigned int i = 0; i < inputlen; i++)
	{
		output[(i << 1) + 0] = NibbleToChar((data[i] & 0xF0) >> 4);
		output[(i << 1) + 1] = NibbleToChar((data[i] & 0x0F) >> 0);
	}

	output[inputlen << 1] = '\0';
	return output;
}

bool LoadDataFromFile(const char *filepath, BYTE **data, uint32_t *length)
{
	*data = NULL;
	*length = 0;

	FILE *fp = fopen(filepath, "rb");

	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		long _fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (_fileSize < 0)
			_fileSize = 0;

		uint32_t fileSize = (uint32_t)_fileSize;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		*data = fileData;
		*length = fileSize;
		return true;
	}

	return false;
}

bool LoadDataFromCompressed_(BYTE *input_data, uint32_t input_length, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2)
{
	*data = NULL;
	*length = 0;

	uint32_t packing = magic1;
	for (uint32_t i = 0; i < (input_length >> 2); i++) {
		((uint32_t *)input_data)[i] += ((i + 1) * packing);
		packing ^= magic2;
	}

	uint32_t decompressed_size = *((uint32_t *)input_data);
	BYTE *compressed_buffer = input_data + sizeof(uint32_t);
	uint32_t compressed_size = input_length - sizeof(uint32_t);

	*data = new BYTE[decompressed_size];
	*length = decompressed_size;
	bool success = true;

	uLongf ulf = decompressed_size;

	if (Z_OK != uncompress(*data, &ulf, compressed_buffer, compressed_size))
	{
		delete[](*data);
		*data = NULL;
		*length = 0;

		success = false;
	}
	*length = (uint32_t)ulf;

	return success;
}

bool LoadDataFromCompressedFile(const char *filepath, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2)
{
	*data = NULL;
	*length = 0;

	BYTE *input_data = NULL;
	uint32_t input_length = 0;
	if (LoadDataFromFile(filepath, &input_data, &input_length))
	{
		bool success = LoadDataFromCompressed_(input_data, input_length, data, length, magic1, magic2);
		
		delete[] input_data;

		return success;
	}

	return false;
}

long FindNeedle(void *haystack, unsigned int haystacklength, void *needle, unsigned int needlelength)
{
	if (haystacklength < needlelength)
		return -1;
	if (!needlelength)
		return -1;

	BYTE *dataPos = (BYTE *)haystack;
	BYTE *dataEnd = (dataPos + haystacklength - needlelength);
	bool found = false;

	while (dataPos <= dataEnd)
	{
		if (!memcmp(dataPos, needle, needlelength))
		{
			found = true;
			break;
		}

		dataPos++;
	}

	if (!found)
		return -1;

	return (dataPos - (BYTE *)haystack);
}

// returns a string of "Xmo, Xd, Xh, Xm, Xs"
std::string TimeToString(int seconds)
{
	int mo = seconds / 2629800; // seconds in a month
	int leftover = seconds % 2629800;
	int d = leftover / 86400; // seconds in a day
	leftover %= 86400;
	int h = leftover / 3600; // seconds in an hour
	leftover %= 3600;
	int m = leftover / 60; // seconds in a minute
	leftover %= 60;
	int s = leftover;
	std::string out = "";
	if (mo)
		out += csprintf("%dmo ", mo);
	if (d)
		out += csprintf("%dd ", d);
	if (h)
		out += csprintf("%dh ", h);
	if (m)
		out += csprintf("%dm ", m);
	out += csprintf("%ds", s);
	return out;
}

bool ReplaceString(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

std::string ReplaceInString(std::string subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

static char szReadBuffer[1024];
static char szWriteBuffer[600];

char* csprintf(const char *format, ...)
{
	szReadBuffer[0] = 0;

	va_list args;
	va_start(args, format);
	vsnprintf(szReadBuffer, 1024, format, args);
	va_end(args);

	szReadBuffer[1023] = '\0';

	return szReadBuffer;
}

char *timestamp()
{
	static char result[64];
	tm *beef;
	time_t cake;

	time(&cake);
	beef = localtime(&cake);

	if (beef)
		sprintf(result, "%.2d:%.2d:%.2d", beef->tm_hour, beef->tm_min, beef->tm_sec);
	else
		result[0] = 0;

	return result;
}

char *timestampDateString(time_t ts)
{
	static char result[64];
	tm *beef;
	time_t cake = ts;
	beef = localtime(&cake);

	if (beef)
		sprintf(result, "%d/%02d/%d %02d:%02d:%02d", beef->tm_mon + 1, beef->tm_mday, beef->tm_year + 1900, beef->tm_hour, beef->tm_min, beef->tm_sec);
	else
		result[0] = 0;

	return result;
}

char *timestampDateStringForFileName(time_t ts)
{
	static char result[64];
	tm *beef;
	time_t cake = ts;
	beef = localtime(&cake);

	if (beef)
		sprintf(result, "%04d_%02d_%02d__%02d_%02d_%02d", beef->tm_year + 1900, beef->tm_mon + 1, beef->tm_mday, beef->tm_hour, beef->tm_min, beef->tm_sec);
	else
		result[0] = 0;

	return result;
}

unsigned long ResolveIPFromHost(const char *host)
{
	ULONG ipaddr;

	// Check if it's in IP format.
	ipaddr = ::inet_addr(host);

	if (ipaddr && ipaddr != INADDR_NONE)
		return (unsigned long)ipaddr;

	// Try to resolve an IP address.

	hostent *lphost;
	lphost = gethostbyname(host);

	if (lphost != NULL)
	{
		ipaddr = ((in_addr*)lphost->h_addr)->s_addr;

		if (ipaddr && ipaddr != INADDR_NONE)
			return (unsigned long)ipaddr;
	}

	return 0;
}

unsigned long GetLocalIP()
{
	char hostname[256];
	gethostname(hostname, 256);
	uint32_t hostaddr = *((uint32_t *)gethostbyname(hostname)->h_addr);

	return *(unsigned long *)gethostbyname(hostname)->h_addr;
}

std::string GetLocalIPString()
{
	unsigned long localIP = GetLocalIP();
	return inet_ntoa(*(in_addr *)&localIP);
}

std::string DebugBytesToString(void *_data, unsigned int len)
{
	BYTE *data = (BYTE *)_data;

	std::string strBytes;

	for (unsigned int i = 0; i < len; i++)
	{
		char temp[3];
		sprintf(temp, "%02X", data[i]);
		strBytes += temp;

		if (!((i + 1) % 16))
			strBytes += "\n";
		else
			strBytes += " ";
	}

	if (len % 16)
	{
		strBytes += "\n";
	}

	return strBytes;
}

//void MsgBox(UINT type, const char *format, ...)
//{
//	szWriteBuffer[0] = 0;
//
//	va_list args;
//	va_start(args, format);
//	_vsnprintf(szWriteBuffer, 1024, format, args);
//	va_end(args);
//
//	HWND hWnd = g_pGlobals ? g_pGlobals->GetWindowHandle() : NULL;
//
//	MessageBox(hWnd, szWriteBuffer, "GDL", MB_OK | type);
//}

//void MsgBox(const char *format, ...)
//{
//	szWriteBuffer[0] = 0;
//
//	va_list args;
//	va_start(args, format);
//	_vsnprintf(szWriteBuffer, 1024, format, args);
//	va_end(args);
//
//	MessageBox(NULL, szWriteBuffer, "GDL", MB_OK);
//}

//void MsgBoxError(uint32_t dwError, const char* event)
//{
//	LPVOID lpMsgBuf;
//	if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
//		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		NULL,
//		GetLastError(),
//		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//		(LPTSTR)&lpMsgBuf,
//		0, NULL))
//	{
//		MsgBox(MB_ICONHAND, "Unknown error #%lu %s", dwError, event);
//		return;
//	}
//
//	MsgBox(MB_ICONHAND, "Error #%lu %s:\r\n%s", dwError, event, lpMsgBuf);
//
//	LocalFree(lpMsgBuf);
//}

//#define REGLOC HKEY_LOCAL_MACHINE, "Software\\GDL"

//BOOL SaveConfigKey(const char* Key, uint32_t value)
//{
//	BOOL bReturn = FALSE;
//	HKEY hKey;
//	if (RegCreateKeyEx(REGLOC, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
//	{
//		if (SUCCEEDED(RegSetValueEx(hKey, Key, NULL, REG_uint32_t, (BYTE *)&value, sizeof(uint32_t))))
//			bReturn = TRUE;
//		RegCloseKey(hKey);
//	}
//	return bReturn;
//}

//BOOL SaveConfigKey(const char* Key, const char* value)
//{
//	BOOL bReturn = FALSE;
//	HKEY hKey;
//	if (RegCreateKeyEx(REGLOC, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
//	{
//		if (SUCCEEDED(RegSetValueEx(hKey, Key, NULL, REG_SZ, (BYTE*)value, (uint32_t)strlen(value) + 1)))
//			bReturn = TRUE;
//		RegCloseKey(hKey);
//	}
//	return bReturn;
//}

//BOOL ReadConfigKey(const char* Key, uint32_t* value)
//{
//	uint32_t type = REG_uint32_t;
//	uint32_t size = sizeof(uint32_t);
//	BOOL bReturn = FALSE;
//	HKEY hKey;
//	if (RegOpenKeyEx(REGLOC, NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
//	{
//		if (!RegQueryValueEx(hKey, Key, NULL, &type, (BYTE*)value, &size))
//			bReturn = TRUE;
//		RegCloseKey(hKey);
//	}
//	return bReturn;
//}

//BOOL ReadConfigKey(const char* Key, char* value, uint32_t size)
//{
//	uint32_t type = REG_SZ;
//	BOOL bReturn = FALSE;
//	HKEY hKey;
//	if (RegOpenKeyEx(REGLOC, NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
//	{
//		if (!RegQueryValueEx(hKey, Key, NULL, &type, (BYTE*)value, &size))
//			bReturn = TRUE;
//		RegCloseKey(hKey);
//	}
//	return bReturn;
//}

bool FileExists(const char *filePath)
{
	if (fs::exists(filePath))
		return !fs::is_directory(filePath);
	return false;
	//uint32_t dwAttrib = GetFileAttributes(filePath);
	//return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void strtrim(char *szText)
{
	//how many left spaces are leading?
	int leading = 0;
	while (szText[leading] == ' ')
		leading++;

	if (leading) { //shift left
		int i = leading;
		while (1)
		{
			szText[i - leading] = szText[i];
			if (szText[i] == 0)
				break;
			i++;
		}
	}

	int pos = (int)strlen(szText) - 1; //remove trailing spaces
	while ((pos >= 0) && szText[pos] == ' ')
		szText[pos--] = 0;
}

BOOL strmask(const char* szTest, const char* szMask)
{
	//We need to someday create a function for validating a string mask.
	return FALSE;
}

long fsize(FILE* fileptr)
{
	long lOld, lSize;

	lOld = ftell(fileptr);
	fseek(fileptr, 0, SEEK_END);
	lSize = ftell(fileptr);
	fseek(fileptr, lOld, SEEK_SET);

	return lSize;
}



float NorthSouth(char *szCoord)
{
    std::string coords = szCoord;
    std::transform(coords.begin(), coords.end(), coords.begin(), ::tolower);
    coords.erase(coords.begin(), std::find_if(coords.begin(), coords.end(), [](int c) { return !std::isspace(c); }));
    coords.erase(std::find_if(coords.rbegin(), coords.rend(), [](int c) { return !std::isspace(c); }).base(), coords.end());

    float dir = 1.0f;

    int len = coords.length();
    if (len < 1)
        return 0;

    size_t pos = coords.find(',');
    if (pos == std::string::npos)   // assume at the end
        pos = len;
    pos--;

    char NS = coords[pos];
    if (NS == 's')
        dir = -1.0f;

	coords[pos] = 0;

    float coord = 0.0f;
    sscanf(coords.data(), "%f", &coord);

    return coord * dir;
}

float EastWest(char *szCoord)
{
    std::string coords = szCoord;
    std::transform(coords.begin(), coords.end(), coords.begin(), ::tolower);
    coords.erase(coords.begin(), std::find_if(coords.begin(), coords.end(), [](int c) { return !std::isspace(c); }));
    coords.erase(std::find_if(coords.rbegin(), coords.rend(), [](int c) { return !std::isspace(c); }).base(), coords.end());

    float dir = 1.0f;

    int len = coords.length();
    if (len < 1)
        return 0;

    size_t pos = coords.find(',');
    if (pos == std::string::npos)   // assume at the end
        pos = len;
    pos--;

    char EW = coords[pos];
    if (EW == 'w')
        dir = -1.0f;

	coords[pos] = 0;

    float coord = 0.0f;
	sscanf(coords.data(), "%f", &coord);

    return coord * dir;
}



uint32_t GetCellFromBase(uint32_t BaseX, uint32_t BaseY)
{
	BYTE blockx = (BYTE)(BaseX >> 3);
	BYTE blocky = (BYTE)(BaseY >> 3);
	BYTE cellx = (BYTE)(BaseX & 7);
	BYTE celly = (BYTE)(BaseY & 7);

	WORD block = (blockx << 8) | (blocky);
	WORD cell = (cellx << 3) | (celly);

	uint32_t dwCell = (block << 16) | (cell + 1);
	return dwCell;
}

bool GetLocation(double NS, double EW, Position &pos)
{
	NS -= 0.5f;
	EW -= 0.5f;
	NS *= 10.0f;
	EW *= 10.0f;

	uint32_t basex = (uint32_t)(EW + 0x400);
	uint32_t basey = (uint32_t)(NS + 0x400);

	if (int32_t(basex) < 0 || int32_t(basey) < 0 || basex >= 0x7F8 || basey >= 0x7F8)
	{
		//Out of bounds.
		pos = Position();
		return false;
	}

	uint32_t dwCell = GetCellFromBase(basex, basey);
	float xOffset = ((basex & 7) * 24.0f) + 12;
	float yOffset = ((basey & 7) * 24.0f) + 12;

	pos = Position(dwCell, Vector(xOffset, yOffset, CalcSurfaceZ(dwCell, xOffset, yOffset)));
	return true;
}

// Water blocks are not passable!
BOOL IsWaterBlock(BlockData *pBlock)
{
	for (unsigned int x = 0; x < 9; x++) {
		for (unsigned int y = 0; y < 9; y++) {
			if ((pBlock->wSurface[x][y] & 0x50) != 0x50)
				return FALSE;
		}
	}

	return TRUE;
}

//
BOOL IsWaterBlock(uint32_t dwCell)
{
	dwCell &= 0xFFFF0000;
	dwCell |= 0x0000FFFF;

	TURBINEFILE *pFile = g_pCell->GetFile(dwCell);

	if (!pFile)
		return FALSE;

	BOOL bResult = IsWaterBlock((BlockData*)pFile->GetData());
	delete pFile;

	return bResult;
}

BOOL HasObjectBlock(BlockData *pBlock)
{
	return ((pBlock->bObject) ? TRUE : FALSE);
}

BOOL HasObjectBlock(uint32_t dwCell)
{
	dwCell &= 0xFFFF0000;
	dwCell |= 0x0000FFFF;

	TURBINEFILE *pFile = g_pCell->GetFile(dwCell);

	if (!pFile)
		return FALSE;

	BOOL bResult = HasObjectBlock((BlockData*)pFile->GetData());
	delete pFile;

	return bResult;
}

float CalcSurfaceZ(uint32_t dwCell, float xOffset, float yOffset, bool bUseLCell)
{
	TURBINEFILE *pFile = g_pCell->GetFile((dwCell & 0xFFFF0000) | 0xFFFF);
	if (!pFile)
		return 0;

	WORD cell = CELL_WORD(dwCell);
	int minix = (cell >> 3) & 7;
	int miniy = (cell >> 0) & 7;
	if (bUseLCell)
	{
		xOffset -= (24 * minix);
		yOffset -= (24 * miniy);
	}

	BlockData *pBlock = (BlockData*)pFile->GetData();

	Vector P1(0, 24, LandDefs::Land_Height_Table[pBlock->bHeight[minix][miniy + 1]] + 0.005f);
	Vector P2;
	Vector P3(24, 0, LandDefs::Land_Height_Table[pBlock->bHeight[minix + 1][miniy]] + 0.005f);

	if ((xOffset + yOffset) < 24.0f)
		P2 = Vector(0, 0, LandDefs::Land_Height_Table[pBlock->bHeight[minix][miniy]] + 0.005f);
	else
		P2 = Vector(24, 24, LandDefs::Land_Height_Table[pBlock->bHeight[minix + 1][miniy + 1]] + 0.005f);

	delete pFile;

	return FindVectorZ(P1, P2, P3, xOffset, yOffset);
}

uint64_t GetProcessMemoryUsage()
{
#if defined(_WIN32)
	PROCESS_MEMORY_COUNTERS_EX procMem;
	memset(&procMem, 0, sizeof(procMem));
	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&procMem, sizeof(procMem));
	return procMem.PrivateUsage;
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    uint64_t result = 0;
    FILE *fp = NULL;
    if ((fp = fopen("/proc/self/statm", "r")))
    {
        fscanf(fp, "%lu", &result);
        fclose(fp);
    }
    return result * (uint64_t)sysconf(_SC_PAGESIZE);
#else
    return 0;
#endif
}

uint64_t GetFreeMemory()
{
#if defined(_WIN32)
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullAvailPhys;
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    struct sysinfo si = { 0 };
    sysinfo(&si);
    
    return si.freeram;
#else
    return 0;
#endif
}

uint64_t GetTotalMemory()
{
#if defined(_WIN32)
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    struct sysinfo si = { 0 };
    sysinfo(&si);
    
    return si.totalram;
#else
    return 0;
#endif
}

// strictness 1 = chat, strictness 2 = player names, titles, allegiance names
bool containsBadCharacters(std::string input, int strictness) 
{
	std::string goodChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'- ";
	if (strictness == 1)
	{
		goodChars += "!\"?$%^&*()_-+={}[]:;<>,.?/@'~#`?|\\";
	}

	for (auto c : input)
	{
		if (goodChars.find(c) != goodChars.npos)
		{
			continue;
		}
		else
			return true;
	}
	return false;
}

float GetRatingMod(int ratingAdj)
{
	if (ratingAdj > 0)
	{
		return (100 + (float)ratingAdj) / 100;
	}
	else if (ratingAdj < 0)
	{
		return 100 / (100 + (float)ratingAdj);
	}

	return 1.0;
}
