
#include "StdAfx.h"
#include "PhatSDK.h"
#include "QuestTable.h"

#if PHATSDK_IS_SERVER
#include "Config.h"
#endif

DEFINE_PACK(QuestProfile)
{
	pWriter->Write<double>(_last_update);
	pWriter->Write<int>(_real_time);
	pWriter->Write<DWORD>(_num_completions);
}

DEFINE_UNPACK(QuestProfile)
{
	_last_update = pReader->Read<double>();
	_real_time = pReader->Read<int>();
	_num_completions = pReader->Read<DWORD>();
	return true;
}

void QuestProfile::Stamp()
{
	_last_update = Timer::cur_time;
	_real_time = g_pPhatSDK->GetCurrTimeStamp();
	IncrementNumCompletions();
}

void QuestProfile::IncrementNumCompletions()
{
	_num_completions++;
}

void QuestProfile::DecrementNumCompletions()
{
	_num_completions--;
	if (_num_completions < 0)
		_num_completions = 0;
}

int QuestProfile::InqTimeUntilOkayToComplete(const char *questName)
{
	if (_last_update <= 0)
	{
		return 0;
	}

	int timeElapsed = (int)(Timer::cur_time - _last_update);

	QuestDef *questDef = CQuestDefDB::GetQuestDef(questName);
	if (!questDef || questDef->_mindelta < 0)
	{
		return 0;
	}
	
#if PHATSDK_IS_SERVER
	int timeLeft = g_pConfig->UseMultiplierForQuestTime(questDef->_mindelta) - timeElapsed;
	return max(0, timeLeft);
#else
	int timeLeft = questDef->_mindelta - timeElapsed;
	return max(0, timeLeft);
#endif
}

DEFINE_PACK(QuestTable)
{
	_quest_table.Pack(pWriter);
}

DEFINE_UNPACK(QuestTable)
{
	_quest_table.UnPack(pReader);

#ifdef PHATSDK_IS_SERVER
	// in order to fix quest casing...
	PackableHashTable<std::string, QuestProfile, std::string> tableNoCase;
	for (auto &entry : _quest_table)
	{
		std::string lowerCaseDef = entry.first;
		std::transform(lowerCaseDef.begin(), lowerCaseDef.end(), lowerCaseDef.begin(), ::tolower);
		tableNoCase[lowerCaseDef] = entry.second;
	}
	_quest_table = tableNoCase;
#endif

	return true;
}

bool QuestTable::HasQuests()
{
	return NumQuests() > 0;
}

unsigned int QuestTable::NumQuests()
{
	return (unsigned int) _quest_table.size();
}

bool QuestTable::HasQuest(const char *questName)
{
	std::string trimmedQuestName = GetNeutralQuestName(questName, true);
	return _quest_table.exists(trimmedQuestName);
}

void QuestTable::AddQuest(const char *questName)
{
	std::string trimmedQuestName = GetNeutralQuestName(questName, true);
	if (HasQuest(trimmedQuestName.c_str()))
		return;

	QuestProfile quest;
	_quest_table.add(trimmedQuestName, &quest);
}

void QuestTable::RemoveQuest(const char *questName)
{
	std::string trimmedQuestName = GetNeutralQuestName(questName, true);
	_quest_table.remove(trimmedQuestName.c_str());
}

void QuestTable::PurgeQuests()
{
	_quest_table.clear();
}

bool QuestTable::TimedQuest(const char *questName)
{
	QuestDef *questDef = CQuestDefDB::GetQuestDef(GetNeutralQuestName(questName, true).c_str());
	if (!questDef || questDef->_mindelta <= 0)
	{
		return false;
	}

	return true;
}

bool QuestTable::InqQuest(const char *questName)
{
	// QuestSuccess if quest is present and within the mindelta
	// QuestFailure if quest not present or outside the mindelta

	std::string trimmedQuestName = GetNeutralQuestName(questName, true);

	if (QuestProfile *prof = GetQuest(trimmedQuestName.c_str()))
	{
		if (prof->_last_update != 0.0 && TimedQuest(questName) && prof->InqTimeUntilOkayToComplete(trimmedQuestName.c_str()) <= 0)
			return false;

		/*
		if (QuestDef *questDef = CQuestDefDB::GetQuestDef(trimmedQuestName.c_str()))
		{
			if (prof->_num_completions < questDef->_maxsolves)
				return true;		
		}
		*/

		return true;
	}

	return false;
}

