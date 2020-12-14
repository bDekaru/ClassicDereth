
#include <StdAfx.h>
#include "SpellComponentTable.h"

SpellComponentBase::SpellComponentBase()
{
}

SpellComponentBase::~SpellComponentBase()
{
}

DEFINE_PACK(SpellComponentBase)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellComponentBase)
{
	_name = pReader->ReadString();

	try
	{
		// these are obfuscated, swap low/high nibbles
		for (int i = 0; i < _name.size(); i++)
			_name[i] = (char)(BYTE)((BYTE)((BYTE)_name[i] << 4) | (BYTE)((BYTE)_name[i] >> 4));
	}
	catch(...)
	{
		SERVER_ERROR << "Error unpacking spell components";
	}

	_category = (SpellComponentCategory)pReader->Read<int>();
	_iconID = pReader->Read<uint32_t>();
	_type = (SpellComponentType)pReader->Read<int>();
	_gesture = pReader->Read<uint32_t>();
	_time = pReader->Read<float>();
	_text = pReader->ReadString();

	try
	{
		// these are obfuscated, swap low/high nibbles
	for (int i = 0; i < _text.size(); i++)
		_text[i] = (char)(BYTE)((BYTE)((BYTE)_text[i] << 4) | (BYTE)((BYTE)_text[i] >> 4));
	}
	catch (...)
	{
		SERVER_ERROR << "Error unpacking spell component names";
	}

	_CDM = pReader->Read<float>();

	return true;
}

SpellComponentTable::SpellComponentTable()
{
}

SpellComponentTable::~SpellComponentTable()
{
}

DEFINE_DBOBJ(SpellComponentTable, SpellComponentTables);

DEFINE_PACK(SpellComponentTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellComponentTable)
{
	pReader->Read<uint32_t>(); // id

	_spellComponentBaseHash.UnPack(pReader);
	return true;
}

DEFINE_LEGACY_PACK_MIGRATOR(SpellComponentTable);

ITEM_TYPE SpellComponentTable::GetTargetTypeFromComponentID(uint32_t scid)
{
	switch (scid)
	{
	case 0x31u:
	case 0x32u:
	case 0x33u:
	case 0x34u:
	case 0x35u:
	case 0x36u:
	case 0x37u:
	case 0x38u:
	case 0x3Cu:
	case 0x3Du:
	case 0x3Eu:
	case 0xBEu:
		return TYPE_CREATURE;

	case 0x39u:
		return TYPE_ITEM_ENCHANTABLE_TARGET;

	case 0x3Bu:
		return TYPE_PORTAL_MAGIC_TARGET;
	}

	return TYPE_UNDEF;
}

const SpellComponentBase *SpellComponentTable::InqSpellComponentBase(uint32_t key)
{
	return _spellComponentBaseHash.lookup(key);
}





