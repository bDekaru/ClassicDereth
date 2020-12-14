
#include <StdAfx.h>
#include "AttributeTransferDevice.h"
#include "UseManager.h"
#include "Player.h"

CAttributeTransferDeviceWeenie::CAttributeTransferDeviceWeenie()
{
}

CAttributeTransferDeviceWeenie::~CAttributeTransferDeviceWeenie()
{
}

int CAttributeTransferDeviceWeenie::Use(CPlayerWeenie *player)
{
	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}
	
	STypeAttribute transferFrom = (STypeAttribute)InqIntQuality(TRANSFER_FROM_ATTRIBUTE_INT, STypeAttribute::UNDEF_ATTRIBUTE);
	STypeAttribute transferTo = (STypeAttribute)InqIntQuality(TRANSFER_TO_ATTRIBUTE_INT, STypeAttribute::UNDEF_ATTRIBUTE);

	if (transferFrom < STRENGTH_ATTRIBUTE || transferFrom > SELF_ATTRIBUTE || transferTo < STRENGTH_ATTRIBUTE || transferTo > SELF_ATTRIBUTE || transferFrom == transferTo)
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	Attribute fromAttrib, toAttrib;
	if (!player->m_Qualities.InqAttribute(transferFrom, fromAttrib) || !player->m_Qualities.InqAttribute(transferTo, toAttrib))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	if (fromAttrib._init_level <= 10)
	{
		player->SendText(csprintf("Your %s is too low to do this.", Attribute::GetAttributeName(transferFrom)), LTT_DEFAULT);
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}
	int maxFromAmount = min((int)fromAttrib._init_level - 10, 10);

	if (toAttrib._init_level >= 100)
	{
		player->SendText(csprintf("Your %s is too high to do this.", Attribute::GetAttributeName(transferTo)), LTT_DEFAULT);
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}
	int maxToAmount = min((int)(100 - toAttrib._init_level), 10);

	// do the attribute transfer
	int transferAmount = min(maxFromAmount, maxToAmount);

	fromAttrib._init_level -= transferAmount;
	player->m_Qualities.SetAttribute(transferFrom, fromAttrib);
	player->NotifyAttributeStatUpdated(transferFrom);

	toAttrib._init_level += transferAmount;
	player->m_Qualities.SetAttribute(transferTo, toAttrib);
	player->NotifyAttributeStatUpdated(transferTo);

	player->SendText(csprintf("Transfer of %s to %s complete.", Attribute::GetAttributeName(transferFrom), Attribute::GetAttributeName(transferTo)), LTT_DEFAULT);
	player->NotifyUseDone(WERROR_NONE);

	DecrementStackOrStructureNum();

	return WERROR_NONE;
}
