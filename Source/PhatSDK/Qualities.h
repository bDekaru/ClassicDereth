
#pragma once

#include "Packable.h"
#include "ExperienceTable.h"
#include "SkillTable.h"
#include "WClassID.h"

class Attribute2ndBase : public PackObj
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		_formula.UnPack(pReader);
		return true;
	}

	SkillFormula _formula;
};

class Attribute2ndTable : public PackObj, public DBObj
{
public:
	DECLARE_DBOBJ(Attribute2ndTable);
	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	Attribute2ndBase _max_health, _max_stamina, _max_mana;
};


// attribute table is 0x0E000003

class Attribute : public PackObj, public PackableJson
{
public:
	Attribute();
	virtual ~Attribute();
	
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	static const char *GetAttributeName(STypeAttribute key); // custom
	virtual uint32_t GetMaxXp();
	uint32_t GetXpNeededForMaxXp();


	uint32_t _level_from_cp = 0;
	uint32_t _init_level = 0;
	uint32_t _cp_spent = 0;
};

class SecondaryAttribute : public Attribute
{
public:
	SecondaryAttribute();
	virtual ~SecondaryAttribute();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	virtual uint32_t GetMaxXp() override;

	uint32_t _current = 0;
};

class AttributeCache : public PackObj, public PackableJson
{
public:
	AttributeCache();
	virtual ~AttributeCache();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Clear();

	BOOL SetAttribute(STypeAttribute key, const Attribute &attrib);
	BOOL SetAttribute(STypeAttribute key, uint32_t initialValue);
	virtual BOOL InqAttribute(STypeAttribute index, Attribute &value);
	virtual BOOL InqAttribute(STypeAttribute index, uint32_t &value);

	BOOL SetAttribute2nd(STypeAttribute2nd key, const SecondaryAttribute &attrib);
	BOOL SetAttribute2nd(STypeAttribute2nd key, uint32_t value);
	virtual BOOL InqAttribute2nd(STypeAttribute2nd index, SecondaryAttribute &value);
	virtual BOOL InqAttribute2nd(STypeAttribute2nd index, uint32_t &value);

	void CopyFrom(AttributeCache *pOther); // custom

	Attribute *_strength = NULL;
	Attribute *_endurance = NULL;
	Attribute *_quickness = NULL;
	Attribute *_coordination = NULL;
	Attribute *_focus = NULL;
	Attribute *_self = NULL;
	SecondaryAttribute *_health = NULL;
	SecondaryAttribute *_stamina = NULL;
	SecondaryAttribute *_mana = NULL;
};

class Skill : public PackObj, public PackableJson
{
public:
	Skill() { }
	virtual ~Skill() { }

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	static const char *SkillSacToName(SKILL_ADVANCEMENT_CLASS type); // custom
	void SetSkillAdvancementClass(SKILL_ADVANCEMENT_CLASS val);
	bool IsMaxed();
	uint32_t GetMaxXP();
	uint32_t GetXpNeededForMaxXp();

	SKILL_ADVANCEMENT_CLASS _sac = UNDEF_SKILL_ADVANCEMENT_CLASS;
	uint32_t _pp = 0;
	uint32_t _init_level = 0;
	uint32_t _level_from_pp = 0;
	int32_t _resistance_of_last_check = 0;
	double _last_used_time = 0.0;
};

class SpellBookPage : public PackObj, public PackableJson
{
public:
	SpellBookPage() { }
	virtual ~SpellBookPage() { }

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	float _casting_likelihood = 0.0f;
};

class CSpellBook : public PackObj, public PackableJson
{
public:
	CSpellBook() { }
	virtual ~CSpellBook() { }

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void AddSpell(uint32_t spellid, const SpellBookPage &spell);
	void RemoveSpell(uint32_t spellid);
	void ClearSpells();

	void TranscribeSpells(const std::list<uint32_t> &spells);
	void Prune();
	bool Exists(uint32_t spellid);

