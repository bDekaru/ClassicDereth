#include <StdAfx.h>
#include "Client.h"
#include "ClientEvents.h"
#include "ClientCommands.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "Door.h"
#include "Network.h"
#include "World.h"
#include "ChatMsgs.h"
#include "Portal.h"
#include "Database2.h"
#include "GameMode.h"
#include "Lifestone.h"
#include "Setup.h"
#include "WorldLandBlock.h"
#include "ClothingTable.h"
#include "PalSet.h"
#include "ClothingCache.h"
#include "GfxObj.h"
#include "Surface.h"
#include "ObjDesc.h"
#include "MotionTable.h"
#include "Movement.h"
#include "MovementManager.h"
#include "PartArray.h"
#include "MonsterAI.h"
#include "WeenieFactory.h"
#include "TownCrier.h"
#include "Vendor.h"
#include "Monster.h"
#include "SpellcastingManager.h"
#include "WClassID.h"
#include "Config.h"
#include "PacketController.h"
#include "InferredPortalData.h"
#include "RandomRange.h"
#include "House.h"
#include "GameEventManager.h"
#include "easylogging++.h"
#include "ObjectMsgs.h"
#include "EnumUtil.h"
#include "ChessManager.h"
#include "AllegianceManager.h"
#include "HouseManager.h"

// Most of these commands are just for experimenting and never meant to be used in a real game
// TODO: Add flags to these commands so they are only accessible under certain modes such as a sandbox mode

//CommandMap CommandBase::m_mCommands;

void CommandBase::Create(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory, bool bSource)
{
	CommandEntry i =
	{
		szName,
		szArguments,
		szHelp,
		pCallback,
		iAccessLevel,
		iCategory,
		bSource
	};
	GetCommandMap()[std::string(szName)] = i;
	//m_mCommands[std::string(szName)] = i;
}

bool g_bSilence = false;
bool g_bSpawnMonsterDisabled = false;

bool SpawningEnabled(CPlayerWeenie *pPlayer, bool item = false)
{
	if (g_bSilence || (!item && g_bSpawnMonsterDisabled))
	{
		if (pPlayer->GetClient() && pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
		{
			return false;
		}
	}

	return true;
}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(skillspendexp, "<skillID> <exp>", "Attempts to spend the input exp to the given skill.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{

	if (argc < 2)
	{
		pPlayer->SendText("An arg is missing", LTT_DEFAULT);
		return true;
	}

	uint32_t seq = 1;
	uint32_t eventNum = 0x0046;
	uint32_t dwSkill = (unsigned)atoi(argv[0]);
	uint32_t dwXP = (unsigned)atoi(argv[1]);

	std::vector<uint32_t> data;
	data.push_back(seq);
	data.push_back(eventNum);
	data.push_back(dwSkill);
	data.push_back(dwXP);

	BinaryReader reader(data.data(), data.size() * sizeof(uint32_t));

	player_client->GetEvents()->ProcessEvent(&reader);

	pPlayer->SendText(csprintf("Attempted to add %i exp to the skill", dwXP), LTT_DEFAULT);

	return false;
}
#endif


#ifndef PUBLIC_BUILD
CLIENT_COMMAND(simulateaccess, "<level>", "Simulate access level.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	pPlayer->GetClient()->SetAccessLevel((unsigned) atoi(argv[0]));
	return false;
}
#endif

bool IsValidEmail(const char* email)
{
	std::string _email = email;

	bool retVal = false;

	//Tolower cast
	std::transform(_email.begin(), _email.end(), _email.begin(), ::tolower);

	//Edit these to change valid characters you want to be supported to be valid. You can edit it for each section. Remember to edit the array size in the for-loops below.

	const char* validCharsName = "abcdefghijklmnopqrstuvwxyz0123456789.%+_-"; //length = 41, change in loop
	const char* validCharsDomain = "abcdefghijklmnopqrstuvwxyz0123456789.-"; //length = 38, changein loop
	const char* validCharsTld = "abcdefghijklmnopqrstuvwxyz"; //length = 26, change in loop

	bool invalidCharacterFound = false;
	bool atFound = false;
	bool dotAfterAtFound = false;
	uint16_t letterCountBeforeAt = 0;
	uint16_t letterCountAfterAt = 0;
	uint16_t letterCountAfterDot = 0;

	for (uint16_t i = 0; i < _email.length(); i++) {
		char currentLetter = _email[i];

		//Found first @? Lets mark that and continue
		if (atFound == false && dotAfterAtFound == false && currentLetter == '@') {
			atFound = true;
			continue;
		}

		//Found '.' after @? lets mark that and continue
		if (atFound == true && dotAfterAtFound == false && currentLetter == '.') {
			dotAfterAtFound = true;
			continue;
		}

		//Count characters before @ (must be > 0)
		if (atFound == false && dotAfterAtFound == false) {
			letterCountBeforeAt++;
		}

		//Count characters after @ (must be > 0)
		if (atFound == true && dotAfterAtFound == false) {
			letterCountAfterAt++;
		}

		//Count characters after '.'(dot) after @ (must be between 2 and 6 characters (.tld)
		if (atFound == true && dotAfterAtFound == true) {
			letterCountAfterDot++;
		}

		//Validate characters, before '@'
		if (atFound == false && dotAfterAtFound == false) {
			bool isValidCharacter = false;
			for (uint16_t j = 0; j < 41; j++) {
				if (validCharsName[j] == currentLetter) {
					isValidCharacter = true;
					break;
				}
			}
			if (isValidCharacter == false) {
				invalidCharacterFound = true;
				break;
			}
		}

		//Validate characters, after '@', before '.' (dot)
		if (atFound == true && dotAfterAtFound == false) {
			bool isValidCharacter = false;
			for (uint16_t k = 0; k < 38; k++) {
				if (validCharsDomain[k] == currentLetter) {
					isValidCharacter = true;
					break;
				}
			}
			if (isValidCharacter == false) {
				invalidCharacterFound = true;
				break;
			}
		}

		//After '.' (dot), and after '@' (.tld)
		if (atFound == true && dotAfterAtFound == true) {
			bool isValidCharacter = false;
			for (uint16_t m = 0; m < 26; m++) {
				if (validCharsTld[m] == currentLetter) {
					isValidCharacter = true;
					break;
				}
			}
			if (isValidCharacter == false) {
				invalidCharacterFound = true;
				break;
			}
		}

		//Break the loop to speed up thigns if one character was invalid
		if (invalidCharacterFound == true) {
			break;
		}
	}

	//Compare collected information and finalize validation. If all matches: retVal -> true!
	if (atFound == true && dotAfterAtFound == true && invalidCharacterFound == false && letterCountBeforeAt >= 1 && letterCountAfterAt >= 1 && letterCountAfterDot >= 2 && letterCountAfterDot <= 6) {
		retVal = true;
	}

	return retVal;
}


CLIENT_COMMAND(getemail, "", "Retrieves the email used on current account.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	std::string accountEmail = g_pDBIO->GetEmailOfAccount(pPlayer->GetID());
	if (accountEmail.size() == 0)
	{
		pPlayer->SendText("No email found associated to this account.", LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(csprintf("Current email address: %s", accountEmail.c_str()), LTT_DEFAULT);
	}

	return false;
}

CLIENT_COMMAND(setemail, "<email address>", "One time command to attach email to an account.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	if (g_pDBIO->IsEmailAlreadySet(pPlayer->GetID()))
	{
		pPlayer->SendText("Email associated already done unable to update.", LTT_DEFAULT);
		return false;
	}

	if (IsValidEmail(argv[0]))
	{
		if (g_pDBIO->SetEmailOfAccount(argv[0], pPlayer->GetID()))
		{
			pPlayer->SendText("Email attached to account. No further change available", LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText("Please contact admins due to issue setting email address.", LTT_DEFAULT);
		}
	}
	else
	{
		pPlayer->SendText("That is not a valid email address.", LTT_DEFAULT);
	}

	return false;
}



CLIENT_COMMAND(myloc, "", "Info on your current location.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	pPlayer->SendText(csprintf("%08X %.2f %.2f %.2f %.2f %.2f %.2f %.2f",
		player_physobj->m_Position.objcell_id,
		player_physobj->m_Position.frame.m_origin.x, player_physobj->m_Position.frame.m_origin.y, player_physobj->m_Position.frame.m_origin.z,
		player_physobj->m_Position.frame.m_angles.w, player_physobj->m_Position.frame.m_angles.x, player_physobj->m_Position.frame.m_angles.y, player_physobj->m_Position.frame.m_angles.z), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(loc_other, "", "Info on your last assessed target's location.", SENTINEL_ACCESS, EXPLORE_CATEGORY)
{
	CWeenieObject *other = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!other)
		return false;

	pPlayer->SendText(csprintf("%08X %.2f %.2f %.2f %.2f %.2f %.2f %.2f",
		other->m_Position.objcell_id,
		other->m_Position.frame.m_origin.x, other->m_Position.frame.m_origin.y, other->m_Position.frame.m_origin.z,
		other->m_Position.frame.m_angles.w, other->m_Position.frame.m_angles.x, other->m_Position.frame.m_angles.y, other->m_Position.frame.m_angles.z), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(petdamage, "on|off", "Have combat pets show attack info.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("Your combat pets will now show combat results.", LTT_DEFAULT);
		pPlayer->CombatPetsDisplayCombatDamage(true);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("Your combat pets will no longer show combat results.", LTT_DEFAULT);
		pPlayer->CombatPetsDisplayCombatDamage(false);
	}
	else
		return true;

	return false;
}

#if 0
CLIENT_COMMAND(startgame, "[gameid]", "Spawns something by name (right now works for monsters, NPCs, players.)", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;
	
	int game = atoi(argv[0]);

	switch (game)
	{
	case 0:
		g_pWorld->SetNewGameMode(NULL);
		break;

	case 1:
		g_pWorld->SetNewGameMode(new CGameMode_Tag());
		break;
	}

	return false;
}

CLIENT_COMMAND(spawnportal, "", "Spawns a dysfunctional portal near you.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CPortal *pPortal = new CPortal();
	pPortal->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pPortal);

	return false;
}

CLIENT_COMMAND(spawndoor, "", "Spawns a door at your location.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CWeenieObject *pDoor = new CBaseDoor();
	pDoor->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pDoor);

	return false;
}
#endif


CLIENT_COMMAND(global, "<text> [color=1]", "Displays text globally.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	if (g_bSilence)
	{
		return false;
	}

	const char* szText = argv[0];
	int32_t lColor = (argc >= 2) ? atoi(argv[1]) : 1;

	g_pWorld->BroadcastGlobal(ServerText(szText, lColor), PRIVATE_MSG);
	return false;
}

#if 0
CLIENT_COMMAND(overlay, "<text>", "Displays <text> at the top-middle of the 3D window.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}
	const char* szText = argv[0];
	pPlayer->SendTextToOverlay(szText);
	return false;
}
#endif


CLIENT_COMMAND(animationall, "<num> [speed]", "Performs an animation for everyone.", ADMIN_ACCESS, SERVER_CATEGORY)
{

	if (argc < 1)
	{
		return true;
	}

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	fSpeed = min(10.0f, max(0.1f, fSpeed));

	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->_server_control_timestamp += 2;

		i->second->last_move_was_autonomous = false;

		MovementParameters params;
		params.action_stamp = ++pPlayer->m_wAnimSequence;
		params.speed = fSpeed;
		params.autonomous = 0;
		params.modify_interpreted_state = 1;

		MovementStruct mvs;
		mvs.motion = GetCommandID(wIndex);
		mvs.params = &params;
		mvs.type = MovementTypes::RawCommand;
		i->second->get_minterp()->PerformMovement(mvs);
		i->second->Animation_Update();

	}

	return false;
}


CLIENT_COMMAND(freezeall, "", "Freezes or unfreezes everyone.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->set_state(i->second->m_PhysicsState ^ (uint32_t)(FROZEN_PS), TRUE);
	}

	return false;
}

#if 0
CLIENT_COMMAND(lineall, "", "Moves everyone.", ADMIN_ACCESS)
{
	Position targetPosition = player_physobj->m_Position;

	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();

	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		if (i->second != pPlayer)
		{
			targetPosition.frame.m_origin.x += 1.5f;
			i->second->SendText("Teleporting you...", LTT_DEFAULT);
			i->second->Movement_Teleport(targetPosition);
		}
	}

	return false;
}
#endif

CLIENT_COMMAND(effect, "<index> [scale=1]", "Emits a particle effect.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	uint32_t dwIndex;
	float flScale;

	dwIndex = atol(argv[0]);
	if (argc >= 2)	flScale = (float)atof(argv[1]);
	else				flScale = (float)1.0f;

	pPlayer->EmitEffect(dwIndex, flScale);

	return false;
}

CLIENT_COMMAND(sound, "<index> [speed?=1]", "Emits a sound effect.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	uint32_t dwIndex = atol(argv[0]);
	float flSpeed = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);

	pPlayer->EmitSound(dwIndex, flSpeed);

	return false;
}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(arwic, "", "Teleports you to Arwic.", SENTINEL_ACCESS, EXPLORE_CATEGORY)
{
	pPlayer->SendText("Teleporting you to Arwic..", LTT_DEFAULT);
	pPlayer->Movement_Teleport(Position(0xC6A90023, Vector(102.4f, 70.1f, 44.0f), Quaternion(0.70710677f, 0, 0, 0.70710677f)));

	return false;
}

