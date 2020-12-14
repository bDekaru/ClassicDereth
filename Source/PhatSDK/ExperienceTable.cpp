
#include <StdAfx.h>
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
	pReader->Read<uint32_t>(); // file ID

	_max_attribute_level = pReader->Read<uint32_t>();
	_max_attribute2nd_level = pReader->Read<uint32_t>();
	_max_trained_skill_level = pReader->Read<uint32_t>();
	_max_specialized_skill_level = pReader->Read<uint32_t>();
	_max_level = pReader->Read<uint32_t>();

	unsigned int numAttribute = _max_attribute_level + 1;
	_attribute_table = new uint32_t[numAttribute];
	memcpy(_attribute_table, pReader->ReadArray(sizeof(uint32_t) * numAttribute), sizeof(uint32_t) * numAttribute);

	unsigned int numAttribute2nd = _max_attribute2nd_level + 1;
	_attribute2nd_table = new uint32_t[numAttribute2nd];
	memcpy(_attribute2nd_table, pReader->ReadArray(sizeof(uint32_t) * numAttribute2nd), sizeof(uint32_t) * numAttribute2nd);

	unsigned int numTrainedSkill = _max_trained_skill_level + 1;
	_trained_skill_table = new uint32_t[numTrainedSkill];
	memcpy(_trained_skill_table, pReader->ReadArray(sizeof(uint32_t) * numTrainedSkill), sizeof(uint32_t) * numTrainedSkill);

	unsigned int numSpecializedSkill = _max_specialized_skill_level + 1;
	_specialized_skill_table = new uint32_t[numSpecializedSkill];
	memcpy(_specialized_skill_table, pReader->ReadArray(sizeof(uint32_t) * numSpecializedSkill), sizeof(uint32_t) * numSpecializedSkill);

	unsigned int numLevel = _max_level + 1;
	_level_table = new uint64_t[numLevel];
	memcpy(_level_table, pReader->ReadArray(sizeof(uint64_t) * numLevel), sizeof(uint64_t) * numLevel);

	_credit_table = new uint32_t[numLevel];
	memcpy(_credit_table, pReader->ReadArray(sizeof(uint32_t) * numLevel), sizeof(uint32_t) * numLevel);

	return true;
}

uint32_t ExperienceTable::GetCreditsForLevel(unsigned int level) // custom
{
	if (level > _max_level)
		return 0;

	return _credit_table[level];
}

uint32_t ExperienceTable::GetAttributeLevelForExperience(uint32_t exp)
{
	return GetLevelForExperience(Attribute_ExperienceType, exp);
}

uint32_t ExperienceTable::GetAttribute2ndLevelForExperience(uint32_t exp)
{
	return GetLevelForExperience(Attribute2nd_ExperienceType, exp);
}

uint32_t ExperienceTable::GetTrainedSkillLevelForExperience(uint32_t exp)
{
	return GetLevelForExperience(TrainedSkill_ExperienceType, exp);
}

uint32_t ExperienceTable::GetSpecializedSkillLevelForExperience(uint32_t exp)
{
	return GetLevelForExperience(SpecializedSkill_ExperienceType, exp);
}

uint32_t ExperienceTable::GetLevelForExperience(ExperienceType type, uint32_t exp)
{
	uint32_t maxLevel = 0;

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
	uint32_t middle = (uint32_t)((maxLevel + 1.0) * 0.5);
	uint32_t lastHigh = maxLevel;
	uint32_t lastLow = 0;

	for (int i = 1; i <= 16; i++)
	{
		uint32_t levelLow = GetExperienceForLevel(type, middle);
		uint32_t levelHigh = GetExperienceForLevel(type, middle + 1);

		if (exp >= levelHigh)
		{
			lastLow = middle;

			uint32_t newmiddle = (uint32_t)(((lastHigh - middle) + 1.0) * 0.5);

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

			uint32_t newmiddle = (uint32_t)(((middle - lastLow) + 1.0) * 0.5);

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

uint32_t ExperienceTable::GetMaxLevel()
{
	return _max_level;
}

uint32_t ExperienceTable::GetExperienceForLevel(ExperienceType type, uint32_t level)
{
	uint32_t *_table;

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

uint32_t ExperienceTable::GetExperienceForTrainedSkillLevel(uint32_t level)
{
	if (level > _max_trained_skill_level)
		return UINT_MAX;

	return _trained_skill_table[level];
}

uint32_t ExperienceTable::GetExperienceForSpecializedSkillLevel(uint32_t level)
{
	if (level > _max_specialized_skill_level)
		return UINT_MAX;

	return _specialized_skill_table[level];
}

uint32_t ExperienceSystem::AttributeLevelFromExperience(uint32_t exp)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetAttributeLevelForExperience(exp);
	}

	return 0;
}

uint32_t ExperienceSystem::Attribute2ndLevelFromExperience(uint32_t exp)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetAttribute2ndLevelForExperience(exp);
	}

	return 0;
}

