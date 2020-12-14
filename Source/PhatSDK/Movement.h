
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
	static float GetJumpHeight(float, int32_t, float, float);
	static int32_t JumpStaminaCost(float power, float load, int bPK);
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
	float get_desired_heading(uint32_t command, BOOL moving_away);

	union {
		uint32_t bitfield;
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
	uint32_t context_id; // 0x20
	HoldKey hold_key_to_apply; // 0x24
	uint32_t action_stamp; // 0x28
};

struct ActionNode
{
	ActionNode(uint32_t action_ = 0, float speed_ = 1.0f, uint32_t stamp_ = 0, BOOL autonomous_ = FALSE)
	{
		action = action_;
		speed = speed_;
		stamp = stamp_;
		autonomous = autonomous_;
	}

	void Pack(BinaryWriter *pWriter);
	bool UnPack(BinaryReader *pReader);

	uint32_t action;
	float speed;
	uint32_t stamp;
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
			uint32_t bitfield;
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
	uint32_t RemoveAction();

	void ApplyMotion(uint32_t motion, MovementParameters *params);
	void RemoveMotion(uint32_t motion);

	std::list<ActionNode> actions;
	HoldKey current_holdkey = HoldKey_None; // 0x0C
	uint32_t current_style = 0x8000003D; // 0x10
	uint32_t forward_command = 0x41000003; // 0x14
	HoldKey forward_holdkey = HoldKey_Invalid; // 0x18
	float forward_speed = 1.0f; // 0x1C
	uint32_t sidestep_command = 0; // 0x20
	HoldKey sidestep_holdkey = HoldKey_Invalid;
	float sidestep_speed = 1.0f; // 0x28
	uint32_t turn_command = 0;
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
			uint32_t bitfield;
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

	uint32_t GetNumActions();
	void AddAction(const ActionNode &node);
	uint32_t RemoveAction();

	void ApplyMotion(uint32_t motion, MovementParameters *params);
	void RemoveMotion(uint32_t motion);

	void copy_movement_from(const InterpretedMotionState &other);

	uint32_t current_style; // 0x04 (uint32_t)
	uint32_t forward_command; // 0x08 (uint32_t)
	float forward_speed; // 0x0C (float)
	uint32_t sidestep_command; // 0x10 (uint32_t?)
	float sidestep_speed; // 0x14 (float)
	uint32_t turn_command; // 0x18 (uint32_t)
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

extern uint32_t GetCommandID(WORD index);
extern uint32_t OldToNewCommandID(uint32_t command_id);
