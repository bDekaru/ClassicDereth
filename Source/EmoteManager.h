
#pragma once

class QueuedEmote
{
public:
	Emote _data;
	DWORD _target_id;
	double _executeTime;
};

class EmoteManager
{
public:
	EmoteManager(class CWeenieObject *weenie);
		
	std::string ReplaceEmoteText(const std::string &text, DWORD target_id, DWORD source_id);
	bool ChanceExecuteEmoteSet(EmoteCategory category, std::string msg, DWORD target_id);
	bool ChanceExecuteEmoteSet(EmoteCategory category, DWORD target_id);
	void ExecuteEmoteSet(const EmoteSet &emoteSet, DWORD target_id);
	void ExecuteEmote(const Emote &emote, DWORD target_id);
	void Tick();

	void Cancel();
	void OnDeath(DWORD killer_id);
	bool IsExecutingAlready();

protected:
	class CWeenieObject *_weenie = NULL;

	double _emoteEndTime;
	std::list<QueuedEmote> _emoteQueue;
};