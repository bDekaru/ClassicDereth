
#pragma once

#include "ObjCache.h"
#include "Frame.h"

class BuildInfo;
class CBldPortal;

class CLandBlockInfo : public DBObj
{
public:
	CLandBlockInfo();
	~CLandBlockInfo();

	static DBObj* Allocator();
	static void Destroyer(DBObj*);
	static CLandBlockInfo* Get(uint32_t ID);
	static void Release(CLandBlockInfo *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);
	uint32_t Unpack_Num_Cells(BYTE *pData);

	uint32_t GetRestrictionIID(uint32_t landcellid);

	uint32_t num_objects; // 0x38
	uint32_t* object_ids; // 0x3C
	Frame* object_frames; // 0x40

	uint32_t num_buildings; // *0x44
	BuildInfo **buildings; // *0x48 

	PackableHashTable<uint32_t, uint32_t> *restriction_table; // *0x4C restrictions
	LPVOID cell_ownership; // 0x50 cell ownership

	uint32_t num_cells; // 0x54 numcells
	uint32_t *cellids; // 0x58 cellids 
	class CEnvCell **cells; // 0x5C cells

#if PHATSDK_USE_EXTENDED_CELL_DATA
	uint32_t num_weenies = 0;
	uint32_t *weenie_wcids = NULL;
	Position *weenie_pos = NULL;
	uint32_t *weenie_iids = 0;

	uint32_t num_weenie_links = 0;
	uint32_t *weenie_link_src = NULL;
	uint32_t *weenie_link_dst = NULL;
#endif
};

class BuildInfo
{
public:
	BuildInfo();
	~BuildInfo();

	uint32_t building_id; // 0x00
	Frame building_frame; // 0x04

	uint32_t num_leaves;
	uint32_t num_portals; // 0x48
	CBldPortal **portals; // 0x4C
};

class CBldPortal
{
public:
	CBldPortal();
	~CBldPortal();

	BOOL UnPack(uint32_t Block, BYTE **ppData, ULONG iSize);
	CEnvCell *GetOtherCell();
	void add_to_stablist(uint32_t * &block_stab_list, uint32_t &max_size, uint32_t &stab_num);

	uint32_t portal_side;
	uint32_t other_cell_id;
	uint32_t other_portal_id;
	uint32_t exact_match;
	uint32_t num_stabs; // 0x10
	uint32_t* stab_list; // 0x14
};







