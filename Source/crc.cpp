
#include <StdAfx.h>
#include "crc.h"

uint32_t GetMagicNumber(BYTE *pbBuf, WORD wSize, BOOL fIncludeSize)
{
	uint32_t dwCS = 0;
	int i;

	//if ( fIncludeSize ) // PRE_TOD only
	dwCS += wSize << 16;

	for (i = 0; i < (wSize >> 2); ++i)
		dwCS += ((uint32_t *)pbBuf)[i];

	int iShift = 3;
	for (i = (i << 2); i < wSize; ++i)
	{
		dwCS += pbBuf[i] << (iShift * 8);
		--iShift;
	}

	return dwCS;
}

uint32_t CalcTransportCRC(uint32_t *pdwWoot)
{
	uint32_t dwOrg = pdwWoot[2];
	uint32_t dwCrc = 0;

	pdwWoot[2] = 0xBADD70DD;
	dwCrc += GetMagicNumber((BYTE *)pdwWoot, 20, TRUE);
	pdwWoot[2] = dwOrg;

	return dwCrc;
}

uint32_t GenericCRC(BlobPacket_s *p)
{
	uint32_t dwCrc1, dwCrc2, *pdwCrc;

	pdwCrc = &p->header.dwCRC;
	*pdwCrc = 0xBADD70DD;
	dwCrc1 = CalcTransportCRC((uint32_t *)p);
	dwCrc2 = GetMagicNumber(p->data, p->header.wSize, FALSE);
	*pdwCrc = dwCrc1 + dwCrc2;

	return (dwCrc1 + dwCrc2);
}

uint32_t BlobCRC(BlobPacket_s *p, uint32_t dwXOR)
{
	BYTE *pbPayload = p->data;
	BYTE *pbPayloadEnd = p->data + p->header.wSize;

	uint32_t dwFlags = p->header.dwFlags;

	uint32_t dwHeaderCRC = CalcTransportCRC((uint32_t *)p);
	uint32_t dwPayloadCRC = 0;

	uint16_t optionSize = 0;

	if (dwFlags & BT_ACKSEQUENCE)
	{
		optionSize += sizeof(uint32_t);
		//dwPayloadCRC += GetMagicNumber(pbPayload, sizeof(uint32_t), TRUE);
		//pbPayload +=
	}

	if (dwFlags & BT_TIMEUPDATE)
	{
		optionSize += sizeof(double);
		//dwPayloadCRC += GetMagicNumber(pbPayload, sizeof(double), TRUE);
		//pbPayload += sizeof(double);
	}

	if (dwFlags & BT_ECHOREQUEST)
	{
		optionSize += sizeof(uint32_t);
		//dwPayloadCRC += GetMagicNumber(pbPayload, 4, TRUE);
		//pbPayload += 4;
	}

	if (dwFlags & BT_ECHORESPONSE)
	{
		optionSize += 8;
		//dwPayloadCRC += GetMagicNumber(pbPayload, 8, TRUE);
		//pbPayload += 8;
	}

	if (dwFlags & BT_FLOW)
	{
		optionSize += 6;
		//dwPayloadCRC += GetMagicNumber(pbPayload, 6, TRUE);
		//pbPayload += 6;
	}
	if (optionSize > 0)
	{
		dwPayloadCRC += GetMagicNumber(pbPayload, optionSize, TRUE);
		pbPayload += optionSize;
	}

	if (dwFlags & BT_FRAGMENTS)
	{
		while (pbPayload < pbPayloadEnd)
		{
			WORD wLength = reinterpret_cast<FragHeader_s *>(pbPayload)->wSize;

			dwPayloadCRC += GetMagicNumber(pbPayload, sizeof(FragHeader_s), TRUE) + GetMagicNumber(pbPayload + sizeof(FragHeader_s), wLength - sizeof(FragHeader_s), TRUE);
			pbPayload += wLength;
		}
	}

	uint32_t dwFinalCRC = dwHeaderCRC + (dwPayloadCRC ^ dwXOR);
	return dwFinalCRC;
}






