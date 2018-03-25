#include "StdAfx.h"
#include "PhatSDK.h"
#include "RandomRange.h"
#include "Random.h"
#include <random>

std::random_device randomDevice;
CSharpRandom rng = CSharpRandom(randomDevice());

//std::mt19937 randomGenerator(randomDevice());
//
//int GenerateRandomInt(int minInclusive, int maxInclusive)
//{
//	std::uniform_int_distribution<int> randomDistribution(minInclusive, maxInclusive);
//	return randomDistribution(randomGenerator);
//}
//
//double GenerateRandomDouble(double minInclusive, double maxInclusive)
//{
//	std::uniform_real_distribution<double> randomDistribution(minInclusive, maxInclusive);
//	return randomDistribution(randomGenerator);
//}

void testRandomValueGenerator()
{
	int testRolls = 10000;
	std::map<int, int> valueDistribution;
	for (int i = 0; i < testRolls; i++)
	{
		//int test = (int)floor(getRandomDouble(0.0, 1.0, eRandomFormula::favorMid, 2, 0) * 10);
		int test = getRandomNumber(1, 10, eRandomFormula::favorSpecificValue, 3, 0, 4);
		//int test = getRandomNumber(0, 10, eRandomFormula::favorMid, 2);
		if (valueDistribution.find(test) != valueDistribution.end())
			valueDistribution[test]++;
		else
			valueDistribution.emplace(test, 1);
	}

	for each(auto entry in valueDistribution)
	{
		LOG(Data, Error, "value: %d amount: %d percent: %f\n", entry.first, entry.second, entry.second * 100.0 / testRolls);
	}
}

int getRandomNumberWithFavoredValue(int minInclusive, int maxInclusive, double favorValue, double favorStrength)
{
	int numValues = (maxInclusive - minInclusive) + 1;
	float maxWeight = (numValues) * 1000;

	std::vector<IntRange> ranges;

	int value = minInclusive;
	for (int i = 0; i < numValues; i++)
	{
		ranges.push_back(IntRange(value, maxWeight / (float)pow(1 + ((pow(favorStrength, 2) / numValues)), abs(favorValue - value))));
		value++;
	}

	return GetRandomNumberFromRange(ranges);
}

int getRandomNumberExclusive(int maxExclusive)
{
	return getRandomNumber(0, maxExclusive - 1, eRandomFormula::equalDistribution, 0, 0, 0);
}

int getRandomNumberExclusive(int maxExclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	return getRandomNumber(0, maxExclusive - 1, formula, favorSpecificValue, favorStrength, favorModifier);
}

int getRandomNumber(int maxInclusive)
{
	return getRandomNumber(0, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
}

int getRandomNumber(int minInclusive, int maxInclusive)
{
	return getRandomNumber(minInclusive, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
}

int getRandomNumber(int minInclusive, int maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	int numbersAmount = maxInclusive - minInclusive;
	switch (formula)
	{
	case eRandomFormula::favorSpecificValue:
	{
		favorSpecificValue = favorSpecificValue + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorLow:
	{
		favorSpecificValue = minInclusive + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorMid:
	{
		int midValue = (int)round(((double)(maxInclusive - minInclusive) / 2)) + minInclusive;
		favorSpecificValue = midValue + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorHigh:
	{
		favorSpecificValue = maxInclusive - (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	default:
	case eRandomFormula::equalDistribution:
	{
		return rng.Next(minInclusive, maxInclusive + 1);
		//return Random::GenInt(minInclusive, maxInclusive);
	}
	}
}

std::set<int> getRandomNumbersNoRepeat(int amount, int minInclusive, int maxInclusive)
{
	std::set<int> numbers;
	for (int i = 0; i < amount; i++)
	{
		numbers.emplace(getRandomNumberNoRepeat(minInclusive, maxInclusive, numbers));
	}
	return numbers;
}

int getRandomNumberNoRepeat(int minInclusive, int maxInclusive, std::set<int> notThese, int maxTries)
{
	int potentialValue = getRandomNumber(minInclusive, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
	for (int i = 0; i < maxTries; i++)
	{
		potentialValue = getRandomNumber(minInclusive, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
		if (notThese.find(potentialValue) == notThese.end())
			break;
	}
	return potentialValue;
}

double getRandomDouble(double maxInclusive)
{
	return getRandomDouble(0, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
}

double getRandomDouble(double maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	return getRandomDouble(0, maxInclusive, formula, favorStrength, favorModifier, favorSpecificValue);
}

double getRandomDouble(double minInclusive, double maxInclusive)
{
	return getRandomDouble(minInclusive, maxInclusive, eRandomFormula::equalDistribution, 0, 0, 0);
}

double getRandomDouble(double minInclusive, double maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	double decimalPlaces = 1000;
	int minInt = (int)round(minInclusive * decimalPlaces);
	int maxInt = (int)round(maxInclusive * decimalPlaces);

	int favorSpecificValueInt = (int)round(favorSpecificValue * decimalPlaces);

	int randomInt = getRandomNumber(minInt, maxInt, formula, favorStrength, favorModifier, favorSpecificValue);
	double returnValue = randomInt / decimalPlaces;

	returnValue = min(returnValue, maxInclusive);
	returnValue = max(returnValue, minInclusive);

	return returnValue;
}

int GetRandomNumberFromRange(std::vector<IntRange> ranges)
{
	if (ranges.size() == 1)
		return rng.Next(ranges[0].Min, ranges[0].Max);

	float total = 0.f;
	for (int i = 0; i < ranges.size(); i++)
		total += ranges[i].Weight;

	float r = rng.NextDouble();
	float s = 0.f;

	int cnt = (int)ranges.size() - 1;
	for (int i = 0; i < cnt; i++)
	{
		s += ranges[i].Weight / total;
		if (s >= r)
		{
			if (ranges[i].Min == ranges[i].Max)
				return ranges[i].Min;
			else
				return rng.Next(ranges[i].Min, ranges[i].Max);
		}
	}

	if (ranges[cnt].Min == ranges[cnt].Max)
		return ranges[cnt].Min;
	else
		return rng.Next(ranges[cnt].Min, ranges[cnt].Max);
}