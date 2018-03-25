// This is the C# random ported to C++
// Credit goes to Demonslay335 of stackoverflow.com

#include <limits>

#pragma once
class CSharpRandom
{
private:
	const int MBIG = INT_MAX;
	const int MSEED = 161803398;
	const int MZ = 0;

	int inext;
	int inextp;
	int *SeedArray = new int[56]();

	double Sample();
	double GetSampleForLargeRange();
	int InternalSample();

public:
	CSharpRandom(int seed);
	~CSharpRandom();
	int Next();
	int Next(int minValue, int maxValue);
	int Next(int maxValue);
	double NextDouble();
};