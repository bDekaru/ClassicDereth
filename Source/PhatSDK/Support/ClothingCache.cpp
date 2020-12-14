
#include <StdAfx.h>
#include "PhatSDK.h"
#include "ClothingCache.h"
#include "ClothingTable.h"

CClothingCache g_ClothingCache;

void CClothingCache::LoadAll()
{
	m_SetupIDToClothingTableID.clear();

	DATDisk::pPortal->FindFileIDsWithinRange(0x10000000, 0x10FFFFFF, ParseClothingFileStatic, NULL, this);
}

void CClothingCache::ReleaseAll()
{
}

void CClothingCache::ParseClothingFileStatic(void *argument, uint32_t id, BTEntry *entry)
{
	if (argument && entry)
	{
		CClothingCache *This = (CClothingCache *)argument;
		This->ParseClothing(id);
	}
}

void CClothingCache::ParseClothing(uint32_t id)
{
	ClothingTable *clothingTable = ClothingTable::Get(id);
	if (clothingTable)
	{
		{
			for (auto &entry : clothingTable->_cloBaseHash)
			{
				m_SetupIDToClothingTableID[entry.first].insert(id);
			}
		}

		{
			for (auto &entry : clothingTable->_paletteTemplatesHash)
			{
				m_IconIDToClothingTableID[entry.second.iconID].insert(id);
			}
		}

		ClothingTable::Release(clothingTable);
	}
	else
	{
		assert(0);
	}
}

uint32_t CClothingCache::GetNumTablesOfSetupID(uint32_t setup_id)
{
	CLOTHINGBYSETUPIDMAP::iterator result = m_SetupIDToClothingTableID.find(setup_id);

	if (result != m_SetupIDToClothingTableID.end())
		return (uint32_t) result->second.size();
	
	return 0;
}

ClothingTable *CClothingCache::GetTableByIndexOfSetupID(uint32_t setup_id, uint32_t index)
{
	if (index >= 1000)
		return NULL; // don't go crazy

	CLOTHINGBYSETUPIDMAP::iterator result = m_SetupIDToClothingTableID.find(setup_id);

	if (result != m_SetupIDToClothingTableID.end())
	{
		std::set<uint32_t> tableIDs = result->second;
		uint32_t i = 0;

		for (const auto &id : tableIDs)
		{
			if (i == index)
			{
				return ClothingTable::Get(id);
			}
			i++;
		}

		return NULL;
	}

	return NULL;
}

uint32_t CClothingCache::GetNumTablesOfIconID(uint32_t icon_id)
{
	CLOTHINGBYICONIDMAP::iterator result = m_IconIDToClothingTableID.find(icon_id);

	if (result != m_IconIDToClothingTableID.end())
		return (uint32_t) result->second.size();

	return 0;
}

ClothingTable *CClothingCache::GetTableByIndexOfIconID(uint32_t icon_id, uint32_t index, uint32_t *palette_key)
{
	if (index >= 1000)
		return NULL; // don't go crazy

	CLOTHINGBYICONIDMAP::iterator result = m_IconIDToClothingTableID.find(icon_id);

	if (result != m_IconIDToClothingTableID.end())
	{
		std::set<uint32_t> tableIDs = result->second;
		uint32_t i = 0;

		for (const auto &id : tableIDs)
		{
			if (i == index)
			{
				ClothingTable *pCT = ClothingTable::Get(id);

				if (pCT)
				{
					for (auto j = pCT->_paletteTemplatesHash.begin(); j != pCT->_paletteTemplatesHash.end(); j++)
					{
						if (j->second.iconID == icon_id)
						{
							if (palette_key)
							{
								*palette_key = j->first;
							}
							return pCT;
						}
					}

					ClothingTable::Release(pCT);
				}
			}
			i++;
		}
	}

	return NULL;
}

