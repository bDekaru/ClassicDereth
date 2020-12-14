
#include <StdAfx.h>
#include "TreasureTable.h"
#include "RandomRange.h"

DEFINE_PACK(TreasureEquipment)
{
	UNFINISHED();
}

DEFINE_PACK_JSON(TreasureEquipment)
{
	writer["WeenieClassId"] = _wcid;
	writer["PaletteId"] = _ptid;
	writer["Unknown1"] = m_08_AlwaysZero; // always zero
	writer["Shade"] = _shade;
	writer["StackSize"] = _amount;
	writer["StackSizeVariance"] = _amountVariance;
	writer["Probability"] = _chance;
	writer["Unknown3"] = m_1C_AlwaysZero; // always zero
	writer["Unknown4"] = m_20_AlwaysZero; // always zero
	writer["Unknown5"] = m_24_AlwaysZero; // always zero
	writer["SetStart"] = _setStart; // bool
	writer["HasSubSet"] = _hasSubSet; // bool
	writer["ContinuesPreviousSet"] = _continuesPreviousSet; // bool
	writer["Unknown9"] = m_34_AlwaysZero; // always zero
	writer["Unknown10"] = m_38_AlwaysZero; // always zero
	writer["Unknown11"] = m_3C_AlwaysZero; // always zero
	writer["Unknown12"] = m_40_AlwaysZero; // always zero
}

DEFINE_UNPACK(TreasureEquipment)
{
	_wcid = (WClassIDEnum) pReader->Read<uint32_t>();
	_ptid = pReader->Read<uint32_t>();
	m_08_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	_shade = pReader->Read<float>();
	_amount = pReader->Read<uint32_t>();
	_amountVariance = pReader->Read<float>();
	_chance = pReader->Read<float>();
	m_1C_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	m_20_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	m_24_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	_setStart = pReader->Read<BOOL>(); // bool
	_hasSubSet = pReader->Read<BOOL>(); // bool
	_continuesPreviousSet = pReader->Read<BOOL>(); // bool
	m_34_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	m_38_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	m_3C_AlwaysZero = pReader->Read<uint32_t>(); // always zero
	m_40_AlwaysZero = pReader->Read<uint32_t>(); // always zero

	return true;
}

DEFINE_UNPACK_JSON(TreasureEquipment)
{
	_wcid = (WClassIDEnum)reader["WeenieClassId"];
	_ptid = reader["PaletteId"];
	m_08_AlwaysZero = reader["Unknown1"]; // always zero
	_shade = reader["Shade"];
	_amount = reader["StackSize"];
	_amountVariance = reader["StackSizeVariance"];
	_chance = reader["Probability"];
	m_1C_AlwaysZero = reader["Unknown3"]; // always zero
	m_20_AlwaysZero = reader["Unknown4"]; // always zero
	m_24_AlwaysZero = reader["Unknown5"]; // always zero
	_setStart = reader["SetStart"]; // bool
	_hasSubSet = reader["HasSubSet"]; // bool
	_continuesPreviousSet = reader["ContinuesPreviousSet"]; // bool
	m_34_AlwaysZero = reader["Unknown9"]; // always zero
	m_38_AlwaysZero = reader["Unknown10"]; // always zero
	m_3C_AlwaysZero = reader["Unknown11"]; // always zero
	m_40_AlwaysZero = reader["Unknown12"]; // always zero

	return true;

}

