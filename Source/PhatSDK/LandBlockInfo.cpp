
#include "StdAfx.h"
#include "LandBlockInfo.h"

CLandBlockInfo::CLandBlockInfo()
{
    num_objects = 0;
    object_ids = NULL;
    object_frames = NULL;

    num_buildings = 0;
    buildings = NULL;

    restriction_table = NULL;
    cell_ownership = NULL;

    num_cells = 0;
    cellids = NULL;
    cells = NULL;
}

CLandBlockInfo::~CLandBlockInfo()
{
    Destroy();
}

DBObj* CLandBlockInfo::Allocator()
{
    return((DBObj *)new CLandBlockInfo());
}

void CLandBlockInfo::Destroyer(DBObj* pLandBlockInfo)
{
    delete ((CLandBlockInfo *)pLandBlockInfo);
}

CLandBlockInfo *CLandBlockInfo::Get(DWORD ID)
{
    return (CLandBlockInfo *)ObjCaches::LandBlockInfos->Get(ID);
}

void CLandBlockInfo::Release(CLandBlockInfo *pLandBlockInfo)
{
    if (pLandBlockInfo)
        ObjCaches::LandBlockInfos->Release(pLandBlockInfo->GetID());
}

void CLandBlockInfo::Destroy()
{
    if (object_ids)
    {
        delete [] object_ids;
        object_ids = NULL;
    }

    if (object_frames)
    {
        delete [] object_frames;
        object_frames = NULL;
    }

    if (cells)
    {
        // MISSING CODE HERE!
        DEBUGOUT("Unfinished LandCell release code..\r\n");

        delete [] cells;
        cells = NULL;
    }

    if (cellids)
    {
        delete [] cellids;
        cellids = NULL;
    }

    if (buildings)
    {
        for (DWORD i = 0; i < num_buildings; i++)
        {
            if (buildings[i])
                delete buildings[i];
        }

        delete [] buildings;
        buildings = NULL;
    }

    if (restriction_table)
    {
        delete restriction_table;
        restriction_table = NULL;
    }

    if (cell_ownership)
    {
        delete cell_ownership;
        cell_ownership = NULL;
    }

    num_cells = 0;
    num_buildings = 0;

#if PHATSDK_USE_EXTENDED_CELL_DATA
	num_weenies = 0;
	SafeDeleteArray(weenie_wcids);
	SafeDeleteArray(weenie_pos);
	SafeDeleteArray(weenie_iids);

	num_weenie_links = 0;
	SafeDeleteArray(weenie_link_src);
	SafeDeleteArray(weenie_link_dst);
#endif
}

DWORD CLandBlockInfo::Unpack_Num_Cells(BYTE *pData)
{
    return *(DWORD *)(pData + sizeof(DWORD));
}

BOOL CLandBlockInfo::UnPack(BYTE **ppData, ULONG iSize)
{
    Destroy();

    UNPACK(DWORD, id);

#if !PHATSDK_USE_EXTENDED_CELL_DATA
    UNPACK(DWORD, num_cells);
#endif

    UNPACK(DWORD, num_objects);

    if (num_objects > 0)
    {
        object_ids = new DWORD[ num_objects ];
        object_frames = new Frame[ num_objects ];

        for (DWORD i = 0; i < num_objects; i++)
        {
            UNPACK(DWORD, object_ids[i]);
            UNPACK_OBJ(object_frames[i]);
        }
    }

    DWORD LandBlock = GetID() & 0xFFFF0000;

    DWORD BuildingInfo, BuildingCount, BuildingFlags;

    UNPACK(DWORD, BuildingInfo);
    BuildingCount = LOWORD(BuildingInfo);
    BuildingFlags = HIWORD(BuildingInfo);

	/*
	enum LBIPackMask
	{
		NoAdditionalData_LBIPackMaskEnum = 0x0,
		RestrictionTable_LBIPackMaskEnum = 0x1,
		CellIDTable_LBIPackMaskEnum = 0x2,
		FORCE_LBIPackMaskEnum_16_BIT = 0xFFFF,
	};
	*/
    
    num_buildings = BuildingCount;

    if (BuildingCount > 0)
    {
        buildings = new BuildInfo*[ BuildingCount ];

        for (DWORD i = 0; i < BuildingCount; i++)
        {
            buildings[i] = new BuildInfo;

            UNPACK(DWORD, buildings[i]->building_id);

            UNPACK_OBJ(buildings[i]->building_frame);

            UNPACK(DWORD, buildings[i]->num_leaves);
            UNPACK(DWORD, buildings[i]->num_portals);

            buildings[i]->portals = new CBldPortal*[ buildings[i]->num_portals ];

            for (DWORD j = 0; j < buildings[i]->num_portals; j++)
            {
                buildings[i]->portals[j] = new CBldPortal;
                buildings[i]->portals[j]->UnPack(LandBlock, ppData, iSize);
            }
        }
    }

#ifdef PRE_TOD
    PACK_ALIGN();
#endif

#if PHATSDK_USE_EXTENDED_CELL_DATA
	DWORD version;
	UNPACK(DWORD, version);

	UNPACK(DWORD, num_weenies);
	if (num_weenies > 0)
	{
		weenie_wcids = new DWORD[num_weenies];
		weenie_pos = new Position[num_weenies];
		weenie_iids = new DWORD[num_weenies];

		for (DWORD i = 0; i < num_weenies; i++)
		{
			UNPACK(DWORD, weenie_wcids[i]);
			UNPACK_OBJ_READER(weenie_pos[i]);
			UNPACK(DWORD, weenie_iids[i]);
		}
	}

	UNPACK(DWORD, num_weenie_links);
	if (num_weenie_links > 0)
	{
		weenie_link_src = new DWORD[num_weenie_links];
		weenie_link_dst = new DWORD[num_weenie_links];

		for (DWORD i = 0; i < num_weenie_links; i++)
		{
			UNPACK(DWORD, weenie_link_src[i]);
			UNPACK(DWORD, weenie_link_dst[i]);
		}
	}
#endif

    if (BuildingFlags & 1)
    {
        restriction_table = new PackableHashTable<DWORD, DWORD>;
		
		if (!UNPACK_OBJ_READER(*restriction_table))
			return FALSE;
    }

    PACK_ALIGN();

#if PHATSDK_USE_EXTENDED_CELL_DATA
	UNPACK(DWORD, num_cells);
	// etc
#endif

    return TRUE;
}

