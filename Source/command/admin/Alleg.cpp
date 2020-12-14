#include <StdAfx.h>
#include "easylogging++.h"
#include "Client.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "Player.h"
#include "World.h"
#include "AllegianceManager.h"

#include "ClientCommands.h"

CLIENT_COMMAND(allegdump, "", "Prints out the allegiance hierarchy info on the last target you assessed.", ADMIN_ACCESS, GENERAL_CATEGORY)
{
	if (argc < 1)
		return true;

	if (pPlayer->m_LastAssessed)
	{
		CWeenieObject *target = g_pWorld->FindObject(pPlayer->m_LastAssessed);
		if (target)
		{
			AllegianceTreeNode *node = g_pAllegianceManager->GetTreeNode(target->GetID());
			if (node)
			{
				AllegianceInfo *info = g_pAllegianceManager->GetInfo(node->_monarchID);
				if (info)
				{
					AllegianceTreeNode *monarch = g_pAllegianceManager->GetTreeNode(node->_monarchID);
					// Monarch name
					pPlayer->SendText(csprintf("Monarch: %s", monarch->_charName.c_str()), LTT_DEFAULT);
					
					AllegianceTreeNode *patron = g_pAllegianceManager->GetTreeNode(node->_patronID);
					if (patron)
					{
						// Patron name
						pPlayer->SendText(csprintf("Patron: %s", patron->_charName.c_str()), LTT_DEFAULT);
					}
					else
						pPlayer->SendText("Is Monarch", LTT_DEFAULT);

					// Allegiance name
					pPlayer->SendText(csprintf("Allegiance Name: %s", info->_info.m_AllegianceName.c_str()), LTT_DEFAULT);
					// Number in allegiance
					pPlayer->SendText(csprintf("Number in Allegiance: %i", monarch->_numFollowers), LTT_DEFAULT);
					// Number of Vassels
					pPlayer->SendText(csprintf("Number in Vassels: %i", node->_numFollowers), LTT_DEFAULT);
					// Bind point
					Vector *f = info->_info.m_BindPoint.get_origin();
					float heading = info->_info.m_BindPoint.heading(info->_info.m_BindPoint);
					pPlayer->SendText(csprintf("Allegiance Hometown: 0x%08x [ X:%.2f Y:%.2f Z:%.2f ] w:%.2f x:0.0 y:0.0 z:0.0", info->_info.m_BindPoint.objcell_id, 
						f->x, f->y, f->z, heading), LTT_DEFAULT);
				}
			}
			
			
		}
	}

	return false;
}
