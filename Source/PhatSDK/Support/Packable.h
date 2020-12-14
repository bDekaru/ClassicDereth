
#pragma once

#include <map>
#include <unordered_map>

#include "PackableJson.h"
#include "BinaryWriter.h"

class BinaryReader;

class PackObj
{
public:
	PackObj() = default;
	virtual ~PackObj() = default;
	// virtual uint32_t GetPackSize() { return 0; }
	virtual void Pack(BinaryWriter *pWriter) { }
	virtual bool UnPack(BinaryReader *pReader) { return true; }
};

#define DECLARE_PACKABLE() \
	virtual void Pack(BinaryWriter *pWriter) override; \
	virtual bool UnPack(BinaryReader *pReader) override;
#define DEFINE_PACK(className) \
	void className::Pack(BinaryWriter *pWriter)
#define DEFINE_UNPACK(className) \
	bool className::UnPack(BinaryReader *pReader)

// to make the old style (AC) packing use our new stream style
#define DECLARE_LEGACY_PACK_MIGRATOR() \
	ULONG Pack(BYTE **ppData, ULONG uiSize) override; \
	BOOL UnPack(BYTE **ppData, ULONG uiSize) override;
#define DEFINE_LEGACY_PACK_MIGRATOR(className) \
	ULONG className::Pack(BYTE **ppData, ULONG uiSize) { \
		std::unique_ptr<BinaryWriter> writer = std::make_unique<BinaryWriter>(); \
		Pack(writer.get()); \
		ULONG dataSize = writer->GetSize(); \
		if (ppData && uiSize >= dataSize) { \
			memcpy(*ppData, writer->GetData(), dataSize); \
			*ppData += dataSize; \
		} \
		return dataSize; \
	} \
	BOOL className::UnPack(BYTE **ppData, ULONG uiSize) { \
		BinaryReader reader(*ppData, uiSize); \
		return UnPack(&reader) ? TRUE : FALSE; \
	}

#ifndef _DEBUG
#define PackableHashTableBase std::unordered_map
#else
#define PackableHashTableBase std::map
#endif

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x40>
class PackableHashTable : public PackableHashTableBase<KeyType, ValueType>, public PackObj
{
public:
	using _base = PackableHashTableBase<KeyType, ValueType>;
	using iterator = typename _base::iterator;
	//typedef PackableHashTableBase<KeyType, ValueType> _base;
#ifndef _DEBUG
	PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>() : _base(DefaultBucketSize)
#else
	PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>()
#endif
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		//BinaryWriter entryData;
		std::unique_ptr<BinaryWriter> entryData = std::make_unique<BinaryWriter>();
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			numEntries++;
			entryData->template Write<KeyPackType, KeyType>(i->first);
			entryData->template Write<ValueType, ValueType>(i->second);
		}

		pWriter->template Write<WORD>(_base::size());
		pWriter->template Write<WORD>(DefaultBucketSize);
		pWriter->Write(entryData.get());
	}
	
	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		//BinaryWriter entryData;
		std::unique_ptr<BinaryWriter> entryData = std::make_unique<BinaryWriter>();
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			numEntries++;
			entryData->template Write<KeyPackType>((KeyPackType) i->first);
			i->second.Pack(entryData.get());
		}

		pWriter->template Write<WORD>(_base::size());
		pWriter->template Write<WORD>(DefaultBucketSize);
		pWriter->Write(entryData.get());
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}
	
	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<WORD>();
		[[maybe_unused]]
		uint32_t bucketSize = pReader->Read<WORD>();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key) = pReader->Read<ValueType>();

			if (pReader->GetLastError())
				break;
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<WORD>();
		[[maybe_unused]]
		uint32_t bucketSize = pReader->Read<WORD>();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);

			if (pReader->GetLastError())
				break;
		}
	}

	void EmptyContents()
	{
		_base::clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
		{
			_base::erase(i);
		}
	}

	bool exists(KeyType key)
	{
		return lookup(key) != NULL;
	}

	ValueType *lookup(const KeyType &key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
			return &i->second;
		return NULL;
	}

	ValueType *lookup_slow_string_case_insensitive(std::string key)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			if (!stricmp(i.first.c_str(), key.c_str()))
				return &i.second;
		}

		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		iterator i = _base::find(key);
		if (i == _base::end())
		{
			_base::insert(std::pair<KeyType, ValueType>(key, *value));
			return TRUE;
		}

		return FALSE;
	}

	void set(KeyType key, const ValueType *value)
	{

	}

	BOOL IsEmpty()
	{
		return !_base::size();
	}

