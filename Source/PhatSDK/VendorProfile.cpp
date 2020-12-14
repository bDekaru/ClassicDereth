
#include <StdAfx.h>
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

	pWriter->Write<uint32_t>((foo << 24)  | amount & 0xFFFFFF);
	pWriter->Write<uint32_t>(iid);	
	if (pwd)
		pwd->Pack(pWriter);
}

DEFINE_UNPACK(ItemProfile)
{
	SafeDelete(pwd);

	uint32_t foo = pReader->Read<uint32_t>();

	amount = foo;
	if (amount & 0x800000)
		amount = ((foo & 0xFFFFFF) | 0xFF000000);

	iid = pReader->Read<uint32_t>();

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
	pWriter->Write<uint32_t>(item_types);
	pWriter->Write<int>(min_value);
	pWriter->Write<int>(max_value);
	pWriter->Write<int>(magic);
	pWriter->Write<float>(buy_price);
	pWriter->Write<float>(sell_price);
	pWriter->Write<uint32_t>(trade_id);
	pWriter->Write<int>(trade_num);
	pWriter->WriteString(trade_name);
}

DEFINE_UNPACK(VendorProfile)
{
	UNFINISHED();
	return true;
}

uint32_t VendorProfile::VendorTradeCurrency()
{
	return trade_id;
}

#endif