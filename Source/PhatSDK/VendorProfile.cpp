
#include "StdAfx.h"
#include "PhatSDK.h"
#include "VendorProfile.h"

#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC

ItemProfile::ItemProfile()
{
}

ItemProfile::~ItemProfile()
{
	SafeDelete(pwd);
}

DEFINE_PACK(ItemProfile)
{
	int foo = 0;
	if (pwd)
		foo = -1;

	pWriter->Write<DWORD>((foo << 24)  | amount & 0xFFFFFF);
	pWriter->Write<DWORD>(iid);	
	if (pwd)
		pwd->Pack(pWriter);
}

DEFINE_UNPACK(ItemProfile)
{
	SafeDelete(pwd);

	DWORD foo = pReader->Read<DWORD>();

	amount = foo;
	if (amount & 0x800000)
		amount = ((foo & 0xFFFFFF) | 0xFF000000);

	iid = pReader->Read<DWORD>();

	int has_pwd = foo >> 24;
	if (has_pwd == -1)
	{
		pwd = new PublicWeenieDesc();
		pwd->UnPack(pReader);
		return true;
	}

	// should unpack old weenie desc here, but it's not used sooo...
	// incase it is used...

	return false;
}

VendorProfile::VendorProfile()
{
}

VendorProfile::~VendorProfile()
{
}

DEFINE_PACK(VendorProfile)
{
	pWriter->Write<DWORD>(item_types);
	pWriter->Write<int>(min_value);
	pWriter->Write<int>(max_value);
	pWriter->Write<int>(magic);
	pWriter->Write<float>(buy_price);
	pWriter->Write<float>(sell_price);
	pWriter->Write<DWORD>(trade_id);
	pWriter->Write<int>(trade_num);
	pWriter->WriteString(trade_name);
}

DEFINE_UNPACK(VendorProfile)
{
	UNFINISHED();
	return true;
}

DWORD VendorProfile::VendorTradeCurrency()
{
	return trade_id;
}

#endif