const char *CClothingCache::PaletteTemplateIDToString(unsigned int pt)
{
	switch (pt)
	{
		default: return "?";
#ifndef PUBLIC_BUILD
		case 0: return "UNDEF_PALETTE_TEMPLATE";
		case 1: return "AQUABLUE_PALETTE_TEMPLATE";
		case 2: return "BLUE_PALETTE_TEMPLATE";
		case 3: return "BLUEPURPLE_PALETTE_TEMPLATE";
		case 4: return "BROWN_PALETTE_TEMPLATE";
		case 5: return "DARKBLUE_PALETTE_TEMPLATE";
		case 6: return "DEEPBROWN_PALETTE_TEMPLATE";
		case 7: return "DEEPGREEN_PALETTE_TEMPLATE";
		case 8: return "GREEN_PALETTE_TEMPLATE";
		case 9: return "GREY_PALETTE_TEMPLATE";
		case 10: return "LIGHTBLUE_PALETTE_TEMPLATE";
		case 11: return "MAROON_PALETTE_TEMPLATE";
		case 12: return "NAVY_PALETTE_TEMPLATE";
		case 13: return "PURPLE_PALETTE_TEMPLATE";
		case 14: return "RED_PALETTE_TEMPLATE";
		case 15: return "REDPURPLE_PALETTE_TEMPLATE";
		case 16: return "ROSE_PALETTE_TEMPLATE";
		case 17: return "YELLOW_PALETTE_TEMPLATE";
		case 18: return "YELLOWBROWN_PALETTE_TEMPLATE";
		case 19: return "COPPER_PALETTE_TEMPLATE";
		case 20: return "SILVER_PALETTE_TEMPLATE";
		case 21: return "GOLD_PALETTE_TEMPLATE";
		case 22: return "AQUA_PALETTE_TEMPLATE";
		case 23: return "DARKAQUAMETAL_PALETTE_TEMPLATE";
		case 24: return "DARKBLUEMETAL_PALETTE_TEMPLATE";
		case 25: return "DARKCOPPERMETAL_PALETTE_TEMPLATE";
		case 26: return "DARKGOLDMETAL_PALETTE_TEMPLATE";
		case 27: return "DARKGREENMETAL_PALETTE_TEMPLATE";
		case 28: return "DARKPURPLEMETAL_PALETTE_TEMPLATE";
		case 29: return "DARKREDMETAL_PALETTE_TEMPLATE";
		case 30: return "DARKSILVERMETAL_PALETTE_TEMPLATE";
		case 31: return "LIGHTAQUAMETAL_PALETTE_TEMPLATE";
		case 32: return "LIGHTBLUEMETAL_PALETTE_TEMPLATE";
		case 33: return "LIGHTCOPPERMETAL_PALETTE_TEMPLATE";
		case 34: return "LIGHTGOLDMETAL_PALETTE_TEMPLATE";
		case 35: return "LIGHTGREENMETAL_PALETTE_TEMPLATE";
		case 36: return "LIGHTPURPLEMETAL_PALETTE_TEMPLATE";
		case 37: return "LIGHTREDMETAL_PALETTE_TEMPLATE";
		case 38: return "LIGHTSILVERMETAL_PALETTE_TEMPLATE";
		case 39: return "BLACK_PALETTE_TEMPLATE";
		case 40: return "BRONZE_PALETTE_TEMPLATE";
		case 41: return "SANDYYELLOW_PALETTE_TEMPLATE";
		case 42: return "DARKBROWN_PALETTE_TEMPLATE";
		case 43: return "LIGHTBROWN_PALETTE_TEMPLATE";
		case 44: return "TANRED_PALETTE_TEMPLATE";
		case 45: return "PALEGREEN_PALETTE_TEMPLATE";
		case 46: return "TAN_PALETTE_TEMPLATE";
		case 47: return "PASTYYELLOW_PALETTE_TEMPLATE";
		case 48: return "SNOWYWHITE_PALETTE_TEMPLATE";
		case 49: return "RUDDYYELLOW_PALETTE_TEMPLATE";
		case 50: return "RUDDIERYELLOW_PALETTE_TEMPLATE";
		case 51: return "MIDGREY_PALETTE_TEMPLATE";
		case 52: return "DARKGREY_PALETTE_TEMPLATE";
		case 53: return "BLUEDULLSILVER_PALETTE_TEMPLATE";
		case 54: return "YELLOWPALESILVER_PALETTE_TEMPLATE";
		case 55: return "BROWNBLUEDARK_PALETTE_TEMPLATE";
		case 56: return "BROWNBLUEMED_PALETTE_TEMPLATE";
		case 57: return "GREENSILVER_PALETTE_TEMPLATE";
		case 58: return "BROWNGREEN_PALETTE_TEMPLATE";
		case 59: return "YELLOWGREEN_PALETTE_TEMPLATE";
		case 60: return "PALEPURPLE_PALETTE_TEMPLATE";
		case 61: return "WHITE_PALETTE_TEMPLATE";
		case 62: return "REDBROWN_PALETTE_TEMPLATE";
		case 63: return "GREENBROWN_PALETTE_TEMPLATE";
		case 64: return "ORANGEBROWN_PALETTE_TEMPLATE";
		case 65: return "PALEGREENBROWN_PALETTE_TEMPLATE";
		case 66: return "PALEORANGE_PALETTE_TEMPLATE";
		case 67: return "GREENSLIME_PALETTE_TEMPLATE";
		case 68: return "BLUESLIME_PALETTE_TEMPLATE";
		case 69: return "YELLOWSLIME_PALETTE_TEMPLATE";
		case 70: return "PURPLESLIME_PALETTE_TEMPLATE";
		case 71: return "DULLRED_PALETTE_TEMPLATE";
		case 72: return "GREYWHITE_PALETTE_TEMPLATE";
		case 73: return "MEDIUMGREY_PALETTE_TEMPLATE";
		case 74: return "DULLGREEN_PALETTE_TEMPLATE";
		case 75: return "OLIVEGREEN_PALETTE_TEMPLATE";
		case 76: return "ORANGE_PALETTE_TEMPLATE";
		case 77: return "BLUEGREEN_PALETTE_TEMPLATE";
		case 78: return "OLIVE_PALETTE_TEMPLATE";
		case 79: return "LEAD_PALETTE_TEMPLATE";
		case 80: return "IRON_PALETTE_TEMPLATE";
		case 81: return "LITEGREEN_PALETTE_TEMPLATE";
		case 82: return "PINKPURPLE_PALETTE_TEMPLATE";
		case 83: return "AMBER_PALETTE_TEMPLATE";
		case 84: return "DYEDARKGREEN_PALETTE_TEMPLATE";
		case 85: return "DYEDARKRED_PALETTE_TEMPLATE";
		case 86: return "DYEDARKYELLOW_PALETTE_TEMPLATE";
		case 87: return "DYEBOTCHED_PALETTE_TEMPLATE";
		case 88: return "DYEWINTERBLUE_PALETTE_TEMPLATE";
		case 89: return "DYEWINTERGREEN_PALETTE_TEMPLATE";
		case 90: return "DYEWINTERSILVER_PALETTE_TEMPLATE";
		case 91: return "DYESPRINGBLUE_PALETTE_TEMPLATE";
		case 92: return "DYESPRINGPURPLE_PALETTE_TEMPLATE";
		case 93: return "DYESPRINGBLACK_PALETTE_TEMPLATE";
#endif
	}
}

