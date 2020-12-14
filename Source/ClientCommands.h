#pragma once

using namespace std;

class CPlayerWeenie;
class CClient;
class CPhysicsObj;

class CommandBase;
class ClientCommand;
class ServerCommand;

//Note: Server commands don't require a source player.
typedef bool(*pfnCommandCallback)(class CClient *player_client, class CPlayerWeenie *player_weenie, class CPhysicsObj *player_physobj, char* argv[], int argc);

enum AccessLevels
{
	BASIC_ACCESS = 1,
	DONOR_ACCESS,
	ADVOCATE_ACCESS,
	SENTINEL_ACCESS,
	PRIVILEGED_ACCESS,
	ADMIN_ACCESS,
	SERVER_ACCESS,
	DUMMY_ACCESS
};

enum AccessFlags
{
	DevelopmentCommands_Flag = 1,
	KickBanCommands_Flag = 2,
	TeleportCommands_Flag = 4,
	AffectOtherCommands_Flag = 8,
	ExperimentCommands_Flag = 0x10,
	ServerEventCommands_Flag = 0x20,
	DatabaseCommands_Flag = 0x40,
	ServerRestartCommands_Flag = 0x80,
	SpawnCommands_Flag = 0x100,
	GiveXPCommands_Flag = 0x200,
	InvincibleCommands_Flag = 0x400
};

enum CommandCategory
{
	HIDDEN_CATEGORY = 0,
	GENERAL_CATEGORY,
	EXPLORE_CATEGORY,
	SPAWN_CATEGORY,
	CHARACTER_CATEGORY,
	SERVER_CATEGORY,

};

#define MAX_ARGUMENTS 12

struct CommandEntry {
	const char* name;
	const char* args;
	const char* help;
	pfnCommandCallback func;
	int access;
	int category;
	bool source;
};
typedef map<string, CommandEntry> CommandMap;

class CommandBase
{
	friend class ClientCommand;
	friend class ServerCommand;

public:
	static bool Execute(char *command, CClient *client);

protected:
	CommandBase()
	{
	}

	static void Create(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory, bool bSource);
	static const char* Info(CommandEntry* pCommand);
	static CommandEntry* FindCommand(const char* szName, int iAccessLevel = -1);

	static void PrintCommandList(CWeenieObject* sendto, int category, int access_level, char* cmdName);

	//static CommandMap m_mCommands;
	static CommandMap& GetCommandMap()
	{
		static CommandMap commands;
		return commands;
	}
};

class ClientCommand : CommandBase
{
	friend class CommandBase;
public:
	ClientCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory);
};

class ServerCommand : CommandBase
{
	friend class CommandBase;
public:
	ServerCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory);
};

#define CLIENT_COMMAND( name, args, help, access, category ) \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc); \
 static ClientCommand name##_command( #name, args, help, name, access, category ); \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc)

#define CLIENT_COMMAND_WITH_CUSTOM_NAME( name, command, args, help, access, category ) \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc); \
 static ClientCommand name##_command( #command, args, help, name, access, category  ); \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc)

#define SERVER_COMMAND( name, args, help, access, category ) \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc); \
 static ServerCommand name##_command( #name, args, help, name, access, category  ); \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc)

#define SERVER_COMMAND_WITH_CUSTOM_NAME( name, command, args, help, access, category ) \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc); \
 static ServerCommand name##_command( #command, args, help, name, access, category  ); \
 static bool name(CClient *player_client, CPlayerWeenie *pPlayer, CPhysicsObj *player_physobj, char* argv[], int argc)

