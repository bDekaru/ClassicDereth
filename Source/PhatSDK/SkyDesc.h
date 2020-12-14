
#pragma once

#include "Packable.h"
#include "SmartArray.h"
#include "Palette.h"

struct SkyObject : public PackObj
{
public:
	void Destroy();
	DECLARE_PACKABLE();

	std::string object_name;
	float begin_time = 0.0f;
	float end_time = 0.0f;
	float begin_angle = 0.0f;
	float end_angle = 0.0f;
	Vector tex_velocity;
	unsigned int properties = 0;
	uint32_t default_gfx_object = 0;
	uint32_t default_pes_object = 0;
};

class SkyObjectReplace : public PackObj
{
public:
	SkyObjectReplace() { }
	virtual ~SkyObjectReplace() { }

	DECLARE_PACKABLE();

	unsigned int object_index = 0;
	SkyObject *object = NULL;
	uint32_t gfx_obj_id = 0;
	float rotate = 0.0f;
	float transparent = -1.0f;
	float luminosity = 0.0f;
	float max_bright = 0.0f;
};

struct SkyTimeOfDay : public PackObj
{
public:
	~SkyTimeOfDay(); // custom

	DECLARE_PACKABLE();

	float begin = 0.0f;
	float dir_bright = 0.0f;
	float dir_heading = 0.0f;
	float dir_pitch = 0.0f;
	RGBAUnion dir_color;
	float amb_bright = 0.0f;
	RGBAUnion amb_color;
	int world_fog = 0;
	float min_world_fog = 0.0f;
	float max_world_fog = 0.0f;
	RGBAUnion world_fog_color;
	SmartArray<SkyObjectReplace *> sky_obj_replace;
};

struct DayGroup : public PackObj
{
public:
	DayGroup();
	virtual ~DayGroup();

	void Destroy();
	DECLARE_PACKABLE();

	std::string day_name;
	float chance_of_occur;
	SmartArray<SkyTimeOfDay *> sky_time;
	SmartArray<SkyObject *> sky_objects;
};

class SkyDesc : public PackObj
{
public:
	SkyDesc();
	virtual ~SkyDesc();

	void Destroy();
	DECLARE_PACKABLE();
	
	void CalcPresentDayGroup();
	int GetSky(float time_of_day, SmartArray<CelestialPosition> *sky_pos);

	unsigned int present_day_group;
	double tick_size;
	double light_tick_size;
	SmartArray<DayGroup *> day_groups;
};

