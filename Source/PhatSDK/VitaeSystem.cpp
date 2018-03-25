
#include "StdAfx.h"
#include "PhatSDK.h"
#include "VitaeSystem.h"

DWORD64 VitaeSystem::VitaeCPPoolThreshold(float cur_vitae, unsigned int level)
{
	return (DWORD64)((pow((double)level, 2.5) * 2.5 + 20.0) * pow(cur_vitae, 5.0) + 0.5);
}

bool VitaeSystem::DetermineNewVitaeLevel(float cur_vitae, unsigned int level, DWORD64 *xpPool, float *newVitae) // custom
{
	if ((cur_vitae + F_EPSILON) >= 1.0f)
	{
		return false;
	}

	bool has_new_vitae = false;
	float new_vitae = cur_vitae;

	while ((new_vitae + F_EPSILON) < 1.0f)
	{
		DWORD64 xpNeeded = VitaeCPPoolThreshold(cur_vitae, level);

		if (xpNeeded > *xpPool)
		{
			*newVitae = new_vitae;
			return has_new_vitae;
		}
		else
		{
			has_new_vitae = true;
			new_vitae += 0.01f;
			*xpPool -= xpNeeded;
		}
	}

	*newVitae = 1.0f;
	*xpPool = 0;
	return true;
}