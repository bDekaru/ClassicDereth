
#include "StdAfx.h"
#include "PhatSDK.h"
#include "InferredCellData.h"

void CInferredCellData::Init()
{
#ifndef PUBLIC_BUILD
	LOG(Data, Normal, "Loading inferred cell data...\n");
#endif

	_data.Destroy();

	BYTE *data = NULL;
	DWORD length = 0;
	if (LoadDataFromPhatDataBin(6, &data, &length, 0xcd57fd07, 0x697a2224)) // lbed.bin
	{
		BinaryReader reader(data, length);
		_data.UnPack(&reader);
		delete[] data;

		/*
		json test;
		_data.PackJson(test);
		std::string testString = test.dump();

		FILE *fp = fopen("data\\json\\worldspawns_aerfalle.json", "wt");
		if (fp)
		{
			fprintf(fp, "%s\n", testString.c_str());
			fclose(fp);
		}
		*/
	}

	std::ifstream fileStream("data\\json\\worldspawns.json");

	if (fileStream.is_open())
	{
		json jsonData;
		fileStream >> jsonData;
		fileStream.close();

		_jsonData.UnPackJson(jsonData);
	}

#ifndef PUBLIC_BUILD
	LOG(Data, Normal, "Finished loading inferred cell data (%u and %u entries)...\n", (DWORD) _data.landblocks.size(), (DWORD) _jsonData.landblocks.size());
#endif
}

CLandBlockExtendedData *CInferredCellData::GetLandBlockData(DWORD landblock)
{
	CLandBlockExtendedData *data = _jsonData.landblocks.lookup(landblock);

	if (!data)
	{
		data = _data.landblocks.lookup(landblock);
	}

	return data;
}
