
#include <StdAfx.h>
#include "PhatSDK.h"
#include "AllegianceProfile.h"
#include "AllegianceManager.h"

DEFINE_PACK(AllegianceData)
{
	_bitfield |= HasPackedLevel_AllegianceIndex | HasAllegianceAge_AllegianceIndex;

	pWriter->Write<uint32_t>(_id);
	pWriter->Write<uint32_t>(_cp_cached);
	pWriter->Write<uint32_t>(_cp_tithed);
	pWriter->Write<uint32_t>(_bitfield);
	pWriter->Write<BYTE>(_gender);
	pWriter->Write<BYTE>(_hg);
	pWriter->Write<WORD>(_rank);
	pWriter->Write<uint32_t>(_level);
	pWriter->Write<WORD>(_loyalty);
	pWriter->Write<WORD>(_leadership);
	pWriter->Write<int>(_time_online);
	pWriter->Write<int>(_allegiance_age);
	pWriter->WriteString(_name);
}

DEFINE_UNPACK(AllegianceData)
{
	_id = pReader->Read<uint32_t>();
	_cp_cached = pReader->Read<uint32_t>();
	_cp_tithed = pReader->Read<uint32_t>();
	_bitfield = pReader->Read<uint32_t>();
	_gender = pReader->Read<BYTE>();
	_hg = pReader->Read<BYTE>();
	_rank = pReader->Read<WORD>();
	if (_bitfield & HasPackedLevel_AllegianceIndex)
		_level = pReader->Read<uint32_t>();
	else
		_bitfield |= MayPassupExperience_AllegianceIndex;

	_loyalty = pReader->Read<WORD>();
	_leadership = pReader->Read<WORD>();
	if (_bitfield & HasAllegianceAge_AllegianceIndex)
	{
		_time_online = pReader->Read<int>();
		_allegiance_age = pReader->Read<int>();
	}
	else
	{
		_time_online = pReader->Read<int>();
		_allegiance_age = 0;
	}
	_name = pReader->ReadString();

	return true;
}

DEFINE_PACK(AllegianceNode)
{
	_data.Pack(pWriter);
}

DEFINE_UNPACK(AllegianceNode)
{
	_data.UnPack(pReader);
	return true;
}

void AllegianceNode::SetMayPassupExperience(BOOL val)
{
	_data._bitfield = val ? _data._bitfield | MayPassupExperience_AllegianceIndex : _data._bitfield & ~MayPassupExperience_AllegianceIndex;
}

AllegianceHierarchy::AllegianceHierarchy()
{
}

AllegianceHierarchy::~AllegianceHierarchy()
{
	Clear();
}

void AllegianceHierarchy::Clear()
{
	for (auto entry : _nodes)
		delete entry;
	_nodes.clear();
}

DEFINE_PACK(AllegianceHierarchy)
{
	// pWriter->Write<uint32_t>(0xB0000 | (m_total & 0xFFFF);
	pWriter->Write<uint32_t>(((uint32_t)Newest_AllegianceVersion << 16) | _nodes.size());
	m_AllegianceOfficers.Pack(pWriter);
	m_OfficerTitles.Pack(pWriter);
	pWriter->Write<int>(m_monarchBroadcastTime);
	pWriter->Write<uint32_t>(m_monarchBroadcastsToday);
	pWriter->Write<int>(m_spokesBroadcastTime);
	pWriter->Write<uint32_t>(m_spokesBroadcastsToday);
	pWriter->WriteString(m_motd);
	pWriter->WriteString(m_motdSetBy);
	pWriter->Write<uint32_t>(m_chatRoomID);
	m_BindPoint.Pack(pWriter);
	pWriter->WriteString(m_AllegianceName);
	pWriter->Write<int>(m_NameLastSetTime);
	pWriter->Write<int>(m_isLocked);
	pWriter->Write<int>(m_ApprovedVassal);

	// normally would pack m_pMonarch nodes here, but we'll use a different means
	bool bMonarch = true;
	for (auto &entry : _nodes)
	{
		if (!bMonarch)
			pWriter->Write<uint32_t>(entry->_patron ? entry->_patron->_data._id : 0);
		else
			bMonarch = false;

		entry->Pack(pWriter);
	}

	if (packForDB) {
		pWriter->WriteString(m_storedMOTD);
		pWriter->WriteString(m_storedMOTDSetBy);

		for (int i = 0; i < 3; i++) {
			pWriter->WriteString(m_officerTitleList[i]);
		}

		m_officerList.Pack(pWriter);
		m_BanList.Pack(pWriter);
		m_chatGagList.Pack(pWriter);
		packForDB = false;
	}
}

DEFINE_UNPACK(AllegianceHierarchy)
{
	Clear();

	uint32_t header = pReader->Read<uint32_t>();

	m_oldVersion = (AllegianceVersion)(header >> 16);
	assert(m_oldVersion >= Newest_AllegianceVersion); // we dont handle old versions

	WORD packedNodes = header & 0xFFFF;

	m_AllegianceOfficers.UnPack(pReader);
	m_OfficerTitles.UnPack(pReader);
	m_monarchBroadcastTime = pReader->Read<int>();
	m_monarchBroadcastsToday = pReader->Read<uint32_t>();
	m_spokesBroadcastTime = pReader->Read<int>();
	m_spokesBroadcastsToday = pReader->Read<uint32_t>();
	m_motd = pReader->ReadString();
	m_motdSetBy = pReader->ReadString();
	m_chatRoomID = pReader->Read<uint32_t>();
	m_BindPoint.UnPack(pReader);
	m_AllegianceName = pReader->ReadString();
	m_NameLastSetTime = pReader->Read<int>();
	m_isLocked = pReader->Read<int>();
	m_ApprovedVassal = pReader->Read<int>();

	// normally would pack m_pMonarch nodes here, but we'll use a different means
	assert(!packedNodes);

	bool bMonarch = true;
	for (uint32_t i = 0; i < packedNodes; i++)
	{
		AllegianceNode *node = new AllegianceNode();

		uint32_t patronID = 0;
		if (!bMonarch)
			patronID = pReader->Read<uint32_t>(); 
		else
			bMonarch = false;

		node->UnPack(pReader);
		_nodes.push_back(node);
	}

	m_storedMOTD = pReader->ReadString();
	m_storedMOTDSetBy = pReader->ReadString();

	for (int i = 0; i < 3; i++) {
		m_officerTitleList[i] = pReader->ReadString();
		if (m_officerTitleList[i].empty())
			switch (i)
			{
			case 0: m_officerTitleList[i] = "Speaker"; break;
			case 1: m_officerTitleList[i] = "Seneschal"; break;
			case 2:m_officerTitleList[i] = "Castellan"; break;
			}
	}

	m_officerList.UnPack(pReader);
	m_BanList.UnPack(pReader);
	m_chatGagList.UnPack(pReader);

	return true;
}

DEFINE_PACK(AllegianceProfile)
{
	pWriter->Write<uint32_t>(_total_members);
	pWriter->Write<uint32_t>(_total_vassals);
	_allegiance.Pack(pWriter);
}

DEFINE_UNPACK(AllegianceProfile)
{
	_total_members = pReader->Read<uint32_t>();
	_total_vassals = pReader->Read<uint32_t>();
	_allegiance.UnPack(pReader);
	return true;
}
