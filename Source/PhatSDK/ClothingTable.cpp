
#include <StdAfx.h>
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

ClothingTable *ClothingTable::Get(uint32_t ID)
{
	return (ClothingTable *)ObjCaches::ClothingTables->Get(ID);
}

void ClothingTable::Release(ClothingTable *pClothingTable)
{
	if (pClothingTable)
		ObjCaches::ClothingTables->Release(pClothingTable->GetID());
}

const uint32_t UNDEAD_MALE_UNDEAD_SETUP_0 = 33561102u;
const uint32_t UNDEAD_MALE_UNDEAD_GEN_SETUP_0 = 33561103u;
const uint32_t UNDEAD_MALE_SKELETON_SETUP_0 = 33561244u;
const uint32_t UNDEAD_MALE_SKELETON_NOFLAME_SETUP_0 = 33561246u;
const uint32_t UNDEAD_MALE_ZOMBIE_SETUP_0 = 33561245u;
const uint32_t UNDEAD_MALE_ZOMBIE_NOFLAME_SETUP_0 = 33561238u;
const uint32_t UNDEAD_FEMALE_UNDEAD_SETUP_0 = 33561100u;
const uint32_t UNDEAD_FEMALE_UNDEAD_GEN_SETUP_0 = 33561101u;
const uint32_t UNDEAD_FEMALE_SKELETON_SETUP_0 = 33561248u;
const uint32_t UNDEAD_FEMALE_SKELETON_NOFLAME_SETUP_0 = 33561247u;
const uint32_t UNDEAD_FEMALE_ZOMBIE_SETUP_0 = 33561249u;
const uint32_t UNDEAD_FEMALE_ZOMBIE_NOFLAME_SETUP_0 = 33561250u;
const uint32_t UMBRAEN_MALE_CROWN_SETUP_0 = 33560943u;
const uint32_t UMBRAEN_MALE_CROWN_GEN_SETUP_0 = 33560946u;
const uint32_t UMBRAEN_MALE_NOCROWN_SETUP_0 = 33561183u;
const uint32_t UMBRAEN_MALE_VOID_SETUP_0 = 33561199u;
const uint32_t UMBRAEN_FEMALE_CROWN_SETUP_0 = 33560944u;
const uint32_t UMBRAEN_FEMALE_NOCROWN_SETUP_0 = 33561182u;
const uint32_t UMBRAEN_FEMALE_VOID_SETUP_0 = 33561198u;
const uint32_t PENUMBRAEN_MALE_CROWN_SETUP_0 = 33560942u;
const uint32_t PENUMBRAEN_MALE_CROWN_GEN_SETUP_0 = 33560945u;
const uint32_t PENUMBRAEN_MALE_NOCROWN_SETUP_0 = 33561181u;
const uint32_t PENUMBRAEN_MALE_VOID_SETUP_0 = 33561200u;
const uint32_t PENUMBRAEN_FEMALE_CROWN_SETUP_0 = 33560941u;
const uint32_t PENUMBRAEN_FEMALE_NOCROWN_SETUP_0 = 33561180u;
const uint32_t PENUMBRAEN_FEMALE_VOID_SETUP_0 = 33561201u;
const uint32_t HUMAN_MALE_CLOTHING_DEFAULT_0 = 33554433u;
const uint32_t HUMAN_FEMALE_CLOTHING_DEFAULT_0 = 33554510u;
const uint32_t ANAKSHAY_MALE_SETUP_0 = 33561251u;
const uint32_t ANAKSHAY_FEMALE_SETUP_0 = 33561252u;

uint32_t ClothingTable::GetAltSetup(uint32_t setup)
{
	switch (setup)
	{
	case UMBRAEN_MALE_CROWN_SETUP_0:
	case UMBRAEN_MALE_CROWN_GEN_SETUP_0:
	case UMBRAEN_MALE_NOCROWN_SETUP_0:
	case UMBRAEN_MALE_VOID_SETUP_0:
		return UMBRAEN_MALE_CROWN_SETUP_0;

	case UMBRAEN_FEMALE_CROWN_SETUP_0:
	case UMBRAEN_FEMALE_NOCROWN_SETUP_0:
	case UMBRAEN_FEMALE_VOID_SETUP_0:
			return UMBRAEN_FEMALE_CROWN_SETUP_0;

	case PENUMBRAEN_MALE_CROWN_SETUP_0:
	case PENUMBRAEN_MALE_CROWN_GEN_SETUP_0:
	case PENUMBRAEN_MALE_NOCROWN_SETUP_0:
	case PENUMBRAEN_MALE_VOID_SETUP_0:
		return PENUMBRAEN_MALE_CROWN_SETUP_0;

	case PENUMBRAEN_FEMALE_CROWN_SETUP_0:
	case PENUMBRAEN_FEMALE_NOCROWN_SETUP_0:
	case PENUMBRAEN_FEMALE_VOID_SETUP_0:
		return PENUMBRAEN_FEMALE_CROWN_SETUP_0;

	case UNDEAD_MALE_UNDEAD_GEN_SETUP_0:
	case UNDEAD_MALE_SKELETON_SETUP_0:
	case UNDEAD_MALE_SKELETON_NOFLAME_SETUP_0:
	case UNDEAD_MALE_ZOMBIE_SETUP_0:
	case UNDEAD_MALE_ZOMBIE_NOFLAME_SETUP_0:
		return UNDEAD_MALE_UNDEAD_SETUP_0;

	case UNDEAD_FEMALE_UNDEAD_GEN_SETUP_0:
	case UNDEAD_FEMALE_SKELETON_SETUP_0:
	case UNDEAD_FEMALE_SKELETON_NOFLAME_SETUP_0:
	case UNDEAD_FEMALE_ZOMBIE_SETUP_0:
	case UNDEAD_FEMALE_ZOMBIE_NOFLAME_SETUP_0:
		return UNDEAD_FEMALE_UNDEAD_SETUP_0;

	case ANAKSHAY_MALE_SETUP_0:
		return HUMAN_MALE_CLOTHING_DEFAULT_0;

	case ANAKSHAY_FEMALE_SETUP_0:
		return HUMAN_FEMALE_CLOTHING_DEFAULT_0;
	}
	return setup;
}

