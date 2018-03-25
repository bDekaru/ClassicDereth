
#pragma once

#include "Movement.h"
#include "PositionManager.h"

class CMotionInterp;
class CWeenieObject;

struct MovementStruct // Unknown real name, 0x64 in size
{
	MovementStruct();

	MovementTypes type; // (int) 0x00
	DWORD motion; // 0x04
	DWORD object_id; // 0x08
	DWORD top_level_id; // 0x0C
	Position pos; // 0x10
	float radius; // 0x58
	float height; // 0x5C
	MovementParameters *params; // 0x60

#if PHATSDK_IS_SERVER
	BOOL send_event = FALSE;
#endif
};

class CMotionInterp
{
public:
	class MotionNode
	{
	public:
		MotionNode(DWORD context_id_, DWORD motion_, DWORD jump_error_code_)
		{
			context_id = context_id_;
			motion = motion_;
			jump_error_code = jump_error_code_;
		}

		//MotionData *m_pNext; // 0x00
		DWORD context_id; // 0x04
		DWORD motion; // 0x08
		DWORD jump_error_code; // 0x0C
	};

	CMotionInterp(CPhysicsObj *pPObject, CWeenieObject *pWObject);
	~CMotionInterp();

	static CMotionInterp* Create(CPhysicsObj *pPObject, CWeenieObject *pWObject);
	void Destroy();

	void HandleExitWorld();
	DWORD DoMotion(DWORD MotionID, MovementParameters *Params);
	DWORD DoInterpretedMotion(DWORD mid, MovementParameters *params);
	void HitGround();
	void LeaveGround();
	void MotionDone(BOOL success);
	DWORD PerformMovement(MovementStruct &mvs);
	void SetHoldKey(HoldKey key, BOOL cancel_moveto);
	void SetPhysicsObject(CPhysicsObj *pObject);
	void SetWeenieObject(CWeenieObject *pObject);
	DWORD StopCompletely();
	DWORD StopMotion(DWORD MotionID, MovementParameters *Params);
	DWORD StopInterpretedMotion(DWORD mid, MovementParameters *params);
	void HandleEnterWorld();

	void add_to_queue(DWORD context_id, DWORD motion, DWORD jump_error_code);
	void adjust_motion(DWORD &motion, float &speed, HoldKey key);
	void apply_current_movement(BOOL cancel_moveto, BOOL disallow_jump);
	void apply_interpreted_movement(BOOL cancel_moveto, BOOL disallow_jump);
	void apply_raw_movement(BOOL cancel_moveto, BOOL disallow_jump);
	void apply_run_to_command(DWORD &motion, float &speed);
	BOOL contact_allows_move(DWORD mid);
	void enter_default_state();
	float get_jump_v_z();
	void get_leave_ground_velocity(Vector& v);
	double get_adjusted_max_speed();
	float get_max_speed();
	Vector get_state_velocity();
	DWORD jump(float extent, long &stamina_adjustment);
	DWORD jump_charge_is_allowed();
	DWORD jump_is_allowed(float extent, long &stamina_cost);
	DWORD motion_allows_jump(DWORD mid);
	BOOL motions_pending();
	BOOL move_to_interpreted_state(const InterpretedMotionState &state);
	DWORD InqStyle() { return interpreted_state.current_style; }

	DWORD initted; // 0x00
	CWeenieObject *weenie_obj; // 0x04
	CPhysicsObj *physics_obj; // 0x08
	RawMotionState raw_state; // 0x0C
	InterpretedMotionState interpreted_state; // 0x44

	float current_speed_factor; // 0x6C (float)
	DWORD standing_longjump; // 0x70
	float jump_extent; // 0x74
	DWORD server_action_stamp; // 0x78
	float my_run_rate; // 0x7C (float)

	std::list<MotionNode> pending_motions; // 0x80, 0x84
};

class MoveToManager
{
public:
	MoveToManager();
	~MoveToManager();

	static MoveToManager *Create(CPhysicsObj *pPhysicsObj, CWeenieObject *pWeenieObj);

