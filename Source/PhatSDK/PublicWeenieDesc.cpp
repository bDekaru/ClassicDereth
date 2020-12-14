
#include <StdAfx.h>
#include "PhatSDK.h"

#if PHATSDK_IS_SERVER
#include "Config.h"
#endif

#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC

PublicWeenieDesc::PublicWeenieDesc()
{
	_db = NULL;
	Reset();
}

PublicWeenieDesc::~PublicWeenieDesc()
{
	SafeDelete(_db);
}

void PublicWeenieDesc::Reset()
{
	_name = "";
	_plural_name = "";

	_wcid = 0;
	_iconID = 0;
	_iconOverlayID = 0;
	_iconUnderlayID = 0;
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
	_workmanship = 0.0f;
	_burden = 0;
	_spellID = 0;
	_house_owner_iid = 0;
	_pscript = PScriptType::PS_Invalid;
	SafeDelete(_db);
	_hook_type = 0;
	_hook_item_types = ITEM_TYPE::TYPE_UNDEF;
	_monarch = 0;
	_material_type = 0;
	_cooldown_id = 0;
	_cooldown_duration = 0.0;
	_pet_owner = 0;
}

DEFINE_UNPACK(PublicWeenieDesc)
{
	Reset();

	uint32_t header = pReader->Read<uint32_t>();
	uint32_t header2 = 0;

	_name = pReader->ReadString();
	_wcid = pReader->Unpack_AsWClassIDCompressed();
	_iconID = pReader->Unpack_AsDataIDOfKnownType(0x06000000);
	_type = (ITEM_TYPE)pReader->Read<uint32_t>();
	_bitfield = pReader->Read<uint32_t>();
	pReader->ReadAlign();
	
	if (_bitfield & BitfieldIndex::BF_INCLUDES_SECOND_HEADER)
		header2 = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_PluralName)
		_plural_name = pReader->ReadString();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ItemsCapacity)
		_itemsCapacity = pReader->Read<char>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ContainersCapacity)
		_containersCapacity = pReader->Read<char>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_AmmoType)
		_ammoType = (AMMO_TYPE)pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Value)
		_value = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Useability)
		_useability = (ITEM_USEABLE)pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_UseRadius)
		_useRadius = pReader->Read<float>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_TargetType)
		_targetType = (ITEM_TYPE)pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_UIEffects)
		_effects = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_CombatUse)
		_combatUse = (COMBAT_USE)pReader->Read<BYTE>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Structure)
		_structure = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaxStructure)
		_maxStructure = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_StackSize)
		_stackSize = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaxStackSize)
		_maxStackSize = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ContainerID)
		_containerID = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_WielderID)
		_wielderID = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ValidLocations)
		_valid_locations = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Location)
		_location = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Priority)
		_priority = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_BlipColor)
		_blipColor = pReader->Read<BYTE>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_RadarEnum)
		_radar_enum = (RadarEnum)pReader->Read<BYTE>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_PScript)
		_pscript = (PScriptType)pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Workmanship)
		_workmanship = pReader->Read<float>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Burden)
		_burden = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_SpellID)
		_spellID = pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HouseOwner)
		_house_owner_iid = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HouseRestrictions)
	{
		_db = new RestrictionDB();
		_db->UnPack(pReader);
	}

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HookItemTypes)
		_hook_item_types = (ITEM_TYPE)pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Monarch)
		_monarch = pReader->Read<uint32_t>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HookType)
		_hook_type = (ITEM_TYPE)pReader->Read<WORD>();

	if (header & PublicWeenieDescPackHeader::PWD_Packed_IconOverlay)
		_iconOverlayID = pReader->Unpack_AsDataIDOfKnownType(0x6000000);

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_IconUnderlay)
		_iconUnderlayID = pReader->Unpack_AsDataIDOfKnownType(0x6000000);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaterialType)
		_material_type = (MaterialType)pReader->Read<uint32_t>();

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_CooldownID)
		_cooldown_id = pReader->Read<uint32_t>();

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_CooldownDuration)
		_cooldown_duration = pReader->Read<double>();

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_PetOwner)
		_pet_owner = pReader->Read<uint32_t>();

	pReader->ReadAlign();
	return true;
}

