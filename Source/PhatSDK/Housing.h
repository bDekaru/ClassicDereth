
#pragma once

#include "Packable.h"

class HousePayment : public PackObj
{
public:
	DECLARE_PACKABLE()

	DWORD wcid = 0;
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

	DWORD _slumlord = 0;
	DWORD _id = 0;
	DWORD _owner = 0;
	std::string _name;
	DWORD _bitmask = 0;
	HousePaymentList _buy;
	HousePaymentList _rent;
	int _min_level = -1;
	int _max_level = -1;
	int _min_alleg_rank = -1;
	int _max_alleg_rank = -1;
	int _maintenance_free = 0;
	DWORD _type = 0;
};