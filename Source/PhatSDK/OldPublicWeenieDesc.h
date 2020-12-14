
#pragma once

#include "Packable.h"
#include "GameEnums.h"

class WeenieDesc : public PackObj
{
public:
};

class OldPublicWeenieDesc : public WeenieDesc
{
public:
	OldPublicWeenieDesc();
	virtual ~OldPublicWeenieDesc() override;

	DECLARE_PACKABLE();

	void Reset();

	std::string _name;
	std::string _plural_name;
	uint32_t _wcid;
	uint32_t _iconID;
	uint32_t _iconOverlayID;
	unsigned int _containerID;
	unsigned int _wielderID;
	unsigned int _priority;
	LocationMask _valid_locations; // mask INVENTORY_LOC
	LocationId _location; // mask of INVENTORY_LOC
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
	float _obvious_distance;
	uint32_t _vndwcid;
	unsigned int _spellID;
	unsigned int _house_owner_iid;
	class RestrictionDB *_db;
	PScriptType _pscript;
	unsigned int _hook_type;
	ITEM_TYPE _hook_item_types;
	unsigned int _monarch;
	int _material_type;
};

