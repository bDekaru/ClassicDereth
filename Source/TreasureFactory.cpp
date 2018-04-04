
#include "StdAfx.h"
#include "TreasureFactory.h"
#include "WeenieObject.h"
#include "WeenieFactory.h"
#include "InferredPortalData.h"
#include "RandomRange.h"
#include "World.h"
#include "Container.h"
#include "Config.h"
#include "Monster.h"

double round(double value, int decimalPlaces)
{
	int multiplier = pow(10, decimalPlaces);
	int converted = (value + (0.05 / multiplier)) * multiplier;

	double returnValue = (double)converted / (double)multiplier;

	return returnValue;
}

bool HasField(const nlohmann::json &reader, std::string name)
{
	if (reader.find(name) != reader.end())
		return true;
	else
		return false;
}

void from_json(const nlohmann::json &reader, CSpellProperties &output)
{
	if (HasField(reader, "spellPower"))
		output.spellPower = reader.at("spellPower").get<std::vector<int>>();
	if (HasField(reader, "spellMana"))
		output.spellMana = reader.at("spellMana").get<std::vector<int>>();
	if (HasField(reader, "cantripPower"))
		output.cantripPower = reader.at("cantripPower").get<std::vector<int>>();
	if (HasField(reader, "cantripMana"))
		output.cantripMana = reader.at("cantripMana").get<std::vector<int>>();
}

void from_json(const nlohmann::json &reader, CMaterialProperties &output)
{
	output.valueMultiplier = reader.value("valueMultiplier", 0.0f);
	if (HasField(reader, "palettes"))
		output.palettes = reader.at("palettes").get<std::vector<int>>();
	output.gemValue = reader.value("gemValue", 0);
	if (HasField(reader, "gemName"))
		output.gemName = reader.at("gemName").get<std::string>();
	if (HasField(reader, "gemPluralName"))
		output.gemPluralName = reader.at("gemPluralName").get<std::string>();
}

void from_json(const nlohmann::json &reader, CWorkmanshipProperties &output)
{
	output.valueMultiplier = reader.value("valueMultiplier", 0.0f);
	output.gemChance = reader.value("gemChance", 0.0f);
	output.maxGemCount = reader.value("maxGemCount", 0);
}

void from_json(const nlohmann::json &reader, CItemTreasureProfileEntry &output)
{
	if (HasField(reader, "name"))
		output.name = reader.at("name").get<std::string>();
	output.wcid = reader.value("wcid", 0);
	output.maxAmount = reader.value("maxAmount", 0);
	if (HasField(reader, "possibleMaterials"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("possibleMaterials").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.possibleMaterials.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "elementalVariants"))
		output.elementalVariants = reader.at("elementalVariants").get<std::vector<int>>();

	if (HasField(reader, "weaponSkill"))
	{
		std::string tempWeaponSkill = reader.at("weaponSkill").get<std::string>();

		output.weaponSkill = g_pTreasureFactory->TranslateSkillStringToEnumValue(tempWeaponSkill);
	}
	else
		output.weaponSkill = UNDEF_SKILL;
}

void from_json(const nlohmann::json &reader, CPossibleSpells &output)
{
	if (HasField(reader, "spellName"))
		output.spellName = reader.at("spellName").get<std::string>();

	if (HasField(reader, "spellCategory"))
	{
		std::string spellCategoryString = reader.at("spellCategory");

		if (spellCategoryString == "magicSkillMastery")
			output.spellCategory = magicSkillMastery;
		else if (spellCategoryString == "weaponSkillMastery")
			output.spellCategory = weaponSkillMastery;
		else
			output.spellCategory = noCategory;
	}
	else
		output.spellCategory = noCategory;

	if (HasField(reader, "spells"))
		output.spells = reader.at("spells").get<std::vector<int>>();
	if (HasField(reader, "cantrips"))
		output.cantrips = reader.at("cantrips").get<std::vector<int>>();

	output.id = 0;
}

void from_json(const nlohmann::json &reader, CWieldTier &output)
{
	output.weaponSkillRequired = reader.value("weaponSkillRequired", 0);
	output.maxAttackSkillBonus = reader.value("maxAttackSkillBonus", 0.0f);
	output.maxMeleeDefenseBonus = reader.value("maxMeleeDefenseBonus", 0.0f);
	output.maxMissileDefenseBonus = reader.value("maxMissileDefenseBonus", 0.0f);
	output.maxMagicDefenseBonus = reader.value("maxMagicDefenseBonus", 0.0f);
	output.attackSkillBonusChance = reader.value("attackSkillBonusChance", 0.0f);
	output.meleeDefenseBonusChance = reader.value("meleeDefenseBonusChance", 0.0f);
	output.missileDefenseBonusChance = reader.value("missileDefenseBonusChance", 0.0f);
	output.magicDefenseBonusChance = reader.value("magicDefenseBonusChance", 0.0f);
	output.elementalChance = reader.value("elementalChance", 0.0f);

	output.slayer_Chance = reader.value("slayer_Chance", 0.0f);
	output.slayer_MinDamageBonus = reader.value("slayer_MinDamageBonus", 0.0f);
	output.slayer_MaxDamageBonus = reader.value("slayer_MaxDamageBonus", 0.0f);

	if (HasField(reader, "slayer_Types"))
	{
		std::vector<std::string> tempSlayerTypes;
		tempSlayerTypes = reader.at("slayer_Types").get<std::vector<std::string>>();

		for (auto iter = tempSlayerTypes.begin(); iter != tempSlayerTypes.end(); ++iter)
			output.slayer_Types.push_back(g_pTreasureFactory->TranslateCreatureStringToEnumValue(iter->data()));
	}

	output.maxPropertyAmount = reader.value("maxPropertyAmount", 0);

	output.crushingBlow_Chance = reader.value("crushingBlow_Chance", 0.0f);
	output.crushingBlow_MinCriticalMultiplier = reader.value("crushingBlow_MinCriticalMultiplier", 0.0f);
	output.crushingBlow_MaxCriticalMultiplier = reader.value("crushingBlow_MaxCriticalMultiplier", 0.0f);

	output.bitingStrike_Chance = reader.value("bitingStrike_Chance", 0.0f);
	output.bitingStrike_MinCriticalFrequency = reader.value("bitingStrike_MinCriticalFrequency", 0.0f);
	output.bitingStrike_MaxCriticalFrequency = reader.value("bitingStrike_MaxCriticalFrequency", 0.0f);

	output.minDamageBonus = reader.value("minDamageBonus", 0);
	output.maxDamageBonus = reader.value("maxDamageBonus", 0);
	output.highVariance = reader.value("highVariance", 0.0f);
	output.lowVariance = reader.value("lowVariance", 0.0f);

	output.slowSpeedMod = reader.value("slowSpeedMod", 0.0f);
	output.fastSpeedMod = reader.value("fastSpeedMod", 0.0f);

	output.minDamageModBonus = reader.value("minDamageModBonus", 0.0f);
	output.maxDamageModBonus = reader.value("maxDamageModBonus", 0.0f);
	output.minElementalDamageBonus = reader.value("minElementalDamageBonus", 0);
	output.maxElementalDamageBonus = reader.value("maxElementalDamageBonus", 0);

	output.maxManaConversionBonus = reader.value("maxManaConversionBonus", 0.0f);
	output.minElementalDamageMod = reader.value("minElementalDamageMod", 0.0f);
	output.maxElementalDamageMod = reader.value("maxElementalDamageMod", 0.0f);
	output.manaConversionBonusChance = reader.value("manaConversionBonusChance", 0.0f);

	output.armorWieldTier = reader.value("armorWieldTier", 0);
	output.meleeDefenseSkillRequired = reader.value("meleeDefenseSkillRequired", 0);
	output.missileDefenseSkillRequired = reader.value("missileDefenseSkillRequired", 0);
	output.magicDefenseSkillRequired = reader.value("magicDefenseSkillRequired", 0);
	output.minArmorBonus = reader.value("minArmorBonus", 0);
	output.maxArmorBonus = reader.value("maxArmorBonus", 0);
	output.minBurdenMod = reader.value("minBurdenMod", 0.0f);
	output.maxBurdenMod = reader.value("maxBurdenMod", 0.0f);
	output.minShieldArmorBonus = reader.value("minShieldArmorBonus", 0);
	output.maxShieldArmorBonus = reader.value("maxShieldArmorBonus", 0);
	output.minLevel = reader.value("minLevel", 0);
}

void from_json(const nlohmann::json &reader, CTreasureProfileCategory &output)
{
	if (HasField(reader, "category"))
		output.category = reader.at("category").get<std::string>();

	if (HasField(reader, "possibleMaterials"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("possibleMaterials").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.possibleMaterials.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "wieldTiers"))
		output.wieldTiers = reader.at("wieldTiers").get<std::vector<CWieldTier>>();
	if (HasField(reader, "entries"))
		output.entries = reader.at("entries").get<std::vector<CItemTreasureProfileEntry>>();
}

void from_json(const nlohmann::json &reader, CTreasureArmorProfile &output)
{
	if (HasField(reader, "wieldTiers"))
		output.wieldTiers = reader.at("wieldTiers").get<std::vector<CWieldTier>>();
	if (HasField(reader, "categories"))
		output.categories = reader.at("categories").get<std::vector<CTreasureProfileCategory>>();
}

