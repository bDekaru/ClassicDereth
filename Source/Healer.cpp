#include <StdAfx.h>
#include "WeenieObject.h"
#include "Healer.h"
#include "Player.h"

CHealerWeenie::CHealerWeenie()
{
	SetName("Healer");
	m_Qualities.m_WeenieType = Healer_WeenieType;
}

CHealerWeenie::~CHealerWeenie()
{
}

void CHealerWeenie::ApplyQualityOverrides()
{
}

int CHealerWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	if (!with->AsPlayer())
	{
		player->NotifyWeenieError(WERROR_HEAL_PLAYERS_ONLY);
		player->NotifyUseDone(0);
		return WERROR_NONE;
	}

	if (player->IsInPortalSpace())
	{
		player->NotifyUseDone(0);
		return WERROR_ACTIONS_LOCKED;
	}

	uint32_t healing_skill = 0;
	if (!player->InqSkill(HEALING_SKILL, healing_skill, TRUE) || !healing_skill)
	{
		player->NotifyWeenieError(WERROR_HEAL_NOT_TRAINED);
		player->NotifyUseDone(0);
		return WERROR_NONE;
	}

	uint32_t boostEnum = m_Qualities.GetInt(BOOSTER_ENUM_INT, 0);

	switch (boostEnum)
	{
	case HEALTH_ATTRIBUTE_2ND:
	{
		if (with->GetHealth() == with->GetMaxHealth())
		{
			player->NotifyWeenieErrorWithString(WERROR_HEAL_FULL_HEALTH, with->GetName().c_str());
			player->NotifyUseDone(0);
			return WERROR_NONE;
		}
		else
			break;
	}
	case STAMINA_ATTRIBUTE_2ND:
	{
		if (with->GetStamina() == with->GetMaxStamina())
		{
			player->NotifyWeenieError(WERROR_NONE);
			player->NotifyUseDone(0);
			return WERROR_NONE;
		}
		else
			break;
	}
	case MANA_ATTRIBUTE_2ND:
	{
		if (with->GetMana() == with->GetMaxMana())
		{
			player->NotifyWeenieError(WERROR_NONE);
			player->NotifyUseDone(0);
			return WERROR_NONE;
		}
		else
			break;
	}
	}

	CHealerUseEvent *useEvent = new CHealerUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 1.0;
	useEvent->_initial_use_position = player->m_Position;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

void CHealerUseEvent::OnReadyToUse()
{
	if (_weenie->GetStamina() == 0)
	{
		//don't perform healing animation if there's zero stamina
		_weenie->NotifyWeenieError(WERROR_STAMINA_TOO_LOW);
		Cancel();
		return;
	}

	if (_target_id == _weenie->GetID())
	{
		_weenie->DoForcedStopCompletely();
		ExecuteUseAnimation(Motion_SkillHealSelf);
	}
	else
	{
		_weenie->DoForcedStopCompletely();
		ExecuteUseAnimation(Motion_Woah);
	}
}

