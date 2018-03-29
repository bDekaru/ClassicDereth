
#include "StdAfx.h"
#include "Qualities.h"
#include "BinaryWriter.h"
#include "ExperienceTable.h"
#include "Movement.h"

DEFINE_PACK(PageData)
{
	pWriter->Write<DWORD>(authorID);
	pWriter->WriteString(authorName);
	pWriter->WriteString(authorAccount);

	pWriter->Write<DWORD>(0xFFFF0002);
	pWriter->Write<int>(textIncluded);
	pWriter->Write<int>(ignoreAuthor);

	pWriter->WriteString(pageText);
}

DEFINE_PACK_JSON(PageData)
{
	writer["authorID"] = authorID;
	writer["authorName"] = authorName;
	writer["authorAccount"] = authorAccount;
	writer["ignoreAuthor"] = ignoreAuthor;

	if (!pageText.empty())
		writer["pageText"] = pageText;
	else
		pageText = "";
}

void PageData::PackNoText(BinaryWriter *pWriter)
{
	pWriter->Write<DWORD>(authorID);
	pWriter->WriteString(authorName);
	pWriter->WriteString(authorAccount);

	pWriter->Write<DWORD>(0xFFFF0002);
	pWriter->Write<int>(textIncluded);
	pWriter->Write<int>(ignoreAuthor);
}

DEFINE_UNPACK(PageData)
{
	authorID = pReader->Read<DWORD>();
	authorName = pReader->ReadString();
	authorAccount = pReader->ReadString();

	DWORD version = pReader->Read<DWORD>();

	if ((version >> 16) == 0xFFFF)
	{
		if ((version & 0xFFFF) == 2)
		{
			textIncluded = pReader->Read<int>();
			ignoreAuthor = pReader->Read<int>();
		}
	}
	else
	{
		textIncluded = version;
		ignoreAuthor = 0;
	}

	if (textIncluded)
		pageText = pReader->ReadString();
	else
		pageText = "";

	return true;
}

DEFINE_UNPACK_JSON(PageData)
{
	authorID = reader["authorID"];
	authorName = reader["authorName"];
	authorAccount = reader["authorAccount"];
	ignoreAuthor = reader["ignoreAuthor"];

	if (reader.find("pageText") != reader.end())
	{
		textIncluded = 1;
		pageText = reader["pageText"];
	}
	else
	{
		textIncluded = 0;
		pageText = "";
	}

	return true;
}

void PageDataList::Flush()
{
	numPages = 0;
	pages.clear();
}

DEFINE_PACK(PageDataList)
{
	pWriter->Write<int>(maxNumPages);
	pWriter->Write<int>(maxNumCharsPerPage);
	pWriter->Write<int>(numPages);

	for (auto &i : pages)
		i.Pack(pWriter);
}

DEFINE_UNPACK(PageDataList)
{
	Flush();

	maxNumPages = pReader->Read<int>();
	maxNumCharsPerPage = pReader->Read<int>();
	numPages = pReader->Read<int>();

	for (DWORD i = 0; i < numPages; i++)
	{
		pages.emplace_back();
		pages.back().UnPack(pReader);
	}

	return true;
}

DEFINE_PACK_JSON(PageDataList)
{
	writer["maxNumPages"] = maxNumPages;
	writer["maxNumCharsPerPage"] = maxNumCharsPerPage;

	json pagesEntry;

	for (auto &i : pages)
	{
		json pageEntry;
		i.PackJson(pageEntry);
		pagesEntry.push_back(pageEntry);
	}

	writer["pages"] = pagesEntry;
}

DEFINE_UNPACK_JSON(PageDataList)
{
	Flush();

	maxNumPages = reader["maxNumPages"];
	maxNumCharsPerPage = reader["maxNumCharsPerPage"];

	const json &pagesEntry = reader["pages"];

	numPages = (DWORD) pagesEntry.size();

	for (const json &pageEntry : pagesEntry)
	{
		pages.emplace_back();
		pages.back().UnPackJson(pageEntry);
	}

	return true;
}

DEFINE_PACK(GeneratorProfile)
{
	pWriter->Write<float>(probability);
	pWriter->Write<DWORD>(type);
	pWriter->Write<long double>(delay);
	pWriter->Write<int>(initCreate);
	pWriter->Write<int>(maxNum);
	pWriter->Write<int>(whenCreate);
	pWriter->Write<int>(whereCreate);
	pWriter->Write<int>(stackSize);
	pWriter->Write<unsigned int>(ptid);
	pWriter->Write<float>(shade);
	pos_val.Pack(pWriter);
	pWriter->Write<unsigned int>(slot);
}

DEFINE_UNPACK(GeneratorProfile)
{
	probability = pReader->Read<float>();
	type = pReader->Read<DWORD>();
	delay = pReader->Read<long double>();
	initCreate = pReader->Read<int>();
	maxNum = pReader->Read<int>();
	whenCreate = (RegenerationType)pReader->Read<int>();
	whereCreate = (RegenLocationType)pReader->Read<int>();
	stackSize = pReader->Read<int>();
	ptid = pReader->Read<unsigned int>();
	shade = pReader->Read<float>();
	pos_val.UnPack(pReader);
	slot = pReader->Read<unsigned int>();

	return true;
}


DEFINE_PACK_JSON(GeneratorProfile)
{
	writer["probability"] = probability;
	writer["type"] = type;
	writer["delay"] = delay;
	writer["initCreate"] = initCreate;
	writer["maxNum"] = maxNum;
	writer["whenCreate"] = (int) whenCreate;
	writer["whereCreate"] = (int) whereCreate;
	writer["stackSize"] = stackSize;
	writer["ptid"] = ptid;
	writer["shade"] = shade;
	pos_val.PackJson(writer);
	writer["slot"] = slot;
}

DEFINE_UNPACK_JSON(GeneratorProfile)
{
	probability = reader["probability"];
	type = reader["type"];
	delay = reader["delay"];
	initCreate = reader["initCreate"];
	maxNum = reader["maxNum"];
	whenCreate = (RegenerationType) (int) reader["whenCreate"];
	whereCreate = (RegenLocationType) (int) reader["whereCreate"];
	stackSize = reader["stackSize"];
	ptid = reader["ptid"];
	shade = reader["shade"];
	pos_val.UnPackJson(reader);
	slot = reader["slot"];

	return true;
}

DEFINE_PACK(GeneratorTable)
{
	_profile_list.Pack(pWriter);
}

