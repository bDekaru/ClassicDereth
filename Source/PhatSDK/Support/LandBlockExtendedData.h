
#pragma once

#include "SmartArray.h"

class CLandBlockWeenie : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	DWORD wcid = 0;
	Position pos;
	DWORD iid = 0;
};

class CLandBlockWeenieLink : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	DWORD source = 0;
	DWORD target = 0;
};

class CLandBlockExtendedData : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	SmartArray<CLandBlockWeenie> weenies;
	SmartArray<CLandBlockWeenieLink> weenie_links;
};

class CLandBlockExtendedDataTable : public PackObj, public PackableJson
{
public:
	CLandBlockExtendedDataTable();
	virtual ~CLandBlockExtendedDataTable();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy();

	WORD get_terrain(DWORD landcell);
	WORD get_terrain(DWORD landblock, int vertex_x, int vertex_y);

	PackableHashTableWithJson<DWORD, CLandBlockExtendedData> landblocks;
	WORD *terrain_data = NULL;
};