CLIENT_COMMAND(removethis, "", "Removes an object.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	std::string itemRemoved = pPlayer->RemoveLastAssessed(true);
	if (itemRemoved != "")
	{
		pPlayer->SendText(std::string("Removed object: ").append(itemRemoved).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to clear!").append(itemRemoved).c_str(), LTT_DEFAULT);
		return true;
	}

	return false;
}
#endif

CLIENT_COMMAND(targeteffect, "[effect] [scale]", "Plays an effect on the last target you assessed.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!pPlayer->m_LastAssessed)
		return true;

	if (argc < 1)
		return true;

	uint32_t dwIndex = atol(argv[0]);

	float flScale;
	if (argc >= 2)
		flScale = (float)atof(argv[1]);
	else
		flScale = (float)1.0f;

	CWeenieObject *pTarget = g_pWorld->FindWithinPVS(pPlayer, pPlayer->m_LastAssessed);
	if (pTarget)
	{
		pTarget->EmitEffect(dwIndex, flScale);
	}

	return false;
}

CLIENT_COMMAND(targetsound, "[effect] [speed]", "Plays an effect on the last target you assessed.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!pPlayer->m_LastAssessed)
		return true;

	if (argc < 1)
		return true;

	uint32_t dwIndex = atol(argv[0]);

	float flScale;
	if (argc >= 2)
		flScale = (float)atof(argv[1]);
	else
		flScale = (float)1.0f;

	CWeenieObject *pTarget = g_pWorld->FindWithinPVS(pPlayer, pPlayer->m_LastAssessed);
	if (pTarget)
	{
		pTarget->EmitSound(dwIndex, flScale);
	}

	return false;
}

#if 0
CLIENT_COMMAND(spawnorbiter, "", "Just a test.", BASIC_ACCESS)
{
	if (argc < 3)
		return true;

	CTargetDrudge *pDrudge = new CTargetDrudge();
	pDrudge->SetID(g_pWorld->GenerateGUID(eDynamicGUID));
	pDrudge->m_Position = player_physobj->m_Position;
	pDrudge->m_Position.frame.m_origin.x += 10.0f;
	pDrudge->m_velocityVector = Vector(0, 2, 0);
	pDrudge->m_Omega = Vector(atof(argv[0]), atof(argv[1]), atof(argv[2]));
	pDrudge->m_dwSoundSet = 0x20000037;
	pDrudge->m_dwEffectSet = 0x34000000 | strtoul(argv[3], NULL, 16);
	pDrudge->m_fFriction = 0.0f;
	pDrudge->m_fElasticity = 0.0f;
	pDrudge->m_PhysicsState = INELASTIC_PS | SCRIPTED_COLLISION_PS | REPORT_COLLISIONS_PS | MISSILE_PS | LIGHTING_ON_PS | PATHCLIPPED_PS | ALIGNPATH_PS;
	pDrudge->m_dwModel = 0x0200087C;
	pDrudge->m_scale = 0.25f;

	g_pWorld->CreateEntity(pDrudge);
	return false;
}
#endif

CLIENT_COMMAND(tele, "<player name>", "Teleports you to a player.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	CPlayerWeenie *pTarget = g_pWorld->FindPlayer(argv[0]);

	if (pTarget)
	{
		if (pTarget->m_bPrivacyMode && player_client->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText(csprintf("Cannot teleport to %s. They have privacy mode enabled.", pTarget->GetName().c_str()), LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText(csprintf("Teleporting to %s ..", argv[0]), LTT_DEFAULT);
			pPlayer->Movement_Teleport(pTarget->m_Position);
		}
	}
	else
	{
		pPlayer->SendText(csprintf("Couldn't find player \"%s\" ?", argv[0]), LTT_DEFAULT);
	}

	return false;
}


CLIENT_COMMAND(teletome, "<player name>", "Teleports someone to you.", ADVOCATE_ACCESS, EXPLORE_CATEGORY)
{
	if (argc < 1)
		return true;

	CPlayerWeenie *pTarget = g_pWorld->FindPlayer(argv[0]);

	if (pTarget)
	{
		pTarget->SendText(csprintf("Teleporting to %s ..", pPlayer->GetName().c_str()), LTT_DEFAULT);
		pTarget->Movement_Teleport(pPlayer->m_Position);
	}
	else
		pPlayer->SendText(csprintf("Couldn't find player \"%s\" ?", argv[0]), LTT_DEFAULT);

	return false;
}

#if 0
CLIENT_COMMAND(teleall, "<target>", "Teleports all players target. If no target specified, teleports to you.", ADMIN_ACCESS)
{
	CPlayerWeenie* target;
	if (argc < 1)
	{
		target = pPlayer;
	}
	else
	{
		target = g_pWorld->FindPlayer(argv[0]);

		if (target == NULL)
		{
			pPlayer->SendText("Invalid target!", LTT_DEFAULT);
			return true;
		}
	}
	PlayerWeenieMap* map = g_pWorld->GetPlayers();
	PlayerWeenieMap::iterator pit = map->begin();
	PlayerWeenieMap::iterator pend = map->end();

	//This is probably really bad..
	bool teleportedOne = false;
	while (pit != pend)
	{
		CPlayerWeenie *them = pit->second;

		if (them != target)
		{
			teleportedOne = true;
			pPlayer->SendText(std::string("Teleported: ").append(them->GetName()).c_str(), LTT_DEFAULT);
			them->Movement_Teleport(target->m_Position);
			them->SendText(std::string("Teleported by: ").append(pPlayer->GetName()).c_str(), LTT_DEFAULT);
		}

		pit++;
	}

	if (teleportedOne)
	{
		return false;
	}
	else
	{
		pPlayer->SendText("Didn't teleport anyone! Nobody on server?", LTT_DEFAULT);
	}

	return true;
}
#endif

CLIENT_COMMAND(teletown, "<town name>", "Teleports you to a town.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (argc == 0)
	{
		pPlayer->SendText("Your teleporting choices are:", LTT_DEFAULT);
		pPlayer->SendText(g_pWorld->GetTeleportList().c_str(), LTT_DEFAULT);
		return true;
	}

	std::string cmdString = argv[0];
	for (int i = 1; i < argc; i++)
	{
		cmdString.append(" ");
		cmdString.append(argv[i]);
	}

	TeleTownList_s var = g_pWorld->GetTeleportLocation(cmdString);
	if (var.m_teleString != "")
	{
		pPlayer->SendText(std::string("Portaling To: ").append(var.m_teleString).c_str(), LTT_DEFAULT);
		pPlayer->Movement_Teleport(var.position);
		return false;
	}
	else
	{
		pPlayer->SendText("Town Not Found! Try again...", LTT_DEFAULT);
	}

	return true;

}

CLIENT_COMMAND(teleto, "<coords>", "Teleports you to coordinates.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < ADVOCATE_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (argc < 2)
	{
		return true;
	}

	float NS = NorthSouth(argv[0]);
	float EW = EastWest(argv[1]);
	char cNS = ' ';
	char cEW = ' ';
	if (NS < 0)	cNS = 'S'; else if (NS > 0) cNS = 'N';
	if (EW < 0)	cEW = 'W'; else if (EW > 0) cEW = 'E';

	Position position;
	GetLocation(NS, EW, position);

	if (!position.objcell_id)
	{
		pPlayer->SendText("Bad coordinate location!", LTT_DEFAULT);
	}
	else if (IsWaterBlock(position.objcell_id))
	{
		pPlayer->SendText("Bad location! That's a water block, dummy!", LTT_DEFAULT);
	}
	else
	{
		char *szTP = csprintf("Teleporting to %0.1f%c %0.1f%c (Cell: %08X Z: %.1f)", fabs(NS), cNS, fabs(EW), cEW, position.objcell_id, position.frame.m_origin.z);
		pPlayer->SendText(szTP, LTT_DEFAULT);
		pPlayer->Movement_Teleport(position);
	}

	return false;
}

bool g_bReviveOverride = false;
Position g_RevivePosition;

bool g_bStartOverride = false;
Position g_StartPosition;

#if 0
CLIENT_COMMAND(reviveoverride, "<setting>", "", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_bReviveOverride = atoi(argv[0]) ? true : false;
	g_RevivePosition = pPlayer->m_Position;
	return false;
}

CLIENT_COMMAND(startoverride, "<setting>", "", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_bStartOverride = atoi(argv[0]) ? true : false;
	g_StartPosition = pPlayer->m_Position;
	return false;
}
#endif

CLIENT_COMMAND(adminvision, "<enabled>", "", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
	{
		return true;
	}

	/*
	if (_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "admin") &&
		_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "cmoskitest") &&
		_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "peatest"))
		return false;
		*/

	pPlayer->m_bAdminVision = atoi(argv[0]) ? true : false;

	if (pPlayer->GetBlock())
		pPlayer->GetBlock()->ExchangePVS(pPlayer, 0);

	return false;
}


CLIENT_COMMAND(serverstatus, "", "Provides information on the server's status.", SENTINEL_ACCESS, SERVER_CATEGORY)
{
	pPlayer->SendText(csprintf("Server status:\n%s", g_pWorld->GetServerStatus().c_str()), LTT_DEFAULT);
	g_pWorld->m_SendPerformanceInfoToPlayer = pPlayer->GetID();

	return false;
}

//CLIENT_COMMAND(netinfo, "", "Provides information on the player's network connection.", BASIC_ACCESS)
//{
//	int sendbuf = 0;
//	int sendbuflen = sizeof(sendbuf);
//	getsockopt(g_pNetwork->m_sockets[0], SOL_SOCKET, SO_SNDBUF, (char *)&sendbuf, &sendbuflen);
//	pPlayer->SendText(csprintf("SNDBUF: %u", sendbuf), LTT_DEFAULT);
//
//	int recvbuf = 0;
//	int recvbuflen = sizeof(recvbuf);
//	getsockopt(g_pNetwork->m_sockets[0], SOL_SOCKET, SO_RCVBUF, (char *)&recvbuf, &recvbuflen);
//	pPlayer->SendText(csprintf("RCVBUF: %u", recvbuf), LTT_DEFAULT);
//
//	pPlayer->SendText(csprintf("INSEQ: %u", player_client->GetPacketController()->m_in.sequence), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("ACTIVESEQ: %u", player_client->GetPacketController()->m_in.activesequence), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("FLUSHSEQ: %u", player_client->GetPacketController()->m_in.flushsequence), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("OUTSEQ: %u", player_client->GetPacketController()->m_out.sequence), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("RECEIVED: %I64u", player_client->GetPacketController()->m_in.receivedbytes), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("SENT: %I64u", player_client->GetPacketController()->m_out._sentBytes), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("RETRANSMIT: %u", player_client->GetPacketController()->m_out.numretransmit), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("DENIED: %u", player_client->GetPacketController()->m_out.numdenied), LTT_DEFAULT);
//	pPlayer->SendText(csprintf("REQUESTED: %u", player_client->GetPacketController()->m_in.numresendrequests), LTT_DEFAULT);
//
//	return false;
//}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(debug, "<index>", "", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	/*
	if (_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "admin") &&
		_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "cmoskitest") &&
		_stricmp(pPlayer->GetClient()->GetAccountInfo().username.c_str(), "peatest"))
		return false;
		*/

	int debugLevel = atoi(argv[0]);

	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!target)
		return false;

	float respawnTime = 0.0f;
	CWeenieObject *pGenerator = g_pWorld->FindObject(target->InqIIDQuality(GENERATOR_IID, 0));

	std::string info;
	info += csprintf("ID: %08X WCID: %u\nWClass: %s @ %08X %.1f %.1f %.1f (%.f away)\nGen: %08X Act: %08X RespawnTime: %.1fm",
		target->GetID(),
		target->m_Qualities.id,
		GetWCIDName(target->m_Qualities.id),
		target->m_Position.objcell_id,
		target->m_Position.frame.m_origin.x,
		target->m_Position.frame.m_origin.y,
		target->m_Position.frame.m_origin.z,
		pPlayer->DistanceTo(target, true),
		target->InqIIDQuality(GENERATOR_IID, 0),
		target->InqIIDQuality(ACTIVATION_TARGET_IID, 0),
		pGenerator ? pGenerator->InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0.0f)  * g_pConfig->RespawnTimeMultiplier() / 60.0f : 0.0f);

	double initInterval = 0.0;
	if (target->m_Qualities.InqFloat(GENERATOR_INITIAL_DELAY_FLOAT, initInterval, TRUE))
	{
		info += csprintf(" GenInitDelay: %.1fm", initInterval  * g_pConfig->RespawnTimeMultiplier() / 60.0);
	}

	double regenInterval = 0.0;
	if (target->m_Qualities.InqFloat(REGENERATION_INTERVAL_FLOAT, regenInterval, TRUE))
	{
		info += csprintf(" GenRegenInterval: %.1fm", regenInterval * g_pConfig->RespawnTimeMultiplier() / 60.0);
	}

	double genRadius = 0.0;
	if (target->m_Qualities.InqFloat(GENERATOR_RADIUS_FLOAT, genRadius, TRUE))
	{
		info += csprintf(" GenRadius: %.1fm", genRadius);
	}

	double resetInterval = 0.0;
	if (target->m_Qualities.InqFloat(RESET_INTERVAL_FLOAT, resetInterval, TRUE))
	{
		info += csprintf(" Reset: %.1fm", resetInterval  * g_pConfig->RespawnTimeMultiplier() / 60.0);
	}

	int initGen = 0;
	if (target->m_Qualities.InqInt(INIT_GENERATED_OBJECTS_INT, initGen, TRUE))
	{
		info += csprintf(" InitGen: %d", initGen);
	}

	int maxGen = 0;
	if (target->m_Qualities.InqInt(MAX_GENERATED_OBJECTS_INT, maxGen, TRUE))
	{
		info += csprintf(" MaxGen: %d", maxGen);
	}

	std::string eventString;
	if (target->m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	{
		info += csprintf(" GenEvent: %s", eventString.c_str());
	}

	if (target->_nextRegen >= 0)
	{
		info += csprintf(" NextRegen: %.1fm", (target->_nextRegen - Timer::cur_time) / 60.0);
	}

	if (!target->cell)
	{
		info += " NOT IN CELL";
	}

	if (target->m_Qualities._create_list)
	{
		info += csprintf("\nCreate List:");
		for (auto &entry : *target->m_Qualities._create_list)
		{
			info += csprintf("\n%u (%s) try_to_bond %u palette %u shade/probability %f destination/regen_algorithm %u stack_size/max_number/amount %d",
				entry.wcid, GetWCIDName(entry.wcid), entry.try_to_bond, entry.palette, entry.shade, entry.destination, entry.stack_size);
		}
	}
	
	if (target->m_Qualities._emote_table)
	{
		info += csprintf("\nEmote Table:");
		for (auto &emoteCategory : target->m_Qualities._emote_table->_emote_table)
		{
			info += csprintf("\nEmoteCategory %s (%u)", Emote::EmoteCategoryToName((EmoteCategory) emoteCategory.first), emoteCategory.first);

			for (auto &emoteSet : emoteCategory.second)
			{
				info += csprintf("\n  EmoteSet %s (%u) probability %f", Emote::EmoteCategoryToName((EmoteCategory)emoteSet.category), emoteSet.category, emoteSet.probability);
				
				switch (emoteSet.category)
				{
				case Refuse_EmoteCategory:
				case Give_EmoteCategory:
					info += csprintf(" %d (%s)", emoteSet.classID, GetWCIDName(emoteSet.classID));
					break;

				case HeartBeat_EmoteCategory:
					info += csprintf(" 0x%08X 0x%08X", emoteSet.style, emoteSet.substyle);
					break;

				case QuestSuccess_EmoteCategory: // 12 0xC
				case QuestFailure_EmoteCategory: // 13 0xD
				case TestSuccess_EmoteCategory: // 22 0x16
				case TestFailure_EmoteCategory: // 23 0x17
				case EventSuccess_EmoteCategory: // 27 0x1B
				case EventFailure_EmoteCategory: // 28 0x1C
				case TestNoQuality_EmoteCategory: // 29 0x1D
				case QuestNoFellow_EmoteCategory: // 30 0x1E
				case TestNoFellow_EmoteCategory: // 31 0x1F
				case GotoSet_EmoteCategory: // 32 0x20
				case NumFellowsSuccess_EmoteCategory: // 33 0x21
				case NumFellowsFailure_EmoteCategory: // 34 0x22
				case NumCharacterTitlesSuccess_EmoteCategory: // 35 0x23
				case NumCharacterTitlesFailure_EmoteCategory: // 36 0x24
				case ReceiveLocalSignal_EmoteCategory: // 37 0x25
				case ReceiveTalkDirect_EmoteCategory: // 38 0x26
					info += csprintf(" %s", emoteSet.quest.c_str());
					break;

				case Vendor_EmoteCategory:
					// 1 = open vendor
					// 2 = walk away
					// 3 = sell
					// 4 = buy
					// 5 = performs a motion 0x87 or 0x7d or 0x86 or 0x83
					info += csprintf(" %d", emoteSet.vendorType);
					break;

				case WoundedTaunt_EmoteCategory:
					info += csprintf(" %f %f", emoteSet.minhealth, emoteSet.maxhealth);
					break;
				}

				for (auto &emote : emoteSet.emotes)
				{
					info += csprintf("\n    %s (%u) %.1f %.1f %s", Emote::EmoteTypeToName(emote.type), emote.type, emote.delay, emote.extent, emote.msg.c_str());

					switch (emote.type)
					{
					case CastSpellInstant_EmoteType:
						info += csprintf(" %u", emote.spellid);
						break;

					case Motion_EmoteType:
					case ForceMotion_EmoteType:
						info += csprintf(" 0x%X", emote.motion);
						break;

					case 0x22u:
					case 0x2Fu:
					case InflictVitaePenalty_EmoteType:
					case 0x5Au:
					case 0x77u:
					case 0x78u:
						info += csprintf(" %d", emote.amount);
						break;

					case InqStringStat_EmoteType:
					case InqYesNo_EmoteType:
						info += csprintf(" %s %d", emote.teststring.c_str(), emote.stat);
						break;

					case InqIntStat_EmoteType:
					case InqAttributeStat_EmoteType:
					case InqRawAttributeStat_EmoteType:
					case InqSecondaryAttributeStat_EmoteType:
					case InqRawSecondaryAttributeStat_EmoteType:
					case InqSkillStat_EmoteType:
					case InqRawSkillStat_EmoteType:
						info += csprintf(" %d %d %d", emote.min, emote.max, emote.stat);
						break;

					case Give_EmoteType:
					case TakeItems_EmoteType:
						info += csprintf(" %u (%s) %d", emote.cprof.wcid, GetWCIDName(emote.cprof.wcid), emote.cprof.amount);
						break;

					case SetIntStat_EmoteType:
					case IncrementIntStat_EmoteType:
					case DecrementIntStat_EmoteType:
					case SetBoolStat_EmoteType:
						info += csprintf(" %d %d", emote.stat, emote.amount);
						break;

					case InqQuestSolves_EmoteType:
					case InqFellowNum_EmoteType:
					case InqNumCharacterTitles_EmoteType:
					case InqMyQuestSolves_EmoteType:
						info += csprintf(" %u %u", emote.min, emote.max);
						break;

					case AwardXP_EmoteType:
					case AwardNoShareXP_EmoteType:
						info += csprintf(" %I64u %I64u", emote.amount64, emote.heroxp64);
						break;

					case AwardSkillXP_EmoteType:
					case AwardSkillPoints_EmoteType:
						info += csprintf(" %u %u", emote.amount, emote.stat);
						break;
					}
				}
			}
		}
	}

	if (target->m_Qualities._generator_table && (debugLevel & 1))
	{
		info += csprintf("\nGenerator Table:");
		for (auto &profile : target->m_Qualities._generator_table->_profile_list)
		{
			info += csprintf("\nProfile probability %f type %d (%s) delay %f init %d maxNum %d when %d where %d stack %d ptid %d shade %f slot %d",
				profile.probability, profile.type, GetWCIDName(profile.type), profile.delay, profile.initCreate, profile.maxNum, profile.whenCreate, profile.whereCreate, profile.stackSize, profile.ptid, profile.shade, profile.slot);
		}
	}

	if (target->m_Qualities._generator_queue && (debugLevel & 2))
	{
		info += csprintf("\nGenerator Queue:");
		for (auto &entry : target->m_Qualities._generator_queue->_queue)
		{
			info += csprintf("\nSlot %u When %ds",
				entry.slot, (int)(entry.when - Timer::cur_time));
		}
	}

	if (target->m_Qualities._generator_registry && (debugLevel & 2))
	{
		info += csprintf("\nGenerator Registry:");
		for (auto &entry : target->m_Qualities._generator_registry->_registry)
		{
			info += csprintf("\n0x%08X - type %u (%s)",
				entry.first, entry.second.m_wcidOrTtype, GetWCIDName(entry.second.m_wcidOrTtype));
		}
	}

	pPlayer->SendText(info.c_str(), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(getinfo, "", "Get Info from targetted object.", BASIC_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!target)
		return false;

	std::string info;
	info += csprintf("WCID: %u \nWClass: %s Name: %s \n Loc: %08X %.6f %.6f %.6f %.6f %.6f %.6f %.6f",

		target->m_Qualities.id,
		GetWCIDName(target->m_Qualities.id),
		target->m_Qualities.GetString(NAME_STRING,"").c_str(), target->m_Position.objcell_id,
		target->m_Position.frame.m_origin.x, target->m_Position.frame.m_origin.y, target->m_Position.frame.m_origin.z,
		target->m_Position.frame.m_angles.w, target->m_Position.frame.m_angles.x, target->m_Position.frame.m_angles.y, target->m_Position.frame.m_angles.z);

	pPlayer->SendText(info.c_str(), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(monsterbrawl, "", "Toggle monsters fighting each other.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	monster_brawl = !monster_brawl;

	if (monster_brawl)
		pPlayer->SendText("Enabled brawling monsters.", LTT_DEFAULT);
	else
		pPlayer->SendText("Disabled brawling monsters.", LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(damagesources, "", "Lists all damage sources and values for the last assessed target if it's a monster.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!target)
		return false;
	if (target->AsMonster())
	{
		std::string info;
		for (std::map<uint32_t, int>::iterator it = target->AsMonster()->m_aDamageSources.begin(), ite = target->AsMonster()->m_aDamageSources.end(); it != ite; ++it)
		{
			CWeenieObject *source = g_pWorld->FindObject(it->first);
			info += csprintf("%s has done %d damage.\n", source->GetName().c_str(), it->second);
		}

		if (!info.empty())
		{
			pPlayer->SendText(info.c_str(), LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText("No damage taken.", LTT_DEFAULT);
		}
	}

	return false;
}

CLIENT_COMMAND(sweartime, "<unix timestamp>", "Sets the time that the last assessed player swore to their patron to <unix timestamp> seconds. If no argument is given, shows the current timestamp.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!target)
		return false;
	if (target->AsPlayer())
	{
		if (AllegianceTreeNode* targetNode = g_pAllegianceManager->GetTreeNode(target->id))
		{
			if (targetNode->_patronID) {
				int oldSwornAt = targetNode->_unixTimeSwornAt;
				if (argc < 1)
				{
					pPlayer->SendText(csprintf("%s's unix time sworn at is %d.", targetNode->_charName.c_str(), oldSwornAt), LTT_DEFAULT);
					return false;
				}
				targetNode->_unixTimeSwornAt = atoi(argv[0]);
				pPlayer->SendText(csprintf("%s's unix time sworn at has been changed from %d to %d.", targetNode->_charName.c_str(), oldSwornAt, targetNode->_unixTimeSwornAt), LTT_DEFAULT);
			}
		}
	}

	return false;
}

CLIENT_COMMAND(addtitle, "id", "Grants the last assessed player's the Title ID.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CPlayerWeenie *target = g_pWorld->FindPlayer(pPlayer->m_LastAssessed);
	if (!target)
		return false;
	if (target)
	{
		int input = atoi(argv[0]);
		if (input > 0)
			target->AddTitle(input);
	}

	return false;
}



CLIENT_COMMAND(passupbool, "<0 | 1>", "Sets the last assessed player's XP passup bool. If no argument is given, shows the current state.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!target)
		return false;
	if (target->AsPlayer())
	{
		if (AllegianceTreeNode* targetNode = g_pAllegianceManager->GetTreeNode(target->id)) 
		{
			bool canPassup = target->InqBoolQuality(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, 0);
			if (argc < 1)
				pPlayer->SendText(csprintf("%s's XP passup is currently %s.", target->GetName().c_str(), canPassup ? "enabled" : "disabled"), LTT_DEFAULT);
			else
			{
				int input = atoi(argv[0]);
				if (!input || input == 1) 
				{
					target->m_Qualities.SetBool(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, input);
					pPlayer->SendText(csprintf("%s's XP passup has now been %s.", target->GetName().c_str(), input ? "enabled" : "disabled"), LTT_DEFAULT);
				}
			}
			
		}
	}

	return false;
}

CLIENT_COMMAND(envmode, "<mode>", "", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	BinaryWriter envMode;
	envMode.Write<uint32_t>(0xEA60);
	envMode.Write<uint32_t>(strtoul(argv[0], NULL, 16));
	g_pWorld->BroadcastGlobal(&envMode, PRIVATE_MSG, 0, FALSE, FALSE);

	/*	/*
	1 = red
	2 = blue
	3 = grey
	4 = dark green
	5 = black
	6 = black

	'Sound_UI_Roar'  0x65
	'Sound_UI_Bell'
	'Sound_UI_Chant1'
	'Sound_UI_Chant2'
	'Sound_UI_DarkWhispers1'
	'Sound_UI_DarkWhispers2'
	'Sound_UI_DarkLaugh'
	'Sound_UI_DarkWind'
	'Sound_UI_DarkSpeech'
	'Sound_UI_Drums'
	'Sound_UI_GhostSpeak'
	'Sound_UI_Breathing' 0x70
	'Sound_UI_Howl'
	'Sound_UI_LostSouls'
	'Sound_UI_Squeal' 0x75
	'Sound_UI_Thunder1'
	'Sound_UI_Thunder2'
	'Sound_UI_Thunder3'
	'Sound_UI_Thunder4'
	'Sound_UI_Thunder5'
	'Sound_UI_Thunder6' 0x7b
	*/

	return false;
}
#endif

CLIENT_COMMAND(vismode, "<mode>", "Changes your physics state.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	pPlayer->set_state(strtoul(argv[0], NULL, 16), TRUE);
	return false;
}

/*
CLIENT_COMMAND(spawnbael, "", "Spawns Bael'Zharon.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CBaelZharon *pBael = new CBaelZharon();
	pBael->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pBael);

	return false;
}
*/

extern double g_TimeAdjustment;

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(timeadjust, "", "Time adjustment. Careful.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	g_TimeAdjustment = atof(argv[0]);
	return false;
}
#endif

CLIENT_COMMAND(squelchall, "", "Squelch all.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;
	
	g_bSilence = atoi(argv[0]) ? true : false;

	if (g_bSilence)
		pPlayer->SendText("Silenced all players and spawning.", LTT_DEFAULT);
	else
		pPlayer->SendText("Unsilenced all players and spawning.", LTT_DEFAULT);

	return false;
}

#if 0
CLIENT_COMMAND(spawnmonsterenabled, "", "Toggle spawning.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_bSpawnMonsterDisabled = atoi(argv[0]) ? false : true;

	if (g_bSpawnMonsterDisabled)
		pPlayer->SendText("Disabled spawning of monsters.", LTT_DEFAULT);
	else
		pPlayer->SendText("Enabled spawning of monsters.", LTT_DEFAULT);

	return false;
}
#endif

/*
CLIENT_COMMAND(targetdrudge, "", "Spawns a Target Drudge.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CTargetDrudge *pDrudge = new CTargetDrudge();
	pDrudge->SetInitialPosition(pPlayer->GetPosition().add_offset(Vector(0, 0, 1.0f)));

	g_pWorld->CreateEntity(pDrudge);

	return false;
}
*/

#if 0
CLIENT_COMMAND(spawnperks, "", "Spawns a aura caster for contributors.", DONOR_ACCESS)
{
	if (!SpawningEnabled(pPlayer, true))
	{
		return false;
	}

	pPlayer->SpawnInContainer(100004);

	return false;
}
#endif

CLIENT_COMMAND(spawnwand, "", "Spawns a wand.", BASIC_ACCESS, SPAWN_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (!SpawningEnabled(pPlayer, true))
	{
		return false;
	}

	/*

	CBaseWand* pWand = new CBaseWand();
	pWand->SetInitialPosition(pPlayer->GetPosition());
	pWand->m_bDontClear = false;
	g_pWorld->CreateEntity(pWand);
	*/

	CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByName("Drudge Wand", &pPlayer->m_Position, true);

	if (weenie)
		weenie->m_bDontClear = false;

	return false;
}

const char *skipHexPrefix(const char *hexString)
{
	if (hexString[0] == '0' && hexString[1] == 'x')
		hexString += 2;
	
	return hexString;
}

const char *skipBracketPrefix(const char *OBracketString)
{
	if (OBracketString[0] == '[')
		OBracketString += 1;

	return OBracketString;
}

const char *skipBracketSuffix(const char *CBracketString)
{
	if (strlen(CBracketString) - 1 == ']')
		CBracketString[strlen(CBracketString)-1];

return CBracketString;
}

CLIENT_COMMAND(teleloc, "<landcell> [x=0] [y=0] [z=0] [anglew=1] [anglex=0] [angley=0] [anglez=0]", "Teleports to a specific location.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < ADVOCATE_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return false;

	Position target;

	target.objcell_id = strtoul(skipHexPrefix(argv[0]), NULL, 16);
	target.frame.m_origin.x = (float)((argc >= 2) ? atof(skipBracketPrefix(argv[1])) : 0);
	target.frame.m_origin.y = (float)((argc >= 3) ? atof(argv[2]) : 0);
	target.frame.m_origin.z = (float)((argc >= 4) ? atof(skipBracketSuffix(argv[3])) : 0);
	target.frame.m_angles.w = (float)((argc >= 8) ? atof(argv[4]) : 1.0f);
	target.frame.m_angles.x = (float)((argc >= 8) ? atof(argv[5]) : 0);
	target.frame.m_angles.y = (float)((argc >= 8) ? atof(argv[6]) : 0);
	target.frame.m_angles.z = (float)((argc >= 8) ? atof(argv[7]) : 0);

	if (!target.objcell_id)
		return false;

	pPlayer->Movement_Teleport(target);

	return false;
}

#if 0
CLIENT_COMMAND(spawnmodel, "<model index> [scale=1] [name=*]", "Spawns a model.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	uint32_t dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = max(0.1f, min(10.0f, (float)((argc >= 2) ? atof(argv[1]) : 1.0f)));
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);

	CWeenieObject *pSpawn = new CWeenieObject();
	pSpawn->SetSetupID(dwModel);
	pSpawn->SetScale(flScale);
	pSpawn->SetName(szName);
	pSpawn->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("This model was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName().c_str(), dwModel, flScale));
	return false;
}

CLIENT_COMMAND(spawnmodels, "<start index> <end index>", "Spawns a range of models.", ADMIN_ACCESS)
{
	if (argc < 2)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
		return false;

	uint32_t dwModelStart = strtoul(argv[0], NULL, 16);
	if (!(dwModelStart & 0xFF000000))
		dwModelStart |= 0x02000000;

	uint32_t dwModelEnd = strtoul(argv[1], NULL, 16);
	if (!(dwModelEnd & 0xFF000000))
		dwModelEnd |= 0x02000000;

	if (dwModelStart > dwModelEnd && dwModelStart != 0)
	{
		return true;
	}

	uint32_t dwCount = (dwModelEnd - dwModelStart) + 1;
	uint32_t dwWidth = (int)sqrt((double)dwCount);

	if ((dwModelEnd - dwModelStart) >= 50)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	int x = 0;
	int y = 0;
	for (uint32_t i = dwModelStart; i <= dwModelEnd; i++)
	{
		CWeenieObject *pSpawn = new CWeenieObject();
		pSpawn->SetSetupID(i);
		pSpawn->SetScale(1.0f);
		pSpawn->SetName(csprintf("Model #%08X", i));
		pSpawn->SetInitialPosition(pPlayer->GetPosition().add_offset(Vector(x * 10.0f, y * 10.0f, 0)));

		pSpawn->SetLongDescription(csprintf("This model was spawned by %s.\nModel #: %08X", pPlayer->GetName().c_str(), i));
		g_pWorld->CreateEntity(pSpawn);

		if (x == dwWidth)
		{
			x = 0;
			y += 1;
		}
		else
			x++;

	}

	return false;
}
#endif

/*
CLIENT_COMMAND(spawnmonster2, "<model index> <base palette>", "Spawns a monster.", BASIC_ACCESS)
{
	if (argc < 3)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	uint32_t dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	uint32_t dwPalette1 = strtoul(argv[1], NULL, 16);
	uint32_t dwPalette2 = strtoul(argv[2], NULL, 16);
	dwPalette1 &= 0xFFFFF;
	dwPalette2 &= 0xFFFFF;

	CMonsterWeenie *pSpawn = new CMonsterWeenie();
	pSpawn->SetSetupID(dwModel);
	pSpawn->m_scale = 1.0f;
	pSpawn->SetName(csprintf("0x%X 0x%X 0x%X", dwModel, dwPalette1, dwPalette2));
	pSpawn->SetInitialPosition(pPlayer->GetPosition());
	pSpawn->m_miBaseModel.dwBasePalette = dwPalette1;
	pSpawn->m_miBaseModel.lPalettes.push_back(PaletteRpl(dwPalette2, 0, 0));

	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("This monster was spawned by %s.\nModel #: %08X\nPalette #: %08X %08X\n", pPlayer->GetName().c_str(), dwModel, dwPalette1, dwPalette2));

	return false;
}
*/

#if 0
CLIENT_COMMAND(spawnmonster, "<model index> [scale=1] [name=*] [dotcolor]", "Spawns a monster.", ADMIN_ACCESS)
{	
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	uint32_t dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = max(0.1f, min(10.0f, (float)((argc >= 2) ? atof(argv[1]) : 1.0f)));
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);
	int dotColor = (int)((argc >= 4) ? atoi(argv[3]) : 0);

	CMonsterWeenie *pSpawn = new CMonsterWeenie();
	pSpawn->SetSetupID(dwModel);
	pSpawn->SetScale(flScale);
	pSpawn->SetName(szName);
	pSpawn->SetInitialPosition(pPlayer->GetPosition().add_offset(Vector(0, 6, 6)));
	pSpawn->SetRadarBlipColor((RadarBlipEnum) dotColor);
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("This monster was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName().c_str(), dwModel, flScale));

	return false;
}

CLIENT_COMMAND(spawnitem, "<model index> [scale=1] [name=*]", "Spawns an item.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	uint32_t dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = max(0.1, min(10, (float)((argc >= 2) ? atof(argv[1]) : 1.0f)));
	const char* szName = (argc >= 3) ? argv[2] : csprintf("Model #%08X", dwModel);

	CWeenieObject *pSpawn = new CWeenieObject();
	pSpawn->m_Qualities.SetBool(STUCK_BOOL, FALSE);
	pSpawn->SetSetupID(dwModel);
	pSpawn->SetScale(flScale);
	pSpawn->SetName(szName);
	pSpawn->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("This item was spawned by %s.\nModel #: %08X\nScale: %f\n", pPlayer->GetName().c_str(), dwModel, flScale));

	return false;
}

CLIENT_COMMAND(spawnlifestone, "", "Spawns a lifestone.", ADVOCATE_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
		return false;

	CBaseLifestone *pSpawn = new CBaseLifestone();
	pSpawn->SetInitialPosition(pPlayer->GetPosition());
	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("Life Stone spawned by %s.", pPlayer->GetName().c_str()));
	return false;
}
#endif

CLIENT_COMMAND(clearspawns, "<force clear on/off>", "Clears the spawns in your current landblock", SENTINEL_ACCESS, SPAWN_CATEGORY)
{
	bool forced = false;

	if (argc >= 1)
	{
		if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
			forced = true;
	}

	if (!SpawningEnabled(pPlayer))
		return false;

	CWorldLandBlock *pBlock = pPlayer->GetBlock();

	if (pBlock)
		pBlock->ClearSpawns(forced);

	pPlayer->SendText("Clearing spawns in your landblock.", LTT_DEFAULT);
	return false;
}

#if 0
CLIENT_COMMAND(clearallspawns, "", "Clears all spawns in all landblocks (slow.)", SENTINEL_ACCESS)
{
	g_pWorld->ClearAllSpawns();

	pPlayer->SendText("Clearing spawns in all landblocks.", LTT_DEFAULT);
	return false;
}
#endif

SERVER_COMMAND(kick, "<player name>", "Kicks the specified player.", SENTINEL_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (pPlayer)
	{
		SERVER_INFO << pPlayer->GetName().c_str() << "is using the kick command.";
	}
	else
	{
		SERVER_INFO << "Server is using the kick command.";
	}

	CPlayerWeenie *pTarget = g_pWorld->FindPlayer(argv[0]);

	if (pTarget)
		g_pNetwork->KickClient(pTarget->GetClient());
	else
		pPlayer->SendText("Couldn't find target player.", LTT_DEFAULT);

	return false;
}

SERVER_COMMAND(ban, "<add/remove/list> <player name (if adding) or IP (if removing)> <reason (if adding)> <duration in seonds (if adding)>", "Kicks the specified player.", SENTINEL_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "list"))
	{
		std::string banList = g_pNetwork->GetBanList();
		pPlayer->SendText(banList.c_str(), LTT_DEFAULT);
		return false;
	}
	else if (!stricmp(argv[0], "add"))
	{
		// Need a reason
		if (argc < 3)
			return true;

		CPlayerWeenie *other = g_pWorld->FindPlayer(argv[1]);
		CClient *client;
		if (other)
			client = other->GetClient();


		if (other && client)
		{
			// If no duration then default to a perma ban
			if (argc < 4)
			{
				// Perma Ban
				g_pNetwork->AddBan(client->GetHostAddress()->sin_addr, pPlayer->GetName().c_str(), argv[2], client->GetAccountInfo().id);
				g_pNetwork->KickBannedClient(client, 0);
				g_pDBIO->UpdateBan(client->GetAccountInfo().id, 1);
			}
			else if (atol(argv[3]) > 0)
			{ 
				// Timed Ban
				g_pNetwork->AddBan(client->GetHostAddress()->sin_addr, pPlayer->GetName().c_str(), argv[2], atol(argv[3]), client->GetAccountInfo().id);
				g_pNetwork->KickBannedClient(client, atol(argv[3]));
				g_pDBIO->UpdateBan(client->GetAccountInfo().id, 1);
			}

			pPlayer->SendText("Ban added.", LTT_DEFAULT);
			SERVER_INFO << "Client" << client->GetSlot() << "(" << client->GetAccount() << ") has been banned.";
		}
		else
		{
			pPlayer->SendText("Couldn't find player.", LTT_DEFAULT);
		}

		return false;
	}
	else if (!stricmp(argv[0], "remove"))
	{
		if (argc < 2)
			return true;

		uint32_t ipaddr = inet_addr(argv[1]);
		uint32_t accountid = g_pNetwork->GetBanID(*(in_addr *)&ipaddr);

		if (ipaddr != 0 && ipaddr != 0xFFFFFFFF)
		{
			if (g_pNetwork->RemoveBan(*(in_addr *)&ipaddr))
			{
				pPlayer->SendText("Ban removed.", LTT_DEFAULT);
				g_pDBIO->UpdateBan(accountid, 0);
			}
			else
				pPlayer->SendText(csprintf("Couldn't find ban for IP \"%s\"", argv[1]), LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText("Please specify an IP address.", LTT_DEFAULT);
		}

		return false;
	}

	return true;
}

CLIENT_COMMAND_WITH_CUSTOM_NAME(_private, private, "<on/off>", "Prevents other users from teleporting to you.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("Privacy mode enabled. Other users can no longer teleport to you using the @tele command.", LTT_DEFAULT);
		pPlayer->m_bPrivacyMode = true;
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("Privacy mode disabled. Other users can now teleport to you using @tele command.", LTT_DEFAULT);
		pPlayer->m_bPrivacyMode = false;
	}

	return true;
}

CLIENT_COMMAND(instakill, "<radius>", "Deals damage to all nearby creatures.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	int radius = argc >= 1 ? atoi(argv[0]) : 30;

	if (radius < 0)
		radius = 0;

	std::list<CWeenieObject *> results;
	g_pWorld->EnumNearby(pPlayer, radius, &results);

	for (auto &entry : results)
	{
		if (!entry->IsCreature())
			continue;
		if (entry->_IsPlayer())
			continue;
		if (entry->IsDead())
			continue;
		if (!entry->IsAttackable())
			continue;

		DamageEventData dmgEvent;
		dmgEvent.weapon = NULL;
		dmgEvent.source = pPlayer;
		dmgEvent.target = entry;
		dmgEvent.damage_form = DF_PHYSICAL;
		dmgEvent.damage_type = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;		
		dmgEvent.damageAfterMitigation = dmgEvent.damageBeforeMitigation = 1000000;
		pPlayer->TryToDealDamage(dmgEvent);
	}

	return false;
}

CLIENT_COMMAND(attackable, "<on/off>", "Prevents you from being targeted by monsters.", BASIC_ACCESS, GENERAL_CATEGORY)
{
	if (!g_pConfig->EnableAttackableCommand() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You are now attackable.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(ATTACKABLE_BOOL, TRUE);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You are no longer attackable.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(ATTACKABLE_BOOL, FALSE);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(dropitemsondeath, "<on/off>", "Determines if you should or not drop items when you die.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You now drop items when you die.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(BONDED_INT, 0);
		pPlayer->NotifyIntStatUpdated(BONDED_INT);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You no longer drop items when you die.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(BONDED_INT, 1);
		pPlayer->NotifyIntStatUpdated(BONDED_INT);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(usecomponents, "<on/off>", "Allows you to cast spells without having the necessary components.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You now require components to cast spells.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(SPELL_COMPONENTS_REQUIRED_BOOL, TRUE);
		pPlayer->NotifyBoolStatUpdated(SPELL_COMPONENTS_REQUIRED_BOOL);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You no longer require components to cast spells.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(SPELL_COMPONENTS_REQUIRED_BOOL, FALSE);
		pPlayer->NotifyBoolStatUpdated(SPELL_COMPONENTS_REQUIRED_BOOL);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(learnschool, "school", "Adds all spells of the given school (war, life, item, creature).", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	std::string school = argv[0];

	int schoolNum = 0;

	if (school == "war")
	{
		schoolNum = 1;
	}
	else if (school == "life")
	{
		schoolNum = 2;
	}
	else if (school == "item")
	{
		schoolNum = 3;
	}
	else if (school == "creature")
	{
		schoolNum = 4;
	}
	else if (school == "void")
	{
		schoolNum = 5;
	}
	else
	{
		pPlayer->SendText("Invalid spell school.", LTT_DEFAULT);
		return false;
	}

	CSpellTable* pSpellTable = MagicSystem::GetSpellTable();

	for (auto spell : pSpellTable->_spellBaseHash)
	{
		if (spell.second._school == schoolNum)
		{
			unsigned int sp = spell.second._meta_spell._spell->_spell_id;
			pPlayer->m_Qualities.AddSpell(sp);
			BinaryWriter addSpellToSpellbook;
			addSpellToSpellbook.Write<uint32_t>(0x2C1);
			addSpellToSpellbook.Write<uint32_t>(sp);
			pPlayer->SendNetMessage(&addSpellToSpellbook, PRIVATE_MSG, TRUE, FALSE);
		}
	}
	return false;
}

CLIENT_COMMAND(addspellbyid, "id", "Adds a spell by ID", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;
	
	int id = atoi(argv[0]);

	CSpellTable * pSpellTable = MagicSystem::GetSpellTable();
	if (!id || !pSpellTable || !pSpellTable->GetSpellBase(id))
	{
		pPlayer->SendText("Invalid spell id.", LTT_DEFAULT);
		return false;
	}
	pPlayer->LearnSpell(id, true);
	
	return false;
}


#if 0
CLIENT_COMMAND(atkc, "", "Commence attack test.", ADMIN_ACCESS)
{
	pPlayer->NotifyCommenceAttack();
	return false;
}

CLIENT_COMMAND(atkd, "", "Attack done test.", ADMIN_ACCESS)
{
	pPlayer->NotifyAttackDone();
	return false;
}
#endif

#if 0
CLIENT_COMMAND(testburden, "", "", ADMIN_ACCESS)
{
	float burden = -1.0f;
	pPlayer->m_Qualities.InqLoad(burden);
	pPlayer->SendText(csprintf("Your burden is %f", burden), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(test, "<index>", "Performs the specified test.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	Position position = player_physobj->m_Position;

	switch (atoi(argv[0]))
	{
	case 1:
		{
			float goodZ = CalcSurfaceZ(position.objcell_id, position.frame.m_origin.x, position.frame.m_origin.y);
			const char* text = csprintf("CalcZ: %f PlayerZ: %f", goodZ, position.frame.m_origin.z);
			pPlayer->SendText(text, LTT_DEFAULT);
			break;
		}
	case 2:
		{
			double v = -1.0;
			int i = -1;
			pPlayer->m_Qualities.InqInt(ARMOR_LEVEL_INT, i);
			pPlayer->SendText(csprintf("ArmorMod: %d", i), LTT_DEFAULT);
			pPlayer->m_Qualities.InqFloat(ARMOR_MOD_VS_FIRE_FLOAT, v);
			pPlayer->SendText(csprintf("ArmorFireMod: %f", v), LTT_DEFAULT);
			pPlayer->m_Qualities.InqFloat(ARMOR_MOD_VS_BLUDGEON_FLOAT, v);
			pPlayer->SendText(csprintf("ArmorBludgeonMod: %f", v), LTT_DEFAULT);
			pPlayer->m_Qualities.InqFloat(RESIST_FIRE_FLOAT, v);
			pPlayer->SendText(csprintf("ResistFireMod: %f", v), LTT_DEFAULT);
			pPlayer->m_Qualities.InqFloat(RESIST_BLUDGEON_FLOAT, v);
			pPlayer->SendText(csprintf("ResistBludgeonMod: %f", v), LTT_DEFAULT);

			break;
		}
	case 3:
		{
			CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByName("Virindi Puppet", &pPlayer->m_Position.add_offset(Vector(0.0, 5, 1)), true);

			if (weenie)
			{
				CMonsterWeenie *monster = (CMonsterWeenie *)weenie;
			}

			break;
		}
	case 4:
		{
			CWeenieObject *weenie = g_pWorld->FindObject(pPlayer->m_LastAssessed);

			if (weenie && weenie->AsMonster() && !weenie->AsPlayer())
			{
				weenie->AsMonster()->MakeSpellcastingManager()->CreatureBeginCast(pPlayer->GetID(), 27);
			}

			break;
		}
	case 5:
		{
			pPlayer->set_state(pPlayer->m_PhysicsState & ~(GRAVITY_PS), TRUE);
			break;
		}
	case 6:
		{
			pPlayer->set_state(pPlayer->m_PhysicsState | (GRAVITY_PS), TRUE);
			break;
		}

	case 3: {
			CBaseItem *pSpawn = new CBoboHelm();
			pSpawn->m_Position = position;
			g_pWorld->CreateEntity(pSpawn);

			BinaryWriter* co = pSpawn->CreateMessage();
			//OutputConsoleBytes(co->GetData(), co->GetSize());
			delete co;

			break;
		}
	case 4: {
			CBaseItem *pSpawn = new CPhatRobe();
			pSpawn->m_Position = position;
			g_pWorld->CreateEntity(pSpawn);

			BinaryWriter* co = pSpawn->CreateMessage();
			//OutputConsoleBytes(co->GetData(), co->GetSize());
			delete co;

			break;
		}

	}

	return false;
}
#endif

CLIENT_COMMAND(animation, "<index> [speed=1]", "Plays a primary animation.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	fSpeed = min(10.0f, max(0.1f, fSpeed));
	
	pPlayer->_server_control_timestamp += 2;

	pPlayer->last_move_was_autonomous = false;

	MovementParameters params;
	params.action_stamp = ++pPlayer->m_wAnimSequence;
	params.speed = fSpeed;
	params.autonomous = 0;
	params.modify_interpreted_state = 1;

	MovementStruct mvs;
	mvs.motion = GetCommandID(wIndex);
	mvs.params = &params;
	mvs.type = MovementTypes::RawCommand;
	pPlayer->get_minterp()->PerformMovement(mvs);
	pPlayer->Animation_Update();

	return false;
}

CLIENT_COMMAND(animationother, "<index> [speed=1]", "Plays a primary animation.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	CWeenieObject *other = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!other)
		return false;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	fSpeed = min(10.0f, max(0.1f, fSpeed));

	other->_server_control_timestamp += 2;
	other->last_move_was_autonomous = false;

	MovementParameters params;
	params.action_stamp = ++other->m_wAnimSequence;
	params.speed = fSpeed;
	params.autonomous = 0;
	params.modify_interpreted_state = 1;

	MovementStruct mvs;
	mvs.motion = GetCommandID(wIndex);
	mvs.params = &params;
	mvs.type = MovementTypes::RawCommand;
	other->get_minterp()->PerformMovement(mvs);
	other->Animation_Update();

	return false;
}

CLIENT_COMMAND(setprefix, "<1 for on, 0 for off>", "Adds a prefix to your name such as +Donor.", DONOR_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	std::string nameString = pPlayer->InqStringQuality(NAME_STRING, "");
	ReplaceString(nameString, "+Contributor ", "");
	ReplaceString(nameString, "+Advocate ", "");
	ReplaceString(nameString, "+Donor ", "");
	ReplaceString(nameString, "+Sentinel ", "");
	ReplaceString(nameString, "+Admin ", "");
	ReplaceString(nameString, "+", "");

	if (atoi(argv[0]))
	{
		if (player_client->GetAccessLevel() >= ADMIN_ACCESS)
			nameString = std::string("+") + nameString;
		else if (player_client->GetAccessLevel() >= SENTINEL_ACCESS)
			nameString = std::string("+Sentinel ") + nameString;
		else if (player_client->GetAccessLevel() >= ADVOCATE_ACCESS)
			nameString = std::string("+Advocate ") + nameString;
		else if (player_client->GetAccessLevel() >= DONOR_ACCESS)
			nameString = std::string("+Donor ") + nameString;
	}

	if (strcmp(nameString.c_str(), pPlayer->InqStringQuality(NAME_STRING, "").c_str()))
	{
		pPlayer->m_Qualities.SetString(NAME_STRING, nameString.c_str());
		pPlayer->NotifyStringStatUpdated(NAME_STRING, false);
	}

	return false;
}

#if 0
CLIENT_COMMAND(setbael, "", "Sets you to be Bael'Zharon.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_pWeenieFactory->ApplyWeenieDefaults(pPlayer, 0x2275);
	pPlayer->m_Qualities.SetInt(RADARBLIP_COLOR_INT, 2);
	pPlayer->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, Baelzharon_PKStatus);
	pPlayer->m_Qualities.SetInt(CONTAINERS_CAPACITY_INT, 7);
	pPlayer->m_Qualities.SetInt(ITEMS_CAPACITY_INT, 200);

	player_client->GetEvents()->BeginLogout();

	return false;
}
#endif

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(setplayer, "[wcid]", "Sets your Player Character defaults to that of the given wcid.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	g_pWeenieFactory->ApplyWeenieDefaults(pPlayer, atoi(argv[0]));
	pPlayer->m_Qualities.SetInt(RADARBLIP_COLOR_INT, 2);
	pPlayer->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, Baelzharon_PKStatus);
	pPlayer->m_Qualities.SetInt(CONTAINERS_CAPACITY_INT, 7);
	pPlayer->m_Qualities.SetInt(ITEMS_CAPACITY_INT, 200);

	player_client->GetEvents()->BeginLogout();

	return false;
}
#endif

CLIENT_COMMAND(setname, "[name]", "Changes the last assessed target's name.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (pPlayer->m_LastAssessed)
	{
		CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
		if (target)
		{
			target->m_Qualities.SetString(NAME_STRING, argv[0]);
			target->NotifyStringStatUpdated(NAME_STRING, false);

			pPlayer->SendText("Target name changed successfully.", LTT_DEFAULT);
		}
	}

	return false;
}

#if 0
CLIENT_COMMAND(setmodel, "[monster]", "Changes your model to a monster.", ADMIN_ACCESS)
{
	pPlayer->SendText("Command disabled.", LTT_DEFAULT);
	return false;

	/*
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData(argv[0]);

	if (!pMonsterInfo)
	{
		pPlayer->SendText("Couldn't find that monster!", LTT_DEFAULT);
		return false;
	}

	float fScale = 1.0f;
	if (argc >= 2)
	{
		fScale = (float)atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	pPlayer->m_miBaseModel = pMonsterInfo->appearance;
	pPlayer->SetSetupID(pMonsterInfo->physics.setup_id);
	pPlayer->SetMotionTableID(pMonsterInfo->physics.mtable_id);
	pPlayer->SetSoundTableID(pMonsterInfo->physics.stable_id);
	pPlayer->SetPETableID(pMonsterInfo->physics.phstable_id);
	pPlayer->m_scale = pMonsterInfo->physics.object_scale * fScale;

	pPlayer->UpdateEntity(pPlayer);
	return false;
	*/
}
#endif


CLIENT_COMMAND(invis, "<on/off>", "Turn invisibility on or off", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You are now invisible.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(VISIBILITY_BOOL, false);
		pPlayer->NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You are now visible.", LTT_DEFAULT);
		pPlayer->m_Qualities.RemoveBool(VISIBILITY_BOOL);
		pPlayer->NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(radar, "<on/off>", "Turn Radar visibility on or off.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You are now visible on radar.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, 4);
		pPlayer->NotifyIntStatUpdated(SHOWABLE_ON_RADAR_INT, false);
	}
	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You are no longer visible on radar.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, 1);
		pPlayer->NotifyIntStatUpdated(SHOWABLE_ON_RADAR_INT, false);
	}
	else
		return true;

	return false;
}


/*
CLIENT_COMMAND(motion, "<index> [speed=1]", "Plays a sequenced animation.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;

	pPlayer->Animation_PlaySimpleAnimation(wIndex, fSpeed);

	return false;
}
*/

void SendDungeonInfo(CPlayerWeenie* pPlayer, DungeonDesc_t* pInfo)
{
	pPlayer->SendText(csprintf("Information for ID %04X:", pInfo->wBlockID), LTT_DEFAULT);
	pPlayer->SendText(csprintf("Name: %s\nAuthor: %s\nDescription: %s", pInfo->szDungeonName, pInfo->szAuthor, pInfo->szDescription), LTT_DEFAULT);
}

void SendDungeonInfo(CPlayerWeenie* pPlayer, WORD wBlockID)
{
	uint32_t bitshift = wBlockID * 65536;
	DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(wBlockID);

	if (!pInfo)
	{
		if (!g_pWorld->DungeonExists(wBlockID))
			pPlayer->SendText("That's not a dungeon.", LTT_DEFAULT);
		else
			pPlayer->SendText("That dungeon has not been described.", LTT_DEFAULT);
	}
	else
		SendDungeonInfo(pPlayer, pInfo);
}

CLIENT_COMMAND(exportrecipe, "<recipeid>", "Export recipe number", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("No recipe ID given", LTT_DEFAULT);
		return true;
	}
	
	uint32_t recipeId = stoi(argv[0], NULL, 10);

	// Get CCraftOperation that aligns with recipeid
	CCraftOperation *craft = g_pPortalDataEx->_craftTableData._operations.lookup(recipeId);

	if (!craft)
	{
		pPlayer->SendText(csprintf("Unable to find recipe ID: %s.", recipeId), LTT_DEFAULT);
		return true;
	}

	JsonCraftOperation jcraft(*craft, recipeId);

	json recipeData;
	//craft->PackJson(recipeData);
	jcraft.PackJson(recipeData);

	string dataFile = std::to_string(recipeId) + ".json";

	ofstream ofstream(dataFile);
	if (!ofstream.bad())
	{
		ofstream << recipeData;
		ofstream.close();
	}

	pPlayer->SendText("RecipeID saved in recipe folder.", LTT_DEFAULT);

	return true;
}

CLIENT_COMMAND(importrecipe, "<recipeid>", "Import a recipes", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("No recipe ID given", LTT_DEFAULT);
		return true;
	}

	uint32_t recipeId = stoi(argv[0], NULL, 10);

	// Get CCraftOperation that aligns with recipeid
	CCraftOperation *craft = g_pPortalDataEx->_craftTableData._operations.lookup(recipeId);

	if (!craft)
	{
		pPlayer->SendText(csprintf("Unable to find recipe ID: %s.", recipeId), LTT_DEFAULT);
		return true;
	}

	string dataFile = std::to_string(recipeId) + ".json";

	ifstream ifstream(dataFile.c_str());
	if (ifstream.is_open())
	{
		json recipeData;
		ifstream >> recipeData;
		ifstream.close();
		
		CCraftOperation newcraft;
		newcraft.UnPackJson(recipeData);
		g_pPortalDataEx->_craftTableData._operations[recipeId] = newcraft;
	}

	pPlayer->SendText(csprintf("RecipeID %d loaded and updated.", recipeId), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(dungeon, "<command>", "Dungeon commands.", BASIC_ACCESS, EXPLORE_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Explorer mode.", LTT_DEFAULT);
		return false;
	}

	if (!stricmp(argv[0], "help"))
	{
		if (argc < 2)
		{
			pPlayer->SendText("Dungeon commands:", LTT_DEFAULT);
			pPlayer->SendText("@dungeon help [command]\n@dungeon info [block/name]\n@dungeon list\n@dungeon random\n@dungeon search <substring>\n@dungeon set <name>\n@dungeon tele <block/name>\n@dungeon unknown", LTT_DEFAULT);
			return false;
		}

		if (!stricmp(argv[1], "info"))
		{
			pPlayer->SendText("@dungeon info - Retrieves information about your current dungeon.", LTT_DEFAULT);
			pPlayer->SendText("@dungeon info [block] - Retrieves dungeon information using a hex BlockID.", LTT_DEFAULT);
			pPlayer->SendText("@dungeon info [name] - Retrieves dungeon information by name.", LTT_DEFAULT);
		}
		else if (!stricmp(argv[1], "list"))
			pPlayer->SendText("@dungeon list - Displays a list of named dungeons.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "set"))
			pPlayer->SendText("@dungeon set <name> - Sets your current dungeon's name, and its drop point.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "tele"))
		{
			pPlayer->SendText("@dungeon tele <block> - Teleports to a dungeon by BlockID.", LTT_DEFAULT);
			pPlayer->SendText("@dungeon tele <name> - Teleports to a dungeon by name.", LTT_DEFAULT);
		}
		else if (!stricmp(argv[1], "unknown"))
			pPlayer->SendText("@dungeon unknown - List of unnamed dungeon BlockIDs.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "random"))
			pPlayer->SendText("@dungeon random - Teleports you to a random dungeon.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "search"))
			pPlayer->SendText("@dungeon search <substring> - Searchs for dungeon names that contain the substring.", LTT_DEFAULT);
		else
			pPlayer->SendText("Unknown @dungeon command. Type @dungeon help for a list of valid choices.", LTT_DEFAULT);

		return false;
	}
	else if (!stricmp(argv[0], "unknown"))
	{
		LocationMap* pDungeons = g_pWorld->GetDungeons();
		LocationMap::iterator i = pDungeons->begin();
		LocationMap::iterator e = pDungeons->end();

		uint32_t dwCount = 0;
		std::string UnkList = "Unknown Dungeons:\n";
		while (i != e)
		{
			//uint32_t dwCell = i->first;
			WORD wBlockID = BLOCK_WORD(i->first);

			if (!g_pWorld->GetDungeonDesc(wBlockID))
			{
				UnkList += csprintf("%04X, ", wBlockID);
				dwCount++;
			}
			i++;
		}
		UnkList += csprintf("\nEnd of Unknown Dungeons (%lu unknown)", dwCount);

		pPlayer->SendText(UnkList.c_str(), LTT_DEFAULT);
	}
	else if (!stricmp(argv[0], "tele"))
	{
		if (argc < 2)
			return true;

		if (strlen(argv[1]) < 5)
		{
			//by ID
			WORD wBlockID = (WORD)strtoul(argv[1], NULL, 16);
			bool forceFindDrop = false;

			if (argc >= 3)
			{
				forceFindDrop = atoi(argv[2]) ? true : false;
			}

			if (!wBlockID)
			{
				pPlayer->SendText("Bad block ID. Please use a valid hex value.", LTT_DEFAULT);
				return false;
			}

			DungeonDesc_t *pInfo = g_pWorld->GetDungeonDesc(wBlockID);

			if (!pInfo || forceFindDrop)
			{
				if (!g_pWorld->DungeonExists(wBlockID))
				{
					pPlayer->SendText("That dungeon doesn't exist!", LTT_DEFAULT);
				}
				else
				{
					Position position = g_pWorld->FindDungeonDrop(wBlockID);
					if (!position.objcell_id)
						pPlayer->SendText("Couldn't find a drop zone there.", LTT_DEFAULT);
					else
					{
						pPlayer->SendText(csprintf("Teleporting you to BlockID %04X ..", wBlockID), LTT_DEFAULT);
						pPlayer->Movement_Teleport(position);
					}
				}
			}
			else
			{
				pPlayer->SendText(csprintf("Teleporting you to %s (%04X) ..", pInfo->szDungeonName, pInfo->wBlockID), LTT_DEFAULT);
				pPlayer->Movement_Teleport(pInfo->position);
			}
		}
		else
		{
			//by name
			const char *name = argv[1];

			DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(name);
			if (pInfo)
			{
				pPlayer->SendText(csprintf("Teleporting you to %s (%04X) ..", pInfo->szDungeonName, pInfo->wBlockID), LTT_DEFAULT);
				pPlayer->Movement_Teleport(pInfo->position);
			}
			else
				pPlayer->SendText("There is no dungeon by that name. Use @dungeon list for named dungeons. Remember to use quotes.", LTT_DEFAULT);
		}
	}
	else if (!stricmp(argv[0], "set"))
	{
		if (argc < 2)
			return true;

		uint32_t dwCell = pPlayer->GetLandcell();

		if (CELL_WORD(dwCell) < 0xFF) {
			pPlayer->SendText("Go inside first, silly!", LTT_DEFAULT);
			return false;
		}

		if (strlen(argv[1]) < 5) {
			pPlayer->SendText("Please enter a name of at least 5 characters in length.", LTT_DEFAULT);
			return false;
		}
		if (strlen(argv[1]) > 80) {
			pPlayer->SendText("Please enter a shorter name.", LTT_DEFAULT);
			return false;
		}

		time_t ltime = time(NULL);
		tm*	tptr = localtime(&ltime);
		char* szTime;

		if (tptr)
		{
			szTime = asctime(tptr);
			szTime[strlen(szTime) - 1] = 0; //Remove \n
		}
		else
			szTime = "???";

		WORD wBlockID = BLOCK_WORD(dwCell);
		const char* szName = argv[1];
		const char* szAuthor = pPlayer->GetName().c_str();
		const char* szDescription = csprintf("Recorded %s", szTime);

		g_pWorld->SetDungeonDesc(wBlockID, szName, szAuthor, szDescription, player_physobj->m_Position);

		pPlayer->SendText(csprintf("Dungeon information set. (%04X = \"%s\")", wBlockID, szName), LTT_DEFAULT);
	}
	else if (!stricmp(argv[0], "info"))
	{
		if (argc < 2)
		{
			uint32_t dwCell = pPlayer->GetLandcell();
			if (CELL_WORD(dwCell) < 0xFF) {
				pPlayer->SendText("Go inside first, silly!", LTT_DEFAULT);
				return false;
			}

			SendDungeonInfo(pPlayer, BLOCK_WORD(dwCell));

			return false;
		}

		if (strlen(argv[1]) < 6)
		{
			//by ID
			WORD wBlockID = (WORD)strtoul(argv[1], NULL, 16);

			if (!wBlockID)
			{
				pPlayer->SendText("Bad block ID. Please use a valid hex value. Remember to use quotes around names!", LTT_DEFAULT);
				return false;
			}

			SendDungeonInfo(pPlayer, wBlockID);
		}
		else
		{
			//by name
			const char* szName = argv[1];

			DungeonDesc_t* pInfo = g_pWorld->GetDungeonDesc(szName);
			if (pInfo)
				SendDungeonInfo(pPlayer, pInfo);
			else
				pPlayer->SendText("There is no dungeon by that name. Use @dungeon list for named dungeons. Use quotes around names.", LTT_DEFAULT);
		}
	}
	else if (!stricmp(argv[0], "list"))
	{
		DungeonDescMap* pDDs = g_pWorld->GetDungeonDescs();
		DungeonDescMap::iterator i = pDDs->begin();
		DungeonDescMap::iterator e = pDDs->end();

		std::string DL = "Dungeon List:";
		while (i != e)
		{
			DL += csprintf("\n%04X - %s", i->second.wBlockID, i->second.szDungeonName);
			i++;
		}
		DL += csprintf("\nEnd of Dungeon List (%lu named dungeons)", (uint32_t)pDDs->size());

		pPlayer->SendText(DL.c_str(), LTT_DEFAULT);
	}
	else if (!stricmp(argv[0], "search"))
	{
		if ((argc < 2) || (strlen(argv[1]) <= 0))
		{
			pPlayer->SendText("You must enter a search string.", LTT_DEFAULT);
			return 0;
		}

		DungeonDescMap* pDDs = g_pWorld->GetDungeonDescs();
		DungeonDescMap::iterator i = pDDs->begin();
		DungeonDescMap::iterator e = pDDs->end();

		uint32_t dwCount = 0;
		std::string DL = "Dungeon Matches:";
		while (i != e)
		{
			std::string name(i->second.szDungeonName);
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			std::string arg(argv[1]);
			std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
			//static char szDungeonName[100];
			//strncpy(szDungeonName, i->second.szDungeonName, 100);

			//if (strstr(strlwr(szDungeonName), strlwr(argv[1])))
			if (name.find(arg))
			{
				DL += csprintf("\n%04X - %s", i->second.wBlockID, i->second.szDungeonName);
				dwCount++;
			}
			i++;
		}
		DL += csprintf("\nEnd of List (%lu matches)", dwCount);

		pPlayer->SendText(DL.c_str(), LTT_DEFAULT);
	}
	else if (!stricmp(argv[0], "random"))
	{
		Position position = g_pWorld->FindDungeonDrop();

		if (!position.objcell_id)
		{
			pPlayer->SendText("No dungeons to teleport to.", LTT_DEFAULT);
		}
		else
		{
			WORD wBlockID = BLOCK_WORD(position.objcell_id);
			DungeonDesc_t *pDD;
			if (pDD = g_pWorld->GetDungeonDesc(wBlockID))
			{
				pPlayer->SendText(csprintf("Teleporting you to %s ..", pDD->szDungeonName), LTT_DEFAULT);
				position = pDD->position;
			}
			else
				pPlayer->SendText("Teleporting you to a random dungeon..", LTT_DEFAULT);

			pPlayer->Movement_Teleport(position);
		}
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(player, "<command>", "Player commands.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return 1;

	if (!stricmp(argv[0], "help"))
	{
		if (argc < 2)
		{
			pPlayer->SendText("Player commands:", LTT_DEFAULT);
			pPlayer->SendText("@player help [command]\n@player info <name>\n@player list\n@player tele <name>", LTT_DEFAULT);
			return false;
		}

		if (!stricmp(argv[1], "info"))
			pPlayer->SendText("@player info <name> - Retrieves information on a player, by name.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "list"))
			pPlayer->SendText("@player list - Displays a list of players currently logged in.", LTT_DEFAULT);
		else if (!stricmp(argv[1], "tele"))
		{
			pPlayer->SendText("@player tele <name> - Teleports to a player by name.", LTT_DEFAULT);
			pPlayer->SendText("NOTE: This is the same as the @tele command.", LTT_DEFAULT);
		}
		else
			pPlayer->SendText("Unknown @player command. Type @player help for a list of valid choices.", LTT_DEFAULT);

		return false;
	}
	else if (!stricmp(argv[0], "tele"))
	{
		pPlayer->SendText("NOTE: Use @tele instead. ;)", LTT_DEFAULT);

		tele(player_client, pPlayer, player_physobj, &argv[1], argc - 1);
	}
	else if (!stricmp(argv[0], "info"))
	{
		if (argc < 2)
			return true;

		CPlayerWeenie *pOther = g_pWorld->FindPlayer(argv[1]);

		if (!pOther)
			pPlayer->SendText(csprintf("Couldn't find player \"%s\"", argv[1]), LTT_DEFAULT);
		else
		{
			if (pPlayer->IsSentinel())
			{
				const char *info = csprintf(
					"Player Info:\nGUID: 0x%08X\nName: %s\nLocation: %08X %.1f %.1f %.1f",
					pOther->GetID(), pOther->GetName().c_str(), pOther->GetLandcell(),
					pOther->m_Position.frame.m_origin.x, pOther->m_Position.frame.m_origin.y, pOther->m_Position.frame.m_origin.z);

				pPlayer->SendText(info, LTT_DEFAULT);
			}
			else
			{
				pPlayer->SendText("You do not have the access to use this command.", LTT_DEFAULT);
			}
		}
	}
	else if (!stricmp(argv[0], "list"))
	{
		PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
		
		std::string playerList = "Player List:";
		for (auto &entry : *pPlayers)
		{
			if (!entry.second->GetClient())
				continue;

			if (player_client->GetAccessLevel() >= SENTINEL_ACCESS)
			{
				playerList += csprintf("\n%s (Account: %s IP: %s)",
					entry.second->GetName().c_str(),
					entry.second->GetClient()->GetAccount(),
					inet_ntoa(entry.second->GetClient()->GetHostAddress()->sin_addr));
			}
			else
			{
				playerList += csprintf("\n%s", entry.second->GetName().c_str());
			}
		}
		playerList += csprintf("\nEnd of Player List (%u) - Unique IPs (%u)", (uint32_t)pPlayers->size(), g_pNetwork->GetUniques());

		pPlayer->SendText(playerList.c_str(), LTT_DEFAULT);
	}
	else
		return true;

	return false;
}

/*
CLIENT_COMMAND(doomshard, "[palette=0xBF7]", "Spawns a doom shard.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	WORD palette = 0xBF7;

	if (argc >= 1)
		palette = (unsigned short)strtoul(argv[0], NULL, 16);

	CMonsterWeenie *pDoomShard = new CMonsterWeenie();
	pDoomShard->SetSetupID(0x02000700);
	pDoomShard->SetScale(1.6f);
	pDoomShard->SetName("Doom Shard");
	pDoomShard->SetInitialPosition(pPlayer->GetPosition());

	pDoomShard->m_miBaseModel.dwBasePalette = 0xBEF;
	pDoomShard->m_miBaseModel.lPalettes.push_back(PaletteRpl(palette, 0x00, 0x00));

	g_pWorld->CreateEntity(pDoomShard);
	return false;
}
*/

#if 0
CLIENT_COMMAND(spawnbsd, "", "Spawn bsd for testing if the data is available.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	g_pGameDatabase->SpawnBSD();
	return false;
}
#endif

CLIENT_COMMAND(freecam, "", "Allows free camera movement.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	pPlayer->set_state(pPlayer->m_PhysicsState ^ (uint32_t)(PARTICLE_EMITTER_PS), TRUE);
	return false;
}


#if 0
CLIENT_COMMAND(spawnsetup, "[id]", "Spawns something by setup ID using default palettes, animations, effect sets etc.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}
	
	uint32_t setupID = strtoul(argv[0], NULL, 16);
	if (!(setupID & 0xFF000000))
		setupID |= 0x02000000;

	CSetup *pSetup = CSetup::Get(setupID);
	if (!pSetup)
	{
		pPlayer->SendText(csprintf("Could not find setup ID 0x%08X", setupID), 1);
		return false;
	}

	CMonsterWeenie *pSpawn = new CMonsterWeenie();
	pSpawn->SetSetupID(setupID);
	pSpawn->SetName(csprintf("Model #%08X", setupID));
	pSpawn->SetInitialPosition(pPlayer->GetPosition());

	pSpawn->SetSoundTableID(pSetup->default_stable_id);
	pSpawn->SetMotionTableID(pSetup->default_mtable_id);
	pSpawn->SetPETableID(pSetup->default_phstable_id);

	g_pWorld->CreateEntity(pSpawn);

	pSpawn->SetLongDescription(csprintf("This monster was spawned by %s.\nSetup #: %08X\n", pPlayer->GetName().c_str(), setupID));

	CSetup::Release(pSetup);
	return false;
}

CLIENT_COMMAND(spawn2, "[name] [replacement index] [palette index] [shade from 0 to 1]", "Spawns a custom decorated monster.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData(argv[0]);

	if (!pMonsterInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (argc < 2)
	{
		pPlayer->SendText(csprintf("Monster has %u replacement types.", g_ClothingCache.GetNumTablesOfSetupID(pMonsterInfo->physics.setup_id)), LTT_DEFAULT);
		return false;
	}

	ClothingTable *pCT = g_ClothingCache.GetTableByIndexOfSetupID(pMonsterInfo->physics.setup_id, atoi(argv[1]));

	if (!pCT)
	{
		pPlayer->SendText(csprintf("Bad replacement index. Monster has %u replacement types.", g_ClothingCache.GetNumTablesOfSetupID(pMonsterInfo->physics.setup_id)), LTT_DEFAULT);
		return true;
	}

	if (argc < 3)
	{
		pPlayer->SendText(csprintf("Monster replacement has %u palette variations.", pCT->_paletteTemplatesHash.size()), LTT_DEFAULT);
		ClothingTable::Release(pCT);
		return false;
	}

	unsigned int paletteIndex = (unsigned)atoi(argv[2]);
	if (paletteIndex >= pCT->_paletteTemplatesHash.size())
	{
		pPlayer->SendText(csprintf("Bad palette index. Monster replacement has %u palette variations.", pCT->_paletteTemplatesHash.size()), LTT_DEFAULT);
		ClothingTable::Release(pCT);
		return false;
	}
	
	float fScale = 1.0f;
	bool bAnimate = false;

	CWeenieObject *pMonster = g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

	// Modify these
	pMonster->SetInitialPosition(pPlayer->GetPosition());
	pMonster->m_bObjDescOverride = true;

	CSetup *pSetup = CSetup::Get(pMonster->GetSetupID());
	if (pSetup)
	{
		if (pSetup->num_parts > 0 && pSetup->parts)
		{
			uint32_t gfxID = pSetup->parts[0];
			CGfxObj *gfxobj = CGfxObj::Get(gfxID);

			if (gfxobj)
			{
				for (uint32_t i = 0; i < gfxobj->num_surfaces; i++)
				{
					if (gfxobj->m_rgSurfaces[i])
					{
						pMonster->m_ObjDescOverride.paletteID = gfxobj->m_rgSurfaces[i]->GetOriginalPaletteID();
					}
				}
				CGfxObj::Release(gfxobj);
			}
		}

		CSetup::Release(pSetup);
	}

	float shade = 1.0f;
	if (argc >= 4)
	{
		shade = atof(argv[3]);
		if (shade < 0)
			shade = 0;
		if (shade > 1)
			shade = 1;
	}

	uint32_t pt = 0;
	for (auto &it : pCT->_paletteTemplatesHash)
	{
		if (paletteIndex == 0)
		{
			pt = it.first;
			break;
		}

		paletteIndex--;
	}

	pMonster->m_ObjDescOverride.Clear();
	pCT->BuildObjDesc(pMonsterInfo->physics.setup_id, pt, &ShadePackage(shade), &pMonster->m_ObjDescOverride);
	ClothingTable::Release(pCT);

	// Add and spawn it
	g_pWorld->CreateEntity(pMonster);

	return false;
}
#endif

bool IsNumeric(const char *text)
{
	if (!*text)
		return false;

	if (*text == '-')
		text++;

	do
	{
		if (*text < '0' || *text > '9')
			return false;
	} while (*++text);

	return true;
}

bool IsHexNumeric(const char *text)
{
	if (text[0] != '0' || text[1] != 'x')
		return false;

	text += 2;

	do
	{
		if ((*text < '0' || *text > '9') && (*text < 'a' || *text > 'f') && (*text < 'A' || *text > 'F'))
			return false;
	} while (*++text);

	return true;
}

#if 0
CLIENT_COMMAND(spawnpreset, "[preset]", "Spawn preset.", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	int preset = atoi(argv[0]);

	switch (preset)
	{
	case 1:
		g_pWeenieFactory->CreateWeenieByClassID(23781, &pPlayer->m_Position, true);
		g_pWeenieFactory->CreateWeenieByClassID(23789, &pPlayer->m_Position, true);
		g_pWeenieFactory->CreateWeenieByClassID(24206, &pPlayer->m_Position, true);
		break;

	case 2:
		g_pWeenieFactory->CreateWeenieByClassID(6600, &pPlayer->m_Position, true);
		g_pWeenieFactory->CreateWeenieByClassID(6606, &pPlayer->m_Position, true);
		g_pWeenieFactory->CreateWeenieByClassID(11986, &pPlayer->m_Position, true);
		break;

	case 3:
		g_pWeenieFactory->CreateWeenieByClassID(100005, &pPlayer->m_Position, true);
		g_pWeenieFactory->CreateWeenieByClassID(100006, &pPlayer->m_Position, true);
		break;
	}

	return false;
}
#endif

bool g_bRestartOnNextTick = false;

#if 0
CLIENT_COMMAND(restartserver, "<yes>", "Restarts the server.", ADVOCATE_ACCESS)
{
	if (argc < 1)
		return true;

	if (_stricmp(argv[0], "yes"))
		return true;

	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < ADMIN_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	g_bRestartOnNextTick = true;
	return false;
}
#endif

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(debugresistances, "", "", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (!target)
	{
		pPlayer->SendText("Assess an object first.", LTT_DEFAULT);
		return false;
	}

	pPlayer->SendText(csprintf("Resistances of the last assessed object:\n"
		"Slash: %.2f (%.2f base)\n"
		"Pierce: %.2f (%.2f base)\n"
		"Bludgeon: %.2f (%.2f base)\n"
		"Fire: %.2f (%.2f base)\n"
		"Cold: %.2f (%.2f base)\n"
		"Acid: %.2f (%.2f base)\n"
		"Lightning: %.2f (%.2f base)\n",
		target->InqFloatQuality(RESIST_SLASH_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_SLASH_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_PIERCE_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_PIERCE_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_BLUDGEON_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_BLUDGEON_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_FIRE_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_FIRE_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_COLD_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_COLD_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_ACID_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_ACID_FLOAT, 1.0f, TRUE),
		target->InqFloatQuality(RESIST_ELECTRIC_FLOAT, 1.0f, FALSE),
		target->InqFloatQuality(RESIST_ELECTRIC_FLOAT, 1.0f, TRUE)), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(activeevents, "", "", ADMIN_ACCESS, SERVER_CATEGORY)
{
	std::string eventText = "Enabled events:";

	for (auto &entry : g_pGameEventManager->_gameEvents)
	{
		if(g_pGameEventManager->IsEventStarted(entry.first.c_str()))
		{		
			eventText += "\n";
			eventText += entry.first;
		}
	}

	pPlayer->SendText(eventText.c_str(), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(startevent, "[event]", "Starts an event.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	auto &events = g_pGameEventManager->_gameEvents;

	std::string eventText = "Event started.";

	std::string normalizedEventName = g_pGameEventManager->NormalizeEventName(argv[0]);
	
	if (GameEventDef *eventDesc = events.lookup(normalizedEventName))
	{
		if (eventDesc->_eventState != GameEventState::On_GameEventState)
		{
			g_pGameEventManager->StartEvent(normalizedEventName.c_str());
		}
		else {
			eventText = "Event already started.";
		}
	}
	else {
		eventText = "Event not found! Event must be present in events.json and MUST be present there in lowercase.";
	}

	pPlayer->SendText(eventText.c_str(), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(stopevent, "[event]", "Stops an event.", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
		return true;

	auto &events = g_pGameEventManager->_gameEvents;

	std::string eventText = "Event stopped.";

	std::string normalizedEventName = g_pGameEventManager->NormalizeEventName(argv[0]);

	if (GameEventDef *eventDesc = events.lookup(normalizedEventName.c_str()))
	{
		if (eventDesc->_eventState != GameEventState::Off_GameEventState)
		{
			g_pGameEventManager->StopEvent(normalizedEventName.c_str());
		}
		else {
			eventText = "Event not active.";
		}
	}
	else {
		eventText = "Event not found! Event must be present in events.json and MUST be present there in lowercase.";
	}
	pPlayer->SendText(eventText.c_str(), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(myquests, "", "", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	for (auto &entry : pPlayer->_questTable._quest_table)
	{
		std::string text = csprintf("%s - %u solves (%d)", entry.first.c_str(), entry.second._num_completions, entry.second._real_time);

		if (QuestDef *questDef = CQuestDefDB::GetQuestDef(entry.first.c_str()))
		{
			text += csprintf("\"%s\" %d %d", questDef->_fullname.c_str(), questDef->_maxsolves, questDef->_mindelta);
		}

		pPlayer->SendText(text.c_str(), LTT_DEFAULT);
	}

	if (pPlayer->_questTable._quest_table.empty())
		pPlayer->SendText("Quest list is empty.", LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(erasequest, "<name>", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;


	pPlayer->_questTable.RemoveQuest(argv[0]);

	pPlayer->SendText(csprintf("%s erased.",argv[0]), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(clearquests, "", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	pPlayer->_questTable.PurgeQuests();

	pPlayer->SendText(csprintf("Quests cleared."), LTT_DEFAULT);
	return false;
}


CLIENT_COMMAND(stampquest, "<name>", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.StampQuest(argv[0]);

	pPlayer->SendText(csprintf("%s stamped.", argv[0]), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(setquest, "<name> [# of times to stamp]", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 2)
		return true;

	if (argc > 1)
	{
		pPlayer->_questTable.SetQuestCompletions(argv[0], atoi(argv[1]));
		pPlayer->SendText(csprintf("%s stamped %s times.", argv[0], argv[1]), LTT_DEFAULT);
	}
	return false;
}

CLIENT_COMMAND(incquest, "<name> [# of times to increment]", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.IncrementQuest(argv[0], atoi(argv[1]));

	pPlayer->SendText(csprintf("%s incremented by %s.", argv[0], argv[1]), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(decquest, "<name> [# of times to increment]", "", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.DecrementQuest(argv[0], atoi(argv[1]));

	pPlayer->SendText(csprintf("%s decremented by %s.", argv[0], argv[1]), LTT_DEFAULT);
	return false;
}
#endif

CLIENT_COMMAND(spawntreasure, "<tier>, <amount>, <category>", "Spawn treasure of a specific tier.", BASIC_ACCESS, SPAWN_CATEGORY)
{
	if (argc < 1)
		return true;

	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Experience bonus mode.", LTT_DEFAULT);
		return false;
	}
	   
	int tier = atoi(argv[0]);
	int num = 1;
	int cat;

		if (argc < 1)
			return true;
		if (argc >= 2)
			num = atoi(argv[1]);
		if (argc >= 3)
			cat = atoi(argv[2]);

		switch (pPlayer->GetAccessLevel())
		{
		case BASIC_ACCESS:
			num = min(num, 10);
			break;
		case SENTINEL_ACCESS:
			num = min(num, 25);
			break;
		default:
			break;
		}


		for (int i = 0; i < num; i++)
		{
			if (argc < 3)
				cat = (eTreasureCategory)getRandomNumber(2, 8);

			CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure((tier), (eTreasureCategory)cat);
			//CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(atoi(argv[0]), eTreasureCategory::TreasureCategory_Armor);

			if (treasure)
			{
				treasure->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-2.0, 2.0), Random::GenFloat(-2.0, 2.0), 1.0)));

				if (!g_pWorld->CreateEntity(treasure))
				{
					treasure->m_bDontClear = false;
					delete treasure;
					return false;
				}
			}
			else
				continue;
		}

	return false;
}

CLIENT_COMMAND(spawnsalvagewcid, "wcid", "Spawn full sack of salvage.", BASIC_ACCESS, SPAWN_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Experience bonus mode.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	CWeenieObject *weenieTemplate;

	if (!IsNumeric(argv[0]))
		return false;

	int salvageWcid = atoi(argv[0]);

	weenieTemplate = g_pWeenieFactory->CreateWeenieByClassID(salvageWcid, NULL, false);

	if(!weenieTemplate)
		return false;

	int itemIntType;
	if (!weenieTemplate->m_Qualities.InqInt(ITEM_TYPE_INT, itemIntType) || itemIntType != 1073741824)
	{
		delete weenieTemplate;
		return false;
	}

	int itemUsableType;
	if (!weenieTemplate->m_Qualities.InqInt(ITEM_USEABLE_INT, itemUsableType) || itemUsableType != 524296)
	{
		delete weenieTemplate;
		return false;
	}

	weenieTemplate->m_Qualities.SetInt(STRUCTURE_INT, 100);

	pPlayer->SpawnCloneInContainer(weenieTemplate, 1);
	delete weenieTemplate;

	return false;

}

CLIENT_COMMAND(mutatewcidinv, "<wcid> <tier> [amount]", "Spawn item in inventory and run mutation filter", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (argc < 2)
		return true;

	using weenie_ptr = std::unique_ptr<CWeenieObject>;
	weenie_ptr weenieTemplate;

	if (IsNumeric(argv[0]))
	{
		weenieTemplate = weenie_ptr(g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), NULL, false));
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieTemplate = weenie_ptr(g_pWeenieFactory->CreateWeenieByClassID(strtoul(argv[0] + 2, NULL, 16), NULL, false));
	}
	else
	{
		weenieTemplate = weenie_ptr(g_pWeenieFactory->CreateWeenieByName(argv[0], NULL, false));
	}

	if (!weenieTemplate)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (!weenieTemplate->CanPickup())
	{
		pPlayer->SendText("That can't be spawned in a container.", LTT_DEFAULT);
		return false;
	}

	int tier = atoi(argv[1]);
	int amount = 1;
	if (argc >= 3)
		amount = atoi(argv[2]);
	//	weenieTemplate->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, atoi(argv[1]));
	//if (argc >= 4)
	//	weenieTemplate->m_Qualities.SetFloat(SHADE_FLOAT, atof(argv[2]));

	// TODO: Check free pack space?

	// get the mutation filter
	uint32_t filterId = weenieTemplate->InqDIDQuality(TSYS_MUTATION_FILTER_DID, 0);
	CMutationFilter *filter = g_pPortalDataEx->GetMutationFilter(filterId);

	if (!filter)
	{
		pPlayer->SendText("That can't be spawned in a container.", LTT_DEFAULT);
		return false;
	}

	tier--;
	for (int i = 0; i < amount; i++)
	{
		CWeenieObject *clone = g_pWeenieFactory->CloneWeenie(weenieTemplate.get());
		filter->TryMutate(clone->m_Qualities, tier);
		pPlayer->SpawnInContainer(clone);
	}
	return false;
}

CLIENT_COMMAND(spawnwcidinv, "<name> [amount] [ptid] [shade]", "Spawn by wcid into inventory.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	CWeenieObject *weenieTemplate;

	if (IsNumeric(argv[0]))
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), NULL, false);
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByClassID(strtoul(argv[0] + 2, NULL, 16), NULL, false);
	}
	else
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByName(argv[0], NULL, false);
	}

	if (!weenieTemplate)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (!pPlayer->IsAdmin() && atoi(g_pConfig->GetValue("weapons_testing", "0")) != 0)
	{
		if (weenieTemplate->m_Qualities.m_WeenieType != MeleeWeapon_WeenieType)
		{
			pPlayer->SendText("Only weapon spawning is enabled.", LTT_DEFAULT);
			delete weenieTemplate;

			return false;
		}
	}

	int amount = 1;
	if (argc >= 2)
		amount = atoi(argv[1]);
	if (argc >= 3)
		weenieTemplate->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, atoi(argv[1]));
	if (argc >= 4)
		weenieTemplate->m_Qualities.SetFloat(SHADE_FLOAT, atof(argv[2]));

	if (!weenieTemplate->CanPickup())
	{
		pPlayer->SendText("That can't be spawned in a container.", LTT_DEFAULT);
		delete weenieTemplate;

		return false;
	}

	int maxStackSize = weenieTemplate->InqIntQuality(MAX_STACK_SIZE_INT, 1);
	if (amount > maxStackSize * 100)
	{
		pPlayer->SendText("The amount requested is too large.", LTT_DEFAULT);
		delete weenieTemplate;

		return false;
	}

	pPlayer->SpawnCloneInContainer(weenieTemplate, amount);
	delete weenieTemplate;

	return false;
}

CLIENT_COMMAND(loadjsonwcid, "<wcid>", "Load json by wcid into server.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	bool refreshed = false;
	if (IsNumeric(argv[0]))
	{
		refreshed = g_pWeenieFactory->LoadLocalWeenieByWcid(atoi(argv[0]));
		if(refreshed)
			pPlayer->SendText("Weenie loaded.", LTT_DEFAULT);
		else
			pPlayer->SendText("Unable to parse Weenie", LTT_DEFAULT);
	}

	return false;
}


CLIENT_COMMAND(spawnwcidinvfresh, "<name> [amount] [ptid] [shade]", "Reload weenie and spawn by wcid into inventory.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	bool refreshed = false;
	if (IsNumeric(argv[0]))
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(atoi(argv[0]));
	}
	else if (IsHexNumeric(argv[0]))
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(strtoul(argv[0] + 2, NULL, 16));
	}
	else
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(g_pWeenieFactory->GetWCIDByName(argv[0]));
	}

	if (!refreshed)
	{
		pPlayer->SendText("Couldn't reload that weenie!", LTT_DEFAULT);
		return false;
	}

	CWeenieObject *weenieTemplate;

	if (IsNumeric(argv[0]))
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), NULL, false);
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByClassID(strtoul(argv[0] + 2, NULL, 16), NULL, false);
	}
	else
	{
		weenieTemplate = g_pWeenieFactory->CreateWeenieByName(argv[0], NULL, false);
	}

	if (!weenieTemplate)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (!pPlayer->IsAdmin() && atoi(g_pConfig->GetValue("weapons_testing", "0")) != 0)
	{
		if (weenieTemplate->m_Qualities.m_WeenieType != MeleeWeapon_WeenieType)
		{
			pPlayer->SendText("Only weapon spawning is enabled.", LTT_DEFAULT);
			delete weenieTemplate;

			return false;
		}
	}

	int amount = 1;
	if (argc >= 2)
		amount = atoi(argv[1]);
	if (argc >= 3)
		weenieTemplate->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, atoi(argv[1]));
	if (argc >= 4)
		weenieTemplate->m_Qualities.SetFloat(SHADE_FLOAT, atof(argv[2]));

	if (!weenieTemplate->CanPickup())
	{
		pPlayer->SendText("That can't be spawned in a container.", LTT_DEFAULT);
		delete weenieTemplate;

		return false;
	}

	int maxStackSize = weenieTemplate->InqIntQuality(MAX_STACK_SIZE_INT, 1);
	if (amount > maxStackSize * 100)
	{
		pPlayer->SendText("The amount requested is too large.", LTT_DEFAULT);
		delete weenieTemplate;

		return false;
	}

	pPlayer->SpawnCloneInContainer(weenieTemplate, amount);
	delete weenieTemplate;

	return false;
}

CLIENT_COMMAND(spawnwcid, "<name> [num] [ptid] [shade]", "Spawn by wcid.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	int num = 1;

	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	if (argc >= 2)
		num = atoi(argv[1]);

	CWeenieObject *weenie;

	for (int i = 0; i < num; i++)
	{
		if (IsNumeric(argv[0]))
		{
			weenie = g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), &pPlayer->m_Position, false);
		}
		else if (IsHexNumeric(argv[0]))
		{
			weenie = g_pWeenieFactory->CreateWeenieByClassID(strtoul(argv[0] + 2, NULL, 16), &pPlayer->m_Position, false);
		}
		else
		{
			weenie = g_pWeenieFactory->CreateWeenieByName(argv[0], &pPlayer->m_Position, false);
		}

		if (i > 1)
		{
			weenie->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-2.0, 2.0), Random::GenFloat(-2.0, 2.0), 1.0)));
		}

		if (!weenie)
		{
			pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
			return false;
		}

		if (!pPlayer->IsAdmin() && atoi(g_pConfig->GetValue("weapons_testing", "0")) != 0)
		{
			if (weenie->m_Qualities.m_WeenieType != MeleeWeapon_WeenieType)
			{
				pPlayer->SendText("Only weapon spawning is enabled.", LTT_DEFAULT);
				delete weenie;

				return false;
			}
		}

		if (argc >= 3)
			weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, atoi(argv[2]));
		if (argc >= 4)
			weenie->m_Qualities.SetFloat(SHADE_FLOAT, atof(argv[3]));

		if (!g_pWorld->CreateEntity(weenie))
			return false;

		if (weenie->IsCreature())
		{
			pPlayer->m_dwLastSpawnedCreatureID = weenie->GetID();
		}

		pPlayer->m_dwLastSpawnedObjectID = weenie->GetID();

		if (weenie)
			weenie->m_bDontClear = false;
	}

	return false;
}

