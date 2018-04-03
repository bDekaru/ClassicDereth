
#include "StdAfx.h"
#include "DATDisk.h"
#include "ObjCache.h"

#include "RegionDesc.h"
#include "LandBlock.h"
#include "LandBlockInfo.h"
#include "EnvCell.h"
#include "Environment.h"
#include "Setup.h"
#include "GfxObj.h"
#include "Surface.h"
#include "Texture.h"
#include "Palette.h"
#include "Animation.h"
#include "Scripts.h"
#include "Particles.h"
#include "MotionTable.h"
#include "ClothingTable.h"
#include "PalSet.h"
#include "Scene.h"
#include "ACCharGenData.h"
#include "Qualities.h"
#include "SkillTable.h"
#include "QualityFilter.h"
#include "Config.h"

ObjCache *ObjCaches::RegionDescs = NULL;
ObjCache *ObjCaches::LandBlocks = NULL;
ObjCache *ObjCaches::LandBlockInfos = NULL;
ObjCache *ObjCaches::EnvCells = NULL;
ObjCache *ObjCaches::Environments = NULL;
ObjCache *ObjCaches::Setups = NULL;
ObjCache *ObjCaches::GfxObjs = NULL;
ObjCache *ObjCaches::GfxObjDegradeInfos = NULL;
ObjCache *ObjCaches::Surfaces = NULL;
ObjCache *ObjCaches::Textures = NULL;
ObjCache *ObjCaches::Palettes = NULL;
ObjCache *ObjCaches::Animations = NULL;
ObjCache *ObjCaches::PhysicsScripts = NULL;
ObjCache *ObjCaches::PhysicsScriptTables = NULL;
ObjCache *ObjCaches::ParticleEmitterInfos = NULL;
ObjCache *ObjCaches::MotionTables = NULL;
ObjCache *ObjCaches::ImgColors = NULL;
ObjCache *ObjCaches::ClothingTables = NULL;
ObjCache *ObjCaches::PalSets = NULL;
ObjCache *ObjCaches::Scenes = NULL;
ObjCache *ObjCaches::SkillTables = NULL;
ObjCache *ObjCaches::ExperienceTables = NULL;
ObjCache *ObjCaches::SpellComponentTables = NULL;
ObjCache *ObjCaches::SpellTables = NULL;
ObjCache *ObjCaches::CharGenTables = NULL;
ObjCache *ObjCaches::CombatManeuverTables = NULL;
ObjCache *ObjCaches::Attribute2ndTables = NULL;
ObjCache *ObjCaches::QualityFilters = NULL;

Attribute2ndTable *CachedAttribute2ndTable = NULL;
SkillTable *CachedSkillTable = NULL;
CSpellTable *CachedSpellTable = NULL;
SpellComponentTable *CachedSpellComponentTable = NULL;
ExperienceTable *CachedExperienceTable = NULL;
ACQualityFilter *CachedEnchantableFilter = NULL;
ACCharGenData *CachedCharGenData = NULL;
CRegionDesc *CachedRegionDesc = NULL;

std::list<ObjCache **> ObjCaches::Caches;
std::string ObjCaches::JsonDataCachePath;

void ObjCaches::OutputCacheInfo()
{
	for (auto cache : Caches)
	{
		if (!*cache)
			continue;

		LOG(Data, Normal, "Total %s: %u\n", (*cache)->GetName(), (*cache)->GetCachedCount());
	}
}

void ObjCaches::UseTime()
{
	for (auto cache : Caches)
	{
		if (*cache)
			(*cache)->UseTime();
	}
}

void ObjCaches::AddObjCache(ObjCache **pStorage, DATDisk *pDisk, DBObj *(*pfnAllocator)(), void(*pfnDestroyer)(DBObj *), const char *cacheName)
{
	*pStorage = new ObjCache(pDisk, pfnAllocator, pfnDestroyer, cacheName);
	Caches.push_back(pStorage);
}

void ObjCaches::AddObjCache(ObjCache **pStorage, DATDisk *pDisk, DBObj *(*pfnAllocator)(), DBObjWithJson *(*pfnAllocatorWithJson)(), void(*pfnDestroyer)(DBObj *), const char *cacheName)
{
	*pStorage = new ObjCache(pDisk, pfnAllocator, pfnAllocatorWithJson, pfnDestroyer, cacheName);
	Caches.push_back(pStorage);
}

