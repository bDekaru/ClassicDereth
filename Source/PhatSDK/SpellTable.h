
#pragma once

#include "Packable.h"
#include "GameEnums.h"

#define SPELLFORMULA_MAX_COMPS 8

class SpellFormula : public PackObj
{
public:
	DECLARE_PACKABLE()

	BOOL Complete();
	ITEM_TYPE GetTargetingType();
	uint32_t GetPowerLevelOfPowerComponent();
	bool RandomizeForName(const char *accountName, int spellVersion);
	bool RandomizeVersion1(const char *accountName);
	bool RandomizeVersion2(const char *accountName);
	bool RandomizeVersion3(const char *accountName);
	void CopyFrom(SpellFormula other);
	uint64_t GetComponentHash();

	unsigned int _comps[SPELLFORMULA_MAX_COMPS] = {};
};

class Spell : public PackObj
{
public:
	virtual ~Spell() { }

	DECLARE_PACKABLE()

	static Spell *BuildSpell(SpellType sp_type);

	virtual bool IsProjectileSpell() { return false; }
	virtual double InqDuration();

	unsigned int _spell_id;
};

#define INVALID_ENCHANTMENT_DEGRADE_LIMIT -666.0f

class EnchantmentSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

		_duration = pReader->Read<double>();
		_degrade_modifier = pReader->Read<float>();
		_degrade_limit = pReader->Read<float>();

#ifdef EMULATE_INFERRED_SPELL_DATA
		_spellCategory = pReader->Read<uint32_t>();
		_smod.UnPack(pReader);
#endif

		return true;
	}

	double _duration = -1.0;
	float _degrade_modifier = 0.0f;
	float _degrade_limit = INVALID_ENCHANTMENT_DEGRADE_LIMIT;

#ifdef EMULATE_INFERRED_SPELL_DATA
	int _spellCategory = 0;
	StatMod _smod;
#endif
};

class FellowshipEnchantmentSpell : public EnchantmentSpell
{
public:
};

class ProjectileSpell : public Spell
{
public:
	virtual bool IsProjectileSpell() { return true; }

	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		uint32_t etype = pReader->Read<uint32_t>();
		int baseIntensity = pReader->Read<int>();
		int variance = pReader->Read<int>();
		uint32_t wcid = pReader->Read<uint32_t>();
		int numProjectiles = pReader->Read<int>();
		float numProjectilesVariance = pReader->Read<int>();
		float spreadAngle = pReader->Read<float>();
		float verticalAngle = pReader->Read<float>();
		float defaultLaunchAngle = pReader->Read<float>();
		int bNonTracking = pReader->Read<int>();

		Vector createOffset;
		createOffset.UnPack(pReader);

		Vector padding;
		padding.UnPack(pReader);

		Vector dims;
		dims.UnPack(pReader);

		Vector peturbation;
		peturbation.UnPack(pReader);

		uint32_t imbuedEffect = pReader->Read<uint32_t>();
		int slayerCreatureType = pReader->Read<int>();
		float slayerDamageBonus = pReader->Read<float>();
		double critFreq = pReader->Read<double>();
		double critMultiplier = pReader->Read<double>();
		int ignoreMagicResist = pReader->Read<int>();
		double elementalModifier = pReader->Read<double>();
#endif		
		return true;
	}
};

class ProjectileLifeSpell : public ProjectileSpell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		ProjectileSpell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<float>();
		pReader->Read<float>();
#endif		
		return true;
	}
};

class ProjectileEnchantmentSpell : public ProjectileSpell
{
public:
};

class BoostSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
		pReader->Read<int>();
		pReader->Read<int>();
#endif
		return true;
	}
};

class FellowshipBoostSpell : public BoostSpell
{
public:
};

class TransferSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
		pReader->Read<int>();
		pReader->Read<float>();
		pReader->Read<float>();
		pReader->Read<float>();
		pReader->Read<float>();
		pReader->Read<float>();
		pReader->Read<uint32_t>();
#endif

		return true;
	}
};

class PortalLinkSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
#endif
		return true;
	}
};

class PortalRecallSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
#endif
		return true;
	}
};

class PortalSendingSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		Position pos;
		pos.UnPack(pReader);
#endif
		return true;
	}
};

class FellowshipPortalSendingSpell : public PortalSendingSpell
{
public:
};

class DispelSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		//  Align:EnchantmentAlignment(u4 - Neutral, Good, Bad), Number:s4, NumberVariance:f4
		Spell::UnPack(pReader);

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
		pReader->Read<int>();
		pReader->Read<float>();
		pReader->Read<int>();
		pReader->Read<int>();
		pReader->Read<int>();
		pReader->Read<float>();
#endif
		return true;
	}
};

class FellowshipDispelSpell : public DispelSpell
{
public:
};

class PortalSummonSpell : public Spell
{
public:
	virtual bool UnPack(class BinaryReader *pReader)
	{
		Spell::UnPack(pReader);
		_portal_lifetime = pReader->Read<double>();

#ifdef EMULATE_INFERRED_SPELL_DATA
		pReader->Read<int>();
#endif

		return true;
	}

	double _portal_lifetime;
};

class MetaSpell : public PackObj
{
public:
	DECLARE_PACKABLE()

	void Destroy(); // custom

	SpellType _sp_type;
	Spell *_spell; // we are just going to leak memory when copying rather than deal with an intrusive list
};

class CSpellBase : public PackObj
{
public:
	DECLARE_PACKABLE()

	void Destroy(); // custom

	SpellFormula InqSpellFormula() const; // decrypts spell formula
	STypeSkill InqSkillForSpell() const;
	int InqTargetType() const;

	std::string _name;
	std::string _desc;
	unsigned int _school;
	uint32_t _iconID;
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
	SpellFormula _formula;
	PScriptType _caster_effect;
	PScriptType _target_effect;
	PScriptType _fizzle_effect;
	double _recovery_interval;
	float _recovery_amount;
	int _display_order;
	unsigned int _non_component_target_type;
	MetaSpell _meta_spell;
};


class SpellSetTierList : public PackObj
{
public:
	DECLARE_PACKABLE()

	// list of spells for a given set count/level
	PackableList<uint32_t> m_tierSpellList;
};

class SpellSet : public PackObj
{
public:
	DECLARE_PACKABLE()

	// key is level/number of pieces, value is a list of spell ids
	PackableHashTable<uint32_t, SpellSetTierList> m_spellSetTiers;
};


class CSpellTable : public PackObj, public DBObj
{
public:
	CSpellTable();
	virtual ~CSpellTable();

	DECLARE_DBOBJ(CSpellTable)
	DECLARE_PACKABLE()
	DECLARE_LEGACY_PACK_MIGRATOR()

	void Destroy(); // custom

	const CSpellBase *GetSpellBase(uint32_t spell_id);
	const SpellSet *GetSpellSet(uint32_t set_id);

	PackableHashTable<uint32_t, CSpellBase> _spellBaseHash;
	PackableHashTable<uint32_t, SpellSet> _spellSetHash;

#if PHATSDK_IS_SERVER
	uint32_t GetSpellByComponentHash(uint64_t componentHash);

	PackableHashTable<uint64_t, SpellID> _componentsToResearchableSpellsMap;
#endif
};
