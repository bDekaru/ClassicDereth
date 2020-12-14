
#pragma once

#include "Packable.h"
#include "ObjCache.h"

class CloTextureEffect : public PackObj, public PackableJson
{
public:
	CloTextureEffect();
	virtual ~CloTextureEffect();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	uint32_t oldTexID = 0;
	uint32_t newTexID = 0;
};

class CloObjectEffect : public PackObj, public PackableJson
{
public:
	CloObjectEffect();
	virtual ~CloObjectEffect();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	unsigned int partNum = 0;
	uint32_t objectID = 0;
	unsigned int numTextureEffects = 0;
	CloTextureEffect *textureEffects = NULL;
};

class ClothingBase : public PackObj, public PackableJson
{
public:
	ClothingBase();
	virtual ~ClothingBase();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	BOOL ApplyPartAndTextureChanges(class ObjDesc *od) const;

	unsigned int numObjectEffects = 0;
	CloObjectEffect *objectEffects = NULL;
};

class CloSubpalEffect : public PackObj, public PackableJson
{
public:
	CloSubpalEffect();
	virtual ~CloSubpalEffect();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	unsigned int numRanges = 0;
	unsigned int *rangeStart = NULL;
	unsigned int *rangeLength = NULL;
	uint32_t palSet = 0;
};

class CloPaletteTemplate : public PackObj, public PackableJson
{
public:
	CloPaletteTemplate();
	virtual ~CloPaletteTemplate();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	uint32_t iconID = 0;
	unsigned int numSubpalEffects = 0;
	CloSubpalEffect *subpalEffects = NULL;
};

class ShadePackage
{
public:
	ShadePackage(double val)
	{
		_val[0] = val;
		_val[1] = val;
		_val[2] = val;
		_val[3] = val;
	}

	double GetVal(const unsigned int index)
	{
		if (index >= 4)
			return _val[3];

		return _val[index];
	}

	double _val[4];
};

class ClothingTable : public DBObjWithJson, public PackObj
{
public:
	ClothingTable();
	virtual ~ClothingTable();

	static DBObj *Allocator();
	static DBObjWithJson *AllocatorWithJson();
	static void Destroyer(DBObj *pObject);
	static ClothingTable *Get(uint32_t ID);
	static void Release(ClothingTable *pClothingTable);

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();
	DECLARE_LEGACY_PACK_MIGRATOR();

	uint32_t GetAltSetup(uint32_t setup);

	BOOL BuildObjDesc(uint32_t setup, uint32_t pt, class ShadePackage *shades, class ObjDesc *od, uint32_t *iconId = 0);

	PackableHashTableWithJson<uint32_t, ClothingBase> _cloBaseHash;
	PackableHashTableWithJson<uint32_t, CloPaletteTemplate> _paletteTemplatesHash;
};
