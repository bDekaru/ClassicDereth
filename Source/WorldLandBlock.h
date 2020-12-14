
#pragma once

#include "World.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "easylogging++.h"

#ifdef _DEBUG
#define DELETE_ENTITY(x) SERVER_INFO << "Delete Entity" << pEntity->id << "@" << __FUNCTION__; delete pEntity
#else
#define DELETE_ENTITY(x) delete pEntity
#endif

enum LandblockDormancyStatus
{
	DoNotGoDormant,
	WaitToGoDormant,
	Dormant,
	NeverDormant
};

class CWorldLandBlock :
	public CLockable
{
public:
	CWorldLandBlock(CWorld *pWorld, WORD wHeader);
	virtual ~CWorldLandBlock();
	
	void ClearOldDatabaseEntries();

	void Init();

	WORD GetHeader();
	BOOL Think();

	void ClearSpawns(bool forced = false);

	void Insert(CWeenieObject *pEntity, WORD wOld = 0, BOOL bNew = FALSE, bool bMakeAware = true);
	void Destroy(CWeenieObject *pEntity, bool bDoRelease = true);
	void Release(CWeenieObject *pEntity);
	void ExchangePVS(CWeenieObject *pSource, WORD old_block_id);
	void ExchangeData(CWeenieObject *pSource);
	void ExchangeDataForCellID(CWeenieObject *pSource, uint32_t cell_id);
	void ExchangeDataForStabChange(CWeenieObject *pSource, uint32_t old_cell_id, uint32_t new_cell_id);

	CPlayerWeenie* FindPlayer(uint32_t dwGUID);
	CWeenieObject* FindEntity(uint32_t dwGUID);

	void Broadcast(void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent, BOOL _game_event, bool ephemeral = false);

	uint32_t PlayerCount() { return (uint32_t)m_PlayerList.size(); }
	uint32_t LiveCount() { return (uint32_t)m_EntityList.size(); }

	void EnumNearbyFastNoSphere(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearby(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearby(CWeenieObject *source, float range, std::list<CWeenieObject *> *results);
	void EnumNearbyPlayers(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearbyPlayers(CWeenieObject *source, float range, std::list<CWeenieObject *> *results);

	class CObjCell *GetObjCell(WORD cell_id, bool bDoPostLoad = true); // , bool bActivate = false);

	bool HasPlayers();
	LandblockDormancyStatus GetDormancyStatus() { return m_DormancyStatus; }
	double GetDormancyTime() { return m_fTimeToGoDormant; }
	void NeverGoDormant() { m_DormancyStatus = LandblockDormancyStatus::NeverDormant; }

	bool IsWaterBlock();
	bool HasAnySeenOutside();
	bool PossiblyVisibleToOutdoors(WORD cell_id);

	bool IsTickingWithWorld() { return m_bTickingWithWorld; }
	void SetIsTickingWithWorld(bool ticking);

	void UnloadSpawnsUntilNextTick();
	void RespawnNextTick();

	bool IsNoDrop() { return m_noDrop; }

protected:
	void MakeNotDormant();

	void LoadLandBlock();
	void SpawnDynamics();

	bool PlayerWithinPVS();
	bool CanGoDormant();

	void ActivateLandblocksWithinPVS(uint32_t cell_id);

	CWorld *m_pWorld;

	WORD m_wHeader;

	PlayerWeenieMap m_PlayerMap;
	PlayerWeenieVector m_PlayerList; // Players, used for message broadcasting.

	WeenieMap m_EntityMap;
	WeenieVector m_EntitiesToAdd;
	WeenieVector m_EntityList;

	bool m_bThinking = false;

	class CLandBlock *m_LoadedLandBlock = NULL;
	std::unordered_map<WORD, class CEnvCell *> m_LoadedEnvCells;

	bool m_bSpawnOnNextTick = false;

	LandblockDormancyStatus m_DormancyStatus = LandblockDormancyStatus::DoNotGoDormant;
	double m_fNextDormancyCheck;
	double m_fTimeToGoDormant;

	bool m_bTickingWithWorld = true;

	bool _cached_any_seen_outside = true;

	bool m_noDrop = false;
};
