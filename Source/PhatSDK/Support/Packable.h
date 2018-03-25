
#pragma once

#include "PackableJson.h"

class PackObj
{
public:
	PackObj() { }
	virtual ~PackObj() { }
	// virtual DWORD GetPackSize() { return 0; }
	virtual void Pack(class BinaryWriter *pWriter) { }
	virtual bool UnPack(class BinaryReader *pReader) { return true; }
};

#define DECLARE_PACKABLE() \
	virtual void Pack(class BinaryWriter *pWriter) override; \
	virtual bool UnPack(class BinaryReader *pReader) override;
#define DEFINE_PACK(className) \
	void className::Pack(class BinaryWriter *pWriter)
#define DEFINE_UNPACK(className) \
	bool className::UnPack(class BinaryReader *pReader)

// to make the old style (AC) packing use our new stream style
#define DECLARE_LEGACY_PACK_MIGRATOR() \
	ULONG Pack(BYTE **ppData, ULONG uiSize) override; \
	BOOL UnPack(BYTE **ppData, ULONG uiSize) override;
#define DEFINE_LEGACY_PACK_MIGRATOR(className) \
	ULONG className::Pack(BYTE **ppData, ULONG uiSize) { \
		BinaryWriter writer; \
		Pack(&writer); \
		ULONG dataSize = writer.GetSize(); \
		if (ppData && uiSize >= writer.GetSize()) { \
			memcpy(*ppData, writer.GetData(), dataSize); \
			*ppData += dataSize; \
		} \
		return writer.GetSize(); \
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
	typedef PackableHashTableBase<KeyType, ValueType> baseClass;
#ifndef _DEBUG
	PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>() : baseClass(DefaultBucketSize)
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

		BinaryWriter entryData;
		for (baseClass::iterator i = begin(); i != end(); i++)
		{
			numEntries++;
			entryData.Write<KeyPackType>(i->first);
			entryData.Write<ValueType>(i->second);
		}

		pWriter->Write<WORD>(size());
		pWriter->Write<WORD>(DefaultBucketSize);
		pWriter->Write(&entryData);
	}
	
	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		BinaryWriter entryData;
		for (baseClass::iterator i = begin(); i != end(); i++)
		{
			numEntries++;
			entryData.Write<KeyPackType>((KeyPackType) i->first);
			i->second.Pack(&entryData);
		}

		pWriter->Write<WORD>(size());
		pWriter->Write<WORD>(DefaultBucketSize);
		pWriter->Write(&entryData);
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}
	
	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		DWORD numEntries = pReader->Read<WORD>();
		DWORD bucketSize = pReader->Read<WORD>();

		for (DWORD i = 0; i < numEntries; i++)
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
		DWORD numEntries = pReader->Read<WORD>();
		DWORD bucketSize = pReader->Read<WORD>();

		for (DWORD i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);

			if (pReader->GetLastError())
				break;
		}
	}

	void EmptyContents()
	{
		clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		baseClass::iterator i = find(key);
		if (i != end())
		{
			erase(i);
		}
	}

	bool exists(KeyType key)
	{
		return lookup(key) != NULL;
	}

	ValueType *lookup(KeyType key)
	{
		baseClass::iterator i = find(key);
		if (i != end())
			return &i->second;
		return NULL;
	}

	ValueType *lookup_slow_string_case_insensitive(std::string key)
	{
		for (baseClass::iterator i = begin(); i != end(); i++)
		{
			if (!_stricmp(i.first.c_str(), key.c_str()))
				return &i.second;
		}

		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		baseClass::iterator i = find(key);
		if (i == end())
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
		return !size();
	}

