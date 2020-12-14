
#pragma once

#include "ObjCache.h"
#include "MathLib.h"
#include "DLListBase.h"
#include "Frame.h"

class Position;
class CObjCell;
class CPhysicsPart;
// class CPhysicsObj;
class CSetup;
class CSingleMush;
class AnimSequenceNode;
class AnimFrame;
class AnimData;
class Palette;
class MotionTableManager;
class MovementParameters;
class LIGHTLIST;

class CSequence : public LegacyPackObj
{
public:
	CSequence();
	~CSequence();

	void clear();
	void clear_animations();
	void clear_physics();

	void set_object(CPhysicsObj *pPhysicsObj);
	void set_placement_frame(AnimFrame *PlacementFrame, uint32_t ID);
	void set_velocity(const Vector& Velocity);
	void set_omega(const Vector& Omega);

	void combine_physics(const Vector& Velocity, const Vector& Omega);
	void subtract_physics(const Vector& Velocity, const Vector& Omega);

	AnimFrame * get_curr_animframe();
	int32_t get_curr_frame_number();

	void multiply_cyclic_animation_framerate(float fRate);

	void remove_cyclic_anims();

	void remove_link_animations(uint32_t amount);
	void append_animation(AnimData *pAnimData);
	void apply_physics(Frame *pframe, double quantum, double sign);
	void update(double time_elapsed, Frame *pframe);
	void update_internal(double time_elapsed, AnimSequenceNode **ppanim, double *pframenum, Frame *pframe);

	void apricot();
	void execute_hooks(AnimFrame *pAnimFrame, int dir);
	void advance_to_next_animation(double TimeElapsed, AnimSequenceNode **ppAnim, double *pFrameNum, Frame *pFrame);

	void remove_all_link_animations();
	BOOL has_anims();

	DLListBase anim_list; // 0x04
	AnimSequenceNode* first_cyclic; // 0x0C
	Vector velocity; // 0x10
	Vector omega; // 0x1C
	CPhysicsObj *hook_obj; // 0x28
	double frame_number; // 0x30
	AnimSequenceNode *curr_anim; // 0x38
	AnimFrame *placement_frame; // 0x3C
	int placement_frame_id; // 0x40
	int bIsTrivial;
};

class CPartArray
{
public:

	CPartArray();
	~CPartArray();

	static CPartArray *CreateMesh(CPhysicsObj *pPhysicsObj, uint32_t ID);
	static CPartArray *CreateSetup(CPhysicsObj *_owner, uint32_t setup_did, BOOL bCreateParts);
	static CPartArray *CreateParticle(CPhysicsObj *_owner, uint32_t _num_parts, CSphere *sorting_sphere);

	void Destroy();
	void DestroySetup();
	void DestroyParts();
	void DestroyPals();
	void DestroyLights();
	BOOL InitParts();
	BOOL InitLights();
	void InitDefaults();

	void AddLightsToCell(CObjCell *pCell);
	void AnimationDone(BOOL success);
	BOOL CacheHasPhysicsBSP();
	void CheckForCompletedMotions();
	uint32_t DoInterpretedMotion(uint32_t mid, MovementParameters *params);
	void HandleEnterWorld();
	void HandleExitWorld();
	void InitializeMotionTables();
	void RemoveLightsFromCell(CObjCell *pCell);
	void SetCellID(uint32_t ID);
	BOOL SetMeshID(uint32_t ID);
	BOOL SetSetupID(uint32_t ID, BOOL bCreateParts);
	BOOL SetPlacementFrame(uint32_t ID);
	void SetFrame(Frame *pFrame);
	void SetNoDrawInternal(BOOL NoDraw);
	void SetTranslucencyInternal(float Amount);
	uint32_t StopCompletely_Internal();
	uint32_t StopInterpretedMotion(uint32_t mid, MovementParameters *params);
	void UpdateParts(Frame *pFrame);
	void Update(float fTimeElapsed, Frame *pFrame);
	void UpdateViewerDistance();
	BOOL HasAnims();
	BOOL SetScaleInternal(Vector &new_scale);

	int GetPlacementFrameID(); // custom
	int GetActivePlacementFrameID(); // custom

	BOOL AllowsFreeHeading();
	uint32_t GetDataID();
	float GetHeight() const;
	float GetRadius() const;
	void HandleMovement();

	void Draw(Position *Pos);

	uint32_t GetNumSphere();
	class CSphere *GetSphere();
	uint32_t GetNumCylsphere();
	class CCylSphere *GetCylsphere();
	CSphere *GetSortingSphere();
	
	TransitionState FindObjCollisions(class CTransition *transition);

	uint32_t GetSetupID();

	uint32_t GetMotionTableID();
	BOOL SetMotionTableID(uint32_t ID);

	float GetStepDownHeight();
	float GetStepUpHeight();

	void AddPartsShadow(CObjCell *obj_cell, unsigned int num_shadow_parts);
	void RemoveParts(CObjCell *obj_cell);
	void calc_cross_cells_static(CObjCell *cell, struct CELLARRAY *cell_array);

	uint32_t pa_state; // 0x00
	CPhysicsObj* owner; // 0x04
	CSequence sequence; // 0x08 -- size 0x48
	MotionTableManager * motion_table_manager; // 0x50
	CSetup* setup; // 0x54
	uint32_t num_parts; // 0x58
	CPhysicsPart** parts; // 0x5C
	Vector scale; // 0x60
	Palette** pals; // 0x6C
	LIGHTLIST* lights; // 0x70
	class AnimFrame * last_animframe; // 0x74
};

class CPartGroup : public CPartArray
{
	// Finish me
};

// Light information is obtained from CSetup
class LIGHTINFO;

class LIGHTOBJ // size 0x48
{
public:
	LIGHTOBJ();
	~LIGHTOBJ();

	LIGHTINFO *lightinfo; // 0x00
	Frame global_offset; // 0x04
	int state; // 0x44
};

class LIGHTLIST // size 0x8
{
public:
	LIGHTLIST(uint32_t LightCount);
	~LIGHTLIST();

	void set_frame(Frame *pFrame);

	uint32_t num_lights;
	LIGHTOBJ *m_Lights;
};