	PackableHashTableWithJson<uint32_t, SpellBookPage> _spellbook;
};

class CBaseQualities : public PackObj, public PackableJson
{
public:
	CBaseQualities();
	virtual ~CBaseQualities();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Clear();

	virtual BOOL EnchantInt(STypeInt key, int &value, BOOL allow_negative) = 0;
	void SetInt(STypeInt key, int value);
	BOOL InqInt(STypeInt key, int &value, BOOL raw = FALSE, BOOL allow_negative = FALSE);
	void RemoveInt(STypeInt key);
	void AdjInt(STypeInt key, int value, int &newvalue);
	void AdjInt(STypeInt key, int value);

	void SetInt64(STypeInt64 key, int64_t value);
	BOOL InqInt64(STypeInt64 key, int64_t &value);
	void RemoveInt64(STypeInt64 key);

	virtual BOOL EnchantFloat(STypeFloat key, double &value) = 0;
	void SetFloat(STypeFloat key, double value);
	BOOL InqFloat(STypeFloat key, double &value, BOOL raw = FALSE);
	void RemoveFloat(STypeFloat key);

	void SetBool(STypeBool key, int value);
	BOOL InqBool(STypeBool key, int &value);
	void RemoveBool(STypeBool key);

	void SetString(STypeString key, const std::string &value);
	BOOL InqString(STypeString key, std::string &value);
	void RemoveString(STypeString key);

	void SetDataID(STypeDID key, uint32_t value);
	BOOL InqDataID(STypeDID key, uint32_t &value);
	void RemoveDataID(STypeDID key);

	void SetInstanceID(STypeIID key, uint32_t value);
	BOOL InqInstanceID(STypeIID key, uint32_t &value);
	void RemoveInstanceID(STypeIID key);

	void SetPosition(STypePosition key, const Position &value);
	BOOL InqPosition(STypePosition key, Position &value);
	void RemovePosition(STypePosition key);

	// custom
	void CopyFrom(CBaseQualities *pOther);
	int GetInt(STypeInt key, int defaultValue);
	int64_t GetInt64(STypeInt64 key, int64_t defaultValue);
	BOOL GetBool(STypeBool key, BOOL defaultValue);
	double GetFloat(STypeFloat key, double defaultValue);
	std::string GetString(STypeString key, std::string defaultValue);
	uint32_t GetDID(STypeDID key, uint32_t defaultValue);
	uint32_t GetIID(STypeIID key, uint32_t defaultValue);
	Position GetPosition(STypePosition key, const Position &defaultValue);
	// end of custom

	int m_WeenieType = Undef_WeenieType;
	PackableHashTableWithJson<STypeInt, int> *m_IntStats = NULL;
	PackableHashTableWithJson<STypeInt64, int64_t> *m_Int64Stats = NULL;
	PackableHashTableWithJson<STypeBool, BOOL> *m_BoolStats = NULL;
	PackableHashTableWithJson<STypeFloat, double> *m_FloatStats = NULL;
	PackableHashTableWithJson<STypeString, std::string> *m_StringStats = NULL;
	PackableHashTableWithJson<STypeDID, uint32_t> *m_DIDStats = NULL;
	PackableHashTableWithJson<STypeIID, uint32_t> *m_IIDStats = NULL;
	PackableHashTableWithJson<STypePosition, Position> *m_PositionStats = NULL;
};

class CreationProfile : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	union {
		uint32_t wcid;
		WClassIDEnum wclass;
	};
	int try_to_bond;
	unsigned int palette;

	union {
		float shade;
		float probability;
	};
	union {
		int destination;
		int regen_algorithm;
	};
	union {
		int stack_size;
		int max_number;
		int amount;
	};
};


class ArmorCache : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	int _base_armor = 0;
	int _armor_vs_slash = 0;
	int _armor_vs_pierce = 0;
	int _armor_vs_bludgeon = 0;
	int _armor_vs_cold = 0;
	int _armor_vs_fire = 0;
	int _armor_vs_acid = 0;
	int _armor_vs_electric = 0;
	int _armor_vs_nether = 0;
};

