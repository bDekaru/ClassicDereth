#include <StdAfx.h>
#include "World.h"
#include "WorldLandBlock.h"
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "LandDefs.h"
#include "EnvCell.h"
#include "LandCell.h"
#include "LandBlock.h"
#include "Database2.h"
#include "WeenieLandCoverage.h"
#include "WeenieFactory.h"
#include "ChatMsgs.h"
#include "MonsterAI.h"
#include "TownCrier.h"
#include "InferredCellData.h"
#include "InferredPortalData.h"
#include "WClassID.h"
#include "Config.h"
#include "DatabaseIO.h"
#include "Scene.h"

const double LANDBLOCK_DORMANCY_DELAY = 120.0;
const double LANDBLOCK_DORMANCY_TICK_INTERVAL = 10.0;

CWorldLandBlock::CWorldLandBlock(CWorld *pWorld, WORD wHeader) : m_pWorld(pWorld), m_wHeader(wHeader)
{
	m_fNextDormancyCheck = Timer::cur_time + LANDBLOCK_DORMANCY_TICK_INTERVAL;
}

CWorldLandBlock::~CWorldLandBlock()
{
	ClearOldDatabaseEntries();

	for (auto &entity : m_EntitiesToAdd)
	{
		if (entity && entity->GetBlock() == this)
			Destroy(entity, false);
	}

	for (auto &entity : m_EntityList)
	{
		if (entity && entity->GetBlock() == this)
			Destroy(entity, false);
	}

	m_PlayerMap.clear();
	m_PlayerList.clear();

	m_EntityMap.clear();
	m_EntitiesToAdd.clear();
	m_EntityList.clear();

	if (m_LoadedLandBlock)
	{
		m_LoadedLandBlock->release_all();
		CLandBlock::Release(m_LoadedLandBlock);
		m_LoadedLandBlock = NULL;
	}

	for (auto &entry : m_LoadedEnvCells)
		CEnvCell::Release(entry.second);

	m_LoadedEnvCells.clear();
}

void CWorldLandBlock::ClearOldDatabaseEntries()
{
	//We wait to clear the database entries until the landblock is being unloaded just in case.
	//If we crash before that all corpses will be rolled back with the other data so nothing is lost.
	//In case a player loots the items and his character is saved before the crash, in theory the corpse should be empty as the items themselves
	//will have a different parent container.
	std::list<unsigned int> weeniesList = g_pDBIO->GetWeeniesAt(m_wHeader);
	for (auto entry : weeniesList)
	{
		try
		{
			bool stillExists = false;
			for (auto &entity : m_EntityList)
			{
				try
				{
					if (entity && (entity->GetID() == entry || entity->ShouldSave()))
					{
						stillExists = true;
						break;
					}
				}
				catch (...)
				{
					SERVER_ERROR << "Error getting ID for " << entity;
				}
			}

			if (!stillExists)
			{
				g_pDBIO->RemoveWeenieFromBlock(entry);
				g_pDBIO->DeleteWeenie(entry);
			}
		}
		catch (...)
		{
			SERVER_ERROR << "Failed to get data for " << entry;
		}
	}
}

void CWorldLandBlock::Init()
{
	m_noDrop = g_pCellDataEx->BlockHasFlags(m_wHeader << 16, LandBlockFlags::NoDrop);
	{
		// CStopWatch stopWatch;
		LoadLandBlock();
		// double elapsed = stopWatch.GetElapsed();

		// if (elapsed >= 0.01)
		// {
		// LOG_PRIVATE(Temp, Warning, "Took %.3f seconds to load %04X\n", elapsed, m_wHeader);
		// }
	}

	m_bSpawnOnNextTick = true;
}

