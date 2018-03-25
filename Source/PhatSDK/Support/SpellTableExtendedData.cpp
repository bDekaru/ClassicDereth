
#include "stdafx.h"
#include "PhatSDK.h"


#include "StdAfx.h"
#include "SpellTable.h"
#include "SpellComponentTable.h"

DEFINE_PACK(SpellFormulaEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellFormulaEx)
{
	for (int i = 0; i < 8; i++)
		_comps[i] = pReader->Read<DWORD>();

	return true;
}

DEFINE_UNPACK_JSON(SpellFormulaEx)
{
	for (int i = 0; i < 8; i++)
		_comps[i] = reader[i];

	return true;
}

DEFINE_PACK_JSON(SpellFormulaEx)
{
	for (int i = 0; i < 8; i++)
		writer[i] = _comps[i];
}

ITEM_TYPE SpellFormulaEx::GetTargetingType()
{
	int i = 5;
	for (; i < 8; i++)
	{
		if (!_comps[i])
			break;
	}
	return SpellComponentTable::GetTargetTypeFromComponentID(_comps[i]);
}

DWORD SpellFormulaEx::GetPowerLevelOfPowerComponent()
{
	return MagicSystem::DeterminePowerLevelOfComponent(_comps[0]);
}

DEFINE_PACK(SpellEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellEx)
{
	_spell_id = pReader->Read<DWORD>();
	return true;
}

DEFINE_UNPACK_JSON(SpellEx)
{
	_spell_id = reader["spell_id"];
	return true;
}

DEFINE_PACK_JSON(SpellEx)
{
	writer["spell_id"] = _spell_id;
}

double SpellEx::InqDuration()
{
	return -1.0;
}

SpellEx *SpellEx::BuildSpell(SpellType sp_type)
{
	switch (sp_type)
	{
	case SpellType::Enchantment_SpellType:
		return new EnchantmentSpellEx;
	case SpellType::Projectile_SpellType:
		return new ProjectileSpellEx;
	case SpellType::Boost_SpellType:
		return new BoostSpellEx;
	case SpellType::Transfer_SpellType:
		return new TransferSpellEx;
	case SpellType::PortalLink_SpellType:
		return new PortalLinkSpellEx;
	case SpellType::PortalRecall_SpellType:
		return new PortalRecallSpellEx;
	case SpellType::PortalSummon_SpellType:
		return new PortalSummonSpellEx;
	case SpellType::PortalSending_SpellType:
		return new PortalSendingSpellEx;
	case SpellType::Dispel_SpellType:
		return new DispelSpellEx;
	case SpellType::LifeProjectile_SpellType:
		return new ProjectileLifeSpellEx;
	case SpellType::EnchantmentProjectile_SpellType:
		return new ProjectileEnchantmentSpellEx;
	case SpellType::FellowBoost_SpellType:
		return new FellowshipBoostSpellEx;
	case SpellType::FellowEnchantment_SpellType:
		return new FellowshipEnchantmentSpellEx;
	case SpellType::FellowPortalSending_SpellType:
		return new FellowshipPortalSendingSpellEx;
	case SpellType::FellowDispel_SpellType:
		return new FellowshipDispelSpellEx;
	}

	return NULL;
}

void MetaSpellEx::Destroy()
{
	SafeDelete(_spell);
}

