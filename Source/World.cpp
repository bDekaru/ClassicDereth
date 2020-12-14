#include <StdAfx.h>
#include "World.h"
#include "InferredCellData.h"
#include "Client.h"
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "Monster.h"
#include "Player.h"
#include "ChatMsgs.h"
#include "WorldLandBlock.h"
#include "WClassID.h"
#include "Database.h"
#include "GameMode.h"
#include "Config.h"
#include "Server.h"
#include "House.h"
#include "HouseManager.h"
#include "GameEventManager.h"

CWorld::CWorld()
{
	WINLOG(Temp, Normal, "Initializing World..\n");
	SERVER_INFO << "Initializing World..";
	// ZeroMemory(m_pBlocks, sizeof(m_pBlocks));
	memset(m_pBlocks, 0, sizeof(m_pBlocks));

	LoadDungeonsFile();
	LoadMOTD();
	EnumerateDungeonsFromCellData();

	if (g_pHouseManager)
		g_pHouseManager->Load();

	m_mAllObjects.reserve(10000000);
	m_mAllPlayers.reserve(5000);
	m_fLastSave = g_pGlobals->Time();
}

void CWorld::SaveWorld()
{
	SaveDungeonsFile();

	if(g_pHouseManager)
		g_pHouseManager->Save();
}

CWorld::~CWorld()
{
	//if (m_pGameMode)
	//{
	//	delete m_pGameMode;
	//	m_pGameMode = NULL;
	//}

	for (auto block : m_vBlocks)
	{
		if (block)
		{
			m_pBlocks[block->GetHeader()] = NULL;
			delete block;
		}
	}
	m_vBlocks.clear();

	for (auto blockEntry : m_mDormantBlocks)
	{
		if (CWorldLandBlock *block = blockEntry.second)
		{
			m_pBlocks[block->GetHeader()] = NULL;
			delete block;
		}
	}
	m_mDormantBlocks.clear();

	for (auto blockEntry : m_mUnloadedBlocks)
	{
		if (CWorldLandBlock *block = blockEntry.second)
		{
			m_pBlocks[block->GetHeader()] = NULL;
			delete block;
		}
	}
	m_mUnloadedBlocks.clear();

	SaveWorld();

	m_mDungeons.clear();
	for (auto &dungeonDescEntry : m_mDungeonDescs)
	{
		DungeonDesc_t *pDungeonDesc = &dungeonDescEntry.second;

		SafeDeleteArray(pDungeonDesc->szDungeonName);
		SafeDeleteArray(pDungeonDesc->szDescription);
		SafeDeleteArray(pDungeonDesc->szAuthor);
	}
	m_mDungeonDescs.clear();
}

void CWorld::Init()
{
	std::list<uint32_t> preload = g_pCellDataEx->GetAlwaysLoaded();
	SERVER_INFO << "Preloading" << preload.size() << "blocks";
	for (auto id : preload)
		ActivateBlock(id >> 16)->NeverGoDormant();
}

Position CWorld::FindDungeonDrop()
{
	if (m_mDungeons.empty())
	{
		return Position();
	}

	LocationMap::iterator dit = m_mDungeons.begin();
	long index = Random::GenUInt(0, (long)m_mDungeons.size() - 1);
	while (index > 0) {
		index--;
		dit++;
	}

	return dit->second;
}

Position CWorld::FindDungeonDrop(WORD wBlockID)
{
	LocationMap::iterator i = m_mDungeons.upper_bound(((uint32_t(wBlockID) << 16) + 0x100) - 1);

	if ((i == m_mDungeons.end()) || (BLOCK_WORD(i->second.objcell_id) != wBlockID))
	{
		return Position();
	}

	return i->second;
}

const char* CWorld::GetMOTD()
{
	return m_strMOTD.c_str();
}

void CWorld::LoadMOTD()
{
	FILE *motd = g_pDB->DataFileOpen("motd.txt", "rt");
	if (motd)
	{
		long lFileSize = fsize(motd);
		char* pcFileData = new char[lFileSize + 1];

		long lEnd = (long)fread(pcFileData, sizeof(char), lFileSize, motd);
		pcFileData[lEnd] = 0;

		m_strMOTD = pcFileData;

		delete[] pcFileData;
		fclose(motd);
	}
	else
		m_strMOTD = "No MOTD set.";
}

void CWorld::LoadDungeonsFile()
{
	FILE *wd = g_pDB->DataFileOpen("worlddesc");
	if (wd)
	{
		long lSize = fsize(wd);
		BYTE* pbData = new BYTE[lSize];
		long lRead = (long)fread(pbData, sizeof(BYTE), lSize, wd);
		BinaryReader input(pbData, lRead);

		uint32_t dwDungeonCount = input.ReadUInt32();

		if (!input.GetLastError())
		{
			for (uint32_t i = 0; i < dwDungeonCount; i++)
			{
				DungeonDesc_t dd;
				dd.wBlockID = input.ReadUInt16();
				dd.szDungeonName = input.ReadString();
				dd.szAuthor = input.ReadString();
				dd.szDescription = input.ReadString();

				dd.position.UnPack(&input);
				if (input.GetLastError()) break;

				// avoid using a buffer thats going to be deleted
				dd.szDungeonName = strdup(dd.szDungeonName);
				dd.szAuthor = strdup(dd.szAuthor);
				dd.szDescription = strdup(dd.szDescription);

				m_mDungeonDescs[dd.wBlockID] = dd;
			}
		}

		delete[] pbData;
		fclose(wd);
	}
}

void CWorld::SaveDungeonsFile()
{
	FILE *wd = g_pDB->DataFileCreate("worlddesc");
	if (wd)
	{
		BinaryWriter output;

		output.Write<uint32_t>((uint32_t)m_mDungeonDescs.size());

		for (DungeonDescMap::iterator i = m_mDungeonDescs.begin(); i != m_mDungeonDescs.end(); i++)
		{
			output.Write<WORD>(i->second.wBlockID);
			output.WriteString(i->second.szDungeonName);
			output.WriteString(i->second.szAuthor);
			output.WriteString(i->second.szDescription);
			i->second.position.Pack(&output);
		}

		fwrite(output.GetData(), output.GetSize(), sizeof(BYTE), wd);
		fclose(wd);
	}
	else
		//MsgBox(MB_ICONHAND, "Error opening WorldDesc file! Close NOW to avoid corruption!");
		SERVER_ERROR << "Error opening WorldDesc file! Close NOW to avoid corruption!";
}

BOOL CWorld::DungeonExists(WORD wBlockID)
{
	LocationMap::iterator i = m_mDungeons.upper_bound(((uint32_t(wBlockID) << 16) + 0x100) - 1);

	if ((i == m_mDungeons.end()) || (BLOCK_WORD(i->second.objcell_id) != wBlockID))
		return FALSE;

	return TRUE;
}

LocationMap* CWorld::GetDungeons()
{
	return &m_mDungeons;
}

DungeonDescMap* CWorld::GetDungeonDescs()
{
	return &m_mDungeonDescs;
}

