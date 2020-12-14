#include <StdAfx.h>
#include "Key.h"
#include "UseManager.h"
#include "Player.h"
#include "Config.h"

CKeyWeenie::CKeyWeenie()
{
}

CKeyWeenie::~CKeyWeenie()
{
}

int CKeyWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 1.0;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CKeyWeenie::DoUseWithResponse(CWeenieObject *player, CWeenieObject *with)
{
	if (CPlayerWeenie *player_weenie = player->AsPlayer())
	{
		if (!with->IsLocked())
		{
			if (with->IsContainer())
			{
				return WERROR_CHEST_ALREADY_UNLOCKED;
			}
		}
		else
		{
			std::string lockCode;
			if ((with->m_Qualities.InqString(LOCK_CODE_STRING, lockCode) && !stricmp(lockCode.c_str(), InqStringQuality(KEY_CODE_STRING, "").c_str()) || InqBoolQuality(OPENS_ANY_LOCK_BOOL, FALSE)))
			{
				DecrementStackOrStructureNum();
				with->SetLocked(FALSE);
				with->EmitSound(Sound_LockSuccess, 1.0f);
				with->m_Qualities.SetInstanceID(LAST_UNLOCKER_IID, player->id);

				if (with->IsContainer() && with->_nextReset < 0)
				{
					// Unlocked chests have 10 minutes before the chest resets.
					with->_nextReset = Timer::cur_time + 600;
				}

				int structureNum = 0;
				if (m_Qualities.InqInt(STRUCTURE_INT, structureNum, TRUE) && structureNum > 0)
				{
					player->SendText(csprintf("The %s has been unlocked.\nYour key has %d %s left.", with->GetName().c_str(), structureNum, structureNum != 1 ? "uses" : "use"), LTT_DEFAULT);
				}
				else
				{
					player->SendText(csprintf("The %s has been unlocked.\nYour key is used up.", with->GetName().c_str()), LTT_DEFAULT);
				}

				return WERROR_NONE;
			}
			else
			{
				return WERROR_CHEST_WRONG_KEY;
			}
		}
	}

	return WERROR_NONE;
}