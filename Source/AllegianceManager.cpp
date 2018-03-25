
#include "StdAfx.h"
#include "AllegianceManager.h"
#include "World.h"
#include "Player.h"
#include "DatabaseIO.h"
#include "ChatMsgs.h"

#define RELEASE_ASSERT(x) if (!(x)) DebugBreak();

DEFINE_PACK(AllegianceInfo)
{
	_info.Pack(pWriter);
}

DEFINE_UNPACK(AllegianceInfo)
{
	_info.UnPack(pReader);
	return true;
}

AllegianceTreeNode::AllegianceTreeNode()
{
}

AllegianceTreeNode::~AllegianceTreeNode()
{
	for (auto &entry : _vassals)
		delete entry.second;
	_vassals.clear();
}

AllegianceTreeNode *AllegianceTreeNode::FindCharByNameRecursivelySlow(const std::string &charName)
{
	const char *targetName = charName.c_str();
	if (targetName[0] == '+')
		targetName++;

	const char *compareName = _charName.c_str();
	if (compareName[0] == '+')
		compareName++;

	if (!_stricmp(targetName, compareName))
	{
		return this;
	}

	AllegianceTreeNode *node = NULL;
	for (auto &entry : _vassals)
	{
		if (node = entry.second->FindCharByNameRecursivelySlow(charName))
			break;
	}

	return node;
}

void AllegianceTreeNode::FillAllegianceNode(AllegianceNode *node)
{
	node->_data._rank = _rank;
	node->_data._level = _level;
	node->_data._cp_cached = _cp_cached;
	node->_data._cp_tithed = _cp_tithed;
	node->_data._gender = _gender;
	node->_data._hg = _hg;
	node->_data._leadership = _leadership;
	node->_data._loyalty = _loyalty;
	node->_data._name = _charName;
	node->_data._id = _charID;
}

void AllegianceTreeNode::UpdateWithWeenie(CWeenieObject *weenie)
{
	_charID = weenie->GetID();
	_charName = weenie->GetName();	
	_hg = (HeritageGroup) weenie->InqIntQuality(HERITAGE_GROUP_INT, Invalid_HeritageGroup);
	_gender = (Gender)weenie->InqIntQuality(GENDER_INT, Invalid_Gender);
	_level = weenie->InqIntQuality(LEVEL_INT, 1);
	_leadership = 0;
	weenie->m_Qualities.InqSkill(LEADERSHIP_SKILL, *(DWORD *)&_leadership, FALSE);
	_loyalty = 0;
	weenie->m_Qualities.InqSkill(LOYALTY_SKILL, *(DWORD *)&_loyalty, FALSE);
}

DEFINE_PACK(AllegianceTreeNode)
{
	pWriter->Write<DWORD>(1); // version
	pWriter->Write<DWORD>(_charID);
	pWriter->WriteString(_charName);

	pWriter->Write<DWORD>(_monarchID);
	pWriter->Write<DWORD>(_patronID);
	pWriter->Write<int>(_hg);
	pWriter->Write<int>(_gender);
	pWriter->Write<DWORD>(_rank);
	pWriter->Write<DWORD>(_level);
	pWriter->Write<DWORD>(_leadership);
	pWriter->Write<DWORD>(_loyalty);
	pWriter->Write<DWORD>(_numFollowers);
	pWriter->Write<DWORD64>(_cp_cached);
	pWriter->Write<DWORD64>(_cp_tithed);
	pWriter->Write<DWORD64>(_cp_pool_to_unload);

	pWriter->Write<DWORD>(_vassals.size());
	for (auto &entry : _vassals)
		entry.second->Pack(pWriter);
}

