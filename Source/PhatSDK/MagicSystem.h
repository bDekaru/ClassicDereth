
#pragma once

class MagicSystem
{
public:
	static class SpellComponentTable *GetSpellComponentTable(); // somewwat custom
	static class CSpellTable *GetSpellTable(); // somewhat custom

	static DWORD GetLowestTaperID();
	static DWORD DeterminePowerLevelOfComponent(DWORD scid);
};

