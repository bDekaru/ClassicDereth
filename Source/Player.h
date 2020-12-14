
#pragma once

#include "UseManager.h"
#include "Monster.h"
#include "Packable.h"
#include "TradeManager.h"
#include "DatabaseIO.h"

class CClient;
class BinaryWriter;

class TradeManager;

class SalvageResult : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t material = 0;
	double workmanship = 0.0;
	int units = 0;
};

class CPlayerWeenie : public CMonsterWeenie
{
public:
	CPlayerWeenie(CClient *, uint32_t dwGUID, WORD instance_ts);
	virtual ~CPlayerWeenie();

	virtual void Tick() override;
	
	virtual class CPlayerWeenie *AsPlayer() override { return this; }

	virtual bool IsAdvocate() override;
	virtual bool IsSentinel() override;
	virtual bool IsAdmin() override;
	int GetAccessLevel();

	virtual void PreSpawnCreate() override;

	void addItemsToDropLists(std::vector<CWeenieObject *>  items, std::vector<CWeenieObject *> &removeList, std::vector<CWeenieObject *> &alwaysDropList, std::vector<CWeenieObject *> &allValidItems);
	void possibleDropItems(std::vector<CWeenieObject *> &allValidItems, std::vector<CWeenieObject *> &alwaysDropList, std::vector<CWeenieObject *> &removeList, bool bWielded = true);
	void sendLostItemMessage(CCorpseWeenie *pCorpse);
	void dropItems(CCorpseWeenie *pCorpse, float coinDropFraction, std::vector<CWeenieObject *> &dropList, std::vector<CWeenieObject *> &alwaysDropList, std::vector<CWeenieObject *> &removeList);
	void doNormalDeath(CCorpseWeenie *pCorpse, bool bPkKill);


	virtual void OnDeathAnimComplete() override;
	virtual void OnDeath(uint32_t killerID) override;
	virtual void OnMotionDone(uint32_t motion, BOOL success) override;
	virtual void OnRegen(STypeAttribute2nd currentAttrib, int newAmount) override;
	