DEFINE_PACK(PublicWeenieDesc)
{
	uint32_t header2 = 0;
	if (_iconUnderlayID != 0)
		header2 = 1;
	if (_cooldown_id)
		header2 |= 2;
	if (_cooldown_duration != 0.0)
		header2 |= 4;
	if (_pet_owner)
		header2 |= 8;

	if (header2)
		_bitfield |= BF_INCLUDES_SECOND_HEADER;
	else
		_bitfield &= ~BF_INCLUDES_SECOND_HEADER;

	uint32_t header = 0;
	set_pack_header(&header);

	pWriter->Write<uint32_t>(header);
	pWriter->WriteString(_name);
	pWriter->Pack_AsWClassIDCompressed(_wcid);
	pWriter->Pack_AsDataIDOfKnownType(0x06000000, _iconID);
	pWriter->Write<uint32_t>(_type);
	pWriter->Write<uint32_t>(_bitfield);
	pWriter->Align();

	if (_bitfield & BitfieldIndex::BF_INCLUDES_SECOND_HEADER)
		pWriter->Write<uint32_t>(header2);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_PluralName)
		pWriter->WriteString(_plural_name.c_str());

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ItemsCapacity)
		pWriter->Write<char>(_itemsCapacity);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ContainersCapacity)
		pWriter->Write<char>(_containersCapacity);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_AmmoType)
		pWriter->Write<WORD>((AMMO_TYPE)_ammoType);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Value)
		pWriter->Write<uint32_t>(_value);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Useability)
		pWriter->Write<uint32_t>((ITEM_USEABLE)_useability);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_UseRadius)
		pWriter->Write<float>(_useRadius);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_TargetType)
		pWriter->Write<uint32_t>((ITEM_TYPE)_targetType);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_UIEffects)
		pWriter->Write<uint32_t>(_effects);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_CombatUse)
		pWriter->Write<BYTE>((COMBAT_USE)_combatUse);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Structure)
		pWriter->Write<WORD>(_structure);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaxStructure)
		pWriter->Write<WORD>(_maxStructure);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_StackSize)
		pWriter->Write<WORD>(_stackSize);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaxStackSize)
		pWriter->Write<WORD>(_maxStackSize);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ContainerID)
		pWriter->Write<uint32_t>(_containerID);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_WielderID)
		pWriter->Write<uint32_t>(_wielderID);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_ValidLocations)
		pWriter->Write<uint32_t>(_valid_locations);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Location)
		pWriter->Write<uint32_t>(_location);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Priority)
		pWriter->Write<uint32_t>(_priority);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_BlipColor)
		pWriter->Write<BYTE>(_blipColor);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_RadarEnum)
		pWriter->Write<BYTE>((RadarEnum)_radar_enum);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_PScript)
		pWriter->Write<WORD>((PScriptType)_pscript);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Workmanship)
		pWriter->Write<float>(_workmanship);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Burden)
		pWriter->Write<WORD>(_burden);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_SpellID)
		pWriter->Write<WORD>(_spellID);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HouseOwner)
		pWriter->Write<uint32_t>(_house_owner_iid);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HouseRestrictions)
	{
		_db->Pack(pWriter);
	}

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HookItemTypes)
		pWriter->Write<uint32_t>((ITEM_TYPE)_hook_item_types);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_Monarch)
		pWriter->Write<uint32_t>(_monarch);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_HookType)
		pWriter->Write<WORD>((ITEM_TYPE)_hook_type);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_IconOverlay)
		pWriter->Pack_AsDataIDOfKnownType(0x6000000, _iconOverlayID);

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_IconUnderlay)
		pWriter->Pack_AsDataIDOfKnownType(0x6000000, _iconUnderlayID);

	if (header & PublicWeenieDescPackHeader::PWD_Packed_MaterialType)
		pWriter->Write<uint32_t>((MaterialType)_material_type);

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_CooldownID)
		pWriter->Write<uint32_t>(_cooldown_id);

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_CooldownDuration)
		pWriter->Write<double>(_cooldown_duration);

	if (header2 & PublicWeenieDescPackHeader2::PWD2_Packed_PetOwner)
		pWriter->Write<uint32_t>(_pet_owner);

	pWriter->Align();
}

