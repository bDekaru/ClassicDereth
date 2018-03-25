
#include "StdAfx.h"
#include "ObjectInventory.h"

int ContentProfile::operator==(const ContentProfile &other)
{
	return m_iid == other.m_iid;
}

DEFINE_PACK(ContentProfile)
{
	pWriter->Write<DWORD>(m_iid);
	pWriter->Write<DWORD>(m_uContainerProperties);
}

DEFINE_UNPACK(ContentProfile)
{
	m_iid = pReader->Read<DWORD>();
	m_uContainerProperties = pReader->Read<DWORD>();
	return true;
}

DEFINE_PACK(InventoryPlacement)
{
	pWriter->Write<DWORD>(iid_);
	pWriter->Write<DWORD>(loc_);
	pWriter->Write<DWORD>(priority_);
}

DEFINE_UNPACK(InventoryPlacement)
{
	iid_ = pReader->Read<DWORD>();
	loc_ = pReader->Read<DWORD>();
	priority_ = pReader->Read<DWORD>();
	return true;
}



