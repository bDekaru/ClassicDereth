
#pragma once

class CWeenieFactory
{
public:
	CWeenieFactory();
	virtual ~CWeenieFactory();

	void Reset();
	void Initialize();

	CWeenieObject *CreateWeenieByClassID(DWORD wcid, const Position *pos = NULL, bool bSpawn = false);
	CWeenieObject *CreateWeenieByName(const char *name, const Position *pos = NULL, bool bSpawn = false);
	CWeenieObject *CloneWeenie(CWeenieObject *weenie);

	void AddWeenieToDestination(CWeenieObject *weenie, CWeenieObject *parent, DWORD destinationType, bool isRegenLocationType, const GeneratorProfile *profile = NULL);
	int AddFromCreateList(CWeenieObject *parent, PackableListWithJson<CreationProfile> *createList, DestinationType validDestinationTypes = DestinationType::Undef_DestinationType);
	int AddFromGeneratorTable(CWeenieObject *parent, bool isInit);

	int GenerateFromTypeOrWcid(CWeenieObject *parent, const GeneratorProfile *profile);
	int GenerateFromTypeOrWcid(CWeenieObject *parent, RegenLocationType destinationType, DWORD treasureType, unsigned int ptid = 0, float shade = 0.0f);
	int GenerateFromTypeOrWcid(CWeenieObject *parent, DestinationType destinationType, DWORD treasureType, unsigned int ptid = 0, float shade = 0.0f);

	bool ApplyWeenieDefaults(CWeenieObject *weenie, DWORD wcid);

	DWORD GetWCIDByName(const char *name, int index = 0);

	CWeenieDefaults *GetWeenieDefaults(DWORD wcid);
	CWeenieDefaults *GetWeenieDefaults(const char *name, int index = 0);
	
	bool TryToResolveAppearanceData(CWeenieObject *weenie);

	CWeenieObject *CreateBaseWeenieByType(int weenieType, unsigned int wcid, const char *weenieName = "");

	DWORD GetScrollSpellForWCID(DWORD wcid);
	DWORD GetWCIDForScrollSpell(DWORD spell_id);

	void RefreshLocalStorage();

	DWORD m_FirstAvatarWCID = 0;
	DWORD m_NumAvatars = 0;

	std::list<DWORD> GetWCIDsWithMotionTable(DWORD mtable);

protected:
	CWeenieObject *CreateWeenie(CWeenieDefaults *defaults, const Position *pos = NULL, bool bSpawn = false);

	void ApplyWeenieDefaults(CWeenieObject *weenie, CWeenieDefaults *defaults);

	void LoadLocalStorage(bool refresh = false);
	void LoadLocalStorageIndexed();
	void LoadAvatarData();

	void MapScrollWCIDs();

	std::unordered_map<DWORD, CWeenieDefaults *> m_WeenieDefaults;
	std::multimap<std::string, CWeenieDefaults *> m_WeenieDefaultsByName;

	std::unordered_map<DWORD, DWORD> m_ScrollWeenies; // keyed by spell ID
};

