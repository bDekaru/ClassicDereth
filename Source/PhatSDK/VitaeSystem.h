
#pragma once

class VitaeSystem
{
public:
	static uint64_t VitaeCPPoolThreshold(float cur_vitae, unsigned int level);
	static bool DetermineNewVitaeLevel(float cur_vitae, unsigned int level, uint64_t *xpPool, float *newVitae); // custom
};

