
#include <StdAfx.h>
#include "AllegianceManager.h"
#include "World.h"
#include "Player.h"
#include "DatabaseIO.h"
#include "ChatMsgs.h"
#include "InferredPortalData.h"
#include "Util.h"
#include "Config.h"

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

	if (!stricmp(targetName, compareName))
	{
		return this;
	}

	AllegianceTreeNode *node = nullptr;
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
	node->_data._cp_cached = min(uint64_t(4294967295ul), _cp_cached);
	node->_data._cp_tithed = min(uint64_t(4294967295ul), _cp_tithed);
	node->_data._gender = _gender;
	node->_data._hg = _hg;
	node->_data._leadership = _leadership;
	node->_data._loyalty = _loyalty;
	node->_data._name = _charName;
	node->_data._id = _charID;
//	node->_data._allegiance_age = _unixTimeSwornAt; what unit is this and what does the client do with this information?
//	node->_data._time_online = _ingameSecondsSworn; what unit is this and what does the client do with this information?
}

void AllegianceTreeNode::UpdateWithWeenie(CWeenieObject *weenie)
{
	_charID = weenie->GetID();
	_charName = weenie->GetName();	
	_hg = static_cast<HeritageGroup>(weenie->InqIntQuality(HERITAGE_GROUP_INT, Invalid_HeritageGroup));
	_gender = static_cast<Gender>(weenie->InqIntQuality(GENDER_INT, Invalid_Gender));
	_level = weenie->InqIntQuality(LEVEL_INT, 1);
	_leadership = 0;
	weenie->m_Qualities.InqSkill(LEADERSHIP_SKILL, *reinterpret_cast<uint32_t *>(&_leadership), FALSE);
	_loyalty = 0;
	weenie->m_Qualities.InqSkill(LOYALTY_SKILL, *reinterpret_cast<uint32_t *>(&_loyalty), FALSE);
}

DEFINE_PACK(AllegianceTreeNode)
{
	pWriter->Write<uint32_t>(2); // version
	pWriter->Write<uint32_t>(_charID);
	pWriter->WriteString(_charName);

	pWriter->Write<uint32_t>(_monarchID);
	pWriter->Write<uint32_t>(_patronID);
	pWriter->Write<int>(_hg);
	pWriter->Write<int>(_gender);
	pWriter->Write<uint32_t>(_rank);
	pWriter->Write<uint32_t>(_level);
	pWriter->Write<uint32_t>(_leadership);
	pWriter->Write<uint32_t>(_loyalty);
	pWriter->Write<uint32_t>(_numFollowers);
	pWriter->Write<uint64_t>(_cp_cached);
	pWriter->Write<uint64_t>(_cp_tithed);
	pWriter->Write<uint64_t>(_cp_pool_to_unload);
	pWriter->Write<uint64_t>(_unixTimeSwornAt);
	pWriter->Write<uint64_t>(_ingameSecondsSworn);

	pWriter->Write<uint32_t>(_vassals.size());
	for (auto &entry : _vassals)
		entry.second->Pack(pWriter);
}

DEFINE_UNPACK(AllegianceTreeNode)
{
	uint32_t version = pReader->Read<uint32_t>(); // version
	_charID = pReader->Read<uint32_t>();
	_charName = pReader->ReadString();

	_monarchID = pReader->Read<uint32_t>();
	_patronID = pReader->Read<uint32_t>();
	_hg = (HeritageGroup)pReader->Read<int>();
	_gender = (Gender)pReader->Read<int>();
	_rank = pReader->Read<uint32_t>();
	_level = pReader->Read<uint32_t>();
	_leadership = pReader->Read<uint32_t>();
	_loyalty = pReader->Read<uint32_t>();
	_numFollowers = pReader->Read<uint32_t>();
	_cp_cached = pReader->Read<uint64_t>();
	_cp_tithed = pReader->Read<uint64_t>();
	_cp_pool_to_unload = pReader->Read<uint64_t>();
	if (version < 2) {
		_unixTimeSwornAt = time(0); // set to now
		_ingameSecondsSworn = 1;
	} else {
		_unixTimeSwornAt = pReader->Read<uint64_t>();
		_ingameSecondsSworn = pReader->Read<uint64_t>();
	}

	uint32_t numVassals = pReader->Read<uint32_t>();
	for (uint32_t i = 0; i < numVassals; i++)
	{
		AllegianceTreeNode *node = new AllegianceTreeNode();
		node->UnPack(pReader);
		_vassals[node->_charID] = node;

		assert(node->_charID >= 0x50000000 && node->_charID < 0x60000000);
	}

	assert(_charID >= 0x50000000 && _charID < 0x60000000);
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
	unsigned long length = 0;
	if (g_pDBIO->GetGlobalData(DBIO_GLOBAL_ALLEGIANCE_DATA, &data, &length))
	{
		// convert to new table schema

		BinaryReader reader(data, length);
		UnPack(&reader);

		int count = 0;
		for (auto alleg : _allegInfos) {
			if (alleg.second->_info.m_officerTitleList.empty()) {
				alleg.second->_info.m_officerTitleList = { "Speaker", "Seneschal", "Castellan" };
				count++;
			}
		}

		if (!count) {
			allAllegiancesUpdated = true;
			Save();
			WINLOG(Temp, Normal, "Successfully updated all allegiances!\n");
		}
	}

	m_LastSave = Timer::cur_time;
}

void AllegianceManager::Save()
{
	for (auto ai : _allegInfos) {
		ai.second->_info.packForDB = true;
	}
	BinaryWriter data;
	Pack(&data);
	bool success = g_pDBIO->CreateOrUpdateGlobalData(DBIO_GLOBAL_ALLEGIANCE_DATA, data.GetData(), data.GetSize());
	if (!success)
		WINLOG(Temp, Normal, "Problem saving allegiance data to DB!\n");
}

void AllegianceManager::WalkTreeAndBumpOnlineTime(AllegianceTreeNode *node, int onlineSecondsDelta) {
	// Walk allegiance tree and bump _ingameSecondsSworn for all online characters. 
	// There's probably a better way of maintaining number of seconds of online-time.
	for (auto &entry : node->_vassals) {
		AllegianceTreeNode *vassal = entry.second;
		CPlayerWeenie *target = g_pWorld->FindPlayer(vassal->_charID);
		if (target)
			vassal->_ingameSecondsSworn += onlineSecondsDelta;
		WalkTreeAndBumpOnlineTime(vassal, onlineSecondsDelta);
	}
}

