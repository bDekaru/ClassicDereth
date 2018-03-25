
#include "stdafx.h"
#include "PhatSDK.h"

DEFINE_PACK(CLandBlockWeenie)
{
	pWriter->Write<DWORD>(wcid);
	pos.Pack(pWriter);
	pWriter->Write<DWORD>(iid);
}

DEFINE_PACK_JSON(CLandBlockWeenie)
{
	writer["wcid"] = wcid;
	pos.PackJson(writer["pos"]);
	writer["id"] = iid & 0xFFF;
}

DEFINE_UNPACK(CLandBlockWeenie)
{
	wcid = pReader->Read<DWORD>();
	pos.UnPack(pReader);
	iid = pReader->Read<DWORD>();
	return true;
}

DEFINE_UNPACK_JSON(CLandBlockWeenie)
{
	wcid = reader["wcid"];
	pos.UnPackJson(reader["pos"]);
	iid = reader["id"];
	return true;
}

DEFINE_PACK(CLandBlockWeenieLink)
{
	pWriter->Write<DWORD>(source);
	pWriter->Write<DWORD>(target);
}

DEFINE_PACK_JSON(CLandBlockWeenieLink)
{
	writer["source"] = source & 0xFFF;
	writer["target"] = target & 0xFFF;
}

DEFINE_UNPACK(CLandBlockWeenieLink)
{
	source = pReader->Read<DWORD>();
	target = pReader->Read<DWORD>();
	return true;
}

DEFINE_UNPACK_JSON(CLandBlockWeenieLink)
{
	source = reader["source"];
	target = reader["target"];
	return true;
}

DEFINE_PACK(CLandBlockExtendedData)
{
	weenies.PackPackObj(pWriter);
	weenie_links.PackPackObj(pWriter);
}

DEFINE_PACK_JSON(CLandBlockExtendedData)
{
	json &weeniesList = writer["weenies"];
	for (DWORD i = 0; i < weenies.num_used; i++)
	{
		json entry;
		weenies.array_data[i].PackJson(entry);
		weeniesList.push_back(entry);
	}

	json &linksList = writer["links"];
	for (DWORD i = 0; i < weenie_links.num_used; i++)
	{
		json entry;
		weenie_links.array_data[i].PackJson(entry);
		linksList.push_back(entry);
	}
}

DEFINE_UNPACK(CLandBlockExtendedData)
{
	weenies.UnPackPackObj(pReader);
	weenie_links.UnPackPackObj(pReader);
	return true;
}

DEFINE_UNPACK_JSON(CLandBlockExtendedData)
{
	if (reader.find("weenies") != reader.end())
	{
		for (const json &entry : reader["weenies"])
		{
			CLandBlockWeenie weenie;
			weenie.UnPackJson(entry);
			weenies.add(&weenie);
		}
	}

	if (reader.find("links") != reader.end())
	{
		for (const json &entry : reader["links"])
		{
			CLandBlockWeenieLink weenie_link;
			weenie_link.UnPackJson(entry);
			weenie_links.add(&weenie_link);
		}
	}

	return true;
}

CLandBlockExtendedDataTable::CLandBlockExtendedDataTable()
{
	terrain_data = new WORD[(255 * 9) * (255 * 9)]; // 10 MB of memory
}

CLandBlockExtendedDataTable::~CLandBlockExtendedDataTable()
{
	SafeDeleteArray(terrain_data);
}

void CLandBlockExtendedDataTable::Destroy()
{
	landblocks.clear();
}

WORD CLandBlockExtendedDataTable::get_terrain(DWORD landcell)
{
	if (!terrain_data)
		return 0;

	long x, y;
	if (!LandDefs::gid_to_lcoord(landcell, x, y))
		return 0;

	return terrain_data[(x * 255 * 9) + y];
}

WORD CLandBlockExtendedDataTable::get_terrain(DWORD landblock, int vertex_x, int vertex_y)
{
	if (!terrain_data)
		return 0;
	
	DWORD block_x = (landblock >> 24) & 0xFF;
	DWORD block_y = (landblock >> 16) & 0xFF;

	WORD *terrain_base = &terrain_data[((block_x * 255) + block_y) * 9 * 9];

	return terrain_base[(vertex_x * 9) + vertex_y];
}

DEFINE_PACK(CLandBlockExtendedDataTable)
{
	landblocks.Pack(pWriter);
	pWriter->Write(terrain_data, (255 * 9) * (255 * 9) * sizeof(WORD));
}

