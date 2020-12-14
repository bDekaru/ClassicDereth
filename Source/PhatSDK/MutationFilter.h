
#pragma once

#include "Packable.h"

class CACQualities;

enum struct EffectArgumentType : int
{
	Invalid = 0,
	Double,
	Int,
	Quality,
	Random,
	Variable
	// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
};

class EffectArgument : public PackObj, public PackableJson
{
public:
	EffectArgument() = default;
	virtual ~EffectArgument() = default;

	EffectArgument(const EffectArgument &effect) = default;
	EffectArgument(double val):
		_type(EffectArgumentType::Double), dbl_value(val), _isValid(true) {}
	EffectArgument(int val) :
		_type(EffectArgumentType::Int), int_value(val), _isValid(true) {}

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	EffectArgument operator+(const EffectArgument &other);
	EffectArgument operator-(const EffectArgument &other);
	EffectArgument operator*(const EffectArgument &other);
	EffectArgument operator/(const EffectArgument &other);

	bool operator<(const EffectArgument &other);
	bool operator>(const EffectArgument &other);

	bool operator==(const EffectArgument &other) { return std::memcmp(this, &other, sizeof(EffectArgument)); }
	bool operator!=(const EffectArgument &other) { return !std::memcmp(this, &other, sizeof(EffectArgument)); }

	EffectArgument& operator=(const EffectArgument &other) = default;

	// output conversions
	double toDouble() const
	{
		switch (_type)
		{
		case EffectArgumentType::Int:
				return (double)int_value;
		case EffectArgumentType::Double:
			return dbl_value;
		}
		assert(false);
		return 0.0;
	}

	int toInt() const
	{
		switch (_type)
		{
		case EffectArgumentType::Int:
			return int_value;
		case EffectArgumentType::Double:
			return (int)dbl_value;
		}
		assert(false);
		return 0;
	}

	EffectArgumentType _type;

	union 
	{
		BYTE _raw[8];
		double dbl_value;
		int int_value;	
		struct /*quality_value_s*/
		{
			StatType statType;

			union
			{
				int statIndex;
				STypeInt intStat;
				STypeFloat floatStat;
			};
		} quality_value;
		struct /*range_value_s*/
		{
			float min;
			float max;
		} range_value;
	};

	// custom
	bool ResolveValue(CACQualities &q);
	void StoreValue(CACQualities &q, const EffectArgument &result);

	bool _isValid = false;
};

enum MutationEffectType : int
{
	Assign = 0,
	Add,
	Subtract,
	Multiply,
	Divide,
	AtLeastAdd,
	AtMostSubtract,
	AddMultiply,
	AddDivide,
	SubtractMultiply,
	SubtractDivide,
	AssignAdd,
	AssignSubtract,
	AssignMultiply,
	AssignDivide

};

class MutationEffect : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	void TryMutate(CACQualities &q);

	EffectArgument _argQuality;
	int _effectType;
	EffectArgument _arg1;
	EffectArgument _arg2;
};

class MutationEffectList : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	void TryMutate(CACQualities &q);

	double _probability;
	PackableListWithJson<MutationEffect> _effects;
};

class MutationOutcome : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	void TryMutate(CACQualities &q, double roll);

	PackableListWithJson<MutationEffectList> _effectList;
};

class MutationChance : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	bool success(int tier, double roll)
	{
		if (tier < 0 || tier >= _chances.size())
			return false;

		// this guy is opposite from most rolls, 1.0 here means 100%
		// we'll use 1-chance to accomodate)
		return (1.0 - *_chances.GetAt(tier)) < roll;
	}

	PackableListWithJson<double> _chances;
};

class Mutation : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	void TryMutate(CACQualities &q, int tier, double roll);

	MutationChance _chance;
	PackableListWithJson<MutationOutcome> _outcomes;
};

class CMutationFilter : public PackObj, public PackableJson //, public DBObj
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	//DECLARE_DBOBJ(CMutationFilter)
	//DECLARE_LEGACY_PACK_MIGRATOR()

	void TryMutate(CACQualities &q, int tier); // custom

	PackableListWithJson<Mutation> _mutations;
};
