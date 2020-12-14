
#pragma once

class CKeyValueConfig
{
public:
	CKeyValueConfig(const char *filepath = "");
	virtual ~CKeyValueConfig();

	void Destroy();

	bool Load();
	bool Load(const char *filepath);
	
	const char *GetValue(const char *valuename, const char *defaultvalue = "");
	const char *GetConfigPath() { return m_strFile.c_str(); }

private:
	virtual void PostLoad() { }

	std::string m_strFile;
	std::map<std::string, std::string> m_KeyValues;
	
};

class CPhatACServerConfig : public CKeyValueConfig
{
public:
	CPhatACServerConfig(const char *filepath = "");
	virtual ~CPhatACServerConfig() override;
	
	unsigned long BindIP() { return m_BindIP; }
	unsigned int BindPort() { return m_BindPort; }
	unsigned int NetBufferSize() { return m_netBufferSize; }

	const char *DatabaseIP() { return m_DatabaseIP.c_str(); }
	unsigned int DatabasePort() { return m_DatabasePort; }
	const char *DatabaseUsername() { return m_DatabaseUsername.c_str(); }
	const char *DatabasePassword() { return m_DatabasePassword.c_str(); }
	const char *DatabaseName() { return m_DatabaseName.c_str(); }

	const char *WorldName() { return m_WorldName.c_str(); }
	const char *WelcomePopup() { return m_WelcomePopup.c_str(); }
	const char *WelcomeMessage() { return m_WelcomeMessage.c_str(); }
	virtual void SetWelcomeMessage(std::string message);

	virtual bool FastTick() { return m_bFastTick; }
	virtual bool UseIncrementalID() { return m_bUseIncrementalIDs; }
	virtual int32_t IDScanType() { return m_idScanType; }

	virtual bool HardcoreMode() { return m_bHardcoreMode; }
	virtual bool HardcoreModePlayersOnly() { return m_bHardcoreModePlayersOnly; }
	virtual bool PKOnly() { return m_bPKOnly; }
	virtual bool ColoredSentinels() { return m_bColoredSentinels; }

	virtual bool SpawnLandscape() { return m_bSpawnLandscape; }
	virtual bool SpawnStaticCreatures() { return m_bSpawnStaticCreatures; }
	virtual bool EverythingUnlocked() { return m_bEverythingUnlocked; }
	virtual bool TownCrierBuffs() { return m_bTownCrierBuffs; }
	virtual unsigned int TownCrierBuffLevel() { return m_TownCrierBuffLevel; };
	virtual bool TownCrierBuffBanes() { return m_bTownCrierBuffBanes; }

	virtual bool EnableTeleCommands() { return m_bEnableTeleCommands; }
	virtual bool EnableXPCommands() { return m_bEnableXPCommands; }
	virtual bool EnableAttackableCommand() { return m_bEnableAttackableCommand; }
	virtual bool EnableGodlyCommand() { return m_bEnableGodlyCommand; }

	virtual double GetMultiplierForQuestTime(int questTime);
	virtual int UseMultiplierForQuestTime(int questTime);

	virtual double GetKillXPMultiplier(int level);
	virtual double GetRewardXPMultiplier(int level);
	virtual double GetLumXPMultiplier();
	virtual void SetGlobalXpMultiplier(double mult);
	virtual void SetKillXpMulitplier(int tier, double mult);
	virtual void SetRewardXpMultiplier(int tier, double mult);
	virtual void SetLumXpMultiplier(double mult);


	virtual double DropRateMultiplier() { return m_fDropRateMultiplier; }
	virtual double RespawnTimeMultiplier() { return m_fRespawnTimeMultiplier; }
	virtual bool IsSpellFociEnabled() { return m_bSpellFociEnabled; }

	virtual bool AutoCreateAccounts() { return m_bAutoCreateAccounts; }

	virtual unsigned int GetOverrideMaxLevel() { return m_OverrideMaxLevel; };
	virtual unsigned int GetOverrideStartingXP() { return m_OverrideStartingXP; };
	virtual bool DisableUnassignedXPAtMaxLevel() { return m_bDisableUnassignedXPAtMaxLevel; };
	virtual double VitaeXPMultiplier() { return m_fVitaeXPMultiplier; }
	