DEFINE_PACK(MetaSpellEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(MetaSpellEx)
{
	_sp_type = (SpellType)pReader->Read<int>();

	if (_spell)
	{
		delete _spell;
	}

	_spell = SpellEx::BuildSpell(_sp_type);
	if (_spell)
	{
		_spell->UnPack(pReader);
	}

	return true;
}

DEFINE_UNPACK_JSON(MetaSpellEx)
{
	_sp_type = (SpellType) (int)reader["sp_type"];

	if (_spell)
	{
		delete _spell;
	}

	_spell = SpellEx::BuildSpell(_sp_type);
	if (_spell)
	{
		_spell->UnPackJson(reader["spell"]);
	}

	return true;
}

DEFINE_PACK_JSON(MetaSpellEx)
{
	writer["sp_type"] = (int)_sp_type;

	json &spellEntry = writer["spell"];
	if (_spell)
	{
		_spell->PackJson(spellEntry);
	}
}

void CSpellBaseEx::Destroy()
{
	_meta_spell.Destroy();
}

DEFINE_PACK(CSpellBaseEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(CSpellBaseEx)
{
	_name = pReader->ReadString();
	_desc = pReader->ReadString();

	_school = pReader->Read<DWORD>();
	_iconID = pReader->Read<DWORD>();
	_category = pReader->Read<DWORD>();
	_bitfield = pReader->Read<DWORD>();
	_base_mana = pReader->Read<int>();
	_base_range_constant = pReader->Read<float>();
	_base_range_mod = pReader->Read<float>();
	_power = pReader->Read<int>();
	_spell_economy_mod = pReader->Read<float>();
	_formula_version = pReader->Read<DWORD>();
	_component_loss = pReader->Read<float>();

	_meta_spell.UnPack(pReader);
	_formula.UnPack(pReader);

	_caster_effect = (PScriptType)pReader->Read<int>();
	_target_effect = (PScriptType)pReader->Read<int>();
	_fizzle_effect = (PScriptType)pReader->Read<int>();
	_recovery_interval = pReader->Read<double>();
	_recovery_amount = pReader->Read<float>();
	_display_order = pReader->Read<int>();
	_non_component_target_type = pReader->Read<DWORD>();
	_mana_mod = pReader->Read<int>();

	return true;
}

DEFINE_UNPACK_JSON(CSpellBaseEx)
{
	_name = reader["name"];
	_desc = reader["desc"];

	_school = reader["school"];
	_iconID = reader["iconID"];
	_category = reader["category"];
	_bitfield = reader["bitfield"];
	_base_mana = reader["base_mana"];
	_base_range_constant = reader["base_range_constant"];
	_base_range_mod = reader["base_range_mod"];
	_power = reader["power"];
	_spell_economy_mod = reader["spell_economy_mod"];
	_formula_version = reader["formula_version"];
	_component_loss = reader["component_loss"];

	_meta_spell.UnPackJson(reader["meta_spell"]);
	_formula.UnPackJson(reader["formula"]);

	_caster_effect = reader["caster_effect"];
	_target_effect = reader["target_effect"];
	_fizzle_effect = reader["fizzle_effect"];
	_recovery_interval = reader["recovery_interval"];
	_recovery_amount = reader["recovery_amount"];
	_display_order = reader["display_order"];
	_non_component_target_type = reader["non_component_target_type"];
	_mana_mod = reader["mana_mod"];

	return true;
}


DEFINE_PACK_JSON(CSpellBaseEx)
{
	writer["name"] = _name.c_str(); // GetJsonSafeText(_name.c_str());
	writer["desc"] = _desc.c_str(); // GetJsonSafeText(_desc.c_str());

	writer["school"] = _school;
	writer["iconID"] = _iconID;
	writer["category"] = _category;
	writer["bitfield"] = _bitfield;
	writer["base_mana"] = _base_mana;
	writer["base_range_constant"] = _base_range_constant;
	writer["base_range_mod"] = _base_range_mod;
	writer["power"] = _power;
	writer["spell_economy_mod"] = _spell_economy_mod;
	writer["formula_version"] = _formula_version;
	writer["component_loss"] = _component_loss;

	_meta_spell.PackJson(writer["meta_spell"]);
	_formula.PackJson(writer["formula"]);

	writer["caster_effect"] = _caster_effect;
	writer["target_effect"] = _target_effect;
	writer["fizzle_effect"] = _fizzle_effect;
	writer["recovery_interval"] = _recovery_interval;
	writer["recovery_amount"] = _recovery_amount;
	writer["display_order"] = _display_order;
	writer["non_component_target_type"] = _non_component_target_type;
	writer["mana_mod"] = _mana_mod;
}

SpellFormulaEx CSpellBaseEx::InqSpellFormula() const
{
	SpellFormulaEx formula = _formula;

	for (DWORD i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
	//	if (formula._comps[i])
	//		formula._comps[i] -= (PString::compute_hash(_name.c_str()) % 0x12107680) + (PString::compute_hash(_desc.c_str()) % 0xBEADCF45);
	}

	return formula;
}

CSpellTableEx::CSpellTableEx()
{
}

CSpellTableEx::~CSpellTableEx()
{
	Destroy();
}

void CSpellTableEx::Destroy()
{
	for (auto &entry : _spellBaseHash)
	{
		entry.second.Destroy();
	}
}

DEFINE_PACK(CSpellTableEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(CSpellTableEx)
{
#ifdef PHATSDK_USE_INFERRED_SPELL_DATA
	pReader->Read<DWORD>();
#endif

	_spellBaseHash.UnPack(pReader);

	return true;
}

DEFINE_UNPACK_JSON(CSpellTableEx)
{
	_spellBaseHash.UnPackJson(reader["spellBaseHash"]);
	return true;
}

DEFINE_PACK_JSON(CSpellTableEx)
{
	_spellBaseHash.PackJson(writer["spellBaseHash"]);
}

const CSpellBaseEx *CSpellTableEx::GetSpellBase(DWORD spell_id)
{
	return _spellBaseHash.lookup(spell_id);
}


CSpellTableExtendedDataTable::CSpellTableExtendedDataTable()
{
}

CSpellTableExtendedDataTable::~CSpellTableExtendedDataTable()
{
	Destroy();
}

void CSpellTableExtendedDataTable::Destroy()
{
}

DEFINE_PACK(CSpellTableExtendedDataTable)
{
}

DEFINE_UNPACK(CSpellTableExtendedDataTable)
{
	_table.UnPack(pReader);
	return true;
}

DEFINE_UNPACK_JSON(CSpellTableExtendedDataTable)
{
	_table.UnPackJson(reader["table"]);
	return true;
}

DEFINE_PACK_JSON(CSpellTableExtendedDataTable)
{
	_table.PackJson(writer["table"]);
}
