
#pragma once

uint32_t GetMagicNumber(BYTE *pbBuf, WORD wSize, BOOL fIncludeSize);
uint32_t CalcTransportCRC(uint32_t *pdwWoot);

uint32_t GenericCRC(BlobPacket_s *);
uint32_t BlobCRC(BlobPacket_s *, uint32_t dwXOR);

#include "crcwheel.h"