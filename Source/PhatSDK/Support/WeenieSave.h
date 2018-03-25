
#pragma once

#include "Packable.h"

class CWeenieSave : public PackObj
{
public:
	CWeenieSave();
	virtual ~CWeenieSave();

	DECLARE_PACKABLE()

	void Destroy();

	DWORD m_SaveVersion = 0;
	DWORD m_SaveTimestamp = 0;
	DWORD m_SaveInstanceTS = 0;
	CACQualities m_Qualities;
	ObjDesc m_ObjDesc;
	ObjDesc m_WornObjDesc; // not ideal, but we'll keep it around for now
	PlayerModule *_playerModule = NULL;
	QuestTable *_questTable = NULL;

	PackableList<DWORD> _equipment;
	PackableList<DWORD> _inventory;
	PackableList<DWORD> _packs;

	DWORD _currentMaintenancePeriod = 0;
	HousePaymentList _rent;
	PackableList<DWORD> _accessList;
	PackableList<DWORD> _storageAccessList;
	bool _allegianceAccess = false;
	bool _allegianceStorageAccess = false;
	bool _everyoneAccess = false;
	bool _everyoneStorageAccess = false;
};



