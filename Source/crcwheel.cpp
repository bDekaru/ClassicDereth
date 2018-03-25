
#include "StdAfx.h"

typedef QTIsaac<(8), unsigned int> AC_Isaac;

void *AllocCrypto(DWORD seed)
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

DWORD GetNextXORVal(void *pCryptoBuffer)
{
	return ((AC_Isaac *)pCryptoBuffer)->rand();
}
