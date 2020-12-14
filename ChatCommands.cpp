#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "Config.h"
#include "Server.h"
#include "ClientCommands.h"

CLIENT_COMMAND(gag, "action [charactername] [duration] [-account]", "Gag players add|remove|list with duration in seconds (-1 = perm) use '-account' for all characters. Put quotes around characters with spaces in name.", ADMIN_ACCESS, CHARACTER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Check usage", LTT_DEFAULT);
		return true;
	}

	std::string action = argv[0];
	std::transform(action.begin(), action.end(), action.begin(), ::tolower);
	if (!stricmp(action.c_str(), "add"))
	{
		if (argc < 3)
		{
			pPlayer->SendText("Check usage", LTT_DEFAULT);
			return true;
		}

		CPlayerWeenie* target = g_pWorld->FindPlayer(argv[1]);
		if (!target)
		{
			pPlayer->SendText("Player not found.", LTT_DEFAULT);
			return true;
		}

		double duration = atof(argv[2]);
		if (!duration)
		{
			pPlayer->SendText("Duration invalid.", LTT_DEFAULT);
			return true;
		}

		bool isAccount = false;
		if (argc == 4)
		{
			std::string accountWide = argv[3];
			std::transform(accountWide.begin(), accountWide.end(), accountWide.begin(), ::tolower);
			if (stricmp(accountWide.c_str(), "-account"))
			{
				pPlayer->SendText("Invalid option.", LTT_DEFAULT);
				return true;
			}
			isAccount = true;
		}
		
		if (!g_pDBIO->SaveServerGag(target->GetID(), duration, isAccount))
		{
			pPlayer->SendText("Gag failed to save.", LTT_DEFAULT);
		}
		pPlayer->SendText("Gag saved.", LTT_DEFAULT);

		target->m_Qualities.SetFloat(GAG_TIMESTAMP_FLOAT, Time::GetTimeCurrent());
		target->NotifyFloatStatUpdated(GAG_TIMESTAMP_FLOAT);
		target->m_Qualities.SetFloat(GAG_DURATION_FLOAT, duration);
		target->NotifyFloatStatUpdated(GAG_DURATION_FLOAT);
		target->m_Qualities.SetBool(IS_GAGGED_BOOL, true);
		target->NotifyBoolStatUpdated(IS_GAGGED_BOOL);
	}
	else if (!stricmp(action.c_str(), "list"))
	{
		if (argc > 1)
		{
			pPlayer->SendText("Check usage", LTT_DEFAULT);
			return true;
		}

		std::list<CharacterGags_t> serverGags = g_pDBIO->GetServerGagData();
		if (serverGags.size() > 0)
		{
			pPlayer->SendText("Name        Duration    Times Gagged", LTT_DEFAULT);
			for (auto gag : serverGags)
			{
				pPlayer->SendText(csprintf("%s    %f    %d", gag.name.c_str(), gag.gag_timer, gag.gag_count), LTT_DEFAULT);
			}
			pPlayer->SendText("Use /gag show [PlayerName] for single player", LTT_DEFAULT);
		}
		else
		{
			pPlayer->SendText("No server gags found", LTT_DEFAULT);
		}
		
	}
	else if (!stricmp(action.c_str(), "show"))
	{
		if (argc < 2 || argc > 3)
		{
			pPlayer->SendText("Check usage", LTT_DEFAULT);
			return true;
		}


		CPlayerWeenie* target = g_pWorld->FindPlayer(argv[1]);
		if (!target)
		{
			pPlayer->SendText("Player not found.", LTT_DEFAULT);
			return true;
		}

		CharacterGags_t playerGag = g_pDBIO->GetCharacterGagData(target->GetID());
		if (playerGag.name.size() > 0)
		{
			if (playerGag.gag_timer == -1)
			{
				pPlayer->SendText(csprintf("%s gagged permanently and has been gagged %d times.", playerGag.name.c_str(), playerGag.gag_count), LTT_DEFAULT);
			}
			else
			{
				pPlayer->SendText(csprintf("%s gagged for %f and has been gagged %d times.", playerGag.name.c_str(), playerGag.gag_timer, playerGag.gag_count), LTT_DEFAULT);
			}
		}
		else
		{
			pPlayer->SendText(csprintf("%s is not gagged", target->GetName().c_str()), LTT_DEFAULT);
		}
	}
	else if (!stricmp(action.c_str(), "remove"))
	{
		if (argc > 3)
		{
			pPlayer->SendText("Check usage", LTT_DEFAULT);
			return true;
		}


		CPlayerWeenie* target = g_pWorld->FindPlayer(argv[1]);
		if (!target)
		{
			pPlayer->SendText("Player not found.", LTT_DEFAULT);
			return true;
		}

		bool isAccount = false;
		if (argc == 3)
		{
			std::string accountWide = argv[2];
			std::transform(accountWide.begin(), accountWide.end(), accountWide.begin(), ::tolower);
			if (stricmp(accountWide.c_str(), "-account"))
			{
				pPlayer->SendText("Invalid option.", LTT_DEFAULT);
				return true;
			}
			isAccount = true;
		}


		if (!g_pDBIO->RemoveServerGag(target->GetID(), isAccount))
		{
			pPlayer->SendText("Gag remove failed.", LTT_DEFAULT);
		}

		target->m_Qualities.SetFloat(GAG_DURATION_FLOAT, 0);
		target->NotifyFloatStatUpdated(GAG_DURATION_FLOAT);
		target->m_Qualities.SetBool(IS_GAGGED_BOOL, false);
		target->NotifyBoolStatUpdated(IS_GAGGED_BOOL);
	}
	else
	{
		pPlayer->SendText("Invalid gag command", LTT_DEFAULT);
		return true;
	}

	return false;
}
