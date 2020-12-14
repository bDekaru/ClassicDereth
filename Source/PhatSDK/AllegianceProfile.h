
#pragma once

#include "Packable.h"

class AllegianceData : public PackObj
{
public:
	DECLARE_PACKABLE()

	unsigned int _id = 0;
	std::string _name;
	unsigned int _gender = Invalid_Gender;
	unsigned int _hg = Invalid_HeritageGroup;
	unsigned int _rank = 0;
	unsigned int _level = 0;
	unsigned int _bitfield = 0;
	unsigned int _cp_tithed = 0;
	unsigned int _cp_cached = 0;
	unsigned int _loyalty = 0;
	unsigned int _leadership = 0;
	int _time_online = 0;
	int _allegiance_age = 0;
};

class AllegianceNode : public PackObj
{
public:
	DECLARE_PACKABLE()

	void SetMayPassupExperience(BOOL val);

	AllegianceNode *_patron = NULL;
	AllegianceNode *_peer = NULL;
	AllegianceNode *_vassal = NULL;
	AllegianceData _data;
};

class AllegianceHierarchy : public PackObj
{
public:
	AllegianceHierarchy();
	virtual ~AllegianceHierarchy();

	DECLARE_PACKABLE()

	void Clear();

	AllegianceVersion m_oldVersion = Newest_AllegianceVersion;
	//  yeah let's not do it this way, thanks
	// AllegianceNode *m_pMonarch = NULL;
	// unsigned int m_total = 0;

	std::list<AllegianceNode *> _nodes;

	int m_monarchBroadcastTime = 0;
	int m_spokesBroadcastTime = 0;
	unsigned int m_monarchBroadcastsToday = 0;
	unsigned int m_spokesBroadcastsToday = 0;
	std::string m_motd; // deprecated?
	std::string m_motdSetBy; // deprecated?
	std::string m_AllegianceName;
	int m_NameLastSetTime = 0;
	unsigned int m_chatRoomID = 0;
	PHashTable<uint32_t, eAllegianceOfficerLevel> m_AllegianceOfficers; // deprecated?
	SmartArray<std::string> m_OfficerTitles; // deprecated?
	Position m_BindPoint; // deprecated?
	int m_isLocked = 0;
	unsigned int m_ApprovedVassal = 0;

	// part of pack/unpack to database only - not sent as part of the allegiance update according to retail PCAPs
	std::string m_storedMOTD = "";
	std::string m_storedMOTDSetBy = "";
	Position m_storedBindPoint = Position();
	std::vector<std::string> m_officerTitleList = { "Speaker", "Seneschal", "Castellan" };
	PHashTable<uint32_t, eAllegianceOfficerLevel> m_officerList = {}; // weenie id, officer level pairs

	PHashTable<uint32_t, std::string> m_BanList = {}; // account id, character name pairs (was never sent as part of allegiance update)
	PHashTable<uint32_t, int> m_chatGagList = {}; // (player id, gag end time in seconds)

	bool allAllegiancesUpdated = false;
	bool packForDB = false;
};

class AllegianceProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	AllegianceHierarchy _allegiance;
	unsigned int _total_members = 0;
	unsigned int _total_vassals = 0;
};