DEFINE_UNPACK(AllegianceTreeNode)
{
	pReader->Read<DWORD>(); // version
	_charID = pReader->Read<DWORD>();
	_charName = pReader->ReadString();

	_monarchID = pReader->Read<DWORD>();
	_patronID = pReader->Read<DWORD>();
	_hg = (HeritageGroup)pReader->Read<int>();
	_gender = (Gender)pReader->Read<int>();
	_rank = pReader->Read<DWORD>();
	_level = pReader->Read<DWORD>();
	_leadership = pReader->Read<DWORD>();
	_loyalty = pReader->Read<DWORD>();
	_numFollowers = pReader->Read<DWORD>();
	_cp_cached = pReader->Read<DWORD64>();
	_cp_tithed = pReader->Read<DWORD64>();
	_cp_pool_to_unload = pReader->Read<DWORD64>();

	DWORD numVassals = pReader->Read<DWORD>();
	for (DWORD i = 0; i < numVassals; i++)
	{
		AllegianceTreeNode *node = new AllegianceTreeNode();
		node->UnPack(pReader);
		_vassals[node->_charID] = node;

		assert(node->_charID >= 0x50000000 && node->_charID < 0x70000000);
	}

	assert(_charID >= 0x50000000 && _charID < 0x70000000);
	return true;
}

AllegianceManager::AllegianceManager()
{
	Load();
}

AllegianceManager::~AllegianceManager()
{
	Save();

	for (auto &entry : _monarchs)
		delete entry.second;
	_monarchs.clear();
	_directNodes.clear();

	for (auto &entry : _allegInfos)
		delete entry.second;
	_allegInfos.clear();
}

void AllegianceManager::Load()
{
	void *data = NULL;
	DWORD length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_ALLEGIANCE_DATA, &data, &length))
	{
		BinaryReader reader(data, length);
		UnPack(&reader);
	}

	m_NextSave = Timer::cur_time + 300.0; // every 5 minutes save
}

void AllegianceManager::Save()
{
	BinaryWriter data;
	Pack(&data);
	g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_ALLEGIANCE_DATA, data.GetData(), data.GetSize());
}

void AllegianceManager::Tick()
{
	if (m_NextSave <= Timer::cur_time)
	{
		Save();
		m_NextSave = Timer::cur_time + 300.0; // every 5 minutes save
	}
}

DEFINE_PACK(AllegianceManager)
{
	pWriter->Write<DWORD>(1); // version
	pWriter->Write<DWORD>(_monarchs.size());

	for (auto &entry : _monarchs)
		entry.second->Pack(pWriter);

	pWriter->Write<DWORD>(_allegInfos.size());

	for (auto &entry : _allegInfos)
	{
		pWriter->Write<DWORD>(entry.first);
		entry.second->Pack(pWriter);
	}
}

DEFINE_UNPACK(AllegianceManager)
{
	DWORD version = pReader->Read<DWORD>();
	DWORD numMonarchs = pReader->Read<DWORD>();

	for (DWORD i = 0; i < numMonarchs; i++)
	{
		AllegianceTreeNode *node = new AllegianceTreeNode();
		node->UnPack(pReader);
		_monarchs[node->_charID] = node;
		CacheInitialDataRecursively(node, NULL);
	}

	DWORD numInfos = pReader->Read<DWORD>();

	for (DWORD i = 0; i < numInfos; i++)
	{
		DWORD monarchID = pReader->Read<DWORD>();

		AllegianceInfo *info = new AllegianceInfo();
		info->UnPack(pReader);
		_allegInfos[monarchID] = info;
	}

	return true;
}

void AllegianceManager::CacheInitialDataRecursively(AllegianceTreeNode *node, AllegianceTreeNode *parent)
{
	if (!node)
		return;

	_directNodes[node->_charID] = node;

	node->_monarchID = parent ? parent->_monarchID : node->_charID;
	node->_patronID = parent ? parent->_charID : 0;
	node->_rank = 1;
	node->_numFollowers = 0;

	unsigned int highestVassalRank = 0;
	bool bRankUp = false;

	for (auto &entry : node->_vassals)
	{
		AllegianceTreeNode *vassal = entry.second;
		CacheInitialDataRecursively(vassal, node);

		if (vassal->_rank > highestVassalRank)
		{
			highestVassalRank = vassal->_rank;
			bRankUp = false;
		}
		else if (vassal->_rank == highestVassalRank)
		{
			bRankUp = true;
		}

		node->_numFollowers += vassal->_numFollowers + 1;
	}

	if (!highestVassalRank)
	{
		node->_rank = 1;
	}
	else
	{
		node->_rank = highestVassalRank;
		if (bRankUp)
			node->_rank++;
	}
}

