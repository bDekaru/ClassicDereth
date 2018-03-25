
#pragma once

#include "LandDefs.h"

typedef void (*TargettingCallback)();

class CameraManager;
class CEnvCell;
class GraphicsOptions;
class CellManager;
class CommandInterpreter;
class LScape;
class Ambient;
class CPhysics;
class CObjectMaint;

enum ObjectSelectStatus
{
	Invalid_OSS = 0x0,
	ObjectOnscreen_OSS = 0x1,
	ObjectOffscreen_OSS = 0x2,
	ObjectNotFound_OSS = 0x3,
	FORCE_ObjectSelectStatus_32_BIT = 0x7FFFFFFF,
};

// for now just a placeholder so we know what the class looks like
class SmartBox
{
public:
	SmartBox(void *in_queue);
	virtual ~SmartBox();

	int testMode;
	Position viewer;
	CObjCell *viewer_cell;
	unsigned int head_index;
	Position viewer_sought_position;
	CameraManager *camera_manager;
	CellManager *cell_manager;
	CPhysics *physics;
	CObjectMaint *m_pObjMaint;
	LScape *lscape;
	Ambient *ambient_sounds;
	CommandInterpreter *cmdinterp;
	int creature_mode;
	float m_fGameFOV;
	float m_fViewDistFOV;
	bool m_bUseViewDistance;
	float game_ambient_level;
	unsigned int game_ambient_color;
	int game_degrades_disabled;
	int hidden;
	int position_update_complete;
	int waiting_for_teleport;
	int has_been_teleported;
	void *in_queue; //NIList<NetBlob *> *in_queue;
	void *netblob_list; //NIList<NetBlob *> *netblob_list;
	void *position_and_movement_file; //_iobuf *position_and_movement_file;
	unsigned int player_id;
	CPhysicsObj *player;
	unsigned int target_object_id;
	void (__cdecl *target_callback)(unsigned int, ObjectSelectStatus, tagRECT *, const float);
	unsigned int num_cells;
	CEnvCell **cells;
	unsigned int num_objects;
	CPhysicsObj **objects;
	void(__cdecl *m_renderingCallback)();

	static CSphere viewer_sphere;
	static float s_fViewerLightIntensity;
	static float s_fViewerLightFalloff;
	static LIGHTINFO viewer_light;
};