void AllegianceManager::Tick()
{
	if (m_LastSave <= (Timer::cur_time - 300.0)) // every 5 minutes save
	{
		for (auto &entry : _monarchs)
			WalkTreeAndBumpOnlineTime(entry.second, ((int)round(Timer::cur_time - m_LastSave)));
		Save();
		m_LastSave = Timer::cur_time;
	}
	
	// check for expired chat gags
	if (m_LastGagCheck <= (Timer::cur_time - 5))
	{
		for (auto entry : _allegInfos) { 
			auto gagList = entry.second->_info.m_chatGagList;
			std::vector<uint32_t> toRemove;
			for (auto gagged : gagList)	{
				if (gagged.second <= Timer::cur_time) {
					toRemove.push_back(gagged.first);
				}
			}
			for (auto id : toRemove) {
				gagList.erase(id);
			}
		}
		m_LastGagCheck = Timer::cur_time;
	}
}

DEFINE_PACK(AllegianceManager)
{
	pWriter->Write<uint32_t>(1); // version
	pWriter->Write<uint32_t>(_monarchs.size());

	for (auto &entry : _monarchs)
		entry.second->Pack(pWriter);

	pWriter->Write<uint32_t>(_allegInfos.size());

	for (auto &entry : _allegInfos)
	{
		pWriter->Write<uint32_t>(entry.first);
		entry.second->Pack(pWriter);
	}
}

DEFINE_UNPACK(AllegianceManager)
{
	uint32_t version = pReader->Read<uint32_t>();
	uint32_t numMonarchs = pReader->Read<uint32_t>();

	for (uint32_t i = 0; i < numMonarchs; i++)
	{
		AllegianceTreeNode *node = new AllegianceTreeNode();
		if (node->UnPack(pReader))
		{
			_monarchs[node->_charID] = node;
			CacheInitialDataRecursively(node, NULL);
		}
	}

	uint32_t numInfos = pReader->Read<uint32_t>();

	for (uint32_t i = 0; i < numInfos; i++)
	{
		uint32_t monarchID = pReader->Read<uint32_t>();

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
		allegianceUpdate.Write<uint32_t>(0x20);
		allegianceUpdate.Write<uint32_t>(rank);
		prof->Pack(&allegianceUpdate);
		weenie->SendNetMessage(&allegianceUpdate, PRIVATE_MSG, TRUE, FALSE);
		delete prof;
		*/
		SetWeenieAllegianceQualities(weenie);
	}

	for (auto &entry : node->_vassals)
		NotifyTreeRefreshRecursively(entry.second);
}

AllegianceTreeNode *AllegianceManager::GetTreeNode(uint32_t charID)
{
	auto i = _directNodes.find(charID);
	if (i != _directNodes.end())
		return i->second;

	return NULL;
}

AllegianceInfo *AllegianceManager::GetInfo(uint32_t monarchID)
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
		weenie->NotifyIntStatUpdated(ALLEGIANCE_RANK_INT);
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
			std::string title = GetRankTitle(monarch->_gender, monarch->_hg, monarch->_rank) + " " + monarch->_charName;
			weenie->m_Qualities.SetString(MONARCHS_TITLE_STRING, title);
		}
		else
		{
			weenie->m_Qualities.RemoveString(MONARCHS_NAME_STRING);
			weenie->m_Qualities.RemoveString(MONARCHS_TITLE_STRING);
		}

		if (AllegianceInfo* ai = GetInfo(monarch->_charID))
		{
			if (!ai->_info.m_AllegianceName.empty())
				weenie->m_Qualities.SetString(ALLEGIANCE_NAME_STRING, ai->_info.m_AllegianceName);
		}
	}
	else
	{
		weenie->m_Qualities.RemoveInstanceID(MONARCH_IID);
		weenie->m_Qualities.RemoveInt(MONARCHS_RANK_INT);
		weenie->m_Qualities.RemoveString(MONARCHS_NAME_STRING);
		weenie->m_Qualities.RemoveString(MONARCHS_TITLE_STRING);
		weenie->m_Qualities.RemoveString(ALLEGIANCE_NAME_STRING);
	}

	if (patron)
	{
		weenie->m_Qualities.SetInstanceID(PATRON_IID, patron->_charID);
		std::string title = GetRankTitle(patron->_gender, patron->_hg, patron->_rank) + " " + patron->_charName;
		weenie->m_Qualities.SetString(PATRONS_TITLE_STRING, title);
	}
	else
	{
		weenie->m_Qualities.RemoveInstanceID(PATRON_IID);
		weenie->m_Qualities.RemoveString(PATRONS_TITLE_STRING);
	}
}

AllegianceProfile *AllegianceManager::CreateAllegianceProfile(uint32_t char_id, unsigned int *pRank)
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

