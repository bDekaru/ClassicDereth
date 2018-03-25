
#include "StdAfx.h"
#include "Corpse.h"
#include "DatabaseIO.h"
#include "WorldLandBlock.h"

CCorpseWeenie::CCorpseWeenie()
{
	_begin_destroy_at = Timer::cur_time + (60.0 * 3);
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

int CCorpseWeenie::CheckOpenContainer(CWeenieObject *other)
{
	int error = CContainerWeenie::CheckOpenContainer(other);

	if (error != WERROR_NONE)
		return error;

	if (_begun_destroy)
		return WERROR_OBJECT_GONE;

	if (!_hasBeenOpened)
	{
		DWORD killerId = InqIIDQuality(KILLER_IID, 0);
		DWORD victimId = InqIIDQuality(VICTIM_IID, 0);
		if (killerId == other->GetID() || victimId == other->GetID())
			return WERROR_NONE;

		if (Fellowship *fellowship = other->GetFellowship())
		{
			if (fellowship->_share_loot)
			{
				for (auto &entry : fellowship->_fellowship_table)
				{
					if (killerId == entry.first)
						return WERROR_NONE;
				}

				for (auto &entry : fellowship->_fellows_departed)
				{
					if (killerId == entry.first)
						return WERROR_NONE;
				}
			}
		}
	}
	else
		return WERROR_NONE;

	return WERROR_CHEST_WRONG_KEY;
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
	m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _begin_destroy_at - Timer::cur_time);
	CContainerWeenie::SaveEx(save);

	save.m_ObjDesc = _objDesc;

	g_pDBIO->AddOrUpdateWeenieToBlock(GetID(), m_Position.objcell_id >> 16);
}

void CCorpseWeenie::LoadEx(class CWeenieSave &save)
{
	CContainerWeenie::LoadEx(save);

	_objDesc = save.m_ObjDesc;
	_begin_destroy_at = Timer::cur_time + m_Qualities.GetFloat(TIME_TO_ROT_FLOAT, 0.0);
	_shouldSave = true;
	m_bDontClear = true;

	InitPhysicsObj();
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

	// TODO drop inventory items on the ground

	_shouldSave = false; //we're on our way out, it's no longer necessary to save us to the database.

	_mark_for_destroy_at = Timer::cur_time + 2.0;
	_begun_destroy = true;
}


