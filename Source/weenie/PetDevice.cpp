#include <StdAfx.h>

#include "MonsterAI.h"
#include "UseManager.h"
#include "WeenieFactory.h"
#include "World.h"
#include "WorldLandBlock.h"
#include "PetDevice.h"

PetDevice::PetDevice()
{
}

PetDevice::~PetDevice()
{
}

void PetDevice::LoadEx(class CWeenieSave &save)
{
	CWeenieObject::LoadEx(save);
}

int PetDevice::Use(CPlayerWeenie *player)
{
	if (!player)
		return WERROR_OBJECT_GONE;

	if (player->IsInPortalSpace())
	{
		player->NotifyUseDone(WERROR_ACTIONS_LOCKED);
		return WERROR_NONE;
	}

	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	int requiredSkill = m_Qualities.GetInt(USE_REQUIRES_SKILL_INT, 0);
	if (requiredSkill)
	{
		Skill skill;
		player->m_Qualities.InqSkill((STypeSkill)requiredSkill, skill);
		if (!(skill._sac >= TRAINED_SKILL_ADVANCEMENT_CLASS))
			return WERROR_SKILL_TOO_LOW;
		int reqSkillLevel = m_Qualities.GetInt(USE_REQUIRES_SKILL_LEVEL_INT, 0);
		uint32_t playerSkill;
		player->m_Qualities.InqSkill((STypeSkill)requiredSkill, playerSkill, false);

		if (reqSkillLevel == 570 && skill._sac != SPECIALIZED_SKILL_ADVANCEMENT_CLASS)
			return WERROR_ACTIVATION_NOT_SPECIALIZED;

		if(playerSkill < reqSkillLevel)
			return WERROR_SKILL_TOO_LOW;
	}

	int requiredLevel = m_Qualities.GetInt(USE_REQUIRES_LEVEL_INT, 0);
	if (requiredLevel)
	{
		if (player->m_Qualities.GetInt(LEVEL_INT, 0) < requiredLevel)
		{
			return WERROR_LEVEL_TOO_HIGH;
		}
	}

	int requiredMastery = m_Qualities.GetInt(SUMMONING_MASTERY_INT, 0);
	if (requiredMastery)
	{
		if (player->m_Qualities.GetInt(SUMMONING_MASTERY_INT, 0) != requiredMastery)
		{
			switch (requiredMastery)
			{
			case 1:
				player->SendText("You must have Primalist Mastery to use this item.", LTT_DEFAULT);
				return WERROR_CRAFT_NO_CHANCE;
				break;
			case 2:
				player->SendText("You must have Necromancer Mastery to use this item.", LTT_DEFAULT);
				return WERROR_CRAFT_NO_CHANCE;
				break;
			case 3:
				player->SendText("You must have Naturalist Mastery to use this item.", LTT_DEFAULT);
				return WERROR_CRAFT_NO_CHANCE;
				break;
			default:
				SERVER_ERROR << "Unknown summon mastery for wcid:" << m_Qualities.id;
				break;
			}
		}
	}

	uint32_t pet_id = player->InqIIDQuality(PET_IID, 0);
	CWeenieObject *pet = nullptr;
	
	// see if it exists and if it is ours
	if (pet_id)
		pet = g_pWorld->FindObject(pet_id);
	if (pet && pet->InqIIDQuality(PET_OWNER_IID, 0) == player->GetID())
	{
		if (InqBoolQuality(UNLIMITED_USE_BOOL, 0) || InqIntQuality(MAX_STRUCTURE_INT, 0) == 0)
		{
			// unlimited use, despawn pet
			DespawnPet(player, pet_id);
		}
		else
		{
			// limited use, error
			player->NotifyUseDone(WERROR_ALREADY_BEING_USED);
			return WERROR_NONE;
		}
	}
	else
	{
		// error if out of uses
		if (InqIntQuality(STRUCTURE_INT, 0, TRUE) <= 0 && !InqBoolQuality(UNLIMITED_USE_BOOL, 0))
		{
			player->NotifyUseDone(WERROR_MAGIC_GENERAL_FAILURE);
			return WERROR_NONE;
		}

		SpawnPet(player);
	}

	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int PetDevice::DoUseResponse(CWeenieObject *player)
{
	DecrementStackOrStructureNum(1, false);
	return CWeenieObject::DoUseResponse(player);
}

void PetDevice::InventoryTick()
{
	CWeenieObject::InventoryTick();
}

void PetDevice::Tick()
{
	CWeenieObject::Tick();
}

void PetDevice::SpawnPet(CPlayerWeenie *player)
{
	uint32_t pet_wcid = (uint32_t)InqIntQuality(PET_CLASS_INT, 0, TRUE);
	if (pet_wcid)
	{
		CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(pet_wcid, nullptr, false);
		if(!weenie)
			return;
		// Move outside collision sphere?
		Position pos = player->GetPosition();
		float heading = pos.frame.m_angles.w; // *0.0174532925;
		/*pos.frame.m_origin.x += cos(heading);
		pos.frame.m_origin.y += sin(heading);
		*///CSphere *sp = player->GetSphere();
		//CSphere *sw = weenie->GetSphere();
		//float radius = sp->radius + sw->radius;
		weenie->SetInitialPosition(pos);

		// copy / setup props
		weenie->m_Qualities.SetInstanceID(PET_OWNER_IID, player->GetID());
		weenie->m_Qualities.SetInstanceID(PET_DEVICE_IID, GetID());

		weenie->SetName(csprintf("%s's %s", player->GetName().c_str(), weenie->GetName().c_str()));

		weenie->m_Qualities.SetInt(DAMAGE_RATING_INT, m_Qualities.GetInt(GEAR_DAMAGE_INT, 0));
		weenie->m_Qualities.SetInt(DAMAGE_RESIST_RATING_INT, m_Qualities.GetInt(GEAR_DAMAGE_RESIST_INT, 0));
		weenie->m_Qualities.SetInt(CRIT_RATING_INT, m_Qualities.GetInt(GEAR_CRIT_INT, 0));
		weenie->m_Qualities.SetInt(CRIT_RESIST_RATING_INT, m_Qualities.GetInt(GEAR_CRIT_RESIST_INT, 0));
		weenie->m_Qualities.SetInt(CRIT_DAMAGE_RATING_INT, m_Qualities.GetInt(GEAR_CRIT_DAMAGE_INT, 0));
		weenie->m_Qualities.SetInt(CRIT_DAMAGE_RESIST_RATING_INT, m_Qualities.GetInt(GEAR_CRIT_DAMAGE_RESIST_INT, 0));
		weenie->m_Qualities.SetInt(REMAINING_LIFESPAN_INT, weenie->m_Qualities.GetInt(LIFESPAN_INT, 0));

		double lifespan = 0;
		if (lifespan = InqFloatQuality(COOLDOWN_DURATION_FLOAT, 0, TRUE) && lifespan > 0)
		{
			weenie->_timeToRot = Timer::cur_time + lifespan;
		}

		if (g_pWorld->CreateEntity(weenie))
		{
			player->m_Qualities.SetInstanceID(PET_IID, weenie->GetID());
		}
		if (player->CombatPetsShowCombatDamage())
			weenie->AsMonster()->SetDisplayCombatDamage(true);
	}
}

void PetDevice::DespawnPet(CPlayerWeenie *player, uint32_t petId)
{
	CWeenieObject *pet = g_pWorld->FindObject(petId, false, false);

	if (pet)
	{
		// which?
		pet->Remove();
		//MarkForDestroy();
	}
	if (player)
		player->m_Qualities.SetInstanceID(PET_IID, 0);
}

///////////////////////////////////////////////////////////////////////////////
// 
//
PetPassive::PetPassive()
{

}

PetPassive::~PetPassive()
{

}

void PetPassive::PostSpawn()
{
	CMonsterWeenie::PostSpawn();
	if (!m_MonsterAI)
		m_MonsterAI = new MonsterAIManager(this, m_Position);
}

void PetPassive::Tick()
{
	CMonsterWeenie::Tick();

	uint32_t oid = InqIIDQuality(PET_OWNER_IID, 0);
	CWeenieObject *owner = nullptr;

	if (oid)
		owner = g_pWorld->FindPlayer(oid);

	if (!owner)
	{
		// no master
		Remove();
		return;
	}

	CSphere *sm = this->GetSphere();
	CSphere *so = owner->GetSphere();
	float radius = sm->radius + so->radius;
	// TODO: Better way of handling follow
	float distance = DistanceTo(owner);
	if (distance - radius > .25f)
	{
		m_MonsterAI->SetHomePosition(owner->m_Position);
		m_MonsterAI->SwitchState(ReturningToSpawn);
	}

}

void PetPassive::OnDeath(uint32_t killer_id)
{
	CWeenieObject::OnDeath(killer_id);

	MakeMovementManager(TRUE);
	StopCompletely(0);

	uint32_t oid = InqIIDQuality(PET_OWNER_IID, 0);
	CWeenieObject *owner = nullptr;

	if (oid)
		owner = g_pWorld->FindPlayer(oid);

	if (owner)
		owner->m_Qualities.SetInstanceID(PET_IID, 0);

	MarkForDestroy();
}

///////////////////////////////////////////////////////////////////////////////
//
//
PetCombat::PetCombat()
{
}

PetCombat::~PetCombat()
{
}

bool PetCombat::CanTarget(CWeenieObject* target)
{
	if (target == nullptr)
		return false;

	if (target->AsPlayer())
		return false;

	return target->m_Qualities.m_WeenieType == WeenieType::Creature_WeenieType;
}

void PetCombat::Tick()
{
	CMonsterWeenie::Tick();

	uint32_t oid = InqIIDQuality(PET_OWNER_IID, 0);
	uint32_t did = InqIIDQuality(PET_DEVICE_IID, 0);

	CWeenieObject *owner = nullptr;
	CWeenieObject *device = nullptr;

	if (oid)
		owner = g_pWorld->FindPlayer(oid);

	if (did)
		device = g_pWorld->FindObject(did, false, false);

	if (!owner)
	{
		// no master
		Remove();
		return;
	}

	// Check lifespan

	// Ensure attacking

	CWeenieObject *target = m_MonsterAI->GetTargetWeenie();
	if (!target || target->IsDead())
	{
		// clear the target
		target = nullptr;

		// ?? COMBAT_PET_RANGE_INT
		float range = (float)InqFloatQuality(VISUAL_AWARENESS_RANGE_FLOAT, 1.0, TRUE);
		std::list<CWeenieObject*> targets;
		g_pWorld->EnumNearby(this, range, &targets);

		float closest = FLT_MAX;
		for (auto weenie : targets)
		{
			float distance = fabs(DistanceTo(weenie));
			if (CanTarget(weenie) && distance < closest && weenie->GetID() != oid)
			{
				closest = distance;
				target = weenie;
				break;
			}
		}

		// if new target found
		if (target)
			m_MonsterAI->SetNewTarget(target);
	}
}
