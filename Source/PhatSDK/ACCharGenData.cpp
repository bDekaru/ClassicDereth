
#include "StdAfx.h"
#include "PhatSDK.h"
#include "ACCharGenData.h"

DEFINE_PACK(ACCharGenStartArea)
{
	UNFINISHED();
}

DEFINE_UNPACK(ACCharGenStartArea)
{
	name = pReader->ReadSerializedString();	
	mPositionList.UnSerializePackObj(pReader);
	return true;
}

DEFINE_PACK(Skill_CG)
{
}

DEFINE_UNPACK(Skill_CG)
{
	skillNum = pReader->Read<DWORD>();
	normalCost = pReader->Read<DWORD>();
	primaryCost = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(Template_CG)
{
}

DEFINE_UNPACK(Template_CG)
{
	name = pReader->ReadSerializedString();

	iconImage = pReader->Read<DWORD>();
	titleID = pReader->Read<DWORD>();
	strength = pReader->Read<int>();
	endurance = pReader->Read<int>();
	coordination = pReader->Read<int>();
	quickness = pReader->Read<int>();
	focus = pReader->Read<int>();
	self = pReader->Read<int>();
	mNormalSkillsList.UnSerialize(pReader);
	mPrimarySkillsList.UnSerialize(pReader);
	return true;
}

DEFINE_PACK(HairStyle_CG)
{
}

DEFINE_UNPACK(HairStyle_CG)
{
	iconImage = pReader->Read<DWORD>();
	bald = pReader->Read<BYTE>() ? true : false;

	alternateSetup = pReader->Read<DWORD>();

	pReader->ReadAlign();
	objDesc.UnPack(pReader);
	return true;
}

DEFINE_PACK(EyesStrip_CG)
{
}

DEFINE_UNPACK(EyesStrip_CG)
{
	iconImage = pReader->Read<DWORD>();
	iconImage_Bald = pReader->Read<DWORD>();
	
	pReader->ReadAlign();
	objDesc.UnPack(pReader);

	pReader->ReadAlign();
	objDesc_Bald.UnPack(pReader);

	return true;
}

DEFINE_PACK(FaceStrip_CG)
{
}

DEFINE_UNPACK(FaceStrip_CG)
{
	iconImage = pReader->Read<DWORD>();	
	pReader->ReadAlign();
	objDesc.UnPack(pReader);

	return true;
}

DEFINE_PACK(Style_CG)
{
}

DEFINE_UNPACK(Style_CG)
{
	name = pReader->ReadSerializedString();
	clothingTable = pReader->Read<DWORD>();
	weenieDefault = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(Sex_CG)
{
}

DEFINE_UNPACK(Sex_CG)
{
	name = pReader->ReadSerializedString();

	scaling = pReader->Read<int>();
	setup = pReader->Read<DWORD>();
	soundTable = pReader->Read<DWORD>();
	iconImage = pReader->Read<DWORD>();
	basePalette = pReader->Read<DWORD>();
	skinPalSet = pReader->Read<DWORD>();
	physicsTable = pReader->Read<DWORD>();
	motionTable = pReader->Read<DWORD>();
	combatTable = pReader->Read<DWORD>();

	pReader->ReadAlign();
	objDesc.UnPack(pReader);
	
	mHairColorList.UnSerialize(pReader);
	mHairStyleList.UnSerializePackObj(pReader);
	mEyeColorList.UnSerialize(pReader);
	mEyeStripList.UnSerializePackObj(pReader);
	mNoseStripList.UnSerializePackObj(pReader);
	mMouthStripList.UnSerializePackObj(pReader);
	mHeadgearList.UnSerializePackObj(pReader);
	mShirtList.UnSerializePackObj(pReader);
	mPantsList.UnSerializePackObj(pReader);
	mFootwearList.UnSerializePackObj(pReader);
	mClothingColorsList.UnSerialize(pReader);

	return true;
}

DEFINE_PACK(HeritageGroup_CG)
{
}

DEFINE_UNPACK(HeritageGroup_CG)
{
	name = pReader->ReadSerializedString();
	iconImage = pReader->Read<DWORD>();
	setupID = pReader->Read<DWORD>();
	environmentSetupID = pReader->Read<DWORD>();
	numAttributeCredits = pReader->Read<DWORD>();
	numSkillCredits = pReader->Read<DWORD>();
	
	mPrimaryStartAreaList.UnSerialize(pReader);
	mSecondaryStartAreaList.UnSerialize(pReader);
	mSkillList.UnSerializePackObj(pReader);
	mTemplateList.UnSerializePackObj(pReader);
	mGenderList.UnPack(pReader);
	return true;
}

DEFINE_DBOBJ(ACCharGenData, CharGenTables);
DEFINE_LEGACY_PACK_MIGRATOR(ACCharGenData);

DEFINE_PACK(ACCharGenData)
{
	UNFINISHED();
}

DEFINE_UNPACK(ACCharGenData)
{
	pReader->Read<DWORD>(); // ID
	pReader->Read<DWORD>(); // ID again..

	mStartAreaList.UnSerializePackObj(pReader);
	mHeritageGroupList.UnPack(pReader);

	return true;
}
