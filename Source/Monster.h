
#pragma once

#include "Container.h"
#include "PhysicsObj.h"

enum MotionUseType
{
	MUT_UNDEF = 0,
	MUT_CONSUME_FOOD,
};

struct MotionUseData
{
	void Reset()
	{
		m_MotionUseType = MUT_UNDEF;
	}

	int m_MotionUseType = MUT_UNDEF;
	uint32_t m_MotionUseMotionID = 0;
	uint32_t m_MotionUseTarget = 0;
	uint32_t m_MotionUseChildID = 0;
	uint32_t m_MotionUseChildLocation = 0;
};

class CMonsterWeenie : public CContainerWeenie
{
public:
	CMonsterWeenie();
	virtual ~CMonsterWeenie() override;

	virtual class CMonsterWeenie *AsMonster() override { return this; }

	virtual void Tick() override;

	static bool ClothingPrioritySorter(const CWeenieObject *first, const CWeenieObject *second);
	virtual void GetObjDesc(ObjDesc &objDesc) override;

	virtual void ApplyQualityOverrides() override;

	virtual void OnDeathAnimComplete();
	virtual void OnMotionDone(uint32_t motion, BOOL success) override;
	virtual void OnDeath(uint32_t killer_id) override;
	virtual void OnDealtDamage(DamageEventData &damageData) override;

	std::map<uint32_t, int> m_aDamageSources;
	virtual void OnTookDamage(DamageEventData &damageData) override;
	void UpdateDamageList(DamageEventData &damageData);
	virtual void OnRegen(STypeAttribute2nd currentAttrib, int newAmount) override;

	virtual void GivePerksForKill(CWeenieObject *pKilled) override;
	virtual void GiveXP(int64_t amount, ExperienceHandlingType flags, bool showText = false) override;