void CWorldLandBlock::SpawnDynamics()
{
	if (g_pConfig->SpawnLandscape() && m_LoadedLandBlock)
	{
		uint32_t block_x = (m_wHeader & 0xFF00) >> 8;
		uint32_t block_y = (m_wHeader & 0x00FF) >> 0;

		for (uint32_t cell_x = 0; cell_x < 9; cell_x++)
		{
			for (uint32_t cell_y = 0; cell_y < 9; cell_y++)
			{
				uint32_t landcell = (m_wHeader << 16) | (cell_x << 3) | cell_y;

				WORD terrain = m_LoadedLandBlock->terrain[(cell_x * 9) + cell_y];
				// scene type / terrain type
				WORD stIdx = (terrain >> 7) >> 4;
				WORD ttIdx = ((terrain & 0x7f) >> 2) & 31;

				CTerrainType *tt = CachedRegionDesc->terrain_info->terrain_types.array_data[ttIdx];
				CSceneType *st = tt->scene_types.array_data[stIdx];

				if (!st || st->scenes.num_used == 0)
					continue;

				uint32_t sx = (block_x << 3) + cell_x;
				uint32_t sy = (block_y << 3) + cell_y;

				uint32_t sceneIdx = 0;
				int sceneCount = st->scenes.num_used;
				if (sceneCount > 1)
				{
					sceneIdx = sy * (712977289 * sx + 1813693831) - 1109124029 * sx + 2139937281;
					sceneIdx = (uint32_t)floor((double)sceneIdx * 2.3283064e-10 * (double)sceneCount);
					if (sceneIdx >= sceneCount)
						sceneIdx = 0;
				}

				Scene *scene = Scene::Get(st->scenes.array_data[sceneIdx]);

				// reused rand calcs
				uint32_t rx = sx * -1109124029;
				uint32_t ry = sy * 1813693831;
				uint32_t rxy = sx * sy * 1360117743 + 1888038839;
				uint32_t rxy2 = rxy * 23399;

				for (int soi = 0; soi < scene->num_objects; soi++, rxy2 += rxy)
				{
					ObjectDesc &od = scene->objects[soi];

					double r = (double)(rx + ry - rxy2) * 2.3283064e-10;
					if (r < od.freq)
					{
						Position pos;
						Vector v;

						od.Place(sx, sy, soi, &v);
						v.x += (double)cell_x * 24.0;
						v.y += (double)cell_y * 24.0;

						// in block
						if (!(v.x >= 0 && v.y >= 0 && v.x < 192 && v.y < 192))
							continue;

						// on road

						pos.objcell_id = m_LoadedLandBlock->id;
						pos.frame.m_origin = v;
						//pos.objcell_id = ((uint32_t)m_wHeader << 16) | 1;
						//pos.frame.m_origin = Vector(x_shift, y_shift, 0.0f);
						pos.adjust_to_outside();

						CLandCell *cell = m_LoadedLandBlock->get_landcell(pos.objcell_id);

						// building?
						if (cell->has_building())
							continue;

						CPolygon *walkable;
						if (!cell->find_terrain_poly(v, &walkable))
							continue;

						// on hill
						if (!od.CheckSlope(walkable->plane.m_normal.z))
							continue;

						walkable->plane.set_height(v);

						if (od.align)
							od.ObjAlign(&walkable->plane, &v, &pos.frame);
						else
							od.GetObjFrame(sx, sy, soi, &v, &pos.frame);

						uint32_t wcid = od.obj_id;
						if (!wcid)
						{
							uint16_t encounterIndex = g_pCellDataEx->GetEncounterIndex(m_wHeader, cell_x, cell_y);
							wcid = g_pPortalDataEx->GetWCIDForTerrain(block_x, block_y, encounterIndex);
						}

						if (wcid == W_UNDEF_CLASS || wcid == W_HUMAN_CLASS || wcid == W_ADMIN_CLASS || wcid == W_SENTINEL_CLASS)
							continue;
						CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, &pos, false);
						if (weenie)
						{
							weenie->SetID(g_pObjectIDGen->GenerateGUID(eEphemeral));
							g_pWorld->CreateEntity(weenie, false);
						}
						else if(od.obj_id)
						{
							// trees/rocks
							CPhysicsObj * obj = CPhysicsObj::makeObject(od.obj_id, 0, FALSE);
							obj->add_obj_to_cell(cell, &pos.frame);
						}

					}
				}
			}
		}
	}

	CLandBlockExtendedData *data = g_pCellDataEx->GetLandBlockData((uint32_t)m_wHeader << 16);
	if (data)
	{
		SmartArray<CLandBlockWeenieLink> tmp_links = data->weenie_links;
		uint32_t id_mask = 0x70000000 | (m_wHeader << 12);

		for (uint32_t i = 0; i < data->weenies.num_used; i++)
		{
			uint32_t wcid = data->weenies.array_data[i].wcid;
			Position pos = data->weenies.array_data[i].pos;
			uint32_t iid = data->weenies.array_data[i].iid;

			if ((pos.objcell_id & 0xFFFF) >= 0x100)
			{
			}
			else
			{
				pos.objcell_id &= 0xFFFF0000;
				pos.objcell_id |= 1;
				pos.adjust_to_outside();
			}

			if (wcid == W_FIREWORKSGEN_CLASS) // no statue fireworks
				continue;
			if (wcid == W_PORTALDESTINATION_CLASS) // dynamic, should probably hide dynamics?
				continue;
			//if (wcid == W_PORTALHOLTBURGSOUTHOUTPOSTTUTORIAL_CLASS)
			//	continue;
			//if (wcid == W_PORTALHOLTBURGWESTOUTPOSTTUTORIAL_CLASS)
			//	continue;
			//if (wcid == W_PORTALYARAQNORTHOUTPOSTTUTORIAL_CLASS)
			//	continue;
			//if (wcid == W_PORTALYARAQEASTOUTPOSTTUTORIAL_CLASS)
			//	continue;
			//if (wcid == W_PORTALSHOUSHIWESTOUTPOSTTUTORIAL_CLASS)
			//	continue;
			//if (wcid == W_PORTALSHOUSHISOUTHEASTOUTPOSTTUTORIAL_CLASS)
			//	continue;

			//fixing the location of a few portals without editing the cache files.
			switch (iid)
			{
			case 0x7A9B4069: //Holtburg->Shoushi portal: move it to the bunker
				pos = Position(0xA9B40180, Vector(55.484226f, 54.375134f, 71.660004f), Quaternion(0.712710f, 0.000000f, 0.000000f, 0.701458f));
				break;
			case 0x7A9B4068: //Holtburg->Rithwic portal: move it to the bunker
				pos = Position(0xA9B40185, Vector(107.945694f, 64.362610f, 73.660004f), Quaternion(1.000000f, 0.000000f, 0.000000f, 0.000000f));
				break;
			case 0x7AAB4009: //Holtburg->Arwic portal: move it to the bunker
				pos = Position(0xAAB40105, Vector(33.671177f, 131.133408f, 51.660004f), Quaternion(-0.699012f, 0.000000f, 0.000000f, 0.715110f));
				break;
			case 0x7AAB4008: //Holtburg->Dryreach portal: move it to the bunker
				pos = Position(0xAAB40100, Vector(45.7923012f, 18.8299007f, 71.6600037f), Quaternion(-0.253717005f, 0.000000f, 0.000000f, 0.967278004f));
				break;
			case 0x77D64058: //Yaraq->Xarabydum portal: move it to the bunker
				pos = Position(0x7D640183, Vector(132.047836f, 55.703876f, 6.660000f), Quaternion(0.000000f, 0.000000f, 0.000000f, -1.000000f));
				break;
			case 0x77E64010: //Yaraq->Linvak Tukal portal: move it to the bunker
				pos = Position(0x7E640129, Vector(127.572594f, 11.987453f, 5.660000f), Quaternion(-0.711385f, 0.000000f, 0.000000f, -0.702802f));
				break;
			case 0x77D64057: //Yaraq->Khayyaban portal: move it to the bunker
				pos = Position(0x7E640124, Vector(16.581305f, 60.043930f, 9.660000f), Quaternion(0.706641f, 0.000000f, 0.000000f, -0.707572f));
				break;
			}

			CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(wcid);
			if (!weenieDefs)
				continue;

			if (!g_pConfig->SpawnStaticCreatures())
			{
				if (weenieDefs->m_Qualities.m_WeenieType == Creature_WeenieType
					|| weenieDefs->m_Qualities.m_WeenieType == Vendor_WeenieType)
				{
					continue;
				}
			}

			CWeenieObject *weenie = nullptr;

			if ((weenieDefs->m_Qualities.m_WeenieType == WeenieType::House_WeenieType ||
					weenieDefs->m_Qualities.m_WeenieType == WeenieType::Hook_WeenieType ||
					weenieDefs->m_Qualities.m_WeenieType == WeenieType::Storage_WeenieType)
				&& g_pDBIO->IsWeenieInDatabase(iid))
			{
				weenie = CWeenieObject::Load(iid);
				weenie->m_Qualities.SetPosition(INSTANTIATION_POSITION, pos);
				weenie->m_Qualities.SetPosition(LOCATION_POSITION, pos);
			}
			else
			{
				weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, &pos, false);
			}

			if (weenie)
			{
				bool creature = weenie->IsCreature();
				bool npc = weenie->InqIntQuality(PLAYER_KILLER_STATUS_INT, 0) & RubberGlue_PKStatus;
				bool stuck = weenie->IsStuck();

				// things that are stuck or an npc, but not another creature type
				// will be assigned a landblock-specific id
				if (stuck && !(creature && !npc))
				{
					weenie->SetID(id_mask | (iid & 0x00000FFF));
				}

				// CreateEntity will determine if the item needs a dynamic or ephemeral id
				g_pWorld->CreateEntity(weenie);

				uint32_t weenie_id = weenie->GetID();
				for (uint32_t i = 0; i < tmp_links.num_used; i++)
				{
					if (tmp_links.array_data[i].source == iid)
						tmp_links.array_data[i].source = weenie_id;
					if (tmp_links.array_data[i].target == iid)
						tmp_links.array_data[i].target = weenie_id;
				}
			}
		}

		for (uint32_t i = 0; i < tmp_links.num_used; i++)
		{
			uint32_t source_id = tmp_links.array_data[i].source;
			uint32_t target_id = tmp_links.array_data[i].target;

			CWeenieObject *source_weenie = g_pWorld->FindObject(source_id); // often creature, or respawnable item
			if (source_weenie)
			{
				if (!source_weenie->cell)
				{
					//LOG_PRIVATE(World, Warning, "Trying to spawn a monster in an invalid position! Deleting instead.\n", source_id);
					g_pWorld->RemoveEntity(source_weenie);
					continue;
				}

				CWeenieObject *target_weenie = g_pWorld->FindObject(target_id); // often generator
				if (target_weenie)
				{
					target_weenie->EnsureLink(source_weenie);
				}
			}
		}
	}
}