protected:
};

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x10>
class PackableHashTableWithJson : public PackableHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>, public PackableJson
{
public:
	PackableHashTableWithJson<KeyType, ValueType, KeyPackType, DefaultBucketSize>()
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		UnPackJson(const json &reader)
	{
		for (const json &entry : reader)
		{
			KeyType key = entry["key"];
			this->operator[](key) = entry["value"];
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
		EmptyContents();
		UnPackJson<ValueType>(reader);
		return true;
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (baseClass::iterator i = begin(); i != end(); i++)
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
		for (baseClass::iterator i = begin(); i != end(); i++)
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
	typedef KeyType (*EnumUnPacker)(const std::string &key);
	typedef std::string (*EnumPacker)(const KeyType &key);

	PackableHashTableWithEnumConverterJson<KeyType, ValueType, KeyPackType, DefaultBucketSize>(
		EnumUnPacker pfnEnumUnPacker = nullptr, EnumPacker pfnEnumPacker = nullptr)
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
		EmptyContents();
		UnPackJson<ValueType>(reader);
		return true;
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJson(json &writer)
	{
		for (baseClass::iterator i = begin(); i != end(); i++)
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
		for (baseClass::iterator i = begin(); i != end(); i++)
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
	typedef PHashTableBase<KeyType, ValueType> baseClass;
	PHashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>()
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		BinaryWriter entryData;
		for (baseClass::iterator i = begin(); i != end(); i++)
		{
			numEntries++;
			entryData.Write<KeyPackType>(i->first);
			entryData.Write<ValueType>(i->second);

			if (numEntries == 0xFFFFFF)
				break;
		}

		pWriter->Write<DWORD>((size() & 0xFFFFFF) | ((DWORD)DefaultBucketSize << 24));
		pWriter->Write(&entryData);
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		unsigned int numEntries = 0;

		BinaryWriter entryData;
		for (baseClass::iterator i = begin(); i != end(); i++)
		{
			numEntries++;
			entryData.Write<KeyPackType>(i->first);
			i->second.Pack(&entryData);

			if (numEntries == 0xFFFFFF)
				break;
		}

		pWriter->Write<DWORD>((size() & 0xFFFFFF) | ((DWORD)DefaultBucketSize << 24));
		pWriter->Write(&entryData);
	}

	virtual void Pack(BinaryWriter *pWriter) override
	{
		PackInternal<ValueType>(pWriter);
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		DWORD numEntries = pReader->Read<DWORD>();
		DWORD bucketSize = numEntries >> 24;
		numEntries &= 0xFFFFFF;

		for (DWORD i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key) = pReader->Read<ValueType>();
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		DWORD numEntries = pReader->Read<DWORD>();
		DWORD bucketSize = numEntries >> 24;
		numEntries &= 0xFFFFFF;

		for (DWORD i = 0; i < numEntries; i++)
		{
			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);
		}
	}

	void EmptyContents()
	{
		clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		baseClass::iterator i = find(key);
		if (i != end())
		{
			erase(i);
		}
	}

	ValueType *lookup(KeyType key)
	{
		baseClass::iterator i = find(key);
		if (i != end())
			return &i->second;
		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		baseClass::iterator i = find(key);
		if (i == end())
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
		return !size();
	}

protected:
};

template<typename KeyType, typename ValueType, typename KeyPackType = uint32_t, int DefaultBucketSize = 0x40>
class HashTable : public std::map<KeyType, ValueType>, public PackObj
{
public:
	HashTable<KeyType, ValueType, KeyPackType, DefaultBucketSize>()
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<BYTE>(DefaultBucketSize);
		pWriter->WriteCompressedUInt32((DWORD) size());

