
#include "StdAfx.h"
#include "PhatSDK.h"

#define WEENIE_SAVE_FILE_VERSION 6

CWeenieSave::CWeenieSave()
{
	m_SaveVersion = WEENIE_SAVE_FILE_VERSION;
	m_SaveTimestamp = time(NULL);
}

CWeenieSave::~CWeenieSave()
{
	Destroy();
}

void CWeenieSave::Destroy()
{
	SafeDelete(_playerModule);
	SafeDelete(_questTable);
}

DEFINE_PACK(CWeenieSave)
{
	pWriter->Write<DWORD>(WEENIE_SAVE_FILE_VERSION);
	pWriter->Write<DWORD>(m_SaveTimestamp);
	pWriter->Write<DWORD>(m_SaveInstanceTS);
	m_Qualities.Pack(pWriter);
	m_ObjDesc.Pack(pWriter);

	m_WornObjDesc.Pack(pWriter); // temporary

	DWORD header = 0;
	if (_playerModule)
		header |= 1;
	if (_equipment.size() > 0 || _inventory.size() > 0 || _packs.size() > 0)
		header |= 2;
	if (_questTable)
		header |= 4;
	if (!_rent.empty() || !_accessList.empty() || !_storageAccessList.empty() || _allegianceAccess || _allegianceStorageAccess || _everyoneAccess || _everyoneStorageAccess || _currentMaintenancePeriod > 0)
		header |= 8; //housing data

	pWriter->Write<DWORD>(header);
	if (header & 1)
	{
		_playerModule->Pack(pWriter);
	}
	if (header & 2)
	{
		_equipment.Pack(pWriter);
		_inventory.Pack(pWriter);
		_packs.Pack(pWriter);
	}
	if (header & 4)
	{
		_questTable->Pack(pWriter);
	}
	if (header & 8)
	{
		pWriter->Write<DWORD>(_currentMaintenancePeriod);
		_rent.Pack(pWriter);
		_accessList.Pack(pWriter);
		_storageAccessList.Pack(pWriter);
		pWriter->Write<bool>(_allegianceAccess);
		pWriter->Write<bool>(_allegianceStorageAccess);
		pWriter->Write<bool>(_everyoneAccess);
		pWriter->Write<bool>(_everyoneStorageAccess);		
	}
}

DEFINE_UNPACK(CWeenieSave)
{
	Destroy();

	m_SaveVersion = pReader->Read<DWORD>();
	m_SaveTimestamp = pReader->Read<DWORD>();
	m_SaveInstanceTS = pReader->Read<DWORD>();
	m_Qualities.UnPack(pReader);
	m_ObjDesc.UnPack(pReader);

	if (m_SaveVersion < 3)
	{
		pReader->Read<DWORD>();
		pReader->Read<DWORD>();
		pReader->Read<DWORD>();
	}

	m_WornObjDesc.UnPack(pReader); // temporary

	if (m_SaveVersion < 2)
		return true;
	
	DWORD fields = pReader->Read<DWORD>();
	if (fields & 1)
	{
		_playerModule = new PlayerModule;
		_playerModule->UnPack(pReader);
	}
	if (fields & 2)
	{
		_equipment.UnPack(pReader);
		_inventory.UnPack(pReader);
		_packs.UnPack(pReader);
	}
	if (fields & 4)
	{
		_questTable = new QuestTable;
		_questTable->UnPack(pReader);
	}
	if (fields & 8)
	{
		_currentMaintenancePeriod = pReader->Read<DWORD>();
		_rent.UnPack(pReader);
		_accessList.UnPack(pReader);
		_storageAccessList.UnPack(pReader);
		_allegianceAccess = pReader->Read<bool>();
		_allegianceStorageAccess = pReader->Read<bool>();
		_everyoneAccess = pReader->Read<bool>();
		_everyoneStorageAccess = pReader->Read<bool>();		
	}

	if (m_SaveVersion < 4)
	{
		m_Qualities.RemovePosition(LINKED_LIFESTONE_POSITION);
	}

	return true;
}
