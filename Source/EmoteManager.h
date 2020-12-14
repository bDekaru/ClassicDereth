
#pragma once

class QueuedEmote
{
public:
	Emote _data;
	uint32_t _target_id;
	double _executeTime;
};

class EmoteManager
{
public:
	EmoteManager(class CWeenieObject *weenie);
		
	std::string ReplaceEmoteText(const std::string &text, uint32_t target_id, uint32_t source_id);
	void killTask(std::string mobName, std::string kCountName, uint32_t target_id);
	void killTaskSub(std::string &mobName, std::string &kCountName, CWeenieObject *targormember);
	bool ChanceExecuteEmoteSet(EmoteCategory category, std::string msg, uint32_t target_id);
	bool ChanceExecuteEmoteSet(EmoteCategory category, uint32_t target_id);
	void ExecuteEmoteSet(const EmoteSet &emoteSet, uint32_t target_id);
	void ExecuteEmote(const Emote &emote, uint32_t target_id);
	void ConfirmationResponse(bool accepted, uint32_t target_id);
	void Tick();

	void Cancel();
	void OnDeath(uint32_t killer_id);
	bool IsExecutingAlready(); 
	bool HasQueue();

	std::map<uint32_t, std::string> _confirmMsgList;

	void SkillRefundValidationLog(uint32_t target_id, STypeSkill skillToAlter, SKILL_ADVANCEMENT_CLASS debugSkillSacInit, int debugInitAvailCredits); //Validation Debug

protected:
	class CWeenieObject *_weenie = NULL;

	double _emoteEndTime = 0;
	std::list<QueuedEmote> _emoteQueue;
};
