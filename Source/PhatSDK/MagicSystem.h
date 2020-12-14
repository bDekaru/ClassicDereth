
#pragma once

class MagicSystem
{
public:
	static class SpellComponentTable *GetSpellComponentTable(); // somewwat custom
	static class CSpellTable *GetSpellTable(); // somewhat custom

	static uint32_t GetLowestTaperID();
	static uint32_t DeterminePowerLevelOfComponent(uint32_t scid);
};

