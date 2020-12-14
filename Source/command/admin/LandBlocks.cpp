#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Player.h"
#include "World.h"
#include "WorldLandBlock.h"
#include "ChatMsgs.h"
#include "InferredCellData.h"

#include "ClientCommands.h"

CLIENT_COMMAND(resetlb, "[lbid]", "Reset the current or specified landblock (in hex)", ADMIN_ACCESS, SERVER_CATEGORY)
{
	uint32_t lbid = 0;

	if (argc > 0)
	{
		lbid = strtoul(argv[0], NULL, 16);
	}
	else
	{
		lbid = (pPlayer->GetLandcell() & LandDefs::blockid_mask) >> LandDefs::block_part_shift;
	}

	SERVER_INFO << "Attempt to reset" << lbid;

	CWorldLandBlock *block = g_pWorld->GetLandblock((WORD)lbid);
	if (block)
	{
		block->RespawnNextTick();
		return false;
	}
	else
	{
		SERVER_INFO << "Couldn't find landblock" << lbid;
		pPlayer->SendText("Couldn't find that landblock!", LTT_DEFAULT);
		return false;
	}

	return true;
}

CLIENT_COMMAND(reloadlb, "[lbid]", "Reset the current or specified landblock (in hex)", ADMIN_ACCESS, SERVER_CATEGORY)
{
	uint32_t lbid = 0;

	if (argc > 0)
	{
		lbid = strtoul(argv[0], NULL, 16);
	}
	else
	{
		lbid = (pPlayer->GetLandcell() & LandDefs::blockid_mask) >> LandDefs::block_part_shift;
	}

	SERVER_INFO << "Attempt to reload" << lbid;

	CWorldLandBlock *block = g_pWorld->GetLandblock((WORD)lbid);
	if (block)
	{
		fs::path dataPath(g_pGlobals->GetGameData("Data", "json"));

		bool refreshed = g_pCellDataEx->RefreshLocalSpawnMap(lbid);

		if (!refreshed)
		{
			pPlayer->SendText("Couldn't reload that landblock! Attempting to reload spawmMaps folder.", LTT_DEFAULT);
			g_pCellDataEx->RefreshLocalSpawnMapStorage();

			if (!g_pCellDataEx->FindSpawnMap(lbid << 16))
			{
				pPlayer->SendText("Failed to reload that landblock!", LTT_DEFAULT);
				return false;
			}
			else
			{
				pPlayer->SendText("Landblock found in spawmMaps folder.", LTT_DEFAULT);
			}
		}

		block->RespawnNextTick();
		pPlayer->SendText("Landblock reloaded.", LTT_DEFAULT);
		return false;
	}
	else
	{
		SERVER_INFO << "Couldn't find landblock" << lbid;
		pPlayer->SendText("Couldn't find that landblock!", LTT_DEFAULT);
	}

	return true;
}
