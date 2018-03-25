
#pragma once

class EncumbranceSystem
{
public:
	static int EncumbranceCapacity(const int strength, const int encumb_augmentations);
	static double Load(const int encumb_capacity, const int encumb_val);
	static double LoadMod(const float load);
};

class MovementSystem
{
public:
	static float GetRunRate(const float load, const int runskill, const float scaling);
	static float GetJumpHeight(float, long, float, float);
	static long JumpStaminaCost(float power, float load, int bPK);
};

class MovementParameters : public PackObj
{
public:
	MovementParameters();

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	virtual void PackNet(MovementTypes type, BinaryWriter *pWriter);
	virtual bool UnPackNet(MovementTypes type, BinaryReader *pReader);

	void towards_and_away(float curr_distance, float curr_heading, unsigned int *command, int *moving_away);
	void get_command(float curr_distance, float curr_heading, unsigned int *command, HoldKey *key, int *moving_away);
	float get_desired_heading(DWORD command, BOOL moving_away);

	union {
		unsigned long bitfield;
		struct {
			int can_walk : 1; // (1 >> 0) 1
			int can_run : 1; // (1 >> 1) 2
			int can_sidestep : 1; // (1 >> 2) 4
			int can_walk_backwards : 1; // (1 >> 3) 8
			int can_charge : 1; // (1 >> 4) 0x10
			int fail_walk : 1; // (1 >> 5) 0x20
			int use_final_heading : 1; // (1 >> 6) 0x40
			int sticky : 1; // (1 >> 7) 0x80
			int move_away : 1; // (1 >> 8) 0x100
			int move_towards : 1; // (1 >> 9) 0x200
			int use_spheres : 1; // (1 >> 10) 0x400
			int set_hold_key : 1; // (1 >> 11) 0x800
			int autonomous : 1; // (1 >> 12) 0x1000
			int modify_raw_state : 1; // (1 >> 13) 0x2000
			int modify_interpreted_state : 1; // (1 >> 14) 0x4000
			int cancel_moveto : 1; // (1 >> 15) 0x8000
			int stop_completely : 1; // (1 >> 16) 0x10000
			int disable_jump_during_link : 1; // (1 >> 17) 0x20000
		};
	};
	float distance_to_object; // 0x8
	float min_distance; // 0xC
	float desired_heading; // 0x10
	float speed; // 0x14
	float fail_distance; // 0x18
	float walk_run_threshhold; // 0x1C
	unsigned long context_id; // 0x20
	HoldKey hold_key_to_apply; // 0x24
	unsigned long action_stamp; // 0x28
};

struct ActionNode
{
	ActionNode(DWORD action_ = 0, float speed_ = 1.0f, DWORD stamp_ = 0, BOOL autonomous_ = FALSE)
	{
		action = action_;
		speed = speed_;
		stamp = stamp_;
		autonomous = autonomous_;
	}

	void Pack(BinaryWriter *pWriter);
	bool UnPack(BinaryReader *pReader);

	DWORD action;
	float speed;
	DWORD stamp;
	BOOL autonomous;
};

class RawMotionState : public PackObj
{
public:
	RawMotionState() { }
	virtual ~RawMotionState();

	void Destroy();
	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	struct PackBitfield
	{
		union {
			unsigned long bitfield;
			struct {
				int current_holdkey : 1;
				int current_style : 1;
				int forward_command : 1;
				int forward_holdkey : 1;
				int forward_speed : 1;
				int sidestep_command : 1;
				int sidestep_holdkey : 1;
				int sidestep_speed : 1;
				int turn_command : 1;
				int turn_holdkey : 1;
				int turn_speed : 1;
				int num_actions : 5;
			};
		};
	};

	void AddAction(const ActionNode &node);
	DWORD RemoveAction();

	void ApplyMotion(DWORD motion, MovementParameters *params);
	void RemoveMotion(DWORD motion);

	std::list<ActionNode> actions;
	HoldKey current_holdkey = HoldKey_None; // 0x0C
	unsigned long current_style = 0x8000003D; // 0x10
	unsigned long forward_command = 0x41000003; // 0x14
	HoldKey forward_holdkey = HoldKey_Invalid; // 0x18
	float forward_speed = 1.0f; // 0x1C
	unsigned long sidestep_command = 0; // 0x20
	HoldKey sidestep_holdkey = HoldKey_Invalid;
	float sidestep_speed = 1.0f; // 0x28
	unsigned long turn_command = 0;
	HoldKey turn_holdkey = HoldKey_Invalid; // 0x30
	float turn_speed = 1.0f;
};

class InterpretedMotionState : public PackObj
{
public:
	InterpretedMotionState();
	virtual ~InterpretedMotionState();

	void Destroy();

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	struct PackBitfield
	{
		union {
			unsigned long bitfield;
			struct {
				int current_style : 1; // 1
				int forward_command : 1; // 2
				int forward_speed : 1; // 4
				int sidestep_command : 1; // 8
				int sidestep_speed : 1; // 0x10
				int turn_command : 1; // 0x20
				int turn_speed : 1; // 0x40
				int num_actions : 5;
			};
		};
	};

	DWORD GetNumActions();
	void AddAction(const ActionNode &node);
	DWORD RemoveAction();

	void ApplyMotion(DWORD motion, MovementParameters *params);
	void RemoveMotion(DWORD motion);

	void copy_movement_from(const InterpretedMotionState &other);

	DWORD current_style; // 0x04 (DWORD)
	DWORD forward_command; // 0x08 (DWORD)
	float forward_speed; // 0x0C (float)
	DWORD sidestep_command; // 0x10 (DWORD?)
	float sidestep_speed; // 0x14 (float)
	DWORD turn_command; // 0x18 (DWORD)
	float turn_speed; // 0x1C (float)
	std::list<ActionNode> actions; // 0x20, 0x24
};

class MoveToStatePack : public PackObj
{
public:
	MoveToStatePack() { }
	virtual ~MoveToStatePack();

	virtual void Pack(BinaryWriter *pWriter) override;
	virtual bool UnPack(BinaryReader *pReader) override;

	RawMotionState raw_motion_state; // 0x4
	Position position; // 0x3C
	int contact = 0; // 0x84
	int longjump_mode = 0; // 0x88
	short instance_timestamp = 0; // 0x8C
	short server_control_timestamp = 0; // 0x8E
	short teleport_timestamp = 0; // 0x90
	short force_position_ts = 0; // 0x92
};

extern DWORD GetCommandID(WORD index);
extern DWORD OldToNewCommandID(DWORD command_id);