	virtual bool EnableSpellFociStarterPack() { return m_bEnableSpellFociStarterPack; };
	virtual unsigned int PrimeNewCharacterSpellbook() { return m_PrimeNewCharacterSpellbook; };

	virtual unsigned int MaxDormantLandblocks() { return m_MaxDormantLandblocks; }
	virtual unsigned int DormantLandblockCleanupTime() { return m_DormantLandblockCleanupTime; }

	virtual bool ShowLogins() { return m_bShowLogins; }
	virtual bool SpeedHackKicking() { return m_bSpeedHackKicking; }
	virtual double SpeedHackKickThreshold() { return m_fSpeedHackKickThreshold; }
	virtual double VoidDamageReduction() { return m_fVoidReduction; }
	virtual void SetVoidDamageReduction(double voidReduction) { m_fVoidReduction = voidReduction; }
	virtual bool ShowDeathMessagesGlobally() { return m_bShowDeathMessagesGlobally; }
	virtual bool ShowPlayerDeathMessagesGlobally() { return m_bShowPlayerDeathMessagesGlobally; }

	virtual const char *HoltburgStartPosition() { return m_HoltburgStartPosition.c_str(); }
	virtual const char *YaraqStartPosition() { return m_YaraqStartPosition.c_str(); }
	virtual const char *ShoushiStartPosition() { return m_ShoushiStartPosition.c_str(); }
	virtual const char *SanamarStartPosition() { return m_SanamarStartPosition.c_str(); }
	virtual const char *OlthoiStartPosition() { return m_OlthoiStartPosition.c_str(); }

	virtual int PKRespiteTime() { return m_PKRespiteTime; }
	virtual bool SpellPurgeOnLogin() { return m_bSpellPurgeOnLogin; }
	virtual bool UpdateAllegianceData() { return m_bUpdateAllegianceData; }
	virtual bool InventoryPurgeOnLogin() { return m_bInventoryPurgeOnLogin; }
	virtual bool RatingPurgeOnLogin() { return m_bRatingPurgeOnLogin; }
	virtual float MissileAttributeAdjust() { return m_missileAttribAdjust; }
	void SetMissileAttributeAdjust(float value) { m_missileAttribAdjust = value; }

	virtual unsigned int WcidForPurge() { return m_WcidForPurge; }
	virtual unsigned int GetMaxCorpses() { return m_maxCorpses; }

	virtual bool AllowGeneralChat() { return m_bAllowGeneralChat; }

	virtual double RareDropMultiplier() { return m_fRareDropMultiplier; }
	virtual bool RealTimeRares() { return m_bRealTimeRares; }

	virtual bool LoginAtLS() { return m_bLoginAtLS; }
	virtual bool CreateTemplates() { return m_bCreateTemplates; }
	virtual bool AllowPKCommands() { return m_bAllowPKCommands; }
	virtual bool AllowOlthoi() { return m_bAllowOlthoi; }

	virtual bool FixOldChars() { return m_bFixOldChars; }
	const char *GetBanString() { return m_BanString.c_str(); }
	virtual bool ShowDotSpells() { return m_bShowDotSpells; }

	virtual float GetFellowHealthThreshold() { return m_fellowHealthThreshold; }
	virtual float GetFellowStamThreshold() { return m_fellowStamThreshold; }
	virtual float GetFellowManaThreshold() { return m_fellowManaThreshold; }

	virtual uint32_t GetDeleteCharLifespan() { return m_defaultsecondstofullchardelete; }

