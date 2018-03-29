
#include "StdAfx.h"
#include "ExperienceTable.h"
#include "ObjCache.h"

ExperienceTable::ExperienceTable()
{
}

ExperienceTable::~ExperienceTable()
{
	Clear();
}

void ExperienceTable::Clear()
{
	SafeDeleteArray(_attribute_table);
	SafeDeleteArray(_attribute2nd_table);
	SafeDeleteArray(_trained_skill_table);
	SafeDeleteArray(_specialized_skill_table);
	SafeDeleteArray(_level_table);
	SafeDeleteArray(_credit_table);
}

DEFINE_DBOBJ(ExperienceTable, ExperienceTables);
DEFINE_LEGACY_PACK_MIGRATOR(ExperienceTable);

DEFINE_PACK(ExperienceTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(ExperienceTable)
{
	pReader->Read<DWORD>(); // file ID

	_max_attribute_level = pReader->Read<DWORD>();
	_max_attribute2nd_level = pReader->Read<DWORD>();
	_max_trained_skill_level = pReader->Read<DWORD>();
	_max_specialized_skill_level = pReader->Read<DWORD>();
	_max_level = pReader->Read<DWORD>();

	unsigned int numAttribute = _max_attribute_level + 1;
	_attribute_table = new DWORD[numAttribute];
	memcpy(_attribute_table, pReader->ReadArray(sizeof(DWORD) * numAttribute), sizeof(DWORD) * numAttribute);

	unsigned int numAttribute2nd = _max_attribute2nd_level + 1;
	_attribute2nd_table = new DWORD[numAttribute2nd];
	memcpy(_attribute2nd_table, pReader->ReadArray(sizeof(DWORD) * numAttribute2nd), sizeof(DWORD) * numAttribute2nd);

	unsigned int numTrainedSkill = _max_trained_skill_level + 1;
	_trained_skill_table = new DWORD[numTrainedSkill];
	memcpy(_trained_skill_table, pReader->ReadArray(sizeof(DWORD) * numTrainedSkill), sizeof(DWORD) * numTrainedSkill);

	unsigned int numSpecializedSkill = _max_specialized_skill_level + 1;
	_specialized_skill_table = new DWORD[numSpecializedSkill];
	memcpy(_specialized_skill_table, pReader->ReadArray(sizeof(DWORD) * numSpecializedSkill), sizeof(DWORD) * numSpecializedSkill);

	unsigned int numLevel = _max_level + 1;
	_level_table = new unsigned __int64[numLevel];
	memcpy(_level_table, pReader->ReadArray(sizeof(unsigned __int64) * numLevel), sizeof(unsigned __int64) * numLevel);

	_credit_table = new DWORD[numLevel];
	memcpy(_credit_table, pReader->ReadArray(sizeof(DWORD) * numLevel), sizeof(DWORD) * numLevel);

	return true;
}

DWORD ExperienceTable::GetCreditsForLevel(unsigned int level) // custom
{
	if (level > _max_level)
		return 0;

	return _credit_table[level];
}

DWORD ExperienceTable::GetAttributeLevelForExperience(DWORD exp)
{
	return GetLevelForExperience(Attribute_ExperienceType, exp);
}

DWORD ExperienceTable::GetAttribute2ndLevelForExperience(DWORD exp)
{
	return GetLevelForExperience(Attribute2nd_ExperienceType, exp);
}

DWORD ExperienceTable::GetTrainedSkillLevelForExperience(DWORD exp)
{
	return GetLevelForExperience(TrainedSkill_ExperienceType, exp);
}

DWORD ExperienceTable::GetSpecializedSkillLevelForExperience(DWORD exp)
{
	return GetLevelForExperience(SpecializedSkill_ExperienceType, exp);
}

DWORD ExperienceTable::GetLevelForExperience(ExperienceType type, DWORD exp)
{
	DWORD maxLevel = 0;

	switch (type)
	{
	case Attribute_ExperienceType:
		maxLevel = _max_attribute_level;
		break;
	case Attribute2nd_ExperienceType:
		maxLevel = _max_attribute2nd_level;
		break;
	case TrainedSkill_ExperienceType:
		maxLevel = _max_trained_skill_level;
		break;
	case SpecializedSkill_ExperienceType:
		maxLevel = _max_specialized_skill_level;
		break;
	case Level_ExperienceType:
		maxLevel = _max_level;
		break;
	}

	// dear horrible code, you are horrible, yours truly.
	DWORD middle = (DWORD)((maxLevel + 1.0) * 0.5);
	DWORD lastHigh = maxLevel;
	DWORD lastLow = 0;

	for (int i = 1; i <= 16; i++)
	{
		DWORD levelLow = GetExperienceForLevel(type, middle);
		DWORD levelHigh = GetExperienceForLevel(type, middle + 1);

		if (exp >= levelHigh)
		{
			lastLow = middle;

			DWORD newmiddle = (DWORD)(((lastHigh - middle) + 1.0) * 0.5);

			if (!newmiddle)
			{
				return middle;
			}

			middle += newmiddle;
		}
		else if (exp >= levelLow)
		{
			return middle;
		}
		else
		{
			lastHigh = middle;

			DWORD newmiddle = (DWORD)(((middle - lastLow) + 1.0) * 0.5);

			if (!newmiddle)
			{
				return middle;
			}

			middle -= newmiddle;
		}
	}

	return middle;
}

UINT64 ExperienceTable::GetExperienceForLevel(unsigned int level)
{
	if (level > _max_level)
		level = _max_level;

	return (_level_table ? _level_table[level] : 0);
}

DWORD ExperienceTable::GetMaxLevel()
{
	return _max_level;
}

DWORD ExperienceTable::GetExperienceForLevel(ExperienceType type, DWORD level)
{
	DWORD *_table;

	switch (type)
	{
	case Attribute_ExperienceType:
		if (level > _max_attribute_level)
			return UINT_MAX;

		_table = _attribute_table;
		break;
	case Attribute2nd_ExperienceType:
		if (level > _max_attribute2nd_level)
			return UINT_MAX;

		_table = _attribute2nd_table;
		break;
	case TrainedSkill_ExperienceType:
		if (level > _max_trained_skill_level)
			return UINT_MAX;

		_table = _trained_skill_table;
		break;
	case SpecializedSkill_ExperienceType:
		if (level > _max_specialized_skill_level)
			return UINT_MAX;

		_table = _specialized_skill_table;
		break;

	default:
		return UINT_MAX;

	}

	return _table[level];
}

DWORD ExperienceTable::GetExperienceForTrainedSkillLevel(DWORD level)
{
	if (level > _max_trained_skill_level)
		return UINT_MAX;

	return _trained_skill_table[level];
}

DWORD ExperienceTable::GetExperienceForSpecializedSkillLevel(DWORD level)
{
	if (level > _max_specialized_skill_level)
		return UINT_MAX;

	return _specialized_skill_table[level];
}

DWORD ExperienceSystem::AttributeLevelFromExperience(DWORD exp)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetAttributeLevelForExperience(exp);
	}

	return 0;
}

