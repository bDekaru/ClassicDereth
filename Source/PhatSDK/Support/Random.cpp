#include "stdafx.h"
#include "Random.h"
#include <limits.h>
#include <math.h>
#include <stdexcept>

double CSharpRandom::Sample() {
	//Including this division at the end gives us significantly improved
	//random number distribution.
	return (this->InternalSample()*(1.0 / MBIG));
}

int CSharpRandom::InternalSample() {
	int retVal;
	int locINext = this->inext;
	int locINextp = this->inextp;

	if (++locINext >= 56) locINext = 1;
	if (++locINextp >= 56) locINextp = 1;

	retVal = SeedArray[locINext] - SeedArray[locINextp];

	if (retVal == MBIG) retVal--;
	if (retVal < 0) retVal += MBIG;

	SeedArray[locINext] = retVal;

	inext = locINext;
	inextp = locINextp;

	return retVal;
}

CSharpRandom::CSharpRandom(int seed) {
	int ii;
	int mj, mk;

	//Initialize our Seed array.
	//This algorithm comes from Numerical Recipes in C (2nd Ed.)
	int subtraction = (seed == INT_MAX) ? INT_MAX : abs(seed);
	mj = MSEED - subtraction;
	SeedArray[55] = mj;
	mk = 1;
	for (int i = 1; i < 55; i++) {  //Apparently the range [1..55] is special (Knuth) and so we're wasting the 0'th position.
		ii = (21 * i) % 55;
		SeedArray[ii] = mk;
		mk = mj - mk;
		if (mk < 0) mk += MBIG;
		mj = SeedArray[ii];
	}
	for (int k = 1; k < 5; k++) {
		for (int i = 1; i < 56; i++) {
			SeedArray[i] -= SeedArray[1 + (i + 30) % 55];
			if (SeedArray[i] < 0) SeedArray[i] += MBIG;
		}
	}
	inext = 0;
	inextp = 21;
	seed = 1;
}

CSharpRandom::~CSharpRandom()
{
	delete SeedArray;
}

int CSharpRandom::Next() {
	return this->InternalSample();
}

double CSharpRandom::GetSampleForLargeRange() {

	int result = this->InternalSample();
	// Note we can't use addition here. The distribution will be bad if we do that.
	bool negative = (InternalSample() % 2 == 0) ? true : false;  // decide the sign based on second sample
	if (negative) {
		result = -result;
	}
	double d = result;
	d += (INT_MAX - 1); // get a number in range [0 .. 2 * Int32MaxValue - 1)
	d /= 2 * (double)(INT_MAX - 1);
	return d;
}

int CSharpRandom::Next(int minValue, int maxValue) {
	if (minValue > maxValue) {
		throw std::invalid_argument("minValue is larger than maxValue");
	}

	long range = (long)maxValue - minValue;
	if (range <= (long)INT_MAX) {
		return ((int)(this->Sample() * range) + minValue);
	}
	else {
		return (int)((long)(this->GetSampleForLargeRange() * range) + minValue);
	}
}


int CSharpRandom::Next(int maxValue) {
	if (maxValue < 0) {
		throw std::invalid_argument("maxValue must be positive");
	}

	return (int)(this->Sample()*maxValue);
}

double CSharpRandom::NextDouble() {
	return this->Sample();
}