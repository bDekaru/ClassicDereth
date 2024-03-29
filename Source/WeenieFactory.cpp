
#include <StdAfx.h>
#include "WeenieFactory.h"
#include "Door.h"
#include "Lifestone.h"
#include "BindStone.h"
#include "Portal.h"
#include "Monster.h"
#include "TownCrier.h"
#include "Vendor.h"
#include "World.h"
#include "ClothingCache.h"
#include "Player.h"
#include "HotSpot.h"
#include "PressurePlate.h"
#include "Switch.h"
#include "Book.h"
#include "Food.h"
#include "Caster.h"
#include "Clothing.h"
#include "Corpse.h"
#include "Healer.h"
#include "House.h"
#include "Scroll.h"
#include "ManaStone.h"
#include "MeleeWeapon.h"
#include "Missile.h"
#include "MissileLauncher.h"
#include "Config.h"
#include "Key.h"
#include "Lockpick.h"
#include "Gem.h"
#include "PKModifier.h"
#include "Ammunition.h"
#include "AttributeTransferDevice.h"
#include "SkillAlterationDevice.h"
#include "GameEventManager.h"
#include "TreasureFactory.h"
#include "AugmentationDevice.h"
#include "Game.h"
#include "GamePiece.h"
#include "weenie/PetDevice.h"
#include "WorldLandBlock.h"

CWeenieFactory::CWeenieFactory()
{
}

CWeenieFactory::~CWeenieFactory()
{
	Reset();
}

void CWeenieFactory::Reset()
{
	for (auto entry : m_WeenieDefaults)
		delete entry.second;

	m_WeenieDefaults.clear();
	m_WeenieDefaultsByName.clear();
	m_ScrollWeenies.clear();
}

void CWeenieFactory::Initialize()
{
	LOG_PRIVATE(Data, Normal, "Loading weenies...\n");
	SERVER_INFO << "Loading weenies...\n";

	CStopWatch watch;
	double elapsed = 0;

	LoadLocalStorage();
	elapsed = watch.GetElapsed();
	LOG_PRIVATE(Data, Normal, "LocalStorage loaded in %fs\n", elapsed);
	SERVER_INFO << "LocalStorage loaded in" << elapsed;

	watch.Reset();
	LoadLocalStorageIndexed();
	elapsed = watch.GetElapsed();
	LOG_PRIVATE(Data, Normal, "Cache.bin loaded in %fs\n", elapsed);
	SERVER_INFO << "Cache.bin loaded in" << elapsed;

	//LoadAvatarData();

	MapScrollWCIDs();
	LOG_PRIVATE(Data, Normal, "Loaded %d weenie defaults...\n", m_WeenieDefaults.size());
	SERVER_INFO << "Loaded" << m_WeenieDefaults.size() << "weenie defaults...";
}

uint32_t CWeenieFactory::GetScrollSpellForWCID(uint32_t wcid)
{
	if (CWeenieDefaults *defaults = GetWeenieDefaults(wcid))
	{
		if (defaults->m_Qualities.m_WeenieType == Scroll_WeenieType)
		{
			uint32_t spell_id = 0;

			if (defaults->m_Qualities.InqDataID(SPELL_DID, spell_id))
			{
				return spell_id;
			}
		}
	}

	return 0;
}

uint32_t CWeenieFactory::GetWCIDForScrollSpell(uint32_t spell_id)
{
	auto i = m_ScrollWeenies.find(spell_id);
	if (i != m_ScrollWeenies.end())
		return i->second;

	return 0;
}

void CWeenieFactory::MapScrollWCIDs()
{
	m_ScrollWeenies.clear();
	for (auto &entry : m_WeenieDefaults)
	{
		if (entry.second->m_Qualities.m_WeenieType != Scroll_WeenieType)
		{
			continue;
		}

		uint32_t spell_id;

		if (entry.second->m_Qualities.InqDataID(SPELL_DID, spell_id))
		{
			m_ScrollWeenies[spell_id] = entry.first;
		}
	}
}

std::list<uint32_t> CWeenieFactory::GetWCIDsWithMotionTable(uint32_t mtable)
{
	std::list<uint32_t> results;

	for (auto &entry : m_WeenieDefaults)
	{
		uint32_t mid = 0;

		if (entry.second->m_Qualities.InqDataID(MOTION_TABLE_DID, mid))
		{
			if (mtable == mid)
			{
				results.push_back(entry.first);
			}
		}
	}

	return results;
}

CWeenieDefaults *CWeenieFactory::GetWeenieDefaults(uint32_t wcid)
{
	auto i = m_WeenieDefaults.find(wcid);
	if (i != m_WeenieDefaults.end())
		return i->second;

	return NULL;
}

uint32_t CWeenieFactory::GetWCIDByName(const char *name, int index)
{
	CWeenieDefaults *defaults = GetWeenieDefaults(name, index);

	if (!defaults)
		return 0;

	return defaults->m_Qualities.GetID();
}

CWeenieDefaults *CWeenieFactory::GetWeenieDefaults(const char *name, int index)
{
	std::string search = name;

	// convert name for easier matching, lowercase and remove any spaces
	std::transform(search.begin(), search.end(), search.begin(), ::tolower);
	search.erase(remove_if(search.begin(), search.end(), isspace), search.end());

	std::pair<
		std::multimap<std::string, CWeenieDefaults *>::iterator,
		std::multimap<std::string, CWeenieDefaults *>::iterator> range = m_WeenieDefaultsByName.equal_range(search);

	for (std::multimap<std::string, CWeenieDefaults *>::iterator i = range.first; i != range.second; i++)
	{
		if (!index)
			return i->second;

		index--;
	}

	return NULL;
}

bool CWeenieFactory::UpdateWeenieBody(uint32_t wcid, Body* newBody)
{
	auto i = m_WeenieDefaults.find(wcid);
	if (i != m_WeenieDefaults.end())
	{
		i->second->m_Qualities._body = new Body(*newBody);
	}

	return false;
}

bool CWeenieFactory::ApplyWeenieDefaults(CWeenieObject *weenie, uint32_t wcid)
{
	auto defaults = GetWeenieDefaults(wcid);

	if (defaults)
	{
		ApplyWeenieDefaults(weenie, defaults);
		return true;
	}
	else
	{
		LOG_PRIVATE(Data, Warning, "Failed to find defaults for WCID %u! This is bad!\n", wcid);
		SERVER_WARN << "Failed to find defaults for WCID " << wcid << "! This is bad!";
		return false;
	}
}