void AllegianceManager::TrySwearAllegiance(CWeenieObject *vassal, CWeenieObject *patron)
{
	if (vassal->IsBusyOrInAction())
	{
		vassal->NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		return;
	}
	if (patron->IsBusyOrInAction())
	{
		vassal->NotifyWeenieError(WERROR_ALLEGIANCE_PATRON_BUSY);
		return;
	}
	if (vassal->DistanceTo(patron) >= 4.0)
	{
		vassal->NotifyWeenieError(WERROR_MISSILE_OUT_OF_RANGE);
		return;
	}
	if (vassal->GetID() == patron->GetID())
		return;

	if(g_pDBIO->GetCharacterInfo(vassal->GetID()).account_id == g_pDBIO->GetCharacterInfo(patron->GetID()).account_id)
		return;

	if (CPlayerWeenie *player = patron->AsPlayer())
	{
		if (player->GetCharacterOptions() & IgnoreAllegianceRequests_CharacterOption)
		{
			vassal->NotifyWeenieError(WERROR_ALLEGIANCE_IGNORING_REQUESTS);
			return;
		}
	}

	AllegianceTreeNode *vassalTreeNode = g_pAllegianceManager->GetTreeNode(vassal->GetID());
	if (vassalTreeNode && vassalTreeNode->_patronID)
	{
		// already sworn to someone
		vassal->NotifyWeenieError(WERROR_ALLEGIANCE_PATRON_EXISTS);
		if (vassalTreeNode->_patronID == patron->GetID())
		{
			patron->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_ADD_HIERARCHY_FAILURE, vassal->GetName().c_str());
		}
		return;
	}

	AllegianceTreeNode *patronTreeNode = GetTreeNode(patron->GetID());
	if (patronTreeNode)
	{
		if (IsBanned(vassal->GetID(), patronTreeNode->_monarchID))
		{
			vassal->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_YOU_ARE_BANNED, patron->GetName().c_str());
			patron->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_BANNED, vassal->GetName().c_str());
			return;
		}
		AllegianceInfo* ai = GetInfo(patronTreeNode->_monarchID);
		if (!IsOfficerWithLevel(patronTreeNode, Castellan_AllegianceOfficerLevel)) // If a player swears allegiance to a monarch or Castellan-level officer in a locked allegiance, the action proceeds as normal.
		{
			if (ai && ai->_info.m_isLocked && (vassal->GetID() != ai->_info.m_ApprovedVassal)) // bypass the lock if it's the approved vassal
			{
				vassal->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_PREVENTS_VASSAL, patron->GetName().c_str());
				patron->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_PREVENTS_PATRON, vassal->GetName().c_str());
				return;
			}
		}

		if (vassalTreeNode && vassalTreeNode->_charID == patronTreeNode->_monarchID)
		{
			// Clearly he doesn't have updated data.
			SendAllegianceProfile(vassal);

			return;
		}

		if (patronTreeNode->_vassals.size() >= MAX_DIRECT_VASSALS)
		{
			// too many vassals
			patron->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_MAX_VASSALS, "You");
			vassal->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_MAX_VASSALS, patron->GetName().c_str());
			return;
		}
	}

	if (!patronTreeNode)
	{
		// target wasn't in an allegiance already

		// make one...
		patronTreeNode = new AllegianceTreeNode();
		patronTreeNode->_monarchID = patron->GetID();
		_directNodes[patron->GetID()] = patronTreeNode;
		_monarchs[patron->GetID()] = patronTreeNode;

		AllegianceInfo *info = new AllegianceInfo();
		_allegInfos[patron->GetID()] = info;
	}
	patronTreeNode->UpdateWithWeenie(patron);

	if (vassalTreeNode)
	{
		// must be a current monarch
		_monarchs.erase(vassal->GetID());

		auto i = _allegInfos.find(vassal->GetID());
		if (i != _allegInfos.end())
		{
			delete i->second;
			_allegInfos.erase(i);
		}
	}
	else
	{
		vassalTreeNode = new AllegianceTreeNode();
		_directNodes[vassal->GetID()] = vassalTreeNode;
	}

	vassalTreeNode->UpdateWithWeenie(vassal);
	vassalTreeNode->_rank = 1;
	vassalTreeNode->_numFollowers = 0;
	vassalTreeNode->_patronID = patronTreeNode->_charID;
	vassalTreeNode->_monarchID = patronTreeNode->_monarchID;
	vassalTreeNode->_unixTimeSwornAt = time(0);
	vassalTreeNode->_ingameSecondsSworn = 1;

	if (vassalTreeNode->_level <= patronTreeNode->_level)
		vassal->m_Qualities.SetBool(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, true);
	else
		vassal->m_Qualities.SetBool(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, false);

	patronTreeNode->_vassals[vassalTreeNode->_charID] = vassalTreeNode;

	// not efficient, can revise later
	RELEASE_ASSERT(_monarchs.find(patronTreeNode->_monarchID) != _monarchs.end());

	AllegianceTreeNode *monarchTreeNode = _monarchs[patronTreeNode->_monarchID];
	CacheDataRecursively(monarchTreeNode, NULL);
	NotifyTreeRefreshRecursively(monarchTreeNode);

	vassal->SendText(csprintf("%s has accepted your oath of Allegiance!", patron->GetName().c_str()), LTT_DEFAULT);
	patron->SendText(csprintf("%s has sworn Allegiance to you.", vassal->GetName().c_str()), LTT_DEFAULT);

	vassal->DoForcedMotion(Motion_Kneel);

	Save();
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

