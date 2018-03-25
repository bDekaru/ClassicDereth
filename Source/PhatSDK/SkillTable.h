
#pragma once

class SkillFormula : public PackObj
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		_w = pReader->Read<DWORD>();
		_x = pReader->Read<DWORD>();
		_y = pReader->Read<DWORD>();
		_z = pReader->Read<DWORD>();
		_attr1 = pReader->Read<DWORD>();
		_attr2 = pReader->Read<DWORD>();
		return true;
	}

	BOOL Calculate(DWORD a, DWORD b, DWORD &c) const
	{
		if (!_z)
			return FALSE;

		float value = (float)(_w + (_x * a) + (_y * b)) / (float)_z;
		c = (DWORD)floor(value + 0.5f);

		return TRUE;
	}

	DWORD _w = 0, _x = 0, _y = 0, _z = 0, _attr1 = 0, _attr2 = 0;
};

class SkillBase : public PackObj
{
public:
	SkillBase() { }
	virtual ~SkillBase() { }

	virtual bool UnPack(class BinaryReader *pReader) override;

	std::string _description;
	std::string _name;
	DWORD _iconID = 0;
	DWORD _trained_cost = 0;
	DWORD _specialized_cost = 0;
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

	PackableHashTable<DWORD, SkillBase> _skillBaseHash;
};

class SkillSystem
{
public:
	static SkillTable *GetSkillTable();
	static BOOL GetSkillName(STypeSkill key, std::string &value);
};