	float GetPkCSMeleeBaseChance() { return pkCSmeleebasechance; }
	void SetPkCSMeleeBaseChance(float newValue) { pkCSmeleebasechance = max(0.0f, newValue); }
	uint32_t GetPkCSMeleeMinSkill() { return pkCSmeleeminskill; }
	void SetPkCSMeleeMinSkill(uint32_t newValue) { pkCSmeleeminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCSMeleeMaxSkill() { return pkCSmeleemaxskill; }
	void SetPkCSMeleeMaxSkill(uint32_t newValue) { pkCSmeleemaxskill = max((uint32_t)1, newValue); }
	float GetPkCSMeleeMaxChance() { return pkCSmeleemaxchance; }
	void SetPkCSMeleeMaxChance(float newValue) { pkCSmeleemaxchance = max(0.0f, newValue); }

	float GetPkCBMeleeBaseMult() { return pkCBmeleebasemult; }
	void SetPkCBMeleeBaseMult(float newValue) { pkCBmeleebasemult = max(0.0f, newValue); }
	uint32_t GetPkCBMeleeMinSkill() { return pkCBmeleeminskill; }
	void SetPkCBMeleeMinSkill(uint32_t newValue) { pkCBmeleeminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCBMeleeMaxSkill() { return pkCBmeleemaxskill; }
	void SetPkCBMeleeMaxSkill(uint32_t newValue) { pkCBmeleemaxskill = max((uint32_t)1, newValue); }
	float GetPkCBMeleeMaxMult() { return pkCBmeleemaxmult; }
	void SetPkCBMeleeMaxMult(float newValue) { pkCBmeleemaxmult = max(0.0f, newValue); }

	float GetPkCSMissileBaseChance() { return pkCSmissilebasechance; }
	void SetPkCSMissileBaseChance(float newValue) { pkCSmissilebasechance = max(0.0f, newValue); }
	uint32_t GetPkCSMissileMinSkill() { return pkCSmissileminskill; }
	void SetPkCSMissileMinSkill(uint32_t newValue) { pkCSmissileminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCSMissileMaxSkill() { return pkCSmissilemaxskill; }
	void SetPkCSMissileMaxSkill(uint32_t newValue) { pkCSmissilemaxskill = max((uint32_t)1, newValue); }
	float GetPkCSMissileMaxChance() { return pkCSmissilemaxchance; }
	void SetPkCSMissileMaxChance(float newValue) { pkCSmissilemaxchance = max(0.0f, newValue); }

	float GetPkCBMissileBaseMult() { return pkCBmissilebasemult; }
	void SetPkCBMissileBaseMult(float newValue) { pkCBmissilebasemult = max(0.0f, newValue); }
	uint32_t GetPkCBMissileMinSkill() { return pkCBmissileminskill; }
	void SetPkCBMissileMinSkill(uint32_t newValue) { pkCBmissileminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCBMissileMaxSkill() { return pkCBmissilemaxskill; }
	void SetPkCBMissileMaxSkill(uint32_t newValue) { pkCBmissilemaxskill = max((uint32_t)1, newValue); }
	float GetPkCBMissileMaxMult() { return pkCBmissilemaxmult; }
	void SetPkCBMissileMaxMult(float newValue) { pkCBmissilemaxmult = max(0.0f, newValue); }

	float GetPkCSMagicBaseChance() { return pkCSmagicbasechance; }
	void SetPkCSMagicBaseChance(float newValue) { pkCSmagicbasechance = max(0.0f, newValue); }
	uint32_t GetPkCSMagicMinSkill() { return pkCSmagicminskill; }
	void SetPkCSMagicMinSkill(uint32_t newValue) { pkCSmagicminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCSMagicMaxSkill() { return pkCSmagicmaxskill; }
	void SetPkCSMagicMaxSkill(uint32_t newValue) { pkCSmagicmaxskill = max((uint32_t)1, newValue); }
	float GetPkCSMagicMaxChance() { return pkCSmagicmaxchance; }
	void SetPkCSMagicMaxChance(float newValue) { pkCSmagicmaxchance = max(0.0f, newValue); }

	float GetPkCBMagicBaseMult() { return pkCBmagicbasemult; }
	void SetPkCBMagicBaseMult(float newValue) { pkCBmagicbasemult = max(0.0f, newValue); }
	uint32_t GetPkCBMagicMinSkill() { return pkCBmagicminskill; }
	void SetPkCBMagicMinSkill(uint32_t newValue) { pkCBmagicminskill = max((uint32_t)1, newValue); }
	uint32_t GetPkCBMagicMaxSkill() { return pkCBmagicmaxskill; }
	void SetPkCBMagicMaxSkill(uint32_t newValue) { pkCBmagicmaxskill = max((uint32_t)1, newValue); }
	float GetPkCBMagicMaxMult() { return pkCBmagicmaxmult; }
	void SetPkCBMagicMaxMult(float newValue) { pkCBmagicmaxmult = max(0.0f, newValue); }

	bool AllowCoalDispel() { return m_bAllowCoalDispel; } // TODO add set

	virtual float GetAllegianceGagTime() { return m_defaultallegchatgagtime; }

	virtual float GetBlueSigilProcRate() { return m_bluesigilprocrate; }
	void SetBlueSigilProcRate(float value);
	virtual float GetYellowSigilProcRate() { return m_yellowsigilprocrate; }
	void SetYellowSigilProcRate(float value);
	virtual float GetRedSigilProcRate() { return m_redsigilprocrate; }
	void SetRedSigilProcRate(float value);

	virtual float GetCloakBaseProcRate() { return m_cloakbaseprocrate; }
	void SetCloakBaseProcRate(float value);
	virtual float GetCloakHalfHealthProcRate() { return m_cloakhalfhealthprocbonus; }
	void SetCloakHalfHealthProcRate(float value);
	virtual float GetCloakQuarterHealthProcRate() { return m_cloakquarterhealthprocbonus; }
	void SetCloakQuarterHealthProcRate(float value);
	virtual float GetCloakTenthHealthProcRate() { return m_cloaktenthhealthprocbonus; }
	void SetCloakTenthHealthProcRate(float value);
	virtual float GetCloakPerLevelProcRate() { return m_cloakperlevelbonus; }
	void SetCloakPerLevelProcRate(float value);

protected:
	virtual void PostLoad() override;

	unsigned long m_BindIP = 0;
	unsigned int m_BindPort = 0;

	unsigned int m_netBufferSize = 0;

	std::string m_DatabaseIP;
	unsigned int m_DatabasePort = 0;
	std::string m_DatabaseUsername;
	std::string m_DatabasePassword;
	std::string m_DatabaseName;

	std::string m_WorldName;
	std::string m_WelcomePopup;
	std::string m_WelcomeMessage;

	bool m_bFastTick = false;
	bool m_bUseIncrementalIDs = false;
	int32_t m_idScanType = 0;

	bool m_bHardcoreMode = false;
	bool m_bHardcoreModePlayersOnly = false;
	bool m_bPKOnly = false;
	bool m_bColoredSentinels = false;
	bool m_bSpawnLandscape = true;
	bool m_bSpawnStaticCreatures = true;
	bool m_bEverythingUnlocked = true;
	bool m_bTownCrierBuffs = true;
	unsigned int m_TownCrierBuffLevel = 7;
	bool m_bTownCrierBuffBanes = true;

	bool m_bEnableTeleCommands = false;
	bool m_bEnableXPCommands = false;
	bool m_bEnableAttackableCommand = false;
	bool m_bEnableGodlyCommand = false;
	
	double m_fQuestMultiplierLessThan1Day = 1.0;
	double m_fQuestMultiplier1Day = 1.0;
	double m_fQuestMultiplier3Day = 1.0;
	double m_fQuestMultiplier7Day = 1.0;
	double m_fQuestMultiplier14Day = 1.0;
	double m_fQuestMultiplier30Day = 1.0;
	double m_fQuestMultiplier60Day = 1.0;

	double m_fKillXPMultiplierT1 = 1.0;
	double m_fKillXPMultiplierT2 = 1.0;
	double m_fKillXPMultiplierT3 = 1.0;
	double m_fKillXPMultiplierT4 = 1.0;
	double m_fKillXPMultiplierT5 = 1.0;
	double m_fKillXPMultiplierT6 = 1.0;

	double m_fRewardXPMultiplierT1 = 1.0;
	double m_fRewardXPMultiplierT2 = 1.0;
	double m_fRewardXPMultiplierT3 = 1.0;
	double m_fRewardXPMultiplierT4 = 1.0;
	double m_fRewardXPMultiplierT5 = 1.0;
	double m_fRewardXPMultiplierT6 = 1.0;

	double m_fLumXPMultiplier = 1.0;

	double m_fDropRateMultiplier = 1.0;
	double m_fRespawnTimeMultiplier = 1.0;
	bool m_bSpellFociEnabled = true;

	bool m_bAutoCreateAccounts = true;

	unsigned int m_OverrideMaxLevel = 126;
	unsigned int m_OverrideStartingXP = 0;
	bool m_bDisableUnassignedXPAtMaxLevel = false;
	double m_fVitaeXPMultiplier = 1.0;

	bool m_bEnableSpellFociStarterPack = false;
	unsigned int m_PrimeNewCharacterSpellbook = 0;

	unsigned int m_MaxDormantLandblocks = 1000;
	unsigned int m_DormantLandblockCleanupTime = 1800;

	bool m_bShowLogins = true;
	bool m_bSpeedHackKicking = true;
	double m_fSpeedHackKickThreshold = 1.2;
	double m_fVoidReduction = 1.0f;

	bool m_bShowDeathMessagesGlobally = false;
	bool m_bShowPlayerDeathMessagesGlobally = false;

	std::string m_HoltburgStartPosition;
	std::string m_YaraqStartPosition;
	std::string m_ShoushiStartPosition;
	std::string m_SanamarStartPosition;
	std::string m_OlthoiStartPosition;

	int m_PKRespiteTime = 300;
	bool m_bSpellPurgeOnLogin = false;
	bool m_bUpdateAllegianceData = false;
	bool m_bInventoryPurgeOnLogin = false;
	bool m_bRatingPurgeOnLogin = false;

	unsigned int m_WcidForPurge = 100000;

	bool m_bAllowGeneralChat = 1;
	bool m_bRealTimeRares = 0;
	double m_fRareDropMultiplier = 0.0;

	bool m_bLoginAtLS = 0;
	bool m_bCreateTemplates = 0;
	bool m_bAllowPKCommands = 0;
	bool m_bAllowOlthoi = 0;
	bool m_bFixOldChars = 1;
	bool m_bShowDotSpells = 0;

	unsigned int m_maxCorpses = 5;

	std::string m_BanString;

	float m_fellowHealthThreshold = 0.05f;
	float m_fellowStamThreshold = 0.3f;
	float m_fellowManaThreshold = 0.3f;

	uint32_t m_defaultsecondstofullchardelete = 3600;

	float m_missileAttribAdjust = 0.0;

	float pkCSmeleebasechance = 0.1f;
	uint32_t pkCSmeleeminskill = 150;
	uint32_t pkCSmeleemaxskill = 400;
	float pkCSmeleemaxchance = 0.5f;
	float pkCBmeleebasemult = 1.0f;
	uint32_t pkCBmeleeminskill = 150;
	uint32_t pkCBmeleemaxskill = 400;
	float pkCBmeleemaxmult = 7.0f;

	float pkCSmissilebasechance = 0.1f;
	uint32_t pkCSmissileminskill = 125;
	uint32_t pkCSmissilemaxskill = 360;
	float pkCSmissilemaxchance = 0.5f;
	float pkCBmissilebasemult = 1.0f;
	uint32_t pkCBmissileminskill = 125;
	uint32_t pkCBmissilemaxskill = 360;
	float pkCBmissilemaxmult = 7.0f;

	float pkCSmagicbasechance = 0.05f;
	uint32_t pkCSmagicminskill = 125;
	uint32_t pkCSmagicmaxskill = 360;
	float pkCSmagicmaxchance = 0.25f;
	float pkCBmagicbasemult = 0.5f;
	uint32_t pkCBmagicminskill = 125;
	uint32_t pkCBmagicmaxskill = 360;
	float pkCBmagicmaxmult = 1.0f;

	bool m_bAllowCoalDispel = true;

	float m_defaultallegchatgagtime = 300;

	float m_bluesigilprocrate = 0.005f;
	float m_yellowsigilprocrate = 0.0075f;
	float m_redsigilprocrate = 0.01f;

	float m_cloakbaseprocrate = 0.05f;
	float m_cloakhalfhealthprocbonus = 0.075f;
	float m_cloakquarterhealthprocbonus = 0.05f;
	float m_cloaktenthhealthprocbonus = 0.025f;
	float m_cloakperlevelbonus = 0.01f;
};

extern CPhatACServerConfig *g_pConfig;
