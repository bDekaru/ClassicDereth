
#pragma once

#include "Packable.h"
#include "ObjCache.h"

#define SET_AND_QUERY_QUALITY_FILTER_METHOD(statName, storageName) \
	BOOL Set##statName(unsigned int stype, const int val) { \
		if (stype < GetNum##statName##Stats())	{ \
			if (!storageName) { \
				storageName = new int[GetNum##statName##Stats()]; \
				memset(storageName, 0, sizeof(int) * GetNum##statName##Stats()); \
			} \
			storageName[stype] = val; \
			return TRUE; \
		} \
		return FALSE; \
	} \
	int Query##statName(unsigned int stype) { \
		return (stype < GetNum##statName##Stats() && storageName) ? storageName[stype] : 0; \
	}

class QualityFilter : public PackObj, public DBObj
{
public:
	QualityFilter();
	virtual ~QualityFilter() override;

	void Clear();

	virtual int GetNumIntStats() = 0;
	virtual int GetNumInt64Stats() = 0;
	virtual int GetNumAttributeStats() = 0;
	virtual int GetNumAttribute2ndStats() = 0;
	virtual int GetNumFloatStats() = 0;
	virtual int GetNumBoolStats() = 0;
	virtual int GetNumDIDStats() = 0;
	virtual int GetNumIIDStats() = 0;
	virtual int GetNumStringStats() = 0;
	virtual int GetNumPositionStats() = 0;

	SET_AND_QUERY_QUALITY_FILTER_METHOD(Int, _int_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Int64, _int64_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Bool, _bool_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Float, _float_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(String, _string_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(DID, _did_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(IID, _iid_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Position, _position_stat_filter)

	DECLARE_PACKABLE();

	int *_int_stat_filter = NULL;
	int *_int64_stat_filter = NULL;
	int *_bool_stat_filter = NULL;
	int *_float_stat_filter = NULL;
	int *_string_stat_filter = NULL;
	int *_did_stat_filter = NULL;
	int *_iid_stat_filter = NULL;
	int *_position_stat_filter = NULL;
};

class ACQualityFilter : public QualityFilter
{
public:
	ACQualityFilter();
	virtual ~ACQualityFilter() override;

	void Clear();

	DECLARE_DBOBJ(ACQualityFilter)
	DECLARE_PACKABLE()
	DECLARE_LEGACY_PACK_MIGRATOR()

	virtual int GetNumIntStats() { return 512; }
	virtual int GetNumInt64Stats() { return 64; }
	virtual int GetNumBoolStats() { return 256; }
	virtual int GetNumFloatStats() { return 512; }
	virtual int GetNumStringStats() { return 64; }
	virtual int GetNumDIDStats() { return 64; }
	virtual int GetNumIIDStats() { return 64; }
	virtual int GetNumPositionStats() { return 32; }
	virtual int GetNumAttributeStats() { return 7; }
	virtual int GetNumAttribute2ndStats() { return 7; }
	virtual int GetNumSkillStats() { return 64; }

	SET_AND_QUERY_QUALITY_FILTER_METHOD(Attribute, _attribute_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Attribute2nd, _attribute2nd_stat_filter)
	SET_AND_QUERY_QUALITY_FILTER_METHOD(Skill, _skill_stat_filter)

	int *_attribute_stat_filter = NULL;
	int *_attribute2nd_stat_filter = NULL;
	int *_skill_stat_filter = NULL;
};

#undef SET_AND_QUERY_QUALITY_FILTER_METHOD