std::string AllegianceManager::GetRankTitle(Gender gender, HeritageGroup heritage, unsigned int rank)
{
	std::string title;

	switch (heritage)
	{
	case Aluvian_HeritageGroup:
		if (gender == Male_Gender)
			title = AluvianMaleRanks[rank - 1];
		else
			title = AluvianFemaleRanks[rank - 1];
		break;
	case Gharundim_HeritageGroup:
		if (gender == Male_Gender)
			title = GharundimMaleRanks[rank - 1];
		else
			title = GharundimFemaleRanks[rank - 1];
		break;
	case Sho_HeritageGroup:
		if (gender == Male_Gender)
			title = ShoMaleRanks[rank - 1];
		else
			title = ShoFemaleRanks[rank - 1];
		break;
	case Viamontian_HeritageGroup:
		if (gender == Male_Gender)
			title = ViamontianMaleRanks[rank - 1];
		else
			title = ViamontianFemaleRanks[rank - 1];
		break;
	case Shadowbound_HeritageGroup:
		if (gender == Male_Gender)
			title = ShadowMaleRanks[rank - 1];
		else
			title = ShadowFemaleRanks[rank - 1];
		break;
	case Gearknight_HeritageGroup:
		if (gender == Male_Gender)
			title = GearKnightMaleRanks[rank - 1];
		else
			title = GearKnightFemaleRanks[rank - 1];
		break;
	case Tumerok_HeritageGroup:
		if (gender == Male_Gender)
			title = TumerokMaleRanks[rank - 1];
		else
			title = TumerokFemaleRanks[rank - 1];
		break;
	case Lugian_HeritageGroup:
		if (gender == Male_Gender)
			title = LugianMaleRanks[rank - 1];
		else
			title = LugianFemaleRanks[rank - 1];
		break;
	case Empyrean_HeritageGroup:
		if (gender == Male_Gender)
			title = EmpyreanMaleRanks[rank - 1];
		else
			title = EmpyreanFemaleRanks[rank - 1];
		break;
	case Penumbraen_HeritageGroup:
		if (gender == Male_Gender)
			title = ShadowMaleRanks[rank - 1];
		else
			title = ShadowFemaleRanks[rank - 1];
		break;
	case Undead_HeritageGroup:
		if (gender == Male_Gender)
			title = UndeadMaleRanks[rank - 1];
		else
			title = UndeadFemaleRanks[rank - 1];
		break;
	}
	return title;
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

void AllegianceManager::TryBreakAllegiance(CWeenieObject *source, uint32_t target_id)
{
	TryBreakAllegiance(source->GetID(), target_id);
}

void AllegianceManager::TryBreakAllegiance(uint32_t source_id, uint32_t target_id)
{
	AllegianceTreeNode *selfTreeNode = GetTreeNode(source_id);
	if (!selfTreeNode)
	{
		// source doesn't have patron or vassals, shouldn't be here
		return;
	}

	AllegianceTreeNode *targetTreeNode = GetTreeNode(target_id);
	if (!targetTreeNode)
	{
		// target doesn't have patron or vassals, shouldn't be here
		return;
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
		return;
	}

	CWeenieObject *source = g_pWorld->FindPlayer(source_id);
	if (source)
	{
		source->SendText(csprintf(" You have broken your Allegiance to %s!", targetCharName.c_str()), LTT_DEFAULT);
		source->m_Qualities.SetBool(IS_ALLEGIANCE_GAGGED_BOOL, false);
	}

	CWeenieObject *target = g_pWorld->FindPlayer(target_id);
	if (target && source)
	{
		target->SendText(csprintf("%s has broken their Allegiance to you!", source->GetName().c_str()), LTT_DEFAULT);
	}
}

void AllegianceManager::BreakAllAllegiance(uint32_t char_id)
{
	AllegianceTreeNode *selfTreeNode = GetTreeNode(char_id);
	if (!selfTreeNode)
	{
		// not in an allegiance
		return;
	}

	if (selfTreeNode->_patronID)
	{
		AllegianceTreeNode *patronTreeNode = GetTreeNode(selfTreeNode->_patronID);
		if (patronTreeNode)
		{
			BreakAllegiance(patronTreeNode, selfTreeNode);
		}
	}

	while ((selfTreeNode = GetTreeNode(char_id)) && !selfTreeNode->_vassals.empty())
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
	assert(selfTreeNode != GetTreeNode(char_id));

	if (selfTreeNode = GetTreeNode(char_id))
	{
		if (ShouldRemoveAllegianceNode(selfTreeNode))
		{
			RemoveAllegianceNode(selfTreeNode);
		}
	}
}

void AllegianceManager::HandleAllegiancePassup(uint32_t source_id, int64_t amount, bool direct)
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

	CPlayerWeenie* source = g_pWorld->FindPlayer(source_id);
	if (!source)
		return;

	if (!source->InqBoolQuality(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, false)) // if bool was set to false at swear time or doesn't have it flagged
	{
		if (node->_unixTimeSwornAt < 1534765700 || patron->_level >= node->_level) // if sworn before allegiance patch (20/08/18) or patron has since outleveled source, allow passup 
		{
			source->m_Qualities.SetBool(EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL, true);
		}
		else
			return;
	}

	time_t currentTime = time(0);

	double realDaysSworn = 0;
	double ingameHoursSworn = 0;

	double avgRealDaysVassalsSworn = 0;
	double avgIngameHoursVassalsSworn = 0;
	for (auto &entry : patron->_vassals) {
		AllegianceTreeNode *vassal = entry.second;
		double vassalDaysSworn = ((currentTime - vassal->_unixTimeSwornAt) / 86400.0);
		double vassalIngameHours = (vassal->_ingameSecondsSworn / 3600.0);
		if (vassal->_charID == node->_charID) {
			realDaysSworn = vassalDaysSworn;
			ingameHoursSworn = vassalIngameHours;
		}
		avgRealDaysVassalsSworn += vassalDaysSworn;
		avgIngameHoursVassalsSworn += vassalIngameHours;
	}

	avgRealDaysVassalsSworn /= max(size_t{ 1 }, patron->_vassals.size());
	avgIngameHoursVassalsSworn /= max(size_t{ 1 }, patron->_vassals.size());

	double vassalFactor = min(1.0, max(0.0, 0.25 * patron->_vassals.size()));
	double realDaysSwornFactor = min(realDaysSworn, 730.0) / 730.0;
	double ingameHoursSwornFactor = min(ingameHoursSworn, 720.0) / 720.0;
	double avgRealDaysVassalsSwornFactor = min(avgRealDaysVassalsSworn, 730.0) / 730.0;
	double avgIngameHoursVassalsSwornFactor = min(avgIngameHoursVassalsSworn, 720.0) / 720.0;
	double loyaltyFactor = min(node->_loyalty, 291u) / 291.0;
	double leadershipFactor = min(patron->_leadership, 291u) / 291.0;

	double factor1 = direct ? 50.0 : 16.0;
	double factor2 = direct ? 22.5 : 8.0;

	double generatedPercent = 0.01 * (factor1 + factor2 * loyaltyFactor * (1.0 + realDaysSwornFactor * ingameHoursSwornFactor));
	double receivedPercent = 0.01 * (factor1 + factor2 * leadershipFactor * (1.0 + vassalFactor * avgRealDaysVassalsSwornFactor * avgIngameHoursVassalsSwornFactor));
	
	double passup = generatedPercent * receivedPercent;

	uint64_t passupAmount = amount * passup;

	if (passupAmount > 0)
	{
		node->_cp_tithed += passupAmount;
		patron->_cp_cached += passupAmount;
		patron->_cp_pool_to_unload = min(uint64_t(4294967295ul), patron->_cp_pool_to_unload + passupAmount);

		CWeenieObject *patron_weenie = g_pWorld->FindPlayer(patron->_charID); // pass up now if the patron is online (otherwise remains in cp_cached until their next login)
		if (patron_weenie)
			patron_weenie->TryToUnloadAllegianceXP(false);

		HandleAllegiancePassup(patron->_charID, passupAmount, false);
	}
}

void AllegianceManager::ChatMonarch(uint32_t sender_id, const char *text)
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

	sender_weenie->SendNetMessage(ChannelChat(Monarch_ChannelID, NULL, text), PRIVATE_MSG, FALSE, TRUE);
	target->SendNetMessage(ChannelChat(Monarch_ChannelID, sender_weenie->GetName().c_str(), text), PRIVATE_MSG, FALSE, TRUE);
}

void AllegianceManager::ChatPatron(uint32_t sender_id, const char *text)
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
	
	sender_weenie->SendNetMessage(ChannelChat(Patron_ChannelID, NULL, text), PRIVATE_MSG, FALSE, TRUE);
	target->SendNetMessage(ChannelChat(Patron_ChannelID, sender_weenie->GetName().c_str(), text), PRIVATE_MSG, FALSE, TRUE);
}

void AllegianceManager::ChatVassals(uint32_t sender_id, const char *text)
{
	AllegianceTreeNode *node = GetTreeNode(sender_id);
	if (!node) // no allegiance
		return;

	CWeenieObject *sender_weenie = g_pWorld->FindPlayer(sender_id);
	if (!sender_weenie)
		return;

	sender_weenie->SendNetMessage(ChannelChat(Vassals_ChannelID, NULL, text), PRIVATE_MSG, FALSE, TRUE);

	for (auto &entry : node->_vassals)
	{
		CWeenieObject *target = g_pWorld->FindPlayer(entry.second->_charID);
		if (!target)
			continue;

		target->SendNetMessage(ChannelChat(Vassals_ChannelID, sender_weenie->GetName().c_str(), text), PRIVATE_MSG, FALSE, TRUE);
	}
}

void AllegianceManager::ChatCovassals(uint32_t sender_id, const char *text)
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

	sender_weenie->SendNetMessage(ChannelChat(Covassals_ChannelID, NULL, text), PRIVATE_MSG, FALSE, TRUE);

	for (auto &entry : patron_node->_vassals)
	{
		if (entry.second->_charID == sender_id)
			continue;

		CWeenieObject *target = g_pWorld->FindPlayer(entry.second->_charID);
		if (target)
			target->SendNetMessage(ChannelChat(Covassals_ChannelID, sender_weenie->GetName().c_str(), text), PRIVATE_MSG, FALSE, TRUE);
	}

	CWeenieObject *patron_weenie = g_pWorld->FindPlayer(node->_patronID);
	if (patron_weenie)
		patron_weenie->SendNetMessage(ChannelChat(Covassals_ChannelID, sender_weenie->GetName().c_str(), text), PRIVATE_MSG, FALSE, TRUE);

}