DWORD ExperienceSystem::Attribute2ndLevelFromExperience(DWORD exp)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetAttribute2ndLevelForExperience(exp);
	}

	return 0;
}

DWORD ExperienceSystem::SkillLevelFromExperience(SKILL_ADVANCEMENT_CLASS sac, DWORD exp)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		switch (sac)
		{
		case TRAINED_SKILL_ADVANCEMENT_CLASS: // trained
			return pExpTable->GetTrainedSkillLevelForExperience(exp);

		case SPECIALIZED_SKILL_ADVANCEMENT_CLASS: // specialized
			return pExpTable->GetSpecializedSkillLevelForExperience(exp);
		}
	}
	
	return 0;
}

DWORD ExperienceSystem::ExperienceToSkillLevel(SKILL_ADVANCEMENT_CLASS sac, DWORD level)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		switch (sac)
		{
		case TRAINED_SKILL_ADVANCEMENT_CLASS: // trained
			return pExpTable->GetExperienceForTrainedSkillLevel(level);

		case SPECIALIZED_SKILL_ADVANCEMENT_CLASS: // specialized
			return pExpTable->GetExperienceForSpecializedSkillLevel(level);
		}
	}

	return UINT_MAX;
}

DWORD ExperienceSystem::GetMaxTrainedSkillLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_trained_skill_level;
	}

	return 0;
}

DWORD ExperienceSystem::GetMaxSpecializedSkillLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_specialized_skill_level;
	}

	return 0;
}

DWORD64 ExperienceSystem::ExperienceToLevel(unsigned int level)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetExperienceForLevel(level);
	}

	return 0;
}

ExperienceTable *ExperienceSystem::GetExperienceTable()
{
	return CachedExperienceTable;
}

DWORD ExperienceSystem::GetMaxLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetMaxLevel();
	}

	return 0;
}

DWORD64 ExperienceSystem::ExperienceToRaiseLevel(unsigned int current_level, unsigned int new_level)
{
	if (new_level > current_level)
	{
		return ExperienceToLevel(new_level) - ExperienceToLevel(current_level);
	}

	return 0;
}

DWORD ExperienceSystem::GetCreditsForLevel(unsigned int level) // custom
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetCreditsForLevel(level);
	}

	return 0;
}

