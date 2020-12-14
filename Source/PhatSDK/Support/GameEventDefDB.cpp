
#include <StdAfx.h>
#include "PhatSDK.h"

DEFINE_PACK_JSON(GameEventDef)
{
	writer["startTime"] = _startTime;
	writer["endTime"] = _endTime;
	writer["eventState"] = (int)_eventState;
}

DEFINE_UNPACK_JSON(GameEventDef)
{
	_startTime = reader["startTime"];
	_endTime = reader["endTime"];
	_eventState = (GameEventState) (int) reader["eventState"];
	return true;
}

DEFINE_PACK(GameEventDef)
{
	pWriter->Write<int>(_startTime);
	pWriter->Write<int>(_endTime);
	pWriter->Write<int>((int)_eventState);
}

DEFINE_UNPACK(GameEventDef)
{
	_startTime = pReader->Read<int>();
	_endTime = pReader->Read<int>();
	_eventState = (GameEventState)pReader->Read<int>();
	return true;
}

DEFINE_PACK_JSON(GameEventDefDB)
{
	_gameEvents.PackJson(writer);
}

DEFINE_UNPACK_JSON(GameEventDefDB)
{
	_gameEvents.UnPackJson(reader);
	return true;
}

DEFINE_PACK(GameEventDefDB)
{
	_gameEvents.Pack(pWriter);
}

DEFINE_UNPACK(GameEventDefDB)
{
	_gameEvents.UnPack(pReader);

#ifdef PHATSDK_IS_SERVER
	// in order to fix quest casing...
	PackableHashTableWithJson<std::string, GameEventDef, std::string> defNoCase;
	for (auto &entry : _gameEvents)
	{
		std::string lowerCaseDef = entry.first;
		std::transform(lowerCaseDef.begin(), lowerCaseDef.end(), lowerCaseDef.begin(), ::tolower);
		defNoCase[lowerCaseDef] = entry.second;
	}
	_gameEvents = defNoCase;
#endif

	return true;
}

