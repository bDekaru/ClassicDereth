
#pragma once

#include "HashData.h"
#include "DArray.h"
#include "ObjCell.h"
#include "Frame.h"
#include "SmartArray.h"
#include "Movement.h"
#include "MovementManager.h"

#if PHATSDK_IS_SERVER
#include "ServerObjectMaint.h"
#endif

#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC
#include "PhysicsDesc.h"
#endif

enum SetPositionError
{
	OK_SPE = 0x0,
	GENERAL_FAILURE_SPE = 0x1,
	NO_VALID_POSITION_SPE = 0x2,
	NO_CELL_SPE = 0x3,
	COLLIDED_SPE = 0x4,
	INVALID_ARGUMENTS = 0x100,
	FORCE_SetPositionError_32_BIT = 0x7FFFFFFF,
};

enum SetPositionFlag
{
	PLACEMENT_SPF = 0x1, // bit 0
	TELEPORT_SPF = 0x2, // bit 1
	RESTORE_SPF = 0x4, // bit 2
	SLIDE_SPF = 0x10, // bit 4
	DONOTCREATECELLS_SPF = 0x20, // bit 5
	SCATTER_SPF = 0x100,
	RANDOMSCATTER_SPF = 0x200,
	LINE_SPF = 0x400,
	SEND_POSITION_EVENT_SPF = 0x1000,
	FORCE_SetPositionFlag_32_BIT = 0x7FFFFFFF,
};

struct SetPositionStruct
{
	void SetPosition(Position *new_pos) { pos = *new_pos; }
	void SetFlags(uint32_t _flags) { flags = _flags; }

	Position pos;
	unsigned int flags = 0;
	Vector line;
	float xrad = 0;
	float yrad = 0;
	unsigned int num_tries = 0;
};

class PhysicsObjHook
{
public:
#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC
	typedef PhysicsDesc::PhysicsDescInfo HookType;
#else
	typedef int HookType;
#endif
	PhysicsObjHook() { UNFINISHED(); }
	virtual ~PhysicsObjHook() { UNFINISHED(); }

	HookType hook_type;
	double time_created;
	double interpolation_time;
	PhysicsObjHook *prev;
	PhysicsObjHook *next;
	void *user_data;
};

class AtkCollisionProfile
{
public:
	int part = -1;
	unsigned int id = 0;
	unsigned int location = 0;
};

class ObjCollisionProfile
{
public:
	void SetMissile(const int isMissile);
	void SetInContact(const int hasContact);
	void SetMeInContact(const int hasContact);
	void SetCreature(const int isCreature);
	void SetPlayer(const int isPlayer);
	void SetAttackable(const int isAttackable);
	void SetDoor(const int isDoor);
	bool IsDoor() const;

	unsigned int id = 0;
	Vector velocity;
	uint32_t wcid = 0;
	ITEM_TYPE itemType = TYPE_UNDEF;
	uint32_t _bitfield = Undef_OCPB;
};

class CPhysicsObj : public LongHashData
{
public:
	CPhysicsObj();
	virtual ~CPhysicsObj();

	void Destroy();

#if !PHATSDK_IS_SERVER
	static class CObjectMaint *obj_maint;
#else
	static class CServerObjectMaint *obj_maint;
#endif

	static CPhysicsObj *GetObject(uint32_t object_id);
	static CPhysicsObj *makeObject(uint32_t data_did, uint32_t object_iid, BOOL bDynamic);
	static CPhysicsObj *makeParticleObject(unsigned int num_parts, CSphere *sorting_sphere);

	void SetScaleStatic(float new_scale);
	Vector get_local_physics_velocity();
	BOOL set_active(BOOL active);
	void clear_target();
	void unstick_from_object();
	void stick_to_object(uint32_t target);
	void cancel_moveto();
	void RemoveLinkAnimations();
	float get_heading();
	void set_heading(float degrees, int send_event);
	void set_frame(class Frame &frame);
	Vector get_velocity();
	double get_target_quantum();
	void set_target_quantum(double new_quantum);
	void add_voyeur(uint32_t object_id, float radius, float quantum);
	int remove_voyeur(uint32_t object_id);
	void receive_detection_update(class DetectionInfo *info);
	void receive_target_update(class TargetInfo *info);
	void HandleUpdateTarget(TargetInfo target_info);
	void MoveToObject(uint32_t object_id, MovementParameters *params);
	void MoveToObject_Internal(uint32_t object_id, uint32_t top_level_id, float object_radius, float object_height, MovementParameters *params);
	uint32_t get_sticky_object_id();
	double GetAutonomyBlipDistance();