void CWeenieFactory::ApplyWeenieDefaults(CWeenieObject *weenie, CWeenieDefaults *defaults)
{
	weenie->m_Qualities.CopyFrom(&defaults->m_Qualities);

	weenie->m_Qualities.RemoveInt(PARENT_LOCATION_INT);

	int weaponSkill;
	if (weenie->m_Qualities.InqInt(WEAPON_SKILL_INT, weaponSkill, TRUE, FALSE))
		weenie->m_Qualities.SetInt(WEAPON_SKILL_INT, weaponSkill);

	int wieldReq;
	wieldReq = weenie->m_Qualities.GetInt(WIELD_REQUIREMENTS_INT, 0);
	if (wieldReq == 1 || wieldReq == 2 || wieldReq == 8)
	{
		if (weenie->m_Qualities.InqInt(WIELD_SKILLTYPE_INT, weaponSkill, TRUE, FALSE))
			weenie->m_Qualities.SetInt(WIELD_SKILLTYPE_INT, weaponSkill);
	}

	wieldReq = weenie->m_Qualities.GetInt(WIELD_REQUIREMENTS_2_INT, 0);
	if (wieldReq == 1 || wieldReq == 2 || wieldReq == 8)
	{
		if (weenie->m_Qualities.InqInt(WIELD_SKILLTYPE_2_INT, weaponSkill, TRUE, FALSE))
			weenie->m_Qualities.SetInt(WIELD_SKILLTYPE_2_INT, weaponSkill);
	}

	wieldReq = weenie->m_Qualities.GetInt(WIELD_REQUIREMENTS_3_INT, 0);
	if (wieldReq == 1 || wieldReq == 2 || wieldReq == 8)
	{
		if (weenie->m_Qualities.InqInt(WIELD_SKILLTYPE_3_INT, weaponSkill, TRUE, FALSE))
			weenie->m_Qualities.SetInt(WIELD_SKILLTYPE_3_INT, weaponSkill);
	}

	wieldReq = weenie->m_Qualities.GetInt(WIELD_REQUIREMENTS_4_INT, 0);
	if (wieldReq == 1 || wieldReq == 2 || wieldReq == 8)
	{
		if (weenie->m_Qualities.InqInt(WIELD_SKILLTYPE_4_INT, weaponSkill, TRUE, FALSE))
			weenie->m_Qualities.SetInt(WIELD_SKILLTYPE_4_INT, weaponSkill);
	}

	//Skill uaSkill;
	//if (weenie->m_Qualities.InqSkill(UNARMED_COMBAT_SKILL, uaSkill))
	//{
	//	Skill finSkill;
	//	if (!weenie->m_Qualities.InqSkill(LIGHT_WEAPONS_SKILL, finSkill))
	//	{
	//		weenie->m_Qualities.SetSkill(LIGHT_WEAPONS_SKILL, uaSkill);
	//	}
	//}

	if (weenie->m_Qualities._skillStatsTable)
	{
		for (auto &entry : *weenie->m_Qualities._skillStatsTable)
		{
			STypeSkill newSkill = (STypeSkill)entry.first;

			if (newSkill != entry.first)
			{
				weenie->m_Qualities.SetSkill(newSkill, entry.second);
			}
		}
	}

	std::string eventString;
	if (weenie->m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	{
		eventString = g_pGameEventManager->NormalizeEventName(eventString.c_str());
		weenie->m_Qualities.SetString(GENERATOR_EVENT_STRING, eventString);
	}

	weenie->m_ObjDescOverride = defaults->m_WorldObjDesc;
	weenie->m_WornObjDesc = defaults->m_WornObjDesc;
}

void CWeenieFactory::RefreshLocalStorage()
{
	LoadLocalStorage(true);
	MapScrollWCIDs();
}

bool CWeenieFactory::RefreshLocalWeenie(uint32_t wcid)
{
	return LoadLocalWeenie(wcid);
}

void CWeenieFactory::LoadLocalStorage(bool refresh)
{
	std::mutex mapLock;
	std::mutex multiMapLock;

	fs::path root = g_pGlobals->GetGameData("Data", "json");

	PerformLoad(root / "weenies", [&](fs::path path)
		{
			std::ifstream fs(path);

			json jsonData;
			CWeenieDefaults *pDefaults = new CWeenieDefaults();
			bool parsed = false;
			try
			{
				fs >> jsonData;
				parsed = pDefaults->m_Qualities.UnPackJson(jsonData);
			}
			catch (std::exception &ex)
			{
				LOG_PRIVATE(Data, Error, "Failed to parse Weenie file %s\n", path.string().c_str());
				SERVER_ERROR << "Failed to parse Weenie file" << path.string();
			}

			fs.close();

			if (parsed)
			{
				pDefaults->m_WCID = pDefaults->m_Qualities.id;
				pDefaults->m_sourceFile = path.string();

				bool bShouldInsert = true;
				bool bDuplicate = false;

				auto existing = m_WeenieDefaults.find(pDefaults->m_WCID);
				if (existing != m_WeenieDefaults.end())
				{
					if (refresh)
					{
					}
					else
					{
						if (existing->second->m_bIsAutoGenerated)
						{
							if (pDefaults->m_bIsAutoGenerated)
							{
								bDuplicate = true;
								bShouldInsert = false;
							}
						}
						else
						{
							// existing was not auto-generated
							bShouldInsert = false;

							if (!pDefaults->m_bIsAutoGenerated)
							{
								bDuplicate = true;
							}
						}
					}
				}

				if (bShouldInsert)
				{
					CWeenieDefaults *toDelete = NULL;

					{
						std::scoped_lock lock(mapLock);

						if (existing != m_WeenieDefaults.end())
						{
							toDelete = existing->second;
						}

						m_WeenieDefaults[pDefaults->m_WCID] = pDefaults;
					}

					std::string name;
					if (pDefaults->m_Qualities.InqString(NAME_STRING, name))
					{
						// convert name for easier matching, lowercase and remove any spaces
						std::transform(name.begin(), name.end(), name.begin(), ::tolower);
						name.erase(remove_if(name.begin(), name.end(), isspace), name.end());

						// remove if this WCID is already in here
						for (std::multimap<std::string, CWeenieDefaults *>::iterator i = m_WeenieDefaultsByName.begin(); i != m_WeenieDefaultsByName.end(); i++)
						{
							if (i->second->m_Qualities.GetID() == pDefaults->m_Qualities.GetID())
							{
								std::scoped_lock lock(multiMapLock);
								m_WeenieDefaultsByName.erase(i);
								break;
							}
						}

						std::scoped_lock lock(multiMapLock);
						m_WeenieDefaultsByName.insert(std::pair<std::string, CWeenieDefaults *>(name, pDefaults));
					}

					if (toDelete)
					{
						delete toDelete;
					}
				}
				else
				{
					if (bDuplicate)
					{
						LOG_PRIVATE(Data, Warning, "Duplicate WCID %d, ignoring WQF file: \"%s\" due to \"%s\"\n", pDefaults->m_WCID, path.string().c_str(), existing->second->m_sourceFile.c_str());
						SERVER_WARN << "Duplicate WCID" << pDefaults->m_WCID << ", ignoring file \"" << path.string() << "\" due to \"" << existing->second->m_sourceFile << "\"";
					}

					delete pDefaults;
				}
			}
		});

}

bool CWeenieFactory::LoadLocalWeenie(uint32_t wcid)
{
	auto existing = m_WeenieDefaults.find(wcid);
	if (existing != m_WeenieDefaults.end())
	{
		fs::path path = m_WeenieDefaults[wcid]->m_sourceFile.c_str();

		if (path.extension().compare(".json") != 0)
		{
			path = g_pGlobals->GetGameData("Data", "json");
			path = path / "weenies" / (std::to_string(wcid) + " - " + m_WeenieDefaults[wcid]->m_Description + ".json");
		}

		std::mutex mapLock;
		std::mutex multiMapLock;

		std::ifstream fs(path);

		if (fs.is_open())
		{
			json jsonData;
			CWeenieDefaults *pDefaults = new CWeenieDefaults();
			bool parsed = false;
			try
			{
				fs >> jsonData;
				parsed = pDefaults->m_Qualities.UnPackJson(jsonData);
			}
			catch (std::exception &ex)
			{
				LOG_PRIVATE(Data, Error, "Failed to parse Weenie file %s\n", path.string().c_str());
				SERVER_ERROR << "Failed to parse Weenie file" << path.string();
			}

			fs.close();

			if (parsed)
			{
				pDefaults->m_WCID = pDefaults->m_Qualities.id;
				pDefaults->m_sourceFile = path.string();


				CWeenieDefaults *toDelete = NULL;
				{
					std::scoped_lock lock(mapLock);

					if (existing != m_WeenieDefaults.end())
					{
						toDelete = existing->second;
					}

					m_WeenieDefaults[pDefaults->m_WCID] = pDefaults;
				}

				std::string name;
				if (pDefaults->m_Qualities.InqString(NAME_STRING, name))
				{
					// convert name for easier matching, lowercase and remove any spaces
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
					name.erase(remove_if(name.begin(), name.end(), isspace), name.end());

					// remove if this WCID is already in here
					for (std::multimap<std::string, CWeenieDefaults *>::iterator i = m_WeenieDefaultsByName.begin(); i != m_WeenieDefaultsByName.end(); i++)
					{
						if (i->second->m_Qualities.GetID() == pDefaults->m_Qualities.GetID())
						{
							std::scoped_lock lock(multiMapLock);
							m_WeenieDefaultsByName.erase(i);
							break;
						}
					}

					std::scoped_lock lock(multiMapLock);
					m_WeenieDefaultsByName.insert(std::pair<std::string, CWeenieDefaults *>(name, pDefaults));

				}

				if (toDelete)
				{
					delete toDelete;
				}

				return true;
			}
			else
			{
				delete pDefaults;
			}
		}
	}

	return false;
}

bool CWeenieFactory::LoadLocalWeenieByWcid(uint32_t wcid)
{
	auto existing = m_WeenieDefaults.find(wcid);
	if (existing == m_WeenieDefaults.end())
	{
		fs::path path = g_pGlobals->GetGameData("Data", "json");
		path = path / "weenies" / (std::to_string(wcid) + ".json");

		std::mutex mapLock;
		std::mutex multiMapLock;

		std::ifstream fs(path);

		if (fs.is_open())
		{
			json jsonData;
			CWeenieDefaults *pDefaults = new CWeenieDefaults();
			bool parsed = false;
			try
			{
				fs >> jsonData;
				parsed = pDefaults->m_Qualities.UnPackJson(jsonData);
			}
			catch (std::exception &ex)
			{
				LOG_PRIVATE(Data, Error, "Failed to parse Weenie file %s\n", path.string().c_str());
			}

			fs.close();

			if (parsed)
			{
				pDefaults->m_WCID = pDefaults->m_Qualities.id;
				pDefaults->m_sourceFile = path.string();


				CWeenieDefaults *toDelete = NULL;
				{
					std::scoped_lock lock(mapLock);

					if (existing != m_WeenieDefaults.end())
					{
						toDelete = existing->second;
					}

					m_WeenieDefaults[pDefaults->m_WCID] = pDefaults;
				}

				std::string name;
				if (pDefaults->m_Qualities.InqString(NAME_STRING, name))
				{
					// convert name for easier matching, lowercase and remove any spaces
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
					name.erase(remove_if(name.begin(), name.end(), isspace), name.end());

					// remove if this WCID is already in here
					for (std::multimap<std::string, CWeenieDefaults *>::iterator i = m_WeenieDefaultsByName.begin(); i != m_WeenieDefaultsByName.end(); i++)
					{
						if (i->second->m_Qualities.GetID() == pDefaults->m_Qualities.GetID())
						{
							std::scoped_lock lock(multiMapLock);
							m_WeenieDefaultsByName.erase(i);
							break;
						}
					}

					std::scoped_lock lock(multiMapLock);
					m_WeenieDefaultsByName.insert(std::pair<std::string, CWeenieDefaults *>(name, pDefaults));

				}

				if (toDelete)
				{
					delete toDelete;
				}

				return true;
			}
			else
			{
				delete pDefaults;
			}
		}
	}

	return false;
}



void CWeenieFactory::LoadLocalStorageIndexed()
{
	BYTE *data = NULL;
	uint32_t length = 0;
	if (LoadDataFromPhatDataBin(9, &data, &length, 0xd8fd6b02, 0xa0427974))
	{
		BinaryReader reader(data, length);

		uint32_t count = reader.Read<uint32_t>();
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t the_wcid = reader.Read<uint32_t>();

			CWeenieDefaults *pDefaults = new CWeenieDefaults();
			if (pDefaults->UnPack(&reader))
			{
				/*
				if (pDefaults->m_Qualities.m_IntStats)
				{
					for (auto &entry : *pDefaults->m_Qualities.m_IntStats)
					{
						if (entry.first == IMBUED_EFFECT_INT)
						{
							if (entry.second & ArmorRending_ImbuedEffectType)
							{
								DebugBreak();
							}
						}
					}
				}
				*/

				bool bShouldInsert = true;
				bool bDuplicate = false;

				auto existing = m_WeenieDefaults.find(pDefaults->m_WCID);
				if (existing != m_WeenieDefaults.end())
				{
					if (existing->second->m_bIsAutoGenerated)
					{
						if (pDefaults->m_bIsAutoGenerated)
						{
							bDuplicate = true;
							bShouldInsert = false;
						}
					}
					else
					{
						// existing was not auto-generated
						bShouldInsert = false;

						if (!pDefaults->m_bIsAutoGenerated)
						{
							bDuplicate = true;
						}
					}
				}

				if (bShouldInsert)
				{
					//auto existing = m_WeenieDefaults.find(pDefaults->m_WCID);
					if (existing != m_WeenieDefaults.end())
						delete existing->second;

					//pDefaults->m_sourceFile = filePath;
					m_WeenieDefaults[pDefaults->m_WCID] = pDefaults;

					std::string name;
					if (pDefaults->m_Qualities.InqString(NAME_STRING, name))
					{
						// convert name for easier matching, lowercase and remove any spaces
						std::transform(name.begin(), name.end(), name.begin(), ::tolower);
						name.erase(remove_if(name.begin(), name.end(), isspace), name.end());

						// remove if this WCID is already in here
						/*
						for (std::multimap<std::string, CWeenieDefaults *>::iterator i = m_WeenieDefaultsByName.begin(); i != m_WeenieDefaultsByName.end(); i++)
						{
							if (i->second->m_Qualities.GetID() == pDefaults->m_Qualities.GetID())
							{
								m_WeenieDefaultsByName.erase(i);
								break;
							}
						}
						*/

						m_WeenieDefaultsByName.insert(std::pair<std::string, CWeenieDefaults *>(name, pDefaults));
					}
				}
				else
				{
					if (bDuplicate)
					{
						SERVER_ERROR << "Duplicate WCID" << pDefaults->m_WCID << ", ignoring due to" << m_WeenieDefaults[pDefaults->m_WCID]->m_sourceFile.c_str();
					}

					delete pDefaults;
				}
			}
			else
			{
				SERVER_ERROR << "Error parsing weenie defaults file";
				delete pDefaults;
			}
		}

		delete[] data;
	}
}

