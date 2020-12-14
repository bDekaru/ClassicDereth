#pragma once

#include <mutex>

namespace Random
{
	extern int m_iSeed;

	void Init();
	uint32_t GenRandom15();
	uint32_t GenRandom32();

	uint32_t GenUInt(uint32_t min, uint32_t max);
	int32_t GenInt(int32_t min, int32_t max);
	double GenFloat();
	double GenFloat(double min, double max);
	float GenFloat(float min, float max);

	float RollDice(float min, float max);

	extern uint32_t currentSeed;
	void SetSeed(uint32_t newSeed);
	uint32_t GetSeed();
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

		return !stricmp(c_str(), other.c_str());
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

//#define SCOPE_LOCK std::scoped_lock lock##__COUNTER__(this->mutex);
#define SCOPE_LOCK std::scoped_lock<std::recursive_mutex>& scope_lock();
class CLockable
{
public:
	CLockable() = default;
	virtual ~CLockable() = default;

	inline void Lock()
	{
		mutex.lock();
	}

	inline void Unlock()
	{
		mutex.unlock();
	}

	__forceinline const std::scoped_lock<std::recursive_mutex> scope_lock()
	{
		return std::scoped_lock(mutex);
	}

private:
	std::recursive_mutex mutex;
};


template <class T>
class TLockable : public T, public CLockable
{
public:
	TLockable() = default;
	virtual ~TLockable() = default;
};

class ThreadedFileLoader
{
protected:
	using file_load_func_t = std::function<void(fs::path)>;
	void PerformLoad(fs::path root, file_load_func_t loadfn)
	{
		if (!fs::exists(root) || !fs::is_directory(root))
			return;

		auto itr = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied);
		const auto end = fs::recursive_directory_iterator();

		std::list< std::future<void> > tasks;

		uint32_t maxTasks = std::thread::hardware_concurrency();

		while (itr != end)
		{
			fs::path path = itr->path();
			if (!itr->is_directory() && path.extension().compare(".json") == 0)
			{
				tasks.push_back(std::async(std::launch::async, [path, loadfn]()
				{
					loadfn(path);
				}));
			}

			try
			{
				itr++;
			}
			catch (std::exception &ex)
			{
			}

			while (tasks.size() > maxTasks)
			{
				auto task = tasks.begin();
				while (task != tasks.end())
				{
					auto status = task->wait_for(std::chrono::milliseconds(0));
					if (status == std::future_status::ready)
					{
						task = tasks.erase(task);
					}
					else
					{
						task++;
					}
				}
				std::this_thread::yield();
			}
		}

		while (tasks.size() > 0)
		{
			tasks.front().wait();
			tasks.pop_front();
		}

	}
};

class CClient;
struct BlockData;

//void EnumerateFolderFilePaths(std::list<std::string> &results, const char *basePath, const char *fileMask, bool bSubFolders);
char *DataToHexString(const void *input, unsigned int inputlen, char *output);
bool LoadDataFromFile(const char *filepath, BYTE **data, uint32_t *length);
bool LoadDataFromPhatDataBin(uint32_t data_id, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2);
bool LoadDataFromCompressedFile(const char *filepath, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2);
long FindNeedle(void *haystack, unsigned int haystacklength, void *needle, unsigned int needlelength);
std::string TimeToString(int seconds);
bool ReplaceString(std::string& str, const std::string& from, const std::string& to);
std::string ReplaceInString(std::string subject, const std::string& search, const std::string& replace);

//extern void MsgBox(const char* format, ...);
//extern void MsgBox(UINT iType, const char* format, ...);
//extern void MsgBoxError(uint32_t dwError, const char* event);

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
//extern void _OutputConsole(const char* format, ...);
#define _DebugMe() LOG(Temp, Normal, "Debug me: %s %u\n", __FUNCTION__, __LINE__);

//extern BOOL SaveConfigKey(const char* Key, uint32_t value);
//extern BOOL SaveConfigKey(const char* Key, const char* value);
//extern BOOL ReadConfigKey(const char* Key, uint32_t* value);
//extern BOOL ReadConfigKey(const char* Key, char* value, uint32_t size);

extern bool FileExists(const char *filePath);

extern float NorthSouth(char *szCoord);
extern float EastWest(char *szCoord);
extern bool GetLocation(double NS, double EW, class Position &pos);
extern BOOL IsWaterBlock(BlockData*);
extern BOOL IsWaterBlock(uint32_t dwCell);
extern BOOL HasObjectBlock(BlockData*);
extern BOOL HasObjectBlock(uint32_t dwCell);
extern float CalcSurfaceZ(uint32_t dwCell, float xOffset, float yOffset, bool bUseLCell = true);

float GetRatingMod(int ratingAdj);

uint64_t GetProcessMemoryUsage();
uint64_t GetFreeMemory();
uint64_t GetTotalMemory();

// Because the land system sucks.
#define BLOCK_WORD(x) ((WORD)((x & 0xFFFF0000) >> 16))
#define BLOCK_X(x) ((BYTE)((x >> 8) & 0xFF))
#define BLOCK_Y(x) ((BYTE)((x >> 0) & 0xFF))
#define CELL_WORD(x) ((WORD)((x & 0xFFFF)-1))
#define CELL_X(x) ((BYTE)((x >> 3) & 7))
#define CELL_Y(x) ((BYTE)((x >> 0) & 7))

// Used for mapping out the above data to offsets from (0, 0).
#define BASE_OFFSET(X, x) (((uint32_t)X << 3) | x)
#define BLOCK_OFFSET(x) ((WORD)((uint32_t)x >> 3))

// Potential view cell-range.
#define PVC_RANGE 8

// Land boundaries.
const uint32_t dwMinimumCellX = 0x0;
const uint32_t dwMinimumCellY = 0x0;
const uint32_t dwMaximumCellX = (0x800 - 1);
const uint32_t dwMaximumCellY = (0x800 - 1);

// strictness 1 = chat, strictness 2 = player names, titles, allegiance names
extern bool containsBadCharacters(std::string, int strictness = 1);
