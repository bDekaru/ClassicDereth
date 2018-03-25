
#include "StdAfx.h"
#include "PhatSDK.h"
#include "SmartBox.h"

#if PHATSDK_USE_SMART_BOX

const float DEFAULT_GAME_BRIGHTNESS = 0.5f;
const float BASE_GAMMA_MULTIPLIER = 4.5f;

CSphere SmartBox::viewer_sphere;
float SmartBox::s_fViewerLightIntensity = DEFAULT_GAME_BRIGHTNESS * BASE_GAMMA_MULTIPLIER;
float SmartBox::s_fViewerLightFalloff = 10.0f;
LIGHTINFO SmartBox::viewer_light;

SmartBox::SmartBox(void *_in_queue)
{
	in_queue = _in_queue;

	testMode = 0;
	viewer_cell = 0;
	head_index = 16;
	camera_manager = 0;
	cell_manager = 0;
	physics = 0;
	m_pObjMaint = 0;
	lscape = 0;
	ambient_sounds = 0;
	cmdinterp = 0;
	creature_mode = 0;
	m_fGameFOV = 1.5707964f;
	m_fViewDistFOV = 0;
	m_bUseViewDistance = 0;
	game_ambient_level = 0;
	game_degrades_disabled = 0;
	hidden = 0;
	position_update_complete = 0;
	waiting_for_teleport = 0;
	has_been_teleported = 0;
	netblob_list = 0;
	position_and_movement_file = 0;
	player_id = 0;
	player = 0;
	target_object_id = 0;
	target_callback = 0;
	num_cells = 0;
	cells = 0;
	num_objects = 0;
	objects = 0;
	m_renderingCallback = 0;
	viewer_sphere = CSphere(Vector(0, 0, 0), 0.30000001f);

	// registers variable and stuff here

	viewer_light.type = LIGHTINFO::POINT_LIGHT;
	viewer_light.color.SetColor32(0xFFFFFFFF);
	viewer_light.intensity = s_fViewerLightIntensity;
	viewer_light.falloff = s_fViewerLightFalloff;
	viewer_light.cone_angle = 360.0;
}

SmartBox::~SmartBox()
{
}



#endif

