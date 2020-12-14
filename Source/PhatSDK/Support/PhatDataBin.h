
#pragma once

#include <filesystem>

#if PHATSDK_INCLUDE_PHATDATABIN

void InitPhatDataBin(const char *path);
void CleanupPhatDataBin();
bool LoadDataFromPhatDataBin(uint32_t data_id, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2);

class CPhatDataBinData : public PackObj
{
public:
	virtual ~CPhatDataBinData() override { }

	virtual bool RetrieveData(BYTE **data, uint32_t *length) = 0;
	virtual bool StoreData(BYTE *data, uint32_t length) = 0;
};

class CPhatDataBinCompressedData : public CPhatDataBinData
{
public:
	virtual ~CPhatDataBinCompressedData() override;

	void Destroy();

	virtual bool RetrieveData(BYTE **data, uint32_t *length) override;
	virtual bool StoreData(BYTE *data, uint32_t length) override;

	DECLARE_PACKABLE()

	uint32_t _compressedLength = 0;
	BYTE *_compressedData = NULL;
	uint32_t _uncompressedLength = 0;
};

class CPhatDataBinEntry : public PackObj
{
public:
	virtual ~CPhatDataBinEntry() override;

	void Destroy();

	virtual bool RetrieveData(BYTE **data, uint32_t *length);
	virtual bool StoreData(BYTE *data, uint32_t length);

	DECLARE_PACKABLE()

	uint32_t _fileID = 0;
	uint32_t _magic1 = 0;
	uint32_t _magic2 = 0;
	uint32_t _magic3 = 0;
	CPhatDataBinData *_data = NULL;
};

class CPhatDataBin
{
public:
	CPhatDataBin();
	virtual ~CPhatDataBin();
	
	void Destroy();
	bool Load(const char *filepath);
	bool Save(const char *filepath);

	bool Add(uint32_t data_id, CPhatDataBinEntry *entry);
	bool Add(uint32_t data_id, BYTE *data, uint32_t length);
	bool Add(uint32_t data_id, const char *filepath);
	bool Remove(uint32_t data_id);

	bool Get(uint32_t data_id, BYTE **data, uint32_t *length);

protected:
	std::map<uint32_t, CPhatDataBinEntry *> _entries;
};

class InferredData
{
protected:

	using load_func_t = std::function<void(json&)>;

	bool LoadJsonData(fs::path path, load_func_t cb);
	bool LoadJsonData(fs::path path, PackableJson &data);
	bool LoadCacheData(uint32_t id, uint32_t magic1, uint32_t magic2, PackObj &data);
	void SaveJsonData(fs::path path, PackableJson &data);
};

#endif