CLIENT_COMMAND(spawnwcidfresh, "<name> [ptid] [shade]", "Spawn by wcid.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	bool refreshed = false;
	if (IsNumeric(argv[0]))
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(atoi(argv[0]));
	}
	else if (IsHexNumeric(argv[0]))
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(strtoul(argv[0] + 2, NULL, 16));
	}
	else
	{
		refreshed = g_pWeenieFactory->RefreshLocalWeenie(g_pWeenieFactory->GetWCIDByName(argv[0]));
	}

	if (!refreshed)
	{
		pPlayer->SendText("Couldn't reload that weenie!", LTT_DEFAULT);
		return false;
	}

	CWeenieObject *weenie;

	if (IsNumeric(argv[0]))
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), &pPlayer->m_Position, false);
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(strtoul(argv[0] + 2, NULL, 16), &pPlayer->m_Position, false);
	}
	else
	{
		weenie = g_pWeenieFactory->CreateWeenieByName(argv[0], &pPlayer->m_Position, false);
	}

	if (!weenie)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (!pPlayer->IsAdmin() && atoi(g_pConfig->GetValue("weapons_testing", "0")) != 0)
	{
		if (weenie->m_Qualities.m_WeenieType != MeleeWeapon_WeenieType)
		{
			pPlayer->SendText("Only weapon spawning is enabled.", LTT_DEFAULT);
			delete weenie;

			return false;
		}
	}

	if (argc >= 2)
		weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, atoi(argv[1]));
	if (argc >= 3)
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, atof(argv[2]));

	if (!g_pWorld->CreateEntity(weenie))
		return false;

	if (weenie->IsCreature())
	{
		pPlayer->m_dwLastSpawnedCreatureID = weenie->GetID();
	}

	weenie->m_bDontClear = false;

	return false;
}

