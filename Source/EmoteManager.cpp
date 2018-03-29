
#include "StdAfx.h"
#include "EmoteManager.h"
#include "WeenieObject.h"
#include "ChatMsgs.h"
#include "World.h"
#include "WeenieFactory.h"
#include "Player.h"
#include "SpellcastingManager.h"
#include "Config.h"
#include "GameEventManager.h"
#include "MonsterAI.h"

EmoteManager::EmoteManager(CWeenieObject *weenie)
{
	_weenie = weenie;
}

bool EmoteManager::ChanceExecuteEmoteSet(EmoteCategory category, std::string msg, DWORD target_id)
{
	PackableList<EmoteSet> *emoteCategory = _weenie->m_Qualities._emote_table->_emote_table.lookup(category);

	if (!emoteCategory)
		return false;

	float diceRoll = Random::RollDice(0.0, 1.0);

	for (auto &entry : *emoteCategory)
	{
		if (!_stricmp(entry.quest.c_str(), msg.c_str()) && diceRoll < entry.probability)
		{
			ExecuteEmoteSet(entry, target_id);
			return true;
		}
	}
	return false;
}

bool EmoteManager::ChanceExecuteEmoteSet(EmoteCategory category, DWORD target_id)
{
	PackableList<EmoteSet> *emoteCategory = _weenie->m_Qualities._emote_table->_emote_table.lookup(category);

	if (!emoteCategory)
		return false;

	float diceRoll = Random::RollDice(0.0, 1.0);

	for (auto &entry : *emoteCategory)
	{
		if (diceRoll < entry.probability)
		{
			ExecuteEmoteSet(entry, target_id);
			return true;
		}
	}
	return false;
}

void EmoteManager::ExecuteEmoteSet(const EmoteSet &emoteSet, DWORD target_id)
{
	if (_emoteEndTime < Timer::cur_time)
		_emoteEndTime = Timer::cur_time;

	double totalEmoteSetTime = 0.0;
	for (auto &emote : emoteSet.emotes)
	{
		totalEmoteSetTime += emote.delay;

		QueuedEmote qe;
		qe._data = emote;
		qe._target_id = target_id;
		qe._executeTime = Timer::cur_time + totalEmoteSetTime;
		_emoteQueue.push_back(qe);
	}
}

std::string EmoteManager::ReplaceEmoteText(const std::string &text, DWORD target_id, DWORD source_id)
{
	std::string result = text;

	if (result.find("%s") != std::string::npos)
	{
		std::string targetName;
		if (!g_pWorld->FindObjectName(target_id, targetName))
			return ""; // Couldn't resolve name, don't display this message.

		while (ReplaceString(result, "%s", targetName));
	}

	if (result.find("%tn") != std::string::npos)
	{
		std::string targetName;
		if (!g_pWorld->FindObjectName(target_id, targetName))
			return ""; // Couldn't resolve name, don't display this message.

		while (ReplaceString(result, "%tn", targetName));
	}

	if (result.find("%n") != std::string::npos)
	{
		std::string sourceName;
		if (!g_pWorld->FindObjectName(source_id, sourceName))
			return ""; // Couldn't resolve name, don't display this message.

		while (ReplaceString(result, "%n", sourceName));
	}

	if (result.find("%mn") != std::string::npos)
	{
		std::string sourceName;
		if (!g_pWorld->FindObjectName(source_id, sourceName))
			return ""; // Couldn't resolve name, don't display this message.

		while (ReplaceString(result, "%mn", sourceName));
	}

	if (result.find("%tqt") != std::string::npos)
	{
		while (ReplaceString(result, "%tqt", "some amount of time"));
	}

	return result;
}

