
#pragma once

#include "WeenieObject.h"

#define MAX_WIELDED_COMBAT 5

class CContainerWeenie : public CWeenieObject
{
public:
	CContainerWeenie();
	virtual ~CContainerWeenie() override;

	virtual class CContainerWeenie *AsContainer() { return this; }

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

	virtual DWORD RecalculateCoinAmount() override;
	virtual DWORD ConsumeCoin(int amountToConsume) override;

	bool IsGroundContainer();
	bool IsInOpenRange(CWeenieObject *other);

	virtual int DoUseResponse(CWeenieObject *other);

	virtual void OnContainerOpened(CWeenieObject *other);
	virtual void OnContainerClosed(CWeenieObject *requestedBy = NULL);
	virtual DWORD OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot);

	virtual void NotifyGeneratedPickedUp(CWeenieObject *weenie) override;

	int GetItemsCapacity();
	int GetContainersCapacity();

	void MakeAwareViewContent(CWeenieObject *other);

	void UnloadContainer();

	CContainerWeenie *FindContainer(DWORD container_id);
	virtual CWeenieObject *FindContainedItem(DWORD object_id) override;

	virtual CWeenieObject *GetWieldedCombat(COMBAT_USE combatUse) override;
	void SetWieldedCombat(CWeenieObject *wielded, COMBAT_USE combatUse);

	CWeenieObject *GetWieldedMelee();
	CWeenieObject *GetWieldedMissile();
	CWeenieObject *GetWieldedAmmo();
	CWeenieObject *GetWieldedShield();
	CWeenieObject *GetWieldedTwoHanded();
	virtual CWeenieObject *GetWieldedCaster() override;

	void Container_GetWieldedByMask(std::list<CWeenieObject *> &wielded, DWORD inv_loc_mask);
	CWeenieObject *GetWielded(INVENTORY_LOC slot) override;

	BOOL Container_CanEquip(CWeenieObject *pItem, DWORD dwCoverage);
	BOOL Container_CanStore(CWeenieObject *pItem, bool bPackSlot);
	BOOL Container_CanStore(CWeenieObject *pItem);

	BOOL IsItemsCapacityFull();
	BOOL IsContainersCapacityFull();

	void Container_EquipItem(DWORD dwCell, CWeenieObject *pItem, DWORD dwCoverage, DWORD child_location, DWORD placement);
	void Container_DeleteItem(DWORD object_id);
	virtual DWORD Container_InsertInventoryItem(DWORD dwCell, CWeenieObject *pItem, DWORD slot);

	DWORD Container_GetNumFreeMainPackSlots();

	virtual void ReleaseContainedItemRecursive(CWeenieObject *item) override;

	bool SpawnTreasureInContainer(eTreasureCategory category, int tier, int workmanship = -1);
	bool SpawnInContainer(DWORD wcid, int amount = 1, int ptid = 0, float shade = 0, bool sendEnvent = true);
	bool SpawnCloneInContainer(CWeenieObject *itemToClone, int amount, bool sendEnvent = true);
	bool SpawnInContainer(CWeenieObject *item, bool sendEnvent = true, bool deleteItemOnFailure = true);

	CWeenieObject *FindContained(DWORD object_id);

	virtual void InitPhysicsObj() override;

	void CheckToClose();

	virtual int CheckOpenContainer(CWeenieObject *other);

	void HandleNoLongerViewing(CWeenieObject *other);

	virtual bool HasContainerContents() override;

	void AdjustToNewCombatMode();

	CWeenieObject *m_WieldedCombat[MAX_WIELDED_COMBAT];
	std::vector<CWeenieObject *> m_Wielded;
	std::vector<CWeenieObject *> m_Items;
	std::vector<CWeenieObject *> m_Packs;

	// For opening/closing containers
	double _nextCheckToClose = 0.0;
	bool _failedPreviousCheckToClose = false;
	DWORD _openedById = 0;

	bool m_bInitiallyLocked = false;

	double _nextInventoryTick = 0.0;
};

