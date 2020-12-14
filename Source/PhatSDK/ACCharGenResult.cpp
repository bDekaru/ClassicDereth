
#include <StdAfx.h>
#include "PhatSDK.h"
#include "ACCharGenResult.h"

ACCharGenResult::~ACCharGenResult()
{
	SafeDeleteArray(skillAdvancementClasses);
}

DEFINE_PACK(ACCharGenResult)
{
	UNFINISHED();
}

DEFINE_UNPACK(ACCharGenResult)
{
	return CG_UnPack(pReader);
}

bool ACCharGenResult::CG_UnPack(BinaryReader *pReader)
{
	uint32_t version = pReader->Read<uint32_t>();
	if (version != 1)
		return false;

	heritageGroup = (HeritageGroup) pReader->Read<uint32_t>();
	gender = (Gender) pReader->Read<uint32_t>();
	eyesStrip = pReader->Read<int>();
	noseStrip = pReader->Read<int>();
	mouthStrip = pReader->Read<int>();
	hairColor = pReader->Read<int>();
	eyeColor = pReader->Read<int>();
	hairStyle = pReader->Read<int>();
	headgearStyle = pReader->Read<int>();
	headgearColor = (PALETTE_TEMPLATE_ID) pReader->Read<uint32_t>();
	shirtStyle = pReader->Read<int>();
	shirtColor = (PALETTE_TEMPLATE_ID)pReader->Read<uint32_t>();
	trousersStyle = pReader->Read<int>();
	trousersColor = (PALETTE_TEMPLATE_ID)pReader->Read<uint32_t>();
	footwearStyle = pReader->Read<int>();
	footwearColor = (PALETTE_TEMPLATE_ID)pReader->Read<uint32_t>();
	skinShade = pReader->Read<double>();
	hairShade = pReader->Read<double>();
	headgearShade = pReader->Read<double>();
	shirtShade = pReader->Read<double>();
	trousersShade = pReader->Read<double>();
	footwearShade = pReader->Read<double>();
	templateNum = pReader->Read<int>();
	strength = pReader->Read<int>();
	endurance = pReader->Read<int>();
	coordination = pReader->Read<int>();
	quickness = pReader->Read<int>();
	focus = pReader->Read<int>();
	self = pReader->Read<int>();
	slot = pReader->Read<int>();
	classID = pReader->Read<uint32_t>();
	numSkills = pReader->Read<int>();
	if (numSkills >= NUM_SKILL)
		numSkills = NUM_SKILL;

	SafeDeleteArray(skillAdvancementClasses);
	if (numSkills > 0)
	{
		skillAdvancementClasses = new SKILL_ADVANCEMENT_CLASS[numSkills];
		for (int i = 0; i < numSkills; i++)
			skillAdvancementClasses[i] = (SKILL_ADVANCEMENT_CLASS)pReader->Read<uint32_t>();
	}

	name = pReader->ReadString();
	startArea = pReader->Read<uint32_t>();
	isAdmin = pReader->Read<int>();
	isEnvoy = pReader->Read<int>();
	return true;
}
