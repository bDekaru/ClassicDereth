
#include <StdAfx.h>
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "BindStone.h"
#include "Player.h"
#include "UseManager.h"
#include "AllegianceManager.h"

CBindStone::CBindStone()
{
}

CBindStone::~CBindStone()
{
}

int CBindStone::Use(CPlayerWeenie *pOther)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	//useEvent->_do_use_animation = Motion_Sanctuary;
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CBindStone::DoUseResponse(CWeenieObject *player)
{
	uint32_t playerId = player->GetID();
	AllegianceTreeNode *allegianceNode = g_pAllegianceManager->GetTreeNode(playerId);
	if (!allegianceNode)
		return WERROR_ALLEGIANCE_NONEXISTENT;

	AllegianceInfo *allegianceInfo = g_pAllegianceManager->GetInfo(allegianceNode->_monarchID);

	eAllegianceOfficerLevel officerLevel = eAllegianceOfficerLevel::Undef_AllegianceOfficerLevel;
	auto iterator = allegianceInfo->_info.m_AllegianceOfficers.find(playerId);
	if (iterator != allegianceInfo->_info.m_AllegianceOfficers.end())
		officerLevel = iterator->second;

	if (allegianceNode->_monarchID == playerId || officerLevel >= eAllegianceOfficerLevel::Castellan_AllegianceOfficerLevel)
	{
		allegianceInfo->_info.m_BindPoint = m_Position;
		g_pAllegianceManager->Save();
	}
	else
		return WERROR_ALLEGIANCE_NOT_AUTHORIZED;

	return CWeenieObject::DoUseResponse(player);
}