void CWorldLandBlock::LoadLandBlock()
{
	uint32_t landBlock = (uint32_t)(m_wHeader << 16) | 0xFFFF;
	m_LoadedLandBlock = CLandBlock::Get(landBlock);

	if (m_LoadedLandBlock)
	{
		int32_t LCoordX, LCoordY;
		LandDefs::blockid_to_lcoord(landBlock, LCoordX, LCoordY);
		m_LoadedLandBlock->block_coord.x = LCoordX;
		m_LoadedLandBlock->block_coord.y = LCoordY;

		/*
		int32_t Magic1, Magic2;
		get_block_orient(x, y, Magic1, Magic2);

		if ((pLB->side_cell_count == LandDefs::lblock_side) &&
		(pLB->side_cell_count != (LandDefs::lblock_side / Magic1)))
		pLB->notify_change_size();
		*/

		if (m_LoadedLandBlock->generate(landBlock, 1, LandDefs::Direction::IN_VIEWER_BLOCK))
		{
			m_LoadedLandBlock->closest.x = -1;
			m_LoadedLandBlock->closest.y = -1;

			m_LoadedLandBlock->init_lcell_ptrs();
			//m_LoadedLandBlock->calc_lighting();
			m_LoadedLandBlock->get_land_limits();
		}

		if (m_LoadedLandBlock->lbi)
		{
			for (uint32_t i = 0; i < m_LoadedLandBlock->lbi->num_cells; i++)
			{
				if (CEnvCell *cell = (CEnvCell *)GetObjCell((uint32_t)(m_wHeader << 16) | (0x100 + i), false))
				{
					if (cell->seen_outside)
					{
						_cached_any_seen_outside = true;
					}
				}
			}
		}

		if (m_LoadedLandBlock->needs_post_load)
		{
			m_LoadedLandBlock->init_buildings();
			m_LoadedLandBlock->init_static_objs(0);
			m_LoadedLandBlock->needs_post_load = false;
		}
	}
}