BuildInfo::BuildInfo()
{
    building_id = 0;

    num_leaves = 0;
    num_portals = 0;
    portals = NULL;
}

BuildInfo::~BuildInfo()
{
    for (DWORD i = 0; i < num_portals; i++)
    {
        if (portals[i])
            delete portals[i];
    }

    delete [] portals;
    portals = NULL;
    building_id = 0;

    num_portals = 0;
    num_leaves = 0;
}

CBldPortal::CBldPortal()
{
    portal_side = -1;
    other_cell_id = 0;
    other_portal_id = -1;
    exact_match = 0;
    num_stabs = 0;
    stab_list = NULL;
}

CBldPortal::~CBldPortal()
{
    delete [] stab_list;
}

BOOL CBldPortal::UnPack(DWORD Block, BYTE **ppData, ULONG iSize)
{
    BYTE Flags;
    UNPACK(WORD, Flags);

    exact_match = FBitSet(Flags, 0);
    portal_side = FBitSet(~Flags, 1);

    DWORD Cell;
    UNPACK(WORD, Cell);

    other_cell_id = Block | Cell;

    UNPACK(WORD, other_portal_id);

    // Unpack the cell IDs visible to this one.
    UNPACK(WORD, num_stabs);
    stab_list = new DWORD[ num_stabs ];

    for (DWORD i = 0; i < num_stabs; i++)
    {
        DWORD VisCellID;
        UNPACK(WORD, VisCellID);

        stab_list[i] = Block | VisCellID;
    }

#ifdef PRE_TOD
    PACK_ALIGN();
#else
	if (num_stabs & 1)
		*ppData += sizeof(WORD);
#endif

    return TRUE;
}

CEnvCell *CBldPortal::GetOtherCell()
{
	return CEnvCell::GetVisible(other_cell_id);
}

void CBldPortal::add_to_stablist(DWORD * &block_stab_list, DWORD &max_size, DWORD &stab_num)
{
	for (DWORD i = 0; i < num_stabs; i++)
	{
		DWORD j = stab_num;
		if (j)
		{
			do 
			{
				if (stab_list[i] == block_stab_list[j - 1])
					break;
				j--;
			} while (j);
		}

		if (!j)
		{
			if (stab_num >= max_size)
			{
				DWORD *old = block_stab_list;
				block_stab_list = new DWORD[max_size + 10];
				max_size += 10;

				for (DWORD k = 0; k < stab_num; k++)
					block_stab_list[k] = old[k];
				delete [] old;
			}
			block_stab_list[stab_num] = stab_list[i];
			stab_num++;
		}
	}
}


DWORD CLandBlockInfo::GetRestrictionIID(DWORD landcellid)
{
	if (restriction_table)
	{
		const DWORD *value = restriction_table->lookup(landcellid);
		if (value != 0)
			return *value;
	}

	return 0;
}


















