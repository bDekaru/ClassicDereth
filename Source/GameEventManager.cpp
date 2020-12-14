
#include <StdAfx.h>
#include "GameEventManager.h"
#include "InferredPortalData.h"
#include "World.h"
#include "Server.h"

extern CPhatServer *g_pPhatServer;

GameEventManager::GameEventManager()
{
}

GameEventManager::~GameEventManager()
{
}

void GameEventManager::Initialize()
{
	_gameEvents = g_pPortalDataEx->_gameEvents._gameEvents;

	// any events that are on by default we have their start time set to server start
	//for (auto &evt : _gameEvents)
	//{
	//	if (evt.second._eventState == GameEventState::On_GameEventState)
	//		evt.second.SetStarted(g_pPhatServer->GetStartupTime());
	//}
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
			eventDesc->SetStarted(g_pGlobals->Time());
			g_pWorld->NotifyEventStarted(normalizedEventName, eventDesc);
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
			eventDesc->SetStarted(-1.0);
			g_pWorld->NotifyEventStopped(normalizedEventName, eventDesc);
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

GameEventDef* GameEventManager::GetEvent(const char *eventName)
{
	std::string normalizedEventName = NormalizeEventName(eventName);

	if (GameEventDef *eventDesc = _gameEvents.lookup(normalizedEventName.c_str()))
	{
		return eventDesc;
	}
	
	return nullptr;
}