WORD CWorldLandBlock::GetHeader()
{
	return m_wHeader;
}

void CWorldLandBlock::MakeNotDormant()
{
	m_DormancyStatus = DoNotGoDormant;
	m_fNextDormancyCheck = Timer::cur_time + LANDBLOCK_DORMANCY_TICK_INTERVAL;
	m_pWorld->EnsureBlockIsTicking(this);
}

void CWorldLandBlock::Insert(CWeenieObject *pEntity, WORD wOld, BOOL bNew, bool bMakeAware)
{
	using player_map_pib = std::pair<PlayerWeenieMap::iterator, bool>;
	using weenie_map_pib = std::pair<WeenieMap::iterator, bool>;

	if (CPlayerWeenie *player = pEntity->AsPlayer())
	{
		Position pos = pEntity->GetPosition();

		player_map_pib pib =
			m_PlayerMap.insert(std::pair<uint32_t, CPlayerWeenie *>(pEntity->GetID(), player));

		if (pib.second)
		{
			m_PlayerList.push_back(player);

			MakeNotDormant();

			// spawn up adjacent landblocks only if outdoors, otherwise only load the block you're on.
			if ((pos.objcell_id & 0xFFFF) < 0x100) //outdoors
				ActivateLandblocksWithinPVS(pEntity->GetLandcell());
		}
	}

	weenie_map_pib pib =
		m_EntityMap.insert(std::pair<uint32_t, CWeenieObject *>(pEntity->GetID(), pEntity));
	if (pib.second)
	{
		m_EntitiesToAdd.push_back(pEntity);

		pEntity->Attach(this);

		if (bNew)
			pEntity->RemovePreviousInstance();

		if (bMakeAware)
			ExchangePVS(pEntity, wOld);
	}
}

CWeenieObject *CWorldLandBlock::FindEntity(uint32_t dwGUID)
{
	WeenieMap::iterator eit = m_EntityMap.find(dwGUID);

	if (eit == m_EntityMap.end())
		return NULL;

	return eit->second;
}

CPlayerWeenie *CWorldLandBlock::FindPlayer(uint32_t dwGUID)
{
	PlayerWeenieMap::iterator pit = m_PlayerMap.find(dwGUID);

	if (pit == m_PlayerMap.end())
		return NULL;

	return pit->second;
}

/*

ExchangeData --
Makes all entities mutually aware of the source.

*/
void CWorldLandBlock::ExchangeData(CWeenieObject *source)
{
	if (source->AsPlayer() && m_DormancyStatus != DoNotGoDormant)
	{
		MakeNotDormant();
	}

	for (auto &entity : m_EntitiesToAdd)
	{
		if (entity)
		{
			if (entity != source)
			{
				entity->MakeAware(source);
				source->MakeAware(entity);
			}
		}
	}

	for (auto &entity : m_EntityList)
	{
		if (entity)
		{
			if (entity != source)
			{
				entity->MakeAware(source);
				source->MakeAware(entity);
			}
		}
	}
}

void CWorldLandBlock::ExchangeDataForStabChange(CWeenieObject *pSource, uint32_t old_cell_id, uint32_t new_cell_id)
{
	CObjCell *old_cell, *new_cell;

	old_cell = (BLOCK_WORD(old_cell_id) == m_wHeader) ? GetObjCell(old_cell_id & 0xFFFF) : NULL;
	new_cell = (BLOCK_WORD(new_cell_id) == m_wHeader) ? GetObjCell(new_cell_id & 0xFFFF) : NULL;

	if (new_cell)
	{
		for (uint32_t i = 0; i < new_cell->num_stabs; i++)
		{
			uint32_t cell_id_check = new_cell->stab_list[i];

			if (old_cell_id == cell_id_check)
				continue;

			bool already_visible = false;

			if (old_cell)
			{
				for (uint32_t j = 0; j < old_cell->num_stabs; j++)
				{
					if (old_cell->stab_list[j] == cell_id_check)
					{
						already_visible = true; // was previously visible already
						break;
					}
				}
			}

			if (!already_visible)
			{
				ExchangeDataForCellID(pSource, cell_id_check);
			}
		}
	}
}

void CWorldLandBlock::ExchangeDataForCellID(CWeenieObject *source, uint32_t cell_id)
{
	for (auto &entity : m_EntitiesToAdd)
	{
		if (entity && entity->GetLandcell() == cell_id)
		{
			if (entity != source)
			{
				entity->MakeAware(source);
				source->MakeAware(entity);
			}
		}
	}

	for (auto &entity : m_EntityList)
	{
		if (entity && entity->GetLandcell() == cell_id)
		{
			if (entity != source)
			{
				entity->MakeAware(source);
				source->MakeAware(entity);
			}
		}
	}
}

