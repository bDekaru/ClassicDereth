
#include <StdAfx.h>
#include "PhatSDK.h"

#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC

OldPublicWeenieDesc::OldPublicWeenieDesc()
{
	_db = NULL;
	Reset();
}

OldPublicWeenieDesc::~OldPublicWeenieDesc()
{
	SafeDelete (_db);
}

const float RADAR_DEFAULT_OBVIOUS_DISTANCE = 10.0f;

void OldPublicWeenieDesc::Reset()
{
	_name = "";
	_plural_name = "";

	_wcid = 0;
	_iconID = 0;
	_iconOverlayID = 0;
	_itemsCapacity = 0;
	_containersCapacity = 0;
	_type = ITEM_TYPE::TYPE_UNDEF;
	_value = 0;
	_useability = ITEM_USEABLE::USEABLE_UNDEF;
	_useRadius = 0.0f;
	_targetType = ITEM_TYPE::TYPE_UNDEF;
	_effects = 0;
	_ammoType = AMMO_TYPE::AMMO_NONE;
	_combatUse = COMBAT_USE::COMBAT_USE_NONE;
	_structure = 0;
	_maxStructure = 0;
	_stackSize = 0;
	_maxStackSize = 0;
	_containerID = 0;
	_wielderID = 0;
	_location = 0;
	_valid_locations = 0;
	_priority = 0;
	_bitfield = 0;
	_blipColor = 0;
	_radar_enum = RadarEnum::Undef_RadarEnum;
	_obvious_distance = RADAR_DEFAULT_OBVIOUS_DISTANCE;
	_vndwcid = 0;
	_spellID = 0;
	_house_owner_iid = 0;
	_pscript = PScriptType::PS_Invalid;
	SafeDelete(_db);
	_hook_type = 0;
	_hook_item_types = ITEM_TYPE::TYPE_UNDEF;
	_monarch = 0;
	_material_type = 0;
}

DEFINE_PACK(OldPublicWeenieDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(OldPublicWeenieDesc)
{
	Reset();

	uint32_t header = pReader->Read<uint32_t>();
	_name = pReader->ReadString();
	_wcid = pReader->Read<WORD>();
	_iconID = 0x06000000 | pReader->Read<WORD>();

	_type = (ITEM_TYPE) pReader->Read<int>();
	_bitfield = pReader->Read<uint32_t>(); // PublicWeenieDesc::BitfieldIndex

	if (header & 1)
		_plural_name = pReader->ReadString();
	if (header & 2)
		_itemsCapacity = pReader->Read<char>();
	if (header & 4)
		_containersCapacity = pReader->Read<char>();
	if (header & 8)
		_value = pReader->Read<unsigned int>();
	if (header & 0x10)
		_useability = (ITEM_USEABLE) pReader->Read<int>();
	if (header & 0x20)
		_useRadius = pReader->Read<float>();
	if (header & 0x80000)
		_targetType = (ITEM_TYPE)pReader->Read<int>();
	if (header & 0x80)
		_effects = pReader->Read<unsigned int>(); // UI_EFFECT_TYPE
	if (header & 0x100)
		_ammoType = (AMMO_TYPE) pReader->Read<BYTE>();
	if (header & 0x200)
		_combatUse = (COMBAT_USE)pReader->Read<BYTE>();
	if (header & 0x400)
		_structure = pReader->Read<WORD>();
	if (header & 0x800)
		_maxStructure = pReader->Read<WORD>();
	if (header & 0x1000)
		_stackSize = pReader->Read<WORD>();
	if (header & 0x2000)
		_maxStackSize = pReader->Read<WORD>();
	if (header & 0x4000)
		_containerID = pReader->Read<uint32_t>();
	if (header & 0x8000)
		_wielderID = pReader->Read<uint32_t>();
	if (header & 0x10000)
		_valid_locations = pReader->Read<uint32_t>(); // LocationMask of INVENTORY_LOC
	if (header & 0x20000)
		_valid_locations = pReader->Read<uint32_t>(); // LocationMask of INVENTORY_LOC
	if (header & 0x40000)
		_priority = pReader->Read<uint32_t>();
	if (header & 0x100000)
		_blipColor = pReader->Read<BYTE>();
	if (header & 0x800000)
		_radar_enum = (RadarEnum) pReader->Read<BYTE>();
	if (header & 0x1000000)
		_obvious_distance = pReader->Read<float>();
	if (header & 0x200000)
		_vndwcid = pReader->Read<WORD>();
	if (header & 0x400000)
		_spellID = pReader->Read<WORD>();
	if (header & 0x2000000)
		_house_owner_iid = pReader->Read<uint32_t>();
	if (header & 0x8000000)
		_pscript = (PScriptType) pReader->Read<WORD>();
	if (header & 0x4000000)
	{
		_db = new RestrictionDB();
		_db->UnPack(pReader);
	}
	if (header & 0x10000000)
		_hook_type = pReader->Read<WORD>();
	if (header & 0x20000000)
		_hook_item_types = (ITEM_TYPE) pReader->Read<uint32_t>();
	if (header & 0x40)
		_monarch = pReader->Read<uint32_t>();
	if (header & 0x40000000)
		_iconOverlayID = 0x06000000 | pReader->Read<WORD>();
	if (header & 0x80000000)
		_material_type = pReader->Read<uint32_t>();

	pReader->ReadAlign();
	return true;
}

#endif