protected:
};

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x10>
class PackableHashTableWithJson : public PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>, public PackableJson
{
public:
	using _base = PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>;
	using iterator = typename _base::iterator;

	PackableHashTableWithJson() = default;
	virtual ~PackableHashTableWithJson() = default;

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		UnPackJson(const json &reader)
	{
		for (const json &entry : reader)
		{
			KeyType key = entry["key"];
			//_base::operator[](key) = entry["value"];
			_base::insert(std::pair<KeyType, ValueType>(key, entry["value"]));
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		UnPackJson(const json &reader)
	{
		for (const json &entry : reader)
		{
			KeyType key = entry["key"];
			this->operator[](key).UnPackJson(entry["value"]);
		}
	}

	virtual bool UnPackJson(const json &reader) override
	{
		_base::EmptyContents();
		UnPackJson<ValueType>(reader);
		return true;
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry;
			entry["key"] = (KeyPackType)i->first;
			entry["value"] = i->second;

			writer.push_back(entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry;
			entry["key"] = (KeyPackType)i->first;

			json entryValue;
			i->second.PackJson(entryValue);
			entry["value"] = entryValue;

			writer.push_back(entry);
		}
	}

	virtual void PackJson(json &writer) override
	{
		PackJson<ValueType>(writer);
	}
};

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x10>
class PackableHashTableWithEnumConverterJson : public PackableHashTableWithJson<KeyType, ValueType, KeyPackType, DefaultBucketSize>
{
public:
	using _base = PackableHashTableWithJson<KeyType, ValueType, KeyPackType, DefaultBucketSize>;
	using iterator = typename _base::iterator;

	typedef KeyType (*EnumUnPacker)(const std::string &key);
	typedef std::string (*EnumPacker)(const KeyType &key);

	PackableHashTableWithEnumConverterJson<KeyType, ValueType, KeyPackType, DefaultBucketSize>(
		EnumUnPacker pfnEnumUnPacker = nullptr, EnumPacker pfnEnumPacker = nullptr) :
		_base()
	{
		_pfnEnumUnPacker = pfnEnumUnPacker;
		_pfnEnumPacker = pfnEnumPacker;
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		UnPackJson(const json &reader)
	{
		if (!_pfnEnumUnPacker)
		{
			assert(0);
		}

		for (const json &entry : reader)
		{
			std::string unconvertedKey = entry["key"];
			KeyType key = _pfnEnumUnPacker ? _pfnEnumUnPacker(unconvertedKey) : (KeyType)atoi(unconvertedKey.c_str());
			this->operator[](key) = entry["value"];
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		UnPackJson(const json &reader)
	{
		if (!_pfnEnumUnPacker)
		{
			assert(0);
		}

		for (const json &entry : reader)
		{
			std::string unconvertedKey = entry["key"];
			KeyType key = _pfnEnumUnPacker ? _pfnEnumUnPacker(unconvertedKey) : (KeyType)atoi(unconvertedKey.c_str());
			this->operator[](key).UnPackJson(entry["value"]);
		}
	}

	virtual bool UnPackJson(const json &reader) override
	{
		_base::EmptyContents();
		UnPackJson<ValueType>(reader);
		return true;
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry;

			if (_pfnEnumPacker)
			{
				entry["key"] = _pfnEnumPacker(i->first);
			}
			else
			{
				assert(0);
				entry["key"] = i->first; // questionable
			}

			entry["value"] = i->second;

			writer.push_back(entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry;

			if (_pfnEnumPacker)
			{
				entry["key"] = _pfnEnumPacker(i->first);
			}
			else
			{
				assert(0);
				entry["key"] = i->first; // questionable
			}

			json entryValue;
			i->second.PackJson(entryValue);
			entry["value"] = entryValue;

			writer.push_back(entry);
		}
	}

	virtual void PackJson(json &writer) override
	{
		PackJson<ValueType>(writer);
	}

	EnumUnPacker _pfnEnumUnPacker = nullptr;
	EnumPacker _pfnEnumPacker = nullptr;
};

// #define PHashTableBase std::unordered_map<KeyType, ValueType>
#define PHashTableBase std::map

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x40>
class PHashTable : public PHashTableBase<KeyType, ValueType>, public PackObj
{
public:
	using _base = PHashTableBase<KeyType, ValueType>;
	using iterator = typename _base::iterator;
	//typedef PHashTableBase<KeyType, ValueType> _base;
	PHashTable() = default;
	virtual ~PHashTable() = default;

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		//BinaryWriter entryData;
		std::unique_ptr<BinaryWriter> entryData = std::make_unique<BinaryWriter>();
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			numEntries++;
			entryData->template Write<KeyPackType>(i->first);
			entryData->template Write<ValueType>(i->second);

			if (numEntries == 0xFFFFFF)
				break;
		}

		pWriter->template Write<uint32_t>((_base::size() & 0xFFFFFF) | ((uint32_t)DefaultBucketSize << 24));
		pWriter->Write(entryData.get());
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		//BinaryWriter entryData;
		std::unique_ptr<BinaryWriter> entryData = std::make_unique<BinaryWriter>();
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			numEntries++;
			entryData->template Write<KeyPackType>(i->first);
			i->second.Pack(&entryData);

			if (numEntries == 0xFFFFFF)
				break;
		}

		pWriter->template Write<uint32_t>((_base::size() & 0xFFFFFF) | ((uint32_t)DefaultBucketSize << 24));
		pWriter->Write(entryData.get());
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<uint32_t>();
		[[maybe_unused]]
		uint32_t bucketSize = numEntries >> 24;
		numEntries &= 0xFFFFFF;

		for (uint32_t i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key) = pReader->Read<ValueType>();
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<uint32_t>();
		uint32_t bucketSize = numEntries >> 24;
		numEntries &= 0xFFFFFF;

		for (uint32_t i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);
		}
	}

	void EmptyContents()
	{
		_base::clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
		{
			_base::erase(i);
		}
	}

	ValueType *lookup(KeyType key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
			return &i->second;
		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		iterator i = _base::find(key);
		if (i == _base::end())
		{
			_base::insert(std::pair<KeyType, ValueType>(key, *value));
			return TRUE;
		}

		return FALSE;
	}

	void set(KeyType key, const ValueType *value)
	{

	}

	BOOL IsEmpty()
	{
		return !_base::size();
	}

protected:
};

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x40>
class HashTable : public std::map<KeyType, ValueType>, public PackObj
{
public:
	using _base = std::map<KeyType, ValueType>;
	using iterator = typename _base::iterator;