		for (std::map<KeyType, ValueType>::iterator i = begin(); i != end(); i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pWriter->Align(); // TODO handle other sizes other than 4 byte aligns

			pWriter->Write<KeyPackType>(i->first);
			pWriter->Write<ValueType>(i->second);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<BYTE>(DefaultBucketSize);
		pWriter->WriteCompressedUInt32((DWORD) size());

		for (std::map<KeyType, ValueType>::iterator i = begin(); i != end(); i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pWriter->Align(); // TODO handle other sizes other than 4 byte aligns

			pWriter->Write<KeyPackType>(i->first);
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
		DWORD bucketSize = pReader->Read<BYTE>();
		DWORD numEntries = pReader->ReadCompressedUInt32();

		for (DWORD i = 0; i < numEntries; i++)
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
		DWORD bucketSize = pReader->Read<BYTE>();
		DWORD numEntries = pReader->ReadCompressedUInt32();

		for (DWORD i = 0; i < numEntries; i++)
		{
			//if (sizeof(KeyPackType) == 4)
			//	pReader->ReadAlign(); // TODO handle other sizes other than 4 byte aligns

			KeyType key = pReader->Read<KeyType>();
			this->operator[](key).UnPack(pReader);
		}
	}

	void EmptyContents()
	{
		clear();
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		EmptyContents();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	void remove(KeyType key)
	{
		std::map<KeyType, ValueType>::iterator i = find(key);
		if (i != end())
		{
			erase(i);
		}
	}

	ValueType *lookup(KeyType key)
	{
		std::map<KeyType, ValueType>::iterator i = find(key);
		if (i != end())
			return &i->second;
		return NULL;
	}

	BOOL add(KeyType key, const ValueType *value)
	{
		std::map<KeyType, ValueType>::iterator i = find(key);
		if (i == end())
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
		return !size();
	}

protected:
};


template<typename ValueType>
class PackableList : public std::list<ValueType>, public PackObj
{
public:
	typedef typename std::list<ValueType>::iterator Iterator;

	PackableList<ValueType>()
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<DWORD>(size());

		for (std::list<ValueType>::iterator i = begin(); i != end(); i++)
			pWriter->Write<ValueType>(*i);
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		PackInternal(BinaryWriter *pWriter)
	{
		pWriter->Write<DWORD>(size());

		for (std::list<ValueType>::iterator i = begin(); i != end(); i++)
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
		DWORD numEntries = pReader->Read<DWORD>();

		for (DWORD i = 0; i < numEntries; i++)
		{
			push_back(pReader->Read<ValueType>());

			if (pReader->GetLastError())
				break;
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackObj, vt>::value>::type
		UnPackInternal(BinaryReader *pReader)
	{
		DWORD numEntries = pReader->Read<DWORD>();

		for (DWORD i = 0; i < numEntries; i++)
		{
			emplace_back();
			back().UnPack(pReader);

			if (pReader->GetLastError())
				break;
		}
	}

	virtual bool UnPack(BinaryReader *pReader) override
	{
		clear();
		UnPackInternal<ValueType>(pReader);
		return true;
	}

	typename Iterator GetAt(int index)
	{
		if (index < 0 || index >= size())
			return end();

		Iterator i = begin();
		std::advance(i, index);
		return i;
	}

	void InsertAt(int index, const ValueType &value)
	{
		insert(GetAt(index), value);
	}

	bool RemoveAt(int index)
	{
		Iterator i = GetAt(index);

		if (i == end())
			return false;

		erase(i);
		return true;
	}

protected:

};

template<typename ValueType>
class PackableListWithJson : public PackableList<ValueType>, public PackableJson
{
public:
	typedef typename std::list<ValueType>::iterator Iterator;

	PackableListWithJson<ValueType>()
	{
	}

	template<typename vt>
	typename std::enable_if<!std::is_base_of<PackableJson, vt>::value>::type
		PackJsonInternal(json &writer)
	{
		for (std::list<ValueType>::iterator i = begin(); i != end(); i++)
		{
			json entry = (ValueType) *i;
			writer.push_back(entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		PackJsonInternal(json &writer)
	{
		for (std::list<ValueType>::iterator i = begin(); i != end(); i++)
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
			push_back((ValueType) entry);
		}
	}

	template<typename vt>
	typename std::enable_if<std::is_base_of<PackableJson, vt>::value>::type
		UnPackJsonInternal(const json &reader)
	{
		for (const auto &entry : reader)
		{
			emplace_back();
			back().UnPackJson(entry);
		}
	}

	virtual bool UnPackJson(const json &reader) override
	{
		clear();
		UnPackJsonInternal<ValueType>(reader);
		return true;
	}
};
