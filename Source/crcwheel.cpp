
#include <StdAfx.h>

typedef QTIsaac<(8), unsigned int> AC_Isaac;

void *AllocCrypto(uint32_t seed)
{
	return new AC_Isaac(seed, seed, seed);
}

void FreeCrypto(void *pCryptoBuffer)
{
	if (pCryptoBuffer)
	{
		delete ((AC_Isaac *)pCryptoBuffer);
	}
}

uint32_t GetNextXORVal(void *pCryptoBuffer)
{
	return ((AC_Isaac *)pCryptoBuffer)->rand();
}