	HashTable() = default;
	virtual ~HashTable() = default;

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->template Write<BYTE>(DefaultBucketSize);
		pWriter->WriteCompressedUInt32((uint32_t) _base::size());

		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pWriter->Align(); // TODO handle other sizes other than 4 byte aligns

			pWriter->template Write<KeyPackType>(i->first);
			pWriter->template Write<ValueType>(i->second);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->template Write<BYTE>(DefaultBucketSize);
		pWriter->WriteCompressedUInt32((uint32_t)_base::size());

		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pWriter->Align(); // TODO handle other sizes other than 4 byte aligns

			pWriter->template Write<KeyPackType>(i->first);
			i->second.Pack(pWriter);
		}
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t bucketSize = pReader->Read<BYTE>();
		uint32_t numEntries = pReader->ReadCompressedUInt32();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pReader->Align(); // TODO handle other sizes other than 4 byte aligns

			KeyType key = pReader->Read<KeyType>();
			this->operator[](key) = pReader->Read<ValueType>();
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		[[maybe_unused]]
		uint32_t bucketSize = pReader->Read<BYTE>();
		uint32_t numEntries = pReader->ReadCompressedUInt32();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pReader->ReadAlign(); // TODO handle other sizes other than 4 byte aligns

			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);
		}
	}

	void EmptyContents()
	{
		_base::clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
		{
			erase(i);
		}
	}

	ValueType *lookup(KeyType key)
	{
		iterator i = _base::find(key);
		if (i != _base::end())
			return &i->second;
		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		iterator i = _base::find(key);
		if (i == _base::end())
		{
			insert(std::pair<KeyType, ValueType>(key, *value));
			return TRUE;
		}

		return FALSE;
	}

	void set(KeyType key, const ValueType *value)
	{

	}

	BOOL IsEmpty()
	{
		return !_base::size();
	}

