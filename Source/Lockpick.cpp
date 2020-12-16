
#include <StdAfx.h>
#include "Lockpick.h"
#include "UseManager.h"
#include "Player.h"
#include "Door.h"
#include "Chest.h"

static const char* lockedString(bool v) {
	return (v ? "locked" : "unlocked");
}

static bool isClosed(CWeenieObject* with) {
	CBaseDoor* door = with->AsDoor();
	if (door) {
		return door->IsClosed();
	}
	CChestWeenie* chest = with->AsChest();
	if (chest) {
		return chest->IsClosed();
	}

	return false;
}

CLockpickWeenie::CLockpickWeenie()
{
}

CLockpickWeenie::~CLockpickWeenie()
{
}

int CLockpickWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 1.0;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CLockpickWeenie::DoUseWithResponse(CWeenieObject *player, CWeenieObject *with)
{
	if (CPlayerWeenie *player_weenie = player->AsPlayer())
	{
		bool questChest = false;
		std::string questString;
		if (with->m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
		{
			questChest = true;

			if (player->InqQuest(questString.c_str()))
			{
				int timeTilOkay = player->InqTimeUntilOkayToComplete(questString.c_str());

				if (timeTilOkay > 0)
				{
					int secs = timeTilOkay % 60;
					timeTilOkay /= 60;

					int mins = timeTilOkay % 60;
					timeTilOkay /= 60;

					int hours = timeTilOkay % 24;
					timeTilOkay /= 24;

					int days = timeTilOkay;

					player->SendText(csprintf("You cannot open this for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
				}

				return WERROR_CHEST_USED_TOO_RECENTLY;
			}
		}



		int resistLockpick = 0;
		BOOL canBePicked = with->m_Qualities.InqInt(RESIST_LOCKPICK_INT, resistLockpick);
		if (canBePicked && resistLockpick > 0)
		{
			SKILL_ADVANCEMENT_CLASS sac;
			if (!player->m_Qualities.InqSkillAdvancementClass(LOCKPICK_SKILL, sac) || (sac != TRAINED_SKILL_ADVANCEMENT_CLASS && sac != SPECIALIZED_SKILL_ADVANCEMENT_CLASS))
			{
				player->SendText("You must be trained in Lockpicking.", LTT_DEFAULT);
				return WERROR_DONT_KNOW_LOCKPICKING;
			}

			//Door / chest must be closed
			if (!isClosed(with)) {
				return WERROR_CHEST_ALREADY_OPEN;
			}

			uint32_t skillLevel = 0;
			if (player->InqSkill(LOCKPICK_SKILL, skillLevel, FALSE))
			{
				DecrementStackOrStructureNum();

				std::string lockpickText;
				int lockpickMod = InqIntQuality(LOCKPICK_MOD_INT, 0);
				if (GenericSkillCheck(skillLevel + lockpickMod, resistLockpick))
				{
					bool newLockState = !with->IsLocked();
					lockpickText += csprintf("You have successfully picked the lock!  It is now %s.\n", lockedString(newLockState));

					//Give a bit of XP for successful lockpicking, unless the lock was so low that the lockpick mod actually fixed it.
					if (resistLockpick > lockpickMod)
						player_weenie->MaybeGiveSkillUsageXP(LOCKPICK_SKILL, resistLockpick - lockpickMod);

					with->SetLocked(newLockState);
					with->EmitSound(Sound_LockSuccess, 1.0f);


					if (questChest && !newLockState) //Take ownership if transitioning from locked -> unlocked
					{
						player->StampQuest(questString.c_str());
						with->m_Qualities.SetInstanceID(OWNER_IID, player->GetID());
						with->_nextReset = Timer::cur_time + 300.0; //if we're the one to unlock the container we have 5 minutes to loot it before we reset the container.
					}
				}
				else
				{
					lockpickText += csprintf("You have failed to pick the lock.  It is still %s. ", lockedString(with->IsLocked()));
					with->EmitSound(Sound_PicklockFail, 1.0f);
				}

				int numUsesLeft = GetStructureNum();
				if (numUsesLeft > 0)
				{
					lockpickText += csprintf(" Your lockpicks have %d uses left.", numUsesLeft);
				}
				else if (numUsesLeft == 0)
				{
					lockpickText += " Your lockpicks are used up.";
				}

				player->SendText(lockpickText.c_str(), LTT_DEFAULT);
			}
			else //i.e. lockpick skill is zero (debuffed)
			{
				player->SendText("Your Lockpicking skill is too low for that.", LTT_DEFAULT);
			}
		}
		else {
			return WERROR_CHEST_NOT_LOCKABLE;
		}
	}

	return WERROR_NONE;
}