DEFINE_UNPACK(GeneratorTable)
{
	_profile_list.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(GeneratorTable)
{
	_profile_list.PackJson(writer);
}

DEFINE_UNPACK_JSON(GeneratorTable)
{
	_profile_list.UnPackJson(reader);
	return true;
}

std::list<GeneratorProfile> GeneratorTable::GetInitialGenerationList()
{
	// ... not done
	std::list<GeneratorProfile> gl;

	for (auto &entry : _profile_list)
	{
		if (entry.initCreate)
		{
			gl.push_back(entry);
		}
	}

	return gl;
}

std::list<GeneratorProfile> GeneratorTable::GetGenerationList()
{
	// ... not done
	std::list<GeneratorProfile> gl;

	for (auto &entry : _profile_list)
	{
		gl.push_back(entry);
	}

	return gl;
}

std::list<GeneratorProfile> GeneratorTable::GetDeathGenerationList()
{
	// don't use this no idea if it's right
	std::list<GeneratorProfile> gl;

	for (auto &entry : _profile_list)
	{
		if (!(entry.whenCreate & RegenerationType::Death_RegenerationType))
			continue;

		gl.push_back(entry);
	}

	return gl;
}

DEFINE_PACK(GeneratorRegistryNode)
{
	pWriter->Write<DWORD>(m_wcidOrTtype);
	pWriter->Write<long double>(ts);
	pWriter->Write<int>(m_bTreasureType);
	pWriter->Write<unsigned int>(slot);
	pWriter->Write<int>(checkpointed);
	pWriter->Write<int>(shop);
	pWriter->Write<int>(amount);
}

DEFINE_UNPACK(GeneratorRegistryNode)
{
	m_wcidOrTtype = pReader->Read<DWORD>();
	ts = pReader->Read<long double>();
	m_bTreasureType = pReader->Read<int>();
	slot = pReader->Read<unsigned int>();
	checkpointed = pReader->Read<int>();
	shop = pReader->Read<int>();
	amount = pReader->Read<int>();
	return true;
}

DEFINE_PACK_JSON(GeneratorRegistryNode)
{
	writer["wcidOrTtype"] = m_wcidOrTtype;
	writer["ts"] = ts;
	writer["bTreasureType"] = m_bTreasureType;
	writer["slot"] = slot;
	writer["checkpointed"] = checkpointed;
	writer["shop"] = shop;
	writer["amount"] = amount;
}

DEFINE_UNPACK_JSON(GeneratorRegistryNode)
{
	m_wcidOrTtype = reader["wcidOrTtype"];
	ts = reader["ts"];
	m_bTreasureType = reader["bTreasureType"];
	slot = reader["slot"];
	checkpointed = reader["checkpointed"];
	shop = reader["shop"];
	amount = reader["amount"];
	return true;
}

DEFINE_PACK(GeneratorRegistry)
{
	_registry.Pack(pWriter);
}

DEFINE_UNPACK(GeneratorRegistry)
{
	_registry.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(GeneratorRegistry)
{
	_registry.PackJson(writer);
}

DEFINE_UNPACK_JSON(GeneratorRegistry)
{
	_registry.UnPackJson(reader);
	return true;
}

DEFINE_PACK(GeneratorQueueNode)
{
	pWriter->Write<DWORD>(slot);
	pWriter->Write<double>(when);
}

DEFINE_UNPACK(GeneratorQueueNode)
{
	slot = pReader->Read<DWORD>();
	when = pReader->Read<double>();
	return true;
}

DEFINE_PACK_JSON(GeneratorQueueNode)
{
	writer["slot"] = slot;
	writer["when"] = when;
}

DEFINE_UNPACK_JSON(GeneratorQueueNode)
{
	slot = reader["slot"];
	when = reader["when"];
	return true;
}

DEFINE_PACK(GeneratorQueue)
{
	_queue.Pack(pWriter);
}

DEFINE_UNPACK(GeneratorQueue)
{
	_queue.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(GeneratorQueue)
{
	_queue.PackJson(writer);
}

DEFINE_UNPACK_JSON(GeneratorQueue)
{
	_queue.UnPackJson(reader);
	return true;
}

const char *Emote::EmoteCategoryToName(EmoteCategory category) // custom
{
	switch (category)
	{
#ifndef PUBLIC_BUILD

	case Invalid_EmoteCategory: return "Invalid_EmoteCategory";
	case Refuse_EmoteCategory: return "Refuse_EmoteCategory";
	case Vendor_EmoteCategory: return "Vendor_EmoteCategory";
	case Death_EmoteCategory: return "Death_EmoteCategory";
	case HeartBeat_EmoteCategory: return "HeartBeat_EmoteCategory";
	case Give_EmoteCategory: return "Give_EmoteCategory";
	case Use_EmoteCategory: return "Use_EmoteCategory";
	case Activation_EmoteCategory: return "Activation_EmoteCategory";
	case Generation_EmoteCategory: return "Generation_EmoteCategory";
	case PickUp_EmoteCategory: return "PickUp_EmoteCategory";
	case Drop_EmoteCategory: return "Drop_EmoteCategory";
	case QuestSuccess_EmoteCategory: return "QuestSuccess_EmoteCategory";
	case QuestFailure_EmoteCategory: return "QuestFailure_EmoteCategory";
	case Taunt_EmoteCategory: return "Taunt_EmoteCategory";
	case WoundedTaunt_EmoteCategory: return "WoundedTaunt_EmoteCategory";
	case KillTaunt_EmoteCategory: return "KillTaunt_EmoteCategory";
	case NewEnemy_EmoteCategory: return "NewEnemy_EmoteCategory";
	case Scream_EmoteCategory: return "Scream_EmoteCategory";
	case Homesick_EmoteCategory: return "Homesick_EmoteCategory";
	case ReceiveCritical_EmoteCategory: return "ReceiveCritical_EmoteCategory";
	case ResistSpell_EmoteCategory: return "ResistSpell_EmoteCategory";
	case TestSuccess_EmoteCategory: return "TestSuccess_EmoteCategory";
	case TestFailure_EmoteCategory: return "TestFailure_EmoteCategory";
	case HearChat_EmoteCategory: return "HearChat_EmoteCategory";
	case Wield_EmoteCategory: return "Wield_EmoteCategory";
	case UnWield_EmoteCategory: return "UnWield_EmoteCategory";
	case EventSuccess_EmoteCategory: return "EventSuccess_EmoteCategory";
	case EventFailure_EmoteCategory: return "EventFailure_EmoteCategory";
	case TestNoQuality_EmoteCategory: return "TestNoQuality_EmoteCategory";
	case QuestNoFellow_EmoteCategory: return "QuestNoFellow_EmoteCategory";
	case TestNoFellow_EmoteCategory: return "TestNoFellow_EmoteCategory";
	case GotoSet_EmoteCategory: return "GotoSet_EmoteCategory";
	case NumFellowsSuccess_EmoteCategory: return "NumFellowsSuccess_EmoteCategory";
	case NumFellowsFailure_EmoteCategory: return "NumFellowsFailure_EmoteCategory";
	case NumCharacterTitlesSuccess_EmoteCategory: return "NumCharacterTitlesSuccess_EmoteCategory";
	case NumCharacterTitlesFailure_EmoteCategory: return "NumCharacterTitlesFailure_EmoteCategory";
	case ReceiveLocalSignal_EmoteCategory: return "ReceiveLocalSignal_EmoteCategory";
	case ReceiveTalkDirect_EmoteCategory: return "ReceiveTalkDirect_EmoteCategory";
#endif
	}

	return "Unknown";
}

const char *Emote::EmoteTypeToName(EmoteType type) // custom
{
	switch (type)
	{
#ifndef PUBLIC_BUILD
	case 0: return "Invalid_EmoteType";
	case 1: return "Act_EmoteType";
	case 2: return "AwardXP_EmoteType";
	case 3: return "Give_EmoteType";
	case 4: return "MoveHome_EmoteType";
	case 5: return "Motion_EmoteType";
	case 6: return "Move_EmoteType";
	case 7: return "PhysScript_EmoteType";
	case 8: return "Say_EmoteType";
	case 9: return "Sound_EmoteType";
	case 10: return "Tell_EmoteType";
	case 11: return "Turn_EmoteType";
	case 12: return "TurnToTarget_EmoteType";
	case 13: return "TextDirect_EmoteType";
	case 14: return "CastSpell_EmoteType";
	case 15: return "Activate_EmoteType";
	case 16: return "WorldBroadcast_EmoteType";
	case 17: return "LocalBroadcast_EmoteType";
	case 18: return "DirectBroadcast_EmoteType";
	case 19: return "CastSpellInstant_EmoteType";
	case 20: return "UpdateQuest_EmoteType";
	case 21: return "InqQuest_EmoteType";
	case 22: return "StampQuest_EmoteType";
	case 23: return "StartEvent_EmoteType";
	case 24: return "StopEvent_EmoteType";
	case 25: return "BLog_EmoteType";
	case 26: return "AdminSpam_EmoteType";
	case 27: return "TeachSpell_EmoteType";
	case 28: return "AwardSkillXP_EmoteType";
	case 29: return "AwardSkillPoints_EmoteType";
	case 30: return "InqQuestSolves_EmoteType";
	case 31: return "EraseQuest_EmoteType";
	case 32: return "DecrementQuest_EmoteType";
	case 33: return "IncrementQuest_EmoteType";
	case 34: return "AddCharacterTitle_EmoteType";
	case 35: return "InqBoolStat_EmoteType";
	case 36: return "InqIntStat_EmoteType";
	case 37: return "InqFloatStat_EmoteType";
	case 38: return "InqStringStat_EmoteType";
	case 39: return "InqAttributeStat_EmoteType";
	case 40: return "InqRawAttributeStat_EmoteType";
	case 41: return "InqSecondaryAttributeStat_EmoteType";
	case 42: return "InqRawSecondaryAttributeStat_EmoteType";
	case 43: return "InqSkillStat_EmoteType";
	case 44: return "InqRawSkillStat_EmoteType";
	case 45: return "InqSkillTrained_EmoteType";
	case 46: return "InqSkillSpecialized_EmoteType";
	case 47: return "AwardTrainingCredits_EmoteType";
	case 48: return "InflictVitaePenalty_EmoteType";
	case 49: return "AwardLevelProportionalXP_EmoteType";
	case 50: return "AwardLevelProportionalSkillXP_EmoteType";
	case 51: return "InqEvent_EmoteType";
	case 52: return "ForceMotion_EmoteType";
	case 53: return "SetIntStat_EmoteType";
	case 54: return "IncrementIntStat_EmoteType";
	case 55: return "DecrementIntStat_EmoteType";
	case 56: return "CreateTreasure_EmoteType";
	case 57: return "ResetHomePosition_EmoteType";
	case 58: return "InqFellowQuest_EmoteType";
	case 59: return "InqFellowNum_EmoteType";
	case 60: return "UpdateFellowQuest_EmoteType";
	case 61: return "StampFellowQuest_EmoteType";
	case 62: return "AwardNoShareXP_EmoteType";
	case 63: return "SetSanctuaryPosition_EmoteType";
	case 64: return "TellFellow_EmoteType";
	case 65: return "FellowBroadcast_EmoteType";
	case 66: return "LockFellow_EmoteType";
	case 67: return "Goto_EmoteType";
	case 68: return "PopUp_EmoteType";
	case 69: return "SetBoolStat_EmoteType";
	case 70: return "SetQuestCompletions_EmoteType";
	case 71: return "InqNumCharacterTitles_EmoteType";
	case 72: return "Generate_EmoteType";
	case 73: return "PetCastSpellOnOwner_EmoteType";
	case 74: return "TakeItems_EmoteType";
	case 75: return "InqYesNo_EmoteType";
	case 76: return "InqOwnsItems_EmoteType";
	case 77: return "DeleteSelf_EmoteType";
	case 78: return "KillSelf_EmoteType";
	case 79: return "UpdateMyQuest_EmoteType";
	case 80: return "InqMyQuest_EmoteType";
	case 81: return "StampMyQuest_EmoteType";
	case 82: return "InqMyQuestSolves_EmoteType";
	case 83: return "EraseMyQuest_EmoteType";
	case 84: return "DecrementMyQuest_EmoteType";
	case 85: return "IncrementMyQuest_EmoteType";
	case 86: return "SetMyQuestCompletions_EmoteType";
	case 87: return "MoveToPos_EmoteType";
	case 88: return "LocalSignal_EmoteType";
	case 89: return "InqPackSpace_EmoteType";
	case 90: return "RemoveVitaePenalty_EmoteType";
	case 91: return "SetEyeTexture_EmoteType";
	case 92: return "SetEyePalette_EmoteType";
	case 93: return "SetNoseTexture_EmoteType";
	case 94: return "SetNosePalette_EmoteType";
	case 95: return "SetMouthTexture_EmoteType";
	case 96: return "SetMouthPalette_EmoteType";
	case 97: return "SetHeadObject_EmoteType";
	case 98: return "SetHeadPalette_EmoteType";
	case 99: return "TeleportTarget_EmoteType";
	case 100: return "TeleportSelf_EmoteType";
	case 101: return "StartBarber_EmoteType";
	case 102: return "InqQuestBitsOn_EmoteType";
	case 103: return "InqQuestBitsOff_EmoteType";
	case 104: return "InqMyQuestBitsOn_EmoteType";
	case 105: return "InqMyQuestBitsOff_EmoteType";
	case 106: return "SetQuestBitsOn_EmoteType";
	case 107: return "SetQuestBitsOff_EmoteType";
	case 108: return "SetMyQuestBitsOn_EmoteType";
	case 109: return "SetMyQuestBitsOff_EmoteType";
	case 110: return "UntrainSkill_EmoteType";
	case 111: return "SetAltRacialSkills_EmoteType";
	case 112: return "SpendLuminance_EmoteType";
	case 113: return "AwardLuminance_EmoteType";
	case 114: return "InqInt64Stat_EmoteType";
	case 115: return "SetInt64Stat_EmoteType";
	case 116: return "OpenMe_EmoteType";
	case 117: return "CloseMe_EmoteType";
	case 118: return "SetFloatStat_EmoteType";
	case 119: return "AddContract_EmoteType";
	case 120: return "RemoveContract_EmoteType";
	case 121: return "InqContractsFull_EmoteType";
#else
	default: return "";
#endif
	}

	return "Unknown";
}

DEFINE_PACK(Emote)
{
	pWriter->Write<DWORD>(type);
	pWriter->Write<float>(delay);
	pWriter->Write<float>(extent);

	switch (type)
	{
	case Act_EmoteType: // 1
	case Say_EmoteType: // 8
	case Tell_EmoteType: // 10
	case TextDirect_EmoteType: // 13
	case WorldBroadcast_EmoteType: // 16
	case LocalBroadcast_EmoteType: // 17
	case DirectBroadcast_EmoteType: // 18
	case UpdateQuest_EmoteType: // 20

	case 0x15u:
	case 0x16u:
	case 0x17u:
	case 0x18u:
	case 0x19u:
	case 0x1Au:
	case 0x1Fu:
	case InqEvent_EmoteType:
	case InqFellowQuest_EmoteType:
	case UpdateFellowQuest_EmoteType:
	case 0x3Du:
	case 0x40u:
	case 0x41u:
	case 0x43u:
	case 0x44u:
	case 0x4Fu:
	case 0x50u:
	case 0x51u:
	case 0x53u:
	case 0x58u:
	case 0x79u:
		pWriter->WriteString(msg);
		break;

	case DecrementQuest_EmoteType:
	case IncrementQuest_EmoteType:
	case SetQuestCompletions_EmoteType:
	case DecrementMyQuest_EmoteType:
	case IncrementMyQuest_EmoteType:
	case SetMyQuestCompletions_EmoteType:
	case InqPackSpace_EmoteType:
	case InqQuestBitsOn_EmoteType:
	case InqQuestBitsOff_EmoteType:
	case 0x68u:
	case 0x69u:
	case 0x6Au:
	case 0x6Bu:
	case 0x6Cu:
	case 0x6Du:
		pWriter->WriteString(msg);
		pWriter->Write<DWORD>(amount);
		break;

	case SetIntStat_EmoteType:
	case IncrementIntStat_EmoteType:
	case DecrementIntStat_EmoteType:
	case SetBoolStat_EmoteType:
		pWriter->Write<DWORD>(stat);
		pWriter->Write<DWORD>(amount);
		break;

	case SetInt64Stat_EmoteType:
		pWriter->Write<DWORD>(stat);
		pWriter->Write<DWORD64>(amount64);
		break;

	case SetFloatStat_EmoteType:
		pWriter->Write<DWORD>(stat);
		pWriter->Write<double>(percent);
		break;

	case InqQuestSolves_EmoteType:
	case InqFellowNum_EmoteType:
	case InqNumCharacterTitles_EmoteType:
	case InqMyQuestSolves_EmoteType:
		pWriter->WriteString(msg);
		pWriter->Write<DWORD>(min);
		pWriter->Write<DWORD>(max);
		break;

	case AwardXP_EmoteType:
	case AwardNoShareXP_EmoteType:
		pWriter->Write<DWORD64>(amount64);
		pWriter->Write<DWORD64>(heroxp64);
		break;

	case 0x70:
	case 0x71:
		pWriter->Write<DWORD64>(amount64);
		pWriter->Write<DWORD64>(heroxp64);
		break;

	case 0x22u:
	case 0x2Fu:
	case 0x30u:
	case 0x5Au:
	case 0x77u:
	case 0x78u:
		pWriter->Write<DWORD>(amount);
		break;

	case 0xEu:
	case 0x13u:
	case 0x1Bu:
	case 0x49u:
		pWriter->Write<DWORD>(spellid);
		break;

	case Give_EmoteType: // 3
	case TakeItems_EmoteType: // 74
		cprof.Pack(pWriter);
		break;

	case InqOwnsItems_EmoteType:
		pWriter->WriteString(msg);
		cprof.Pack(pWriter);
		break;

	case CreateTreasure_EmoteType:
		pWriter->Write<DWORD>(wealth_rating);
		pWriter->Write<DWORD>(treasure_class);
		pWriter->Write<DWORD>(treasure_type);
		break;

	case Motion_EmoteType:
	case ForceMotion_EmoteType:
		pWriter->Write<DWORD>(motion);
		break;

	case MoveHome_EmoteType:
	case Move_EmoteType:
	case Turn_EmoteType:
	case MoveToPos_EmoteType:
		frame.Pack(pWriter);
		break;

	case PhysScript_EmoteType:
		pWriter->Write<DWORD>(pscript);
		break;

	case Sound_EmoteType:
		pWriter->Write<DWORD>(sound);
		break;

	case AwardSkillXP_EmoteType:
	case AwardSkillPoints_EmoteType:
		pWriter->Write<DWORD>(amount);
		pWriter->Write<DWORD>(stat);
		break;

	case UntrainSkill_EmoteType:
		pWriter->Write<DWORD>(stat);
		break;

	case SetAltRacialSkills_EmoteType:
		pWriter->Write<DWORD>(amount);
		break;

	case 0x23u:
	case 0x2Du:
	case 0x2Eu:
		pWriter->WriteString(msg);
		pWriter->Write<DWORD>(stat);
		break;

	case InqStringStat_EmoteType:
	case InqYesNo_EmoteType:
		pWriter->WriteString(msg);
		pWriter->WriteString(teststring);
		pWriter->Write<DWORD>(stat);
		break;

	case InqIntStat_EmoteType:
	case InqAttributeStat_EmoteType:
	case InqRawAttributeStat_EmoteType:
	case InqSecondaryAttributeStat_EmoteType:
	case InqRawSecondaryAttributeStat_EmoteType:
	case InqSkillStat_EmoteType:
	case InqRawSkillStat_EmoteType:
		pWriter->WriteString(msg);
		pWriter->Write<DWORD>(min);
		pWriter->Write<DWORD>(max);
		pWriter->Write<DWORD>(stat);
		break;
	case InqInt64Stat_EmoteType:
		pWriter->WriteString(msg);
		pWriter->Write<DWORD64>(min64);
		pWriter->Write<DWORD64>(max64);
		pWriter->Write<DWORD>(stat);
		break;
	case InqFloatStat_EmoteType:
		pWriter->WriteString(msg);
		pWriter->Write<double>(fmin);
		pWriter->Write<double>(fmax);
		pWriter->Write<DWORD>(stat);
		break;
	case AwardLevelProportionalXP_EmoteType:
		pWriter->Write<double>(percent);
		pWriter->Write<DWORD64>(min64);
		pWriter->Write<DWORD64>(max64);
		pWriter->Write<int>(display);
		break;
	case AwardLevelProportionalSkillXP_EmoteType:
		pWriter->Write<DWORD>(stat);
		pWriter->Write<double>(percent);
		pWriter->Write<DWORD>(min);
		pWriter->Write<DWORD>(max);
		pWriter->Write<int>(display);
		break;
	case SetSanctuaryPosition_EmoteType:
		mPosition.Pack(pWriter);
		break;
	case TeleportTarget_EmoteType:
	case TeleportSelf_EmoteType:
		mPosition.Pack(pWriter);
		break;
	}
}

DEFINE_UNPACK(Emote)
{
	type = (EmoteType) pReader->Read<DWORD>();
	delay = pReader->Read<float>();
	extent = pReader->Read<float>();

	if (!type && !delay && !extent)
		DebugBreak();

	switch (type)
	{
	case 1u:
	case 8u:
	case 0xAu:
	case 0xDu:
	case 0x10u:
	case 0x11u:
	case 0x12u:
	case 0x14u:
	case 0x15u:
	case 0x16u:
	case 0x17u:
	case 0x18u:
	case 0x19u:
	case 0x1Au:
	case 0x1Fu:
	case 0x33u:
	case 0x3Au:
	case 0x3Cu:
	case 0x3Du:
	case 0x40u:
	case 0x41u:
	case 0x43u:
	case 0x44u:
	case 0x4Fu:
	case 0x50u:
	case 0x51u:
	case 0x53u:
	case 0x58u:
	case 0x79u:
		msg = pReader->ReadString();
		break;

	case 0x20u:
	case 0x21u:
	case 0x46u:
	case 0x54u:
	case 0x55u:
	case 0x56u:
	case 0x59u:
	case 0x66u:
	case 0x67u:
	case 0x68u:
	case 0x69u:
	case 0x6Au:
	case 0x6Bu:
	case 0x6Cu:
	case 0x6Du:
		msg = pReader->ReadString();
		amount = pReader->Read<DWORD>();
		break;

	case 0x35u:
	case 0x36u:
	case 0x37u:
	case 0x45u:
		stat = pReader->Read<DWORD>();
		amount = pReader->Read<DWORD>();
		break;

	case 0x73:
		stat = pReader->Read<DWORD>();
		amount64 = pReader->Read<DWORD64>();
		break;

	case 0x76:
		stat = pReader->Read<DWORD>();
		percent = pReader->Read<double>();
		break;

	case 0x1Eu:
	case 0x3Bu:
	case 0x47u:
	case 0x52u:
		msg = pReader->ReadString();
		min = pReader->Read<DWORD>();
		max = pReader->Read<DWORD>();
		break;

	case AwardXP_EmoteType: // 2
	case AwardNoShareXP_EmoteType: // 62

#ifdef PRE_TOD_DATA_FILES
		amount = pReader->Read<DWORD>();

		amount64 = amount;
		heroxp64 = 0;
#else
		amount64 = pReader->Read<DWORD64>();
		heroxp64 = pReader->Read<DWORD64>();
#endif

		break;

	case 0x70:
	case 0x71:
		amount64 = pReader->Read<DWORD64>();
		heroxp64 = pReader->Read<DWORD64>();
		break;

	case 0x22u:
#ifdef PRE_TOD_DATA_FILES
		msg = pReader->ReadString(); // was a string in DM, ChangeTitle
#else
		amount = pReader->Read<DWORD>(); // in ToD at some point, an integer AddCharacterTitle
#endif
		break;

	case 0x2Fu:
	case 0x30u:
	case 0x5Au:
	case 0x77u:
	case 0x78u:
		amount = pReader->Read<DWORD>();
		break;

	case 0xEu:
	case 0x13u:
	case 0x1Bu:
	case 0x49u:
		spellid = pReader->Read<DWORD>();
		break;

	case Give_EmoteType:
	case TakeItems_EmoteType:
		cprof.UnPack(pReader);
		break;

	case InqOwnsItems_EmoteType:
		msg = pReader->ReadString();
		cprof.UnPack(pReader);
		break;

	case CreateTreasure_EmoteType:
		wealth_rating = pReader->Read<DWORD>();
		treasure_class = pReader->Read<DWORD>();
		treasure_type = pReader->Read<DWORD>();
		break;

	case Motion_EmoteType:
	case ForceMotion_EmoteType:
		motion = pReader->Read<DWORD>();
		break;

	case 4u:
	case 6u:
	case 0xBu:
	case 0x57u:
		frame.UnPack(pReader);
		break;

	case 7:
		pscript = (PScriptType) pReader->Read<DWORD>();
		break;

	case 9:
		sound = (SoundType) pReader->Read<DWORD>();
		break;

	case 0x1Cu:
	case 0x1Du:
		amount = pReader->Read<DWORD>();
		stat = pReader->Read<DWORD>();
		break;

	case 0x6Eu:
		stat = pReader->Read<DWORD>();
		break;

	case 0x6Fu:
		amount = pReader->Read<DWORD>();
		break;

	case 0x23u:
	case 0x2Du:
	case 0x2Eu:
		msg = pReader->ReadString();
		stat = pReader->Read<DWORD>();
		break;

	case 0x26u:
	case 0x4Bu:
		msg = pReader->ReadString();
		teststring = pReader->ReadString();
		stat = pReader->Read<DWORD>();
		break;

	case 0x24u:
	case 0x27u:
	case 0x28u:
	case 0x29u:
	case 0x2Au:
	case 0x2Bu:
	case 0x2Cu:
		msg = pReader->ReadString();
		min = pReader->Read<DWORD>();
		max = pReader->Read<DWORD>();
		stat = pReader->Read<DWORD>();
		break;
	case 0x72:
		msg = pReader->ReadString();
		min64 = pReader->Read<DWORD64>();
		max64 = pReader->Read<DWORD64>();
		stat = pReader->Read<DWORD>();
		break;
	case 0x25:
		msg = pReader->ReadString();
		fmin = pReader->Read<double>();
		fmax = pReader->Read<double>();
		stat = pReader->Read<DWORD>();
		break;
	case 0x31:
#ifdef PRE_TOD_DATA_FILES
		percent = pReader->Read<double>();
		min64 = min = pReader->Read<DWORD>();
		max64 = max = pReader->Read<DWORD>();
		display = pReader->Read<int>();
#else
		percent = pReader->Read<double>();
		min64 = pReader->Read<DWORD64>();
		max64 = pReader->Read<DWORD64>();
		display = pReader->Read<int>();
#endif
		break;
	case 0x32:
		stat = pReader->Read<DWORD>();
		percent = pReader->Read<double>();
		min = pReader->Read<DWORD>();
		max = pReader->Read<DWORD>();
		display = pReader->Read<int>();
		break;
	case 0x3F:
		mPosition.UnPack(pReader);
		break;
	case 0x63:
	case 0x64:
		mPosition.UnPack(pReader);
		break;
	}

	return true; // Emote::IsValid()
}

DEFINE_PACK_JSON(Emote)
{
	writer["type"] = (int) type;
	writer["delay"] = delay;
	writer["extent"] = extent;

	switch (type)
	{
	case 1u:
	case 8u:
	case 0xAu:
	case 0xDu:
	case 0x10u:
	case 0x11u:
	case 0x12u:
	case 0x14u:
	case 0x15u:
	case 0x16u:
	case 0x17u:
	case 0x18u:
	case 0x19u:
	case 0x1Au:
	case 0x1Fu:
	case 0x33u:
	case 0x3Au:
	case 0x3Cu:
	case 0x3Du:
	case 0x40u:
	case 0x41u:
	case 0x43u:
	case 0x44u:
	case 0x4Fu:
	case 0x50u:
	case 0x51u:
	case 0x53u:
	case 0x58u:
	case 0x79u:
		writer["msg"] = msg;
		break;

	case 0x20u:
	case 0x21u:
	case 0x46u:
	case 0x54u:
	case 0x55u:
	case 0x56u:
	case 0x59u:
	case 0x66u:
	case 0x67u:
	case 0x68u:
	case 0x69u:
	case 0x6Au:
	case 0x6Bu:
	case 0x6Cu:
	case 0x6Du:
		writer["msg"] = msg;
		writer["amount"] = amount;
		break;

	case 0x35u:
	case 0x36u:
	case 0x37u:
	case 0x45u:
		writer["stat"] = stat;
		writer["amount"] = amount;
		break;

	case 0x73:
		writer["stat"] = stat;;
		writer["amount64"] = amount64;
		break;

	case 0x76:
		writer["stat"] = stat;
		writer["percent"] = percent;
		break;

	case 0x1Eu:
	case 0x3Bu:
	case 0x47u:
	case 0x52u:
		writer["msg"] = msg;
		writer["min"] = min;
		writer["max"] = max;
		break;

	case AwardXP_EmoteType: // 2
	case AwardNoShareXP_EmoteType: // 62
		writer["amount64"] = amount64;
		writer["heroxp64"] = heroxp64;
		break;

	case 0x70:
	case 0x71:
		writer["amount64"] = amount64;
		writer["heroxp64"] = heroxp64;
		break;

	case 0x22u:
		writer["amount"] = amount; // in ToD at some point, an integer AddCharacterTitle
		break;

	case 0x2Fu:
	case 0x30u:
	case 0x5Au:
	case 0x77u:
	case 0x78u:
		writer["amount"] = amount;
		break;

	case 0xEu:
	case 0x13u:
	case 0x1Bu:
	case 0x49u:
		writer["spellid"] = spellid;
		break;

	case Give_EmoteType:
	case TakeItems_EmoteType:
		cprof.PackJson(writer["cprof"]);
		break;

	case InqOwnsItems_EmoteType:
		writer["msg"] = msg;
		cprof.PackJson(writer["cprof"]);
		break;

	case CreateTreasure_EmoteType:
		writer["wealth_rating"] = wealth_rating;
		writer["treasure_class"] = treasure_class;
		writer["treasure_type"] = treasure_type;
		break;

	case Motion_EmoteType:
	case ForceMotion_EmoteType:
		writer["motion"] = motion;
		break;

	case 4u:
	case 6u:
	case 0xBu:
	case 0x57u:
		frame.PackJson(writer["frame"]);
		break;

	case 7:
		writer["pscript"] = (int) pscript;
		break;

	case 9:
		writer["sound"] = (INT) sound;
		break;

	case 0x1Cu:
	case 0x1Du:
		writer["amount"] = amount;
		writer["stat"] = stat;
		break;

	case 0x6Eu:
		writer["stat"] = stat;
		break;

	case 0x6Fu:
		writer["amount"] = amount;
		break;

	case 0x23u:
	case 0x2Du:
	case 0x2Eu:
		writer["msg"] = msg;
		writer["stat"] = stat;
		break;

	case 0x26u:
	case 0x4Bu:
		writer["msg"] = msg;
		writer["teststring"] = teststring;
		writer["stat"] = stat;
		break;

	case 0x24u:
	case 0x27u:
	case 0x28u:
	case 0x29u:
	case 0x2Au:
	case 0x2Bu:
	case 0x2Cu:
		writer["msg"] = msg;
		writer["min"] = min;
		writer["max"] = max;
		writer["stat"] = stat;
		break;
	case 0x72:
		writer["msg"] = msg;
		writer["min64"] = min64;
		writer["max64"] = max64;
		writer["stat"] = stat;
		break;
	case 0x25:
		writer["msg"] = msg;
		writer["fmin"] = fmin;
		writer["fmax"] = fmax;
		writer["stat"] = stat;
		break;
	case 0x31:
		writer["percent"] = percent;
		writer["min64"] = min64;
		writer["max64"] = max64;
		writer["display"] = display;
		break;
	case 0x32:
		writer["stat"] = stat;
		writer["percent"] = percent;
		writer["min"] = min;
		writer["max"] = max;
		writer["display"] = display;
		break;
	case 0x3F:
		mPosition.PackJson(writer["mPosition"]);
		break;
	case 0x63:
	case 0x64:
		mPosition.PackJson(writer["mPosition"]);
		break;
	}
}

DEFINE_UNPACK_JSON(Emote)
{
	type = (EmoteType) (int) reader["type"];
	delay = reader["delay"];
	extent = reader["extent"];

	//if (!type && !delay && !extent)
	//	DebugBreak();

	switch (type)
	{
	case 1u:
	case 8u:
	case 0xAu:
	case 0xDu:
	case 0x10u:
	case 0x11u:
	case 0x12u:
	case 0x14u:
	case 0x15u:
	case 0x16u:
	case 0x17u:
	case 0x18u:
	case 0x19u:
	case 0x1Au:
	case 0x1Fu:
	case 0x33u:
	case 0x3Au:
	case 0x3Cu:
	case 0x3Du:
	case 0x40u:
	case 0x41u:
	case 0x43u:
	case 0x44u:
	case 0x4Fu:
	case 0x50u:
	case 0x51u:
	case 0x53u:
	case 0x58u:
	case 0x79u:
		msg = reader["msg"];
		break;

	case 0x20u:
	case 0x21u:
	case 0x46u:
	case 0x54u:
	case 0x55u:
	case 0x56u:
	case 0x59u:
	case 0x66u:
	case 0x67u:
	case 0x68u:
	case 0x69u:
	case 0x6Au:
	case 0x6Bu:
	case 0x6Cu:
	case 0x6Du:
		msg = reader["msg"];
		amount = reader["amount"];
		break;

	case 0x35u:
	case 0x36u:
	case 0x37u:
	case 0x45u:
		stat = reader["stat"];
		amount = reader["amount"];
		break;

	case 0x73:
		stat = reader["stat"];;
		amount64 = reader["amount64"];
		break;

	case 0x76:
		stat = reader["stat"];
		percent = reader["percent"];
		break;

	case 0x1Eu:
	case 0x3Bu:
	case 0x47u:
	case 0x52u:
		msg = reader["msg"];
		min = reader["min"];
		max = reader["max"];
		break;

	case AwardXP_EmoteType: // 2
	case AwardNoShareXP_EmoteType: // 62
		amount64 = reader["amount64"];
		heroxp64 = reader["heroxp64"];
		break;

	case 0x70:
	case 0x71:
		amount64 = reader["amount64"];
		heroxp64 = reader["heroxp64"];
		break;

	case 0x22u:
		amount = reader["amount"]; // in ToD at some point, an integer AddCharacterTitle
		break;

	case 0x2Fu:
	case 0x30u:
	case 0x5Au:
	case 0x77u:
	case 0x78u:
		amount = reader["amount"];
		break;

	case 0xEu:
	case 0x13u:
	case 0x1Bu:
	case 0x49u:
		spellid = reader["spellid"];
		break;

	case Give_EmoteType:
	case TakeItems_EmoteType:
		cprof.UnPackJson(reader["cprof"]);
		break;

	case InqOwnsItems_EmoteType:
		msg = reader["msg"];
		cprof.UnPackJson(reader["cprof"]);
		break;

	case CreateTreasure_EmoteType:
		wealth_rating = reader["wealth_rating"];
		treasure_class = reader["treasure_class"];
		treasure_type = reader["treasure_type"];
		break;

	case Motion_EmoteType:
	case ForceMotion_EmoteType:
		motion = reader["motion"];
		break;

	case 4u:
	case 6u:
	case 0xBu:
	case 0x57u:
		frame.UnPackJson(reader["frame"]);
		break;

	case 7:
		pscript = (PScriptType)(int) reader["pscript"];
		break;

	case 9:
		sound = (SoundType)(int)reader["sound"];
		break;

	case 0x1Cu:
	case 0x1Du:
		amount = reader["amount"];
		stat = reader["stat"];
		break;

	case 0x6Eu:
		stat = reader["stat"];
		break;

	case 0x6Fu:
		amount = reader["amount"];
		break;

	case 0x23u:
	case 0x2Du:
	case 0x2Eu:
		msg = reader["msg"];
		stat = reader["stat"];
		break;

	case 0x26u:
	case 0x4Bu:
		msg = reader["msg"];
		teststring = reader["teststring"];
		stat = reader["stat"];
		break;

	case 0x24u:
	case 0x27u:
	case 0x28u:
	case 0x29u:
	case 0x2Au:
	case 0x2Bu:
	case 0x2Cu:
		msg = reader["msg"];
		min = reader["min"];
		max = reader["max"];
		stat = reader["stat"];
		break;
	case 0x72:
		msg = reader["msg"];
		min64 = reader["min64"];
		max64 = reader["max64"];
		stat = reader["stat"];
		break;
	case 0x25:
		msg = reader["msg"];
		fmin = reader["fmin"];
		fmax = reader["fmax"];
		stat = reader["stat"];
		break;
	case 0x31:
		percent = reader["percent"];
		min64 = reader["min64"];
		max64 = reader["max64"];
		display = reader["display"];
		break;
	case 0x32:
		stat = reader["stat"];
		percent = reader["percent"];
		min = reader["min"];
		max = reader["max"];
		display = reader["display"];
		break;
	case 0x3F:
		mPosition.UnPackJson(reader["mPosition"]);
		break;
	case 0x63:
	case 0x64:
		mPosition.UnPackJson(reader["mPosition"]);
		break;
	}

	return true; // Emote::IsValid()
}

DEFINE_PACK(EmoteSet)
{
	pWriter->Write<DWORD>(category);
	pWriter->Write<float>(probability);

	switch (category)
	{
	case Refuse_EmoteCategory:
	case Give_EmoteCategory:
		pWriter->Write<DWORD>(classID);
		break;

	case HeartBeat_EmoteCategory:
		pWriter->Write<DWORD>(style);
		pWriter->Write<DWORD>(substyle);
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
		pWriter->WriteString(quest);
		break;

	case Vendor_EmoteCategory:
		// 1 = open vendor
		// 2 = walk away
		// 3 = sell
		// 4 = buy
		// 5 = performs a motion 0x87 or 0x7d or 0x86 or 0x83
		pWriter->Write<DWORD>(vendorType);
		break;

	case WoundedTaunt_EmoteCategory:
		pWriter->Write<float>(minhealth);
		pWriter->Write<float>(maxhealth);
		break;
	}

	emotes.Pack(pWriter);
}

DEFINE_UNPACK(EmoteSet)
{
	category = (EmoteCategory) pReader->Read<DWORD>();
	probability = pReader->Read<float>();

	switch (category)
	{
	case Refuse_EmoteCategory:
	case Give_EmoteCategory:
		classID = pReader->Read<DWORD>();
		break;

	case HeartBeat_EmoteCategory:
		style = pReader->Read<DWORD>();
		substyle = pReader->Read<DWORD>();
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
		quest = pReader->ReadString();
		break;

	case Vendor_EmoteCategory:
		vendorType = pReader->Read<DWORD>();
		break;

	case WoundedTaunt_EmoteCategory:
		minhealth = pReader->Read<float>();
		maxhealth = pReader->Read<float>();
		break;
	}

	emotes.UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(EmoteSet)
{
	writer["category"] = (int) category;
	writer["probability"] = probability;

	switch (category)
	{
	case Refuse_EmoteCategory:
	case Give_EmoteCategory:
		writer["classID"] = classID;
		break;

	case HeartBeat_EmoteCategory:
		writer["style"] = style;
		writer["substyle"] = substyle;
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
		writer["quest"] = quest;
		break;

	case Vendor_EmoteCategory:
		// 1 = open vendor
		// 2 = walk away
		// 3 = sell
		// 4 = buy
		// 5 = performs a motion 0x87 or 0x7d or 0x86 or 0x83
		writer["vendorType"] = vendorType;
		break;

	case WoundedTaunt_EmoteCategory:
		writer["minhealth"] = minhealth;
		writer["maxhealth"] = maxhealth;
		break;
	}

	emotes.PackJson(writer["emotes"]);
}

DEFINE_UNPACK_JSON(EmoteSet)
{
	category = (EmoteCategory) (int) reader["category"];
	probability = reader["probability"];

	switch (category)
	{
	case Refuse_EmoteCategory:
	case Give_EmoteCategory:
		classID = reader["classID"];
		break;

	case HeartBeat_EmoteCategory:
		style = reader["style"];
		substyle = reader["substyle"];
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
		quest = reader["quest"];
		break;

	case Vendor_EmoteCategory:
		vendorType = reader["vendorType"];
		break;

	case WoundedTaunt_EmoteCategory:
		minhealth = reader["minhealth"];
		maxhealth = reader["maxhealth"];
		break;
	}

	emotes.UnPackJson(reader["emotes"]);

	return true;
}

DEFINE_PACK(CEmoteTable)
{
	_emote_table.Pack(pWriter);
}

DEFINE_UNPACK(CEmoteTable)
{
	_emote_table.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(CEmoteTable)
{
	_emote_table.PackJson(writer);
}

DEFINE_UNPACK_JSON(CEmoteTable)
{
	_emote_table.UnPackJson(reader);
	return true;
}

EventFilter::EventFilter()
{
}

EventFilter::~EventFilter()
{
	SafeDeleteArray(event_filter);
}

DEFINE_PACK(EventFilter)
{
	pWriter->Write<unsigned int>(num_events);
	for (DWORD i = 0; i < num_events; i++)
		pWriter->Write<unsigned int>(event_filter[i]);
}

DEFINE_UNPACK(EventFilter)
{
	SafeDeleteArray(event_filter);

	num_events = pReader->Read<unsigned int>();

	event_filter = new unsigned int[num_events];
	for (DWORD i = 0; i < num_events; i++)
		event_filter[i] = pReader->Read<unsigned int>();

	return true;
}

DEFINE_PACK_JSON(EventFilter)
{
	json events;
	for (DWORD i = 0; i < num_events; i++)
	{
		events.push_back(event_filter[i]);
	}

	writer["events"] = events;
}

DEFINE_UNPACK_JSON(EventFilter)
{
	SafeDeleteArray(event_filter);

	const json &events = reader["events"];

	num_events = (unsigned int) events.size();

	event_filter = new unsigned int[num_events];
	for (DWORD i = 0; i < num_events; i++)
		event_filter[i] = events.at(i);

	return true;
}

DEFINE_PACK(StatMod)
{
	pWriter->Write<unsigned int>(type);
	pWriter->Write<unsigned int>(key);
	pWriter->Write<float>(val);
}

DEFINE_UNPACK(StatMod)
{
	type = pReader->Read<unsigned int>();
	key = pReader->Read<unsigned int>();
	val = pReader->Read<float>();
	return true;
}

DEFINE_PACK_JSON(StatMod)
{
	writer["type"] = type;
	writer["key"] = key;
	writer["val"] = val;
}

DEFINE_UNPACK_JSON(StatMod)
{
	type = reader["type"];
	key = reader["key"];
	val = reader["val"];
	return true;
}

DEFINE_PACK(Enchantment)
{
	pWriter->Write<unsigned int>(_id);
	pWriter->Write<unsigned int>((_spell_category & 0xFFFF) | 0x10000);
	pWriter->Write<int>(_power_level);
	pWriter->Write<long double>(_start_time - Timer::cur_time);
	pWriter->Write<long double>(_duration);
	pWriter->Write<unsigned int>(_caster);
	pWriter->Write<float>(_degrade_modifier);
	pWriter->Write<float>(_degrade_limit);
	pWriter->Write<long double>(_last_time_degraded - Timer::cur_time);
	_smod.Pack(pWriter);
	pWriter->Write<unsigned int>(m_SpellSetID);
}

DEFINE_UNPACK(Enchantment)
{
	_id = pReader->Read<unsigned int>();

	DWORD spellCategory = pReader->Read<unsigned int>();
	_spell_category = spellCategory & 0xFFFF;
	_power_level = pReader->Read<int>();
	_start_time = pReader->Read<long double>() + Timer::cur_time;
	_duration = pReader->Read<long double>();
	_caster = pReader->Read<unsigned int>();
	_degrade_modifier = pReader->Read<float>();
	_degrade_limit = pReader->Read<float>();
	_last_time_degraded = pReader->Read<long double>() + Timer::cur_time;
	_smod.UnPack(pReader);

	if (spellCategory >> 16)
		m_SpellSetID = pReader->Read<unsigned int>();

	return true;
}


DEFINE_PACK_JSON(Enchantment)
{
	writer["id"] = _id;
	writer["spell_category"] = _spell_category;
	writer["power_level"] = _power_level;
	writer["start_time"] = _start_time - Timer::cur_time;
	writer["duration"] = _duration;
	writer["caster"] = _caster;
	writer["degrade_modifier"] = _degrade_modifier;
	writer["degrade_limit"] = _degrade_limit;
	writer["last_time_degraded"] = _last_time_degraded - Timer::cur_time;
	_smod.PackJson(writer["smod"]);
	writer["spellset_id"] = m_SpellSetID;
}

DEFINE_UNPACK_JSON(Enchantment)
{
	_id = reader["id"];
	_spell_category = reader["spell_category"];
	_power_level = reader["power_level"];
	_start_time = reader["start_time"] + Timer::cur_time;
	_duration = reader["duration"];
	_caster = reader["caster"];
	_degrade_modifier = reader["degrade_modifier"];
	_degrade_limit = reader["degrade_limit"];
	_last_time_degraded = reader["last_time_degraded"] + Timer::cur_time;
	_smod.UnPackJson(reader["smod"]);
	m_SpellSetID = reader["spellset_id"];

	return true;
}

BOOL Enchantment::Enchant(float *value)
{
	if (!(_smod.type & 0x8000))
	{
		if (_smod.type & 0x4000)
		{
			*value = _smod.val * *value;
			return TRUE;
		}
	}
	else
	{
		*value = this->_smod.val + *value;
		return TRUE;
	}

	return FALSE;
}

BOOL Enchantment::HasExpired()
{
	if (_duration < 0.0)
		return FALSE;
	if ((_start_time + _duration) > Timer::cur_time)
		return FALSE;

	return TRUE;
}

CEnchantmentRegistry::CEnchantmentRegistry()
{
}

CEnchantmentRegistry::~CEnchantmentRegistry()
{
	Clear();
}

void CEnchantmentRegistry::Clear()
{
	SafeDelete(_vitae);
	SafeDelete(_mult_list);
	SafeDelete(_add_list);
	SafeDelete(_cooldown_list);
	m_cHelpfulEnchantments = 0;
	m_cHarmfulEnchantments = 0;
}

DEFINE_PACK(CEnchantmentRegistry)
{
	DWORD header = 0;

	if (_mult_list)
		header |= 1;
	if (_add_list)
		header |= 2;
	if (_cooldown_list)
		header |= 8;
	if (_vitae)
		header |= 4;

	pWriter->Write<DWORD>(header);

	if (_mult_list)
		_mult_list->Pack(pWriter);
	if (_add_list)
		_add_list->Pack(pWriter);
	if (_cooldown_list)
		_cooldown_list->Pack(pWriter);
	if (_vitae)
		_vitae->Pack(pWriter);
}

DEFINE_UNPACK(CEnchantmentRegistry)
{
	DWORD header = pReader->Read<DWORD>();

	if (header & 1)
	{
		if (!_mult_list)
			_mult_list = new PackableListWithJson<Enchantment>();

		_mult_list->UnPack(pReader);
	}
	else
	{
		SafeDelete(_mult_list);
	}


	if (header & 2)
	{
		if (!_add_list)
			_add_list = new PackableListWithJson<Enchantment>();

		_add_list->UnPack(pReader);
	}
	else
	{
		SafeDelete(_add_list);
	}

	if (header & 8)
	{
		if (!_cooldown_list)
			_cooldown_list = new PackableListWithJson<Enchantment>();

		_cooldown_list->UnPack(pReader);
	}
	else
	{
		SafeDelete(_cooldown_list);
	}

	if (header & 4)
	{
		if (!_vitae)
			_vitae = new Enchantment();

		_vitae->UnPack(pReader);
	}
	else
	{
		SafeDelete(_vitae);
	}

	m_cHelpfulEnchantments = 0;
	m_cHarmfulEnchantments = 0;
	CountSpellsInList(_mult_list);
	CountSpellsInList(_add_list);

	return true;
}

DEFINE_PACK_JSON(CEnchantmentRegistry)
{
	if (_mult_list)
		_mult_list->PackJson(writer["mult_list"]);
	if (_add_list)
		_add_list->PackJson(writer["add_list"]);
	if (_cooldown_list)
		_cooldown_list->PackJson(writer["cooldown_list"]);
	if (_vitae)
		_vitae->PackJson(writer["vitae"]);
}

DEFINE_UNPACK_JSON(CEnchantmentRegistry)
{
	if (reader.find("mult_list") != reader.end())
	{
		if (!_mult_list)
			_mult_list = new PackableListWithJson<Enchantment>();

		_mult_list->UnPackJson(reader["mult_list"]);
	}
	else
	{
		SafeDelete(_mult_list);
	}

	if (reader.find("add_list") != reader.end())
	{
		if (!_add_list)
			_add_list = new PackableListWithJson<Enchantment>();

		_add_list->UnPackJson(reader["add_list"]);
	}
	else
	{
		SafeDelete(_add_list);
	}

	if (reader.find("cooldown_list") != reader.end())
	{
		if (!_cooldown_list)
			_cooldown_list = new PackableListWithJson<Enchantment>();

		_cooldown_list->UnPackJson(reader["cooldown_list"]);
	}
	else
	{
		SafeDelete(_cooldown_list);
	}

	if (reader.find("vitae") != reader.end())
	{
		if (!_vitae)
			_vitae = new Enchantment();

		_vitae->UnPackJson(reader["vitae"]);
	}
	else
	{
		SafeDelete(_vitae);
	}

	m_cHelpfulEnchantments = 0;
	m_cHarmfulEnchantments = 0;
	CountSpellsInList(_mult_list);
	CountSpellsInList(_add_list);

	return true;
}

BOOL CEnchantmentRegistry::EnchantAttribute(unsigned int stype, unsigned int *val)
{
	float new_value = *val;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, 1, stype, &affecting);
	CullEnchantmentsFromList(_add_list, 1, stype, &affecting);

	BOOL bEnchanted = FALSE;

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	if (*val < 10)
	{
		if (new_value < 1.0f)
			new_value = 1.0f;
	}
	else if (new_value < 10.0f)
		new_value = 10.0f;

	*val = (unsigned int)(new_value + 0.5);
	return bEnchanted;
}

BOOL CEnchantmentRegistry::Enchant(PackableListWithJson<Enchantment> *affecting, float *new_value)
{
	BOOL bEnchanted = FALSE;
	for (PackableListWithJson<Enchantment>::iterator i = affecting->begin(); i != affecting->end(); i++)
	{
		if (i->Enchant(new_value))
			bEnchanted = TRUE;
	}

	return bEnchanted;
}

BOOL CEnchantmentRegistry::EnchantAttribute2nd(unsigned int stype, unsigned int *val)
{
	ACQualityFilter *filter = CachedEnchantableFilter; // Enchantable
	if (!filter)
		return FALSE;
	if (!filter->QueryAttribute2nd(stype))
		return FALSE;

	float new_value = *val;

	BOOL bEnchanted = FALSE;

	if (_vitae && _vitae->Enchant(&new_value))
		bEnchanted = TRUE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, 2, stype, &affecting);
	CullEnchantmentsFromList(_add_list, 2, stype, &affecting);

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	if (*val < 5)
	{
		if (new_value < 1.0f)
			new_value = 1.0f;
	}
	else if (new_value < 5.0f)
		new_value = 5.0f;

	*val = (unsigned int)(new_value + 0.5);
	return bEnchanted;
}

BOOL CEnchantmentRegistry::EnchantSkill(unsigned int stype, int *val)
{
	float new_value = *val;

	BOOL bEnchanted = FALSE;

	if (_vitae && _vitae->Enchant(&new_value))
		bEnchanted = TRUE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, Skill_EnchantmentType, stype, &affecting);
	CullEnchantmentsFromList(_add_list, Skill_EnchantmentType, stype, &affecting);

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	if (new_value <= 0.5)
		new_value = 0.0;

	*val = (int)(new_value + 0.5);
	return bEnchanted;
}

BOOL CEnchantmentRegistry::EnchantInt(unsigned int stype, int *val, BOOL allow_negative)
{
	ACQualityFilter *filter = CachedEnchantableFilter; // Enchantable
	if (!filter)
		return FALSE;
	if (!filter->QueryInt(stype))
		return FALSE;

	float new_value = *val;

	BOOL bEnchanted = FALSE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, Int_EnchantmentType, stype, &affecting);
	CullEnchantmentsFromList(_add_list, Int_EnchantmentType, stype, &affecting);

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	if (!allow_negative)
	{
		if (new_value <= 0.5f)
			new_value = 0.0f;

		new_value += 0.5f;
	}

	*val = (int)new_value;
	return bEnchanted;
}

BOOL CEnchantmentRegistry::EnchantFloat(unsigned int stype, double *val)
{
	ACQualityFilter *filter = CachedEnchantableFilter; // Enchantable
	if (!filter)
		return FALSE;
	if (!filter->QueryFloat(stype))
		return FALSE;

	float new_value = *val;

	BOOL bEnchanted = FALSE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, Float_EnchantmentType, stype, &affecting);
	CullEnchantmentsFromList(_add_list, Float_EnchantmentType, stype, &affecting);

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	*val = new_value;
	return bEnchanted;
}

