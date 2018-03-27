
#pragma once

#include "WeenieObject.h"
#include "Portal.h"
#include "Chest.h"

class CHouseData: PackObj
{
public:
	CHouseData() { };
	~CHouseData() { };
	DECLARE_PACKABLE()

	void ClearOwnershipData();
	void SetHookVisibility(bool newSetting);
	void AbandonHouse();
	void Save();

	DWORD _slumLordId = 0;
	Position _position;
	DWORD _houseType = 0;
	DWORD _ownerId = 0;
	DWORD _ownerAccount = 0;
	DWORD _purchaseTimestamp = 0;
	DWORD _currentMaintenancePeriod = 0;
	HousePaymentList _buy;
	HousePaymentList _rent;
	PackableList<DWORD> _accessList;
	PackableList<DWORD> _storageAccessList;
	bool _allegianceAccess = false;
	bool _allegianceStorageAccess = false;
	bool _everyoneAccess = false;
	bool _everyoneStorageAccess = false;
	bool _hooksVisible = true;

	//dynamic fields
	DWORD _houseId = 0;
	std::set<DWORD> _hookList;
};

class CHouseManager
{
public:
	CHouseManager();
	~CHouseManager();

	void Load();
	void Save();

	DWORD _currentHouseMaintenancePeriod = 0;
	DWORD _nextHouseMaintenancePeriod = 0;
	bool _freeHouseMaintenancePeriod = false;

	CHouseData *GetHouseData(DWORD houseId);
	void SaveHouseData(DWORD houseId);
	void SendHouseData(CPlayerWeenie *player, DWORD houseId);

private:
	PackableHashTable<DWORD, CHouseData> _houseDataMap;
};

class CHouseWeenie : public CWeenieObject
{
public:
	CHouseWeenie();

	virtual class CHouseWeenie *AsHouse() { return this; }

	virtual void EnsureLink(CWeenieObject *source) override;

	virtual bool ShouldSave() override { return true; }
	
	virtual bool HasAccess(CPlayerWeenie *requester);
	virtual bool HasStorageAccess(CPlayerWeenie *requester);

	CHouseData *GetHouseData();
	std::string GetHouseOwnerName();
	DWORD GetHouseOwner();
	DWORD GetHouseDID();
	int GetHouseType();
	CSlumLordWeenie *GetSlumLord();

	//std::set<DWORD> _hookList;
	//DWORD _currentMaintenancePeriod;
	//HousePaymentList _rent;
	//PackableList<DWORD> _accessList;
	//PackableList<DWORD> _storageAccessList;
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

	virtual class CSlumLordWeenie *AsSlumLord() { return this; }

	CHouseWeenie *GetHouse();
	void GetHouseProfile(HouseProfile &prof);

	virtual int DoUseResponse(CWeenieObject *other) override;

	void BuyHouse(CPlayerWeenie *player, const PackableList<DWORD> &items);
	void RentHouse(CPlayerWeenie *player, const PackableList<DWORD> &items);
	void CheckRentPeriod();

	bool _initialized = false;
	double _nextSave = -1.0;
};

class CHookWeenie : public CContainerWeenie
{
public:
	CHookWeenie();

	virtual class CHookWeenie *AsHook() { return this; }

	virtual void Tick() override;

	virtual bool ShouldSave() override { return true; }
	virtual void SaveEx(class CWeenieSave &save) override;
	virtual void LoadEx(class CWeenieSave &save) override;

	int DoUseResponse(CWeenieObject *other) override;
	void Identify(CWeenieObject *other, DWORD overrideId = 0) override;

	virtual DWORD Container_InsertInventoryItem(DWORD dwCell, CWeenieObject *pItem, DWORD slot) override;
	virtual void ReleaseContainedItemRecursive(CWeenieObject *item) override;
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

	virtual class CDeedWeenie *AsDeed() { return this; }

	class CHouseWeenie *GetHouse();
};

class CBootSpotWeenie : public CWeenieObject
{
public:
	CBootSpotWeenie();

	virtual class CBootSpotWeenie *AsBootSpot() { return this; }

	class CHouseWeenie *GetHouse();
};

class CHousePortalWeenie : public CPortal
{
public:
	CHousePortalWeenie();

	virtual class CHousePortalWeenie *AsHousePortal() { return this; }
	virtual void ApplyQualityOverrides() override;

	class CHouseWeenie *GetHouse();

	virtual int Use(CPlayerWeenie *other) override;

	virtual bool GetDestination(Position &position) override;
};

class CStorageWeenie : public CChestWeenie
{
public:
	CStorageWeenie();

	virtual class CStorageWeenie *AsStorage() { return this; }
	virtual bool ShouldSave() override { return true; }

	int DoUseResponse(CWeenieObject *other) override;

	class CHouseWeenie *GetHouse();
};



