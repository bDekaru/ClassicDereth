
#include "StdAfx.h"
#include "SkillTable.h"

bool SkillBase::UnPack(BinaryReader *pReader)
{
	_description = pReader->ReadString();
	_name = pReader->ReadString();
	_iconID = pReader->Read<DWORD>();
	_trained_cost = pReader->Read<DWORD>();
	_specialized_cost = pReader->Read<DWORD>();
	_category = pReader->Read<int>();
	_chargen_use = pReader->Read<int>();
	_min_level = pReader->Read<int>();
	_formula.UnPack(pReader);
	_upper_bound = pReader->Read<double>();
	_lower_bound = pReader->Read<double>();
	_learn_mod = pReader->Read<double>();
	return true;
}

DEFINE_DBOBJ(SkillTable, SkillTables);
DEFINE_LEGACY_PACK_MIGRATOR(SkillTable);

DEFINE_PACK(SkillTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(SkillTable)
{
	// ignore the file ID
	pReader->ReadDWORD();

	return _skillBaseHash.UnPack(pReader);
}

const SkillBase *SkillTable::GetSkillBaseRaw(STypeSkill key) // custom
{
	return _skillBaseHash.lookup(key);
}

const SkillBase *SkillTable::GetSkillBase(STypeSkill key)
{
	return _skillBaseHash.lookup(key);
}

std::string SkillTable::GetSkillName(STypeSkill key) // custom
{
	if (const SkillBase *skillBase = GetSkillBaseRaw(key))
	{
		return skillBase->_name;
	}
	
	return "";
}

SkillTable *SkillSystem::GetSkillTable()
{
	return CachedSkillTable;
}

BOOL SkillSystem::GetSkillName(STypeSkill key, std::string &value)
{
	if (SkillTable *pSkillTable = SkillSystem::GetSkillTable())
	{
		const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
		if (pSkillBase)
		{
			value = pSkillBase->_name;
			return TRUE;
		}
	}

	return FALSE;
}