class BodyPartSelectionData : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	float HLF;
	float MLF;
	float LLF;
	float HRF;
	float MRF;
	float LRF;
	float HLB;
	float MLB;
	float LLB;
	float HRB;
	float MRB;
	float LRB;
};

class BodyPart : public PackObj, public PackableJson
{
public:
	BodyPart();
	virtual ~BodyPart();

	BodyPart &operator=(BodyPart const &);

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	DAMAGE_TYPE _dtype = DAMAGE_TYPE::UNDEF_DAMAGE_TYPE; // damage type
	int _dval = 0; // damage value
	float _dvar = 0.0; // damage variance
	ArmorCache _acache;
	BODY_HEIGHT _bh = BODY_HEIGHT::UNDEF_BODY_HEIGHT; // body height
	BodyPartSelectionData *_bpsd = NULL;
};

class Body : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableHashTableWithJson<int32_t, BodyPart> _body_part_table;
};

class StatMod : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	unsigned int type;
	unsigned int key;
	float val;
};

struct EnchantedQualityDetails
{
	double rawValue = 0.0;
	double valueIncreasingMultiplier = 1.0;
	double valueDecreasingMultiplier = 1.0;
	double valueIncreasingAdditive = 0.0;
	double valueDecreasingAdditive = 0.0;

	double enchantedValue = 0.0;
	double enchantedValue_IncreasingOnly = 0.0;
	double enchantedValue_DecreasingOnly = 0.0;

	void CalculateEnchantedValue()
	{
		enchantedValue = rawValue;
		enchantedValue *= valueIncreasingMultiplier;
		enchantedValue *= valueDecreasingMultiplier;
		enchantedValue += valueIncreasingAdditive;
		enchantedValue += valueDecreasingAdditive;
		enchantedValue = std::max(enchantedValue, 0.0);

		CalculateIncreasingEnchantedValue();
		CalculateDecreasingEnchantedValue();
	}

	void CalculateDecreasingEnchantedValue()
	{
		enchantedValue_DecreasingOnly = rawValue;
		enchantedValue_DecreasingOnly *= valueDecreasingMultiplier;
		enchantedValue_DecreasingOnly += valueDecreasingAdditive;
		enchantedValue_DecreasingOnly = std::max(enchantedValue_DecreasingOnly, 0.0);
	}

	void CalculateIncreasingEnchantedValue()
	{
		enchantedValue_IncreasingOnly = rawValue;
		enchantedValue_IncreasingOnly *= valueIncreasingMultiplier;
		enchantedValue_IncreasingOnly += valueIncreasingAdditive;
		enchantedValue_IncreasingOnly = std::max(enchantedValue_IncreasingOnly, 0.0);
	}
};

class Enchantment : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	BOOL AffectsAttackSkills(unsigned int key);
	BOOL AffectsDefenseSkills(unsigned int key);
	BOOL Duel(Enchantment *challenger);
	BOOL Enchant(float *value);
	BOOL Enchant(EnchantedQualityDetails *value);

	BOOL HasExpired(); //custom

	unsigned int _id = 0;
	unsigned int m_SpellSetID = 0;
	unsigned int _spell_category = 0;
	int _power_level = 0;
	double _start_time = -1.0;
	double _duration = -1.0;
	unsigned int _caster = 0;
	float _degrade_modifier = 0.0f;
	float _degrade_limit = 0.0f;
	double _last_time_degraded = -1.0;
	uint32_t _dtype = 0;
	StatMod _smod;
};

class CEnchantmentRegistry : public PackObj, public PackableJson
{
public:
	CEnchantmentRegistry();
	virtual ~CEnchantmentRegistry();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Clear();

