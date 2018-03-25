
#include "StdAfx.h"
#include "PhatSDK.h"
#include "Housing.h"

DEFINE_PACK(HousePayment)
{
	pWriter->Write<int>(num);
	pWriter->Write<int>(paid);
	pWriter->Write<DWORD>(wcid);
	pWriter->WriteString(name);
	pWriter->WriteString(pname);
}

DEFINE_UNPACK(HousePayment)
{
	num = pReader->Read<int>();
	paid = pReader->Read<int>();
	wcid = pReader->Read<DWORD>();
	name = pReader->ReadString();
	pname = pReader->ReadString();

	return true;
}

DEFINE_PACK(HouseProfile)
{
	pWriter->Write<DWORD>(_slumlord);
	pWriter->Write<DWORD>(_id);
	pWriter->Write<DWORD>(_owner);
	pWriter->Write<DWORD>(_bitmask);
	pWriter->Write<int>(_min_level);
	pWriter->Write<int>(_max_level);
	pWriter->Write<int>(_min_alleg_rank);
	pWriter->Write<int>(_max_alleg_rank);
	pWriter->Write<int>(_maintenance_free);
	pWriter->Write<DWORD>(_type);
	pWriter->WriteString(_name);
	_buy.Pack(pWriter);
	_rent.Pack(pWriter);
}

DEFINE_UNPACK(HouseProfile)
{
	_slumlord = pReader->Read<DWORD>();
	_id = pReader->Read<DWORD>();
	_owner = pReader->Read<DWORD>();
	_bitmask = pReader->Read<DWORD>();
	_min_level = pReader->Read<int>();
	_max_level = pReader->Read<int>();
	_min_alleg_rank = pReader->Read<int>();
	_max_alleg_rank = pReader->Read<int>();
	_maintenance_free = pReader->Read<int>();
	_type = pReader->Read<DWORD>();
	_name = pReader->ReadString();
	_buy.UnPack(pReader);
	_rent.UnPack(pReader);
	return true;
}