CLIENT_COMMAND(spawnfresh, "<wcid>", "Refresh local json and spawn by wcid only.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (!IsNumeric(argv[0]))
	{
		pPlayer->SendText("Only wcids are supported.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	g_pWeenieFactory->RefreshLocalWeenie(atoi(argv[0]));

	CWeenieObject *weenie;

	weenie = g_pWeenieFactory->CreateWeenieByClassID(atoi(argv[0]), &pPlayer->m_Position, false);

	if (!weenie)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	if (!g_pWorld->CreateEntity(weenie))
		return false;

	weenie->m_bDontClear = false;

	if (weenie->IsCreature())
	{
		pPlayer->m_dwLastSpawnedCreatureID = weenie->GetID();
	}
	return false;
}

void SpawnAllAppearancesForWeenie(CPlayerWeenie *pPlayer, uint32_t wcid, bool bSpawnWithoutVariances = false, uint32_t setup_override = 0)
{
	CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (!weenieDefs)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return;
	}

	uint32_t clothing_did = 0;

	if (!weenieDefs->m_Qualities.InqDataID(CLOTHINGBASE_DID, clothing_did))
	{
		if (bSpawnWithoutVariances)
		{
			CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, &pPlayer->m_Position, false);
			weenie->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-10.0, 10.0), Random::GenFloat(-10.0, 10.0), 1.0)));

			if (setup_override)
			{
				weenie->m_Qualities.SetDataID(SETUP_DID, setup_override);
			}

			g_pWorld->CreateEntity(weenie);
		}

		pPlayer->SendText("Does not have appearance variances.", LTT_DEFAULT);
		return;
	}

	ClothingTable *ct = ClothingTable::Get(clothing_did);

	if (!ct)
	{
		pPlayer->SendText("Could not get clothing table.", LTT_DEFAULT);
		return;
	}

	for (auto ptit = ct->_paletteTemplatesHash.begin(); ptit != ct->_paletteTemplatesHash.end(); ptit++)
	{
		CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, &pPlayer->m_Position, false);

		weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptit->first);
		weenie->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-30.0, 30.0), Random::GenFloat(-30.0, 30.0), 1.0)));

		if (setup_override)
		{
			weenie->m_Qualities.SetDataID(SETUP_DID, setup_override);
			weenie->SetName(csprintf("Model 0x%08X PTID: %u (%s)", setup_override, ptit->first, CClothingCache::PaletteTemplateIDToString(ptit->first)));
		}

		g_pWorld->CreateEntity(weenie);
	}

	ClothingTable::Release(ct);
}

