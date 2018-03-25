
#include "StdAfx.h"
#include "LegacyPackObj.h"
#include "PString.h"

PString::PString()
{
}

PString::PString(const char* szString)
{
	m_str = szString;
}

PString::~PString() {
    Destroy();
}

void PString::Destroy()
{
	m_str = "";
}

BOOL PString::UnPack(BYTE** ppData, ULONG iSize)
{
    DWORD Length;

    // Default pack includes 16-bit length.
    WORD Length16;
    if (!UNPACK(WORD, Length16))
        return FALSE;

    if (Length16 == 0xFFFF)
    {
        // If necessary will include 32-bit length.
        DWORD Length32;
        if (!UNPACK(DWORD, Length32))
            return FALSE;

        Length = Length32;
    }
    else
        Length = Length16;

    if (iSize < Length)
        return FALSE;

    // Not the real code.
    DWORD allocLength = Length + 1;
    char *TempString = new char[allocLength];
    memcpy(TempString, *ppData, Length);
    TempString[Length] = '\0';
    *ppData += Length;
    iSize -= Length;

	m_str = TempString;
	delete [] TempString;

    PACK_ALIGN();

    return TRUE;
}

DWORD PString::compute_hash(const char *str)
{
	DWORD result = 0;

	while (*str)
	{
		result = (result * 16) + *str;
		if (result & 0xF0000000)
			result = (result ^ ((result & 0xF0000000) >> 24)) & 0xFFFFFFF;
		str++;
	}
	if (result == (DWORD) -1)
		result = (DWORD) -2;

	return result;
}