void CWeenieFactory::LoadAvatarData()
{
	m_NumAvatars = 0;

	BYTE *data = NULL;
	uint32_t length = 0;
	if (LoadDataFromPhatDataBin(7, &data, &length, 0x591c34e9, 0x2250b020))
	{
		BinaryReader reader(data, length);
		PackableHashTable<std::string, ObjDesc, std::string> avatarTable;
		avatarTable.UnPack(&reader);
		CWeenieDefaults *pRobeWithHood = GetWeenieDefaults(5851);

		uint32_t autoWcid = m_FirstAvatarWCID = 7000000;

		if (pRobeWithHood)
		{
			for (auto &avatar : avatarTable)
			{
				autoWcid++;
				m_NumAvatars++;

				CWeenieDefaults *pDefaults = new CWeenieDefaults();
				pDefaults->m_Qualities.CopyFrom(&pRobeWithHood->m_Qualities);
				pDefaults->m_Qualities.SetString(NAME_STRING, csprintf("%s Jumpsuit", avatar.first.c_str()));
				pDefaults->m_Qualities.SetInt(CLOTHING_PRIORITY_INT, pDefaults->m_Qualities.GetInt(CLOTHING_PRIORITY_INT, 0) | HAND_WEAR_CLOTHING_PRIORITY);
				pDefaults->m_Qualities.SetInt(LOCATIONS_INT, pDefaults->m_Qualities.GetInt(LOCATIONS_INT, 0) | HAND_WEAR_LOC);
				pDefaults->m_Qualities.SetDataID(ICON_DID, 0x6001036);
				pDefaults->m_Qualities.SetBool(IGNORE_CLO_ICONS_BOOL, TRUE);
				pDefaults->m_WornObjDesc = avatar.second;
				pDefaults->m_WCID = autoWcid;
				pDefaults->m_Qualities.id = autoWcid;

				CWeenieDefaults *toDelete = NULL;

				auto existing = m_WeenieDefaults.find(pDefaults->m_WCID);
				if (existing != m_WeenieDefaults.end())
				{
					toDelete = existing->second;
				}

				m_WeenieDefaults[pDefaults->m_WCID] = pDefaults;

				std::string name;
				if (pDefaults->m_Qualities.InqString(NAME_STRING, name))
				{
					// convert name for easier matching, lowercase and remove any spaces
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
					name.erase(remove_if(name.begin(), name.end(), isspace), name.end());

					// remove if this WCID is already in here
					/*
					for (std::multimap<std::string, CWeenieDefaults *>::iterator i = m_WeenieDefaultsByName.begin(); i != m_WeenieDefaultsByName.end(); i++)
					{
						if (i->second->m_Qualities.GetID() == pDefaults->m_Qualities.GetID())
						{
							m_WeenieDefaultsByName.erase(i);
							break;
						}
					}
					*/

					m_WeenieDefaultsByName.insert(std::pair<std::string, CWeenieDefaults *>(name, pDefaults));
				}

				if (toDelete)
				{
					delete toDelete;
				}
			}
		}

		delete[] data;
	}
}