CLIENT_COMMAND(spawnwcidpts, "<name>", "Spawn all base appearances of a wcid.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	CWeenieDefaults *weenieDefs;
	if (IsNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(atoi(argv[0]));
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(strtoul(argv[0] + 2, NULL, 16));
	}
	else
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(argv[0]);
	}
	
	if (!weenieDefs)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	SpawnAllAppearancesForWeenie(pPlayer, weenieDefs->m_WCID);
	return false;
}

#if 0
CLIENT_COMMAND(spawnweenieswithsamemotiontable, "<name>", "Spawn all setups with motion table.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
		return true;

	CWeenieDefaults *weenieDefs;
	if (IsNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(atoi(argv[0]));
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(strtoul(argv[0] + 2, NULL, 16));
	}
	else
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(argv[0]);
	}

	if (!weenieDefs)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	uint32_t motion_table_did = 0;




	if (!weenieDefs->m_Qualities.InqDataID(MOTION_TABLE_DID, motion_table_did))
	{
		pPlayer->SendText("Does not have a motion table.", LTT_DEFAULT);
		return false;
	}

	std::list<uint32_t> sameMotionTable = g_pWeenieFactory->GetWCIDsWithMotionTable(motion_table_did);

	for (auto entry : sameMotionTable)
	{
		SpawnAllAppearancesForWeenie(pPlayer, entry);
	}

	return false;
}
#endif

