#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "ObjectMsgs.h"
#include "EnumUtil.h"

#include "ClientCommands.h"

/*CLIENT_COMMAND(fixbusy, "", "Makes you unbusy if you are stuck.", BASIC_ACCESS, GENERAL_CATEGORY)
{
	pPlayer->NotifyAttackDone();
	pPlayer->NotifyInventoryFailedEvent(0, 0);
	if (pPlayer->m_UseManager)
		pPlayer->m_UseManager->Cancel();
	pPlayer->NotifyUseDone(0);
	pPlayer->NotifyWeenieError(0);
	pPlayer->m_bChangingStance = false;

	pPlayer->ChangeCombatMode(NONCOMBAT_COMBAT_MODE, false);
	pPlayer->m_bChangingStance = false;

	return false;
}*/

CLIENT_COMMAND(fixcombat, "", "Forces Peace mode if you are stuck in combat.", BASIC_ACCESS, GENERAL_CATEGORY)
{
    pPlayer->StopCompletely(0);
	pPlayer->NotifyAttackDone();
	pPlayer->m_bChangingStance = true;
	pPlayer->ChangeCombatMode(NONCOMBAT_COMBAT_MODE, false);
	pPlayer->m_bChangingStance = false;

	return false;
}

CLIENT_COMMAND(fixclient, "", "Resets the client back to login state.", BASIC_ACCESS, GENERAL_CATEGORY)
{
	BinaryWriter *LC = ::LoginCharacter(pPlayer);
	pPlayer->SendNetMessage(LC->GetData(), LC->GetSize(), PRIVATE_MSG, TRUE);
	delete LC;

	return false;
}

CLIENT_COMMAND(config, "<setting> <on/off>", "Manually sets a character option on the server.\nUse /config list to see a list of settings.", BASIC_ACCESS, CHARACTER_CATEGORY)
{
	bool bError = false;
	if (argc < 1)
	{
		bError = true;
	}

	int iSetTo = 0;
	if (argc > 1)
	{
		if (strcmp(argv[1], "on") == 0)
		{
			iSetTo = 1;
		}
		else if (strcmp(argv[1], "off") == 0)
		{
			iSetTo = -1;
		}
	}

	if (!bError)
	{
		if (strcmp(argv[0], "list") == 0)
		{
			pPlayer->SendText("Common settings:\nConfirmVolatileRareUse, MainPackPreferred, SalvageMultiple, SideBySideVitals, UseCraftSuccessDialog", LTT_DEFAULT);
			pPlayer->SendText("Interaction settings:\nAcceptLootPermits, AllowGive, AppearOffline, AutoAcceptFellowRequest, DragItemOnPlayerOpensSecureTrade, FellowshipShareLoot, FellowshipShareXP, IgnoreAllegianceRequests, IgnoreFellowshipRequests, IgnoreTradeRequests, UseDeception", LTT_DEFAULT);
			pPlayer->SendText("UI settings:\nCoordinatesOnRadar, DisableDistanceFog, DisableHouseRestrictionEffects, DisableMostWeatherEffects, FilterLanguage, LockUI, PersistentAtDay, ShowCloak, ShowHelm, ShowTooltips, SpellDuration, TimeStamp, ToggleRun, UseMouseTurning", LTT_DEFAULT);
			pPlayer->SendText("Chat settings:\nHearAllegianceChat, HearGeneralChat, HearLFGChat, HearRoleplayChat, HearSocietyChat, HearTradeChat, StayInChatMode", LTT_DEFAULT);
			pPlayer->SendText("Combat settings:\nAdvancedCombatUI, AutoRepeatAttack, AutoTarget, LeadMissileTargets, UseChargeAttack, UseFastMissiles, ViewCombatTarget, VividTargetingIndicator", LTT_DEFAULT);
			pPlayer->SendText("Character display settings:\nDisplayAge, DisplayAllegianceLogonNotifications, DisplayChessRank, DisplayDateOfBirth, DisplayFishingSkill, DisplayNumberCharacterTitles, DisplayNumberDeaths", LTT_DEFAULT);
			return false;
		}

		//int settingMask = 0;
		//bool bOptions1 = FALSE;
		//bool bOptions2 = FALSE;

		std::string strOption = argv[0];
		CharacterOption option1 = EnumUtil.StringToCharacterOption(strOption);
		if (option1)
		{
			uint32_t charOptions = pPlayer->GetCharacterOptions();

			iSetTo = (charOptions & option1) ? -1 : 1;

			if (iSetTo == 1)
			{
				charOptions |= option1;
			}
			else
			{
				charOptions &= ~option1;
			}

			pPlayer->SetCharacterOptions(charOptions);

			std::string onOff = iSetTo == 1 ? "on" : "off";

			pPlayer->SendText(("Character option " + strOption + " is now " + onOff + ".").c_str(), LTT_SYSTEM_EVENT);

			// Update the client
			BinaryWriter *LC = ::LoginCharacter(pPlayer);
			pPlayer->SendNetMessage(LC->GetData(), LC->GetSize(), PRIVATE_MSG, TRUE);
			delete LC;
			return false;
		}

		CharacterOptions2 option2 = EnumUtil.StringToCharacterOptions2(argv[0]);
		if (option2)
		{
			uint32_t charOptions2 = pPlayer->GetCharacterOptions2();

			iSetTo = (charOptions2 & option2) ? -1 : 1;

			if (iSetTo == 1)
			{
				charOptions2 |= option2;
			}
			else
			{
				charOptions2 &= ~option2;
			}

			pPlayer->SetCharacterOptions2(charOptions2);

			std::string onOff = iSetTo == 1 ? "on" : "off";

			pPlayer->SendText(("Character option " + strOption + " is now " + onOff + ".").c_str(), LTT_SYSTEM_EVENT);

			// Update the client
			BinaryWriter *LC = ::LoginCharacter(pPlayer);
			pPlayer->SendNetMessage(LC->GetData(), LC->GetSize(), PRIVATE_MSG, TRUE);
			delete LC;
			return false;
		}

		pPlayer->SendText("Unrecognised setting!", LTT_DEFAULT);
	}

	return true;
}
