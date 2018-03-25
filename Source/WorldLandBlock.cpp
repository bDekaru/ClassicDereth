
#include "StdAfx.h"
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

const double LANDBLOCK_DORMANCY_DELAY = 30.0;
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
		bool stillExists = false;
		for (auto &entity : m_EntityList)
		{
			if (entity->GetID() == entry)
			{
				stillExists = true;
				break;
			}
		}

		if (!stillExists)
		{
			g_pDBIO->RemoveWeenieFromBlock(entry);
			g_pDBIO->DeleteWeenie(entry);
		}
	}
}

void CWorldLandBlock::Init()
{
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
#if 0
	WORD block = m_wHeader;

	for (DWORD i = 0; i < 64; i++)
	{
		DWORD landcell_id = (DWORD)((DWORD)m_wHeader << 16) | (i + 1);

		SmartArray<DWORD> *spawn_ids = g_pWeenieLandCoverage->GetSpawnsForCell(landcell_id);
		if (!spawn_ids)
			continue;

		int spawn_amount = Random::GenUInt(0, min(3, spawn_ids->num_used));

		for (DWORD j = 0; j < spawn_amount; j++)
		{
			extern float CalcSurfaceZ(DWORD dwCell, float xOffset, float yOffset);

			Position pos;
			pos.objcell_id = landcell_id;

			float x_shift = ((landcell_id >> 3) & 7) * 24.0f;
			float y_shift = ((landcell_id >> 0) & 7) * 24.0f;

			x_shift += Random::GenFloat(0.5f, 23.5f);
			y_shift += Random::GenFloat(0.5f, 23.5f);

			/*
			float x_shift = Random::GenFloat(0.5f, 191.5f);
			float y_shift = Random::GenFloat(0.5f, 191.5f);

			WORD cellw = CELL_WORD(landcell_id);
			int minix = (cellw >> 3) & 7;
			int miniy = (cellw >> 0) & 7;
			x_shift += (24 * minix);
			y_shift += (24 * miniy);
			*/

			pos.frame.m_origin = Vector(x_shift, y_shift, CalcSurfaceZ(landcell_id, x_shift, y_shift));

			CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(spawn_ids->array_data[j], &pos, true);

			if (weenie && weenie->IsMonsterWeenie())
			{
				CMonsterWeenie *monster = (CMonsterWeenie *)weenie;
				monster->m_MonsterAI = new MonsterAIManager(monster, monster->m_Position);
			}
		}
	}
#endif

	if (g_pConfig->SpawnLandscape() && m_LoadedLandBlock)
	{
		DWORD block_x = (m_wHeader & 0xFF00) >> 8;
		DWORD block_y = (m_wHeader & 0x00FF) >> 0;

		for (DWORD cell_x = 0; cell_x < 8; cell_x++)
		{
			for (DWORD cell_y = 0; cell_y < 8; cell_y++)
			{
				WORD old_terrain = m_LoadedLandBlock->terrain[(cell_x * 9) + cell_y];
				WORD terrain = g_pCellDataEx->_data.get_terrain((DWORD)m_wHeader << 16, cell_x, cell_y);

				int encounterIndex = (terrain >> 7) & 0xF;
				DWORD wcid = g_pPortalDataEx->GetWCIDForTerrain(block_x, block_y, encounterIndex);

				if (!encounterIndex && !wcid)
					continue;

				float x_shift = 24.0f * cell_x;
				float y_shift = 24.0f * cell_y;

				Position pos;
				pos.objcell_id = ((DWORD)m_wHeader << 16) | 1;
				pos.frame.m_origin = Vector(x_shift, y_shift, 0.0f);
				pos.adjust_to_outside();

				pos.frame.m_origin.z = CalcSurfaceZ(pos.objcell_id, x_shift, y_shift);

				if (wcid == W_HUMAN_CLASS || wcid == W_ADMIN_CLASS || wcid == W_SENTINEL_CLASS)
					continue;

				CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, &pos, true);

				if (!weenie)
				{
				}

				// LOG(Temp, Normal, TEXT("Loaded auto-spawn %s (%u, %X) ei: %d\n"), GetWCIDName(wcid), wcid, wcid, encounterIndex);
			}
		}
	}

	CLandBlockExtendedData *data = g_pCellDataEx->GetLandBlockData((DWORD)m_wHeader << 16);
	if (data)
	{
		for (DWORD i = 0; i < data->weenies.num_used; i++)
		{
			DWORD wcid = data->weenies.array_data[i].wcid;
			Position pos = data->weenies.array_data[i].pos;
			DWORD iid = data->weenies.array_data[i].iid;

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

			if (!g_pConfig->SpawnStaticCreatures())
			{
				CWeenieDefaults *weenieDefaults = g_pWeenieFactory->GetWeenieDefaults(wcid);
				if (!weenieDefaults
					|| weenieDefaults->m_Qualities.m_WeenieType == Creature_WeenieType
					|| weenieDefaults->m_Qualities.m_WeenieType == Vendor_WeenieType)
				{
					continue;
				}
			}

			CWeenieObject *weenie;

			CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(wcid);
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
				bool bDynamicID = !weenie->IsStuck();

				if (!bDynamicID)
				{
					weenie->SetID(iid);
				}

				g_pWorld->CreateEntity(weenie);

				if (bDynamicID)
				{
					DWORD weenie_id = weenie->GetID();
					for (DWORD i = 0; i < data->weenie_links.num_used; i++)
					{
						if (data->weenie_links.array_data[i].source == iid)
							data->weenie_links.array_data[i].source = weenie_id;
						if (data->weenie_links.array_data[i].target == iid)
							data->weenie_links.array_data[i].target = weenie_id;
					}
				}
			}
		}

		for (DWORD i = 0; i < data->weenie_links.num_used; i++)
		{
			DWORD source_id = data->weenie_links.array_data[i].source;
			DWORD target_id = data->weenie_links.array_data[i].target;

			CWeenieObject *source_weenie = g_pWorld->FindObject(source_id); // often creature, or respawnable item
			if (source_weenie)
			{
				CWeenieObject *target_weenie = g_pWorld->FindObject(target_id); // often generator
				if (target_weenie)
				{
					target_weenie->EnsureLink(source_weenie);

					if (target_weenie->m_Qualities._generator_table)
					{
						GeneratorProfile prof;

						if (target_weenie->m_Qualities._generator_table->_profile_list.size() >= 1)
						{
							prof = *target_weenie->m_Qualities._generator_table->_profile_list.begin();
							prof.delay *= g_pConfig->RespawnTimeMultiplier();
							//prof.delay = target_weenie->InqFloatQuality(REGENERATION_INTERVAL_FLOAT, prof.delay, TRUE);
						}
						else
						{
							prof.whereCreate = Specific_RegenLocationType;
							prof.delay *= g_pConfig->RespawnTimeMultiplier();
							//prof.delay = 600.0f;
						}

						prof.type = source_weenie->m_Qualities.id;
						prof.initCreate = false;
						prof.ptid = source_weenie->m_Qualities.GetInt(PALETTE_TEMPLATE_INT, 0);
						prof.shade = source_weenie->m_Qualities.GetFloat(SHADE_FLOAT, 0.0);
						prof.pos_val = source_weenie->m_Position;
						prof.slot = (DWORD)target_weenie->m_Qualities._generator_table->_profile_list.size();
						// prof.delay = (target_weenie->m_Qualities._generator_table->_profile_list.size() > 0) ? target_weenie->m_Qualities._generator_table->_profile_list.begin()->delay : 600.0f;
						prof.probability = -1;

						target_weenie->m_Qualities._generator_table->_profile_list.push_back(prof);

						if (!target_weenie->m_Qualities._generator_registry)
							target_weenie->m_Qualities._generator_registry = new GeneratorRegistry();

						GeneratorRegistryNode regNode;
						regNode.slot = prof.slot;
						regNode.amount = 1;
						regNode.shop = 0;
						regNode.m_wcidOrTtype = prof.type;
						regNode.checkpointed = 0;
						regNode.ts = Timer::cur_time;
						regNode.m_bTreasureType = false;
						target_weenie->m_Qualities._generator_registry->_registry.add(source_weenie->id, &regNode);
					}
				}
			}
		}
	}
}

