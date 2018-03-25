
#pragma once

#if PHATSDK_INCLUDE_PHATDATABIN

void InitPhatDataBin(const char *path);
void CleanupPhatDataBin();
bool LoadDataFromPhatDataBin(DWORD data_id, BYTE **data, DWORD *length, DWORD magic1, DWORD magic2);

class CPhatDataBinData : public PackObj
{
public:
	virtual ~CPhatDataBinData() override { }

	virtual bool RetrieveData(BYTE **data, DWORD *length) = 0;
	virtual bool StoreData(BYTE *data, DWORD length) = 0;
};

class CPhatDataBinCompressedData : public CPhatDataBinData
{
public:
	virtual ~CPhatDataBinCompressedData() override;

	void Destroy();

	virtual bool RetrieveData(BYTE **data, DWORD *length) override;
	virtual bool StoreData(BYTE *data, DWORD length) override;

	DECLARE_PACKABLE()

	DWORD _compressedLength = 0;
	BYTE *_compressedData = NULL;
	DWORD _uncompressedLength = 0;
};

class CPhatDataBinEntry : public PackObj
{
public:
	virtual ~CPhatDataBinEntry() override;

	void Destroy();

	virtual bool RetrieveData(BYTE **data, DWORD *length);
	virtual bool StoreData(BYTE *data, DWORD length);

	DECLARE_PACKABLE()

	DWORD _fileID = 0;
	DWORD _magic1 = 0;
	DWORD _magic2 = 0;
	DWORD _magic3 = 0;
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

	bool Add(DWORD data_id, CPhatDataBinEntry *entry);
	bool Add(DWORD data_id, BYTE *data, DWORD length);
	bool Add(DWORD data_id, const char *filepath);
	bool Remove(DWORD data_id);

	bool Get(DWORD data_id, BYTE **data, DWORD *length);

protected:
	std::map<DWORD, CPhatDataBinEntry *> _entries;
};

#endif
