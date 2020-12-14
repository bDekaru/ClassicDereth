
#include <StdAfx.h>
#include "WeenieObject.h"
#include "ManaStone.h"
#include "Player.h"
#include <queue>

CManaStoneWeenie::CManaStoneWeenie()
{
	SetName("ManaStone");
	m_Qualities.m_WeenieType = ManaStone_WeenieType;
}

CManaStoneWeenie::~CManaStoneWeenie()
{
}

void CManaStoneWeenie::ApplyQualityOverrides()
{
}

struct CompareManaNeeds //: public std::function<CWeenieObject*, CWeenieObject*, bool>
{
	bool operator()(CWeenieObject* left, CWeenieObject* right)
	{
		// comparator for making a min-heap based on remaining mana
		return ((left->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE) - left->InqIntQuality(ITEM_CUR_MANA_INT, -1, TRUE))
			> (right->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE) - right->InqIntQuality(ITEM_CUR_MANA_INT, -1, TRUE)));
	}
};

int CManaStoneWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	CManaStoneUseEvent *useEvent = new CManaStoneUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 1.0; // todo: change to 0?
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CManaStoneWeenie::DoUseWithResponse(CWeenieObject* source, CWeenieObject* pTarget)
{
	CPlayerWeenie* player = source->AsPlayer();
	int targetType = pTarget->InqIntQuality(ITEM_TYPE_INT, 0);
	if (!(targetType & TYPE_ITEM) && !(targetType & TYPE_CREATURE)) 
	{
		player->SendTextToOverlay(csprintf("You can't use %s on %s", this->GetName().c_str(), pTarget->GetName().c_str())); //todo: made up message, confirm if it's correct
		return WERROR_NONE;
	}

	int manaStoneCurrentMana = InqIntQuality(ITEM_CUR_MANA_INT, -1, TRUE);
	double manaStoneDestroyChance = InqFloatQuality(MANA_STONE_DESTROY_CHANCE_FLOAT, -1, TRUE);
	double manaStoneEfficiency = InqFloatQuality(ITEM_EFFICIENCY_FLOAT, -1, TRUE);
	int targetCurrentMana = pTarget->InqIntQuality(ITEM_CUR_MANA_INT, -1, TRUE);
	int targetMaxMana = pTarget->InqIntQuality(ITEM_MAX_MANA_INT, -1, TRUE);

	if (manaStoneCurrentMana <= 0) 
	{
		if (targetType & TYPE_CREATURE || targetCurrentMana <= 0) 
		{
			player->SendTextToOverlay(csprintf("You can't use %s on %s",this->GetName().c_str(),pTarget->GetName().c_str())); //todo: made up message, confirm if it's correct
			return WERROR_NONE;
		}

		if (!pTarget->InqBoolQuality(RETAINED_BOOL, FALSE)) 
		{
			int drainedMana = round(targetCurrentMana * manaStoneEfficiency);
			m_Qualities.SetInt(ITEM_CUR_MANA_INT, drainedMana);
			m_Qualities.SetInt(UI_EFFECTS_INT, 1);
			NotifyIntStatUpdated(ITEM_CUR_MANA_INT, false);
			NotifyIntStatUpdated(UI_EFFECTS_INT, false);
			pTarget->Remove();
			RecalculateEncumbrance();

			// The Mana Stone drains 4,434 points of mana from the Pocket Watch.
			// The Pocket Watch is destroyed.
			player->SendText(csprintf("The %s drains %s points of mana from the %s.\nThe %s is destroyed.", GetName().c_str(), FormatNumberString(drainedMana).c_str(), pTarget->GetName().c_str(), pTarget->GetName().c_str()), LTT_DEFAULT);
		}
		else {
			player->SendTextToOverlay("That item is retained!"); //todo: made up message, confirm if it's correct
			return WERROR_BAD_PARAM;
		}
	}
	else
	{
		if (targetType & TYPE_CREATURE)  // mana stone is being emptied into user, not filled
		{
			if (pTarget->id != player->id)
			{
				// somehow not using on self. shouldn't be possible with the client. Made up message.
				player->SendText(csprintf("You cannot use the %s on other creatures or players.", GetName().c_str()), LTT_DEFAULT);
				return WERROR_BAD_PARAM;
			}

			std::priority_queue<CWeenieObject*, std::vector<CWeenieObject*>, CompareManaNeeds > itemsNeedingMana, itemsStillNeedingMana; // MIN heaps sorted by mana deficit

			for (auto wielded : player->m_Wielded)
			{
				int currentMana = wielded->InqIntQuality(ITEM_CUR_MANA_INT, 0, TRUE);
				int maxMana = wielded->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE);
				int deficit = maxMana - currentMana;

				if (deficit > 0)
				{
					itemsNeedingMana.push(wielded);
				}
			}

			if (itemsNeedingMana.empty())
			{
				player->SendText("None of your items need mana.", LTT_DEFAULT); // What's the correct text?
				return WERROR_BAD_PARAM;
			}

			int manaToDistribute = manaStoneCurrentMana;
			int manaDistributed = 0;
			std::set<CWeenieObject*> objectsReceivingMana; // for the chatmessage at the end

			manaToDistribute *= player->m_Qualities.GetPositiveRating(LUM_AUG_ITEM_MANA_GAIN_INT, false);

			while (manaToDistribute > 0 && !itemsNeedingMana.empty())
			{
				CWeenieObject* item = itemsNeedingMana.top();
				itemsNeedingMana.pop();
				int itemMaxMana = item->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE);
				int itemCurrentMana = item->InqIntQuality(ITEM_CUR_MANA_INT, 0, TRUE);
				int itemDeficit = itemMaxMana - itemCurrentMana;

				if (manaToDistribute >= itemDeficit)
				{
					manaToDistribute -= itemDeficit;
					manaDistributed += itemDeficit;
					item->m_Qualities.SetInt(ITEM_CUR_MANA_INT, itemMaxMana);
				}
				else
				{
					int newManaAmount = itemCurrentMana + manaToDistribute;
					manaDistributed += manaToDistribute;
					manaToDistribute = 0;
					item->m_Qualities.SetInt(ITEM_CUR_MANA_INT, newManaAmount);
					itemsStillNeedingMana.push(item);
				}

				item->NotifyIntStatUpdated(ITEM_CUR_MANA_INT, false);
				objectsReceivingMana.insert(item);
			}

			std::stringstream ss;
			bool first = true;
			ss << csprintf("The %s gives %s points of mana to the following items: ", GetName().c_str(), FormatNumberString(manaDistributed).c_str());
			for (auto item : objectsReceivingMana)
			{
				if (first)
				{
					ss << item->GetName();
					first = false;
					continue;
				}
				ss << ", ";
				ss << item->GetName();
			}
			ss << ".";
			player->SendText(ss.str().c_str(), LTT_DEFAULT);

			int remainingDeficit = 0;
			while (!itemsNeedingMana.empty())
			{
				remainingDeficit += (itemsNeedingMana.top()->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE) - itemsNeedingMana.top()->InqIntQuality(ITEM_CUR_MANA_INT, 0, TRUE));
				itemsNeedingMana.pop();
			}

			while (!itemsStillNeedingMana.empty())
			{
				remainingDeficit += (itemsStillNeedingMana.top()->InqIntQuality(ITEM_MAX_MANA_INT, 0, TRUE) - itemsStillNeedingMana.top()->InqIntQuality(ITEM_CUR_MANA_INT, 0, TRUE));
				itemsStillNeedingMana.pop();
			}

			if (remainingDeficit == 0)
			{
				player->SendText("Your items are fully charged.", LTT_DEFAULT);
			}
			else
			{
				player->SendText(csprintf("You need %s more mana to fully charge your items.", FormatNumberString(remainingDeficit).c_str()), LTT_DEFAULT);
			}
		}
		else // manastone being emptied into an item
		{
			if (targetMaxMana <= 0)
			{
				SendText(csprintf("The %s does not require mana.", pTarget->GetName().c_str()), LTT_DEFAULT); //todo: made up message, confirm if it's correct
				return WERROR_NONE;
			}
			else if (targetCurrentMana >= targetMaxMana)
			{
				SendText(csprintf("The %s is already fully charged.", pTarget->GetName().c_str()), LTT_DEFAULT); //todo: made up message, confirm if it's correct
				return WERROR_NONE;
			}
			else
			{
				pTarget->m_Qualities.SetInt(ITEM_CUR_MANA_INT, min(targetCurrentMana + manaStoneCurrentMana, targetMaxMana));
				pTarget->NotifyIntStatUpdated(ITEM_CUR_MANA_INT, false);
				player->SendText(csprintf("The %s gives %s points of mana to the %s.", GetName().c_str(), FormatNumberString(manaStoneCurrentMana).c_str(), pTarget->GetName().c_str()), LTT_DEFAULT);
			}
		}

		if (manaStoneDestroyChance >= 1.0 || Random::RollDice(0.0, 1.0) <= manaStoneDestroyChance)
		{
			player->SendText(csprintf("The %s is destroyed.", GetName().c_str()), LTT_DEFAULT);
			Remove();
			RecalculateEncumbrance();
		}
		else
		{
			m_Qualities.SetInt(ITEM_CUR_MANA_INT, 0);
			NotifyIntStatUpdated(ITEM_CUR_MANA_INT, false);
			m_Qualities.SetInt(UI_EFFECTS_INT, 0);
			NotifyIntStatUpdated(UI_EFFECTS_INT, false);
		}	
	}
	return WERROR_NONE;
}



void CManaStoneUseEvent::OnReadyToUse()
{
	CWeenieObject *target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	CWeenieObject *tool = GetTool();
	if (!tool && _tool_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	int error = WERROR_NONE;

	if (target)
	{
		error = tool->DoUseWithResponse(_weenie, target);
	}

	Done(error);
}
