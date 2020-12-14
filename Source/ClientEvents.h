
#pragma once
#include "BinaryReader.h"

class CPlayerWeenie;

class TradeManager;

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
	uint32_t GetPlayerID();
	CPlayerWeenie* GetPlayer();

	void LoginError(int iError);
	void LoginCharacter(uint32_t dwGUID, const char *szAccount);
	void BeginLogout();
	void OnLogoutCompleted();
	void ExitWorld();
	void ForceLogout();

	void ActionComplete(int error = WERROR_NONE);
	void SendText(const char *szText, int32_t lColor);

	bool IsServerGagged();
	bool IsAllegGagged();

	bool CheckForChatSpam();

	// Network events
	void ActionText(const char* szText);
	void Attack(uint32_t dwTarget, uint32_t dwHeight, float flPower);
	void MissileAttack(uint32_t dwTarget, uint32_t dwHeight, float flPower);
	void ChangeCombatStance(COMBAT_MODE mode);
	void ChannelText(uint32_t dwChannel, const char* szText);
	void ClientText(const char* szText);
	void EmoteText(const char* szText);
	void ExitPortal();
	void Identify(uint32_t dwObjectID);
	void LifestoneRecall();
	void MarketplaceRecall();
	void PKArenaRecall();
	void PKLArenaRecall();
	void Ping();
	void RequestHealthUpdate(uint32_t dwGUID);
	void SendTellByGUID(const char *text, uint32_t dwGUID);
	void SendTellByName(const char *text, const char *name);
	void SpendAttributeXP(STypeAttribute key, uint32_t exp);
	void SpendAttribute2ndXP(STypeAttribute2nd key, uint32_t exp);
	void SpendSkillXP(STypeSkill key, uint32_t exp);
	void SpendSkillCredits(STypeSkill key, uint32_t credits);
	void TryBuyItems(uint32_t vendor_id, std::list<class ItemProfile *> &items);
	void TrySellItems(uint32_t vendor_id, std::list<class ItemProfile *> &items);
	void TryInscribeItem(uint32_t object_id, const std::string &text);
	void UseItemEx(uint32_t dwSourceID, uint32_t dwDestID);
	void UseObject(uint32_t dwEID);
	void SendTellByGUID(const char* szText, const CPlayerWeenie *pTarget);
	void SendTell(const char* szText, const char* targetName, const uint32_t targetId = 0);
	void GetHousesAvailable(uint32_t houseType);

	void ProcessEvent(BinaryReader *);

	// Fellowship functionality
	void TryFellowshipCreate(const std::string name, int shareXP);
	void TryFellowshipQuit(int disband);
	void TryFellowshipDismiss(uint32_t dismissed);
	void TryFellowshipRecruit(uint32_t target);
	void TryFellowshipUpdate(int on);
	void TryFellowshipAssignNewLeader(uint32_t target);
	void TryFellowshipChangeOpenness(int open);
	void SendFellowshipUpdate();
	void SetCharacterSquelchSetting(bool squelchSet, uint32_t squelchPlayer, std::string squelchName, BYTE squelchChatType, bool account);
	void SendSquelchDB();
	void SendGearRatings();

	// Allegiance functionality
	void TrySwearAllegiance(uint32_t target);
	void TryBreakAllegiance(uint32_t target);
	void SetRequestAllegianceUpdate(bool on);
	void SendAllegianceUpdate();
	void SendAllegianceMOTD();
	void AllegianceInfoRequest(const std::string &target);
	void TrySetAllegianceMOTD(const std::string &text);
	void AllegianceHometownRecall();

	// Player Options
	void ChangePlayerOption(PlayerOptions option, bool value);

	// House
	void HouseBuy(uint32_t slumlord, const PackableList<uint32_t> &items);
	void HouseRent(uint32_t slumlord, const PackableList<uint32_t> &items);
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
	void NoLongerViewingContents(uint32_t container_id);

	void PKLiteEnable();

private:
	CClient *m_pClient;

	CPlayerWeenie *m_pPlayer;

	WORD m_MoveActionStamp = 0xFFFF;

	double m_fNextAllegianceUpdate = 0.0;
	BOOL m_bSendAllegianceUpdates = FALSE;
	BOOL m_bSentFirstAllegianceUpdate = FALSE;
	int last_age_update = 0;

	double _next_chat_interval = 0.0;
	double _next_chat_allowed = 0.0;
	unsigned int _num_chat_sent = 0;

	double _next_allowed_identify = 0.0;
};



