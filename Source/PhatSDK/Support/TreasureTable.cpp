
#include "StdAfx.h"
#include "TreasureTable.h"
#include "RandomRange.h"

DEFINE_PACK(TreasureEntry)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry)
{
	_wcid = (WClassIDEnum) pReader->Read<DWORD>();
	_ptid = pReader->Read<DWORD>();
	m_08_AlwaysZero = pReader->Read<DWORD>(); // always zero
	_shade = pReader->Read<float>();
	_amount = pReader->Read<DWORD>();
	_amountVariance = pReader->Read<float>();
	_chance = pReader->Read<float>();
	m_1C_AlwaysZero = pReader->Read<DWORD>(); // always zero
	m_20_AlwaysZero = pReader->Read<DWORD>(); // always zero
	m_24_AlwaysZero = pReader->Read<DWORD>(); // always zero
	_setStart = pReader->Read<BOOL>(); // bool
	_hasSubSet = pReader->Read<BOOL>(); // bool
	_continuesPreviousSet = pReader->Read<BOOL>(); // bool
	m_34_AlwaysZero = pReader->Read<DWORD>(); // always zero
	m_38_AlwaysZero = pReader->Read<DWORD>(); // always zero
	m_3C_AlwaysZero = pReader->Read<DWORD>(); // always zero
	m_40_AlwaysZero = pReader->Read<DWORD>(); // always zero

	return true;
}

DEFINE_PACK(TreasureEntry2)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry2)
{
	_tier = pReader->Read<DWORD>();
	_lootQualityMod = pReader->Read<float>();
	_unknownChances = pReader->Read<DWORD>();
	_itemChance = pReader->Read<DWORD>();
	_itemMinAmount = pReader->Read<DWORD>();
	_itemMaxAmount = pReader->Read<DWORD>();
	_itemTreasureTypeSelectionChances = pReader->Read<DWORD>();
	_magicItemChance = pReader->Read<DWORD>();
	_magicItemMinAmount = pReader->Read<DWORD>();
	_magicItemMaxAmount = pReader->Read<DWORD>();
	_magicItemTreasureTypeSelectionChances = pReader->Read<DWORD>();
	_mundaneItemChance = pReader->Read<DWORD>();
	_mundaneItemMinAmount = pReader->Read<DWORD>();
	_mundaneItemMaxAmount = pReader->Read<DWORD>();
	_mundaneItemTypeSelectionChances = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(TreasureEntry3)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry3)
{
	m_00 = pReader->Read<DWORD>();
	m_04 = pReader->Read<DWORD>();
	m_08 = pReader->Read<DWORD>();
	m_0C = pReader->Read<DWORD>();
	m_10 = pReader->Read<DWORD>();
	m_14 = pReader->Read<DWORD>();
	m_18 = pReader->Read<DWORD>();
	m_1C = pReader->Read<DWORD>();
	m_20 = pReader->Read<DWORD>();
	m_24 = pReader->Read<DWORD>();
	m_28 = pReader->Read<DWORD>();
	m_2C = pReader->Read<DWORD>();
	m_30 = pReader->Read<DWORD>();
	m_34 = pReader->Read<DWORD>();
	m_38 = pReader->Read<DWORD>();
	m_3C = pReader->Read<DWORD>();
	m_40 = pReader->Read<DWORD>();
	m_44 = pReader->Read<DWORD>();
	m_48 = pReader->Read<DWORD>();
	m_4C = pReader->Read<DWORD>();
	m_50 = pReader->Read<DWORD>();
	m_54 = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(TreasureEntry4)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry4)
{
	m_00 = pReader->Read<DWORD>();
	m_04 = pReader->Read<DWORD>();
	m_08 = pReader->Read<DWORD>();
	m_0C = pReader->Read<DWORD>();
	m_10 = pReader->Read<DWORD>();
	m_14 = pReader->Read<DWORD>();
	m_18 = pReader->Read<DWORD>();
	m_1C = pReader->Read<DWORD>();
	m_20 = pReader->Read<DWORD>();
	m_24 = pReader->Read<DWORD>();
	m_28 = pReader->Read<DWORD>();
	m_2C = pReader->Read<DWORD>();
	m_30 = pReader->Read<DWORD>();
	m_34 = pReader->Read<DWORD>();
	m_38 = pReader->Read<DWORD>();
	m_3C = pReader->Read<DWORD>();
	m_40 = pReader->Read<DWORD>();
	m_44 = pReader->Read<DWORD>();
	m_48 = pReader->Read<DWORD>();
	return true;
}


