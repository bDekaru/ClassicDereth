
#include <StdAfx.h>
#include "WeenieObject.h"
#include "Book.h"
#include "Player.h"

CBookWeenie::CBookWeenie()
{
	SetName("Book");
	m_Qualities.m_WeenieType = Book_WeenieType;
}

CBookWeenie::~CBookWeenie()
{
}

void CBookWeenie::ApplyQualityOverrides()
{
}

int CBookWeenie::Use(CPlayerWeenie *pOther)
{
	BinaryWriter bookDataMessage;
	bookDataMessage.Write<uint32_t>(0xB4);
	bookDataMessage.Write<uint32_t>(GetID());
	
	uint32_t maxNumPages = m_Qualities._pageDataList ? m_Qualities._pageDataList->maxNumPages : 0;
	bookDataMessage.Write<uint32_t>(maxNumPages);
	if (m_Qualities._pageDataList)
		m_Qualities._pageDataList->Pack(&bookDataMessage);

	bookDataMessage.WriteString(InqStringQuality(INSCRIPTION_STRING, ""));
	bookDataMessage.Write<uint32_t>(InqIIDQuality(SCRIBE_IID, 0));
	bookDataMessage.WriteString(InqStringQuality(SCRIBE_NAME_STRING, ""));

	pOther->SendNetMessage(&bookDataMessage, PRIVATE_MSG, TRUE, FALSE);
	pOther->NotifyUseDone(WERROR_NONE);

	return WERROR_NONE;
}