DungeonDesc_t* CWorld::GetDungeonDesc(const char* szDungeonName)
{
	DungeonDescMap::iterator i = m_mDungeonDescs.begin();
	DungeonDescMap::iterator iend = m_mDungeonDescs.end();

	std::string dungeonName = "";
	std::string searchName = szDungeonName;

	while (i != iend)
	{
		dungeonName = i->second.szDungeonName;

		std::transform(dungeonName.begin(), dungeonName.end(), dungeonName.begin(), ::tolower);
		std::transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);


		if (dungeonName.find(searchName) != std::string::npos)
			return &i->second;

		i++;
	}

	return NULL;
}

DungeonDesc_t* CWorld::GetDungeonDesc(WORD wBlockID)
{
	DungeonDescMap::iterator it = m_mDungeonDescs.find(wBlockID);

	if (it != m_mDungeonDescs.end())
		return &it->second;
	else
		return NULL;
}

void CWorld::SetDungeonDesc(WORD wBlockID, const char* szDungeonName, const char* szAuthor, const char* szDescription, const Position &position)
{
	DungeonDescMap::iterator it = m_mDungeonDescs.find(wBlockID);

	if (it != m_mDungeonDescs.end())
	{
		DungeonDesc_t* pdd = &it->second;
		SafeDeleteArray(pdd->szDungeonName);
		SafeDeleteArray(pdd->szDescription);
		SafeDeleteArray(pdd->szAuthor);
		m_mDungeonDescs.erase(it);
	}

	DungeonDesc_t dd;
	dd.wBlockID = wBlockID;
	dd.szDungeonName = strdup(szDungeonName);
	dd.szAuthor = strdup(szAuthor);
	dd.szDescription = strdup(szDescription);
	dd.position = position;

	m_mDungeonDescs[wBlockID] = dd;
}

void CWorld::ClearAllSpawns()
{
	for (uint32_t i = 0; i < (256 * 256); i++)
	{
		if (m_pBlocks[i])
			m_pBlocks[i]->ClearSpawns();
	}
}

CWorldLandBlock *CWorld::GetLandblock(WORD wHeader, bool bActivate)
{
	CWorldLandBlock *pBlock = m_pBlocks[wHeader];
	if (!pBlock && bActivate)
	{
		pBlock = ActivateBlock(wHeader);
	}

	return pBlock;
}

CWorldLandBlock *CWorld::ActivateBlock(WORD wHeader)
{
	CWorldLandBlock **ppBlock = &m_pBlocks[wHeader];
	CWorldLandBlock *pBlock;

#if _DEBUG
	pBlock = *ppBlock;
	if (pBlock != NULL)
	{
		SERVER_INFO << "Landblock already active!";
		return pBlock;
	}
#endif

	// make sure this isn't already in our spawned blocked...
	pBlock = new CWorldLandBlock(this, wHeader);
	m_vSpawns.push_back(pBlock);
	*ppBlock = pBlock;

	pBlock->Init();

	return pBlock;
}

bool CWorld::CreateEntity(CWeenieObject *pEntity, bool bMakeAware)
{
	if (!pEntity)
		return false;

	CWeenieObject *pExistingWeenie = FindObject(pEntity->GetID());
    if (pExistingWeenie)
	{
		if (pExistingWeenie == pEntity)
		{
			LOG_PRIVATE(World, Warning, "Trying to spawn weenie twice! ID 0x%08X\n");
			return true;
		}
		else
		{
			//LOG_PRIVATE(World, Warning, "Trying to spawn second (different) weenie with existing ID 0x%08X! Deleting instead.\n", pEntity->GetID());

			// Already exists.
			delete pEntity;
		}

		return false;
	}

	double createTime;
	if (!pEntity->m_Qualities.InqFloat(CREATION_TIMESTAMP_FLOAT, createTime))
		pEntity->m_Qualities.SetFloat(CREATION_TIMESTAMP_FLOAT, Timer::cur_time);

	int lifespan = pEntity->m_Qualities.GetInt(LIFESPAN_INT, 0);

	if (lifespan && !pEntity->m_Qualities.GetInt(CREATION_TIMESTAMP_INT, 0))
	{
		time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		pEntity->m_Qualities.SetInt(CREATION_TIMESTAMP_INT, t);
		pEntity->_timeToRot = Timer::cur_time + lifespan;
	}

	if (!pEntity->GetID())
	{
		pEntity->SetID(CreateWorldID(pEntity));
	}

	if (pEntity->AsPlayer() && pEntity->AsPlayer()->GetClient())
	{
		m_mAllPlayers[pEntity->GetID()] = pEntity->AsPlayer();

		if (g_pConfig->ShowLogins() && pEntity->m_Qualities.id == W_HUMAN_CLASS) // Admins playing as creatures should not show as logging in
		{
			BroadcastGlobal(ServerBroadcast("System", csprintf("%s has logged in.", pEntity->GetName().c_str()), LTT_DEFAULT), PRIVATE_MSG, pEntity->GetID(), FALSE, TRUE);
		}
	}

	m_mAllObjects[pEntity->GetID()] = pEntity;

	//std::string eventString;
	//if (pEntity->m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	//{
	//	_eventWeenies.insert(std::pair<std::string, uint32_t>(eventString, pEntity->GetID()));
	//	GameEventDef *ed = g_pGameEventManager->GetEvent(eventString.c_str());
	//	if (ed)
	//		pEntity->CheckEventState(eventString, ed);
	//}

	pEntity->InitPhysicsObj();
	pEntity->SetupWeenie();
	pEntity->RecacheHasOwner();

	if (pEntity->m_Position.objcell_id)
	{
		Position startPos = pEntity->m_Position;

		pEntity->enter_world(&startPos);

		if (!pEntity->m_Position.objcell_id)
		{
			LOG_PRIVATE(World, Warning, "Trying to spawn a weenie in an invalid position! Deleting instead.\n", pEntity->GetID());

			RemoveEntity(pEntity);

			// Bad spawn position.
			// delete pEntity;

			return false;
		}

		if (!pEntity->CachedHasOwner())
		{
			WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());
			CWorldLandBlock *pBlock = m_pBlocks[wHeader];

			if (!pBlock)
			{
				pBlock = ActivateBlock(wHeader);
			}

			pEntity->PreSpawnCreate();

			pBlock->Insert(pEntity, 0, TRUE, bMakeAware);
		}
	}
	else
	{
		pEntity->PreSpawnCreate();
	}

	if (bMakeAware)
		pEntity->MakeAware(pEntity, true);

	std::string eventString;
	if (pEntity->m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	{
		_eventWeenies.insert(std::pair<std::string, uint32_t>(eventString, pEntity->GetID()));
		GameEventDef *ed = g_pGameEventManager->GetEvent(eventString.c_str());
		if (ed)
			pEntity->CheckEventState(eventString, ed);
	}
	else
		pEntity->InitCreateGenerator();

	pEntity->PostSpawn();

	if (pEntity->cell)
	{
		assert(pEntity->GetLandcell() == pEntity->cell->id);
	}

#ifdef _DEBUG
	SERVER_INFO << "Spawned ID" << pEntity->GetID() << "- " << pEntity->GetName().c_str() << "memory object @" << (uint64_t)pEntity;
#endif

	return true;
}