DEFINE_PACK(TreasureEntry5)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry5)
{
	slot = pReader->Read<DWORD>();
	chance = pReader->Read<double>();
	return true;
}


DEFINE_PACK(TreasureEntry6)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry6)
{
	for (int i = 0; i < 6; i++)
		chances[i] = pReader->Read<float>();

	return true;
}

DEFINE_PACK(TreasureEntry7)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry7)
{
	for (DWORD i = 0; i < 6; i++)
		m_entries[i].UnPack(pReader);

	return true;
}


TreasureTable::TreasureTable()
{
}

TreasureTable::~TreasureTable()
{
}

DEFINE_PACK(TreasureTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureTable)
{
#ifdef PHATSDK_USE_INFERRED_SPELL_DATA
	pReader->Read<DWORD>();
#endif

	_treasureList.UnPack(pReader);	
	_treasureGenerationProfiles.UnPack(pReader);

	for (DWORD i = 0; i < 4; i++)
		_treasure3[i].UnPack(pReader);

	for (DWORD i = 0; i < 48; i++)
		_treasure7[i].UnPack(pReader);

	_treasure8.UnPack(pReader);

	_healerChances.UnPack(pReader);
	_lockpickChances.UnPack(pReader);
	_consumableChances.UnPack(pReader);
	_peaChances.UnPack(pReader);
	
	_itemBaneSpells.UnPack(pReader);
	_meleeWeaponSpells.UnPack(pReader);
	_missileWeaponSpells.UnPack(pReader);
	_casterWeaponSpells.UnPack(pReader);

	_spellLevelProbabilitiesMaybe.UnPack(pReader);
	_keyedSpells.UnPack(pReader);	

	_otherBuffCasterSpell.UnPack(pReader);
	_otherWarCasterSpell.UnPack(pReader);

	_scrollChances.UnPack(pReader);
	_manaStoneChances.UnPack(pReader);

	_materialCodeToBaseMaterialMap.UnPack(pReader);

	_ceramicMaterials.UnPack(pReader);
	_clothMaterials.UnPack(pReader);
	_leatherMaterials.UnPack(pReader);
	_metalMaterials.UnPack(pReader);
	_stoneMaterials.UnPack(pReader);
	_woodMaterials.UnPack(pReader);

	for (DWORD i = 0; i < 2; i++)
	{		
		_treasure18[i].UnPack(pReader);
	}	

	_treasure19.UnPack(pReader);
	_treasure20.UnPack(pReader);
	_gemProbabilitiesMaybe.UnPack(pReader);
	_gemMaterials.UnPack(pReader);
	_materialValueAddedPossibly.UnPack(pReader);

	for (DWORD i = 0; i < 3; i++)
		_treasure24[i].UnPack(pReader);

	_materialColorKeyMap.UnPack(pReader);

	for (DWORD i = 0; i < 5; i++)
		_cantripChances[i].UnPack(pReader);

	return true;
}

TreasureEntry2 *TreasureTable::GetTreasureGenerationProfile(DWORD treasureId)
{
	return _treasureGenerationProfiles.lookup(treasureId);
}

eTreasureCategory TreasureTable::RollTreasureCategory(DWORD tableId, DWORD selectionChanceId)
{
	if (tableId < _treasure3->size())
	{
		PackableHashTable<DWORD, PackableList<TreasureEntry5>> *table = &_treasure3[tableId];

		if (selectionChanceId < table->size())
			return (eTreasureCategory)RollRandomSlotFromList(*table->lookup(selectionChanceId));
	}
	return eTreasureCategory::TreasureCategory_Undef;
}

int TreasureTable::GetLootTierForTreasureEntry(DWORD treasure_id)
{
	auto entry = _treasureGenerationProfiles.lookup(treasure_id);

	if (!entry)
		return 0;

	return entry->_tier;
}

DWORD TreasureTable::RollRandomSlotFromList(const PackableList<TreasureEntry5>& list)
{
	float dice = getRandomDouble(0.0, 1.0); //g_pPhatSDK->GetRandomFloat(0.0, 1.0);

	for (auto &entry : list)
	{
		if (dice <= entry.chance)
		{
			return entry.slot;
		}

		dice -= entry.chance;
	}

	return 0;
}

DWORD TreasureTable::RollRandomScrollWCID(int tier)
{
	if (tier > 6)
		return 0;
	if (tier < 1)
		return 0;
	
	return RollRandomSlotFromList(_scrollChances.m_entries[tier - 1]);
}

