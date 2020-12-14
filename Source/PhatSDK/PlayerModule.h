
#pragma once

#include "Packable.h"

class ShortCutData : public PackObj
{
public:
	DECLARE_PACKABLE()

	int index_;
	unsigned int objectID_;
	unsigned int spellID_;
};

class ShortCutManager : public PackObj
{
public:
	ShortCutManager() { }
	ShortCutManager(const ShortCutManager &other) { *this = other; }
	virtual ~ShortCutManager() override;

	DECLARE_PACKABLE()

	ShortCutManager &operator =(const ShortCutManager &other);

	void Destroy();
	void AddShortCut(const ShortCutData &data);
	void RemoveShortCut(int index);

	ShortCutData **shortCuts_ = NULL;
};


class GenericQualitiesData : public PackObj
{
public:
	virtual ~GenericQualitiesData() override;

	DECLARE_PACKABLE()

	BOOL InqString(uint32_t key, std::string &value);

	PackableHashTable<uint32_t, int32_t> *m_pIntStatsTable = NULL;
	PackableHashTable<uint32_t, int32_t> *m_pBoolStatsTable = NULL;
	PackableHashTable<uint32_t, double> *m_pFloatStatsTable = NULL;
	PackableHashTable<uint32_t, std::string> *m_pStrStatsTable = NULL;
};

class BasePropertyValue : public PackObj
{
public:
	virtual void CopyFrom(BasePropertyValue *other) = 0;
};

class BaseProperty : public PackObj // Similar functionality, but not the real thing
{
public:
	virtual ~BaseProperty() override
	{
		SafeDelete(m_propertyValue);
	}

	BaseProperty &operator=(const BaseProperty &other)
	{
		m_propertyName = other.m_propertyName;

		if (m_propertyValue == other.m_propertyValue)
			return *this; // don't copy onto self

		SafeDelete(m_propertyValue);

		if (other.m_propertyValue)
		{
			m_propertyValue = CreatePropertyValue(m_propertyName);

			if (m_propertyValue)
				m_propertyValue->CopyFrom(other.m_propertyValue);
		}

		return *this;
	}

	BasePropertyValue *CreatePropertyValue(uint32_t propName);

	DECLARE_PACKABLE()

	uint32_t m_propertyName = 0;
	BasePropertyValue *m_propertyValue = NULL;
};

enum TriState
{
	Undef_TriState = 0xFF,
	False_TriState = 0x0,
	True_TriState = 0x1,
};

class TriStatePropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(TriStatePropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<BYTE>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = (TriState)pReader->Read<BYTE>();
		return true;
	}

	TriState m_value;
};

class ArrayPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(ArrayPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		m_value.Pack(pWriter);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value.UnPack(pReader);
		return true;
	}

	SmartArray<BaseProperty> m_value;
};

class Bitfield32PropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(Bitfield32PropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<uint32_t>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<uint32_t>();
		return true;
	}

	uint32_t m_value;
};

class Bitfield64PropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(Bitfield64PropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<uint64_t>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<uint64_t>();
		return true;
	}

	uint64_t m_value;
};

class BoolPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(BoolPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<BYTE>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<BYTE>() ? true : false;
		return true;
	}

	bool m_value;
};

class IntegerPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(IntegerPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<int>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<int>();
		return true;
	}

	int m_value;
};

class LongIntegerPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(LongIntegerPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<int64_t>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<int64_t>();
		return true;
	}

	int64_t m_value;
};

class FloatPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(FloatPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<float>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<float>();
		return true;
	}

	float m_value;
};

class VectorPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(VectorPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		m_value.Pack(pWriter);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value.UnPack(pReader);
		return true;
	}

	Vector m_value;
};

class ColorPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(ColorPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->Write<uint32_t>(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->Read<uint32_t>();
		return true;
	}

	uint32_t m_value;
};

class StringPropertyValue : public BasePropertyValue
{
public:
	virtual void CopyFrom(StringPropertyValue *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		pWriter->WriteString(m_value);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value = pReader->ReadString();
		return true;
	}

	std::string m_value;
};

class StringInfo : public PackObj
{
public:
	enum StringInfo_Override_Flag
	{
		SI_NONE = 0x0,
		SI_LITERAL = 0x1,
		SI_AUTOGEN = 0x2,
	};

	virtual void CopyFrom(StringInfo *other)
	{
		*this = *other;
	}
	virtual void Pack(BinaryWriter *pWriter) override
	{
		UNFINISHED();
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		UNFINISHED();
		return true;
	}

	StringInfo_Override_Flag _override;
	std::wstring _literal_value;
	uint32_t _string_id;
	uint32_t _table_id;
	bool _has_strings;
	std::string _str_token;
	std::string _str_english;
	std::string _str_comment;
	// HashTable<uint32_t, etc...> _variables;
};

class StringInfoPropertyValue : public BasePropertyValue
{
public:
	virtual void Pack(BinaryWriter *pWriter) override
	{
		m_value.Pack(pWriter);
	}
	virtual bool UnPack(BinaryReader *pReader) override
	{
		m_value.UnPack(pReader);
		return true;
	}

	StringInfo m_value;

};

class PackObjPropertyCollection : public PackObj // Similar functionality, but not the real thing
{
public:
	virtual ~PackObjPropertyCollection() { }

	DECLARE_PACKABLE()

	HashTable<uint32_t, BaseProperty> m_hashProperties;
};

class PlayerModule : public PackObj
{
public:
	PlayerModule() { }
	PlayerModule(const PlayerModule &other) { *this = other; }
	virtual ~PlayerModule() override;

	DECLARE_PACKABLE()

	void Destroy();

	PlayerModule &operator=(const PlayerModule &other);

	void SetPackHeader(uint32_t *bitfield);

	void AddShortCut(ShortCutData &data);
	void RemoveShortCut(int index);
	void AddSpellFavorite(uint32_t spell_id, int index, int spellBar);
	void RemoveSpellFavorite(uint32_t spell_id, int spellBar);

	void AddOrUpdateDesiredComp(uint32_t & compWCID, uint32_t & compQTY);

	ShortCutManager *shortcuts_ = NULL;
	PackableList<uint32_t> favorite_spells_[8];
	PackableHashTable<uint32_t, int32_t> *desired_comps_ = NULL;
	unsigned int options_ = 0x50C4A542; // 0x50C4A54A; Custom (we don't want to ignore fellowship requests by default)
	unsigned int options2_ = 0x948700 | DisplayNumberDeaths_CharacterOptions2; // Custom, show deaths by default
	unsigned int spell_filters_ = 0x3FFF;
	GenericQualitiesData *m_pPlayerOptionsData = NULL;
	std::string m_TimeStampFormat = "%#H:%M:%S ";
	int windowDataLength = 0;
	BYTE* windowData;
};