	int UpdateSpellTotals(unsigned int spell, int iDelta);
	void CountSpellsInList(PackableListWithJson<Enchantment> *list);
	static BOOL IsEnchantmentInList(const unsigned int spell, PackableListWithJson<Enchantment> *list);
	BOOL IsEnchanted(const unsigned int spell);
	BOOL UpdateEnchantment(Enchantment *to_update);

	static BOOL Enchant(PackableListWithJson<Enchantment> *affecting, EnchantedQualityDetails *val);
	BOOL GetBodyArmorEnchantmentDetails(unsigned int part, DAMAGE_TYPE dt, EnchantedQualityDetails *val);
	BOOL GetIntEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val);
	BOOL GetFloatEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val);

	static BOOL AttemptToReplaceSpellInList(Enchantment *spell, PackableListWithJson<Enchantment> **list);
	static BOOL ReplaceEnchantmentInList(Enchantment *new_guy, PackableListWithJson<Enchantment> *list);
	BOOL AddEnchantmentToList(Enchantment *to_update, PackableListWithJson<Enchantment> **list);
	BOOL UpdateVitae(Enchantment *vitae);
	BOOL RemoveEnchantment(const unsigned int eid);
	BOOL RemoveEnchantmentFromList(const unsigned int eid, PackableListWithJson<Enchantment> *list);
	static BOOL CullEnchantmentsFromList(PackableListWithJson<Enchantment> *list, const unsigned int type, const unsigned int key, PackableListWithJson<Enchantment> *affecting);
	static BOOL Duel(Enchantment *challenger, PackableListWithJson<Enchantment> *list);
	BOOL PurgeEnchantments();
	BOOL PurgeEnchantmentList(PackableListWithJson<Enchantment> *list);
	BOOL RemoveEnchantments(PackableListWithJson<uint32_t> *to_remove);
	BOOL PurgeBadEnchantments();
	BOOL PurgeBadEnchantmentList(PackableListWithJson<Enchantment> *list);
	BOOL InqVitae(Enchantment *vitae);
	double GetVitaeValue();

	BOOL Enchant(PackableListWithJson<Enchantment> *affecting, float *new_value);
	BOOL EnchantAttribute(unsigned int stype, unsigned int *val);
	BOOL EnchantAttribute2nd(unsigned int stype, unsigned int *val);
	BOOL EnchantSkill(unsigned int stype, int *val);
	BOOL EnchantInt(unsigned int stype, int *val, BOOL allow_negative);
	BOOL EnchantFloat(unsigned int stype, double *val);
	BOOL EnchantBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int *val);

	void GetExpiredEnchantments(PackableListWithJson<Enchantment> *list, PackableListWithJson<uint32_t> *expired); // custom
	void GetExpiredEnchantments(PackableListWithJson<uint32_t> *expired); // custom

	Enchantment* AddCooldown(uint32_t cooldownId, double duration, uint32_t caster);
	BOOL InqCooldown(uint32_t cooldownId);

	Enchantment* GetHighestEnchantOfCategory(unsigned int category, unsigned int smod_type);

	PackableListWithJson<Enchantment> *_mult_list = NULL;
	PackableListWithJson<Enchantment> *_add_list = NULL;
	PackableListWithJson<Enchantment> *_cooldown_list = NULL;
	std::list<Enchantment> *_rating_list = NULL;

	Enchantment *_vitae = NULL;
	unsigned int m_cHelpfulEnchantments = 0;
	unsigned int m_cHarmfulEnchantments = 0;
};

class EventFilter : public PackObj, public PackableJson
{
public:
	EventFilter();
	virtual ~EventFilter();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	unsigned int num_events = 0;
	unsigned int *event_filter = NULL;
};

