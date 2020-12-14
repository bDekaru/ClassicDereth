
#pragma once

#include "Packable.h"

class ItemProfile : public PackObj
{
public:
	ItemProfile();
	virtual ~ItemProfile() override;

	DECLARE_PACKABLE()

	int amount = 0;
	unsigned int iid = 0;
	class PublicWeenieDesc *pwd = NULL;
};

class VendorProfile : public PackObj
{
public:
	VendorProfile();
	virtual ~VendorProfile() override;

	DECLARE_PACKABLE()

	uint32_t VendorTradeCurrency();

	unsigned int item_types = 0;
	int min_value = -1;
	int max_value = -1;
	int magic = 0;
	float buy_price = 0.9f;
	float sell_price = 1.1f;
	uint32_t trade_id = 0;
	int trade_num = 0;
	std::string trade_name;
};