CWeenieObject *CWeenieFactory::CreateWeenieByClassID(uint32_t wcid, const Position *pos, bool bSpawn)
{
	if (!wcid)
		return NULL;

	CWeenieDefaults *defaults = GetWeenieDefaults(wcid);
	if (!defaults)
		return NULL;

	if (!g_pConfig->CreateTemplates() && (!strcmp(defaults->m_Qualities.GetString(NAME_STRING, "").c_str(), "Name Me Please") || !strcmp(defaults->m_Qualities.GetString(NAME_STRING, "").c_str(), "CreatureName")))
		return NULL;

	return CreateWeenie(defaults, pos, bSpawn);
}

CWeenieObject *CWeenieFactory::CreateWeenieByName(const char *name, const Position *pos, bool bSpawn)
{
	CWeenieDefaults *defaults = GetWeenieDefaults(name);
	if (!defaults)
		return NULL;

	if (!g_pConfig->CreateTemplates() && (!strcmp(defaults->m_Qualities.GetString(NAME_STRING, "").c_str(), "Name Me Please") || !strcmp(defaults->m_Qualities.GetString(NAME_STRING, "").c_str(), "CreatureName")))
		return NULL;

	return CreateWeenie(defaults, pos, bSpawn);
}

CWeenieObject *CWeenieFactory::CreateBaseWeenieByType(int weenieType, unsigned int wcid, const char *weenieName)
{
	//Disabled this as it's only used during character creation, and at that point we don't want to treat 
	//the weenie as a CPlayerWeenie as that entails client communication and both the client and the weenie
	//are not ready for that at that moment.
	//if (wcid == 1)
	//	return new CPlayerWeenie(NULL, 0, 0);

	CWeenieObject *weenie;
	switch (weenieType)
	{
	case Generic_WeenieType:
	case Admin_WeenieType:
	{
		weenie = new CWeenieObject();
		break;
	}

	case Cow_WeenieType:
	case Creature_WeenieType:
	{
		if (!strcmp(weenieName, "Town Crier"))
			weenie = new CTownCrier();
		else
			weenie = new CMonsterWeenie();
		break;
	}
	case AttributeTransferDevice_WeenieType:
	{
		weenie = new CAttributeTransferDeviceWeenie();
		break;
	}
	case SkillAlterationDevice_WeenieType:
	{
		weenie = new CSkillAlterationDeviceWeenie();
		break;
	}
	case Container_WeenieType:
	{
		weenie = new CContainerWeenie();
		break;
	}
	case Portal_WeenieType:
	{
		weenie = new CPortal();
		break;
	}
	case LifeStone_WeenieType:
	{
		weenie = new CBaseLifestone();
		break;
	}
	case AllegianceBindstone_WeenieType:
	{
		weenie = new CBindStone();
		break;
	}
	case Door_WeenieType:
	{
		weenie = new CBaseDoor();
		break;
	}
	case HotSpot_WeenieType:
	{
		weenie = new CHotSpotWeenie();
		break;
	}
	case Switch_WeenieType:
	{
		weenie = new CSwitchWeenie();
		break;
	}
	case PressurePlate_WeenieType:
	{
		weenie = new CPressurePlateWeenie();
		break;
	}
	case Vendor_WeenieType:
	{
		/*
		if (wcid == 719)
			weenie = new CAvatarVendor(); // holtburg avatar vendor
		else
		*/
		weenie = new CVendor();
		break;
	}

	case Clothing_WeenieType:
	{
		weenie = new CClothingWeenie();
		break;
	}
	case Caster_WeenieType:
	{
		weenie = new CCasterWeenie();
		break;
	}
	case Healer_WeenieType:
	{
		weenie = new CHealerWeenie();
		break;
	}
	case House_WeenieType:
	{
		weenie = new CHouseWeenie();
		break;
	}
	case SlumLord_WeenieType:
	{
		weenie = new CSlumLordWeenie();
		break;
	}
	case Scroll_WeenieType:
	{
		weenie = new CScrollWeenie();
		break;
	}

	case Ammunition_WeenieType:
	{
		weenie = new CAmmunitionWeenie();
		break;
	}

	case Missile_WeenieType:
	{
		weenie = new CMissileWeenie();
		break;
	}

	case ManaStone_WeenieType:
	{
		weenie = new CManaStoneWeenie();
		break;
	}
	case Coin_WeenieType:
	{
		weenie = new CWeenieObject();
		break;
	}
	case Book_WeenieType:
	{
		weenie = new CBookWeenie();
		break;
	}
	case Food_WeenieType:
	{
		weenie = new CFoodWeenie();
		break;
	}
	case PKModifier_WeenieType:
	{
		weenie = new CPKModifierWeenie();
		break;
	}
	case Gem_WeenieType:
	{
		weenie = new CGemWeenie();
		break;
	}
	case Key_WeenieType:
	{
		weenie = new CKeyWeenie();
		break;
	}
	case Lockpick_WeenieType:
	{
		weenie = new CLockpickWeenie();
		break;
	}
	case Corpse_WeenieType:
	{
		weenie = new CCorpseWeenie();
		break;
	}
	case MeleeWeapon_WeenieType:
	{
		weenie = new CMeleeWeaponWeenie();
		break;
	}
	case MissileLauncher_WeenieType:
	{
		weenie = new CMissileLauncherWeenie();
		break;
	}
	case Hook_WeenieType:
	{
		weenie = new CHookWeenie();
		break;
	}
	case Deed_WeenieType:
	{
		weenie = new CDeedWeenie();
		break;
	}
	case BootSpot_WeenieType:
	{
		weenie = new CBootSpotWeenie();
		break;
	}
	case HousePortal_WeenieType:
	{
		weenie = new CHousePortalWeenie();
		break;
	}
	case Chest_WeenieType:
	{
		weenie = new CChestWeenie();
		break;
	}
	case Storage_WeenieType:
	{
		weenie = new CStorageWeenie();
		break;
	}
	case AugmentationDevice_WeenieType:
	{
		weenie = new CAugmentationDeviceWeenie();
		break;
	}
	case Game_WeenieType:
		weenie = new GameWeenie();
		break;
	case GamePiece_WeenieType:
		weenie = new GamePieceWeenie();
		break;

	case Pet_WeenieType:
		weenie = new PetPassive();
		break;

	case PetDevice_WeenieType:
		weenie = new PetDevice();
		break;

	case CombatPet_WeenieType:
		weenie = new PetCombat();
		break;

	default:
		weenie = new CWeenieObject();
		break;
	}

	return weenie;
}

