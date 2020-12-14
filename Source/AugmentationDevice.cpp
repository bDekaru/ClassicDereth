#include <StdAfx.h>
#include "AugmentationDevice.h"
#include "UseManager.h"
#include "Player.h"
#include "Qualities.h"
#include "ChatMsgs.h"
#include "World.h"

CAugmentationDeviceWeenie::CAugmentationDeviceWeenie()
{
}

CAugmentationDeviceWeenie::~CAugmentationDeviceWeenie()
{
}

int CAugmentationDeviceWeenie::Use(CPlayerWeenie *player)
{
	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	return UseEx(player, false);
}

int CAugmentationDeviceWeenie::UseEx(CPlayerWeenie *player, bool bConfirmed)
{
	int64_t augCost = InqInt64Quality(AUGMENTATION_COST_INT64, 0);

	// Save these for later for the confirmation response
	player->m_pCraftingTool = this;
	player->m_pCraftingTarget = player;
	
	if (player->m_pCraftingTool == NULL || player->m_pCraftingTarget == NULL || !player->FindContainedItem(GetID()))
	{
		// Item missing.
		return WERROR_NONE;
	}

	if (!bConfirmed)
	{
		std::ostringstream sstrMessage;
		sstrMessage.imbue(std::locale(""));
		sstrMessage << "This action will augment your character with " << GetName() << " and will cost " << augCost << " available experience.";

		BinaryWriter confirmAug;
		confirmAug.Write<uint32_t>(0x274);	// Message Type
		confirmAug.Write<uint32_t>(0x06);		// Confirm type (augmentation)
		confirmAug.Write<int>(0);			// Sequence number??
		confirmAug.WriteString(sstrMessage.str());

		player->SendNetMessage(&confirmAug, PRIVATE_MSG, TRUE, FALSE);

		return WERROR_NONE;
	}

	int64_t unassignedXP = player->InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0);
	int aug = InqIntQuality(AUGMENTATION_STAT_INT, 0);
	int augJackOfTrades = player->InqIntQuality(AUGMENTATION_JACK_OF_ALL_TRADES_INT, 0);
	int augSkilledMelee = player->InqIntQuality(AUGMENTATION_SKILLED_MELEE_INT, 0);
	int augSkilledMissile = player->InqIntQuality(AUGMENTATION_SKILLED_MISSILE_INT, 0);
	int augSkilledMagic = player->InqIntQuality(AUGMENTATION_SKILLED_MAGIC_INT, 0);
	int augInnates = player->InqIntQuality(AUGMENTATION_INNATE_FAMILY_INT, 0);
	int augIncreasedBurden = player->InqIntQuality(AUGMENTATION_INCREASED_CARRYING_CAPACITY_INT, 0);
	int augSpecSalvage = player->InqIntQuality(AUGMENTATION_SPECIALIZE_SALVAGING_INT, 0);
	int augSpecItemTink = player->InqIntQuality(AUGMENTATION_SPECIALIZE_ITEM_TINKERING_INT, 0);
	int augSpecArmorTink = player->InqIntQuality(AUGMENTATION_SPECIALIZE_ARMOR_TINKERING_INT, 0);
	int augSpecMagicTink = player->InqIntQuality(AUGMENTATION_SPECIALIZE_MAGIC_ITEM_TINKERING_INT, 0);
	int augSpecWeaponTink = player->InqIntQuality(AUGMENTATION_SPECIALIZE_WEAPON_TINKERING_INT, 0);
	int augExtraPack = player->InqIntQuality(AUGMENTATION_EXTRA_PACK_SLOT_INT, 0);
	int augDropLess = player->InqIntQuality(AUGMENTATION_LESS_DEATH_ITEM_LOSS_INT, 0);
	int augCreatureFoci = player->InqIntQuality(AUGMENTATION_INFUSED_CREATURE_MAGIC_INT, 0);
	int augItemFoci = player->InqIntQuality(AUGMENTATION_INFUSED_ITEM_MAGIC_INT, 0);
	int augLifeFoci = player->InqIntQuality(AUGMENTATION_INFUSED_LIFE_MAGIC_INT, 0);
	int augWarFoci = player->InqIntQuality(AUGMENTATION_INFUSED_WAR_MAGIC_INT, 0);
	int augBuffDuration = player->InqIntQuality(AUGMENTATION_INCREASED_SPELL_DURATION_INT, 0);
	int augStickyBuffs = player->InqIntQuality(AUGMENTATION_SPELLS_REMAIN_PAST_DEATH_INT, 0);
	int augCritDefense = player->InqIntQuality(AUGMENTATION_CRITICAL_DEFENSE_INT, 0);
	int augBonusXp = player->InqIntQuality(AUGMENTATION_BONUS_XP_INT, 0);
	int augBonusSalvage = player->InqIntQuality(AUGMENTATION_BONUS_SALVAGE_INT, 0);
	int augBonusImbue = player->InqIntQuality(AUGMENTATION_BONUS_IMBUE_CHANCE_INT, 0);
	int augCritExpertise = player->InqIntQuality(AUGMENTATION_CRITICAL_EXPERTISE_INT, 0);
	int augCritPower = player->InqIntQuality(AUGMENTATION_CRITICAL_POWER_INT, 0);
	int augDamageBonus = player->InqIntQuality(AUGMENTATION_DAMAGE_BONUS_INT, 0);
	int augDamageReduction = player->InqIntQuality(AUGMENTATION_DAMAGE_REDUCTION_INT, 0);
	int augVoidFoci = player->InqIntQuality(AUGMENTATION_INFUSED_VOID_MAGIC_INT, 0);
	int augFasterRegen = player->InqIntQuality(AUGMENTATION_FASTER_REGEN_INT, 0);
	int augResistances = player->InqIntQuality(AUGMENTATION_RESISTANCE_FAMILY_INT, 0);
	int augResistBludge = player->InqIntQuality(AUGMENTATION_RESISTANCE_BLUNT_INT, 0);
	int augResistSlash = player->InqIntQuality(AUGMENTATION_RESISTANCE_SLASH_INT, 0);
	int augResistPierce = player->InqIntQuality(AUGMENTATION_RESISTANCE_PIERCE_INT, 0);
	int augResistAcid = player->InqIntQuality(AUGMENTATION_RESISTANCE_ACID_INT, 0);
	int augResistFire = player->InqIntQuality(AUGMENTATION_RESISTANCE_FIRE_INT, 0);
	int augResistFrost = player->InqIntQuality(AUGMENTATION_RESISTANCE_FROST_INT, 0);
	int augResistLightning = player->InqIntQuality(AUGMENTATION_RESISTANCE_LIGHTNING_INT, 0);

	switch (aug)
	{
	case 1:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute strength;
				player->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength);

				if (strength._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, min((int)strength._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(STRENGTH_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Reinforcement of the Lugians augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 2:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute endurance;
				player->m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance);

				if (endurance._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, min((int)endurance._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(ENDURANCE_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Bleeargh's Fortitude augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 3:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute coordination;
				player->m_Qualities.InqAttribute(COORDINATION_ATTRIBUTE, coordination);

				if (coordination._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, min((int)coordination._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(COORDINATION_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Oswald's Enhancement augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 4:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute quickness;
				player->m_Qualities.InqAttribute(QUICKNESS_ATTRIBUTE, quickness);

				if (quickness._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, min((int)quickness._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(QUICKNESS_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Siraluun's Blessing augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 5:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute focus;
				player->m_Qualities.InqAttribute(FOCUS_ATTRIBUTE, focus);

				if (focus._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, min((int)focus._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(FOCUS_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Enduring Calm augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 6:
		if (augInnates == 10)
		{
			player->NotifyWeenieError(WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES);
			break;
		}
		if (augInnates < 10)
		{
			if (unassignedXP >= augCost)
			{
				Attribute self;
				player->m_Qualities.InqAttribute(SELF_ATTRIBUTE, self);

				if (self._init_level == 100)
				{

					player->SendText("You are already at the maximum innate level.", LTT_DEFAULT);
					break;

				}
				else
				{
					player->m_Qualities.SetAttribute(SELF_ATTRIBUTE, min((int)self._init_level + 5, 100));
					player->m_Qualities.SetInt(AUGMENTATION_INNATE_FAMILY_INT, augInnates + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifyAttributeStatUpdated(SELF_ATTRIBUTE);

					player->EmitEffect(PS_AugmentationUseAttribute, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Steadfast Will augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 7:
		if (augSpecSalvage == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSpecSalvage == 0)
		{
			if (unassignedXP >= augCost)
			{
				Skill skill;
				player->m_Qualities.InqSkill(SALVAGING_SKILL, skill);
				if (skill._sac <= 1)
				{
					player->SendText("You are not trained in this skill!", LTT_DEFAULT);
					break;
				}

				else
				{
					skill.SetSkillAdvancementClass(SPECIALIZED_SKILL_ADVANCEMENT_CLASS);
					skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
					skill._init_level = 10;
					player->m_Qualities.SetSkill(SALVAGING_SKILL, skill);
					player->m_Qualities.SetInt(AUGMENTATION_SPECIALIZE_SALVAGING_INT, augSpecSalvage + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifySkillStatUpdated(SALVAGING_SKILL);

					player->EmitEffect(PS_AugmentationUseSkill, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Ciandra's Essence augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 8:
		if (augSpecItemTink == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSpecItemTink == 0)
		{
			if (unassignedXP >= augCost)
			{
				Skill skill;
				player->m_Qualities.InqSkill(ITEM_APPRAISAL_SKILL, skill);
				if (skill._sac <= 1)
				{
					player->SendText("You are not trained in this skill!", LTT_DEFAULT);
					break;
				}

				else
				{
					skill.SetSkillAdvancementClass(SPECIALIZED_SKILL_ADVANCEMENT_CLASS);
					skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
					skill._init_level = 10;
					player->m_Qualities.SetSkill(ITEM_APPRAISAL_SKILL, skill);
					player->m_Qualities.SetInt(AUGMENTATION_SPECIALIZE_ITEM_TINKERING_INT, augSpecItemTink + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifySkillStatUpdated(ITEM_APPRAISAL_SKILL);

					player->EmitEffect(PS_AugmentationUseSkill, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Yoshi's Essence augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 9:
		if (augSpecArmorTink == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSpecArmorTink == 0)
		{
			if (unassignedXP >= augCost)
			{
				Skill skill;
				player->m_Qualities.InqSkill(ARMOR_APPRAISAL_SKILL, skill);
				if (skill._sac <= 1)
				{
					player->SendText("You are not trained in this skill!", LTT_DEFAULT);
					break;
				}

				else
				{
					skill.SetSkillAdvancementClass(SPECIALIZED_SKILL_ADVANCEMENT_CLASS);
					skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
					skill._init_level = 10;
					player->m_Qualities.SetSkill(ARMOR_APPRAISAL_SKILL, skill);
					player->m_Qualities.SetInt(AUGMENTATION_SPECIALIZE_ARMOR_TINKERING_INT, augSpecArmorTink + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifySkillStatUpdated(ARMOR_APPRAISAL_SKILL);

					player->EmitEffect(PS_AugmentationUseSkill, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Jibril's Essence augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 10:
		if (augSpecMagicTink == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSpecMagicTink == 0)
		{
			if (unassignedXP >= augCost)
			{
				Skill skill;
				player->m_Qualities.InqSkill(MAGIC_ITEM_APPRAISAL_SKILL, skill);
				if (skill._sac <= 1)
				{
					player->SendText("You are not trained in this skill!", LTT_DEFAULT);
					break;
				}

				else
				{
					skill.SetSkillAdvancementClass(SPECIALIZED_SKILL_ADVANCEMENT_CLASS);
					skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
					skill._init_level = 10;
					player->m_Qualities.SetSkill(MAGIC_ITEM_APPRAISAL_SKILL, skill);
					player->m_Qualities.SetInt(AUGMENTATION_SPECIALIZE_MAGIC_ITEM_TINKERING_INT, augSpecMagicTink + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifySkillStatUpdated(MAGIC_ITEM_APPRAISAL_SKILL);

					player->EmitEffect(PS_AugmentationUseSkill, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Celdiseth's Essence augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 11:
		if (augSpecWeaponTink == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSpecWeaponTink == 0)
		{
			if (unassignedXP >= augCost)
			{
				Skill skill;
				player->m_Qualities.InqSkill(WEAPON_APPRAISAL_SKILL, skill);
				if (skill._sac <= 1)
				{
					player->SendText("You are not trained in this skill!", LTT_DEFAULT);
					break;
				}

				else
				{
					skill.SetSkillAdvancementClass(SPECIALIZED_SKILL_ADVANCEMENT_CLASS);
					skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
					skill._init_level = 10;
					player->m_Qualities.SetSkill(WEAPON_APPRAISAL_SKILL, skill);
					player->m_Qualities.SetInt(AUGMENTATION_SPECIALIZE_WEAPON_TINKERING_INT, augSpecWeaponTink + 1);
					player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
					player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
					player->NotifySkillStatUpdated(WEAPON_APPRAISAL_SKILL);

					player->EmitEffect(PS_AugmentationUseSkill, 1.0f);
					player->SendText("Congratulations! You have succeeded in acquiring the Koga's Essence augmentation.", LTT_DEFAULT);
					std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
					if (!text.empty())
					{
						g_pWorld->BroadcastLocal(player->GetLandcell(), text);
					}

					DecrementStackOrStructureNum();
					break;
				}
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 12:
		if (augExtraPack == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augExtraPack == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_EXTRA_PACK_SLOT_INT, augExtraPack + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->m_Qualities.SetInt(CONTAINERS_CAPACITY_INT, 8);
				player->NotifyIntStatUpdated(CONTAINERS_CAPACITY_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Shadow of The Seventh Mule augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 13:
		if (augIncreasedBurden == 5)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augIncreasedBurden < 5)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INCREASED_CARRYING_CAPACITY_INT, augIncreasedBurden + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INCREASED_CARRYING_CAPACITY_INT);
				player->NotifyIntStatUpdated(ENCUMB_CAPACITY_INT);
				player->CPlayerWeenie::RecalculateEncumbrance();
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Might of The Seventh Mule augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 14:
		if (augDropLess == 3)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augDropLess < 3)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_LESS_DEATH_ITEM_LOSS_INT, augDropLess + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_LESS_DEATH_ITEM_LOSS_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Clutch of the Miser augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 15:
		if (augStickyBuffs == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augStickyBuffs == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_SPELLS_REMAIN_PAST_DEATH_INT, augStickyBuffs + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_SPELLS_REMAIN_PAST_DEATH_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Enduring Enchantment augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 16:
		if (augCritDefense == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augCritDefense == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_CRITICAL_DEFENSE_INT, augCritDefense + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_CRITICAL_DEFENSE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Critical Protection augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 17:
		if (augBonusXp == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augBonusXp == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_BONUS_XP_INT, augBonusXp + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_BONUS_XP_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Quick Learner augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 18:
		if (augBonusSalvage == 4)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augBonusSalvage < 4)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_BONUS_SALVAGE_INT, augBonusSalvage + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_BONUS_SALVAGE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Ciandra's Fortune augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 19:
		if (augBonusImbue == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augBonusImbue == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_BONUS_IMBUE_CHANCE_INT, augBonusImbue + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_BONUS_IMBUE_CHANCE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Charmed Smith augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 20:
		if (augFasterRegen == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augFasterRegen < 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_FASTER_REGEN_INT, augFasterRegen + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_FASTER_REGEN_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Innate Renewal augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 21:
		if (augBuffDuration == 5)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augBuffDuration < 5)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INCREASED_SPELL_DURATION_INT, augBuffDuration + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INCREASED_SPELL_DURATION_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Archmage's Endurance augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 22:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistSlash <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_SLASH_INT, augResistSlash + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_SLASH_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Enhancement of the Blade Turner augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 23:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistPierce <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_PIERCE_INT, augResistPierce + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_PIERCE_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Enhancement of the Arrow Turner augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 24:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistBludge <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_BLUNT_INT, augResistBludge + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_BLUNT_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Enhancement of the Mace Turner augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 25:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistAcid <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_ACID_INT, augResistAcid + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_ACID_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Caustic Enhancement augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 26:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistFire <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FIRE_INT, augResistFire + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_FIRE_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Fiery Enhancement augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 27:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistFrost <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FROST_INT, augResistFrost + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_FROST_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Icy Enhancement augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 28:
		if (augResistances == 2)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augResistLightning <= 2)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_LIGHTNING_INT, augResistLightning + 1);
				player->m_Qualities.SetInt(AUGMENTATION_RESISTANCE_FAMILY_INT, augResistances + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_RESISTANCE_LIGHTNING_INT);
				player->EmitEffect(PS_AugmentationUseResistances, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Storm's Enhancement augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
		break;
	case 29:
		if (augCreatureFoci == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augCreatureFoci == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INFUSED_CREATURE_MAGIC_INT, augCreatureFoci + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INFUSED_CREATURE_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Infused Creature Magic augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 30:
		if (augItemFoci == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augItemFoci == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INFUSED_ITEM_MAGIC_INT, augItemFoci + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INFUSED_ITEM_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Infused Item Magic augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 31:
		if (augLifeFoci == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augLifeFoci == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INFUSED_LIFE_MAGIC_INT, augLifeFoci + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INFUSED_LIFE_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Infused Life Magic augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 32:
		if (augWarFoci == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augWarFoci == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INFUSED_WAR_MAGIC_INT, augWarFoci + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INFUSED_WAR_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Infused War Magic augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 33:
		if (augCritExpertise == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augCritExpertise == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_CRITICAL_EXPERTISE_INT, augCritExpertise + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_CRITICAL_EXPERTISE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Eye of the Remorseless augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 34:
		if (augCritPower == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augCritPower == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_CRITICAL_POWER_INT, augCritPower + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_CRITICAL_POWER_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Hand of the Remorseless augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 35:
		if (augSkilledMelee == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSkilledMelee == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_SKILLED_MELEE_INT, augSkilledMelee + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_SKILLED_MELEE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Master of The Steel Circle augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 36:
		if (augSkilledMissile == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSkilledMissile == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_SKILLED_MISSILE_INT, augSkilledMissile + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_SKILLED_MISSILE_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Master of the Focused Eye augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 37:
		if (augSkilledMagic == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augSkilledMagic == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_SKILLED_MAGIC_INT, augSkilledMagic + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_SKILLED_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Master of The Five Fold Path augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 38:
		if (augDamageBonus == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augDamageBonus == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_DAMAGE_BONUS_INT, augDamageBonus + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_DAMAGE_BONUS_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Frenzy of the Slayer augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 39:
		if (augDamageReduction == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augDamageReduction == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_DAMAGE_REDUCTION_INT, augDamageReduction + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_DAMAGE_REDUCTION_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Iron Skin of the Invincible augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 40:
		if (augJackOfTrades == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augJackOfTrades == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_JACK_OF_ALL_TRADES_INT, augJackOfTrades + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_JACK_OF_ALL_TRADES_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Jack of All Trades augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	case 42:
		if (augVoidFoci == 1)
		{
			player->SendText("This augmentation is already active.", LTT_DEFAULT);
			break;
		}
		if (augVoidFoci == 0)
		{
			if (unassignedXP >= augCost)
			{
				player->m_Qualities.SetInt(AUGMENTATION_INFUSED_VOID_MAGIC_INT, augVoidFoci + 1);
				player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, unassignedXP - augCost);
				player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
				player->NotifyIntStatUpdated(AUGMENTATION_INFUSED_VOID_MAGIC_INT);
				player->EmitEffect(PS_AugmentationUseOther, 1.0f);
				player->SendText("Congratulations! You have succeeded in acquiring the Infused Void Magic augmentation.", LTT_DEFAULT);
				std::string text = csprintf("%s has acquired the %s augmentation!", player->GetName().c_str(), GetName().c_str());
				if (!text.empty())
				{
					g_pWorld->BroadcastLocal(player->GetLandcell(), text);
				}

				DecrementStackOrStructureNum();
				break;
			}
			else
				player->NotifyWeenieError(WERROR_AUGMENTATION_NOT_ENOUGH_XP);
			break;
		}
	default:
		player->SendText("This Augmentation is not supported at this time!", LTT_DEFAULT);
		break;
	}


	player->NotifyUseDone(WERROR_NONE);
	return WERROR_NONE;

}