void PublicWeenieDesc::set_pack_header(uint32_t *header)
{
	if (_plural_name.size())
		*header |= 1;
	if (_valid_locations)
		*header |= 0x10000;
	if (_location)
		*header |= 0x20000;
	if (_containerID)
		*header |= 0x4000;
	if (_wielderID)
		*header |= 0x8000;
	if (_itemsCapacity)
		*header |= 2;
	if (_containersCapacity)
		*header |= 4;
	if (_value)
		*header |= 8;
	if (_useability)
		*header |= 0x10;
	if (_useRadius != 0.0f)
		*header |= 0x20;
	if (_targetType)
		*header |= 0x80000;
	if (_effects)
		*header |= 0x80;
	if (_ammoType)
		*header |= 0x100;
	if (_combatUse)
		*header |= 0x200;
	if (_structure)
		*header |= 0x400;
	if (_maxStructure)
		*header |= 0x800;
	if (_stackSize)
		*header |= 0x1000;
	if (_maxStackSize)
		*header |= 0x2000;
	if (_priority)
		*header |= 0x40000;
	if (_blipColor)
		*header |= 0x100000;
	if (_radar_enum)
		*header |= 0x800000;
	if (_workmanship > 0.0)
		*header |= 0x1000000;
	if (_burden > 0)
		*header |= 0x200000;
	if (_spellID)
		*header |= 0x400000;
	if (_house_owner_iid)
		*header |= 0x2000000;
	if (_pscript)
		*header |= 0x8000000;
	if (_db)
		*header |= 0x4000000;
	if (_hook_type)
		*header |= 0x10000000;
	if (_hook_item_types)
		*header |= 0x20000000;
	if (_monarch)
		*header |= 0x40;
	if (_iconOverlayID != 0)
		*header |= 0x40000000;
	if (_material_type)
		*header |= 0x80000000;
}

BOOL PublicWeenieDesc::IsTalkable(ITEM_TYPE _itemType)
{
	return _itemType == TYPE_CREATURE;
}

void PublicWeenieDesc::SetPlayerKillerStatus(unsigned int pk)
{
	switch (pk)
	{
	case 4:
		_bitfield = (_bitfield & 0xFDDFFFFF) | 0x20;
		break;
	case 0x40:
		_bitfield = (_bitfield & 0xFFDFFFDF) | 0x2000000;
		break;
	case 0x20:
		_bitfield = (_bitfield & 0xFDFFFFDF) | 0x200000;
		break;
	default:
		_bitfield &= 0xFDDFFFDF;
		break;
	}
}

#if !PHATSDK_USE_WEENIE_STUB

uint32_t PublicWeenieDesc::CalculateBitfieldFromWeenie(CWeenieObject *weenie)
{
	uint32_t bitfield = CalculateBitfieldFromQualities(&weenie->m_Qualities);

	if (weenie->AsPlayer())
	{
		bitfield |= BF_PLAYER; //  | BF_PLAYER_KILLER | BF_ADMIN;

		if (weenie->IsPK())
		{
			bitfield |= BF_PLAYER_KILLER;
		}

		if (weenie->IsPKLite())
		{
			bitfield |= BF_PKLITE_PKSTATUS;
		}

#if PHATSDK_IS_SERVER
		if ((g_pConfig->ColoredSentinels() && weenie->IsSentinel()) || weenie->IsAdmin())
		{
			bitfield |= BF_ADMIN;
		}
#else
		bitfield |= BF_ADMIN;
#endif
	}

	return bitfield;
}

