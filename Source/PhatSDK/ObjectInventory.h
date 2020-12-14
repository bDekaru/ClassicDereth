
#pragma once

#include "Packable.h"

class ContentProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	int operator==(const ContentProfile &other);

	uint32_t m_iid = 0;
	uint32_t m_uContainerProperties = 0;
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
	PackableList<uint32_t> _itemsList; // IDList
	PackableList<uint32_t> _containersList; // IDList
	PackableList<InventoryPlacement> _invPlacement;
};