uint32_t ExperienceSystem::SkillLevelFromExperience(SKILL_ADVANCEMENT_CLASS sac, uint32_t exp)
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

uint32_t ExperienceSystem::ExperienceToSkillLevel(SKILL_ADVANCEMENT_CLASS sac, uint32_t level)
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

uint32_t ExperienceSystem::ExperienceToAttributeLevel(uint32_t level)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetExperienceForLevel(ExperienceType::Attribute_ExperienceType, level);
	}

	return 0;
}

uint32_t ExperienceSystem::ExperienceToAttribute2ndLevel(uint32_t level)
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetExperienceForLevel(ExperienceType::Attribute2nd_ExperienceType, level);
	}

	return 0;
}


uint32_t ExperienceSystem::GetMaxTrainedSkillLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_trained_skill_level;
	}

	return 0;
}

uint32_t ExperienceSystem::GetMaxSpecializedSkillLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_specialized_skill_level;
	}

	return 0;
}

uint32_t ExperienceSystem::GetMaxAttributeLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_attribute_level;
	}

	return 0;
}

uint32_t ExperienceSystem::GetMaxAttribute2ndLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->_max_attribute2nd_level;
	}

	return 0;
}


uint64_t ExperienceSystem::ExperienceToLevel(unsigned int level)
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

uint32_t ExperienceSystem::GetMaxLevel()
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetMaxLevel();
	}

	return 0;
}

uint64_t ExperienceSystem::ExperienceToRaiseLevel(unsigned int current_level, unsigned int new_level)
{
	if (new_level > current_level)
	{
		return ExperienceToLevel(new_level) - ExperienceToLevel(current_level);
	}

	return 0;
}

uint32_t ExperienceSystem::GetCreditsForLevel(unsigned int level) // custom
{
	if (ExperienceTable *pExpTable = GetExperienceTable())
	{
		return pExpTable->GetCreditsForLevel(level);
	}

	return 0;
}

int32_t ExperienceSystem::ItemTotalXpToLevel(uint64_t xp, uint64_t basexp, int32_t maxLevel, int32_t style)
{
	int32_t result = 0;
	uint64_t levelxp = basexp;
	uint64_t totalxp = basexp;

	if (basexp <= 0)
		return result;

	switch (style)
	{
	case 1:	// straight division
		result = (int32_t)(xp / basexp);
		break;

	case 2: // double-per-level
		while (totalxp <= xp)
		{
			result++;
			levelxp *= uint64_t(2);
			totalxp += levelxp;
		}
		break;

	case 3: // curve
		while (totalxp <= xp)
		{
			result++;
			levelxp += basexp;
			totalxp += levelxp;
		}
		break;
	}

	if (result > maxLevel)
		return maxLevel;

	return result;
}

uint64_t ExperienceSystem::ItemLevelToTotalXp(int32_t level, uint64_t basexp, int32_t maxLevel, int32_t style)
{
	if (level < 1)
		return 0;

	if (basexp <= 0)
		return 0;

	if (level > maxLevel)
		level = maxLevel;

	uint64_t result = basexp;
	uint64_t levelxp = basexp;
	int32_t xplevel = 1;

	switch (style)
	{
	case 1:	// straight division
		result = (uint64_t)level * basexp;
		break;

	case 2: // double-per-level
		while (xplevel < level)
		{
			xplevel++;
			levelxp *= uint64_t(2);
			result += levelxp;
		}
		break;

	case 3: // curve
		while (xplevel < level)
		{
			xplevel++;
			levelxp += basexp;
			result += levelxp;
		}
		break;
	}

	return result;
}
