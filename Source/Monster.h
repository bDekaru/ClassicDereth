
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
	DWORD m_MotionUseMotionID = 0;
	DWORD m_MotionUseTarget = 0;
	DWORD m_MotionUseChildID = 0;
	DWORD m_MotionUseChildLocation = 0;
};

class CMonsterWeenie : public CContainerWeenie
{
public:
	CMonsterWeenie();
	virtual ~CMonsterWeenie() override;

	virtual class CMonsterWeenie *AsMonster() { return this; }

	virtual void Tick();

	static bool ClothingPrioritySorter(const CWeenieObject *first, const CWeenieObject *second);
	virtual void GetObjDesc(ObjDesc &objDesc) override;

	virtual void ApplyQualityOverrides() override;

	virtual void OnDeathAnimComplete();
	virtual void OnMotionDone(DWORD motion, BOOL success) override;
	virtual void OnDeath(DWORD killer_id) override;
	virtual void OnDealtDamage(DamageEventData &damageData) override;
	virtual void OnTookDamage(DamageEventData &damageData) override;
	virtual void OnIdentifyAttempted(CWeenieObject *other) override;
	virtual void OnResistSpell(CWeenieObject *attacker) override;
	virtual void OnEvadeAttack(CWeenieObject *attacker) override;
	virtual DWORD OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot) override;

	DWORD DoForcedUseMotion(MotionUseType useType, DWORD motion, DWORD target = 0, DWORD childID = 0, DWORD childLoc = 0, MovementParameters *params = NULL);

	virtual void PreSpawnCreate() override;
	virtual void PostSpawn() override;

	virtual void TryMeleeAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion = 0) override;
	virtual void TryMissileAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion = 0) override;

	virtual bool IsDead() override;

	virtual double GetMeleeDefenseModUsingWielded() override;
	virtual double GetMissileDefenseModUsingWielded() override;
	virtual double GetMagicDefenseModUsingWielded() override;

	virtual int GetAttackTime() override;
	virtual int GetAttackTimeUsingWielded() override;
	virtual int GetAttackDamage() override;
	virtual float GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor) override;

	virtual void HandleAggro(CWeenieObject *attacker) override;
	
	void DropAllLoot(CCorpseWeenie *pCorpse);
	virtual void GenerateDeathLoot(CCorpseWeenie *pCorpse);

	virtual BOOL DoCollision(const class ObjCollisionProfile &prof);

	CCorpseWeenie *CreateCorpse();

	bool IsAttackMotion(DWORD motion);

	DWORD m_LastAttackTarget = 0;
	DWORD m_LastAttackHeight = 1;
	float m_LastAttackPower = 0.0f;

	DWORD m_AttackAnimTarget = 0;
	DWORD m_AttackAnimHeight = 1;
	float m_AttackAnimPower = 0.0f;

	bool m_bChargingAttack = false;
	DWORD m_ChargingAttackTarget = 0;
	DWORD m_ChargingAttackHeight = false;
	float m_ChargingAttackPower = 0.0f;
	float m_fChargeAttackStartTime = (float) INVALID_TIME;

	unsigned int m_MeleeDamageBonus = 0;
	
	virtual void ChangeCombatMode(COMBAT_MODE mode, bool playerRequested) override;

	class MonsterAIManager *m_MonsterAI = NULL;

	CWeenieObject *SpawnWielded(CWeenieObject *item, bool deleteItemOnFailure = true);
	CWeenieObject *SpawnWielded(DWORD wcid, int ptid, float shade);
	CWeenieObject *SpawnWielded(DWORD index, SmartArray<Style_CG> possibleStyles, DWORD color, SmartArray<DWORD> validColors, long double shade);

	// Inventory
	CWeenieObject *FindValidNearbyItem(DWORD itemId, float maxDistance = 2.0);
	CContainerWeenie *FindValidNearbyContainer(DWORD itemId, float maxDistance = 2.0);
	bool GetEquipPlacementAndHoldLocation(CWeenieObject *item, DWORD location, DWORD *pPlacementFrame, DWORD *pHoldLocation);
	BYTE GetEnchantmentSerialByteForMask(int priority);
	int CheckWieldRequirements(CWeenieObject *item, CWeenieObject *wielder, STypeInt requirementStat, STypeInt skillStat, STypeInt difficultyStat);

	bool MoveItemToContainer(DWORD sourceItemId, DWORD targetContainerId, DWORD targetSlot, bool animationDone = false);
	void FinishMoveItemToContainer(CWeenieObject *sourceItem, CContainerWeenie *targetContainer, DWORD targetSlot, bool bSendEvent = true, bool silent = false);

	bool MoveItemTo3D(DWORD sourceItemId, bool animationDone = false);
	void FinishMoveItemTo3D(CWeenieObject *sourceItem);

	bool MoveItemToWield(DWORD sourceItemId, DWORD targetLoc, bool animationDone = false);
	bool FinishMoveItemToWield(CWeenieObject *sourceItem, DWORD targetLoc);

	bool MergeItem(DWORD sourceItemId, DWORD targetItemId, DWORD amountToTransfer, bool animationDone = false);
	bool SplitItemToContainer(DWORD sourceItemId, DWORD targetContainerId, DWORD targetSlot, DWORD amountToMove, bool animationDone = false);
	bool SplitItemto3D(DWORD sourceItemId, DWORD amountToTransfer, bool animationDone = false);
	bool SplitItemToWield(DWORD sourceItemId, DWORD targetLoc, DWORD amountToTransfer, bool animationDone = false);

	void GiveItem(DWORD targetContainerId, DWORD sourceItemId, DWORD amountToTransfer);
	void FinishGiveItem(CContainerWeenie *targetContainer, CWeenieObject *sourceItem, DWORD amountToTransfer);

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
	DWORD m_DeathKillerIDForCorpse;

	MotionUseData m_MotionUseData;
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
