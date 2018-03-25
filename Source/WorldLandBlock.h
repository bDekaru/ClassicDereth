
#pragma once

#include "World.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"

#ifdef _DEBUG
#define DELETE_ENTITY(x) LOG(Temp, Normal, csprintf("Delete Entity %u @ %s\r\n", pEntity->id, __FUNCTION__)); delete pEntity
#else
#define DELETE_ENTITY(x) delete pEntity
#endif

enum LandblockDormancyStatus
{
	DoNotGoDormant,
	WaitToGoDormant,
	Dormant
};

class CWorldLandBlock
{
public:
	CWorldLandBlock(CWorld *pWorld, WORD wHeader);
	~CWorldLandBlock();
	
	void ClearOldDatabaseEntries();

	void Init();

	WORD GetHeader();
	BOOL Think();

	void ClearSpawns();

	void Insert(CWeenieObject *pEntity, WORD wOld = 0, BOOL bNew = FALSE, bool bMakeAware = true);
	void Destroy(CWeenieObject *pEntity, bool bDoRelease = true);
	void Release(CWeenieObject *pEntity);
	void ExchangePVS(CWeenieObject *pSource, WORD old_block_id);
	void ExchangeData(CWeenieObject *pSource);
	void ExchangeDataForCellID(CWeenieObject *pSource, DWORD cell_id);
	void ExchangeDataForStabChange(CWeenieObject *pSource, DWORD old_cell_id, DWORD new_cell_id);

	CPlayerWeenie* FindPlayer(DWORD dwGUID);
	CWeenieObject* FindEntity(DWORD dwGUID);

	void Broadcast(void *_data, DWORD _len, WORD _group, DWORD ignore_ent, BOOL _game_event);

	DWORD PlayerCount() { return (DWORD)m_PlayerList.size(); }
	DWORD LiveCount() { return (DWORD)m_EntityList.size(); }

	void EnumNearbyFastNoSphere(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearby(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearby(CWeenieObject *source, float range, std::list<CWeenieObject *> *results);
	void EnumNearbyPlayers(const Position &pos, float range, std::list<CWeenieObject *> *results);
	void EnumNearbyPlayers(CWeenieObject *source, float range, std::list<CWeenieObject *> *results);

	class CObjCell *GetObjCell(WORD cell_id, bool bDoPostLoad = true); // , bool bActivate = false);

	bool HasPlayers();
	LandblockDormancyStatus GetDormancyStatus() { return m_DormancyStatus; }
	double GetDormancyTime() { return m_fTimeToGoDormant; }

	bool IsWaterBlock();
	bool HasAnySeenOutside();
	bool PossiblyVisibleToOutdoors(DWORD cell_id);

	bool IsTickingWithWorld() { return m_bTickingWithWorld; }
	void SetIsTickingWithWorld(bool ticking);

	void UnloadSpawnsUntilNextTick();

protected:
	void MakeNotDormant();

	void LoadLandBlock();
	void SpawnDynamics();

	bool PlayerWithinPVS();
	bool CanGoDormant();

	void ActivateLandblocksWithinPVS(DWORD cell_id);

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
};