CSetup *original_setup_to_find = NULL;

void FindSetupWithMotionID(void *argument, uint32_t id, BTEntry *entry)
{
	CSetup *mySetup = CSetup::Get(id);

	if (mySetup)
	{
		if ((!!original_setup_to_find->sphere == !!mySetup->sphere) && (!mySetup->sphere || (mySetup->sphere->center.is_equal(original_setup_to_find->sphere->center) && fabs(mySetup->sphere->radius - original_setup_to_find->sphere->radius) < F_EPSILON)))
		{
			if (mySetup->num_parts == original_setup_to_find->num_parts &&
				fabs(mySetup->step_down_height - original_setup_to_find->step_down_height) < F_EPSILON &&
				fabs(mySetup->step_up_height - original_setup_to_find->step_up_height) < F_EPSILON)
			{
				((std::list<uint32_t> *)argument)->push_back(id);
			}
		}

		CSetup::Release(mySetup);
	}
}

#if 0
CLIENT_COMMAND(spawnsimilarsetups, "<name> <todonly>", "Spawn all setups with motion table.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 2)
		return true;

	int todOnly = atoi(argv[1]);

	CWeenieDefaults *weenieDefs;
	if (IsNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(atoi(argv[0]));
	}
	else if (IsHexNumeric(argv[0]))
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(strtoul(argv[0] + 2, NULL, 16));
	}
	else
	{
		weenieDefs = g_pWeenieFactory->GetWeenieDefaults(argv[0]);
	}

	if (!weenieDefs)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return false;
	}

	uint32_t setup_did = 0;


	if (!weenieDefs->m_Qualities.InqDataID(SETUP_DID, setup_did))
	{
		pPlayer->SendText("Does not have a setup.", LTT_DEFAULT);
		return false;
	}

	CSetup *setup = CSetup::Get(setup_did);
	if (!setup)
	{
		pPlayer->SendText("Could not get setup.", LTT_DEFAULT);
		return false;
	}

	original_setup_to_find = setup;

	std::list<uint32_t> setupList;
	DATDisk::pPortal->FindFileIDsWithinRange(0x02000000, 0x02FFFFFF, FindSetupWithMotionID, NULL, &setupList);

	for (auto entry : setupList)
	{
		if (todOnly && entry < 0x020012F0)
			continue;

		SpawnAllAppearancesForWeenie(pPlayer, weenieDefs->m_WCID, true, entry);
	}

	CSetup::Release(setup);
	return false;
}


CLIENT_COMMAND(spawnarchitect, "[name]", "Spawn architect.", ADMIN_ACCESS)
{
	pPlayer->SendText("Disabled.", 1);
	return false;

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CWeenieObject *weenie;
	weenie = g_pWeenieFactory->CreateWeenieByClassID(6854, &pPlayer->m_Position, false);

	if (!weenie)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	weenie->SetScale(2.0f);
	weenie->SetName("The Architect");
	((CVendor *)weenie)->GenerateAllItems();

	g_pWorld->CreateEntity(weenie);

	return false;
}

CLIENT_COMMAND(spawnavatarvendor, "[name]", "Spawn avatar vendor.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CAvatarVendor *pVendor = new CAvatarVendor();
	g_pWeenieFactory->ApplyWeenieDefaults(pVendor, 719);
	SafeDelete(pVendor->m_Qualities._emote_table);
	SafeDelete(pVendor->m_Qualities._create_list);

	pVendor->SetInitialPosition(pPlayer->m_Position);
	pVendor->SetScale(1.0f);
	// Jumpsuit Vendor
	pVendor->SetName("Jumpsuit Tailor");

	g_pWorld->CreateEntity(pVendor);

	return false;
}
#endif