void CWorldLandBlock::ExchangePVS(CWeenieObject *pSource, WORD old_block_id)
{
	if (!pSource)
		return;

	WORD cell = CELL_WORD(pSource->GetLandcell());

	CWorldLandBlock *pBlock = pSource->GetBlock();

	//if in a Dungeon only exchange data on this landblock. Indoor landblocks, such as inside buildings should still exchange.
	if ((pBlock && (pSource->GetLandcell() & 0xFFFF) > 0x100) && !PossiblyVisibleToOutdoors(pSource->GetLandcell()))
	{
		pBlock->ExchangeData(pSource);
	}
	else
	{
		// outdoor exchange -- this should eventually become obselete

		BYTE xold, xstart, xend;
		BYTE yold, ystart, yend;

		xold = (old_block_id >> 8);
		xstart = (m_wHeader >> 8) - 1; if (xstart == (BYTE)-1) xstart = 0;
		xend = (m_wHeader >> 8) + 1; if (xend == (BYTE)0) xend = 0xFF;
		yold = (old_block_id & 0xFF);
		ystart = (m_wHeader & 0xFF) - 1; if (ystart == (BYTE)-1) ystart = 0;
		yend = (m_wHeader & 0xFF) + 1; if (yend == (BYTE)0) yend = 0xFF;

		for (WORD xit = xstart; xit <= xend; xit++)
		{
			for (WORD yit = ystart; yit <= yend; yit++)
			{
				if (old_block_id)
				{
					if ((xold >= (xit - 1)) && (xold <= (xit + 1)))
					{
						if ((yold >= (yit - 1)) && (yold <= (yit + 1)))
							continue;
					}
				}

				WORD wHeader = ((WORD)xit << 8) | yit;
				CWorldLandBlock *pBlock = m_pWorld->GetLandblock(wHeader);

				if (pBlock)
					pBlock->ExchangeData(pSource);
			}
		}
	}
}

void CWorldLandBlock::Broadcast(void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral)
{
	for (PlayerWeenieVector::iterator pit = m_PlayerList.begin(); pit != m_PlayerList.end();)
	{
		CPlayerWeenie *pPlayer = (*pit);

		if (!pPlayer)
		{
			pit = m_PlayerList.erase(pit);
			continue;
		}

		if (!ignore_ent || (ignore_ent != pPlayer->GetID()))
		{
			pPlayer->SendNetMessage(_data, _len, _group, _game_event, ephemeral);
		}

		pit++;
	}
}

bool CWorldLandBlock::CanGoDormant()
{
	return m_DormancyStatus != LandblockDormancyStatus::NeverDormant && !HasPlayers();
}

bool CWorldLandBlock::HasPlayers()
{
	return !m_PlayerList.empty();
}

bool CWorldLandBlock::IsWaterBlock()
{
	return (m_LoadedLandBlock && m_LoadedLandBlock->water_type == 2);
}

bool CWorldLandBlock::HasAnySeenOutside()
{
	return _cached_any_seen_outside;
}

bool CWorldLandBlock::PossiblyVisibleToOutdoors(WORD cell_id)
{
	if (cell_id >= LandDefs::first_envcell_id && cell_id <= LandDefs::last_envcell_id)
	{
		if (!HasAnySeenOutside())
		{
			return false;
		}

		auto entry = m_LoadedEnvCells.find(cell_id);

		if (entry != m_LoadedEnvCells.end())
		{
			CEnvCell *pEnvCell = entry->second;

			return pEnvCell->seen_outside ? true : false;
		}
	}

	return true;
}

void CWorldLandBlock::Release(CWeenieObject *pEntity)
{
	if (pEntity->GetBlock() == this)
	{
		pEntity->Detach();
	}

	if (pEntity->AsPlayer())
	{
		m_PlayerMap.erase(pEntity->GetID());

		PlayerWeenieVector::iterator pit = m_PlayerList.begin();
		PlayerWeenieVector::iterator pend = m_PlayerList.end();

		while (pit != pend)
		{
			if (pEntity == (*pit))
			{
				if (!m_bThinking)
				{
					pit = m_PlayerList.erase(pit);
					pend = m_PlayerList.end();
					continue;
				}
				else
				{
					// Set entry to NULL -- it will be removed during next iteration
					*pit = NULL;
				}
			}

			pit++;
		}
	}

	m_EntityMap.erase(pEntity->GetID());

	WeenieVector::iterator eit = m_EntitiesToAdd.begin();
	WeenieVector::iterator eend = m_EntitiesToAdd.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
		{
			if (!m_bThinking)
			{
				eit = m_EntitiesToAdd.erase(eit);
				eend = m_EntitiesToAdd.end();
				continue;
			}
			else
			{
				// Set entry to NULL -- it will be removed during next iteration				
				*eit = NULL;
			}
		}

		eit++;
	}

	eit = m_EntityList.begin();
	eend = m_EntityList.end();

	while (eit != eend)
	{
		if (pEntity == (*eit))
		{
			if (!m_bThinking)
			{
				eit = m_EntityList.erase(eit);
				eend = m_EntityList.end();
				continue;
			}
			else
			{
				// Set entry to NULL -- it will be removed during next iteration
				*eit = NULL;
			}
		}

		eit++;
	}
}

