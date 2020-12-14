
#pragma once

#include "ISAAC.h"

extern void *AllocCrypto(uint32_t seed);
extern void FreeCrypto(void *pCryptoBuffer);
extern uint32_t GetNextXORVal(void *pCryptoBuffer);


