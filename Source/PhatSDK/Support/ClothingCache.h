
#pragma once

class CClothingCache
{
public:
	void LoadAll();
	void ReleaseAll();

	DWORD GetNumTablesOfSetupID(DWORD setup_id);
	class ClothingTable *GetTableByIndexOfSetupID(DWORD setup_id, DWORD index);

	DWORD GetNumTablesOfIconID(DWORD icon_id);
	class ClothingTable *GetTableByIndexOfIconID(DWORD icon_id, DWORD index, DWORD *palette_key);

	static const char *PaletteTemplateIDToString(unsigned int pt);

	bool TryToMatchBySetupAndObjDesc(DWORD setup_id, ObjDesc &od, DWORD *table_id, DWORD *pt, double *shades);
	bool TryToMatchBySetupAndObjDesc(DWORD setup_id, ClothingTable *ct, ObjDesc &od, DWORD *pt, double *shades);
	bool TryToMatchByObjDescWithPaletteTemplateAndGetShades(CloPaletteTemplate *pt, ObjDesc &od, double *shades);

private:
	static void ParseClothingFileStatic(void *argument, DWORD id, BTEntry *entry);
	void ParseClothing(DWORD id);

	typedef std::map<DWORD, std::set<DWORD>> CLOTHINGBYSETUPIDMAP;
	CLOTHINGBYSETUPIDMAP m_SetupIDToClothingTableID;

	typedef std::map<DWORD, std::set<DWORD>> CLOTHINGBYICONIDMAP;
	CLOTHINGBYICONIDMAP m_IconIDToClothingTableID;
};

extern CClothingCache g_ClothingCache;