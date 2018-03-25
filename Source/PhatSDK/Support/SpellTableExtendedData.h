
#pragma once


#pragma once

#include "Packable.h"
#include "GameEnums.h"

#define SPELLFORMULA_MAX_COMPS 8

class SpellFormulaEx : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	ITEM_TYPE GetTargetingType();
	DWORD GetPowerLevelOfPowerComponent();

	unsigned int _comps[SPELLFORMULA_MAX_COMPS];
};

class SpellEx : public PackObj, public PackableJson
{
public:
	virtual ~SpellEx() { }

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	static SpellEx *BuildSpell(SpellType sp_type);

	virtual bool IsProjectileSpell() { return false; }
	virtual double InqDuration();

	virtual class ProjectileSpellEx *AsProjectileSpell() { return NULL; }
	virtual class ProjectileLifeSpellEx *AsLifeProjectileSpell() { return NULL; }

	unsigned int _spell_id;
};

#define INVALID_ENCHANTMENT_DEGRADE_LIMIT -666.0f

class EnchantmentSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

		_duration = pReader->Read<double>();
		_degrade_modifier = pReader->Read<float>();
		_degrade_limit = pReader->Read<float>();

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_spellCategory = pReader->Read<DWORD>();
		_smod.UnPack(pReader);
#endif

		return true;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_duration = reader["duration"];
		_degrade_modifier = reader["degrade_modifier"];
		_degrade_limit = reader["degrade_limit"];
		_spellCategory = reader["spellCategory"];
		_smod.UnPackJson(reader["smod"]);
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["duration"] = _duration;
		writer["degrade_modifier"] = _degrade_modifier;
		writer["degrade_limit"] = _degrade_limit;
		writer["spellCategory"] = _spellCategory;
		_smod.PackJson(writer["smod"]);
	}

	long double _duration = -1.0;
	float _degrade_modifier = 0.0f;
	float _degrade_limit = INVALID_ENCHANTMENT_DEGRADE_LIMIT;

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
	int _spellCategory = 0;
	StatMod _smod;
#endif
};

class FellowshipEnchantmentSpellEx : public EnchantmentSpellEx
{
public:
};

class ProjectileSpellEx : public SpellEx
{
public:
	virtual bool IsProjectileSpell() { return true; }

	virtual class ProjectileSpellEx *AsProjectileSpell() { return this; }

	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_etype = pReader->Read<DWORD>();
		_baseIntensity = pReader->Read<int>();
		_variance = pReader->Read<int>();
		_wcid = pReader->Read<DWORD>();
		_numProjectiles = pReader->Read<int>();
		_numProjectilesVariance = pReader->Read<int>();
		_spreadAngle = pReader->Read<float>();
		_verticalAngle = pReader->Read<float>();
		_defaultLaunchAngle = pReader->Read<float>();
		_bNonTracking = pReader->Read<int>();

		_createOffset.UnPack(pReader);
		_padding.UnPack(pReader);
		_dims.UnPack(pReader);
		_peturbation.UnPack(pReader);

		_imbuedEffect = pReader->Read<DWORD>();
		_slayerCreatureType = pReader->Read<int>();
		_slayerDamageBonus = pReader->Read<float>();
		_critFreq = pReader->Read<double>();
		_critMultiplier = pReader->Read<double>();
		_ignoreMagicResist = pReader->Read<int>();
		_elementalModifier = pReader->Read<double>();
#endif		
		return true;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_etype = reader["etype"];
		_baseIntensity = reader["baseIntensity"];
		_variance = reader["variance"];
		_wcid = reader["wcid"];
		_numProjectiles = reader["numProjectiles"];
		_numProjectilesVariance = reader["numProjectilesVariance"];
		_spreadAngle = reader["spreadAngle"];
		_verticalAngle = reader["verticalAngle"];
		_defaultLaunchAngle = reader["defaultLaunchAngle"];
		_bNonTracking = reader["bNonTracking"];

		_createOffset.UnPackJson(reader["createOffset"]);
		_padding.UnPackJson(reader["padding"]);
		_dims.UnPackJson(reader["dims"]);
		_peturbation.UnPackJson(reader["peturbation"]);

		_imbuedEffect = reader["imbuedEffect"];
		_slayerCreatureType = reader["slayerCreatureType"];
		_slayerDamageBonus = reader["slayerDamageBonus"];
		_critFreq = reader["critFreq"];
		_critMultiplier = reader["critMultiplier"];
		_ignoreMagicResist = reader["ignoreMagicResist"];
		_elementalModifier = reader["elementalModifier"];
#endif		
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		writer["etype"] = _etype;
		writer["baseIntensity"] = _baseIntensity;
		writer["variance"] = _variance;
		writer["wcid"] = _wcid;
		writer["numProjectiles"] = _numProjectiles;
		writer["numProjectilesVariance"] = _numProjectilesVariance;
		writer["spreadAngle"] = _spreadAngle;
		writer["verticalAngle"] = _verticalAngle;
		writer["defaultLaunchAngle"] = _defaultLaunchAngle;
		writer["bNonTracking"] = _bNonTracking;

