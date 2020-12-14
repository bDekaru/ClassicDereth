
#pragma once

#include "NetworkDefs.h"
#include "Frame.h"
#include "ObjectIDGen.h"
#include "FellowshipManager.h"

typedef struct DungeonDesc_s
{
	WORD wBlockID;
	const char* szDungeonName;
	const char* szAuthor;
	const char* szDescription;
	Position position;
} DungeonDesc_t;

typedef struct TeleTownList_s
{
	std::string	m_teleString;	//! String representing the town name
	Position position;
} TeleTownList_t;

typedef std::unordered_map<uint32_t, CPhysicsObj *> PhysObjMap;
typedef std::vector<CWeenieObject *> PhysObjVector;
typedef std::unordered_map<uint32_t, CWeenieObject *> WeenieMap;
typedef std::vector<CWeenieObject *> WeenieVector;
typedef std::unordered_map<uint32_t, CPlayerWeenie *> PlayerWeenieMap;
typedef std::vector<CPlayerWeenie *> PlayerWeenieVector;

typedef std::vector<TeleTownList_s> TeletownVector;
typedef std::map<uint32_t, Position> LocationMap;
typedef std::vector<class CWorldLandBlock *> LandblockVector;
typedef std::unordered_map<WORD, class CWorldLandBlock *> LandblockMap;
typedef std::map<WORD, DungeonDesc_t> DungeonDescMap;

class CNetDeliveryTargets
{
public:
	// these can populate the values below
	std::set<uint32_t> _target_cell_pvs;
	std::set<uint32_t> _target_weenie_pvs;

	// these will be appended with the PVS of the above
	std::set<uint32_t> _target_cells;
	std::set<uint32_t> _target_players;
};

class CWorld
{
public:
	CWorld();
	~CWorld();

	void Init();

	bool CreateEntity(CWeenieObject *, bool bMakeAware = true);

	uint32_t CreateWorldID(CWeenieObject * pEntity);

	void InsertTeleportLocation(TeleTownList_s l);
	std::string GetTeleportList();
	TeleTownList_s GetTeleportLocation(std::string location);
	void InsertEntity(CWeenieObject *, BOOL bSilent = FALSE);
	void Test();

	CWorldLandBlock *ActivateBlock(WORD wHeader);

	void RemoveEntity(CWeenieObject *pWeenie);

	void Think();

	void SendNetMessage(CNetDeliveryTargets *target, void *_data, uint32_t _len, WORD _group = OBJECT_MSG, uint32_t ignore_ent = 0, BOOL _game_event = 0);

	void BroadcastPVS(CPhysicsObj *physobj, void *_data, uint32_t _len, WORD _group = OBJECT_MSG, uint32_t ignore_ent = 0, BOOL _game_event = 0, bool ephemeral = false);
	void BroadcastPVS(CWeenieObject *weenie, void *_data, uint32_t _len, WORD _group = OBJECT_MSG, uint32_t ignore_ent = 0, BOOL _game_event = 0, bool ephemeral = false);
	void BroadcastPVS(const Position &pos, void *_data, uint32_t _len, WORD _group = OBJECT_MSG, uint32_t ignore_ent = 0, BOOL _game_event = 0, bool ephemeral = false);
	void BroadcastPVS(uint32_t dwCell, void *_data, uint32_t _len, WORD _group = OBJECT_MSG, uint32_t ignore_ent = 0, BOOL _game_event = 0, bool ephemeral = false);
	void BroadcastGlobal(void *_data, uint32_t _len, WORD _group, uint32_t ignore_ent = 0, BOOL _game_event = 0);
	void BroadcastLocal(uint32_t cellid, std::string text);
	void BroadcastLocal(uint32_t cellid, std::string text, LogTextType channel);
	void BroadcastGlobal(BinaryWriter *food, WORD _group, uint32_t ignore_ent = 0, BOOL _game_event = 0, BOOL del = 1);