bool CClothingCache::TryToMatchByObjDescWithPaletteTemplateAndGetShades(CloPaletteTemplate *pt, ObjDesc &od, double *shades)
{
	bool bFoundAll = true;

	uint32_t numMatches = 0;

	CloSubpalEffect *se = pt->subpalEffects;
	for (unsigned int i = 0; i < pt->numSubpalEffects; i++)
	{
		bool bSuccess = false;

		PalSet *palSet = PalSet::Get(se[i].palSet);
		if (palSet)
		{
			/*
			if (shade > 0)
			{
				uint32_t palette_id = palSet->GetPaletteID(shade);

				bool bFound = true;
				for (uint32_t k = 0; k < se[i].numRanges; k++)
				{
					if (!od.ContainsSubpalette(palette_id, se[i].rangeStart[k], se[i].rangeLength[k]))
					{
						bFound = false;
						break;
					}
				}

				if (bFound)
					bSuccess = true;
			}
			else
			{
			*/
				// go through all of them

				for (uint32_t j = 0; j < palSet->numPals; j++)
				{
					uint32_t palette_id = palSet->palette_IDs[j];

					bool bFound = true;
					for (uint32_t k = 0; k < se[i].numRanges; k++)
					{
						if (!od.ContainsSubpalette(palette_id, se[i].rangeStart[k], se[i].rangeLength[k]))
						{
							bFound = false;
							break;
						}
					}

					if (bFound)
					{
						if (i < 4)
							shades[i] = ((double)j / (double)palSet->numPals) + 0.00001;

						bSuccess = true;
						numMatches += se[i].numRanges;
						break;
					}
				}
			// }

			PalSet::Release(palSet);
		}

		if (!bSuccess)
		{
			bFoundAll = false;
			break;
		}
	}

	/*
	if (bFoundAll && numMatches < od.num_subpalettes)
	{
		DebugBreak();
		return false;
	}
	*/

	// if (numMatches < od.num_subpalettes)
	//	return false;

	return bFoundAll;
}

