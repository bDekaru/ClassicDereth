
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

	const char *DatabaseIP() { return m_DatabaseIP.c_str(); }
	unsigned int DatabasePort() { return m_DatabasePort; }
	const char *DatabaseUsername() { return m_DatabaseUsername.c_str(); }
	const char *DatabasePassword() { return m_DatabasePassword.c_str(); }
	const char *DatabaseName() { return m_DatabaseName.c_str(); }

	const char *WorldName() { return m_WorldName.c_str(); }
	const char *WelcomePopup() { return m_WelcomePopup.c_str(); }
	const char *WelcomeMessage() { return m_WelcomeMessage.c_str(); }

	virtual bool FastTick() { return m_bFastTick; }

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

	virtual double KillXPMultiplier(int level);
	virtual double RewardXPMultiplier(int level);
	virtual double DropRateMultiplier() { return m_fDropRateMultiplier; }
	virtual double RespawnTimeMultiplier() { return m_fRespawnTimeMultiplier; }
	virtual double IsSpellFociEnabled() { return m_bSpellFociEnabled; }

	virtual bool AutoCreateAccounts() { return m_bAutoCreateAccounts; }

	virtual unsigned int OverrideMaxLevel() { return m_OverrideMaxLevel; };
	virtual unsigned int OverrideStartingXP() { return m_OverrideStartingXP; };
	virtual bool DisableUnassignedXPAtMaxLevel() { return m_bDisableUnassignedXPAtMaxLevel; };
	virtual double VitaeXPMultiplier() { return m_fVitaeXPMultiplier; }
	
	virtual unsigned int PKTrophyStartingLevel() { return m_PKTrophyStartingLevel; };
	virtual bool EnablePKTrophyWithVitae() { return m_bEnablePKTrophyWithVitae; };
	virtual unsigned int PKTrophyID(int level);

	virtual bool EnableSpellFociStarterPack() { return m_bEnableSpellFociStarterPack; };
	virtual unsigned int PrimeNewCharacterSpellbook() { return m_PrimeNewCharacterSpellbook; };

	virtual unsigned int MaxDormantLandblocks() { return m_MaxDormantLandblocks; }
	virtual unsigned int DormantLandblockCleanupTime() { return m_DormantLandblockCleanupTime; }

	virtual bool ShowLogins() { return m_bShowLogins; }
	virtual bool SpeedHackKicking() { return m_bSpeedHackKicking; }
	virtual bool ShowDeathMessagesGlobally() { return m_bShowDeathMessagesGlobally; }
	virtual bool ShowPlayerDeathMessagesGlobally() { return m_bShowPlayerDeathMessagesGlobally; }

	virtual const char *HoltburgStartPosition() { return m_HoltburgStartPosition.c_str(); }
	virtual const char *YaraqStartPosition() { return m_YaraqStartPosition.c_str(); }
	virtual const char *ShoushiStartPosition() { return m_ShoushiStartPosition.c_str(); }
	virtual const char *SanamarStartPosition() { return m_SanamarStartPosition.c_str(); }

	virtual int PKRespiteTime() { return m_PKRespiteTime; }

protected:
	virtual void PostLoad() override;

	unsigned long m_BindIP = 0;
	unsigned int m_BindPort = 0;

	std::string m_DatabaseIP;
	unsigned int m_DatabasePort = 0;
	std::string m_DatabaseUsername;
	std::string m_DatabasePassword;
	std::string m_DatabaseName;

	std::string m_WorldName;
	std::string m_WelcomePopup;
	std::string m_WelcomeMessage;

	bool m_bFastTick = false;

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

	double m_fDropRateMultiplier = 1.0;
	double m_fRespawnTimeMultiplier = 1.0;
	bool m_bSpellFociEnabled = true;

	bool m_bAutoCreateAccounts = true;

	unsigned int m_OverrideMaxLevel = 275;
	unsigned int m_OverrideStartingXP = 0;
	bool m_bDisableUnassignedXPAtMaxLevel = false;
	double m_fVitaeXPMultiplier = 1.0;

	unsigned int m_PKTrophyStartingLevel = 1;
	bool m_bEnablePKTrophyWithVitae = false;
	unsigned int m_PKTrophyIDT0 = 0;
	unsigned int m_PKTrophyIDT1 = 0;
	unsigned int m_PKTrophyIDT2 = 0;
	unsigned int m_PKTrophyIDT3 = 0;
	unsigned int m_PKTrophyIDT4 = 0;
	unsigned int m_PKTrophyIDT5 = 0;
	unsigned int m_PKTrophyIDT6 = 0;
	
	bool m_bEnableSpellFociStarterPack = false;
	unsigned int m_PrimeNewCharacterSpellbook = 0;

	unsigned int m_MaxDormantLandblocks = 1000;
	unsigned int m_DormantLandblockCleanupTime = 1800;

	bool m_bShowLogins = true;
	bool m_bSpeedHackKicking = true;

	bool m_bShowDeathMessagesGlobally = false;
	bool m_bShowPlayerDeathMessagesGlobally = false;

	std::string m_HoltburgStartPosition;
	std::string m_YaraqStartPosition;
	std::string m_ShoushiStartPosition;
	std::string m_SanamarStartPosition;

	int m_PKRespiteTime = 300;
};

extern CPhatACServerConfig *g_pConfig;