
#pragma once
#include "easylogging++.h"

class CKillable
{
public:
	CKillable()
	{
		bAlive = TRUE;
	}

	void Kill(const char *szSource, uint32_t dwLine)
	{
#ifdef _DEBUG
		if (szSource)
		{
			DEBUG_DATA << "Kill() @" << szSource << ":" << dwLine;
		}

#endif
		bAlive = FALSE;
	}

	BOOL IsAlive()
	{
		return bAlive ? TRUE : FALSE;
	}

protected:
	BOOL bAlive;
};