void AllegianceManager::SendAllegianceProfile(CWeenieObject *pPlayer)
{
	unsigned int rank = 0;
	AllegianceProfile *prof = g_pAllegianceManager->CreateAllegianceProfile(pPlayer->GetID(), &rank);

	BinaryWriter allegianceUpdate;
	allegianceUpdate.Write<uint32_t>(0x20);
	allegianceUpdate.Write<uint32_t>(rank);
	prof->Pack(&allegianceUpdate);
	pPlayer->SendNetMessage(&allegianceUpdate, PRIVATE_MSG, TRUE, FALSE, true);

	// Not found in end of retail pcaps
	//BinaryWriter allegianceUpdateDone;
	//allegianceUpdateDone.Write<uint32_t>(0x1C8);
	//allegianceUpdateDone.Write<uint32_t>(0);
	//pPlayer->SendNetMessage(&allegianceUpdateDone, PRIVATE_MSG, TRUE, FALSE);

	delete prof;
}

uint32_t AllegianceManager::GetCachedMonarchIDForPlayer(CPlayerWeenie * player)
{
	// this data may not be trustworthy, should use tree node for anything important
	return player->InqIIDQuality(MONARCH_IID, 0);
}

// This will return true if they are an officer or the monarch
bool AllegianceManager::IsOfficer(AllegianceTreeNode* playerNode)
{
	if (playerNode)
	{
		if (IsMonarch(playerNode))
			return true;

		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID)) {
			for (auto officer : ai->_info.m_officerList)
			{
				if (officer.first == playerNode->_charID)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool AllegianceManager::IsMonarch(AllegianceTreeNode* playerNode)
{
	if (playerNode)
	{
		if (playerNode->_monarchID == playerNode->_charID)
		{
			return true;
		}
	}
	return false;
}

eAllegianceOfficerLevel AllegianceManager::GetOfficerLevel(std::string player_name)
{
	uint32_t playerID = g_pDBIO->GetPlayerCharacterId(player_name.c_str());

	return GetOfficerLevel(playerID);
}

eAllegianceOfficerLevel AllegianceManager::GetOfficerLevel(uint32_t player_id)
{
	if (AllegianceTreeNode* playerNode = GetTreeNode(player_id))
	{
		if (IsOfficer(playerNode))
		{
			if (IsMonarch(playerNode))
				return Castellan_AllegianceOfficerLevel;

			if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID)) {
				for (auto officer : ai->_info.m_officerList)
				{
					if (officer.first == player_id)
					{
						return officer.second;
					}
				}
			}
		}
	}
	return Undef_AllegianceOfficerLevel;
}

std::string AllegianceManager::GetOfficerTitle(std::string player_name)
{
	uint32_t playerID = g_pDBIO->GetPlayerCharacterId(player_name.c_str());

	return GetOfficerTitle(playerID);
}

std::string AllegianceManager::GetOfficerTitle(uint32_t player_id)
{
	eAllegianceOfficerLevel officerLevel = GetOfficerLevel(player_id);

	if (officerLevel) // player is actually an officer
	{
		if (AllegianceTreeNode* playerNode = GetTreeNode(player_id))
		{
			if (IsMonarch(playerNode)) {
				return "Monarch";
			}

			if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
			{
				return ai->_info.m_officerTitleList[officerLevel - 1];
			}
		}
	}
	return "";
}

// this will always return true for the monarch
bool AllegianceManager::IsOfficerWithLevel(AllegianceTreeNode* playerNode, eAllegianceOfficerLevel min, eAllegianceOfficerLevel max)
{
	if (playerNode)
	{
		if (IsMonarch(playerNode))
			return true;

		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID)) {
			for (auto officer : ai->_info.m_officerList)
			{
				if (officer.first == playerNode->_charID && officer.second >= min && officer.second <= max)
				{
					return true;
				}
			}
		}
	}
	return false;
}

void AllegianceManager::SetMOTD(CPlayerWeenie* player, std::string msg)
{
	if (msg.empty())
		return;

	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficer(playerNode))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		std::string rank = GetRankTitle(playerNode->_gender, playerNode->_hg, playerNode->_rank);

		ai->_info.m_storedMOTD = msg;
		ai->_info.m_storedMOTDSetBy = rank + " " + player->GetName();
		player->SendText("Your allegiance message of the day has been set.", LTT_DEFAULT);
		Save();
	}
}

void AllegianceManager::LoginMOTD(CPlayerWeenie * player)
{
	AllegianceTreeNode *self = GetTreeNode(player->GetID());
	if (!self)
		return;

	AllegianceInfo* ai = GetInfo(self->_monarchID);
	if (!ai)
		return;

	if (!ai->_info.m_storedMOTD.empty())
	{
		player->SendText(csprintf("\"%s\" -- %s", ai->_info.m_storedMOTD.c_str(), ai->_info.m_storedMOTDSetBy.c_str()), LTT_DEFAULT);
	}
}

void AllegianceManager::ClearMOTD(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficer(playerNode))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		ai->_info.m_storedMOTD.clear();
		ai->_info.m_storedMOTDSetBy.clear();
		player->SendText("Your allegiance message of the day has been cleared.", LTT_DEFAULT);
		Save();
	}
}

void AllegianceManager::QueryMOTD(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}
	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		if (ai->_info.m_storedMOTD.empty())
		{
			player->SendText("Your allegiance has not set a message of the day.", LTT_DEFAULT);
		}
		else
		{
			player->SendText(csprintf("%s -- %s %s", ai->_info.m_storedMOTD.c_str(), GetOfficerTitle(ai->_info.m_storedMOTDSetBy).c_str(), ai->_info.m_storedMOTDSetBy.c_str()), LTT_DEFAULT);
		}
	}
}

void AllegianceManager::SetAllegianceName(CPlayerWeenie * player, std::string name)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		if (ai->_info.m_NameLastSetTime + 86400 <= time(0)) // check if 24 hours since last change
		{
			if (name == ai->_info.m_AllegianceName)
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_SAME_NAME);
				return;
			}
			if (name.empty())
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_EMPTY);
				return;
			}
			if (name.length() > 40)
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_TOO_LONG);
			}

			for (auto info : _allegInfos)
			{
				if (info.second->_info.m_AllegianceName == name)
				{
					player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_IN_USE);
					return;
				}
			}

			if (containsBadCharacters(name, 2)) // space, single quote, hyphen, A-Z, a-z
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_BAD_CHARACTER);
				return;
			}

			for (auto const& value : g_pPortalDataEx->GetBannedwords())
			{
				if (name.find(value) != std::string::npos)
				{
					player->NotifyWeenieError(WERROR_ALLEGIANCE_NAME_NOT_APPROPRIATE);
					return;
				}
			}

			ai->_info.m_AllegianceName = name;
			player->SendText("Your allegiance name has been set.", LTT_DEFAULT);
			ai->_info.m_NameLastSetTime = time(0);
			Save();
			NotifyTreeRefreshRecursively(GetTreeNode(playerNode->_monarchID));
		}
		else
		{
			int nextSetTime = (ai->_info.m_NameLastSetTime + 86400) - time(0); // how long before it can be changed again in seconds

			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_NAME_TIMER, TimeToString(nextSetTime).c_str());
		}
	}
}