	virtual void NotifyAttackerEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int heath, unsigned int crit, unsigned int attackConditions) override;
	virtual void NotifyDefenderEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int health, BODY_PART_ENUM hitPart, unsigned int crit, unsigned int attackConditions) override;
	virtual void NotifyKillerEvent(const char *text) override;
	virtual void NotifyVictimEvent(const char *text) override;

	virtual void NotifyAttackDone(int error = WERROR_NONE) override;
	virtual void NotifyCommenceAttack() override;
	virtual void NotifyUseDone(int error = WERROR_NONE) override;
	virtual void NotifyWeenieError(int error) override;
	virtual void NotifyWeenieErrorWithString(int error, const char *text) override;
	virtual void NotifyInventoryFailedEvent(uint32_t object_id, int error) override;
	std::string ToUpperCase(std::string tName);

	CWeenieObject *m_pCraftingTool;
	CWeenieObject *m_pCraftingTarget;

	virtual int UseEx(CWeenieObject *pTool, CWeenieObject *pTarget);
	virtual int UseEx(bool bConfirmed = false);

	virtual bool CheckUseRequirements(int index, CCraftOperation *op, CWeenieObject *item);
	virtual void PerformUseModifications(int index, CCraftOperation *op, CWeenieObject *pTool, CWeenieObject *pTarget, CWeenieObject *pCreatedItem);
	virtual void PerformUseModificationScript(uint32_t scriptId, CCraftOperation *op, CWeenieObject *pTool, CWeenieObject *pTarget, CWeenieObject *pCreatedItem);
	virtual int GetMaterialMod(int material);

	void PerformSalvaging(uint32_t toolId, PackableList<uint32_t> items);
	int CalculateSalvageAmount(int salvagingSkill, int dWorkmanship, int numAugs);

	uint32_t MaterialToSalvageBagId(MaterialType material);
	bool SpawnSalvageBagInContainer(MaterialType material, int amount, int workmanship, int value, int numItems);

	void SetLoginPlayerQualities();
	void AuditEquipmentSpells();
	void ClearPlayerSpells();
	void AuditSetSpells(bool isLogin = false);
	void CastSetSpells(int setid, int level, bool islogin, int32_t caster = 0);
	void UpdateSetSpells(int setid, int32_t caster);
	void RemoveSetSpells(std::vector<int> &setsToClear, bool islogin);

	void RecalculateGearRatings();
	void LoadSquelches();
	std::list<CharacterSquelch_t> GetSquelches();
	
	void HandleItemManaRequest(uint32_t itemId);

	

	//base virtuals

	virtual void MarkForDestroy() override
	{
		m_pClient = NULL;
		CMonsterWeenie::MarkForDestroy();
	}

	//Movement overrides
	//...

	//Animation overrides
	//...

	//Control events.
	virtual void MakeAware(CWeenieObject *, bool bForceUpdate = false) override;

	void AddSpellByID(uint32_t id);

	void DetachClient();

	//Network events.
	virtual void SendNetMessage(void *_data, uint32_t _len, WORD _group, BOOL _event = 0, bool ephemeral = false) override;
	virtual void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1, bool ephemeral = false) override;
	virtual void ExitPortal();
	virtual void LoginCharacter();
	virtual void SendCharacterData();
	
	virtual bool IsDead() override;

	bool IsAllegGagged();

	virtual uint32_t OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot) override;

	void SetLastHealthRequest(uint32_t guid);
	void RemoveLastHealthRequest();
	void RefreshTargetHealth();

	//cmoski -- remove last assessed item
	void SetLastAssessed(uint32_t guid);
	std::string RemoveLastAssessed(bool forced = false);

	CClient *GetClient() { return m_pClient; }

	uint32_t m_LastAssessed;
	uint32_t m_dwLastSpawnedObjectID;
	
	uint32_t GetCharacterOptions() { return _playerModule.options_; }
	uint32_t GetCharacterOptions2() { return _playerModule.options2_; }
	void SetCharacterOptions(uint32_t options) { _playerModule.options_ = options; }
	void SetCharacterOptions2(uint32_t options) { _playerModule.options2_ = options; }

	// AutoRepeatAttack_CharacterOption
	// AllowGive_CharacterOption
	// ShowTooltips_CharacterOption | ToggleRun_CharacterOption
	// VividTargetingIndicator_CharacterOption | AutoTarget_CharacterOption
	// FellowshipShareXP_CharacterOption
	// SpellDuration_CharacterOption | SideBySideVitals_CharacterOption
	// UseChargeAttack_CharacterOption | HearAllegianceChat_CharacterOption
	// uint32_t m_CharacterOptions = 0x50C4A542;

	// HearLFGChat_CharacterOptions2 | HearTradeChat_CharacterOptions2 | HearGeneralChat_CharacterOptions2
	// LeadMissileTargets_CharacterOptions2
	// ConfirmVolatileRareUse_CharacterOptions2
	// ShowHelm_CharacterOptions2 | ShowCloak_CharacterOptions2
	// uint32_t m_CharacterOptions2 = 0x00948700 | DisplayNumberDeaths_CharacterOptions2;

	bool ShouldRepeatAttacks() { return GetCharacterOptions() & AutoRepeatAttack_CharacterOption ? true : false; }
	
	bool ShareFellowshipXP() { return GetCharacterOptions() & FellowshipShareXP_CharacterOption ? true : false; }
	bool ShareFellowshipLoot() { return GetCharacterOptions() & FellowshipShareLoot_CharacterOption ? true : false; }

	virtual bool ImmuneToDamage(class CWeenieObject *other) override;

	bool m_bAdminVision = false;
	bool m_bPrivacyMode = false;
	bool m_bChangingStance = false;
	bool m_bCancelAttack = false;
	bool m_bCancelAttackDuplicate = false;

	void UpdateModuleFromClient(PlayerModule &module);
	virtual void SaveEx(class CWeenieSave &save) override;
	virtual void LoadEx(class CWeenieSave &save) override;

	virtual bool ShowHelm() override;
	virtual bool ShowCloak() override;

	PlayerModule _playerModule;
	QuestTable _questTable;

	bool AlreadyMadeAwareOf(uint32_t object_id);
	void SetMadeAwareOf(uint32_t object_id);
	void FlushMadeAwareof(bool clear = false);

	virtual bool InqQuest(const char *questName) override;
	virtual int InqTimeUntilOkayToComplete(const char *questName) override;
	virtual unsigned int InqQuestSolves(const char *questName) override;
	virtual bool UpdateQuest(const char *questName) override;
	virtual void StampQuest(const char *questName) override;
	virtual void IncrementQuest(const char *questName, int amount) override;
	virtual void DecrementQuest(const char *questName, int amount) override;
	virtual void EraseQuest(const char *questName) override;
	virtual void SetQuestCompletions(const char *questName, int numCompletions) override;
	virtual unsigned int InqQuestMax(const char *questName) override;
	virtual std::string Ktref(const char *questName) override;

	std::unordered_map<uint32_t, double> _objMadeAwareOf;

	double _nextTryFixBrokenPosition = 0.0;
	double _nextRareUse = 0.0;
	double _deathTimer = -1.0;
	double _dieTextTimer = -1.0;
	int _dieTextCounter = 0;

	virtual void OnGivenXP(int64_t amount, ExperienceHandlingType flags) override;

	virtual void UpdateVitaePool(uint32_t pool);
	virtual void ReduceVitae(float amount);
	virtual void UpdateVitaeEnchantment();

	virtual void BeginLogout() override;
	virtual void OnLogout();
	
	bool IsLoggingOut() { return _logoutTime >= 0.0; }
	bool IsRecalling() { return _recallTime >= 0.0; }
	void BeginRecall(const Position &targetPos);
	void CancelLifestoneProtection();

	bool HasPortalUseCooldown() { return InqFloatQuality(LAST_PORTAL_TELEPORT_TIMESTAMP_FLOAT, 0) >= 1.0; }
	bool HasTeleportUseCooldown() { return InqFloatQuality(LAST_TELEPORT_START_TIMESTAMP_FLOAT, 0) >= 1.0; }

	virtual bool IsBusy() override;
	virtual void OnTeleported() override;
	bool IsPlayerSquelched(const uint32_t dwGUID, bool checkAccount = TRUE);

	//bool HasTeleportCooldown() { return; }

	CCorpseWeenie *_pendingCorpse = NULL;
	uint32_t GetAccountHouseId();
	
	TradeManager *GetTradeManager();
	void SetTradeManager(TradeManager *tradeManager);

	virtual void ReleaseContainedItemRecursive(CWeenieObject *item) override;

	virtual void ChangeCombatMode(COMBAT_MODE mode, bool playerRequested) override;

	void UpdatePKActivity();
	bool CheckPKActivity() { return m_iPKActivity > Timer::cur_time; }
	void ClearPKActivity() { m_iPKActivity = Timer::cur_time; }

	void AddCorpsePermission(CPlayerWeenie * target);
	void RemoveCorpsePermission(CPlayerWeenie * target);
	bool HasPermission(CPlayerWeenie * target);
	void UpdateCorpsePermissions();
	void ClearPermissions(); // TODO: Add call on logout
	void RemoveConsent(CPlayerWeenie * target);
	void DisplayConsent();
	void ClearConsent(bool onLogout); // TODO: Add call on logout
	std::unordered_map<int, int> m_umCorpsePermissions;
	std::unordered_map<int, int> m_umConsentList;

	void LoadTitles();
	void SendTitles();
	void SetTitle(int titleId);
	void AddTitle(int titleid);
	void NotifyNewTitle(int titleId, bool set);
	void SaveTitles();
	CCraftOperation *TryGetAlternativeOperation(CWeenieObject *target, CWeenieObject *tool, CCraftOperation *op);
	std::string GetTitleStringById(int titledid);
	void InitTitle(int titleId);


	void AddFriend(std::string friendName);
	void RemoveFriend(uint32_t frenemy_id);
	void ClearFriends();
	void FriendOnlineAlert(uint32_t player_id, std::string player_name, bool online);
	void NotifyFriendsOfStatus(bool online);
	void PackFriendData(BinaryWriter &msg, uint32_t friend_id, std::string friend_name, bool isOnline);
	void SendFullFriendData();
	void NotifyAllegianceOfStatus(bool online);
	void AllegianceOnlineAlert(uint32_t target, bool online);
	bool IsPkLogging();

	void SetSanctuaryAsLogin();
	uint32_t GetTotalSkillCredits(bool removeCreditQuests = false);
	uint32_t GetExpectedSkillCredits(bool countCreditQuests = true);

	void LoadCorpses();
	void SaveCorpses();

	void CombatPetsDisplayCombatDamage(bool show);
	bool CombatPetsShowCombatDamage();

	bool HasSigils();
	int32_t GetSigilLevels(int setid);
	std::map<int, double> GetSigilProcRate(CWeenieObject* &caster);
	void HandleAetheriaProc(uint32_t targetid);
	void UpdateSigilProcRate();