uint32_t PublicWeenieDesc::CalculateBitfieldFromQualities(CACQualities *qualities)
{
	uint32_t bitfield = 0;

	switch (qualities->m_WeenieType)
	{
	case Vendor_WeenieType:
		bitfield |= BF_VENDOR;
		break;
	case Book_WeenieType:
		bitfield |= BF_BOOK;
		break;
	case Admin_WeenieType:
		bitfield |= BF_ADMIN;
		break;
	case Door_WeenieType:
		bitfield |= BF_DOOR;
		break;
	case LifeStone_WeenieType:
		bitfield |= BF_LIFESTONE;
		break;
	case AllegianceBindstone_WeenieType:
		bitfield |= BF_BINDSTONE;
		break;
	case Corpse_WeenieType:
		bitfield |= BF_CORPSE;
		break;
	case PKModifier_WeenieType:
		bitfield |= BF_PKSWITCH;
		break;
	case Portal_WeenieType:
		bitfield |= BF_PORTAL;
		break;
	case Healer_WeenieType:
		bitfield |= BF_HEALER;
		break;
	case Lockpick_WeenieType:
		bitfield |= BF_LOCKPICK;
		break;
	case Food_WeenieType:
		bitfield |= BF_FOOD;
		break;
	}

	if ((qualities->GetInt(ITEM_TYPE_INT, 0) & TYPE_CONTAINER) && qualities->GetBool(LOCKED_BOOL, FALSE) == FALSE)
		bitfield |= BF_OPENABLE;
	if (qualities->GetBool(INSCRIBABLE_BOOL, FALSE))
		bitfield |= BF_INSCRIBABLE;
	if (qualities->GetBool(STUCK_BOOL, FALSE))
		bitfield |= BF_STUCK;
	if (qualities->GetBool(ATTACKABLE_BOOL, TRUE))
		bitfield |= BF_ATTACKABLE;
	if (qualities->GetBool(HIDDEN_ADMIN_BOOL, FALSE))
		bitfield |= BF_HIDDEN_ADMIN;
	if (qualities->GetBool(UI_HIDDEN_BOOL, FALSE))
		bitfield |= BF_UI_HIDDEN;

	// PK status
	if (qualities->GetInt(PLAYER_KILLER_STATUS_INT, Undef_PKStatus) == PK_PKStatus)
		bitfield |= BF_PLAYER_KILLER;
	if (qualities->GetInt(PLAYER_KILLER_STATUS_INT, Undef_PKStatus) == PKLite_PKStatus)
		bitfield |= BF_PKLITE_PKSTATUS;
	if (qualities->GetInt(PLAYER_KILLER_STATUS_INT, Undef_PKStatus) == Free_PKStatus)
		bitfield |= BF_FREE_PKSTATUS;

	if (qualities->GetBool(IGNORE_HOUSE_BARRIERS_BOOL, FALSE)) // not sure about this
		bitfield |= BF_IMMUNE_CELL_RESTRICTIONS;
	if (qualities->GetBool(REQUIRES_BACKPACK_SLOT_BOOL, FALSE))
		bitfield |= BF_REQUIRES_PACKSLOT;
	if (qualities->GetBool(RETAINED_BOOL, FALSE))
		bitfield |= BF_RETAINED;
	if (qualities->GetBool(RARE_USES_TIMER_BOOL, FALSE))
		bitfield |= BF_VOLATILE_RARE;
	if (qualities->GetBool(WIELD_ON_USE_BOOL, FALSE))
		bitfield |= BF_WIELD_ON_USE;
	if (qualities->GetBool(AUTOWIELD_LEFT_BOOL, FALSE))
		bitfield |= BF_WIELD_LEFT;

	return bitfield;
}

PublicWeenieDesc *PublicWeenieDesc::CreateFromWeenie(CWeenieObject *weenie)
{
	PublicWeenieDesc *desc = CreateFromQualities(&weenie->m_Qualities);
	desc->_bitfield = CalculateBitfieldFromWeenie(weenie);
	
	desc->_itemsCapacity = max(weenie->InqIntQuality(ITEMS_CAPACITY_INT, 0), 0);
	desc->_containersCapacity = max(weenie->InqIntQuality(CONTAINERS_CAPACITY_INT, 0), 0);
	desc->_containerID = weenie->GetContainerID();

	return desc;
}