#ifndef PUBLIC_BUILD
CLIENT_COMMAND(barber, "", "", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	BinaryWriter writer;
	writer.Write<uint32_t>(0x75);
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(PALETTE_BASE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(HEAD_OBJECT_DID, 0));
	writer.Write<uint32_t>(0); // Head Texture
	writer.Write<uint32_t>(0); // Default Head Texture
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(EYES_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(DEFAULT_EYES_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(NOSE_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(DEFAULT_NOSE_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(MOUTH_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(DEFAULT_MOUTH_TEXTURE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(SKIN_PALETTE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(HAIR_PALETTE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(EYES_PALETTE_DID, 0));
	writer.Write<uint32_t>(pPlayer->InqDIDQuality(SETUP_DID, 0));
	if (pPlayer->m_Qualities.GetInt(HERITAGE_GROUP_INT, 0) == Empyrean_HeritageGroup)
	{
		// Option1 for Undead - TODO check for hover enabled, if yes, send 1, else 0.
		writer.Write<int>(0);
	}
	else if (pPlayer->m_Qualities.GetInt(HERITAGE_GROUP_INT, 0) == Undead_HeritageGroup)
	{
		// Option1 for Undead - TODO check for head flame enabled, if yes, send 1, else 0.
		writer.Write<int>(0);
	}
	else
		writer.Write<int>(0);
	writer.Write<int>(0); // Option2 - Unused?
	pPlayer->SendNetMessage(&writer, PRIVATE_MSG, FALSE, FALSE);

	return false;
}

CLIENT_COMMAND(spawnfollow, "", "", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (!pPlayer->m_dwLastSpawnedCreatureID)
		return false;

	CWeenieObject *pObject = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedCreatureID);
	if (!pObject)
		return false;

	MovementParameters params;
	params.can_charge = 1;

	// params.desired_heading = fmod(pObject->m_Position.frame.get_heading() + 90.0, 360.0);
	pObject->MoveToObject(pPlayer->GetID(), &params);
	return false;
}

CLIENT_COMMAND(spawnfollow2, "", "", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	if (!pPlayer->m_dwLastSpawnedCreatureID)
		return false;

	CWeenieObject *pObject = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedCreatureID);
	if (!pObject)
		return false;

	MovementParameters params;
	params.can_charge = 1;

	// params.desired_heading = fmod(pObject->m_Position.frame.get_heading() + 90.0, 360.0);
	pPlayer->MoveToObject(pObject->GetID(), &params);
	return false;
}

#endif

#if 0
CLIENT_COMMAND(spawnamuli, "[#] [#]", "Spawns an amuli variation.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer, true))
	{
		return false;
	}

	if (argc < 2)
	{
		return true;
	}

	int variation = atoi(argv[0]);
	float shade = atof(argv[1]);

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetCapturedArmorData("Greater Amuli Shadow Coat");

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	CWeenieObject *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);
	if (!pItem)
	{
		return false;
	}

	pItem->SetInitialPosition(pPlayer->GetPosition());

	ClothingTable *pCT = ClothingTable::Get(0x100001A1);
	if (!pCT)
	{
		delete pItem;
		return false;
	}

	pItem->m_miWornModel.ClearInfo();
	pItem->m_miWornModel.dwBasePalette = 0x7E;

	uint32_t sid = pPlayer->GetSetupID();
	const ClothingBase *pCB = pCT->_cloBaseHash.lookup(sid);
	if (pCB)
	{
		for (uint32_t i = 0; i < pCB->numObjectEffects; i++)
		{
			pItem->m_miWornModel.lModels.push_back(
				ModelRpl(
				(BYTE)pCB->objectEffects[i].partNum,
				(WORD)pCB->objectEffects[i].objectID));

			for (uint32_t j = 0; j < pCB->objectEffects[i].numTextureEffects; j++)
			{
				pItem->m_miWornModel.lTextures.push_back(
					TextureRpl(
						(BYTE)pCB->objectEffects[i].partNum,
						(WORD)pCB->objectEffects[i].textureEffects[j].oldTexID,
						(WORD)pCB->objectEffects[i].textureEffects[j].newTexID
					));
			}
		}
	}
	if (variation < 0 || variation >= pCT->_paletteTemplatesHash.size())
		variation = 0;

	std::map<uint32_t, CloPaletteTemplate>::iterator entry = pCT->_paletteTemplatesHash.begin();
	while (variation != 0 && entry != pCT->_paletteTemplatesHash.end())
	{
		variation--;
		entry++;
	}

	if (entry != pCT->_paletteTemplatesHash.end())
	{
		pItem->SetIcon(entry->second.iconID);
		
		for (uint32_t i = 0; i < entry->second.numSubpalEffects; i++)
		{
			uint32_t set = entry->second.subpalEffects[i].palSet;

			PalSet *pPS = PalSet::Get(set);
			if (pPS)
			{
				uint32_t pal = pPS->GetPaletteID(shade);
				
				for (uint32_t j = 0; j < entry->second.subpalEffects[i].numRanges; j++)
				{
					pItem->m_miWornModel.lPalettes.push_back(
						PaletteRpl(
							(WORD)pal,
							entry->second.subpalEffects[i].rangeStart[j] >> 3,
							entry->second.subpalEffects[i].rangeLength[j] >> 3
						));
				}

				PalSet::Release(pPS);
			}
		}
	}

	ClothingTable::Release(pCT);

	g_pWorld->CreateEntity(pItem);
	return false;
}
#endif

#if 0
CLIENT_COMMAND(spawnarmor, "[name]", "Spawns armor by name.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetCapturedArmorData(argv[0]);

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	float fScale = 1.0f;
	bool bAnimate = true;

	if (argc >= 2)
	{
		fScale = (float)atof(argv[1]);
		if (fScale < 0.01)
			fScale = 0.01f;
		if (fScale > 1000.0)
			fScale = 1000;
	}

	if (argc >= 3)
	{
		bAnimate = atoi(argv[2]) ? true : false;
	}

	CWeenieObject *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->m_scale = pItemInfo->physics.object_scale * fScale;
	pItem->m_dwCoverage2 = 0;
	pItem->SetInitialPosition(pPlayer->GetPosition());

	if (!bAnimate)
	{
		pItem->m_AnimOverrideData = NULL;
		pItem->m_AnimOverrideDataLen = 0;
		pItem->m_AutonomousMovement = 0;
	}

	g_pWorld->CreateEntity(pItem);
	return false;
}
#endif

#if 0
CLIENT_COMMAND(spawnarmor, "[name] [palette] [shade]", "Spawns armor by name.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer, true))
	{
		return false;
	}

	if (argc < 1)
	{
		return true;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetCapturedItemData(argv[0]);
	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}
	if (!(pItemInfo->weenie._type & (ITEM_TYPE::TYPE_ARMOR | ITEM_TYPE::TYPE_CLOTHING)))
	{
		pPlayer->SendText("That's not armor?", 1);
		return false;
	}

	CWeenieObject *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->SetInitialPosition(pPlayer->GetPosition());

	g_pWorld->CreateEntity(pItem);
	return false;
}

CLIENT_COMMAND(spawnlist, "[num to list]", "Lists spawnable objects.", ADMIN_ACCESS)
{
	int num = 20;
	if (argc >= 1)
	{
		num = atoi(argv[0]);
		if (num < 0)
			num = 0;
		if (num >= 100)
			num = 100;
	}

	while (num > 0)
	{
		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetRandomCapturedMonsterData();

		if (!pMonsterInfo)
		{
			pPlayer->SendText("Couldn't find anything to spawn!", 1);
			return false;
		}

		pPlayer->SendText(pMonsterInfo->weenie._name.c_str(), 1);
		num--;
	}
	return false;
}

CLIENT_COMMAND(spawnrandomarmor, "", "Spawns random armor.", SENTINEL_ACCESS)
{
	if (!SpawningEnabled(pPlayer, true))
	{
		return false;
	}

	CCapturedWorldObjectInfo *pItemInfo = g_pGameDatabase->GetRandomCapturedArmorData();

	if (!pItemInfo)
	{
		pPlayer->SendText("Couldn't find anything to spawn!", 1);
		return false;
	}

	CWeenieObject *pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

	pItem->SetInitialPosition(pPlayer->GetPosition());

	g_pWorld->CreateEntity(pItem);
	return false;
}

CLIENT_COMMAND(spawnrandom, "[num to spawn] [scale]", "Spawns random objects.", SENTINEL_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	int num = atoi(argv[0]);
	if (num < 0)
		num = 0;
	if (num >= 20)
		num = 20;
	
	float fScale = 1.0f;
	if (argc >= 2)
		fScale = max(0.1f, min(10.0f, (float) atof(argv[1])));

	int total = num;

	while (num > 0)
	{
		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetRandomCapturedMonsterData();

		if (!pMonsterInfo)
		{
			pPlayer->SendText("Couldn't find anything to spawn!", 1);
			return false;
		}

		CWeenieObject *pMonster = g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

		pMonster->SetScale(pMonsterInfo->physics.object_scale * fScale);
		pMonster->SetInitialPosition(pPlayer->GetPosition().add_offset(Vector(Random::GenFloat(-2.0f * total, 2.0f * total), Random::GenFloat(-2.0f * total, 2.0f * total), 0)));

		g_pWorld->CreateEntity(pMonster);

		num--;
	}
	return false;
}

CLIENT_COMMAND(spawnrandomshadows, "[phase] [num to spawn]", "Spawns random shadows.", ADMIN_ACCESS)
{
	if (argc < 2)
		return true;

	int phase = atoi(argv[0]);
	if (phase < 1)
		return true;
	if (phase > 4)
		return true;

	int num = atoi(argv[1]);
	if (num < 0)
		num = 0;
	if (num >= 20)
		num = 20;

	std::vector<std::string> phase_monsters[4];
	phase_monsters[0].push_back("Shadow Child");
	phase_monsters[0].push_back("Shadow Child");
	phase_monsters[0].push_back("Devious Shadow");
	phase_monsters[0].push_back("Devious Shadow");
	phase_monsters[0].push_back("Shadow Sprite");

	phase_monsters[1].push_back("Shadow Flyer");
	phase_monsters[1].push_back("Shadow Phantom");
	phase_monsters[1].push_back("Abyssal Shadow");
	phase_monsters[1].push_back("Tenebrous Shadow");
	phase_monsters[1].push_back("Panumbris Shadow");
	phase_monsters[1].push_back("Sinister Shadow");
	phase_monsters[1].push_back("Shadow");
	phase_monsters[1].push_back("Shadow Spectre");
	phase_monsters[1].push_back("Umbris Shadow");
	phase_monsters[1].push_back("Shadow Wraith");

	phase_monsters[2].push_back("Paroxysm Shadow");
	phase_monsters[2].push_back("Pandemonium Shadow");

	phase_monsters[3].push_back("Shadow");

	int total = num;
	while (num > 0)
	{
		std::string spawnName = phase_monsters[phase-1][Random::GenInt(0, (unsigned int)(phase_monsters[phase-1].size() - 1))];

		CCapturedWorldObjectInfo *pMonsterInfo = g_pGameDatabase->GetCapturedMonsterData(spawnName.c_str());
		if (pMonsterInfo)
		{
			CMonsterWeenie *pMonster = (CMonsterWeenie *)g_pGameDatabase->CreateFromCapturedData(pMonsterInfo);

			Position spawnPos = pPlayer->GetPosition().add_offset(Vector(Random::GenFloat(-4.0f * total, 4.0f * total), Random::GenFloat(-4.0f * total, 4.0f * total), 0));
			pMonster->SetInitialPosition(spawnPos);

			switch (phase)
			{
			case 1:
				pMonster->m_Qualities.SetInt(LEVEL_INT, 50);
				pMonster->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 100);
				pMonster->m_Qualities.SetAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, 120);
				pMonster->SetMaxVitals();
				pMonster->m_MeleeDamageBonus = 3;
				break;
			case 2:
				pMonster->m_Qualities.SetInt(LEVEL_INT, 100);
				pMonster->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 200);
				pMonster->m_Qualities.SetAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, 300);
				pMonster->SetMaxVitals();
				pMonster->m_MeleeDamageBonus = 6;
				break;

			case 3:
				pMonster->m_Qualities.SetInt(LEVEL_INT, 200);
				pMonster->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 300);
				pMonster->m_Qualities.SetAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, 600);
				pMonster->SetMaxVitals();
				pMonster->m_MeleeDamageBonus = 13;
				break;

			case 4:
				pMonster->m_Qualities.SetInt(LEVEL_INT, 500);
				pMonster->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 400);
				pMonster->m_Qualities.SetAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, 12000);
				pMonster->SetMaxVitals();
				pMonster->m_MeleeDamageBonus = 50;

				pMonster->SetName("Shadow of Turbine");
				pMonster->SetScale(pMonsterInfo->physics.object_scale * 2.5f);
				break;
			}

			g_pWorld->CreateEntity(pMonster);
		}

		num--;
	}
	return false;
}
#endif

CLIENT_COMMAND(pk, "", "Makes you a player killer.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (!pPlayer->IsAdmin() && !g_pConfig->AllowPKCommands())
	{
		pPlayer->SendText("This command is no longer enabled. You should use the PK/NPK altars instead.", LTT_DEFAULT);
		return false;
	}

	pPlayer->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, PK_PKStatus);
	pPlayer->NotifyIntStatUpdated(PLAYER_KILLER_STATUS_INT, false);

	pPlayer->m_Qualities.RemoveFloat(PK_TIMESTAMP_FLOAT);

	//pPlayer->m_Qualities.SetInt(RADARBLIP_COLOR_INT, 5);
	//pPlayer->NotifyIntStatUpdated(RADARBLIP_COLOR_INT, false);

	pPlayer->SendText("You are now a player killer.", 1);
	return false;
}

CLIENT_COMMAND(npk, "", "Makes you a non-player killer.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!pPlayer->IsAdmin() && !g_pConfig->AllowPKCommands())
	{
		pPlayer->SendText("This command is no longer enabled. You should use the PK/NPK altars instead.", LTT_DEFAULT);
		return false;
	}

	pPlayer->m_Qualities.SetInt(PLAYER_KILLER_STATUS_INT, NPK_PKStatus);
	pPlayer->NotifyIntStatUpdated(PLAYER_KILLER_STATUS_INT, false);

	pPlayer->m_Qualities.RemoveFloat(PK_TIMESTAMP_FLOAT);

	//if (player_client && player_client->GetAccessLevel() >= ADVOCATE_ACCESS)
	//	pPlayer->m_Qualities.SetInt(RADARBLIP_COLOR_INT, 9);
	//else
	//	pPlayer->m_Qualities.SetInt(RADARBLIP_COLOR_INT, 3);
	// pPlayer->NotifyIntStatUpdated(RADARBLIP_COLOR_INT, false);

	pPlayer->SendText("You are now a non-player killer.", 1);
	return false;
}