BOOL CEnchantmentRegistry::EnchantBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int *val)
{
	float new_value = *val;

	BOOL bEnchanted = FALSE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, BodyArmorValue_EnchantmentType, dt, &affecting);
	CullEnchantmentsFromList(_add_list, BodyArmorValue_EnchantmentType, dt, &affecting);

	if (Enchant(&affecting, &new_value))
		bEnchanted = TRUE;

	*val = (int)new_value;
	return bEnchanted;
}

BOOL CEnchantmentRegistry::UpdateSpellTotals(unsigned int spell, int iDelta)
{
	if (spell >= 0x8000)
		return TRUE;

	CSpellTable *pSpellTable = MagicSystem::GetSpellTable();

	const CSpellBase *pSpellBase = pSpellTable->GetSpellBase(spell);
	if (!pSpellBase)
		return FALSE;

	if (pSpellBase->_bitfield & 4)
		m_cHelpfulEnchantments += iDelta;
	else
		m_cHarmfulEnchantments += iDelta;

	return TRUE;
}

void CEnchantmentRegistry::CountSpellsInList(PackableListWithJson<Enchantment> *list)
{
	if (list)
	{
		for (auto &entry : *list)
			UpdateSpellTotals(entry._id & 0xFFFF, 1);
	}
}

BOOL CEnchantmentRegistry::IsEnchantmentInList(const unsigned int spell, PackableListWithJson<Enchantment> *list)
{
	if (list)
	{
		for (auto &entry : *list)
		{
			if ((entry._id & 0xFFFF) == spell)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CEnchantmentRegistry::IsEnchanted(const unsigned int spell)
{
	if (!spell)
		return FALSE;

	if (spell == 666)
		return _vitae != NULL;

	if (CEnchantmentRegistry::IsEnchantmentInList(spell, _cooldown_list))
		return TRUE;	
	if (CEnchantmentRegistry::IsEnchantmentInList(spell, _mult_list))
		return TRUE;
	if (CEnchantmentRegistry::IsEnchantmentInList(spell, _add_list))
		return TRUE;

	return FALSE;
}

BOOL CEnchantmentRegistry::AttemptToReplaceSpellInList(Enchantment *spell, PackableListWithJson<Enchantment> **list)
{
	if (*list && spell->_duration > 0.0)
	{
		for (PackableListWithJson<Enchantment>::iterator i = (*list)->begin(); i != (*list)->end(); i++)
		{
			if (!((spell->_id ^ i->_id) & 0xFFFF) && i->_duration > 0.0)
			{
				*i = *spell;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CEnchantmentRegistry::AddEnchantmentToList(Enchantment *to_update, PackableListWithJson<Enchantment> **list)
{
	if (!*list)
		*list = new PackableListWithJson<Enchantment>();
	
	if (!AttemptToReplaceSpellInList(to_update, list))
	{
		(*list)->push_front(*to_update);
		UpdateSpellTotals(to_update->_id & 0xFFFF, 1);
	}

	return TRUE;
}

BOOL CEnchantmentRegistry::ReplaceEnchantmentInList(Enchantment *new_guy, PackableListWithJson<Enchantment> *list)
{
	BOOL found = FALSE;

	if (list)
	{
		for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
		{
			if (i->_id == new_guy->_id)
			{
				*i = *new_guy;
				found = TRUE;
			}
		}
	}

	return found;
}

BOOL CEnchantmentRegistry::UpdateVitae(Enchantment *vitae)
{
	if (vitae->_smod.type & 0x800000)
	{
		if (!_vitae)
			_vitae = new Enchantment(*vitae);

		*_vitae = *vitae;
		return TRUE;
	}

	return FALSE;
}

BOOL CEnchantmentRegistry::UpdateEnchantment(Enchantment *to_update)
{
	DWORD what = to_update->_smod.type ^ ((to_update->_smod.type ^ (to_update->_smod.type >> 9)) >> 1);
	if (!(what & 0x4000))
		return FALSE;

	if (to_update->_smod.type & Vitae_EnchantmentType)
		return UpdateVitae(to_update);

	if (to_update->_smod.type & Cooldown_EnchantmentType && ReplaceEnchantmentInList(to_update, _cooldown_list))
		return TRUE;
	
	if (to_update->_smod.type & Multiplicative_EnchantmentType)
	{
		if (ReplaceEnchantmentInList(to_update, _mult_list))
			return TRUE;
	}
	else if (to_update->_smod.type & Additive_EnchantmentType && ReplaceEnchantmentInList(to_update, _add_list))
	{
		return TRUE;
	}

	if (to_update->_smod.type & Cooldown_EnchantmentType)
	{
		AddEnchantmentToList(to_update, &_cooldown_list);
		return TRUE;
	}
	if (to_update->_smod.type & Multiplicative_EnchantmentType)
	{
		AddEnchantmentToList(to_update, &_mult_list);
		return TRUE;
	}
	if (to_update->_smod.type & Additive_EnchantmentType)
		AddEnchantmentToList(to_update, &_add_list);

	return TRUE;
}

BOOL CEnchantmentRegistry::RemoveEnchantmentFromList(const unsigned int eid, PackableListWithJson<Enchantment> *list)
{
	BOOL bRemoved = FALSE;

	if (list)
	{
		for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
		{
			if (i->_id == eid)
			{
				list->erase(i);
				bRemoved = TRUE;
				break;
			}
		}

		if (bRemoved)
			UpdateSpellTotals((WORD) eid, -1);
	}

	return bRemoved;
}

BOOL CEnchantmentRegistry::RemoveEnchantment(const unsigned int eid)
{
	if (_vitae && _vitae->_id == eid)
	{
		delete _vitae;
		_vitae = NULL;
		return TRUE;
	}

	if (RemoveEnchantmentFromList(eid, _cooldown_list) || RemoveEnchantmentFromList(eid, _mult_list) || RemoveEnchantmentFromList(eid, _add_list))
		return TRUE;

	return FALSE;
}

BOOL Enchantment::AffectsAttackSkills(unsigned int key)
{
	if (_smod.type & 0x10000)
	{
		switch (key)
		{
		case 0x21u:
		case 0x22u:
		case 0x29u:
		case 0x2Bu:
		case 0x2Cu:
		case 0x2Du:
		case 0x2Eu:
		case 0x2Fu:
		case 0x31u:
			return TRUE;
		}
	}

	return FALSE;
}

BOOL Enchantment::AffectsDefenseSkills(unsigned int key)
{
	if (_smod.type & 0x20000)
	{
		switch (key)
		{
		case 6u:
		case 7u:
		case 0xFu:
		case 0x30u:
			return TRUE;
		}
	}

	return FALSE;
}

BOOL Enchantment::Duel(Enchantment *challenger)
{
	return _power_level > challenger->_power_level || (_power_level == challenger->_power_level && challenger->_start_time < _start_time);
}

BOOL CEnchantmentRegistry::CullEnchantmentsFromList(PackableListWithJson<Enchantment> *list, const unsigned int type, const unsigned int key, PackableListWithJson<Enchantment> *affecting)
{
	BOOL culled = FALSE;

	if (list)
	{
		for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
		{
			if (i->_smod.type & type &&
				(i->_smod.type & 0x20 || i->_smod.key == key || i->AffectsAttackSkills(key) || i->AffectsDefenseSkills(key)))
			{
				culled = Duel((Enchantment *)&*i, affecting) | culled;
			}
		}
	}

	return culled;
}

BOOL CEnchantmentRegistry::Duel(Enchantment *challenger, PackableListWithJson<Enchantment> *list)
{
	for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
	{
		if (i->_spell_category == challenger->_spell_category)
		{
			if (i->Duel(challenger))
				return FALSE;

			if (i->_id)
				list->erase(i);

			break;
		}
	}

	list->push_back(Enchantment(*challenger));
	return TRUE;
}

BOOL CEnchantmentRegistry::RemoveEnchantments(PackableListWithJson<DWORD> *to_remove)
{
	BOOL bRemoved = FALSE;
	for (PackableListWithJson<DWORD>::iterator i = to_remove->begin(); i != to_remove->end(); i++)
	{
		bRemoved = RemoveEnchantment(*i);
	}

	return bRemoved;
}

BOOL CEnchantmentRegistry::PurgeEnchantmentList(PackableListWithJson<Enchantment> *list)
{
	if (!list)
		return FALSE;

	PackableListWithJson<DWORD> toRemove;

	for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
	{
		if (i->_duration == -1.0)
			continue;

		toRemove.push_back(i->_id);
	}

	return RemoveEnchantments(&toRemove);
}

BOOL CEnchantmentRegistry::PurgeBadEnchantmentList(PackableListWithJson<Enchantment> *list)
{
	if (!list)
		return FALSE;

	PackableListWithJson<DWORD> toRemove;

	for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
	{
		if ((i->_smod.type & 0x2000000) || i->_duration == -1.0)
			continue;

		toRemove.push_back(i->_id);
	}

	return RemoveEnchantments(&toRemove);
}

BOOL CEnchantmentRegistry::PurgeEnchantments()
{
	return PurgeEnchantmentList(_mult_list) || PurgeEnchantmentList(_add_list);
}

BOOL CEnchantmentRegistry::PurgeBadEnchantments()
{
	return PurgeBadEnchantmentList(_mult_list) || PurgeBadEnchantmentList(_add_list);
}

void CEnchantmentRegistry::GetExpiredEnchantments(PackableListWithJson<Enchantment> *list, PackableListWithJson<DWORD> *expired)
{
	if (!list)
		return;

	for (PackableListWithJson<Enchantment>::iterator i = list->begin(); i != list->end(); i++)
	{
		if (i->HasExpired())
			expired->push_back(i->_id);
	}
}

void CEnchantmentRegistry::GetExpiredEnchantments(PackableListWithJson<DWORD> *expired)
{
	GetExpiredEnchantments(_mult_list, expired);
	GetExpiredEnchantments(_add_list, expired);
	GetExpiredEnchantments(_cooldown_list, expired);

	if (_vitae && _vitae->HasExpired())
		expired->push_back(_vitae->_id);
}

BOOL CACQualities::PurgeEnchantments()
{
	if (_enchantment_reg)
		return _enchantment_reg->PurgeEnchantments();

	return FALSE;
}

BOOL CACQualities::PurgeBadEnchantments()
{
	if (_enchantment_reg)
		return _enchantment_reg->PurgeBadEnchantments();

	return FALSE;
}

BOOL CEnchantmentRegistry::InqVitae(Enchantment *vitae)
{
	if (_vitae)
	{
		*vitae = *_vitae;
		return TRUE;
	}

	return FALSE;
}

BOOL CACQualities::InqVitae(Enchantment *vitae)
{
	if (_enchantment_reg)
		return _enchantment_reg->InqVitae(vitae);

	return FALSE;
}

double CEnchantmentRegistry::GetVitaeValue()
{
	if (_vitae)
		return _vitae->_smod.val;

	return 1.0;
}

double CACQualities::GetVitaeValue()
{
	if (_enchantment_reg)
		return _enchantment_reg->GetVitaeValue();

	return 1.0;
}

DEFINE_PACK(CreationProfile)
{
	pWriter->Write<DWORD>(wcid);
	pWriter->Write<DWORD>(palette);
	pWriter->Write<float>(shade);
	pWriter->Write<int>(destination);
	pWriter->Write<long>(stack_size);
	pWriter->Write<int>(try_to_bond);
}

DEFINE_UNPACK(CreationProfile)
{
	wcid = pReader->Read<DWORD>();
	palette = pReader->Read<DWORD>();
	shade = pReader->Read<float>();
	destination = pReader->Read<int>();
	stack_size = pReader->Read<long>();
	try_to_bond = pReader->Read<int>();
	return true;
}

DEFINE_PACK_JSON(CreationProfile)
{
	writer["wcid"] = wcid;
	writer["palette"] = palette;
	writer["shade"] = shade;
	writer["destination"] = destination;
	writer["stack_size"] = stack_size;
	writer["try_to_bond"] = try_to_bond;
}

DEFINE_UNPACK_JSON(CreationProfile)
{
	wcid = reader["wcid"];
	palette = reader["palette"];
	shade = reader["shade"];
	destination = reader["destination"];
	stack_size = reader["stack_size"];
	try_to_bond = reader["try_to_bond"];
	return true;
}

DEFINE_PACK(ArmorCache)
{
	pWriter->Write<int>(_base_armor);
	pWriter->Write<int>(_armor_vs_slash);
	pWriter->Write<int>(_armor_vs_pierce);
	pWriter->Write<int>(_armor_vs_bludgeon);
	pWriter->Write<int>(_armor_vs_cold);
	pWriter->Write<int>(_armor_vs_fire);
	pWriter->Write<int>(_armor_vs_acid);
	pWriter->Write<int>(_armor_vs_electric);
	pWriter->Write<int>(_armor_vs_nether);
}

DEFINE_UNPACK(ArmorCache)
{
	_base_armor = pReader->Read<int>();
	_armor_vs_slash = pReader->Read<int>();
	_armor_vs_pierce = pReader->Read<int>();
	_armor_vs_bludgeon = pReader->Read<int>();
	_armor_vs_cold = pReader->Read<int>();
	_armor_vs_fire = pReader->Read<int>();
	_armor_vs_acid = pReader->Read<int>();
	_armor_vs_electric = pReader->Read<int>();

#ifndef PRE_TOD_DATA_FILES
	_armor_vs_nether = pReader->Read<int>();
#endif

	return true;
}

DEFINE_PACK_JSON(ArmorCache)
{
	writer["base_armor"] = _base_armor;
	writer["armor_vs_slash"] = _armor_vs_slash;
	writer["armor_vs_pierce"] = _armor_vs_pierce;
	writer["armor_vs_bludgeon"] = _armor_vs_bludgeon;
	writer["armor_vs_cold"] = _armor_vs_cold;
	writer["armor_vs_fire"] = _armor_vs_fire;
	writer["armor_vs_acid"] = _armor_vs_acid;
	writer["armor_vs_electric"] = _armor_vs_electric;
	writer["armor_vs_nether"] = _armor_vs_nether;
}

DEFINE_UNPACK_JSON(ArmorCache)
{
	_base_armor = reader["base_armor"];
	_armor_vs_slash = reader["armor_vs_slash"];
	_armor_vs_pierce = reader["armor_vs_pierce"];
	_armor_vs_bludgeon = reader["armor_vs_bludgeon"];
	_armor_vs_cold = reader["armor_vs_cold"];
	_armor_vs_fire = reader["armor_vs_fire"];
	_armor_vs_acid = reader["armor_vs_acid"];
	_armor_vs_electric = reader["armor_vs_electric"];
	_armor_vs_nether = reader["armor_vs_nether"];
	return true;
}

DEFINE_PACK(BodyPartSelectionData)
{
	pWriter->Write<float>(HLF);
	pWriter->Write<float>(MLF);
	pWriter->Write<float>(LLF);
	pWriter->Write<float>(HRF);
	pWriter->Write<float>(MRF);
	pWriter->Write<float>(LRF);
	pWriter->Write<float>(HLB);
	pWriter->Write<float>(MLB);
	pWriter->Write<float>(LLB);
	pWriter->Write<float>(HRB);
	pWriter->Write<float>(MRB);
	pWriter->Write<float>(LRB);
}

DEFINE_UNPACK(BodyPartSelectionData)
{
	HLF = pReader->Read<float>();
	MLF = pReader->Read<float>();
	LLF = pReader->Read<float>();
	HRF = pReader->Read<float>();
	MRF = pReader->Read<float>();
	LRF = pReader->Read<float>();
	HLB = pReader->Read<float>();
	MLB = pReader->Read<float>();
	LLB = pReader->Read<float>();
	HRB = pReader->Read<float>();
	MRB = pReader->Read<float>();
	LRB = pReader->Read<float>();
	return true;
}

DEFINE_PACK_JSON(BodyPartSelectionData)
{
	writer["HLF"] = HLF;
	writer["MLF"] = MLF;
	writer["LLF"] = LLF;
	writer["HRF"] = HRF;
	writer["MRF"] = MRF;
	writer["LRF"] = LRF;
	writer["HLB"] = HLB;
	writer["MLB"] = MLB;
	writer["LLB"] = LLB;
	writer["HRB"] = HRB;
	writer["MRB"] = MRB;
	writer["LRB"] = LRB;
}

DEFINE_UNPACK_JSON(BodyPartSelectionData)
{
	HLF = reader["HLF"];
	MLF = reader["MLF"];
	LLF = reader["LLF"];
	HRF = reader["HRF"];
	MRF = reader["MRF"];
	LRF = reader["LRF"];
	HLB = reader["HLB"];
	MLB = reader["MLB"];
	LLB = reader["LLB"];
	HRB = reader["HRB"];
	MRB = reader["MRB"];
	LRB = reader["LRB"];
	return true;
}

BodyPart::BodyPart()
{
}

BodyPart::~BodyPart()
{
	SafeDelete(_bpsd);
}

BodyPart &BodyPart::operator=(BodyPart const &other)
{
	_dtype = other._dtype;
	_dval = other._dval;
	_dvar = other._dvar;
	_acache = other._acache;
	_bh = other._bh;
	
	if (!other._bpsd)
	{
		if (_bpsd)
		{
			delete _bpsd;
			_bpsd = NULL;
		}

		return *this;
	}

	if (_bpsd)
		*_bpsd = *other._bpsd;
	else
		_bpsd = new BodyPartSelectionData(*other._bpsd);

	return *this;
}

DEFINE_PACK(BodyPart)
{
	pWriter->Write<int>(_bpsd ? 1 : 0);
	pWriter->Write<DWORD>(_dtype);
	pWriter->Write<int>(_dval);
	pWriter->Write<float>(_dvar);
	_acache.Pack(pWriter);
	pWriter->Write<DWORD>(_bh);
	if (_bpsd)
		_bpsd->Pack(pWriter);
}

DEFINE_UNPACK(BodyPart)
{
	int hasBPSD = pReader->Read<int>();

	_dtype = (DAMAGE_TYPE) pReader->Read<DWORD>();
	_dval = pReader->Read<int>();
	_dvar = pReader->Read<float>();
	_acache.UnPack(pReader);
	_bh = (BODY_HEIGHT)pReader->Read<DWORD>();

	SafeDelete (_bpsd);
	if (hasBPSD)
	{
		_bpsd = new BodyPartSelectionData();
		_bpsd->UnPack(pReader);
	}

	return true;
}

DEFINE_PACK_JSON(BodyPart)
{
	writer["dtype"] = (DWORD) _dtype;
	writer["dval"] = _dval;
	writer["dvar"] = _dvar;
	_acache.PackJson(writer["acache"]);
	writer["bh"] = (DWORD) _bh;

	if (_bpsd)
	{
		_bpsd->PackJson(writer["bpsd"]);
	}
}

DEFINE_UNPACK_JSON(BodyPart)
{
	_dtype = (DAMAGE_TYPE) (DWORD) reader["dtype"];
	_dval = reader["dval"];
	_dvar = reader["dvar"];
	_acache.UnPackJson(reader["acache"]);
	_bh = (BODY_HEIGHT) (DWORD) reader["bh"];

	SafeDelete(_bpsd);

	if (reader.find("bpsd") != reader.end())
	{
		_bpsd = new BodyPartSelectionData();
		_bpsd->UnPackJson(reader["bpsd"]);
	}

	return true;
}

DEFINE_PACK(Body)
{
	_body_part_table.Pack(pWriter);
}

DEFINE_UNPACK(Body)
{
	_body_part_table.UnPack(pReader);
	return true;
}

DEFINE_PACK_JSON(Body)
{
	_body_part_table.PackJson(writer["body_part_table"]);
}

DEFINE_UNPACK_JSON(Body)
{
	_body_part_table.UnPackJson(reader["body_part_table"]);
	return true;
}


Attribute::Attribute()
{
	_level_from_cp = 0;
	_init_level = 0;
	_cp_spent = 0;
}

Attribute::~Attribute()
{
}

DEFINE_UNPACK(Attribute)
{
	_level_from_cp = pReader->Read<DWORD>();
	_init_level = pReader->Read<DWORD>();
	_cp_spent = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(Attribute)
{
	pWriter->Write<DWORD>(_level_from_cp);
	pWriter->Write<DWORD>(_init_level);
	pWriter->Write<DWORD>(_cp_spent);
}

DEFINE_UNPACK_JSON(Attribute)
{
	_level_from_cp = reader["level_from_cp"];
	_init_level = reader["init_level"];
	_cp_spent = reader["cp_spent"];
	return true;
}

DEFINE_PACK_JSON(Attribute)
{
	writer["level_from_cp"] = _level_from_cp;
	writer["init_level"] = _init_level;
	writer["cp_spent"] = _cp_spent;
}

const char *Attribute::GetAttributeName(STypeAttribute key) // custom
{
	switch (key)
	{
	case STRENGTH_ATTRIBUTE: return "Strength";
	case ENDURANCE_ATTRIBUTE: return "Endurance";
	case COORDINATION_ATTRIBUTE: return "Coordination";
	case QUICKNESS_ATTRIBUTE: return "Quickness";
	case FOCUS_ATTRIBUTE: return "Focus";
	case SELF_ATTRIBUTE: return "Self";
	default: return "";
	}
}

SecondaryAttribute::SecondaryAttribute()
{
	_current = 0;
}

SecondaryAttribute::~SecondaryAttribute()
{
}

DEFINE_UNPACK(SecondaryAttribute)
{
	Attribute::UnPack(pReader);
	_current = pReader->Read<DWORD>();

	return true;
}

DEFINE_PACK(SecondaryAttribute)
{
	Attribute::Pack(pWriter);
	pWriter->Write<DWORD>(_current);
}

DEFINE_UNPACK_JSON(SecondaryAttribute)
{
	Attribute::UnPackJson(reader);
	_current = reader["current"];
	return true;
}

DEFINE_PACK_JSON(SecondaryAttribute)
{
	Attribute::PackJson(writer);
	writer["current"] = _current;
}

DEFINE_DBOBJ(Attribute2ndTable, Attribute2ndTables);
DEFINE_LEGACY_PACK_MIGRATOR(Attribute2ndTable);

DEFINE_PACK(Attribute2ndTable)
{
}

DEFINE_UNPACK(Attribute2ndTable)
{
	pReader->Read<DWORD>(); // skip file ID
	_max_health.UnPack(pReader);
	_max_stamina.UnPack(pReader);
	_max_mana.UnPack(pReader);
	return true;
}

AttributeCache::AttributeCache()
{
}

AttributeCache::~AttributeCache()
{
	Clear();
}

void AttributeCache::Clear()
{
	SafeDelete(_strength);
	SafeDelete(_endurance);
	SafeDelete(_quickness);
	SafeDelete(_coordination);
	SafeDelete(_focus);
	SafeDelete(_self);
	SafeDelete(_health);
	SafeDelete(_stamina);
	SafeDelete(_mana);
}

DEFINE_UNPACK(AttributeCache)
{
	DWORD contentFlags = pReader->Read<DWORD>();

	if (contentFlags & 1)
	{
		if (!_strength)
			_strength = new Attribute();
		_strength->UnPack(pReader);
	}
	if (contentFlags & 2)
	{
		if (!_endurance)
			_endurance = new Attribute();
		_endurance->UnPack(pReader);
	}
	if (contentFlags & 4)
	{
		if (!_quickness)
			_quickness = new Attribute();
		_quickness->UnPack(pReader);
	}
	if (contentFlags & 8)
	{
		if (!_coordination)
			_coordination = new Attribute();
		_coordination->UnPack(pReader);
	}
	if (contentFlags & 0x10)
	{
		if (!_focus)
			_focus = new Attribute();
		_focus->UnPack(pReader);
	}
	if (contentFlags & 0x20)
	{
		if (!_self)
			_self = new Attribute();
		_self->UnPack(pReader);
	}

	if (contentFlags & 0x40)
	{
		if (!_health)
			_health = new SecondaryAttribute();
		_health->UnPack(pReader);
	}
	if (contentFlags & 0x80)
	{
		if (!_stamina)
			_stamina = new SecondaryAttribute();
		_stamina->UnPack(pReader);
	}
	if (contentFlags & 0x100)
	{
		if (!_mana)
			_mana = new SecondaryAttribute();
		_mana->UnPack(pReader);
	}

	return true;
}

DEFINE_PACK(AttributeCache)
{
	BinaryWriter content;
	DWORD contentFlags = 0;

	if (_strength)
	{
		contentFlags |= 1;
		content.Write(_strength);
	}
	if (_endurance)
	{
		contentFlags |= 2;
		content.Write(_endurance);
	}
	if (_quickness)
	{
		contentFlags |= 4;
		content.Write(_quickness);
	}
	if (_coordination)
	{
		contentFlags |= 8;
		content.Write(_coordination);
	}
	if (_focus)
	{
		contentFlags |= 0x10;
		content.Write(_focus);
	}
	if (_self)
	{
		contentFlags |= 0x20;
		content.Write(_self);
	}
	if (_health)
	{
		contentFlags |= 0x40;
		content.Write(_health);
	}
	if (_stamina)
	{
		contentFlags |= 0x80;
		content.Write(_stamina);
	}
	if (_mana)
	{
		contentFlags |= 0x100;
		content.Write(_mana);
	}

	pWriter->Write<DWORD>(contentFlags);
	pWriter->Write(&content);
}


DEFINE_UNPACK_JSON(AttributeCache)
{
	if (reader.find("strength") != reader.end())
	{
		if (!_strength)
			_strength = new Attribute();
		_strength->UnPackJson(reader["strength"]);
	}
	if (reader.find("endurance") != reader.end())
	{
		if (!_endurance)
			_endurance = new Attribute();
		_endurance->UnPackJson(reader["endurance"]);
	}
	if (reader.find("quickness") != reader.end())
	{
		if (!_quickness)
			_quickness = new Attribute();
		_quickness->UnPackJson(reader["quickness"]);
	}
	if (reader.find("coordination") != reader.end())
	{
		if (!_coordination)
			_coordination = new Attribute();
		_coordination->UnPackJson(reader["coordination"]);
	}
	if (reader.find("focus") != reader.end())
	{
		if (!_focus)
			_focus = new Attribute();
		_focus->UnPackJson(reader["focus"]);
	}
	if (reader.find("self") != reader.end())
	{
		if (!_self)
			_self = new Attribute();
		_self->UnPackJson(reader["self"]);
	}

	if (reader.find("health") != reader.end())
	{
		if (!_health)
			_health = new SecondaryAttribute();
		_health->UnPackJson(reader["health"]);
	}
	if (reader.find("stamina") != reader.end())
	{
		if (!_stamina)
			_stamina = new SecondaryAttribute();
		_stamina->UnPackJson(reader["stamina"]);
	}
	if (reader.find("mana") != reader.end())
	{
		if (!_mana)
			_mana = new SecondaryAttribute();
		_mana->UnPackJson(reader["mana"]);
	}

	return true;
}

DEFINE_PACK_JSON(AttributeCache)
{
	if (_strength)
	{
		_strength->PackJson(writer["strength"]);
	}
	if (_endurance)
	{
		_endurance->PackJson(writer["endurance"]);
	}
	if (_quickness)
	{
		_quickness->PackJson(writer["quickness"]);
	}
	if (_coordination)
	{
		_coordination->PackJson(writer["coordination"]);
	}
	if (_focus)
	{
		_focus->PackJson(writer["focus"]);
	}
	if (_self)
	{
		_self->PackJson(writer["self"]);
	}
	if (_health)
	{
		_health->PackJson(writer["health"]);
	}
	if (_stamina)
	{
		_stamina->PackJson(writer["stamina"]);
	}
	if (_mana)
	{
		_mana->PackJson(writer["mana"]);
	}
}

BOOL AttributeCache::SetAttribute(STypeAttribute key, DWORD val)
{
	switch (key)
	{
	case STRENGTH_ATTRIBUTE:
		if (!_strength)
			_strength = new Attribute();
		
		_strength->_init_level = val;
		return TRUE;

	case ENDURANCE_ATTRIBUTE:
		if (!_endurance)
			_endurance = new Attribute();
		
		_endurance->_init_level = val;
		return TRUE;

	case COORDINATION_ATTRIBUTE:
		if (!_coordination)
			_coordination = new Attribute();

		_coordination->_init_level = val;
		return TRUE;

	case QUICKNESS_ATTRIBUTE:
		if (!_quickness)
			_quickness = new Attribute();
		
		_quickness->_init_level = val;
		return TRUE;

	case FOCUS_ATTRIBUTE:
		if (!_focus)
			_focus = new Attribute();
		
		_focus->_init_level = val;
		return TRUE;

	case SELF_ATTRIBUTE:
		if (!_self)
			_self = new Attribute();
		
		_self->_init_level = val;
		return TRUE;
	}

	return FALSE;
}


BOOL AttributeCache::SetAttribute(STypeAttribute key, const Attribute& attrib)
{
	switch (key)
	{
	case STRENGTH_ATTRIBUTE:
		if (!_strength)
			_strength = new Attribute(attrib);
		else
			*_strength = attrib;
		return TRUE;

	case ENDURANCE_ATTRIBUTE:
		if (!_endurance)
			_endurance = new Attribute(attrib);
		else
			*_endurance = attrib;
		return TRUE;

	case COORDINATION_ATTRIBUTE:
		if (!_coordination)
			_coordination = new Attribute(attrib);
		else
			*_coordination = attrib;
		return TRUE;

	case QUICKNESS_ATTRIBUTE:
		if (!_quickness)
			_quickness = new Attribute(attrib);
		else
			*_quickness = attrib;
		return TRUE;

	case FOCUS_ATTRIBUTE:
		if (!_focus)
			_focus = new Attribute(attrib);
		else
			*_focus = attrib;
		return TRUE;

	case SELF_ATTRIBUTE:
		if (!_self)
			_self = new Attribute(attrib);
		else
			*_self = attrib;
		return TRUE;
	}

	return FALSE;
}

BOOL AttributeCache::InqAttribute(STypeAttribute index, Attribute &value)
{
	switch (index)
	{
	case STRENGTH_ATTRIBUTE:
		if (!_strength)
			return FALSE;

		value = *_strength;
		return TRUE;

	case ENDURANCE_ATTRIBUTE:
		if (!_endurance)
			return FALSE;

		value = *_endurance;
		return TRUE;

	case COORDINATION_ATTRIBUTE:
		if (!_coordination)
			return FALSE;

		value = *_coordination;
		return TRUE;

	case QUICKNESS_ATTRIBUTE:
		if (!_quickness)
			return FALSE;

		value = *_quickness;
		return TRUE;

	case FOCUS_ATTRIBUTE:
		if (!_focus)
			return FALSE;

		value = *_focus;
		return TRUE;

	case SELF_ATTRIBUTE:
		if (!_self)
			return FALSE;

		value = *_self;
		return TRUE;
	}

	return FALSE;
}

BOOL AttributeCache::InqAttribute(STypeAttribute index, DWORD &value)
{
	switch (index)
	{
	case STRENGTH_ATTRIBUTE:
		if (!_strength)
			return FALSE;

		value = _strength->_init_level + _strength->_level_from_cp;
		return TRUE;

	case ENDURANCE_ATTRIBUTE:
		if (!_endurance)
			return FALSE;

		value = _endurance->_init_level + _endurance->_level_from_cp;
		return TRUE;

	case COORDINATION_ATTRIBUTE:
		if (!_coordination)
			return FALSE;

		value = _coordination->_init_level + _coordination->_level_from_cp;
		return TRUE;

	case QUICKNESS_ATTRIBUTE:
		if (!_quickness)
			return FALSE;

		value = _quickness->_init_level + _quickness->_level_from_cp;
		return TRUE;

	case FOCUS_ATTRIBUTE:
		if (!_focus)
			return FALSE;

		value = _focus->_init_level + _focus->_level_from_cp;
		return TRUE;

	case SELF_ATTRIBUTE:
		if (!_self)
			return FALSE;

		value = _self->_init_level + _self->_level_from_cp;
		return TRUE;
	}

	return FALSE;
}

BOOL AttributeCache::InqAttribute2nd(STypeAttribute2nd key, SecondaryAttribute &value)
{
	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
	case HEALTH_ATTRIBUTE_2ND:
		if (_health)
		{
			value = *_health;
			return TRUE;
		}

		break;

	case MAX_STAMINA_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
		if (_stamina)
		{
			value = *_stamina;
			return TRUE;
		}

		break;

	case MAX_MANA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		if (_mana)
		{
			value = *_mana;
			return TRUE;
		}

		break;
	}

	return FALSE;
}

BOOL AttributeCache::InqAttribute2nd(STypeAttribute2nd key, DWORD &value)
{
	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
		if (!_health)
			return FALSE;

		value = _health->_init_level + _health->_level_from_cp;
		return TRUE;
	case HEALTH_ATTRIBUTE_2ND:
		if (!_health)
			return FALSE;

		value = _health->_current;
		return TRUE;
	case MAX_STAMINA_ATTRIBUTE_2ND:
		if (!_stamina)
			return FALSE;

		value = _stamina->_init_level + _stamina->_level_from_cp;
		return TRUE;
	case STAMINA_ATTRIBUTE_2ND:
		if (!_stamina)
			return FALSE;

		value = _stamina->_current;
		return TRUE;
	case MAX_MANA_ATTRIBUTE_2ND:
		if (!_mana)
			return FALSE;

		value = _mana->_init_level + _mana->_level_from_cp;
		return TRUE;
	case MANA_ATTRIBUTE_2ND:
		if (!_mana)
			return FALSE;

		value = _mana->_current;
		return TRUE;
	}

	return FALSE;
}

BOOL AttributeCache::SetAttribute2nd(STypeAttribute2nd key, const SecondaryAttribute &attrib)
{
	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
	case HEALTH_ATTRIBUTE_2ND:
		if (!_health)
			_health = new SecondaryAttribute(attrib);
		else
			*_health = attrib;

		return TRUE;
	case MAX_STAMINA_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
		if (!_stamina)
			_stamina = new SecondaryAttribute(attrib);
		else
			*_stamina = attrib;

		return TRUE;
	case MAX_MANA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		if (!_mana)
			_mana = new SecondaryAttribute(attrib);
		else
			*_mana = attrib;

		return TRUE;
	}

	return FALSE;
}

BOOL AttributeCache::SetAttribute2nd(STypeAttribute2nd key, DWORD value)
{
	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
		if (!_health)
		{
			_health = new SecondaryAttribute();
		}

		_health->_init_level = value;
		return TRUE;
	case HEALTH_ATTRIBUTE_2ND:
		if (!_health)
		{
			_health = new SecondaryAttribute();
		}

		_health->_current = value;
		return TRUE;
	case MAX_STAMINA_ATTRIBUTE_2ND:
		if (!_stamina)
		{
			_stamina = new SecondaryAttribute();
		}

		_stamina->_init_level = value;
		return TRUE;
	case STAMINA_ATTRIBUTE_2ND:
		if (!_stamina)
		{
			_stamina = new SecondaryAttribute();
		}

		_stamina->_current = value;
		return TRUE;
	case MAX_MANA_ATTRIBUTE_2ND:
		if (!_mana)
		{
			_mana = new SecondaryAttribute();
		}

		_mana->_init_level = value;
		return TRUE;
	case MANA_ATTRIBUTE_2ND:
		if (!_mana)
		{
			_mana = new SecondaryAttribute();
		}

		_mana->_current = value;
		return TRUE;
	}

	return FALSE;
}

DEFINE_UNPACK(Skill)
{
	DWORD ppFlag = pReader->Read<DWORD>();
	_level_from_pp = ppFlag & 0xFFFF;

	_sac = (SKILL_ADVANCEMENT_CLASS) pReader->Read<int>();
	_pp = pReader->Read<DWORD>();
	_init_level = pReader->Read<DWORD>();
	_resistance_of_last_check = pReader->Read<long>();
	_last_used_time = Timer::cur_time - pReader->Read<double>(); // this doesn't seem right

	return true;
}

DEFINE_PACK(Skill)
{
	pWriter->Write<DWORD>(0x10000 | (_level_from_pp & 0xFFFF));
	pWriter->Write<int>(_sac);
	pWriter->Write<DWORD>(_pp);
	pWriter->Write<DWORD>(_init_level);
	pWriter->Write<long>(_resistance_of_last_check);
	pWriter->Write<double>(Timer::cur_time - _last_used_time); // this doesn't seem right
}

DEFINE_UNPACK_JSON(Skill)
{
	_level_from_pp = reader["level_from_pp"];
	_sac = reader["sac"];
	_pp = reader["pp"];
	_init_level = reader["init_level"];
	_resistance_of_last_check = reader["resistance_of_last_check"];
	_last_used_time = Timer::cur_time - reader["last_used_time"];
	return true;
}

DEFINE_PACK_JSON(Skill)
{
	writer["level_from_pp"] = _level_from_pp;
	writer["sac"] = _sac;
	writer["pp"] = _pp;
	writer["init_level"] = _init_level;
	writer["resistance_of_last_check"] = _resistance_of_last_check;
	writer["last_used_time"] = Timer::cur_time - _last_used_time;
}

void Skill::SetSkillAdvancementClass(SKILL_ADVANCEMENT_CLASS val)
{
	_sac = val;
	_level_from_pp = ExperienceSystem::SkillLevelFromExperience(val, _pp);
}

DEFINE_UNPACK(SpellBookPage)
{
	_casting_likelihood = pReader->Read<float>() - 2.0f;
	return true;
}

DEFINE_PACK(SpellBookPage)
{
	pWriter->Write<float>(_casting_likelihood + 2.0f);
}

DEFINE_UNPACK_JSON(SpellBookPage)
{
	_casting_likelihood = ((float)reader["casting_likelihood"]) - 2.0f;
	return true;
}

DEFINE_PACK_JSON(SpellBookPage)
{
	writer["casting_likelihood"] = _casting_likelihood + 2.0f;
}

DEFINE_UNPACK(CSpellBook)
{
	_spellbook.UnPack(pReader);
	return true;
}

DEFINE_PACK(CSpellBook)
{
	_spellbook.Pack(pWriter);
}

DEFINE_UNPACK_JSON(CSpellBook)
{
	_spellbook.UnPackJson(reader);
	return true;
}

DEFINE_PACK_JSON(CSpellBook)
{
	_spellbook.PackJson(writer);
}

void CSpellBook::TranscribeSpells(const std::list<DWORD> &spells)
{
}

void CSpellBook::Prune()
{
	UNFINISHED();
}

bool CSpellBook::Exists(DWORD spellid)
{
	return _spellbook.lookup(spellid) != NULL;
}

void CSpellBook::AddSpell(DWORD spellid, const SpellBookPage &spell)
{
	_spellbook[spellid] = spell;
}

void CSpellBook::RemoveSpell(DWORD spellid)
{
	_spellbook.remove(spellid);
}

void CSpellBook::ClearSpells()
{
	_spellbook.clear();
}

CBaseQualities::CBaseQualities()
{
}

CBaseQualities::~CBaseQualities()
{
	Clear();
}

void CBaseQualities::Clear()
{
	SafeDelete(m_IntStats);
	SafeDelete(m_Int64Stats);
	SafeDelete(m_BoolStats);
	SafeDelete(m_FloatStats);
	SafeDelete(m_StringStats);
	SafeDelete(m_DIDStats);
	SafeDelete(m_IIDStats);
	SafeDelete(m_PositionStats);
}

DEFINE_UNPACK(CBaseQualities)
{
	Clear();

	DWORD contentFlags = pReader->Read<DWORD>();
	m_WeenieType = (ITEM_TYPE) pReader->Read<int>();

	if (contentFlags & 1)
	{
		if (!m_IntStats)
		{
			m_IntStats = new PackableHashTableWithJson<STypeInt, int>();
		}

		m_IntStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_IntStats);
	}

	if (contentFlags & 0x80)
	{
		if (!m_Int64Stats)
		{
			m_Int64Stats = new PackableHashTableWithJson<STypeInt64, long long>();
		}

		m_Int64Stats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_Int64Stats);
	}

	if (contentFlags & 2)
	{
		if (!m_BoolStats)
		{
			m_BoolStats = new PackableHashTableWithJson<STypeBool, BOOL>();
		}

		m_BoolStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_BoolStats);
	}

	if (contentFlags & 4)
	{
		if (!m_FloatStats)
		{
			m_FloatStats = new PackableHashTableWithJson<STypeFloat, double>();
		}

		m_FloatStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_FloatStats);
	}

	if (contentFlags & 0x10)
	{
		if (!m_StringStats)
		{
			m_StringStats = new PackableHashTableWithJson<STypeString, std::string>();
		}

		m_StringStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_StringStats);
	}

	if (contentFlags & 8)
	{
		if (!m_DIDStats)
		{
			m_DIDStats = new PackableHashTableWithJson<STypeDID, DWORD>();
		}

		m_DIDStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_DIDStats);
	}

	if (contentFlags & 0x40)
	{
		if (!m_IIDStats)
		{
			m_IIDStats = new PackableHashTableWithJson<STypeIID, DWORD>();
		}

		m_IIDStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_IIDStats);
	}

	if (contentFlags & 0x20)
	{
		if (!m_PositionStats)
		{
			m_PositionStats = new PackableHashTableWithJson<STypePosition, Position>();
		}

		m_PositionStats->UnPack(pReader);
	}
	else
	{
		SafeDelete(m_PositionStats);
	}

	return true;
}