protected:
};


template<typename ValueType>
class PackableList : public std::list<ValueType>, public PackObj
{
public:
	using _base = std::list<ValueType>;
	using iterator = typename _base::iterator;
	//typedef typename std::list<ValueType>::iterator Iterator;

	PackableList() = default;
	virtual ~PackableList() = default;

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->template Write<uint32_t>(_base::size());

		for (iterator i = _base::begin(); i != _base::end(); i++)
			pWriter->template Write<ValueType>(*i);
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->template Write<uint32_t>(_base::size());

		for (iterator i = _base::begin(); i != _base::end(); i++)
			i->Pack(pWriter);
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<uint32_t>();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			_base::push_back(pReader->Read<ValueType>());

			if (pReader->GetLastError())
				break;
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		uint32_t numEntries = pReader->Read<uint32_t>();

		for (uint32_t i = 0; i < numEntries; i++)
		{
			_base::emplace_back();
			_base::back().UnPack(pReader);

			if (pReader->GetLastError())
				break;
		}
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		_base::clear();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	iterator GetAt(int index)
	{
		if (index < 0 || index >= _base::size())
			return _base::end();

		iterator i = _base::begin();
		std::advance(i, index);
		return i;
	}

	void InsertAt(int index, const ValueType &value)
	{
		_base::insert(GetAt(index), value);
	}

	bool RemoveAt(int index)
	{
		iterator i = GetAt(index);

		if (i == _base::end())
			return false;

		_base::erase(i);
		return true;
	}

protected:

};

template<typename ValueType>
class PackableListWithJson : public PackableList<ValueType>, public PackableJson
{
public:
	using _base = PackableList<ValueType>;
	using iterator = typename _base::iterator;
	//typedef typename std::list<ValueType>::iterator Iterator;

	PackableListWithJson() = default;
	virtual ~PackableListWithJson() = default;

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJsonInternal(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry = (ValueType) *i;
			writer.push_back(entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		PackJsonInternal(json &writer)
	{
		for (iterator i = _base::begin(); i != _base::end(); i++)
		{
			json entry;
			i->PackJson(entry);
			writer.push_back(entry);
		}
	}

	virtual void PackJson(json &writer) override
	{
		PackJsonInternal<ValueType>(writer);
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		UnPackJsonInternal(const json &reader)
	{
		for (const auto &entry : reader)
		{
			_base::push_back((ValueType) entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		UnPackJsonInternal(const json &reader)
	{
		for (const auto &entry : reader)
		{
			_base::emplace_back();
			_base::back().UnPackJson(entry);
		}
	}

	virtual bool UnPackJson(const json &reader) override
	{
		_base::clear();
		UnPackJsonInternal<ValueType>(reader);
		return true;
	}
};
