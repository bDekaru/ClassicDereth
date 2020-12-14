
#include <StdAfx.h>
#include "PhatSDK.h"
#include "MutationFilter.h"
#include "Qualities.h"

using variable_storage_t = std::array<EffectArgument, 32>;
thread_local variable_storage_t gt_variables;

//DEFINE_DBOBJ(CMutationFilter, MutationFilters)
//DEFINE_LEGACY_PACK_MIGRATOR(CMutationFilter)

EffectArgument EffectArgument::operator+(const EffectArgument &other)
{
	switch (_type)
	{
	case EffectArgumentType::Double:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument(dbl_value + other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(dbl_value + (double)other.int_value);
		}
		break;

	case EffectArgumentType::Int:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument((double)int_value + other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(int_value + other.int_value);
		}
		break;
	}

	throw new std::runtime_error("EffectArgument: Invalid Addition Operation");
}

EffectArgument EffectArgument::operator-(const EffectArgument &other)
{
	switch (_type)
	{
	case EffectArgumentType::Double:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument(dbl_value - other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(dbl_value - (double)other.int_value);
		}
		break;

	case EffectArgumentType::Int:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument((double)int_value - other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(int_value - other.int_value);
		}
		break;
	}

	throw new std::runtime_error("EffectArgument: Invalid Subtraction Operation");
}

EffectArgument EffectArgument::operator*(const EffectArgument &other)
{
	switch (_type)
	{
	case EffectArgumentType::Double:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument(dbl_value * other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(dbl_value * (double)other.int_value);
		}
		break;

	case EffectArgumentType::Int:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument((double)int_value * other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(int_value * other.int_value);
		}
		break;
	}

	throw new std::runtime_error("EffectArgument: Invalid Multiplication Operation");
}

EffectArgument EffectArgument::operator/(const EffectArgument &other)
{
	switch (_type)
	{
	case EffectArgumentType::Double:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument(dbl_value / other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(dbl_value / (double)other.int_value);
		}
		break;

	case EffectArgumentType::Int:
		switch (other._type)
		{
		case EffectArgumentType::Double:
			return EffectArgument((double)int_value / other.dbl_value);
		case EffectArgumentType::Int:
			return EffectArgument(int_value / other.int_value);
		}
		break;
	}

	throw new std::runtime_error("EffectArgument: Invalid Division Operation");
}

bool EffectArgument::operator<(const EffectArgument &other)
{
	assert(_type == other._type);
	switch (_type)
	{
	case EffectArgumentType::Double:
		return dbl_value < other.dbl_value;

	case EffectArgumentType::Int:
		return int_value < other.int_value;
	}
	return false;
}

bool EffectArgument::operator>(const EffectArgument &other)
{
	assert(_type == other._type);
	switch (_type)
	{
	case EffectArgumentType::Double:
		return dbl_value > other.dbl_value;

	case EffectArgumentType::Int:
		return int_value > other.int_value;
	}
	return false;
}

bool EffectArgument::ResolveValue(CACQualities &q)
{
	// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
	switch (_type)
	{
	case EffectArgumentType::Double:
	case EffectArgumentType::Int:
		// these are ok as-is
		_isValid = true;
		break;

	case EffectArgumentType::Quality: // quality
		switch (quality_value.statType)
		{
		case Int_StatType:
			_type = EffectArgumentType::Int;
			if (q.InqInt((STypeInt)quality_value.statIndex, int_value, TRUE, TRUE))
			{
				_isValid = true;
			}
			break;

		case Bool_StatType:
			int_value = q.GetBool((STypeBool)quality_value.statIndex, 0);
			_type = EffectArgumentType::Int;
			_isValid = true;
			break;

		case Float_StatType:
			_type = EffectArgumentType::Double;
			if (q.InqFloat((STypeFloat)quality_value.statIndex, dbl_value, TRUE))
			{
				_isValid = true;
			}
			break;

		case DID_StatType:
			int_value = (int)q.GetDID((STypeDID)quality_value.statIndex, 0);
			_type = EffectArgumentType::Int;
			_isValid = true;
			break;
		}

		break;

	case EffectArgumentType::Random:
		dbl_value = Random::GenFloat(range_value.min, range_value.max);
		_type = EffectArgumentType::Double;
		_isValid = true;
		break;

	case EffectArgumentType::Variable:
		if (int_value < 0 || int_value >= gt_variables.size())
			break;

		*this = gt_variables[int_value];
		_isValid = true;
		break;
	}

	return _isValid;
}

