
#include "StdAfx.h"
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
	DWORD healing_skill = 0;
	if (!player->InqSkill(HEALING_SKILL, healing_skill, TRUE) || !healing_skill)
	{
		player->NotifyWeenieError(WERROR_HEAL_NOT_TRAINED);
		player->NotifyUseDone(0);
		return WERROR_NONE;
	}

	CHealerUseEvent *useEvent = new CHealerUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 1.0;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

void CHealerUseEvent::OnReadyToUse()
{
	if (_target_id == _weenie->GetID())
	{
		ExecuteUseAnimation(Motion_SkillHealSelf);
	}
	else
	{
		// TODO find the right animation for healing another player
		ExecuteUseAnimation(Motion_SkillHealSelf);
	}
}

void CHealerUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();
	CWeenieObject *tool = GetTool();

	if (tool && target && !target->IsDead() && !target->IsInPortalSpace())
	{
		int amountHealed = 0;
		int usesLeft = tool->InqIntQuality(STRUCTURE_INT, -1);
		
		if (usesLeft != 0)
		{
			if (usesLeft > 0)
				usesLeft--;

			const char *prefix = "";				
			const char *vitalName = "";

			DWORD boost_stat = tool->InqIntQuality(BOOSTER_ENUM_INT, 0);
			DWORD boost_value = tool->InqIntQuality(BOOST_VALUE_INT, 0);
			bool success = false;

			switch (boost_stat)
			{
			case HEALTH_ATTRIBUTE_2ND:
			case STAMINA_ATTRIBUTE_2ND:
			case MANA_ATTRIBUTE_2ND:
				{
					STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
					STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

					DWORD statValue = 0, maxStatValue = 0;
					target->m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
					target->m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);

					int missingVital = max(0, (int)maxStatValue - (int)statValue);
					int difficulty = max(0, (missingVital * 2) - tool->InqIntQuality(BOOST_VALUE_INT, 0));

					DWORD healing_skill = 0;
					_weenie->InqSkill(HEALING_SKILL, healing_skill, FALSE);

					// this is wrong, but whatever we'll fake it
					DWORD heal_min = (int)(healing_skill * 0.2) * tool->InqFloatQuality(HEALKIT_MOD_FLOAT, 1.0);
					DWORD heal_max = (int)(healing_skill * 0.5) * tool->InqFloatQuality(HEALKIT_MOD_FLOAT, 1.0);

					amountHealed = Random::GenInt(heal_min, heal_max);
					if (amountHealed > missingVital)
						amountHealed = missingVital;

					int staminaNecessary = amountHealed / 5; //1 point of stamina used for every 5 health healed.

					if (_weenie->GetStamina() < staminaNecessary)
					{
						//can't heal if there's not enough stamina
						_weenie->NotifyWeenieError(WERROR_STAMINA_TOO_LOW);
						Cancel();
						return;
					}
					_weenie->AdjustStamina(-staminaNecessary);

					success = Random::RollDice(0.0, 1.0) <= GetSkillChance(healing_skill, difficulty);
					if (success)
					{
						if (amountHealed > (int)(heal_max * 0.8))
							prefix = "expertly ";

						DWORD newStatValue = min(statValue + amountHealed, maxStatValue);

						int statChange = newStatValue - statValue;
						if (statChange)
						{
							target->m_Qualities.SetAttribute2nd(statType, newStatValue);
							target->NotifyAttribute2ndStatUpdated(statType);
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
					if (usesLeft >= 0)
						_weenie->SendText(csprintf("You %sheal yourself for %d %s points. Your %s has %u uses left.", prefix, amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
					else
						_weenie->SendText(csprintf("You %sheal yourself for %d %s points with %s.", prefix, amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);
				}
				else
				{
					if (usesLeft >= 0)
						_weenie->SendText(csprintf("You %sheal %s for %d %s points. Your %s has %u uses left.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
					else
						_weenie->SendText(csprintf("You %sheal %s for %d %s points with %s.", prefix, target->GetName().c_str(), amountHealed, vitalName, tool->GetName().c_str()), LTT_DEFAULT);

					target->SendText(csprintf("%s heals you for %d %s points.", _weenie->GetName().c_str(), amountHealed, vitalName), LTT_DEFAULT);
				}
			}
			else
			{
				if (_target_id == _weenie->GetID())
				{
					_weenie->SendText(csprintf("You fail to heal yourself. Your %s has %d uses left.", tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
				}
				else
				{
					_weenie->SendText(csprintf("You fail to heal %s. Your %s has %d uses left.", target->GetName().c_str(), tool->GetName().c_str(), usesLeft), LTT_DEFAULT);
					target->SendText(csprintf("%s fails to heal you.", _weenie->GetName().c_str()), LTT_DEFAULT);
				}
			}

			tool->DecrementStackOrStructureNum(true);
		}
	}

	Done();
}
