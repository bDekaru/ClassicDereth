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
#include "InferredPortalData.h"

CLIENT_COMMAND(setvoidconfig, "[multipler]", "Used to limit void damage 0.1 = 10%, 1.0 = 100%", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Multiplier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);
	g_pConfig->SetVoidDamageReduction(newValue);

	pPlayer->SendText(csprintf("Void Damage set to %f of max", newValue), LTT_DEFAULT);

	return false;
}

CLIENT_COMMAND(reloadquests, "", "Reloads quests.json", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if(g_pPortalDataEx->ReloadQuestData())
		return false;

	return true;
}

CLIENT_COMMAND(reloadevents, "", "Reloads events.json and sets active per json settings", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (g_pPortalDataEx->ReloadEventData())
		return false;

	return true;
}

CLIENT_COMMAND(globalxpmult, "value", "Sets global xp mod for Kill, Quest, and Lum", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Multiplier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetGlobalXpMultiplier(newValue);
	return false;
}

CLIENT_COMMAND(killxpmult, "<tier> <value>", "Sets xp mod for Kill", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
	{
		pPlayer->SendText("Tier or multiplier missing", LTT_DEFAULT);
		return true;
	}
	int tier = atof(argv[0]);
	double newValue = atof(argv[1]);

	g_pConfig->SetKillXpMulitplier(tier, newValue);
	return false;
}

CLIENT_COMMAND(rewardxpmult, "<tier> <value>", "Sets  xp mod for Quest", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 2)
	{
		pPlayer->SendText("Tier or multiplier missing", LTT_DEFAULT);
		return true;
	}
	int tier = atof(argv[0]);
	double newValue = atof(argv[1]);

	g_pConfig->SetRewardXpMultiplier(tier, newValue);
	return false;
}

CLIENT_COMMAND(glumxpmult, "value", "Sets xp mod for Lum", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Multiplier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetLumXpMultiplier(newValue);
	return false;
}

CLIENT_COMMAND(welcomemessage, "message", "Sets new welcome message in chat window", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Entry missing use two double quotes to clear message", LTT_DEFAULT);
		return true;
	}
	
	g_pConfig->SetWelcomeMessage(argv[0]);
	return false;
}

CLIENT_COMMAND(reloadtreasure, "", "Reloads treasureprofile.json", ADMIN_ACCESS, SERVER_CATEGORY)
{
	g_pTreasureFactory->Initialize();
	pPlayer->SendText("Treasure profile reloaded", LTT_DEFAULT);
	return false;
}

CLIENT_COMMAND(missileadj, "value", "adjusts missle attribute modifier", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetMissileAttributeAdjust(newValue);
	return false;
}

CLIENT_COMMAND(blueproc, "value", "Sets blue sigil proc rate", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetBlueSigilProcRate(newValue);
	return false;
}

CLIENT_COMMAND(yellowproc, "value", "Sets yellow sigil proc rate", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetYellowSigilProcRate(newValue);
	return false;
}

CLIENT_COMMAND(redproc, "value", "Sets red sigil proc rate", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetRedSigilProcRate(newValue);
	return false;
}

CLIENT_COMMAND(cloakbaseproc, "value", "Sets cloak base proc rate", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetCloakBaseProcRate(newValue);
	return false;
}

CLIENT_COMMAND(cloakhalfhealthproc, "value", "Sets cloak bonus proc rate for losing 50% or more health at once", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetCloakHalfHealthProcRate(newValue);
	return false;
}

CLIENT_COMMAND(cloakquarterhealthproc, "value", "Sets cloak bonus proc rate for losing 25% or more health at once", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetCloakQuarterHealthProcRate(newValue);
	return false;
}

CLIENT_COMMAND(cloaktenthhealthproc, "value", "Sets cloak bonus proc rate for losing 10% or more health at once", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetCloakTenthHealthProcRate(newValue);
	return false;
}

CLIENT_COMMAND(cloakperlevelproc, "value", "Sets cloak bonus proc rate for each level above 1", ADMIN_ACCESS, SERVER_CATEGORY)
{
	if (argc < 1)
	{
		pPlayer->SendText("Modifier missing", LTT_DEFAULT);
		return true;
	}
	double newValue = atof(argv[0]);

	g_pConfig->SetCloakPerLevelProcRate(newValue);
	return false;
}