void AllegianceManager::CacheDataRecursively(AllegianceTreeNode *node, AllegianceTreeNode *parent)
{
	if (!node)
		return;

	node->_monarchID = parent ? parent->_monarchID : node->_charID;
	node->_patronID = parent ? parent->_charID : 0;
	node->_rank = 1;
	node->_numFollowers = 0;

	unsigned int highestVassalRank = 0;
	bool bRankUp = false;

	for (auto &entry : node->_vassals)
	{
		AllegianceTreeNode *vassal = entry.second;
		CacheDataRecursively(vassal, node);

		if (vassal->_rank > highestVassalRank)
		{
			highestVassalRank = vassal->_rank;
			bRankUp = false;
		}
		else if (vassal->_rank == highestVassalRank)
		{
			bRankUp = true;
		}

		node->_numFollowers += vassal->_numFollowers + 1;
	}

	if (!highestVassalRank)
	{
		node->_rank = 1;
	}
	else
	{
		node->_rank = highestVassalRank;
		if (bRankUp)
			node->_rank++;
	}
}


void AllegianceManager::NotifyTreeRefreshRecursively(AllegianceTreeNode *node)
{
	if (!node)
		return;

	CWeenieObject *weenie = g_pWorld->FindPlayer(node->_charID);
	if (weenie)
	{
		/*
		unsigned int rank = 0;
		AllegianceProfile *prof = CreateAllegianceProfile(weenie, &rank);

		BinaryWriter allegianceUpdate;
		allegianceUpdate.Write<DWORD>(0x20);
		allegianceUpdate.Write<DWORD>(rank);
		prof->Pack(&allegianceUpdate);
		weenie->SendNetMessage(&allegianceUpdate, PRIVATE_MSG, TRUE, FALSE);
		delete prof;
		*/
		SetWeenieAllegianceQualities(weenie);
	}

	for (auto &entry : node->_vassals)
		NotifyTreeRefreshRecursively(entry.second);
}

AllegianceTreeNode *AllegianceManager::GetTreeNode(DWORD charID)
{
	auto i = _directNodes.find(charID);
	if (i != _directNodes.end())
		return i->second;

	return NULL;
}

AllegianceInfo *AllegianceManager::GetInfo(DWORD monarchID)
{
	auto i = _allegInfos.find(monarchID);
	if (i != _allegInfos.end())
		return i->second;

	return NULL;
}