void AllegianceManager::ClearAllegianceName(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		ai->_info.m_AllegianceName.clear();
		player->SendText("Your allegiance name has been cleared.", LTT_DEFAULT);
		Save();
		NotifyTreeRefreshRecursively(GetTreeNode(playerNode->_monarchID));
	}
}

void AllegianceManager::QueryAllegianceName(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		if (ai->_info.m_AllegianceName.empty())
		{
			player->SendText("Your allegiance hasn't set an allegiance name.", LTT_DEFAULT);
		}
		else
		{
			player->SendText(csprintf("Your allegiance name is %s", ai->_info.m_AllegianceName.c_str()), LTT_DEFAULT);
		}
	}
}

void AllegianceManager::SetOfficerTitle(CPlayerWeenie * player, int level, std::string title)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficer(playerNode))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (level > 3 || level < 1)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_INVALID_LEVEL);
		return;
	}

	if (title.length() > 20)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_TITLE_TOO_LONG);
		return;
	}

	if (containsBadCharacters(title, 2))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_TITLE_BAD_CHARACTER);
		return;
	}

	for (auto const& value : g_pPortalDataEx->GetBannedwords())
	{
		if (title.find(value) != std::string::npos)
		{
			player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_TITLE_NOT_APPROPRIATE);
			return;
		}
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		ai->_info.m_officerTitleList[level-1] = title;

		player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_TITLE_SET, title.c_str());
		Save();
	}
}

void AllegianceManager::ClearOfficerTitles(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		ai->_info.m_officerTitleList.clear();
		ai->_info.m_officerTitleList = { "Speaker", "Seneschal", "Castellan" };
		player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_TITLE_CLEARED);
		Save();
	}
}

void AllegianceManager::ListOfficerTitles(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		if (ai->_info.m_officerTitleList.empty()) {
			ai->_info.m_officerTitleList = { "Speaker", "Seneschal", "Castellan" };
		}
		player->SendText("Allegiance Officer Titles:", LTT_DEFAULT);
		for (int i = 0; i < 3; i++)
		{
			player->SendText(csprintf("%d. %s", i + 1, ai->_info.m_officerTitleList[i].c_str()), LTT_DEFAULT);
		}
	}
}

void AllegianceManager::SetOfficer(CPlayerWeenie * player, std::string officer_name, eAllegianceOfficerLevel level)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficer(playerNode))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	uint32_t officerID = g_pDBIO->GetPlayerCharacterId(officer_name.c_str());
	if (!officerID)
		return;

	if (AllegianceTreeNode* officerNode = GetTreeNode(officerID))
	{
		if (IsMonarch(officerNode))
		{
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_NOT_SET, officer_name.c_str());
			return;
		}

		if (IsOfficer(officerNode) && GetOfficerLevel(officerID) == level)
		{
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_ALREADY_OFFICER, officer_name.c_str());
			return;
		}

		if (level <= 0 || level > 3)
		{
			player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_INVALID_LEVEL);
			return;
		}

		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
		{
			if (ai->_info.m_officerList.size() >= 12) {
				player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICER_FULL);
				return;
			}

			if (IsOfficer(officerNode))	{
				ai->_info.m_officerList[officerID] = level;
				player->SendText(csprintf("%s's allegiance officer status has been modified. %s now holds the position of: %s.", officer_name.c_str(), officer_name.c_str(), ai->_info.m_officerTitleList[level - 1].c_str()), LTT_DEFAULT);
			}
			else {
				ai->_info.m_officerList.emplace(officerID, level);
				player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_SET, officer_name.c_str());
			}

			if (CPlayerWeenie* officer = g_pWorld->FindPlayer(officerID)) {
				officer->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_YOU_ARE_NOW_AN_OFFICER, GetOfficerTitle(officer_name).c_str());
			}
			Save();
		}
	}
	else 
		player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_NOT_SET, officer_name.c_str());
}

void AllegianceManager::RemoveOfficer(CPlayerWeenie * player, std::string officer_name)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}
	// only monarchs and level 2+ officers are authorized to remove officers
	if (!IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	uint32_t officerID = g_pDBIO->GetPlayerCharacterId(officer_name.c_str());

	if (AllegianceTreeNode* officerNode = GetTreeNode(officerID))
	{
		if (!IsOfficer(officerNode) || IsMonarch(officerNode)) // can't remove someone who isn't on the list or the monarch
		{
			player->SendText(csprintf("%s is not an allegiance officer!", officer_name.c_str()), LTT_DEFAULT);
			return;
		}

		// only level 3 officers and monarchs can remove level 2+ officers
		if (IsOfficerWithLevel(officerNode, Seneschal_AllegianceOfficerLevel))
		{
			if (!IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel))
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
				return;
			}
		}

		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
		{
			for (auto officer : ai->_info.m_officerList)
			{
				if (officer.first == officerID)
				{
					ai->_info.m_officerList.remove(officerID);
					player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_REMOVED, officer_name.c_str());

					if (CPlayerWeenie* officer = g_pWorld->FindPlayer(officerID)) {
						officer->NotifyWeenieError(WERROR_ALLEGIANCE_YOU_ARE_NO_LONGER_AN_OFFICER);
					}
					Save();
					return;
				}
			}	
		}
	}
	player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_OFFICER_NOT_REMOVED, officer_name.c_str());
}

void AllegianceManager::ListOfficers(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		player->SendText("Allegiance Officers:", LTT_DEFAULT);
		player->SendText(csprintf("%s (Monarch)", g_pDBIO->GetPlayerCharacterName(playerNode->_monarchID).c_str()), LTT_DEFAULT);
		for (auto officer : ai->_info.m_officerList)
		{
			player->SendText(csprintf("%s (%s)", g_pDBIO->GetPlayerCharacterName(officer.first).c_str(), GetOfficerTitle(officer.first).c_str()), LTT_DEFAULT);
		}
	}
}

