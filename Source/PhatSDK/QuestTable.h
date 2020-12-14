
#pragma once

class QuestProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	/*
		+0x0 (type ? ? ? ) Update;
		+0x0 (type ? ? ? ) Stamp;
		+0x0 (type ? ? ? ) IsOkToPortal;
		+0x0 (type ? ? ? ) IsOkToComplete;
		+0x0 (type ? ? ? ) InqTimeUntilOkayToComplete;
		+0x0 (type ? ? ? ) HasCompletedMaxTimes;
		+0x0 (type ? ? ? ) HasCompletedTooRecently;
		+0x0 (type ? ? ? ) IsValid;
		+0x0 (type ? ? ? ) GetLastUpdateTime;
		+0x0 (type ? ? ? ) GetNumCompletions;
		+0x0 (type ? ? ? ) GetRealTime;
		+0x0 (type ? ? ? ) Touch;
		+0x0 (type ? ? ? ) SetLastUpdateTime;
		+0x0 (type ? ? ? ) SetRealTime;
		+0x0 (type ? ? ? ) SetNumCompletions;
		+0x0 (type ? ? ? ) IncrementNumCompletions;
		+0x0 (type ? ? ? ) DecrementNumCompletions;
		+0x0 (type ? ? ? ) pack_size;
		+0x0 (type ? ? ? ) Pack;
		+0x0 (type ? ? ? ) UnPack;
		*/

	void Stamp();
	int InqTimeUntilOkayToComplete(const char *questName);
	void IncrementNumCompletions(int amount = 1);
	void DecrementNumCompletions(int amount = 1);

	double _last_update = 0;
	int _real_time = 0;
	unsigned int _num_completions = 0;
};

class QuestTable : public PackObj
{
public:
	DECLARE_PACKABLE()

	bool HasQuests();
	unsigned int NumQuests();
	bool HasQuest(const char *questName);
	void AddQuest(const char *questName);
	void RemoveQuest(const char *questName);
	void PurgeQuests();
	bool InqQuest(const char *questName);
	unsigned int InqQuestSolves(const char *questName);
	unsigned int InqQuestMax(const char *questName);
	void SetQuestCompletions(const char *questName, int numCompletions);
	void IncrementQuest(const char *questName, int amount);
	void DecrementQuest(const char *questName, int amount);
	bool UpdateQuest(const char *questName);
	void StampQuest(const char *questName);
	int InqTimeUntilOkayToComplete(const char *questName);
	std::string Ktref(const char *questName);
	QuestProfile *GetQuest(const char *questName);
	PackableHashTable<std::string, QuestProfile, std::string> *GetQuestTable();

	static bool TimedQuest(const char *questName); // custom
	static std::string GetTrimmedQuestName(const char *questName); // custom
	static std::string GetNeutralQuestName(const char *questName, bool trimSuffix); // custom

	/*
	+ 0x0 (type ? ? ? ) PackObj;
	+0x0 (type ? ? ? ) QuestTable;
	+0x0 (type ? ? ? ) QuestTable;
	+0x0 (type ? ? ? ) HasQuests;
	+0x0 (type ? ? ? ) NumQuests;
	+0x0 (type ? ? ? ) HasQuest;
	+0x0 (type ? ? ? ) AddQuest;
	+0x0 (type ? ? ? ) RemoveQuest;
	+0x0 (type ? ? ? ) RemoveQuest;
	+0x0 (type ? ? ? ) PurgeQuests;
	+0x0 (type ? ? ? ) InqQuest;
	+0x0 (type ? ? ? ) SetQuestCompletions;
	+0x0 (type ? ? ? ) IncrementQuest;
	+0x0 (type ? ? ? ) DecrementQuest;
	+0x0 (type ? ? ? ) UpdateQuest;
	+0x0 (type ? ? ? ) StampQuest;
	+0x0 (type ? ? ? ) InqTimeUntilOkayToComplete;
	+0x0 (type ? ? ? ) GetQuest;
	+0x0 (type ? ? ? ) GetQuest;
	+0x0 (type ? ? ? ) GetQuestTable;
	+0x0 (type ? ? ? ) GetQuestTable;
	+0x0 (type ? ? ? ) Pack;
	+0x0 (type ? ? ? ) UnPack;
	+0x0 (type ? ? ? ) packed_size;
	*/

	PackableHashTable<std::string, QuestProfile, std::string> _quest_table;
};