uint32_t CWorld::CreateWorldID(CWeenieObject * pEntity)
{
	bool useEphemeral = (pEntity->m_PhysicsState & MISSILE_PS)
		// player corpses are invisible at create time
		|| (pEntity->IsCorpse() && pEntity->m_Qualities.GetBool(VISIBILITY_BOOL, true))
		|| pEntity->IsStuck()
		;

	if (pEntity->GetWorldTopLevelOwner() && pEntity->GetWorldTopLevelOwner()->m_Qualities.GetInt(MERCHANDISE_ITEM_TYPES_INT, 0) > 0)
		useEphemeral = true;

	uint32_t isPlayerRange = 0;
	if(pEntity->m_Qualities.InqInstanceID(VICTIM_IID, isPlayerRange) && isPlayerRange >= 1342177280 && isPlayerRange <= 1610612735)
		useEphemeral = false;

	return GenerateGUID(useEphemeral ? eEphemeral : eDynamicGUID);
}

void CWorld::InsertTeleportLocation(TeleTownList_s l)
{
	m_vTeleTown.push_back(l);
}

std::string CWorld::GetTeleportList()
{
	std::string result;

	for (TeleTownList_s var : m_vTeleTown)
	{
		result.append(var.m_teleString).append(", ");
	}
	if (!result.empty() && result.back() == 0x20) { //Get out behind of bad formatting
		result.pop_back();
		result.pop_back();
	}

	return result;
}

TeleTownList_s CWorld::GetTeleportLocation(std::string location)
{
	TeleTownList_s val;
	std::transform(location.begin(), location.end(), location.begin(), ::tolower);
	for (TeleTownList_s var : m_vTeleTown)
	{
		//Lets waste a bunch of time with this.. Hey, if its the first one on the list its O(1)
		std::string town = var.m_teleString;
		std::transform(town.begin(), town.end(), town.begin(), ::tolower);
		if (town[0] == location[0] && town.find(location) != std::string::npos) {
			val = var;
			break;
		}
	}
	return val;
}

void CWorld::InsertEntity(CWeenieObject *pEntity, BOOL bSilent)
{
	uint32_t cell_id = pEntity->GetLandcell();

	if (cell_id && !pEntity->HasOwner())
	{
		WORD wHeader = BLOCK_WORD(cell_id);

		CWorldLandBlock *pBlock = m_pBlocks[wHeader];
		if (!pBlock)
		{
			pBlock = ActivateBlock(wHeader);
		}

		if (bSilent)
			pBlock->Insert(pEntity, wHeader);
		else
			pBlock->Insert(pEntity);
	}

	// these should not be necessary... ?
	if (pEntity->AsPlayer())
	{
		m_mAllPlayers[pEntity->GetID()] = pEntity->AsPlayer();
	}

	m_mAllObjects[pEntity->GetID()] = pEntity;
}

void CWorld::JuggleEntity(WORD wOld, CWeenieObject* pEntity)
{
	if (!pEntity->HasOwner())
	{
		WORD wHeader = BLOCK_WORD(pEntity->GetLandcell());

		CWorldLandBlock *pBlock = GetLandblock(wHeader);
		if (!pBlock)
		{
			pBlock = ActivateBlock(wHeader);
		}

		if (pEntity->_IsPlayer() || pBlock->IsTickingWithWorld())
		{
			pBlock->Insert(pEntity, wOld);
		}
		else
		{
			if (pEntity->ShouldSave())
			{
				pEntity->Save();
			}

			EnsureRemoved(pEntity);

			uint32_t RemoveObject[3];
			RemoveObject[0] = 0xF747;
			RemoveObject[1] = pEntity->GetID();
			RemoveObject[2] = pEntity->_instance_timestamp;
			BroadcastPVS(pEntity->GetLandcell(), RemoveObject, sizeof(RemoveObject));

			pEntity->exit_world();
			pEntity->leave_world();
			pEntity->unset_parent();
			pEntity->unparent_children();

			if (uint32_t generator_id = pEntity->InqIIDQuality(GENERATOR_IID, 0))
			{
				CWeenieObject *target = g_pWorld->FindObject(generator_id);
				if (target)
					target->NotifyGeneratedDeath(pEntity);
			}

			DELETE_ENTITY(pEntity);
		}
	}
}

PlayerWeenieMap *CWorld::GetPlayers()
{
	return &m_mAllPlayers;
}

uint32_t CWorld::GetNumPlayers()
{
	return (uint32_t)m_mAllPlayers.size();
}

CWeenieObject *CWorld::FindObject(uint32_t object_id, bool allowLandblockActivation, bool lockObject)
{
	if (!object_id)
		return NULL;

	WeenieMap::iterator result = m_mAllObjects.find(object_id);

	if (result == m_mAllObjects.end() && allowLandblockActivation)
	{
		if (!g_pDBIO->IsWeenieInDatabase(object_id))
			return NULL; //not in the database either

		CWeenieObject *weenie = CWeenieObject::Load(object_id);
		if (!weenie)
			return NULL;

		//try getting a valid cellId to search in
		Position pos;
		uint32_t cellId = weenie->m_Position.objcell_id;
		if (!cellId && weenie->m_Qualities.InqPosition(LOCATION_POSITION, pos))
			cellId = pos.objcell_id;
		if (!cellId && weenie->m_Qualities.InqPosition(INSTANTIATION_POSITION, pos))
			cellId = pos.objcell_id;
			
		if (cellId)
		{
			WORD header = (cellId >> 16);
			CWorldLandBlock *block = GetLandblock(header, false);
			if (!block) //we're not active
			{
				block = GetLandblock(header, true);
				if (block) //now we're active!
				{
					block->Think(); //force a tick so we spawn stuff and hopefully what we are looking for.
					result = m_mAllObjects.find(object_id); //try again now that we loaded the landblock that supposedly contains the weenie.
				}
			}
		}
		delete weenie;
	}
	
	if(result == m_mAllObjects.end())
		return NULL;
	
	return result->second;
}

bool CWorld::FindObjectName(uint32_t object_id, std::string &name)
{
	WeenieMap::iterator result = m_mAllObjects.find(object_id);

	if (result == m_mAllObjects.end())
		return false;

	name = result->second->GetName();
	return true;
}

//==================================================
//Global, player search by GUID.
//==================================================
CPlayerWeenie *CWorld::FindPlayer(uint32_t dwGUID)
{
	PlayerWeenieMap::iterator result = m_mAllPlayers.find(dwGUID);

	if (result == m_mAllPlayers.end())
		return NULL;

	return result->second;
}

std::string CWorld::GetPlayerName(uint32_t playerId, bool allowOffline)
{
	CPlayerWeenie *player = FindPlayer(playerId);

	if (player)
		return player->GetName();

	if (allowOffline)
		return g_pDBIO->GetPlayerCharacterName(playerId);

	return "";
}

