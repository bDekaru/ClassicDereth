
#include "StdAfx.h"
#include "PhatSDK.h"
#include "InferredPortalData.h"

CInferredPortalData::CInferredPortalData()
{
}

CInferredPortalData::~CInferredPortalData()
{
	for (auto &entry : _mutationFilters)
	{
		delete entry.second;
	}
	_mutationFilters.clear();
}

void CInferredPortalData::Init()
{
#ifndef PUBLIC_BUILD
	LOG(Data, Normal, "Loading inferred portal data...\n");
#endif

	{
		_regionData.Destroy();

		BYTE *data = NULL;
		DWORD length = 0;
		if (LoadDataFromPhatDataBin(1, &data, &length, 0xe8b00434, 0x82092270)) // rded.bin
		{
			BinaryReader reader(data, length);
			_regionData.UnPack(&reader);
			delete[] data;
		}
	}

	{
		std::ifstream fileStream("data\\json\\spells.json");

		if (fileStream.is_open())
		{
			json jsonData;
			fileStream >> jsonData;
			fileStream.close();

			_spellTableData.UnPackJson(jsonData);
		}
		else
		{
			BYTE *data = NULL;
			DWORD length = 0;
			if (LoadDataFromPhatDataBin(2, &data, &length, 0x5D97BAEC, 0x41675123)) //sted.bin
			{
				BinaryReader reader(data, length);
				_spellTableData.UnPack(&reader);
				delete[] data;

	#if 0
	
				for (auto &entry : CachedSpellTable->_spellBaseHash)
				{
					CSpellBase *src = &entry.second;

					if (CSpellBaseEx *dst = (CSpellBaseEx *)_spellTableData._table._spellBaseHash.lookup(entry.first))
					{
						dst->_base_mana = src->_base_mana;
						dst->_base_range_constant = src->_base_range_constant;
						dst->_base_range_mod = src->_base_range_mod;
						dst->_bitfield = src->_bitfield;
						dst->_caster_effect = src->_caster_effect;
						dst->_category = src->_category;
						dst->_component_loss = src->_component_loss;
						dst->_desc = src->_desc;
						dst->_display_order = src->_display_order;
						dst->_fizzle_effect = src->_fizzle_effect;

						SpellFormula formula = src->InqSpellFormula();
						memcpy(dst->_formula._comps, formula._comps, sizeof(src->_formula._comps));
						dst->_formula_version = src->_formula_version;
						dst->_iconID = src->_iconID;
						dst->_mana_mod = src->_mana_mod;
						dst->_name = src->_name;
						dst->_non_component_target_type = src->_non_component_target_type;
						dst->_power = src->_power;
						dst->_recovery_amount = src->_recovery_amount;
						dst->_recovery_interval = src->_recovery_interval;
						dst->_school = src->_school;
						dst->_spell_economy_mod = src->_spell_economy_mod;
						dst->_target_effect = src->_target_effect;

						if (dst->_meta_spell._sp_type == Enchantment_SpellType)
						{
							if (EnchantmentSpellEx *spellEx = (EnchantmentSpellEx *)dst->_meta_spell._spell)
							{
								if (spellEx->_smod.type & Skill_EnchantmentType)
								{
									spellEx->_smod.key = (DWORD)SkillTable::OldToNewSkill((STypeSkill)spellEx->_smod.key);
								}
							}
						}

						if (dst->_meta_spell._sp_type == Projectile_SpellType)
						{
							std::regex rgx("[<base>0-9]+\\-[<var>0-9]+ points");
							std::smatch matches;

							if (std::regex_search(dst->_desc, matches, rgx))
							{
								int base, high;
								if (sscanf(matches[0].str().c_str(), "%d-%d", &base, &high) == 2)
								{
									ProjectileSpellEx *proj = (ProjectileSpellEx *)dst->_meta_spell._spell;

									proj->_baseIntensity = base;
									proj->_variance = high - base;

									// LOG(Temp, Normal, "Damage for %s is %d-%d\n", dst->_name.c_str(), base, high);
								}
							}
						}
					}
					else
					{
						// does not exist
						if (src->_meta_spell._sp_type == Projectile_SpellType)
						{
							if (strstr(src->_name.c_str(), "Incantation of "))
							{
								DWORD sample_spell_id = 0;

								if (strstr(src->_name.c_str(), "Acid"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = AcidArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = AcidBlast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = AcidStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Stream"))
										sample_spell_id = AcidStream7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = AcidVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Bludgeon") || strstr(src->_name.c_str(), "Shock"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = ShockArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = Shockblast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = ShockwaveStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Incantation of Shock Wave"))
										sample_spell_id = Shockwave7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = BludgeoningVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Flame"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = FlameArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = FlameBlast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = FlameStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Bolt"))
										sample_spell_id = FlameBolt7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = FlameVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Frost"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = FrostArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = Frostblast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = FrostStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Bolt"))
										sample_spell_id = FrostBolt7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = FrostVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Lightning"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = LightningArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = Lightningblast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = LightningStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Bolt"))
										sample_spell_id = Lightningbolt7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = LightningVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Force"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = ForceArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = ForceBlast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = ForceStreak7_SpellID;
									else if (strstr(src->_name.c_str(), "Bolt"))
										sample_spell_id = ForceBolt7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = ForceVolley7_SpellID;
								}
								else if (strstr(src->_name.c_str(), "Blade"))
								{
									if (strstr(src->_name.c_str(), "Arc"))
										sample_spell_id = BladeArc7_SpellID;
									else if (strstr(src->_name.c_str(), "Blast"))
										sample_spell_id = BladeBlast7_SpellID;
									else if (strstr(src->_name.c_str(), "Streak"))
										sample_spell_id = WhirlingBladeStreak7_SpellID;
									else if (!strcmp(src->_name.c_str(), "Incantation of Whirling Blade"))
										sample_spell_id = Whirlingblade7_SpellID;
									else if (strstr(src->_name.c_str(), "Volley"))
										sample_spell_id = BladeVolley7_SpellID;
								}

								if (sample_spell_id)
								{
									CSpellBaseEx *sample_src = (CSpellBaseEx *)_spellTableData._table._spellBaseHash.lookup(sample_spell_id);

									// if (CSpellBase *src = (CSpellBase *)CachedSpellTable->_spellBaseHash.lookup(entry.first))
									{
										CSpellBaseEx newSpellEx;									
										CSpellBaseEx *dst = &newSpellEx;

										dst->_base_mana = src->_base_mana;
										dst->_base_range_constant = src->_base_range_constant;
										dst->_base_range_mod = src->_base_range_mod;
										dst->_bitfield = src->_bitfield;
										dst->_caster_effect = src->_caster_effect;
										dst->_category = src->_category;
										dst->_component_loss = src->_component_loss;
										dst->_desc = src->_desc;
										dst->_display_order = src->_display_order;
										dst->_fizzle_effect = src->_fizzle_effect;

										SpellFormula formula = src->InqSpellFormula();
										memcpy(dst->_formula._comps, formula._comps, sizeof(formula._comps));
										dst->_formula_version = src->_formula_version;
										dst->_iconID = src->_iconID;
										dst->_mana_mod = src->_mana_mod;
										dst->_name = src->_name;
										dst->_non_component_target_type = src->_non_component_target_type;
										dst->_power = src->_power;
										dst->_recovery_amount = src->_recovery_amount;
										dst->_recovery_interval = src->_recovery_interval;
										dst->_school = src->_school;
										dst->_spell_economy_mod = src->_spell_economy_mod;
										dst->_target_effect = src->_target_effect;

										dst->_meta_spell._sp_type = Projectile_SpellType;
										dst->_meta_spell._spell = new ProjectileSpellEx;
										*((ProjectileSpellEx *)dst->_meta_spell._spell) = *(ProjectileSpellEx *)sample_src->_meta_spell._spell;

										std::regex rgx("[<base>0-9]+\\-[<var>0-9]+ points");
										std::smatch matches;

										if (std::regex_search(dst->_desc, matches, rgx))
										{
											int base, high;
											if (sscanf(matches[0].str().c_str(), "%d-%d", &base, &high) == 2)
											{
												ProjectileSpellEx *proj = (ProjectileSpellEx *)dst->_meta_spell._spell;
												proj->_baseIntensity = base;
												proj->_variance = high - base;

												_spellTableData._table._spellBaseHash[entry.first] = newSpellEx;
											}
										}
									}
								}
							}
						}
					}
				}

				json test;
				_spellTableData.PackJson(test);
				std::string testString = test.dump(1, '\t');

				FILE *fp = fopen("d:\\temp\\spells.json", "wt");
				if (fp)
				{
					fprintf(fp, "%s\n", testString.c_str());
					fclose(fp);
				}
