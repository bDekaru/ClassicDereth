
#include "StdAfx.h"
#include "PhatSDK.h"
#include "QualityFilter.h"

#define UNPACK_STATS(statName, countName) \
	for (int i = 0; i < countName; i++) \
		Set##statName(pReader->Read<DWORD>(), 1);

QualityFilter::QualityFilter()
{
}

QualityFilter::~QualityFilter()
{
	Clear();
}

void QualityFilter::Clear()
{
	SafeDeleteArray(_int_stat_filter);
	SafeDeleteArray(_int64_stat_filter);
	SafeDeleteArray(_bool_stat_filter);
	SafeDeleteArray(_float_stat_filter);
	SafeDeleteArray(_string_stat_filter);
	SafeDeleteArray(_did_stat_filter);
	SafeDeleteArray(_iid_stat_filter);
	SafeDeleteArray(_position_stat_filter);
}

DEFINE_PACK(QualityFilter)
{
}

DEFINE_UNPACK(QualityFilter)
{
	int num_ints = pReader->Read<int>();
	int num_int64s = pReader->Read<int>();
	int num_bools = pReader->Read<int>();
	int num_floats = pReader->Read<int>();
	int num_dids = pReader->Read<int>();
	int num_iids = pReader->Read<int>();
	int num_strings = pReader->Read<int>();
	int num_positions = pReader->Read<int>();

	UNPACK_STATS(Int, num_ints);
	UNPACK_STATS(Int64, num_int64s);
	UNPACK_STATS(Bool, num_bools);
	UNPACK_STATS(Float, num_floats);
	UNPACK_STATS(DID, num_dids);
	UNPACK_STATS(IID, num_iids);
	UNPACK_STATS(String, num_strings);
	UNPACK_STATS(Position, num_positions);

	return true;
}

ACQualityFilter::ACQualityFilter()
{
}

ACQualityFilter::~ACQualityFilter()
{
	Clear();
}

DEFINE_DBOBJ(ACQualityFilter, QualityFilters)
DEFINE_LEGACY_PACK_MIGRATOR(ACQualityFilter)

void ACQualityFilter::Clear()
{
	QualityFilter::Clear();

	SafeDeleteArray(_attribute_stat_filter);
	SafeDeleteArray(_attribute2nd_stat_filter);
	SafeDeleteArray(_skill_stat_filter);
}

DEFINE_PACK(ACQualityFilter)
{
}

DEFINE_UNPACK(ACQualityFilter)
{
	pReader->Read<DWORD>(); // id

	QualityFilter::UnPack(pReader);

	int num_attributes = pReader->Read<int>();
	int num_attribute2nds = pReader->Read<int>();
	int num_skills = pReader->Read<int>();

	UNPACK_STATS(Attribute, num_attributes);
	UNPACK_STATS(Attribute2nd, num_attribute2nds);
	UNPACK_STATS(Skill, num_skills);

	return true;
}

#undef UNPACK_STATS

