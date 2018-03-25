
#pragma once

#include "Packable.h"

class ContentProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	int operator==(const ContentProfile &other);

	DWORD m_iid = 0;
	DWORD m_uContainerProperties = 0;
};

class InventoryPlacement : public PackObj
{
public:
	DECLARE_PACKABLE()

	unsigned int iid_ = 0;
	unsigned int loc_ = 0;
	unsigned int priority_ = 0;
};

class CObjectInventory : public LongHashData
{
public:
	PackableList<DWORD> _itemsList; // IDList
	PackableList<DWORD> _containersList; // IDList
	PackableList<InventoryPlacement> _invPlacement;
};
