
#include <StdAfx.h>
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "Lifestone.h"
#include "Player.h"
#include "UseManager.h"

CBaseLifestone::CBaseLifestone()
{
	m_Qualities.id = 0x1FD;
	m_Qualities.m_WeenieType = LifeStone_WeenieType;

	SetName("Life Stone");
	SetItemType(TYPE_LIFESTONE);
	SetIcon(0x06001355);

	SetSetupID(0x020002EE);
	SetSoundTableID(0x20000014);
	SetMotionTableID(0x09000026);

	SetInitialPhysicsState(PhysicsState::GRAVITY_PS | PhysicsState::IGNORE_COLLISIONS_PS);
	
	m_Qualities.SetInt(ITEM_USEABLE_INT, USEABLE_REMOTE);
	m_Qualities.SetFloat(USE_RADIUS_FLOAT, 3.0f);

	m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, ShowAlways_RadarEnum);

	m_Qualities.m_WeenieType = LifeStone_WeenieType;
}

CBaseLifestone::~CBaseLifestone()
{
}

void CBaseLifestone::ApplyQualityOverrides()
{
	SetRadarBlipColor(LifeStone_RadarBlipEnum);
}

int CBaseLifestone::Use(CPlayerWeenie *pOther)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	useEvent->_do_use_animation = Motion_Sanctuary;
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CBaseLifestone::DoUseResponse(CWeenieObject *player)
{
	player->m_Qualities.SetPosition(SANCTUARY_POSITION, player->m_Position);
	player->AdjustStamina(player->GetStamina() * -0.5);
	return CWeenieObject::DoUseResponse(player);
}
