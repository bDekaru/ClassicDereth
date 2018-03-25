
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

	DWORD oldTexID = 0;
	DWORD newTexID = 0;
};

class CloObjectEffect : public PackObj, public PackableJson
{
public:
	CloObjectEffect();
	virtual ~CloObjectEffect();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	unsigned int partNum = 0;
	DWORD objectID = 0;
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
	DWORD palSet = 0;
};

class CloPaletteTemplate : public PackObj, public PackableJson
{
public:
	CloPaletteTemplate();
	virtual ~CloPaletteTemplate();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	DWORD iconID = 0;
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
	static ClothingTable *Get(DWORD ID);
	static void Release(ClothingTable *pClothingTable);

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();
	DECLARE_LEGACY_PACK_MIGRATOR();

	BOOL BuildObjDesc(DWORD setup, DWORD pt, class ShadePackage *shades, class ObjDesc *od);

	PackableHashTableWithJson<DWORD, ClothingBase> _cloBaseHash;
	PackableHashTableWithJson<DWORD, CloPaletteTemplate> _paletteTemplatesHash;
};