void AllegianceManager::ClearOfficers(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}
	// only monarch or level 3 officers can clear officer list
	if (!IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		for (auto officer : ai->_info.m_officerList)
		{
			if (CPlayerWeenie* player = g_pWorld->FindPlayer(officer.first))
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_YOU_ARE_NO_LONGER_AN_OFFICER);
			}
		}
		ai->_info.m_officerList.clear();
		player->NotifyWeenieError(WERROR_ALLEGIANCE_OFFICERS_CLEARED);
		Save();
	}
}
// TODO info request time limit
void AllegianceManager::AllegianceInfoRequest(CPlayerWeenie * player, std::string target_name)
{
	AllegianceTreeNode *myNode = GetTreeNode(player->GetID());
	if (!myNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (!IsOfficerWithLevel(myNode, Seneschal_AllegianceOfficerLevel))
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	if (target_name.empty())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_INFO_EMPTY_NAME);
		return;
	}

	if (target_name == player->GetName())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_INFO_SELF);
		return;
	}

	uint32_t targetID = g_pWorld->GetPlayerId(target_name.c_str(), true);

	AllegianceTreeNode *myTargetNode = GetTreeNode(targetID);
	if (!myTargetNode)
	{
		player->SendText("Could not find allegiance member.", LTT_DEFAULT);
		return;
	}

	if (myTargetNode->_monarchID != myNode->_monarchID)
	{
		player->SendText(csprintf("%s is not a member of your allegiance.", target_name.c_str()), LTT_DEFAULT);
		return;
	}	

	// TODO: convert this to server to client message
	unsigned int rank = 0;
	if (AllegianceProfile *profile = CreateAllegianceProfile(myTargetNode->_charID, &rank))
	{
		BinaryWriter allegianceUpdate;
		allegianceUpdate.Write<uint32_t>(0x27C);
		allegianceUpdate.Write<uint32_t>(myTargetNode->_charID);
		profile->Pack(&allegianceUpdate);
		player->SendNetMessage(&allegianceUpdate, PRIVATE_MSG, TRUE, FALSE);

		delete profile;
	}
	else
	{
		player->SendText("Error retrieving allegiance member information.", LTT_DEFAULT);
	}
}

void AllegianceManager::AllegianceLockAction(CPlayerWeenie * player, uint32_t lock_action)
{
	if (AllegianceTreeNode* playerNode = GetTreeNode(player->GetID()))
	{
		if (IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel))
		{
			if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
			{
				switch (lock_action)
				{
				case LockedOff:
				{
					ai->_info.m_isLocked = false;
					player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_SET, "Unlocked");
					Save();
					break;
				}
				case LockedOn:
				{
					ai->_info.m_isLocked = true;
					player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_SET, "Locked");
					Save();
					break;
				}
				case ToggleLocked:
				{
					if (ai->_info.m_isLocked == false)
					{
						ai->_info.m_isLocked = true;
						player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_SET, "Locked");
						Save();
						break;
					}
					else
					{
						ai->_info.m_isLocked = false;
						player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_SET, "Unlocked");
						Save();
						break;
					}
				}
				case CheckLocked:
				{
					player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_DISPLAY, ai->_info.m_isLocked ? "Locked" : "Unlocked");
					break;
				}
				case DisplayBypass: // show approved vassal
				{
					if (!ai->_info.m_ApprovedVassal)
					{
						player->NotifyWeenieError(WERROR_ALLEGIANCE_LOCK_NO_APPROVED);
						break;
					}
					else
					{
						std::string vassal = g_pWorld->GetPlayerName(ai->_info.m_ApprovedVassal, true);
						player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LOCK_APPROVED_DISPLAY, vassal.c_str());
						break;
					}
				}
				case ClearBypass: // clear approved vassal
				{
					ai->_info.m_ApprovedVassal = 0;
					player->NotifyWeenieError(WERROR_ALLEGIANCE_APPROVED_CLEARED);
					Save();
					break;
				}
				}
			}
		}
		else
		{
			player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		}
	}
}

void AllegianceManager::RecallHometown(CPlayerWeenie * player)
{
	if (player->CheckPKActivity())
	{
		player->NotifyWeenieError(WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY);
		return;
	}

	AllegianceTreeNode *allegianceNode = GetTreeNode(player->GetID());

	if (!allegianceNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	AllegianceInfo* ai = GetInfo(allegianceNode->_monarchID);

	if (ai && ai->_info.m_BindPoint.objcell_id) // TODO change to server side store?
	{
		player->ExecuteUseEvent(new CAllegianceHometownRecallUseEvent());
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_HOMETOWN_NOT_SET);
	}
}

void AllegianceManager::ApproveVassal(CPlayerWeenie * player, std::string vassal_name)
{
	uint32_t approvedVassalID = g_pWorld->GetPlayerId(vassal_name.c_str(), true);
	uint32_t playerID = player->GetID();
	AllegianceTreeNode* playerNode = GetTreeNode(playerID);

	if (playerNode && IsOfficerWithLevel(playerNode, Castellan_AllegianceOfficerLevel)) // either a monarch or a level 3 officer
	{
		if (IsBanned(approvedVassalID, playerNode->_monarchID)) // check potential approved vassal's account isn't banned from the allegiance
		{
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_BANNED, vassal_name.c_str());
			return;
		}
		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID)) 
		{
			ai->_info.m_ApprovedVassal = approvedVassalID;
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_APPROVED_SET, vassal_name.c_str());
		}
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
	}
}

void AllegianceManager::BootPlayer(CPlayerWeenie * player, std::string bootee, bool whole_account)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	
	if (!playerNode) // check caller even has an allegiance
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (bootee.empty())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_BOOT_EMPTY_NAME);
		return;
	}

	if (bootee == player->GetName())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_BOOT_SELF);
		return;
	}

	if (IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel)) // check caller has correct authority
	{
		uint32_t booteeID = g_pWorld->GetPlayerId(bootee.c_str(), true);
		if (AllegianceTreeNode* booteeNode = GetTreeNode(booteeID))
		{
			if (booteeNode->_monarchID == playerNode->_monarchID)
			{
				if (whole_account)
				{
					std::list<CharacterDesc_t> bootees = g_pDBIO->GetCharacterList(g_pDBIO->GetCharacterInfo(booteeID).account_id);
					for (auto bootee : bootees)
					{
						if (AllegianceTreeNode* bNode = GetTreeNode(bootee.weenie_id))
						{
							if (bNode->_monarchID == playerNode->_monarchID)
							{
								TryBreakAllegiance(bNode->_patronID, bNode->_charID);
							}
						}
					}
					player->SendText(csprintf("You have successfully booted the account of %s from the allegiance.",bootee.c_str()), LTT_DEFAULT);
					return;
				}

				TryBreakAllegiance(booteeNode->_patronID, booteeID);
				player->SendText(csprintf("You have successfully booted %s from the allegiance.", bootee.c_str()), LTT_DEFAULT);
				Save();
			}
		}
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
	}
}

bool AllegianceManager::IsGagged(uint32_t player_id)
{
	if (AllegianceTreeNode* playerNode = GetTreeNode(player_id)) {
		if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID)) {
			auto it = ai->_info.m_chatGagList.find(player_id);
			if (it != ai->_info.m_chatGagList.end()) {
				return true;
			}
		}
	}
	return false;
}

