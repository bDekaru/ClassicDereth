#pragma once

#include <StdAfx.h>
#include "EnumUtil.h"

EnumUtility EnumUtil = EnumUtility();

EnumUtility::EnumUtility()
{
	loadStringToCharacterOptions();
}

CharacterOption EnumUtility::StringToCharacterOption(std::string strOption)
{
	if (m_mapStringToCharacterOption.find(strOption) != m_mapStringToCharacterOption.end())
		return m_mapStringToCharacterOption[strOption];

	return Undef_CharacterOption;
}

std::string EnumUtility::CharacterOptionToString(CharacterOption coOption)
{
	if (m_mapCharacterOptionToString.find(coOption) != m_mapCharacterOptionToString.end())
		return m_mapCharacterOptionToString[coOption];

	return "Unknown Option";
}

CharacterOptions2 EnumUtility::StringToCharacterOptions2(std::string strOption)
{
	if (m_mapStringToCharacterOptions2.find(strOption) != m_mapStringToCharacterOptions2.end())
		return m_mapStringToCharacterOptions2[strOption];

	return Undef_CharacterOptions2;
}

std::string EnumUtility::CharacterOptions2ToString(CharacterOptions2 coOption)
{
	if (m_mapCharacterOptions2ToString.find(coOption) != m_mapCharacterOptions2ToString.end())
		return m_mapCharacterOptions2ToString[coOption];

	return "Unknown Option";
}

