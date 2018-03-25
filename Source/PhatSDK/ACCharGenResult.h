
#pragma once

#include "Packable.h"
#include "GameEnums.h"

class CharGenResult : public PackObj
{
};

class ACCharGenResult : public CharGenResult
{
public:
	DECLARE_PACKABLE();

	bool CG_UnPack(BinaryReader *pReader);

	HeritageGroup heritageGroup = Invalid_HeritageGroup;
	Gender gender = Invalid_Gender;
	int eyesStrip = 0;
	int noseStrip = 0;
	int mouthStrip = 0;
	int hairColor = 0;
	int eyeColor = 0;
	int hairStyle = 0;
	int headgearStyle = 0;
	int shirtStyle = 0;
	int trousersStyle = 0;
	int footwearStyle = 0;
	PALETTE_TEMPLATE_ID headgearColor = PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
	PALETTE_TEMPLATE_ID shirtColor = PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
	PALETTE_TEMPLATE_ID trousersColor = PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
	PALETTE_TEMPLATE_ID footwearColor = PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
	long double skinShade = 0;
	long double hairShade = 0;
	long double headgearShade = 0;
	long double shirtShade = 0;
	long double trousersShade = 0;
	long double footwearShade = 0;
	int templateNum = 0;
	int strength = 0;
	int endurance = 0;
	int coordination = 0;
	int quickness = 0;
	int focus = 0;
	int self = 0;
	int numSkills = 0;
	SKILL_ADVANCEMENT_CLASS *skillAdvancementClasses = NULL;
	std::string name;
	int slot = 0;
	DWORD classID = 0;
	unsigned int startArea = 0;
	int isAdmin = 0;
	int isEnvoy = 0;
};