DEFINE_PACK(TreasureLoot)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureLoot)
{
	_tier = pReader->Read<uint32_t>();
	_lootQualityMod = pReader->Read<float>();
	_unknownChances = pReader->Read<uint32_t>();
	_itemChance = pReader->Read<uint32_t>();
	_itemMinAmount = pReader->Read<uint32_t>();
	_itemMaxAmount = pReader->Read<uint32_t>();
	_itemTreasureTypeSelectionChances = pReader->Read<uint32_t>();
	_magicItemChance = pReader->Read<uint32_t>();
	_magicItemMinAmount = pReader->Read<uint32_t>();
	_magicItemMaxAmount = pReader->Read<uint32_t>();
	_magicItemTreasureTypeSelectionChances = pReader->Read<uint32_t>();
	_mundaneItemChance = pReader->Read<uint32_t>();
	_mundaneItemMinAmount = pReader->Read<uint32_t>();
	_mundaneItemMaxAmount = pReader->Read<uint32_t>();
	_mundaneItemTypeSelectionChances = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK_JSON(TreasureLoot)
{
	writer["tier"] = _tier;
	writer["qualityMod"] = _lootQualityMod;
	writer["unkChance"] = _unknownChances;

	writer["chance"] = _itemChance;
	writer["min"] = _itemMinAmount;
	writer["max"] = _itemMaxAmount;
	writer["typeChance"] = _itemTreasureTypeSelectionChances;

	writer["magicChance"] = _magicItemChance;
	writer["magicMin"] = _magicItemMinAmount;
	writer["magicMax"] = _magicItemMaxAmount;
	writer["magicTypeChance"] = _magicItemTreasureTypeSelectionChances;

	writer["mundaneChance"] = _mundaneItemChance;
	writer["mundaneMin"] = _mundaneItemMinAmount;
	writer["mundaneMax"] = _mundaneItemMaxAmount;
	writer["mundaneTypeChance"] = _mundaneItemTypeSelectionChances;
}

DEFINE_UNPACK_JSON(TreasureLoot)
{
	_tier = reader["tier"];
	_lootQualityMod = reader["qualityMod"];
	_unknownChances = reader["unkChance"];
	
	_itemChance = reader["chance"];
	_itemMinAmount = reader["min"];
	_itemMaxAmount = reader["max"];
	_itemTreasureTypeSelectionChances = reader["typeChance"];
	
	_magicItemChance = reader["magicChance"];
	_magicItemMinAmount = reader["magicMin"];
	_magicItemMaxAmount = reader["magicMax"];
	_magicItemTreasureTypeSelectionChances = reader["magicTypeChance"];

	_mundaneItemChance = reader["mundaneChance"];
	_mundaneItemMinAmount = reader["mundaneMin"];
	_mundaneItemMaxAmount = reader["mundaneMax"];
	_mundaneItemTypeSelectionChances = reader["mundaneTypeChance"];

	return true;
}

DEFINE_PACK(TreasureEntry3)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry3)
{
	m_00 = pReader->Read<uint32_t>();
	m_04 = pReader->Read<uint32_t>();
	m_08 = pReader->Read<uint32_t>();
	m_0C = pReader->Read<uint32_t>();
	m_10 = pReader->Read<uint32_t>();
	m_14 = pReader->Read<uint32_t>();
	m_18 = pReader->Read<uint32_t>();
	m_1C = pReader->Read<uint32_t>();
	m_20 = pReader->Read<uint32_t>();
	m_24 = pReader->Read<uint32_t>();
	m_28 = pReader->Read<uint32_t>();
	m_2C = pReader->Read<uint32_t>();
	m_30 = pReader->Read<uint32_t>();
	m_34 = pReader->Read<uint32_t>();
	m_38 = pReader->Read<uint32_t>();
	m_3C = pReader->Read<uint32_t>();
	m_40 = pReader->Read<uint32_t>();
	m_44 = pReader->Read<uint32_t>();
	m_48 = pReader->Read<uint32_t>();
	m_4C = pReader->Read<uint32_t>();
	m_50 = pReader->Read<uint32_t>();
	m_54 = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK_JSON(TreasureEntry3)
{
	writer["_00"] = m_00;
	writer["_04"] = m_04;
	writer["_08"] = m_08;
	writer["_0C"] = m_0C;
	writer["_10"] = m_10;
	writer["_14"] = m_14;
	writer["_18"] = m_18;
	writer["_1C"] = m_1C;
	writer["_20"] = m_20;
	writer["_24"] = m_24;
	writer["_28"] = m_28;
	writer["_2C"] = m_2C;
	writer["_30"] = m_30;
	writer["_34"] = m_34;
	writer["_38"] = m_38;
	writer["_3C"] = m_3C;
	writer["_40"] = m_40;
	writer["_44"] = m_44;
	writer["_48"] = m_48;
	writer["_4C"] = m_4C;
	writer["_50"] = m_50;
	writer["_54"] = m_54;
}

DEFINE_UNPACK_JSON(TreasureEntry3)
{
	m_00 = reader["_00"];
	m_04 = reader["_04"];
	m_08 = reader["_08"];
	m_0C = reader["_0C"];
	m_10 = reader["_10"];
	m_14 = reader["_14"];
	m_18 = reader["_18"];
	m_1C = reader["_1C"];
	m_20 = reader["_20"];
	m_24 = reader["_24"];
	m_28 = reader["_28"];
	m_2C = reader["_2C"];
	m_30 = reader["_30"];
	m_34 = reader["_34"];
	m_38 = reader["_38"];
	m_3C = reader["_3C"];
	m_40 = reader["_40"];
	m_44 = reader["_44"];
	m_48 = reader["_48"];
	m_4C = reader["_4C"];
	m_50 = reader["_50"];
	m_54 = reader["_54"];

	return true;
}

DEFINE_PACK(TreasureEntry4)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry4)
{
	m_00 = pReader->Read<uint32_t>();
	m_04 = pReader->Read<uint32_t>();
	m_08 = pReader->Read<uint32_t>();
	m_0C = pReader->Read<uint32_t>();
	m_10 = pReader->Read<uint32_t>();
	m_14 = pReader->Read<uint32_t>();
	m_18 = pReader->Read<uint32_t>();
	m_1C = pReader->Read<uint32_t>();
	m_20 = pReader->Read<uint32_t>();
	m_24 = pReader->Read<uint32_t>();
	m_28 = pReader->Read<uint32_t>();
	m_2C = pReader->Read<uint32_t>();
	m_30 = pReader->Read<uint32_t>();
	m_34 = pReader->Read<uint32_t>();
	m_38 = pReader->Read<uint32_t>();
	m_3C = pReader->Read<uint32_t>();
	m_40 = pReader->Read<uint32_t>();
	m_44 = pReader->Read<uint32_t>();
	m_48 = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK_JSON(TreasureEntry4)
{
	writer["_00"] = m_00;
	writer["_04"] = m_04;
	writer["_08"] = m_08;
	writer["_0C"] = m_0C;
	writer["_10"] = m_10;
	writer["_14"] = m_14;
	writer["_18"] = m_18;
	writer["_1C"] = m_1C;
	writer["_20"] = m_20;
	writer["_24"] = m_24;
	writer["_28"] = m_28;
	writer["_2C"] = m_2C;
	writer["_30"] = m_30;
	writer["_34"] = m_34;
	writer["_38"] = m_38;
	writer["_3C"] = m_3C;
	writer["_40"] = m_40;
	writer["_44"] = m_44;
	writer["_48"] = m_48;
}

DEFINE_UNPACK_JSON(TreasureEntry4)
{
	m_00 = reader["_00"];
	m_04 = reader["_04"];
	m_08 = reader["_08"];
	m_0C = reader["_0C"];
	m_10 = reader["_10"];
	m_14 = reader["_14"];
	m_18 = reader["_18"];
	m_1C = reader["_1C"];
	m_20 = reader["_20"];
	m_24 = reader["_24"];
	m_28 = reader["_28"];
	m_2C = reader["_2C"];
	m_30 = reader["_30"];
	m_34 = reader["_34"];
	m_38 = reader["_38"];
	m_3C = reader["_3C"];
	m_40 = reader["_40"];
	m_44 = reader["_44"];
	m_48 = reader["_48"];

	return true;
}

DEFINE_PACK(TreasureChance)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureChance)
{
	slot = pReader->Read<uint32_t>();
	chance = pReader->Read<double>();
	return true;
}