unsigned int QuestTable::InqQuestSolves(const char *questName) // custom
{
	std::string trimmedQuestName = GetNeutralQuestName(questName, true);

	if (QuestProfile *prof = GetQuest(trimmedQuestName.c_str()))
	{
		return prof->_num_completions;
	}

	return 0;
}

void QuestTable::SetQuestCompletions(const char *questName, int numCompletions)
{
	AddQuest(questName);
	GetQuest(questName)->_num_completions = numCompletions;
}

void QuestTable::IncrementQuest(const char *questName)
{
	AddQuest(questName);
	
	if (QuestProfile *quest = GetQuest(questName))
	{
		quest->IncrementNumCompletions();
	}
}

void QuestTable::DecrementQuest(const char *questName)
{
	AddQuest(questName);

	if (QuestProfile *quest = GetQuest(questName))
	{
		quest->DecrementNumCompletions();
	}
}

std::string QuestTable::GetTrimmedQuestName(const char *questName) // custom
{
	std::string trimmedQuestName = questName;
	size_t suffixPos = trimmedQuestName.find_first_of("@");
	if (suffixPos != std::string::npos)
	{
		trimmedQuestName = trimmedQuestName.substr(0, suffixPos);
	}
	return trimmedQuestName;
}

std::string QuestTable::GetNeutralQuestName(const char *questName, bool trimSuffix) // custom
{
	std::string neutralQuestName = trimSuffix ? GetTrimmedQuestName(questName) : questName;
	std::transform(neutralQuestName.begin(), neutralQuestName.end(), neutralQuestName.begin(), ::tolower);
	return neutralQuestName;
}

bool QuestTable::UpdateQuest(const char *questName)
{
	std::string trimmedQuestName = GetNeutralQuestName(questName, true);
	
	if (QuestProfile *prof = GetQuest(trimmedQuestName.c_str()))
	{
		if (prof->_last_update != 0.0 && TimedQuest(questName) && prof->InqTimeUntilOkayToComplete(trimmedQuestName.c_str()) > 0)
			return false; // can't do this yet?

		if (QuestDef *questDef = CQuestDefDB::GetQuestDef(trimmedQuestName.c_str()))
		{
			if (questDef->_maxsolves >= 1 && prof->_num_completions >= questDef->_maxsolves)
				return false;

			prof->Stamp();
			return true;
		}

		return false;
	}
	else
	{
		AddQuest(trimmedQuestName.c_str());

		if (QuestProfile *prof = GetQuest(trimmedQuestName.c_str()))
		{
			prof->Stamp();
			return true;
		}

		return false;
	}

	/*

	if (InqQuest(trimmedQuestName.c_str()))
		return false;

	AddQuest(trimmedQuestName.c_str());

	if (QuestProfile *prof = GetQuest(trimmedQuestName.c_str()))
	{
		if (QuestDef *questDef = CQuestDefDB::GetQuestDef(trimmedQuestName.c_str()))
		{
			if (prof->_num_completions < questDef->_maxsolves)
			{
				prof->Stamp();
				// prof->IncrementNumCompletions();
				return true;
			}
		}
	}

	return false;
	*/
}

void QuestTable::StampQuest(const char *questName)
{
	AddQuest(questName);

	if (QuestProfile *quest = GetQuest(questName))
	{
		quest->Stamp();
	}
}

int QuestTable::InqTimeUntilOkayToComplete(const char *questName)
{
	if (QuestProfile *quest = GetQuest(questName))
	{
		return quest->InqTimeUntilOkayToComplete(questName);
	}

	return 0;
}

QuestProfile *QuestTable::GetQuest(const char *questName)
{
	return _quest_table.lookup(GetNeutralQuestName(questName, true));
}

PackableHashTable<std::string, QuestProfile, std::string> *QuestTable::GetQuestTable()
{
	return &_quest_table;
}