	void Destroy();

	void MoveToObject_Internal(Position *_target_position, Position *interpolated_position);
	void TurnToObject_Internal(Position *_target_position);
	void HandleUpdateTarget(TargetInfo *target_info);
	void CancelMoveTo(DWORD err);
	void CleanUp();
	void CleanUpAndCallWeenie(DWORD retval);
	void HitGround();
	void InitializeLocalVariables();
	void LeaveGround();
	DWORD PerformMovement(MovementStruct &mvs);
	void SetPhysicsObject(CPhysicsObj *pPhysicsObj);
	void SetWeenieObject(CWeenieObject *pWeenie);
	double GetCurrentDistance();
	void MoveToPosition(Position *p, MovementParameters *params);
	void AddTurnToHeadingNode(float global_heading);
	void AddMoveToPositionNode();
	int CheckProgressMade(float curr_distance);

	DWORD _StopMotion(DWORD arg_0, MovementParameters *Params);
	void BeginNextNode();
	void TurnToObject(DWORD object_id, DWORD top_level_id, MovementParameters *params);
	void TurnToHeading(MovementParameters *pparams);
	void UseTime();
	void HandleMoveToPosition();
	void HandleTurnToHeading();
	void BeginTurnToHeading();
	void BeginMoveForward();
	DWORD _DoMotion(DWORD motionid, MovementParameters *pparams);
	void RemovePendingActionsHead();
	void MoveToObject(DWORD object_id, DWORD top_level_id, float object_radius, float object_height, MovementParameters *params);

	BOOL is_moving_to(void);

	class MovementNode
	{
	public:
		MovementNode() { }
		// 0 and 4 are linked list node
		DWORD type = 0; // 0x08
		float heading = 0.0f; // 0x0C
	};

	MovementTypes movement_type = MovementTypes::Invalid; // 0x00
	Position sought_position; // 0x04
	Position current_target_position; // 0x4C
	Position starting_position; // 0x94
	MovementParameters movement_params; // 0xDC
	float previous_heading = 0.0f;
	float previous_distance = FLT_MAX; // 0x10C
	double previous_distance_time; // 0x110
	float original_distance = FLT_MAX; // 0x118
	double original_distance_time; // 0x120
	DWORD fail_progress_count = 0; // 0x128
	DWORD sought_object_id = 0; // 0x12C
	DWORD top_level_object_id = 0; // 0x130
	float sought_object_radius; // 0x134
	float sought_object_height; // 0x138
	DWORD current_command = 0; // 0x13C
	DWORD aux_command = 0; // 0x140
	int moving_away = 0; // 0x144
	BOOL initialized = FALSE; // 0x148
	std::list<MovementNode> pending_actions; // 0x14C
	CPhysicsObj * physics_obj; // 0x154
	CWeenieObject * weenie_obj; // 0x158
};

class MovementManager
{
public:
	MovementManager();
	virtual ~MovementManager();

	static MovementManager *Create(CPhysicsObj *pPObject, CWeenieObject *pWObject);

	CMotionInterp *get_minterp();
	BOOL motions_pending();
	BOOL move_to_interpreted_state(const InterpretedMotionState &state);
	BOOL unpack_movement(BinaryReader *pReader);

	void HandleExitWorld();
	void HandleUpdateTarget(TargetInfo target_info);
	void CancelMoveTo(DWORD Unknown);
	void Destroy();
	void EnterDefaultState();
	void HitGround();
	BOOL IsMovingTo();
	void LeaveGround();
	void MakeMoveToManager();
	void MotionDone(DWORD motion, BOOL success);
	DWORD PerformMovement(MovementStruct &mvs);
	void SetPhysicsObject(CPhysicsObj *pObject);
	void SetWeenieObject(CWeenieObject *pObject);
	void UseTime();
	void HandleEnterWorld();

	CMotionInterp *motion_interpreter; // 0x00
	MoveToManager *moveto_manager; // 0x04
	CPhysicsObj * physics_obj; // 0x08
	CWeenieObject *weenie_obj; // 0x0C
};

