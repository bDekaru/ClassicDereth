
#include <StdAfx.h>
#include "SkyDesc.h"

SkyDesc::SkyDesc()
{
	present_day_group = 0;
	tick_size = 3.0;
	light_tick_size = 20.0;
}

SkyDesc::~SkyDesc()
{
	Destroy();
}

void SkyDesc::Destroy()
{
	for (int i = 0; i < day_groups.num_used; i++)
	{
		DayGroup *pGroup = day_groups.array_data[i];
		delete pGroup;
	}
	day_groups.num_used = 0;
}

DEFINE_PACK(SkyDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(SkyDesc)
{
	tick_size = pReader->Read<double>();
	light_tick_size = pReader->Read<double>();

	pReader->ReadAlign();

	uint32_t numDayGroups = pReader->ReadUInt32();
	day_groups.grow(numDayGroups);

	for (uint32_t i = 0; i < numDayGroups; i++)
	{
		DayGroup *newDay = new DayGroup();
		newDay->UnPack(pReader);
		day_groups.add(&newDay);
	}

	return true;
}

void SkyDesc::CalcPresentDayGroup()
{
	int day = 0;
	int days_per_year = 0;
	int current_year = 0;

	if (GameTime::current_game_time)
	{
		day = GameTime::current_game_time->current_day;
		days_per_year = GameTime::current_game_time->days_per_year;
		current_year = GameTime::current_game_time->current_year;
	}

	unsigned int dayGroup = (unsigned int)floor((double)(unsigned int)(1782775218 * (day + days_per_year * current_year) - 1967253934) * 2.3283064e-10 * (double)day_groups.num_used);

	if (dayGroup < day_groups.num_used)
		present_day_group = dayGroup;
	else
		present_day_group = 0;
}

int SkyDesc::GetSky(float time_of_day, SmartArray<CelestialPosition> *sky_pos)
{
	// UNFINISHED
	return 0;
}

DayGroup::DayGroup()
{
	chance_of_occur = 0;
}

DayGroup::~DayGroup()
{
	Destroy();
}

void DayGroup::Destroy()
{
	chance_of_occur = 0;
	for (int i = 0; i < sky_time.num_used; i++)
	{
		delete sky_time.array_data[i];
	}
	sky_time.num_used = 0;

	for (int i = 0; i < sky_objects.num_used; i++)
	{
		delete sky_objects.array_data[i];
	}
	sky_objects.num_used = 0;
}

DEFINE_PACK(DayGroup)
{
	UNFINISHED();
}

DEFINE_UNPACK(DayGroup)
{
	chance_of_occur = pReader->Read<float>();
	day_name = pReader->ReadString();

	pReader->ReadAlign();
	
	uint32_t numSkyObjs = pReader->Read<uint32_t>();
	sky_objects.grow(numSkyObjs);

	for (uint32_t i = 0; i < numSkyObjs; i++)
	{
		SkyObject *skyObject = new SkyObject();
		skyObject->UnPack(pReader);
		sky_objects.add(&skyObject);
	}

	uint32_t numTimeOfDay = pReader->Read<uint32_t>();
	sky_time.grow(numTimeOfDay);

	for (uint32_t i = 0; i < numTimeOfDay; i++)
	{
		SkyTimeOfDay *skyTime = new SkyTimeOfDay();
		skyTime->UnPack(pReader);

		for (uint32_t j = 0; j < skyTime->sky_obj_replace.num_used; j++)
		{
			skyTime->sky_obj_replace.array_data[j]->object =
				sky_objects.array_data[skyTime->sky_obj_replace.array_data[j]->object_index];
		}

		sky_time.add(&skyTime);
	}

	return true;
}

DEFINE_PACK(SkyObjectReplace)
{
	UNFINISHED();
}

DEFINE_UNPACK(SkyObjectReplace)
{
	object_index = pReader->Read<uint32_t>();
	gfx_obj_id = pReader->Read<uint32_t>();
	rotate = pReader->Read<float>();
	transparent = pReader->Read<float>();
	luminosity = pReader->Read<float>();
	max_bright = pReader->Read<float>();

	pReader->ReadAlign();
	return true;
}

void SkyObject::Destroy()
{
	// missing code to reset values here
}

DEFINE_PACK(SkyObject)
{
	UNFINISHED();
}

DEFINE_UNPACK(SkyObject)
{
	Destroy();

	begin_time = pReader->Read<float>();
	end_time = pReader->Read<float>();
	begin_angle = pReader->Read<float>();
	end_angle = pReader->Read<float>();
	tex_velocity.x = pReader->Read<float>();
	tex_velocity.y = pReader->Read<float>();
	tex_velocity.z = 0.0f;
	default_gfx_object = pReader->Read<uint32_t>();
	default_pes_object = pReader->Read<uint32_t>();
	properties = pReader->Read<uint32_t>();

	pReader->ReadAlign();
	return true;
}

SkyTimeOfDay::~SkyTimeOfDay() // custom
{
	// custom
	for (int i = 0; i < sky_obj_replace.num_used; i++)
	{
		delete sky_obj_replace.array_data[i];
	}
	sky_obj_replace.num_used = 0;
	// custom
}

DEFINE_PACK(SkyTimeOfDay)
{
	UNFINISHED();
}

DEFINE_UNPACK(SkyTimeOfDay)
{
	begin = pReader->Read<float>();
	dir_bright = pReader->Read<float>();
	dir_heading = pReader->Read<float>();
	dir_pitch = pReader->Read<float>();
	dir_color.color = pReader->Read<uint32_t>();
	amb_bright = pReader->Read<float>();
	amb_color.color = pReader->Read<uint32_t>();
	min_world_fog = pReader->Read<float>();
	max_world_fog = pReader->Read<float>();
	world_fog_color.color = pReader->Read<uint32_t>();
	world_fog = pReader->Read<int>();

	pReader->ReadAlign();

	uint32_t numSkyReplace = pReader->Read<uint32_t>();
	sky_obj_replace.grow(numSkyReplace);

	for (uint32_t i = 0; i < numSkyReplace; i++)
	{
		SkyObjectReplace *replace = new SkyObjectReplace();
		replace->UnPack(pReader);
		sky_obj_replace.add(&replace);
	}

	return true;
}