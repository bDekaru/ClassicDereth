
#pragma once

#include "SmartArray.h"

class CLandBlockWeenie : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	uint32_t wcid = 0;
	Position pos;
	uint32_t iid = 0;
};

class CLandBlockWeenieLink : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	uint32_t source = 0;
	uint32_t target = 0;
};

class CLandBlockExtendedData : public PackObj, public PackableJson
{
public:
	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	SmartArray<CLandBlockWeenie> weenies;
	SmartArray<CLandBlockWeenieLink> weenie_links;
	std::string m_sourceFile;
};

class CLandBlockExtendedDataTable : public PackObj, public PackableJson
{
public:
	CLandBlockExtendedDataTable();
	virtual ~CLandBlockExtendedDataTable();

	DECLARE_PACKABLE()
	DECLARE_PACKABLE_JSON()

	void Destroy();

	//WORD get_terrain(uint32_t landcell);
	WORD get_terrain(uint32_t landblock, int vertex_x, int vertex_y);

	PackableHashTableWithJson<uint32_t, CLandBlockExtendedData> landblocks;
	WORD *terrain_data = NULL;
};