CWeenieObject *CWeenieFactory::CreateWeenie(CWeenieDefaults *defaults, const Position *pos, bool bSpawn)
{
	CWeenieObject *weenie = CreateBaseWeenieByType(
		(defaults->m_Qualities.GetInt(ITEM_TYPE_INT, 0) & TYPE_VESTEMENTS) ? Clothing_WeenieType : defaults->m_Qualities.m_WeenieType,
		defaults->m_Qualities.GetID(), defaults->m_Qualities.GetString(NAME_STRING, "").c_str());
	ApplyWeenieDefaults(weenie, defaults);
	weenie->ApplyQualityOverrides();

	if (!weenie->IsAvatarJumpsuit())
		TryToResolveAppearanceData(weenie);

	if (weenie->IsCreature())
		weenie->SetMaxVitals();

	//TODO: Lots of unlocked (i.e. normal) doors have 50 LP resistance.
	//Need to fix the weenies, but this works for now.
	if (weenie->AsDoor()) {
		BOOL defaultLocked = weenie->InqBoolQuality(DEFAULT_LOCKED_BOOL, FALSE);
		if (!defaultLocked && weenie->InqIntQuality(RESIST_LOCKPICK_INT, 0) == 50) {
			weenie->m_Qualities.SetInt(RESIST_LOCKPICK_INT, 0);
		}
	}

	if (pos)
		weenie->SetInitialPosition(*pos);

	if (bSpawn)
	{
		if (!g_pWorld->CreateEntity(weenie))
			return NULL;
	}

	return weenie;
}

