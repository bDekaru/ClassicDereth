
#include <StdAfx.h>
#include "Corpse.h"
#include "DatabaseIO.h"
#include "WorldLandBlock.h"

#define CORPSE_EXIST_TIME 180

CCorpseWeenie::CCorpseWeenie()
{
	_begin_destroy_at = Timer::cur_time + CORPSE_EXIST_TIME;
}

CCorpseWeenie::~CCorpseWeenie()
{
}

void CCorpseWeenie::ApplyQualityOverrides()
{
	CContainerWeenie::ApplyQualityOverrides();
}

void CCorpseWeenie::SetObjDesc(const ObjDesc &desc)
{
	_objDesc = desc;
}

void CCorpseWeenie::GetObjDesc(ObjDesc &desc)
{
	desc = _objDesc;
}

int CCorpseWeenie::CheckOpenContainer(CWeenieObject *looter)
{
	int error = CContainerWeenie::CheckOpenContainer(looter);

	if (error != WERROR_NONE)
		return error;

	if (_begun_destroy)
		return WERROR_OBJECT_GONE;

	if (!_hasBeenOpened)
	{
		uint32_t killerId = InqIIDQuality(KILLER_IID, 0);
		uint32_t victimId = InqIIDQuality(VICTIM_IID, 0);
		if (killerId == looter->GetID() || victimId == looter->GetID())
			return WERROR_NONE;

		CPlayerWeenie *corpsePlayer = g_pWorld->FindPlayer(victimId);

		if (!corpsePlayer && _begin_destroy_at - (CORPSE_EXIST_TIME / 2) <= Timer::cur_time) // corpse isn't of a player so allow it to open after a certain time
		{
			return WERROR_NONE;
		}

		CPlayerWeenie *looterAsPlayer = looter->AsPlayer();
		bool killedByPK = m_Qualities.GetBool(PK_KILLER_BOOL, 0);

		if (corpsePlayer && !killedByPK && looterAsPlayer) // Make sure we're both players & don't let corpse permissions work on PK kills
		{
			if (!corpsePlayer->m_umCorpsePermissions.empty()) // if the corpse owner has players on their permissions list
			{
				if (!corpsePlayer->HasPermission(looterAsPlayer))
				{
					looter->SendText("You do not have permission to loot that corpse!", LTT_ERROR);
					return WERROR_FROZEN;
				}
				else
				{
					corpsePlayer->RemoveCorpsePermission(looterAsPlayer);
					looterAsPlayer->RemoveConsent(corpsePlayer);
					return WERROR_NONE;
				}
			}
		}

		fellowship_ptr_t fellowship = looter->GetFellowship();
		if (fellowship)
		{
			if (corpsePlayer != looterAsPlayer && m_Qualities.GetBool(CORPSE_GENERATED_RARE_BOOL, 0))
			{
				looter->SendText("You do not have permission to loot that corpse!", LTT_ERROR);
				return WERROR_FROZEN;
			}

			if (!killedByPK)
			{
				if (fellowship->_share_loot)
				{
					for (auto &entry : fellowship->_fellowship_table)
					{
						if (killerId == entry.first && entry.second._share_loot)
							return WERROR_NONE;
					}
				}
			}
		}

		if (corpsePlayer != looterAsPlayer)
		{
			looter->SendText("You do not have permission to loot that corpse!", LTT_ERROR);
			return WERROR_FROZEN;
		}

		if (corpsePlayer)
		{
			return WERROR_NONE;
		}

		looter->SendText("You do not have permission to loot that corpse!", LTT_ERROR);
		return WERROR_FROZEN;
	}

	return WERROR_NONE;
}

void CCorpseWeenie::OnContainerOpened(CWeenieObject *other)
{
	CContainerWeenie::OnContainerOpened(other);

	_hasBeenOpened = true;
}

void CCorpseWeenie::OnContainerClosed(CWeenieObject *other)
{
	CContainerWeenie::OnContainerClosed(other);

	if (!m_Items.size() && !m_Packs.size())
	{
		BeginGracefulDestroy();
	}
}

