
#pragma once

class VitaeSystem
{
public:
	static DWORD64 VitaeCPPoolThreshold(float cur_vitae, unsigned int level);
	static bool DetermineNewVitaeLevel(float cur_vitae, unsigned int level, DWORD64 *xpPool, float *newVitae); // custom
};

