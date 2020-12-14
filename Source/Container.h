
#pragma once

#include "WeenieObject.h"

#define MAX_WIELDED_COMBAT 5

class CContainerWeenie : public CWeenieObject
{
public:
	CContainerWeenie();
	virtual ~CContainerWeenie() override;

	virtual class CContainerWeenie *AsContainer() override { return this; }

	virtual bool IsAttunedOrContainsAttuned() override;

	virtual void ApplyQualityOverrides() override;
	virtual void ResetToInitialState() override;
	virtual void PostSpawn() override;

	virtual void InventoryTick() override;
	virtual void Tick() override;
	virtual void DebugValidate() override;

	virtual bool RequiresPackSlot() override { return true; }

	virtual void SaveEx(class CWeenieSave &save) override;
	virtual void LoadEx(class CWeenieSave &save) override;

	virtual void RecalculateEncumbrance() override;

	virtual uint32_t RecalculateCoinAmount(int currencyid) override;
	virtual uint32_t ConsumeCoin(int amountToConsume, int currencyid) override;

	bool IsGroundContainer();
	bool IsInOpenRange(CWeenieObject *other);

	virtual int DoUseResponse(CWeenieObject *other) override;

	virtual void OnContainerOpened(CWeenieObject *other);
	virtual void OnContainerClosed(CWeenieObject *requestedBy = NULL);
	virtual uint32_t OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot);

	virtual void NotifyGeneratedPickedUp(CWeenieObject *weenie) override;

	int GetItemsCapacity();
	int GetContainersCapacity();

	void MakeAwareViewContent(CWeenieObject *other);

	void UnloadContainer();

	CContainerWeenie *FindContainer(uint32_t container_id);
	virtual CWeenieObject *FindContainedItem(uint32_t object_id) override;

	virtual CWeenieObject *GetWieldedCombat(COMBAT_USE combatUse) override;
	void SetWieldedCombat(CWeenieObject *wielded, COMBAT_USE combatUse);

	bool HasWielded();
	CWeenieObject *GetWieldedMelee();
	CWeenieObject *GetWieldedMissile();
	CWeenieObject *GetWieldedAmmo();
	CWeenieObject *GetWieldedShield();
	CWeenieObject *GetWieldedTwoHanded();
	virtual CWeenieObject *GetWieldedCaster() override;

	void Container_GetWieldedByMask(std::list<CWeenieObject *> &wielded, uint32_t inv_loc_mask);
	CWeenieObject *GetWielded(INVENTORY_LOC slot) override;

	BOOL Container_CanEquip(CWeenieObject *pItem, uint32_t dwCoverage);
	BOOL Container_CanStore(CWeenieObject *pItem, bool bPackSlot);
	BOOL Container_CanStore(CWeenieObject *pItem);

	BOOL IsItemsCapacityFull();
	BOOL IsContainersCapacityFull();

	void Container_EquipItem(uint32_t dwCell, CWeenieObject *pItem, uint32_t dwCoverage, uint32_t child_location, uint32_t placement);
	void Container_DeleteItem(uint32_t object_id);
	virtual uint32_t Container_InsertInventoryItem(uint32_t dwCell, CWeenieObject *pItem, uint32_t slot);

	uint32_t Container_GetNumFreeMainPackSlots();

	virtual void ReleaseContainedItemRecursive(CWeenieObject *item) override;

	bool SpawnTreasureInContainer(eTreasureCategory category, int tier, int workmanship = -1);
	bool SpawnInContainer(uint32_t wcid, int amount = 1, int ptid = 0, float shade = 0, bool sendEnvent = true);
	bool SpawnCloneInContainer(CWeenieObject *itemToClone, int amount, bool sendEnvent = true);
	bool SpawnInContainer(CWeenieObject *item, bool sendEnvent = true, bool deleteItemOnFailure = true);

	CWeenieObject *FindContained(uint32_t object_id) override;

	virtual void InitPhysicsObj() override;

	void CheckToClose();

	virtual int CheckOpenContainer(CWeenieObject *other);

	void HandleNoLongerViewing(CWeenieObject *other);

	virtual bool HasContainerContents() override;

	void AdjustToNewCombatMode();

	virtual uint32_t GetItemCount(int itemid) override;
	virtual uint32_t ConsumeItem(int amountToConsume, int itemid) override;

	CWeenieObject *m_WieldedCombat[MAX_WIELDED_COMBAT];
	std::vector<CWeenieObject *> m_Wielded;
	std::vector<CWeenieObject *> m_Items;
	std::vector<CWeenieObject *> m_Packs;

	// For opening/closing containers
	double _nextCheckToClose = 0.0;
	bool _failedPreviousCheckToClose = false;
	uint32_t _openedById = 0;

	bool m_bInitiallyLocked = false;

	double _nextInventoryTick = 0.0;
};