#endif
			}
		}
	}

	{
		BYTE *data = NULL;
		DWORD length = 0;
		if (LoadDataFromPhatDataBin(3, &data, &length, 0x7DC126EB, 0x5F41B9AD)) // tted.bin
		{
			BinaryReader reader(data, length);
			_treasureTableData.UnPack(&reader);
			delete[] data;
		}
	}

	{
		BYTE *data = NULL;
		DWORD length = 0;
		if (LoadDataFromPhatDataBin(4, &data, &length, 0x5F41B9AD, 0x7DC126EB)) // cted.bin
		{
			BinaryReader reader(data, length);
			_craftTableData.UnPack(&reader);
			delete[] data;
		}
	}

	{
		BYTE *data = NULL;
		DWORD length = 0;
		if (LoadDataFromPhatDataBin(5, &data, &length, 0x887aef9c, 0xa92ec9ac)) // hpd.bin
		{
			BinaryReader reader(data, length);
			_housePortalDests.UnPack(&reader);
			delete[] data;
		}
	}

	{
		std::ifstream fileStream("data\\json\\quests.json");

		if (fileStream.is_open())
		{
			json jsonData;
			fileStream >> jsonData;
			fileStream.close();

			_questDefDB.UnPackJson(jsonData);
		}
		else
		{
			BYTE *data = NULL;
			DWORD length = 0;
			if (LoadDataFromPhatDataBin(8, &data, &length, 0xE80D81CA, 0x8ECA9786))
			{
				BinaryReader reader(data, length);
				_questDefDB.UnPack(&reader);
				delete[] data;
			
				/*
				json test;
				_questDefDB.PackJson(test);
				std::string testString = test.dump(4);

				FILE *fp = fopen("d:\\temp\\test_quest_json.txt", "wt");
				if (fp)
				{
					fprintf(fp, "%s\n", testString.c_str());
					fclose(fp);
				}
				*/
			}
		}
	}

	{
		BYTE *data = NULL;
		DWORD length = 0;
		if (LoadDataFromPhatDataBin(10, &data, &length, 0x5f1fa913, 0xe345c74c))
		{
			BinaryReader reader(data, length);

			DWORD numEntries = reader.Read<DWORD>();
			for (DWORD i = 0; i < numEntries; i++)
			{
				DWORD key = reader.Read<DWORD>();

				DWORD dataLength = reader.Read<DWORD>();

				BinaryReader entryReader(reader.ReadArray(dataLength), dataLength);
				CMutationFilter *val = new CMutationFilter();
				val->UnPack(&entryReader);

				_mutationFilters[key] = val;
			}

			delete[] data;
		}
	}

	{
		std::ifstream fileStream("data\\json\\events.json");

		if (fileStream.is_open())
		{
			json jsonData;
			fileStream >> jsonData;
			fileStream.close();

			_gameEvents.UnPackJson(jsonData);
		}
		else
		{
			BYTE *data = NULL;
			DWORD length = 0;
			if (LoadDataFromPhatDataBin(11, &data, &length, 0x812a7823, 0x8b28e107))
			{
				BinaryReader reader(data, length);
				_gameEvents.UnPack(&reader);
				delete[] data;

				/*
				json test;
				_gameEvents.PackJson(test);
				std::string testString = test.dump(4);

				FILE *fp = fopen("d:\\temp\\test_game_json.txt", "wt");
				if (fp)
				{
					fprintf(fp, "%s\n", testString.c_str());
					fclose(fp);
				}
				*/
			}
		}
	}