MaterialType TreasureTable::RollBaseMaterialFromMaterialCode(int materialCode, int tier)
{
	if (tier < 1 || tier > 6)
	{
		return MaterialType::Undef_MaterialType;
	}

	auto materialCodeMap = _materialCodeToBaseMaterialMap.lookup(materialCode);

	if (materialCodeMap == nullptr)
	{
		return MaterialType::Undef_MaterialType;
	}
	
	float dice = getRandomDouble(0.0, 1.0);//g_pPhatSDK->GetRandomFloat(0.0, 1.0);

	for (auto &entry : materialCodeMap->m_entries[tier - 1])
	{
		if (dice <= entry.chance)
		{
			return (MaterialType) entry.slot;
		}

		dice -= entry.chance;
	}

	return MaterialType::Undef_MaterialType;
}

float TreasureTable::RollValueEnchantmentForMaterial(MaterialType mat, int tier)
{
	float *valAdded = _materialValueAddedPossibly.lookup(mat);

	if (valAdded)
	{
		return g_pPhatSDK->GetRandomFloat(*valAdded * 0.8, *valAdded);
	}

	return 0.0f;
}

MaterialType TreasureTable::RollMaterialFromBaseMaterial(MaterialType baseMaterial, int tier)
{
	float dice = getRandomDouble(0.0, 1.0);//g_pPhatSDK->GetRandomFloat(0.0, 1.0);

	PackableList<TreasureEntry5> *chanceList = NULL;
	TreasureEntry7 *materialMap = NULL;

	if (baseMaterial != Gem_MaterialType)
	{
		switch (baseMaterial)
		{
		case Ceramic_MaterialType:
			materialMap = &_ceramicMaterials;
			break;

		case Cloth_MaterialType:
			materialMap = &_clothMaterials;
			break;

		case Leather_MaterialType:
			materialMap = &_leatherMaterials;
			break;

		case Metal_MaterialType:
			materialMap = &_metalMaterials;
			break;

		case Stone_MaterialType:
			materialMap = &_stoneMaterials;
			break;

		case Wood_MaterialType:
			materialMap = &_woodMaterials;
			break;
		}

		if (!materialMap || tier < 1 || tier > 6)
		{
			return baseMaterial;
		}

		chanceList = &materialMap->m_entries[tier - 1];
	}
	else
	{
		auto gemMapForTier = _gemMaterials.lookup(tier);

		if (!gemMapForTier)
		{
			return baseMaterial;
		}

		chanceList = gemMapForTier;
	}
	
	for (auto &entry : *chanceList)
	{
		if (dice <= entry.chance)
		{
			return (MaterialType)entry.slot;
		}

		dice -= entry.chance;
	}

	return baseMaterial;
}

int TreasureTable::RollPaletteTemplateIDFromMaterialAndColorCode(MaterialType baseMaterial, int colorCode)
{
	auto materialColorMap = _materialColorKeyMap.lookup(baseMaterial);

	auto colorMap = materialColorMap->lookup(colorCode);

	if (!colorMap)
	{
		return PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
	}

	float dice = getRandomDouble(0.0, 1.0);//g_pPhatSDK->GetRandomFloat(0.0, 1.0);

	for (auto &entry : *colorMap)
	{
		if (dice <= entry.chance)
		{
			return (int) entry.slot;
		}

		dice -= entry.chance;
	}

	return PALETTE_TEMPLATE_ID::UNDEF_PALETTE_TEMPLATE;
}

