
#include "StdAfx.h"
#include "ClothingTable.h"
#include "ObjDesc.h"
#include "PalSet.h"

ClothingTable::ClothingTable()
{
}

ClothingTable::~ClothingTable()
{
}

DBObj *ClothingTable::Allocator()
{
	return ((DBObj *)new ClothingTable());
}

DBObjWithJson *ClothingTable::AllocatorWithJson()
{
	return ((DBObjWithJson *)new ClothingTable());
}

void ClothingTable::Destroyer(DBObj *pClothingTable)
{
	delete ((ClothingTable *)pClothingTable);
}

ClothingTable *ClothingTable::Get(DWORD ID)
{
	return (ClothingTable *)ObjCaches::ClothingTables->Get(ID);
}

void ClothingTable::Release(ClothingTable *pClothingTable)
{
	if (pClothingTable)
		ObjCaches::ClothingTables->Release(pClothingTable->GetID());
}

BOOL ClothingTable::BuildObjDesc(DWORD setup, DWORD pt, ShadePackage *shades, ObjDesc *od)
{
	const ClothingBase *pClothingBase = _cloBaseHash.lookup(setup);

	if (!pClothingBase)
	{
		// TODO: MISSING CODE HERE TO HANDLE OTHER RACES

		pClothingBase = _cloBaseHash.lookup(0x02000001); // HUMAN MALE
	}

	if (!pClothingBase)
		return TRUE;

	if (!pClothingBase->ApplyPartAndTextureChanges(od))
		return FALSE;

	const CloPaletteTemplate *pPaletteTemplate = _paletteTemplatesHash.lookup(pt);

	if (!pPaletteTemplate)
		return FALSE;

	for (DWORD i = 0; i < pPaletteTemplate->numSubpalEffects; i++)
	{
		PalSet *pPalSet = PalSet::Get(pPaletteTemplate->subpalEffects[i].palSet);

		if (!pPalSet)
			return FALSE;

		Subpalette subpalette;
		subpalette.subID = pPalSet->GetPaletteID(shades->GetVal(i));

		PalSet::Release(pPalSet);

		for (DWORD j = 0; j < pPaletteTemplate->subpalEffects[i].numRanges; j++)
		{
			subpalette.offset = pPaletteTemplate->subpalEffects[i].rangeStart[j];
			subpalette.numcolors = pPaletteTemplate->subpalEffects[i].rangeLength[j];
			od->AddSubpalette(new Subpalette(subpalette));
		}
	}

	return TRUE;
}

