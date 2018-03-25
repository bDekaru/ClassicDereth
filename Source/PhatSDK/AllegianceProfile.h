
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
	std::string m_motd;
	std::string m_motdSetBy;
	std::string m_AllegianceName;
	int m_NameLastSetTime = 0;
	unsigned int m_chatRoomID = 0;
	PHashTable<DWORD, eAllegianceOfficerLevel> m_AllegianceOfficers;
	SmartArray<std::string> m_OfficerTitles;
	Position m_BindPoint;
	int m_isLocked = 0;
	unsigned int m_ApprovedVassal = 0;
};

class AllegianceProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	AllegianceHierarchy _allegiance;
	unsigned int _total_members = 0;
	unsigned int _total_vassals = 0;
};