void from_json(const nlohmann::json &reader, CTreasureTier &output)
{
	output.tierId = reader.value("tierId", 0);

	if (HasField(reader, "miscItemsCategories"))
		output.miscItemsCategories = reader.at("miscItemsCategories").get<std::vector<std::string>>();
	if (HasField(reader, "scrollCategories"))
		output.scrollCategories = reader.at("scrollCategories").get<std::vector<std::string>>();

	output.lootChance = reader.value("lootChance", 0.0f);
	output.chestLootChance = reader.value("chestLootChance", 0.0f);
	output.miscLootChance = reader.value("miscLootChance", 0.0f);
	output.scrollLootChance = reader.value("scrollLootChance", 0.0f);

	output.lootAmountMultiplier = reader.value("lootAmountMultiplier", 0.0f);
	output.chestLootAmountMultiplier = reader.value("chestLootAmountMultiplier", 0.0f);
	output.qualityLootLevelThreshold = reader.value("qualityLootLevelThreshold", 0);
	output.qualityLootModifier = reader.value("qualityLootModifier", 0.0f);

	output.minMeleeWeaponWieldTier = reader.value("minMeleeWeaponWieldTier", 0);
	output.maxMeleeWeaponWieldTier = reader.value("maxMeleeWeaponWieldTier", 0);
	output.minMissileWeaponWieldTier = reader.value("minMissileWeaponWieldTier", 0);
	output.maxMissileWeaponWieldTier = reader.value("maxMissileWeaponWieldTier", 0);
	output.minCasterWieldTier = reader.value("minCasterWieldTier", 0);
	output.maxCasterWieldTier = reader.value("maxCasterWieldTier", 0);

	output.minArmorWieldTier = reader.value("minArmorWieldTier", 0);
	output.maxArmorWieldTier = reader.value("maxArmorWieldTier", 0);
	output.minArmorMeleeWieldTier = reader.value("minArmorMeleeWieldTier", 0);
	output.maxArmorMeleeWieldTier = reader.value("maxArmorMeleeWieldTier", 0);
	output.minArmorMissileWieldTier = reader.value("minArmorMissileWieldTier", 0);
	output.maxArmorMissileWieldTier = reader.value("maxArmorMissileWieldTier", 0);
	output.minArmorMagicWieldTier = reader.value("minArmorMagicWieldTier", 0);
	output.maxArmorMagicWieldTier = reader.value("maxArmorMagicWieldTier", 0);

	output.meleeWeaponsLootProportion = reader.value("meleeWeaponsLootProportion", 0);
	output.missileWeaponsLootProportion = reader.value("missileWeaponsLootProportion", 0);
	output.castersLootProportion = reader.value("castersLootProportion", 0);
	output.armorLootProportion = reader.value("armorLootProportion", 0);
	output.clothingLootProportion = reader.value("clothingLootProportion", 0);
	output.jewelryLootProportion = reader.value("jewelryLootProportion", 0);

	if (HasField(reader, "commonArmorCategories"))
		output.commonArmorCategoryNames = reader.at("commonArmorCategories").get<std::vector<std::string>>();
	if (HasField(reader, "rareArmorCategories"))
		output.rareArmorCategoryNames = reader.at("rareArmorCategories").get<std::vector<std::string>>();
	output.rareArmorChance = reader.value("rareArmorChance", 0.0f);

	output.minWorkmanship = reader.value("minWorkmanship", 0);
	output.maxWorkmanship = reader.value("maxWorkmanship", 0);

	output.maxAmountOfSpells = reader.value("maxAmountOfSpells", 0);
	output.chanceOfSpells = reader.value("chanceOfSpells", 0.0f);
	output.minSpellLevel = reader.value("minSpellLevel", 0);
	output.maxSpellLevel = reader.value("maxSpellLevel", 0);
	output.preferredSpellLevel = reader.value("preferredSpellLevel", 0);
	output.preferredSpellLevelStrength = reader.value("preferredSpellLevelStrength", 0.0f);

	output.maxAmountOfCantrips = reader.value("maxAmountOfCantrips", 0);
	output.chanceOfCantrips = reader.value("chanceOfCantrips", 0.0f);
	output.minCantripLevel = reader.value("minCantripLevel", 0);
	output.maxCantripLevel = reader.value("maxCantripLevel", 0);
	output.preferredCantripLevel = reader.value("preferredCantripLevel", 0);
	output.preferredCantripLevelStrength = reader.value("preferredCantripLevelStrength", 0.0f);

	output.heritageRequirementChance = reader.value("heritageRequirementChance", 0.0f);
	output.minSpellsForHeritageRequirement = reader.value("minSpellsForHeritageRequirement", 0);
	output.allegianceRankRequirementChance = reader.value("allegianceRankRequirementChance", 0.0f);
	output.minSpellsForAllegianceRankRequirement = reader.value("minSpellsForAllegianceRankRequirement", 0);
	output.maxAllegianceRankRequired = reader.value("maxAllegianceRankRequired", 0);

	if (HasField(reader, "materialsCeramic"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsCeramic").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsCeramic.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsCloth"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsCloth").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsCloth.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsGem"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsGem").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsGem.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsLeather"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsLeather").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsLeather.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsMetal"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsMetal").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsMetal.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsStone"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsStone").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsStone.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}

	if (HasField(reader, "materialsWood"))
	{
		std::vector<std::string> tempMaterials;
		tempMaterials = reader.at("materialsWood").get<std::vector<std::string>>();

		for (auto iter = tempMaterials.begin(); iter != tempMaterials.end(); ++iter)
			output.materialsWood.push_back(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->data()));
	}
}

void from_json(const nlohmann::json &reader, CTreasureType &output)
{
	output.tier = reader.value("tier", 0);
	output.maxTreasureAmount = reader.value("maxTreasureAmount", 0);
	output.lootChance = reader.value("lootChance", 0.0f);
	output.maxMundaneAmount = reader.value("maxMundaneAmount", 0);
	output.mundaneLootChance = reader.value("mundaneLootChance", 0.0f);
	output.qualityModifier = reader.value("qualityModifier", 0.0f);
}

DEFINE_UNPACK_JSON(CTreasureProfile)
{
	if (HasField(reader, "options"))
	{
		if (HasField(reader.at("options"), "extraMutations"))
			extraMutations = reader.at("options").value("extraMutations", false);
		if (HasField(reader.at("options"), "rareJunkChance"))
			rareJunkChance = reader.at("options").value("rareJunkChance", 0.0f);
	}

	if (HasField(reader, "tiers"))
	{
		std::vector<CTreasureTier> tempTiers;
		tempTiers = reader.at("tiers").get<std::vector<CTreasureTier>>();

		for (auto iter = tempTiers.begin(); iter != tempTiers.end(); ++iter)
			tiers.emplace(iter->tierId, (*iter));
	}

	if (HasField(reader, "meleeWeapons"))
		meleeWeapons = reader.at("meleeWeapons").get<std::vector<CTreasureProfileCategory>>();
	if (HasField(reader, "missileWeapons"))
		missileWeapons = reader.at("missileWeapons").get<std::vector<CTreasureProfileCategory>>();
	if (HasField(reader, "casters"))
		casters = reader.at("casters");
	if (HasField(reader, "armor"))
		armor = reader.at("armor");
	if (HasField(reader, "clothing"))
		clothing = reader.at("clothing").get<std::vector<CTreasureProfileCategory>>();
	if (HasField(reader, "jewelry"))
		jewelry = reader.at("jewelry").get<std::vector<CTreasureProfileCategory>>();
	if (HasField(reader, "miscItems"))
		miscItems = reader.at("miscItems").get<std::vector<CTreasureProfileCategory>>();
	if (HasField(reader, "scrolls"))
		scrolls = reader.at("scrolls").get<std::vector<CTreasureProfileCategory>>();

	if (HasField(reader, "spellProperties"))
		spellProperties = reader.at("spellProperties");

	if (HasField(reader, "meleeWeaponSpells"))
		meleeWeaponSpells = reader.at("meleeWeaponSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "missileWeaponSpells"))
		missileWeaponSpells = reader.at("missileWeaponSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "casterSpells"))
		casterSpells = reader.at("casterSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "shieldSpells"))
		shieldSpells = reader.at("shieldSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "jewelrySpells"))
		jewelrySpells = reader.at("jewelrySpells").get<std::vector<CPossibleSpells>>();

	if (HasField(reader, "clothingSpells"))
		clothingSpells = reader.at("clothingSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "clothingHeadSpells"))
		clothingHeadSpells = reader.at("clothingHeadSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "clothingHandsSpells"))
		clothingHandsSpells = reader.at("clothingHandsSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "clothingFeetSpells"))
		clothingFeetSpells = reader.at("clothingFeetSpells").get<std::vector<CPossibleSpells>>();

	if (HasField(reader, "armorItemSpells"))
		armorItemSpells = reader.at("armorItemSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "armorHeadSpells"))
		armorHeadSpells = reader.at("armorHeadSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "armorUpperBodySpells"))
		armorUpperBodySpells = reader.at("armorUpperBodySpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "armorHandsSpells"))
		armorHandsSpells = reader.at("armorHandsSpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "armorLowerBodySpells"))
		armorLowerBodySpells = reader.at("armorLowerBodySpells").get<std::vector<CPossibleSpells>>();
	if (HasField(reader, "armorFeetSpells"))
		armorFeetSpells = reader.at("armorFeetSpells").get<std::vector<CPossibleSpells>>();

	if (HasField(reader, "materialProperties"))
	{
		std::map<std::string, CMaterialProperties> tempMaterialProperties;
		tempMaterialProperties = reader.at("materialProperties").get<std::map<std::string, CMaterialProperties>>();

		for (auto iter = tempMaterialProperties.begin(); iter != tempMaterialProperties.end(); ++iter)
			materialProperties.emplace(g_pTreasureFactory->TranslateMaterialStringToEnumValue(iter->first), iter->second);
	}

	if (HasField(reader, "workmanshipProperties"))
	{
		// nlohmann::json doesn't like int keys in maps, so we read as string and convert to int.
		std::map<std::string, CWorkmanshipProperties> tempWorkmanshipProperties;
		tempWorkmanshipProperties = reader.at("workmanshipProperties").get<std::map<std::string, CWorkmanshipProperties>>();

		for (auto iter = tempWorkmanshipProperties.begin(); iter != tempWorkmanshipProperties.end(); ++iter)
			workmanshipProperties.emplace(atoi(iter->first.c_str()), iter->second);
	}

	if (HasField(reader, "chestTreasureTypeReplacementTable"))
	{
		// nlohmann::json doesn't like int keys in maps, so we read as string and convert to int.
		std::map<std::string, int> tempChestTreasureTypeReplacementTable;
		tempChestTreasureTypeReplacementTable = reader.at("chestTreasureTypeReplacementTable").get<std::map<std::string, int>>();

		for (auto iter = tempChestTreasureTypeReplacementTable.begin(); iter != tempChestTreasureTypeReplacementTable.end(); ++iter)
			chestTreasureTypeReplacementTable.emplace(atoi(iter->first.c_str()), iter->second);
	}

	if (HasField(reader, "treasureTypeOverrides"))
	{
		// nlohmann::json doesn't like int keys in maps, so we read as string and convert to int.
		std::map<std::string, CTreasureType> tempTreasureTypeOverrides;
		tempTreasureTypeOverrides = reader.at("treasureTypeOverrides").get<std::map<std::string, CTreasureType>>();

		for (auto iter = tempTreasureTypeOverrides.begin(); iter != tempTreasureTypeOverrides.end(); ++iter)
			treasureTypeOverrides.emplace(atoi(iter->first.c_str()), iter->second);
	}
	return true;
}

DEFINE_PACK_JSON(CTreasureProfile)
{
	UNFINISHED();
}

void CTreasureProfile::ComputeUniqueSpellIds(std::vector<CPossibleSpells> spellList)
{
	for (int i = 0; i < spellList.size(); i++)
	{
		CPossibleSpells spell = spellList[i];
		int id;
		if (spellNameToIdMap.find(spell.spellName) != spellNameToIdMap.end())
		{
			id = nextSpellId++;
			spell.id = id;
			spellNameToIdMap.emplace(spell.spellName, id);
			spells.emplace(id, spell);
		}
		else
			id = spellNameToIdMap[spell.spellName];
		spell.id = id;
		spellList[i] = spell;
	}
}

void CTreasureProfile::Initialize()
{
	if (isInitialized)
		return;

	isInitialized = true;

	nextSpellId = 0;

	ComputeUniqueSpellIds(meleeWeaponSpells);
	ComputeUniqueSpellIds(missileWeaponSpells);
	ComputeUniqueSpellIds(casterSpells);
	ComputeUniqueSpellIds(shieldSpells);
	ComputeUniqueSpellIds(clothingSpells);
	ComputeUniqueSpellIds(jewelrySpells);

	ComputeUniqueSpellIds(armorItemSpells);
	ComputeUniqueSpellIds(armorHeadSpells);
	ComputeUniqueSpellIds(armorUpperBodySpells);
	ComputeUniqueSpellIds(armorHandsSpells);
	ComputeUniqueSpellIds(armorLowerBodySpells);
	ComputeUniqueSpellIds(armorFeetSpells);

	ComputeUniqueSpellIds(clothingHeadSpells);
	ComputeUniqueSpellIds(clothingHandsSpells);
	ComputeUniqueSpellIds(clothingFeetSpells);

	for (auto iter = tiers.begin(); iter != tiers.end(); ++iter)
	{
		CTreasureTier *tier = &iter->second;

		float totalProportion = tier->meleeWeaponsLootProportion +
			tier->missileWeaponsLootProportion +
			tier->castersLootProportion +
			tier->armorLootProportion +
			tier->clothingLootProportion +
			tier->jewelryLootProportion;

		float meleeWeaponChance = tier->meleeWeaponsLootProportion / totalProportion;
		float missileWeaponChance = tier->missileWeaponsLootProportion / totalProportion;
		float casterChance = tier->castersLootProportion / totalProportion;
		float armorChance = tier->armorLootProportion / totalProportion;
		float clothingChance = tier->clothingLootProportion / totalProportion;
		float jewelryChance = tier->jewelryLootProportion / totalProportion;

		float cumulativeMeleeWeaponChance = meleeWeaponChance;
		float cumulativeMissileWeaponChance = cumulativeMeleeWeaponChance + missileWeaponChance;
		float cumulativeCasterChance = cumulativeMissileWeaponChance + casterChance;
		float cumulativeArmorChance = cumulativeCasterChance + armorChance;
		float cumulativeClothingChance = cumulativeArmorChance + clothingChance;
		float cumulativeJewelryChance = cumulativeClothingChance + jewelryChance;

		tier->categoryChances.emplace(cumulativeMeleeWeaponChance, eTreasureCategory::TreasureCategory_Melee_Weapon);
		tier->categoryChances.emplace(cumulativeMissileWeaponChance, eTreasureCategory::TreasureCategory_Missile_Weapon);
		tier->categoryChances.emplace(cumulativeCasterChance, eTreasureCategory::TreasureCategory_Caster);
		tier->categoryChances.emplace(cumulativeArmorChance, eTreasureCategory::TreasureCategory_Armor);
		tier->categoryChances.emplace(cumulativeClothingChance, eTreasureCategory::TreasureCategory_Clothing);
		tier->categoryChances.emplace(cumulativeJewelryChance, eTreasureCategory::TreasureCategory_Jewelry);

		for (auto iter2 = iter->second.commonArmorCategoryNames.begin(); iter2 != iter->second.commonArmorCategoryNames.end(); ++iter2)
		{
			for (auto iter3 = armor.categories.begin(); iter3 != armor.categories.end(); ++iter3)
			{
				if (iter3->category == iter2->data())
				{
					iter->second.commonArmorCategories.push_back(&(*iter3));
					break;
				}
			}
		}

		for (auto iter2 = iter->second.rareArmorCategoryNames.begin(); iter2 != iter->second.rareArmorCategoryNames.end(); ++iter2)
		{
			for (auto iter3 = armor.categories.begin(); iter3 != armor.categories.end(); ++iter3)
			{
				if (iter3->category == iter2->data())
				{
					iter->second.rareArmorCategories.push_back(&(*iter3));
					break;
				}
			}
		}
	}
}

eTreasureCategory CTreasureTier::GetRandomTreasureCategory()
{
	float roll = getRandomDouble(1.0);
	for each(auto entry in categoryChances)
	{
		if (entry.first >= roll)
			return entry.second;
	}
	return categoryChances.rbegin()->second;
}

CTreasureFactory::CTreasureFactory()
{
	_materialTypeTranslationTable.emplace("none", Undef_MaterialType);
	_materialTypeTranslationTable.emplace("ceramic", Ceramic_MaterialType);
	_materialTypeTranslationTable.emplace("porcelain", Porcelain_MaterialType);
	_materialTypeTranslationTable.emplace("cloth", Cloth_MaterialType);
	_materialTypeTranslationTable.emplace("linen", Linen_MaterialType);
	_materialTypeTranslationTable.emplace("satin", Satin_MaterialType);
	_materialTypeTranslationTable.emplace("silk", Silk_MaterialType);
	_materialTypeTranslationTable.emplace("velvet", Velvet_MaterialType);
	_materialTypeTranslationTable.emplace("wool", Wool_MaterialType);
	_materialTypeTranslationTable.emplace("gem", Gem_MaterialType);
	_materialTypeTranslationTable.emplace("agate", Agate_MaterialType);
	_materialTypeTranslationTable.emplace("amber", Amber_MaterialType);
	_materialTypeTranslationTable.emplace("amethyst", Amethyst_MaterialType);
	_materialTypeTranslationTable.emplace("aquamarine", Aquamarine_MaterialType);
	_materialTypeTranslationTable.emplace("azurite", Azurite_MaterialType);
	_materialTypeTranslationTable.emplace("blackGarnet", Black_Garnet_MaterialType);
	_materialTypeTranslationTable.emplace("blackOpal", Black_Opal_MaterialType);
	_materialTypeTranslationTable.emplace("bloodstone", Bloodstone_MaterialType);
	_materialTypeTranslationTable.emplace("carnelian", Carnelian_MaterialType);
	_materialTypeTranslationTable.emplace("citrine", Citrine_MaterialType);
	_materialTypeTranslationTable.emplace("diamond", Diamond_MaterialType);
	_materialTypeTranslationTable.emplace("emerald", Emerald_MaterialType);
	_materialTypeTranslationTable.emplace("fireOpal", Fire_Opal_MaterialType);
	_materialTypeTranslationTable.emplace("greenGarnet", Green_Garnet_MaterialType);
	_materialTypeTranslationTable.emplace("greenJade", Green_Jade_MaterialType);
	_materialTypeTranslationTable.emplace("hematite", Hematite_MaterialType);
	_materialTypeTranslationTable.emplace("imperialTopaz", Imperial_Topaz_MaterialType);
	_materialTypeTranslationTable.emplace("jet", Jet_MaterialType);
	_materialTypeTranslationTable.emplace("lapisLazuli", Lapis_Lazuli_MaterialType);
	_materialTypeTranslationTable.emplace("lavenderJade", Lavender_Jade_MaterialType);
	_materialTypeTranslationTable.emplace("malachite", Malachite_MaterialType);
	_materialTypeTranslationTable.emplace("moonstone", Moonstone_MaterialType);
	_materialTypeTranslationTable.emplace("onyx", Onyx_MaterialType);
	_materialTypeTranslationTable.emplace("opal", Opal_MaterialType);
	_materialTypeTranslationTable.emplace("peridot", Peridot_MaterialType);
	_materialTypeTranslationTable.emplace("redGarnet", Red_Garnet_MaterialType);
	_materialTypeTranslationTable.emplace("RedJade", Red_Jade_MaterialType);
	_materialTypeTranslationTable.emplace("roseQuartz", Rose_Quartz_MaterialType);
	_materialTypeTranslationTable.emplace("ruby", Ruby_MaterialType);
	_materialTypeTranslationTable.emplace("sapphire", Sapphire_MaterialType);
	_materialTypeTranslationTable.emplace("smokeyQuartz", Smoky_Quartz_MaterialType);
	_materialTypeTranslationTable.emplace("sunstone", Sunstone_MaterialType);
	_materialTypeTranslationTable.emplace("tigerEye", Tiger_Eye_MaterialType);
	_materialTypeTranslationTable.emplace("tourmaline", Tourmaline_MaterialType);
	_materialTypeTranslationTable.emplace("turquoise", Turquoise_MaterialType);
	_materialTypeTranslationTable.emplace("whiteJade", White_Jade_MaterialType);
	_materialTypeTranslationTable.emplace("whiteQuartz", White_Quartz_MaterialType);
	_materialTypeTranslationTable.emplace("whiteSapphire", White_Sapphire_MaterialType);
	_materialTypeTranslationTable.emplace("yellowGarnet", Yellow_Garnet_MaterialType);
	_materialTypeTranslationTable.emplace("yellowTopaz", Yellow_Topaz_MaterialType);
	_materialTypeTranslationTable.emplace("zircon", Zircon_MaterialType);
	_materialTypeTranslationTable.emplace("ivory", Ivory_MaterialType);
	_materialTypeTranslationTable.emplace("leather", Leather_MaterialType);
	_materialTypeTranslationTable.emplace("armoredilloHide", Armoredillo_Hide_MaterialType);
	_materialTypeTranslationTable.emplace("gromnieHide", Gromnie_Hide_MaterialType);
	_materialTypeTranslationTable.emplace("reedSharkHide", Reed_Shark_Hide_MaterialType);
	_materialTypeTranslationTable.emplace("metal", Metal_MaterialType);
	_materialTypeTranslationTable.emplace("brass", Brass_MaterialType);
	_materialTypeTranslationTable.emplace("bronze", Bronze_MaterialType);
	_materialTypeTranslationTable.emplace("copper", Copper_MaterialType);
	_materialTypeTranslationTable.emplace("gold", Gold_MaterialType);
	_materialTypeTranslationTable.emplace("iron", Iron_MaterialType);
	_materialTypeTranslationTable.emplace("pyreal", Pyreal_MaterialType);
	_materialTypeTranslationTable.emplace("silver", Silver_MaterialType);
	_materialTypeTranslationTable.emplace("steel", Steel_MaterialType);
	_materialTypeTranslationTable.emplace("stone", Stone_MaterialType);
	_materialTypeTranslationTable.emplace("alabaster", Alabaster_MaterialType);
	_materialTypeTranslationTable.emplace("granite", Granite_MaterialType);
	_materialTypeTranslationTable.emplace("marble", Marble_MaterialType);
	_materialTypeTranslationTable.emplace("obsidian", Obsidian_MaterialType);
	_materialTypeTranslationTable.emplace("sandstone", Sandstone_MaterialType);
	_materialTypeTranslationTable.emplace("serpentine", Serpentine_MaterialType);
	_materialTypeTranslationTable.emplace("wood", Wood_MaterialType);
	_materialTypeTranslationTable.emplace("ebony", Ebony_MaterialType);
	_materialTypeTranslationTable.emplace("mahogany", Mahogany_MaterialType);
	_materialTypeTranslationTable.emplace("oak", Oak_MaterialType);
	_materialTypeTranslationTable.emplace("pine", Pine_MaterialType);
	_materialTypeTranslationTable.emplace("teak", Teak_MaterialType);

	_skillTypeTranslationTable.emplace("None", UNDEF_SKILL);
	_skillTypeTranslationTable.emplace("Axe", AXE_SKILL);
	_skillTypeTranslationTable.emplace("Bow", BOW_SKILL);
	_skillTypeTranslationTable.emplace("Crossbow", CROSSBOW_SKILL);
	_skillTypeTranslationTable.emplace("Dagger", DAGGER_SKILL);
	_skillTypeTranslationTable.emplace("Mace", MACE_SKILL);
	_skillTypeTranslationTable.emplace("MeleeDefense", MELEE_DEFENSE_SKILL);
	_skillTypeTranslationTable.emplace("MissileDefense", MISSILE_DEFENSE_SKILL);
	_skillTypeTranslationTable.emplace("Sling", SLING_SKILL);
	_skillTypeTranslationTable.emplace("Spear", SPEAR_SKILL);
	_skillTypeTranslationTable.emplace("Staff", STAFF_SKILL);
	_skillTypeTranslationTable.emplace("Sword", SWORD_SKILL);
	_skillTypeTranslationTable.emplace("ThrownWeapon", THROWN_WEAPON_SKILL);
	_skillTypeTranslationTable.emplace("UnarmedCombat", UNARMED_COMBAT_SKILL);
	_skillTypeTranslationTable.emplace("ArcaneLore", ARCANE_LORE_SKILL);
	_skillTypeTranslationTable.emplace("MagicDefense", MAGIC_DEFENSE_SKILL);
	_skillTypeTranslationTable.emplace("ManaConversion", MANA_CONVERSION_SKILL);
	_skillTypeTranslationTable.emplace("SpellCraft", SPELLCRAFT_SKILL);
	_skillTypeTranslationTable.emplace("ItemAppraisal", ITEM_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("PersonalAppraisal", PERSONAL_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("Deception", DECEPTION_SKILL);
	_skillTypeTranslationTable.emplace("Healing", HEALING_SKILL);
	_skillTypeTranslationTable.emplace("Jump", JUMP_SKILL);
	_skillTypeTranslationTable.emplace("Lockpick", LOCKPICK_SKILL);
	_skillTypeTranslationTable.emplace("Run", RUN_SKILL);
	_skillTypeTranslationTable.emplace("Awareness", AWARENESS_SKILL);
	_skillTypeTranslationTable.emplace("ArmsAndArmorRepair", ARMS_AND_ARMOR_REPAIR_SKILL);
	_skillTypeTranslationTable.emplace("CreatureAppraisal", CREATURE_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("WeaponAppraisal", WEAPON_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("ArmorAppraisal", ARMOR_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("MagicItemAppraisal", MAGIC_ITEM_APPRAISAL_SKILL);
	_skillTypeTranslationTable.emplace("CreatureEnchantment", CREATURE_ENCHANTMENT_SKILL);
	_skillTypeTranslationTable.emplace("ItemEnchantment", ITEM_ENCHANTMENT_SKILL);
	_skillTypeTranslationTable.emplace("LifeMagic", LIFE_MAGIC_SKILL);
	_skillTypeTranslationTable.emplace("WarMagic", WAR_MAGIC_SKILL);
	_skillTypeTranslationTable.emplace("Leadership", LEADERSHIP_SKILL);
	_skillTypeTranslationTable.emplace("Loyalty", LOYALTY_SKILL);
	_skillTypeTranslationTable.emplace("Fletching", FLETCHING_SKILL);
	_skillTypeTranslationTable.emplace("Alchemy", ALCHEMY_SKILL);
	_skillTypeTranslationTable.emplace("Cooking", COOKING_SKILL);
	_skillTypeTranslationTable.emplace("Salvaging", SALVAGING_SKILL);
	_skillTypeTranslationTable.emplace("TwoHandedCombat", TWO_HANDED_COMBAT_SKILL);
	_skillTypeTranslationTable.emplace("GearCraft", GEARCRAFT_SKILL);
	_skillTypeTranslationTable.emplace("VoidMagic", VOID_MAGIC_SKILL);
	_skillTypeTranslationTable.emplace("HeavyWeapons", HEAVY_WEAPONS_SKILL);
	_skillTypeTranslationTable.emplace("LightWeapons", LIGHT_WEAPONS_SKILL);
	_skillTypeTranslationTable.emplace("FinesseWeapons", FINESSE_WEAPONS_SKILL);
	_skillTypeTranslationTable.emplace("MissileWeapons", MISSILE_WEAPONS_SKILL);
	_skillTypeTranslationTable.emplace("ShieldSkill", SHIELD_SKILL);
	_skillTypeTranslationTable.emplace("DualWield", DUAL_WIELD_SKILL);
	_skillTypeTranslationTable.emplace("Recklessness", RECKLESSNESS_SKILL);
	_skillTypeTranslationTable.emplace("SneakAttack", SNEAK_ATTACK_SKILL);
	_skillTypeTranslationTable.emplace("DirtyFighting", DIRTY_FIGHTING_SKILL);
	_skillTypeTranslationTable.emplace("Challenge", CHALLENGE_SKILL);
	_skillTypeTranslationTable.emplace("Summoning", SUMMONING_SKILL);

	_creatureTypeTranslationTable.emplace("none", none);
	_creatureTypeTranslationTable.emplace("olthoi", olthoi);
	_creatureTypeTranslationTable.emplace("banderling", banderling);
	_creatureTypeTranslationTable.emplace("drudge", drudge);
	_creatureTypeTranslationTable.emplace("mosswart", mosswart);
	_creatureTypeTranslationTable.emplace("lugian", lugian);
	_creatureTypeTranslationTable.emplace("tumerok", tumerok);
	_creatureTypeTranslationTable.emplace("mite", mite);
	_creatureTypeTranslationTable.emplace("tusker", tusker);
	_creatureTypeTranslationTable.emplace("phyntosWasp", phyntosWasp);
	_creatureTypeTranslationTable.emplace("rat", rat);
	_creatureTypeTranslationTable.emplace("auroch", auroch);
	_creatureTypeTranslationTable.emplace("cow", cow);
	_creatureTypeTranslationTable.emplace("golem", golem);
	_creatureTypeTranslationTable.emplace("undead", undead);
	_creatureTypeTranslationTable.emplace("gromnie", gromnie);
	_creatureTypeTranslationTable.emplace("reedshark", reedshark);
	_creatureTypeTranslationTable.emplace("armoredillo", armoredillo);
	_creatureTypeTranslationTable.emplace("fae", fae);
	_creatureTypeTranslationTable.emplace("virindi", virindi);
	_creatureTypeTranslationTable.emplace("wisp", wisp);
	_creatureTypeTranslationTable.emplace("knathtaed", knathtaed);
	_creatureTypeTranslationTable.emplace("shadow", shadow);
	_creatureTypeTranslationTable.emplace("mattekar", mattekar);
	_creatureTypeTranslationTable.emplace("mumiyah", mumiyah);
	_creatureTypeTranslationTable.emplace("rabbit", rabbit);
	_creatureTypeTranslationTable.emplace("sclavus", sclavus);
	_creatureTypeTranslationTable.emplace("shallowsShark", shallowsShark);
	_creatureTypeTranslationTable.emplace("monouga", monouga);
	_creatureTypeTranslationTable.emplace("zefir", zefir);
	_creatureTypeTranslationTable.emplace("skeleton", skeleton);
	_creatureTypeTranslationTable.emplace("human", human);
	_creatureTypeTranslationTable.emplace("shreth", shreth);
	_creatureTypeTranslationTable.emplace("chittick", chittick);
	_creatureTypeTranslationTable.emplace("moarsman", moarsman);
	_creatureTypeTranslationTable.emplace("slithis", slithis);
	_creatureTypeTranslationTable.emplace("fireElemental", fireElemental);
	_creatureTypeTranslationTable.emplace("snowman", snowman);
	_creatureTypeTranslationTable.emplace("bunny", bunny);
	_creatureTypeTranslationTable.emplace("lightningElemental", lightningElemental);
	_creatureTypeTranslationTable.emplace("grievver", grievver);
	_creatureTypeTranslationTable.emplace("niffis", niffis);
	_creatureTypeTranslationTable.emplace("ursuin", ursuin);
	_creatureTypeTranslationTable.emplace("crystal", crystal);
	_creatureTypeTranslationTable.emplace("hollowMinion", hollowMinion);
	_creatureTypeTranslationTable.emplace("scarecrow", scarecrow);
	_creatureTypeTranslationTable.emplace("idol", idol);
	_creatureTypeTranslationTable.emplace("empyrean", empyrean);
	_creatureTypeTranslationTable.emplace("hopeslayer", hopeslayer);
	_creatureTypeTranslationTable.emplace("doll", doll);
	_creatureTypeTranslationTable.emplace("marionette", marionette);
	_creatureTypeTranslationTable.emplace("carenzi", carenzi);
	_creatureTypeTranslationTable.emplace("siraluun", siraluun);
	_creatureTypeTranslationTable.emplace("aunTumerok", aunTumerok);
	_creatureTypeTranslationTable.emplace("heaTumerok", heaTumerok);
	_creatureTypeTranslationTable.emplace("simulacrum", simulacrum);
	_creatureTypeTranslationTable.emplace("acidElemental", acidElemental);
	_creatureTypeTranslationTable.emplace("frostElemental", frostElemental);
	_creatureTypeTranslationTable.emplace("elemental", elemental);
	_creatureTypeTranslationTable.emplace("statue", statue);
	_creatureTypeTranslationTable.emplace("wall", wall);
	_creatureTypeTranslationTable.emplace("alteredHuman", alteredHuman);
	_creatureTypeTranslationTable.emplace("device", device);
	_creatureTypeTranslationTable.emplace("harbinger", harbinger);
	_creatureTypeTranslationTable.emplace("darkSarcophagus", darkSarcophagus);
	_creatureTypeTranslationTable.emplace("chicken", chicken);
	_creatureTypeTranslationTable.emplace("gotrokLugian", gotrokLugian);
	_creatureTypeTranslationTable.emplace("margul", margul);
	_creatureTypeTranslationTable.emplace("bleachedRabbit", bleachedRabbit);
	_creatureTypeTranslationTable.emplace("nastyRabbit", nastyRabbit);
	_creatureTypeTranslationTable.emplace("grimacingRabbit", grimacingRabbit);
	_creatureTypeTranslationTable.emplace("burun", burun);
	_creatureTypeTranslationTable.emplace("target", target);
	_creatureTypeTranslationTable.emplace("ghost", ghost);
	_creatureTypeTranslationTable.emplace("fiun", fiun);
	_creatureTypeTranslationTable.emplace("eater", eater);
	_creatureTypeTranslationTable.emplace("penguin", penguin);
	_creatureTypeTranslationTable.emplace("ruschk", ruschk);
	_creatureTypeTranslationTable.emplace("thrungus", thrungus);
	_creatureTypeTranslationTable.emplace("viamontianKnight", viamontianKnight);
	_creatureTypeTranslationTable.emplace("remoran", remoran);
	_creatureTypeTranslationTable.emplace("swarm", swarm);
	_creatureTypeTranslationTable.emplace("moar", moar);
	_creatureTypeTranslationTable.emplace("enchantedArms", enchantedArms);
	_creatureTypeTranslationTable.emplace("sleech", sleech);
	_creatureTypeTranslationTable.emplace("mukkir", mukkir);
	_creatureTypeTranslationTable.emplace("merwart", merwart);
	_creatureTypeTranslationTable.emplace("food", food);
	_creatureTypeTranslationTable.emplace("paradoxOlthoi", paradoxOlthoi);
	_creatureTypeTranslationTable.emplace("energy", energy);
	_creatureTypeTranslationTable.emplace("apparition", apparition);
	_creatureTypeTranslationTable.emplace("aerbax", aerbax);
	_creatureTypeTranslationTable.emplace("touched", touched);
	_creatureTypeTranslationTable.emplace("blightedMoarsman", blightedMoarsman);
	_creatureTypeTranslationTable.emplace("gearKnight", gearKnight);
	_creatureTypeTranslationTable.emplace("gurog", gurog);
	_creatureTypeTranslationTable.emplace("anekshay", anekshay);
}

CTreasureFactory::~CTreasureFactory()
{
	SafeDelete(_TreasureProfile)
}

MaterialType CTreasureFactory::TranslateMaterialStringToEnumValue(std::string stringValue)
{
	if (_materialTypeTranslationTable.find(stringValue) != _materialTypeTranslationTable.end())
		return _materialTypeTranslationTable[stringValue];
	return MaterialType::Undef_MaterialType;
}

STypeSkill CTreasureFactory::TranslateSkillStringToEnumValue(std::string stringValue)
{
	if (_skillTypeTranslationTable.find(stringValue) != _skillTypeTranslationTable.end())
		return _skillTypeTranslationTable[stringValue];
	return STypeSkill::UNDEF_SKILL;
}

CreatureType CTreasureFactory::TranslateCreatureStringToEnumValue(std::string stringValue)
{
	if (_creatureTypeTranslationTable.find(stringValue) != _creatureTypeTranslationTable.end())
		return _creatureTypeTranslationTable[stringValue];
	return CreatureType::none;
}

void CTreasureFactory::Initialize()
{
	LOG(Data, Normal, "Loading treasure generation profile...\n");

	std::ifstream fileStream("data\\json\\treasureProfile.json");

	if (fileStream.is_open())
	{
		json jsonData;
		try
		{
			fileStream >> jsonData;
			fileStream.close();
		}
		catch (const nlohmann::detail::parse_error &e)
		{
			LOG(Data, Error, "----------------------\nError parsing treasureProfile.json:\n%s\n----------------------\n", e.what());
			return;
		}

		_TreasureProfile = new CTreasureProfile();

		try
		{
			_TreasureProfile->UnPackJson(jsonData);
			_TreasureProfile->Initialize();
		}
		catch (...)
		{
			LOG(Data, Error, "----------------------\nError loading treasure generation profile!\n----------------------\n");
			SafeDelete(_TreasureProfile);
			return;
		}
	}

	LOG(Data, Normal, "Finished loading treasure generation profile.\n");
}

CWeenieObject *CreateFromEntry(PackableList<TreasureEntry>::iterator entry , unsigned int ptid, float shade)
{
	if (CWeenieObject *newItem = g_pWeenieFactory->CreateWeenieByClassID(entry->_wcid))
	{
		if (entry->_amount > 1)
		{
			int maxStackSize = newItem->InqIntQuality(MAX_STACK_SIZE_INT, 1);
			if (maxStackSize > 1)
			{
				double minAmount = (double)entry->_amount * entry->_amountVariance;
				int modifiedAmount = round(Random::RollDice(minAmount, entry->_amount));
				newItem->SetStackSize(min(modifiedAmount, maxStackSize));
			}
		}

		if (ptid != 0)
			newItem->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);
		else if (entry->_ptid)
			newItem->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, entry->_ptid);

		if (shade > 0)
			newItem->m_Qualities.SetFloat(SHADE_FLOAT, shade);
		else if (entry->_shade > 0.0)
			newItem->m_Qualities.SetFloat(SHADE_FLOAT, entry->_shade);

		return newItem;
	}
	return NULL;
}

int SkipToNextSet(PackableList<TreasureEntry> *treasureList, int index)
{
	int i = index;
	while (i < treasureList->size())
	{
		PackableList<TreasureEntry>::iterator entry = treasureList->GetAt(i);
		if (entry->_setStart)
			return i;
		else if (entry->_hasSubSet)
			i = SkipToNextSet(treasureList, i + 2); //next entry is the sublist's first entry so skip 2.
		else
			i++;
	}
	return (int)treasureList->size();
}

int ProcessList(PackableList<TreasureEntry> *treasureList, int index, SItemListCreationData creationData)
{
	int i = index;
	bool alreadySpawnedInThisSet = false;
	while (i < treasureList->size())
	{
		PackableList<TreasureEntry>::iterator entry = treasureList->GetAt(i);

		if (entry->_continuesPreviousSet && alreadySpawnedInThisSet)
		{
			if (entry->_hasSubSet)
				i = SkipToNextSet(treasureList, i + 2);
			else
				i = SkipToNextSet(treasureList, i + 1);
			continue;
		}
		else
			alreadySpawnedInThisSet = false;

		double diceRoll = creationData.diceRoll;
		double chance = entry->_chance + creationData.accumulatedChance;
		creationData.accumulatedChance = chance;

		if (diceRoll <= chance)
		{
			if (CWeenieObject *newItem = CreateFromEntry(entry, creationData.ptid, creationData.shade))
			{
				g_pWeenieFactory->AddWeenieToDestination(newItem, creationData.parent, creationData.destinationType, creationData.isRegenLocationType, creationData.profile);
				creationData.amountCreated++;
				alreadySpawnedInThisSet = true;

				if (entry->_hasSubSet)
				{
					SItemListCreationData subListCreationData;
					subListCreationData.amountCreated = 0;
					subListCreationData.parent = creationData.parent;
					subListCreationData.destinationType = creationData.destinationType;
					subListCreationData.isRegenLocationType = creationData.isRegenLocationType;
					subListCreationData.treasureTypeOrWcid = creationData.treasureTypeOrWcid;
					subListCreationData.ptid = creationData.ptid;
					subListCreationData.shade = creationData.shade;
					subListCreationData.profile = creationData.profile;
					subListCreationData.accumulatedChance = 0.0;
					subListCreationData.diceRoll = Random::RollDice(0.0, 1.0);
					subListCreationData.isSubSet = true;

					i = ProcessList(treasureList, i + 1, subListCreationData);

					creationData.amountCreated += subListCreationData.amountCreated;
					continue;
				}

				//prepare the data for the next list.
				creationData.accumulatedChance = 0.0;
				creationData.diceRoll = Random::RollDice(0.0, 1.0);
				//and skip to it.
				if (creationData.isSubSet)
					return i + 1;
				else
					i = SkipToNextSet(treasureList, i + 1);
				continue;
			}
		}
		else if (entry->_hasSubSet)
		{
			i = SkipToNextSet(treasureList, i + 2); //next entry is the sublist's first entry so skip 2.
			continue;
		}
		i++;
	}
	return (int)treasureList->size();
}

int CTreasureFactory::GenerateFromTypeOrWcid(CWeenieObject *parent, int destinationType, bool isRegenLocationType, DWORD treasureTypeOrWcid, unsigned int ptid, float shade, const GeneratorProfile *profile)
{
	int amountCreated = 0;
	bool isTreasureType = false;

	if (isRegenLocationType && (destinationType & Treasure_RegenLocationType))
		isTreasureType = true;
	else if (!isRegenLocationType && destinationType & Treasure_DestinationType)
		isTreasureType = true;

	if (isTreasureType)
	{
		if (PackableList<TreasureEntry> *entry = g_pPortalDataEx->_treasureTableData._treasureList.lookup(treasureTypeOrWcid))
		{
			//we're a list of items
			SItemListCreationData creationData;
			creationData.amountCreated = 0;
			creationData.parent = parent;
			creationData.destinationType = destinationType;
			creationData.isRegenLocationType = isRegenLocationType;
			creationData.treasureTypeOrWcid = treasureTypeOrWcid;
			creationData.ptid = ptid;
			creationData.shade = shade;
			creationData.profile = profile;
			creationData.accumulatedChance = 0.0;
			creationData.diceRoll = Random::RollDice(0.0, 1.0);

			ProcessList(entry, 0, creationData);
		}
		else if (_TreasureProfile->treasureTypeOverrides.find(treasureTypeOrWcid) != _TreasureProfile->treasureTypeOverrides.end())
		{
			CTreasureType *entry = &_TreasureProfile->treasureTypeOverrides.at(treasureTypeOrWcid);
			return GenerateFromType(entry, parent, destinationType, isRegenLocationType, treasureTypeOrWcid, ptid, shade, profile);
		}
		else if (TreasureEntry2 *entry = g_pPortalDataEx->_treasureTableData.GetTreasureGenerationProfile(treasureTypeOrWcid))
		{
			//we're instructions for random treasure generation

			//setup some default values
			int tierId = entry->_tier;
			CTreasureTier *tier = &_TreasureProfile->tiers[tierId];
			if (tier == NULL)
				return amountCreated;
			float qualityModifier = entry->_lootQualityMod;
			int maxTreasureAmount = entry->_itemMaxAmount + entry->_magicItemMaxAmount;
			int maxMundaneAmount = entry->_mundaneItemMaxAmount;
			float lootChance = 0.5f;
			if (maxTreasureAmount == 1)
				lootChance = 1.0f;
			float mundaneLootChance = 0.5f;
			if (maxMundaneAmount == 1)
				mundaneLootChance = 1.0f;

			CTreasureType treasureType;
			treasureType.tier = tierId;
			treasureType.maxTreasureAmount = maxTreasureAmount;
			treasureType.lootChance = lootChance;
			treasureType.maxMundaneAmount = maxMundaneAmount;
			treasureType.mundaneLootChance = mundaneLootChance;
			treasureType.qualityModifier = qualityModifier;

			return GenerateFromType(&treasureType, parent, destinationType, isRegenLocationType, treasureTypeOrWcid, ptid, shade, profile);
		}
	}
	else
	{
		//we're a wcid
		if (CWeenieObject *newItem = g_pWeenieFactory->CreateWeenieByClassID(treasureTypeOrWcid))
		{
			g_pWeenieFactory->AddWeenieToDestination(newItem, parent, destinationType, isRegenLocationType, profile);
			amountCreated++;
		}
	}

	return amountCreated;
}

int CTreasureFactory::GenerateFromType(CTreasureType *type, CWeenieObject * parent, int destinationType, bool isRegenLocationType, DWORD treasureTypeOrWcid, unsigned int ptid, float shade, const GeneratorProfile * profile)
{
	int amountCreated = 0;
	int tierId = type->tier;
	CTreasureTier *tier = &_TreasureProfile->tiers[type->tier];
	if (tier == NULL)
		return amountCreated;
	int maxTreasureAmount = type->maxTreasureAmount;
	int maxMundaneAmount = type->maxMundaneAmount;
	float lootChance = type->lootChance;
	float mundaneLootChance = type->mundaneLootChance;
	float qualityModifier = type->qualityModifier;

	if (parent->AsCorpse())
	{
		//we're a corpse
		maxTreasureAmount = round(maxTreasureAmount * tier->lootAmountMultiplier);
		lootChance = tier->lootChance;
		mundaneLootChance = tier->miscLootChance;

		int level = parent->InqIntQuality(LEVEL_INT, 0);
		if (level >= tier->qualityLootLevelThreshold && qualityModifier < tier->qualityLootModifier)
			qualityModifier = tier->qualityLootModifier;
	}
	else if (parent->AsContainer() && !parent->AsMonster())
	{
		//we're a container so apply container related stuff.

		//Turbine did a bad job of converting chests to the new treasure profiles resulting in chests with loot that
		//doesnt match the level of the area they are located in. This is a quick fix for that. A better fix would be
		//to edit the chest generator tables themselves. But even then it would require changing some chest wcids
		//because there are chests wcids that are uses both in newbie and high level areas.
		int newTreasureTypeOrWcid = 0;
		for each(auto entry in _TreasureProfile->chestTreasureTypeReplacementTable)
		{
			if (treasureTypeOrWcid == entry.first)
			{
				newTreasureTypeOrWcid = entry.second;
				break;
			}
		}

		if (newTreasureTypeOrWcid != 0 && treasureTypeOrWcid != newTreasureTypeOrWcid)
		{
			if (_TreasureProfile->treasureTypeOverrides.find(treasureTypeOrWcid) != _TreasureProfile->treasureTypeOverrides.end())
			{
				CTreasureType *entry = &_TreasureProfile->treasureTypeOverrides.at(treasureTypeOrWcid);
				treasureTypeOrWcid = newTreasureTypeOrWcid;

				tierId = type->tier;
				tier = &_TreasureProfile->tiers[type->tier];
				if (tier == NULL)
					return amountCreated;
				qualityModifier = type->qualityModifier;
				maxTreasureAmount = type->maxTreasureAmount;
				if (maxTreasureAmount == 1)
					maxTreasureAmount = 3;

				if (qualityModifier < tier->qualityLootModifier)
					qualityModifier = tier->qualityLootModifier;
			}
			else if (TreasureEntry2 *entry = g_pPortalDataEx->_treasureTableData.GetTreasureGenerationProfile(treasureTypeOrWcid))
			{
				treasureTypeOrWcid = newTreasureTypeOrWcid;

				tierId = entry->_tier;
				tier = &_TreasureProfile->tiers[tierId];
				if (tier == NULL)
					return amountCreated;
				qualityModifier = entry->_lootQualityMod;
				maxTreasureAmount = entry->_itemMaxAmount + entry->_magicItemMaxAmount;
				if (maxTreasureAmount == 1)
					maxTreasureAmount = 3;

				if (qualityModifier < tier->qualityLootModifier)
					qualityModifier = tier->qualityLootModifier;
			}
			else
				return amountCreated;
		}

		maxTreasureAmount = round(maxTreasureAmount * tier->chestLootAmountMultiplier);
		lootChance = tier->chestLootChance;

		//chests do not drop mundane items.
		mundaneLootChance = 0;
		maxMundaneAmount = 0;
	}

	for (int i = 0; i < maxTreasureAmount; i++)
	{
		if (getRandomNumberExclusive(100) < lootChance * 100)
		{
			if (CWeenieObject *newItem = GenerateTreasure(tierId, tier->GetRandomTreasureCategory(), qualityModifier))
			{
				g_pWeenieFactory->AddWeenieToDestination(newItem, parent, destinationType, isRegenLocationType, profile);
				amountCreated++;
			}
		}
	}

	for (int i = 0; i < maxMundaneAmount; i++)
	{
		if (getRandomNumberExclusive(100) < mundaneLootChance * 100)
		{
			if (CWeenieObject *newItem = GenerateMundaneItem(tier))
			{
				g_pWeenieFactory->AddWeenieToDestination(newItem, parent, destinationType, isRegenLocationType, profile);
				amountCreated++;
			}
		}
	}

	if (getRandomNumberExclusive(100) < tier->scrollLootChance * 100)
	{
		if (CWeenieObject *newItem = GenerateScroll(tier))
		{
			g_pWeenieFactory->AddWeenieToDestination(newItem, parent, destinationType, isRegenLocationType, profile);
			amountCreated++;
		}
	}
	return amountCreated;
}

CWeenieObject *CTreasureFactory::GenerateMundaneItem(CTreasureTier *tier)
{
	int miscItemWcid = 0;
	if (tier->miscItemsCategories.size() == 0)
		return NULL;
	std::string miscCategory = tier->miscItemsCategories[getRandomNumberExclusive((int)tier->miscItemsCategories.size())];

	for each (CTreasureProfileCategory category in _TreasureProfile->miscItems)
	{
		if (category.category == miscCategory)
		{
			if (!category.entries.empty())
				miscItemWcid = category.entries[getRandomNumberExclusive((int)category.entries.size())].wcid;
			break;
		}
	}

	if (miscItemWcid)
		return g_pWeenieFactory->CreateWeenieByClassID(miscItemWcid);
	return NULL;
}

CWeenieObject *CTreasureFactory::GenerateScroll(CTreasureTier *tier)
{
	int scrollWcid = 0;
	if (tier->scrollCategories.size() == 0)
		return NULL;
	std::string scrollCategory = tier->scrollCategories[getRandomNumberExclusive((int)tier->scrollCategories.size())];

	for each (CTreasureProfileCategory category in _TreasureProfile->scrolls)
	{
		if (category.category == scrollCategory)
		{
			if (!category.entries.empty())
				scrollWcid = category.entries[getRandomNumberExclusive((int)category.entries.size())].wcid;
			break;
		}
	} 

	if (scrollWcid)
		return g_pWeenieFactory->CreateWeenieByClassID(scrollWcid);
	return NULL;
}

CWeenieObject *CTreasureFactory::GenerateJunk()
{
	int miscItemWcid = 0;
	std::string miscCategory; 
	if(getRandomNumberExclusive(100) < _TreasureProfile->rareJunkChance * 100)
		miscCategory = "rareJunk";
	else
		miscCategory = "commonJunk";

	for each (CTreasureProfileCategory category in _TreasureProfile->miscItems)
	{
		if (category.category == miscCategory)
		{
			if (!category.entries.empty())
				miscItemWcid = category.entries[getRandomNumberExclusive((int)category.entries.size())].wcid;
			break;
		}
	}

	if (miscItemWcid)
		return g_pWeenieFactory->CreateWeenieByClassID(miscItemWcid);
	return NULL;
}

CWeenieObject *CTreasureFactory::GenerateTreasure(int tierId, eTreasureCategory treasureCategory, double qualityModifier)
{
	if (!_TreasureProfile)
		return NULL;

	if (_TreasureProfile->tiers.find(tierId) == _TreasureProfile->tiers.end())
		return NULL;

	CTreasureTier *tier = &_TreasureProfile->tiers[tierId];
	CTreasureProfileCategory *category = NULL;

	switch (treasureCategory)
	{
	default:
	case TreasureCategory_Undef:
	case TreasureCategory_Unknown:
		return NULL; //not implemented.
	case TreasureCategory_Junk:
		return GenerateJunk();
	case TreasureCategory_Scroll:
		return GenerateScroll(tier);
	case TreasureCategory_Mana_Stone:
	case TreasureCategory_Consumable:
	case TreasureCategory_Healer:
	case TreasureCategory_Lockpick:
	case TreasureCategory_Pea:
		return GenerateMundaneItem(tier);
		//return GenerateMundaneItemInferred(tierId, treasureCategory);
	case TreasureCategory_Jewelry:
		if (!_TreasureProfile->jewelry.empty())
			category = &_TreasureProfile->jewelry[getRandomNumberExclusive((int)_TreasureProfile->jewelry.size())];
		break;
	case TreasureCategory_Caster:
		category = &_TreasureProfile->casters;
		break;
	case TreasureCategory_Clothing:
		if (!_TreasureProfile->clothing.empty())
			category = &_TreasureProfile->clothing[getRandomNumberExclusive((int)_TreasureProfile->clothing.size())];
		break;
	case TreasureCategory_Melee_Weapon:
		if (!_TreasureProfile->meleeWeapons.empty())
			category = &_TreasureProfile->meleeWeapons[getRandomNumberExclusive((int)_TreasureProfile->meleeWeapons.size())];
		break;
	case TreasureCategory_Missile_Weapon:
		if (!_TreasureProfile->missileWeapons.empty())
			category = &_TreasureProfile->missileWeapons[getRandomNumberExclusive((int)_TreasureProfile->missileWeapons.size())];
		break;
	case TreasureCategory_Armor:
		if (!tier->rareArmorCategories.empty() && getRandomNumberExclusive(100) < tier->rareArmorChance * 100)
			category = tier->rareArmorCategories[getRandomNumberExclusive((int)tier->rareArmorCategories.size())];
		else if(!tier->commonArmorCategories.empty())
			category = tier->commonArmorCategories[getRandomNumberExclusive((int)tier->commonArmorCategories.size())];
		break;
	}

	if (!category || category->entries.size() == 0)
		return NULL;

	for (int i = 0; i < 10; i++) //max amount of retries for failed mutations.
	{
		sItemCreationInfo creationInfo;
		creationInfo.qualityModifier = qualityModifier;

		CItemTreasureProfileEntry *entry = &category->entries[getRandomNumberExclusive((int)category->entries.size())];

		CWeenieObject *weenie;
		if (weenie = g_pWeenieFactory->CreateWeenieByClassID(entry->wcid))
		{
			if(MutateItem(weenie, creationInfo, tier, category, entry))
				return weenie;
		}
	}

	return NULL;
}

bool CTreasureFactory::MutateItem(CWeenieObject *newItem, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	ITEM_TYPE itemType = (ITEM_TYPE)newItem->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF, true);

	if (itemType != TYPE_MELEE_WEAPON &&
		itemType != TYPE_MISSILE_WEAPON &&
		itemType != TYPE_CASTER &&
		itemType != TYPE_ARMOR &&
		itemType != TYPE_CLOTHING &&
		itemType != TYPE_JEWELRY)
	{
		return false; // we only know how to mutate the above types.
	}

	int itemWorkmanship = getRandomNumber(tier->minWorkmanship, tier->maxWorkmanship, eRandomFormula::favorMid, 2, creationInfo.qualityModifier);
	newItem->m_Qualities.SetInt(ITEM_WORKMANSHIP_INT, itemWorkmanship);

	if (itemType == TYPE_MELEE_WEAPON ||
		itemType == TYPE_MISSILE_WEAPON ||
		itemType == TYPE_CASTER)		
	{
		if (!category->wieldTiers.empty())
		{
			CWieldTier zeroReqWieldTier;
			std::vector<CWieldTier> possibleWieldTiers;
			CWieldTier *wieldTier;
			for each(CWieldTier wieldTierEntry in category->wieldTiers)
			{
				if (itemType == TYPE_MELEE_WEAPON &&
					wieldTierEntry.weaponSkillRequired >= tier->minMeleeWeaponWieldTier &&
					wieldTierEntry.weaponSkillRequired <= tier->maxMeleeWeaponWieldTier)
				{
					possibleWieldTiers.push_back(wieldTierEntry);
				}
				else if (itemType == TYPE_MISSILE_WEAPON &&
					wieldTierEntry.weaponSkillRequired >= tier->minMissileWeaponWieldTier &&
					wieldTierEntry.weaponSkillRequired <= tier->maxMissileWeaponWieldTier)
				{
					possibleWieldTiers.push_back(wieldTierEntry);
				}
				else if (itemType == TYPE_CASTER &&
					wieldTierEntry.weaponSkillRequired >= tier->minCasterWieldTier &&
					wieldTierEntry.weaponSkillRequired <= tier->maxCasterWieldTier)
				{
					if (wieldTierEntry.weaponSkillRequired == 0)
						zeroReqWieldTier = wieldTierEntry;
					possibleWieldTiers.push_back(wieldTierEntry);
				}
			}

			if (itemType == TYPE_CASTER)
			{
				for (int i = 0; i < category->wieldTiers.size() - 1; i++)
				{
					//increase chance of no weapon skill casters as that is only used for war magic.
					possibleWieldTiers.push_back(zeroReqWieldTier);
				}
			}

			if (possibleWieldTiers.size() == 0)
				return false;
			wieldTier = &possibleWieldTiers[(int)getRandomNumberExclusive((int)possibleWieldTiers.size(), eRandomFormula::favorMid, 2, 0)];

			MutateWeapon(newItem, wieldTier, creationInfo, tier, category, entry);
			if (itemType == TYPE_MELEE_WEAPON)
				MutateMeleeWeapon(newItem, wieldTier, creationInfo, tier, category, entry);
			else if (itemType == TYPE_MISSILE_WEAPON)
				MutateMissileWeapon(newItem, wieldTier, creationInfo, tier, category, entry);
			else if (itemType == TYPE_CASTER)
				MutateCaster(newItem, wieldTier, creationInfo, tier, category, entry);
		}
	}
	else if (itemType == TYPE_ARMOR)
	{
		std::vector<CWieldTier> *wieldTiers;
		if (!category->wieldTiers.empty())
			wieldTiers = &category->wieldTiers;
		else
			wieldTiers = &_TreasureProfile->armor.wieldTiers;

		if (wieldTiers->size() > 0)
		{
			std::vector<CWieldTier> possibleWieldTiers;
			CWieldTier *wieldTier;

			for each(CWieldTier wieldTierEntry in *wieldTiers)
			{
				if ((wieldTierEntry.armorWieldTier >= tier->minArmorWieldTier && wieldTierEntry.armorWieldTier <= tier->maxArmorWieldTier) ||
					(wieldTierEntry.meleeDefenseSkillRequired != 0 && wieldTierEntry.meleeDefenseSkillRequired >= tier->minMeleeWeaponWieldTier && wieldTierEntry.meleeDefenseSkillRequired <= tier->maxMeleeWeaponWieldTier) &&
					(wieldTierEntry.missileDefenseSkillRequired != 0 && wieldTierEntry.missileDefenseSkillRequired >= tier->minArmorMissileWieldTier && wieldTierEntry.missileDefenseSkillRequired <= tier->maxArmorMissileWieldTier) &&
					(wieldTierEntry.magicDefenseSkillRequired != 0 && wieldTierEntry.magicDefenseSkillRequired >= tier->minArmorMagicWieldTier && wieldTierEntry.magicDefenseSkillRequired <= tier->maxArmorMagicWieldTier))
				{
					INVENTORY_LOC coveredAreas = (INVENTORY_LOC)newItem->InqIntQuality(LOCATIONS_INT, NONE_LOC, TRUE);
					if (coveredAreas & SHIELD_LOC)
					{
						//if both shieldMinArmorBonus and shieldMaxArmorBonus are 0 it means there's no shield in this wieldTier.
						if (wieldTierEntry.minShieldArmorBonus != 0 || wieldTierEntry.maxShieldArmorBonus != 0)
							possibleWieldTiers.push_back(wieldTierEntry);
					}
					else
						possibleWieldTiers.push_back(wieldTierEntry);
				}
			}

			if (possibleWieldTiers.size() == 0)
				return false;
			wieldTier = &possibleWieldTiers[getRandomNumberExclusive((int)possibleWieldTiers.size(), eRandomFormula::favorMid, 2, 0)];
			MutateArmor(newItem, wieldTier, creationInfo, tier, category, entry);
		}
	}

	int tsysMutationDataInt = 0;
	float materialValueMultiplier = 1.0;
	float gemValueMultiplier = 1.0;
	MaterialType material = MaterialType::Undef_MaterialType;
	int gemCount = 0;
	MaterialType gemType = Undef_MaterialType;

	if (newItem->m_Qualities.InqInt(TSYS_MUTATION_DATA_INT, tsysMutationDataInt, TRUE, TRUE) && tsysMutationDataInt)
	{
		DWORD tsysMutationData = (DWORD)tsysMutationDataInt;

		BYTE spellCode = (tsysMutationData >> 24) & 0xFF;
		BYTE colorCode = (tsysMutationData >> 16) & 0xFF;
		BYTE gemCode = (tsysMutationData >> 8) & 0xFF;
		BYTE materialCode = (tsysMutationData >> 0) & 0xFF;

		std::vector<MaterialType> possibleMaterialsList;
		if (!entry->possibleMaterials.empty())
		{
			for each(auto possibleMaterial in entry->possibleMaterials)
			{
				if (possibleMaterial == MaterialType::Ceramic_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsCeramic.begin(), tier->materialsCeramic.end());
				if (possibleMaterial == MaterialType::Cloth_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsCloth.begin(), tier->materialsCloth.end());
				if (possibleMaterial == MaterialType::Gem_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsGem.begin(), tier->materialsGem.end());
				if (possibleMaterial == MaterialType::Leather_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsLeather.begin(), tier->materialsLeather.end());
				if (possibleMaterial == MaterialType::Metal_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsMetal.begin(), tier->materialsMetal.end());
				if (possibleMaterial == MaterialType::Stone_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsStone.begin(), tier->materialsStone.end());
				if (possibleMaterial == MaterialType::Wood_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsWood.begin(), tier->materialsWood.end());
			}
		}
		else if (!category->possibleMaterials.empty())
		{ //if we do not have our own materials entry try the category
			for each(auto possibleMaterial in category->possibleMaterials)
			{
				if (possibleMaterial == MaterialType::Ceramic_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsCeramic.begin(), tier->materialsCeramic.end());
				if (possibleMaterial == MaterialType::Cloth_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsCloth.begin(), tier->materialsCloth.end());
				if (possibleMaterial == MaterialType::Gem_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsGem.begin(), tier->materialsGem.end());
				if (possibleMaterial == MaterialType::Leather_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsLeather.begin(), tier->materialsLeather.end());
				if (possibleMaterial == MaterialType::Metal_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsMetal.begin(), tier->materialsMetal.end());
				if (possibleMaterial == MaterialType::Stone_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsStone.begin(), tier->materialsStone.end());
				if (possibleMaterial == MaterialType::Wood_MaterialType)
					possibleMaterialsList.insert(possibleMaterialsList.end(), tier->materialsWood.begin(), tier->materialsWood.end());
			}
		}
		material = possibleMaterialsList[getRandomNumberExclusive(possibleMaterialsList.size())];

		if (material != MaterialType::Undef_MaterialType)
		{
			newItem->m_Qualities.SetInt(MATERIAL_TYPE_INT, material);

			int ptid = g_pPortalDataEx->_treasureTableData.RollPaletteTemplateIDFromMaterialAndColorCode(material, colorCode);
			if (ptid != 0)
			{
				newItem->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);
				newItem->m_Qualities.SetFloat(SHADE_FLOAT, getRandomDouble(1.0));
				newItem->m_Qualities.SetFloat(SHADE2_FLOAT, getRandomDouble(1.0));
				newItem->m_Qualities.SetFloat(SHADE3_FLOAT, getRandomDouble(1.0));
				newItem->m_Qualities.SetFloat(SHADE4_FLOAT, getRandomDouble(1.0));
			}
		}

		//The following code generated materials from the inferred data
		//MaterialType materialCategory = g_pPortalDataEx->_treasureTableData.RollBaseMaterialFromMaterialCode(materialCode, tier->tierId);
		//if (materialCategory != MaterialType::Undef_MaterialType)
		//{
		//	material = g_pPortalDataEx->_treasureTableData.RollMaterialFromBaseMaterial(materialCategory, tier->tierId);
		//	if (material != MaterialType::Undef_MaterialType)
		//	{
		//		newItem->m_Qualities.SetInt(MATERIAL_TYPE_INT, material);
		//		//materialValueMultiplier = *g_pPortalDataEx->_treasureTableData._materialValueAddedPossibly.lookup(material);

		//		int ptid = g_pPortalDataEx->_treasureTableData.RollPaletteTemplateIDFromMaterialAndColorCode(material, colorCode);
		//		if (ptid != 0)
		//		{
		//			newItem->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);
		//			newItem->m_Qualities.SetFloat(SHADE_FLOAT, getRandomDouble(1.0));
		//			newItem->m_Qualities.SetFloat(SHADE2_FLOAT, getRandomDouble(1.0));
		//			newItem->m_Qualities.SetFloat(SHADE3_FLOAT, getRandomDouble(1.0));
		//			newItem->m_Qualities.SetFloat(SHADE4_FLOAT, getRandomDouble(1.0));
		//		}
		//	}
		//}
	}

	int maxGemCount = _TreasureProfile->workmanshipProperties[itemWorkmanship].maxGemCount;
	if (maxGemCount > 0)
	{
		if (getRandomNumberExclusive(100) < _TreasureProfile->workmanshipProperties[itemWorkmanship].gemChance * 100)
		{
			gemCount = getRandomNumber(1, maxGemCount, eRandomFormula::favorLow, 2, 0);

			int gemTypeIndex = getRandomNumberExclusive((int)tier->materialsGem.size() - 1); //-1 because ivory is the last gem type in the profile and we don't want that
			gemType = tier->materialsGem[gemTypeIndex];

			//gemType = g_pPortalDataEx->_treasureTableData.RollMaterialFromBaseMaterial(Gem_MaterialType, tier->tierId);
			//gemValueMultiplier = *g_pPortalDataEx->_treasureTableData._materialValueAddedPossibly.lookup(gemType);

			newItem->m_Qualities.SetInt(GEM_COUNT_INT, gemCount);
			newItem->m_Qualities.SetInt(GEM_TYPE_INT, gemType);
		}
	}

	//make sure we clean the item of any existing spells
	if (newItem->m_Qualities._spell_book != NULL)
		newItem->m_Qualities._spell_book->ClearSpells();
	AddSpells(newItem, creationInfo, tier, category, entry);

	int baseValue = newItem->InqIntQuality(VALUE_INT, 0, TRUE);
	float valueCalc = baseValue;
	//valueCalc += baseValue * materialValueMultiplier;
	//valueCalc += baseValue * (gemValueMultiplier * gemCount);

	valueCalc += _TreasureProfile->workmanshipProperties[itemWorkmanship].valueMultiplier;
	valueCalc += _TreasureProfile->materialProperties[material].valueMultiplier;
	valueCalc += _TreasureProfile->materialProperties[gemType].gemValue * gemCount;

	if (creationInfo.isMagical)
		valueCalc += creationInfo.totalPower * 5;

	if (itemType == TYPE_MELEE_WEAPON)
	{
		DAMAGE_TYPE damageType = (DAMAGE_TYPE)newItem->InqIntQuality(DAMAGE_TYPE_INT, DAMAGE_TYPE::UNDEF_DAMAGE_TYPE);
		if (damageType == ACID_DAMAGE_TYPE || damageType == FIRE_DAMAGE_TYPE || damageType == COLD_DAMAGE_TYPE || damageType == ELECTRIC_DAMAGE_TYPE)
			valueCalc += 0.25f * baseValue;
	}
	else if (itemType == TYPE_MISSILE_WEAPON)
	{
		int elementalDamageBonus = (DAMAGE_TYPE)newItem->InqIntQuality(ELEMENTAL_DAMAGE_BONUS_INT, 0, TRUE);
		if (elementalDamageBonus > 0)
			valueCalc += 0.25f * baseValue;
	}
	else if (itemType == TYPE_CASTER)
	{
		double elementalDamageMod = newItem->InqFloatQuality(ELEMENTAL_DAMAGE_MOD_FLOAT, 0.0, TRUE);
		if (elementalDamageMod > 0)
			valueCalc += 0.25f * baseValue;
	}

	int value = round(getRandomDouble(valueCalc * 0.9, valueCalc * 1.1));
	newItem->m_Qualities.SetInt(VALUE_INT, value);

	std::string longDesc = "";
	if (_TreasureProfile->extraMutations)
	{
		if (itemType == TYPE_CASTER || itemType == TYPE_MELEE_WEAPON || itemType == TYPE_MISSILE_WEAPON)
		{
			float critChance = 0.1f;
			float critMultiplier = 1.0f;
			if (itemType == TYPE_CASTER)
			{
				critChance = 0.05f;
				critMultiplier = 0.5f;
			}

			critChance += (critChance * newItem->InqFloatQuality(CRITICAL_FREQUENCY_FLOAT, 0.0));
			critMultiplier += newItem->InqFloatQuality(CRITICAL_MULTIPLIER_FLOAT, 0.0);

			if (!longDesc.empty())
				longDesc += "\n";
			longDesc += csprintf("Critical Hit Chance: %.1f%%", critChance * 100.0);

			if (!longDesc.empty())
				longDesc += "\n";
			longDesc += csprintf("Critical Hit Extra Damage: %d%%", (int)round(critMultiplier * 100.0));

			if (double slayerMod = newItem->InqFloatQuality(SLAYER_DAMAGE_BONUS_FLOAT, 0))
			{
				if (!longDesc.empty())
					longDesc += "\n";
				longDesc += csprintf("Slayer Extra Damage: %d%%", (int)round(slayerMod * 100.0));
			}

			if (gemCount > 0)
			{
				std::string gemName = _TreasureProfile->materialProperties[gemType].gemName;

				if (gemCount > 1)
				{
					if (!_TreasureProfile->materialProperties[gemType].gemPluralName.empty())
						gemName = _TreasureProfile->materialProperties[gemType].gemPluralName;
					else
						gemName += "s";
				}

				if (!longDesc.empty())
					longDesc += "\n";
				longDesc += csprintf("Gem: %d %s", gemCount, gemName.c_str());
			}
		}

		if (!longDesc.empty())
			longDesc += "\n";
		longDesc += csprintf("Tier: %d", tier->tierId);
		if (creationInfo.qualityModifier > 0)
			longDesc += "(Quality)";

		newItem->m_Qualities.SetString(LONG_DESC_STRING, longDesc);
	}
	else if (gemCount > 0)
	{
		longDesc = newItem->InqStringQuality(NAME_STRING, "");
		newItem->m_Qualities.SetString(LONG_DESC_STRING, longDesc);
		newItem->m_Qualities.SetInt(APPRAISAL_LONG_DESC_DECORATION_INT, (prependWorkmanship | prependMaterial | appendGemInfo));
	}

	return true;
}

void CTreasureFactory::MutateWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	if (entry->weaponSkill != UNDEF_SKILL)
	{
		//this allow overwriting a weapon's default weapon skill.
		newItem->m_Qualities.SetInt(WEAPON_SKILL_INT, entry->weaponSkill);
	}

	if (wieldTier->weaponSkillRequired > 0)
	{
		newItem->m_Qualities.SetInt(WIELD_REQUIREMENTS_INT, eWieldRequirements::baseSkill);
		newItem->m_Qualities.SetInt(WIELD_SKILLTYPE_INT, newItem->InqIntQuality(WEAPON_SKILL_INT, UNDEF_SKILL));
		newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_INT, wieldTier->weaponSkillRequired);
	}

	if (wieldTier->maxMagicDefenseBonus > 0 && getRandomNumberExclusive(100) < wieldTier->magicDefenseBonusChance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(WEAPON_MAGIC_DEFENSE_FLOAT, round(getRandomDouble(1, 1 + (wieldTier->maxMagicDefenseBonus / 100), eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (wieldTier->maxMissileDefenseBonus > 0 && getRandomNumberExclusive(100) < wieldTier->missileDefenseBonusChance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(WEAPON_MISSILE_DEFENSE_FLOAT, round(getRandomDouble(1, 1 + (wieldTier->maxMissileDefenseBonus / 100), eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (wieldTier->maxMeleeDefenseBonus > 0 && getRandomNumberExclusive(100) < wieldTier->meleeDefenseBonusChance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(WEAPON_DEFENSE_FLOAT, round(getRandomDouble(1, 1 + (wieldTier->maxMeleeDefenseBonus / 100), eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (_TreasureProfile->extraMutations)
	{
		int propertyCount = 0;

		if (propertyCount < wieldTier->maxPropertyAmount && !wieldTier->slayer_Types.empty() && wieldTier->slayer_MaxDamageBonus > 0.0 && getRandomNumberExclusive(100) < wieldTier->slayer_Chance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		{
			propertyCount++;
			CreatureType slayerType = wieldTier->slayer_Types[getRandomNumberExclusive((int)wieldTier->slayer_Types.size())];
			newItem->m_Qualities.SetInt(SLAYER_CREATURE_TYPE_INT, slayerType);

			newItem->m_Qualities.SetFloat(SLAYER_DAMAGE_BONUS_FLOAT, round(getRandomDouble(wieldTier->slayer_MinDamageBonus, wieldTier->slayer_MaxDamageBonus, eRandomFormula::favorLow, 1.5, creationInfo.qualityModifier), 2));
		}

		if (propertyCount < wieldTier->maxPropertyAmount && getRandomNumberExclusive(100) < wieldTier->crushingBlow_Chance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		{
			propertyCount++;
			newItem->m_Qualities.SetFloat(CRITICAL_MULTIPLIER_FLOAT, round(getRandomDouble(wieldTier->crushingBlow_MinCriticalMultiplier, wieldTier->crushingBlow_MaxCriticalMultiplier, eRandomFormula::favorLow, 1.5, creationInfo.qualityModifier), 2));
		}

		if (propertyCount < wieldTier->maxPropertyAmount && getRandomNumberExclusive(100) < wieldTier->bitingStrike_Chance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		{
			propertyCount++;
			newItem->m_Qualities.SetFloat(CRITICAL_FREQUENCY_FLOAT, round(getRandomDouble(wieldTier->bitingStrike_MinCriticalFrequency, wieldTier->bitingStrike_MaxCriticalFrequency, eRandomFormula::favorLow, 1.5, creationInfo.qualityModifier), 2));
		}
	}
}

void CTreasureFactory::MutateMeleeWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	newItem->m_Qualities.SetInt(DAMAGE_INT, newItem->InqIntQuality(DAMAGE_INT, 0, TRUE) + getRandomNumber(wieldTier->minDamageBonus, wieldTier->maxDamageBonus, eRandomFormula::favorMid, 2, creationInfo.qualityModifier));
	newItem->m_Qualities.SetFloat(DAMAGE_VARIANCE_FLOAT, round(getRandomDouble(1.f - wieldTier->highVariance, 1.f - wieldTier->lowVariance, eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (wieldTier->slowSpeedMod > 0 && wieldTier->fastSpeedMod > 0)
	{
		int weaponTime = newItem->InqIntQuality(WEAPON_TIME_INT, 50, TRUE);
		double weaponTimeMod = getRandomDouble(wieldTier->fastSpeedMod, wieldTier->slowSpeedMod, eRandomFormula::favorHigh, 2, creationInfo.qualityModifier);
		newItem->m_Qualities.SetInt(WEAPON_TIME_INT, round(weaponTime * weaponTimeMod));
	}

	if (wieldTier->maxAttackSkillBonus > 0 && getRandomNumberExclusive(100) < wieldTier->attackSkillBonusChance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(WEAPON_OFFENSE_FLOAT, round(getRandomDouble(1, 1 + (wieldTier->maxAttackSkillBonus / 100), eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (!entry->elementalVariants.empty())
	{
		if (getRandomNumberExclusive(100) < wieldTier->elementalChance * 100)
		{
			int variantId = entry->elementalVariants[getRandomNumberExclusive((int)entry->elementalVariants.size())];

			CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(variantId);

			if (weenieDefs == NULL)
				return;

			int elementalType;
			weenieDefs->m_Qualities.InqInt(DAMAGE_TYPE_INT, elementalType, TRUE);

			DWORD setup;
			if (weenieDefs->m_Qualities.InqDataID(SETUP_DID, setup))
				newItem->m_Qualities.SetDataID(SETUP_DID, setup);

			if (weenieDefs->m_WCID == 3750) //acid battleaxe has the wrong graphics
				newItem->m_Qualities.SetDataID(SETUP_DID, 33555690);

			switch ((DAMAGE_TYPE)elementalType)
			{
			case DAMAGE_TYPE::ACID_DAMAGE_TYPE:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ACID_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_ACID);
				newItem->m_Qualities.SetString(NAME_STRING, "Acid " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case DAMAGE_TYPE::COLD_DAMAGE_TYPE:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, COLD_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FROST);
				newItem->m_Qualities.SetString(NAME_STRING, "Frost " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case DAMAGE_TYPE::FIRE_DAMAGE_TYPE:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, FIRE_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FIRE);
				newItem->m_Qualities.SetString(NAME_STRING, "Flaming " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case DAMAGE_TYPE::ELECTRIC_DAMAGE_TYPE:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ELECTRIC_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_LIGHTNING);
				newItem->m_Qualities.SetString(NAME_STRING, "Lightning " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			}
		}
	}
}

void CTreasureFactory::MutateMissileWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	newItem->m_Qualities.SetFloat(DAMAGE_MOD_FLOAT, round(newItem->InqFloatQuality(DAMAGE_MOD_FLOAT, 1.0, TRUE) + getRandomDouble(wieldTier->minDamageModBonus, wieldTier->maxDamageModBonus, eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 2));

	if (wieldTier->slowSpeedMod > 0 && wieldTier->fastSpeedMod > 0)
	{
		int weaponTime = newItem->InqIntQuality(WEAPON_TIME_INT, 50, TRUE);
		double weaponTimeMod = getRandomDouble(wieldTier->fastSpeedMod, wieldTier->slowSpeedMod, eRandomFormula::favorHigh, 2, creationInfo.qualityModifier);
		newItem->m_Qualities.SetInt(WEAPON_TIME_INT, round(weaponTime * weaponTimeMod));
	}

	if (wieldTier->maxElementalDamageBonus > 0)
	{
		int elementalDamage = getRandomNumber(wieldTier->minElementalDamageBonus, wieldTier->maxElementalDamageBonus, eRandomFormula::favorMid, 2, 0);

		if (elementalDamage > 0)
		{
			eElements elementalType = (eElements)getRandomNumber(1, 3);
			if (getRandomNumberExclusive(100) < wieldTier->elementalChance * 100)
				elementalType = (eElements)getRandomNumber(4, 7);
			newItem->m_Qualities.SetInt(ELEMENTAL_DAMAGE_BONUS_INT, elementalDamage);

			switch (elementalType)
			{
			case acid:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ACID_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_ACID);
				newItem->m_Qualities.SetString(NAME_STRING, "Acid " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case cold:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, COLD_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FROST);
				newItem->m_Qualities.SetString(NAME_STRING, "Frost " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case fire:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, FIRE_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FIRE);
				newItem->m_Qualities.SetString(NAME_STRING, "Fire " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case lightning:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ELECTRIC_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_LIGHTNING);
				newItem->m_Qualities.SetString(NAME_STRING, "Electric " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case bludgeoning:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, BLUDGEON_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_BLUDGEONING);
				newItem->m_Qualities.SetString(NAME_STRING, "Blunt " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case piercing:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, PIERCE_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_PIERCING);
				newItem->m_Qualities.SetString(NAME_STRING, "Piercing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case slashing:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, SLASH_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_SLASHING);
				newItem->m_Qualities.SetString(NAME_STRING, "Slashing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			}
		}
	}
}

void CTreasureFactory::MutateCaster(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	if (wieldTier->weaponSkillRequired > 0)
	{
		std::vector<STypeSkill> possibleRequirements;
		//possibleRequirements.push_back(CREATURE_ENCHANTMENT_SKILL);
		//possibleRequirements.push_back(ITEM_ENCHANTMENT_SKILL);
		//possibleRequirements.push_back(LIFE_MAGIC_SKILL);
		possibleRequirements.push_back(WAR_MAGIC_SKILL);
		STypeSkill skill = possibleRequirements[getRandomNumberExclusive((int)possibleRequirements.size())];
		newItem->m_Qualities.SetInt(WIELD_SKILLTYPE_INT, skill);

		switch (skill)
		{
		case CREATURE_ENCHANTMENT_SKILL:
			creationInfo.favoredMagicSchoolMasterySpell = _TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Creature Enchantment Mastery"]];
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Item Enchantment Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Life Magic Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["War Magic Mastery"]]);
			break;
		case ITEM_ENCHANTMENT_SKILL:
			creationInfo.favoredMagicSchoolMasterySpell = _TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Item Enchantment Mastery"]];
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Creature Enchantment Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Life Magic Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["War Magic Mastery"]]);
			break;
		case LIFE_MAGIC_SKILL:
			creationInfo.favoredMagicSchoolMasterySpell = _TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Creature Enchantment Mastery"]];
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Item Enchantment Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Life Magic Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["War Magic Mastery"]]);
			break;
		case WAR_MAGIC_SKILL:
			creationInfo.favoredMagicSchoolMasterySpell = _TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["War Magic Mastery"]];
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Creature Enchantment Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Item Enchantment Mastery"]]);
			creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Life Magic Mastery"]]);
			break;
		}

		if (wieldTier->maxManaConversionBonus > 0 && getRandomNumberExclusive(100) < wieldTier->manaConversionBonusChance * 100 * (1 + (creationInfo.qualityModifier * 2)))
		{
			double manaConversionMod = round(getRandomDouble(0, wieldTier->maxManaConversionBonus / 100, eRandomFormula::favorMid, 2, creationInfo.qualityModifier), 1);
			if (manaConversionMod > 0)
				newItem->m_Qualities.SetFloat(MANA_CONVERSION_MOD_FLOAT, manaConversionMod);
		}

		if (wieldTier->maxElementalDamageMod > 0)
		{
			double elementalDamageMod = round(getRandomDouble(wieldTier->minElementalDamageMod, wieldTier->maxElementalDamageMod, eRandomFormula::favorMid, 2, 0), 2);

			elementalDamageMod = 1.0 + (elementalDamageMod / 100.0);

			eElements elementalType = (eElements)getRandomNumber(4, 7);
			if (getRandomNumberExclusive(100) < wieldTier->elementalChance * 100)
				elementalType = (eElements)getRandomNumber(1, 3);
			newItem->m_Qualities.SetFloat(ELEMENTAL_DAMAGE_MOD_FLOAT, elementalDamageMod);

			switch (elementalType)
			{
			case acid:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ACID_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_ACID);
				newItem->m_Qualities.SetString(NAME_STRING, "Searing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case cold:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, COLD_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FROST);
				newItem->m_Qualities.SetString(NAME_STRING, "Freezing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case fire:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, FIRE_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_FIRE);
				newItem->m_Qualities.SetString(NAME_STRING, "Flaming " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case lightning:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, ELECTRIC_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_LIGHTNING);
				newItem->m_Qualities.SetString(NAME_STRING, "Zapping " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case bludgeoning:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, BLUDGEON_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_BLUDGEONING);
				newItem->m_Qualities.SetString(NAME_STRING, "Smashing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case piercing:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, PIERCE_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_PIERCING);
				newItem->m_Qualities.SetString(NAME_STRING, "Prickly " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			case slashing:
				newItem->m_Qualities.SetInt(DAMAGE_TYPE_INT, SLASH_DAMAGE_TYPE);
				newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_TYPE::UI_EFFECT_SLASHING);
				newItem->m_Qualities.SetString(NAME_STRING, "Slicing " + newItem->m_Qualities.GetString(NAME_STRING, ""));
				break;
			}
		}
	}
}

void CTreasureFactory::MutateArmor(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	int encumbValue = newItem->InqIntQuality(ENCUMB_VAL_INT, 0, TRUE);
	if (encumbValue)
	{
		encumbValue = round(encumbValue * getRandomDouble(wieldTier->minBurdenMod, wieldTier->maxBurdenMod, eRandomFormula::favorMid, 1.4, -creationInfo.qualityModifier));
		newItem->m_Qualities.SetInt(ENCUMB_VAL_INT, encumbValue);
	}

	double resistanceAcid = newItem->InqFloatQuality(ARMOR_MOD_VS_ACID_FLOAT, 0.0, true);
	double resistanceElectric = newItem->InqFloatQuality(ARMOR_MOD_VS_ELECTRIC_FLOAT, 0.0, true);
	double resistanceFire = newItem->InqFloatQuality(ARMOR_MOD_VS_FIRE_FLOAT, 0.0, true);
	double resistanceCold = newItem->InqFloatQuality(ARMOR_MOD_VS_COLD_FLOAT, 0.0, true);
	double resistanceSlash = newItem->InqFloatQuality(ARMOR_MOD_VS_SLASH_FLOAT, 0.0, true);
	double resistancePierce = newItem->InqFloatQuality(ARMOR_MOD_VS_PIERCE_FLOAT, 0.0, true);
	double resistanceBludgeon = newItem->InqFloatQuality(ARMOR_MOD_VS_BLUDGEON_FLOAT, 0.0, true);

	resistanceAcid = min(resistanceAcid + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistanceElectric = min(resistanceElectric + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistanceFire = min(resistanceFire + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistanceCold = min(resistanceCold + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistanceSlash = min(resistanceSlash + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistancePierce = min(resistancePierce + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);
	resistanceBludgeon = min(resistanceBludgeon + (getRandomNumber(1, 2, eRandomFormula::favorLow, 1.4, creationInfo.qualityModifier) / 10), 2);

	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_ACID_FLOAT, resistanceAcid);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_ELECTRIC_FLOAT, resistanceElectric);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_FIRE_FLOAT, resistanceFire);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_COLD_FLOAT, resistanceCold);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_SLASH_FLOAT, resistanceSlash);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_PIERCE_FLOAT, resistancePierce);
	if (getRandomNumberExclusive(100) < 25 * (1 + (creationInfo.qualityModifier * 2)))
		newItem->m_Qualities.SetFloat(ARMOR_MOD_VS_BLUDGEON_FLOAT, resistanceBludgeon);

	INVENTORY_LOC coveredAreas = (INVENTORY_LOC)newItem->InqIntQuality(LOCATIONS_INT, NONE_LOC, TRUE);
	int armorLevel = newItem->m_Qualities.GetInt(ARMOR_LEVEL_INT, 0);
	if (coveredAreas & SHIELD_LOC)
		armorLevel += getRandomNumber(wieldTier->minShieldArmorBonus, wieldTier->maxShieldArmorBonus, eRandomFormula::favorMid, 1.4, creationInfo.qualityModifier);
	else
		armorLevel += getRandomNumber(wieldTier->minArmorBonus, wieldTier->maxArmorBonus, eRandomFormula::favorMid, 1.4, creationInfo.qualityModifier);
	newItem->m_Qualities.SetInt(ARMOR_LEVEL_INT, armorLevel);

	std::vector<STypeSkill> possibleRequirementTypes;
	if (wieldTier->meleeDefenseSkillRequired != 0)
		possibleRequirementTypes.push_back(STypeSkill::MELEE_DEFENSE_SKILL);
	if (wieldTier->missileDefenseSkillRequired != 0)
		possibleRequirementTypes.push_back(STypeSkill::MISSILE_DEFENSE_SKILL);
	if (wieldTier->magicDefenseSkillRequired != 0)
		possibleRequirementTypes.push_back(STypeSkill::MAGIC_DEFENSE_SKILL);

	bool hasRequirement1 = false;
	if (!possibleRequirementTypes.empty())
	{
		STypeSkill requirementType = possibleRequirementTypes[getRandomNumberExclusive((int)possibleRequirementTypes.size())];
		newItem->m_Qualities.SetInt(WIELD_REQUIREMENTS_INT, eWieldRequirements::baseSkill);
		newItem->m_Qualities.SetInt(WIELD_SKILLTYPE_INT, requirementType);
		hasRequirement1 = true;
		switch (requirementType)
		{
		case STypeSkill::MELEE_DEFENSE_SKILL:
			newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_INT, wieldTier->meleeDefenseSkillRequired);
			newItem->m_Qualities.SetDataID(ITEM_SKILL_LIMIT_DID, MELEE_DEFENSE_SKILL); //used by tinkering to know if the item can have it's requirement changed.
			break;
		case STypeSkill::MISSILE_DEFENSE_SKILL:
			newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_INT, wieldTier->missileDefenseSkillRequired);
			newItem->m_Qualities.SetDataID(ITEM_SKILL_LIMIT_DID, MISSILE_DEFENSE_SKILL); //used by tinkering to know if the item can have it's requirement changed.
			break;
		case STypeSkill::MAGIC_DEFENSE_SKILL:
			newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_INT, wieldTier->magicDefenseSkillRequired);
			newItem->m_Qualities.SetDataID(ITEM_SKILL_LIMIT_DID, MAGIC_DEFENSE_SKILL); //used by tinkering to know if the item can have it's requirement changed.
			break;
		}
	}

	if (_TreasureProfile->extraMutations)
	{
		if (wieldTier->minLevel > 0)
		{
			if (hasRequirement1)
			{
				newItem->m_Qualities.SetInt(WIELD_REQUIREMENTS_2_INT, eWieldRequirements::level);
				newItem->m_Qualities.SetInt(WIELD_SKILLTYPE_2_INT, 1);
				newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_2_INT, wieldTier->minLevel);
			}
			else
			{
				newItem->m_Qualities.SetInt(WIELD_REQUIREMENTS_INT, eWieldRequirements::level);
				newItem->m_Qualities.SetInt(WIELD_SKILLTYPE_INT, 1);
				newItem->m_Qualities.SetInt(WIELD_DIFFICULTY_INT, wieldTier->minLevel);
			}
		}
	}
}

std::vector<CPossibleSpells> CTreasureFactory::MergeSpellLists(std::vector<CPossibleSpells> list1, std::vector<CPossibleSpells> list2)
{
	std::vector<CPossibleSpells> mergedList = std::vector<CPossibleSpells>(list1);

	for each(CPossibleSpells spell2 in list2)
	{
		bool isRepeat = false;
		for each(CPossibleSpells spell1 in list1)
		{
			if (spell1.id == spell2.id)
			{
				isRepeat = true;
				break;
			}
		}
		if (!isRepeat)
			mergedList.push_back(spell2);
	}

	return mergedList;
}

void CTreasureFactory::AddSpells(CWeenieObject *newItem, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	ITEM_TYPE itemType = (ITEM_TYPE)newItem->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF, true);
	INVENTORY_LOC coveredAreas = (INVENTORY_LOC)newItem->InqIntQuality(LOCATIONS_INT, NONE_LOC, TRUE);
	creationInfo.spellAmountMultiplier = 1;

	switch (itemType)
	{
	case TYPE_MELEE_WEAPON:
		AddSpell(newItem, _TreasureProfile->meleeWeaponSpells, creationInfo, tier, category, entry);
		break;
	case TYPE_MISSILE_WEAPON:
		AddSpell(newItem, _TreasureProfile->missileWeaponSpells, creationInfo, tier, category, entry);
		break;
	case TYPE_CASTER:
		AddSpell(newItem, _TreasureProfile->casterSpells, creationInfo, tier, category, entry);
		break;
	case TYPE_CLOTHING:
		if(coveredAreas & HEAD_WEAR_LOC)
			AddSpell(newItem, _TreasureProfile->clothingHeadSpells, creationInfo, tier, category, entry);
		else if (coveredAreas & HAND_WEAR_LOC)
			AddSpell(newItem, _TreasureProfile->clothingHandsSpells, creationInfo, tier, category, entry);
		else if (coveredAreas & FOOT_WEAR_LOC)
			AddSpell(newItem, _TreasureProfile->clothingFeetSpells, creationInfo, tier, category, entry);
		else
			AddSpell(newItem, _TreasureProfile->clothingSpells, creationInfo, tier, category, entry);
		break;
	case TYPE_JEWELRY:
		AddSpell(newItem, _TreasureProfile->jewelrySpells, creationInfo, tier, category, entry);
		break;
	case TYPE_ARMOR:
		int spellAmountMultiplier = 0;
		std::vector<CPossibleSpells> armorSpells = std::vector<CPossibleSpells>(_TreasureProfile->armorItemSpells);

		if (coveredAreas & SHIELD_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->shieldSpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & HEAD_WEAR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorHeadSpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & CHEST_ARMOR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorUpperBodySpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & UPPER_ARM_ARMOR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorUpperBodySpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & LOWER_ARM_ARMOR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorUpperBodySpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & ABDOMEN_ARMOR_LOC)
		{
			//leg armor that also cover the abdomen do not have abdomen specific spells
			if (!(coveredAreas & UPPER_LEG_ARMOR_LOC) && !(coveredAreas & LOWER_LEG_ARMOR_LOC))
			{
				armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorUpperBodySpells);
				spellAmountMultiplier++;
			}
		}

		if (coveredAreas & HAND_WEAR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorHandsSpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & UPPER_LEG_ARMOR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorLowerBodySpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & LOWER_LEG_ARMOR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorLowerBodySpells);
			spellAmountMultiplier++;
		}

		if (coveredAreas & FOOT_WEAR_LOC)
		{
			armorSpells = MergeSpellLists(armorSpells, _TreasureProfile->armorFeetSpells);
			spellAmountMultiplier++;
		}

		creationInfo.spellAmountMultiplier = spellAmountMultiplier;
		AddSpell(newItem, armorSpells, creationInfo, tier, category, entry);
		break;
	}

	if (creationInfo.isMagical)
	{
		if (newItem->InqIntQuality(UI_EFFECTS_INT, 0, true) == 0)
			newItem->m_Qualities.SetInt(UI_EFFECTS_INT, UI_EFFECT_MAGICAL);

		newItem->m_Qualities.SetInt(ITEM_SPELLCRAFT_INT, getRandomNumber(creationInfo.highestPower, creationInfo.highestPower * 1.3, eRandomFormula::favorMid, 2, 0));
		newItem->m_Qualities.SetInt(ITEM_MAX_MANA_INT, creationInfo.totalMana);
		newItem->m_Qualities.SetInt(ITEM_CUR_MANA_INT, getRandomNumber(creationInfo.totalMana / 2, creationInfo.totalMana, eRandomFormula::favorMid, 2, 0));

		double averagePower = (double)creationInfo.totalPower / creationInfo.totalSpellsCount;

		double manaRate = min(-1.0 * creationInfo.totalSpellsCount * averagePower / 10000.0, -0.0166);
		newItem->m_Qualities.SetFloat(MANA_RATE_FLOAT, manaRate);

		double arcaneLoreModifier = 1.0;
		if (creationInfo.totalSpellsCount >= tier->minSpellsForHeritageRequirement)
		{
			if (getRandomNumberExclusive(100) < tier->heritageRequirementChance * 100)
			{
				HeritageGroup heritageRequirement = (HeritageGroup)getRandomNumber(1, 3);
				newItem->m_Qualities.SetInt(HERITAGE_GROUP_INT, heritageRequirement);
				switch (heritageRequirement)
				{
				case Aluvian_HeritageGroup:
					newItem->m_Qualities.SetString(ITEM_HERITAGE_GROUP_RESTRICTION_STRING, "Aluvian");
					break;
				case Gharundim_HeritageGroup:
					newItem->m_Qualities.SetString(ITEM_HERITAGE_GROUP_RESTRICTION_STRING, "Gharu'ndim");
					break;
				case Sho_HeritageGroup:
					newItem->m_Qualities.SetString(ITEM_HERITAGE_GROUP_RESTRICTION_STRING, "Sho");
					break;
				}
				arcaneLoreModifier -= 0.20;
			}
		}

		if (creationInfo.totalSpellsCount >= tier->minSpellsForAllegianceRankRequirement && tier->maxAllegianceRankRequired > 0)
		{
			if (getRandomNumberExclusive(100) < tier->allegianceRankRequirementChance * 100)
			{
				int favoredRank = (int)ceil(tier->maxAllegianceRankRequired * 0.4);
				favoredRank = max(favoredRank, 1);
				int allegianceRankRequirement = getRandomNumber(1, tier->maxAllegianceRankRequired, eRandomFormula::favorSpecificValue, 3, 0, favoredRank);
				newItem->m_Qualities.SetInt(ITEM_ALLEGIANCE_RANK_LIMIT_INT, allegianceRankRequirement);
				arcaneLoreModifier -= (allegianceRankRequirement * 0.08);
			}
		}

		int cappedTotalSpellsCount = creationInfo.totalSpellsCount;
		if (creationInfo.totalSpellsCount > tier->maxAmountOfSpells)
			cappedTotalSpellsCount = tier->maxAmountOfSpells; //cap amount of spells influence on arcane lore on items that cover more than one body part.

		int arcaneLoreRequirement = (int)round((pow(averagePower, 2) + ((creationInfo.totalSpellsCount - 1) * averagePower * 15)) / 200);
		arcaneLoreRequirement = (int)round(getRandomDouble(arcaneLoreRequirement * 0.9, arcaneLoreRequirement * 1.1, eRandomFormula::favorMid, 2, 0));
		arcaneLoreRequirement = (int)round(arcaneLoreRequirement * arcaneLoreModifier);
		arcaneLoreRequirement = max(arcaneLoreRequirement, 5);
		newItem->m_Qualities.SetInt(ITEM_DIFFICULTY_INT, arcaneLoreRequirement);
	}
}

void CTreasureFactory::AddSpell(CWeenieObject *newItem, std::vector<CPossibleSpells> possibleSpellsTemplate, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry)
{
	if (possibleSpellsTemplate.size() == 0)
		return;

	if (tier->maxAmountOfSpells > 0 && tier->minSpellLevel > 0 && tier->maxSpellLevel > 0)
	{
		//we have possible spells
		ITEM_TYPE itemType = (ITEM_TYPE)newItem->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF, true);
		//make it so clothing and jewelry always have at least 1 spell
		if (getRandomNumberExclusive(100) < tier->chanceOfSpells * 100 * (1 + (creationInfo.qualityModifier * 2)) || itemType == TYPE_JEWELRY || itemType == TYPE_CLOTHING)
		{
			std::vector<CPossibleSpells> possibleSpells = std::vector<CPossibleSpells>(possibleSpellsTemplate);
			int spellCount = getRandomNumber(1, tier->maxAmountOfSpells, eRandomFormula::favorLow, 3, creationInfo.qualityModifier);
			for (int i = 1; i < creationInfo.spellAmountMultiplier; i++)
				spellCount += getRandomNumber(0, tier->maxAmountOfSpells, eRandomFormula::favorLow, 3, creationInfo.qualityModifier);

			for (int i = 0; i < spellCount; i++)
			{
				if(possibleSpells.size() == 0)
					break; //no more spells;

				int spellRoll = getRandomNumberExclusive((int)possibleSpells.size());
				CPossibleSpells possibleSpell = possibleSpells[spellRoll];

				if (!creationInfo.hasAlreadyReplacedSpell && possibleSpell.spellCategory == eSpellCategory::magicSkillMastery && creationInfo.favoredMagicSchoolMasterySpell.id != 0 && !creationInfo.otherMagicSchoolMasterySpells.empty())
				{
					//replace magic school mastery spell for the one that the item has a requirement for.
					for each(CPossibleSpells spellToBeReplaced in creationInfo.otherMagicSchoolMasterySpells)
					{
						if (possibleSpell.id == spellToBeReplaced.id)
						{
							possibleSpell = creationInfo.favoredMagicSchoolMasterySpell;
							creationInfo.hasAlreadyReplacedSpell = true;
							break;
						}
					}
				}

				std::vector<int> spellLevelVariants = possibleSpell.spells;
				int minSpellVariantId = tier->minSpellLevel - 1;
				int maxSpellVariantId = min(tier->maxSpellLevel, (int)spellLevelVariants.size()) - 1;

				int spellVariantIndex = getRandomNumber(minSpellVariantId, maxSpellVariantId, eRandomFormula::favorSpecificValue, tier->preferredSpellLevelStrength, creationInfo.qualityModifier, tier->preferredSpellLevel - 1);
				int spell = spellLevelVariants[spellVariantIndex];

				if (spell == 0)
				{
					//this spell doesnt exit at this level
					continue;
				}

				possibleSpells.erase(possibleSpells.begin() + spellRoll);//remove this from the list of possible spells so we do not readd it
				if (possibleSpell.spellCategory == eSpellCategory::weaponSkillMastery)
				{
					//remove other weapon masteries from the possible spells list.
					std::vector<CPossibleSpells> updatedPossibleSpells;
					for each(CPossibleSpells otherSpell in possibleSpells)
					{
						if (otherSpell.spellCategory != eSpellCategory::weaponSkillMastery)
							updatedPossibleSpells.push_back(otherSpell);
					}
					possibleSpells = updatedPossibleSpells;
				}

				CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
				if (!spell || !pSpellTable || !pSpellTable->GetSpellBase(spell))
					continue; //invalid spell id

				newItem->m_Qualities.AddSpell(spell);
				creationInfo.spellIds.push_back(spell);
				creationInfo.spells.push_back(possibleSpell);
				creationInfo.isMagical = true;
				creationInfo.totalSpellsCount++;

				int spellPower = _TreasureProfile->spellProperties.spellPower[spellVariantIndex];
				creationInfo.totalPower += spellPower;
				creationInfo.totalMana += _TreasureProfile->spellProperties.spellMana[spellVariantIndex];
				if (creationInfo.highestPower < spellPower)
					creationInfo.highestPower = spellPower;
			}
		}
	}

	if (tier->maxAmountOfCantrips > 0 && tier->minCantripLevel > 0 && tier->maxCantripLevel > 0)
	{
		//we have possible cantrips
		if (getRandomNumberExclusive(100) < tier->chanceOfCantrips * 100 * (1 + (creationInfo.qualityModifier * 2)))
		{
			std::vector<CPossibleSpells> possibleCantrips = std::vector<CPossibleSpells>(possibleSpellsTemplate);
			if (!creationInfo.spells.empty())
			{
				//if we also have spells, coordinate cantrips with them
				bool hasWeaponMastery = false;
				std::vector<CPossibleSpells> updatedPossibleCantrips;
				std::vector<CPossibleSpells > magicMasterySpells;
				for each(CPossibleSpells spell in creationInfo.spells)
				{
					if (spell.spellCategory == eSpellCategory::weaponSkillMastery)
					{
						hasWeaponMastery = true;
					}
					else if (spell.spellCategory == eSpellCategory::magicSkillMastery)
					{
						ITEM_TYPE itemType = (ITEM_TYPE)newItem->InqIntQuality(ITEM_TYPE_INT, TYPE_UNDEF, true);
						STypeSkill wieldSkill = (STypeSkill)newItem->InqIntQuality(WIELD_SKILLTYPE_INT, UNDEF_SKILL, true);
						if (itemType != TYPE_CASTER || wieldSkill == UNDEF_SKILL)
							magicMasterySpells.push_back(spell);
					}
				}
				for each(CPossibleSpells cantrip in possibleCantrips)
				{
					if (hasWeaponMastery && cantrip.spellCategory == eSpellCategory::weaponSkillMastery)
					{
						for each(CPossibleSpells spell in creationInfo.spells)
						{
							if (spell.id == cantrip.id)
							{
								updatedPossibleCantrips.push_back(cantrip);
								break;
							}
						}
					}
					else
						updatedPossibleCantrips.push_back(cantrip);
				}
				possibleCantrips = updatedPossibleCantrips;

				if (!magicMasterySpells.empty())
				{
					creationInfo.favoredMagicSchoolMasterySpell = magicMasterySpells[getRandomNumberExclusive((int)magicMasterySpells.size())];

					creationInfo.otherMagicSchoolMasterySpells;
					if (creationInfo.favoredMagicSchoolMasterySpell.spellName != "Creature Enchantment Mastery")
						creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Creature Enchantment Mastery"]]);
					if (creationInfo.favoredMagicSchoolMasterySpell.spellName != "Item Enchantment Mastery")
						creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Item Enchantment Mastery"]]);
					if (creationInfo.favoredMagicSchoolMasterySpell.spellName != "Life Magic Mastery")
						creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["Life Magic Mastery"]]);
					if (creationInfo.favoredMagicSchoolMasterySpell.spellName != "War Magic Mastery")
						creationInfo.otherMagicSchoolMasterySpells.push_back(_TreasureProfile->spells[_TreasureProfile->spellNameToIdMap["War Magic Mastery"]]);
				}
			}

			int cantripCount = getRandomNumber(1, tier->maxAmountOfCantrips, eRandomFormula::favorLow, 10.0, 0);
			for (int i = 1; i < creationInfo.spellAmountMultiplier; i++)
				cantripCount += getRandomNumber(0, tier->maxAmountOfCantrips, eRandomFormula::favorLow, 10.0, 0);

			for (int i = 0; i < cantripCount; i++)
			{
				if (possibleCantrips.size() == 0)
					break; //no more cantrips;

				int cantripRoll = getRandomNumberExclusive((int)possibleCantrips.size());
				CPossibleSpells possibleCantrip = possibleCantrips[cantripRoll];

				if (!creationInfo.hasAlreadyReplacedCantrip && possibleCantrip.spellCategory == eSpellCategory::magicSkillMastery && creationInfo.favoredMagicSchoolMasterySpell.id != 0 && !creationInfo.otherMagicSchoolMasterySpells.empty())
				{
					//replace magic school mastery cantrip for the one that the item has a requirement for.
					for each(CPossibleSpells spellToBeReplaced in creationInfo.otherMagicSchoolMasterySpells)
					{
						if (possibleCantrip.id == spellToBeReplaced.id)
						{
							possibleCantrip = creationInfo.favoredMagicSchoolMasterySpell;
							creationInfo.hasAlreadyReplacedCantrip = true;
							break;
						}
					}
				}

				std::vector<int> cantripLevelVariants = possibleCantrip.cantrips;
				int minCantripVariantId = tier->minCantripLevel - 1;
				int maxCantripVariantId = min(tier->maxCantripLevel, (int)cantripLevelVariants.size()) - 1;

				int cantripVariantIndex = getRandomNumber(minCantripVariantId, maxCantripVariantId, eRandomFormula::favorSpecificValue, tier->preferredCantripLevelStrength, creationInfo.qualityModifier / 2, tier->preferredCantripLevel - 1);
				int cantrip = cantripLevelVariants[cantripVariantIndex];
				if (cantrip == 0)
				{
					//this cantrip doesnt exit at this level
					continue;
				}

				possibleCantrips.erase(possibleCantrips.begin() + cantripRoll);//remove this from the list of possible cantrips so we do not readd it
				if (possibleCantrip.spellCategory == eSpellCategory::weaponSkillMastery)
				{
					//remove other weapon masteries from the possible cantrips list.
					std::vector<CPossibleSpells> updatedPossibleCantrips;
					for each(CPossibleSpells otherCantrip in possibleCantrips)
					{
						if (otherCantrip.spellCategory != eSpellCategory::weaponSkillMastery)
							updatedPossibleCantrips.push_back(otherCantrip);
					}
					possibleCantrips = updatedPossibleCantrips;
				}

				CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
				if (!cantrip || !pSpellTable || !pSpellTable->GetSpellBase(cantrip))
					continue; //invalid cantrip id

				newItem->m_Qualities.AddSpell(cantrip);

				creationInfo.cantripIds.push_back(cantrip);
				creationInfo.cantrips.push_back(possibleCantrip);

				creationInfo.isMagical = true;
				creationInfo.totalSpellsCount++;
				int cantripPower = _TreasureProfile->spellProperties.cantripPower[cantripVariantIndex];
				creationInfo.totalPower += cantripPower;
				creationInfo.totalMana += _TreasureProfile->spellProperties.cantripMana[cantripVariantIndex];
				if (creationInfo.highestPower < cantripPower)
					creationInfo.highestPower = cantripPower;
			}
		}
	}
}

//bool CTreasureFactory::AddTreasureToContainerInferred(CContainerWeenie *container, DWORD treasureType)
//{
//	TreasureEntry2 *entry = g_pPortalDataEx->_treasureTableData.GetTreasureGenerationProfile(treasureType);
//	if (!entry)
//		return false;
//
//	int tier = entry->_tier;
//	double qualityModifier = entry->_lootQualityMod;
//
//	// Even though they are called item and magicItem we're not doing anything to make them respect that.
//	// We leave that to the generator to decide according to the settings specified in the treasureProfile.json.
//	int itemCount = getRandomNumber(entry->_itemMinAmount, entry->_itemMaxAmount);
//	for (int i = 0; i < itemCount; i++)
//	{
//		CWeenieObject *newItem = GenerateTreasure(tier, g_pPortalDataEx->_treasureTableData.RollTreasureCategory(0, entry->_itemTreasureTypeSelectionChances), qualityModifier);
//		if (newItem)
//		{
//			g_pWorld->CreateEntity(newItem);
//			newItem->SetWeenieContainer(container->GetID());
//			container->Container_InsertInventoryItem(0, newItem, 0);
//		}
//	}
//
//	int magicItemCount = getRandomNumber(entry->_magicItemMinAmount, entry->_magicItemMaxAmount);
//	for (int i = 0; i < magicItemCount; i++)
//	{
//		CWeenieObject *newItem = GenerateTreasure(tier, g_pPortalDataEx->_treasureTableData.RollTreasureCategory(1, entry->_magicItemTreasureTypeSelectionChances), qualityModifier);
//		if (newItem)
//		{
//			g_pWorld->CreateEntity(newItem);
//			newItem->SetWeenieContainer(container->GetID());
//			container->Container_InsertInventoryItem(0, newItem, 0);
//		}
//	}
//
//	int mundaneItemCount = getRandomNumber(entry->_mundaneItemMinAmount, entry->_mundaneItemMaxAmount);
//	for (int i = 0; i < mundaneItemCount; i++)
//	{
//		CWeenieObject *newItem = GenerateTreasure(tier, g_pPortalDataEx->_treasureTableData.RollTreasureCategory(2, entry->_mundaneItemTypeSelectionChances), qualityModifier);
//		if (newItem)
//		{
//			g_pWorld->CreateEntity(newItem);
//			newItem->SetWeenieContainer(container->GetID());
//			container->Container_InsertInventoryItem(0, newItem, 0);
//		}
//	}
//
//	return true;
//}
//
//CWeenieObject *CTreasureFactory::GenerateMundaneItemInferred(int tierId, eTreasureCategory treasureCategory)
//{
//	DWORD wcid = 0;
//	switch (treasureCategory)
//	{
//	case TreasureCategory_Mana_Stone:
//		wcid = g_pPortalDataEx->_treasureTableData.RollManaStone(tierId);
//		break;
//	case TreasureCategory_Consumable:
//		wcid = g_pPortalDataEx->_treasureTableData.RollConsumable(tierId);
//		break;
//	case TreasureCategory_Healer:
//		wcid = g_pPortalDataEx->_treasureTableData.RollHealer(tierId);
//		break;
//	case TreasureCategory_Lockpick:
//		wcid = g_pPortalDataEx->_treasureTableData.RollLockpick(tierId);
//		break;
//	case TreasureCategory_Pea:
//		wcid = g_pPortalDataEx->_treasureTableData.RollPea(tierId);
//		break;
//	}
//
//	if (wcid)
//		return g_pWeenieFactory->CreateWeenieByClassID(wcid);
//	return NULL;
//}