uint32_t CWorld::GetPlayerId(const char *name, bool allowOffline)
{
	if (*name == '+')
		name++;

	CPlayerWeenie *player = FindPlayer(name);

	if (player)
		return player->GetID();

	if (allowOffline)
		return g_pDBIO->GetPlayerCharacterId(name);

	return 0;
}

//==================================================
//Global, case insensitive player name search.
//==================================================
CPlayerWeenie *CWorld::FindPlayer(const char *target_name)
{
	if (!target_name)
		return NULL;
	
	if (*target_name == '+')
		target_name++;

	for (auto entry : m_mAllPlayers)
	{
		CPlayerWeenie *player = entry.second;

		if (player)
		{
			std::string player_name_ = player->GetName();
			const char *player_name = player_name_.c_str();

			if (*player_name == '+')
				player_name++;

			if (!stricmp(target_name, player_name))
				return player;
		}
	}

	return NULL;
}

void CWorld::SendNetMessage(CNetDeliveryTargets *target, void *data, uint32_t len, WORD group, uint32_t ignore_ent, BOOL game_event)
{
	UNFINISHED();

	for (uint32_t cell_id : target->_target_cell_pvs)
	{
		CWorldLandBlock *landBlock = GetLandblock(cell_id >> 16, false);
		if (!landBlock)
			continue;

		CObjCell *cell = landBlock->GetObjCell(cell_id & 0xFFFF);
		if (!cell)
			continue;

		// could use stab list, but sometimes messages are sent outside of stab list.. sigh, really need to use some form of voyeur tracking
	}
}

void CWorld::BroadcastPVS(CPhysicsObj *physobj, void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral)
{
	if (!physobj)
		return;

	CPhysicsObj *topLevel = physobj->parent ? physobj->parent : physobj;

	uint32_t cell_id;

	cell_id = topLevel->m_Position.objcell_id;
	BroadcastPVS(cell_id, _data, _len, _group, ignore_ent, _game_event, ephemeral);

	uint32_t old_cell_id = topLevel->last_tick_cell_id;
	if (cell_id != old_cell_id && old_cell_id)
	{
		BroadcastPVS(old_cell_id, _data, _len, _group, ignore_ent, _game_event, ephemeral);
	}
}

void CWorld::BroadcastPVS(CWeenieObject *weenie, void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral)
{
	if (!weenie)
		return;

	if (weenie->IsContained() ||
	   (weenie->IsEquipped() && weenie->InqIntQuality(PARENT_LOCATION_INT, 0) == 0)) //we're equipped but we dont have a physical presence in the world, make sure to include us here.
	{
		uint32_t topLevelID = weenie->GetTopLevelID();
		if (topLevelID != weenie->GetID() && ignore_ent != topLevelID)
		{
			CWeenieObject *owner = FindObject(topLevelID);

			if (owner)
			{
				owner->SendNetMessage(_data, _len, _group, _game_event, ephemeral);

				if (owner->AsContainer() != NULL && owner->AsContainer()->_openedById != 0)
				{
					CWeenieObject *openedBy = FindObject(owner->AsContainer()->_openedById);

					if(openedBy)
						openedBy->SendNetMessage(_data, _len, _group, _game_event, ephemeral);
				}
			}
		}
	}
	else
	{
		BroadcastPVS((CPhysicsObj *)weenie, _data, _len, _group, ignore_ent, _game_event, ephemeral);
	}
}

void CWorld::BroadcastPVS(const Position &pos, void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral)
{
	BroadcastPVS(pos.objcell_id, _data, _len, _group, ignore_ent, _game_event, ephemeral);
}

void CWorld::BroadcastPVS(uint32_t dwCell, void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral)
{
	if (!dwCell)
		return;

	// Don't ask.
	WORD block = BLOCK_WORD(dwCell);
	WORD cell = CELL_WORD(dwCell);

	uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
	uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

	uint32_t minx = basex;
	uint32_t maxx = basex;
	uint32_t miny = basey;
	uint32_t maxy = basey;

	//if ( cell < 0xFF ) //indoor structure
	{
		if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
		if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

		if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
		if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
	}

	minx = BLOCK_OFFSET(minx) << 8;
	miny = BLOCK_OFFSET(miny);
	maxx = BLOCK_OFFSET(maxx) << 8;
	maxy = BLOCK_OFFSET(maxy);

	CWorldLandBlock *pBlock = m_pBlocks[block];
	if ((pBlock && (dwCell & 0xFFFF) > 0x100) && !pBlock->PossiblyVisibleToOutdoors(dwCell)) //check if inside
	{
		// dungeons are usually contained in water blocks usually and we will only broadcast to them individually
		pBlock->Broadcast(_data, _len, _group, ignore_ent, _game_event, ephemeral);
	}
	else
	{
		for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
			for (uint32_t yit = miny; yit <= maxy; yit += 1)
			{
				CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
				if (pBlock)
					pBlock->Broadcast(_data, _len, _group, ignore_ent, _game_event, ephemeral);
			}
		}
	}
}

void CWorld::BroadcastGlobal(BinaryWriter *writer, WORD group, uint32_t ignore_ent, BOOL game_event, BOOL should_delete)
{
	BroadcastGlobal(writer->GetData(), writer->GetSize(), group, ignore_ent, game_event);

	if (should_delete)
	{
		delete writer;
	}
}

void CWorld::BroadcastGlobal(void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event)
{
	for (auto &playerEntry : m_mAllPlayers)
	{
		if (CPlayerWeenie *pPlayer = playerEntry.second)
		{
			if (!ignore_ent || (pPlayer->GetID() != ignore_ent))
			{
				pPlayer->SendNetMessage(_data, _len, _group, _game_event);
			}
		}
	}
}

void CWorld::BroadcastLocal(uint32_t cellid, std::string text)
{
	BinaryWriter *textMsg = ServerText(text.c_str(), LTT_DEFAULT);
	g_pWorld->BroadcastPVS(cellid, textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0, false);
	delete textMsg;
}

void CWorld::BroadcastLocal(uint32_t cellid, std::string text, LogTextType channel)
{
	BinaryWriter *textMsg = ServerText(text.c_str(), channel);
	g_pWorld->BroadcastPVS(cellid, textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0, false);
	delete textMsg;
}

