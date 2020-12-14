
#pragma once

#include "MathLib.h"
#include "Frame.h"

class CPhysicsObj;

enum TargetStatus
{
	Undef_TargetStatus = 0x0,
	Ok_TargetStatus = 0x1,
	ExitWorld_TargetStatus = 0x2,
	Teleported_TargetStatus = 0x3,
	Contained_TargetStatus = 0x4,
	Parented_TargetStatus = 0x5,
	TimedOut_TargetStatus = 0x6,
	FORCE_TargetStatus_32_BIT = 0x7FFFFFFF,
};

class TargetInfo
{
public:
	unsigned int context_id = 0;
	unsigned int object_id = 0;
	float radius = 0;
	double quantum = 0.0;
	Position target_position;
	Position interpolated_position;
	Vector interpolated_heading;
	Vector velocity;
	TargetStatus status = Undef_TargetStatus;
	double last_update_time = 0.0;
};

class ConstraintManager
{
public:
	ConstraintManager(CPhysicsObj *physobj);
	~ConstraintManager();

	static ConstraintManager *Create(CPhysicsObj *physobj);

	void SetPhysicsObject(CPhysicsObj *_physics_obj);

	void UseTime();

	BOOL IsFullyConstrained();
	void ConstrainTo(Position *p, float start_distance, float max_distance);
	void UnConstrain();
	void adjust_offset(Frame *offset, double quantum);

	CPhysicsObj *physics_obj;
	int is_constrained;
	float constraint_pos_offset;
	Position constraint_pos;
	float constraint_distance_start;
	float constraint_distance_max;
};

class StickyManager
{
public:
	StickyManager(CPhysicsObj *_physics_obj);
	~StickyManager();

	void Destroy();

	static StickyManager *Create(CPhysicsObj *_physics_obj);

	void UseTime();
	void SetPhysicsObject(CPhysicsObj *_physics_obj);
	void HandleExitWorld();
	void HandleUpdateTarget(TargetInfo target_info);
	void StickTo(unsigned int _target_id, float _target_radius, float _target_height);
	void adjust_offset(Frame *offset, double quantum);

	unsigned int target_id;
	float target_radius;
	Position target_position;
	CPhysicsObj *physics_obj;
	int initialized;
	double sticky_timeout_time;
};

class InterpolationNode
{
public:
	unsigned int type = 0;
	Position p;
	Vector v;
	float extent;
};

class InterpolationManager
{
public:
	InterpolationManager(CPhysicsObj *new_physobj);
	~InterpolationManager();

	void Destroy();

	static InterpolationManager *Create(CPhysicsObj *_physics_obj);

	void UseTime();
	void SetPhysicsObject(CPhysicsObj *_physics_obj);

	void adjust_offset(Frame *offset, double quantum);

	void StopInterpolating();
	BOOL IsInterpolating();

	void NodeCompleted(BOOL success);

	void InterpolateTo(Position *p, int _keep_heading);

	static BOOL fUseAdjustedSpeed_;

	std::list<InterpolationNode> position_queue;
	CPhysicsObj *physics_obj;
	int keep_heading;
	unsigned int frame_counter;
	float original_distance;
	float progress_quantum;
	int node_fail_counter;
	Position blipto_position;
};

class PositionManager
{
public:
	PositionManager(CPhysicsObj *_physics_obj);
	~PositionManager();

	void Destroy();

	static PositionManager *Create(CPhysicsObj *_physics_obj);

	void UseTime();
	void SetPhysicsObject(CPhysicsObj *_physics_obj);
	void adjust_offset(Frame *offset, double quantum);
	
	BOOL IsInterpolating();

	void StickTo(uint32_t object_id, float radius, float height);
	void UnStick();
	void HandleUpdateTarget(TargetInfo target_info);
	uint32_t GetStickyObjectID();

	void ConstrainTo(Position *p, float start_distance, float max_distance);
	void UnConstrain();
	BOOL IsFullyConstrained();

	void InterpolateTo(Position *p, int keep_heading);

	class InterpolationManager *interpolation_manager;
	class StickyManager *sticky_manager;
	class ConstraintManager *constraint_manager;
	CPhysicsObj *physics_obj;
};