		_createOffset.PackJson(writer["createOffset"]);
		_padding.PackJson(writer["padding"]);
		_dims.PackJson(writer["dims"]);
		_peturbation.PackJson(writer["peturbation"]);

		writer["imbuedEffect"] = _imbuedEffect;
		writer["slayerCreatureType"] = _slayerCreatureType;
		writer["slayerDamageBonus"] = _slayerDamageBonus;
		writer["critFreq"] = _critFreq;
		writer["critMultiplier"] = _critMultiplier;
		writer["ignoreMagicResist"] = _ignoreMagicResist;
		writer["elementalModifier"] = _elementalModifier;
#endif
	}

	DWORD _etype = 0;
	int _baseIntensity = 0;
	int _variance = 0;
	DWORD _wcid = 0;
	int _numProjectiles = 0;
	float _numProjectilesVariance = 0.0f;
	float _spreadAngle = 0.0f;
	float _verticalAngle = 0.0f;
	float _defaultLaunchAngle = 0.0f;
	int _bNonTracking = 0;
	Vector _createOffset;
	Vector _padding;
	Vector _dims;
	Vector _peturbation;

	DWORD _imbuedEffect = 0;
	int _slayerCreatureType = 0;
	float _slayerDamageBonus = 0.0f;
	double _critFreq = 0.0;
	double _critMultiplier = 0.0;
	int _ignoreMagicResist = 0;
	double _elementalModifier = 0.0;
};

class ProjectileLifeSpellEx : public ProjectileSpellEx
{
public:
	virtual class ProjectileLifeSpellEx *AsLifeProjectileSpell() { return this; }

	virtual bool UnPack(class BinaryReader *pReader)
	{
		ProjectileSpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_drain_percentage = pReader->Read<float>();
		_damage_ratio = pReader->Read<float>();
#endif		
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["drain_percentage"] = _drain_percentage;
		writer["damage_ratio"] = _damage_ratio;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_drain_percentage = reader["drain_percentage"];
		_damage_ratio = reader["damage_ratio"];
		return true;
	}

	float _drain_percentage = 0.0f;
	float _damage_ratio = 0.0f;
};

class ProjectileEnchantmentSpellEx : public ProjectileSpellEx
{
public:
};

class BoostSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_dt = (DAMAGE_TYPE) pReader->Read<int>();
		_boost = pReader->Read<int>();
		_boostVariance = pReader->Read<int>();
#endif
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["dt"] = (DWORD) _dt;
		writer["boost"] = _boost;
		writer["boostVariance"] = _boostVariance;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_dt = (DAMAGE_TYPE) (DWORD)reader["dt"];
		_boost = reader["boost"];
		_boostVariance = reader["boostVariance"];
		return true;
	}

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
	DAMAGE_TYPE _dt;
	int _boost; // e.g. 4-6 would be _boost=4 _boostVariance=2, and -4 to -6 would be _boost=-4 _boostVariance=-2
	int _boostVariance; // boost+boost variance = max change
#endif
};

class FellowshipBoostSpellEx : public BoostSpellEx
{
public:
};

class TransferSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_src = (STypeAttribute2nd)pReader->Read<int>();
		_dest = (STypeAttribute2nd)pReader->Read<int>();
		_proportion = pReader->Read<float>();
		_lossPercent = pReader->Read<float>();
		_sourceLoss = pReader->Read<int>();
		_transferCap = pReader->Read<int>();
		_maxBoostAllowed = pReader->Read<int>();
		_bitfield = pReader->Read<DWORD>();
#endif

		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["src"] = (int) _src;
		writer["dest"] = (int) _dest;
		writer["proportion"] = _proportion;
		writer["lossPercent"] = _lossPercent;
		writer["sourceLoss"] = _sourceLoss;
		writer["transferCap"] = _transferCap;
		writer["maxBoostAllowed"] = _maxBoostAllowed;
		writer["bitfield"] = _bitfield;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_src = (STypeAttribute2nd)(int)reader["src"];
		_dest = (STypeAttribute2nd)(int)reader["dest"];
		_proportion = reader["proportion"];
		_lossPercent = reader["lossPercent"];
		_sourceLoss = reader["sourceLoss"];
		_transferCap = reader["transferCap"];
		_maxBoostAllowed = reader["maxBoostAllowed"];
		_bitfield = reader["bitfield"];
		return true;
	}
	
	enum TransferBitfield
	{
		SourceSelf = 1,
		SourceOther = 2,
		DestinationSelf = 4,
		DestinationOther = 8
	};

	STypeAttribute2nd _src;
	STypeAttribute2nd _dest;
	float _proportion;
	float _lossPercent;
	int _sourceLoss;
	int _transferCap;
	int _maxBoostAllowed;
	DWORD _bitfield; // 1 = source self, 2 = source other, 4 = destination self, 8 = destination other
};

class PortalLinkSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_index = pReader->Read<int>();
#endif
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["index"] = _index;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_index = reader["index"];
		return true;
	}

	int _index;
};

class PortalRecallSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_index = pReader->Read<int>();
#endif
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["index"] = _index;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_index = reader["index"];
		return true;
	}

	int _index = -1; // guess
};

class PortalSendingSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_pos.UnPack(pReader);
#endif
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		_pos.PackJson(writer["pos"]);
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_pos.UnPackJson(reader["pos"]);
		return true;
	}

	Position _pos;
};

class FellowshipPortalSendingSpellEx : public PortalSendingSpellEx
{
public:
};

class DispelSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		//  Align:EnchantmentAlignment(u4 - Neutral, Good, Bad), Number:s4, NumberVariance:f4
		SpellEx::UnPack(pReader);

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_min_power = pReader->Read<int>();
		_max_power = pReader->Read<int>();
		_power_variance = pReader->Read<float>();
		_school = pReader->Read<int>();
		_align = pReader->Read<int>();
		_number = pReader->Read<int>();
		_number_variance = pReader->Read<float>();
#endif
		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["min_power"] = _min_power;
		writer["max_power"] = _max_power;
		writer["power_variance"] = _power_variance;
		writer["school"] = _school;
		writer["align"] = _align;
		writer["number"] = _number;
		writer["number_variance"] = _number_variance;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_min_power = reader["min_power"];
		_max_power = reader["max_power"];
		_power_variance = reader["power_variance"];
		_school = reader["school"];
		_align = reader["align"];
		_number = reader["number"];
		_number_variance = reader["number_variance"];
		return true;
	}

	int _min_power = 0;
	int _max_power = 0;
	float _power_variance = 0.0f;
	int _align = 0;
	int _school = 0;
	int _number = 0;
	float _number_variance = 0.0f;
};

class FellowshipDispelSpellEx : public DispelSpellEx
{
public:
};

class PortalSummonSpellEx : public SpellEx
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		SpellEx::UnPack(pReader);
		_portal_lifetime = pReader->Read<double>();

#if 1 // PHATSDK_USE_INFERRED_SPELL_DATA
		_link = pReader->Read<int>();
#endif

		return true;
	}

	DEFINE_LOCAL_PACK_JSON()
	{
		SpellEx::PackJson(writer);

		writer["portal_lifetime"] = _portal_lifetime;
		writer["link"] = _link;
	}

	DEFINE_LOCAL_UNPACK_JSON()
	{
		SpellEx::UnPackJson(reader);

		_portal_lifetime = reader["portal_lifetime"];
		_link = reader["link"];
		return true;
	}

	double _portal_lifetime;
	int _link;
};

class MetaSpellEx : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy(); // custom

	SpellType _sp_type;
	SpellEx *_spell; // we are just going to leak memory when copying rather than deal with an intrusive list
};

class CSpellBaseEx : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy(); // custom

	SpellFormulaEx InqSpellFormula() const; // decrypts spell formula

	std::string _name;
	std::string _desc;
	unsigned int _school;
	DWORD _iconID;
	unsigned int _category;
	unsigned int _bitfield;
	int _base_mana;
	int _mana_mod;
	float _base_range_constant;
	float _base_range_mod;
	int _power;
	float _spell_economy_mod;
	unsigned int _formula_version;
	float _component_loss;
	SpellFormulaEx _formula;
	PScriptType _caster_effect;
	PScriptType _target_effect;
	PScriptType _fizzle_effect;
	long double _recovery_interval;
	float _recovery_amount;
	int _display_order;
	unsigned int _non_component_target_type;
	MetaSpellEx _meta_spell;
};

class SpellSetTierList : public PackObj
{
public:
	DECLARE_PACKABLE()

	unsigned int m_PieceCount;
	std::list<unsigned long> m_SpellList;
};

class SpellSetEx : public PackObj
{
public:
	DECLARE_PACKABLE()

	std::list<SpellSetTierList> m_countTiers;
};

class CSpellTableEx : public PackObj, public PackableJson
{
public:
	CSpellTableEx();
	virtual ~CSpellTableEx();

	void Destroy(); // custom

	// DECLARE_DBOBJ(CSpellTableEx)
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()
	// DECLARE_LEGACY_PACK_MIGRATOR()

	const CSpellBaseEx *GetSpellBase(DWORD spell_id);

	PackableHashTableWithJson<unsigned long, CSpellBaseEx> _spellBaseHash;
	// don't do this for now PHashTable<unsigned long, SpellSetEx> m_SpellSetHash;
};


class CSpellTableExtendedDataTable : public PackObj, public PackableJson
{
public:
	CSpellTableExtendedDataTable();
	virtual ~CSpellTableExtendedDataTable();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy();

	CSpellTableEx _table;
};
