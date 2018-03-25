#pragma once

#include "BinaryReader.h"
#include "OldPublicWeenieDesc.h"
#include "Packable.h"

class RestrictionDB : public PackObj
{
public:
	RestrictionDB();

	DECLARE_PACKABLE()

	DWORD _bitmask = 0;
	DWORD _monarch_iid = 0;
	PackableHashTable<DWORD, DWORD> _table;
};

class PublicWeenieDesc : public WeenieDesc
{
public:
	enum BitfieldIndex
	{
		BF_OPENABLE = 0x1,
		BF_INSCRIBABLE = 0x2,
		BF_STUCK = 0x4,
		BF_PLAYER = 0x8,
		BF_ATTACKABLE = 0x10,
		BF_PLAYER_KILLER = 0x20,
		BF_HIDDEN_ADMIN = 0x40,
		BF_UI_HIDDEN = 0x80,
		BF_BOOK = 0x100,
		BF_VENDOR = 0x200,
		BF_PKSWITCH = 0x400,
		BF_NPKSWITCH = 0x800,
		BF_DOOR = 0x1000,
		BF_CORPSE = 0x2000,
		BF_LIFESTONE = 0x4000,
		BF_FOOD = 0x8000,
		BF_HEALER = 0x10000,
		BF_LOCKPICK = 0x20000,
		BF_PORTAL = 0x40000,
		BF_ADMIN = 0x100000,
		BF_FREE_PKSTATUS = 0x200000,
		BF_IMMUNE_CELL_RESTRICTIONS = 0x400000,
		BF_REQUIRES_PACKSLOT = 0x800000,
		BF_RETAINED = 0x1000000,
		BF_PKLITE_PKSTATUS = 0x2000000,
		BF_INCLUDES_SECOND_HEADER = 0x4000000,
		BF_BINDSTONE = 0x8000000,
		BF_VOLATILE_RARE = 0x10000000,
		BF_WIELD_ON_USE = 0x20000000,
		BF_WIELD_LEFT = 0x40000000,
		FORCE_BitfieldIndex_32_BIT = 0x7FFFFFFF,
	};

	PublicWeenieDesc();
	virtual ~PublicWeenieDesc();

	DECLARE_PACKABLE();
	
	void Reset();
	void set_pack_header(DWORD *header);

	static BOOL IsTalkable(ITEM_TYPE _itemType);
	void SetPlayerKillerStatus(unsigned int pk);

#if !PHATSDK_USE_WEENIE_STUB
	// custom
	static DWORD CalculateBitfieldFromWeenie(CWeenieObject *weenie);
	static DWORD CalculateBitfieldFromQualities(CACQualities *qualities);
	static PublicWeenieDesc *CreateFromWeenie(CWeenieObject *weenie);
	static PublicWeenieDesc *CreateFromQualities(CACQualities *qualities);
	// end of custom
#endif

	std::string _name;
	std::string _plural_name;
	DWORD _wcid;
	DWORD _iconID;
	DWORD _iconOverlayID;
	DWORD _iconUnderlayID;
	unsigned int _containerID;
	unsigned int _wielderID;
	unsigned int _priority;
	unsigned int _valid_locations;
	unsigned int _location;
	int _itemsCapacity;
	int _containersCapacity;
	ITEM_TYPE _type;
	unsigned int _value;
	ITEM_USEABLE _useability;
	float _useRadius;
	ITEM_TYPE _targetType;
	unsigned int _effects;
	AMMO_TYPE _ammoType;
	COMBAT_USE _combatUse;
	unsigned int _structure;
	unsigned int _maxStructure;
	unsigned int _stackSize;
	unsigned int _maxStackSize;
	unsigned int _bitfield;
	int _blipColor;
	RadarEnum _radar_enum;
	int _burden;
	unsigned int _spellID;
	unsigned int _house_owner_iid;
	RestrictionDB *_db;
	PScriptType _pscript; // RESTRICTION_EFFECT_DID
	unsigned int _hook_type;
	ITEM_TYPE _hook_item_types;
	unsigned int _monarch;
	int _material_type;
	float _workmanship;
	int _cooldown_id;
	long double _cooldown_duration;
	unsigned int _pet_owner;
};

