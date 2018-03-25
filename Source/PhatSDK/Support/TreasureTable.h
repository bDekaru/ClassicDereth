
#pragma once

#include "Packable.h"
#include "WClassID.h"
#include "TreasureFactory.h"

class TreasureEntry : public PackObj
{
public:
	DECLARE_PACKABLE()

	WClassIDEnum _wcid = WClassIDEnum::W_UNDEF_CLASS;
	DWORD _ptid = 0;
	DWORD m_08_AlwaysZero = 0; // always zero
	float _shade = 0.0f;
	DWORD _amount = 0;
	float _amountVariance = 0.0f;
	float _chance = 0.0f;
	DWORD m_1C_AlwaysZero = 0; // always zero
	DWORD m_20_AlwaysZero = 0; // always zero
	DWORD m_24_AlwaysZero = 0; // always zero
	BOOL _setStart = 0;
	BOOL _hasSubSet = 0;
	BOOL _continuesPreviousSet = 0;
	DWORD m_34_AlwaysZero = 0; // always zero
	DWORD m_38_AlwaysZero = 0; // always zero
	DWORD m_3C_AlwaysZero = 0; // always zero
	DWORD m_40_AlwaysZero = 0; // always zero
};

class TreasureEntry2 : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD _tier = 0;
	float _lootQualityMod = 0.0f;
	DWORD _unknownChances = 0; //13, 14, 15, 16, 17, 18, 19  - treasure3[3] = 3 chance floats that add up to 1?

	DWORD _itemChance = 0; //0, 100
	DWORD _itemMinAmount = 0;
	DWORD _itemMaxAmount = 0;
	DWORD _itemTreasureTypeSelectionChances = 0; //1, 2, 4, 8, 8, 9, 10, 11 - treasure3[0]

	DWORD _magicItemChance = 0; //0, 100
	DWORD _magicItemMinAmount = 0;
	DWORD _magicItemMaxAmount = 0;
	DWORD _magicItemTreasureTypeSelectionChances = 0; //1, 2, 3, 6, 8, 9, 10, 11 - treasure3[1]

	DWORD _mundaneItemChance = 0; //chance 0, 100
	DWORD _mundaneItemMinAmount = 0;
	DWORD _mundaneItemMaxAmount = 0;
	DWORD _mundaneItemTypeSelectionChances = 0; //1, 4, 5, 7, 8 - treasure3[2]

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

class TreasureEntry3 : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD m_00 = 0;
	DWORD m_04 = 0;
	DWORD m_08 = 0;
	DWORD m_0C = 0;
	DWORD m_10 = 0;
	DWORD m_14 = 0;
	DWORD m_18 = 0;
	DWORD m_1C = 0;
	DWORD m_20 = 0;
	DWORD m_24 = 0;
	DWORD m_28 = 0;
	DWORD m_2C = 0;
	DWORD m_30 = 0;
	DWORD m_34 = 0;
	DWORD m_38 = 0;
	DWORD m_3C = 0;
	DWORD m_40 = 0;
	DWORD m_44 = 0;
	DWORD m_48 = 0;
	DWORD m_4C = 0;
	DWORD m_50 = 0;
	DWORD m_54 = 0;
};

class TreasureEntry4 : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD m_00 = 0;
	DWORD m_04 = 0;
	DWORD m_08 = 0;
	DWORD m_0C = 0;
	DWORD m_10 = 0;
	DWORD m_14 = 0;
	DWORD m_18 = 0;
	DWORD m_1C = 0;
	DWORD m_20 = 0;
	DWORD m_24 = 0;
	DWORD m_28 = 0;
	DWORD m_2C = 0;
	DWORD m_30 = 0;
	DWORD m_34 = 0;
	DWORD m_38 = 0;
	DWORD m_3C = 0;
	DWORD m_40 = 0;
	DWORD m_44 = 0;
	DWORD m_48 = 0;
};

class TreasureEntry5 : public PackObj
{
public:
	DECLARE_PACKABLE()

	union {
		DWORD slot;
		WClassIDEnum wcid;
		SpellID spell;
	};
	double chance = 0.0;
};

class TreasureEntry6 : public PackObj
{
public:
	DECLARE_PACKABLE()

	float chances[6];
};

class TreasureEntry7 : public PackObj
{
public:
	DECLARE_PACKABLE()

	PackableList<TreasureEntry5> m_entries[6];
};

