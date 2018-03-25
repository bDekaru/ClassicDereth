
#include "StdAfx.h"
#include "HotSpot.h"
#include "World.h"

CHotSpotWeenie::CHotSpotWeenie()
{
}

CHotSpotWeenie::~CHotSpotWeenie()
{
}

void CHotSpotWeenie::ApplyQualityOverrides()
{
}

void CHotSpotWeenie::PostSpawn()
{
	CWeenieObject::PostSpawn();

	SetNextCycleTime();
}

void CHotSpotWeenie::SetNextCycleTime()
{
	double cycleTime = m_Qualities.GetFloat(HOTSPOT_CYCLE_TIME_FLOAT, 5.0);
	double cycleTimeVariance = m_Qualities.GetFloat(HOTSPOT_CYCLE_TIME_VARIANCE_FLOAT, 0.0);

	m_fNextCycleTime = Timer::cur_time + (cycleTime * (1.0 - cycleTimeVariance));
}

void CHotSpotWeenie::Tick()
{
	if (m_fNextCycleTime <= Timer::cur_time)
	{
		DoCycle();
		SetNextCycleTime();
	}

	if (_timeToRot >= 0 && _timeToRot <= Timer::cur_time)
	{
		if (!HasOwner())
		{
			if (_beganRot)
			{
				if ((_timeToRot + 2.0) <= Timer::cur_time)
				{
					MarkForDestroy();
				}
			}
			else
			{
				EmitEffect(PS_Destroy, 1.0f);
				_beganRot = true;
			}
		}
	}
}

int CHotSpotWeenie::DoCollision(const class ObjCollisionProfile &prof)
{
	if (prof._bitfield & Player_OCPB)
	{
		m_ContactedWeenies.insert(prof.id);
	}

	return 1;
}

void CHotSpotWeenie::DoCollisionEnd(DWORD object_id)
{
	m_ContactedWeenies.erase(object_id);
}

void CHotSpotWeenie::DoCycle()
{
	for (std::set<DWORD>::iterator i = m_ContactedWeenies.begin(); i != m_ContactedWeenies.end();)
	{
		CWeenieObject *other = g_pWorld->FindObject(*i);

		//if (other && check_collision(other))
		//{
			DoCycleDamage(other);
			i++;
		//}
		//else
		//{
		//	i = m_ContactedWeenies.erase(i);
		//}
	}
}

void CHotSpotWeenie::DoCycleDamage(CWeenieObject *other)
{
	DamageEventData damageEvent;
	damageEvent.damage_type = (DAMAGE_TYPE) m_Qualities.GetInt(DAMAGE_TYPE_INT, 0);
	damageEvent.damage_form = DAMAGE_FORM::DF_HOTSPOT;
	damageEvent.damageAfterMitigation = damageEvent.damageBeforeMitigation = (int)(m_Qualities.GetInt(DAMAGE_INT, 0) * (1.0 - (Random::GenFloat(0.0f, m_Qualities.GetFloat(DAMAGE_VARIANCE_FLOAT, 0.0)))));
	damageEvent.target = other;
	damageEvent.source = this;

	TryToDealDamage(damageEvent);
}