protected:
	CClient *m_pClient;
	std::list<CharacterSquelch_t> squelches;
	uint32_t m_LastHealthRequest = 0;

	double m_fNextMakeAwareCacheFlush = 0.0;
	bool m_bAttackable = true;
	double m_NextCorpsePermissionsUpdate = 0.0;
	double m_NextSave = 0.0;

	double m_NextHealthUpdate = 0.0;

	double _logoutTime = -1.0;
	double _beginLogoutTime = -1.0;
	double _recallTime = -1.0;
	Position _recallPos;
	bool _isFirstPortalInSession = true;

	TradeManager *m_pTradeManager = NULL;
	double m_fNextTradeCheck = 0;

	std::vector<int> charTitles;
	std::vector<unsigned int> corpses;
private:
	int m_iPKActivity = 0;
	bool m_bShowCombatPetDamage = false;
	CWeenieObject* procSigil;
	std::map<int, double> _sigilProcs;
};

class CWandSpellUseEvent : public CUseEventData
{
private:
	uint32_t _wandId = 0;
	uint32_t _spellId = 0;
	uint32_t _targetId = 0;
	int _newManaValue = -1;
public:
	CWandSpellUseEvent(uint32_t wandId, uint32_t targetId);
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
	virtual void Cancel(uint32_t error = 0) override;
	virtual void Done(uint32_t error = 0) override;
};

class CLifestoneRecallUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CHouseRecallUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CMansionRecallUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CMarketplaceRecallUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CPKArenaUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CPKLArenaUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CAllegianceHometownRecallUseEvent : public CRecallUseEvent
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};