class TreasureTable : public PackObj
{
public:
	TreasureTable();
	virtual ~TreasureTable();

	DECLARE_PACKABLE()

	TreasureEntry2 *GetTreasureGenerationProfile(DWORD treasureId);
	eTreasureCategory RollTreasureCategory(DWORD tableId, DWORD selectionChanceId);

	int GetLootTierForTreasureEntry(DWORD treasure_id);
	DWORD RollRandomSlotFromList(const PackableList<TreasureEntry5>& list);
	DWORD RollRandomScrollWCID(int tier);
	MaterialType RollBaseMaterialFromMaterialCode(int materialCode, int tier);
	MaterialType RollMaterialFromBaseMaterial(MaterialType baseMaterial, int tier);
	int RollPaletteTemplateIDFromMaterialAndColorCode(MaterialType baseMaterial, int colorCode);
	float RollValueEnchantmentForMaterial(MaterialType mat, int tier);

	DWORD RollHealer(int tier);
	DWORD RollLockpick(int tier);
	DWORD RollConsumable(int tier);
	DWORD RollPea(int tier);
	DWORD RollScroll(int tier);
	DWORD RollManaStone(int tier);
	DWORD RollItem(int tier);
	int RollWorkmanship(int tier);

	DWORD RollSpell(int spellCode, int tier);
	DWORD RollCantrip(int tier);

	std::list<DWORD> RollArmorSpells(int spellCode, int tier);
	std::list<DWORD> RollMeleeWeaponSpells(int spellCode, int tier);
	std::list<DWORD> RollMissileWeaponSpells(int spellCode, int tier);
	std::list<DWORD> RollCasterSpells(int spellCode, int tier);
	DWORD RollCasterBuffSpell(int spellCode, int tier);
	DWORD RollCasterWarSpell(int spellCode, int tier);

	PackableHashTable<DWORD, PackableList<TreasureEntry>> _treasureList; // hashA
	PackableHashTable<DWORD, TreasureEntry2> _treasureGenerationProfiles; // hashB
	PackableHashTable<DWORD, PackableList<TreasureEntry5>> _treasure3[4]; // hashC x 4
	TreasureEntry7 _treasure7[48]; // listyA x 48
	PackableHashTable<DWORD, PackableList<TreasureEntry5>> _treasure8; // hash D
	
	TreasureEntry7 _healerChances;
	TreasureEntry7 _lockpickChances;
	TreasureEntry7 _consumableChances;
	TreasureEntry7 _peaChances;

	PackableHashTable<SpellID, TreasureEntry6> _itemBaneSpells;
	PackableHashTable<SpellID, TreasureEntry6> _meleeWeaponSpells;
	PackableHashTable<SpellID, TreasureEntry6> _missileWeaponSpells;
	PackableHashTable<SpellID, TreasureEntry6> _casterWeaponSpells;

	TreasureEntry7 _spellLevelProbabilitiesMaybe; // listyA
	PackableList<PackableList<TreasureEntry5>> _keyedSpells; // listyBcount + listyB

	TreasureEntry7 _otherBuffCasterSpell;
	TreasureEntry7 _otherWarCasterSpell;

	TreasureEntry7 _scrollChances; // listyA
	TreasureEntry7 _manaStoneChances; // listyA
	PackableHashTable<DWORD, TreasureEntry7> _materialCodeToBaseMaterialMap; // hash F

	TreasureEntry7 _ceramicMaterials;
	TreasureEntry7 _clothMaterials;
	TreasureEntry7 _leatherMaterials;
	TreasureEntry7 _metalMaterials;
	TreasureEntry7 _stoneMaterials;
	TreasureEntry7 _woodMaterials;

	TreasureEntry7 _treasure18[2]; // listyA x 8
	PackableHashTable<DWORD, TreasureEntry6> _treasure19; // hash G
	PackableHashTable<DWORD, TreasureEntry7> _treasure20; // hashF
	TreasureEntry7 _gemProbabilitiesMaybe; // listyA
	PackableHashTable<DWORD, PackableList<TreasureEntry5>> _gemMaterials; // hashD
	PackableHashTable<DWORD, float> _materialValueAddedPossibly; // hashH
	PackableList<TreasureEntry5> _treasure24[3]; // listyB x3
	PackableHashTable<DWORD, PackableHashTable<DWORD, PackableList<TreasureEntry5>>> _materialColorKeyMap;
	TreasureEntry7 _cantripChances[5];

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

