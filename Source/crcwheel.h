
#pragma once

#include "ISAAC.h"

extern void *AllocCrypto(DWORD seed);
extern void FreeCrypto(void *pCryptoBuffer);
extern DWORD GetNextXORVal(void *pCryptoBuffer);


