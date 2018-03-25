
#include "StdAfx.h"
#include "PhatDataBin.h"

#if PHATSDK_INCLUDE_PHATDATABIN

CPhatDataBin *g_pPhatDataBin = NULL;

void InitPhatDataBin(const char *path)
{
	g_pPhatDataBin = new CPhatDataBin();
	g_pPhatDataBin->Load(path);
}

void CleanupPhatDataBin()
{
	SafeDelete(g_pPhatDataBin);
}

bool LoadDataFromCompressed(BYTE *input_data, DWORD input_length, BYTE **data, DWORD *length, DWORD magic1, DWORD magic2)
{
	*data = NULL;
	*length = 0;

	DWORD packing = magic1;
	for (DWORD i = 0; i < (input_length >> 2); i++) {
		((DWORD *)input_data)[i] += ((i + 1) * packing);
		packing ^= magic2;
	}

	DWORD decompressed_size = *((DWORD *)input_data);
	BYTE *compressed_buffer = input_data + sizeof(DWORD);
	DWORD compressed_size = input_length - sizeof(DWORD);

	*data = new BYTE[decompressed_size];
	*length = decompressed_size;
	bool success = true;

	if (Z_OK != uncompress(*data, length, compressed_buffer, compressed_size))
	{
		delete[](*data);
		*data = NULL;
		*length = 0;

		success = false;
	}

	return success;
}

bool LoadDataFromPhatDataBin(DWORD data_id, BYTE **data, DWORD *length, DWORD magic1, DWORD magic2)
{
	if (!g_pPhatDataBin)
		return false;

	BYTE *input_data;
	DWORD input_length;
	if (g_pPhatDataBin->Get(data_id, &input_data, &input_length))
	{
		bool success = LoadDataFromCompressed(input_data, input_length, data, length, magic1, magic2);

		delete[] input_data;

		return success;
	}

	return false;
}

CPhatDataBinCompressedData::~CPhatDataBinCompressedData()
{
	Destroy();
}

void CPhatDataBinCompressedData::Destroy()
{
	SafeDeleteArray(_compressedData);
	_compressedLength = 0;
	_uncompressedLength = 0;
}

bool CPhatDataBinCompressedData::RetrieveData(BYTE **data, DWORD *length)
{
	*data = NULL;
	*length = 0;

	if (!_compressedData || !_compressedLength)
	{
		return false;
	}

	*data = new BYTE[_uncompressedLength];
	*length = _uncompressedLength;
	bool success = true;

	if (Z_OK != uncompress(*data, length, _compressedData, _compressedLength))
	{
		delete[] (*data);
		*data = NULL;
		*length = 0;

		return false;
	}

	return true;
}

bool CPhatDataBinCompressedData::StoreData(BYTE *data, DWORD length)
{
	Destroy();

	if (!data || !length)
	{
		return true;
	}

	_uncompressedLength = length;
	_compressedLength = (DWORD)((length * 1.02f) + 12 + 1);
	_compressedData = new BYTE[_compressedLength];

	if (Z_OK != compress2(_compressedData, &_compressedLength, data, length, Z_BEST_COMPRESSION))
	{
		Destroy();
		return false;
	}

	return true;
}

DEFINE_PACK(CPhatDataBinCompressedData)
{
	pWriter->Write<DWORD>(_compressedLength);
	pWriter->Write(_compressedData, _compressedLength);
	pWriter->Write<DWORD>(_uncompressedLength);
}

DEFINE_UNPACK(CPhatDataBinCompressedData)
{
	Destroy();

	_compressedLength = pReader->Read<DWORD>();
	_compressedData = new BYTE[_compressedLength];
	memcpy(_compressedData, pReader->ReadArray(_compressedLength), _compressedLength);
	_uncompressedLength = pReader->Read<DWORD>();
	return true;
}

CPhatDataBinEntry::~CPhatDataBinEntry()
{
	Destroy();
}

void CPhatDataBinEntry::Destroy()
{
	_fileID = 0;
	_magic1 = 0;
	_magic2 = 0;
	_magic3 = 0;
	SafeDelete(_data);
}

bool CPhatDataBinEntry::RetrieveData(BYTE **data, DWORD *length)
{
	if (!_data || !_data->RetrieveData(data, length))
		return NULL;

	DWORD packing = _magic1;
	for (DWORD i = 0; i < (*length >> 2); i++) {
		packing ^= _magic2;
		packing += _magic3;
		((DWORD *)*data)[i] += ((i + 1) * packing);
	}

	return true;
}

bool CPhatDataBinEntry::StoreData(BYTE *data, DWORD length)
{
	if (_data)
	{
		delete _data;
	}

	_data = new CPhatDataBinCompressedData();

	BYTE *dataCopy = new BYTE[length];
	memcpy(dataCopy, data, length);

	DWORD packing = _magic1;
	for (DWORD i = 0; i < (length >> 2); i++) {
		packing ^= _magic2;
		packing += _magic3;
		((DWORD *)dataCopy)[i] -= ((i + 1) * packing);
	}

	bool success = _data->StoreData(dataCopy, length);

	delete [] dataCopy;
	return success;
}

