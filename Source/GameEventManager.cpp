
#include "StdAfx.h"
#include "GameEventManager.h"
#include "InferredPortalData.h"
#include "World.h"

GameEventManager::GameEventManager()
{
}

GameEventManager::~GameEventManager()
{
}

void GameEventManager::Initialize()
{
	_gameEvents = g_pPortalDataEx->_gameEvents._gameEvents;
}

std::string GameEventManager::NormalizeEventName(const char *eventName)
{
	std::string lowerCaseName = eventName;
	std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);
	return lowerCaseName;
}

void GameEventManager::StartEvent(const char *eventName)
{
	std::string normalizedEventName = NormalizeEventName(eventName);

	if (GameEventDef *eventDesc = _gameEvents.lookup(normalizedEventName.c_str()))
	{
		if (eventDesc->_eventState != GameEventState::On_GameEventState)
		{
			eventDesc->_eventState = GameEventState::On_GameEventState;
			g_pWorld->NotifyEventStarted(normalizedEventName.c_str());
		}
	}
}

void GameEventManager::StopEvent(const char *eventName)
{
	std::string normalizedEventName = NormalizeEventName(eventName);

	if (GameEventDef *eventDesc = _gameEvents.lookup(normalizedEventName.c_str()))
	{
		if (eventDesc->_eventState != GameEventState::Off_GameEventState)
		{
			eventDesc->_eventState = GameEventState::Off_GameEventState;
			g_pWorld->NotifyEventStopped(normalizedEventName.c_str());
		}
	}
}

bool GameEventManager::IsEventStarted(const char *eventName)
{
	std::string normalizedEventName = NormalizeEventName(eventName);

	if (GameEventDef *eventDesc = _gameEvents.lookup(normalizedEventName.c_str()))
	{
		if (eventDesc->_eventState == GameEventState::On_GameEventState)
		{
			return true;
		}
	}

	return false;
}



