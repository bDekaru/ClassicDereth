
#pragma once

#include "HashData.h"
#include "LegacyPackObj.h"
#include "PackableJson.h"

class DATDisk;
class ObjCache;

class DBObj : public LongHashData, public LegacyPackObj
{
public:
	DBObj();
	virtual ~DBObj();

	long Link();
	long Unlink();
	long GetLinkCount() { return m_lLinks; }
	void SetCache(ObjCache *pCache) { m_pCache = pCache; }
	double GetTimeFreed() { return m_fTimeFreed; }
	virtual bool ShouldFreeAfterTime() { return true; }

protected:
	ObjCache* m_pCache = NULL;
	long m_lLinks = 1;
	double m_fTimeFreed = 0.0;
};

class DBObjWithJson : public DBObj, public PackableJson
{
public:
	virtual bool ShouldFreeAfterTime() { return !m_bLoadedFromJson; }

	DEFINE_LOCAL_PACK_JSON()
	{
		// not implemented
		assert(0);
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		// not implemented
		assert(0);
		return true;
	}

	bool m_bLoadedFromJson = false;
};

class ObjCache
{
public:
	ObjCache(DATDisk *pDisk, DBObj *(*pfnAllocator)(), void(*pfnDestroyer)(DBObj *), const char *cacheName);
	ObjCache(DATDisk *pDisk, DBObj *(*pfnAllocator)(), DBObjWithJson *(*pfnAllocatorWithJson)(), void(*pfnDestroyer)(DBObj *), const char *cacheName);
	virtual ~ObjCache();

	DBObj *Get(DWORD ID);
	void Release(DWORD ID);

	DWORD GetCachedCount();
	void UseTime();

	bool ReleaseFreeObjects(bool bForce);

	const char *GetName();

protected:

	bool GetDataFromJson(DWORD ID, json **ppReader);

	DATDisk *m_pDisk;

	DBObj *(*m_pfnAllocator)();
	void(*m_pfnDestroyer)(DBObj *);

	DBObjWithJson *(*m_pfnAllocatorWithJson)();
	void(*m_pfnDestroyerWithJson)(DBObjWithJson *);

	LongHash<DBObj> m_Objects;
	std::set<DWORD> m_FreeObjects;
	double m_fLastUpdate = 0.0;

	std::string m_CacheName;
};

namespace ObjCaches
{
	extern ObjCache *RegionDescs;
	extern ObjCache *LandBlocks;
	extern ObjCache *LandBlockInfos;
	extern ObjCache *EnvCells;
	extern ObjCache *Environments;
	extern ObjCache *Setups;
	extern ObjCache *GfxObjs;
	extern ObjCache *GfxObjDegradeInfos;
	extern ObjCache *Surfaces;
	extern ObjCache *Textures;
	extern ObjCache *Palettes;
	extern ObjCache *Animations;
	extern ObjCache *PhysicsScripts;
	extern ObjCache *PhysicsScriptTables;
	extern ObjCache *ParticleEmitterInfos;
	extern ObjCache *MotionTables;
	extern ObjCache *ImgColors;
	extern ObjCache *ClothingTables;
	extern ObjCache *PalSets;
	extern ObjCache *Scenes;
	extern ObjCache *SkillTables;
	extern ObjCache *ExperienceTables;
	extern ObjCache *SpellComponentTables;
	extern ObjCache *SpellTables;
	extern ObjCache *CharGenTables;
	extern ObjCache *CombatManeuverTables;
	extern ObjCache *Attribute2ndTables;
	extern ObjCache *QualityFilters;

	extern void InitCaches(bool initTables = true, const char *jsonDataPath = "");
	extern void DestroyCaches();
	extern void OutputCacheInfo();
	extern void UseTime();

	extern void AddObjCache(ObjCache **pStorage, DATDisk *pDisk, DBObj *(*pfnAllocator)(), void(*pfnDestroyer)(DBObj *), const char *cacheName);
	extern void AddObjCache(ObjCache **pStorage, DATDisk *pDisk, DBObj *(*pfnAllocator)(), DBObjWithJson *(*pfnAllocatorWithJson)(), void(*pfnDestroyer)(DBObj *), const char *cacheName);

	extern std::list<class ObjCache **> Caches;
	extern std::string JsonDataCachePath;
};

extern class Attribute2ndTable *CachedAttribute2ndTable;
extern class SkillTable *CachedSkillTable;
extern class CSpellTable *CachedSpellTable;
extern class SpellComponentTable *CachedSpellComponentTable;
extern class ExperienceTable *CachedExperienceTable;
extern class ACQualityFilter *CachedEnchantableFilter;
extern class ACCharGenData *CachedCharGenData;
extern class CRegionDesc *CachedRegionDesc;

#define DECLARE_DBOBJ(classname) \
    static DBObj* Allocator(); \
    static void Destroyer(DBObj *); \
    static classname *Get(DWORD ID); \
    static void Release(classname *);

#define DEFINE_DBOBJ(classname, cachename) \
    DBObj* classname::Allocator() { \
        return((DBObj *)new classname()); \
    } \
    void classname::Destroyer(DBObj* pObj) { \
        delete ((classname *)pObj); \
    } \
    classname *classname::Get(DWORD ID) { \
        return (classname *)ObjCaches::cachename->Get(ID); \
    } \
    void classname::Release(classname *pObj) { \
        if (pObj) \
            ObjCaches::cachename->Release(pObj->GetID()); \
    }
