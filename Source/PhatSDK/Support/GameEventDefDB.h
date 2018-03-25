
#pragma once

#include "Packable.h"
#include "GameEnums.h"

class GameEventDef : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	int _startTime = -1;
	int _endTime = -1;
	GameEventState _eventState = Undef_GameEventState;
};

class GameEventDefDB : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	PackableHashTableWithJson<std::string, GameEventDef, std::string> _gameEvents;
};