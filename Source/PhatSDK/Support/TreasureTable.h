
#pragma once

#include "Packable.h"
#include "PackableJson.h"
#include "WClassID.h"
#include "TreasureFactory.h"

class TreasureEquipment : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	WClassIDEnum _wcid = WClassIDEnum::W_UNDEF_CLASS;
	uint32_t _ptid = 0;
	uint32_t m_08_AlwaysZero = 0; // always zero
	float _shade = 0.0f;
	uint32_t _amount = 0;
	float _amountVariance = 0.0f;
	float _chance = 0.0f;
	uint32_t m_1C_AlwaysZero = 0; // always zero
	uint32_t m_20_AlwaysZero = 0; // always zero
	uint32_t m_24_AlwaysZero = 0; // always zero
	BOOL _setStart = 0;
	BOOL _hasSubSet = 0;
	BOOL _continuesPreviousSet = 0;
	uint32_t m_34_AlwaysZero = 0; // always zero
	uint32_t m_38_AlwaysZero = 0; // always zero
	uint32_t m_3C_AlwaysZero = 0; // always zero
	uint32_t m_40_AlwaysZero = 0; // always zero
};
using treasure_equipment_list_t = PackableListWithJson<TreasureEquipment>;

class TreasureLoot : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	uint32_t _tier = 0;
	float _lootQualityMod = 0.0f;
	uint32_t _unknownChances = 0; //13, 14, 15, 16, 17, 18, 19  - treasure3[3] = 3 chance floats that add up to 1?

	uint32_t _itemChance = 0; //0, 100
	uint32_t _itemMinAmount = 0;
	uint32_t _itemMaxAmount = 0;
	uint32_t _itemTreasureTypeSelectionChances = 0; //1, 2, 4, 8, 8, 9, 10, 11 - treasure3[0]

	uint32_t _magicItemChance = 0; //0, 100
	uint32_t _magicItemMinAmount = 0;
	uint32_t _magicItemMaxAmount = 0;
	uint32_t _magicItemTreasureTypeSelectionChances = 0; //1, 2, 3, 6, 8, 9, 10, 11 - treasure3[1]

	uint32_t _mundaneItemChance = 0; //chance 0, 100
	uint32_t _mundaneItemMinAmount = 0;
	uint32_t _mundaneItemMaxAmount = 0;
	uint32_t _mundaneItemTypeSelectionChances = 0; //1, 4, 5, 7, 8 - treasure3[2]

	//	treasure3[0] - 1=0 2=1 3=2 4=3 5=4 6=5 7=6
	//	8 = all equal
	//	9 = all(0, 1 high) (2 medium) (3, 4, 5, 6 low)
	//	10 = 0, 1, 2
	//	11 = 3, 4, 5, 6

	//	treasure3[1] - 1=0 2=1 3=2 4=3 5=4 6=5 7=6
	//	8 = all equal
	//	9 = all(0, 1 high) (2, 3, 4 medium) (5, 6 low)
	//	10 = 0, 1, 2
	//	11 = 3, 4, 5, 6
	//	12 = 2, 4, 5, 6

	//  For both tables above:
	//	0 - 5 = melee?
	//	1 - 6 = armor?
	//	2 - 8 = missile?
	//	3 - 7 = scroll?
	//	4 - 3 = caster?
	//	5 - 2 = jewelry?
	//	6 - 4 = clothing?

	//	treasure3[2] - 1=1 2=2 3=3 4=4 5=5 6=0 7=all 8=0+4+5
	//	0 - 1 = gem?
	//	1 - 11 = consumable?
	//	2 - 12 = healer?
	//	3 - 13 = lockpick?
	//	4 - 14 = pea?
	//	5 - 10 = mana stone?
};

class TreasureEntry3 : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	uint32_t m_00 = 0;
	uint32_t m_04 = 0;
	uint32_t m_08 = 0;
	uint32_t m_0C = 0;
	uint32_t m_10 = 0;
	uint32_t m_14 = 0;
	uint32_t m_18 = 0;
	uint32_t m_1C = 0;
	uint32_t m_20 = 0;
	uint32_t m_24 = 0;
	uint32_t m_28 = 0;
	uint32_t m_2C = 0;
	uint32_t m_30 = 0;
	uint32_t m_34 = 0;
	uint32_t m_38 = 0;
	uint32_t m_3C = 0;
	uint32_t m_40 = 0;
	uint32_t m_44 = 0;
	uint32_t m_48 = 0;
	uint32_t m_4C = 0;
	uint32_t m_50 = 0;
	uint32_t m_54 = 0;
};

class TreasureEntry4 : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	uint32_t m_00 = 0;
	uint32_t m_04 = 0;
	uint32_t m_08 = 0;
	uint32_t m_0C = 0;
	uint32_t m_10 = 0;
	uint32_t m_14 = 0;
	uint32_t m_18 = 0;
	uint32_t m_1C = 0;
	uint32_t m_20 = 0;
	uint32_t m_24 = 0;
	uint32_t m_28 = 0;
	uint32_t m_2C = 0;
	uint32_t m_30 = 0;
	uint32_t m_34 = 0;
	uint32_t m_38 = 0;
	uint32_t m_3C = 0;
	uint32_t m_40 = 0;
	uint32_t m_44 = 0;
	uint32_t m_48 = 0;
};

class TreasureChance : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	union {
		uint32_t slot;
		WClassIDEnum wcid;
		SpellID spell;
	};
	double chance = 0.0;
};
using treasure_chance_list_t = PackableListWithJson<TreasureChance>;
using treasure_chance_list_map_t = PackableHashTableWithJson<uint32_t, treasure_chance_list_t>;

