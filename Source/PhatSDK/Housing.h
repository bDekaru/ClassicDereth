
#pragma once

#include "Packable.h"

class HousePayment : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t wcid = 0;
	int num = 0;
	int paid = 0;
	std::string name;
	std::string pname;
};

class HousePaymentList : public PackableList<HousePayment>
{
public:
};

class HouseProfile : public PackObj
{
public:
	DECLARE_PACKABLE()

	uint32_t _slumlord = 0;
	uint32_t _id = 0;
	uint32_t _owner = 0;
	std::string _name;
	uint32_t _bitmask = 0;
	HousePaymentList _buy;
	HousePaymentList _rent;
	int _min_level = -1;
	int _max_level = -1;
	int _min_alleg_rank = -1;
	int _max_alleg_rank = -1;
	int _maintenance_free = 0;
	uint32_t _type = 0;
};