CWeenieObject *CWeenieFactory::CloneWeenie(CWeenieObject *weenie)
{
	CWeenieObject *clone = CreateBaseWeenieByType(
		(weenie->m_Qualities.GetInt(ITEM_TYPE_INT, 0) & TYPE_VESTEMENTS) ? Clothing_WeenieType : weenie->m_Qualities.m_WeenieType,
		weenie->m_Qualities.GetID(), weenie->InqStringQuality(NAME_STRING, "").c_str());
	clone->m_Qualities.CopyFrom(&weenie->m_Qualities);
	clone->m_bObjDescOverride = weenie->m_bObjDescOverride;
	clone->m_ObjDescOverride = weenie->m_ObjDescOverride;
	clone->m_WornObjDesc = weenie->m_WornObjDesc;
	return clone;
}

void CWeenieFactory::AddWeenieToDestination(CWeenieObject *weenie, CWeenieObject *parent, uint32_t destinationType, bool isRegenLocationType, const GeneratorProfile *profile)
{
	if (!isRegenLocationType)
	{
		switch ((DestinationType)destinationType)
		{
		case Contain_DestinationType:
			destinationType = Contain_RegenLocationType;
			break;
		case ContainTreasure_DestinationType:
			destinationType = ContainTreasure_RegenLocationType;
			break;
		case Wield_DestinationType:
			destinationType = Wield_RegenLocationType;
			break;
		case WieldTreasure_DestinationType:
			destinationType = WieldTreasure_RegenLocationType;
			break;
		case Shop_DestinationType:
			destinationType = Shop_RegenLocationType;
			break;
		case ShopTreasure_DestinationType:
			destinationType = ShopTreasure_RegenLocationType;
			break;
		case Checkpoint_DestinationType:
			destinationType = Checkpoint_RegenLocationType;
			break;
		case Treasure_DestinationType:
			destinationType = Treasure_RegenLocationType;
			break;
		case HouseBuy_DestinationType:
		case HouseRent_DestinationType:
			return;
		}
	}

	Position pos = parent->GetPosition();

	if (profile && profile->stackSize > 1 && !weenie->AsMonster()) { //Use stack size from generator profile so long as the weenie is NOT a creature.
		if (profile->stackSize <= weenie->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0))
			weenie->SetStackSize(profile->stackSize);
		else
			weenie->SetStackSize(weenie->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0));
	}

	switch ((RegenLocationType)destinationType)
	{
	case Contain_RegenLocationType:
	case ContainTreasure_RegenLocationType:
	case Treasure_RegenLocationType:
		if (CContainerWeenie *container = parent->AsContainer())
			container->SpawnInContainer(weenie);
		break;
	case Wield_RegenLocationType:
	case WieldTreasure_RegenLocationType:
		if (CMonsterWeenie *creature = parent->AsMonster())
			creature->SpawnWielded(weenie);
		else if (CContainerWeenie *container = parent->AsContainer())
			container->SpawnInContainer(weenie);
		break;
	case Specific_RegenLocationType:
	case SpecificTreasure_RegenLocationType:
		if (!profile->pos_val.objcell_id)
			pos.frame.m_origin = pos.localtoglobal(profile->pos_val.frame.m_origin);
		else
			pos = profile->pos_val;

		weenie->SetInitialPosition(pos);
		if (!g_pWorld->CreateEntity(weenie))
		{
			SERVER_ERROR << "Failed creating generated spawn" << GetWCIDName(profile->type);
			return;
		}
		break;
	case Scatter_RegenLocationType:
	case ScatterTreasure_RegenLocationType:
	{
		float genRadius = (float)parent->InqFloatQuality(GENERATOR_RADIUS_FLOAT, 0.0f);
		float randomAngle = Random::GenFloat(0, 2.0f*(float)M_PI);
		float randomRadius = Random::GenFloat(0, genRadius);

		pos.frame.m_origin += Vector(cosf(randomAngle) * randomRadius, sinf(randomAngle) * randomRadius, 0.05f);

		weenie->SetInitialPosition(pos);
		if (!g_pWorld->CreateEntity(weenie))
		{
			SERVER_ERROR << "Failed creating generated spawn" << GetWCIDName(profile->type);
			return;
		}

		break;
	}
	case OnTop_RegenLocationType:
	case OnTopTreasure_RegenLocationType:
		//untested
		if (!profile->pos_val.objcell_id)
			pos.frame.m_origin = pos.localtoglobal(profile->pos_val.frame.m_origin);
		else
			pos = profile->pos_val;

		if ((pos.objcell_id & 0xFFFF) < 0x100) //outdoors
		{
			// TODO: Get rid of calcsurfacez b/c it doesn't work properly
			pos.frame.m_origin.z = CalcSurfaceZ(pos.objcell_id, pos.frame.m_origin.x, pos.frame.m_origin.y, true);
			pos.frame.m_origin.z += 0.5f; // add a little fudge factor to ensure mobs don't spawn in ground.
		}

		weenie->SetInitialPosition(pos);
		if (!g_pWorld->CreateEntity(weenie))
		{
			SERVER_ERROR << "Failed creating generated spawn" << GetWCIDName(profile->type);
			return;
		}
		break;
	case Shop_RegenLocationType:
	case ShopTreasure_RegenLocationType:
		break;
	case Checkpoint_RegenLocationType:
		break;
	}

	if (isRegenLocationType && profile != NULL)
	{
		weenie->m_Qualities.SetInstanceID(GENERATOR_IID, parent->GetID());

		if (!weenie->IsContained() && !weenie->IsStuck() && weenie->m_Position.objcell_id && !weenie->cell)
		{
			weenie->MarkForDestroy();
			return;
		}

		parent->GeneratorAddToRegistry(weenie, *profile);

		//SERVER_INFO << "Created Object" << weenie->GetName() << "(" << weenie->m_Qualities.id << ") at " << weenie->m_Position;

		//GeneratorRegistryNode node;
		//node.m_wcidOrTtype = profile->type;
		//node.slot = profile->slot;
		//node.ts = Timer::cur_time;
		//node.amount = weenie->InqIntQuality(STACK_SIZE_INT, 1);
		//node.checkpointed = 0;
		//node.m_bTreasureType = profile->IsTreasureType();
		//node.shop = 0;
		//node.m_objectId = weenie->GetID();

		//if (!parent->m_Qualities._generator_registry)
		//	parent->m_Qualities._generator_registry = new GeneratorRegistry();
		//parent->m_Qualities._generator_registry->_registry.add(weenie->GetID(), &node);
		//parent->m_GeneratorSpawns[weenie->GetID()] = weenie->m_Qualities.id;

		//weenie->ChanceExecuteEmoteSet(parent->GetID(), Generation_EmoteCategory);
	}
}

