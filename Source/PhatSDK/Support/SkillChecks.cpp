
#include "StdAfx.h"
#include "PhatSDK.h"
#include "SkillChecks.h"

double GetSkillChance(int skill, int difficulty)
{
	double chance = 1.0 - (1.0 / (1.0 + exp(0.03 * (skill - difficulty))));

	return min(1.0, max(0.0, chance));
}

double GetAppraisalSkillChance(int skill, int difficulty)
{
	double chance = 1.0 - (1.0 / (1.0 + exp(0.05 * (skill - difficulty))));

	return min(1.0, max(0.0, chance));
}

double GetMagicSkillChance(int skill, int difficulty)
{
	double chance = 1.0 - (1.0 / (1.0 + exp(0.07 * (skill - difficulty))));

	return min(1.0, max(0.0, chance));
}

int GetManaCost(int skill, int difficulty, int manaCost, int manaConversion)
{
	if (!manaConversion)
		return manaCost;

	//	A mana conversion skill check is made at 25 diff per level of
	//	spell, and the cost of the spell is reduced if the skill check is
	//	successful.The reduction is a random percentage from 0 to the
	//	chance of your mana conversion skill check success rate.In other
	//	words, if you have an 80 % chance of your mana conversion working
	//	on a spell, then 20 % of the time you will save nothing, and the
	//	other 80 % you will save between 0 and 80 % of the mana.Obviously,
	//	lower level spells will succeed much better, because you will have
	//	nearly a 100 % chance of conversion success.This is not a skill a
	//	mage should neglect, but because it tapers off, it probably should
	//	not be specialized.Unfortunately, a mana conversion of 300 is
	//	not much worse than 350.

	int manaConversionDifficulty = round((((float)difficulty / 50.0) + 1) * 25.0);

	double chance = GetSkillChance(manaConversion, manaConversionDifficulty);
	
	if (Random::RollDice(0.0, 1.0) > chance)
	{
		// fail conversion, full cost
		return manaCost;
	}

	// roll again to select conversion amount
	double conversionFactor = 1.0 - Random::RollDice(0.0, chance);

	return (int)(manaCost * conversionFactor);
}

bool GenericSkillCheck(int offense, int defense)
{
	double chance = GetSkillChance(offense, defense);

	if (Random::RollDice(0.0, 1.0) <= chance)
	{
		// succeeded
		return true;
	}

	// failed
	return false;
}

bool AppraisalSkillCheck(int offense, int defense)
{
	double chance = GetAppraisalSkillChance(offense, defense);

	if (Random::RollDice(0.0, 1.0) <= chance)
	{
		// succeeded
		return true;
	}

	// failed
	return false;
}

bool TryMagicResist(int offense, int defense)
{
	return !GenericSkillCheck(offense, defense);
}

bool TryMeleeEvade(int offense, int defense)
{
	return !GenericSkillCheck(offense, defense);
}

bool TryMissileEvade(int offense, int defense)
{
	return !GenericSkillCheck(offense, defense);
}
