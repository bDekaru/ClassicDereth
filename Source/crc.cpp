
#include "StdAfx.h"
#include "crc.h"

DWORD GetMagicNumber(BYTE *pbBuf, WORD wSize, BOOL fIncludeSize)
{
	DWORD dwCS = 0;
	int i;

	//if ( fIncludeSize ) // PRE_TOD only
	dwCS += wSize << 16;

	for (i = 0; i < (wSize >> 2); ++i)
		dwCS += ((DWORD *)pbBuf)[i];

	int iShift = 3;
	for (i = (i << 2); i < wSize; ++i)
	{
		dwCS += pbBuf[i] << (iShift * 8);
		--iShift;
	}

	return dwCS;
}

DWORD CalcTransportCRC(DWORD *pdwWoot)
{
	DWORD dwOrg = pdwWoot[2];
	DWORD dwCrc = 0;

	pdwWoot[2] = 0xBADD70DD;
	dwCrc += GetMagicNumber((BYTE *)pdwWoot, 20, TRUE);
	pdwWoot[2] = dwOrg;

	return dwCrc;
}

DWORD GenericCRC(BlobPacket_s *p)
{
	DWORD dwCrc1, dwCrc2, *pdwCrc;

	pdwCrc = &p->header.dwCRC;
	*pdwCrc = 0xBADD70DD;
	dwCrc1 = CalcTransportCRC((DWORD *)p);
	dwCrc2 = GetMagicNumber(p->data, p->header.wSize, FALSE);
	*pdwCrc = dwCrc1 + dwCrc2;

	return (dwCrc1 + dwCrc2);
}

DWORD BlobCRC(BlobPacket_s *p, DWORD dwXOR)
{
	BYTE *pbPayload = p->data;
	BYTE *pbPayloadEnd = p->data + p->header.wSize;

	DWORD dwFlags = p->header.dwFlags;

	DWORD dwHeaderCRC = CalcTransportCRC((DWORD *)p);
	DWORD dwPayloadCRC = 0;

	if (dwFlags & BT_TIMEUPDATE)
	{
		dwPayloadCRC += GetMagicNumber(pbPayload, sizeof(double), TRUE);
		pbPayload += sizeof(double);
	}

	if (dwFlags & BT_ECHOREQUEST)
	{
		dwPayloadCRC += GetMagicNumber(pbPayload, 4, TRUE);
		pbPayload += 4;
	}

	if (dwFlags & BT_ECHORESPONSE)
	{
		dwPayloadCRC += GetMagicNumber(pbPayload, 8, TRUE);
		pbPayload += 8;
	}

	if (dwFlags & BT_FLOW)
	{
		dwPayloadCRC += GetMagicNumber(pbPayload, 6, TRUE);
		pbPayload += 6;
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

	DWORD dwFinalCRC = dwHeaderCRC + (dwPayloadCRC ^ dwXOR);
	return dwFinalCRC;
}