	void MakePositionManager();
	void InterpolateTo(Position *p, int keep_heading);

	void set_target(unsigned int context_id, unsigned int object_id, float radius, double quantum);

	BOOL IsInterpolating();
	BOOL motions_pending();
	uint32_t StopInterpretedMotion(uint32_t motion, class MovementParameters *params);
	uint32_t DoInterpretedMotion(uint32_t motion, class MovementParameters *params);
	BOOL movement_is_autonomous();
	void TurnToHeading(MovementParameters *params);
	BOOL HasAnims();

	class PositionManager *get_position_manager();

	class MovementManager *get_movement_manager(BOOL make = TRUE); // custom
	class CMotionInterp *get_minterp();
	void InitializeMotionTables();
	void StopCompletely(int send_event);

	void set_local_velocity(const Vector &new_velocity, int send_event);
	void set_on_walkable(BOOL is_on_walkable);

	void StopCompletely_Internal();
	void CheckForCompletedMotions();
	BOOL IsFullyConstrained();

	void UpdateChildrenInternal();
	void UpdateChild(CPhysicsObj *child_obj, unsigned int part_index, Frame *child_frame);
	void DrawRecursive();
	void UpdateViewerDistance();
	void set_sequence_animation(uint32_t AnimationID, BOOL ClearAnimations, int32_t StartFrame, float FrameRate);

	void MakeMovementManager(BOOL init_motion);

	void exit_world();
	void leave_world();
	BOOL is_completely_visible();
	int ethereal_check_for_collisions();
	int set_ethereal(int ethereal, int send_event);
	void calc_friction(float quantum, float velocity_mag2);
	void UpdatePhysicsInternal(float quantum, Frame &offset_frame);
	void UpdatePositionInternal(float quantum, Frame &o_newFrame);
	void UpdateObjectInternal(float quantum);
	void update_object();
	void animate_static_object();
	void set_nodraw(BOOL NoDraw, BOOL Unused);
	void Hook_AnimDone();
	void calc_cross_cells_static();
	void add_obj_to_cell(CObjCell *pCell, Frame *pFrame);
	void MotionDone(uint32_t motion, BOOL success);
	void add_anim_hook(class CAnimHook *pHook);
	void InitDefaults(class CSetup *pSetup);
	int report_object_collision_end(const unsigned int object_id);
	void report_collision_end(const int force_end);
	void remove_shadows_from_cells();
	void leave_cell(BOOL is_changing_cell);
	void set_cell_id(uint32_t CellID);
	void calc_acceleration();
	void enter_cell(CObjCell *pCell);
	void set_initial_frame(Frame *Pos);
	uint32_t create_particle_emitter(uint32_t emitter_info_id, unsigned int part_index, Frame *offset, unsigned int emitter_id);
	void remove_parts(CObjCell *obj_cell);
	int build_collision_profile(ObjCollisionProfile *prof, CPhysicsObj *obj, Vector *vel, const int amIInContact, const int objIsMissile, const int objHasContact) const;
	int report_object_collision(CPhysicsObj *object, int prev_has_contact);

	BOOL InitObjectBegin(uint32_t object_iid, BOOL bDynamic);
	BOOL InitPartArrayObject(uint32_t data_did, BOOL bCreateParts);
	BOOL CacheHasPhysicsBSP();
	void SetTranslucencyInternal(float Amount);

	BOOL InitObjectEnd();
	BOOL SetPlacementFrameInternal(uint32_t frame_id);
	BOOL SetPlacementFrame(uint32_t frame_id, BOOL send_event);
	BOOL play_script_internal(uint32_t ScriptID);

	uint32_t SetMotionTableID(uint32_t ID);
	void set_stable_id(uint32_t ID);
	void set_phstable_id(uint32_t ID);

	BOOL makeAnimObject(uint32_t setup_id, BOOL bCreateParts);

	void UpdatePartsInternal();
	void process_hooks();

