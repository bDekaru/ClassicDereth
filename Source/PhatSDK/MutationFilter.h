
#pragma once

#include "Packable.h"

class EffectArgument : public PackObj
{
public:
	DECLARE_PACKABLE();

	int _type;

	union 
	{
		BYTE _raw[8];
		double dbl_value;
		int int_value;	
		struct quality_value_s
		{
			StatType statType;

			union
			{
				int statIndex;
				STypeInt intStat;
				STypeFloat floatStat;
			};
		} quality_value;
		struct range_value_s
		{
			float min;
			float max;
		} range_value;
	};

	// custom
	bool ResolveValue(class CACQualities *q, double *vars);
	void StoreValue(class CACQualities *q, double *vars);

	bool _isValid = false;

	/*
	bool _isFloat = false;
	int _resolvedValueInt = 0;
	double _resolvedValueFloat = 0.0;
	*/

	double _resolvedValue = 0.0;
};

class MutationEffect : public PackObj
{
public:
	DECLARE_PACKABLE();

	EffectArgument _argQuality;
	int _effectType;
	EffectArgument _arg1;
	EffectArgument _arg2;
};

class MutationEffectList : public PackObj
{
public:
	DECLARE_PACKABLE();

	double _probability;
	PackableList<MutationEffect> _effects;
};

class MutationOutcome : public PackObj
{
public:
	DECLARE_PACKABLE();

	PackableList<MutationEffectList> _effectList;
};

class MutationChance : public PackObj
{
public:
	DECLARE_PACKABLE();

	PackableList<double> _chances;
};

class Mutation : public PackObj
{
public:
	DECLARE_PACKABLE();

	MutationChance _chance;
	PackableList<MutationOutcome> _outcomes;
};

class CMutationFilter : public PackObj //, public DBObj
{
public:
	DECLARE_PACKABLE();

	//DECLARE_DBOBJ(CMutationFilter)
	//DECLARE_LEGACY_PACK_MIGRATOR()

	void TryMutate(class CACQualities *q); // custom

	PackableList<Mutation> _mutations;
};