
#pragma once

#include "Packable.h"
#include "ObjCache.h"
#include "SmartArray.h"
#include "Frame.h"
#include "ObjDesc.h"

class ACCharGenStartArea : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	SmartArray<Position> mPositionList;
};

class Skill_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	int skillNum;
	int normalCost;
	int primaryCost;
};

class Template_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	uint32_t iconImage;
	unsigned int titleID;
	int strength;
	int endurance;
	int coordination;
	int quickness;
	int focus;
	int self;
	SmartArray<int> mNormalSkillsList;
	SmartArray<int> mPrimarySkillsList;
};

class HairStyle_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t iconImage;
	bool bald;
	uint32_t alternateSetup;
	ObjDesc objDesc;
};

class EyesStrip_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t iconImage;
	uint32_t iconImage_Bald;
	ObjDesc objDesc;
	ObjDesc objDesc_Bald;
};

class FaceStrip_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t iconImage;
	ObjDesc objDesc;
};

class Style_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	uint32_t clothingTable;
	uint32_t weenieDefault;
};

class Sex_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	int scaling;
	uint32_t setup;
	uint32_t soundTable;
	uint32_t iconImage;
	ObjDesc objDesc;
	uint32_t physicsTable;
	uint32_t motionTable;
	uint32_t combatTable;
	uint32_t basePalette;
	uint32_t skinPalSet;
	SmartArray<uint32_t> mHairColorList;
	SmartArray<HairStyle_CG> mHairStyleList;
	SmartArray<uint32_t> mEyeColorList;
	SmartArray<EyesStrip_CG> mEyeStripList;
	SmartArray<FaceStrip_CG> mNoseStripList;
	SmartArray<FaceStrip_CG> mMouthStripList;
	SmartArray<Style_CG> mHeadgearList;
	SmartArray<Style_CG> mShirtList;
	SmartArray<Style_CG> mPantsList;
	SmartArray<Style_CG> mFootwearList;
	SmartArray<uint32_t> mClothingColorsList;

	inline HairStyle_CG* FindHairStyle(uint32_t head_id)
	{
		for (int i = 0; i < mHairStyleList.num_used; i++)
		{
			AnimPartChange *change = mHairStyleList.array_data[i].objDesc.firstAPChange;
			while (change != nullptr)
			{
				if (change->part_index == Head && change->part_id == head_id)
					return &(mHairStyleList.array_data[i]);
				
				change = change->next;
			}
		}
		return nullptr;
	}

	inline HairStyle_CG* FindHairStyleAltSetup(uint32_t setup_id)
	{
		for (int i = 0; i < mHairStyleList.num_used; i++)
		{
			if (mHairStyleList.array_data[i].alternateSetup == setup_id)
				return &(mHairStyleList.array_data[i]);
		}
		return nullptr;
	}
};

class HeritageGroup_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	uint32_t iconImage;
	uint32_t setupID;
	uint32_t environmentSetupID;
	int numAttributeCredits;
	int numSkillCredits;
	SmartArray<uint32_t> mPrimaryStartAreaList;
	SmartArray<uint32_t> mSecondaryStartAreaList;
	SmartArray<Skill_CG> mSkillList;
	SmartArray<Template_CG> mTemplateList;
	HashTable<uint32_t, Sex_CG> mGenderList; // actually a HashTable
};

class CharGenData : public DBObj
{
public:
};

class ACCharGenData : public CharGenData, public PackObj
{
public:
	DECLARE_DBOBJ(ACCharGenData);
	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	SmartArray<ACCharGenStartArea> mStartAreaList;
	HashTable<uint32_t, HeritageGroup_CG> mHeritageGroupList; // actually a HashTable
};
