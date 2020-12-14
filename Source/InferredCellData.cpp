
#include <StdAfx.h>
#include "PhatSDK.h"
#include "InferredCellData.h"

void CInferredCellData::Init()
{
#ifndef PUBLIC_BUILD
	SERVER_INFO << "Loading inferred cell data...";
#endif

	_data.Destroy();

	LoadCacheData(6, 0xcd57fd07, 0x697a2224, _data);

	fs::path dataPath(g_pGlobals->GetGameData("Data", "json"));

	block_flags_map_t &flags = _blockFlags;
	LoadJsonData(dataPath / "restrictedlandblocks.json", [&flags](json &data)
	{
		flags.UnPackJson(data.at("blockFlags"));
	});

	LoadJsonData(dataPath / "worldspawns.json", _jsonData);

	LoadJsonData(dataPath / "encounters.json", _encounters);

	// load individual spawn maps to override worldspawns.json
	RefreshLocalSpawnMapStorage();

#ifndef PUBLIC_BUILD
	SERVER_INFO << "Finished loading inferred cell data ("
		<< (uint32_t)_data.landblocks.size() << "and" << (uint32_t) _jsonData.landblocks.size()
		<< ") encounters" << _encounters.size() << "...";
#endif
}

bool CInferredCellData::RefreshLocalSpawnMapStorage()
{
	return LoadLocalSpawnMapStorage(true);
}

bool CInferredCellData::RefreshLocalSpawnMap(uint32_t landblock)
{
	return LoadLocalSpawnMap(landblock);
}

bool CInferredCellData::LoadLocalSpawnMapStorage(bool refresh)
{
	fs::path dataPath = g_pGlobals->GetGameData("Data", "json");

	// load individual spawn maps to override worldspawns.json
	std::mutex mapLock;
	PerformLoad(dataPath / "spawnMaps", [&](fs::path path)
	{
		std::ifstream fs(path);

		json jsonData;
		bool parsed = false;
		CLandBlockExtendedData blockData;
		uint32_t blockId = 0;
		try
		{
			fs >> jsonData;

			json::const_iterator end = jsonData.end();
			json::const_iterator key = jsonData.find("key");
			json::const_iterator val = jsonData.find("value");

			if (key != end)
			{
				blockId = *key;

				if (val != end)
					parsed = blockData.UnPackJson(*val);
			}
		}
		catch (std::exception &ex)
		{
			LOG_PRIVATE(Data, Error, "Failed to parse spawn map file %s\n", path.string().c_str());
		}

		fs.close();

		if (parsed)
		{
			std::scoped_lock lock(mapLock);
			_jsonData.landblocks.insert_or_assign(blockId, blockData);
			_jsonData.landblocks[blockId].m_sourceFile = path.string();
		}

	});

	return true;
}

bool CInferredCellData::LoadLocalSpawnMap(uint32_t landblock)
{
	uint32_t landcell = landblock << 16;

	auto existing = _jsonData.landblocks.find(landcell);
	if (existing != _jsonData.landblocks.end())
	{
		fs::path path = _jsonData.landblocks[landcell].m_sourceFile.c_str();

		std::mutex mapLock;

		std::ifstream fs(path);

		json jsonData;
		bool parsed = false;
		CLandBlockExtendedData blockData;
		uint32_t blockId = 0;
		try
		{
			fs >> jsonData;

			json::const_iterator end = jsonData.end();
			json::const_iterator key = jsonData.find("key");
			json::const_iterator val = jsonData.find("value");

			if (key != end)
			{
				blockId = *key;

				if (val != end)
					parsed = blockData.UnPackJson(*val);
			}
		}
		catch (std::exception &ex)
		{
			LOG_PRIVATE(Data, Error, "Failed to parse spawn map file %s\n", path.string().c_str());
			return false;
		}

		fs.close();

		if (parsed)
		{
			std::scoped_lock lock(mapLock);
			_jsonData.landblocks.insert_or_assign(blockId, blockData);
			_jsonData.landblocks[blockId].m_sourceFile = path.string();
			return true;
		}
	}

	return false;
}

bool CInferredCellData::FindSpawnMap(uint32_t landblock)
{
	return _jsonData.landblocks[landblock].m_sourceFile.size() > 0;
}

CLandBlockExtendedData *CInferredCellData::GetLandBlockData(uint32_t landblock)
{
	CLandBlockExtendedData *data = _jsonData.landblocks.lookup(landblock);

	if (!data)
	{
		data = _data.landblocks.lookup(landblock);
	}

	return data;
}