	virtual void OnIdentifyAttempted(CWeenieObject *other) override;
	virtual void OnResistSpell(CWeenieObject *attacker) override;
	virtual void OnEvadeAttack(CWeenieObject *attacker) override;
	virtual uint32_t OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot) override;

	uint32_t DoForcedUseMotion(MotionUseType useType, uint32_t motion, uint32_t target = 0, uint32_t childID = 0, uint32_t childLoc = 0, MovementParameters *params = NULL);

	virtual void PreSpawnCreate() override;
	virtual void PostSpawn() override;

	virtual bool TryMeleeAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion = 0) override;
	virtual void TryMissileAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion = 0) override;

	virtual bool IsDead() override;

	virtual double GetMeleeDefenseModUsingWielded() override;
	virtual double GetMissileDefenseModUsingWielded() override;
	virtual double GetMagicDefenseModUsingWielded() override;

	virtual int GetAttackTime() override;
	virtual int GetAttackTimeUsingWielded() override;
	virtual int GetAttackDamage(bool isAssess = false) override;
	virtual float GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor) override;

	virtual void HandleAggro(CWeenieObject *attacker) override;
	
	void DropAllLoot(CCorpseWeenie *pCorpse);
	virtual void GenerateDeathLoot(CCorpseWeenie *pCorpse);

	virtual BOOL DoCollision(const class ObjCollisionProfile &prof) override;
	virtual int AdjustHealth(int amount, bool useRatings = true) override;

	CCorpseWeenie *CreateCorpse(bool visible = true);

	bool IsAttackMotion(uint32_t motion);

	bool CheckRareEligible(CWeenieObject *highestDamageDealer);

	uint32_t m_highestDamageSource = 0;
	int m_totalDamageTaken = 0;

	uint32_t m_LastAttackTarget = 0;
	uint32_t m_LastAttackHeight = 1;
	float m_LastAttackPower = 0.0f;

	uint32_t m_AttackAnimTarget = 0;
	uint32_t m_AttackAnimHeight = 1;
	float m_AttackAnimPower = 0.0f;

	bool m_bChargingAttack = false;
	uint32_t m_ChargingAttackTarget = 0;
	uint32_t m_ChargingAttackHeight = false;
	float m_ChargingAttackPower = 0.0f;
	float m_fChargeAttackStartTime = (float) INVALID_TIME;
	bool m_bIsRareEligible = false;

	unsigned int m_MeleeDamageBonus = 0;
	
	virtual void ChangeCombatMode(COMBAT_MODE mode, bool playerRequested) override;

	class MonsterAIManager *m_MonsterAI = NULL;

	CWeenieObject *SpawnWielded(CWeenieObject *item, bool deleteItemOnFailure = true);
	CWeenieObject *SpawnWielded(uint32_t wcid, int ptid, float shade);
	CWeenieObject *SpawnWielded(uint32_t index, SmartArray<Style_CG> possibleStyles, uint32_t color, SmartArray<uint32_t> validColors, double shade);

	// Inventory
	CWeenieObject *FindValidNearbyItem(uint32_t itemId, float maxDistance = 2.0);
	CContainerWeenie *FindValidNearbyContainer(uint32_t itemId, float maxDistance = 2.0);
	bool GetEquipPlacementAndHoldLocation(CWeenieObject *item, uint32_t location, uint32_t *pPlacementFrame, uint32_t *pHoldLocation);
	BYTE GetEnchantmentSerialByteForMask(int priority);
	int CheckWieldRequirements(CWeenieObject *item, CWeenieObject *wielder, STypeInt requirementStat, STypeInt skillStat, STypeInt difficultyStat);

	bool MoveItemToContainer(uint32_t sourceItemId, uint32_t targetContainerId, uint32_t targetSlot, bool animationDone = false);
	void FinishMoveItemToContainer(CWeenieObject *sourceItem, CContainerWeenie *targetContainer, uint32_t targetSlot, bool bSendEvent = true, bool silent = false);

	bool MoveItemTo3D(uint32_t sourceItemId, bool animationDone = false);
	void FinishMoveItemTo3D(CWeenieObject *sourceItem);

	bool MoveItemToWield(uint32_t sourceItemId, uint32_t targetLoc, bool animationDone = false);
	bool FinishMoveItemToWield(CWeenieObject *sourceItem, uint32_t targetLoc);

	int GetAetheriaSetCount(int setid);

	void UpdateRatingFromGear(STypeInt rating, int gearRating);

	bool MergeItem(uint32_t sourceItemId, uint32_t targetItemId, uint32_t amountToTransfer, bool animationDone = false);
	bool SplitItemToContainer(uint32_t sourceItemId, uint32_t targetContainerId, uint32_t targetSlot, uint32_t amountToMove, bool animationDone = false);
	bool SplitItemto3D(uint32_t sourceItemId, uint32_t amountToTransfer, bool animationDone = false);
	bool SplitItemToWield(uint32_t sourceItemId, uint32_t targetLoc, uint32_t amountToTransfer, bool animationDone = false);

	void GiveItem(uint32_t targetContainerId, uint32_t sourceItemId, uint32_t amountToTransfer);
	void FinishGiveItem(CContainerWeenie *targetContainer, CWeenieObject *sourceItem, uint32_t amountToTransfer);

	void SetDisplayCombatDamage(bool show);
	bool ShowCombatDamage();
	bool CanAcceptGive(CWeenieObject* item);

private:
	void CheckRegeneration(bool &bRegenerateNext, double &lastRegen, float regenRate, STypeAttribute2nd currentAttrib, STypeAttribute2nd maxAttrib);

	bool m_bRegenHealthNext = false;
	double m_fLastHealthRegen = 0.0;

	bool m_bRegenStaminaNext = false;
	double m_fLastStaminaRegen = 0.0;

	bool m_bRegenManaNext = false;
	double m_fLastManaRegen = 0.0;

	bool m_bWaitingForDeathToFinish = false;
	std::string m_DeathKillerNameForCorpse;
	uint32_t m_DeathKillerIDForCorpse;

	MotionUseData m_MotionUseData;

	
	bool m_bGiveCombatData = false;
};

/*
class CBaelZharon : public CMonsterWeenie
{
public:
	CBaelZharon();

	BOOL CrazyThink();
};

class CTargetDrudge : public CMonsterWeenie
{
public:
	CTargetDrudge();
};
*/
