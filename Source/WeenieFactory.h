
#pragma once

class CWeenieFactory : public ThreadedFileLoader
{
public:
	CWeenieFactory();
	virtual ~CWeenieFactory();

	void Reset();
	void Initialize();

	CWeenieObject *CreateWeenieByClassID(uint32_t wcid, const Position *pos = NULL, bool bSpawn = false);
	CWeenieObject *CreateWeenieByName(const char *name, const Position *pos = NULL, bool bSpawn = false);
	CWeenieObject *CloneWeenie(CWeenieObject *weenie);

	void AddWeenieToDestination(CWeenieObject *weenie, CWeenieObject *parent, uint32_t destinationType, bool isRegenLocationType, const GeneratorProfile *profile = NULL);
	int AddFromCreateList(CWeenieObject *parent, PackableListWithJson<CreationProfile> *createList, DestinationType validDestinationTypes = DestinationType::Undef_DestinationType);
	int AddFromGeneratorTable(CWeenieObject *parent, bool isInit);

	int GenerateFromTypeOrWcid(CWeenieObject *parent, const GeneratorProfile *profile);
	int GenerateFromTypeOrWcid(CWeenieObject *parent, RegenLocationType destinationType, uint32_t treasureType, unsigned int ptid = 0, float shade = 0.0f);
	int GenerateFromTypeOrWcid(CWeenieObject *parent, DestinationType destinationType, uint32_t treasureType, unsigned int ptid = 0, float shade = 0.0f);
	int GenerateRareItem(CWeenieObject *parent, CWeenieObject *killer);

	bool ApplyWeenieDefaults(CWeenieObject *weenie, uint32_t wcid);

	uint32_t GetWCIDByName(const char *name, int index = 0);

	CWeenieDefaults *GetWeenieDefaults(uint32_t wcid);
	CWeenieDefaults *GetWeenieDefaults(const char *name, int index = 0);

	bool UpdateWeenieBody(uint32_t wcid, Body* newBody);

	bool TryToResolveAppearanceData(CWeenieObject *weenie);

	CWeenieObject *CreateBaseWeenieByType(int weenieType, unsigned int wcid, const char *weenieName = "");

	uint32_t GetScrollSpellForWCID(uint32_t wcid);
	uint32_t GetWCIDForScrollSpell(uint32_t spell_id);

	void RefreshLocalStorage();
	bool RefreshLocalWeenie(uint32_t wcid);
	bool LoadLocalWeenieByWcid(uint32_t wcid);
	uint32_t m_FirstAvatarWCID = 0;
	uint32_t m_NumAvatars = 0;

	std::list<uint32_t> GetWCIDsWithMotionTable(uint32_t mtable);

protected:
	CWeenieObject *CreateWeenie(CWeenieDefaults *defaults, const Position *pos = NULL, bool bSpawn = false);

	void ApplyWeenieDefaults(CWeenieObject *weenie, CWeenieDefaults *defaults);

	void LoadLocalStorage(bool refresh = false);
	bool LoadLocalWeenie(uint32_t wcid);
	
	void LoadLocalStorageIndexed();
	void LoadAvatarData();

	void MapScrollWCIDs();

	std::unordered_map<uint32_t, CWeenieDefaults *> m_WeenieDefaults;
	std::multimap<std::string, CWeenieDefaults *> m_WeenieDefaultsByName;

	std::unordered_map<uint32_t, uint32_t> m_ScrollWeenies; // keyed by spell ID
};

