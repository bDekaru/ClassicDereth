
#include <StdAfx.h>
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
	skillNum = pReader->Read<uint32_t>();
	normalCost = pReader->Read<uint32_t>();
	primaryCost = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK(Template_CG)
{
}

DEFINE_UNPACK(Template_CG)
{
	name = pReader->ReadSerializedString();

	iconImage = pReader->Read<uint32_t>();
	titleID = pReader->Read<uint32_t>();
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
	iconImage = pReader->Read<uint32_t>();
	bald = pReader->Read<BYTE>() ? true : false;

	alternateSetup = pReader->Read<uint32_t>();

	pReader->ReadAlign();
	objDesc.UnPack(pReader);
	return true;
}

DEFINE_PACK(EyesStrip_CG)
{
}

DEFINE_UNPACK(EyesStrip_CG)
{
	iconImage = pReader->Read<uint32_t>();
	iconImage_Bald = pReader->Read<uint32_t>();
	
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
	iconImage = pReader->Read<uint32_t>();	
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
	clothingTable = pReader->Read<uint32_t>();
	weenieDefault = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK(Sex_CG)
{
}

DEFINE_UNPACK(Sex_CG)
{
	name = pReader->ReadSerializedString();

	scaling = pReader->Read<int>();
	setup = pReader->Read<uint32_t>();
	soundTable = pReader->Read<uint32_t>();
	iconImage = pReader->Read<uint32_t>();
	basePalette = pReader->Read<uint32_t>();
	skinPalSet = pReader->Read<uint32_t>();
	physicsTable = pReader->Read<uint32_t>();
	motionTable = pReader->Read<uint32_t>();
	combatTable = pReader->Read<uint32_t>();

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
	iconImage = pReader->Read<uint32_t>();
	setupID = pReader->Read<uint32_t>();
	environmentSetupID = pReader->Read<uint32_t>();
	numAttributeCredits = pReader->Read<uint32_t>();
	numSkillCredits = pReader->Read<uint32_t>();
	
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
	pReader->Read<uint32_t>(); // ID
	pReader->Read<uint32_t>(); // ID again..

	mStartAreaList.UnSerializePackObj(pReader);
	mHeritageGroupList.UnPack(pReader);

	return true;
}
