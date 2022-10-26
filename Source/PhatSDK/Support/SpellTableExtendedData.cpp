
#include <StdAfx.h>
#include "PhatSDK.h"
#include "SpellTable.h"
#include "SpellComponentTable.h"

DEFINE_PACK(SpellFormulaEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellFormulaEx)
{
	for (int i = 0; i < 8; i++)
		_comps[i] = pReader->Read<uint32_t>();

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

uint32_t SpellFormulaEx::GetPowerLevelOfPowerComponent()
{
	return MagicSystem::DeterminePowerLevelOfComponent(_comps[0]);
}

uint64_t SpellFormulaEx::GetComponentHash()
{
	uint64_t componentHash = 0;
	for (int i = 0; i < 8; i++)
	{
		int comp = _comps[i];

		if (comp > 62 && comp < 75) //tapers turn to 0 so as that's being replaced with the personal ones.
			comp = 0;

		componentHash |= (uint64_t)comp << (8 * i);
	}

	return componentHash;
}

bool SpellFormulaEx::RandomizeForName(const char* accountName, int spellVersion)
{
	switch (spellVersion)
	{
	case 1:
		return RandomizeVersion1(accountName);
	case 2:
		return RandomizeVersion2(accountName);
	case 3:
		return RandomizeVersion3(accountName);
	}
	return false;
}

bool SpellFormulaEx::RandomizeVersion1(const char* accountName)
{
	int amountOfComponents = 0;
	if (_comps[0])
		amountOfComponents = 1;
	if (_comps[1])
		amountOfComponents++;
	if (_comps[2])
		amountOfComponents++;
	if (_comps[3])
		amountOfComponents++;
	if (_comps[4])
		amountOfComponents++;
	if (_comps[5])
		amountOfComponents++;
	if (_comps[6])
		amountOfComponents++;
	if (_comps[7])
		amountOfComponents++;

	int seed = PString::compute_hash(accountName) % 0x13D573;
	int accent1 = 0;
	int accent2 = 0;
	int accent3 = 0;

	int v8 = _comps[0];
	int v9 = 1;
	if (amountOfComponents > 5)
	{
		v9 = 2;
		accent1 = 2;
	}

	int v10 = _comps[v9];
	int v11 = v9 + 1;
	int a = v10;
	if (amountOfComponents > 6)
	{
		v11++;
		accent2 = 1;
	}

	int v16;
	if (v11 < 0 || v11 >= 8)
		v16 = 0;
	else
		v16 = _comps[v11];

	int v12 = v11 + 1;
	int v3 = 0;
	if (v12 >= 0 && v12 < 8)
		v3 = _comps[v12];
	int v13 = v12 + 1;
	int x = v3;

	if (amountOfComponents > 7)
	{
		v13++;
		accent3 = 1;
	}

	int v14;
	if (v13 < 0 || v13 >= 8)
		v14 = 0;
	else
		v14 = _comps[v13];

	if (accent1)
	{
		if (!(v8 + v10))
			v8 = 1;
		_comps[1] = (v8 + v16 + v3 + v14 + 2 * v10) % 0xC + MagicSystem::GetLowestTaperID();
	}
	if (accent2)
	{
		int v15 = v16 + v3;
		if (!(v8 + v15))
			v8 = 1;
		_comps[3] = (v8 + a + v14 + 2 * v15) * (seed / (v8 + v15)) % 0xC + MagicSystem::GetLowestTaperID();
	}
	if (accent3)
	{
		if (!(v14 + v8))
			v8 = 1;
		_comps[6] = (v8 + a + v16 + x + 2 * v14) * (seed / (v14 + v8)) % 0xC + MagicSystem::GetLowestTaperID();
	}

	return true;
}

bool SpellFormulaEx::RandomizeVersion2(const char* accountName)
{
	unsigned int v5;
	unsigned int v6;
	unsigned int v7;
	unsigned int seed;
	unsigned int p1;
	unsigned int c;
	unsigned int x;
	unsigned int v12;
	unsigned int v13;

	seed = PString::compute_hash(accountName);
	v5 = _comps[0];
	v6 = _comps[1];
	v7 = _comps[2];
	p1 = _comps[0];
	c = _comps[4];
	x = _comps[5];
	v13 = _comps[7];
	v12 = MagicSystem::GetLowestTaperID();
	_comps[3] = (v6 + v7 + v5 + v13 + 2 * v5 + 2 * c * x) % 0xC + MagicSystem::GetLowestTaperID();
	_comps[6] = (c + p1 * v7 + v13 + 2 * p1 * v7 + 2 * x) * (seed % 0x13D573 / (v6 * v13 + 2 * c)) % 0xC + v12;
	return true;
}

bool SpellFormulaEx::RandomizeVersion3(const char* accountName)
{
	unsigned int seed = PString::compute_hash(accountName);

	unsigned int s1 = seed % 0x13D573;
	unsigned int s2 = seed % 0x4AEFD;
	unsigned int s3 = seed % 0x96A7F;
	unsigned int v18 = seed % 0x100A03;
	unsigned int v32 = seed % 0xEB2EF;
	unsigned int s6 = seed % 0x121E7D;
	unsigned int v25 = seed % 0x65039;

	unsigned int v26 = (s1 + _comps[0]) % 0xC;
	unsigned int v27 = (s2 + _comps[1]) % 0xC;
	unsigned int v28 = (s3 + _comps[2]) % 0xC;
	unsigned int s4 = (v18 + _comps[4]) % 0xC;
	unsigned int v33 = (v32 + _comps[5]) % 0xC;
	unsigned int v30 = (s6 + _comps[7]) % 0xC;

	_comps[3] = (v26 + v27 + v28 + s4 + v33 + v28 * v33 + v26 * v27 + v30 * (s4 + 1)) % 0xC + MagicSystem::GetLowestTaperID();

	_comps[6] = (v26 + v27 + v28 + s4 + v25 % 0xC + v30 * (s4 * (v26 * v27 * v28 * v33 + 7) + 1) + v33 + 4 * v26 * v27 + v26 * v27 + 11 * v28 * v33) % 0xC + MagicSystem::GetLowestTaperID();

	return true;
}

void SpellFormulaEx::CopyFrom(SpellFormulaEx other)
{
	for (int i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
		this->_comps[i] = other._comps[i];
	}
}

DEFINE_PACK(SpellEx)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellEx)
{
	_spell_id = pReader->Read<uint32_t>();
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

	_school = pReader->Read<uint32_t>();
	_iconID = pReader->Read<uint32_t>();
	_category = pReader->Read<uint32_t>();
	_bitfield = pReader->Read<uint32_t>();
	_base_mana = pReader->Read<int>();
	_base_range_constant = pReader->Read<float>();
	_base_range_mod = pReader->Read<float>();
	_power = pReader->Read<int>();
	_spell_economy_mod = pReader->Read<float>();
	_formula_version = pReader->Read<uint32_t>();
	_component_loss = pReader->Read<float>();

	_meta_spell.UnPack(pReader);
	_formula.UnPack(pReader);

	_caster_effect = (PScriptType)pReader->Read<int>();
	_target_effect = (PScriptType)pReader->Read<int>();
	_fizzle_effect = (PScriptType)pReader->Read<int>();
	_recovery_interval = pReader->Read<double>();
	_recovery_amount = pReader->Read<float>();
	_display_order = pReader->Read<int>();
	_non_component_target_type = pReader->Read<uint32_t>();
	_mana_mod = pReader->Read<int>();

	return true;
}