void CWorld::Test()
{
	WINLOG(Temp, Normal, "<CWorld::Test()>\n");
	WINLOG(Temp, Normal, "Portal: v%lu, %lu files.\n", g_pPortal->GetVersion(), g_pPortal->GetFileCount());
	WINLOG(Temp, Normal, "Cell: v%lu, %u files.\n", g_pCell->GetVersion(), g_pCell->GetFileCount());
	WINLOG(Temp, Normal, "%u objects", m_mAllObjects.size());
	WINLOG(Temp, Normal, "%u players:\n", m_mAllPlayers.size());
	for (PlayerWeenieMap::iterator pit = m_mAllPlayers.begin(); pit != m_mAllPlayers.end(); pit++)
	{
		CPlayerWeenie *pPlayer = pit->second;
		WINLOG(Temp, Normal, "%08X %s\n", pPlayer->GetID(), pPlayer->GetName().c_str());
	}
	WINLOG(Temp, Normal, "%u active blocks:\n", m_vBlocks.size());
	for (LandblockVector::iterator it = m_vBlocks.begin(); it != m_vBlocks.end(); it++)
	{
		CWorldLandBlock *pBlock = *it;
		WINLOG(Temp, Normal, "%04X %u players %u entities\n", pBlock->GetHeader(), pBlock->PlayerCount(), pBlock->LiveCount());
	}
	WINLOG(Temp, Normal, "%u dormant blocks:\n", m_mDormantBlocks.size());
	for (LandblockMap::iterator it = m_mDormantBlocks.begin(); it != m_mDormantBlocks.end(); it++)
	{
		CWorldLandBlock *pBlock = it->second;
		WINLOG(Temp, Normal, "%04X %u players %u entities\n", pBlock->GetHeader(), pBlock->PlayerCount(), pBlock->LiveCount());
	}
	WINLOG(Temp, Normal, "%u unloaded blocks:\n", m_mUnloadedBlocks.size());
	for (LandblockMap::iterator it = m_mUnloadedBlocks.begin(); it != m_mUnloadedBlocks.end(); it++)
	{
		CWorldLandBlock *pBlock = it->second;
		WINLOG(Temp, Normal, "%04X %u players %u entities\n", pBlock->GetHeader(), pBlock->PlayerCount(), pBlock->LiveCount());
	}

	WINLOG(Temp, Normal, "</CWorld::Test()>\n");
}

void CWorld::RemoveEntity(CWeenieObject *pEntity)
{
	if (!pEntity)
		return;

	//if (m_pGameMode)
	//	m_pGameMode->OnRemoveEntity(pEntity);

	if (pEntity->AsPlayer() && g_pConfig->ShowLogins() && pEntity->m_Qualities.id == W_HUMAN_CLASS)
	{
		BroadcastGlobal(ServerBroadcast("System", csprintf("%s has logged out.", pEntity->GetName().c_str()), LTT_DEFAULT), PRIVATE_MSG, pEntity->GetID(), FALSE, TRUE);
	}

	CWorldLandBlock *pBlock = pEntity->GetBlock();

	if (!pBlock)
	{
		if (pEntity->ShouldSave())
			pEntity->Save();

		pEntity->exit_world();
		pEntity->leave_world();
		pEntity->unset_parent();
		pEntity->unparent_children();

		EnsureRemoved(pEntity);

		if (uint32_t generator_id = pEntity->InqIIDQuality(GENERATOR_IID, 0))
		{
			CWeenieObject *target = g_pWorld->FindObject(generator_id);
			if (target)
				target->NotifyGeneratedDeath(pEntity);
		}

		DELETE_ENTITY(pEntity);
	}
	else
	{
		pBlock->Destroy(pEntity);
	}
}

void CWorld::EnsureRemoved(CWeenieObject *pEntity)
{
	m_mAllPlayers.erase(pEntity->GetID());
	m_mAllObjects.erase(pEntity->GetID());

	std::string eventString;
	if (pEntity->m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	{
		auto range_pair = _eventWeenies.equal_range(eventString);
		
		for (auto it = range_pair.first; it != range_pair.second; ++it) {
			if (it->second == pEntity->GetID()) {
				_eventWeenies.erase(it);
				break;
			}
		}
	}
}

void CWorld::CheckDormantLandblocks()
{
	LandblockMap::iterator lit = m_mDormantBlocks.begin();
	LandblockMap::iterator lend = m_mDormantBlocks.end();

	unsigned int minNumToCleanup = 0;
	if (m_mDormantBlocks.size() >= g_pConfig->MaxDormantLandblocks())
		minNumToCleanup = (uint32_t)(m_mDormantBlocks.size() - g_pConfig->MaxDormantLandblocks());

	double dormantCleanupTime = g_pConfig->DormantLandblockCleanupTime();

	while (lit != lend)
	{
		CWorldLandBlock *pBlock = lit->second;

		if (!minNumToCleanup && (pBlock->GetDormancyTime() + dormantCleanupTime) >= Timer::cur_time)
		{
			// theoretically these are sorted with the most recent last
			break;
		}
		else
		{
			lit = m_mDormantBlocks.erase(lit);
			lend = m_mDormantBlocks.end();

			pBlock->UnloadSpawnsUntilNextTick();

			m_mUnloadedBlocks[pBlock->GetHeader()] = pBlock;
			minNumToCleanup--;
		}
	}
}

void CWorld::Think()
{
	if (!m_SendPerformanceInfoToPlayer)
	{
		LandblockVector::iterator lit = m_vBlocks.begin();
		LandblockVector::iterator lend = m_vBlocks.end();

		while (lit != lend)
		{
			CWorldLandBlock *pBlock = *lit;

			if (pBlock->Think() || pBlock->GetDormancyStatus() != LandblockDormancyStatus::Dormant)
			{
				lit++;
			}
			else
			{
				lit = m_vBlocks.erase(lit);
				lend = m_vBlocks.end();
				m_mDormantBlocks[pBlock->GetHeader()] = pBlock;
				pBlock->SetIsTickingWithWorld(false);
			}
		}

		CheckDormantLandblocks();
	}
	else
	{
		// Debug performance

		double fSlowestBlock = -1.0;
		CWorldLandBlock *pSlowestBlock = NULL;
		CStopWatch measureBlocks;

		LandblockVector::iterator lit = m_vBlocks.begin();
		LandblockVector::iterator lend = m_vBlocks.end();

		while (lit != lend)
		{
			CWorldLandBlock *pBlock = *lit;

			CStopWatch measureBlock;

			if (pBlock->Think())
			{
				double elapsed = measureBlock.GetElapsed();

				if (elapsed > fSlowestBlock)
				{
					fSlowestBlock = elapsed;
					pSlowestBlock = pBlock;
				}

				lit++;
			}
			else
			{
				lit++;
				// dead
				//lit = m_vBlocks.erase(lit);
				//lend = m_vBlocks.end();
				//m_pBlocks[pBlock->GetHeader()] = NULL;
				//delete pBlock;
			}
		}

		double totalElapsed = measureBlocks.GetElapsed();

		CPlayerWeenie *player = FindPlayer(m_SendPerformanceInfoToPlayer);
		if (player)
		{
			player->SendText(csprintf("FrameTime: %f", totalElapsed), LTT_DEFAULT);

			if (pSlowestBlock)
			{
				player->SendText(csprintf("Slowest block %04X took %f seconds (%u objects, %u players), avg block took. %f seconds",
					pSlowestBlock->GetHeader(), fSlowestBlock, pSlowestBlock->LiveCount(), pSlowestBlock->PlayerCount(), totalElapsed / (double)m_vBlocks.size()), LTT_DEFAULT);
			}
		}

		m_SendPerformanceInfoToPlayer = 0;
	}

	if (!m_vSpawns.empty())
	{
		std::copy(m_vSpawns.begin(), m_vSpawns.end(), std::back_inserter(m_vBlocks));
		m_vSpawns.clear();
	}

	if ((m_fLastSave + 300.0f) <= g_pGlobals->Time())
	{
		SaveWorld();
		m_fLastSave = g_pGlobals->Time();
	}

	//if (m_pGameMode)
	//{
	//	m_pGameMode->Think();
	//}

//#ifdef _DEBUG
//	for (auto worldObject : m_mAllObjects)
//	{
//		if (worldObject.second)
//			worldObject.second->DebugValidate();
//	}
//#endif
}

CGameMode *CWorld::GetGameMode()
{
	return m_pGameMode;
}

void CWorld::SetNewGameMode(CGameMode *pGameMode)
{
	return;

	if (pGameMode)
	{
		g_pWorld->BroadcastGlobal(ServerText(csprintf("Setting game mode to %s", pGameMode->GetName())), PRIVATE_MSG);
	}
	else
	{
		if (!pGameMode && m_pGameMode)
		{
			g_pWorld->BroadcastGlobal(ServerText(csprintf("Turning off game mode %s", m_pGameMode->GetName())), PRIVATE_MSG);
		}
	}

	if (m_pGameMode)
	{
		delete m_pGameMode;
	}

	m_pGameMode = pGameMode;
}

void CWorld::EnumNearby(const Position &position, float fRange, std::list<CWeenieObject *> *pResults)
{
	// Enumerate nearby world objects
	uint32_t dwCell = position.objcell_id;
	WORD block = BLOCK_WORD(dwCell);
	WORD cell = CELL_WORD(dwCell);

	CWorldLandBlock *pBlock = m_pBlocks[block];
	if ((pBlock && (dwCell & 0xFFFF) > 0x100) && !pBlock->PossiblyVisibleToOutdoors(dwCell)) //check if inside
	{
		pBlock->EnumNearby(position, fRange, pResults);
	}
	else
	{
		uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
		uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

		uint32_t minx = basex;
		uint32_t maxx = basex;
		uint32_t miny = basey;
		uint32_t maxy = basey;

		//if ( cell < 0xFF ) // indoor structure
		{
			if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
			if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

			if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
			if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
		}

		minx = BLOCK_OFFSET(minx) << 8;
		miny = BLOCK_OFFSET(miny);
		maxx = BLOCK_OFFSET(maxx) << 8;
		maxy = BLOCK_OFFSET(maxy);

		for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
			for (uint32_t yit = miny; yit <= maxy; yit += 1)
			{
				CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
				if (pBlock)
				{
					pBlock->EnumNearby(position, fRange, pResults);
				}
			}
		}
	}
}