	void ClearAllSpawns();
	CWorldLandBlock *GetLandblock(WORD wHeader, bool bActivate = false);
	Position FindDungeonDrop();
	Position FindDungeonDrop(WORD wBlockID);
	LocationMap* GetDungeons();

	BOOL DungeonExists(WORD wBlockID);
	DungeonDescMap* GetDungeonDescs();
	DungeonDesc_t* GetDungeonDesc(WORD wBlockID);
	DungeonDesc_t* GetDungeonDesc(const char* szDungeonName);
	void SetDungeonDesc(WORD wBlockID, const char* szDungeonName, const char* szAuthor, const char* szDescription, const Position &position);

	void JuggleEntity(WORD, CWeenieObject* pEntity);

	CWeenieObject *FindObject(uint32_t object_id, bool allowLandblockActivation = false, bool lockObject = false);
	bool FindObjectName(uint32_t, std::string &name);
	PlayerWeenieMap *GetPlayers();
	uint32_t GetNumPlayers();
	std::string GetPlayerName(uint32_t playerId, bool allowOffline = true);
	uint32_t GetPlayerId(const char *name, bool allowOffline = true);
	CPlayerWeenie *FindPlayer(uint32_t);
	CPlayerWeenie *FindPlayer(const char *);
	CWeenieObject *FindWithinPVS(CWeenieObject *pSource, uint32_t dwGUID, bool allowOwnedByOthers = false);
	void EnumNearby(const Position &position, float fRange, std::list<CWeenieObject *> *pResults);
	void EnumNearby(CWeenieObject *pSource, float fRange, std::list<CWeenieObject *> *pResults);
	void EnumNearbyPlayers(const Position &position, float fRange, std::list<CWeenieObject *> *pResults);
	void EnumNearbyPlayers(CWeenieObject *pSource, float fRange, std::list<CWeenieObject *> *pResults);

	void SaveWorld();

	const char* GetMOTD();

	void SetNewGameMode(class CGameMode *pGameMode);
	class CGameMode *GetGameMode();

	static uint32_t GenerateGUID(eGUIDClass type);

	void EnsureRemoved(CWeenieObject *pEntity);

	void BroadcastChatChannel(uint32_t channel_id, CPlayerWeenie *sender, const std::u16string &message);

	void EnsureBlockIsTicking(CWorldLandBlock *pBlock);

	std::string GetServerStatus();
	uint32_t m_SendPerformanceInfoToPlayer = 0;

	void NotifyEventStarted(std::string eventName, GameEventDef *event);
	void NotifyEventStopped(std::string eventName, GameEventDef *event);

	void AddToUsedMergedItems(uint32_t item);
	bool IsItemInUse(uint32_t item);
	void RemoveMergedItem(uint32_t item);


private:
	void LoadDungeonsFile();
	void SaveDungeonsFile();

	void LoadMOTD();

	void EnumerateDungeonsFromCellData();

	void CheckDormantLandblocks();

	CWorldLandBlock *m_pBlocks[256 * 256]; // 256 x 256 array of landblocks.
	LandblockVector m_vBlocks; // Vector of active landblocks.
	LandblockMap m_mDormantBlocks; // Map of dormant landblocks.
	LandblockMap m_mUnloadedBlocks; // Map of unloaded landblocks.
	LandblockVector m_vSpawns; // Vector of spawned landblocks.
	TeletownVector m_vTeleTown; //Vector of Teletowns

	PlayerWeenieMap m_mAllPlayers; // Global list of players.
	WeenieMap m_mAllObjects; // Global list of objects.

	LocationMap m_mDungeons; // Global list of dungeons.
	DungeonDescMap m_mDungeonDescs; // Dungeon Descriptors

	double m_fLastSave = 0.0;
	std::string m_strMOTD;
	class CGameMode *m_pGameMode = NULL;

	double m_fNextDebugValidate = 0.0;

	std::unordered_multimap<std::string, uint32_t> _eventWeenies;

	std::list<uint32_t> _usedMergedItems;
	std::map<uint32_t, bool> _stackableOnGround;
};