void EmoteManager::ExecuteEmote(const Emote &emote, DWORD target_id)
{
	switch (emote.type)
	{
	default:
		{
#ifndef PUBLIC_BUILD
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target || !target->IsAdmin())
				break;

			target->SendText(csprintf("Unhandled emote %s (%u)", Emote::EmoteTypeToName(emote.type), emote.type), LTT_DEFAULT);
#endif
			break;
		}
	case Act_EmoteType:
	{
		std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

		if (!text.empty())
		{
			BinaryWriter *textMsg = ServerText(text.c_str(), LTT_EMOTE);

			std::list<CWeenieObject *> results;
			g_pWorld->EnumNearbyPlayers(_weenie->GetPosition(), 30.0f, &results);

			for (auto target : results)
			{
				if (target == _weenie)
					continue;

				target->SendNetMessage(textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0);
			}
			delete textMsg;
		}

		break;
	}
	case LocalBroadcast_EmoteType:
		{
			std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

			if (!text.empty())
			{
				BinaryWriter *textMsg = ServerText(text.c_str(), LTT_DEFAULT);
				g_pWorld->BroadcastPVS(_weenie, textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0, false);
				delete textMsg;
			}

			break;
		}
	case WorldBroadcast_EmoteType:
		{
			std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

			if (!text.empty())
			{
				BinaryWriter *textMsg = ServerText(text.c_str(), LTT_DEFAULT);
				g_pWorld->BroadcastGlobal(textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0, false);
				delete textMsg;
			}

			break;
		}
	case AdminSpam_EmoteType:
	{
		std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

		if (!text.empty())
		{
			BinaryWriter *textMsg = ServerText(text.c_str(), LTT_ALL_CHANNELS);
			g_pWorld->BroadcastGlobal(textMsg->GetData(), textMsg->GetSize(), PRIVATE_MSG, 0, false);
			delete textMsg;
		}

		break;
	}
	case Activate_EmoteType:
		{
			if (DWORD activation_target_id = _weenie->InqIIDQuality(ACTIVATION_TARGET_IID, 0))
			{
				CWeenieObject *target = g_pWorld->FindObject(activation_target_id);
				if (target)
					target->Activate(target_id);
			}

			break;
		}
	case CastSpellInstant_EmoteType:
		{
			if (target_id == 0)
				target_id = _weenie->GetID();
			_weenie->MakeSpellcastingManager()->CastSpellInstant(target_id, emote.spellid);
			break;
		}
	case CastSpell_EmoteType:
		{
			if (target_id == 0)
				target_id = _weenie->GetID();
			_weenie->MakeSpellcastingManager()->CreatureBeginCast(target_id, emote.spellid);
			break;
		}
	case AwardXP_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			long long amount = emote.amount64;

			amount = (int)(amount * g_pConfig->RewardXPMultiplier(target->InqIntQuality(LEVEL_INT, 0)));

			if (amount < 0)
				amount = 0;

			target->GiveSharedXP(amount, true);
			break;
		}
	case AwardNoShareXP_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			long long amount = emote.amount64;

			amount = (int)(amount * g_pConfig->RewardXPMultiplier(target->InqIntQuality(LEVEL_INT, 0)));

			if (amount < 0)
				amount = 0;

			target->GiveXP(amount, true);
			break;
		}
	case AwardSkillXP_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			target->GiveSkillXP((STypeSkill)emote.stat, emote.amount, false);
			break;
		}
	case AwardLevelProportionalSkillXP_EmoteType:
	{
		CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
		if (!target)
			break;

		Skill skill;
		target->m_Qualities.InqSkill((STypeSkill)emote.stat, skill);

		if (skill._sac < TRAINED_SKILL_ADVANCEMENT_CLASS)
			break;

		int currentLevel = skill._level_from_pp;

		if (currentLevel <= 10)
			currentLevel += 5 - (currentLevel * 0.4);

		DWORD64 xp_to_next_level = ExperienceSystem::ExperienceToSkillLevel(TRAINED_SKILL_ADVANCEMENT_CLASS, currentLevel + 1) - ExperienceSystem::ExperienceToSkillLevel(TRAINED_SKILL_ADVANCEMENT_CLASS, currentLevel);

		DWORD64 xp_to_give = (DWORD64)round((long double)xp_to_next_level * emote.percent);
		if (emote.min > 0)
			xp_to_give = max(xp_to_give, emote.min);
		if (emote.max > 0)
			xp_to_give = min(xp_to_give, emote.max);

		target->GiveSkillXP((STypeSkill)emote.stat, xp_to_give, false);
		break;
	}
	case AwardSkillPoints_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			target->GiveSkillPoints((STypeSkill)emote.stat, emote.amount);
			break;
		}
	case AwardTrainingCredits_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			target->GiveSkillCredits(emote.amount, true);
			break;
		}
	case AwardLevelProportionalXP_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			int current_level = target->m_Qualities.GetInt(LEVEL_INT, 1);
			DWORD64 xp_to_next_level = ExperienceSystem::ExperienceToRaiseLevel(current_level, current_level + 1);

			DWORD64 xp_to_give = (DWORD64)round((long double)xp_to_next_level * emote.percent);
			if (emote.min64 > 0)
				xp_to_give = max(xp_to_give, emote.min64);
			if (emote.max64 > 0)
				xp_to_give = min(xp_to_give, emote.max64);
			target->GiveXP(xp_to_give, true, false);
			break;
		}
	case Give_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (!target)
				break;

			_weenie->SimulateGiveObject(target, emote.cprof.wcid, emote.cprof.stack_size, emote.cprof.palette, emote.cprof.shade, emote.cprof.try_to_bond);
			break;
		}
	case Motion_EmoteType:
		{
			_weenie->DoAutonomousMotion(OldToNewCommandID(emote.motion));
			break;
		}
	case ForceMotion_EmoteType:
		{
			_weenie->DoForcedMotion(OldToNewCommandID(emote.motion));
			break;
		}
	case PhysScript_EmoteType:
	{
		CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
		if (target)
			target->EmitEffect(emote.pscript, 1.0);

		break;
	}
	case Say_EmoteType:
	{
		std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

		if (!text.empty())
			_weenie->SpeakLocal(text.c_str(), LTT_EMOTE);

		break;
	}
	case Sound_EmoteType:
	{
		_weenie->EmitSound(emote.sound, 1.0);

		break;
	}
	case Tell_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

				if (!text.empty())
					target->SendNetMessage(DirectChat(text.c_str(), _weenie->GetName().c_str(), _weenie->GetID(), target->GetID(), LTT_SPEECH_DIRECT), PRIVATE_MSG, TRUE);
			}

			break;
		}
	case InflictVitaePenalty_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				target->UpdateVitaePool(0);
				target->ReduceVitae((float)emote.amount / 100.f);
				target->UpdateVitaeEnchantment();
			}

			break;
		}
	case TellFellow_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

				if (text.empty())
					break;

				Fellowship *fellow = target->GetFellowship();

				if (!fellow)
				{
					target->SendNetMessage(DirectChat(text.c_str(), _weenie->GetName().c_str(), _weenie->GetID(), target->GetID(), LTT_SPEECH_DIRECT), PRIVATE_MSG, TRUE);
				}
				else
				{
					for (auto &entry : fellow->_fellowship_table)
					{
						if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
						{
							member->SendNetMessage(DirectChat(text.c_str(), _weenie->GetName().c_str(), _weenie->GetID(), target->GetID(), LTT_SPEECH_DIRECT), PRIVATE_MSG, TRUE);
						}
					}
				}
			}

			break;
		}
	case TextDirect_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

				if (!text.empty())
					target->SendNetMessage(ServerText(text.c_str(), LTT_DEFAULT), PRIVATE_MSG, TRUE);
			}

			break;
		}
	case DirectBroadcast_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

				if (!text.empty())
					target->SendNetMessage(ServerText(text.c_str(), LTT_DEFAULT), PRIVATE_MSG, TRUE);
			}

			break;
		}
	case BLog_EmoteType:
	{
		CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
		if (target)
		{
			std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

			if (!text.empty())
				target->SendNetMessage(ServerText(text.c_str(), LTT_COMBAT), PRIVATE_MSG, TRUE);
		}

		break;
	}
	case FellowBroadcast_EmoteType:
		{
			CPlayerWeenie *target = g_pWorld->FindPlayer(target_id);
			if (target)
			{
				std::string text = ReplaceEmoteText(emote.msg, target_id, _weenie->GetID());

				if (text.empty())
					break;

				Fellowship *fellow = target->GetFellowship();

				if (!fellow)
				{
					target->SendNetMessage(ServerText(text.c_str(), LTT_DEFAULT), PRIVATE_MSG, TRUE);
				}
				else
				{
					for (auto &entry : fellow->_fellowship_table)
					{
						if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
						{
							member->SendNetMessage(ServerText(text.c_str(), LTT_DEFAULT), PRIVATE_MSG, TRUE);
						}
					}
				}
			}

			break;
		}
	case TurnToTarget_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				MovementParameters params;
				_weenie->TurnToObject(target_id, &params);
			}
			break;
		}
	case Turn_EmoteType:
		{
			MovementParameters params;
			params.desired_heading = emote.frame.get_heading();
			params.speed = 1.0f;
			params.action_stamp = ++_weenie->m_wAnimSequence;
			params.modify_interpreted_state = 0;
			_weenie->last_move_was_autonomous = false;

			_weenie->cancel_moveto();
			_weenie->TurnToHeading(&params);
			break;
		}
	case TeachSpell_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				target->LearnSpell(emote.spellid, true);
			}
			break;
		}
	case InqEvent_EmoteType:
		{
			bool success = g_pGameEventManager->IsEventStarted(emote.msg.c_str());

			ChanceExecuteEmoteSet(success ? EventSuccess_EmoteCategory : EventFailure_EmoteCategory, emote.msg, target_id);
			break;
		}

	case StartEvent_EmoteType:
		{
			g_pGameEventManager->StartEvent(emote.msg.c_str());
			break;
		}

	case StopEvent_EmoteType:
		{
			g_pGameEventManager->StopEvent(emote.msg.c_str());
			break;
		}

	case InqQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = target->InqQuest(emote.msg.c_str());

				ChanceExecuteEmoteSet(success ? QuestSuccess_EmoteCategory : QuestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqFellowNum_EmoteType:
		{
		if (!_weenie->m_Qualities._emote_table)
		{
			break;
		}

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			Fellowship *fellow = target->GetFellowship();

			if (!fellow)
				ChanceExecuteEmoteSet(TestNoFellow_EmoteCategory, emote.msg, target_id);
			else
			{
				bool success = false;

				int numberOfMembers = (int)fellow->_fellowship_table.size();

				if (numberOfMembers >= emote.min && numberOfMembers <= emote.max)
					success = true;

				ChanceExecuteEmoteSet(success ? NumFellowsSuccess_EmoteCategory : NumFellowsFailure_EmoteCategory, emote.msg, target_id);
			}
		}

		break;
	}
	case InqFellowQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
			{
				break;
			}

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				Fellowship *fellow = target->GetFellowship();

				if (!fellow)
					ChanceExecuteEmoteSet(QuestNoFellow_EmoteCategory, emote.msg, target_id);
				else
				{
					bool success = false;

					for (auto &entry : fellow->_fellowship_table)
					{
						if (CWeenieObject *member = g_pWorld->FindObject(entry.first))
						{
							success = member->InqQuest(emote.msg.c_str());

							if (success)
								break;
						}
					}

					ChanceExecuteEmoteSet(success ? QuestSuccess_EmoteCategory : QuestFailure_EmoteCategory, emote.msg, target_id);
				}
			}

			break;
		}
	case UpdateQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = target->UpdateQuest(emote.msg.c_str());
					
				ChanceExecuteEmoteSet(success ? QuestSuccess_EmoteCategory : QuestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case StampQuest_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				target->StampQuest(emote.msg.c_str());
			}
			break;
		}
	case StampFellowQuest_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				Fellowship *fellow = target->GetFellowship();

				if (fellow)
				{
					for (auto &entry : fellow->_fellowship_table)
					{
						if (CWeenieObject *member = g_pWorld->FindObject(entry.first))
						{
							member->StampQuest(emote.msg.c_str());
						}
					}
				}
			}

			break;
		}
	case UpdateFellowQuest_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				Fellowship *fellow = target->GetFellowship();
				if (!fellow)
					ChanceExecuteEmoteSet(QuestNoFellow_EmoteCategory, emote.msg, target_id);
				else
				{
					/*
					bool success = false;

					for (auto &entry : fellow->_fellowship_table)
					{
						if (CWeenieObject *member = g_pWorld->FindObject(entry.first))
						{
							success = member->UpdateQuest(emote.msg.c_str());

							if (success)
								break;
						}
					}

					PackableList<EmoteSet> *emoteCategory = _weenie->m_Qualities._emote_table->_emote_table.lookup(success ? QuestSuccess_EmoteCategory : QuestFailure_EmoteCategory);
					if (!emoteCategory)
						break;

					for (auto &entry : *emoteCategory)
					{
						if (!_stricmp(entry.quest.c_str(), emote.msg.c_str()))
						{
							// match
							ExecuteEmoteSet(entry, target_id);
							break;
						}
					}
					*/

					target->SendText("Unsupported quest logic, please report to Pea how you received this.", LTT_DEFAULT);

					ChanceExecuteEmoteSet(QuestFailure_EmoteCategory, emote.msg, target_id);
				}
			}

			break;
		}
	case IncrementQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				target->IncrementQuest(emote.msg.c_str());
			}

			break;
		}
	case DecrementQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				target->DecrementQuest(emote.msg.c_str());
			}

			break;
		}
	case EraseQuest_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				target->EraseQuest(emote.msg.c_str());
			}

			break;
		}
	case Goto_EmoteType:
		{
			ChanceExecuteEmoteSet(GotoSet_EmoteCategory, emote.msg, target_id);
		}
	case InqBoolStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			int boolStat;
			if (target->m_Qualities.InqBool((STypeBool)emote.stat, boolStat))
			{
				hasQuality = true;
				if (boolStat > 0)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqIntStat_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;
				bool hasQuality = false;

				int intStat;
				if (target->m_Qualities.InqInt((STypeInt) emote.stat, intStat))
				{
					hasQuality = true;
					if (intStat >= emote.min && intStat <= emote.max)
					{
						success = true;
					}
				}

				if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
					break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
				ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqFloatStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			double floatStat;
			if (target->m_Qualities.InqFloat((STypeFloat)emote.stat, floatStat))
			{
				hasQuality = true;
				if (floatStat >= emote.fmin && floatStat <= emote.fmax)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqStringStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			std::string stringStat;
			if (target->m_Qualities.InqString((STypeString)emote.stat, stringStat))
			{
				hasQuality = true;
				if (stringStat == emote.teststring)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqAttributeStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			DWORD attributeStat;
			if (target->m_Qualities.InqAttribute((STypeAttribute)emote.stat, attributeStat, FALSE))
			{
				hasQuality = true;
				if (attributeStat >= emote.min && attributeStat <= emote.max)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqRawAttributeStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			DWORD attributeStat;
			if (target->m_Qualities.InqAttribute((STypeAttribute)emote.stat, attributeStat, TRUE))
			{
				hasQuality = true;
				if (attributeStat >= emote.min && attributeStat <= emote.max)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqSecondaryAttributeStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			DWORD attribute2ndStat;
			if (target->m_Qualities.InqAttribute2nd((STypeAttribute2nd)emote.stat, attribute2ndStat, FALSE))
			{
				hasQuality = true;
				if (attribute2ndStat >= emote.min && attribute2ndStat <= emote.max)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqRawSecondaryAttributeStat_EmoteType:
	{
		if (!_weenie->m_Qualities._emote_table)
			break;

		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			bool success = false;
			bool hasQuality = false;

			DWORD attribute2ndStat;
			if (target->m_Qualities.InqAttribute2nd((STypeAttribute2nd)emote.stat, attribute2ndStat, TRUE))
			{
				hasQuality = true;
				if (attribute2ndStat >= emote.min && attribute2ndStat <= emote.max)
					success = true;
			}

			if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
				break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
			ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
		}

		break;
	}
	case InqSkillTrained_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;

				SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
				bool hasQuality = target->m_Qualities.InqSkillAdvancementClass((STypeSkill)emote.stat, sac);

				if (sac == TRAINED_SKILL_ADVANCEMENT_CLASS || sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					success = true;

				if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
					break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
				ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqSkillSpecialized_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;

				SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
				bool hasQuality = target->m_Qualities.InqSkillAdvancementClass((STypeSkill)emote.stat, sac);

				if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					success = true;

				if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
					break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
				ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqSkillStat_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;
				bool hasQuality = false;

				DWORD skillStat;
				if (target->m_Qualities.InqSkill((STypeSkill)emote.stat, skillStat, FALSE))
				{
					hasQuality = true;
					if (skillStat >= emote.min && skillStat <= emote.max)
					{
						success = true;
					}
				}

				if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
					break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
				ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqRawSkillStat_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;
				bool hasQuality = false;

				DWORD skillStat;
				if (target->m_Qualities.InqSkill((STypeSkill)emote.stat, skillStat, TRUE))
				{
					hasQuality = true;
					if (skillStat >= emote.min && skillStat <= emote.max)
					{
						success = true;
					}
				}

				if (!hasQuality && ChanceExecuteEmoteSet(TestNoQuality_EmoteCategory, emote.msg, target_id))
					break; //if we have a TestNoQuality_EmoteCategory break otherwise try the categories below.
				ChanceExecuteEmoteSet(success ? TestSuccess_EmoteCategory : TestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case InqQuestSolves_EmoteType:
		{
			if (!_weenie->m_Qualities._emote_table)
				break;

			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				bool success = false;

				int intQuestSolves = target->InqQuestSolves(emote.msg.c_str());

				if (intQuestSolves >= emote.min && intQuestSolves <= emote.max)
					success = true;

				ChanceExecuteEmoteSet(success ? QuestSuccess_EmoteCategory : QuestFailure_EmoteCategory, emote.msg, target_id);
			}

			break;
		}
	case SetIntStat_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
				target->m_Qualities.SetInt((STypeInt)emote.stat, emote.amount);
			break;
		}
	case IncrementIntStat_EmoteType:
		{
			CWeenieObject *target = g_pWorld->FindObject(target_id);
			if (target)
			{
				int intStat = target->InqIntQuality((STypeInt)emote.stat, 0, TRUE) + 1;
				target->m_Qualities.SetInt((STypeInt)emote.stat, intStat);
			}
			break;
		}
	case DecrementIntStat_EmoteType:
	{
		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			int intStat = target->InqIntQuality((STypeInt)emote.stat, 0, TRUE) - 1;
			target->m_Qualities.SetInt((STypeInt)emote.stat, intStat);
		}
		break;
	}
	case CreateTreasure_EmoteType:
	{
		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target && target->AsContainer())
			target->AsContainer()->SpawnTreasureInContainer((eTreasureCategory)emote.treasure_type, emote.wealth_rating, emote.treasure_class);
		break;
	}
	case ResetHomePosition_EmoteType:
	{
		CMonsterWeenie *monster = _weenie->AsMonster();
		if (monster && monster->m_MonsterAI)
			monster->m_MonsterAI->SetHomePosition(monster->m_Position);

		_weenie->SetInitialPosition(_weenie->m_Position);
		break;
	}
	case MoveHome_EmoteType:
	{
		CMonsterWeenie *monster = _weenie->AsMonster();
		if (monster && monster->m_MonsterAI)
			monster->m_MonsterAI->SwitchState(ReturningToSpawn);
		else
		{
			//Position destination;
			//_weenie->m_Qualities.InqPosition(INSTANTIATION_POSITION, destination);

			//if (destination.objcell_id)
			//{
			//	MovementParameters params;
			//	params.desired_heading = destination.frame.get_heading();

			//	MovementStruct mvs;
			//	mvs.type = MovementTypes::MoveToPosition;
			//	mvs.pos = destination;
			//	mvs.params = &params;

			//	_weenie->last_move_was_autonomous = false;
			//	_weenie->movement_manager->PerformMovement(mvs);
			//}

			//todo: make creatures that do not normally have an AI(vendors/etc) move home.
			//Position initialPos;
			//if (_weenie->m_Qualities.InqPosition(INSTANTIATION_POSITION, initialPos) && initialPos.objcell_id)
			//{
			//	_weenie->Movement_Teleport(initialPos, false);
			//}
		}
		break;
	}
	case Move_EmoteType:
	{
		//Position destination = _weenie->m_Position.add_offset(emote.frame.m_origin);
		//destination.frame.m_origin.z = CalcSurfaceZ(destination.objcell_id, destination.frame.m_origin.x, destination.frame.m_origin.y);

		//MovementParameters params;
		//params.desired_heading = emote.frame.get_heading();

		//MovementStruct mvs;
		//mvs.type = MovementTypes::MoveToPosition;
		//mvs.pos = destination;
		//mvs.params = &params;

		//_weenie->last_move_was_autonomous = false;
		//_weenie->movement_manager->PerformMovement(mvs);

		break;
	}
	case SetSanctuaryPosition_EmoteType:
	{
		CWeenieObject *target = g_pWorld->FindObject(target_id);
		if (target)
		{
			target->SetInitialPosition(target->m_Position);
			target->m_Qualities.SetPosition(SANCTUARY_POSITION, target->m_Position);
		}
		break;
	}
	}
}

bool EmoteManager::IsExecutingAlready()
{
	return !_emoteQueue.empty();
}

void EmoteManager::Tick()
{
	if (_emoteQueue.empty())
		return;

	for (std::list<QueuedEmote>::iterator i = _emoteQueue.begin(); i != _emoteQueue.end();)
	{
		if (i->_executeTime > Timer::cur_time || _weenie->IsBusyOrInAction() || _weenie->IsMovingTo())
			break;

		ExecuteEmote(i->_data, i->_target_id);
		i = _emoteQueue.erase(i);
		if (i != _emoteQueue.end())
			i->_executeTime = Timer::cur_time + i->_data.delay;
	}
}

void EmoteManager::Cancel()
{
	_emoteQueue.clear();
}

void EmoteManager::OnDeath(DWORD killer_id)
{
	Cancel();

	if (_weenie->m_Qualities._emote_table)
		ChanceExecuteEmoteSet(Death_EmoteCategory, killer_id);
}


