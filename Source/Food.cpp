
#include "StdAfx.h"
#include "WeenieObject.h"
#include "Food.h"
#include "Player.h"

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

	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	useEvent->_do_use_animation = Motion_Eat;
	pOther->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CFoodWeenie::DoUseResponse(CWeenieObject *other)
{
	if (!other->FindContainedItem(GetID()))
		return WERROR_OBJECT_GONE;

	other->DoForcedStopCompletely();
	
	if (DWORD use_sound_did = InqDIDQuality(USE_SOUND_DID, 0))
		other->EmitSound(use_sound_did, 1.0f);
	else
		other->EmitSound(Sound_Eat1, 1.0f);

	DWORD boost_stat = InqIntQuality(BOOSTER_ENUM_INT, 0);
	DWORD boost_value = InqIntQuality(BOOST_VALUE_INT, 0);

	switch (boost_stat)
	{
	case HEALTH_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		{
			STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
			STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

			DWORD statValue = 0, maxStatValue = 0;
			other->m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
			other->m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);

			DWORD newStatValue = min(statValue + boost_value, maxStatValue);

			int statChange = newStatValue - statValue;
			if (statChange)
			{
				other->m_Qualities.SetAttribute2nd(statType, newStatValue);
				other->NotifyAttribute2ndStatUpdated(statType);
			}

			const char *vitalName = "";
			switch (boost_stat)
			{
			case HEALTH_ATTRIBUTE_2ND: vitalName = "health"; break;
			case STAMINA_ATTRIBUTE_2ND: vitalName = "stamina"; break;
			case MANA_ATTRIBUTE_2ND: vitalName = "mana"; break;
			}

			other->SendText(csprintf("The %s restores %d points of your %s.", GetName().c_str(), max(0, statChange), vitalName), LTT_DEFAULT);
			break;
		}
	}

	DecrementStackOrStructureNum(true);
	return WERROR_NONE;
}

