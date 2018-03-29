
#include "StdAfx.h"
#include "Portal.h"
#include "World.h"
#include "ChatMsgs.h"
#include "Player.h"

#define PORTAL_TRIGGER_DISTANCE 2.0f
#define PORTAL_TRIGGER_FREQUENCY 1.0f

CPortal::CPortal()
{
	m_Qualities.id = 0x82D;
	m_Qualities.m_WeenieType = Portal_WeenieType;

	SetName("Portal");
	SetIcon(0x0600106B);
	SetSetupID(0x20001B3);
	SetInitialPhysicsState(ETHEREAL_PS | REPORT_COLLISIONS_PS | LIGHTING_ON_PS | GRAVITY_PS);
	SetItemType(TYPE_PORTAL);
	
	m_Qualities.SetBool(STUCK_BOOL, TRUE);
	m_Qualities.SetBool(ATTACKABLE_BOOL, TRUE);
	m_Qualities.SetInt(ITEM_USEABLE_INT, USEABLE_REMOTE);
	m_Qualities.SetFloat(USE_RADIUS_FLOAT, -0.1f);
	
	// Arwic, for testing if we want to
	// m_Destination = Position(0xC6A90023, Vector(102.4f, 70.1f, 44.0f), Quaternion(0.70710677f, 0, 0, 0.70710677f));

#if 0 // deprecated
	m_fTickFrequency = 0.5f;
#endif

	m_Qualities.m_WeenieType = Portal_WeenieType;
}

CPortal::~CPortal()
{
}

#if 0 // deprecated
void CPortal::Tick()
{
	ProximityThink();
}
#endif

void CPortal::Tick()
{
	CWeenieObject::Tick();
}

void CPortal::CheckedTeleport(CWeenieObject *pOther)
{
	if (pOther && !(pOther->m_PhysicsState & PhysicsState::HIDDEN_PS))
	{
		ChanceExecuteEmoteSet(pOther->GetID(), Use_EmoteCategory);

		std::string restriction;
		if (m_Qualities.InqString(QUEST_RESTRICTION_STRING, restriction))
		{
			if (CPlayerWeenie *player = pOther->AsPlayer())
			{
				if (!player->InqQuest(restriction.c_str()))
				{
					pOther->SendText("You try to enter the portal but there is no effect.", LTT_MAGIC);
					return;
				}
			}
		}

		int minLevel = InqIntQuality(MIN_LEVEL_INT, 0);
		int maxLevel = InqIntQuality(MAX_LEVEL_INT, 0);

		int currentLevel = pOther->InqIntQuality(LEVEL_INT, 1);

		if (minLevel && currentLevel < minLevel)
		{
			pOther->SendText("You are not powerful enough to use this portal yet.", LTT_MAGIC);
		}
		else if (maxLevel && currentLevel > maxLevel)
		{
			pOther->SendText("You are too powerful to use this portal.", LTT_MAGIC);
		}
		else
		{
			Teleport(pOther);

			std::string questString;
			if (m_Qualities.InqString(QUEST_STRING, restriction))
			{
				if (CPlayerWeenie *player = pOther->AsPlayer())
					player->StampQuest(questString.c_str());
			}

			ChanceExecuteEmoteSet(pOther->GetID(), Portal_EmoteCategory);
		}
	}
}

int CPortal::DoCollision(const class ObjCollisionProfile &prof)
{
	if (prof._bitfield & Player_OCPB)
	{
		CheckedTeleport(g_pWorld->FindObject(prof.id));
	}

	return 1;
}

bool CPortal::GetDestination(Position &dest)
{
	return !!m_Qualities.InqPosition(DESTINATION_POSITION, dest) && dest.objcell_id;
}

void CPortal::Teleport(CWeenieObject *pTarget)
{
	Position dest;
	if (GetDestination(dest))
	{
		pTarget->StopCompletely(0);
		pTarget->Movement_Teleport(dest);

		if (!(m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x20))
		{
			pTarget->m_Qualities.SetPosition(LAST_PORTAL_POSITION, dest);
		}

		std::string questName;
		if (m_Qualities.InqString(QUEST_STRING, questName))
		{
			if (CPlayerWeenie *player = pTarget->AsPlayer())
			{
				player->StampQuest(questName.c_str());
			}
		}
	}
	else
	{
		pTarget->SendNetMessage(ServerText("This portal has no destination set.", 7), PRIVATE_MSG, FALSE, TRUE);
	}
}

#if 0 // deprecated
void CPortal::ProximityThink()
{
	// Clear old teleports cache
	if ((m_fLastCacheClear + PORTAL_TRIGGER_FREQUENCY) < g_pGlobals->Time())
	{
		m_RecentlyTeleported.clear();
		m_fLastCacheClear = g_pGlobals->Time();
	}

	std::list<CWeenieObject *> nearbyObjects;
	g_pWorld->EnumNearby(this, PORTAL_TRIGGER_DISTANCE, &nearbyObjects);

	for (std::list<CWeenieObject *>::iterator i = nearbyObjects.begin(); i != nearbyObjects.end(); i++)
	{
		CWeenieObject *pOther = *i;

		if (pOther->_IsPlayer() && !(pOther->m_PhysicsState & PhysicsState::LIGHTING_ON_PS))
		{
			if (m_RecentlyTeleported.find(pOther->GetID()) == m_RecentlyTeleported.end())
			{
				Teleport(pOther);
				m_RecentlyTeleported.insert(pOther->GetID());
			}
		}
	}
}
#endif

int CPortal::Use(CPlayerWeenie *pOther)
{
	CPortalUseEvent *useEvent = new CPortalUseEvent;
	useEvent->_target_id = GetID();
	pOther->ExecuteUseEvent(useEvent);
	return WERROR_NONE;
}

void CPortalUseEvent::OnReadyToUse()
{
	CWeenieObject *target = GetTarget();
	if (!target)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	((CPortal *)target)->CheckedTeleport(_weenie);
	Done();
}


