
#pragma once

enum eElements
{
	slashing = 1,
	piercing = 2,
	bludgeoning = 3,
	cold = 4,
	fire = 5,
	acid = 6,
	lightning = 7
};

enum eWieldRequirements
{
	undef = 0,
	skill = 1,
	baseSkill = 2,
	attribute = 3,
	baseAttribute = 4,
	vital = 5, //unconfirmed
	baseVital = 6,
	level = 7,
	skillAdvancementClass = 8,
	factionMember = 9
};

enum eSpellCategory
{
	noCategory,
	magicSkillMastery,
	weaponSkillMastery
};

enum eTreasureTypeEntryType
{
	eTreasureTypeEntryType_General,
	eTreasureTypeEntryType_Warrior,
	eTreasureTypeEntryType_Magic,
};

enum eTreasureCategory
{
	TreasureCategory_Undef = 0,
	TreasureCategory_Junk = 1, //unconfirmed
	TreasureCategory_Jewelry = 2, //unconfirmed
	TreasureCategory_Caster = 3, //unconfirmed
	TreasureCategory_Clothing = 4, //unconfirmed
	TreasureCategory_Melee_Weapon = 5, //unconfirmed
	TreasureCategory_Armor = 6, //unconfirmed
	TreasureCategory_Scroll = 7, //unconfirmed
	TreasureCategory_Missile_Weapon = 8, //unconfirmed
	TreasureCategory_Unknown = 9,
	TreasureCategory_Mana_Stone = 10, //unconfirmed
	TreasureCategory_Consumable = 11, //unconfirmed
	TreasureCategory_Healer = 12, //unconfirmed
	TreasureCategory_Lockpick = 13, //unconfirmed
	TreasureCategory_Pea = 14 //unconfirmed
};

enum eAppraisalLongDescDecorations
{
	prependWorkmanship = 0x1,
	prependMaterial = 0x2,
	appendGemInfo = 0x4,
};

class CSpellProperties
{
public:
	std::vector<int> spellPower;
	std::vector<int> spellMana;
	std::vector<int> cantripPower;
	std::vector<int> cantripMana;
};

class CMaterialProperties
{
public:
	float valueMultiplier;
	std::vector<int> palettes;
	int gemValue;
	std::string gemName;
	std::string gemPluralName;
};

class CWorkmanshipProperties
{
public:
	float valueMultiplier;
	float gemChance;
	int maxGemCount;
};

class CItemTreasureProfileEntry
{
public:
	std::string name;
	int wcid;
	int maxAmount;
	std::vector<MaterialType> possibleMaterials;
	std::vector<int> elementalVariants;
	STypeSkill weaponSkill;
};

class CPossibleSpells
{
public:
	std::string spellName;
	eSpellCategory spellCategory;
	std::vector<int> spells;
	std::vector<int> cantrips;

	int id;
};

class CWieldTier
{
public:
	int weaponSkillRequired;
	float maxAttackSkillBonus;
	float maxMeleeDefenseBonus;
	float maxMissileDefenseBonus;
	float maxMagicDefenseBonus;
	float attackSkillBonusChance;
	float meleeDefenseBonusChance;
	float missileDefenseBonusChance;
	float magicDefenseBonusChance;
	float elementalChance;

	float slayer_Chance;
	float slayer_MinDamageBonus;
	float slayer_MaxDamageBonus;
	std::vector<CreatureType> slayer_Types;

	int maxPropertyAmount;

	float crushingBlow_Chance;
	float crushingBlow_MinCriticalMultiplier;
	float crushingBlow_MaxCriticalMultiplier;

	float bitingStrike_Chance;
	float bitingStrike_MinCriticalFrequency;
	float bitingStrike_MaxCriticalFrequency;

	//melee weapons
	int minDamageBonus;
	int maxDamageBonus;
	float highVariance;
	float lowVariance;

	//melee and missile weapons
	float slowSpeedMod;
	float fastSpeedMod;

	//missile weapons
	float minDamageModBonus;
	float maxDamageModBonus;
	int minElementalDamageBonus;
	int maxElementalDamageBonus;

	//casters
	float maxManaConversionBonus;
	float minElementalDamageMod;
	float maxElementalDamageMod;
	float manaConversionBonusChance;