CLIENT_COMMAND(decent, "[name]", "Gives you great attributes.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableGodlyCommand() && pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
	{
		pPlayer->SendText("Command disabled.", 1);
		return false;
	}

	pPlayer->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 10);
	pPlayer->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 100);
	pPlayer->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 10);
	pPlayer->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 10);
	pPlayer->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 100);
	pPlayer->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 100);
	pPlayer->m_Qualities.SetInt(LEVEL_INT, 100);
	pPlayer->NotifyAttributeStatUpdated(STRENGTH_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(ENDURANCE_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(QUICKNESS_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(COORDINATION_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(FOCUS_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(SELF_ATTRIBUTE);
	pPlayer->NotifyIntStatUpdated(LEVEL_INT);

	pPlayer->EmitEffect(138, 1.0f);

	Skill runSkill;
	pPlayer->m_Qualities.InqSkill(RUN_SKILL, runSkill);
	runSkill._init_level = 0;
	pPlayer->m_Qualities.SetSkill(RUN_SKILL, runSkill);
	pPlayer->NotifySkillStatUpdated(RUN_SKILL);

	Skill jumpSkill;
	pPlayer->m_Qualities.InqSkill(JUMP_SKILL, jumpSkill);
	jumpSkill._init_level = 0;
	pPlayer->m_Qualities.SetSkill(JUMP_SKILL, jumpSkill);
	pPlayer->NotifySkillStatUpdated(JUMP_SKILL);

	// pPlayer->SetHealth(pPlayer->GetMaxHealth());
	// pPlayer->SetStamina(pPlayer->GetMaxStamina());
	// pPlayer->SetMana(pPlayer->GetMaxMana());

	pPlayer->SendText("Your attributes have been made decent.", 1);

	pPlayer->CheckVitalRanges();

	return false;
}

CLIENT_COMMAND(godly, "", "Gives you great attributes.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableGodlyCommand() && pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
	{
		pPlayer->SendText("Command disabled.", 1);
		return false;
	}

	pPlayer->SendText("Use /decent instead if you want to play fair.", 1);

	pPlayer->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 790);
	pPlayer->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetInt(LEVEL_INT, 999);
	pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, 100000000000);
	pPlayer->NotifyAttributeStatUpdated(STRENGTH_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(ENDURANCE_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(QUICKNESS_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(COORDINATION_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(FOCUS_ATTRIBUTE);
	pPlayer->NotifyAttributeStatUpdated(SELF_ATTRIBUTE);

	pPlayer->NotifyIntStatUpdated(LEVEL_INT);
	pPlayer->NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);
	pPlayer->EmitEffect(138, 1.0f);

	Skill runSkill;
	pPlayer->m_Qualities.InqSkill(RUN_SKILL, runSkill);
	runSkill._init_level = 0;
	pPlayer->m_Qualities.SetSkill(RUN_SKILL, runSkill);
	pPlayer->NotifySkillStatUpdated(RUN_SKILL);

	Skill jumpSkill;
	pPlayer->m_Qualities.InqSkill(JUMP_SKILL, jumpSkill);
	jumpSkill._init_level = 1000000000;
	pPlayer->m_Qualities.SetSkill(JUMP_SKILL, jumpSkill);
	pPlayer->NotifySkillStatUpdated(JUMP_SKILL);

	Skill axeSkill;
	pPlayer->m_Qualities.InqSkill(AXE_SKILL, axeSkill);
	axeSkill._init_level = 1000000000;
	pPlayer->m_Qualities.SetSkill(AXE_SKILL, axeSkill);
	pPlayer->NotifySkillStatUpdated(AXE_SKILL);

	if (!pPlayer->IsDead())
	{
		pPlayer->SetHealth(pPlayer->GetMaxHealth());
		pPlayer->SetStamina(pPlayer->GetMaxStamina());
		pPlayer->SetMana(pPlayer->GetMaxMana());
	}

	pPlayer->SendText("Your attributes have been greatly increased.", 1);

	return false;
}

CLIENT_COMMAND(givexp, "[value]", "Gives you some XP for testing.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	int64_t amount = 0;
	if (argc >= 1)
	{
		amount = strtoll(argv[0], NULL, 10);
	}

	//amount = min(amount, 4000000000u);
	// pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, amount);

	// pPlayer->SendText("Your attributes have been greatly increased.", 1);
	pPlayer->GiveXP(amount, ExperienceHandlingType::QuestXpNoShare, true);

	return false;
}

CLIENT_COMMAND(getcreditother, "", "Gets the current unassigned skill credits of the last identified target.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	CWeenieObject *targetID = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!targetID)
	{
		pPlayer->SendText("You must assess a valid target.", LTT_DEFAULT);
		return true;
	}
	if (!targetID->AsPlayer())
	{
		pPlayer->SendText("You must assess a player target.", LTT_DEFAULT);
		return true;
	}

	auto targetName = targetID->GetName();
	uint32_t currentcredits = targetID->AsPlayer()->GetTotalSkillCredits();
	uint32_t expectedCredits = targetID->AsPlayer()->GetExpectedSkillCredits();
	const char *AunR = "";
	const char *Oswald = "";
	if (targetID->InqQuest("arantahkill1")) //Aun Ralirea
		AunR = "Aun Ralirea";

	if (targetID->InqQuest("ChasingOswaldDone")) //Finding Oswald
		Oswald = "Finding Oswald";

	//todo add Luminance Skill Credit Checks (2 credits).

	BinaryWriter popupMessage;
	popupMessage.Write<uint32_t>(0x4);
	popupMessage.WriteString(csprintf("%s has %d skill credits total. \n\n A character of level %d should have %d skill credits. \n\n Skill Credit Quests Complete: \n %s \n %s",
		targetName.c_str(),
		currentcredits,
		targetID->InqIntQuality(LEVEL_INT, 0, TRUE),
		expectedCredits,
		AunR,
		Oswald));

	pPlayer->SendNetMessage(&popupMessage, PRIVATE_MSG, TRUE, FALSE);


	return false;
}

CLIENT_COMMAND(givecreditother, "<player name> [value]", "Gives your last assessed AND named target some skill credits.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	CPlayerWeenie *targetID = g_pWorld->FindPlayer(pPlayer->m_LastAssessed);
	if (!targetID)
	{
		pPlayer->SendText("You must assess a player target", LTT_DEFAULT);
		return true;
	}
	auto targetName = targetID->GetName();
	if (targetID->GetName() != string(argv[0]))
	{
		pPlayer->SendText("Last Assessed player target does not match named target!", LTT_DEFAULT);
		return true;
	}
	int initialcredits = targetID->GetSkillCredits();
	int amount = 0;
	if (argc >= 1)
	{
		amount = stoi(argv[1], NULL, 10);
	}
	else
		return true;

	// Cant go below Initialcredits if initialcredits at 0 or if amount would take available below 0
	// Can give neg credits
	if ((amount + initialcredits) < 0)
		amount = (-initialcredits);
	
	targetID->GiveSkillCredit(amount);
	uint32_t postcredits = targetID->GetSkillCredits();
	if (postcredits - amount == initialcredits)
		pPlayer->SendText(csprintf("%s has been awarded %d skill credits! Initial skill credits were %d, post command credits are %d.", targetName.c_str(), amount, initialcredits, postcredits), LTT_DEFAULT);
	else
		pPlayer->SendText(csprintf("Failed to award %s %d skill credits. Initial skill credits were %d, post command credits are %d.", targetName.c_str(), amount, initialcredits, postcredits), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(givecredit, "[value]", "Gives you some skill credits for testing.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	uint32_t amount = 0;
	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = max(amount, (uint32_t)1);
	amount = min(amount, (uint32_t)100);

	pPlayer->GiveSkillCredit(amount);

	return false;
}


CLIENT_COMMAND(givexpother, "[value]", "Gives you some XP for testing.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	uint32_t amount = 0;

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = min(amount, 4000000000u);
	// pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, amount);

	// pPlayer->SendText("Your attributes have been greatly increased.", 1);

	CWeenieObject *assessed = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (assessed && assessed->AsPlayer())
	{
		assessed->GiveXP(amount, ExperienceHandlingType::QuestXpNoShare, true);
	}

	return false;
}

//CLIENT_COMMAND(hover, "<on / off>", "Turns hovering on or off.", BASIC_ACCESS, CHARACTER_CATEGORY)
//{
//	if (pPlayer->InqIntQuality(HERITAGE_GROUP_INT, 0, true) != 9)
//	{
//		pPlayer->SendText("Command only available for Empyrean characters.", LTT_DEFAULT);
//		return false;
//	}

//	if (argc < 1)
//	{
//		pPlayer->SendText("Usage: /hover on | off", LTT_DEFAULT);
//		return false;
//	}

//	if (!stricmp(argv[0], "on") || !stricmp(argv[0], "1"))
//	{

//		pPlayer->SendText("Hovering on. You may need to relog for the change to take effect.", LTT_DEFAULT);
//		pPlayer->m_Qualities.SetDataID(MOTION_TABLE_DID, 0x9000207);
//		pPlayer->NotifyDIDStatUpdated(MOTION_TABLE_DID);
//	}
//	else if (!stricmp(argv[0], "off") || !stricmp(argv[0], "0"))
//	{
//		pPlayer->SendText("Hovering off. You may need to relog for the change to take effect.", LTT_DEFAULT);
//		pPlayer->m_Qualities.SetDataID(MOTION_TABLE_DID, 0x900020D);
//		pPlayer->NotifyDIDStatUpdated(MOTION_TABLE_DID);
//	}

//	return false;
//}

CLIENT_COMMAND(movetome, "", "Brings an object to you.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	CWeenieObject *pObject = g_pWorld->FindWithinPVS(pPlayer, pPlayer->m_LastAssessed);
	if (pObject)
	{
		pObject->Movement_Teleport(pPlayer->GetPosition());
	}
	else
	{
		pPlayer->SendText("Please assess a valid object you wish to move!", LTT_DEFAULT);
		return true;
	}

	return false;
}

CLIENT_COMMAND(challengeai, "", "Challenge an AI to a game of Chess.", BASIC_ACCESS, GENERAL_CATEGORY)
{
	sChessManager->ChallengeAi(pPlayer);
	return false;
}

CLIENT_COMMAND(sethealth, "[value]", "Set my health to value must be below max health", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	uint32_t amount = 0;

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}
	else
		return false;

	pPlayer->SetHealth(amount, true);
	return true;
}

CLIENT_COMMAND(setstamina, "[value]", "Set my staminahealth to value must be below max health", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	uint32_t amount = 0;

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}
	else
		return false;

	pPlayer->SetStamina(amount, true);
	return true;
}

CLIENT_COMMAND(givelum, "[value]", "Gives you some Luminance for testing.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	uint32_t amount = 0;
	uint32_t total = pPlayer->m_Qualities.GetInt64(AVAILABLE_LUMINANCE_INT64, 0);

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = min(amount, (uint32_t)1000000);
	pPlayer->m_Qualities.SetInt64(AVAILABLE_LUMINANCE_INT64, min(amount + total, (uint32_t)1000000));
	pPlayer->NotifyInt64StatUpdated(AVAILABLE_LUMINANCE_INT64, false);

	if (!pPlayer->m_Qualities.GetInt64(MAXIMUM_LUMINANCE_INT64, 0))
	{
		pPlayer->m_Qualities.SetInt64(MAXIMUM_LUMINANCE_INT64, 1000000);
		pPlayer->NotifyInt64StatUpdated(MAXIMUM_LUMINANCE_INT64, false);
	}

	return false;
}

CLIENT_COMMAND(setsolid, "", "Toggles ethereal for an object.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedObject = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedObject)
		editedObject = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string nameString;
	if (editedObject)
	{
		editedObject->set_state(editedObject->m_PhysicsState ^ (uint32_t)(ETHEREAL_PS), TRUE);
		nameString = editedObject->GetName();
	}
	else
	{
		pPlayer->SendText("Please assess a valid object you wish to alter ethereal state!", LTT_DEFAULT);
		return false;
	}
		
	std::string status;
	if (editedObject->m_PhysicsState & ETHEREAL_PS)
		status = "Ethereal";	
	else	
		status = "Solid";
	
	pPlayer->SendText(csprintf("%s set for: %s", status.c_str(), nameString.c_str()), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(nudge_z, "", "Nudges an item in the z axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_origin.z += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Nudged object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(purgespells, "", "Removes all spells from you", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (pPlayer->m_Qualities._enchantment_reg)
	{
		PackableList<uint32_t> removed;

		if (pPlayer->m_Qualities._enchantment_reg->_add_list)
		{
			for (auto it = pPlayer->m_Qualities._enchantment_reg->_add_list->begin(); it != pPlayer->m_Qualities._enchantment_reg->_add_list->end();)
			{
				if (it->_duration == -1.0)
				{
					removed.push_back(it->_id);
					it = pPlayer->m_Qualities._enchantment_reg->_add_list->erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		if (pPlayer->m_Qualities._enchantment_reg->_mult_list)
		{
			for (auto it = pPlayer->m_Qualities._enchantment_reg->_mult_list->begin(); it != pPlayer->m_Qualities._enchantment_reg->_mult_list->end();)
			{
				if (it->_duration == -1.0)
				{
					removed.push_back(it->_id);
					it = pPlayer->m_Qualities._enchantment_reg->_mult_list->erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		if (removed.size())
		{
			// m_Qualities._enchantment_reg->PurgeEnchantments();

			BinaryWriter expireMessage;
			expireMessage.Write<uint32_t>(0x2C8);
			removed.Pack(&expireMessage);

			pPlayer->SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
		}
		pPlayer->SendText("Spells purged", LTT_DEFAULT);

	}
	else
		pPlayer->SendText("What?!? You got nothing ta purge!", LTT_DEFAULT);


	return false;
}


CLIENT_COMMAND(nudge_x, "", "Nudges an item in the x axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_origin.x += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Nudged object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(nudge_y, "", "Nudges an item in the y axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_origin.y += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Nudged object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to nudge!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

#if 0
CLIENT_COMMAND(spin_w, "", "Spins an item in the w axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_angles.w += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Spun object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(spin_x, "", "Spins an item in the x axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_angles.x += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Spun object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(spin_y, "", "Spins an item in the y axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_angles.y += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Spun object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(spin_z, "", "Spins an item in the z axis.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();

	float value = 0.0f;

	if (editedItem && argc >= 1)
	{
		value = atof(argv[0]);

		if (value)
		{
			if (!pos.objcell_id)
				pos.objcell_id = pPlayer->m_Position.objcell_id;
			pos.frame.m_angles.z += value;
			editedItem->Movement_Teleport(pos, false);
		}
	}
	else
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Spun object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}
#endif

CLIENT_COMMAND(spin, "", "Sets the spin of an item to that of the player.", ADMIN_ACCESS, SPAWN_CATEGORY)
{
	CWeenieObject *editedItem = g_pWorld->FindObject(pPlayer->m_dwLastSpawnedObjectID);

	if (!editedItem)
		editedItem = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	std::string itemString = editedItem->GetName();
	Position pos = editedItem->GetPosition();
	Position playerpos = pPlayer->GetPosition();

	if (editedItem)
	{
		if (!pos.objcell_id)
			pos.objcell_id = pPlayer->m_Position.objcell_id;
		pos.frame.m_angles.w = playerpos.frame.m_angles.w;
		pos.frame.m_angles.x = playerpos.frame.m_angles.x;
		pos.frame.m_angles.y = playerpos.frame.m_angles.y;
		pos.frame.m_angles.z = playerpos.frame.m_angles.z;
		editedItem->Movement_Teleport(pos, false);
	}

	if (itemString != "")
	{
		pPlayer->SendText(std::string("Spun object: ").append(itemString).c_str(), LTT_DEFAULT);
	}
	else
	{
		pPlayer->SendText(std::string("Please assess a valid object you wish to spin!").append(itemString).c_str(), LTT_DEFAULT);
		return false;
	}

	return false;
}

CLIENT_COMMAND(testsummons, "<Summoning Mastery Value>", "Set user Summon mastery and provides full set of petdevices", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		return false;
	}

	uint32_t mastery = 0;

	if (argc >= 1)
	{
		mastery = strtoul(argv[0], NULL, 10);
	}
	else
		return false;

	if (g_pConfig->GetValue("weapons_testing", "0") == 0)
	{
		if (pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
		{
			pPlayer->SendText("You do not have access to this command.", LTT_DEFAULT);
			return false;
		}
	}

	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	switch (mastery)
	{
	case 1:
		pPlayer->m_Qualities.SetInt(SUMMONING_MASTERY_INT, mastery);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49261, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49262, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49263, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49264, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49265, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49266, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48959, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48961, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48963, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48965, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48967, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48969, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49275, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49276, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49277, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49278, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49279, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49280, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49268, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49269, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49270, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49271, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49272, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49273, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48957, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49260, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49267, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49274, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49282, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49296, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49303, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49289, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49283, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49297, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49304, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49290, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49284, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49298, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49305, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49291, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49285, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49299, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49306, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49292, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49286, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49300, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49307, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49293, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49287, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49301, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49308, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49294, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49302, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49281, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49295, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49288, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49310, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49317, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49324, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49331, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49311, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49318, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49325, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49332, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49312, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49319, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49326, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49333, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49313, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49320, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49327, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49334, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49314, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49321, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49328, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49335, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49315, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49322, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49329, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49336, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49309, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49316, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49323, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49330, NULL, false), 1);
		break;
	case 2:
		pPlayer->m_Qualities.SetInt(SUMMONING_MASTERY_INT, mastery);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49427, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49434, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49441, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49448, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48946, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49216, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49223, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49230, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48947, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49217, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49224, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49231, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48948, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49218, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49225, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49232, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48942, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49213, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49220, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49227, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48944, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49214, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49221, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49228, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48945, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49215, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49222, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49229, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48956, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49212, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49219, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49226, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49421, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49428, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49435, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49442, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49422, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49429, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49436, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49443, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49423, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49430, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49437, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49444, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49424, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49431, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49438, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49445, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49425, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49432, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49439, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49446, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49426, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49433, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49440, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49447, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(48972, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49240, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49247, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49254, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49234, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49241, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49248, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49255, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49235, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49242, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49249, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49256, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49236, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49243, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49250, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49257, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49237, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49244, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49251, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49258, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49238, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49245, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49252, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49259, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49233, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49239, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49246, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49253, NULL, false), 1);
		break;
	case 3:
		pPlayer->m_Qualities.SetInt(SUMMONING_MASTERY_INT, mastery);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49366, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49380, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49387, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49373, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49367, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49381, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49388, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49374, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49368, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49382, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49389, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49375, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49369, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49383, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49390, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49376, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49370, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49384, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49391, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49377, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49371, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49385, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49392, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49378, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49365, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49372, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49379, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49386, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49338, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49345, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49352, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49359, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49339, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49346, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49353, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49360, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49340, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49347, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49354, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49361, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49341, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49348, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49355, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49362, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49342, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49349, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49356, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49363, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49343, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49350, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49357, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49364, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49337, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49344, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49351, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49358, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49530, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49537, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49544, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49551, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49524, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49531, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49538, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49545, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49525, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49532, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49539, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49546, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49526, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49533, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49540, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49547, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49527, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49534, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49541, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49548, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49528, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49535, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49542, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49549, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49529, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49536, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49543, NULL, false), 1);
		pPlayer->SpawnCloneInContainer(g_pWeenieFactory->CreateWeenieByClassID(49550, NULL, false), 1);
		break;
	default:
		break;
	}

	return false;
}

CLIENT_COMMAND(spawnset, "setid", "spawns studded leather with set", SENTINEL_ACCESS, CHARACTER_CATEGORY)
{
	if (!g_pConfig->EnableTeleCommands() && pPlayer->GetAccessLevel() < ADVOCATE_ACCESS)
	{
		pPlayer->SendText("This command is now only available on servers running in Test mode.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	int set = atoi(argv[0]);
	

	auto helm = g_pWeenieFactory->CreateWeenieByClassID(554, NULL, false);
	helm->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(helm, 1);

	auto breastplate = g_pWeenieFactory->CreateWeenieByClassID(42, NULL, false);
	breastplate->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(breastplate, 1);

	auto pauldrons = g_pWeenieFactory->CreateWeenieByClassID(89, NULL, false);
	pauldrons->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(pauldrons, 1);

	auto bracers = g_pWeenieFactory->CreateWeenieByClassID(38, NULL, false);
	bracers->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(bracers, 1);

	auto gauntlets = g_pWeenieFactory->CreateWeenieByClassID(59, NULL, false);
	gauntlets->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(gauntlets, 1);

	auto girth = g_pWeenieFactory->CreateWeenieByClassID(63, NULL, false);
	girth->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(girth, 1);

	auto tassets = g_pWeenieFactory->CreateWeenieByClassID(112, NULL, false);
	tassets->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(tassets, 1);

	auto greaves = g_pWeenieFactory->CreateWeenieByClassID(68, NULL, false);
	greaves->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(greaves, 1);

	auto boots = g_pWeenieFactory->CreateWeenieByClassID(116, NULL, false);
	boots->m_Qualities.SetInt(EQUIPMENT_SET_ID_INT, set);
	pPlayer->SpawnCloneInContainer(boots, 1);

	
	return false;
}


const char* CommandBase::Info(CommandEntry* pCommand)
{
	const char* szName = pCommand->name;
	const char* szArgs = pCommand->args;
	const char* szHelp = pCommand->help;

	if (strlen(szArgs) <= 0)
		return csprintf("Syntax: @%s - %s", szName, szHelp);
	else
		return csprintf("Syntax: @%s %s - %s", szName, szArgs, szHelp);
}

CommandEntry* CommandBase::FindCommand(const char* szName, int iAccessLevel)
{
	CommandMap& commands = GetCommandMap();
	CommandMap::iterator i = commands.find(szName);

	if (i == commands.end())
		return NULL;

	if (iAccessLevel == -1 || iAccessLevel >= i->second.access)
		return &i->second;
	else
		return NULL;
}

void CommandBase::PrintCommandList(CWeenieObject* sendto, int category, int access_level, char* cmdName)
{
	if (cmdName)
	{
		CommandEntry* pCommand = FindCommand(cmdName, access_level);

		if (pCommand)
		{
			sendto->SendText(Info(pCommand), 1);
		}
		else
		{
			sendto->SendText("Unknown command. Use !help to receive a list of commands.", 1);
		}
	}
	else
	{
		// List all commands.
		std::string strCommandList = "Command List: \n";
		CommandMap& commands = GetCommandMap();

		for (CommandMap::iterator it = commands.begin(); it != commands.end(); it++)
		{
			if (it->second.access <= access_level && it->second.category == category) {
				strCommandList += Info(&it->second);
				strCommandList += "\n";
			}
		}
		strCommandList += "Use !help <command> to receive command syntax.\n";
		sendto->SendText(strCommandList.c_str(), 1);
	}
}

bool CommandBase::Execute(char *command, CClient *client)
{
	if (!client)
		return true;

	char* argv[MAX_ARGUMENTS];
	int	argc = 0;
	bool inquote = false;
	char *argstart = command;
	char *p = command;

	// Quote-delimited arguments.
	for (; p[0] != '\0' && argc < MAX_ARGUMENTS; p++)
	{
		char feed = p[0];

		if (inquote)
		{
			if (feed == '\"')
			{
				inquote = false;

				*p = 0;
				argv[argc++] = argstart;
				argstart = p + 1;
				continue;
			}
		}
		else
		{
			if (feed == ' ')
			{
				if (argstart == p)
					argstart++;
				else
				{
					*p = 0;
					argv[argc++] = argstart;
					argstart = p + 1;
				}
			}

			if (feed == '\"')
			{
				inquote = true;
				argstart = p + 1;
			}
		}
	}

	if (argstart[0] != 0 && argc < MAX_ARGUMENTS)
	{
		argv[argc++] = argstart;
	}

	if (argc > 0)
	{
		CPlayerWeenie *player_weenie = client ? client->GetEvents()->GetPlayer() : NULL;
		CPhysicsObj *player_physobj = player_weenie ? player_weenie->_phys_obj : NULL;
		int access_level = client ? client->GetAccessLevel() : BASIC_ACCESS;

		char *command_name = argv[0];
		if (!stricmp(command_name, "emuhelp"))
		{
			player_weenie->SendText("Please use one of the following to see a list of commands:\n/generalhelp - Lists general commands that don't fall into any other category.\n/explorehelp - Lists commands pertaining to exploring (dungeon information, teleport commands (if enabled), etc.. \n/spawnhelp - Lists commands pertaining to the spawning of items and mobs.\n/characterhelp - Lists commands pertaining to your character.\n/serverhelp - Lists commands that affect the entire server.", 1);
		}
		else if (!stricmp(command_name, "generalhelp") || !stricmp(command_name, "ghelp"))
		{
			PrintCommandList(player_weenie, GENERAL_CATEGORY, access_level, (argc > 1) ? argv[1] : NULL);
		}
		else if (!stricmp(command_name, "explorehelp") || !stricmp(command_name, "ehelp"))
		{
			PrintCommandList(player_weenie, EXPLORE_CATEGORY, access_level, (argc > 1) ? argv[1] : NULL);
		}
		else if (!stricmp(command_name, "spawnhelp") || !stricmp(command_name, "sphelp"))
		{
			PrintCommandList(player_weenie, SPAWN_CATEGORY, access_level, (argc > 1) ? argv[1] : NULL);
		}
		else if (!stricmp(command_name, "characterhelp") || !stricmp(command_name, "chelp"))
		{
			PrintCommandList(player_weenie, CHARACTER_CATEGORY, access_level, (argc > 1) ? argv[1] : NULL);
		}
		else if (!stricmp(command_name, "serverhelp") || !stricmp(command_name, "svhelp"))
		{
			PrintCommandList(player_weenie, SERVER_CATEGORY, access_level, (argc > 1) ? argv[1] : NULL);
		}
		else
		{
			CommandEntry* pCommand = FindCommand(command_name, access_level);

			if (!pCommand)
			{
				if (player_weenie)
					player_weenie->SendText("Unknown command!", 1);
			}
			else
			{
				if (!pCommand->source || player_weenie)
				{
					SERVER_INFO << "EXECUTING CLIENT COMMAND" << command << "FROM" << client->GetDescription();

					// run the command callback
					if ((*pCommand->func)(client, player_weenie, player_physobj, argv + 1, argc - 1))
					{
						// send error text
						if (player_weenie)
							player_weenie->SendText(Info(pCommand), 1);
					}
					else
						return true;
				}
			}
		}
	}

	return false;
}

ClientCommand::ClientCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory)
	: CommandBase()
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, iCategory, true);
}

ServerCommand::ServerCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, int iCategory)
	: CommandBase()
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, iCategory, false);
}