	int check_collision(CPhysicsObj *);
	int get_object_info(class CTransition *transit, int admin_move);
	static BOOL is_valid_walkable(Vector *normal);

	float GetHeight() const;
	float GetRadius() const;

	uint32_t GetNumSphere(); // Inlined
	class CSphere *GetSphere(); // inlined

	uint32_t GetNumCylsphere(); // Inlined
	class CCylSphere *GetCylsphere(); // inlined

	float GetStepDownHeight();
	float GetStepUpHeight();

	int check_contact(int contact);

	TransitionState FindObjCollisions(CTransition *transition);

	void ConstrainTo(Position *p, float start_distance, float max_distance);
	void UnConstrain();

	float get_walkable_z();
	void set_velocity(const Vector &new_velocity, int send_event);
	CTransition *transition(Position *old_pos, Position *new_pos, int admin_move);
	uint32_t DoMotion(uint32_t motion, MovementParameters *params, int send_event);

	uint32_t GetSetupID();
	void SetNoDraw(int no_draw);
	void unset_parent();
	void unparent_children();
	void clear_transient_states();
	BOOL set_parent(CPhysicsObj *obj, unsigned int part_index, Frame *frame);
	BOOL set_parent(CPhysicsObj *obj, uint32_t placement);
	void recalc_cross_cells();
	BOOL add_child(CPhysicsObj *obj, uint32_t location_id);
	BOOL add_child(CPhysicsObj *obj, unsigned int part_index, Frame *frame);
	void RemovePartFromShadowCells(CPhysicsPart *part);
	void AddPartToShadowCells(CPhysicsPart *part);

	void prepare_to_enter_world();
	int prepare_to_leave_visibility();
	void leave_visibility();
	int CheckPositionInternal(CObjCell *new_cell, Position *new_pos, CTransition *transit, const SetPositionStruct &sps);
	int track_object_collision(const CPhysicsObj *object, int prev_has_contact);
	int play_default_script();
	int report_environment_collision(int prev_has_contact);
	int handle_all_collisions(COLLISIONINFO *collisions, int prev_has_contact, int prev_on_walkable);
	int SetPositionInternal(CTransition *transit);
	SetPositionError SetPositionSimple(Position *p, int sliding);
	SetPositionError SetScatterPositionInternal(const SetPositionStruct &sps, CTransition *transit);
	SetPositionError SetPositionInternal(Position *p, const SetPositionStruct &sps, CTransition *transit);
	SetPositionError SetPositionInternal(const SetPositionStruct &sps, CTransition *transit);
	SetPositionError SetPosition(const SetPositionStruct &sps);
	void store_position(Position *position);
	BOOL enter_world(Position *position);
	BOOL enter_world(BOOL slide);
	SetPositionError ForceIntoCell(CObjCell *_pNewCell, Position *_pos);
	void change_cell(CObjCell *new_cell);
	void calc_cross_cells();
	void add_shadows_to_cells(struct CELLARRAY *cell_array);
	void add_particle_shadow_to_cell();
	void find_bbox_cell_list(CELLARRAY *cell_array);
	BOOL set_state(uint32_t new_state, BOOL send_event);
	void set_hidden(BOOL hidden, BOOL send_event);
	static BOOL AdjustPosition(Position *p, Vector *low_pt, CObjCell **new_cell, int bDontCreateCells, int bSearchCells);

	void TurnToObject(uint32_t object_id, MovementParameters *params);
	void TurnToObject_Internal(uint32_t object_id, uint32_t top_level_id, MovementParameters *params);

	uint32_t PerformMovement(MovementStruct &mvs); // not seen in client but seems logical it should exist

	struct CollisionRecord
	{
		double touched_time;
		int ethereal;
	};

	// These should probably be defined elsewhere
	static const float DEFAULT_FRICTION;
	static const float DEFAULT_TRANSLUCENCY;
	static const float DEFAULT_ELASTICITY;
	static const float DEFAULT_MASS;