	//armor
	int armorWieldTier;
	int meleeDefenseSkillRequired;
	int missileDefenseSkillRequired;
	int magicDefenseSkillRequired;
	int minArmorBonus;
	int maxArmorBonus;
	float minBurdenMod;
	float maxBurdenMod;
	int minShieldArmorBonus;
	int maxShieldArmorBonus;
	int minLevel;
};

class CTreasureProfileCategory
{
public:
	std::string category;
	std::vector<MaterialType> possibleMaterials;
	std::vector<CWieldTier> wieldTiers;
	std::vector<CItemTreasureProfileEntry> entries;
};

class CTreasureArmorProfile
{
public:
	std::vector<CWieldTier> wieldTiers;
	std::vector<CTreasureProfileCategory> categories;
};

class CTreasureTier
{
public:
	int tierId;
	std::vector<std::string> miscItemsCategories;
	std::vector<std::string> scrollCategories;

	float lootChance;
	float chestLootChance;
	float miscLootChance;
	float scrollLootChance;

	float lootAmountMultiplier;
	float chestLootAmountMultiplier;
	int qualityLootLevelThreshold;
	float qualityLootModifier;

	int minMeleeWeaponWieldTier;
	int maxMeleeWeaponWieldTier;
	int minMissileWeaponWieldTier;
	int maxMissileWeaponWieldTier;
	int minCasterWieldTier;
	int maxCasterWieldTier;

	int minArmorWieldTier;
	int maxArmorWieldTier;
	int minArmorMeleeWieldTier;
	int maxArmorMeleeWieldTier;
	int minArmorMissileWieldTier;
	int maxArmorMissileWieldTier;
	int minArmorMagicWieldTier;
	int maxArmorMagicWieldTier;

	int meleeWeaponsLootProportion;
	int missileWeaponsLootProportion;
	int castersLootProportion;
	int armorLootProportion;
	int clothingLootProportion;
	int jewelryLootProportion;

	std::vector<std::string> commonArmorCategoryNames;
	std::vector<std::string> rareArmorCategoryNames;
	float rareArmorChance;

	int minWorkmanship;
	int maxWorkmanship;

	int maxAmountOfSpells;
	float chanceOfSpells;
	int minSpellLevel;
	int maxSpellLevel;
	int preferredSpellLevel;
	float preferredSpellLevelStrength;

	int maxAmountOfCantrips;
	float chanceOfCantrips;
	int minCantripLevel;
	int maxCantripLevel;
	int preferredCantripLevel;
	float preferredCantripLevelStrength;

	float heritageRequirementChance;
	int minSpellsForHeritageRequirement;
	float allegianceRankRequirementChance;
	int minSpellsForAllegianceRankRequirement;
	int maxAllegianceRankRequired;

	std::vector<MaterialType> materialsCeramic;
	std::vector<MaterialType> materialsCloth;
	std::vector<MaterialType> materialsGem;
	std::vector<MaterialType> materialsLeather;
	std::vector<MaterialType> materialsMetal;
	std::vector<MaterialType> materialsStone;
	std::vector<MaterialType> materialsWood;

	//dynamic fields
	std::map<float, eTreasureCategory> categoryChances;
	std::vector<CTreasureProfileCategory *> commonArmorCategories;
	std::vector<CTreasureProfileCategory *> rareArmorCategories;

	eTreasureCategory GetRandomTreasureCategory();
};

class CTreasureProfile : public PackableJson
{
public:
	DECLARE_PACKABLE_JSON()

	bool extraMutations = false;
	float rareJunkChance = 0.0;

	std::map<int, CTreasureTier> tiers;

	std::vector<CTreasureProfileCategory> meleeWeapons;
	std::vector<CTreasureProfileCategory> missileWeapons;
	CTreasureProfileCategory casters;
	CTreasureArmorProfile armor;
	std::vector<CTreasureProfileCategory> clothing;
	std::vector<CTreasureProfileCategory> jewelry;
	std::vector<CTreasureProfileCategory> miscItems;
	std::vector<CTreasureProfileCategory> scrolls;

	CSpellProperties spellProperties;

	std::vector<CPossibleSpells> meleeWeaponSpells;
	std::vector<CPossibleSpells> missileWeaponSpells;
	std::vector<CPossibleSpells> casterSpells;
	std::vector<CPossibleSpells> shieldSpells;
	std::vector<CPossibleSpells> jewelrySpells;

