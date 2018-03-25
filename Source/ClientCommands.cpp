
#include "StdAfx.h"
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

// Most of these commands are just for experimenting and never meant to be used in a real game
// TODO: Add flags to these commands so they are only accessible under certain modes such as a sandbox mode

CommandMap CommandBase::m_mCommands;

void CommandBase::Create(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel, bool bSource)
{
	CommandEntry i =
	{
		szName,
		szArguments,
		szHelp,
		pCallback,
		iAccessLevel,
		bSource
	};

	m_mCommands[std::string(szName)] = i;
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
CLIENT_COMMAND(simulateaccess, "<level>", "Simulate access level.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->GetClient()->SetAccessLevel((unsigned) atoi(argv[0]));
	return false;
}
#endif

CLIENT_COMMAND(myloc, "", "Info on your current location.", BASIC_ACCESS)
{
	pPlayer->SendText(csprintf("%08X %.2f %.2f %.2f %.2f %.2f %.2f %.2f",
		player_physobj->m_Position.objcell_id,
		player_physobj->m_Position.frame.m_origin.x, player_physobj->m_Position.frame.m_origin.y, player_physobj->m_Position.frame.m_origin.z,
		player_physobj->m_Position.frame.m_angles.w, player_physobj->m_Position.frame.m_angles.x, player_physobj->m_Position.frame.m_angles.y, player_physobj->m_Position.frame.m_angles.z), LTT_DEFAULT);

	return false;
}

#ifndef PUBLIC_BUILD
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

CLIENT_COMMAND(global, "<text> [color=1]", "Displays text globally.", ADMIN_ACCESS)
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
	long lColor = (argc >= 2) ? atoi(argv[1]) : 1;

	g_pWorld->BroadcastGlobal(ServerText(szText, lColor), PRIVATE_MSG);
	return false;
}

/*
CLIENT_COMMAND(animationall, "<num> [speed]", "Performs an animation for everyone.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	float fDelay = 0.5f;

	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->Animation_PlayPrimary(wIndex, fSpeed, fDelay);
	}

	return false;
}
*/

CLIENT_COMMAND(freezeall, "", "Freezes or unfreezes everyone.", ADMIN_ACCESS)
{
	if (argc < 1)
	{
		return true;
	}

	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		i->second->set_state(i->second->m_PhysicsState ^ (DWORD)(FROZEN_PS), TRUE);
	}

	return false;
}

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

CLIENT_COMMAND(effect, "<index> [scale=1]", "Emits a particle effect.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	DWORD dwIndex;
	float flScale;

	dwIndex = atol(argv[0]);
	if (argc >= 2)	flScale = (float)atof(argv[1]);
	else				flScale = (float)1.0f;

	pPlayer->EmitEffect(dwIndex, flScale);

	return false;
}

CLIENT_COMMAND(sound, "<index> [speed?=1]", "Emits a sound effect.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	DWORD dwIndex = atol(argv[0]);
	float flSpeed = (float)((argc >= 2) ? atof(argv[1]) : 1.0f);

	pPlayer->EmitSound(dwIndex, flSpeed);

	return false;
}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(arwic, "", "Teleports you to Arwic.", SENTINEL_ACCESS)
{
	pPlayer->SendText("Teleporting you to Arwic..", LTT_DEFAULT);
	pPlayer->Movement_Teleport(Position(0xC6A90023, Vector(102.4f, 70.1f, 44.0f), Quaternion(0.70710677f, 0, 0, 0.70710677f)));

	return false;
}