class TreasureEntry6 : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	std::vector< float > chances;
	//float chances[6];
};

class TreasureTiers : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	std::vector< treasure_chance_list_t > m_entries;
};

class TreasureTable : public PackObj, public PackableJson
{
public:
	TreasureTable();
	virtual ~TreasureTable();

	DECLARE_PACKABLE();
	DECLARE_PACKABLE_JSON();

	TreasureLoot *GetTreasureGenerationProfile(uint32_t treasureId);
	eTreasureCategory RollTreasureCategory(uint32_t tableId, uint32_t selectionChanceId);

	int GetLootTierForTreasureEntry(uint32_t treasure_id);
	uint32_t RollRandomSlotFromList(const treasure_chance_list_t& list);
	uint32_t RollRandomScrollWCID(int tier);
	MaterialType RollBaseMaterialFromMaterialCode(int materialCode, int tier);
	MaterialType RollMaterialFromBaseMaterial(MaterialType baseMaterial, int tier);
	int RollPaletteTemplateIDFromMaterialAndColorCode(MaterialType baseMaterial, int colorCode);
	float RollValueEnchantmentForMaterial(MaterialType mat, int tier);

	uint32_t RollHealer(int tier);
	uint32_t RollLockpick(int tier);
	uint32_t RollConsumable(int tier);
	uint32_t RollPea(int tier);
	uint32_t RollScroll(int tier);
	uint32_t RollManaStone(int tier);
	uint32_t RollItem(int tier);
	int RollWorkmanship(int tier);

	uint32_t RollSpell(int spellCode, int tier);
	uint32_t RollCantrip(int tier);

	std::list<uint32_t> RollArmorSpells(int spellCode, int tier);
	std::list<uint32_t> RollMeleeWeaponSpells(int spellCode, int tier);
	std::list<uint32_t> RollMissileWeaponSpells(int spellCode, int tier);
	std::list<uint32_t> RollCasterSpells(int spellCode, int tier);
	uint32_t RollCasterBuffSpell(int spellCode, int tier);
	uint32_t RollCasterWarSpell(int spellCode, int tier);

	PackableHashTableWithJson<uint32_t, treasure_equipment_list_t> _treasureEquipment; // hashA
	PackableHashTableWithJson<uint32_t, TreasureLoot> _treasureLoot; // hashB
	treasure_chance_list_map_t _treasure3[4]; // hashC x 4
	TreasureTiers _treasure7[48]; // listyA x 48
	treasure_chance_list_map_t _treasure8; // hash D
	
	TreasureTiers _healerChances;
	TreasureTiers _lockpickChances;
	TreasureTiers _consumableChances;
	TreasureTiers _peaChances;

	PackableHashTableWithJson<SpellID, TreasureEntry6> _itemBaneSpells;
	PackableHashTableWithJson<SpellID, TreasureEntry6> _meleeWeaponSpells;
	PackableHashTableWithJson<SpellID, TreasureEntry6> _missileWeaponSpells;
	PackableHashTableWithJson<SpellID, TreasureEntry6> _casterWeaponSpells;

	TreasureTiers _spellLevelProbabilitiesMaybe; // listyA
	PackableListWithJson<treasure_chance_list_t> _keyedSpells; // listyBcount + listyB

	TreasureTiers _otherBuffCasterSpell;
	TreasureTiers _otherWarCasterSpell;

	TreasureTiers _scrollChances; // listyA
	TreasureTiers _manaStoneChances; // listyA
	PackableHashTableWithJson<uint32_t, TreasureTiers> _materialCodeToBaseMaterialMap; // hash F

	TreasureTiers _ceramicMaterials;
	TreasureTiers _clothMaterials;
	TreasureTiers _leatherMaterials;
	TreasureTiers _metalMaterials;
	TreasureTiers _stoneMaterials;
	TreasureTiers _woodMaterials;

	TreasureTiers _workmanshipChance;

	TreasureTiers _treasure18; // listyA x 8

	PackableHashTableWithJson<uint32_t, TreasureEntry6> _treasure19; // hash G
	PackableHashTableWithJson<uint32_t, TreasureTiers> _treasure20; // hashF
	TreasureTiers _gemProbabilitiesMaybe; // listyA
	treasure_chance_list_map_t _gemMaterials; // hashD
	PackableHashTableWithJson<uint32_t, float> _materialValueAddedPossibly; // hashH

	treasure_chance_list_t _treasure24;
	treasure_chance_list_t _treasure25;
	treasure_chance_list_t _treasure26;
	
	PackableHashTableWithJson<uint32_t, treasure_chance_list_map_t> _materialColorKeyMap;

	// 0 - armor
	// 1 - weapons (melee)
	// 2 - shield
	// 3 - missile
	// 4 - casters
	TreasureTiers _cantripsArmor;
	TreasureTiers _cantripsMelee;
	TreasureTiers _cantripsShield;
	TreasureTiers _cantripsMissile;
	TreasureTiers _cantripsCaster;

	/*
	HashA
	HashB
	HashC x 4
	ListyA x 48
	HashD
	ListyA x 4
	HashE x 4
	ListyA
	ListyBCount
	ListyBs
	ListyA x 4
	HashF
	ListyA x 8
	HashG
	HashF
	ListyA
	HashD
	HashH
	ListyB x 3
	HashI
	ListyA x 5
	*/
};

class MaterialTypeEnumMapper
{
public:
	static BOOL MaterialTypeToString(unsigned int ct, std::string &materialName);
};