DEFINE_PACK_JSON(TreasureChance)
{
	writer["chance"] = chance;
	writer["value"] = slot;
}

DEFINE_UNPACK_JSON(TreasureChance)
{
	chance = reader["chance"];
	slot = reader["value"];

	return true;
}

DEFINE_PACK(TreasureEntry6)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureEntry6)
{
	chances.resize(6);

	for (int i = 0; i < 6; i++)
		chances[i] = pReader->Read<float>();

	return true;
}

DEFINE_PACK_JSON(TreasureEntry6)
{
	for (auto itr = chances.begin(); itr != chances.end(); itr++)
		writer.push_back(*itr);
}

DEFINE_UNPACK_JSON(TreasureEntry6)
{
	if (!reader.is_array())
		return false;

	size_t len = reader.size();
	chances.resize(len);
	int index = 0;

	for (auto itr = reader.begin(); itr != reader.end(); itr++)
		chances[index++] = itr->get<float>();

	return true;
}

DEFINE_PACK(TreasureTiers)
{
	UNFINISHED();
}

DEFINE_UNPACK(TreasureTiers)
{
	m_entries.resize(6);

	for (uint32_t i = 0; i < 6; i++)
		m_entries[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(TreasureTiers)
{
	for (auto itr = m_entries.begin(); itr != m_entries.end(); itr++)
	{
		json tmp;
		itr->PackJson(tmp);
		writer.push_back(tmp);
	}
}

DEFINE_UNPACK_JSON(TreasureTiers)
{
	if (!reader.is_array())
		return false;

	size_t len = reader.size();
	m_entries.resize(len);
	int index = 0;

	for (auto itr = reader.begin(); itr != reader.end(); itr++)
		m_entries[index++].UnPackJson(*itr);

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
	pReader->Read<uint32_t>();
#endif

	_treasureEquipment.UnPack(pReader);	
	_treasureLoot.UnPack(pReader);

	for (uint32_t i = 0; i < 4; i++)
		_treasure3[i].UnPack(pReader);

	//_treasure7.resize(48);
	for (uint32_t i = 0; i < 48; i++)
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

	_workmanshipChance.UnPack(pReader);

	_treasure18.UnPack(pReader);


	_treasure19.UnPack(pReader);
	_treasure20.UnPack(pReader);
	_gemProbabilitiesMaybe.UnPack(pReader);
	_gemMaterials.UnPack(pReader);
	_materialValueAddedPossibly.UnPack(pReader);

	_treasure24.UnPack(pReader);
	_treasure25.UnPack(pReader);
	_treasure26.UnPack(pReader);

	_materialColorKeyMap.UnPack(pReader);

	_cantripsArmor.UnPack(pReader);
	_cantripsMelee.UnPack(pReader);
	_cantripsShield.UnPack(pReader);
	_cantripsMissile.UnPack(pReader);
	_cantripsCaster.UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(TreasureTable)
{
	PackObjJson(writer, "equipment", _treasureEquipment);
	PackObjJson(writer, "loot", _treasureLoot);

	PackObjArrayJson(writer, "unk3", std::begin(_treasure3), std::end(_treasure3));
	PackObjArrayJson(writer, "unk7", std::begin(_treasure7), std::end(_treasure7));

	PackObjJson(writer, "unk8", _treasure8);
	PackObjJson(writer, "healChances", _healerChances);
	PackObjJson(writer, "lockpickChances", _lockpickChances);
	PackObjJson(writer, "consumableChances", _consumableChances);
	PackObjJson(writer, "peaChances", _peaChances);

	PackObjJson(writer, "banes", _itemBaneSpells);
	PackObjJson(writer, "meleeSpells", _meleeWeaponSpells);
	PackObjJson(writer, "missileSpells", _missileWeaponSpells);
	PackObjJson(writer, "casterChances", _casterWeaponSpells);

	PackObjJson(writer, "spellLevelChances", _spellLevelProbabilitiesMaybe);
	PackObjJson(writer, "keyedSpells", _keyedSpells);

	PackObjJson(writer, "casterSpellsOther", _otherBuffCasterSpell);
	PackObjJson(writer, "casterSpellsWar", _otherWarCasterSpell);

	PackObjJson(writer, "scrollChances", _scrollChances);
	PackObjJson(writer, "manaStoneChances", _manaStoneChances);

	PackObjJson(writer, "materialCategories", _materialCodeToBaseMaterialMap);

	PackObjJson(writer, "ceramicMaterials", _ceramicMaterials);
	PackObjJson(writer, "clothMaterials", _clothMaterials);
	PackObjJson(writer, "leatherMaterials", _leatherMaterials);
	PackObjJson(writer, "metalMaterials", _metalMaterials);
	PackObjJson(writer, "stoneMaterials", _stoneMaterials);
	PackObjJson(writer, "woodMaterials", _woodMaterials);

	PackObjJson(writer, "workmanshipChance", _workmanshipChance);

	PackObjJson(writer, "unk18", _treasure18);

	PackObjJson(writer, "unk19", _treasure19);
	PackObjJson(writer, "unk20", _treasure20);

	PackObjJson(writer, "gemChances", _gemProbabilitiesMaybe);
	PackObjJson(writer, "gemMaterials", _gemMaterials);

	PackObjJson(writer, "materialValue", _materialValueAddedPossibly);

	PackObjJson(writer, "unk24", _treasure24);
	PackObjJson(writer, "unk25", _treasure25);
	PackObjJson(writer, "unk26", _treasure26);

	PackObjJson(writer, "materialColors", _materialColorKeyMap);

	PackObjJson(writer, "cantripsArmor", _cantripsArmor);
	PackObjJson(writer, "cantripsMelee", _cantripsMelee);
	PackObjJson(writer, "cantripsShield", _cantripsShield);
	PackObjJson(writer, "cantripsMissile", _cantripsMissile);
	PackObjJson(writer, "cantripsCaster", _cantripsCaster);
}

DEFINE_UNPACK_JSON(TreasureTable)
{
	int length = 0;

	UnPackObjJson(reader, "equipment", _treasureEquipment);
	UnPackObjJson(reader, "loot", _treasureLoot);

	// 4
	UnPackObjArrayJson(reader, "unk3", _treasure3, length);
	// 48
	UnPackObjArrayJson(reader, "unk7", _treasure7, length);

	UnPackObjJson(reader, "unk8", _treasure8);
	UnPackObjJson(reader, "healChances", _healerChances);
	UnPackObjJson(reader, "lockpickChances", _lockpickChances);
	UnPackObjJson(reader, "consumableChances", _consumableChances);
	UnPackObjJson(reader, "peaChances", _peaChances);

	UnPackObjJson(reader, "banes", _itemBaneSpells);
	UnPackObjJson(reader, "meleeSpells", _meleeWeaponSpells);
	UnPackObjJson(reader, "missileSpells", _missileWeaponSpells);
	UnPackObjJson(reader, "casterChances", _casterWeaponSpells);

	UnPackObjJson(reader, "spellLevelChances", _spellLevelProbabilitiesMaybe);
	UnPackObjJson(reader, "keyedSpells", _keyedSpells);

	UnPackObjJson(reader, "casterSpellsOther", _otherBuffCasterSpell);
	UnPackObjJson(reader, "casterSpellsWar", _otherWarCasterSpell);

	UnPackObjJson(reader, "scrollChances", _scrollChances);
	UnPackObjJson(reader, "manaStoneChances", _manaStoneChances);

	UnPackObjJson(reader, "materialCategories", _materialCodeToBaseMaterialMap);

	UnPackObjJson(reader, "ceramicMaterials", _ceramicMaterials);
	UnPackObjJson(reader, "clothMaterials", _clothMaterials);
	UnPackObjJson(reader, "leatherMaterials", _leatherMaterials);
	UnPackObjJson(reader, "metalMaterials", _metalMaterials);
	UnPackObjJson(reader, "stoneMaterials", _stoneMaterials);
	UnPackObjJson(reader, "woodMaterials", _woodMaterials);

	UnPackObjJson(reader, "workmanshipChance", _workmanshipChance);

	UnPackObjJson(reader, "unk18", _treasure18);

	UnPackObjJson(reader, "unk19", _treasure19);
	UnPackObjJson(reader, "unk20", _treasure20);

	UnPackObjJson(reader, "gemChances", _gemProbabilitiesMaybe);
	UnPackObjJson(reader, "gemMaterials", _gemMaterials);

	UnPackObjJson(reader, "materialValue", _materialValueAddedPossibly);

	UnPackObjJson(reader, "unk24", _treasure24);
	UnPackObjJson(reader, "unk25", _treasure25);
	UnPackObjJson(reader, "unk26", _treasure26);

	UnPackObjJson(reader, "materialColors", _materialColorKeyMap);

	UnPackObjJson(reader, "cantripsArmor", _cantripsArmor);
	UnPackObjJson(reader, "cantripsMelee", _cantripsMelee);
	UnPackObjJson(reader, "cantripsShield", _cantripsShield);
	UnPackObjJson(reader, "cantripsMissile", _cantripsMissile);
	UnPackObjJson(reader, "cantripsCaster", _cantripsCaster);

	return true;
}

TreasureLoot *TreasureTable::GetTreasureGenerationProfile(uint32_t treasureId)
{
	return _treasureLoot.lookup(treasureId);
}

eTreasureCategory TreasureTable::RollTreasureCategory(uint32_t tableId, uint32_t selectionChanceId)
{
	if (tableId < _treasure3->size())
	{
		PackableHashTableWithJson<uint32_t, PackableListWithJson<TreasureChance>> *table = &_treasure3[tableId];

		if (selectionChanceId < table->size())
			return (eTreasureCategory)RollRandomSlotFromList(*table->lookup(selectionChanceId));
	}
	return eTreasureCategory::TreasureCategory_Undef;
}

int TreasureTable::GetLootTierForTreasureEntry(uint32_t treasure_id)
{
	auto entry = _treasureLoot.lookup(treasure_id);

	if (!entry)
		return 0;

	return entry->_tier;
}

uint32_t TreasureTable::RollRandomSlotFromList(const treasure_chance_list_t& list)
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

uint32_t TreasureTable::RollRandomScrollWCID(int tier)
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

	PackableList<TreasureChance> *chanceList = NULL;
	TreasureTiers *materialMap = NULL;

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
		if (baseMaterial == Linen_MaterialType || baseMaterial == Satin_MaterialType || baseMaterial == Silk_MaterialType ||
			baseMaterial == Velvet_MaterialType || baseMaterial == Wool_MaterialType)
		{
			return Random::GenInt(1, 18);
		}
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

uint32_t TreasureTable::RollHealer(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_healerChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollLockpick(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_lockpickChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollConsumable(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_consumableChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollPea(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_peaChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollScroll(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_scrollChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollManaStone(int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_manaStoneChances.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollItem(int tier)
{
	int initialTier = g_pPhatSDK->GetRandomInt(1, tier);

	if (g_pPhatSDK->GetRandomInt(0, 1))
	{
		switch (initialTier)
		{
		case 1:
			{
				static uint32_t lootSets[] = { 1, 2, 3, 42, 44, 45, 46 };
				uint32_t lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(uint32_t)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 2:
			{
				static uint32_t lootSets[] = { 5, 6, 7 };
				uint32_t lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(uint32_t)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 3:
		case 4:
			{
				static uint32_t lootSets[] = { 4, 8, 9, 10 };
				uint32_t lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(uint32_t)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}

		case 5:
		case 6:
			{
				static uint32_t lootSets[] = { 11, 12, 13 };
				uint32_t lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(uint32_t)) - 1)];
				return RollRandomSlotFromList(_treasure7[lootSet].m_entries[tier-1]);
			}
		}
	}
	else
	{
		static uint32_t lootSets[] = { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 };
		uint32_t lootSet = lootSets[g_pPhatSDK->GetRandomUInt(0, (sizeof(lootSets) / sizeof(uint32_t)) - 1)];
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

uint32_t TreasureTable::RollSpell(int spellCode, int tier)
{
	if (!spellCode || spellCode > _keyedSpells.size())
	{
		return 0;
	}

	auto entry = _keyedSpells.begin();
	std::advance(entry, spellCode - 1);

	return RollRandomSlotFromList(*entry);
}

uint32_t TreasureTable::RollCantrip(int tier)
{
	return 0;
}

std::list<uint32_t> TreasureTable::RollArmorSpells(int spellCode, int tier)
{
	std::list<uint32_t> spells;

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

std::list<uint32_t> TreasureTable::RollMeleeWeaponSpells(int spellCode, int tier)
{
	std::list<uint32_t> spells;

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

std::list<uint32_t> TreasureTable::RollMissileWeaponSpells(int spellCode, int tier)
{
	std::list<uint32_t> spells;

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

std::list<uint32_t> TreasureTable::RollCasterSpells(int spellCode, int tier)
{
	std::list<uint32_t> spells;

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

uint32_t TreasureTable::RollCasterBuffSpell(int spellCode, int tier)
{
	if (tier < 1 || tier > 6)
		return 0;

	return RollRandomSlotFromList(_otherBuffCasterSpell.m_entries[tier - 1]);
}

uint32_t TreasureTable::RollCasterWarSpell(int spellCode, int tier)
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