void CWorld::EnumNearbyPlayers(const Position &position, float fRange, std::list<CWeenieObject *> *pResults)
{
	// Enumerate nearby world objects
	uint32_t dwCell = position.objcell_id;
	WORD block = BLOCK_WORD(dwCell);
	WORD cell = CELL_WORD(dwCell);

	CWorldLandBlock *pBlock = m_pBlocks[block];
	if ((pBlock && (dwCell & 0xFFFF) > 0x100) && !pBlock->PossiblyVisibleToOutdoors(dwCell)) //check if inside
	{
		pBlock->EnumNearbyPlayers(position, fRange, pResults);
	}
	else
	{
		uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
		uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

		uint32_t minx = basex;
		uint32_t maxx = basex;
		uint32_t miny = basey;
		uint32_t maxy = basey;

		//if ( cell < 0xFF ) // indoor structure
		{
			if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
			if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

			if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
			if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
		}

		minx = BLOCK_OFFSET(minx) << 8;
		miny = BLOCK_OFFSET(miny);
		maxx = BLOCK_OFFSET(maxx) << 8;
		maxy = BLOCK_OFFSET(maxy);

		for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
			for (uint32_t yit = miny; yit <= maxy; yit += 1)
			{
				CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
				if (pBlock)
				{
					pBlock->EnumNearbyPlayers(position, fRange, pResults);
				}
			}
		}
	}
}

void CWorld::EnsureBlockIsTicking(CWorldLandBlock *pBlock)
{
	if (!pBlock->IsTickingWithWorld())
	{
		LandblockMap::iterator entry;

		entry = m_mDormantBlocks.find(pBlock->GetHeader());
		if (entry != m_mDormantBlocks.end())
		{
			m_mDormantBlocks.erase(entry);
			m_vSpawns.push_back(pBlock);
			pBlock->SetIsTickingWithWorld(true);
			return;
		}

		entry = m_mUnloadedBlocks.find(pBlock->GetHeader());
		if (entry != m_mUnloadedBlocks.end())
		{
			m_mUnloadedBlocks.erase(entry);
			m_vSpawns.push_back(pBlock);
			pBlock->SetIsTickingWithWorld(true);
			return;
		}
	}
}

void CWorld::EnumNearby(CWeenieObject *pSource, float fRange, std::list<CWeenieObject *> *pResults)
{
	// Enumerate nearby world objects
	if (pSource != NULL && !pSource->HasOwner())
	{
		uint32_t dwCell = pSource->GetLandcell();
		WORD block = BLOCK_WORD(dwCell);
		WORD cell = CELL_WORD(dwCell);

		CWorldLandBlock *pBlock = m_pBlocks[block];
		if ((pBlock && (dwCell & 0xFFFF) > 0x100) && !pBlock->PossiblyVisibleToOutdoors(dwCell)) //check if inside
		{
			pBlock->EnumNearby(pSource, fRange, pResults);
		}
		else
		{
			uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
			uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

			uint32_t minx = basex;
			uint32_t maxx = basex;
			uint32_t miny = basey;
			uint32_t maxy = basey;

			//if ( cell < 0xFF ) // indoor structure
			{
				if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
				if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

				if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
				if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
			}

			minx = BLOCK_OFFSET(minx) << 8;
			miny = BLOCK_OFFSET(miny);
			maxx = BLOCK_OFFSET(maxx) << 8;
			maxy = BLOCK_OFFSET(maxy);

			for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
				for (uint32_t yit = miny; yit <= maxy; yit += 1)
				{
					CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
					if (pBlock)
					{
						pBlock->EnumNearby(pSource, fRange, pResults);
					}
				}
			}
		}
	}
}

void CWorld::EnumNearbyPlayers(CWeenieObject *pSource, float fRange, std::list<CWeenieObject *> *pResults)
{
	// Enumerate nearby world objects
	if (pSource != NULL && !pSource->HasOwner())
	{
		uint32_t dwCell = pSource->GetLandcell();
		WORD block = BLOCK_WORD(dwCell);
		WORD cell = CELL_WORD(dwCell);

		CWorldLandBlock *pBlock = m_pBlocks[block];
		if ((pBlock && (dwCell & 0xFFFF) > 0x100) && !pBlock->PossiblyVisibleToOutdoors(dwCell)) //check if inside
		{
			pBlock->EnumNearbyPlayers(pSource, fRange, pResults);
		}
		else
		{
			uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
			uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

			uint32_t minx = basex;
			uint32_t maxx = basex;
			uint32_t miny = basey;
			uint32_t maxy = basey;

			//if ( cell < 0xFF ) // indoor structure
			{
				if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
				if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

				if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
				if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
			}

			minx = BLOCK_OFFSET(minx) << 8;
			miny = BLOCK_OFFSET(miny);
			maxx = BLOCK_OFFSET(maxx) << 8;
			maxy = BLOCK_OFFSET(maxy);

			for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
				for (uint32_t yit = miny; yit <= maxy; yit += 1)
				{
					CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
					if (pBlock)
					{
						pBlock->EnumNearbyPlayers(pSource, fRange, pResults);
					}
				}
			}
		}
	}
}