void CWorldLandBlock::Destroy(CWeenieObject *pEntity, bool bDoRelease)
{
	if (pEntity->ShouldSave())
	{
		pEntity->Save();
	}

	if (bDoRelease)
	{
		Release(pEntity);
	}

	g_pWorld->EnsureRemoved(pEntity);

#if FALSE
	uint32_t DestroyObject[2];
	DestroyObject[0] = 0x0024;
	DestroyObject[1] = pEntity->GetID();

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), DestroyObject, sizeof(DestroyObject), OBJECT_MSG, 0);
#else

	uint32_t RemoveObject[3];
	RemoveObject[0] = 0xF747;
	RemoveObject[1] = pEntity->GetID();
	RemoveObject[2] = pEntity->_instance_timestamp;

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), RemoveObject, sizeof(RemoveObject));

	// LOG(Temp, Normal, "Removing entity %08X %04X @ %08X \n", pEntity->GetID(), pEntity->_instance_timestamp, pEntity->GetLandcell());
#endif

	pEntity->NotifyRemoveFromWorld();

	pEntity->exit_world();
	pEntity->leave_world();
	pEntity->unset_parent();
	pEntity->unparent_children();

	m_pWorld->EnsureRemoved(pEntity);

	if (uint32_t generator_id = pEntity->InqIIDQuality(GENERATOR_IID, 0))
	{
		CWeenieObject *target = g_pWorld->FindObject(generator_id);
		if (target)
			target->NotifyGeneratedDeath(pEntity);
	}

	if (pEntity)
		delete pEntity;
}

BOOL CWorldLandBlock::Think()
{
	scope_lock();

	if (m_DormancyStatus == LandblockDormancyStatus::Dormant)
	{
		return FALSE;
	}

	if (m_bSpawnOnNextTick)
	{
		// g_pGameDatabase->SpawnStaticsForLandBlock(m_wHeader);

		{
			// CStopWatch stopWatch;
			SpawnDynamics();
			// double elapsed = stopWatch.GetElapsed();

			// if (elapsed >= 0.01)
			// {
			// 	LOG(Temp, Warning, "Took %.3f seconds to spawn %04X\n", elapsed, m_wHeader);
			// }
		}

		std::list<unsigned int> weeniesList = g_pDBIO->GetWeeniesAt(m_wHeader);

		for (auto entry : weeniesList)
		{
			CWeenieObject *weenie = CWeenieObject::Load(entry);

			if (weenie)
				g_pWorld->CreateEntity(weenie);
		}

		m_bSpawnOnNextTick = false;
	}

	m_bThinking = TRUE;

	WeenieVector::iterator eit = m_EntityList.begin();
	WeenieVector::iterator eend = m_EntityList.end();

	while (eit != eend)
	{
		CWeenieObject *pEntity = (*eit);

		if (!pEntity || pEntity->GetBlock() != this)
		{
			// remove NULL entries -- happens if entities are deleted			
			eit = m_EntityList.erase(eit);
			eend = m_EntityList.end();
			continue;
	}

		// This entity *is* under our control

		uint32_t cell_id = pEntity->GetLandcell();
		WORD wHeader = BLOCK_WORD(cell_id);
		if (wHeader == m_wHeader && !pEntity->CachedHasOwner())
		{
			if (pEntity->last_tick_cell_id != cell_id)
			{
				ExchangeDataForStabChange(pEntity, pEntity->last_tick_cell_id, cell_id);
				pEntity->last_tick_cell_id = cell_id;
			}

			pEntity->last_tick_parent = pEntity->parent;

			if (!pEntity->ShouldDestroy())
			{
				// ...why?
#ifdef DEBUG
				static bool checkMe = true;
				if (checkMe)
				{
					assert(pEntity->cell);
					checkMe = false;
				}
#endif

				if (!pEntity->cell)
				{
					if (CPlayerWeenie *player = pEntity->AsPlayer())
					{
						if (player->_nextTryFixBrokenPosition < Timer::cur_time)
						{
							player->_nextTryFixBrokenPosition = Timer::cur_time + 2.0;

							if (player->m_LastValidPosition.objcell_id != 0)
								player->Movement_Teleport(player->m_LastValidPosition);
							else
								//player->Movement_Teleport(Position(0xA9B4001F, Vector(87.750603f, 147.722321f, 66.005005f), Quaternion(0.011819f, 0.000000, 0.000000, -0.999930f)), false);
								player->TeleportToLifestone();
						}
					}
				}

				pEntity->update_object();
				pEntity->Tick();

				uint32_t cell_id = pEntity->GetLandcell();
				WORD wHeader = BLOCK_WORD(cell_id);
				if (wHeader != m_wHeader)
				{
					break;
				}
			}
			else
			{
				Destroy(pEntity);
			}

			eit++;
			continue;
		}

		// The entity should shift control.
		if (pEntity->AsPlayer())
		{
			m_PlayerMap.erase(pEntity->GetID());

			PlayerWeenieVector::iterator pit = m_PlayerList.begin();
			PlayerWeenieVector::iterator pend = m_PlayerList.end();
			while (pit != pend)
			{
				if (pEntity == (*pit))
				{
					pit = m_PlayerList.erase(pit);
					pend = m_PlayerList.end();
				}
				else
					pit++;
			}
		}

		if (pEntity->GetBlock() == this)
			pEntity->Detach();

		m_EntityMap.erase(pEntity->GetID());
		eit = m_EntityList.erase(eit);
		eend = m_EntityList.end();

		m_pWorld->JuggleEntity(m_wHeader, pEntity);
}

	m_bThinking = FALSE;

	std::copy(m_EntitiesToAdd.begin(), m_EntitiesToAdd.end(), std::back_inserter(m_EntityList));
	m_EntitiesToAdd.clear();

	if (m_fNextDormancyCheck <= Timer::cur_time)
	{
		m_fNextDormancyCheck = Timer::cur_time + LANDBLOCK_DORMANCY_TICK_INTERVAL;

		switch (m_DormancyStatus)
		{
		case LandblockDormancyStatus::DoNotGoDormant:
			if (CanGoDormant())
			{
				if (!PlayerWithinPVS())
				{
					m_fTimeToGoDormant = Timer::cur_time + LANDBLOCK_DORMANCY_DELAY;
					m_DormancyStatus = LandblockDormancyStatus::WaitToGoDormant;
				}
			}
			break;

		case LandblockDormancyStatus::WaitToGoDormant:
			if (!CanGoDormant() || PlayerWithinPVS())
			{
				m_DormancyStatus = LandblockDormancyStatus::DoNotGoDormant;
			}
			else if (m_fTimeToGoDormant <= Timer::cur_time)
			{
				m_DormancyStatus = LandblockDormancyStatus::Dormant;
			}

			break;
		}
	}

	// this should be changed to be tied out
	//if (m_EntityList.empty())
	//{
	// world needs to do something with this; remove me probably
	//	return FALSE;
	//}

	return TRUE;
}

