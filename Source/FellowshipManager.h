#pragma once

#include "Packable.h"

class Fellow : public PackObj
{
public:
	Fellow() = default;
	virtual ~Fellow() = default;

	Fellow(uint32_t player_id);
	Fellow(CWeenieObject* weenie);

	Fellow& operator=(const Fellow &other);

	DECLARE_PACKABLE()

	bool updated(const Fellow &other) const;

	std::string _name;
	unsigned int _level = 0;
	unsigned int _cp_cache = 0;
	unsigned int _lum_cache = 0;
	unsigned int _share_loot = 0;
	unsigned int _max_health = 0;
	unsigned int _max_stamina = 0;
	unsigned int _max_mana = 0;
	unsigned int _current_health = 0;
	unsigned int _current_stamina = 0;
	unsigned int _current_mana = 0;

	// not sent to the client:
	BOOL _updates = TRUE;
	CWeenieObject *_cachedWeenie = NULL;
	double splitPercent = 1.0;
};

enum Fellow_UpdateType
{
	Fellow_UpdateFull = 1,
	Fellow_UpdateStats = 2,
	Fellow_UpdateVitals = 3
};

class Fellowship : public PackObj
{
public:
	Fellowship() = default;
	virtual ~Fellowship() = default;

	DECLARE_PACKABLE()

	bool IsEmpty();
	bool IsFull();
	bool IsLocked();
	bool IsOpen();
	void Disband(uint32_t disbander_id);
	void RemoveFellow(uint32_t dismissee_id);
	void Quit(uint32_t player_id);
	bool CanJoin(uint32_t recruitee_id);
	void AddFellow(uint32_t recruitee_id);
	void ChangeOpen(BOOL open);
	void SetUpdates(uint32_t fellow_id, BOOL on);
	void AssignNewLeader(uint32_t new_leader_id);
	bool ShouldUpdate(uint32_t fellow_id);
	void FullUpdate();
	void UpdateData();
	void VitalsUpdate(uint32_t player_id);
	void StatsUpdate(uint32_t player_id);
	void GiveXP(CWeenieObject *source, int64_t amount, ExperienceHandlingType flags, bool bShowText);
	void GiveLum(CWeenieObject *source, int64_t amount, bool bShowText);
	unsigned int CalculateExperienceProportionSum();
	void Chat(uint32_t sender_id, const char *text);
	double CalculateDegradeMod(CWeenieObject *source, CWeenieObject *target);

	bool InqQuest(const char *questName) { return _quests.InqQuest(questName); }
	bool UpdateQuest(const char *questName) { return _quests.UpdateQuest(questName); }
	void StampQuest(const char *questName) { _quests.StampQuest(questName); }

	using fellows_departed_table = PackableHashTable<uint32_t, uint32_t>;

	PackableHashTable<uint32_t, Fellow> _fellowship_table;

	std::string _name;
	unsigned int _leader = 0;
	BOOL _share_xp = 0;
	BOOL _even_xp_split = 0;
	BOOL _open_fellow = 0;
	BOOL _locked = 0;
	BOOL _share_loot = 0;
	fellows_departed_table _fellows_departed;

	QuestTable _quests;

	double m_NextUpdate = 0.0;

	// not sent to the client:
	BOOL _desiredShareXP = false; // just because we want to share, doesn't mean we can (if levels don't match up anymore)
};

using fellowship_ptr_t = std::shared_ptr<Fellowship>;

class FellowshipManager
{
public:
	FellowshipManager() = default;
	virtual ~FellowshipManager() = default;

	void Tick();

	int Create(const std::string &name, uint32_t creator_id, BOOL shareXP);
	int Disband(const fellowship_ptr_t &fellowship, uint32_t disbander_id);
	int Quit(const fellowship_ptr_t &fellowship, uint32_t quiter_id);
	int Dismiss(const fellowship_ptr_t &fellowship, uint32_t dismisser, uint32_t dismissee);
	int Recruit(const fellowship_ptr_t &fellowship, uint32_t recruitor_id, uint32_t recruitee_id);

	int ChangeOpen(const fellowship_ptr_t &fellowship, uint32_t changer_id, BOOL open);
	int AssignNewLeader(const fellowship_ptr_t &fellowship, uint32_t changer_id, uint32_t new_leader_id);
	int RequestUpdates(const fellowship_ptr_t &fellowship, uint32_t requester_id, BOOL on);
	void Chat(const fellowship_ptr_t &fellowship, uint32_t sender_id, const char *text);

	static double GetEvenSplitXPPctg(unsigned int uiNumFellows);
	static uint64_t GetExperienceProportion(unsigned int level);

	double m_fNextUpdates = 0.0;
};

