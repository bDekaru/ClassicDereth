
#include "StdAfx.h"
#include "PhatSDK.h"
#include "AllegianceProfile.h"

DEFINE_PACK(AllegianceData)
{
	_bitfield |= HasPackedLevel_AllegianceIndex | HasAllegianceAge_AllegianceIndex;

	pWriter->Write<DWORD>(_id);
	pWriter->Write<DWORD>(_cp_cached);
	pWriter->Write<DWORD>(_cp_tithed);
	pWriter->Write<DWORD>(_bitfield);
	pWriter->Write<BYTE>(_gender);
	pWriter->Write<BYTE>(_hg);
	pWriter->Write<WORD>(_rank);
	pWriter->Write<DWORD>(_level);
	pWriter->Write<WORD>(_loyalty);
	pWriter->Write<WORD>(_leadership);
	pWriter->Write<int>(_time_online);
	pWriter->Write<int>(_allegiance_age);
	pWriter->WriteString(_name);
}

DEFINE_UNPACK(AllegianceData)
{
	_id = pReader->Read<DWORD>();
	_cp_cached = pReader->Read<DWORD>();
	_cp_tithed = pReader->Read<DWORD>();
	_bitfield = pReader->Read<DWORD>();
	_gender = pReader->Read<BYTE>();
	_hg = pReader->Read<BYTE>();
	_rank = pReader->Read<WORD>();
	if (_bitfield & HasPackedLevel_AllegianceIndex)
		_level = pReader->Read<DWORD>();
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
	// pWriter->Write<DWORD>(0xB0000 | (m_total & 0xFFFF);
	pWriter->Write<DWORD>(((DWORD)Newest_AllegianceVersion << 16) | _nodes.size());
	m_AllegianceOfficers.Pack(pWriter);
	m_OfficerTitles.Pack(pWriter);
	pWriter->Write<int>(m_monarchBroadcastTime);
	pWriter->Write<DWORD>(m_monarchBroadcastsToday);
	pWriter->Write<int>(m_spokesBroadcastTime);
	pWriter->Write<DWORD>(m_spokesBroadcastsToday);
	pWriter->WriteString(m_motd);
	pWriter->WriteString(m_motdSetBy);
	pWriter->Write<DWORD>(m_chatRoomID);
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
			pWriter->Write<DWORD>(entry->_patron ? entry->_patron->_data._id : 0);
		else
			bMonarch = false;

		entry->Pack(pWriter);
	}
}

DEFINE_UNPACK(AllegianceHierarchy)
{
	Clear();

	DWORD header = pReader->Read<DWORD>();

	m_oldVersion = (AllegianceVersion)(header >> 16);
	assert(m_oldVersion >= Newest_AllegianceVersion); // we dont handle old versions

	WORD packedNodes = header & 0xFFFF;

	m_AllegianceOfficers.UnPack(pReader);
	m_OfficerTitles.UnPack(pReader);
	m_monarchBroadcastTime = pReader->Read<int>();
	m_monarchBroadcastsToday = pReader->Read<DWORD>();
	m_spokesBroadcastTime = pReader->Read<int>();
	m_spokesBroadcastsToday = pReader->Read<DWORD>();
	m_motd = pReader->ReadString();
	m_motdSetBy = pReader->ReadString();
	m_chatRoomID = pReader->Read<DWORD>();
	m_BindPoint.UnPack(pReader);
	m_AllegianceName = pReader->ReadString();
	m_NameLastSetTime = pReader->Read<int>();
	m_isLocked = pReader->Read<int>();
	m_ApprovedVassal = pReader->Read<int>();
	
	/*
	AllegianceData data;
	for (DWORD i = 0; i < packedNodes; i++)
	{
		// this way of doing things is unsettling, let's not
		UNFINISHED();
	}
	*/

	// normally would pack m_pMonarch nodes here, but we'll use a different means
	assert(!packedNodes);

	bool bMonarch = true;
	for (DWORD i = 0; i < packedNodes; i++)
	{
		AllegianceNode *node = new AllegianceNode();

		DWORD patronID = 0;
		if (!bMonarch)
			patronID = pReader->Read<DWORD>(); // pWriter->Write<DWORD>(entry->_patron ? entry->_patron->_data._id : 0);
		else
			bMonarch = false;

		node->UnPack(pReader);
		_nodes.push_back(node);
	}

	// if (m_pMonarch)
	//	m_pMonarch->SetMayPassupExperience(FALSE);

	return true;
}

DEFINE_PACK(AllegianceProfile)
{
	pWriter->Write<DWORD>(_total_members);
	pWriter->Write<DWORD>(_total_vassals);
	_allegiance.Pack(pWriter);
}

DEFINE_UNPACK(AllegianceProfile)
{
	_total_members = pReader->Read<DWORD>();
	_total_vassals = pReader->Read<DWORD>();
	_allegiance.UnPack(pReader);
	return true;
}
