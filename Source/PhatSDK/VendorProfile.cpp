
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
	uint32_t amount24 = (uint32_t)amount & 0xFFFFFFu;

	if(pwd) {
		pWriter->Write<uint32_t>(0xFF000000u  | amount24);
		pWriter->Write<uint32_t>(iid);
		pwd->Pack(pWriter);
	} else {
		pWriter->Write<uint32_t>(amount24);
		pWriter->Write<uint32_t>(iid);
	}
}

DEFINE_UNPACK(ItemProfile)
{
	SafeDelete(pwd);

	uint32_t packed = pReader->Read<uint32_t>();
	bool has_pwd = (packed >> 24) == 0xFFu;
	this->amount = packed & 0x00FFFFFFu;

	//Sign-extend 24-bit value to 32-bit integer
	if(this->amount & 0x80000u) {
		this->amount |= 0xFF000000;
	}

	iid = pReader->Read<uint32_t>();

	if(has_pwd)
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