void CWorldLandBlock::ActivateLandblocksWithinPVS(uint32_t cell_id)
{
	if (IsWaterBlock())
	{
		return;
	}

	BYTE xstart, xend;
	BYTE ystart, yend;

	xstart = (m_wHeader >> 8) - 1; if (xstart == (BYTE)-1) xstart = 0;
	xend = (m_wHeader >> 8) + 1; if (xend == (BYTE)0) xend = 0xFF;
	ystart = (m_wHeader & 0xFF) - 1; if (ystart == (BYTE)-1) ystart = 0;
	yend = (m_wHeader & 0xFF) + 1; if (yend == (BYTE)0) yend = 0xFF;

	for (WORD xit = xstart; xit <= xend; xit++)
	{
		for (WORD yit = ystart; yit <= yend; yit++)
		{
			WORD wHeader = ((WORD)xit << 8) | yit;
			if (wHeader == m_wHeader)
				continue;

			m_pWorld->GetLandblock(wHeader, true);
		}
	}
}

bool CWorldLandBlock::PlayerWithinPVS()
{
	if (!IsWaterBlock())
	{
		BYTE xstart, xend;
		BYTE ystart, yend;

		xstart = (m_wHeader >> 8) - 1; if (xstart == (BYTE)-1) xstart = 0;
		xend = (m_wHeader >> 8) + 1; if (xend == (BYTE)0) xend = 0xFF;
		ystart = (m_wHeader & 0xFF) - 1; if (ystart == (BYTE)-1) ystart = 0;
		yend = (m_wHeader & 0xFF) + 1; if (yend == (BYTE)0) yend = 0xFF;

		for (WORD xit = xstart; xit <= xend; xit++)
		{
			for (WORD yit = ystart; yit <= yend; yit++)
			{
				WORD wHeader = ((WORD)xit << 8) | yit;
				CWorldLandBlock *pBlock = m_pWorld->GetLandblock(wHeader);

				if (pBlock && pBlock->HasPlayers())
					return true;
			}
		}
	}
	else
	{
		if (HasPlayers())
			return true;
	}

	return false;
}

void CWorldLandBlock::ClearSpawns(bool forced)
{
	scope_lock();

	WeenieVector::iterator eit = m_EntityList.begin();
	WeenieVector::iterator eend = m_EntityList.end();

	CWeenieObject *pEntity;

	while (eit != eend)
	{
		pEntity = *eit;

		if (pEntity && !pEntity->HasOwner() && !pEntity->AsPlayer())
		{
			if (forced || !pEntity->m_bDontClear)
			{
				if (pEntity)
				{
					Destroy(pEntity);

					eit = m_EntityList.begin();
					eend = m_EntityList.end();
				}
				else
				{
					eit = m_EntityList.erase(eit);
					eend = m_EntityList.end();
				}
			}
			else
				eit++;
		}
		else
		{
			eit++;
		}
	}

	// clear the awareness of the players
	for (auto player : m_PlayerList)
		if (player)
			player->FlushMadeAwareof(true);
}

void CWorldLandBlock::EnumNearbyFastNoSphere(const Position &pos, float range, std::list<CWeenieObject *> *results)
{
	float range_squared = range * range;

	for (WeenieMap::iterator i = m_EntityMap.begin(); i != m_EntityMap.end(); i++)
	{
		CWeenieObject *other = i->second;

		if (pos.distance_squared(other->m_Position) <= range_squared)
		{
			results->push_back(other);
		}
	}
}

