
#pragma once

#include "MathLib.h"
#include "SmartArray.h"

class CelestialPosition
{
public:
	DWORD gfx_id;
	DWORD pes_id;
	float heading;
	float rotation;
	Vector tex_velocity;
	float transparent;
	float luminosity;
	float max_bright;
	unsigned int properties;
};

class GameSky
{
public:
	GameSky();
	~GameSky();

	class CPhysicsObj *MakeObject(DWORD id, Vec2D &tex_velocity, int after, int weather);
	void UseTime(void);

	SmartArray<CelestialPosition> sky_obj_pos; // 0x00
	SmartArray<class CPhysicsObj *> sky_obj; // 0x0C
	SmartArray<DWORD> property_array; // 0x18
	class CEnvCell *before_sky_cell; // 0x24
	class CEnvCell *after_sky_cell; // 0x28
};