void CHealerUseEvent::OnUseAnimSuccess(uint32_t motion)
{
	CWeenieObject *target = GetTarget();
	CWeenieObject *tool = GetTool();

	if (tool && target && !target->IsDead() && !target->IsInPortalSpace())
	{
		if (_target_id == _weenie->GetID() && _weenie->m_Position.distance(_initial_use_position) > 5.0) //Distance check between initial use and now but only if heal was executed on self.
			return Cancel(WERROR_MOVED_TOO_FAR);

		int amountHealed = 0;
		int usesLeft = tool->InqIntQuality(STRUCTURE_INT, -1);
		
		if (usesLeft != 0)
		{
			if (usesLeft > 0)
				usesLeft--;

			const char *prefix = "";				
			const char *vitalName = "";

			uint32_t boost_stat = tool->InqIntQuality(BOOSTER_ENUM_INT, 0);
			uint32_t boost_value = tool->InqIntQuality(BOOST_VALUE_INT, 0);
			bool success = false;

			switch (boost_stat)
			{
			case HEALTH_ATTRIBUTE_2ND:
			case STAMINA_ATTRIBUTE_2ND:
			case MANA_ATTRIBUTE_2ND:
				{
					STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
					STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

					uint32_t statValue = 0, maxStatValue = 0;
					target->m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
					target->m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);

					int missingVital = max(0, (int)maxStatValue - (int)statValue);
					double combat_mod = _weenie->IsInPeaceMode() ? 1.0 : 1.1;
					int difficulty = max(0, (int)((missingVital * 2) * combat_mod));

					Skill skill;
					uint32_t healing_skill = 0;
					_weenie->InqSkill(HEALING_SKILL, healing_skill, FALSE);
					_weenie->m_Qualities.InqSkill(HEALING_SKILL, skill);
					uint32_t base_skill = healing_skill;

					double sac_mod = skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS ? 1.5 : 1.1;

					healing_skill = (healing_skill + boost_value) * sac_mod;
					
					success = Random::RollDice(0.0, 1.0) <= GetSkillChance(healing_skill, difficulty);
					CPlayerWeenie* player = _weenie->AsPlayer();
					if (success)
					{
						double heal_min = 0;
						double heal_max = 0;

						if(player)
							player->MaybeGiveSkillUsageXP(HEALING_SKILL, difficulty);

						if (skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
						{
							heal_min = 0.09;
							heal_max = 0.28;
						}
						else
						{
							heal_min = 0.07;
							heal_max = 0.235;
						}

						double healedVariance = Random::GenFloat(heal_min, heal_max);
					
						amountHealed = (int)(base_skill * healedVariance * tool->InqFloatQuality(HEALKIT_MOD_FLOAT, 1.0));
						
						if (Random::RollDice(0.0, 1.0) <= 0.15)
						{
							prefix = "expertly ";
							amountHealed *= difficulty / 2;
						}

						if (amountHealed > missingVital)
							amountHealed = missingVital;

						int staminaNecessary = min(1, (int)round(amountHealed / 5.0)); //1 point of stamina used for every 5 health healed.

						if (_weenie->GetStamina() < staminaNecessary)
						{
							staminaNecessary = _weenie->GetStamina();
							amountHealed = staminaNecessary * 5;
							prefix = ""; //no exhausted experts
							if (player)
							{
								player->SendText("You're exhausted!", LTT_ERROR);
							}
						}
						_weenie->AdjustStamina(-staminaNecessary);

						uint32_t newStatValue = min(statValue + amountHealed, maxStatValue);

						int statChange = newStatValue - statValue;
						if (statChange)
						{
							if (target->AsPlayer() && statType == HEALTH_ATTRIBUTE_2ND)
							{
								target->AdjustHealth(statChange);
								target->NotifyAttribute2ndStatUpdated(statType);
							}
							else
							{
								target->m_Qualities.SetAttribute2nd(statType, newStatValue);
								target->NotifyAttribute2ndStatUpdated(statType);
							}

						}
					} else {
						//Still use 1 stam for failure.
						int amountDecreased = _weenie->AdjustStamina(-1);
						if (amountDecreased == 0 && player) {
							player->SendText("You're exhausted!", LTT_ERROR);
						}
					}

					switch (boost_stat)
					{
					case HEALTH_ATTRIBUTE_2ND: vitalName = "Health"; break;
					case STAMINA_ATTRIBUTE_2ND: vitalName = "Stamina"; break;
					case MANA_ATTRIBUTE_2ND: vitalName = "Mana"; break;
					}

					break;
				}
			}

			amountHealed = max(0, amountHealed);

			// heal up
			if (success)
			{
				if (_target_id == _weenie->GetID())
				{
					if (vitalName == "Health")
					{
						if (usesLeft >= 0)
							_weenie->SendText(csprintf("You %sheal yourself for %d %s points. Your %s has %u uses left.", prefix, amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						else
							_weenie->SendText(csprintf("You %sheal yourself for %d %s points with %s.", prefix, amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);
					}
					else
					{
						if (usesLeft >= 0)
							_weenie->SendText(csprintf("You %srecover %d %s points. Your %s has %u uses left.", prefix, amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						else
							_weenie->SendText(csprintf("You %srecover %d %s points with %s.", prefix, amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);
					}
				}
				else
				{
					if (vitalName == "Health")
					{
						if (usesLeft >= 0)
							_weenie->SendText(csprintf("You %sheal %s for %d %s points. Your %s has %u uses left.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						else
							_weenie->SendText(csprintf("You %sheal %s for %d %s points with %s.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);

						target->SendText(csprintf("%s heals you for %d %s points.", _weenie->GetName().c_str(), amountHealed, vitalName), LTT_DEFAULT);
					}
					else
					{
						if (usesLeft >= 0)
							_weenie->SendText(csprintf("You %srestore %s for %d %s points. Your %s has %u uses left.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						else
							_weenie->SendText(csprintf("You %srestore %s for %d %s points with %s.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);

						target->SendText(csprintf("%s restores you for %d %s points.", _weenie->GetName().c_str(), amountHealed, vitalName), LTT_DEFAULT);
					}

				}

				if (boost_stat == HEALTH_ATTRIBUTE_2ND)
				{
					if (_weenie->AsPlayer())
					{
						// update the target's health on the healing player asap
						((CPlayerWeenie*)_weenie)->RefreshTargetHealth();
					}
				}
			}
			else
			{
				if (_target_id == _weenie->GetID())
				{
					if(vitalName == "Health")
						_weenie->SendText(csprintf("You fail to heal yourself. Your %s has %d uses left.", tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
					else
						_weenie->SendText(csprintf("You fail to recover any vitals. Your %s has %d uses left.", tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
				}
				else
				{
					if (vitalName == "Health")
					{
						_weenie->SendText(csprintf("You fail to heal %s. Your %s has %d uses left.", target->GetName().c_str(), tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						target->SendText(csprintf("%s fails to heal you.", _weenie->GetName().c_str()), LTT_DEFAULT);
					}
					else
					{
						_weenie->SendText(csprintf("You fail to restore any vitals to %s. Your %s has %d uses left.", target->GetName().c_str(), tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
						target->SendText(csprintf("%s fails to restore any of your vitals.", _weenie->GetName().c_str()), LTT_DEFAULT);
					}
				}
			}

			tool->DecrementStackOrStructureNum(true);
		}
	}

	Done();
}