void AllegianceManager::SetWeenieAllegianceQualities(CWeenieObject *weenie)
{
	if (!weenie)
		return;
	
	AllegianceTreeNode *monarch;
	AllegianceTreeNode *patron;

	AllegianceTreeNode *node = GetTreeNode(weenie->GetID());
	if (node)
	{
		monarch = GetTreeNode(node->_monarchID);
		patron = GetTreeNode(node->_patronID);

		weenie->m_Qualities.SetInt(ALLEGIANCE_FOLLOWERS_INT, node->_numFollowers);
		weenie->m_Qualities.SetInt(ALLEGIANCE_RANK_INT, node->_rank);
		// wrong weenie->m_Qualities.SetInt(ALLEGIANCE_CP_POOL_INT, node->_cp_cached);
	}
	else
	{
		monarch = NULL;
		patron = NULL;

		weenie->m_Qualities.RemoveInt(ALLEGIANCE_FOLLOWERS_INT);
		weenie->m_Qualities.RemoveInt(ALLEGIANCE_RANK_INT);
		// wrong weenie->m_Qualities.RemoveInt(ALLEGIANCE_CP_POOL_INT);
	}

	if (monarch)
	{
		weenie->m_Qualities.SetInstanceID(MONARCH_IID, monarch->_charID);
		weenie->m_Qualities.SetInt(MONARCHS_RANK_INT, monarch->_rank);

		if (monarch->_charID != weenie->GetID())
		{
			weenie->m_Qualities.SetString(MONARCHS_NAME_STRING, monarch->_charName);
			weenie->m_Qualities.SetString(MONARCHS_TITLE_STRING, monarch->_charName); // TODO prefix
		}
		else
		{
			weenie->m_Qualities.RemoveString(MONARCHS_NAME_STRING);
			weenie->m_Qualities.RemoveString(MONARCHS_TITLE_STRING);
		}
	}
	else
	{
		weenie->m_Qualities.RemoveInstanceID(MONARCH_IID);
		weenie->m_Qualities.RemoveInt(MONARCHS_RANK_INT);
		weenie->m_Qualities.RemoveString(MONARCHS_NAME_STRING);
		weenie->m_Qualities.RemoveString(MONARCHS_TITLE_STRING);
	}

	if (patron)
	{
		weenie->m_Qualities.SetInstanceID(PATRON_IID, patron->_charID);
		weenie->m_Qualities.SetString(PATRONS_TITLE_STRING, patron->_charName); // TODO prefix
	}
	else
	{
		weenie->m_Qualities.RemoveInstanceID(PATRON_IID);
		weenie->m_Qualities.RemoveString(PATRONS_TITLE_STRING);
	}
}

AllegianceProfile *AllegianceManager::CreateAllegianceProfile(DWORD char_id, unsigned int *pRank)
{
	AllegianceProfile *prof = new AllegianceProfile;
	*pRank = 0;

	AllegianceTreeNode *node = GetTreeNode(char_id);
	if (node)
	{
		*pRank = node->_rank;

		AllegianceTreeNode *monarch = GetTreeNode(node->_monarchID);
		AllegianceTreeNode *patron = GetTreeNode(node->_patronID);
		AllegianceInfo *info = GetInfo(node->_monarchID);

		RELEASE_ASSERT(monarch);
		if (monarch)
			prof->_total_members = monarch->_numFollowers;

		prof->_total_vassals = node->_numFollowers;

		if (!info)
		{
			info = new AllegianceInfo();
			_allegInfos[node->_monarchID] = info;
		}

		if (info)
		{
			RELEASE_ASSERT(!info->_info._nodes.size());
			prof->_allegiance = info->_info;
		}

		if (monarch)
		{
			AllegianceNode *patronNode = NULL;

			if (monarch != node)
			{
				AllegianceNode *monarchNode = new AllegianceNode;
				monarch->FillAllegianceNode(monarchNode);

				if (g_pWorld->FindPlayer(monarch->_charID))
					monarchNode->_data._bitfield |= LoggedIn_AllegianceIndex;

				prof->_allegiance._nodes.push_back(monarchNode);

				patronNode = monarchNode;
				if (patron && patron != monarch)
				{
					patronNode = new AllegianceNode;
					patron->FillAllegianceNode(patronNode);
					patronNode->_patron = monarchNode;

					if (g_pWorld->FindPlayer(patron->_charID))
						patronNode->_data._bitfield |= LoggedIn_AllegianceIndex;

					prof->_allegiance._nodes.push_back(patronNode);
				}
			}

			AllegianceNode *selfNode = new AllegianceNode;
			node->FillAllegianceNode(selfNode);
			selfNode->_patron = patronNode;
			selfNode->_data._bitfield |= LoggedIn_AllegianceIndex;
			prof->_allegiance._nodes.push_back(selfNode);

			for (auto &entry : node->_vassals)
			{
				AllegianceTreeNode *vassal = entry.second;

				AllegianceNode *vassalNode = new AllegianceNode;
				vassal->FillAllegianceNode(vassalNode);
				vassalNode->_patron = selfNode;

				if (g_pWorld->FindPlayer(vassal->_charID))
					vassalNode->_data._bitfield |= LoggedIn_AllegianceIndex;
				
				prof->_allegiance._nodes.push_back(vassalNode);
			}
		}
	}

	return prof;
}