	std::vector<CPossibleSpells> clothingSpells;
	std::vector<CPossibleSpells> clothingHeadSpells;
	std::vector<CPossibleSpells> clothingHandsSpells;
	std::vector<CPossibleSpells> clothingFeetSpells;

	std::vector<CPossibleSpells> armorItemSpells;
	std::vector<CPossibleSpells> armorHeadSpells;
	std::vector<CPossibleSpells> armorUpperBodySpells;
	std::vector<CPossibleSpells> armorHandsSpells;
	std::vector<CPossibleSpells> armorLowerBodySpells;
	std::vector<CPossibleSpells> armorFeetSpells;

	std::map<MaterialType, CMaterialProperties> materialProperties;
	std::map<int, CWorkmanshipProperties> workmanshipProperties;

	std::map<int, int> chestTreasureTypeReplacementTable;

	bool isInitialized;
	int nextSpellId;
	std::map<std::string, int> spellNameToIdMap;
	std::map<int, CPossibleSpells> spells;

	void ComputeUniqueSpellIds(std::vector<CPossibleSpells> spellList);
	void Initialize();
};

struct sItemCreationInfo
{
	double qualityModifier = 0.0;
	bool isMagical = false;
	int totalPower = 0;
	int totalMana = 0;
	int highestPower = 0;
	int totalSpellsCount = 0;
	int spellAmountMultiplier = 0;
	CPossibleSpells favoredMagicSchoolMasterySpell;
	std::vector<CPossibleSpells> otherMagicSchoolMasterySpells;
	bool hasAlreadyReplacedSpell = false;
	bool hasAlreadyReplacedCantrip = false;
	std::vector<CPossibleSpells> spells;
	std::vector<int> spellIds;
	std::vector<CPossibleSpells> cantrips;
	std::vector<int> cantripIds;
};

struct SItemListCreationData
{
	CWeenieObject *parent;
	int destinationType;
	bool isRegenLocationType;
	DWORD treasureTypeOrWcid;
	unsigned int ptid;
	float shade;
	const GeneratorProfile *profile;
	int amountCreated = 0;
	double accumulatedChance = 0.0;
	double diceRoll = 0.0;
	bool isSubSet = false;
};

class CTreasureFactory
{
public:
	std::map<std::string, MaterialType> _materialTypeTranslationTable;
	std::map<std::string, STypeSkill> _skillTypeTranslationTable;
	std::map<std::string, CreatureType> _creatureTypeTranslationTable;

	CTreasureProfile *_TreasureProfile = NULL;

	CTreasureFactory();
	virtual ~CTreasureFactory();

	MaterialType TranslateMaterialStringToEnumValue(std::string stringValue);
	STypeSkill TranslateSkillStringToEnumValue(std::string stringValue);
	CreatureType TranslateCreatureStringToEnumValue(std::string stringValue);

	void Initialize();

	int GenerateFromTypeOrWcid(CWeenieObject *parent, int destinationType, bool isRegenLocationType, DWORD treasureTypeOrWcid, unsigned int ptid = 0, float shade = 0.0f, const GeneratorProfile *profile = NULL);

	//bool AddTreasureToContainerInferred(CContainerWeenie *container, DWORD treasureType);
	//CWeenieObject *GenerateMundaneItemInferred(int tierId, eTreasureCategory treasureCategory);

	CWeenieObject *GenerateTreasure(int tier, eTreasureCategory treasureCategory, double qualityModifier = 0.0);
	CWeenieObject *GenerateJunk();
	CWeenieObject *GenerateMundaneItem(CTreasureTier *tier);
	CWeenieObject *GenerateScroll(CTreasureTier *tier);

	bool MutateItem(CWeenieObject *newItem, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);

	void MutateWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	void MutateMeleeWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	void MutateMissileWeapon(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	void MutateCaster(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	void MutateArmor(CWeenieObject *newItem, CWieldTier *wieldTier, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	std::vector<CPossibleSpells> MergeSpellLists(std::vector<CPossibleSpells> list1, std::vector<CPossibleSpells> list2);
	void AddSpells(CWeenieObject *newItem, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
	void AddSpell(CWeenieObject * newItem, std::vector<CPossibleSpells> possibleSpellsTemplate, sItemCreationInfo &creationInfo, CTreasureTier *tier, CTreasureProfileCategory *category, CItemTreasureProfileEntry *entry);
};