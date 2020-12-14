
#pragma once

typedef std::unordered_map<uint32_t, class AllegianceTreeNode *> AllegianceTreeNodeMap;
typedef std::unordered_map<uint32_t, class AllegianceInfo *> AllegianceInfoMap;

class AllegianceInfo : public PackObj
{
public:
	DECLARE_PACKABLE()

	AllegianceHierarchy _info;
};

class AllegianceTreeNode : public PackObj
{
public:
	AllegianceTreeNode();
	virtual ~AllegianceTreeNode();

	DECLARE_PACKABLE()

	AllegianceTreeNode *FindCharByNameRecursivelySlow(const std::string &charName);
	void FillAllegianceNode(AllegianceNode *node);
	void UpdateWithWeenie(CWeenieObject *weenie);
	
	unsigned int _charID = 0;
	std::string _charName;

	unsigned int _monarchID = 0;
	unsigned int _patronID = 0;
	HeritageGroup _hg = Invalid_HeritageGroup;
	Gender _gender = Invalid_Gender;
	unsigned int _rank = 0;
	unsigned int _level = 0;
	unsigned int _leadership = 0;
	unsigned int _loyalty = 0;
	unsigned int _numFollowers = 0;
	uint64_t _cp_cached = 0;
	uint64_t _cp_tithed = 0;

	uint64_t _cp_pool_to_unload = 0;
	uint64_t _unixTimeSwornAt = 0;
	uint64_t _ingameSecondsSworn = 0;
	AllegianceTreeNodeMap _vassals;
};

class AllegianceManager : public PackObj
{
public:
	AllegianceManager();
	virtual ~AllegianceManager();

	DECLARE_PACKABLE()
	
	void Load();
	void Save();
	void Tick();

	void CacheInitialDataRecursively(AllegianceTreeNode *node, AllegianceTreeNode *parent);
	void WalkTreeAndBumpOnlineTime(AllegianceTreeNode *node, int onlineSecondsDelta);
	void CacheDataRecursively(AllegianceTreeNode *node, AllegianceTreeNode *parent);
	void NotifyTreeRefreshRecursively(AllegianceTreeNode *node);

	AllegianceTreeNode *GetTreeNode(uint32_t charID);
	AllegianceInfo *GetInfo(uint32_t monarchID);

	void SetWeenieAllegianceQualities(CWeenieObject *weenie);
	AllegianceProfile *CreateAllegianceProfile(uint32_t char_id, unsigned int *pRank);
	void SendAllegianceProfile(CWeenieObject *pPlayer);
	void TrySwearAllegiance(CWeenieObject *source, CWeenieObject *target);
	void TryBreakAllegiance(CWeenieObject *source, uint32_t target_id);
	void TryBreakAllegiance(uint32_t source_id, uint32_t target_id);
	void BreakAllAllegiance(uint32_t char_id);

	void ChatMonarch(uint32_t sender_id, const char *text);
	void ChatPatron(uint32_t sender_id, const char *text);
	void ChatVassals(uint32_t sender_id, const char *text);
	void ChatCovassals(uint32_t sender_id, const char *text);

	AllegianceInfoMap _allegInfos;
	AllegianceTreeNodeMap _monarchs;
	AllegianceTreeNodeMap _directNodes;

	void HandleAllegiancePassup(uint32_t source_id, int64_t amount, bool direct);

	uint32_t GetCachedMonarchIDForPlayer(CPlayerWeenie *player);

	bool IsMonarch(AllegianceTreeNode* playerNode);
	bool IsOfficer(AllegianceTreeNode* playerNode);
	eAllegianceOfficerLevel GetOfficerLevel(std::string player_name);
	eAllegianceOfficerLevel GetOfficerLevel(uint32_t player_id);
	std::string GetOfficerTitle(std::string player_name);
	std::string GetOfficerTitle(uint32_t player_id);
	bool IsOfficerWithLevel(AllegianceTreeNode* playerNode, eAllegianceOfficerLevel min, eAllegianceOfficerLevel max = Castellan_AllegianceOfficerLevel);

	void SetMOTD(CPlayerWeenie * player, std::string msg);
	void LoginMOTD(CPlayerWeenie* player);
	void ClearMOTD(CPlayerWeenie * player);
	void QueryMOTD(CPlayerWeenie * player);

	void SetAllegianceName(CPlayerWeenie * player, std::string name);
	void ClearAllegianceName(CPlayerWeenie * player);
	void QueryAllegianceName(CPlayerWeenie * player);

	void SetOfficerTitle(CPlayerWeenie * player, int level, std::string title);
	void ClearOfficerTitles(CPlayerWeenie * player);
	void ListOfficerTitles(CPlayerWeenie * player);

	void SetOfficer(CPlayerWeenie * player, std::string officer_name, eAllegianceOfficerLevel level);
	void RemoveOfficer(CPlayerWeenie * player, std::string officer_name);
	void ListOfficers(CPlayerWeenie * player);
	void ClearOfficers(CPlayerWeenie* player);

	void AllegianceInfoRequest(CPlayerWeenie* player, std::string target_name);
	void AllegianceLockAction(CPlayerWeenie* player, uint32_t lock_action);
	void RecallHometown(CPlayerWeenie* player);

