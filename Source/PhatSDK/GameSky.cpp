
#include <StdAfx.h>
#include "GameSky.h"
#include "RegionDesc.h"

GameSky::GameSky()
{
	before_sky_cell = new CEnvCell;
	after_sky_cell = new CEnvCell;
}

GameSky::~GameSky()
{
	if (before_sky_cell) 
		delete before_sky_cell; // should probably be ->release()
	if (after_sky_cell)
		delete after_sky_cell; // should probably be ->release()
}

void GameSky::UseTime(void)
{
	CRegionDesc::CalcDayGroup();

	UNFINISHED();

#if 0 // UNFINISHED
	double time = (GameTime::current_game_time ? GameTime::current_game_time->present_time_of_day : 0.0);

	if (CRegionDesc::GetSky(time, &sky_obj_pos))
	{
		CreateDeletePhysicsObjects();

		for (uint32_t i = 0; i < m_SkyDescs.num_used; i++)
		{
			CPhysicsObj *    object = m_Objs0C.array_data[i];
			SkyDesc *        skydesc = &m_SkyDescs.array_data[i];

			if (Object = m_Objs0C.array_data[i])
			{
				Frame drawframe;
				CalcDrawFrame(&drawframe, skydesc->m_DrawVec);

				Object->set_frame(&drawframe);

				// Update luminosity, diffusion, and translucency..
			}
		}
	}
#endif
}

CPhysicsObj *GameSky::MakeObject(uint32_t id, Vec2D &tex_velocity, int after, int weather)
{
#if INCLUDE_SKY_CODE
	if (!ModelID)
		return NULL;

	CPhysicsObj *Object;

	if (Object = CPhysicsObj::makeObject(ModelID, 0, 0))
	{
		if (TexVel.x != 0 || TexVel.y != 0)
		{
			// Missing code.
			// Object->SetTextureVelocity(TexVel);
		}

		Object->AddObjectToSingleCell(arg_8 ? m_EnvCell28 : m_EnvCell24);

		if (ImgTex::DoChunkification())
			UNFINISHED("Object->NotifySurfaceTiles()");
	}

	return Object;
#else
	return NULL;
#endif
}



