const unsigned int MAX_DIRECT_VASSALS = 11;

int AllegianceManager::TrySwearAllegiance(CWeenieObject *source, CWeenieObject *target)
{
	if (source->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;
	if (target->IsBusyOrInAction())
		return WERROR_ACTIONS_LOCKED;

	if (source->DistanceTo(target) >= 4.0)
		return WERROR_TOO_FAR;

	if (CPlayerWeenie *player = target->AsPlayer())
	{
		if (player->GetCharacterOptions() & IgnoreAllegianceRequests_CharacterOption)
			return WERROR_ALLEGIANCE_IGNORING_REQUESTS;
	}

	if (source->InqIntQuality(LEVEL_INT, 0) > target->InqIntQuality(LEVEL_INT, 0))
		return WERROR_ALLEGIANCE_ILLEGAL_LEVEL;

	AllegianceTreeNode *selfTreeNode = g_pAllegianceManager->GetTreeNode(source->GetID());
	if (selfTreeNode && selfTreeNode->_patronID)
	{
		// already sworn to someone
		return WERROR_NONE;
	}

	AllegianceTreeNode *targetTreeNode = g_pAllegianceManager->GetTreeNode(target->GetID());
	if (targetTreeNode)
	{
		if (selfTreeNode && selfTreeNode->_charID == targetTreeNode->_monarchID)
		{
			// Clearly he doesn't have updated data.
			SendAllegianceProfile(source);

			return WERROR_ALLEGIANCE_ADD_HIERARCHY_FAILURE;
		}

		if (targetTreeNode->_vassals.size() >= MAX_DIRECT_VASSALS)
		{
			// too many vassals
			return WERROR_NONE;
		}
	}

	if (!targetTreeNode)
	{
		// target wasn't in an allegiance already

		// make one...
		targetTreeNode = new AllegianceTreeNode();
		targetTreeNode->_monarchID = target->GetID();
		_directNodes[target->GetID()] = targetTreeNode;
		_monarchs[target->GetID()] = targetTreeNode;

		AllegianceInfo *info = new AllegianceInfo();
		_allegInfos[target->GetID()] = info;
	}
	targetTreeNode->UpdateWithWeenie(target);

	if (selfTreeNode)
	{
		// must be a current monarch
		_monarchs.erase(source->GetID());

		auto i = _allegInfos.find(source->GetID());
		if (i != _allegInfos.end())
		{
			delete i->second;
			_allegInfos.erase(i);
		}
	}
	else
	{
		selfTreeNode = new AllegianceTreeNode();
		_directNodes[source->GetID()] = selfTreeNode;
	}

	selfTreeNode->UpdateWithWeenie(source);
	selfTreeNode->_rank = 1;
	selfTreeNode->_numFollowers = 0;
	selfTreeNode->_patronID = targetTreeNode->_charID;
	selfTreeNode->_monarchID = targetTreeNode->_monarchID;

	targetTreeNode->_vassals[selfTreeNode->_charID] = selfTreeNode;

	// not efficient, can revise later
	RELEASE_ASSERT(_monarchs.find(targetTreeNode->_monarchID) != _monarchs.end());

	AllegianceTreeNode *monarchTreeNode = _monarchs[targetTreeNode->_monarchID];
	CacheDataRecursively(monarchTreeNode, NULL);
	NotifyTreeRefreshRecursively(monarchTreeNode);

	source->SendText(csprintf("%s has accepted your oath of Allegiance!", target->GetName().c_str()), LTT_DEFAULT);
	target->SendText(csprintf("%s has sworn Allegiance to you.", source->GetName().c_str()), LTT_DEFAULT);

	source->DoForcedMotion(Motion_Kneel);

	Save();

	return WERROR_NONE;
}

bool AllegianceManager::ShouldRemoveAllegianceNode(AllegianceTreeNode *node)
{
	if (!node->_patronID && !node->_vassals.size())
		return true;

	return false;
}

void AllegianceManager::RemoveAllegianceNode(AllegianceTreeNode *node)
{
	_monarchs.erase(node->_charID);
	_directNodes.erase(node->_charID);

	auto allegInfoEntry = _allegInfos.find(node->_charID);
	if (allegInfoEntry != _allegInfos.end())
	{
		delete allegInfoEntry->second;
		_allegInfos.erase(allegInfoEntry);
	}

	delete node;
}

void AllegianceManager::BreakAllegiance(AllegianceTreeNode *patronNode, AllegianceTreeNode *vassalNode)
{
	// the target is a vassal
	patronNode->_vassals.erase(vassalNode->_charID);

	vassalNode->_monarchID = vassalNode->_charID;
	vassalNode->_patronID = 0;

	_monarchs[vassalNode->_charID] = vassalNode;

	AllegianceInfo *info = new AllegianceInfo();
	_allegInfos[vassalNode->_charID] = info;

	{
		RELEASE_ASSERT(_monarchs.find(patronNode->_monarchID) != _monarchs.end());

		AllegianceTreeNode *patronMonarchNode = _monarchs[patronNode->_monarchID];
		CacheDataRecursively(patronMonarchNode, NULL);
		NotifyTreeRefreshRecursively(patronMonarchNode);

		// check if i should even have allegiance data anymore (no vassals/patron)
		if (ShouldRemoveAllegianceNode(patronNode))
		{
			// don't need allegiance data anymore 
			RemoveAllegianceNode(patronNode);
		}
	}

	{
		RELEASE_ASSERT(_monarchs.find(vassalNode->_monarchID) != _monarchs.end());

		// check if i should even have allegiance data anymore (no vassals/patron)
		AllegianceTreeNode *vassalMonarchNode = _monarchs[vassalNode->_monarchID];
		CacheDataRecursively(vassalMonarchNode, NULL);
		NotifyTreeRefreshRecursively(vassalMonarchNode);

		// check if i should even have allegiance data anymore (no vassals/patron)
		if (ShouldRemoveAllegianceNode(vassalNode))
		{
			// don't need allegiance data anymore 
			RemoveAllegianceNode(vassalNode);
		}
	}

	Save();
}

int AllegianceManager::TryBreakAllegiance(CWeenieObject *source, DWORD target_id)
{
	AllegianceTreeNode *selfTreeNode = g_pAllegianceManager->GetTreeNode(source->GetID());
	if (!selfTreeNode)
	{
		// not in an allegiance
		return WERROR_NONE;
	}

	AllegianceTreeNode *targetTreeNode = g_pAllegianceManager->GetTreeNode(target_id);
	if (!targetTreeNode)
	{
		// target not in an allegiance
		return WERROR_NONE;
	}

	std::string targetCharName = targetTreeNode->_charName;

	if (selfTreeNode->_charID == targetTreeNode->_patronID)
	{
		// the target is a vassal
		BreakAllegiance(selfTreeNode, targetTreeNode);
	}
	else if (selfTreeNode->_patronID == target_id)
	{
		// the target is the patron
		BreakAllegiance(targetTreeNode, selfTreeNode);
	}
	else
	{
		return WERROR_NO_OBJECT;
	}

	source->SendText(csprintf(" You have broken your Allegiance to %s!", targetCharName.c_str()), LTT_DEFAULT);

	CWeenieObject *target = g_pWorld->FindPlayer(target_id);
	if (target)
		target->SendText(csprintf("%s has broken their Allegiance to you!", source->GetName().c_str()), LTT_DEFAULT);

	return WERROR_NONE;
}

void AllegianceManager::BreakAllAllegiance(DWORD char_id)
{
	AllegianceTreeNode *selfTreeNode = g_pAllegianceManager->GetTreeNode(char_id);
	if (!selfTreeNode)
	{
		// not in an allegiance
		return;
	}

	if (selfTreeNode->_patronID)
	{
		AllegianceTreeNode *patronTreeNode = g_pAllegianceManager->GetTreeNode(selfTreeNode->_patronID);
		if (patronTreeNode)
		{
			BreakAllegiance(patronTreeNode, selfTreeNode);
		}
	}

	while ((selfTreeNode = g_pAllegianceManager->GetTreeNode(char_id)) && !selfTreeNode->_vassals.empty())
	{
		AllegianceTreeNodeMap::iterator vassalEntry = selfTreeNode->_vassals.begin();

		if (vassalEntry->second)
		{
			BreakAllegiance(selfTreeNode, vassalEntry->second);
		}
		else
		{
			selfTreeNode->_vassals.erase(vassalEntry);
		}
	}

	// should not be necessary
	assert(selfTreeNode != g_pAllegianceManager->GetTreeNode(char_id));

	if (selfTreeNode = g_pAllegianceManager->GetTreeNode(char_id))
	{
		if (ShouldRemoveAllegianceNode(selfTreeNode))
		{
			RemoveAllegianceNode(selfTreeNode);
		}
	}
}

void AllegianceManager::HandleAllegiancePassup(DWORD source_id, long long amount, bool direct)
{
	AllegianceTreeNode *node = GetTreeNode(source_id);
	if (!node) // no allegiance
		return;

	if (!node->_patronID) // no patron (only vassals)
		return;

	AllegianceTreeNode *patron = GetTreeNode(node->_patronID);
	RELEASE_ASSERT(patron);

	if (!patron) // shouldn't happen
		return;

	double realDaysSworn = 0;
	double ingameHoursSworn = 0;
	double vassalFactor = min(1.0, max(0.0, 0.25 * patron->_vassals.size()));

	double avgRealDaysVassalsSworn = 0;
	double avgIngameHoursVassalsSworn = 0;

	double factor1 = direct ? 50.0 : 16.0;
	double factor2 = direct ? 22.5 : 8.0;

	double generatedPercent = 0.01 * (factor1 + factor2 * (node->_loyalty / 291.0) * (1.0 + (realDaysSworn / 730.0) * (ingameHoursSworn / 720.0)));
	double receivedPercent = 0.01 * (factor1 + factor2 * (patron->_leadership / 291.0) * (1.0 + vassalFactor * (avgRealDaysVassalsSworn / 730.0) * (avgIngameHoursVassalsSworn / 720.0)));
	
	double passup = generatedPercent * receivedPercent;

	DWORD generatedAmount = (DWORD)(amount * generatedPercent);
	DWORD passupAmount = (DWORD)(amount * passup);

	if (passup > 0)
	{
		node->_cp_tithed += generatedAmount;
		patron->_cp_cached += passupAmount;
		patron->_cp_pool_to_unload += passupAmount;

		CWeenieObject *patron_weenie = g_pWorld->FindPlayer(patron->_charID);
		if (patron_weenie)
			patron_weenie->TryToUnloadAllegianceXP(false);

		HandleAllegiancePassup(patron->_charID, passup, false);
	}
}

void AllegianceManager::ChatMonarch(DWORD sender_id, const char *text)
{
	AllegianceTreeNode *node = GetTreeNode(sender_id);
	if (!node || !node->_monarchID || node->_monarchID == sender_id) // no allegiance
		return;

	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (!sender_weenie)
		return;

	CWeenieObject *target = g_pWorld->FindPlayer(node->_monarchID);
	if (!target)
		return;

	sender_weenie->SendNetMessage(ServerText(csprintf("You say to your Monarch, \"%s\"", text), LTT_SPEECH_DIRECT_SEND), PRIVATE_MSG, FALSE, TRUE);
	target->SendNetMessage(ServerText(csprintf("Your follower %s says to you, \"%s\"", sender_weenie->GetName().c_str(), text), LTT_SPEECH_DIRECT), PRIVATE_MSG, FALSE, TRUE);
}

void AllegianceManager::ChatPatron(DWORD sender_id, const char *text)
{
	AllegianceTreeNode *node = GetTreeNode(sender_id);
	if (!node || !node->_patronID) // no allegiance
		return;

	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (!sender_weenie)
		return;

	CWeenieObject *target = g_pWorld->FindPlayer(node->_patronID);
	if (!target)
		return;

	sender_weenie->SendNetMessage(ServerText(csprintf("You say to your Patron, \"%s\"", text), LTT_SPEECH_DIRECT_SEND), PRIVATE_MSG, FALSE, TRUE);
	target->SendNetMessage(ServerText(csprintf("Your vassal %s says to you, \"%s\"", sender_weenie->GetName().c_str(), text), LTT_SPEECH_DIRECT), PRIVATE_MSG, FALSE, TRUE);
}

void AllegianceManager::ChatVassals(DWORD sender_id, const char *text)
{
	AllegianceTreeNode *node = GetTreeNode(sender_id);
	if (!node) // no allegiance
		return;

	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (!sender_weenie)
		return;

	sender_weenie->SendNetMessage(ServerText(csprintf("You say to your Vassals, \"%s\"", text), LTT_SPEECH_DIRECT_SEND), PRIVATE_MSG, FALSE, TRUE);

	for (auto &entry : node->_vassals)
	{
		CWeenieObject *target = g_pWorld->FindPlayer(entry.second->_charID);
		if (!target)
			continue;

		target->SendNetMessage(ServerText(csprintf("Your patron %s says to you, \"%s\"", sender_weenie->GetName().c_str(), text), LTT_SPEECH_DIRECT), PRIVATE_MSG, FALSE, TRUE);
	}
}

void AllegianceManager::ChatCovassals(DWORD sender_id, const char *text)
{
	AllegianceTreeNode *node = GetTreeNode(sender_id);
	if (!node || !node->_patronID) // no allegiance
		return;

	AllegianceTreeNode *patron_node = GetTreeNode(node->_patronID);
	if (!patron_node) // no patron
		return;

	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (!sender_weenie)
		return;

	sender_weenie->SendNetMessage(ServerText(csprintf("[Co-Vassals] You Say, \"%s\"", text), LTT_SPEECH_DIRECT_SEND), PRIVATE_MSG, FALSE, TRUE);

	for (auto &entry : patron_node->_vassals)
	{
		if (entry.second->_charID == sender_id)
			continue;

		CWeenieObject *target = g_pWorld->FindPlayer(entry.second->_charID);
		if (target)
			target->SendNetMessage(ServerText(csprintf("[Co-Vassals] %s says, \"%s\"", sender_weenie->GetName().c_str(), text), LTT_SPEECH_DIRECT), PRIVATE_MSG, FALSE, TRUE);
	}

	CWeenieObject *patron_weenie = g_pWorld->FindPlayer(node->_patronID);
	if (patron_weenie)
		patron_weenie->SendNetMessage(ServerText(csprintf("[Co-Vassals] %s says, \"%s\"", sender_weenie->GetName().c_str(), text), LTT_SPEECH_DIRECT), PRIVATE_MSG, FALSE, TRUE);

}

void AllegianceManager::SendAllegianceProfile(CWeenieObject *pPlayer)
{
	unsigned int rank = 0;
	AllegianceProfile *prof = g_pAllegianceManager->CreateAllegianceProfile(pPlayer->GetID(), &rank);

	BinaryWriter allegianceUpdate;
	allegianceUpdate.Write<DWORD>(0x20);
	allegianceUpdate.Write<DWORD>(rank);
	prof->Pack(&allegianceUpdate);
	pPlayer->SendNetMessage(&allegianceUpdate, PRIVATE_MSG, TRUE, FALSE);

	BinaryWriter allegianceUpdateDone;
	allegianceUpdateDone.Write<DWORD>(0x1C8);
	allegianceUpdateDone.Write<DWORD>(0);
	pPlayer->SendNetMessage(&allegianceUpdateDone, PRIVATE_MSG, TRUE, FALSE);

	delete prof;
}

DWORD AllegianceManager::GetCachedMonarchIDForPlayer(CPlayerWeenie *player)
{
	// this data may not be trustworthy, should use tree node for anything important
	return player->InqIIDQuality(MONARCH_IID, 0);
}