void EffectArgument::StoreValue(CACQualities &q, const EffectArgument &result)
{
	// here the resolved value (result) is applied to the qualities specified by our values

	if (!result._isValid)
	{
		return;
	}

	switch (_type)
	{
	case EffectArgumentType::Quality: // quality
		switch (quality_value.statType)
		{
		case Int_StatType:
			q.SetInt((STypeInt)quality_value.statIndex, result.toInt());
			break;

		case Bool_StatType:
			q.SetBool((STypeBool)quality_value.statIndex, result.toInt());
			break;

		case Float_StatType:
			q.SetFloat((STypeFloat)quality_value.statIndex, result.toDouble());
			break;

		case DID_StatType:
			q.SetDataID((STypeDID)quality_value.statIndex, (uint32_t)(result.toInt()));
			break;
		}

		break;

	case EffectArgumentType::Variable:
		if (int_value < 0 || int_value >= gt_variables.size())
			break;

		gt_variables[int_value] = result;
		break;
	}
}

void MutationEffect::TryMutate(CACQualities &q)
{
	// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
	// a=b,a+=b,a-=b,a*=b,a/=b,a=a<b?b:a+c,a=a>b?b:a-c,a+=b*c,a+=b/c,a-=b*c,a-=b/c,a=b+c,a=b-c,a=b*c,a=b/c

	// do not make changes to the members since this object will be reused

	EffectArgument arg0 = _argQuality;
	EffectArgument arg1 = _arg1;
	EffectArgument arg2 = _arg2;

	arg0.ResolveValue(q);
	arg1.ResolveValue(q);
	arg2.ResolveValue(q);

	EffectArgument result;
	switch (_effectType)
	{
	case MutationEffectType::Assign:
		assert(arg1._isValid);
		result = arg1;
		break;

	case MutationEffectType::Add:
		assert(arg1._isValid);
		result = arg0 + _arg1;
		break;

	case MutationEffectType::Subtract:
		assert(arg1._isValid);
		result = arg0 - arg1;
		break;

	case MutationEffectType::Multiply:
		assert(arg1._isValid);
		result = arg0 * arg1;
		break;

	case MutationEffectType::Divide:
		assert(arg1._isValid);
		result = arg0 / arg1;
		break;

	case MutationEffectType::AtLeastAdd:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = (!arg0._isValid || arg0 < arg1) ? arg1 : arg0 + arg2;
		break;

	case MutationEffectType::AtMostSubtract:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = (!arg0._isValid || arg0 > arg1) ? arg1 : arg0 - arg2;
		break;

	case MutationEffectType::AddMultiply:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg0 + (arg1 * arg2);
		break;

	case MutationEffectType::AddDivide:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg0 + (arg1 / arg2);
		break;

	case MutationEffectType::SubtractMultiply:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg0 - (arg1 * arg2);
		break;

	case MutationEffectType::SubtractDivide:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg0 - (arg1 / arg2);
		break;

	case MutationEffectType::AssignAdd:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg1 + arg2;
		break;

	case MutationEffectType::AssignSubtract:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg1 - arg2;
		break;

	case MutationEffectType::AssignMultiply:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg1 * arg2;
		break;

	case MutationEffectType::AssignDivide:
		assert(arg1._isValid);
		assert(arg2._isValid);
		result = arg1 / arg2;
		break;
	}

	_argQuality.StoreValue(q, result);
}

void MutationEffectList::TryMutate(CACQualities &q)
{
	for (auto &e : _effects)
	{
		e.TryMutate(q);
	}
}

void MutationOutcome::TryMutate(CACQualities &q, double roll)
{
	for (MutationEffectList &el : _effectList)
	{
		if (el._probability < roll)
			continue;

		el.TryMutate(q);
		break;
	}
}

