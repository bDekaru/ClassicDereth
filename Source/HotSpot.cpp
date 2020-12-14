
#include <StdAfx.h>
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
	
	if (cycleTime == 0.0)
		return;

	cycleTimeVariance = std::max(1.0 - std::min(cycleTimeVariance, 1.0), 0.0);
	cycleTimeVariance = Random::RollDice(cycleTimeVariance, 1.0f);

	m_fNextCycleTime = Timer::cur_time + (cycleTime * cycleTimeVariance);
}

void CHotSpotWeenie::Tick()
{
	if (m_fNextCycleTime <= Timer::cur_time)
	{
		DoCycle();
		SetNextCycleTime();
	}

	CWeenieObject::Tick();
}

int CHotSpotWeenie::DoCollision(const class ObjCollisionProfile &prof)
{
	if (prof._bitfield & Player_OCPB)
	{
		m_ContactedWeenies.insert(prof.id);
	}

	return 1;
}

void CHotSpotWeenie::DoCollisionEnd(uint32_t object_id)
{
	m_ContactedWeenies.erase(object_id);
}

void CHotSpotWeenie::DoCycle()
{
	for (std::set<uint32_t>::iterator i = m_ContactedWeenies.begin(); i != m_ContactedWeenies.end();)
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
	damageEvent.damageAfterMitigation = damageEvent.damageBeforeMitigation = (int)(m_Qualities.GetInt(DAMAGE_INT, 0) * (1.0 - (Random::GenFloat(0.0f, (float)m_Qualities.GetFloat(DAMAGE_VARIANCE_FLOAT, 0.0)))));
	damageEvent.target = other;
	damageEvent.source = this;

	TryToDealDamage(damageEvent);
}

