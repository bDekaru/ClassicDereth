
#pragma once

class SkillFormula : public PackObj
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		_w = pReader->Read<uint32_t>();
		_x = pReader->Read<uint32_t>();
		_y = pReader->Read<uint32_t>();
		_z = pReader->Read<uint32_t>();
		_attr1 = pReader->Read<uint32_t>();
		_attr2 = pReader->Read<uint32_t>();
		return true;
	}

	BOOL Calculate(uint32_t a, uint32_t b, uint32_t &c) const
	{
		if (!_z)
			return FALSE;

		float value = (float)(_w + (_x * a) + (_y * b)) / (float)_z;
		c = (uint32_t)floor(value + 0.5f);

		return TRUE;
	}

	uint32_t _w = 0, _x = 0, _y = 0, _z = 0, _attr1 = 0, _attr2 = 0;
};

class SkillBase : public PackObj
{
public:
	SkillBase() { }
	virtual ~SkillBase() { }

	virtual bool UnPack(class BinaryReader *pReader) override;

	std::string _description;
	std::string _name;
	uint32_t _iconID = 0;
	uint32_t _trained_cost = 0;
	uint32_t _specialized_cost = 0;
	int _category = 0;
	int _chargen_use = 0;
	int _min_level = 0;
	SkillFormula _formula;
	double _upper_bound = 0;
	double _lower_bound = 0;
	double _learn_mod = 0;
};

class SkillTable : public PackObj, public DBObj
{
public:
	virtual ~SkillTable() { }

	DECLARE_DBOBJ(SkillTable)
	DECLARE_PACKABLE()
	DECLARE_LEGACY_PACK_MIGRATOR()

	const SkillBase *GetSkillBaseRaw(STypeSkill key); // custom
	const SkillBase *GetSkillBase(STypeSkill key);

	std::string GetSkillName(STypeSkill key); // custom

	PackableHashTable<uint32_t, SkillBase> _skillBaseHash;
};

class SkillSystem
{
public:
	static SkillTable *GetSkillTable();
	static BOOL GetSkillName(STypeSkill key, std::string &value);
};
