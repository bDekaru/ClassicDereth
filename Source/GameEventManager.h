
#pragma once

class GameEventManager
{
public:
	GameEventManager();
	~GameEventManager();

	void Initialize();

	std::string NormalizeEventName(const char *eventName);
	void StartEvent(const char *eventName);
	void StopEvent(const char *eventName);
	bool IsEventStarted(const char *eventName);

	PackableHashTableWithJson<std::string, GameEventDef, std::string> _gameEvents;
};