CLIENT_COMMAND(removethis, "", "Removes an object.", ADMIN_ACCESS)
{
	std::string itemRemoved = pPlayer->RemoveLastAssessed();
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

CLIENT_COMMAND(targeteffect, "[effect] [scale]", "Plays an effect on the last target you assessed.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (!pPlayer->m_LastAssessed)
		return true;

	if (argc < 1)
		return true;

	DWORD dwIndex = atol(argv[0]);

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

CLIENT_COMMAND(targetsound, "[effect] [speed]", "Plays an effect on the last target you assessed.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (!pPlayer->m_LastAssessed)
		return true;

	if (argc < 1)
		return true;

	DWORD dwIndex = atol(argv[0]);

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

CLIENT_COMMAND(tele, "<player name>", "Teleports you to a player.", BASIC_ACCESS)
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


CLIENT_COMMAND(teletome, "<player name>", "Teleports someone to you.", ADVOCATE_ACCESS)
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

CLIENT_COMMAND(teletown, "<town name>", "Teleports you to a town.", BASIC_ACCESS)
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

CLIENT_COMMAND(teleto, "<coords>", "Teleports you to coordinates.", BASIC_ACCESS)
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

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(reviveoverride, "<setting>", "", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_bReviveOverride = atoi(argv[0]) ? true : false;
	g_RevivePosition = pPlayer->m_Position;
	return false;
}
#endif

bool g_bStartOverride = false;
Position g_StartPosition;
#ifndef PUBLIC_BUILD
CLIENT_COMMAND(startoverride, "<setting>", "", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_bStartOverride = atoi(argv[0]) ? true : false;
	g_StartPosition = pPlayer->m_Position;
	return false;
}

CLIENT_COMMAND(adminvision, "<enabled>", "", ADMIN_ACCESS)
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
#endif

CLIENT_COMMAND(serverstatus, "", "Provides information on the server's status.", SENTINEL_ACCESS)
{
	pPlayer->SendText(csprintf("Server status:\n%s", g_pWorld->GetServerStatus().c_str()), LTT_DEFAULT);
	g_pWorld->m_SendPerformanceInfoToPlayer = pPlayer->GetID();

	return false;
}

CLIENT_COMMAND(netinfo, "", "Provides information on the player's network connection.", BASIC_ACCESS)
{
	int sendbuf = 0;
	int sendbuflen = sizeof(sendbuf);
	getsockopt(g_pNetwork->m_sockets[0], SOL_SOCKET, SO_SNDBUF, (char *)&sendbuf, &sendbuflen);
	pPlayer->SendText(csprintf("SNDBUF: %u", sendbuf), LTT_DEFAULT);

	int recvbuf = 0;
	int recvbuflen = sizeof(recvbuf);
	getsockopt(g_pNetwork->m_sockets[0], SOL_SOCKET, SO_RCVBUF, (char *)&recvbuf, &recvbuflen);
	pPlayer->SendText(csprintf("RCVBUF: %u", recvbuf), LTT_DEFAULT);

	pPlayer->SendText(csprintf("INSEQ: %u", player_client->GetPacketController()->m_in.sequence), LTT_DEFAULT);
	pPlayer->SendText(csprintf("ACTIVESEQ: %u", player_client->GetPacketController()->m_in.activesequence), LTT_DEFAULT);
	pPlayer->SendText(csprintf("FLUSHSEQ: %u", player_client->GetPacketController()->m_in.flushsequence), LTT_DEFAULT);
	pPlayer->SendText(csprintf("OUTSEQ: %u", player_client->GetPacketController()->m_out.sequence), LTT_DEFAULT);
	pPlayer->SendText(csprintf("RECEIVED: %I64u", player_client->GetPacketController()->m_in.receivedbytes), LTT_DEFAULT);
	pPlayer->SendText(csprintf("SENT: %I64u", player_client->GetPacketController()->m_out._sentBytes), LTT_DEFAULT);
	pPlayer->SendText(csprintf("RETRANSMIT: %u", player_client->GetPacketController()->m_out.numretransmit), LTT_DEFAULT);
	pPlayer->SendText(csprintf("DENIED: %u", player_client->GetPacketController()->m_out.numdenied), LTT_DEFAULT);
	pPlayer->SendText(csprintf("REQUESTED: %u", player_client->GetPacketController()->m_in.numresendrequests), LTT_DEFAULT);

	return false;
}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(debug, "<index>", "", ADMIN_ACCESS)
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

CLIENT_COMMAND(envmode, "<mode>", "", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	BinaryWriter envMode;
	envMode.Write<DWORD>(0xEA60);
	envMode.Write<DWORD>(strtoul(argv[0], NULL, 16));
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

CLIENT_COMMAND(vismode, "<mode>", "Changes your physics state.", ADMIN_ACCESS)
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
CLIENT_COMMAND(timeadjust, "", "Time adjustment. Careful.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	g_TimeAdjustment = atof(argv[0]);
	return false;
}
#endif

CLIENT_COMMAND(squelchall, "", "Squelch all.", ADMIN_ACCESS)
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

#ifndef PUBLIC_BUILD
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

#ifndef PUBLIC_BUILD
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

CLIENT_COMMAND(spawnwand, "", "Spawns a wand.", BASIC_ACCESS)
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

CLIENT_COMMAND(teleloc, "<landcell> [x=0] [y=0] [z=0] [anglew=1] [anglex=0] [angley=0] [anglez=0]", "Teleports to a specific location.", BASIC_ACCESS)
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
	target.frame.m_origin.x = (float)((argc >= 2) ? atof(argv[1]) : 0);
	target.frame.m_origin.y = (float)((argc >= 3) ? atof(argv[2]) : 0);
	target.frame.m_origin.z = (float)((argc >= 4) ? atof(argv[3]) : 0);
	target.frame.m_angles.w = (float)((argc >= 8) ? atof(argv[4]) : 1.0f);
	target.frame.m_angles.x = (float)((argc >= 8) ? atof(argv[5]) : 0);
	target.frame.m_angles.y = (float)((argc >= 8) ? atof(argv[6]) : 0);
	target.frame.m_angles.z = (float)((argc >= 8) ? atof(argv[7]) : 0);

	if (!target.objcell_id)
		return false;

	pPlayer->Movement_Teleport(target);

	return false;
}

#ifndef PUBLIC_BUILD
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

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = max(0.1, min(10, (float)((argc >= 2) ? atof(argv[1]) : 1.0f)));
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

	DWORD dwModelStart = strtoul(argv[0], NULL, 16);
	if (!(dwModelStart & 0xFF000000))
		dwModelStart |= 0x02000000;

	DWORD dwModelEnd = strtoul(argv[1], NULL, 16);
	if (!(dwModelEnd & 0xFF000000))
		dwModelEnd |= 0x02000000;

	if (dwModelStart > dwModelEnd && dwModelStart != 0)
	{
		return true;
	}

	DWORD dwCount = (dwModelEnd - dwModelStart) + 1;
	DWORD dwWidth = (int)sqrt((double)dwCount);

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
	for (DWORD i = dwModelStart; i <= dwModelEnd; i++)
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

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	DWORD dwPalette1 = strtoul(argv[1], NULL, 16);
	DWORD dwPalette2 = strtoul(argv[2], NULL, 16);
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

#ifndef PUBLIC_BUILD
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

	DWORD dwModel = strtoul(argv[0], NULL, 16);
	if (!(dwModel & 0xFF000000))
		dwModel |= 0x02000000;

	float flScale = max(0.1, min(10, (float)((argc >= 2) ? atof(argv[1]) : 1.0f)));
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
#endif

#if 0
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

	DWORD dwModel = strtoul(argv[0], NULL, 16);
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
#endif

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

CLIENT_COMMAND(clearspawns, "", "Clears the spawns in your current landblock", SENTINEL_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
		return false;

	CWorldLandBlock *pBlock = pPlayer->GetBlock();

	if (pBlock)
		pBlock->ClearSpawns();

	pPlayer->SendText("Clearing spawns in your landblock.", LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(clearallspawns, "", "Clears all spawns in all landblocks (slow.)", SENTINEL_ACCESS)
{
	g_pWorld->ClearAllSpawns();

	pPlayer->SendText("Clearing spawns in all landblocks.", LTT_DEFAULT);
	return false;
}

SERVER_COMMAND(kick, "<player name>", "Kicks the specified player.", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	if (pPlayer)
	{
		LOG(Command, Normal, "\"%s\" is using the kick command.\n", pPlayer->GetName().c_str());
	}
	else
	{
		LOG(Command, Normal, "Server is using the kick command.\n");
	}

	CPlayerWeenie *pTarget = g_pWorld->FindPlayer(argv[0]);

	if (pTarget)
		g_pNetwork->KickClient(pTarget->GetClient());
	else
		pPlayer->SendText("Couldn't find target player.", LTT_DEFAULT);

	return false;
}

SERVER_COMMAND(ban, "<add/remove/list> <player name (if adding) or IP (if removing)> <reason (if adding)>", "Kicks the specified player.", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "list"))
	{
		std::string banList = g_pNetwork->GetBanList();
		pPlayer->SendText(banList.c_str(), LTT_DEFAULT);
		return false;
	}
	else if (!_stricmp(argv[0], "add"))
	{
		if (argc < 3)
			return true;

		CPlayerWeenie *other = g_pWorld->FindPlayer(argv[1]);
		if (other && other->GetClient())
		{
			g_pNetwork->AddBan(other->GetClient()->GetHostAddress()->sin_addr, pPlayer->GetName().c_str(), argv[2]);
			g_pNetwork->KickClient(other->GetClient());

			pPlayer->SendText("Ban added.", LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText("Couldn't find player.", LTT_DEFAULT);
		}

		return false;
	}
	else if (!_stricmp(argv[0], "remove"))
	{
		if (argc < 2)
			return true;

		DWORD ipaddr = inet_addr(argv[1]);
		if (ipaddr != 0 && ipaddr != 0xFFFFFFFF)
		{
			if (g_pNetwork->RemoveBan(*(in_addr *)&ipaddr))
				pPlayer->SendText("Ban removed.", LTT_DEFAULT);
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

CLIENT_COMMAND_WITH_CUSTOM_NAME(_private, private, "<on/off>", "Prevents other users from teleporting to you.", BASIC_ACCESS)
{
	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "on") || !_stricmp(argv[0], "1"))
	{
		pPlayer->SendText("Privacy mode enabled. Other users can no longer teleport to you using the @tele command.", LTT_DEFAULT);
		pPlayer->m_bPrivacyMode = true;
	}
	else if (!_stricmp(argv[0], "off") || !_stricmp(argv[0], "0"))
	{
		pPlayer->SendText("Privacy mode disabled. Other users can now teleport to you using @tele command.", LTT_DEFAULT);
		pPlayer->m_bPrivacyMode = false;
	}

	return true;
}

CLIENT_COMMAND(instakill, "<radius>", "Deals damage to all nearby creatures.", ADMIN_ACCESS)
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

CLIENT_COMMAND(attackable, "<on/off>", "Prevents you from being targeted by monsters.", BASIC_ACCESS)
{
	if (!g_pConfig->EnableAttackableCommand() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "on") || !_stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You are now attackable.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(ATTACKABLE_BOOL, TRUE);
	}
	else if (!_stricmp(argv[0], "off") || !_stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You are no longer attackable.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(ATTACKABLE_BOOL, FALSE);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(dropitemsondeath, "<on/off>", "Determines if you should or not drop items when you die.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "on") || !_stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You now drop items when you die.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(BONDED_INT, 0);
		pPlayer->NotifyIntStatUpdated(BONDED_INT);
	}
	else if (!_stricmp(argv[0], "off") || !_stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You no longer drop items when you die.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetInt(BONDED_INT, 1);
		pPlayer->NotifyIntStatUpdated(BONDED_INT);
	}
	else
		return true;

	return false;
}

CLIENT_COMMAND(resethousepurchasetimestamp, "", "Resets the house purchase timestamp allowing the purchase of another house instantly.", ADMIN_ACCESS)
{
	pPlayer->m_Qualities.RemoveInt(HOUSE_PURCHASE_TIMESTAMP_INT);
	pPlayer->NotifyIntStatUpdated(HOUSE_PURCHASE_TIMESTAMP_INT);
	pPlayer->SendText("Your house purchase timestamp has been reset.", LTT_DEFAULT);

	DWORD houseId = pPlayer->InqIIDQuality(HOUSE_IID, 0);
	if (CWeenieObject *houseWeenie = g_pWorld->FindObject(houseId, true)) //we may have to activate the landblock the house is in
	{
		if (CHouseWeenie *house = houseWeenie->AsHouse())
		{
			if (CSlumLordWeenie *slumlord = house->GetSlumLord())
			{
				if (house->GetHouseOwner() == pPlayer->GetID())
				{
					slumlord->SendHouseData(pPlayer);
					return false;
				}
			}
		}
	}

	//if we can't get the data send the "no house" packet
	BinaryWriter noHouseData;
	noHouseData.Write<DWORD>(0x0226);
	noHouseData.Write<DWORD>(0);
	pPlayer->SendNetMessage(&noHouseData, PRIVATE_MSG, TRUE, FALSE);

	return false;
}

CLIENT_COMMAND(waivenextrent, "<on/off>", "Toggles this rent period rent.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "on") || !_stricmp(argv[0], "1"))
	{
		g_pWorld->BroadcastGlobal(ServerBroadcast("System", "This rent period has been waived, rents do not need to be paid for this period.", LTT_DEFAULT), PRIVATE_MSG);
		g_FreeHouseMaintenancePeriod = true;
	}
	else if (!_stricmp(argv[0], "off") || !_stricmp(argv[0], "0"))
	{
		g_pWorld->BroadcastGlobal(ServerBroadcast("System", "Rent is back in effect for this rent period.", LTT_DEFAULT), PRIVATE_MSG);
		g_FreeHouseMaintenancePeriod = false;
	}
	else
		return true;

	//update everyone's house panel
	PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();
	for (PlayerWeenieMap::iterator i = pPlayers->begin(); i != pPlayers->end(); i++)
	{
		CPlayerWeenie *player = i->second;
		DWORD houseId = player->InqIIDQuality(HOUSE_IID, 0);
		if (CWeenieObject *houseWeenie = g_pWorld->FindObject(houseId, true)) //we may have to activate the landblock the house is in
		{
			if (CHouseWeenie *house = houseWeenie->AsHouse())
			{
				if (CSlumLordWeenie *slumlord = house->GetSlumLord())
				{
					if (house->GetHouseOwner() == player->GetID())
					{
						slumlord->SendHouseData(player);
					}
				}
			}
		}
	}

	return false;
}

CLIENT_COMMAND(usecomponents, "<on/off>", "Allows you to cast spells without having the necessary components.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	if (!_stricmp(argv[0], "on") || !_stricmp(argv[0], "1"))
	{
		pPlayer->SendText("You now require components to cast spells.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(SPELL_COMPONENTS_REQUIRED_BOOL, TRUE);
		pPlayer->NotifyBoolStatUpdated(SPELL_COMPONENTS_REQUIRED_BOOL);
	}
	else if (!_stricmp(argv[0], "off") || !_stricmp(argv[0], "0"))
	{
		pPlayer->SendText("You no longer require components to cast spells.", LTT_DEFAULT);
		pPlayer->m_Qualities.SetBool(SPELL_COMPONENTS_REQUIRED_BOOL, FALSE);
		pPlayer->NotifyBoolStatUpdated(SPELL_COMPONENTS_REQUIRED_BOOL);
	}
	else
		return true;

	return false;
}


CLIENT_COMMAND(addspellbyid, "id", "Adds a spell by ID", ADMIN_ACCESS)
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


#ifndef PUBLIC_BUILD
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

CLIENT_COMMAND(fixbusy, "", "Makes you unbusy if you are stuck.", BASIC_ACCESS)
{
	pPlayer->NotifyAttackDone();
	pPlayer->NotifyInventoryFailedEvent(0, 0);
	pPlayer->NotifyUseDone(0);
	pPlayer->NotifyWeenieError(0);

	return false;
}

CLIENT_COMMAND(testburden, "", "", ADMIN_ACCESS)
{
	float burden = -1.0f;
	pPlayer->m_Qualities.InqLoad(burden);
	pPlayer->SendText(csprintf("Your burden is %f", burden), LTT_DEFAULT);

	return false;
}

#ifndef PUBLIC_BUILD
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

#if 0
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
#endif
	}

	return false;
}
#endif

CLIENT_COMMAND(animation, "<index> [speed=1]", "Plays a primary animation.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	fSpeed = min(10.0, max(0.1, fSpeed));
	
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

CLIENT_COMMAND(animationother, "<index> [speed=1]", "Plays a primary animation.", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	CWeenieObject *other = g_pWorld->FindObject(pPlayer->m_LastAssessed);
	if (!other)
		return false;

	WORD wIndex = atoi(argv[0]);
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	fSpeed = min(10.0, max(0.1, fSpeed));

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

CLIENT_COMMAND(setprefix, "<1 for on, 0 for off>", "Adds a prefix to your name such as +Donor.", DONOR_ACCESS)
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

#ifndef PUBLIC_BUILD
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

CLIENT_COMMAND(setname, "[name]", "Changes the last assessed target's name.", ADMIN_ACCESS)
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

#ifndef PUBLIC_BUILD
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

/*
CLIENT_COMMAND(invisible, "", "Go Invisible", BASIC_ACCESS)
{
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	pPlayer->Animation_PlayPrimary(160, fSpeed, 0.5f);
	return false;
}

CLIENT_COMMAND(visible, "", "Go Visible", BASIC_ACCESS)
{
	float fSpeed = (argc >= 2) ? (float)atof(argv[1]) : 1.0f;
	pPlayer->Animation_PlayPrimary(161, fSpeed, 0.5f);
	return false;
}
*/

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

CLIENT_COMMAND(dungeon, "<command>", "Dungeon commands.", BASIC_ACCESS)
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

		DWORD dwCount = 0;
		std::string UnkList = "Unknown Dungeons:\n";
		while (i != e)
		{
			//DWORD dwCell = i->first;
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

		if (strlen(argv[1]) < 6)
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

		DWORD dwCell = pPlayer->GetLandcell();

		if (CELL_WORD(dwCell) < 0xFF) {
			pPlayer->SendText("Go inside first, silly!", LTT_DEFAULT);
			return false;
		}

		if (strlen(argv[1]) < 6) {
			pPlayer->SendText("Please enter a name of at least 6 characters in length.", LTT_DEFAULT);
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
			DWORD dwCell = pPlayer->GetLandcell();
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
		DL += csprintf("\nEnd of Dungeon List (%lu named dungeons)", (DWORD)pDDs->size());

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

		DWORD dwCount = 0;
		std::string DL = "Dungeon Matches:";
		while (i != e)
		{
			static char szDungeonName[100];
			strncpy(szDungeonName, i->second.szDungeonName, 100);

			if (strstr(strlwr(szDungeonName), strlwr(argv[1])))
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

CLIENT_COMMAND(player, "<command>", "Player commands.", BASIC_ACCESS)
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
		playerList += csprintf("\nEnd of Player List (%u)", (DWORD)pPlayers->size());

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

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(spawnbsd, "", "Spawn bsd for testing if the data is available.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	g_pGameDatabase->SpawnBSD();
	return false;
}

CLIENT_COMMAND(freecam, "", "Allows free camera movement.", ADMIN_ACCESS)
{
	pPlayer->set_state(pPlayer->m_PhysicsState ^ (DWORD)(PARTICLE_EMITTER_PS), TRUE);
	return false;
}

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
	
	DWORD setupID = strtoul(argv[0], NULL, 16);
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
			DWORD gfxID = pSetup->parts[0];
			CGfxObj *gfxobj = CGfxObj::Get(gfxID);

			if (gfxobj)
			{
				for (DWORD i = 0; i < gfxobj->num_surfaces; i++)
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

	DWORD pt = 0;
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

#ifndef PUBLIC_BUILD
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

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(debugresistances, "", "", ADMIN_ACCESS)
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

CLIENT_COMMAND(activeevents, "", "", ADMIN_ACCESS)
{
	std::string eventText = "Enabled events:";

	for (auto &entry : g_pPortalDataEx->_gameEvents._gameEvents)
	{
		if (entry.second._eventState != GameEventState::Off_GameEventState)
		{		
			eventText += "\n";
			eventText += entry.first;
		}
	}

	pPlayer->SendText(eventText.c_str(), LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(myquests, "", "", BASIC_ACCESS)
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

	return false;
}

CLIENT_COMMAND(erasequest, "<name>", "", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.RemoveQuest(argv[0]);
	return false;
}

CLIENT_COMMAND(stampquest, "<name>", "", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.StampQuest(argv[0]);
	return false;
}

CLIENT_COMMAND(incquest, "<name>", "", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.IncrementQuest(argv[0]);
	return false;
}

CLIENT_COMMAND(decquest, "<name>", "", SENTINEL_ACCESS)
{
	if (argc < 1)
		return true;

	pPlayer->_questTable.DecrementQuest(argv[0]);
	return false;
}
#endif

CLIENT_COMMAND(spawntreasure, "<tier>", "Spawn treasure of a specific tier", ADMIN_ACCESS)
{
	if (argc < 1)
		return true;

	//pPlayer->SpawnTreasureInContainer(eTreasureCategory::TreasureCategory_Junk, 1, 3);

	CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(atoi(argv[0]), (eTreasureCategory)getRandomNumber(2, 8));
	//CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(atoi(argv[0]), eTreasureCategory::TreasureCategory_Caster);

	if (treasure)
	{
		treasure->SetInitialPosition(pPlayer->GetPosition());
		if (!g_pWorld->CreateEntity(treasure))
			delete treasure;
	}

	return false;
}

CLIENT_COMMAND(spawntreasure2, "<tier> <num>", "Spawn treasure of a specific tier", ADMIN_ACCESS)
{
	if (argc < 2)
		return true;

	int tier = atoi(argv[0]);
	int num = atoi(argv[1]);

	for (int i = 0; i < num; i++)
	{
		CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(atoi(argv[0]), (eTreasureCategory)getRandomNumber(2, 8));
		//CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(atoi(argv[0]), eTreasureCategory::TreasureCategory_Armor);

		if (treasure)
		{
			treasure->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-2.0, 2.0), Random::GenFloat(-2.0, 2.0), 1.0)));

			if (!g_pWorld->CreateEntity(treasure))
			{
				delete treasure;
				return false;
			}
		}
		else
			continue;
	}

	return false;
}

CLIENT_COMMAND(spawnwcidinv, "<name> [amount] [ptid] [shade]", "Spawn by wcid into inventory.", ADMIN_ACCESS)
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

CLIENT_COMMAND(spawnwcid, "<name> [ptid] [shade]", "Spawn by wcid.", ADMIN_ACCESS)
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

	return false;
}

CLIENT_COMMAND(spawnwcidfresh, "<name> [ptid] [shade]", "Spawn by wcid.", ADMIN_ACCESS)
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

	g_pWeenieFactory->RefreshLocalStorage();

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

	return false;
}

CLIENT_COMMAND(spawnwcidstack, "<name> <amount>", "Spawn a stackable wcid.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 2)
		return true;

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
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	weenie->SetStackSize(atoi(argv[1]));

	if (!g_pWorld->CreateEntity(weenie))
		return false;

	return false;
}

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(spawnwcidgroup, "<name> [num]", "Spawn by wcid.", ADMIN_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	if (argc < 2)
		return true;

	int num = atoi(argv[1]);
	
	for (int i = 0; i < num; i++)
	{
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
			pPlayer->SendText("Couldn't find that to spawn!", 1);
			break;
		}

		weenie->SetInitialPosition(pPlayer->m_Position.add_offset(Vector(Random::GenFloat(-2.0, 2.0), Random::GenFloat(-2.0, 2.0), 1.0)));

		if (!g_pWorld->CreateEntity(weenie))
			break;
	}

	return false;
}

void SpawnAllAppearancesForWeenie(CPlayerWeenie *pPlayer, DWORD wcid, bool bSpawnWithoutVariances = false, DWORD setup_override = 0)
{
	CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(wcid);

	if (!weenieDefs)
	{
		pPlayer->SendText("Couldn't find that to spawn!", LTT_DEFAULT);
		return;
	}

	DWORD clothing_did = 0;
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

CLIENT_COMMAND(spawnwcidpts, "<name>", "Spawn all base appearances of a wcid.", ADMIN_ACCESS)
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

	DWORD motion_table_did = 0;
	if (!weenieDefs->m_Qualities.InqDataID(MOTION_TABLE_DID, motion_table_did))
	{
		pPlayer->SendText("Does not have a motion table.", LTT_DEFAULT);
		return false;
	}

	std::list<DWORD> sameMotionTable = g_pWeenieFactory->GetWCIDsWithMotionTable(motion_table_did);

	for (auto entry : sameMotionTable)
	{
		SpawnAllAppearancesForWeenie(pPlayer, entry);
	}

	return false;
}

CSetup *original_setup_to_find = NULL;

void FindSetupWithMotionID(void *argument, DWORD id, BTEntry *entry)
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
				((std::list<DWORD> *)argument)->push_back(id);
			}
		}

		CSetup::Release(mySetup);
	}
}

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

	DWORD setup_did = 0;
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

	std::list<DWORD> setupList;
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