DEFINE_PACK(CBaseQualities)
{
	BinaryWriter content;
	DWORD contentFlags = 0;

	if (m_IntStats)
	{
		contentFlags |= 1;
		content.Write(m_IntStats);
	}
	if (m_Int64Stats)
	{
		contentFlags |= 0x80;
		content.Write(m_Int64Stats);
	}
	if (m_BoolStats)
	{
		contentFlags |= 2;
		content.Write(m_BoolStats);
	}
	if (m_FloatStats)
	{
		contentFlags |= 4;
		content.Write(m_FloatStats);
	}
	if (m_StringStats)
	{
		contentFlags |= 0x10;
		content.Write(m_StringStats);
	}
	if (m_DIDStats)
	{
		contentFlags |= 8;
		content.Write(m_DIDStats);
	}
	if (m_IIDStats)
	{
		contentFlags |= 0x40;
		content.Write(m_IIDStats);
	}
	if (m_PositionStats)
	{
		contentFlags |= 0x20;
		content.Write(m_PositionStats);
	}

	pWriter->Write<DWORD>(contentFlags);
	pWriter->Write<int>((int)m_WeenieType);
	pWriter->Write(&content);
}

DEFINE_UNPACK_JSON(CBaseQualities)
{
	Clear();

	m_WeenieType = (ITEM_TYPE) (int) reader["weenieType"];

	if (reader.find("intStats") != reader.end())
	{
		if (!m_IntStats)
		{
			m_IntStats = new PackableHashTableWithJson<STypeInt, int>();
		}

		m_IntStats->UnPackJson(reader["intStats"]);
	}
	else
	{
		SafeDelete(m_IntStats);
	}

	if (reader.find("int64Stats") != reader.end())
	{
		if (!m_Int64Stats)
		{
			m_Int64Stats = new PackableHashTableWithJson<STypeInt64, long long>();
		}

		m_Int64Stats->UnPackJson(reader["int64Stats"]);
	}
	else
	{
		SafeDelete(m_Int64Stats);
	}

	if (reader.find("boolStats") != reader.end())
	{
		if (!m_BoolStats)
		{
			m_BoolStats = new PackableHashTableWithJson<STypeBool, BOOL>();
		}

		m_BoolStats->UnPackJson(reader["boolStats"]);
	}
	else
	{
		SafeDelete(m_BoolStats);
	}

	if (reader.find("floatStats") != reader.end())
	{
		if (!m_FloatStats)
		{
			m_FloatStats = new PackableHashTableWithJson<STypeFloat, double>();
		}

		m_FloatStats->UnPackJson(reader["floatStats"]);
	}
	else
	{
		SafeDelete(m_FloatStats);
	}

	if (reader.find("stringStats") != reader.end())
	{
		if (!m_StringStats)
		{
			m_StringStats = new PackableHashTableWithJson<STypeString, std::string>();
		}

		m_StringStats->UnPackJson(reader["stringStats"]);
	}
	else
	{
		SafeDelete(m_StringStats);
	}

	if (reader.find("didStats") != reader.end())
	{
		if (!m_DIDStats)
		{
			m_DIDStats = new PackableHashTableWithJson<STypeDID, DWORD>();
		}

		m_DIDStats->UnPackJson(reader["didStats"]);
	}
	else
	{
		SafeDelete(m_DIDStats);
	}

	if (reader.find("iidStats") != reader.end())
	{
		if (!m_IIDStats)
		{
			m_IIDStats = new PackableHashTableWithJson<STypeIID, DWORD>();
		}

		m_IIDStats->UnPackJson(reader["iidStats"]);
	}
	else
	{
		SafeDelete(m_IIDStats);
	}

	if (reader.find("posStats") != reader.end())
	{
		if (!m_PositionStats)
		{
			m_PositionStats = new PackableHashTableWithJson<STypePosition, Position>();
		}

		m_PositionStats->UnPackJson(reader["posStats"]);
	}
	else
	{
		SafeDelete(m_PositionStats);
	}

	return true;
}