class Emote : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	static const char *EmoteCategoryToName(EmoteCategory category); // custom
	static const char *EmoteTypeToName(EmoteType type); // custom

	EmoteType type = EmoteType::Invalid_EmoteType;
	float delay = 0.0f;
	float extent = 1.0f;
	unsigned int amount = 0;
	uint64_t amount64 = 0;
	uint64_t heroxp64 = 0;
	uint64_t min64 = 0;
	uint64_t max64 = 0;
	unsigned int min = 0;
	unsigned int max = 0;
	double fmin = 0.0;
	double fmax = 0.0;
	unsigned int stat = 0;
	unsigned int motion = 0;
	PScriptType pscript = PScriptType::PS_Invalid;
	SoundType sound = SoundType::Sound_Invalid;
	CreationProfile cprof;
	Frame frame;
	unsigned int spellid = 0;
	std::string teststring;
	std::string msg;
	double percent = 0.0;
	int display = 0;
	unsigned int wealth_rating = 0;
	unsigned int treasure_class = 0;
	int treasure_type = 0;
	Position mPosition;
};

class EmoteSet : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	EmoteCategory category = EmoteCategory::Invalid_EmoteCategory;
	float probability = 0.0f;
	uint32_t classID = 0;
	std::string quest;
	unsigned int style = 0;
	unsigned int substyle = 0;
	unsigned int vendorType = 0;
	float minhealth = 0.0f;
	float maxhealth = 0.0f;
	PackableListWithJson<Emote> emotes;
};

class CEmoteTable : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableHashTableWithJson<uint32_t, PackableListWithJson<EmoteSet>> _emote_table;
};

class PageData : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void PackNoText(BinaryWriter *pWriter);

	unsigned int authorID = 0;
	std::string authorName;
	std::string authorAccount;
	int textIncluded = 1;
	int ignoreAuthor = 0;
	std::string pageText;
	// PageData *prev = NULL;
	// PageData *next = NULL;
};

class PageDataList : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Flush();

	// PageData *first = NULL;
	// PageData *last = NULL;

	std::list<PageData> pages;

	int numPages = 0;
	int maxNumPages = 0;
	int maxNumCharsPerPage = 0;
	int packWithText = 1;
};

class GeneratorProfile : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	bool IsTreasureType() const
	{
		return (whereCreate & Treasure_RegenLocationType) ? true : false;
	}

	bool IsPlaceHolder()
	{
		return type == W_PLACEHOLDER_CLASS;
	}

	float probability = 0.0f;
	uint32_t type = 0; // ID
	double delay = 600.0;
	int initCreate = 1;
	int maxNum = -1;
	RegenerationType whenCreate = Destruction_RegenerationType;
	RegenLocationType whereCreate  = Scatter_RegenLocationType;
	int stackSize = 0;
	unsigned int ptid  = 0;
	float shade = 0.0f;
	Position pos_val;
	unsigned int slot = 0;
};

class GeneratorTable : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	std::list<GeneratorProfile> GetInitialGenerationList(); // inferred
	std::list<GeneratorProfile> GetGenerationList(); // inferred
	std::list<GeneratorProfile> GetDeathGenerationList();

	PackableListWithJson<GeneratorProfile> _profile_list;
};

class GeneratorRegistryNode : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	uint32_t m_wcidOrTtype = 0; // ID
	double ts = 0.0;
	int m_bTreasureType = 0;
	unsigned int slot = 0;
	int checkpointed = 0;
	int shop = 0;
	int amount = 0;

	uint32_t m_objectId = 0;
};

class GeneratorRegistry : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableHashTableWithJson<uint32_t, GeneratorRegistryNode> _registry;
};

class GeneratorQueueNode : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	unsigned int slot = 0;
	double when = 0.0;
};

class GeneratorQueue : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableListWithJson<GeneratorQueueNode> _queue;
};

class CACQualities : public CBaseQualities, public DBObj
{
public:
	CACQualities();
	virtual ~CACQualities();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Clear();

