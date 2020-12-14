
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

enum ExperienceHandlingType
{
	NoHandling				= 0x00,
	ApplyLevelMod		= 0x01,
	ShareWithFellows	= 0x02,
	AddFellowshipBonus	= 0x04,
	ShareWithAllegiance	= 0x08,
	ApplyToVitae		= 0x10,
	EarnsCP				= 0x20,
	ReducedByDistance	= 0x40,

	DefaultXp = ApplyLevelMod | ShareWithFellows | AddFellowshipBonus | ShareWithAllegiance | ApplyToVitae | ReducedByDistance,
	QuestXp = ShareWithFellows | ShareWithAllegiance | ApplyToVitae | ReducedByDistance,
	QuestXpNoShare = ApplyToVitae,
	PossibleItemXp = ShareWithFellows | ShareWithAllegiance | ApplyToVitae
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

	uint32_t GetAttributeLevelForExperience(uint32_t exp);
	uint32_t GetAttribute2ndLevelForExperience(uint32_t exp);
	uint32_t GetTrainedSkillLevelForExperience(uint32_t exp);
	uint32_t GetSpecializedSkillLevelForExperience(uint32_t exp);
	uint32_t GetLevelForExperience(ExperienceType type, uint32_t exp);
	uint32_t GetExperienceForLevel(ExperienceType type, uint32_t level);

	uint32_t GetExperienceForTrainedSkillLevel(uint32_t level);
	uint32_t GetExperienceForSpecializedSkillLevel(uint32_t level);

	uint32_t GetCreditsForLevel(unsigned int level);
	UINT64 GetExperienceForLevel(unsigned int level);
	uint32_t GetMaxLevel();

	uint32_t _max_attribute_level = 0;
	uint32_t * _attribute_table = NULL;
	uint32_t _max_attribute2nd_level = 0;
	uint32_t * _attribute2nd_table = NULL;
	uint32_t _max_trained_skill_level = 0;
	uint32_t * _trained_skill_table = NULL;
	uint32_t _max_specialized_skill_level = 0;
	uint32_t * _specialized_skill_table = NULL;
	uint32_t _max_level = 0;
	uint64_t * _level_table = NULL;
	uint32_t * _credit_table = 0;
};

class ExperienceSystem
{
public:
	static uint32_t AttributeLevelFromExperience(uint32_t exp);
	static uint32_t Attribute2ndLevelFromExperience(uint32_t exp);
	static uint32_t SkillLevelFromExperience(SKILL_ADVANCEMENT_CLASS sac, uint32_t exp);
	static uint32_t ExperienceToSkillLevel(SKILL_ADVANCEMENT_CLASS sac, uint32_t level);
	static uint32_t GetMaxTrainedSkillLevel();
	static uint32_t GetMaxSpecializedSkillLevel();

	static uint64_t ExperienceToLevel(unsigned int level);
	static uint32_t ExperienceToAttributeLevel(uint32_t level);
	static uint32_t ExperienceToAttribute2ndLevel(uint32_t level);

	static uint32_t GetMaxLevel();

	static uint64_t GetExperienceForLevel(unsigned int level);
	static uint32_t GetMaxAttributeLevel();
	static uint32_t GetMaxAttribute2ndLevel();

	static uint64_t ExperienceToRaiseLevel(unsigned int current_level, unsigned int new_level);

	static uint32_t GetCreditsForLevel(unsigned int level); // custom

	static ExperienceTable *GetExperienceTable();

	static int32_t ItemTotalXpToLevel(uint64_t xp, uint64_t basexp, int32_t maxLevel, int32_t style);
	static uint64_t ItemLevelToTotalXp(int32_t level, uint64_t basexp, int32_t maxLevel, int32_t style);
};

