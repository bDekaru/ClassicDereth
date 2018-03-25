
#include "StdAfx.h"
#include "World.h"
#include "GameMode.h"
#include "Player.h"
#include "WeenieObject.h"
#include "ChatMsgs.h"

CGameMode::CGameMode()
{
}

CGameMode::~CGameMode()
{
}

CGameMode_Tag::CGameMode_Tag()
{
	m_pSelectedPlayer = NULL;
}

CGameMode_Tag::~CGameMode_Tag()
{
	UnselectPlayer();
}

const char *CGameMode_Tag::GetName()
{
	return "Tag";
}

void CGameMode_Tag::Think()
{
	if (!m_pSelectedPlayer)
	{
		// Find a player to make "it."
		PlayerWeenieMap *pPlayers = g_pWorld->GetPlayers();

		if (pPlayers->size() < 2)
		{
			return;
		}

		int index = Random::GenUInt(0, (unsigned int )(pPlayers->size() - 1));

		CPlayerWeenie *pSelected = NULL;
		int i = 0;

		for (auto& player : *pPlayers)
		{
			if (i == index)
			{
				pSelected = player.second;
				break;
			}

			i++;
		}

		SelectPlayer(pSelected);
	}
}

void CGameMode_Tag::SelectPlayer(CPlayerWeenie *pPlayer)
{
	if (!pPlayer)
	{
		UnselectPlayer();
		return;
	}

	m_pSelectedPlayer = pPlayer;

	m_pSelectedPlayer->EmitEffect(PS_HealthDownRed, 1.0f);
	g_pWorld->BroadcastGlobal(ServerText(csprintf("%s is it!", m_pSelectedPlayer->GetName().c_str()), LTT_DEFAULT), PRIVATE_MSG);
}

void CGameMode_Tag::UnselectPlayer()
{
	if (!m_pSelectedPlayer)
	{
		return;
	}
}

void CGameMode_Tag::OnTargetAttacked(CWeenieObject *pTarget, CWeenieObject *pSource)
{
	if (pSource == m_pSelectedPlayer)
	{
		if (CPlayerWeenie *pTargetPlayer = pTarget->AsPlayer())
		{
			UnselectPlayer();
			SelectPlayer(pTargetPlayer);
		}
	}
}

void CGameMode_Tag::OnRemoveEntity(CWeenieObject *pEntity)
{
	if (pEntity)
	{
		if (pEntity == m_pSelectedPlayer)
		{
			UnselectPlayer();
			m_pSelectedPlayer = NULL;
		}
	}
}