void EnumUtility::loadStringToCharacterOptions()
{
	m_mapStringToCharacterOption.emplace("AutoRepeatAttack", AutoRepeatAttack_CharacterOption);
	m_mapCharacterOptionToString.emplace(AutoRepeatAttack_CharacterOption, "AutoRepeatAttack");
	m_mapStringToCharacterOption.emplace("IgnoreAllegianceRequests", IgnoreAllegianceRequests_CharacterOption);
	m_mapCharacterOptionToString.emplace(IgnoreAllegianceRequests_CharacterOption, "IgnoreAllegianceRequests");
	m_mapStringToCharacterOption.emplace("IgnoreFellowshipRequests", IgnoreFellowshipRequests_CharacterOption);
	m_mapCharacterOptionToString.emplace(IgnoreFellowshipRequests_CharacterOption, "IgnoreFellowshipRequests");
	m_mapStringToCharacterOption.emplace("AllowGive", AllowGive_CharacterOption);
	m_mapCharacterOptionToString.emplace(AllowGive_CharacterOption, "AllowGive");
	m_mapStringToCharacterOption.emplace("ViewCombatTarget", ViewCombatTarget_CharacterOption);
	m_mapCharacterOptionToString.emplace(ViewCombatTarget_CharacterOption, "ViewCombatTarget");
	m_mapStringToCharacterOption.emplace("ShowTooltips", ShowTooltips_CharacterOption);
	m_mapCharacterOptionToString.emplace(ShowTooltips_CharacterOption, "ShowTooltips");
	m_mapStringToCharacterOption.emplace("UseDeception", UseDeception_CharacterOption);
	m_mapCharacterOptionToString.emplace(UseDeception_CharacterOption, "UseDeception");
	m_mapStringToCharacterOption.emplace("ToggleRun", ToggleRun_CharacterOption);
	m_mapCharacterOptionToString.emplace(ToggleRun_CharacterOption, "ToggleRun");
	m_mapStringToCharacterOption.emplace("StayInChatMode", StayInChatMode_CharacterOption);
	m_mapCharacterOptionToString.emplace(StayInChatMode_CharacterOption, "StayInChatMode");
	m_mapStringToCharacterOption.emplace("AdvancedCombatUI", AdvancedCombatUI_CharacterOption);
	m_mapCharacterOptionToString.emplace(AdvancedCombatUI_CharacterOption, "AdvancedCombatUI");
	m_mapStringToCharacterOption.emplace("AutoTarget", AutoTarget_CharacterOption);
	m_mapCharacterOptionToString.emplace(AutoTarget_CharacterOption, "AutoTarget");
	m_mapStringToCharacterOption.emplace("VividTargetingIndicator", VividTargetingIndicator_CharacterOption);
	m_mapCharacterOptionToString.emplace(VividTargetingIndicator_CharacterOption, "VividTargetingIndicator");
	m_mapStringToCharacterOption.emplace("DisableMostWeatherEffects", DisableMostWeatherEffects_CharacterOption);
	m_mapCharacterOptionToString.emplace(DisableMostWeatherEffects_CharacterOption, "DisableMostWeatherEffects");
	m_mapStringToCharacterOption.emplace("IgnoreTradeRequests", IgnoreTradeRequests_CharacterOption);
	m_mapCharacterOptionToString.emplace(IgnoreTradeRequests_CharacterOption, "IgnoreTradeRequests");
	m_mapStringToCharacterOption.emplace("FellowshipShareXP", FellowshipShareXP_CharacterOption);
	m_mapCharacterOptionToString.emplace(FellowshipShareXP_CharacterOption, "FellowshipShareXP");
	m_mapStringToCharacterOption.emplace("AcceptLootPermits", AcceptLootPermits_CharacterOption);
	m_mapCharacterOptionToString.emplace(AcceptLootPermits_CharacterOption, "AcceptLootPermits");
	m_mapStringToCharacterOption.emplace("FellowshipShareLoot", FellowshipShareLoot_CharacterOption);
	m_mapCharacterOptionToString.emplace(FellowshipShareLoot_CharacterOption, "FellowshipShareLoot");
	m_mapStringToCharacterOption.emplace("SideBySideVitals", SideBySideVitals_CharacterOption);
	m_mapCharacterOptionToString.emplace(SideBySideVitals_CharacterOption, "SideBySideVitals");
	m_mapStringToCharacterOption.emplace("CoordinatesOnRadar", CoordinatesOnRadar_CharacterOption);
	m_mapCharacterOptionToString.emplace(CoordinatesOnRadar_CharacterOption, "CoordinatesOnRadar");
	m_mapStringToCharacterOption.emplace("SpellDuration", SpellDuration_CharacterOption);
	m_mapCharacterOptionToString.emplace(SpellDuration_CharacterOption, "SpellDuration");
	m_mapStringToCharacterOption.emplace("DisableHouseRestrictionEffects", DisableHouseRestrictionEffects_CharacterOption);
	m_mapCharacterOptionToString.emplace(DisableHouseRestrictionEffects_CharacterOption, "DisableHouseRestrictionEffects");
	m_mapStringToCharacterOption.emplace("DragItemOnPlayerOpensSecureTrade", DragItemOnPlayerOpensSecureTrade_CharacterOption);
	m_mapCharacterOptionToString.emplace(DragItemOnPlayerOpensSecureTrade_CharacterOption, "DragItemOnPlayerOpensSecureTrade");
	m_mapStringToCharacterOption.emplace("DisplayAllegianceLogonNotifications", DisplayAllegianceLogonNotifications_CharacterOption);
	m_mapCharacterOptionToString.emplace(DisplayAllegianceLogonNotifications_CharacterOption, "DisplayAllegianceLogonNotifications");
	m_mapStringToCharacterOption.emplace("UseChargeAttack", UseChargeAttack_CharacterOption);
	m_mapCharacterOptionToString.emplace(UseChargeAttack_CharacterOption, "UseChargeAttack");
	m_mapStringToCharacterOption.emplace("AutoAcceptFellowRequest", AutoAcceptFellowRequest_CharacterOption);
	m_mapCharacterOptionToString.emplace(AutoAcceptFellowRequest_CharacterOption, "AutoAcceptFellowRequest");
	m_mapStringToCharacterOption.emplace("HearAllegianceChat", HearAllegianceChat_CharacterOption);
	m_mapCharacterOptionToString.emplace(HearAllegianceChat_CharacterOption, "HearAllegianceChat");
	m_mapStringToCharacterOption.emplace("UseCraftSuccessDialog", UseCraftSuccessDialog_CharacterOption);
	m_mapCharacterOptionToString.emplace(UseCraftSuccessDialog_CharacterOption, "UseCraftSuccessDialog");

	m_mapStringToCharacterOptions2.emplace("PersistentAtDay", PersistentAtDay_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(PersistentAtDay_CharacterOptions2, "PersistentAtDay");
	m_mapStringToCharacterOptions2.emplace("DisplayDateOfBirth", DisplayDateOfBirth_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayDateOfBirth_CharacterOptions2, "DisplayDateOfBirth");
	m_mapStringToCharacterOptions2.emplace("DisplayChessRank", DisplayChessRank_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayChessRank_CharacterOptions2, "DisplayChessRank");
	m_mapStringToCharacterOptions2.emplace("DisplayFishingSkill", DisplayFishingSkill_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayFishingSkill_CharacterOptions2, "DisplayFishingSkill");
	m_mapStringToCharacterOptions2.emplace("DisplayNumberDeaths", DisplayNumberDeaths_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayNumberDeaths_CharacterOptions2, "DisplayNumberDeaths");
	m_mapStringToCharacterOptions2.emplace("DisplayAge", DisplayAge_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayAge_CharacterOptions2, "DisplayAge");
	m_mapStringToCharacterOptions2.emplace("TimeStamp", TimeStamp_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(TimeStamp_CharacterOptions2, "TimeStamp");
	m_mapStringToCharacterOptions2.emplace("SalvageMultiple", SalvageMultiple_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(SalvageMultiple_CharacterOptions2, "SalvageMultiple");
	m_mapStringToCharacterOptions2.emplace("HearGeneralChat", HearGeneralChat_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(HearGeneralChat_CharacterOptions2, "HearGeneralChat");
	m_mapStringToCharacterOptions2.emplace("HearTradeChat", HearTradeChat_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(HearTradeChat_CharacterOptions2, "HearTradeChat");
	m_mapStringToCharacterOptions2.emplace("HearLFGChat", HearLFGChat_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(HearLFGChat_CharacterOptions2, "HearLFGChat");
	m_mapStringToCharacterOptions2.emplace("HearRoleplayChat", HearRoleplayChat_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(HearRoleplayChat_CharacterOptions2, "HearRoleplayChat");
	m_mapStringToCharacterOptions2.emplace("AppearOffline", AppearOffline_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(AppearOffline_CharacterOptions2, "AppearOffline");
	m_mapStringToCharacterOptions2.emplace("DisplayNumberCharacterTitles", DisplayNumberCharacterTitles_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisplayNumberCharacterTitles_CharacterOptions2, "DisplayNumberCharacterTitles");
	m_mapStringToCharacterOptions2.emplace("MainPackPreferred", MainPackPreferred_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(MainPackPreferred_CharacterOptions2, "MainPackPreferred");
	m_mapStringToCharacterOptions2.emplace("LeadMissileTargets", LeadMissileTargets_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(LeadMissileTargets_CharacterOptions2, "LeadMissileTargets");
	m_mapStringToCharacterOptions2.emplace("UseFastMissiles", UseFastMissiles_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(UseFastMissiles_CharacterOptions2, "UseFastMissiles");
	m_mapStringToCharacterOptions2.emplace("FilterLanguage", FilterLanguage_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(FilterLanguage_CharacterOptions2, "FilterLanguage");
	m_mapStringToCharacterOptions2.emplace("ConfirmVolatileRareUse", ConfirmVolatileRareUse_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(ConfirmVolatileRareUse_CharacterOptions2, "ConfirmVolatileRareUse");
	m_mapStringToCharacterOptions2.emplace("HearSocietyChat", HearSocietyChat_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(HearSocietyChat_CharacterOptions2, "HearSocietyChat");
	m_mapStringToCharacterOptions2.emplace("ShowHelm", ShowHelm_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(ShowHelm_CharacterOptions2, "ShowHelm");
	m_mapStringToCharacterOptions2.emplace("DisableDistanceFog", DisableDistanceFog_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(DisableDistanceFog_CharacterOptions2, "DisableDistanceFog");
	m_mapStringToCharacterOptions2.emplace("UseMouseTurning", UseMouseTurning_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(UseMouseTurning_CharacterOptions2, "UseMouseTurning");
	m_mapStringToCharacterOptions2.emplace("ShowCloak", ShowCloak_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(ShowCloak_CharacterOptions2, "ShowCloak");
	m_mapStringToCharacterOptions2.emplace("LockUI", LockUI_CharacterOptions2);
	m_mapCharacterOptions2ToString.emplace(LockUI_CharacterOptions2, "LockUI");

}
