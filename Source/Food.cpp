
#include <StdAfx.h>
#include "WeenieObject.h"
#include "Food.h"
#include "Player.h"
#include "SpellcastingManager.h"

CFoodWeenie::CFoodWeenie()
{
	SetName("Food");
	m_Qualities.m_WeenieType = Food_WeenieType;
}

CFoodWeenie::~CFoodWeenie()
{
}

void CFoodWeenie::ApplyQualityOverrides()
{
}

int CFoodWeenie::Use(CPlayerWeenie *pOther)
{
	if (!pOther->FindContainedItem(GetID()))
		return WERROR_OBJECT_GONE;

	if (pOther->IsInPortalSpace())
		return WERROR_ACTIONS_LOCKED;

	CFoodUseEvent *useEvent = new CFoodUseEvent;
	useEvent->_target_id = GetID();
	useEvent->_do_use_animation = Motion_Eat;
	useEvent->_initial_use_position = pOther->m_Position;
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CFoodWeenie::DoUseResponse(CWeenieObject *other)
{
	if (!other->FindContainedItem(GetID()))
		return WERROR_OBJECT_GONE;

	other->DoForcedStopCompletely();
	
	if (uint32_t use_sound_did = InqDIDQuality(USE_SOUND_DID, 0))
		other->EmitSound(use_sound_did, 1.0f);
	else
		other->EmitSound(Sound_Eat1, 1.0f);

	if (uint32_t spell_did = InqDIDQuality(SPELL_DID, 0))
		MakeSpellcastingManager()->CastSpellInstant(other->GetID(), spell_did);
	else
		other->SendText("You can't do that yet!", LTT_ERROR);

	uint32_t boost_stat = InqIntQuality(BOOSTER_ENUM_INT, 0);
	int boost_value = InqIntQuality(BOOST_VALUE_INT, 0);

	switch (boost_stat)
	{
	case HEALTH_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		{
			STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
			STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

			uint32_t statValue = 0, maxStatValue = 0;
			other->m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
			other->m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);
			
			int currentMax = static_cast<int>(maxStatValue);
			int currentStat = static_cast<int>(statValue);
			int diff = 0;

			if (boost_value + currentStat < currentMax)
			{
				if (boost_value + currentStat <= 0)
				{
					diff = currentStat;
					currentStat = 0;
				}
				else
				{
					currentStat += boost_value;
					diff = boost_value;
				}
			}
			else
			{
				diff = currentMax - currentStat;
				currentStat = currentMax;
			}

			if (other->AsPlayer() && statType == HEALTH_ATTRIBUTE_2ND)
			{
				other->SetHealth(currentStat);
				other->NotifyAttribute2ndStatUpdated(statType);
			}
			else
			{
				other->m_Qualities.SetAttribute2nd(statType, currentStat);
				other->NotifyAttribute2ndStatUpdated(statType);
			}

			const char *vitalName = "";
			switch (boost_stat)
			{
			case HEALTH_ATTRIBUTE_2ND: vitalName = "health"; break;
			case STAMINA_ATTRIBUTE_2ND: vitalName = "stamina"; break;
			case MANA_ATTRIBUTE_2ND: vitalName = "mana"; break;
			}

			if (boost_value >= 0)
				other->SendText(csprintf("The %s restores %d points of your %s.", GetName().c_str(), abs(diff), vitalName), LTT_DEFAULT);
			else
				other->SendText(csprintf("The %s takes %d points of your %s.", GetName().c_str(), abs(diff), vitalName), LTT_DEFAULT);

			if (boost_stat == HEALTH_ATTRIBUTE_2ND)
			{
				if (other->AsPlayer())
				{
					// update the target's health on the healing player asap
					((CPlayerWeenie*)other)->RefreshTargetHealth();
				}
			}
			break;
		}
	}

	DecrementStackOrStructureNum(true);
	return CWeenieObject::DoUseResponse(other);
}

void CFoodUseEvent::OnUseAnimSuccess(uint32_t motion)
{
	CWeenieObject *item = GetTarget();
	if (item && item->InqIntQuality(BOOSTER_ENUM_INT, 0) == HEALTH_ATTRIBUTE_2ND && _weenie->AsPlayer() && _weenie->m_Position.distance(_initial_use_position) > 5.0)
		return Cancel(WERROR_MOVED_TOO_FAR);

	Finish();
}