void CCorpseWeenie::SaveEx(class CWeenieSave &save)
{
	if (_shouldSave)
	{
		m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _begin_destroy_at - Timer::cur_time);
		CContainerWeenie::SaveEx(save);

		save.m_ObjDesc = _objDesc;

		g_pDBIO->AddOrUpdateWeenieToBlock(GetID(), m_Position.objcell_id >> 16);
	}
}

void CCorpseWeenie::RemoveEx()
{
	g_pDBIO->RemoveWeenieFromBlock(GetID());
	g_pDBIO->DeleteWeenie(GetID());
}

void CCorpseWeenie::LoadEx(class CWeenieSave &save)
{
	CContainerWeenie::LoadEx(save);

	_objDesc = save.m_ObjDesc;
	double dbTimeToRot = m_Qualities.GetFloat(TIME_TO_ROT_FLOAT, 0.0);
	_begin_destroy_at = Timer::cur_time + (dbTimeToRot == 0 ? 60 * 60 : dbTimeToRot);
	_shouldSave = true;
	m_bDontClear = true;

	InitPhysicsObj();
	//set velocity so that corpses are affected by gravity after server restarts.
	_phys_obj->set_velocity(Vector(0, 0, 1.0f), 0);
	MakeMovementManager(TRUE);
	MovementParameters params;
	params.autonomous = 0;
	last_move_was_autonomous = false;
	DoMotion(GetCommandID(17), &params, 0);
}

bool CCorpseWeenie::ShouldSave()
{
	return _shouldSave;
}

void CCorpseWeenie::Tick()
{
	CContainerWeenie::Tick();

	//if (!m_Items.size())
	//	BeginGracefulDestroy();

	if (!_openedById && ContainsDecayableItems() && _begin_destroy_at > Timer::cur_time + 60.0 * 5 * 50)
		BeginGracefulDestroy();

	if (_hasBeenOpened && !_openedById && ContainsDecayableItems())
		BeginGracefulDestroy();

	if (!_begun_destroy)
	{
		if (!_openedById && _begin_destroy_at <= Timer::cur_time)
		{
			BeginGracefulDestroy();
		}
	}
	else
	{
		if (_mark_for_destroy_at <= Timer::cur_time)
		{
			MarkForDestroy();
		}
	}
}

void CCorpseWeenie::BeginGracefulDestroy()
{
	if (_begun_destroy)
	{
		return;
	}

	EmitEffect(PS_Destroy, 1.0f);
	if (_shouldSave)
	{
		for (auto drop : m_Items)
		{
			drop->ReleaseFromAnyWeenieParent(false, true);
			drop->SetWeenieContainer(0);
			drop->SetWielderID(0);
			drop->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
			drop->_timeToRot = Timer::cur_time + 300.0;
			drop->_beganRot = false;
			drop->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, drop->_timeToRot);
			drop->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
			drop->unset_parent();
			drop->enter_world(&m_Position);

			if (!drop->m_Qualities.m_PositionStats)
				drop->m_Qualities.SetPosition(LOCATION_POSITION, m_Position);

			drop->SetPlacementFrame(0x65, FALSE);
			g_pWorld->InsertEntity(drop);
			drop->Movement_Teleport(GetPosition());
		}
	}

	_shouldSave = false; //we're on our way out, it's no longer necessary to save us to the database.
	RemoveEx(); // and in fact, delete entries in the db

	_mark_for_destroy_at = Timer::cur_time + 2.0;
	_begun_destroy = true;
}

bool CCorpseWeenie::ContainsDecayableItems()
{
	bool autoDecay = true;

	for( auto ii : m_Items)
	{
		if (ii->m_Qualities.m_WeenieType == Coin_WeenieType || ii->m_Qualities.m_WeenieType == Healer_WeenieType || ii->m_Qualities.m_WeenieType == SpellComponent_WeenieType)
		{
			continue;
		}
		else
		{
			autoDecay = false;
		}
	}

	return autoDecay;
}