bool CClothingCache::TryToMatchBySetupAndObjDesc(uint32_t setup_id, ClothingTable *ct, ObjDesc &od, uint32_t *pt, double *shades)
{
	const ClothingBase *cb = ct->_cloBaseHash.lookup(setup_id);
	if (!cb)
		return false;

	for (unsigned int i = 0; i < cb->numObjectEffects; i++)
	{
		unsigned int partNum = cb->objectEffects[i].partNum;
		if (!od.ContainsAnimPartChange(partNum, cb->objectEffects[i].objectID))
			return false;

		uint32_t numTextureEffects = cb->objectEffects[i].numTextureEffects;
		CloTextureEffect *te = cb->objectEffects[i].textureEffects;

		for (unsigned int j = 0; j < numTextureEffects; j++)
		{
			// check if this is a duplicate texture
			bool bDuplicate = false;
			for (unsigned int k = j + 1; k < numTextureEffects; k++)
			{
				if (te[j].oldTexID == te[k].oldTexID)
				{
					bDuplicate = true;
					break;
				}
			}

			if (bDuplicate)
				continue; // ignore this one

			if (te[j].oldTexID == te[j].newTexID)
				continue; // ignore this one

			if (!od.ContainsTextureMapChange(partNum, te[j].oldTexID, te[j].newTexID))
				return false;
			if (!od.ContainsTextureMapChange(partNum, te[j].newTexID))
				return false;
		}
	}
	
	for (auto &entry : ct->_paletteTemplatesHash)
	{
		if (TryToMatchByObjDescWithPaletteTemplateAndGetShades(&entry.second, od, shades))
		{
			*pt = entry.first;
			return true;
		}
	}

	if (!od.num_subpalettes)
	{
		*pt = UNDEF_PALETTE_TEMPLATE;
		return true;
	}

	return false;
}

bool CClothingCache::TryToMatchBySetupAndObjDesc(uint32_t setup_id, ObjDesc &od, uint32_t *table_id, uint32_t *pt, double *shades)
{
	*table_id = 0;
	*pt = 0;
	shades[0] = 0.0;
	shades[1] = 0.0;
	shades[2] = 0.0;
	shades[3] = 0.0;

	CLOTHINGBYSETUPIDMAP::iterator i = m_SetupIDToClothingTableID.find(setup_id);

	if (i != m_SetupIDToClothingTableID.end())
	{
		for (auto j : i->second)
		{
			// check this clothing table
			ClothingTable *ct = ClothingTable::Get(j);

			if (ct)
			{
				if (TryToMatchBySetupAndObjDesc(setup_id, ct, od, pt, shades))
				{
					*table_id = j;
					ClothingTable::Release(ct);
					return true;
				}

				ClothingTable::Release(ct);
			}
		}
	}

	return false;
}