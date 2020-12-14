
#include <StdAfx.h>
#include "SpellTable.h"
#include "SpellComponentTable.h"

DEFINE_PACK(SpellFormula)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellFormula)
{
	for (int i = 0; i < 8; i++)
		_comps[i] = pReader->Read<uint32_t>();

	return true;
}

BOOL SpellFormula::Complete()
{
	for (uint32_t i = 0; i < 5; i++)
	{
		if (!_comps[i])
			return FALSE;
	}

	return TRUE;
}

ITEM_TYPE SpellFormula::GetTargetingType()
{
	int i = 5;
	for (; i < 8; i++)
	{
		if (!_comps[i])
			break;
	}
	return SpellComponentTable::GetTargetTypeFromComponentID(_comps[i - 1]);
}

uint32_t SpellFormula::GetPowerLevelOfPowerComponent()
{
	return MagicSystem::DeterminePowerLevelOfComponent(_comps[0]);
}

bool SpellFormula::RandomizeForName(const char *accountName, int spellVersion)
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

bool SpellFormula::RandomizeVersion1(const char *accountName)
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

bool SpellFormula::RandomizeVersion2(const char *accountName)
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
	v5 =_comps[0];
	v6 =_comps[1];
	v7 =_comps[2];
	p1 =_comps[0];
	c =_comps[4];
	x =_comps[5];
	v13 =_comps[7];
	v12 = MagicSystem::GetLowestTaperID();
	_comps[3] = (v6 + v7 + v5 + v13 + 2 * v5 + 2 * c * x) % 0xC + MagicSystem::GetLowestTaperID();
	_comps[6] = (c + p1 * v7 + v13 + 2 * p1 * v7 + 2 * x) * (seed % 0x13D573 / (v6 * v13 + 2 * c)) % 0xC + v12;
	return true;
}

bool SpellFormula::RandomizeVersion3(const char *accountName)
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

void SpellFormula::CopyFrom(SpellFormula other)
{
	for (int i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
		this->_comps[i] = other._comps[i];
	}
}

DEFINE_PACK(Spell)
{
	UNFINISHED();
}

DEFINE_UNPACK(Spell)
{
	_spell_id = pReader->Read<uint32_t>();
	return true;
}

double Spell::InqDuration()
{
	return -1.0;
}

Spell *Spell::BuildSpell(SpellType sp_type)
{
	switch (sp_type)
	{
	case SpellType::Enchantment_SpellType:
		return new EnchantmentSpell;
	case SpellType::Projectile_SpellType:
		return new ProjectileSpell;
	case SpellType::Boost_SpellType:
		return new BoostSpell;
	case SpellType::Transfer_SpellType:
		return new TransferSpell;
	case SpellType::PortalLink_SpellType:
		return new PortalLinkSpell;
	case SpellType::PortalRecall_SpellType:
		return new PortalRecallSpell;
	case SpellType::PortalSummon_SpellType:
		return new PortalSummonSpell;
	case SpellType::PortalSending_SpellType:
		return new PortalSendingSpell;
	case SpellType::Dispel_SpellType:
		return new DispelSpell;
	case SpellType::LifeProjectile_SpellType:
		return new ProjectileLifeSpell;
	case SpellType::EnchantmentProjectile_SpellType:
		return new ProjectileEnchantmentSpell;
	case SpellType::FellowBoost_SpellType:
		return new FellowshipBoostSpell;
	case SpellType::FellowEnchantment_SpellType:
		return new FellowshipEnchantmentSpell;
	case SpellType::FellowPortalSending_SpellType:
		return new FellowshipPortalSendingSpell;
	case SpellType::FellowDispel_SpellType:
		return new FellowshipDispelSpell;
	}

	return NULL;
}

void MetaSpell::Destroy()
{
	SafeDelete(_spell);
}

DEFINE_PACK(MetaSpell)
{
	UNFINISHED();
}

DEFINE_UNPACK(MetaSpell)
{
	_sp_type = (SpellType) pReader->Read<int>();

	if (_spell)
	{
		delete _spell;
	}

	_spell = Spell::BuildSpell(_sp_type);
	if (_spell)
	{
		_spell->UnPack(pReader);
	}

	return true;
}

void CSpellBase::Destroy()
{
	_meta_spell.Destroy();
}

DEFINE_PACK(CSpellBase)
{
	UNFINISHED();
}

DEFINE_UNPACK(CSpellBase)
{
	_name = pReader->ReadString();

#ifndef EMULATE_INFERRED_SPELL_DATA
	// these are obfuscated, swap low/high nibbles
	for (int i = 0; i < _name.size(); i++)
		_name[i] = (char)(BYTE)((BYTE)((BYTE)_name[i]<<4)| (BYTE)((BYTE)_name[i] >> 4));
#endif

	_desc = pReader->ReadString();

#ifndef EMULATE_INFERRED_SPELL_DATA
	// these are obfuscated, swap low/high nibbles
	for (int i = 0; i < _desc.size(); i++)
		_desc[i] = (char)(BYTE)((BYTE)((BYTE)_desc[i] << 4) | (BYTE)((BYTE)_desc[i] >> 4));
#endif

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

	_caster_effect = (PScriptType) pReader->Read<int>();
	_target_effect = (PScriptType)pReader->Read<int>();
	_fizzle_effect = (PScriptType)pReader->Read<int>();
	_recovery_interval = pReader->Read<double>();
	_recovery_amount = pReader->Read<float>();
	_display_order = pReader->Read<int>();
	_non_component_target_type = pReader->Read<uint32_t>();
	_mana_mod = pReader->Read<int>();

	return true;
}