int CWeenieFactory::AddFromCreateList(CWeenieObject *parent, PackableListWithJson<CreationProfile> *createList, DestinationType validDestinationTypes)
{
	int amountCreated = 0;

	if (!createList)
		return amountCreated;

	bool seekingNext = true;
	bool forceNext = true;
	double diceRoll;
	double accumulatedChance;

	for (auto i = createList->begin(); i != createList->end(); i++)
	{
		float probability = 0.0;
		bool isTreasureDestination = false;
		if (i->destination & Treasure_DestinationType)
		{
			isTreasureDestination = true;
			probability = i->shade * g_pConfig->DropRateMultiplier();
		}
		// By virtue of the way the loot system works increasing the drop rate may cause some items to actually drop less often.
		// Items are grouped in "sets" and only one item per set will actually drop, if the drop chance of the first item is increased
		// the chance of the subsequent items decreases. Although this isn't used very often in the data. Most of the time there's just one item
		// per set. For a good use case see the Drudge Robber Baron.
		// If we wanted we could do this properly by checking the wcid == 0 entries and reallocating the chance of nothing dropping to the items.
		// This would increase the drop rate as much as it is possible without lowering other item drop rates. This would be best done as a modification
		// to the data and not on the fly.

		if (seekingNext)
		{
			if (forceNext || accumulatedChance >= 1.0)
			{
				// We've reached the end of this set.
				seekingNext = false;
				forceNext = false;
				diceRoll = Random::RollDice(0.0, 1.0);
				accumulatedChance = 0.0;
			}
			else
			{
				// Just keep going.
				accumulatedChance += probability;
				continue;
			}
		}

		if (!isTreasureDestination || diceRoll <= accumulatedChance + probability || probability <= 0.0)
		{
			// We're the one to spawn!
			if (i->wcid != 0) // Unless we're a big pile of nothing.
			{
				if (validDestinationTypes == DestinationType::Undef_DestinationType || (i->destination & validDestinationTypes)) // Or we're not of the correct destinationType.
				{
					if (CWeenieObject *newItem = g_pWeenieFactory->CreateWeenieByClassID(i->wcid, NULL, false))
					{
						if (i->palette)
							newItem->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, i->palette);

						if (!isTreasureDestination && i->shade > 0.0) // treasure destinations use the shade variable as a probability to spawn.
							newItem->m_Qualities.SetFloat(SHADE_FLOAT, i->shade);

						int maxStackSize = newItem->InqIntQuality(MAX_STACK_SIZE_INT, 1);
						if (i->amount > 1)
						{
							if (i->amount <= maxStackSize)
								newItem->SetStackSize(i->amount);
							else
								newItem->SetStackSize(maxStackSize);
						}

						AddWeenieToDestination(newItem, parent, i->destination, false);
						amountCreated++;
					}
				}
			}
			accumulatedChance += probability;
			seekingNext = true;
			if (!isTreasureDestination)
				forceNext = true;
		}
		else
			accumulatedChance += probability;

		if (isTreasureDestination && i->wcid == 0)
		{
			//we've reached the end of this set even if the probabilities didn't add up to 1.0.
			forceNext = true;
			seekingNext = true;
		}
	}

	return amountCreated;
}

