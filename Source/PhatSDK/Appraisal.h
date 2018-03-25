
#pragma once

#include "Packable.h"
#include "SmartArray.h"
#include "GameEnums.h"

class ArmorProfile : public PackObj
{
public:
	DECLARE_PACKABLE();

	float mod_vs_slash = 0.0f;
	float mod_vs_pierce = 0.0f;
	float mod_vs_bludgeon = 0.0f;
	float mod_vs_cold = 0.0f;
	float mod_vs_fire = 0.0f;
	float mod_vs_acid = 0.0f;
	float mod_vs_electric = 0.0f;
	float mod_vs_nether = 0.0f;
};

class WeaponProfile : public PackObj
{
public:
	DECLARE_PACKABLE();

	DAMAGE_TYPE damage_type = DAMAGE_TYPE::UNDEF_DAMAGE_TYPE;
	unsigned int weapon_skill = 0;
	int weapon_time = 0;
	int weapon_damage = 0;
	long double damage_variance = 0.25;
	long double damage_mod = 1.0;
	long double weapon_length = 0;
	long double max_velocity = 0;
	long double weapon_offense = 1.0;
	int max_velocity_estimated = 0;
};

class HookAppraisalProfile : public PackObj
{
public:
	DECLARE_PACKABLE();

	unsigned int mBitfield = 0;
	unsigned int mValidLocations = 0;
	AMMO_TYPE mAmmoType = AMMO_TYPE::AMMO_NONE;
};

class CreatureAppraisalProfile : public PackObj
{
public:
	CreatureAppraisalProfile() { }
	virtual ~CreatureAppraisalProfile() { }

	DECLARE_PACKABLE();

	enum Enchantment_BFIndex
	{
		BF_STRENGTH = 0x1,
		BF_ENDURANCE = 0x2,
		BF_QUICKNESS = 0x4,
		BF_COORDINATION = 0x8,
		BF_FOCUS = 0x10,
		BF_SELF = 0x20,
		BF_MAX_HEALTH = 0x40,
		BF_MAX_STAMINA = 0x80,
		BF_MAX_MANA = 0x100,
		BF_STRENGTH_HI = 0x10000,
		BF_ENDURANCE_HI = 0x20000,
		BF_QUICKNESS_HI = 0x40000,
		BF_COORDINATION_HI = 0x80000,
		BF_FOCUS_HI = 0x100000,
		BF_SELF_HI = 0x200000,
		BF_MAX_HEALTH_HI = 0x400000,
		BF_MAX_STAMINA_HI = 0x800000,
		BF_MAX_MANA_HI = 0x1000000,
		FORCE_Enchantment_BFIndex_32_BIT = 0x7FFFFFFF,
	};

	DWORD strength = 0;
	DWORD endurance = 0;
	DWORD quickness = 0;
	DWORD coordination = 0;
	DWORD focus = 0;
	DWORD self = 0;
	DWORD health = 0;
	DWORD stamina = 0;
	DWORD mana = 0;
	DWORD max_health = 0;
	DWORD max_stamina = 0;
	DWORD max_mana = 0;
	DWORD enchantment_bitfield = 0;
};

class AppraisalProfile : public PackObj
{
public:
	AppraisalProfile();
	virtual ~AppraisalProfile();

	DECLARE_PACKABLE();

	int success_flag = 0;
	class CreatureAppraisalProfile * creature_profile = NULL; // 0x08
	class HookAppraisalProfile * hook_profile = NULL; // 0x0C
	class WeaponProfile * weapon_profile = NULL; // 0x10
	class ArmorProfile * armor_profile = NULL; // 0x14
	class PackableHashTableWithJson<STypeInt, int> * _intStatsTable = NULL; // 0x18
	class PackableHashTableWithJson<STypeInt64, __int64> * _int64StatsTable = NULL; // 0x1C
	class PackableHashTableWithJson<STypeBool, int> * _boolStatsTable = NULL; // 0x20
	class PackableHashTableWithJson<STypeFloat, double> * _floatStatsTable = NULL; // 0x24
	class PackableHashTableWithJson<STypeString, std::string> * _strStatsTable = NULL; // 0x28
	class PackableHashTableWithJson<STypeDID, DWORD> * _didStatsTable = NULL; // 0x2C
	class SmartArray<DWORD> *_spellBook = NULL; // 0x30 actually a PSmartArray

	unsigned long armor_ench_bitfield = 0;
	unsigned long weapon_ench_bitfield = 0;
	unsigned long resist_ench_bitfield = 0;

	long base_armor_head = 0;
	long base_armor_chest = 0;
	long base_armor_groin = 0;
	long base_armor_bicep = 0;
	long base_armor_wrist = 0;
	long base_armor_hand = 0;
	long base_armor_thigh = 0;
	long base_armor_shin = 0;
	long base_armor_foot = 0;
};