void CWorldLandBlock::LoadLandBlock()
{
	DWORD landBlock = (DWORD)(m_wHeader << 16) | 0xFFFF;
	m_LoadedLandBlock = CLandBlock::Get(landBlock);

	if (m_LoadedLandBlock)
	{
		long LCoordX, LCoordY;
		LandDefs::blockid_to_lcoord(landBlock, LCoordX, LCoordY);
		m_LoadedLandBlock->block_coord.x = LCoordX;
		m_LoadedLandBlock->block_coord.y = LCoordY;

		/*
		long Magic1, Magic2;
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

		if (m_LoadedLandBlock->needs_post_load)
		{
			m_LoadedLandBlock->init_buildings();
			m_LoadedLandBlock->init_static_objs(0);
			m_LoadedLandBlock->needs_post_load = false;
		}

		if (m_LoadedLandBlock->lbi)
		{
			for (DWORD i = 0; i < m_LoadedLandBlock->lbi->num_cells; i++)
			{
				if (CEnvCell *cell = (CEnvCell *)GetObjCell((DWORD)(m_wHeader << 16) | (0x100 + i), false))
				{
					if (cell->seen_outside)
					{
						_cached_any_seen_outside = true;
					}
				}
			}
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
	if (CPlayerWeenie *player = pEntity->AsPlayer())
	{
		m_PlayerMap.insert(std::pair<DWORD, CPlayerWeenie *>(pEntity->GetID(), player));
		m_PlayerList.push_back(player);

		MakeNotDormant();

		// spawn up adjacent landblocks
		ActivateLandblocksWithinPVS(pEntity->GetLandcell());
	}

	m_EntityMap.insert(std::pair<DWORD, CWeenieObject *>(pEntity->GetID(), pEntity));
	m_EntitiesToAdd.push_back(pEntity);

	pEntity->Attach(this);

	if (bNew)
		pEntity->RemovePreviousInstance();

	if (bMakeAware)
		ExchangePVS(pEntity, wOld);
}

CWeenieObject *CWorldLandBlock::FindEntity(DWORD dwGUID)
{
	WeenieMap::iterator eit = m_EntityMap.find(dwGUID);

	if (eit == m_EntityMap.end())
		return NULL;

	return eit->second;
}

CPlayerWeenie *CWorldLandBlock::FindPlayer(DWORD dwGUID)
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

void CWorldLandBlock::ExchangeDataForStabChange(CWeenieObject *pSource, DWORD old_cell_id, DWORD new_cell_id)
{
	CObjCell *old_cell, *new_cell;

	old_cell = (BLOCK_WORD(old_cell_id) == m_wHeader) ? GetObjCell(old_cell_id & 0xFFFF) : NULL;
	new_cell = (BLOCK_WORD(new_cell_id) == m_wHeader) ? GetObjCell(new_cell_id & 0xFFFF) : NULL;

	if (new_cell)
	{
		for (DWORD i = 0; i < new_cell->num_stabs; i++)
		{
			DWORD cell_id_check = new_cell->stab_list[i];

			if (old_cell_id == cell_id_check)
				continue;

			bool already_visible = false;

			if (old_cell)
			{
				for (DWORD j = 0; j < old_cell->num_stabs; j++)
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

void CWorldLandBlock::ExchangeDataForCellID(CWeenieObject *source, DWORD cell_id)
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

void CWorldLandBlock::Broadcast(void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event)
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
			pPlayer->SendNetMessage(_data, _len, _group, _game_event);
		}

		pit++;
	}
}

bool CWorldLandBlock::CanGoDormant()
{
	return !HasPlayers();
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

bool CWorldLandBlock::PossiblyVisibleToOutdoors(DWORD cell_id)
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
	DWORD DestroyObject[2];
	DestroyObject[0] = 0x0024;
	DestroyObject[1] = pEntity->GetID();

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), DestroyObject, sizeof(DestroyObject), OBJECT_MSG, 0);
#else

	DWORD RemoveObject[3];
	RemoveObject[0] = 0xF747;
	RemoveObject[1] = pEntity->GetID();
	RemoveObject[2] = pEntity->_instance_timestamp;

	m_pWorld->BroadcastPVS(pEntity->GetLandcell(), RemoveObject, sizeof(RemoveObject));

	// LOG(Temp, Normal, "Removing entity %08X %04X @ %08X \n", pEntity->GetID(), pEntity->_instance_timestamp, pEntity->GetLandcell());
#endif

	pEntity->exit_world();
	pEntity->leave_world();
	pEntity->unset_parent();
	pEntity->unparent_children();

	m_pWorld->EnsureRemoved(pEntity);

	if (DWORD generator_id = pEntity->InqIIDQuality(GENERATOR_IID, 0))
	{
		CWeenieObject *target = g_pWorld->FindObject(generator_id);
		if (target)
			target->NotifyGeneratedDeath(pEntity);
	}

	DELETE_ENTITY(pEntity);
}

BOOL CWorldLandBlock::Think()
{
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

		DWORD cell_id = pEntity->GetLandcell();
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
				static bool checkMe = true;
				if (checkMe)
				{
					assert(pEntity->cell);
					checkMe = false;
				}

				if (!pEntity->cell)
				{
					if (CPlayerWeenie *player = pEntity->AsPlayer())
					{
						if (player->_nextTryFixBrokenPosition < Timer::cur_time)
						{
							player->_nextTryFixBrokenPosition = Timer::cur_time + 5.0;
							player->Movement_Teleport(Position(0xA9B4001F, Vector(87.750603f, 147.722321f, 66.005005f), Quaternion(0.011819f, 0.000000, 0.000000, -0.999930f)), false);
						}
					}
				}

				pEntity->update_object();
				pEntity->Tick();

				DWORD cell_id = pEntity->GetLandcell();
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

void CWorldLandBlock::ActivateLandblocksWithinPVS(DWORD cell_id)
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

void CWorldLandBlock::ClearSpawns()
{
	WeenieVector::iterator eit = m_EntityList.begin();
	WeenieVector::iterator eend = m_EntityList.end();

	CWeenieObject *pEntity;

	while (eit != eend)
	{
		pEntity = *eit;

		if (pEntity && !pEntity->HasOwner() && !pEntity->m_bDontClear)
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
		{
			eit++;
		}
	}
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

		CEnvCell *pEnvCell = CEnvCell::Get(((DWORD)m_wHeader << 16) | (DWORD)(cell_id & 0xFFFF));
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
			return m_LoadedLandBlock->get_landcell(((DWORD)m_wHeader << 16) | (DWORD)(cell_id & 0xFFFF));
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




