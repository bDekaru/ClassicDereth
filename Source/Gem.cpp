
#include "StdAfx.h"
#include "Gem.h"
#include "UseManager.h"
#include "Player.h"
#include "SpellcastingManager.h"

CGemWeenie::CGemWeenie()
{
}

CGemWeenie::~CGemWeenie()
{
}

int CGemWeenie::Use(CPlayerWeenie *player)
{
	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CGemWeenie::DoUseResponse(CWeenieObject *player)
{
	if (DWORD spell_did = InqDIDQuality(SPELL_DID, 0))
	{
		MakeSpellcastingManager()->CastSpellInstant(player->GetID(), spell_did);
	}

	DecrementStackOrStructureNum();

	return WERROR_NONE;
}