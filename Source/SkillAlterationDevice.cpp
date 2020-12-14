
#include <StdAfx.h>
#include "SkillAlterationDevice.h"
#include "UseManager.h"
#include "Player.h"
#include "ObjCache.h"

CSkillAlterationDeviceWeenie::CSkillAlterationDeviceWeenie()
{
}

CSkillAlterationDeviceWeenie::~CSkillAlterationDeviceWeenie()
{
}

int CSkillAlterationDeviceWeenie::Use(CPlayerWeenie *player)
{
	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	// 1 = raise
	// 2 = lower
	int alterationType = InqIntQuality(TYPE_OF_ALTERATION_INT, 0);
	STypeSkill skillToAlter = (STypeSkill)InqIntQuality(SKILL_TO_BE_ALTERED_INT, STypeSkill::UNDEF_SKILL);

	if (alterationType <= 0 || alterationType > 2 || skillToAlter <= 0 || skillToAlter > NUM_SKILL)
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	// 1 = specialize ?

	Skill skill;
	if (!player->m_Qualities.InqSkill(skillToAlter, skill))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	int numSkillCredits = player->InqIntQuality(AVAILABLE_SKILL_CREDITS_INT, 0, TRUE);

	switch (alterationType)
	{
	case 1: // raise
		{
			switch (skill._sac)
			{
			default:
				player->SendText("You may only use this if the skill is trained.", LTT_DEFAULT);
				break;

			case TRAINED_SKILL_ADVANCEMENT_CLASS:
				{
					SkillTable *pSkillTable = SkillSystem::GetSkillTable();
					const SkillBase *pSkillBase = pSkillTable->GetSkillBase(skillToAlter);
					if (pSkillBase != NULL)
					{
						numSkillCredits += pSkillBase->_trained_cost;
						if (numSkillCredits < pSkillBase->_specialized_cost)
						{
							player->SendText(csprintf("You need %d credits to specialize this skill.", pSkillBase->_specialized_cost - pSkillBase->_trained_cost), LTT_DEFAULT);
						}
						else
						{

							int speccCount = 0;
							for (PackableHashTableWithJson<STypeSkill, Skill>::iterator i = player->m_Qualities._skillStatsTable->begin(); i != player->m_Qualities._skillStatsTable->end(); i++)
							{
								if (i->second._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
								{	
									// Salvaging and tinkering skills all have > 999 spec cost.
									if (pSkillTable->GetSkillBase(i->first)->_specialized_cost >= 999)
										continue;

									//Arcane technically costs 4 credits to train even though you can't unspec it. Should only count as 2 toward number of spec credits.
									if (i->first != ARCANE_LORE_SKILL) 
										speccCount += (pSkillTable->GetSkillBase(i->first)->_specialized_cost);
									else
										speccCount += (pSkillTable->GetSkillBase(i->first)->_specialized_cost - pSkillTable->GetSkillBase(i->first)->_trained_cost);
								}
							}
							if (speccCount + pSkillBase->_specialized_cost > 70)
							{
								if (skillToAlter != ARCANE_LORE_SKILL)
								{
									player->SendText("Unable to specialize this skill.", LTT_DEFAULT);
									break;
								}
								else if (speccCount + (pSkillBase->_specialized_cost - pSkillBase->_trained_cost) > 70)
								{
									player->SendText("Unable to specialize this skill.", LTT_DEFAULT);
									break;
								}
							}

							numSkillCredits -= pSkillBase->_specialized_cost;
							player->m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, numSkillCredits);
							player->NotifyIntStatUpdated(AVAILABLE_SKILL_CREDITS_INT);

							skill._sac = SPECIALIZED_SKILL_ADVANCEMENT_CLASS;
							skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
							skill._init_level = 10;
							player->m_Qualities.SetSkill(skillToAlter, skill);
							player->NotifySkillStatUpdated(skillToAlter);
							player->SendText(csprintf("You are now specialized in %s!", pSkillBase->_name.c_str()), LTT_ADVANCEMENT);
							player->EmitSound(Sound_RaiseTrait, 1.0, true);

							DecrementStackOrStructureNum();
						}
					}
					else
					{
						player->SendText("Cannot raise or lower this skill.", LTT_DEFAULT);
					}
					break;
				}
			}

			break;
		}
	case 2: // lower
		{
			switch (skill._sac)
			{
			default:
				player->SendText("You may only use this if the skill is trained or specialized.", LTT_DEFAULT);
				break;

			case SPECIALIZED_SKILL_ADVANCEMENT_CLASS:
				{
					SkillTable *pSkillTable = SkillSystem::GetSkillTable();
					const SkillBase *pSkillBase = pSkillTable->GetSkillBase(skillToAlter);
					if (pSkillBase != NULL)
					{
						bool isTinker = (skillToAlter == SALVAGING_SKILL ||
							skillToAlter == WEAPON_APPRAISAL_SKILL ||
							skillToAlter == ARMOR_APPRAISAL_SKILL ||
							skillToAlter == MAGIC_ITEM_APPRAISAL_SKILL ||
							skillToAlter == ITEM_APPRAISAL_SKILL);

						//Per Wiki, using gems of forgetfullness on specialized tinkering skills only returned XP and did not unspecialize the skill.
						//Salvaging is NEVER unspec'd once spec'd (no credit skill - just xp).

						if (!isTinker)
						{   
							numSkillCredits += (pSkillBase->_specialized_cost - pSkillBase->_trained_cost);
							player->m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, numSkillCredits);
							player->NotifyIntStatUpdated(AVAILABLE_SKILL_CREDITS_INT);
						}

						uint64_t xpToAward = 0;

						xpToAward = skill._pp;
						if (isTinker) 
						{
							skill._init_level = 10;
							skill._pp = 0;
							skill._level_from_pp = 5;
						}
						else
						{
							skill._sac = TRAINED_SKILL_ADVANCEMENT_CLASS;
							skill._pp = 0;
							skill._init_level = 0;
							skill._level_from_pp = 5;
						}


						player->m_Qualities.SetSkill(skillToAlter, skill);
						player->NotifySkillStatUpdated(skillToAlter);

						if (xpToAward > 0)
						{
							player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, player->InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + xpToAward);
							player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
						}
						if (isTinker)
							player->SendText(csprintf("Cannot raise or lower %s. All the experience that you spent on this skill have been refunded to you.", pSkillBase->_name.c_str()), LTT_ADVANCEMENT);
						else
							player->SendText(csprintf("You are no longer specialized in %s!", pSkillBase->_name.c_str()), LTT_ADVANCEMENT);

						player->EmitSound(Sound_RaiseTrait, 1.0, true);

						DecrementStackOrStructureNum();
					}
					else
					{
						player->SendText("Cannot raise or lower this skill.", LTT_DEFAULT);
					}
					break;
				}

			case TRAINED_SKILL_ADVANCEMENT_CLASS:
				{
					SkillTable *pSkillTable = SkillSystem::GetSkillTable();
					const SkillBase *pSkillBase = pSkillTable->GetSkillBase(skillToAlter);

					if (pSkillBase != NULL)
					{

						HeritageGroup_CG *heritageGroup = CachedCharGenData->mHeritageGroupList.lookup(player->InqIntQuality(HERITAGE_GROUP_INT, 0, true));
						if (heritageGroup)
						{
							//first we check our heritage specific skills as we cannot untrain those.
							for (uint32_t i = 0; i < heritageGroup->mSkillList.num_used; i++)
							{
								if (heritageGroup->mSkillList.array_data[i].skillNum == skillToAlter)
								{
									uint64_t xpToAward = skill._pp;
									skill._pp = 0;
									skill._level_from_pp = 5;
									skill._init_level = 0;
									player->m_Qualities.SetSkill(skillToAlter, skill);
									player->NotifySkillStatUpdated(skillToAlter);

									if (xpToAward > 0)
									{
										player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, player->InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + xpToAward);
										player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
									}

									player->SendText(csprintf("You refund the experience you had spent in %s!", pSkillBase->_name.c_str()), LTT_DEFAULT);
									player->EmitSound(Sound_RaiseTrait, 1.0, true);

									DecrementStackOrStructureNum();

									player->NotifyUseDone(WERROR_NONE);
									return WERROR_NONE;
								}
							}
						}

						if (pSkillBase->_trained_cost > 0)
						{
							numSkillCredits += pSkillBase->_trained_cost;
							player->m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, numSkillCredits);
							player->NotifyIntStatUpdated(AVAILABLE_SKILL_CREDITS_INT);

							uint64_t xpToAward = skill._pp;
							skill._pp = 0;

							skill._sac = UNTRAINED_SKILL_ADVANCEMENT_CLASS;
							skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);
							skill._init_level = 0;
							player->m_Qualities.SetSkill(skillToAlter, skill);
							player->NotifySkillStatUpdated(skillToAlter);

							if (xpToAward > 0)
							{
								player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, player->InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + xpToAward);
								player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
							}

							player->SendText(csprintf("You are no longer trained in %s!", pSkillBase->_name.c_str()), LTT_ADVANCEMENT);
							player->EmitSound(Sound_RaiseTrait, 1.0, true);

							DecrementStackOrStructureNum();
						}
						else
						{
							//player->SendText(csprintf("You cannot untrain %s!", pSkillBase->_name.c_str()), LTT_DEFAULT);
							uint64_t xpToAward = skill._pp;
							skill._pp = 0;
							skill._level_from_pp = 5;
							skill._init_level = 0;
							player->m_Qualities.SetSkill(skillToAlter, skill);
							player->NotifySkillStatUpdated(skillToAlter);

							if (xpToAward > 0)
							{
								player->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, player->InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + xpToAward);
								player->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
							}

							player->SendText(csprintf("You refund the experience you had spent in %s!", pSkillBase->_name.c_str()), LTT_DEFAULT);
							player->EmitSound(Sound_RaiseTrait, 1.0, true);

							DecrementStackOrStructureNum();
						}
					}
					else
					{
						player->SendText("Cannot raise or lower this skill.", LTT_DEFAULT);
					}
					break;
				}
			}

			break;
		}
	}

	player->NotifyUseDone(WERROR_NONE);
	return WERROR_NONE;
}