SpellFormula CSpellBase::InqSpellFormula() const
{
	SpellFormula formula = _formula;
	
	for (uint32_t i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
		if (formula._comps[i])
			formula._comps[i] -= (PString::compute_hash(_name.c_str()) % 0x12107680) + (PString::compute_hash(_desc.c_str()) % 0xBEADCF45);
	}

	return formula;
}

STypeSkill CSpellBase::InqSkillForSpell() const
{
	switch (this->_school)
	{
	case 5:
		return STypeSkill::VOID_MAGIC_SKILL;

	case 1:
		return STypeSkill::WAR_MAGIC_SKILL;
	
	case 2:
		return STypeSkill::LIFE_MAGIC_SKILL;

	case 3:
		return STypeSkill::ITEM_ENCHANTMENT_SKILL;

	case 4:
		return STypeSkill::CREATURE_ENCHANTMENT_SKILL;
	}

	return STypeSkill::UNDEF_SKILL;
}

int CSpellBase::InqTargetType() const
{
	SpellFormula formula = InqSpellFormula();

	if (formula.Complete())
		return formula.GetTargetingType();

	return 0;
}


DEFINE_PACK(SpellSetTierList)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellSetTierList)
{
	m_tierSpellList.UnPack(pReader);
	return true;
}

DEFINE_PACK(SpellSet)
{
	UNFINISHED();
}

DEFINE_UNPACK(SpellSet)
{
	m_spellSetTiers.UnPack(pReader);
	return true;
}


CSpellTable::CSpellTable()
{
}

CSpellTable::~CSpellTable()
{
	Destroy();
}

void CSpellTable::Destroy()
{
	for (auto &entry : _spellBaseHash)
	{
		entry.second.Destroy();
	}
}

DEFINE_DBOBJ(CSpellTable, SpellTables)
DEFINE_LEGACY_PACK_MIGRATOR(CSpellTable)

DEFINE_PACK(CSpellTable)
{
	pWriter->Write<uint32_t>(id);
	_spellBaseHash.Pack(pWriter);
	// m_SpellSetHash....
}

DEFINE_UNPACK(CSpellTable) // type 0x0E00000E
{
	uint32_t data_id = pReader->Read<uint32_t>(); // id
	_spellBaseHash.UnPack(pReader);
	_spellSetHash.UnPack(pReader);

#if 0
	for (auto it : _spellSetHash)
	{
		SERVER_INFO << "Set ID: " << it.first;
		for (auto it2 : it.second.m_spellSetTiers)
		{
			SERVER_INFO << "Level/Pieces: " << it2.first;
			for (auto it3 : it2.second.m_tierSpellList)
			{
				SERVER_INFO << "Set spells: " << it3;
			}
		}

	}
#endif


#if PHATSDK_IS_SERVER
	_categoryToResearchableSpellsMap.clear();

	for (auto &entry : _spellBaseHash)
	{
		int spellLevel = entry.second.InqSpellFormula().GetPowerLevelOfPowerComponent();
		if (spellLevel < 1 || spellLevel > 8)
			continue;

		if (spellLevel < 7)
		{
			if (entry.second._bitfield & NotResearchable_SpellIndex)
				continue;

			const char *suffix;
			switch (spellLevel)
			{
			case 1: suffix = " I"; break;
			case 2: suffix = " II"; break;
			case 3: suffix = " III"; break;
			case 4: suffix = " IV"; break;
			case 5: suffix = " V"; break;
			case 6: suffix = " VI"; break;
			default: continue;
			}

			std::string spellName = entry.second._name;
			const char *p = strstr(spellName.c_str(), suffix);
			if (!p)
				continue;

			int suffixPos = p - spellName.c_str();
			if (suffixPos != (spellName.length() - strlen(suffix)))
				continue;
		}

		if (auto entry1 = _categoryToResearchableSpellsMap.lookup(entry.second._category))
		{
			if (auto entry2 = entry1->lookup(entry.second._bitfield & (SelfTargeted_SpellIndex | FellowshipSpell_SpellIndex)))
			{
				if (auto entry3 = entry2->lookup(spellLevel))
				{
					// already have an entry
					continue;
				}
			}
		}

		_categoryToResearchableSpellsMap[entry.second._category][entry.second._bitfield & (SelfTargeted_SpellIndex|FellowshipSpell_SpellIndex)][spellLevel] = (SpellID) entry.first;
	}

#endif

	return true;
}

#if PHATSDK_IS_SERVER
uint32_t CSpellTable::ChangeSpellToDifferentLevel(uint32_t spell_id, uint32_t spell_level)
{
	if (const CSpellBase *spell = GetSpellBase(spell_id))
	{
		if (auto categoryMap = _categoryToResearchableSpellsMap.lookup(spell->_category))
		{
			if (auto levelMap = categoryMap->lookup(spell->_bitfield & (SelfTargeted_SpellIndex | FellowshipSpell_SpellIndex)))
			{
				if (auto levelEntry = levelMap->lookup(spell_level))
				{
					return (uint32_t) *levelEntry;
				}
			}
		}
	}

	return 0;
}
#endif

const CSpellBase *CSpellTable::GetSpellBase(uint32_t spell_id)
{
	return _spellBaseHash.lookup(spell_id);
}

const SpellSet *CSpellTable::GetSpellSet(uint32_t set_id)
{
	return _spellSetHash.lookup(set_id);
}
