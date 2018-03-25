
#pragma once

class CKillable
{
public:
	CKillable()
	{
		bAlive = TRUE;
	}

	void Kill(const char *szSource, DWORD dwLine)
	{
#ifdef _DEBUG
		if (szSource)
		{
			LOG(Temp, Debug, "Kill() @ %s: %u\n", szSource, dwLine);
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