void ObjCaches::InitCaches(bool initTables, const char *jsonDataPath)
{
	JsonDataCachePath = jsonDataPath;

	#define ADD_OBJ_CACHE(name, disk, allocator, destroyer) \
		AddObjCache(&name, disk, allocator, destroyer, #name);
	#define ADD_OBJ_CACHE_WITH_JSON(name, disk, allocator, destroyer) \
		AddObjCache(&name, disk, allocator, allocator##WithJson, destroyer, #name);

	ADD_OBJ_CACHE(RegionDescs, DATDisk::pPortal, &CRegionDesc::Allocator, &CRegionDesc::Destroyer);
	ADD_OBJ_CACHE(LandBlocks, DATDisk::pCell, &CLandBlock::Allocator, &CLandBlock::Destroyer);
	ADD_OBJ_CACHE(LandBlockInfos, DATDisk::pCell, &CLandBlockInfo::Allocator, &CLandBlockInfo::Destroyer);
	ADD_OBJ_CACHE(EnvCells, DATDisk::pCell, &CEnvCell::Allocator, &CEnvCell::Destroyer);
	ADD_OBJ_CACHE(Environments, DATDisk::pPortal, &CEnvironment::Allocator, &CEnvironment::Destroyer);
	ADD_OBJ_CACHE(Setups, DATDisk::pPortal, &CSetup::Allocator, &CSetup::Destroyer);
	ADD_OBJ_CACHE(GfxObjs, DATDisk::pPortal, &CGfxObj::Allocator, &CGfxObj::Destroyer);
	ADD_OBJ_CACHE(GfxObjDegradeInfos, DATDisk::pPortal, &GfxObjDegradeInfo::Allocator, &GfxObjDegradeInfo::Destroyer);
	ADD_OBJ_CACHE(Palettes, DATDisk::pPortal, &Palette::Allocator, &Palette::Destroyer);
	ADD_OBJ_CACHE(Textures, DATDisk::pPortal, &ImgTex::Allocator, &ImgTex::Destroyer);
	ADD_OBJ_CACHE(Surfaces, DATDisk::pPortal, &CSurface::Allocator, &CSurface::Destroyer);
	ADD_OBJ_CACHE(Animations, DATDisk::pPortal, &CAnimation::Allocator, &CAnimation::Destroyer);
	ADD_OBJ_CACHE(PhysicsScripts, DATDisk::pPortal, &PhysicsScript::Allocator, &PhysicsScript::Destroyer);
	ADD_OBJ_CACHE(PhysicsScriptTables, DATDisk::pPortal, &PhysicsScriptTable::Allocator, &PhysicsScriptTable::Destroyer);
	ADD_OBJ_CACHE(ParticleEmitterInfos, DATDisk::pPortal, &ParticleEmitterInfo::Allocator, &ParticleEmitterInfo::Destroyer);
	ADD_OBJ_CACHE(MotionTables, DATDisk::pPortal, &CMotionTable::Allocator, &CMotionTable::Destroyer);
	ADD_OBJ_CACHE(ImgColors, DATDisk::pPortal, &ImgColor::Allocator, &ImgColor::Destroyer);
	ADD_OBJ_CACHE_WITH_JSON(ClothingTables, DATDisk::pPortal, &ClothingTable::Allocator, &ClothingTable::Destroyer);
	ADD_OBJ_CACHE(PalSets, DATDisk::pPortal, &PalSet::Allocator, &PalSet::Destroyer);
	ADD_OBJ_CACHE(Scenes, DATDisk::pPortal, &Scene::Allocator, &Scene::Destroyer);
	ADD_OBJ_CACHE(SkillTables, DATDisk::pPortal, &SkillTable::Allocator, &SkillTable::Destroyer);
	ADD_OBJ_CACHE(ExperienceTables, DATDisk::pPortal, &ExperienceTable::Allocator, &ExperienceTable::Destroyer);
	ADD_OBJ_CACHE(SpellComponentTables, DATDisk::pPortal, &SpellComponentTable::Allocator, &SpellComponentTable::Destroyer);
	ADD_OBJ_CACHE(SpellTables, DATDisk::pPortal, &CSpellTable::Allocator, &CSpellTable::Destroyer);
	ADD_OBJ_CACHE(CharGenTables, DATDisk::pPortal, &ACCharGenData::Allocator, &ACCharGenData::Destroyer);
	ADD_OBJ_CACHE(CombatManeuverTables, DATDisk::pPortal, &CombatManeuverTable::Allocator, &CombatManeuverTable::Destroyer);
	ADD_OBJ_CACHE(Attribute2ndTables, DATDisk::pPortal, &Attribute2ndTable::Allocator, &Attribute2ndTable::Destroyer);
	ADD_OBJ_CACHE(QualityFilters, DATDisk::pPortal, &ACQualityFilter::Allocator, &ACQualityFilter::Destroyer);

	if (initTables)
	{
#if PHATSDK_IS_SERVER
		CachedCharGenData = ACCharGenData::Get(0x0E000002);
#endif

		CachedAttribute2ndTable = Attribute2ndTable::Get(0x0E000003);
		CachedSkillTable = SkillTable::Get(0x0E000004);
		CachedSpellTable = CSpellTable::Get(0x0E00000E);
		CachedSpellComponentTable = SpellComponentTable::Get(0x0E00000F);
		CachedExperienceTable = ExperienceTable::Get(0x0E000018);

		if(g_pConfig->OverrideMaxLevel() > 0 && g_pConfig->OverrideMaxLevel() < 275)
			CachedExperienceTable->_max_level = g_pConfig->OverrideMaxLevel();

		CachedEnchantableFilter = ACQualityFilter::Get(0x0E010001);

#if PHATSDK_IS_SERVER
		CachedRegionDesc = CRegionDesc::Get(0x13000000);
#endif
	}
}

void ObjCaches::DestroyCaches()
{
	if (GameTime::current_game_time)
	{
		delete GameTime::current_game_time;
		GameTime::current_game_time = NULL;
	}

	if (CachedCharGenData)
	{
		ACCharGenData::Release(CachedCharGenData);
		CachedCharGenData = NULL;
	}

	if (CachedAttribute2ndTable)
	{
		Attribute2ndTable::Release(CachedAttribute2ndTable);
		CachedAttribute2ndTable = NULL;
	}

	if (CachedSkillTable)
	{
		SkillTable::Release(CachedSkillTable);
		CachedSkillTable = NULL;
	}

	if (CachedSpellTable)
	{
		CSpellTable::Release(CachedSpellTable);
		CachedSpellTable = NULL;
	}

	if (CachedSpellComponentTable)
	{
		SpellComponentTable::Release(CachedSpellComponentTable);
		CachedSpellComponentTable = NULL;
	}

	if (CachedExperienceTable)
	{
		ExperienceTable::Release(CachedExperienceTable);
		CachedExperienceTable = NULL;
	}

	if (CachedEnchantableFilter)
	{
		ACQualityFilter::Release(CachedEnchantableFilter);
		CachedEnchantableFilter = NULL;
	}

	if (CachedRegionDesc)
	{
		CRegionDesc::Release(CachedRegionDesc);
		CachedRegionDesc = NULL;
	}

	bool bReleasedAny;

	do
	{
		bReleasedAny = false;
		for (ObjCache **ppcache : Caches)
		{
			ObjCache *pcache = *ppcache;
			if (pcache->ReleaseFreeObjects(true))
				bReleasedAny = true;
		}

	} while (bReleasedAny);

	for (ObjCache **ppcache : Caches)
	{
		ObjCache *pcache = *ppcache;
		if (pcache->GetCachedCount() > 0)
		{
			LOG(Data, Warning, "ObjCache \"%s\" still has %u objects cached! Missing release somewhere?\n", pcache->GetName(), pcache->GetCachedCount());
		}
	}

	for (ObjCache **ppcache : Caches)
	{
		ObjCache *pcache = *ppcache;
		delete pcache;
		*ppcache = NULL;
	}

	Caches.clear();
}

DBObj::DBObj()
{
}

DBObj::~DBObj()
{
}

long DBObj::Link()
{
	return ++m_lLinks;
}

long DBObj::Unlink()
{
	--m_lLinks;

	if (m_lLinks <= 0)
		m_fTimeFreed = Timer::cur_time;

	return m_lLinks;
}

ObjCache::ObjCache(DATDisk *pDisk, DBObj *(*pfnAllocator)(), void(*pfnDestroyer)(DBObj *), const char *cacheName) : m_Objects(4096)
{
	m_pDisk = pDisk;
	m_pfnAllocator = pfnAllocator;
	m_pfnAllocatorWithJson = NULL;
	m_pfnDestroyer = pfnDestroyer;
	m_CacheName = cacheName;
	m_fLastUpdate = Timer::cur_time;
}

ObjCache::ObjCache(DATDisk *pDisk, DBObj *(*pfnAllocator)(), DBObjWithJson *(*pfnAllocatorWithJson)(), void(*pfnDestroyer)(DBObj *), const char *cacheName) : m_Objects(4096)
{
	m_pDisk = pDisk;
	m_pfnAllocator = pfnAllocator;
	m_pfnAllocatorWithJson = pfnAllocatorWithJson;
	m_pfnDestroyer = pfnDestroyer;
	m_CacheName = cacheName;
	m_fLastUpdate = Timer::cur_time;
}

ObjCache::~ObjCache()
{
	m_Objects.destroy_contents();
}

const char *ObjCache::GetName()
{
	return m_CacheName.c_str();
}

bool ObjCache::GetDataFromJson(DWORD ID, json **ppReader)
{
#ifndef PUBLIC_BUILD
	if (!ObjCaches::JsonDataCachePath.empty())
	{
		char filePath[300];
		_snprintf(filePath, 300, "%s\\%08X.json", ObjCaches::JsonDataCachePath.c_str(), ID);
		filePath[299] = 0;

		std::ifstream fileStream(filePath);

		if (fileStream.is_open())
		{
			json *pReader = new json;
			json &reader = *pReader;
			fileStream >> reader;
			fileStream.close();

			*ppReader = pReader;
			return true;
		}
	}
#endif
	return false;
}

DBObj *ObjCache::Get(DWORD ID)
{
	DBObj *pObject = m_Objects.lookup(ID);

	if (!pObject)
	{
		if (!m_pDisk)
			return NULL;

		json *pReader;

		if (m_pfnAllocatorWithJson && GetDataFromJson(ID, &pReader))
		{
			pObject = m_pfnAllocatorWithJson();

			if (pObject)
			{
				((DBObjWithJson *)pObject)->m_bLoadedFromJson = true;
								
				pObject->SetID(ID);
				if (((DBObjWithJson *)pObject)->UnPackJson(*pReader))
				{
					// DEBUGOUT("Successfully unpacked object %08X(Memory@%08X) to cache.\r\n", ID, pObject);
					pObject->SetCache(this);
					m_Objects.add(pObject);
				}
				else
				{
					// DEBUGOUT("Failed unpacking object %08X(Memory@%08X) to cache.\r\n", ID, pObject);

					delete pObject;
					pObject = NULL;
				}
			}

			delete pReader;
		}
		else
		{
			DATEntry File;
			if (m_pDisk->GetData(ID, &File))
			{
				// DEBUGOUT("DATDisk::Get(0x%08X) Success\r\n", ID);
			}
			else
			{
				// DEBUGOUT("DATDisk::Get(0x%08X) Failure\r\n", ID);
				return NULL;
			}

			pObject = m_pfnAllocator();

			if (pObject)
			{
				BYTE *PackData = File.Data + sizeof(DWORD);

				pObject->SetID(ID);
				if (pObject->UnPack(&PackData, File.Length))
				{
					// DEBUGOUT("Successfully unpacked object %08X(Memory@%08X) to cache.\r\n", ID, pObject);
					pObject->SetCache(this);
					m_Objects.add(pObject);
				}
				else
				{
					// DEBUGOUT("Failed unpacking object %08X(Memory@%08X) to cache.\r\n", ID, pObject);

					delete pObject;
					pObject = NULL;
				}
			}

			delete[] File.Data;
		}

		return pObject;
	}
	else
		pObject->Link();

	return pObject;
}

DWORD ObjCache::GetCachedCount()
{
	HashBaseIter<unsigned long> Iter(&m_Objects);

	DWORD Count = 0;
	while (!Iter.EndReached()) {
		Count++;
		Iter.Next();
	}

	return Count;
}

void ObjCache::Release(DWORD ID)
{
	DBObj *pObject = m_Objects.lookup(ID);

	if (pObject)
	{
		if (pObject->Unlink() <= 0)
		{
			m_FreeObjects.insert(pObject->GetID());
		}
	}
}

bool ObjCache::ReleaseFreeObjects(bool bForce)
{
	bool bReleasedAny = false;
	for (std::set<DWORD>::iterator i = m_FreeObjects.begin(); i != m_FreeObjects.end();)
	{
		DWORD objectID = *i;
		DBObj *pFreedObject = m_Objects.lookup(objectID);
		if (pFreedObject->GetLinkCount() <= 0 && (bForce || (pFreedObject->GetTimeFreed() + 30.0) <= Timer::cur_time))
		{
			m_Objects.remove(objectID);
			m_pfnDestroyer(pFreedObject);
			i = m_FreeObjects.erase(i);
			bReleasedAny = true;
		}
		else
		{
			i++;
		}
	}

	return bReleasedAny;
}

void ObjCache::UseTime()
{
	if ((m_fLastUpdate + 10.0) <= Timer::cur_time)
	{
		ReleaseFreeObjects(false);
		m_fLastUpdate = Timer::cur_time;
	}
}

