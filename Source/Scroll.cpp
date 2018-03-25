
#include "StdAfx.h"
#include "WeenieObject.h"
#include "Scroll.h"
#include "Player.h"

CScrollWeenie::CScrollWeenie()
{
	SetName("Scroll");
	m_Qualities.m_WeenieType = Scroll_WeenieType;
}

CScrollWeenie::~CScrollWeenie()
{
}

void CScrollWeenie::ApplyQualityOverrides()
{
}

const CSpellBase *CScrollWeenie::GetSpellBase()
{
	DWORD spell_id = 0;
	if (m_Qualities.InqDataID(SPELL_DID, spell_id))
	{
		return MagicSystem::GetSpellTable()->GetSpellBase(spell_id);
	}

	return NULL;
}

int CScrollWeenie::Use(CPlayerWeenie *player)
{
	const CSpellBase *spell = GetSpellBase();

	if (!spell)
	{
		player->SendText("This scroll doesn't have a spell?", LTT_DEFAULT);
		player->NotifyInventoryFailedEvent(GetID(), WERROR_NONE);
		player->NotifyUseDone();
		return WERROR_NONE;
	}

	//You must have a certain skill level in the spell skill of the spell you wish to learn. The required skill levels are the following:
	//Level 2 : 0 skill
	//Level 3 : 50 skill
	//Level 4 : 100 skill
	//Level 5 : 150 skill
	//Level 6 : 200 skill
	//Levels 1 and 7 spells can be learned by anyone, regardless if they have the school trained or not.

	bool alwaysSucceed = false;
	if (spell->_power < 50 || spell->_power >= 300)
		alwaysSucceed = true;

	DWORD magicSkill = 0;
	if (!alwaysSucceed)
	{
		if (!player->InqSkill(spell->InqSkillForSpell(), magicSkill, TRUE) || !magicSkill)
		{
			player->SendText(csprintf("You are not trained in %s!", CachedSkillTable->GetSkillName(spell->InqSkillForSpell()).c_str()), LTT_DEFAULT);
			player->NotifyInventoryFailedEvent(GetID(), WERROR_SKILL_TOO_LOW);
			player->NotifyUseDone();
			return WERROR_NONE;
		}
		else if (magicSkill < spell->_power - 50)
		{
			player->SendText(csprintf("You are not skilled enough in %s to learn this spell.", CachedSkillTable->GetSkillName(spell->InqSkillForSpell()).c_str()), LTT_DEFAULT);
			player->NotifyInventoryFailedEvent(GetID(), WERROR_SKILL_TOO_LOW);
			player->NotifyUseDone();
			return WERROR_NONE;
		}
	}

	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyInventoryFailedEvent(GetID(), WERROR_OBJECT_GONE);
		player->NotifyUseDone();
		return WERROR_NONE;
	}

	CScrollUseEvent *useEvent = new CScrollUseEvent;
	useEvent->_target_id = GetID();
	player->ExecuteUseEvent(useEvent);
	return WERROR_NONE;
}

void CScrollUseEvent::OnReadyToUse()
{
	ExecuteUseAnimation(Motion_Reading);
}

void CScrollUseEvent::OnUseAnimSuccess(DWORD motion)
{
	CWeenieObject *target = GetTarget();

	if (target)
	{
		if (_weenie->LearnSpell(target->InqDIDQuality(SPELL_DID, 0), true))
		{
			// destroy it if the spell was learned
			target->ReleaseFromAnyWeenieParent();
			_weenie->NotifyContainedItemRemoved(target->GetID());

			target->MarkForDestroy();
		}
	}

	_weenie->DoForcedStopCompletely();

	Done();
}