DWORD TreasureTable::RollHealer(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_healerChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollLockpick(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_lockpickChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollConsumable(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_consumableChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollPea(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_peaChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollScroll(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_scrollChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollManaStone(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_manaStoneChances.m_entries[tier - 1]);
}

DWORD TreasureTable::RollItem(int tier)
{
	int initialTier = g_pPhatSDK->GetRandomInt(1, tier);

	if (g_pPhatSDK->GetRandomInt(0, 1))
	{
		switch (initialTier)
		{
		case 1:
			{
				static DWORD lootSets[] = { 1, 2, 3, 42, 44, 45, 46 };
				DWORD lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(DWORD)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 2:
			{
				static DWORD lootSets[] = { 5, 6, 7 };
				DWORD lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(DWORD)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 3:
		case 4:
			{
				static DWORD lootSets[] = { 4, 8, 9, 10 };
				DWORD lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(DWORD)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 5:
		case 6:
			{
				static DWORD lootSets[] = { 11, 12, 13 };
				DWORD lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(DWORD)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}
		}
	}
	else
	{
		static DWORD lootSets[] = { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 };
		DWORD lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(DWORD)) - 1)];
		return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier - 1]);
	}

	return 0;
}

int TreasureTable::RollWorkmanship(int tier)
{
	switch (tier)
	{
	case 1: return g_pPhatSDK->GetRandomInt(1, 5);
	case 2: return g_pPhatSDK->GetRandomInt(2, 6);
	case 3: return g_pPhatSDK->GetRandomInt(3, 7);
	case 4: return g_pPhatSDK->GetRandomInt(3, 8);
	case 5: return g_pPhatSDK->GetRandomInt(3, 9);
	case 6: return g_pPhatSDK->GetRandomInt(4, 10);
	}

	return 1;
}

DWORD TreasureTable::RollSpell(int spellCode, int tier)
{
	if (!spellCode || spellCode > _keyedSpells.size())
	{
		return 0;
	}

	auto entry = _keyedSpells.begin();
	std::advance(entry, spellCode - 1);

	return RollRandomSlotFromList(*entry);
}

DWORD TreasureTable::RollCantrip(int tier)
{
	return 0;
}

std::list<DWORD> TreasureTable::RollArmorSpells(int spellCode, int tier)
{
	std::list<DWORD> spells;

	if (tier >= 1 && tier <= 6)
	{
		for (auto &entry : _itemBaneSpells)
		{
			if (entry.second.chances[tier - 1] >= Random::RollDice(0.0f, 1.0f))
				spells.push_back(entry.first);
		}
	}

	return spells;
}

std::list<DWORD> TreasureTable::RollMeleeWeaponSpells(int spellCode, int tier)
{
	std::list<DWORD> spells;

	if (tier >= 1 && tier <= 6)
	{
		for (auto &entry : _meleeWeaponSpells)
		{
			if (entry.second.chances[tier - 1] >= Random::RollDice(0.0f, 1.0f))
				spells.push_back(entry.first);
		}
	}

	return spells;
}

std::list<DWORD> TreasureTable::RollMissileWeaponSpells(int spellCode, int tier)
{
	std::list<DWORD> spells;

	if (tier >= 1 && tier <= 6)
	{
		for (auto &entry : _missileWeaponSpells)
		{
			if (entry.second.chances[tier - 1] >= Random::RollDice(0.0f, 1.0f))
				spells.push_back(entry.first);
		}
	}

	return spells;
}

std::list<DWORD> TreasureTable::RollCasterSpells(int spellCode, int tier)
{
	std::list<DWORD> spells;

	if (tier >= 1 && tier <= 6)
	{
		for (auto &entry : _casterWeaponSpells)
		{
			if (entry.second.chances[tier - 1] >= Random::RollDice(0.0f, 1.0f))
				spells.push_back(entry.first);
		}
	}

	return spells;
}

DWORD TreasureTable::RollCasterBuffSpell(int spellCode, int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_otherBuffCasterSpell.m_entries[tier - 1]);
}

DWORD TreasureTable::RollCasterWarSpell(int spellCode, int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_otherWarCasterSpell.m_entries[tier - 1]);
}

