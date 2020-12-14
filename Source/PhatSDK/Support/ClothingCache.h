
#pragma once

class CClothingCache
{
public:
	void LoadAll();
	void ReleaseAll();

	uint32_t GetNumTablesOfSetupID(uint32_t setup_id);
	class ClothingTable *GetTableByIndexOfSetupID(uint32_t setup_id, uint32_t index);

	uint32_t GetNumTablesOfIconID(uint32_t icon_id);
	class ClothingTable *GetTableByIndexOfIconID(uint32_t icon_id, uint32_t index, uint32_t *palette_key);

	static const char *PaletteTemplateIDToString(unsigned int pt);

	bool TryToMatchBySetupAndObjDesc(uint32_t setup_id, ObjDesc &od, uint32_t *table_id, uint32_t *pt, double *shades);
	bool TryToMatchBySetupAndObjDesc(uint32_t setup_id, ClothingTable *ct, ObjDesc &od, uint32_t *pt, double *shades);
	bool TryToMatchByObjDescWithPaletteTemplateAndGetShades(CloPaletteTemplate *pt, ObjDesc &od, double *shades);

private:
	static void ParseClothingFileStatic(void *argument, uint32_t id, BTEntry *entry);
	void ParseClothing(uint32_t id);

	typedef std::map<uint32_t, std::set<uint32_t>> CLOTHINGBYSETUPIDMAP;
	CLOTHINGBYSETUPIDMAP m_SetupIDToClothingTableID;

	typedef std::map<uint32_t, std::set<uint32_t>> CLOTHINGBYICONIDMAP;
	CLOTHINGBYICONIDMAP m_IconIDToClothingTableID;
};

extern CClothingCache g_ClothingCache;