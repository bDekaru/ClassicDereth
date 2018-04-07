
#pragma once

#include "UseManager.h"
#include "Monster.h"

class CClient;
class BinaryWriter;

class CPlayerWeenie : public CMonsterWeenie
{
public:
	CPlayerWeenie(CClient *, DWORD dwGUID, WORD instance_ts);
	~CPlayerWeenie();

	virtual void Tick() override;
	
	virtual class CPlayerWeenie *AsPlayer() { return this; }

	virtual bool IsAdvocate() override;
	virtual bool IsSentinel() override;
	virtual bool IsAdmin() override;
	int GetAccessLevel();

	virtual void PreSpawnCreate() override;

	void CalculateAndDropDeathItems(CCorpseWeenie *pCorpse);

	virtual void OnDeathAnimComplete() override;
	virtual void OnDeath(DWORD killer_id) override;
	virtual void OnMotionDone(DWORD motion, BOOL success) override;
	
	virtual void NotifyAttackerEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int heath, unsigned int crit, unsigned int attackConditions);
	virtual void NotifyDefenderEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int health, BODY_PART_ENUM hitPart, unsigned int crit, unsigned int attackConditions);
	virtual void NotifyKillerEvent(const char *text);
	virtual void NotifyVictimEvent(const char *text);

	virtual void NotifyAttackDone(int error = WERROR_NONE) override;
	virtual void NotifyCommenceAttack() override;
	virtual void NotifyUseDone(int error = WERROR_NONE) override;
	virtual void NotifyWeenieError(int error) override;
	virtual void NotifyWeenieErrorWithString(int error, const char *text) override;
	virtual void NotifyInventoryFailedEvent(DWORD object_id, int error) override;
	
	virtual int UseEx(CWeenieObject *pTool, CWeenieObject *pTarget);
	virtual bool CheckUseRequirements(int index, CCraftOperation *op, CWeenieObject *pTool, CWeenieObject *pTarget);
	virtual void PerformUseModifications(int index, CCraftOperation *op, CWeenieObject *pTool, CWeenieObject *pTarget, CWeenieObject *pCreatedItem);
	virtual void PerformUseModificationScript(DWORD scriptId, CCraftOperation *op, CWeenieObject *pTool, CWeenieObject *pTarget, CWeenieObject *pCreatedItem);

	void PerformSalvaging(DWORD toolId, PackableList<DWORD> items);
	DWORD MaterialToSalvageBagId(MaterialType material);
	bool SpawnSalvageBagInContainer(MaterialType material, int amount, int workmanship, int value, int numItems);

	void SetLoginPlayerQualities();

	void HandleItemManaRequest(DWORD itemId);

	//base virtuals

	virtual void MarkForDestroy()
	{
		m_pClient = NULL;
		CMonsterWeenie::MarkForDestroy();
	}

	//Movement overrides
	//...

	//Animation overrides
	//...

	//Control events.
	virtual void MakeAware(CWeenieObject *, bool bForceUpdate = false);

	void AddSpellByID(DWORD id);

	void DetachClient();

	//Network events.
	virtual void SendNetMessage(void *_data, DWORD _len, WORD _group, BOOL _event = 0) override;
	virtual void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1) override;
	virtual void ExitPortal();
	virtual void LoginCharacter();
	
	virtual bool IsDead() override;

	virtual DWORD OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot) override;

	//cmoski -- remove last assessed item
	void SetLastAssessed(DWORD guid);
	std::string RemoveLastAssessed();

	CClient *GetClient() { return m_pClient; }

	DWORD m_LastAssessed;
	
	DWORD GetCharacterOptions() { return _playerModule.options_; }
	DWORD GetCharacterOptions2() { return _playerModule.options2_; }

	// AutoRepeatAttack_CharacterOption
	// AllowGive_CharacterOption
	// ShowTooltips_CharacterOption | ToggleRun_CharacterOption
	// VividTargetingIndicator_CharacterOption | AutoTarget_CharacterOption
	// FellowshipShareXP_CharacterOption
	// SpellDuration_CharacterOption | SideBySideVitals_CharacterOption
	// UseChargeAttack_CharacterOption | HearAllegianceChat_CharacterOption
	// DWORD m_CharacterOptions = 0x50C4A542;

	// HearLFGChat_CharacterOptions2 | HearTradeChat_CharacterOptions2 | HearGeneralChat_CharacterOptions2
	// LeadMissileTargets_CharacterOptions2
	// ConfirmVolatileRareUse_CharacterOptions2
	// ShowHelm_CharacterOptions2 | ShowCloak_CharacterOptions2
	// DWORD m_CharacterOptions2 = 0x00948700 | DisplayNumberDeaths_CharacterOptions2;

	bool ShouldRepeatAttacks() { return GetCharacterOptions() & AutoRepeatAttack_CharacterOption ? true : false; }
	
	bool ShareFellowshipXP() { return GetCharacterOptions() & FellowshipShareXP_CharacterOption ? true : false; }
	bool ShareFellowshipLoot() { return GetCharacterOptions() & FellowshipShareLoot_CharacterOption ? true : false; }

	virtual bool ImmuneToDamage(class CWeenieObject *other) override;

	bool m_bAdminVision = false;
	bool m_bPrivacyMode = false;

	void UpdateModuleFromClient(PlayerModule &module);
	virtual void SaveEx(class CWeenieSave &save) override;
	virtual void LoadEx(class CWeenieSave &save) override;

	virtual bool ShowHelm() override;

	PlayerModule _playerModule;
	QuestTable _questTable;

	bool AlreadyMadeAwareOf(DWORD object_id);
	void SetMadeAwareOf(DWORD object_id);
	void FlushMadeAwareof();

	virtual bool InqQuest(const char *questName) override;
	virtual int InqTimeUntilOkayToComplete(const char *questName) override;
	virtual unsigned int InqQuestSolves(const char *questName) override;
	virtual bool UpdateQuest(const char *questName) override;
	virtual void StampQuest(const char *questName) override;
	virtual void IncrementQuest(const char *questName) override;
	virtual void DecrementQuest(const char *questName) override;
	virtual void EraseQuest(const char *questName) override;

	std::unordered_map<DWORD, double> _objMadeAwareOf;

	double _nextTryFixBrokenPosition = 0.0;

	virtual void OnGivenXP(long long amount, bool allegianceXP) override;

	virtual void UpdateVitaePool(DWORD pool);
	virtual void ReduceVitae(float amount);
	virtual void UpdateVitaeEnchantment();

	virtual void BeginLogout() override;
	
	bool IsLoggingOut() { return _logoutTime >= 0.0; }
	bool IsRecalling() { return _recallTime >= 0.0; }
	void BeginRecall(const Position &targetPos);

	virtual bool IsBusy() override;
	virtual void OnTeleported() override;

	CCorpseWeenie *_pendingCorpse = NULL;
	DWORD GetAccountHouseId();

protected:
	CClient *m_pClient;

	double m_fNextMakeAwareCacheFlush = 0.0;
	bool m_bAttackable = true;

	double m_NextSave = 0.0;

	double _logoutTime = -1.0;
	double _recallTime = -1.0;
	Position _recallPos;
	bool _isFirstPortalInSession = true;
};

class CWandSpellUseEvent : public CUseEventData
{
private:
	DWORD _wandId = 0;
	DWORD _spellId = 0;
	DWORD _targetId = 0;
	int _newManaValue = -1;
public:
	CWandSpellUseEvent(DWORD wandId, DWORD targetId);
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
	virtual void Cancel(DWORD error = 0) override;
	virtual void Done(DWORD error = 0) override;
};

class CLifestoneRecallUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CHouseRecallUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CMansionRecallUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CMarketplaceRecallUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CAllegianceHometownRecallUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};