#if 0
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
#endif

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

/*
CLIENT_COMMAND(spawnpyreal, "[name]", "Spawns pyreal.", BASIC_ACCESS)
{
	if (!SpawningEnabled(pPlayer))
	{
		return false;
	}

	CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(273, &pPlayer->m_Position, false);

	if (!weenie)
	{
		pPlayer->SendText("Couldn't find that to spawn!", 1);
		return false;
	}

	weenie->m_Qualities.SetInt(STACK_SIZE_INT, 25000);
	weenie->m_Qualities.SetInt(MAX_STACK_SIZE_INT, 25000);
	
	g_pWorld->CreateEntity(weenie);

	return false;
}
*/

#ifndef PUBLIC_BUILD
CLIENT_COMMAND(barber, "", "", ADMIN_ACCESS)
{
	//BinaryWriter writer;
	//writer.Write<DWORD>(0x75);
	//writer.Write<DWORD>(pPlayer->InqDIDQuality(BASE_PALETTE_DID, 0x0400007E));
	//pPlayer->SendNetMessage(&writer, PRIVATE_MSG, FALSE, FALSE);

	return false;
}

CLIENT_COMMAND(spawnfollow, "", "", ADMIN_ACCESS)
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

CLIENT_COMMAND(spawnfollow2, "", "", ADMIN_ACCESS)
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