DEFINE_PACK(ClothingTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(ClothingTable)
{
	DWORD file_id = pReader->ReadDWORD(); // file ID

	_cloBaseHash.UnPack(pReader);
	_paletteTemplatesHash.UnPack(pReader);

	/*
	json test;
	PackJson(test);
	std::string testString = test.dump();

	FILE *fp = fopen(csprintf("d:\\temp\\clothing_%08X.json", file_id), "wt");
	if (fp)
	{
		fprintf(fp, "%s\n", testString.c_str());
		fclose(fp);
	}
	*/

	return true;
}

DEFINE_PACK_JSON(ClothingTable)
{
	_cloBaseHash.PackJson(writer["cloBaseHash"]);
	_paletteTemplatesHash.PackJson(writer["paletteTemplatesHash"]);
}

DEFINE_UNPACK_JSON(ClothingTable)
{
	_cloBaseHash.UnPackJson(reader["cloBaseHash"]);
	_paletteTemplatesHash.UnPackJson(reader["paletteTemplatesHash"]);
	return true;
}

DEFINE_LEGACY_PACK_MIGRATOR(ClothingTable)

ClothingBase::ClothingBase()
{
	objectEffects = NULL;
}

ClothingBase::~ClothingBase()
{
	if (objectEffects)
		delete[] objectEffects;
}

BOOL ClothingBase::ApplyPartAndTextureChanges(ObjDesc *od) const
{
	AnimPartChange apc;
	TextureMapChange tmc;
	BOOL success = TRUE;

	for (DWORD i = 0; i < numObjectEffects; i++)
	{
		apc.part_index = objectEffects[i].partNum;
		apc.part_id = objectEffects[i].objectID;
		od->AddAnimPartChange(new AnimPartChange(apc));

		for (DWORD j = 0; j < objectEffects[i].numTextureEffects; j++)
		{
			tmc.part_index = objectEffects[i].partNum;
			tmc.old_tex_id = objectEffects[i].textureEffects[j].oldTexID;
			tmc.new_tex_id = objectEffects[i].textureEffects[j].newTexID;
			if (tmc.old_tex_id == 0 || tmc.new_tex_id == 0)
				success = FALSE;

			od->AddTextureMapChange(new TextureMapChange(tmc));
		}
	}

	return success;
}


DEFINE_PACK(ClothingBase)
{
	UNFINISHED();
}

DEFINE_UNPACK(ClothingBase)
{
	numObjectEffects = pReader->Read<DWORD>();

	objectEffects = new CloObjectEffect[numObjectEffects];
	for (DWORD i = 0; i < numObjectEffects; i++)
		objectEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(ClothingBase)
{
	json& entryList = writer["objectEffects"];

	for (DWORD i = 0; i < numObjectEffects; i++)
	{
		json entry;
		objectEffects[i].PackJson(entry);
		entryList.push_back(entry);
	}
}

DEFINE_UNPACK_JSON(ClothingBase)
{
	const json& entryList = reader["objectEffects"];

	numObjectEffects = (DWORD) entryList.size();
	objectEffects = new CloObjectEffect[numObjectEffects];

	DWORD index = 0;
	for (const json &entry : entryList)
	{
		objectEffects[index++].UnPackJson(entry);
	}

	return true;
}

CloObjectEffect::CloObjectEffect()
{
	textureEffects = NULL;
}

CloObjectEffect::~CloObjectEffect()
{
	if (textureEffects)
		delete[] textureEffects;
}

DEFINE_PACK(CloObjectEffect)
{
	UNFINISHED();
}

DEFINE_UNPACK(CloObjectEffect)
{
	partNum = pReader->Read<DWORD>();
	objectID = pReader->Read<DWORD>();
	numTextureEffects = pReader->Read<DWORD>();

	textureEffects = new CloTextureEffect[numTextureEffects];
	for (DWORD i = 0; i < numTextureEffects; i++)
		textureEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(CloObjectEffect)
{
	writer["partNum"] = partNum;
	writer["objectID"] = objectID;
	
	json& entryList = writer["textureEffects"];

	for (DWORD i = 0; i < numTextureEffects; i++)
	{
		json entry;
		textureEffects[i].PackJson(entry);
		entryList.push_back(entry);
	}
}

DEFINE_UNPACK_JSON(CloObjectEffect)
{
	partNum = reader["partNum"];
	objectID = reader["objectID"];

	const json& entryList = reader["textureEffects"];

	numTextureEffects = (DWORD)entryList.size();
	textureEffects = new CloTextureEffect[numTextureEffects];

	DWORD index = 0;
	for (const json &entry : entryList)
	{
		textureEffects[index++].UnPackJson(entry);
	}

	return true;
}

CloTextureEffect::CloTextureEffect()
{
}

CloTextureEffect::~CloTextureEffect()
{
}

DEFINE_PACK(CloTextureEffect)
{
	UNFINISHED();
}

DEFINE_UNPACK(CloTextureEffect)
{
	oldTexID = pReader->Read<DWORD>();
	newTexID = pReader->Read<DWORD>();

	return true;
}

DEFINE_PACK_JSON(CloTextureEffect)
{
	writer["oldTexID"] = oldTexID;
	writer["newTexID"] = newTexID;
}

DEFINE_UNPACK_JSON(CloTextureEffect)
{
	oldTexID = reader["oldTexID"];
	newTexID = reader["newTexID"];
	return true;
}

CloPaletteTemplate::CloPaletteTemplate()
{
	subpalEffects = NULL;
}

CloPaletteTemplate::~CloPaletteTemplate()
{
	if (subpalEffects)
		delete[] subpalEffects;
}

DEFINE_PACK(CloPaletteTemplate)
{
	UNFINISHED();
}

DEFINE_UNPACK(CloPaletteTemplate)
{
	iconID = pReader->Read<DWORD>();
	numSubpalEffects = pReader->Read<DWORD>();

	subpalEffects = new CloSubpalEffect[numSubpalEffects];
	for (DWORD i = 0; i < numSubpalEffects; i++)
		subpalEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(CloPaletteTemplate)
{
	writer["iconID"] = iconID;
	
	json& entryList = writer["subpalEffects"];

	for (DWORD i = 0; i < numSubpalEffects; i++)
	{
		json entry;
		subpalEffects[i].PackJson(entry);
		entryList.push_back(entry);
	}
}

DEFINE_UNPACK_JSON(CloPaletteTemplate)
{
	iconID = reader["iconID"];

	const json& entryList = reader["subpalEffects"];

	numSubpalEffects = (DWORD)entryList.size();
	subpalEffects = new CloSubpalEffect[numSubpalEffects];

	DWORD index = 0;
	for (const json &entry : entryList)
	{
		subpalEffects[index++].UnPackJson(entry);
	}

	return true;
}


CloSubpalEffect::CloSubpalEffect()
{
	rangeStart = NULL;
	rangeLength = NULL;
}

CloSubpalEffect::~CloSubpalEffect()
{
	if (rangeStart)
		delete[] rangeStart;
	if (rangeLength)
		delete[] rangeLength;
}

DEFINE_PACK(CloSubpalEffect)
{
	UNFINISHED();
}

DEFINE_UNPACK(CloSubpalEffect)
{
	numRanges = pReader->Read<DWORD>();
	rangeStart = new unsigned int[numRanges];
	rangeLength = new unsigned int[numRanges];

	for (DWORD i = 0; i < numRanges; i++)
	{
		rangeStart[i] = pReader->Read<DWORD>();
		rangeLength[i] = pReader->Read<DWORD>();
	}

	palSet = pReader->Read<DWORD>();

	return true;
}

DEFINE_PACK_JSON(CloSubpalEffect)
{
	json &rangesList = writer["ranges"];

	for (DWORD i = 0; i < numRanges; i++)
	{
		json rangeEntry;
		rangeEntry["start"] = rangeStart[i];
		rangeEntry["length"] = rangeLength[i];
		rangesList.push_back(rangesList);
	}

	writer["palSet"] = palSet;
}

DEFINE_UNPACK_JSON(CloSubpalEffect)
{
	const json &rangesList = reader["ranges"];

	numRanges = (DWORD) rangesList.size();

	rangeStart = new unsigned int[numRanges];
	rangeLength = new unsigned int[numRanges];

	DWORD index = 0;
	for (const json &rangeEntry : rangesList)
	{
		rangeStart[index] = rangeEntry["start"];
		rangeLength[index] = rangeEntry["length"];
		index++;
	}

	palSet = reader["palSet"];

	return true;
}