#ifndef PUBLIC_BUILD
	LOG(Data, Normal, "Finished loading inferred cell data.\n");
#endif
}

DWORD CInferredPortalData::GetWCIDForTerrain(long x, long y, int index)
{
	return _regionData.GetEncounter(x, y, index);
}

CSpellTableEx *CInferredPortalData::GetSpellTableEx()
{
	return &_spellTableData._table;
}

CCraftOperation *CInferredPortalData::GetCraftOperation(DWORD source_wcid, DWORD dest_wcid)
{
	DWORD64 toolComboKey = ((DWORD64)source_wcid << 32) | dest_wcid;
	const DWORD *opKey = g_pPortalDataEx->_craftTableData._precursorMap.lookup(toolComboKey);

	if (!opKey)
		return NULL;

	return g_pPortalDataEx->_craftTableData._operations.lookup(*opKey);
}

Position *CInferredPortalData::GetHousePortalDest(DWORD house_id, DWORD ignore_cell_id)
{
	PackableList<Position> *dests = _housePortalDests.lookup(house_id);

	if (dests)
	{
		for (auto &entry : *dests)
		{
			if (entry.objcell_id != ignore_cell_id)
				return &entry;
		}
	}

	return NULL;
}

CMutationFilter *CInferredPortalData::GetMutationFilter(DWORD id)
{
	std::unordered_map<DWORD, CMutationFilter *>::iterator entry = _mutationFilters.find(id & 0xFFFFFF);
	
	if (entry != _mutationFilters.end())
	{
		return entry->second;
	}

	return NULL;
}