CLIENT_COMMAND(spawnitem2, "[name] [scale]", "Spawns something by name (works for most items.)", ADMIN_ACCESS)
{
	if (pPlayer->GetAccessLevel() < ADMIN_ACCESS)
	{
		pPlayer->SendText("This command is no longer available.", LTT_DEFAULT);
		return false;
	}

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

	float fScale = 1.0f;
	if (argc >= 2)
		fScale = max(0.1, min(10, (float)atof(argv[1])));

	CWeenieObject *pItem = g_pWeenieFactory->CreateWeenieByName(argv[0], &pPlayer->m_Position, false);

	if (pItem)
	{
		g_pWorld->CreateEntity(pItem);
	}
	else
	{
		pItem = g_pGameDatabase->CreateFromCapturedData(pItemInfo);

		pItem->m_scale = pItemInfo->physics.object_scale * fScale;
		pItem->SetInitialPosition(pPlayer->GetPosition());
		pItem->m_bDontClear = false;

		g_pWorld->CreateEntity(pItem);
	}
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

	DWORD sid = pPlayer->GetSetupID();
	const ClothingBase *pCB = pCT->_cloBaseHash.lookup(sid);
	if (pCB)
	{
		for (DWORD i = 0; i < pCB->numObjectEffects; i++)
		{
			pItem->m_miWornModel.lModels.push_back(
				ModelRpl(
				(BYTE)pCB->objectEffects[i].partNum,
				(WORD)pCB->objectEffects[i].objectID));

			for (DWORD j = 0; j < pCB->objectEffects[i].numTextureEffects; j++)
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

	std::map<DWORD, CloPaletteTemplate>::iterator entry = pCT->_paletteTemplatesHash.begin();
	while (variation != 0 && entry != pCT->_paletteTemplatesHash.end())
	{
		variation--;
		entry++;
	}

	if (entry != pCT->_paletteTemplatesHash.end())
	{
		pItem->SetIcon(entry->second.iconID);
		
		for (DWORD i = 0; i < entry->second.numSubpalEffects; i++)
		{
			DWORD set = entry->second.subpalEffects[i].palSet;

			PalSet *pPS = PalSet::Get(set);
			if (pPS)
			{
				DWORD pal = pPS->GetPaletteID(shade);
				
				for (DWORD j = 0; j < entry->second.subpalEffects[i].numRanges; j++)
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

#ifndef PUBLIC_BUILD
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
		fScale = max(0.1, min(10, (float) atof(argv[1])));

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

CLIENT_COMMAND(pk, "", "Makes you a player killer.", ADMIN_ACCESS)
{
	if (!pPlayer->IsAdmin())
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

CLIENT_COMMAND(npk, "", "Makes you a non-player killer.", BASIC_ACCESS)
{
	if (!pPlayer->IsAdmin())
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

CLIENT_COMMAND(decent, "[name]", "Gives you great attributes.", SENTINEL_ACCESS)
{
	if (pPlayer->GetClient()->GetAccessLevel() < ADVOCATE_ACCESS)
	{
		pPlayer->SendText("Command temporarily disabled for this play test. XP is awarded off monsters.", 1);
		return false;
	}

	pPlayer->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetAttribute(SELF_ATTRIBUTE, 200);
	pPlayer->m_Qualities.SetInt(LEVEL_INT, 100);
	pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, 0);
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

CLIENT_COMMAND(godly, "", "Gives you great attributes.", BASIC_ACCESS)
{
	if (!g_pConfig->EnableGodlyCommand() && pPlayer->GetClient()->GetAccessLevel() < ADMIN_ACCESS)
	{
		pPlayer->SendText("Command disabled.", 1);
		return false;
	}

	pPlayer->SendText("Use /decent instead if you want to play fair.", 1);

	pPlayer->m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 1000000);
	pPlayer->m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 800);
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

CLIENT_COMMAND(givexp, "[value]", "Gives you some XP for testing.", BASIC_ACCESS)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	DWORD amount = 0;

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = min(amount, 4000000000);
	// pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, amount);

	// pPlayer->SendText("Your attributes have been greatly increased.", 1);
	pPlayer->GiveXP(amount, true);

	return false;
}

CLIENT_COMMAND(givecredit, "[value]", "Gives you some skill credits for testing.", BASIC_ACCESS)
{
	if (!g_pConfig->EnableXPCommands() && pPlayer->GetAccessLevel() < SENTINEL_ACCESS)
	{
		pPlayer->SendText("This command is not enabled on this server.", LTT_DEFAULT);
		return false;
	}

	DWORD amount = 0;
	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = max(amount, 1);
	amount = min(amount, 100);

	pPlayer->GiveSkillCredits(amount, true);

	return false;
}


CLIENT_COMMAND(givexpother, "[value]", "Gives you some XP for testing.", ADMIN_ACCESS)
{
	DWORD amount = 0;

	if (argc >= 1)
	{
		amount = strtoul(argv[0], NULL, 10);
	}

	amount = min(amount, 4000000000);
	// pPlayer->m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, amount);

	// pPlayer->SendText("Your attributes have been greatly increased.", 1);

	CWeenieObject *assessed = g_pWorld->FindObject(pPlayer->m_LastAssessed);

	if (assessed && assessed->AsPlayer())
	{
		assessed->GiveXP(amount, true);
	}

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
	CommandMap::iterator i = m_mCommands.find(szName);

	if (i == m_mCommands.end())
		return NULL;

	if (iAccessLevel == -1 || iAccessLevel >= i->second.access)
		return &i->second;
	else
		return NULL;
}

bool CommandBase::Execute(char *command, CClient *client)
{
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
		if (!stricmp(command_name, "help"))
		{
			if (!client)
				return true;

			if (argc > 1)
			{
				CommandEntry* pCommand = FindCommand(argv[1], access_level);

				if (pCommand)
				{
					player_weenie->SendText(Info(pCommand), 1);
					return true;
				}
				else
				{
					player_weenie->SendText("Unknown command. Use !help to receive a list of commands.", 1);
				}
			}
			else
			{
				// List all commands.
				std::string strCommandList = "Command List: \n";

				for (CommandMap::iterator it = m_mCommands.begin(); it != m_mCommands.end(); it++)
				{
					if (it->second.access <= access_level) {
						strCommandList += Info(&it->second);
						strCommandList += "\n";
					}
				}
				strCommandList += "Use !help <command> to receive command syntax.\n";
				player_weenie->SendText(strCommandList.c_str(), 1);
				return true;
			}
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
					LOG(Temp, Normal, "EXECUTING CLIENT COMMAND %s FROM %s\n", command, client->GetDescription());

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

ClientCommand::ClientCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel)
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, true);
}

ServerCommand::ServerCommand(const char* szName, const char* szArguments, const char* szHelp, pfnCommandCallback pCallback, int iAccessLevel)
{
	CommandBase::Create(szName, szArguments, szHelp, pCallback, iAccessLevel, false);
}