DEFINE_PACK_JSON(CLandBlockExtendedDataTable)
{
	/*
	for (PackableHashTableWithJson<DWORD, CLandBlockExtendedData>::iterator i = landblocks.begin(); i != landblocks.end();)
	{
		if ((i->first & 0xFFFF0000) != 0x1f50000)
		{
			i = landblocks.erase(i);
			continue;
		}

		i++;
	}
	*/

	landblocks.PackJson(writer["landblocks"]);

	/*
	json terrain_data_entry;

	for (DWORD x = 0; x < 255; x++)
	{
		for (DWORD y = 0; y < 255; y++)
		{
			json landblock_entry;
			landblock_entry["block_id"] = (x << 8) | y;

			for (DWORD vertex_x = 0; vertex_x < 9; vertex_x++)
			{
				for (DWORD vertex_y = 0; vertex_y < 9; vertex_y++)
				{
					json vertex_entry;
					vertex_entry["x"] = vertex_x;
					vertex_entry["y"] = vertex_y;

					WORD *terrain_base = &terrain_data[((x * 255) + y) * 9 * 9];
					vertex_entry["encounter"] = (terrain_base[(vertex_x * 9) + vertex_y] >> 7) & 0xF;

					landblock_entry["vertices"].push_back(vertex_entry);
				}
			}

			terrain_data_entry.push_back(landblock_entry);
		}
	}

	writer["encounter_blocks"] = terrain_data_entry;
	*/
}

DEFINE_UNPACK(CLandBlockExtendedDataTable)
{
	Destroy();

	landblocks.UnPack(pReader);

	memcpy(terrain_data, pReader->ReadArray((255 * 9) * (255 * 9) * sizeof(WORD)), (255 * 9) * (255 * 9) * sizeof(WORD));

#ifdef _DEBUG
	if (pReader->GetLastError())
	{
		DebugBreak();
	}
#endif
	
	/*
	for (auto &entry : landblocks)
	{
		DWORD block_id = entry.first;

		for (DWORD i = 0; i < entry.second.weenies.num_used; i++)
		{
			if (((entry.second.weenies.array_data[i].pos.objcell_id >> 16) & 0xFFFF) != (block_id >> 16))
			{
				if (entry.second.weenies.array_data[i].pos.objcell_id)
				{
					DebugBreak();
				}
			}
		}

		for (DWORD i = 0; i < entry.second.weenie_links.num_used; i++)
		{
			if (((entry.second.weenie_links.array_data[i].source >> 12) & 0xFFFF) != (block_id >> 16))
			{
				DebugBreak();
			}

			if (((entry.second.weenie_links.array_data[i].target >> 12) & 0xFFFF) != (block_id >> 16))
			{
				DebugBreak();
			}
		}
	}
	*/

	return true;
}

DEFINE_UNPACK_JSON(CLandBlockExtendedDataTable)
{
	landblocks.UnPackJson(reader["landblocks"]);

	memset(terrain_data, 0, (255 * 9) * (255 * 9) * sizeof(WORD));

	for (auto &entry : landblocks)
	{
		DWORD block_id = entry.first;

		for (DWORD i = 0; i < entry.second.weenies.num_used; i++)
		{
			entry.second.weenies.array_data[i].iid |= 0x70000000 | (block_id >> 4);
		}

		for (DWORD i = 0; i < entry.second.weenie_links.num_used; i++)
		{
			entry.second.weenie_links.array_data[i].source |= 0x70000000 | (block_id >> 4);
			entry.second.weenie_links.array_data[i].target |= 0x70000000 | (block_id >> 4);
		}
	}

	/*

	for (const json &entry : reader["encounter_blocks"])
	{
		if (!entry.count("block_id"))
			continue;

		DWORD block_id = entry["block_id"];

		DWORD x = (block_id & 0xFF00) >> 8;
		DWORD y = (block_id & 0xFF);

		if (x >= 255)
			continue;
		if (y >= 255)
			continue;

		for (const json &vertex_entry : entry["vertices"])
		{
			DWORD vertex_x = vertex_entry["x"];
			DWORD vertex_y = vertex_entry["y"];
			DWORD encounter = vertex_entry["encounter"];

			if (vertex_x >= 9)
				continue;
			if (vertex_y >= 9)
				continue;
			if (encounter >= 16)
				continue;

			WORD *terrain_base = &terrain_data[((x * 255) + y) * 9 * 9];
			terrain_base[(vertex_x * 9) + vertex_y] = ((encounter & 0xF) << 7);
		}
	}
	*/
	
	return true;
}