	void ApproveVassal(CPlayerWeenie * player, std::string vassal_name);
	void BootPlayer(CPlayerWeenie* player, std::string bootee, bool whole_account);
	
	bool IsGagged(uint32_t player_id);
	void ChatGag(CPlayerWeenie* player, std::string target, bool toggle);
	void ChatBoot(CPlayerWeenie* player, std::string target, std::string reason);

	bool IsBanned(uint32_t player_to_check_id, uint32_t monarch_id);
	void AddBan(CPlayerWeenie * player, std::string char_name);
	void RemoveBan(CPlayerWeenie * player, std::string char_name);
	void GetBanList(CPlayerWeenie * player);

	bool allAllegiancesUpdated = false;

	std::string GetRankTitle(Gender gender, HeritageGroup heritage, unsigned int rank);

	const std::vector<std::string> AluvianMaleRanks = { "Yeoman", "Baronet","Baron","Reeve","Thane","Ealdor","Duke","Aetheling","King","High King" };
	const std::vector<std::string> AluvianFemaleRanks = { "Yeoman", "Baronet","Baroness","Reeve","Thane","Ealdor","Duchess","Aetheling","Queen","High Queen" };
	const std::vector<std::string> GharundimMaleRanks = { "Sayyid", "Shayk","Maulan","Mu'allim","Naqib","Qadi","Mushir","Amir","Malik","Sultan" };
	const std::vector<std::string> GharundimFemaleRanks = { "Sayyida", "Shayka","Maulana","Mu'allima","Naqiba","Qadiya","Mushira","Amira","Malika","Sultana" };
	const std::vector<std::string> ShoMaleRanks = { "Jinin", "Jo-Chueh","Nan-Chueh","Shi-Chueh","Ta-Chueh","Kun-Chueh","Kou","Taikou","Ou","Koutei" };
	const std::vector<std::string> ShoFemaleRanks = { "Jinin", "Jo-Chueh","Nan-Chueh","Shi-Chueh","Ta-Chueh","Kun-Chueh","Kou","Taikou","Jo-Ou","Koutei" };
	const std::vector<std::string> ViamontianMaleRanks = { "Squire", "Banner","Baron","Viscount","Count","Marquis","Duke","Grand Duke","King","High King" };
	const std::vector<std::string> ViamontianFemaleRanks = { "Dame", "Banner","Baroness","Viscountess","Countess","Marquise","Duchess","Grand Duchess","Queen","High Queen" };
	const std::vector<std::string> ShadowMaleRanks = { "Tenebrous", "Shade","Squire","Knight","Void Knight","Void Lord","Duke","Archduke","Highborn","King" };
	const std::vector<std::string> ShadowFemaleRanks = { "Tenebrous", "Shade","Squire","Knight","Void Knight","Void Lady","Duchess","Archduchess","Highborn","Queen" };
	const std::vector<std::string> GearKnightMaleRanks = { "Tribunus", "Praefectus","Optio","Centurion","Principes","Legatus","Consul","Dux","Secondus","Primus" };
	const std::vector<std::string> GearKnightFemaleRanks = { "Tribunus", "Praefectus","Optio","Centurion","Principes","Legatus","Consul","Dux","Secondus","Primus" };
	const std::vector<std::string> UndeadMaleRanks = { "Neophyte", "Acolyte","Adept","Esquire","Squire","Knight","Count","Viscount","Highness","Anointed" };
	const std::vector<std::string> UndeadFemaleRanks = { "Neophyte", "Acolyte","Adept","Esquire","Squire","Knight","Countess","Viscountess","Highness","Anointed" };
	const std::vector<std::string> EmpyreanMaleRanks = { "Ensign", "Corporal","Lieutenant","Commander","Captain","Commodore","Admiral","Warlord","Ipharsin","Aulin" };
	const std::vector<std::string> EmpyreanFemaleRanks = { "Ensign", "Corporal","Lieutenant","Commander","Captain","Commodore","Admiral","Warlord","Ipharsia","Aulia" };
	const std::vector<std::string> TumerokMaleRanks = { "Xutua", "Tuona","Ona","Nuona","Turea","Rea","Nurea","Kauh","Sutah","Tah" };
	const std::vector<std::string> TumerokFemaleRanks = { "Xutua", "Tuona","Ona","Nuona","Turea","Rea","Nurea","Kauh","Sutah","Tah" };
	const std::vector<std::string> LugianMaleRanks = { "Laigus", "Raigus","Amploth","Arintoth","Obeloth","Lithos","Kantos","Gigas","Extas","Tiatus" };
	const std::vector<std::string> LugianFemaleRanks = { "Laigus", "Raigus","Amploth","Arintoth","Obeloth","Lithos","Kantos","Gigas","Extas","Tiatus" };

private:
	void BreakAllegiance(AllegianceTreeNode *patron, AllegianceTreeNode *vassal);

	bool ShouldRemoveAllegianceNode(AllegianceTreeNode *node);
	void RemoveAllegianceNode(AllegianceTreeNode *node);

	double m_LastSave = 0.0;
	double m_LastGagCheck = 0.0;
};