DEFINE_PACK_JSON(CBaseQualities)
{
	writer["weenieType"] = m_WeenieType;

	if (m_IntStats)
	{
		m_IntStats->PackJson(writer["intStats"]);
	}
	if (m_Int64Stats)
	{
		m_Int64Stats->PackJson(writer["int64Stats"]);
	}
	if (m_BoolStats)
	{
		m_BoolStats->PackJson(writer["boolStats"]);
	}
	if (m_FloatStats)
	{
		m_FloatStats->PackJson(writer["floatStats"]);
	}
	if (m_StringStats)
	{
		m_StringStats->PackJson(writer["stringStats"]);
	}
	if (m_DIDStats)
	{
		m_DIDStats->PackJson(writer["didStats"]);
	}
	if (m_IIDStats)
	{
		m_IIDStats->PackJson(writer["iidStats"]);
	}
	if (m_PositionStats)
	{
		m_PositionStats->PackJson(writer["posStats"]);
	}
}

void CBaseQualities::SetInt(STypeInt key, int value)
{
	if (!m_IntStats)
	{
		m_IntStats = new PackableHashTableWithJson<STypeInt, int>();
	}

	m_IntStats->operator[](key) = value;
}

BOOL CBaseQualities::InqInt(STypeInt key, int &value, BOOL raw, BOOL allow_negative)
{
	if (m_IntStats)
	{
		const int *pValue = m_IntStats->lookup(key);

		if (pValue)
		{
			value = *pValue;

			if (!raw)
				EnchantInt(key, value, allow_negative);

			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveInt(STypeInt key)
{
	if (m_IntStats)
	{
		m_IntStats->remove(key);
	}
}

void CBaseQualities::SetInt64(STypeInt64 key, __int64 value)
{
	if (!m_Int64Stats)
	{
		m_Int64Stats = new PackableHashTableWithJson<STypeInt64, __int64>();
	}

	m_Int64Stats->operator[](key) = value;
}

BOOL CBaseQualities::InqInt64(STypeInt64 key, __int64 &value)
{
	if (m_Int64Stats)
	{
		const __int64 *pValue = m_Int64Stats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveInt64(STypeInt64 key)
{
	if (m_Int64Stats)
	{
		m_Int64Stats->remove(key);
	}
}

void CBaseQualities::SetFloat(STypeFloat key, double value)
{
	if (!m_FloatStats)
	{
		m_FloatStats = new PackableHashTableWithJson<STypeFloat, double>();
	}

	m_FloatStats->operator[](key) = value;
}

BOOL CBaseQualities::InqFloat(STypeFloat key, double &value, BOOL raw)
{
	if (m_FloatStats)
	{
		const double *pValue = m_FloatStats->lookup(key);

		if (pValue)
		{
			value = *pValue;

			if (!raw)
				EnchantFloat(key, value);

			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveFloat(STypeFloat key)
{
	if (m_FloatStats)
	{
		m_FloatStats->remove(key);
	}
}


void CBaseQualities::SetBool(STypeBool key, int value)
{
	if (!m_BoolStats)
	{
		m_BoolStats = new PackableHashTableWithJson<STypeBool, int>();
	}

	m_BoolStats->operator[](key) = value;
}

BOOL CBaseQualities::InqBool(STypeBool key, int &value)
{
	if (m_BoolStats)
	{
		const int *pValue = m_BoolStats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveBool(STypeBool key)
{
	if (m_BoolStats)
	{
		m_BoolStats->remove(key);
	}
}


void CBaseQualities::SetString(STypeString key, const std::string &value)
{
	if (!m_StringStats)
	{
		m_StringStats = new PackableHashTableWithJson<STypeString, std::string>();
	}

	m_StringStats->operator[](key) = value;
}

BOOL CBaseQualities::InqString(STypeString key, std::string &value)
{
	if (m_StringStats)
	{
		const std::string *pValue = m_StringStats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveString(STypeString key)
{
	if (m_StringStats)
	{
		m_StringStats->remove(key);
	}
}


void CBaseQualities::SetDataID(STypeDID key, DWORD value)
{
	if (!m_DIDStats)
	{
		m_DIDStats = new PackableHashTableWithJson<STypeDID, DWORD>();
	}

	m_DIDStats->operator[](key) = value;
}

BOOL CBaseQualities::InqDataID(STypeDID key, DWORD &value)
{
	if (m_DIDStats)
	{
		const DWORD *pValue = m_DIDStats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveDataID(STypeDID key)
{
	if (m_DIDStats)
	{
		m_DIDStats->remove(key);
	}
}


void CBaseQualities::SetInstanceID(STypeIID key, DWORD value)
{
	if (!m_IIDStats)
	{
		m_IIDStats = new PackableHashTableWithJson<STypeIID, DWORD>();
	}

	m_IIDStats->operator[](key) = value;
}

BOOL CBaseQualities::InqInstanceID(STypeIID key, DWORD &value)
{
	if (m_IIDStats)
	{
		const DWORD *pValue = m_IIDStats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemoveInstanceID(STypeIID key)
{
	if (m_IIDStats)
	{
		m_IIDStats->remove(key);
	}
}

void CBaseQualities::SetPosition(STypePosition key, const Position &value)
{
	if (!m_PositionStats)
	{
		m_PositionStats = new PackableHashTableWithJson<STypePosition, Position>();
	}

	m_PositionStats->operator[](key) = value;
}

BOOL CBaseQualities::InqPosition(STypePosition key, Position &value)
{
	if (m_PositionStats)
	{
		const Position *pValue = m_PositionStats->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

void CBaseQualities::RemovePosition(STypePosition key)
{
	if (m_PositionStats)
	{
		m_PositionStats->remove(key);
	}
}

int CBaseQualities::GetInt(STypeInt key, int defaultValue)
{
	int value = defaultValue;
	InqInt(key, value);
	return value;
}

long long CBaseQualities::GetInt64(STypeInt64 key, long long defaultValue)
{
	long long value = defaultValue;
	InqInt64(key, value);
	return value;
}

BOOL CBaseQualities::GetBool(STypeBool key, BOOL defaultValue)
{
	BOOL value = defaultValue;
	InqBool(key, value);
	return value;
}

double CBaseQualities::GetFloat(STypeFloat key, double defaultValue)
{
	double value = defaultValue;
	InqFloat(key, value);
	return value;
}

std::string CBaseQualities::GetString(STypeString key, std::string defaultValue)
{
	std::string value = defaultValue;
	InqString(key, value);
	return value;
}

DWORD CBaseQualities::GetDID(STypeDID key, DWORD defaultValue)
{
	DWORD value = defaultValue;
	InqDataID(key, value);
	return value;
}

DWORD CBaseQualities::GetIID(STypeIID key, DWORD defaultValue)
{
	DWORD value = defaultValue;
	InqInstanceID(key, value);
	return value;
}

Position CBaseQualities::GetPosition(STypePosition key, const Position &defaultValue)
{
	Position value = defaultValue;
	InqPosition(key, value);
	return value;
}

CACQualities::CACQualities()
{
}

CACQualities::~CACQualities()
{
	Clear();
}

DEFINE_UNPACK(CACQualities)
{
	Clear(); // custom

	if (!CBaseQualities::UnPack(pReader))
		return false;

	DWORD contentFlags = pReader->Read<DWORD>();
	
	SetID(pReader->Read<DWORD>()); // WCID

	if (contentFlags & 1)
	{
		if (!_attribCache)
		{
			_attribCache = new AttributeCache();
		}

		_attribCache->UnPack(pReader);
	}
	else
	{
		SafeDelete(_attribCache);
	}

	if (contentFlags & 2)
	{
		if (!_skillStatsTable)
		{
			_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
		}

		_skillStatsTable->UnPack(pReader);
	}
	else
	{
		SafeDelete(_skillStatsTable);
	}

	if (contentFlags & 0x4)
	{
		if (!_body)
		{
			_body = new Body();
		}

		_body->UnPack(pReader);
	}
	else
	{
		SafeDelete(_body);
	}

	if (contentFlags & 0x100)
	{
		if (!_spell_book)
		{
			_spell_book = new CSpellBook();
		}

		_spell_book->UnPack(pReader);
	}
	else
	{
		SafeDelete(_spell_book);
	}

	if (contentFlags & 0x200)
	{
		if (!_enchantment_reg)
		{
			_enchantment_reg = new CEnchantmentRegistry();
		}

		_enchantment_reg->UnPack(pReader);
	}
	else
	{
		SafeDelete(_enchantment_reg);
	}

	if (contentFlags & 0x8)
	{
		if (!_event_filter)
		{
			_event_filter = new EventFilter();
		}

		_event_filter->UnPack(pReader);
	}
	else
	{
		SafeDelete(_event_filter);
	}

	if (contentFlags & 0x10)
	{
		if (!_emote_table)
		{
			_emote_table = new CEmoteTable();
		}

		_emote_table->UnPack(pReader);
	}
	else
	{
		SafeDelete(_emote_table);
	}

	if (contentFlags & 0x20)
	{
		if (!_create_list)
		{
			_create_list = new PackableListWithJson<CreationProfile>();
		}

		_create_list->UnPack(pReader);
	}
	else
	{
		SafeDelete(_create_list);
	}

	if (contentFlags & 0x40)
	{
		if (!_pageDataList)
		{
			_pageDataList = new PageDataList();
		}

		_pageDataList->UnPack(pReader);
	}
	else
	{
		SafeDelete(_pageDataList);
	}

	if (contentFlags & 0x80)
	{
		if (!_generator_table)
		{
			_generator_table = new GeneratorTable();
		}

		_generator_table->UnPack(pReader);
	}
	else
	{
		SafeDelete(_generator_table);
	}

	if (contentFlags & 0x400)
	{
		if (!_generator_registry)
		{
			_generator_registry = new GeneratorRegistry();
		}

		_generator_registry->UnPack(pReader);
	}
	else
	{
		SafeDelete(_generator_registry);
	}

	if (contentFlags & 0x800)
	{
		if (!_generator_queue)
		{
			_generator_queue = new GeneratorQueue();
		}

		_generator_queue->UnPack(pReader);
	}
	else
	{
		SafeDelete(_generator_queue);
	}

	return true;
}

DEFINE_PACK(CACQualities)
{
	CBaseQualities::Pack(pWriter);

	BinaryWriter content;
	DWORD contentFlags = 0;

	if (_attribCache)
	{
		contentFlags |= 1;
		content.Write(_attribCache);
	}
	if (_skillStatsTable)
	{
		contentFlags |= 2;
		content.Write(_skillStatsTable);
	}
	if (_body)
	{
		contentFlags |= 4;
		content.Write(_body);
	}
	if (_spell_book)
	{
		contentFlags |= 0x100;
		content.Write(_spell_book);
	}
	if (_enchantment_reg)
	{
		contentFlags |= 0x200;
		content.Write(_enchantment_reg);
	}
	if (_event_filter)
	{
		contentFlags |= 8;
		content.Write(_event_filter);
	}
	if (_emote_table)
	{
		contentFlags |= 0x10;
		content.Write(_emote_table);
	}
	if (_create_list)
	{
		contentFlags |= 0x20;
		content.Write(_create_list);
	}
	if (_pageDataList)
	{
		contentFlags |= 0x40;
		content.Write(_pageDataList);
	}
	if (_generator_table)
	{
		contentFlags |= 0x80;
		content.Write(_generator_table);
	}
	if (_generator_registry)
	{
		contentFlags |= 0x400;
		content.Write(_generator_registry);
	}
	if (_generator_queue)
	{
		contentFlags |= 0x800;
		content.Write(_generator_queue);
	}

	pWriter->Write<DWORD>(contentFlags);
	pWriter->Write<DWORD>(GetID());
	pWriter->Write(&content);
}

DEFINE_UNPACK_JSON(CACQualities)
{
	Clear(); // custom

	if (!CBaseQualities::UnPackJson(reader))
		return false;

	SetID(reader["wcid"]);

	if (reader.find("attributes") != reader.end())
	{
		if (!_attribCache)
		{
			_attribCache = new AttributeCache();
		}

		_attribCache->UnPackJson(reader["attributes"]);
	}
	else
	{
		SafeDelete(_attribCache);
	}

	if (reader.find("skills") != reader.end())
	{
		if (!_skillStatsTable)
		{
			_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
		}

		_skillStatsTable->UnPackJson(reader["skills"]);
	}
	else
	{
		SafeDelete(_skillStatsTable);
	}

	if (reader.find("body") != reader.end())
	{
		if (!_body)
		{
			_body = new Body();
		}

		_body->UnPackJson(reader["body"]);
	}
	else
	{
		SafeDelete(_body);
	}

	if (reader.find("spellbook") != reader.end())
	{
		if (!_spell_book)
		{
			_spell_book = new CSpellBook();
		}

		_spell_book->UnPackJson(reader["spellbook"]);
	}
	else
	{
		SafeDelete(_spell_book);
	}

	if (reader.find("enchantments") != reader.end())
	{
		if (!_enchantment_reg)
		{
			_enchantment_reg = new CEnchantmentRegistry();
		}

		_enchantment_reg->UnPackJson(reader["enchantments"]);
	}
	else
	{
		SafeDelete(_enchantment_reg);
	}

	if (reader.find("eventFilter") != reader.end())
	{
		if (!_event_filter)
		{
			_event_filter = new EventFilter();
		}

		_event_filter->UnPackJson(reader["eventFilter"]);
	}
	else
	{
		SafeDelete(_event_filter);
	}

	if (reader.find("emoteTable") != reader.end())
	{
		if (!_emote_table)
		{
			_emote_table = new CEmoteTable();
		}

		_emote_table->UnPackJson(reader["emoteTable"]);
	}
	else
	{
		SafeDelete(_emote_table);
	}

	if (reader.find("createList") != reader.end())
	{
		if (!_create_list)
		{
			_create_list = new PackableListWithJson<CreationProfile>();
		}

		_create_list->UnPackJson(reader["createList"]);
	}
	else
	{
		SafeDelete(_create_list);
	}

	if (reader.find("pageDataList") != reader.end())
	{
		if (!_pageDataList)
		{
			_pageDataList = new PageDataList();
		}

		_pageDataList->UnPackJson(reader["pageDataList"]);
	}
	else
	{
		SafeDelete(_pageDataList);
	}

	if (reader.find("generatorTable") != reader.end())
	{
		if (!_generator_table)
		{
			_generator_table = new GeneratorTable();
		}

		_generator_table->UnPackJson(reader["generatorTable"]);
	}
	else
	{
		SafeDelete(_generator_table);
	}

	if (reader.find("generatorRegistry") != reader.end())
	{
		if (!_generator_registry)
		{
			_generator_registry = new GeneratorRegistry();
		}

		_generator_registry->UnPackJson(reader["generatorRegistry"]);
	}
	else
	{
		SafeDelete(_generator_registry);
	}

	if (reader.find("generatorQueue") != reader.end())
	{
		if (!_generator_queue)
		{
			_generator_queue = new GeneratorQueue();
		}

		_generator_queue->UnPackJson(reader["generatorQueue"]);
	}
	else
	{
		SafeDelete(_generator_queue);
	}

	return true;
}

DEFINE_PACK_JSON(CACQualities)
{
	CBaseQualities::PackJson(writer);

	writer["wcid"] = GetID();

	if (_attribCache)
	{
		_attribCache->PackJson(writer["attributes"]);
	}

	if (_skillStatsTable)
	{
		_skillStatsTable->PackJson(writer["skills"]);
	}

	if (_body)
	{
		_body->PackJson(writer["body"]);
	}

	if (_spell_book)
	{
		_spell_book->PackJson(writer["spellbook"]);
	}

	if (_enchantment_reg)
	{
		_enchantment_reg->PackJson(writer["enchantments"]);
	}

	if (_event_filter)
	{
		_event_filter->PackJson(writer["eventFilter"]);
	}

	if (_emote_table)
	{
		_emote_table->PackJson(writer["emoteTable"]);
	}

	if (_create_list)
	{
		_create_list->PackJson(writer["createList"]);
	}

	if (_pageDataList)
	{
		_pageDataList->PackJson(writer["pageDataList"]);
	}

	if (_generator_table)
	{
		_generator_table->PackJson(writer["generatorTable"]);
	}

	if (_generator_registry)
	{
		_generator_registry->PackJson(writer["generatorRegistry"]);
	}

	if (_generator_queue)
	{
		_generator_queue->PackJson(writer["generatorQueue"]);
	}
}

void CACQualities::Clear()
{
	SafeDelete(_attribCache);
	SafeDelete(_skillStatsTable);
	SafeDelete(_body);
	SafeDelete(_spell_book);
	SafeDelete(_enchantment_reg);
	SafeDelete(_event_filter);
	SafeDelete(_emote_table);
	SafeDelete(_create_list);
	SafeDelete(_pageDataList);
	SafeDelete(_generator_table);
	SafeDelete(_generator_registry);
	SafeDelete(_generator_queue);
}

BOOL CACQualities::EnchantInt(STypeInt key, int &value, BOOL allow_negative)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantInt(key, &value, allow_negative);

	return FALSE;
}

BOOL CACQualities::EnchantFloat(STypeFloat key, double &value)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantFloat(key, &value);

	return FALSE;
}

BOOL CACQualities::EnchantAttribute(STypeAttribute key, DWORD &value)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantAttribute(key, (unsigned int *)&value);

	return FALSE;
}

BOOL CACQualities::EnchantBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int *val)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantBodyArmorValue(part, dt, val);

	return FALSE;
}

BOOL CACQualities::SetAttribute(STypeAttribute key, DWORD initialValue)
{
	if (!_attribCache)
	{
		_attribCache = new AttributeCache();
	}

	return _attribCache->SetAttribute(key, initialValue);
}

BOOL CACQualities::SetAttribute(STypeAttribute key, const Attribute &value)
{
	if (!_attribCache)
	{
		_attribCache = new AttributeCache();
	}

	return _attribCache->SetAttribute(key, value);
}

BOOL CACQualities::InqAttribute(STypeAttribute key, Attribute &pvalue)
{
	if (_attribCache)
	{
		return _attribCache->InqAttribute(key, pvalue);
	}

	return FALSE;
}

BOOL CACQualities::InqAttribute(STypeAttribute key, DWORD &value, BOOL raw)
{
	if (_attribCache)
	{
		if (_attribCache->InqAttribute(key, value))
		{
			if (!raw)
				EnchantAttribute(key, value);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CACQualities::BoundsCheck(STypeAttribute2nd key, DWORD &val, DWORD &max)
{
	switch (key)
	{
	case HEALTH_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		{
			if (val < 0)
			{
				val = 0;
				return TRUE;
			}

			if (InqAttribute2nd((STypeAttribute2nd)(int)(key - 1), max, FALSE))
			{
				if (val > max)
					val = max;

				return TRUE;
			}

			return FALSE;
		}
	}

	return TRUE;
}

BOOL CACQualities::SetAttribute2nd(STypeAttribute2nd key, DWORD value, DWORD &result, DWORD &max)
{
	if (!_attribCache)
	{
		_attribCache = new AttributeCache();
	}

	result = value;
	if (!BoundsCheck(key, result, max))
		return FALSE;

	_attribCache->SetAttribute2nd(key, result);
	return TRUE;
}

BOOL CACQualities::SetAttribute2nd(STypeAttribute2nd key, DWORD value)
{
	if (!_attribCache)
	{
		_attribCache = new AttributeCache();
	}

	DWORD resultValue;
	DWORD maxValue;
	return SetAttribute2nd(key, value, resultValue, maxValue);
}

BOOL CACQualities::SetAttribute2nd(STypeAttribute2nd key, const SecondaryAttribute &value)
{
	if (!_attribCache)
	{
		_attribCache = new AttributeCache();
	}

	SecondaryAttribute newValue = value;

	switch (key)
	{
	case HEALTH_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:

		DWORD maxValue = 0;
		if (!_attribCache->InqAttribute2nd((STypeAttribute2nd)((int)key - 1), maxValue))
			return FALSE;

		if (newValue._current > maxValue)
			newValue._current = maxValue;
		
		break;
	}

	return _attribCache->SetAttribute2nd(key, newValue);
}

BOOL CACQualities::InqAttribute2nd(STypeAttribute2nd key, SecondaryAttribute &value)
{
	if (_attribCache)
	{
		return _attribCache->InqAttribute2nd(key, value);
	}

	return FALSE;
}

BOOL CACQualities::InqAttribute2ndBaseLevel(STypeAttribute2nd key, DWORD &value, BOOL raw)
{
	Attribute2ndTable *pTable = CachedAttribute2ndTable; // Attribute2ndTable::Get(0x0E000003);
	
	Attribute2ndBase *base = NULL;

	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
		base = &pTable->_max_health;
		break;
	case MAX_STAMINA_ATTRIBUTE_2ND:
		base = &pTable->_max_stamina;
		break;
	case MAX_MANA_ATTRIBUTE_2ND:
		base = &pTable->_max_mana;
		break;

	default:
		// Attribute2ndTable::Release(pTable);
		return FALSE;
	}

	DWORD attr1val = 0;
	DWORD attr2val = 0;

	if (base->_formula._attr1 != 0)
		InqAttribute((STypeAttribute)base->_formula._attr1, attr1val, raw);
	if (base->_formula._attr2 != 0)
		InqAttribute((STypeAttribute)base->_formula._attr2, attr2val, raw);

	base->_formula.Calculate(attr1val, attr2val, value);

	// Attribute2ndTable::Release(pTable);
	return TRUE;
}

BOOL CACQualities::InqAttribute2nd(STypeAttribute2nd key, DWORD &value, BOOL raw)
{
	DWORD baseLevel = 0;

	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
	case MAX_STAMINA_ATTRIBUTE_2ND:
	case MAX_MANA_ATTRIBUTE_2ND:
		if (!InqAttribute2ndBaseLevel(key, baseLevel, raw))
		{
			return FALSE;
		}

		break;
	}

	if (key == MAX_HEALTH_ATTRIBUTE_2ND)
	{
		int gearHealth = 0;
		if (InqInt(GEAR_MAX_HEALTH_INT, gearHealth, FALSE, FALSE))
			baseLevel += (unsigned) gearHealth;
	}

	if (_attribCache && _attribCache->InqAttribute2nd(key, value))
	{
		value += baseLevel;
	}
	else
	{
		if (!baseLevel)
			return FALSE;

		value = baseLevel;
	}

	if (!raw)
	{
		EnchantAttribute2nd(key, value);
	}

	return TRUE;
}

BOOL CACQualities::EnchantAttribute2nd(STypeAttribute2nd key, DWORD &value)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantAttribute2nd(key, (unsigned int *)&value);

	return FALSE;
}

BOOL CACQualities::EnchantSkill(STypeSkill key, DWORD &value)
{
	if (_enchantment_reg)
		return _enchantment_reg->EnchantSkill(key, (int *)&value);

	return FALSE;
}

BOOL CACQualities::InqSkill(STypeSkill key, DWORD &value, BOOL raw)
{
	if (!InqSkillBaseLevel(key, value, raw))
		return FALSE;

	if (_skillStatsTable)
	{
		const Skill *pSkill = _skillStatsTable->lookup(key);
		if (pSkill)
			value += pSkill->_init_level + pSkill->_level_from_pp;
	}

	int augmentAll = 0;
	if (InqInt(LUM_AUG_ALL_SKILLS_INT, augmentAll))
	{
		if (augmentAll > 0)
			value += augmentAll;
	}

	int augmentType = 0;
	// oh jesus here we go
	switch (key)
	{
	case CREATURE_ENCHANTMENT_SKILL: // case 0
	case ITEM_ENCHANTMENT_SKILL:
	case LIFE_MAGIC_SKILL:
	case WAR_MAGIC_SKILL:
	case VOID_MAGIC_SKILL:
		InqInt(AUGMENTATION_SKILLED_MAGIC_INT, augmentType);
		break;

	case AXE_SKILL: // case 1
	case DAGGER_SKILL:
	case MACE_SKILL:
	case SPEAR_SKILL:
	case STAFF_SKILL:
	case SWORD_SKILL:
	case UNARMED_COMBAT_SKILL:
		InqInt(AUGMENTATION_SKILLED_MELEE_INT, augmentType);
		break;

	case BOW_SKILL:
	case CROSSBOW_SKILL:
	case THROWN_WEAPON_SKILL:
		InqInt(AUGMENTATION_SKILLED_MISSILE_INT, augmentType);
		break;
	}

	if (augmentType > 0)
		value += 10;

	if (!raw)
	{
		EnchantSkill(key, value);

		int valCheck = 0;
		if (InqInt(AUGMENTATION_JACK_OF_ALL_TRADES_INT, valCheck, 0, 0) && valCheck > 0)
			value += 5;

		int lum_aug = 0;
		InqInt(LUM_AUG_SKILLED_SPEC_INT, lum_aug, 0, 0);
		if (lum_aug > 0)
		{
			if (_skillStatsTable)
			{
				const Skill *skill = _skillStatsTable->lookup(key);
				if (skill && skill->_sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
					value += 2 * lum_aug;
			}
		}
	}

	return TRUE;
}

BOOL CACQualities::InqSkill(STypeSkill key, Skill &value)
{
	if (_skillStatsTable)
	{
		const Skill *pValue = _skillStatsTable->lookup(key);

		if (pValue)
		{
			value = *pValue;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CACQualities::InqSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS &value)
{
	if (_skillStatsTable)
	{
		const Skill *pValue = _skillStatsTable->lookup(key);

		if (pValue)
		{
			value = pValue->_sac;
			return TRUE;
		}
	}

	return FALSE;
}

void CACQualities::SetSkill(STypeSkill key, const Skill &val)
{
	if (!_skillStatsTable)
	{
		_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
	}

	_skillStatsTable->operator[](key) = val;
}

void CACQualities::SetSkillLevel(STypeSkill key, DWORD val)
{
	if (!_skillStatsTable)
	{
		_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
	}

	_skillStatsTable->operator[](key)._init_level = val;
}

void CACQualities::SetSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS val)
{
	if (!_skillStatsTable)
	{
		_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
	}

	_skillStatsTable->operator[](key).SetSkillAdvancementClass(val);
}

BOOL CACQualities::InqSkillLevel(STypeSkill key, DWORD &value)
{
	if (_skillStatsTable)
	{
		const Skill *pValue = _skillStatsTable->lookup(key);

		if (pValue)
		{
			value = pValue->_init_level + pValue->_level_from_pp;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CACQualities::InqSkillBaseLevel(STypeSkill key, DWORD &value, BOOL raw)
{
	SkillTable *pSkillTable = SkillSystem::GetSkillTable();

	if (!pSkillTable)
		return FALSE;

	const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
	if (!pSkillBase)
		return FALSE;

	SKILL_ADVANCEMENT_CLASS sac_ = UNTRAINED_SKILL_ADVANCEMENT_CLASS;
	if (_skillStatsTable)
	{
		const Skill *pSkill = _skillStatsTable->lookup(key);
		if (pSkill)
			sac_ = pSkill->_sac;
	}

	if (sac_ < pSkillBase->_min_level)
	{
		value = 0;
		return TRUE;
	}

	if (sac_ < pSkillBase->_min_level)
	{
		value = 0;
		return TRUE;
	}

	DWORD attr1value = 0, attr2value = 0;

	if (pSkillBase->_formula._attr1)
	{
		InqAttribute((STypeAttribute)pSkillBase->_formula._attr1, attr1value, raw);
	}
	if (pSkillBase->_formula._attr2)
	{
		InqAttribute((STypeAttribute)pSkillBase->_formula._attr2, attr2value, raw);
	}

	return pSkillBase->_formula.Calculate(attr1value, attr2value, value);
}

BOOL CACQualities::InqBodyArmorValue(unsigned int part, DAMAGE_TYPE dt, int &value, BOOL raw)
{
	int baseArmor1 = 0;
	if (!raw)
		EnchantBodyArmorValue(part, DAMAGE_TYPE::UNDEF_DAMAGE_TYPE, &baseArmor1);

	if (!_body)
	{
		value += baseArmor1;

		switch (dt)
		{
		case DAMAGE_TYPE::SLASH_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_SLASH_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::PIERCE_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_PIERCE_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_BLUDGEON_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::FIRE_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_FIRE_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::COLD_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_COLD_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::ACID_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_ACID_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::ELECTRIC_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_ELECTRIC_FLOAT, 1.0);
			return TRUE;

		case DAMAGE_TYPE::NETHER_DAMAGE_TYPE:
			value *= GetFloat(ARMOR_MOD_VS_NETHER_FLOAT, 1.0);
			return TRUE;
		}

		return TRUE;
	}
	else
	{
		BodyPart *pPart = _body->_body_part_table.lookup(part);

		if (pPart)
		{
			int baseArmor2 = pPart->_acache._base_armor;
			if (!raw)
				EnchantBodyArmorValue(part, DAMAGE_TYPE::BASE_DAMAGE_TYPE, &baseArmor2);

			int baseArmor = baseArmor1; // + baseArmor2;

			bool bAddBaseArmor = true;

			switch (dt)
			{
			case DAMAGE_TYPE::SLASH_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_slash;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::PIERCE_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_pierce;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_bludgeon;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::FIRE_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_fire;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::COLD_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_cold;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::ACID_DAMAGE_TYPE:

				value = pPart->_acache._armor_vs_acid;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::ELECTRIC_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_electric;
				if (!raw)
					EnchantBodyArmorValue(part, dt, &value);

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;

			case DAMAGE_TYPE::NETHER_DAMAGE_TYPE:
				value = pPart->_acache._armor_vs_nether;
				if (!raw)
				{
					EnchantBodyArmorValue(part, dt, &value);
				}

				if (bAddBaseArmor)
					value += baseArmor;

				return TRUE;
			}
		}
	}

	return FALSE;
}

void CACQualities::AddSpell(DWORD spellid)
{
	if (!_spell_book)
	{
		_spell_book = new CSpellBook();
	}
	
	SpellBookPage page;
	page._casting_likelihood = 0;
	_spell_book->AddSpell(spellid, page);
}

BOOL CACQualities::InqLoad(float &burden)
{
	DWORD strength = 10;
	InqAttribute(STRENGTH_ATTRIBUTE, strength, FALSE);

	int encumb_augmentations = 0;
	InqInt(AUGMENTATION_INCREASED_CARRYING_CAPACITY_INT, encumb_augmentations, FALSE, FALSE);

	int capacity = EncumbranceSystem::EncumbranceCapacity(strength, encumb_augmentations);

	int encumb_val = 0;
	InqInt(ENCUMB_VAL_INT, encumb_val, FALSE, FALSE);

	burden = EncumbranceSystem::Load(capacity, encumb_val);
	return TRUE;

	// burden = 1.0f;
	// return TRUE;
}

BOOL CACQualities::InqJumpVelocity(float extent, float &v_z)
{
	float load = 1.0f;
	if (!InqLoad(load))
		return FALSE;

	DWORD jumpskill = 1;
	DWORD stamina = 0;
	if (_attribCache && _attribCache->InqAttribute2nd(STAMINA_ATTRIBUTE_2ND, stamina))
	{
		EnchantAttribute2nd(STAMINA_ATTRIBUTE_2ND, stamina);

		if (InqSkillBaseLevel(JUMP_SKILL, jumpskill, FALSE))
		{
			DWORD increase = 0;
			if (InqSkillLevel(JUMP_SKILL, increase))
				jumpskill += increase;

			int augment = 0;
			if (InqInt(LUM_AUG_ALL_SKILLS_INT, augment))
				jumpskill += augment;

			EnchantSkill(JUMP_SKILL, jumpskill);

			int val = 0;
			if (InqInt(AUGMENTATION_JACK_OF_ALL_TRADES_INT, val) && val > 0)
				jumpskill += 5;
			
			int key = 0;
			InqInt(LUM_AUG_SKILLED_SPEC_INT, key);

			if (key > 0)
			{
				SKILL_ADVANCEMENT_CLASS sac;
				if (InqSkillAdvancementClass(JUMP_SKILL, sac))
				{
					if (sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
						jumpskill += 2 * key;
				}
			}

			if (!stamina)
				jumpskill = 0;

			double jumpHeight = MovementSystem::GetJumpHeight(load, jumpskill, extent, 1.0);
			v_z = sqrt(jumpHeight * 19.6);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CACQualities::InqRunRate(float &rate)
{
	float burden = 1.0f;
	if (!InqLoad(burden))
		return FALSE;

	if (!_attribCache)
		return FALSE;

	DWORD stamina;
	if (!_attribCache->InqAttribute2nd(STAMINA_ATTRIBUTE_2ND, stamina))
		return FALSE;

	EnchantAttribute2nd(STAMINA_ATTRIBUTE_2ND, stamina);

	DWORD runskill;
	if (!InqSkill(RUN_SKILL, runskill, FALSE))
		return FALSE;

	rate = MovementSystem::GetRunRate(burden, runskill, 1.0f);	
	return TRUE;
}

BOOL CACQualities::CanJump(float extent)
{
	float load = 0.0f;
	if (!InqLoad(load))
		return FALSE;
	if (load >= 2.0f)
		return FALSE;

	return TRUE;
}

BOOL CACQualities::JumpStaminaCost(float extent, long & cost)
{
	float load = 0.0f;
	if (!InqLoad(load))
		return FALSE;

	BOOL bRecentAttack = FALSE;
	int pkStatus = Unprotected_PKStatus;
	InqInt(PLAYER_KILLER_STATUS_INT, pkStatus);
	if (pkStatus == PK_PKStatus || pkStatus == PKLite_PKStatus)
	{
		double timeLastPKAttack;
		if (InqFloat(LAST_PK_ATTACK_TIMESTAMP_FLOAT, timeLastPKAttack))
		{
			if (timeLastPKAttack + 20.0 > Timer::cur_time)
				bRecentAttack = TRUE;
		}
	}

	cost = MovementSystem::JumpStaminaCost(extent, load, bRecentAttack);
	return TRUE;
}

BOOL CACQualities::HasSpellBook()
{
	return _spell_book != NULL;
}

BOOL CACQualities::IsSpellKnown(const unsigned int spell)
{
	return _spell_book ? _spell_book->Exists(spell) : NULL;
}

BOOL  CACQualities::UpdateEnchantment(Enchantment *to_update)
{
	if (!_enchantment_reg)
		_enchantment_reg = new CEnchantmentRegistry();

	return _enchantment_reg->UpdateEnchantment(to_update);
}

void AttributeCache::CopyFrom(AttributeCache *pOther)
{
	Clear();

	if (pOther->_strength) 
	{
		_strength = new Attribute();
		*_strength = *pOther->_strength;
	}
	if (pOther->_endurance)
	{
		_endurance = new Attribute();
		*_endurance = *pOther->_endurance;
	}
	if (pOther->_quickness)
	{
		_quickness = new Attribute();
		*_quickness = *pOther->_quickness;
	}
	if (pOther->_coordination)
	{
		_coordination = new Attribute();
		*_coordination = *pOther->_coordination;
	}
	if (pOther->_focus)
	{
		_focus = new Attribute();
		*_focus = *pOther->_focus;
	}
	if (pOther->_self)
	{
		_self = new Attribute();
		*_self = *pOther->_self;
	}
	if (pOther->_health)
	{
		_health = new SecondaryAttribute();
		*_health = *pOther->_health;
	}
	if (pOther->_stamina)
	{
		_stamina = new SecondaryAttribute();
		*_stamina = *pOther->_stamina;
	}
	if (pOther->_mana)
	{
		_mana = new SecondaryAttribute();
		*_mana = *pOther->_mana;
	}
}

/*
STypeInt IntStatKeyEnumUnPacker(const std::string &key)
{
	static std::unordered_map<std::string, STypeInt> stats( 
	{
		{ "test1", STypeInt::UNDEF_INT },
		{ "test2", STypeInt::LEVEL_INT }
	}
	);

	auto entry = stats.find(key);

	if (entry != stats.end())
		return entry->second;

	return (STypeInt) atoi(key.c_str());
}

std::string IntStatKeyEnumPacker(const STypeInt &key)
{
	static std::unordered_map<STypeInt, std::string> stats(
	{
		{ STypeInt::UNDEF_INT, "test1" },
		{ STypeInt::LEVEL_INT, "test2" }
	}
	);

	auto entry = stats.find(key);

	if (entry != stats.end())
		return entry->second;

	char numStr[32];
	_itoa((int)key, numStr, 10);

	return std::string(numStr);
}
*/

void CBaseQualities::CopyFrom(CBaseQualities *pOther)
{
	Clear();

	m_WeenieType = pOther->m_WeenieType;

	if (pOther->m_IntStats)
	{
		// m_IntStats = new PackableHashTableWithEnumConverterJson<STypeInt, int>(IntStatKeyEnumUnPacker, IntStatKeyEnumPacker);
		m_IntStats = new PackableHashTableWithJson<STypeInt, int>();
		*m_IntStats = *pOther->m_IntStats;
	}
	if (pOther->m_Int64Stats)
	{
		m_Int64Stats = new PackableHashTableWithJson<STypeInt64, long long>();
		*m_Int64Stats = *pOther->m_Int64Stats;
	}
	if (pOther->m_BoolStats)
	{
		m_BoolStats = new PackableHashTableWithJson<STypeBool, int>();
		*m_BoolStats = *pOther->m_BoolStats;
	}
	if (pOther->m_FloatStats)
	{
		m_FloatStats = new PackableHashTableWithJson<STypeFloat, double>();
		*m_FloatStats = *pOther->m_FloatStats;
	}
	if (pOther->m_StringStats)
	{
		m_StringStats = new PackableHashTableWithJson<STypeString, std::string>();
		*m_StringStats = *pOther->m_StringStats;
	}	
	if (pOther->m_DIDStats)
	{
		m_DIDStats = new PackableHashTableWithJson<STypeDID, DWORD>();
		*m_DIDStats = *pOther->m_DIDStats;
	}
	if (pOther->m_IIDStats)
	{
		m_IIDStats = new PackableHashTableWithJson<STypeIID, DWORD>();
		*m_IIDStats = *pOther->m_IIDStats;
	}
	if (pOther->m_PositionStats)
	{
		m_PositionStats = new PackableHashTableWithJson<STypePosition, Position>();
		*m_PositionStats = *pOther->m_PositionStats;
	}
}

void CACQualities::CopyFrom(CACQualities *pOther)
{
	Clear();

	CBaseQualities::CopyFrom(pOther);

	SetID(pOther->GetID());

	if (pOther->_attribCache)
	{
		_attribCache = new AttributeCache();
		_attribCache->CopyFrom(pOther->_attribCache);
	}

	if (pOther->_skillStatsTable)
	{
		_skillStatsTable = new PackableHashTableWithJson<STypeSkill, Skill>();
		*_skillStatsTable = *pOther->_skillStatsTable;
	}

#define COPY_BY_PACKING_SLOW(className, varName) \
		if (pOther->varName) { \
			varName = new className(); \
			BinaryWriter w; \
			pOther->varName->Pack(&w); \
			BinaryReader r(w.GetData(), w.GetSize()); \
			varName->UnPack(&r); \
		}
	
	COPY_BY_PACKING_SLOW(Body, _body);

	if (pOther->_spell_book)
	{
		_spell_book = new CSpellBook();
		*_spell_book = *pOther->_spell_book;
	}
	
	COPY_BY_PACKING_SLOW(CEnchantmentRegistry, _enchantment_reg);
	COPY_BY_PACKING_SLOW(EventFilter, _event_filter);
	COPY_BY_PACKING_SLOW(CEmoteTable, _emote_table);
	COPY_BY_PACKING_SLOW(PackableListWithJson<CreationProfile>, _create_list);

	if (pOther->_pageDataList)
	{
		_pageDataList = new PageDataList();
		*_pageDataList = *pOther->_pageDataList;
	}
	if (pOther->_generator_table)
	{
		_generator_table = new GeneratorTable();
		*_generator_table = *pOther->_generator_table;
	}
	if (pOther->_generator_registry)
	{
		_generator_registry = new GeneratorRegistry();
		*_generator_registry = *pOther->_generator_registry;
	}
	if (pOther->_generator_queue)
	{
		_generator_queue = new GeneratorQueue();
		*_generator_queue = *pOther->_generator_queue;
	}
}

BOOL Enchantment::Enchant(EnchantedQualityDetails *value)
{
	BOOL bEnchanted = FALSE;

	if (!(_smod.type & Additive_EnchantmentType))
	{
		if (_smod.type & Multiplicative_EnchantmentType)
		{
			if (_smod.val <= 1.0)
			{
				value->valueDecreasingMultiplier = _smod.val * value->valueDecreasingMultiplier;
				bEnchanted = TRUE;
			}
			else
			{
				value->valueIncreasingMultiplier = _smod.val * value->valueIncreasingMultiplier;
				bEnchanted = TRUE;
			}
		}
	}
	else if (_smod.val <= 0.0)
	{
		value->valueDecreasingAdditive = _smod.val + value->valueDecreasingAdditive;
		bEnchanted = TRUE;
	}
	else
	{
		value->valueIncreasingAdditive = _smod.val + value->valueIncreasingAdditive;
		bEnchanted = TRUE;
	}

	return bEnchanted;
}

BOOL CEnchantmentRegistry::Enchant(PackableListWithJson<Enchantment> *affecting, EnchantedQualityDetails *val)
{
	BOOL bEnchanted = FALSE;

	for (auto &entry : *affecting)
	{
		if (entry.Enchant(val))
			bEnchanted = TRUE;
	}

	return bEnchanted;
}

BOOL CEnchantmentRegistry::GetFloatEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val)
{
	ACQualityFilter *filter = CachedEnchantableFilter; // Enchantable
	if (!filter)
		return FALSE;
	if (!filter->QueryFloat(stype))
		return TRUE;
	
	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, Float_EnchantmentType, stype, &affecting);
	CullEnchantmentsFromList(_add_list, Float_EnchantmentType, stype, &affecting);
	Enchant(&affecting, val);
	return TRUE;
}

BOOL CEnchantmentRegistry::GetIntEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val) // custom, implied
{
	ACQualityFilter *filter = CachedEnchantableFilter; // Enchantable
	if (!filter)
		return FALSE;
	if (!filter->QueryInt(stype))
		return TRUE;

	PackableListWithJson<Enchantment> affecting;
	CullEnchantmentsFromList(_mult_list, Int_EnchantmentType, stype, &affecting);
	CullEnchantmentsFromList(_add_list, Int_EnchantmentType, stype, &affecting);
	Enchant(&affecting, val);
	return TRUE;
}

BOOL CACQualities::GetIntEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val) // custom, implied
{
	if (_enchantment_reg)
	{
		return _enchantment_reg->GetIntEnchantmentDetails(stype, val);
	}

	return FALSE;
}

BOOL CACQualities::GetFloatEnchantmentDetails(unsigned int stype, EnchantedQualityDetails *val)
{
	if (_enchantment_reg)
	{
		return _enchantment_reg->GetFloatEnchantmentDetails(stype, val);
	}

	return FALSE;
}