void Mutation::TryMutate(CACQualities &q, int tier, double roll)
{
	// does it pass the roll to mutate for the tier
	if (!_chance.success(tier, roll))
		return;

	// roll again to select the mutations
	roll = Random::RollDice(0.0f, 1.0f);

	for (auto &o : _outcomes)
	{
		o.TryMutate(q, roll);
	}
}

void CMutationFilter::TryMutate(CACQualities &q, int tier)
{
	double roll = Random::RollDice(0.0f, 1.0f);

	for (auto &m : _mutations)
	{
		m.TryMutate(q, tier, roll);
	}
}

DEFINE_PACK(CMutationFilter)
{
	UNFINISHED();
}

DEFINE_UNPACK(CMutationFilter)
{
	// pReader->Read<uint32_t>(); // file ID

	_mutations.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(CMutationFilter)
{
	_mutations.PackJson(writer);
}

DEFINE_UNPACK_JSON(CMutationFilter)
{
	_mutations.UnPackJson(reader);
	return true;
}

DEFINE_PACK(Mutation)
{
	UNFINISHED();
}

DEFINE_UNPACK(Mutation)
{
	_chance.UnPack(pReader);
	_outcomes.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(Mutation)
{
	PackObjJson(writer, "chance", _chance);
	PackObjJson(writer, "outcomes", _outcomes);
}

DEFINE_UNPACK_JSON(Mutation)
{
	UnPackObjJson(reader, "chance", _chance);
	UnPackObjJson(reader, "outcomes", _outcomes);

	return true;
}

DEFINE_PACK(MutationChance)
{
	UNFINISHED();
}

DEFINE_UNPACK(MutationChance)
{
	_chances.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(MutationChance)
{
	_chances.PackJson(writer);
}

DEFINE_UNPACK_JSON(MutationChance)
{
	_chances.UnPackJson(reader);
	return true;
}

DEFINE_PACK(MutationOutcome)
{
	UNFINISHED();
}

DEFINE_UNPACK(MutationOutcome)
{
	_effectList.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(MutationOutcome)
{
	_effectList.PackJson(writer);
}

DEFINE_UNPACK_JSON(MutationOutcome)
{
	_effectList.UnPackJson(reader);
	return true;
}

DEFINE_PACK(MutationEffectList)
{
	UNFINISHED();
}

DEFINE_UNPACK(MutationEffectList)
{
	_probability = pReader->Read<double>();
	_effects.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(MutationEffectList)
{
	writer["probability"] = _probability;
	PackObjJson(writer, "effects", _effects);
}

DEFINE_UNPACK_JSON(MutationEffectList)
{
	_probability = reader["probability"];
	UnPackObjJson(reader, "effects", _effects);
	return true;
}

DEFINE_PACK(MutationEffect)
{
	UNFINISHED();
}

DEFINE_UNPACK(MutationEffect)
{
	_argQuality.UnPack(pReader);
	_effectType = pReader->Read<int>();
	_arg1.UnPack(pReader);
	_arg2.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(MutationEffect)
{
	writer["type"] = _effectType;
	PackObjJson(writer, "quality", _argQuality);
	PackObjJson(writer, "arg1", _arg1);
	PackObjJson(writer, "arg2", _arg2);
}

DEFINE_UNPACK_JSON(MutationEffect)
{
	_effectType = reader["type"];
	UnPackObjJson(reader, "quality", _argQuality);
	UnPackObjJson(reader, "arg1", _arg1);
	UnPackObjJson(reader, "arg2", _arg2);
	return true;
}

DEFINE_PACK(EffectArgument)
{
	UNFINISHED();
}

DEFINE_UNPACK(EffectArgument)
{
	_type = pReader->Read<EffectArgumentType>();
	memcpy(_raw, pReader->ReadArray(8), 8);
	return true;
}

DEFINE_PACK_JSON(EffectArgument)
{
	writer["type"] = _type;
	writer["data"] = json::parse(std::begin(_raw), std::end(_raw));
}

DEFINE_UNPACK_JSON(EffectArgument)
{
	_type = reader["type"].get<EffectArgumentType>();
	json data = reader["data"];
	std::copy(data.begin(), data.end(), _raw);
	return true;
}
