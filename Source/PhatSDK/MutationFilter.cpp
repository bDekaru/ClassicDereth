
#include "StdAfx.h"
#include "PhatSDK.h"
#include "MutationFilter.h"
#include "Qualities.h"

//DEFINE_DBOBJ(CMutationFilter, MutationFilters)
//DEFINE_LEGACY_PACK_MIGRATOR(CMutationFilter)

// custom
bool EffectArgument::ResolveValue(CACQualities *q, double *vars)
{
	// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
	switch (_type)
	{
	case 1:
		_resolvedValue = dbl_value;
		_isValid = true;
		break;

	case 2:
		_resolvedValue = int_value;
		_isValid = true;
		break;

	case 3: // quality
		switch (quality_value.statType)
		{
		case Int_StatType:
			_resolvedValue = q->GetInt((STypeInt)quality_value.statIndex, 0);
			_isValid = true;
			break;

		case Bool_StatType:
			_resolvedValue = q->GetBool((STypeBool)quality_value.statIndex, 0);
			_isValid = true;
			break;

		case Float_StatType:
			_resolvedValue = q->GetFloat((STypeFloat)quality_value.statIndex, 0.0);
			_isValid = true;
			break;

		case DID_StatType:
			_resolvedValue = (int)q->GetDID((STypeDID)quality_value.statIndex, 0);
			_isValid = true;
			break;
		}

		break;

	case 4:
		_resolvedValue = g_pPhatSDK->GetRandomFloat(range_value.min, range_value.max);
		_isValid = true;
		break;

	case 5:
		if (int_value < 0 || int_value >= 256)
			break;

		_resolvedValue = vars[int_value];
		_isValid = true;
		break;
	}

	return _isValid;
}

void EffectArgument::StoreValue(CACQualities *q, double *vars)
{
	// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
	
	if (!_isValid)
	{
		return;
	}

	switch (_type)
	{
	case 3: // quality
		switch (quality_value.statType)
		{
		case Int_StatType:
			q->SetInt((STypeInt)quality_value.statIndex, (int)(_resolvedValue + 0.5));
			break;

		case Bool_StatType:
			q->SetBool((STypeBool)quality_value.statIndex, (int)(_resolvedValue + 0.5));
			break;

		case Float_StatType:
			q->SetFloat((STypeFloat)quality_value.statIndex, _resolvedValue);
			break;

		case DID_StatType:
			q->SetDataID((STypeDID)quality_value.statIndex, (DWORD)(_resolvedValue + 0.5));
			break;
		}

		break;
		
	case 5:
		if (int_value < 0 || int_value >= 256)
			break;

		vars[int_value] = _resolvedValue;
		break;
	}
}

void CMutationFilter::TryMutate(class CACQualities *q)
{
	// this isn't right, don't use this

	double vars[256];
	memset(vars, 0, sizeof(vars));

	for (auto &m : _mutations)
	{
		for (auto &o : m._outcomes)
		{
			float dice = Random::RollDice(0.0, 1.0);

			for (MutationEffectList &el : o._effectList)
			{
				if (el._probability > dice)
					continue;

				for (auto &e : el._effects)
				{
					// type:enum - invalid, double, int32, quality (2 int32s: type and quality), float range (min, max), variable index (int32)
					// a=b,a+=b,a-=b,a*=b,a/=b,a=a<b?b:a+c,a=a>b?b:a-c,a+=b*c,a+=b/c,a-=b*c,a-=b/c,a=b+c,a=b-c,a=b*c,a=b/c
					
					e._argQuality.ResolveValue(q, vars);
					e._arg1.ResolveValue(q, vars);
					e._arg2.ResolveValue(q, vars);

					assert(e._argQuality._isValid);

					switch (e._effectType)
					{
					case 0:
						assert(e._arg1._isValid);
						e._argQuality._resolvedValue = e._arg1._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 1:
						assert(e._arg1._isValid);
						e._argQuality._resolvedValue += e._arg1._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 2:
						assert(e._arg1._isValid);
						e._argQuality._resolvedValue -= e._arg1._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 3:
						assert(e._arg1._isValid);
						e._argQuality._resolvedValue *= e._arg1._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 4:
						assert(e._arg1._isValid);
						e._argQuality._resolvedValue /= e._arg1._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 5:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=a<b?b:a+c
						e._argQuality._resolvedValue = (e._argQuality._resolvedValue < e._arg1._resolvedValue) ? e._arg1._resolvedValue : (e._argQuality._resolvedValue + e._arg2._resolvedValue);
						e._argQuality.StoreValue(q, vars);
						break;

					case 6:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=a>b?b:a-c
						e._argQuality._resolvedValue = (e._argQuality._resolvedValue > e._arg1._resolvedValue) ? e._arg1._resolvedValue : (e._argQuality._resolvedValue - e._arg2._resolvedValue);
						e._argQuality.StoreValue(q, vars);
						break;

					case 7:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a+=b*c
						e._argQuality._resolvedValue += e._arg1._resolvedValue * e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 8:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a+=b/c
						e._argQuality._resolvedValue += e._arg1._resolvedValue / e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 9:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a-=b*c
						e._argQuality._resolvedValue -= e._arg1._resolvedValue * e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 10:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a-=b/c
						e._argQuality._resolvedValue -= e._arg1._resolvedValue / e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 11:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=b+c
						e._argQuality._resolvedValue = e._arg1._resolvedValue + e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 12:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=b-c
						e._argQuality._resolvedValue = e._arg1._resolvedValue - e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 13:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=b*c
						e._argQuality._resolvedValue = e._arg1._resolvedValue * e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;

					case 14:
						assert(e._arg1._isValid);
						assert(e._arg2._isValid);

						// a=b/c
						e._argQuality._resolvedValue = e._arg1._resolvedValue / e._arg2._resolvedValue;
						e._argQuality.StoreValue(q, vars);
						break;
					}
				}

				break;
			}
		}

		break;
	}
}

DEFINE_PACK(CMutationFilter)
{
	UNFINISHED();
}

DEFINE_UNPACK(CMutationFilter)
{
	// pReader->Read<DWORD>(); // file ID

	_mutations.UnPack(pReader);
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

DEFINE_PACK(MutationChance)
{
	UNFINISHED();
}

DEFINE_UNPACK(MutationChance)
{
	_chances.UnPack(pReader);
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

DEFINE_PACK(EffectArgument)
{
	UNFINISHED();
}

DEFINE_UNPACK(EffectArgument)
{
	_type = pReader->Read<int>();
	memcpy(_raw, pReader->ReadArray(8), 8);
	return true;
}