BOOL ClothingTable::BuildObjDesc(uint32_t setup, uint32_t pt, ShadePackage *shades, ObjDesc *od, uint32_t *iconId)
{
	setup = GetAltSetup(setup);

	const ClothingBase *pClothingBase = _cloBaseHash.lookup(setup);
	const CloPaletteTemplate *pPaletteTemplate = _paletteTemplatesHash.lookup(pt);

	// TODO: MISSING CODE HERE TO HANDLE OTHER RACES ??

	if (!pClothingBase || !pClothingBase->ApplyPartAndTextureChanges(od))
		return FALSE;

	if (!pPaletteTemplate)
		return FALSE;

	if (pPaletteTemplate->iconID && iconId)
		*iconId = pPaletteTemplate->iconID;

	for (uint32_t i = 0; i < pPaletteTemplate->numSubpalEffects; i++)
	{
		PalSet *pPalSet = PalSet::Get(pPaletteTemplate->subpalEffects[i].palSet);

		if (!pPalSet)
			return FALSE;

		Subpalette subpalette;
		subpalette.subID = pPalSet->GetPaletteID(shades->GetVal(i));

		PalSet::Release(pPalSet);

		for (uint32_t j = 0; j < pPaletteTemplate->subpalEffects[i].numRanges; j++)
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
	uint32_t file_id = pReader->ReadUInt32(); // file ID

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

	for (uint32_t i = 0; i < numObjectEffects; i++)
	{
		apc.part_index = objectEffects[i].partNum;
		apc.part_id = objectEffects[i].objectID;
		od->AddAnimPartChange(new AnimPartChange(apc));

		for (uint32_t j = 0; j < objectEffects[i].numTextureEffects; j++)
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
	numObjectEffects = pReader->Read<uint32_t>();

	objectEffects = new CloObjectEffect[numObjectEffects];
	for (uint32_t i = 0; i < numObjectEffects; i++)
		objectEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(ClothingBase)
{
	json& entryList = writer["objectEffects"];

	for (uint32_t i = 0; i < numObjectEffects; i++)
	{
		json entry;
		objectEffects[i].PackJson(entry);
		entryList.push_back(entry);
	}
}

DEFINE_UNPACK_JSON(ClothingBase)
{
	const json& entryList = reader["objectEffects"];

	numObjectEffects = (uint32_t) entryList.size();
	objectEffects = new CloObjectEffect[numObjectEffects];

	uint32_t index = 0;
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
	partNum = pReader->Read<uint32_t>();
	objectID = pReader->Read<uint32_t>();
	numTextureEffects = pReader->Read<uint32_t>();

	textureEffects = new CloTextureEffect[numTextureEffects];
	for (uint32_t i = 0; i < numTextureEffects; i++)
		textureEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(CloObjectEffect)
{
	writer["partNum"] = partNum;
	writer["objectID"] = objectID;
	
	json& entryList = writer["textureEffects"];

	for (uint32_t i = 0; i < numTextureEffects; i++)
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

	numTextureEffects = (uint32_t)entryList.size();
	textureEffects = new CloTextureEffect[numTextureEffects];

	uint32_t index = 0;
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
	oldTexID = pReader->Read<uint32_t>();
	newTexID = pReader->Read<uint32_t>();

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
	iconID = pReader->Read<uint32_t>();
	numSubpalEffects = pReader->Read<uint32_t>();

	subpalEffects = new CloSubpalEffect[numSubpalEffects];
	for (uint32_t i = 0; i < numSubpalEffects; i++)
		subpalEffects[i].UnPack(pReader);

	return true;
}

DEFINE_PACK_JSON(CloPaletteTemplate)
{
	writer["iconID"] = iconID;
	
	json& entryList = writer["subpalEffects"];

	for (uint32_t i = 0; i < numSubpalEffects; i++)
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

	numSubpalEffects = (uint32_t)entryList.size();
	subpalEffects = new CloSubpalEffect[numSubpalEffects];

	uint32_t index = 0;
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
	numRanges = pReader->Read<uint32_t>();
	if (pReader->GetLastError())
		return false;
	rangeStart = new unsigned int[numRanges];
	rangeLength = new unsigned int[numRanges];

	uint32_t requiredReaderLength = (numRanges * 8) + 4;

	if (pReader->GetDataRemaining() < requiredReaderLength)
		return false;

	for (uint32_t i = 0; i < numRanges; i++)
	{
		rangeStart[i] = pReader->Read<uint32_t>();
		rangeLength[i] = pReader->Read<uint32_t>();
	}

	palSet = pReader->Read<uint32_t>();

	return true;
}

DEFINE_PACK_JSON(CloSubpalEffect)
{
	json &rangesList = writer["ranges"];

	for (uint32_t i = 0; i < numRanges; i++)
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

	numRanges = (uint32_t) rangesList.size();

	rangeStart = new unsigned int[numRanges];
	rangeLength = new unsigned int[numRanges];

	uint32_t index = 0;
	for (const json &rangeEntry : rangesList)
	{
		rangeStart[index] = rangeEntry["start"];
		rangeLength[index] = rangeEntry["length"];
		index++;
	}

	palSet = reader["palSet"];

	return true;
}