void CWorldLandBlock::EnumNearby(const Position &pos, float range, std::list<CWeenieObject *> *results)
{
	for (auto pOther : m_EntityList)
	{
		if (!pOther)
			continue;

		float other_radius_plus_range = range + pOther->GetRadius();

		// if ((pos.distance(pOther->m_Position) - pOther->GetRadius()) <= fRange)
		if (pos.distance_squared(pOther->m_Position) <= (other_radius_plus_range*other_radius_plus_range))
		{
			results->push_back(pOther);
		}
	}

	for (auto pOther : m_EntitiesToAdd)
	{
		if (!pOther)
			continue;

		float other_radius_plus_range = range + pOther->GetRadius();

		// if ((pos.distance(pOther->m_Position) - pOther->GetRadius()) <= fRange)
		if (pos.distance_squared(pOther->m_Position) <= (other_radius_plus_range*other_radius_plus_range))
		{
			results->push_back(pOther);
		}
	}
}

void CWorldLandBlock::EnumNearby(CWeenieObject *source, float range, std::list<CWeenieObject *> *results)
{
	float selfRadius = source->GetRadius();

	for (auto pOther : m_EntityList)
	{
		if (!pOther || source == pOther)
			continue;

		float self_radius_plus_other_radius_plus_range = range + selfRadius + pOther->GetRadius();
		if (source->DistanceSquared(pOther) <= (self_radius_plus_other_radius_plus_range*self_radius_plus_other_radius_plus_range))
		{
			results->push_back(pOther);
		}
	}

	for (auto pOther : m_EntitiesToAdd)
	{
		if (!pOther || source == pOther)
			continue;

		float self_radius_plus_other_radius_plus_range = range + selfRadius + pOther->GetRadius();
		if (source->DistanceSquared(pOther) <= (self_radius_plus_other_radius_plus_range*self_radius_plus_other_radius_plus_range))
		{
			results->push_back(pOther);
		}
	}
}

void CWorldLandBlock::EnumNearbyPlayers(const Position &pos, float range, std::list<CWeenieObject *> *results)
{
	for (auto player : m_PlayerList)
	{
		if (!player)
			continue;

		float other_radius_plus_range = range + player->GetRadius();

		// if ((pos.distance(pOther->m_Position) - pOther->GetRadius()) <= fRange)
		if (pos.distance_squared(player->m_Position) <= (other_radius_plus_range*other_radius_plus_range))
		{
			results->push_back(player);
		}
	}
}

void CWorldLandBlock::EnumNearbyPlayers(CWeenieObject *source, float range, std::list<CWeenieObject *> *results)
{
	float selfRadius = source->GetRadius();

	for (auto player : m_PlayerList)
	{
		if (!player || source == player)
			continue;

		float self_radius_plus_other_radius_plus_range = range + selfRadius + player->GetRadius();
		if (source->DistanceSquared(player) <= (self_radius_plus_other_radius_plus_range*self_radius_plus_other_radius_plus_range))
		{
			results->push_back(player);
		}
	}
}

CObjCell *CWorldLandBlock::GetObjCell(WORD cell_id, bool bDoPostLoad) // , bool bActivate)
{
	// env cell
	if (cell_id >= LandDefs::first_envcell_id && cell_id <= LandDefs::last_envcell_id)
	{
		auto entry = m_LoadedEnvCells.find(cell_id);

		if (entry != m_LoadedEnvCells.end())
		{
			CEnvCell *pEnvCell = entry->second;

			if (pEnvCell->needs_post_load && bDoPostLoad)
			{
				pEnvCell->needs_post_load = false;
				pEnvCell->init_static_objects();
			}

			return pEnvCell;
		}

		CEnvCell *pEnvCell = CEnvCell::Get(((uint32_t)m_wHeader << 16) | (uint32_t)(cell_id & 0xFFFF));
		if (pEnvCell)
		{
			m_LoadedEnvCells[cell_id] = pEnvCell;

			if (pEnvCell->needs_post_load && bDoPostLoad)
			{
				pEnvCell->needs_post_load = false;
				pEnvCell->init_static_objects();
			}
		}

		return pEnvCell;
	}

	// land cell
	if (cell_id >= LandDefs::first_lcell_id && cell_id <= LandDefs::last_lcell_id)
	{
		if (m_LoadedLandBlock)
		{
			return m_LoadedLandBlock->get_landcell(((uint32_t)m_wHeader << 16) | (uint32_t)(cell_id & 0xFFFF));
		}
	}

	return NULL;
}

void CWorldLandBlock::SetIsTickingWithWorld(bool ticking)
{
	m_bTickingWithWorld = ticking;
}

void CWorldLandBlock::UnloadSpawnsUntilNextTick()
{
	ClearOldDatabaseEntries();

	for (auto &entity : m_EntitiesToAdd)
	{
		if (entity && entity->GetBlock() == this)
			Destroy(entity, false);
	}

	for (auto &entity : m_EntityList)
	{
		if (entity && entity->GetBlock() == this)
			Destroy(entity, false);
	}

	m_PlayerMap.clear();
	m_PlayerList.clear();

	m_EntityMap.clear();
	m_EntitiesToAdd.clear();
	m_EntityList.clear();

	m_bSpawnOnNextTick = true;
}

void CWorldLandBlock::RespawnNextTick()
{
	ClearSpawns(true);
	m_bSpawnOnNextTick = true;
}
