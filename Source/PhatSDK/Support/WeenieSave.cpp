
#include <StdAfx.h>
#include "PhatSDK.h"

#define WEENIE_SAVE_FILE_VERSION 7

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
	pWriter->Write<uint32_t>(WEENIE_SAVE_FILE_VERSION);
	pWriter->Write<uint32_t>(m_SaveTimestamp);
	pWriter->Write<uint32_t>(m_SaveInstanceTS);
	m_Qualities.Pack(pWriter);
	m_ObjDesc.Pack(pWriter);

	m_WornObjDesc.Pack(pWriter); // temporary

	uint32_t header = 0;
	if (_playerModule)
		header |= 1;
	if (_equipment.size() > 0 || _inventory.size() > 0 || _packs.size() > 0)
		header |= 2;
	if (_questTable)
		header |= 4;

	pWriter->Write<uint32_t>(header);
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
}

DEFINE_UNPACK(CWeenieSave)
{
	Destroy();

	m_SaveVersion = pReader->Read<uint32_t>();
	m_SaveTimestamp = pReader->Read<uint32_t>();
	m_SaveInstanceTS = pReader->Read<uint32_t>();
	m_Qualities.UnPack(pReader);
	m_ObjDesc.UnPack(pReader);

	if (m_SaveVersion < 3)
	{
		pReader->Read<uint32_t>();
		pReader->Read<uint32_t>();
		pReader->Read<uint32_t>();
	}

	m_WornObjDesc.UnPack(pReader); // temporary

	if (m_SaveVersion < 2)
		return true;
	
	uint32_t fields = pReader->Read<uint32_t>();
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

	if (m_SaveVersion < 4)
	{
		m_Qualities.RemovePosition(LINKED_LIFESTONE_POSITION);
	}

	return true;
}