PublicWeenieDesc *PublicWeenieDesc::CreateFromQualities(CACQualities *qualities)
{
	PublicWeenieDesc *desc = new PublicWeenieDesc();	

	desc->_wcid = qualities->id;
	desc->_bitfield = CalculateBitfieldFromQualities(qualities);	
	desc->_name = qualities->GetString(NAME_STRING, std::string());
	desc->_iconID = qualities->GetDID(ICON_DID, 0);
	desc->_type = (ITEM_TYPE) qualities->GetInt(ITEM_TYPE_INT, 0);
	desc->_plural_name = qualities->GetString(PLURAL_NAME_STRING, std::string());
	desc->_itemsCapacity = qualities->GetInt(ITEMS_CAPACITY_INT, 0);
	desc->_containersCapacity = qualities->GetInt(CONTAINERS_CAPACITY_INT, 0);
	desc->_ammoType = (AMMO_TYPE) qualities->GetInt(AMMO_TYPE_INT, 0);
	desc->_value = qualities->GetInt(VALUE_INT, 0);
	desc->_useability = (ITEM_USEABLE)qualities->GetInt(ITEM_USEABLE_INT, 0);
	desc->_useRadius = qualities->GetFloat(USE_RADIUS_FLOAT, 0.0f);
	desc->_targetType = (ITEM_TYPE) qualities->GetInt(TARGET_TYPE_INT, 0);
	desc->_effects = qualities->GetInt(UI_EFFECTS_INT, 0);
	desc->_combatUse = (COMBAT_USE) qualities->GetInt(COMBAT_USE_INT, 0);
	desc->_structure = qualities->GetInt(STRUCTURE_INT, 0);
	desc->_maxStructure = qualities->GetInt(MAX_STRUCTURE_INT, 0);
	desc->_stackSize = qualities->GetInt(STACK_SIZE_INT, 0);
	desc->_maxStackSize = qualities->GetInt(MAX_STACK_SIZE_INT, 0);
	desc->_containerID = qualities->GetIID(CONTAINER_IID, 0);
	desc->_wielderID = qualities->GetIID(WIELDER_IID, 0);
	desc->_valid_locations = qualities->GetInt(LOCATIONS_INT, 0);
	desc->_location = qualities->GetInt(CURRENT_WIELDED_LOCATION_INT, 0);
	desc->_priority = qualities->GetInt(CLOTHING_PRIORITY_INT, 0);
	desc->_blipColor = qualities->GetInt(RADARBLIP_COLOR_INT, 0);
	desc->_radar_enum = (RadarEnum)qualities->GetInt(SHOWABLE_ON_RADAR_INT, 0);
	desc->_pscript = (PScriptType) qualities->GetDID(RESTRICTION_EFFECT_DID, 0);
	//calculate here for Salvage Workmanship Bags
	if (qualities->GetInt(ITEM_TYPE_INT, 0) == ITEM_TYPE::TYPE_TINKERING_MATERIAL)
	{
		desc->_workmanship = (float)qualities->GetInt(ITEM_WORKMANSHIP_INT, 0) / (float)qualities->GetInt(NUM_ITEMS_IN_MATERIAL_INT, 0);
	}
	else if (qualities->GetInt(ITEM_WORKMANSHIP_INT, 0) > 0)
	{
		desc->_workmanship = (float)qualities->GetInt(ITEM_WORKMANSHIP_INT, 0);
	}
	desc->_burden = qualities->GetInt(ENCUMB_VAL_INT, 0);
	desc->_spellID = qualities->GetDID(SPELL_DID, 0); // questionable
	desc->_house_owner_iid = qualities->GetIID(HOUSE_OWNER_IID, 0);
	desc->_hook_item_types = (ITEM_TYPE) qualities->GetInt(HOOK_ITEM_TYPE_INT, 0);
	desc->_monarch = qualities->GetIID(MONARCH_IID, 0);
	desc->_hook_type = qualities->GetInt(HOOK_TYPE_INT, 0);
	desc->_iconOverlayID = qualities->GetDID(ICON_OVERLAY_DID, 0);
	desc->_iconUnderlayID = qualities->GetDID(ICON_UNDERLAY_DID, 0);
	desc->_material_type = qualities->GetInt(MATERIAL_TYPE_INT, 0);
	desc->_cooldown_id = qualities->GetInt(SHARED_COOLDOWN_INT, 0);
	desc->_cooldown_duration = qualities->GetFloat(COOLDOWN_DURATION_FLOAT, 0);
	desc->_pet_owner = qualities->GetIID(PET_OWNER_IID, 0);

	return desc;
}

#endif

RestrictionDB::RestrictionDB()
{
}

DEFINE_PACK(RestrictionDB)
{
	pWriter->Write<uint32_t>(0x10000002); // version number
	pWriter->Write<uint32_t>(_bitmask);
	pWriter->Write<uint32_t>(_monarch_iid);
	_table.Pack(pWriter);
}

DEFINE_UNPACK(RestrictionDB)
{
	pReader->Read<uint32_t>(); // 0x10000002
	_bitmask = pReader->Read<uint32_t>();
	_monarch_iid = pReader->Read<uint32_t>();
	_table.UnPack(pReader);

	return true;
}

#endif
