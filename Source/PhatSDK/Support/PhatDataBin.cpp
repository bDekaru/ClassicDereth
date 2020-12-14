
#include <StdAfx.h>
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

bool LoadDataFromCompressed(BYTE *input_data, uint32_t input_length, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2)
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

bool LoadDataFromPhatDataBin(uint32_t data_id, BYTE **data, uint32_t *length, uint32_t magic1, uint32_t magic2)
{
	if (!g_pPhatDataBin)
		return false;

	BYTE *input_data;
	uint32_t input_length;
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

bool CPhatDataBinCompressedData::RetrieveData(BYTE **data, uint32_t *length)
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

	uLongf ulf = _uncompressedLength;

	if (Z_OK != uncompress(*data, &ulf, _compressedData, _compressedLength))
	{
		delete[] (*data);
		*data = NULL;
		*length = 0;

		return false;
		*length = (uint32_t)ulf;
	}

	return true;
}

bool CPhatDataBinCompressedData::StoreData(BYTE *data, uint32_t length)
{
	Destroy();

	if (!data || !length)
	{
		return true;
	}

	_uncompressedLength = length;
	_compressedLength = (uint32_t)((length * 1.02f) + 12 + 1);
	_compressedData = new BYTE[_compressedLength];

	uLongf ulf = _compressedLength;

	if (Z_OK != compress2(_compressedData, &ulf, data, length, Z_BEST_COMPRESSION))
	{
		Destroy();
		return false;
	}
	_compressedLength = (uint32_t)ulf;

	return true;
}

DEFINE_PACK(CPhatDataBinCompressedData)
{
	pWriter->Write<uint32_t>(_compressedLength);
	pWriter->Write(_compressedData, _compressedLength);
	pWriter->Write<uint32_t>(_uncompressedLength);
}

DEFINE_UNPACK(CPhatDataBinCompressedData)
{
	Destroy();

	_compressedLength = pReader->Read<uint32_t>();
	_compressedData = new BYTE[_compressedLength];
	memcpy(_compressedData, pReader->ReadArray(_compressedLength), _compressedLength);
	_uncompressedLength = pReader->Read<uint32_t>();
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

bool CPhatDataBinEntry::RetrieveData(BYTE **data, uint32_t *length)
{
	if (!_data || !_data->RetrieveData(data, length))
		return NULL;

	uint32_t packing = _magic1;
	for (uint32_t i = 0; i < (*length >> 2); i++) {
		packing ^= _magic2;
		packing += _magic3;
		((uint32_t *)*data)[i] += ((i + 1) * packing);
	}

	return true;
}

bool CPhatDataBinEntry::StoreData(BYTE *data, uint32_t length)
{
	if (_data)
	{
		delete _data;
	}

	_data = new CPhatDataBinCompressedData();

	BYTE *dataCopy = new BYTE[length];
	memcpy(dataCopy, data, length);

	uint32_t packing = _magic1;
	for (uint32_t i = 0; i < (length >> 2); i++) {
		packing ^= _magic2;
		packing += _magic3;
		((uint32_t *)dataCopy)[i] -= ((i + 1) * packing);
	}

	bool success = _data->StoreData(dataCopy, length);

	delete [] dataCopy;
	return success;
}

DEFINE_PACK(CPhatDataBinEntry)
{
	pWriter->Write<uint32_t>(_fileID);
	pWriter->Write<uint32_t>(_magic1);
	pWriter->Write<uint32_t>(_magic2);
	pWriter->Write<uint32_t>(_magic3);

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

	_fileID = pReader->Read<uint32_t>();
	_magic1 = pReader->Read<uint32_t>();
	_magic2 = pReader->Read<uint32_t>();
	_magic3 = pReader->Read<uint32_t>();

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

bool CPhatDataBin::Add(uint32_t data_id, CPhatDataBinEntry *entry)
{
	auto mapEntry = _entries.find(data_id);

	if (mapEntry != _entries.end())
	{
		return false;
	}

	_entries[data_id] = entry;
	return true;
}

bool CPhatDataBin::Add(uint32_t data_id, BYTE *data_, uint32_t length)
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

bool CPhatDataBin::Add(uint32_t data_id, const char *filepath)
{
	if (FILE *fp = fopen(filepath, "rb"))
	{
		fseek(fp, 0, SEEK_END);
		long fileSize_ = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (fileSize_ < 0)
			fileSize_ = 0;

		uint32_t fileSize = (uint32_t)fileSize_;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		return Add(data_id, fileData, fileSize);
	}

	return false;
}


bool CPhatDataBin::Remove(uint32_t data_id)
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


bool CPhatDataBin::Get(uint32_t data_id, BYTE **data, uint32_t *length)
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

		uint32_t fileSize = (uint32_t)fileSize_;

		BYTE *fileData = new BYTE[fileSize];
		fread(fileData, fileSize, 1, fp);
		fclose(fp);

		uint32_t xor_val = fileSize + (fileSize << 15);
		for (uint32_t i = 0; i < (fileSize >> 2); i++)
		{
			((uint32_t *)fileData)[i] ^= xor_val;
			xor_val <<= 3;
			xor_val += fileSize;
		}

		BinaryReader reader(fileData, fileSize);

		uint32_t version = reader.Read<uint32_t>();
		uint32_t numEntries = reader.Read<uint32_t>();

		bool fail = false;

		for (uint32_t i = 0; i < numEntries; i++)
		{
			uint32_t data_id = reader.Read<uint32_t>();

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

		writer.Write<uint32_t>(1);
		writer.Write<uint32_t>(_entries.size());

		for (auto &entry : _entries)
		{
			writer.Write<uint32_t>(entry.first);
			entry.second->Pack(&writer);
		}

		BYTE *fileData = writer.GetData();
		uint32_t fileSize = writer.GetSize();

		uint32_t xor_val = fileSize + (fileSize << 15);
		for (uint32_t i = 0; i < (fileSize >> 2); i++)
		{
			((uint32_t *)fileData)[i] ^= xor_val;
			xor_val <<= 3;
			xor_val += fileSize;
		}

		fwrite(fileData, fileSize, 1, fp);
		fclose(fp);
		return true;
	}

	return false;
}

bool InferredData::LoadJsonData(fs::path path, load_func_t cb)
{
	std::ifstream fileStream(path);

	if (fileStream.is_open())
	{
		json temp;
		fileStream >> temp;
		fileStream.close();
		cb(temp);

		return true;
	}

	return false;
}

bool InferredData::LoadJsonData(fs::path path, PackableJson &data)
{
	return LoadJsonData(path, [&data](json& json) { data.UnPackJson(json); });
}

bool InferredData::LoadCacheData(uint32_t id, uint32_t magic1, uint32_t magic2, PackObj &data)
{
	BYTE *buffer = NULL;
	uint32_t length = 0;
	if (LoadDataFromPhatDataBin(id, &buffer, &length, magic1, magic2))
	{
		BinaryReader reader(buffer, length);
		data.UnPack(&reader);
		delete[] buffer;

		return true;
	}

	return false;
}

void InferredData::SaveJsonData(fs::path path, PackableJson &data)
{
	json tmp;
	data.PackJson(tmp);

	std::ofstream out(path);
	out << std::setw(4) << tmp << std::endl;
	out.flush();
	out.close();
}

#endif