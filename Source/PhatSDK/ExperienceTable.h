
#pragma once

#include "Packable.h"
#include "ObjCache.h"

enum ExperienceType
{
	Attribute_ExperienceType = 1,
	Attribute2nd_ExperienceType,
	TrainedSkill_ExperienceType,
	SpecializedSkill_ExperienceType,
	Level_ExperienceType,
	Credit_ExperienceType
};

class ExperienceTable : public PackObj, public DBObj
{
public:
	ExperienceTable();
	virtual ~ExperienceTable();

	DECLARE_DBOBJ(ExperienceTable);
	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	void Clear();

	DWORD GetAttributeLevelForExperience(DWORD exp);
	DWORD GetAttribute2ndLevelForExperience(DWORD exp);
	DWORD GetTrainedSkillLevelForExperience(DWORD exp);
	DWORD GetSpecializedSkillLevelForExperience(DWORD exp);
	DWORD GetLevelForExperience(ExperienceType type, DWORD exp);
	DWORD GetExperienceForLevel(ExperienceType type, DWORD level);

	DWORD GetExperienceForTrainedSkillLevel(DWORD level);
	DWORD GetExperienceForSpecializedSkillLevel(DWORD level);

	DWORD GetCreditsForLevel(unsigned int level);
	UINT64 GetExperienceForLevel(unsigned int level);
	DWORD GetMaxLevel();

	unsigned long _max_attribute_level = 0;
	unsigned long * _attribute_table = NULL;
	unsigned long _max_attribute2nd_level = 0;
	unsigned long * _attribute2nd_table = NULL;
	unsigned long _max_trained_skill_level = 0;
	unsigned long * _trained_skill_table = NULL;
	unsigned long _max_specialized_skill_level = 0;
	unsigned long * _specialized_skill_table = NULL;
	unsigned long _max_level = 0;
	unsigned __int64 * _level_table = NULL;
	unsigned long * _credit_table = 0;
};

class ExperienceSystem
{
public:
	static DWORD AttributeLevelFromExperience(DWORD exp);
	static DWORD Attribute2ndLevelFromExperience(DWORD exp);
	static DWORD SkillLevelFromExperience(SKILL_ADVANCEMENT_CLASS sac, DWORD exp);
	static DWORD ExperienceFromSkillLevel(SKILL_ADVANCEMENT_CLASS sac, DWORD level);
	static DWORD GetMaxTrainedSkillLevel();
	static DWORD GetMaxSpecializedSkillLevel();

	static DWORD64 ExperienceToLevel(unsigned int level);
	static DWORD GetMaxLevel();

	static DWORD64 GetExperienceForLevel(unsigned int level);
	static DWORD64 ExperienceToRaiseLevel(unsigned int current_level, unsigned int new_level);

	static DWORD GetCreditsForLevel(unsigned int level); // custom

	static ExperienceTable *GetExperienceTable();
};

