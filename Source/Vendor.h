
#pragma once

#include "Monster.h"
#include "VendorProfile.h"

class CVendorItem
{
public:
	CVendorItem();
	~CVendorItem();

	CWeenieObject *weenie = NULL;
	int amount = -1;
	bool isPlayerSold = false;
};

class CVendor : public CMonsterWeenie
{
public:
	CVendor();
	virtual ~CVendor() override;
	virtual void Tick() override;

	virtual class CVendor *AsVendor() override { return this; }

	virtual int DoUseResponse(CWeenieObject *player) override;

	void DoVendorEmote(VendorTypeEmote type, uint32_t target_id);

	virtual void PreSpawnCreate() override;

	void SendVendorInventory(CWeenieObject *other);

	void ResetItems();
	void AddVendorItem(uint32_t wcid, int ptid, float shade, int amount);
	void AddVendorItem(uint32_t wcid, int amount);
	CVendorItem *FindVendorItem(uint32_t item_id);
	int TrySellItemsToPlayer(CPlayerWeenie *buyer, const std::list<class ItemProfile *> &desiredItems);
	int TryBuyItemsFromPlayer(CPlayerWeenie *seller, const std::list<ItemProfile *> &desiredItems);
	const std::list<ItemProfile *> GetFilteredItems(std::list<ItemProfile *> desiredItems, CPlayerWeenie *seller);

private:
	VendorProfile profile;
	std::list<CVendorItem *> m_Items;

	void ResellItemToPlayer(CPlayerWeenie* player, uint32_t item_id, uint32_t amount);

	void AddPlayerSoldItem(CWeenieObject* weenie, uint32_t amount);

	/**
		@brief Deletes all player-sold items.
	*/
	void PurgePlayerSoldItems();

	/**
		@brief Try to merge an item into the existing inventory. For example, selling back a healing kit should
		not create a new item in the vendor's inventory.
		@param[in] weenie The weenie to try to merge.
		@return True if the item was merged into an existing stack and can be deleted, false otherwise.
	*/
	bool MergeWithExistingInventory(CWeenieObject* weenie, uint32_t amount);


protected:
	void CheckRange();
	std::set<uint32_t> m_ActiveBuyers;
	double m_VendorCycleTime = 0.0;
	double m_NextPurgeTime;

	std::list<uint32_t> queuedThanks;
	int thankState = 0;
	uint32_t currentlyThanking = 0;
};

class CAvatarVendor : public CVendor
{
public:
	CAvatarVendor() { }

	virtual void PreSpawnCreate() override;
};