CWeenieObject *CWorld::FindWithinPVS(CWeenieObject *source, uint32_t object_id, bool allowOwnedByOthers)
{
	if (!source || !object_id)
		return NULL;

	if (source->GetID() == object_id)
		return source;

	//Find nearby world objects.
	if (!source->HasOwner())
	{
		uint32_t dwCell = source->GetLandcell();
		WORD block = BLOCK_WORD(dwCell);
		WORD cell = CELL_WORD(dwCell);

		uint32_t basex = BASE_OFFSET(BLOCK_X(block), CELL_X(cell));
		uint32_t basey = BASE_OFFSET(BLOCK_Y(block), CELL_Y(cell));

		uint32_t minx = basex;
		uint32_t maxx = basex;
		uint32_t miny = basey;
		uint32_t maxy = basey;

		//if ( cell < 0xFF ) //indoor structure
		{
			if (minx >= (dwMinimumCellX + PVC_RANGE)) minx -= PVC_RANGE; else minx = dwMinimumCellX;
			if (maxx <= (dwMaximumCellX - PVC_RANGE)) maxx += PVC_RANGE; else maxx = dwMaximumCellX;

			if (miny >= (dwMinimumCellY + PVC_RANGE)) miny -= PVC_RANGE; else miny = dwMinimumCellY;
			if (maxy <= (dwMaximumCellY - PVC_RANGE)) maxy += PVC_RANGE; else maxy = dwMaximumCellY;
		}

		minx = BLOCK_OFFSET(minx) << 8;
		miny = BLOCK_OFFSET(miny);
		maxx = BLOCK_OFFSET(maxx) << 8;
		maxy = BLOCK_OFFSET(maxy);

		for (uint32_t xit = minx; xit <= maxx; xit += 0x100) {
			for (uint32_t yit = miny; yit <= maxy; yit += 1)
			{
				CWorldLandBlock *pBlock = m_pBlocks[xit | yit];
				if (pBlock)
				{
					CWeenieObject *pEntity = pBlock->FindEntity(object_id);
					if (pEntity)
						return pEntity;
				}
			}
		}
	}

	if (CWeenieObject *externalObject = g_pWorld->FindObject(object_id))
	{
		if (CContainerWeenie *externalContainer = externalObject->GetWorldTopLevelContainer())
		{
			if (allowOwnedByOthers || externalContainer->_openedById == source->GetID())
			{
				return externalObject;
			}
		}
	}

	return source->FindContained(object_id);
}

void CWorld::EnumerateDungeonsFromCellData()
{
#ifndef QUICKSTART

	// FILE *dropsFile = fopen("d:\\temp\\dungeonDrops.txt", "wt");

	// This creates a list of dungeons
	FILEMAP* pFiles = g_pCell->GetFiles();
	FILEMAP::iterator i = pFiles->begin();
	FILEMAP::iterator iend = pFiles->end();

	while (i != iend)
	{
		uint32_t dwID = i->first;

		if (((dwID & 0xFFFF) >= 0xFFFE) || ((dwID & 0xFFFF) < 0x100))
		{
			i++;
			continue;
		}

		CEnvCell *pEnvCell = CEnvCell::Get(dwID);
		if (!pEnvCell)
		{
			i++;
			continue;
		}

		Frame drop;
		bool bFoundDrop = false;

		for (uint32_t i = 0; i < pEnvCell->num_static_objects; i++)
		{
			uint32_t setupID = pEnvCell->static_object_ids[i];
			if ((setupID >= 0x02000C39 && setupID <= 0x02000C48) || (setupID == 0x02000F4A))
			{
				drop = pEnvCell->static_object_frames[i];
				bFoundDrop = true;
				break;
			}
		}

		/*
		if (dropsFile)
		{
		for (uint32_t i = 0; i < pEnvCell->num_static_objects; i++)
		{
		uint32_t setupID = pEnvCell->static_object_ids[i];
		if ((setupID >= 0x02000C39 && setupID <= 0x02000C48) || (setupID == 0x02000F4A))
		{
		fprintf(dropsFile, "%08X %f %f %f %f %f %f %f\n",
		dwID,
		pEnvCell->static_object_frames[i].m_origin.x,
		pEnvCell->static_object_frames[i].m_origin.y,
		pEnvCell->static_object_frames[i].m_origin.z,
		pEnvCell->static_object_frames[i].m_angles.w,
		pEnvCell->static_object_frames[i].m_angles.x,
		pEnvCell->static_object_frames[i].m_angles.y,
		pEnvCell->static_object_frames[i].m_angles.z);
		break;
		}
		}
		}
		*/

		CEnvCell::Release(pEnvCell);

		if (bFoundDrop)
		{
			Position dropPos;
			dropPos.objcell_id = dwID;
			dropPos.frame = drop;
			m_mDungeons[dwID] = dropPos;

			if (dwID < (uint32_t)(0 - 0x10100))
				i = pFiles->upper_bound(((dwID & 0xFFFF0000) + 0x10100) - 1);
			else
				break;
		}
		else
		{
			i++;
		}
	}

#endif
}

