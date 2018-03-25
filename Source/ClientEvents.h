
#pragma once
#include "BinaryReader.h"

class CPlayerWeenie;

const float sidestep_factor = 0.5f;
const float backwards_factor = 0.64999998f;
const float run_turn_factor = 1.5f;
const float run_anim_speed = 4.0f;
const float walk_anim_speed = 3.1199999f;
const float sidestep_anim_speed = 1.25f;
const float max_sidestep_anim_rate = 3.0f;

// Client/World interaction
class CClientEvents
{
public:
	CClientEvents(CClient *);
	~CClientEvents();

	void Think();

	void DetachPlayer();
	DWORD GetPlayerID();
	CPlayerWeenie* GetPlayer();

	void LoginError(int iError);
	void LoginCharacter(DWORD dwGUID, const char *szAccount);
	void BeginLogout();
	void OnLogoutCompleted();
	void ExitWorld();

	void ActionComplete(int error = WERROR_NONE);
	void SendText(const char *szText, long lColor);

	bool CheckForChatSpam();

	// Network events
	void ActionText(const char* szText);
	void Attack(DWORD dwTarget, DWORD dwHeight, float flPower);
	void MissileAttack(DWORD dwTarget, DWORD dwHeight, float flPower);
	void ChangeCombatStance(COMBAT_MODE mode);
	void ChannelText(DWORD dwChannel, const char* szText);
	void ClientText(const char* szText);
	void EmoteText(const char* szText);
	void ExitPortal();
	void Identify(DWORD dwObjectID);
	void LifestoneRecall();
	void MarketplaceRecall();
	void Ping();
	void RequestHealthUpdate(DWORD dwGUID);
	void SendTellByGUID(const char *text, DWORD dwGUID);
	void SendTellByName(const char *text, const char *name);
	void SpendAttributeXP(STypeAttribute key, DWORD exp);
	void SpendAttribute2ndXP(STypeAttribute2nd key, DWORD exp);
	void SpendSkillXP(STypeSkill key, DWORD exp);
	void SpendSkillCredits(STypeSkill key, DWORD credits);
	void TryBuyItems(DWORD vendor_id, std::list<class ItemProfile *> &items);
	void TrySellItems(DWORD vendor_id, std::list<class ItemProfile *> &items);
	void TryInscribeItem(DWORD object_id, const std::string &text);
	void UseItemEx(DWORD dwSourceID, DWORD dwDestID);
	void UseObject(DWORD dwEID);

	void ProcessEvent(BinaryReader *);

	// Fellowship functionality
	void TryFellowshipCreate(const std::string name, int shareXP);
	void TryFellowshipQuit(int disband);
	void TryFellowshipDismiss(DWORD dismissed);
	void TryFellowshipRecruit(DWORD target);
	void TryFellowshipUpdate(int on);
	void TryFellowshipAssignNewLeader(DWORD target);
	void TryFellowshipChangeOpenness(int open);
	void SendFellowshipUpdate();

	// Allegiance functionality
	void TrySwearAllegiance(DWORD target);
	void TryBreakAllegiance(DWORD target);
	void SetRequestAllegianceUpdate(int on);
	void SendAllegianceUpdate();
	void SendAllegianceMOTD();
	void AllegianceInfoRequest(const std::string &target);
	void TrySetAllegianceMOTD(const std::string &text);
	void AllegianceHometownRecall();

	// Player Options
	void ChangePlayerOption(PlayerOptions option, bool value);

	// House
	void HouseBuy(DWORD slumlord, const PackableList<DWORD> &items);
	void HouseRent(DWORD slumlord, const PackableList<DWORD> &items);
	void HouseAbandon();
	void HouseRequestData();

	void HouseToggleHooks(bool newSetting);

	void HouseRecall();
	void HouseMansionRecall();

	void HouseRequestAccessList();
	
	void HouseAddPersonToAccessList(std::string name);
	void HouseRemovePersonFromAccessList(std::string name);
	void HouseAddOrRemovePersonToStorageList(std::string name, bool isAdd);

	void HouseAddOrRemoveAllegianceToAccessList(bool isAdd);
	void HouseAddOrRemoveAllegianceToStorageList(bool isAdd);
	
	void HouseToggleOpenAccess(bool newSetting);
	void HouseToggleOpenStorageAccess();

	void HouseClearAccessList();
	void HouseClearStorageAccess();

	// Remote Container
	void NoLongerViewingContents(DWORD container_id);

private:
	CClient *m_pClient;

	CPlayerWeenie *m_pPlayer;

	WORD m_MoveActionStamp = 0xFFFF;

	double m_fNextAllegianceUpdate = 0.0;
	BOOL m_bSendAllegianceUpdates = FALSE;
	BOOL m_bSentFirstAllegianceUpdate = FALSE;

	double _next_chat_interval = 0.0;
	double _next_chat_allowed = 0.0;
	unsigned int _num_chat_sent = 0;

	double _next_allowed_identify = 0.0;
};



