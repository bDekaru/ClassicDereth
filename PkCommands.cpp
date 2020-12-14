#include <StdAfx.h>
#include "ClientCommands.h"
#include "Player.h"
#include "Config.h"

CLIENT_COMMAND(getpksettings, "", "Prints out the current server PK CS/CB settings", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	pPlayer->SendText("Melee Settings", LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Base Chance: %f", g_pConfig->GetPkCSMeleeBaseChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Max Chance: %f", g_pConfig->GetPkCSMeleeMaxChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Base: %d", g_pConfig->GetPkCSMeleeMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Max: %d", g_pConfig->GetPkCSMeleeMaxSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Base Mutliplier: %f", g_pConfig->GetPkCBMeleeBaseMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Max Multiplier: %f", g_pConfig->GetPkCBMeleeMaxMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Base: %d", g_pConfig->GetPkCBMeleeMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Max: %d", g_pConfig->GetPkCBMeleeMaxSkill()), LTT_DEFAULT);
	pPlayer->SendText("Missile Settings", LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Base Chance: %f", g_pConfig->GetPkCSMissileBaseChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Max Chance: %f", g_pConfig->GetPkCSMissileMaxChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Base: %d", g_pConfig->GetPkCSMissileMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Max: %d", g_pConfig->GetPkCSMissileMaxSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Base Mutliplier: %f", g_pConfig->GetPkCBMissileBaseMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Max Multiplier: %f", g_pConfig->GetPkCBMissileMaxMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Base: %d", g_pConfig->GetPkCBMissileMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Max: %d", g_pConfig->GetPkCBMissileMaxSkill()), LTT_DEFAULT);
	pPlayer->SendText("Magic Settings", LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Base Chance: %f", g_pConfig->GetPkCSMagicBaseChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Max Chance: %f", g_pConfig->GetPkCSMagicMaxChance()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Base: %d", g_pConfig->GetPkCSMagicMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CS Skill Max: %d", g_pConfig->GetPkCSMagicMaxSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Base Mutliplier: %f", g_pConfig->GetPkCBMagicBaseMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Max Multiplier: %f", g_pConfig->GetPkCBMagicMaxMult()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Base: %d", g_pConfig->GetPkCBMagicMinSkill()), LTT_DEFAULT);
	pPlayer->SendText(csprintf("CB Skill Max: %d", g_pConfig->GetPkCBMagicMaxSkill()), LTT_DEFAULT);


	return false;
}

CLIENT_COMMAND(setpksetting, "[Combatmode] [Imbue] [Setting] [minmax] value", "Updates setting for melee|missile|magic CS|CB skill|chance|mult base|max", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 5)
	{
		pPlayer->SendText("Input invalid", LTT_DEFAULT);
		return true;
	}

	std::string cmode(argv[0]);
	std::transform(cmode.begin(), cmode.end(), cmode.begin(), ::tolower);
	int combatMode = 0;
	if (!stricmp(cmode.c_str(), "melee"))
	{
		combatMode = 1;
	}
	else if (!stricmp(cmode.c_str(), "missile"))
	{
		combatMode = 2;
	}
	else if (!stricmp(cmode.c_str(), "magic"))
	{
		combatMode = 3;
	}
	else
	{
		pPlayer->SendText("Invalid combat mode - melee or missile or magic", LTT_DEFAULT);
		return true;
	}

	std::string imbue(argv[1]);
	std::transform(imbue.begin(), imbue.end(), imbue.begin(), ::tolower);
	int imbueType = 0;
	if (!stricmp(imbue.c_str(), "cs"))
	{
		imbueType = 1;
	}
	else if (!stricmp(imbue.c_str(), "cb"))
	{
		imbueType = 2;
	}
	else
	{
		pPlayer->SendText("Invalid imbue - CS or CB", LTT_DEFAULT);
		return true;
	}

	std::string setting(argv[2]);
	std::transform(setting.begin(), setting.end(), setting.begin(), ::tolower);
	int settingValue = 0;
	if (!stricmp(setting.c_str(), "skill"))
	{
		settingValue = 1;
	}
	else if (!stricmp(setting.c_str(), "chance"))
	{
		settingValue = 2;
	}
	else if (!stricmp(setting.c_str(), "mult"))
	{
		settingValue = 3;
	}
	else
	{
		pPlayer->SendText("Invalid setting - skill or chance", LTT_DEFAULT);
		return true;
	}

	std::string minmax(argv[3]);
	std::transform(minmax.begin(), minmax.end(), minmax.begin(), ::tolower);
	int minmaxValue = 0;
	if (!stricmp(minmax.c_str(), "base"))
	{
		minmaxValue = 1;
	}
	else if (!stricmp(minmax.c_str(), "max"))
	{
		minmaxValue = 2;
	}
	else
	{
		pPlayer->SendText("Invalid value - min or max", LTT_DEFAULT);
		return true;
	}


	if (combatMode == 1 && imbueType == 1 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMeleeMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCSMeleeMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 1 && imbueType == 1 && settingValue == 2)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMeleeBaseChance(atof(argv[4])); 
		else
			g_pConfig->SetPkCSMeleeMaxChance(atof(argv[4])); 
	}
	else if (combatMode == 1 && imbueType == 2 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMeleeMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCBMeleeMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 1 && imbueType == 2 && settingValue == 3)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMeleeBaseMult(atof(argv[4])); 
		else
			g_pConfig->SetPkCBMeleeMaxMult(atof(argv[4])); 
	}
	else if (combatMode == 2 && imbueType == 1 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMissileMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCSMissileMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 2 && imbueType == 1 && settingValue == 2)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMissileBaseChance(atof(argv[4]));
		else
			g_pConfig->SetPkCSMissileMaxChance(atof(argv[4]));
	}
	else if (combatMode == 2 && imbueType == 2 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMissileMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCBMissileMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 2 && imbueType == 2 && settingValue == 3)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMissileBaseMult(atof(argv[4]));
		else
			g_pConfig->SetPkCBMissileMaxMult(atof(argv[4]));
	}
	else if (combatMode == 3 && imbueType == 1 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMagicMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCSMagicMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 3 && imbueType == 1 && settingValue == 2)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCSMagicBaseChance(atof(argv[4]));
		else
			g_pConfig->SetPkCSMagicMaxChance(atof(argv[4]));
	}
	else if (combatMode == 3 && imbueType == 2 && settingValue == 1)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMagicMinSkill(stoi(argv[4], NULL, 10));
		else
			g_pConfig->SetPkCBMagicMaxSkill(stoi(argv[4], NULL, 10));
	}
	else if (combatMode == 3 && imbueType == 2 && settingValue == 3)
	{
		if (minmaxValue == 1)
			g_pConfig->SetPkCBMagicBaseMult(atof(argv[4]));
		else
			g_pConfig->SetPkCBMagicMaxMult(atof(argv[4]));
	}
	else
	{
		pPlayer->SendText("Invalid value combination", LTT_DEFAULT);
		return true;
	}
	return false;
}