DEFINE_PACK(CPhatDataBinEntry)
{
	pWriter->Write<DWORD>(_fileID);
	pWriter->Write<DWORD>(_magic1);
	pWriter->Write<DWORD>(_magic2);
	pWriter->Write<DWORD>(_magic3);

	if (_data)
	{
		pWriter->Write<BYTE>(1);
		_data->Pack(pWriter);
	}
	else
	{
		pWriter->Write<BYTE>(0);
	}
}

DEFINE_UNPACK(CPhatDataBinEntry)
{
	Destroy();

	_fileID = pReader->Read<DWORD>();
	_magic1 = pReader->Read<DWORD>();
	_magic2 = pReader->Read<DWORD>();
	_magic3 = pReader->Read<DWORD>();

	BYTE dataType = pReader->Read<BYTE>();
	if (dataType == 1)
	{
		_data = new CPhatDataBinCompressedData();
		_data->UnPack(pReader);
	}

	return true;
}

CPhatDataBin::CPhatDataBin()
{
}

CPhatDataBin::~CPhatDataBin()
{
	Destroy();
}

void CPhatDataBin::Destroy()
{
	for (auto &entry : _entries)
	{
		delete entry.second;
	}

	_entries.clear();
}

bool CPhatDataBin::Add(DWORD data_id, CPhatDataBinEntry *entry)
{
	auto mapEntry = _entries.find(data_id);

	if (mapEntry != _entries.end())
	{
		return false;
	}

	_entries[data_id] = entry;
	return true;
}

bool CPhatDataBin::Add(DWORD data_id, BYTE *data_, DWORD length)
{
	CPhatDataBinEntry *entry = new CPhatDataBinEntry;

	entry->_fileID = data_id;

	// using rand() is not ideal
	entry->_magic1 = rand() + (rand() << 16);
	entry->_magic2 = rand() + (rand() << 16);
	entry->_magic3 = rand() + (rand() << 16);

	entry->StoreData(data_, length);

	if (!Add(data_id, entry))
	{
		delete entry;
		return false;
	}

	return true;
}

bool CPhatDataBin::Add(DWORD data_id, const char *filepath)
{
	if (FILE *fp = fopen(filepath, "rb"))
	{
		fseek(fp, 0, SEEK_END);
		long fileSize_ = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (fileSize_ < 0)
			fileSize_ = 0;

		DWORD fileSize = (DWORD)fileSize_;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		return Add(data_id, fileData, fileSize);
	}

	return false;
}


bool CPhatDataBin::Remove(DWORD data_id)
{
	auto mapEntry = _entries.find(data_id);

	if (mapEntry != _entries.end())
	{
		delete mapEntry->second;
		_entries.erase(mapEntry);
		return true;
	}

	return false;
}


bool CPhatDataBin::Get(DWORD data_id, BYTE **data, DWORD *length)
{
	auto mapEntry = _entries.find(data_id);

	if (mapEntry != _entries.end())
	{
		return mapEntry->second->RetrieveData(data, length);
	}

	return false;
}

bool CPhatDataBin::Load(const char *filepath)
{
	Destroy();

	if (FILE *fp = fopen(filepath, "rb"))
	{
		fseek(fp, 0, SEEK_END);
		long fileSize_ = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (fileSize_ < 0)
			fileSize_ = 0;

		DWORD fileSize = (DWORD)fileSize_;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		DWORD xor_val = fileSize + (fileSize << 15);
		for (DWORD i = 0; i < (fileSize >> 2); i++)
		{
			((DWORD *)fileData)[i] ^= xor_val;
			xor_val <<= 3;
			xor_val += fileSize;
		}

		BinaryReader reader(fileData, fileSize);

		DWORD version = reader.Read<DWORD>();
		DWORD numEntries = reader.Read<DWORD>();

		bool fail = false;

		for (DWORD i = 0; i < numEntries; i++)
		{
			DWORD data_id = reader.Read<DWORD>();

			CPhatDataBinEntry *entry = new CPhatDataBinEntry();
			if (!entry->UnPack(&reader))
			{
				fail = true;
				delete entry;

				break;
			}

			Remove(data_id);
			Add(data_id, entry);
		}

		delete [] fileData;

		if (fail)
		{
			Destroy();
			return false;
		}

		return true;
	}

	return false;
}

bool CPhatDataBin::Save(const char *filepath)
{
	if (FILE *fp = fopen(filepath, "wb"))
	{
		BinaryWriter writer;

		writer.Write<DWORD>(1);
		writer.Write<DWORD>(_entries.size());

		for (auto &entry : _entries)
		{
			writer.Write<DWORD>(entry.first);
			entry.second->Pack(&writer);
		}

		BYTE *fileData = writer.GetData();
		DWORD fileSize = writer.GetSize();

		DWORD xor_val = fileSize + (fileSize << 15);
		for (DWORD i = 0; i < (fileSize >> 2); i++)
		{
			((DWORD *)fileData)[i] ^= xor_val;
			xor_val <<= 3;
			xor_val += fileSize;
		}

		fwrite(fileData, fileSize, 1, fp);
		fclose(fp);
		return true;
	}

	return false;
}


#endif