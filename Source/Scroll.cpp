
#include <StdAfx.h>
#include "WeenieObject.h"
#include "Scroll.h"
#include "Player.h"
#include "Client.h"
#include "World.h"
#include "SpellcastingManager.h"

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
	uint32_t spell_id = 0;


	if (m_Qualities.InqDataID(SPELL_DID, spell_id))
	{
		return MagicSystem::GetSpellTable()->GetSpellBase(spell_id);
	}

	return NULL;
}

bool CScrollWeenie::AttemptSpellResearch(CPlayerWeenie *player, uint32_t target_Id)
{
	if (!player)
		return false;

	if (player->IsInPeaceMode() || !player->GetWieldedCasterID())
	{
		player->SendText("You must be in spell casting mode to research spells.", LTT_DEFAULT);
		return false;
	}

	bool found = false;
	CContainerWeenie *container = NULL;
	//check the main container first:
	if (player->GetID() == GetContainerID())
	{
		container = player;
	}
	else
	{
		for (auto pack : player->m_Packs)
		{
			if (pack->GetID() == GetContainerID())
			{
				container = (CContainerWeenie *)pack;
				break;
			}
		}
	}

	if (container)
	{
		//here we go!
		SpellFormula formula;

		bool found = false;
		int componentCounter = 0;
		std::string componentString = "You attempt to cast a spell using ";

		for (auto item : container->m_Items)
		{
			if (componentCounter > 7)
				break; //At most 8 spell components are allowed.

			if (item->GetID() == GetID())
			{
				found = true; //We've found ourselves! What follows are the spell components.
				continue;
			}

			if (found)
			{
				if (item->m_Qualities.m_WeenieType == SpellComponent_WeenieType && !item->InqIntQuality(BONDED_INT, 0))//peas have BONDED_INT value of 1, use that to discard them as a valid component.
				{
					formula._comps[componentCounter] = item->InqDIDQuality(SPELL_COMPONENT_DID, 0);
					//player->SendText(csprintf("Component %d: %s.", componentCounter + 1, item->GetName().c_str()), LTT_DEFAULT);
					if (componentCounter > 0)
						componentString.append(", ");
					componentString.append(item->GetName());
					componentCounter++;
				}
				else
					break;
			}
		}

		if (!componentCounter)
			player->SendText("You attempt to cast a spell using no spell components! Now wouldn't that be nice!", LTT_DEFAULT);
		else
		{
			CWeenieObject *target = g_pWorld->FindObject(target_Id);
			if (target == player)
				componentString.append(" and targetting yourself.");
			else if(target)
				componentString.append(csprintf(" and targetting %s.", target->GetName().c_str()));
			else
				componentString.append(" without an specific target.");
			player->SendText(componentString.c_str(), LTT_DEFAULT);
		}

		CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
		uint32_t spellId = pSpellTable->GetSpellByComponentHash(formula.GetComponentHash());

		if (spellId)
		{
			const CSpellBase *spellBase = pSpellTable->GetSpellBase(spellId);

			SpellFormula randomizedComponents;
			randomizedComponents.CopyFrom(spellBase->InqSpellFormula());

			randomizedComponents.RandomizeForName(player->GetClient()->GetAccount(), spellBase->_formula_version);

			for (int i = 0; i < 8; i++)
			{
				if (formula._comps[i] != randomizedComponents._comps[i])
				{
					//mismatch! abort!
					//player->SendText(csprintf("Invalid tapers!"), LTT_DEFAULT);
					spellId = 0; //set spellId to 0 so we get a false cast attempt and not the real thing. 
					break;
				}
			}

			//if (spellId) //we still have a valid spellId that means it's the real deal.
			//	player->SendText(csprintf("Valid formula found:  %s!", spellBase->_name.c_str()), LTT_DEFAULT);
		}
		//else
		//	player->SendText(csprintf("Invalid formula!"), LTT_DEFAULT);

		player->MakeSpellcastingManager()->TryResearchCast(this, target_Id, formula, spellId);
		return true;
	}

	return false;
}

void CScrollWeenie::ResearchAttemptFinished(CPlayerWeenie *player, uint32_t spellId, bool fizzled)
{
	if (!player || fizzled || !spellId)
		return;

	player->LearnSpell(spellId, true);
}

int CScrollWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	CScrollUseEvent *useEvent = new CScrollUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 20.0;
	useEvent->_initial_use_position = player->m_Position;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
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

	uint32_t magicSkill = 0;
	if (!alwaysSucceed)
	{
		if (!player->InqSkill(spell->InqSkillForSpell(), magicSkill, FALSE) || !magicSkill)
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
	CWeenieObject *scroll = GetTool();
	if (scroll && scroll->m_Qualities.id == 80000) //skip the reading when researching spells
	{
		bool handled = (scroll->AsScroll())->AttemptSpellResearch(_weenie->AsPlayer(), _target_id);
		Done(WERROR_NONE, handled); //Silent if AttemptSpellResearch() already took care of it. Otherwise we get a stuck hourglass pointer.
	}
	else
		ExecuteUseAnimation(Motion_Reading);
}

void CScrollUseEvent::OnUseAnimSuccess(uint32_t motion)
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
