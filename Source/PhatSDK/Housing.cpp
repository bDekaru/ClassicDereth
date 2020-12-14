
#include <StdAfx.h>
#include "PhatSDK.h"
#include "Housing.h"

DEFINE_PACK(HousePayment)
{
	pWriter->Write<int>(num);
	pWriter->Write<int>(paid);
	pWriter->Write<uint32_t>(wcid);
	pWriter->WriteString(name);
	pWriter->WriteString(pname);
}

DEFINE_UNPACK(HousePayment)
{
	num = pReader->Read<int>();
	paid = pReader->Read<int>();
	wcid = pReader->Read<uint32_t>();
	name = pReader->ReadString();
	pname = pReader->ReadString();

	return true;
}

DEFINE_PACK(HouseProfile)
{
	pWriter->Write<uint32_t>(_slumlord);
	pWriter->Write<uint32_t>(_id);
	pWriter->Write<uint32_t>(_owner);
	pWriter->Write<uint32_t>(_bitmask);
	pWriter->Write<int>(_min_level);
	pWriter->Write<int>(_max_level);
	pWriter->Write<int>(_min_alleg_rank);
	pWriter->Write<int>(_max_alleg_rank);
	pWriter->Write<int>(_maintenance_free);
	pWriter->Write<uint32_t>(_type);
	pWriter->WriteString(_name);
	_buy.Pack(pWriter);
	_rent.Pack(pWriter);
}

DEFINE_UNPACK(HouseProfile)
{
	_slumlord = pReader->Read<uint32_t>();
	_id = pReader->Read<uint32_t>();
	_owner = pReader->Read<uint32_t>();
	_bitmask = pReader->Read<uint32_t>();
	_min_level = pReader->Read<int>();
	_max_level = pReader->Read<int>();
	_min_alleg_rank = pReader->Read<int>();
	_max_alleg_rank = pReader->Read<int>();
	_maintenance_free = pReader->Read<int>();
	_type = pReader->Read<uint32_t>();
	_name = pReader->ReadString();
	_buy.UnPack(pReader);
	_rent.UnPack(pReader);
	return true;
}