int CWeenieFactory::AddFromGeneratorTable(CWeenieObject *parent, bool isInit)
{
	int numSpawned = parent->m_GeneratorSpawns.size();

	if (!parent->m_Qualities._generator_table)
		return 0;

	int maxSpawns = isInit ? parent->InqIntQuality(INIT_GENERATED_OBJECTS_INT, 0) : parent->InqIntQuality(MAX_GENERATED_OBJECTS_INT, 0);

	//if (!isInit)
	//{
	//	if (parent->m_Qualities._generator_registry)
	//		numSpawned += (int)parent->m_Qualities._generator_registry->_registry.size();
	//}

	if (numSpawned >= maxSpawns)
		return 0;

	std::list<GeneratorProfile> genList = isInit ? parent->m_Qualities._generator_table->GetInitialGenerationList() : parent->m_Qualities._generator_table->GetGenerationList();

	// always spawn everything with zero or subzero values
	for (GeneratorProfile &entry : genList)
	{
		bool fullLoopWithNoValidEntries = true;
		for (GeneratorProfile &entry : genList)
		{
			if (!parent->IsGeneratorSlotReady(entry.slot))
				continue;

			if (entry.probability > 0 || entry.IsPlaceHolder())
				continue;

			fullLoopWithNoValidEntries = false;

			int maxCreate = isInit ? entry.initCreate : entry.maxNum;
			if (maxCreate < 0) //this is -1 for unlimited amount;
				maxCreate = maxSpawns;
			maxCreate = min(maxCreate, maxSpawns - numSpawned); //cap maxCreate to however many we can still spawn.

			int amountCreated;
			for (amountCreated = 0; amountCreated < maxCreate; amountCreated++)
			{
				int newAmount = g_pTreasureFactory->GenerateFromTypeOrWcid(parent, entry.whereCreate, true, entry.type, entry.ptid, entry.shade, &entry);

				//disabled the code below to make each treasure entry count as just 1 spawned item.
				//if (newAmount > 1)
				//	amountCreated += newAmount - 1;
			}

			numSpawned += amountCreated;
			if (numSpawned >= maxSpawns)
				break;
		}

		if (fullLoopWithNoValidEntries)
			break;
	}

	while (numSpawned < maxSpawns)
	{
		float diceRoll = Random::RollDice(0.0f, 1.0f);

		bool fullLoopWithNoValidEntries = true;
		for (GeneratorProfile &entry : genList)
		{
			if (!parent->IsGeneratorSlotReady(entry.slot))
				continue;

			if (entry.probability <= 0 || entry.IsPlaceHolder())
				continue;

			if (diceRoll > entry.probability)
				continue;

			fullLoopWithNoValidEntries = false;

			int maxCreate = isInit ? entry.initCreate : entry.maxNum;
			if (maxCreate < 0) //this is -1 for unlimited amount;
				maxCreate = maxSpawns;
			maxCreate = min(maxCreate, maxSpawns - numSpawned); //cap maxCreate to however many we can still spawn.

			int amountCreated;
			for (amountCreated = 0; amountCreated < maxCreate; amountCreated++)
			{
				int newAmount = g_pTreasureFactory->GenerateFromTypeOrWcid(parent, entry.whereCreate, true, entry.type, entry.ptid, entry.shade, &entry);

				//disabled the code below to make each treasure entry count as just 1 spawned item.
				//if(newAmount > 1)
				//	amountCreated += newAmount - 1;
			}

			numSpawned += amountCreated;

			//Found an item. Now we need a fresh roll, so stop iterating the list.
			break;
		}

		if (fullLoopWithNoValidEntries)
			break;
	}

	//if (isInit)
	//{
	//	int numSpawned = parent->m_Qualities._generator_registry ? (int)parent->m_Qualities._generator_registry->_registry.size() : 0;
	//	int maxSpawn = parent->InqIntQuality(MAX_GENERATED_OBJECTS_INT, 0, TRUE);
	//	bool queueEmpty = (!parent->m_Qualities._generator_queue || parent->m_Qualities._generator_queue->_queue.empty());
	//	if (numSpawned >= maxSpawn && (maxSpawn > 0 || queueEmpty))
	//		parent->_nextRegen = -1.0;
	//	else
	//		parent->_nextRegen = Timer::cur_time + (parent->InqFloatQuality(REGENERATION_INTERVAL_FLOAT, -1.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
	//}

	return numSpawned;
}

int CWeenieFactory::GenerateFromTypeOrWcid(CWeenieObject *parent, const GeneratorProfile *profile)
{
	return g_pTreasureFactory->GenerateFromTypeOrWcid(parent, profile->whereCreate, true, profile->type, profile->ptid, profile->shade, profile);
}

int CWeenieFactory::GenerateFromTypeOrWcid(CWeenieObject *parent, RegenLocationType destinationType, uint32_t treasureTypeOrWcid, unsigned int ptid, float shade)
{
	return g_pTreasureFactory->GenerateFromTypeOrWcid(parent, destinationType, true, treasureTypeOrWcid, ptid, shade);
}

int CWeenieFactory::GenerateFromTypeOrWcid(CWeenieObject *parent, DestinationType destinationType, uint32_t treasureTypeOrWcid, unsigned int ptid, float shade)
{
	return g_pTreasureFactory->GenerateFromTypeOrWcid(parent, destinationType, false, treasureTypeOrWcid, ptid, shade);
}

int CWeenieFactory::GenerateRareItem(CWeenieObject *parent, CWeenieObject *killer)
{
	return g_pTreasureFactory->GenerateRareItem(parent, killer);
}

bool CWeenieFactory::TryToResolveAppearanceData(CWeenieObject *weenie)
{
	// this function is temporary

	if (weenie->GetItemType() & (ITEM_TYPE::TYPE_ARMOR | ITEM_TYPE::TYPE_CLOTHING))
	{
		uint32_t iconID = weenie->GetIcon();

		if (iconID)
		{
			uint32_t paletteKey;
			ClothingTable *pCT = g_ClothingCache.GetTableByIndexOfIconID(iconID, 0, &paletteKey);

			if (pCT)
			{
				weenie->m_WornObjDesc.Clear();
				weenie->m_WornObjDesc.paletteID = 0x0400007E; // pObject->m_miBaseModel.dwBasePalette;

				//double shade = 1.0;
				ShadePackage shade(1.0);
				pCT->BuildObjDesc(0x02000001, paletteKey, &shade, &weenie->m_WornObjDesc);

				// use the original palettes specified?		
				weenie->m_WornObjDesc.ClearSubpalettes();
				Subpalette *pSPC = weenie->m_ObjDescOverride.firstSubpal;
				while (pSPC)
				{
					weenie->m_WornObjDesc.AddSubpalette(new Subpalette(*pSPC));
					pSPC = pSPC->next;
				}

				ClothingTable::Release(pCT);
				return true;
			}
		}
	}

	return false;
}


