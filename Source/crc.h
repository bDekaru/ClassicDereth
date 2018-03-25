
#pragma once

DWORD GetMagicNumber(BYTE *pbBuf, WORD wSize, BOOL fIncludeSize);
DWORD CalcTransportCRC(DWORD *pdwWoot);

DWORD GenericCRC(BlobPacket_s *);
DWORD BlobCRC(BlobPacket_s *, DWORD dwXOR);

#include "crcwheel.h"