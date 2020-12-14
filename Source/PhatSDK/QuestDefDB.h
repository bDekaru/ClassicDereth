
#pragma once

#include "Packable.h"

class QuestDef : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	std::string _fullname;
	int _mindelta = 0;
	int _maxsolves = 0;
};

class QuestDefDB : public PackObj, public PackableJson
{
public:
	virtual ~QuestDefDB() = default;

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableHashTableWithJson<std::string, QuestDef, std::string> _defs;
};

class CQuestDefDB : public QuestDefDB // , public DBObj
{
public:
	static QuestDef *GetQuestDef(const char *questName); // custom
};
