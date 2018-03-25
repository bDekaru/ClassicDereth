
#pragma once

#include "Packable.h"

class Fellow : public PackObj
{
public:
	DECLARE_PACKABLE()

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
	DECLARE_PACKABLE()

	bool IsEmpty();
	bool IsFull();
	bool IsLocked();
	bool IsOpen();
	void Disband(DWORD disbander_id);
	void Dismiss(DWORD dismissee_id);
	void Quit(DWORD player_id);
	void Recruit(DWORD recruitee_id);
	void ChangeOpen(BOOL open);
	void SetUpdates(DWORD fellow_id, BOOL on);
	void AssignNewLeader(DWORD new_leader_id);
	void SendUpdate(int updateType);
	void UpdateData();
	void TickUpdate();
	void GiveXP(CWeenieObject *source, long long amount, bool bShowText);
	unsigned int CalculateExperienceProportionSum();
	void Chat(DWORD sender_id, const char *text);
	double CalculateDegradeMod(CWeenieObject *source, CWeenieObject *target);

	PackableHashTable<unsigned long, Fellow> _fellowship_table;

	std::string _name;
	unsigned int _leader = 0;
	BOOL _share_xp = 0;
	BOOL _even_xp_split = 0;
	BOOL _open_fellow = 0;
	BOOL _locked = 0;
	BOOL _share_loot = 0;
	PackableHashTable<unsigned long, long> _fellows_departed;

	// not sent to the client:
	BOOL _desiredShareXP = false; // just because we want to share, doesn't mean we can (if levels don't match up anymore)
};

class FellowshipManager
{
public:
	FellowshipManager();
	virtual ~FellowshipManager();

	void Tick();

	Fellowship *GetFellowship(const std::string &name);

	int Create(const std::string &name, DWORD creator_id, BOOL shareXP);
	int Disband(const std::string &name, DWORD disbander_id);
	int Quit(const std::string &name, DWORD quiter_id);
	int Dismiss(const std::string &name, DWORD dismisser, DWORD dismissee);
	int Recruit(const std::string &name, DWORD recruitor_id, DWORD recruitee_id);
	int ChangeOpen(const std::string &name, DWORD changer_id, BOOL open);
	int AssignNewLeader(const std::string &name, DWORD changer_id, DWORD new_leader_id);
	int RequestUpdates(const std::string &name, DWORD requester_id, BOOL on);
	void Chat(const std::string &name, DWORD sender_id, const char *text);

	static double GetEvenSplitXPPctg(unsigned int uiNumFellows);
	static unsigned __int64 GetExperienceProportion(unsigned int level);

	std::unordered_map<std::string, Fellowship *> m_Fellowships;
	double m_fNextUpdates = 0.0;
};