void AllegianceManager::ChatGag(CPlayerWeenie * player, std::string target, bool toggle)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	if (target.empty())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_BOOT_EMPTY_NAME);
		return;
	}

	if (!IsOfficer(playerNode)) // all officers can gag/ungag
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	uint32_t gagTargetID = g_pDBIO->GetPlayerCharacterId(target.c_str());

	AllegianceTreeNode* gagTargetNode = GetTreeNode(gagTargetID);
	if (!gagTargetNode)
		return;

	if (gagTargetNode->_monarchID != playerNode->_monarchID)
		return;

	if (AllegianceInfo* ai = GetInfo(playerNode->_monarchID))
	{
		CPlayerWeenie* gagTarget = g_pWorld->FindPlayer(gagTargetID);
		auto it = ai->_info.m_chatGagList.find(gagTargetID);

		if (it != ai->_info.m_chatGagList.end()) { // target is in the gag list
			if (toggle) { // adding a gag, fail		
					player->NotifyWeenieError(WERROR_ALLEGIANCE_GAG_ALREADY);
					return;
			}
			else { // removing a gag, success
				ai->_info.m_chatGagList.erase(it);
				player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_GAG_UNGAG_OFFICER, target.c_str());

				if (gagTarget) {
					gagTarget->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_GAG_UNGAG_TARGET, player->GetName().c_str());
				}
				return;
			}
		}
		else { // target not on gag list
			if (toggle) { // adding a gag, success

				int gagEndTime = Timer::cur_time + 300;
				ai->_info.m_chatGagList.add(gagTargetID, &gagEndTime);

				player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_GAG_OFFICER, target.c_str());

				if (gagTarget) {
					gagTarget->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_GAG_TARGET, player->GetName().c_str());
				}
				return;
			}
			else { // removing a gag, fail
				player->NotifyWeenieError(WERROR_ALLEGIANCE_GAG_NOT_ALREADY);
				return;
			}
		}
	}
	// TODO: WERROR_ALLEGIANCE_GAG_AUTO_UNGAG once time is up
	// check player on F7DE send/receive to allegiance channel for a gag
}

void AllegianceManager::ChatBoot(CPlayerWeenie * player, std::string target, std::string reason)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode)
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}
	if (target.empty())
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_BOOT_EMPTY_NAME);
		return;
	}

	if (!IsOfficer(playerNode)) // all officers can boot from allegiance chat
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
		return;
	}

	CPlayerWeenie* targetPlayer = g_pWorld->FindPlayer(target.c_str());
	if (!targetPlayer)
		return;

	AllegianceTreeNode* targetNode = GetTreeNode(targetPlayer->GetID());
	if (!targetNode)
		return;

	if (targetNode->_monarchID != playerNode->_monarchID)
		return;

	if (targetPlayer->_playerModule.options_)
		targetPlayer->_playerModule.options_ &= ~HearAllegianceChat_CharacterOption;

	// TODO: set player option for hear allegiance chat to off
}

bool AllegianceManager::IsBanned(uint32_t player_to_check_id, uint32_t monarch_id)
{
	CharacterDesc_t potentialVassal = g_pDBIO->GetCharacterInfo(player_to_check_id); // get chardesc for account id
	if (AllegianceInfo* ai = GetInfo(monarch_id))
	{
		if (ai->_info.m_BanList.find(potentialVassal.account_id) != ai->_info.m_BanList.end()) // if account id is on the ban list, return true
		{
			return true;
		}
	}
	return false;
}

void AllegianceManager::AddBan(CPlayerWeenie* player, std::string char_name)
{
	uint32_t bannedPlayerID = g_pWorld->GetPlayerId(char_name.c_str());
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode) // check caller even has an allegiance
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	uint32_t monarchID = playerNode->_monarchID;

	if (IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel)) // check caller has correct authority
	{
		CharacterDesc_t bannedCharInfo = g_pDBIO->GetCharacterInfo(bannedPlayerID);
		if (AllegianceInfo* ai = GetInfo(monarchID))
		{
			if (ai->_info.m_BanList.size() > 50) 	// max ban list as of 01/06 patch notes
			{
				player->NotifyWeenieError(WERROR_ALLEGIANCE_BANNED_LIST_FULL);
				return;
			}

			if (ai->_info.m_BanList.find(bannedCharInfo.account_id) != ai->_info.m_BanList.end()) // check if their account is already banned
			{
				player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_CHAR_ALREADY_BANNED, bannedCharInfo.name.c_str());
				return;
			}

			ai->_info.m_BanList.add(bannedCharInfo.account_id, &bannedCharInfo.name); // add account id & name of character to ban list of allegiance

			std::list<CharacterDesc_t> bannedCharacters = g_pDBIO->GetCharacterList(bannedCharInfo.account_id);
			for (auto character : bannedCharacters)
			{
				AllegianceTreeNode* characterNode = GetTreeNode(character.weenie_id);
				if (characterNode && characterNode->_monarchID == monarchID) // if any characters are currently in the allegiance...
				{
					TryBreakAllegiance(characterNode->_patronID, character.weenie_id); // ...boot them from allegiance
				}
			}
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_CHAR_BANNED_SUCCESSFULLY, char_name.c_str());
			Save(); // TODO should we send updated allegiance profile here?
		}
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
	}
}

void AllegianceManager::RemoveBan(CPlayerWeenie * player, std::string char_name)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode) // check caller even has an allegiance
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	uint32_t monarchID = playerNode->_monarchID;

	if (IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel)) // check caller has correct authority
	{
		if (AllegianceInfo* ai = GetInfo(monarchID)) {

			for (auto c : ai->_info.m_BanList) // check if their account is already banned
			{
				if (c.second == char_name)
				{
					ai->_info.m_BanList.remove(c.first);
					player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_CHAR_UNBANNED_SUCCESSFULLY, char_name.c_str());
					Save();
					return;
				}
			}
		}
		player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_CHAR_NOT_BANNED, char_name.c_str());
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
	}
}

void AllegianceManager::GetBanList(CPlayerWeenie * player)
{
	AllegianceTreeNode* playerNode = GetTreeNode(player->GetID());
	if (!playerNode) // check caller even has an allegiance
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NONEXISTENT);
		return;
	}

	uint32_t monarchID = playerNode->_monarchID;

	if (IsOfficerWithLevel(playerNode, Seneschal_AllegianceOfficerLevel)) // check caller has correct authority
	{
		if (AllegianceInfo* ai = GetInfo(monarchID))
		{
			std::string bans;
			for (auto banned : ai->_info.m_BanList)
			{
				bans += banned.second;
				bans += "\n";
			}
			player->NotifyWeenieErrorWithString(WERROR_ALLEGIANCE_LIST_BANNED_CHARACTERS, bans.c_str());
		}
	}
	else
	{
		player->NotifyWeenieError(WERROR_ALLEGIANCE_NOT_AUTHORIZED);
	}
}
