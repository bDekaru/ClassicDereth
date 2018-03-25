
#include "StdAfx.h"
#include "LegacyPackObj.h"

LegacyPackObj::LegacyPackObj()
{
}

LegacyPackObj::~LegacyPackObj()
{
}

ULONG LegacyPackObj::GetPackSize()
{
    return pack_size();
}

ULONG LegacyPackObj::pack_size()
{
    return 0;
}

BOOL LegacyPackObj::ALIGN_PTR(BYTE **ppData, ULONG* piSize)
{
    unsigned long alignBytes = 4 - (((size_t)*ppData) & 3);

    if (alignBytes != 4)
    {
        if ((*piSize) < alignBytes)
            return FALSE; // Not enough room.

        // Adjust the size.
        *piSize = (*piSize) - alignBytes;

        // Write null bytes on the padding. (ugly, matches turbine -- this can be changed later?)
        while(alignBytes > 0)
        {
            *(*ppData) = 0;
            *ppData = *ppData + 1;
            alignBytes--;
        }
    }

    return TRUE;
}