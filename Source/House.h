
#pragma once

#include "WeenieObject.h"
#include "Portal.h"
#include "Chest.h"


class CHouseData : public PackObj
{
public:
	CHouseData() = default;
	virtual ~CHouseData() = default;
	DECLARE_PACKABLE()

	void ClearOwnershipData();
	void SetHookVisibility(bool newSetting);
	void AbandonHouse();
	void Save();

	uint32_t _slumLordId = 0;
	Position _position;
	uint32_t _houseType = 0;
	uint32_t _ownerId = 0;
	uint32_t _ownerAccount = 0;
	uint32_t _purchaseTimestamp = 0;
	uint32_t _currentMaintenancePeriod = 0;
	HousePaymentList _buy;
	HousePaymentList _rent;
	PackableList<uint32_t> _accessList;
	PackableList<uint32_t> _storageAccessList;
	bool _allegianceAccess = false;
	bool _allegianceStorageAccess = false;
	bool _everyoneAccess = false;
	bool _everyoneStorageAccess = false;
	bool _hooksVisible = true;

	//dynamic fields
	uint32_t _houseId = 0;
	std::set<uint32_t> _hookList;
};


class CHouseWeenie : public CWeenieObject
{
public:
	CHouseWeenie();

	virtual class CHouseWeenie *AsHouse() override { return this; }

	virtual void EnsureLink(CWeenieObject *source) override;

	virtual bool HasAccess(CPlayerWeenie *requester);
	virtual bool HasStorageAccess(CPlayerWeenie *requester);

	CHouseData *GetHouseData();
	std::string GetHouseOwnerName();
	uint32_t GetHouseOwner();
	uint32_t GetHouseDID();
	int GetHouseType();
	CSlumLordWeenie *GetSlumLord();

	//std::set<uint32_t> _hookList;
	//uint32_t _currentMaintenancePeriod;
	//HousePaymentList _rent;
	//PackableList<uint32_t> _accessList;
	//PackableList<uint32_t> _storageAccessList;
	//bool _allegianceAccess = false;
	//bool _allegianceStorageAccess = false;
	//bool _everyoneAccess = false;
	//bool _everyoneStorageAccess = false;
};

class CSlumLordWeenie : public CWeenieObject
{
public:
	CSlumLordWeenie();

	virtual void Tick() override;

	virtual class CSlumLordWeenie *AsSlumLord() override { return this; }

	CHouseWeenie *GetHouse();
	void GetHouseProfile(HouseProfile &prof);

	virtual int DoUseResponse(CWeenieObject *other) override;

	void BuyHouse(CPlayerWeenie *player, const PackableList<uint32_t> &items);
	void RentHouse(CPlayerWeenie *player, const PackableList<uint32_t> &items);
	void CheckRentPeriod();

	bool _initialized = false;
	double _nextSave = -1.0;
};

class CHookWeenie : public CContainerWeenie
{
public:
	CHookWeenie();

	virtual class CHookWeenie *AsHook() override { return this; }

	virtual void Tick() override;

	virtual void SaveEx(class CWeenieSave &save) override;
	virtual void LoadEx(class CWeenieSave &save) override;

	virtual void PreSpawnCreate() override;

	int DoUseResponse(CWeenieObject *other) override;
	void Identify(CWeenieObject *other, uint32_t overrideId = 0) override;

	virtual uint32_t Container_InsertInventoryItem(uint32_t dwCell, CWeenieObject *pItem, uint32_t slot) override;
	virtual void ReleaseContainedItemRecursive(CWeenieObject *item) override;
	void OnContainerClosed(CWeenieObject *requestedBy = NULL) override;

	void UpdateHookedObject(CWeenieObject *hookedItem = NULL, bool sendUpdate = true);
	void ClearHookedObject(bool sendUpdate = true);
	void SetHookVisibility(bool newSetting);

	CHouseWeenie *GetHouse();
	CHouseData *GetHouseData();

	bool _initialized = false;
	double _nextInitCheck = -1.0;
};

class CDeedWeenie : public CWeenieObject
{
public:
	CDeedWeenie();

	virtual class CDeedWeenie *AsDeed() override { return this; }

	class CHouseWeenie *GetHouse();
};

class CBootSpotWeenie : public CWeenieObject
{
public:
	CBootSpotWeenie();

	virtual class CBootSpotWeenie *AsBootSpot() override { return this; }

	class CHouseWeenie *GetHouse();
};

class CHousePortalWeenie : public CPortal
{
public:
	CHousePortalWeenie();

	virtual class CHousePortalWeenie *AsHousePortal() override { return this; }
	virtual void ApplyQualityOverrides() override;

	class CHouseWeenie *GetHouse();

	virtual int Use(CPlayerWeenie *other) override;

	virtual bool GetDestination(Position &position) override;
};

class CStorageWeenie : public CChestWeenie
{
public:
	CStorageWeenie();

	virtual class CStorageWeenie *AsStorage() override { return this; }

	int DoUseResponse(CWeenieObject *other) override;
	void OnContainerClosed(CWeenieObject *requestedBy = NULL) override;

	class CHouseWeenie *GetHouse();
};