void CWorld::BroadcastChatChannel(uint32_t channel_id, CPlayerWeenie *sender, const std::u16string &message)
{
	uint32_t sender_monarch_id = 0;
	if (channel_id == Allegiance_ChatChannel)
	{
		sender_monarch_id = sender->InqIIDQuality(MONARCH_IID, 0);
		if (!sender_monarch_id)
		{
			return;
		}
	}

	BinaryWriter chatPayload;
	chatPayload.Write<uint32_t>(channel_id);
	chatPayload.AppendWStringFromString(sender->GetName());
	//chatPayload.AppendWStringFromString(message);
	chatPayload.Write<std::u16string>(message);
	chatPayload.Write<uint32_t>(12); // size of next data
	chatPayload.Write<uint32_t>(sender->GetID());
	chatPayload.Write<uint32_t>(0);
	chatPayload.Write<uint32_t>(2);

	BinaryWriter chatPackage;
	chatPackage.Write<uint32_t>(0xF7DE);
	chatPackage.Write<uint32_t>(chatPayload.GetSize() + 32);
	chatPackage.Write<uint32_t>(1); // type
	chatPackage.Write<uint32_t>(1);
	chatPackage.Write<uint32_t>(1);
	chatPackage.Write<uint32_t>(0xB0045);
	chatPackage.Write<uint32_t>(1);
	chatPackage.Write<uint32_t>(0xB0045);
	chatPackage.Write<uint32_t>(0);
	chatPackage.Write<uint32_t>(chatPayload.GetSize());
	chatPackage.Write(&chatPayload);

	for (auto &entry : m_mAllPlayers)
	{
		CPlayerWeenie *player = entry.second;

		if (player && !player->IsPlayerSquelched(sender->GetID(), true))
		{
			BOOL bShouldHear = FALSE;

			switch (channel_id)
			{
			case General_ChatChannel:
				bShouldHear = (player->GetCharacterOptions2() & HearGeneralChat_CharacterOptions2);
				break;
			case Trade_ChatChannel:
				bShouldHear = (player->GetCharacterOptions2() & HearTradeChat_CharacterOptions2);
				break;
			case LFG_ChatChannel:
				bShouldHear = (player->GetCharacterOptions2() & HearLFGChat_CharacterOptions2);
				break;
			case Roleplay_ChatChannel:
				bShouldHear = (player->GetCharacterOptions2() & HearRoleplayChat_CharacterOptions2);
				break;
			case Allegiance_ChatChannel:
				if (player->GetCharacterOptions() & HearAllegianceChat_CharacterOption)
				{
					if (sender_monarch_id && sender_monarch_id == player->InqIIDQuality(MONARCH_IID, 0))
						bShouldHear = TRUE;
					if (player->IsAllegGagged())
						bShouldHear = FALSE;
				}
				break;
			}

			if (bShouldHear)
			{
				player->SendNetMessage(&chatPackage, 4, FALSE, FALSE);
			}
		}
	}
}

std::string CWorld::GetServerStatus()
{
	std::string status;

	extern CPhatServer *g_pPhatServer;
	uint32_t runTime = (uint32_t)(g_pGlobals->Time() - g_pPhatServer->GetStartupTime());
	uint32_t runSeconds = runTime % 60;
	runTime /= 60;
	uint32_t runMinutes = runTime % 60;
	runTime /= 60;
	uint32_t runHours = runTime;

	status += csprintf("Server runtime: %uh %um %us\n", runHours, runMinutes, runSeconds);
	status += csprintf("%u MB used, %u / %u MB physical mem free.\n", (uint32_t)(GetProcessMemoryUsage() >> 20), (uint32_t)(GetFreeMemory() >> 20), (uint32_t)(GetTotalMemory() >> 20));

	{
		// check bucket health
		uint32_t biggestBucket = 0;
		for (uint32_t i = 0; i < m_mAllPlayers.bucket_count(); i++)
		{
			uint32_t bucketSize = (uint32_t)m_mAllPlayers.bucket_size(i);

			if (bucketSize >= biggestBucket)
				biggestBucket = bucketSize;
		}

		status += csprintf("%u players (%u buckets [%u in biggest].)\n", m_mAllPlayers.size(), m_mAllPlayers.bucket_count(), biggestBucket);
	}

	{
		std::map<uint64_t, CWorldLandBlock *> sortedBlocks;

		unsigned int activeBlocks = 0;
		unsigned int dormantPendingBlocks = 0;
		unsigned int dormantBlocks = 0;
		uint32_t totalActiveObjects = 0;

		// find the top 5 most active blocks
		for (auto block : m_vBlocks)
		{
			if (!block)
				continue;

			uint32_t liveCount = block->LiveCount();

			switch (block->GetDormancyStatus())
			{
			case LandblockDormancyStatus::DoNotGoDormant:
				activeBlocks++;
				totalActiveObjects += liveCount;
				break;
			case LandblockDormancyStatus::WaitToGoDormant:
				dormantPendingBlocks++;
				totalActiveObjects += liveCount;
				break;
			case LandblockDormancyStatus::Dormant:
				dormantBlocks++;
				break;
			}

			liveCount = (liveCount << 16) | block->PlayerCount();
			sortedBlocks[((0xFFFFFFFF - ((uint64_t)liveCount)) << 32) | (uint64_t)block->GetHeader()] = block;
		}

		{
			// check bucket health
			/*
			uint32_t biggestBucket = 0;
			for (uint32_t i = 0; i < m_mAllObjects.bucket_count(); i++)
			{
			uint32_t bucketSize = (uint32_t) m_mAllObjects.bucket_size(i);

			if (bucketSize >= biggestBucket)
			biggestBucket = bucketSize;
			}

			status += csprintf("%u objects (%u buckets [%u in biggest].)\n", m_mAllObjects.size(), m_mAllObjects.bucket_count(), biggestBucket);
			*/
			status += csprintf("%u active objects, %u total objects (%u buckets.)\n", totalActiveObjects, m_mAllObjects.size(), m_mAllObjects.bucket_count());
		}

		status += csprintf("%u total blocks loaded. %u active. %u pending dormancy. %u dormant. %u unloaded.\n",
			m_vBlocks.size() + m_vSpawns.size(), activeBlocks, dormantPendingBlocks, (uint32_t)m_mDormantBlocks.size(), (uint32_t)m_mUnloadedBlocks.size());

		unsigned int numBlocksToShow = 5;
		status += csprintf("Top %u most active blocks\n", numBlocksToShow);
		for (auto block : sortedBlocks)
		{
			uint32_t numObjects = (0xFFFFFFFF - (uint32_t)(block.first >> 32));
			uint32_t blockID = (block.first & 0xFFFFFFFF);

			status += csprintf("%04X - %u objects %u players\n", blockID, HIWORD(numObjects), LOWORD(numObjects));
			numBlocksToShow--;

			if (!numBlocksToShow)
				break;
		}
	}

	return status;
}

uint32_t CWorld::GenerateGUID(eGUIDClass type)
{
	uint32_t new_iid = g_pObjectIDGen->GenerateGUID(type);

	//while (FindObject(new_iid))
	//{
	//	new_iid = g_pObjectIDGen->GenerateGUID(type);
	//}

	return new_iid;
}

void CWorld::NotifyEventStarted(std::string eventName, GameEventDef *event)
{
	auto result = _eventWeenies.equal_range(eventName);

	for (auto it = result.first; it != result.second; it++)
	{
		if (CWeenieObject *weenie = FindObject(it->second))
		{
			weenie->CheckEventState(eventName, event);
		}
	}
}

void CWorld::NotifyEventStopped(std::string eventName, GameEventDef *event)
{
	auto result = _eventWeenies.equal_range(eventName);

	for (auto it = result.first; it != result.second; it++)
	{
		if (CWeenieObject *weenie = FindObject(it->second))
		{
			weenie->CheckEventState(eventName, event);
		}
	}
}

void CWorld::AddToUsedMergedItems(uint32_t item)
{
	_stackableOnGround.try_emplace(item, false);
}

bool CWorld::IsItemInUse(uint32_t item)
{
	std::map<uint32_t, bool>::const_iterator it = _stackableOnGround.find(item);
	if (it == _stackableOnGround.end())
		return false;
	if (it->second == true)
		return true;
	_stackableOnGround[item] = true;
	return false;
}

void CWorld::RemoveMergedItem(uint32_t item)
{
	_stackableOnGround.erase(item);
}


