
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
	DWORD iconImage;
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

	DWORD iconImage;
	bool bald;
	DWORD alternateSetup;
	ObjDesc objDesc;
};

class EyesStrip_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD iconImage;
	DWORD iconImage_Bald;
	ObjDesc objDesc;
	ObjDesc objDesc_Bald;
};

class FaceStrip_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD iconImage;
	ObjDesc objDesc;
};

class Style_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	DWORD clothingTable;
	DWORD weenieDefault;
};

class Sex_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	int scaling;
	DWORD setup;
	DWORD soundTable;
	DWORD iconImage;
	ObjDesc objDesc;
	DWORD physicsTable;
	DWORD motionTable;
	DWORD combatTable;
	DWORD basePalette;
	DWORD skinPalSet;
	SmartArray<DWORD> mHairColorList;
	SmartArray<HairStyle_CG> mHairStyleList;
	SmartArray<DWORD> mEyeColorList;
	SmartArray<EyesStrip_CG> mEyeStripList;
	SmartArray<FaceStrip_CG> mNoseStripList;
	SmartArray<FaceStrip_CG> mMouthStripList;
	SmartArray<Style_CG> mHeadgearList;
	SmartArray<Style_CG> mShirtList;
	SmartArray<Style_CG> mPantsList;
	SmartArray<Style_CG> mFootwearList;
	SmartArray<DWORD> mClothingColorsList;
};

class HeritageGroup_CG : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::string name;
	DWORD iconImage;
	DWORD setupID;
	DWORD environmentSetupID;
	int numAttributeCredits;
	int numSkillCredits;
	SmartArray<DWORD> mPrimaryStartAreaList;
	SmartArray<DWORD> mSecondaryStartAreaList;
	SmartArray<Skill_CG> mSkillList;
	SmartArray<Template_CG> mTemplateList;
	HashTable<unsigned long, Sex_CG> mGenderList; // actually a HashTable
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
	HashTable<unsigned long, HeritageGroup_CG> mHeritageGroupList; // actually a HashTable
};