DEFINE_UNPACK_JSON(CSpellBaseEx)
{
	_name = reader["name"].get<std::string>();
	_desc = reader["desc"].get<std::string>();

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

	for (uint32_t i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
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
	pReader->Read<uint32_t>();
#endif

	_spellBaseHash.UnPack(pReader);

	InitializeSpellResearchTables();

	return true;
}

DEFINE_UNPACK_JSON(CSpellTableEx)
{
	_spellBaseHash.UnPackJson(reader["spellBaseHash"]);

	InitializeSpellResearchTables();
	return true;
}

DEFINE_PACK_JSON(CSpellTableEx)
{
	_spellBaseHash.PackJson(writer["spellBaseHash"]);
}

void CSpellTableEx::InitializeSpellResearchTables()
{
	_componentsToResearchableSpellsMap.clear();

	for (auto& entry : _spellBaseHash)
	{
		if (entry.second._bitfield & NotResearchable_SpellIndex)
			continue;

		uint64_t hash = entry.second.InqSpellFormula().GetComponentHash();

		if (_componentsToResearchableSpellsMap.lookup(hash) == NULL)
			_componentsToResearchableSpellsMap[hash] = (SpellID)entry.first;
		//else
		//	LOG_PRIVATE(Data, Normal, (std::to_string(entry.first) + "\t" + entry.second._name + "\n").c_str()); // Uncomment to log formula conflicts.
	}
}

uint32_t CSpellTableEx::GetSpellByComponentHash(uint64_t componentHash)
{
	SpellID* returnValue = _componentsToResearchableSpellsMap.lookup(componentHash);
	if (returnValue == NULL)
		return 0;
	return *returnValue;
}

const CSpellBaseEx *CSpellTableEx::GetSpellBase(uint32_t spell_id)
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
