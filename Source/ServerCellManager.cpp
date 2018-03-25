
#include "StdAfx.h"
#include "ServerCellManager.h"
#include "World.h"
#include "WorldLandBlock.h"

ServerCellManager::ServerCellManager()
{
}

ServerCellManager::~ServerCellManager()
{
}

CObjCell *ServerCellManager::GetObjCell(DWORD cell_id)
{
	CWorldLandBlock *wlb = g_pWorld->GetLandblock((WORD)(cell_id >> 16), true);
	return wlb->GetObjCell((WORD)(cell_id & 0xFFFF));
}