	virtual BOOL EnchantInt(STypeInt key, int &value, BOOL allow_negative) override;
	virtual BOOL EnchantFloat(STypeFloat key, double &value) override;
	BOOL EnchantAttribute(STypeAttribute key, uint32_t &value);
	BOOL EnchantAttribute2nd(STypeAttribute2nd key, uint32_t &value);
	BOOL EnchantSkill(STypeSkill key, uint32_t &value);
	BOOL EnchantBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int *val);
	BOOL PurgeEnchantments();
	BOOL PurgeBadEnchantments();
	BOOL InqVitae(Enchantment *vitae);
	double GetVitaeValue();

	BOOL GetBodyArmorEnchantmentDetails(unsigned int part, DAMAGE_TYPE dt, EnchantedQualityDetails *val); // custom, implied
	BOOL GetIntEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val); // custom, implied
	BOOL GetFloatEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val);

	float GetAddRating(STypeInt rating, bool raw);
	float GetPositiveRating(STypeInt rating, bool raw);
	float GetNegativeRating(STypeInt rating, bool raw);

	BOOL BoundsCheck(STypeAttribute2nd key, uint32_t &val, uint32_t &max);

	BOOL SetAttribute(STypeAttribute key, uint32_t initialValue);
	BOOL SetAttribute(STypeAttribute key, const Attribute &value);
	virtual BOOL InqAttribute(STypeAttribute key, Attribute &value);
	virtual BOOL InqAttribute(STypeAttribute key, uint32_t &value, BOOL raw);

	BOOL InqAttribute2ndBaseLevel(STypeAttribute2nd key, uint32_t &value, BOOL raw);
	BOOL SetAttribute2nd(STypeAttribute2nd key, uint32_t value);
	BOOL SetAttribute2nd(STypeAttribute2nd key, const SecondaryAttribute &value);
	BOOL SetAttribute2nd(STypeAttribute2nd key, uint32_t value, uint32_t &result, uint32_t &max);
	BOOL InqAttribute2nd(STypeAttribute2nd key, SecondaryAttribute &value);
	BOOL InqAttribute2nd(STypeAttribute2nd key, uint32_t &value, BOOL raw);

	void SetSkill(STypeSkill key, const Skill &value);
	void SetSkillLevel(STypeSkill key, uint32_t value);
	void SetSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS value);
	BOOL InqSkill(STypeSkill key, uint32_t &value, BOOL raw);
	BOOL InqSkill(STypeSkill key, Skill &value);
	BOOL InqSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS &value);
	BOOL InqSkillLevel(STypeSkill key, uint32_t &value);
	BOOL InqSkillBaseLevel(STypeSkill key, uint32_t &value, BOOL raw);

	BOOL InqBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int &value, BOOL raw);

	void AddSpell(uint32_t spellid);
	void RemoveSpell(uint32_t spellid);

	BOOL InqLoad(float &load);
	BOOL InqJumpVelocity(float extent, float &v_z);
	BOOL InqRunRate(float &rate);
	BOOL CanJump(float extent);
	BOOL JumpStaminaCost(float extent, int32_t & cost);

	BOOL HasSpellBook();
	BOOL IsSpellKnown(const unsigned int spell);
	
	BOOL UpdateEnchantment(Enchantment *to_update);

	void CopyFrom(CACQualities *pOther); // custom

	Enchantment* AddCooldown(uint32_t cooldownId, double duration);
	BOOL InqCooldown(uint32_t cooldownId);
	
	class AttributeCache *_attribCache = NULL;
	PackableHashTableWithJson<STypeSkill, Skill> *_skillStatsTable = NULL;
	class Body * _body = NULL;
	class CSpellBook * _spell_book = NULL;
	class CEnchantmentRegistry * _enchantment_reg = NULL;
	class EventFilter * _event_filter = NULL;
	class CEmoteTable * _emote_table = NULL;
	class PackableListWithJson<CreationProfile> * _create_list = NULL;
	class PageDataList * _pageDataList = NULL;
	class GeneratorTable * _generator_table = NULL;
	class GeneratorRegistry * _generator_registry = NULL;
	class GeneratorQueue * _generator_queue = NULL;
};

class PlayerDesc
{
	// ...
};