	// class NIList<NetBlob *> * netblob_list; // 0x0C
	class CPartArray *part_array = NULL; // 0x10
	Vector player_vector; // 0x14
	float player_distance = 0.0f; // 0x20
	float CYpt = 0.0f; // 0x24
	class CSoundTable *sound_table = NULL; // 0x28
	bool m_bExaminationObject = false; // 0x2C
	class ScriptManager *script_manager = NULL; // 0x30
	class PhysicsScriptTable *physics_script_table = NULL; // 0x34
	uint32_t m_DefaultScript = 0; // 0x38
	float m_DefaultScriptIntensity = 0.0f; // 0x3C
	CPhysicsObj *parent = NULL; // 0x40
	class CHILDLIST *children = NULL; // 0x44
	Position m_Position; // 0x48
	Position m_LastValidPosition;	// custom
	class CObjCell *cell = NULL; // 0x90
	uint32_t num_shadow_objects = 0; // 0x94
	DArray<CShadowObj> shadow_objects; // 0x98
	uint32_t m_PhysicsState = 123; // 0xA8
	uint32_t transient_state = 0; // 0xAC
	float m_fElasticity = DEFAULT_ELASTICITY; // 0xB0
	float m_fTranslucency = DEFAULT_TRANSLUCENCY; // 0xB4
	float translucencyOriginal = DEFAULT_TRANSLUCENCY; // 0xB8
	float m_fFriction = DEFAULT_FRICTION; // 0xBC
	float massinv = 1.0f;
	class MovementManager *movement_manager = NULL; // 0xC4
	class PositionManager *position_manager = NULL; // 0xC8
	int last_move_was_autonomous = 1; // 0xCC
	int jumped_this_frame = 0; // 0xD0
	double update_time = 0.0; // 0xD8
	Vector m_velocityVector; // 0xE0
	Vector m_Acceleration; // 0xE0
	Vector m_Omega; // 0xE0
	class PhysicsObjHook *hooks = NULL; // 0x104
	SmartArray<CAnimHook *> anim_hooks; // 0x108
	float m_scale = 1.0f; // 0x114
	float attack_radius = 0.0f; // 0x118
	class DetectionManager * detection_manager = NULL; // 0x11C
	class AttackManager * attack_manager = NULL; // 0x120
	class TargetManager * target_manager = NULL; // 0x124
	class ParticleManager * particle_manager = NULL;// 0x128
	class CWeenieObject *weenie_obj = NULL; // 0x12C
	Plane contact_plane; // 0x130
	uint32_t contact_plane_cell_id = 0; // 0x140
	Vector sliding_normal; // 0x144
	Vector cached_velocity; // 0x150
	class LongNIValHash<CPhysicsObj::CollisionRecord> * collision_table;
	int colliding_with_environment = 0; // 0x160

	union
	{
		WORD update_times[9];
		struct {
			WORD _position_timestamp; // 0x164
			WORD _movement_timestamp; // 0x166 m_wNumAnimInteracts
			WORD _state_timestamp; // 0x168
			WORD _vector_timestamp; // 0x16A
			WORD _teleport_timestamp; // 0x16C
			WORD _server_control_timestamp; // 0x16E server_control_timestamp
			WORD _force_position_timestamp; // 0x170
			WORD _objdesc_timestamp; // 0x172
			WORD _instance_timestamp; // 0x174

			/*
			list[0] = LF_ENUMERATE, public, value = 0, name = 'POSITION_TS'
			list[1] = LF_ENUMERATE, public, value = 1, name = 'MOVEMENT_TS'
			list[2] = LF_ENUMERATE, public, value = 2, name = 'STATE_TS'
			list[3] = LF_ENUMERATE, public, value = 3, name = 'VECTOR_TS'
			list[4] = LF_ENUMERATE, public, value = 4, name = 'TELEPORT_TS'
			list[5] = LF_ENUMERATE, public, value = 5, name = 'SERVER_CONTROLLED_MOVE_TS'
			list[6] = LF_ENUMERATE, public, value = 6, name = 'FORCE_POSITION_TS'
			list[7] = LF_ENUMERATE, public, value = 7, name = 'OBJDESC_TS'
			list[8] = LF_ENUMERATE, public, value = 8, name = 'INSTANCE_TS'
			list[9] = LF_ENUMERATE, public, value = 9, name = 'NUM_PHYSICS_TS'
			*/
		};
	};

#if PHATSDK_IS_SERVER
#include "Animate.h"
#include "Moves.h"
#include "PhysicsObjCustom.h"

	WORD _last_teleport_timestamp = 0;
#endif
};