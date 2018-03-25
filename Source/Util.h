
#pragma once

namespace Random
{
	extern int m_iSeed;

	void Init();
	uint32_t GenRandom15();
	uint32_t GenRandom32();

	uint32_t GenUInt(uint32_t min, uint32_t max);
	int32_t GenInt(int32_t min, int32_t max);
	double GenFloat(double min, double max);
};

class PhatString : public std::string
{
public:
	PhatString() { }

	void TrimLeft(int numChars)
	{
		int currentLength = (int)length();
		if (numChars > currentLength)
		{
			numChars = currentLength;
		}

		PhatString original = *this;
		*(std::string *)this = original.c_str() + numChars;
	}

	bool IsEqual(const PhatString& other, bool caseSensitive)
	{
		if (caseSensitive)
		{
			return !strcmp(c_str(), other.c_str());
		}

		return !_stricmp(c_str(), other.c_str());
	}

	PhatString AsUppercase()
	{
		PhatString result = *this;
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		return result;
	}

	PhatString AsLowercase()
	{
		PhatString result = *this;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		return result;
	}

	void ToUppercase()
	{
		std::transform(begin(), end(), begin(), ::toupper);
	}

	void ToLowercase()
	{
		std::transform(begin(), end(), begin(), ::tolower);
	}
};

class CLockable
{
public:
	CLockable()
	{
		InitializeCriticalSection(&_cs);
	}

	~CLockable()
	{
		DeleteCriticalSection(&_cs);
	}

	inline void Lock()
	{
		EnterCriticalSection(&_cs);
	}

	inline void Unlock()
	{
		LeaveCriticalSection(&_cs);
	}

private:
	CRITICAL_SECTION _cs;
};


template <class T>
class TLockable : public T
{
public:
	TLockable()
	{
		InitializeCriticalSection(&_cs);
	}

	virtual ~TLockable()
	{
		DeleteCriticalSection(&_cs);
	}

	inline void Lock()
	{
		EnterCriticalSection(&_cs);
	}

	inline void Unlock()
	{
		LeaveCriticalSection(&_cs);
	}

private:
	CRITICAL_SECTION _cs;
};

class CScopedLock
{
public:
	inline CScopedLock(CLockable *lock)
	{
		_lock = lock;
		_lock->Lock();
	}

	inline ~CScopedLock()
	{
		_lock->Unlock();
	}

private:
	CLockable *_lock;
};

class CClient;
struct BlockData;

void EnumerateFolderFilePaths(std::list<std::string> &results, const char *basePath, const char *fileMask, bool bSubFolders);
char *DataToHexString(const void *input, unsigned int inputlen, char *output);
bool LoadDataFromFile(const char *filepath, BYTE **data, DWORD *length);
bool LoadDataFromPhatDataBin(DWORD data_id, BYTE **data, DWORD *length, DWORD magic1, DWORD magic2);
bool LoadDataFromCompressedFile(const char *filepath, BYTE **data, DWORD *length, DWORD magic1, DWORD magic2);
long FindNeedle(void *haystack, unsigned int haystacklength, void *needle, unsigned int needlelength);
bool ReplaceString(std::string& str, const std::string& from, const std::string& to);

extern void MsgBox(const char* format, ...);
extern void MsgBox(UINT iType, const char* format, ...);
extern void MsgBoxError(DWORD dwError, const char* event);

#define cs(x) csprintf(x)
extern char* csprintf(const char* format, ...); //static buffer
extern char* timestamp(); //static buffer
extern char *timestampDateString(time_t ts);
extern char *timestampDateStringForFileName(time_t ts);
extern void strtrim(char *szText); //specified buffer
extern BOOL strmask(const char* szTest, const char* szMask);
extern long fsize(FILE* fp); //returns a FILE* size

unsigned long ResolveIPFromHost(const char *host);

unsigned long GetLocalIP();
std::string GetLocalIPString();

extern std::string DebugBytesToString(void *data, unsigned int len);
extern void _OutputConsole(const char* format, ...);
#define _DebugMe() LOG(Temp, Normal, "Debug me: %s %u\n", __FUNCTION__, __LINE__);

extern BOOL SaveConfigKey(const char* Key, DWORD value);
extern BOOL SaveConfigKey(const char* Key, const char* value);
extern BOOL ReadConfigKey(const char* Key, DWORD* value);
extern BOOL ReadConfigKey(const char* Key, char* value, DWORD size);

extern bool FileExists(const char *filePath);

extern float NorthSouth(char *szCoord);
extern float EastWest(char *szCoord);
extern bool GetLocation(double NS, double EW, class Position &pos);
extern BOOL IsWaterBlock(BlockData*);
extern BOOL IsWaterBlock(DWORD dwCell);
extern BOOL HasObjectBlock(BlockData*);
extern BOOL HasObjectBlock(DWORD dwCell);
extern float CalcSurfaceZ(DWORD dwCell, float xOffset, float yOffset, bool bUseLCell = true);

DWORD64 GetProcessMemoryUsage();
DWORD64 GetFreeMemory();
DWORD64 GetTotalMemory();

// Because the land system sucks.
#define BLOCK_WORD(x) ((WORD)((x & 0xFFFF0000) >> 16))
#define BLOCK_X(x) ((BYTE)((x >> 8) & 0xFF))
#define BLOCK_Y(x) ((BYTE)((x >> 0) & 0xFF))
#define CELL_WORD(x) ((WORD)((x & 0xFFFF)-1))
#define CELL_X(x) ((BYTE)((x >> 3) & 7))
#define CELL_Y(x) ((BYTE)((x >> 0) & 7))

// Used for mapping out the above data to offsets from (0, 0).
#define BASE_OFFSET(X, x) (((DWORD)X << 3) | x)
#define BLOCK_OFFSET(x) ((WORD)((DWORD)x >> 3))

// Potential view cell-range.
#define PVC_RANGE 8

// Land boundaries.
const DWORD dwMinimumCellX = 0x0;
const DWORD dwMinimumCellY = 0x0;
const DWORD dwMaximumCellX = (0x800 - 1);
const DWORD dwMaximumCellY = (0x800 - 1);
