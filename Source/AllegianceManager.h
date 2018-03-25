
#pragma once

typedef std::unordered_map<DWORD, class AllegianceTreeNode *> AllegianceTreeNodeMap;
typedef std::unordered_map<DWORD, class AllegianceInfo *> AllegianceInfoMap;

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
	unsigned __int64 _cp_cached = 0;
	unsigned __int64 _cp_tithed = 0;

	unsigned __int64 _cp_pool_to_unload = 0;

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
	void CacheDataRecursively(AllegianceTreeNode *node, AllegianceTreeNode *parent);
	void NotifyTreeRefreshRecursively(AllegianceTreeNode *node);

	AllegianceTreeNode *GetTreeNode(DWORD charID);
	AllegianceInfo *GetInfo(DWORD monarchID);

	void SetWeenieAllegianceQualities(CWeenieObject *weenie);
	AllegianceProfile *CreateAllegianceProfile(DWORD char_id, unsigned int *pRank);
	void SendAllegianceProfile(CWeenieObject *pPlayer);
	int TrySwearAllegiance(CWeenieObject *source, CWeenieObject *target);
	int TryBreakAllegiance(CWeenieObject *source, DWORD target_id);
	void BreakAllAllegiance(DWORD char_id);

	void ChatMonarch(DWORD sender_id, const char *text);
	void ChatPatron(DWORD sender_id, const char *text);
	void ChatVassals(DWORD sender_id, const char *text);
	void ChatCovassals(DWORD sender_id, const char *text);

	AllegianceInfoMap _allegInfos;
	AllegianceTreeNodeMap _monarchs;
	AllegianceTreeNodeMap _directNodes;

	void HandleAllegiancePassup(DWORD source_id, long long amount, bool direct);

	DWORD GetCachedMonarchIDForPlayer(CPlayerWeenie *player);

private:
	void BreakAllegiance(AllegianceTreeNode *patron, AllegianceTreeNode *vassal);

	bool ShouldRemoveAllegianceNode(AllegianceTreeNode *node);
	void RemoveAllegianceNode(AllegianceTreeNode *node);

	double m_NextSave = 0.0;
};