BOOL MaterialTypeEnumMapper::MaterialTypeToString(unsigned int ct, std::string &materialName)
{

	switch (ct)
	{
	case 1u:
		materialName = "Ceramic";
		return TRUE;

	case 2u:
		materialName = "Porcelain";
		return TRUE;

	case 3u:
		materialName = "Cloth";
		return TRUE;

	case 4u:
		materialName = "Linen";
		return TRUE;

	case 5u:
		materialName = "Satin";
		return TRUE;

	case 6u:
		materialName = "Silk";
		return TRUE;

	case 7u:
		materialName = "Velvet";
		return TRUE;

	case 8u:
		materialName = "Wool";
		return TRUE;

	case 9u:
		materialName = "Gem";
		return TRUE;

	case 0xAu:
		materialName = "Agate";
		return TRUE;

	case 0xBu:
		materialName = "Amber";
		return TRUE;

	case 0xCu:
		materialName = "Amethyst";
		return TRUE;

	case 0xDu:
		materialName = "Aquamarine";
		return TRUE;

	case 0xEu:
		materialName = "Azurite";
		return TRUE;

	case 0xFu:
		materialName = "Black Garnet";
		return TRUE;

	case 0x10u:
		materialName = "Black Opal";
		return TRUE;

	case 0x11u:
		materialName = "Bloodstone";
		return TRUE;

	case 0x12u:
		materialName = "Carnelian";
		return TRUE;

	case 0x13u:
		materialName = "Citrine";
		return TRUE;

	case 0x14u:
		materialName = "Diamond";
		return TRUE;

	case 0x15u:
		materialName = "Emerald";
		return TRUE;

	case 0x16u:
		materialName = "Fire Opal";
		return TRUE;

	case 0x17u:
		materialName = "Green Garnet";
		return TRUE;

	case 0x18u:
		materialName = "Green Jade";
		return TRUE;

	case 0x19u:
		materialName = "Hematite";
		return TRUE;

	case 0x1Au:
		materialName = "Imperial Topaz";
		return TRUE;

	case 0x1Bu:
		materialName = "Jet";
		return TRUE;

	case 0x1Cu:
		materialName = "Lapis Lazuli";
		return TRUE;

	case 0x1Du:
		materialName = "Lavender Jade";
		return TRUE;

	case 0x1Eu:
		materialName = "Malachite";
		return TRUE;

	case 0x1Fu:
		materialName = "Moonstone";
		return TRUE;

	case 0x20u:
		materialName = "Onyx";
		return TRUE;

	case 0x21u:
		materialName = "Opal";
		return TRUE;

	case 0x22u:
		materialName = "Peridot";
		return TRUE;

	case 0x23u:
		materialName = "Red Garnet";
		return TRUE;

	case 0x24u:
		materialName = "Red Jade";
		return TRUE;

	case 0x25u:
		materialName = "Rose Quartz";
		return TRUE;

	case 0x26u:
		materialName = "Ruby";
		return TRUE;

	case 0x27u:
		materialName = "Sapphire";
		return TRUE;

	case 0x28u:
		materialName = "Smoky Quartz";
		return TRUE;

	case 0x29u:
		materialName = "Sunstone";
		return TRUE;

	case 0x2Au:
		materialName = "Tiger Eye";
		return TRUE;

	case 0x2Bu:
		materialName = "Tourmaline";
		return TRUE;

	case 0x2Cu:
		materialName = "Turquoise";
		return TRUE;

	case 0x2Du:
		materialName = "White Jade";
		return TRUE;

	case 0x2Eu:
		materialName = "White Quartz";
		return TRUE;

	case 0x2Fu:
		materialName = "White Sapphire";
		return TRUE;

	case 0x30u:
		materialName = "Yellow Garnet";
		return TRUE;

	case 0x31u:
		materialName = "Yellow Topaz";
		return TRUE;

	case 0x32u:
		materialName = "Zircon";
		return TRUE;

	case 0x33u:
		materialName = "Ivory";
		return TRUE;

	case 0x34u:
		materialName = "Leather";
		return TRUE;

	case 0x35u:
		materialName = "Armoredillo Hide";
		return TRUE;

	case 0x36u:
		materialName = "Gromnie Hide";
		return TRUE;

	case 0x37u:
		materialName = "Reed Shark Hide";
		return TRUE;

	case 0x38u:
		materialName = "Metal";
		return TRUE;

	case 0x39u:
		materialName = "Brass";
		return TRUE;

	case 0x3Au:
		materialName = "Bronze";
		return TRUE;

	case 0x3Bu:
		materialName = "Copper";
		return TRUE;

	case 0x3Cu:
		materialName = "Gold";
		return TRUE;

	case 0x3Du:
		materialName = "Iron";
		return TRUE;

	case 0x3Eu:
		materialName = "Pyreal";
		return TRUE;

	case 0x3Fu:
		materialName = "Silver";
		return TRUE;

	case 0x40u:
		materialName = "Steel";
		return TRUE;

	case 0x41u:
		materialName = "Stone";
		return TRUE;

	case 0x42u:
		materialName = "Alabaster";
		return TRUE;

	case 0x43u:
		materialName = "Granite";
		return TRUE;

	case 0x44u:
		materialName = "Marble";
		return TRUE;

	case 0x45u:
		materialName = "Obsidian";
		return TRUE;

	case 0x46u:
		materialName = "Sandstone";
		return TRUE;

	case 0x47u:
		materialName = "Serpentine";
		return TRUE;

	case 0x48u:
		materialName = "Wood";
		return TRUE;

	case 0x49u:
		materialName = "Ebony";
		return TRUE;

	case 0x4Au:
		materialName = "Mahogany";
		return TRUE;

	case 0x4Bu:
		materialName = "Oak";
		return TRUE;

	case 0x4Cu:
		materialName = "Pine";
		return TRUE;

	case 0x4Du:
		materialName = "Teak";
		return TRUE;

	case 0x80u:
		materialName = "NumMaterialTypes";
		return TRUE;

	default:
		materialName = "Unknown";
		return FALSE;
	}
}