
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
	static CLandBlockInfo* Get(DWORD ID);
	static void Release(CLandBlockInfo *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);
	DWORD Unpack_Num_Cells(BYTE *pData);

	DWORD GetRestrictionIID(DWORD landcellid);

	DWORD num_objects; // 0x38
	DWORD* object_ids; // 0x3C
	Frame* object_frames; // 0x40

	DWORD num_buildings; // *0x44
	BuildInfo **buildings; // *0x48 

	PackableHashTable<DWORD, DWORD> *restriction_table; // *0x4C restrictions
	LPVOID cell_ownership; // 0x50 cell ownership

	DWORD num_cells; // 0x54 numcells
	DWORD *cellids; // 0x58 cellids 
	class CEnvCell **cells; // 0x5C cells

#if PHATSDK_USE_EXTENDED_CELL_DATA
	DWORD num_weenies = 0;
	DWORD *weenie_wcids = NULL;
	Position *weenie_pos = NULL;
	DWORD *weenie_iids = 0;

	DWORD num_weenie_links = 0;
	DWORD *weenie_link_src = NULL;
	DWORD *weenie_link_dst = NULL;
#endif
};

class BuildInfo
{
public:
	BuildInfo();
	~BuildInfo();

	DWORD building_id; // 0x00
	Frame building_frame; // 0x04

	DWORD num_leaves;
	DWORD num_portals; // 0x48
	CBldPortal **portals; // 0x4C
};

class CBldPortal
{
public:
	CBldPortal();
	~CBldPortal();

	BOOL UnPack(DWORD Block, BYTE **ppData, ULONG iSize);
	CEnvCell *GetOtherCell();
	void add_to_stablist(DWORD * &block_stab_list, DWORD &max_size, DWORD &stab_num);

	DWORD portal_side;
	DWORD other_cell_id;
	DWORD other_portal_id;
	DWORD exact_match;
	DWORD num_stabs; // 0x10
	DWORD* stab_list; // 0x14
};







