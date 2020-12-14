
#include <StdAfx.h>
#include "ObjectInventory.h"

int ContentProfile::operator==(const ContentProfile &other)
{
	return m_iid == other.m_iid;
}

DEFINE_PACK(ContentProfile)
{
	pWriter->Write<uint32_t>(m_iid);
	pWriter->Write<uint32_t>(m_uContainerProperties);
}

DEFINE_UNPACK(ContentProfile)
{
	m_iid = pReader->Read<uint32_t>();
	m_uContainerProperties = pReader->Read<uint32_t>();
	return true;
}

DEFINE_PACK(InventoryPlacement)
{
	pWriter->Write<uint32_t>(iid_);
	pWriter->Write<uint32_t>(loc_);
	pWriter->Write<uint32_t>(priority_);
}

DEFINE_UNPACK(InventoryPlacement)
{
	iid_ = pReader->Read<uint32_t>();
	loc_ = pReader->Read<uint32_t>();
	priority_ = pReader->Read<uint32_t>();
	return true;
}



