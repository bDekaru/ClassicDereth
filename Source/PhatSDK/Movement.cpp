
#include "StdAfx.h"
#include "Movement.h"

int EncumbranceSystem::EncumbranceCapacity(const int strength, const int encumb_augmentations)
{
	if (strength > 0)
	{
		int bonusBurden = 30 * encumb_augmentations;

		if (bonusBurden >= 0)
		{
			if (bonusBurden > 150)
				bonusBurden = 150;

			return (150 * strength + strength * bonusBurden);
		}
		else
		{
			return (150 * strength);
		}
	}

	return 0;
}

double EncumbranceSystem::Load(const int encumb_capacity, const int encumb_val)
{
	if (encumb_capacity > 0)
	{
		if (encumb_val >= 0)
		{
			return ((double)encumb_val / (double)encumb_capacity);
		}

		return 0.0;
	}

	return 3.0;
}

double EncumbranceSystem::LoadMod(const float load)
{
	if (load >= 1.0)
	{
		if (load >= 2.0)
			return (0.0);
		else
			return (2.0 - load);
	}
	else
		return (1.0);
}

float MovementSystem::GetRunRate(const float load, const int runskill, const float scaling)
{
	float loadMod = EncumbranceSystem::LoadMod(load);

	double fRunSkill = (double)runskill;

	if (fRunSkill == 800.0)
		return (18.0 / 4.0); // Special case, fastest
	else
		return ((loadMod * (fRunSkill / (fRunSkill + 200.0) * 11.0) + 4.0) / scaling) / 4.0;
}

float MovementSystem::GetJumpHeight(float load, long jumpskill, float _power, float scaling)
{
	if (_power <= 1.0)
	{
		if (_power < 0.0)
			_power = 0.0;
	}
	else
	{
		_power = 1.0;
	}

	double jump_power = (double)jumpskill;

	double result = EncumbranceSystem::LoadMod(load) * (jump_power / (jump_power + 1300.0) * 22.200001 + 0.050000001) * _power / scaling;
	if (result < 0.35)
		result = 0.35;

	return result;
}

long MovementSystem::JumpStaminaCost(float power, float load, int bPK)
{
	if (bPK)
		return (long)((power + 1.0) * 100.0);

	return (long)ceil((load + 0.5) * power * 8.0 + 2.0);
}

const float DEFAULT_MIN_DISTANCE = 0.0f;
const float DEFAULT_DISTANCE_TO_OBJECT = 0.6f;
const float DEFAULT_WALK_RUN_THRESHHOLD = 15.0f;
const float DEFAULT_FAIL_DISTANCE = FLT_MAX;

MovementParameters::MovementParameters()
{
	min_distance = DEFAULT_MIN_DISTANCE;
	distance_to_object = DEFAULT_DISTANCE_TO_OBJECT;
	fail_distance = DEFAULT_FAIL_DISTANCE;
	desired_heading = 0;
	speed = 1.0f;
	walk_run_threshhold = DEFAULT_WALK_RUN_THRESHHOLD;
	context_id = 0;
	hold_key_to_apply = HoldKey_Invalid;
	action_stamp = 0;
	
	bitfield = 0x1EE0F; // this is actually a bug in the code, but I believe this is the value they intended
	/*
	equivalent to this maybe
	stop_completely = 1;
	cancel_moveto = 1;
	modify_interpreted_state = 1;
	modify_raw_state = 1;
	set_hold_key = 1;
	use_spheres = 1;
	move_towards = 1;
	can_walk_backwards = 1;
	can_sidestep = 1;
	can_run = 1;
	can_walk = 1;
	*/
}

void MovementParameters::Pack(BinaryWriter *pWriter)
{
	UNFINISHED();
}

void MovementParameters::PackNet(MovementTypes type, BinaryWriter *pWriter)
{
	switch (type)
	{
	case MovementTypes::MoveToObject:
	case MovementTypes::MoveToPosition:
		pWriter->Write<DWORD>(bitfield);
		pWriter->Write<float>(distance_to_object);
		pWriter->Write<float>(min_distance);
		pWriter->Write<float>(fail_distance);
		pWriter->Write<float>(speed);
		pWriter->Write<float>(walk_run_threshhold);
		pWriter->Write<float>(desired_heading);
		break;

	case MovementTypes::TurnToObject:
	case MovementTypes::TurnToHeading:
		pWriter->Write<DWORD>(bitfield);
		pWriter->Write<float>(speed);
		pWriter->Write<float>(desired_heading);
		break;
	}
}

bool MovementParameters::UnPack(BinaryReader *pReader)
{
	bitfield = pReader->Read<DWORD>();
	distance_to_object = pReader->Read<float>();
	min_distance = pReader->Read<float>();
	fail_distance = pReader->Read<float>();
	desired_heading = pReader->Read<float>();
	speed = pReader->Read<float>();
	walk_run_threshhold = pReader->Read<float>();
	context_id = pReader->Read<DWORD>();
	hold_key_to_apply = (HoldKey)pReader->Read<int>();
	action_stamp = pReader->Read<DWORD>();
	return true;
}

bool MovementParameters::UnPackNet(MovementTypes type, BinaryReader *pReader)
{
	switch (type)
	{
	case MovementTypes::MoveToObject:
	case MovementTypes::MoveToPosition:
		bitfield = pReader->Read<DWORD>();
		distance_to_object = pReader->Read<float>();
		min_distance = pReader->Read<float>();
		fail_distance = pReader->Read<float>();
		speed = pReader->Read<float>();
		walk_run_threshhold = pReader->Read<float>();
		desired_heading = pReader->Read<float>();
		return true;

	case MovementTypes::TurnToObject:
	case MovementTypes::TurnToHeading:
		bitfield = pReader->Read<DWORD>();
		speed = pReader->Read<float>();
		desired_heading = pReader->Read<float>();
		return true;

	default:
		return false;
	}
}

void MovementParameters::towards_and_away(float curr_distance, float curr_heading, unsigned int *command, int *moving_away)
{
	if (curr_distance > distance_to_object)
	{
		*command = 0x45000005;
		*moving_away = FALSE;
	}
	else if ((curr_distance - min_distance) < F_EPSILON)
	{
		*command = 0x45000006;
		*moving_away = TRUE;
	}
	else
		*command = 0;
}

void MovementParameters::get_command(float curr_distance, float curr_heading, unsigned int *command, HoldKey *key, int *moving_away)
{
	if (move_towards)
	{
		if (move_away)
			towards_and_away(curr_distance, curr_heading, command, moving_away);
		else
		{
			if (curr_distance <= distance_to_object)
				*command = 0;
			else
			{
				*command = 0x45000005;
				*moving_away = FALSE;
			}
		}
	}
	else if (move_away)
	{
		if (curr_distance >= min_distance)
			*command = 0;
		else
		{
			*command = 0x45000005;
			*moving_away = TRUE;
		}
	}
	else
	{
		if (curr_distance <= distance_to_object)
			*command = 0;
		else
		{
			*command = 0x45000005;
			*moving_away = FALSE;
		}
	}

	if (can_charge || (can_run && (!can_walk || ((curr_distance - distance_to_object) > walk_run_threshhold))))
		*key = HoldKey_Run;
	else
		*key = HoldKey_None;
}

float MovementParameters::get_desired_heading(DWORD command, BOOL moving_away)
{
	float heading = 0;

	switch (command)
	{
	case 0x44000007:
	case 0x45000005:
		return (moving_away ? ((float)180.0) : ((float)0.0));
	case 0x45000006:
		return (moving_away ? ((float)0.0) : ((float)180.0));
	default:
		return ((float)0);
	}
}

const DWORD command_ids[] =
{
	0x80000000,
	0x85000001,
	0x85000002,
	0x41000003,
	0x40000004,
	0x45000005,
	0x45000006,
	0x44000007,
	0x40000008,
	0x40000009,
	0x4000000A,
	0x4000000B,
	0x4000000C,
	0x6500000D,
	0x6500000E,
	0x6500000F,
	0x65000010,
	0x40000011,
	0x41000012,
	0x41000013,
	0x41000014,
	0x40000015,
	0x40000016,
	0x40000017,
	0x40000018,
	0x40000019,
	0x4000001A,
	0x4000001B,
	0x4000001C,
	0x4000001D,
	0x4000001E,
	0x4000001F,
	0x40000020,
	0x40000021,
	0x40000022,
	0x40000023,
	0x40000024,
	0x40000025,
	0x40000026,
	0x40000027,
	0x40000028,
	0x40000029,
	0x4000002A,
	0x4000002B,
	0x4000002C,
	0x4000002D,
	0x4000002E,
	0x4000002F,
	0x40000030,
	0x40000031,
	0x40000032,
	0x40000033,
	0x40000034,
	0x40000035,
	0x40000036,
	0x40000037,
	0x40000038,
	0x40000039,
	0x2000003A,
	0x2500003B,
	0x8000003C,
	0x8000003D,
	0x8000003E,
	0x8000003F,
	0x80000040,
	0x80000041,
	0x80000042,
	0x80000043,
	0x80000044,
	0x80000045,
	0x80000046,
	0x80000047,
	0x80000048,
	0x80000049,
	0x1000004A,
	0x1000004B,
	0x1300004C,
	0x1000004D,
	0x1000004E,
	0x1000004F,
	0x10000050,
	0x10000051,
	0x10000052,
	0x10000053,
	0x10000054,
	0x10000055,
	0x10000056,
	0x10000057,
	0x10000058,
	0x10000059,
	0x1000005A,
	0x1000005B,
	0x1000005C,
	0x1000005D,
	0x1000005E,
	0x1000005F,
	0x10000060,
	0x10000061,
	0x10000062,
	0x10000063,
	0x10000064,
	0x10000065,
	0x10000066,
	0x10000067,
	0x10000068,
	0x10000069,
	0x1000006A,
	0x1000006B,
	0x1000006C,
	0x1000006D,
	0x1000006E,
	0x1000006F,
	0x10000070,
	0x10000071,
	0x10000072,
	0x10000073,
	0x10000074,
	0x10000075,
	0x10000076,
	0x10000077,
	0x10000078,
	0x13000079,
	0x1300007A,
	0x1300007B,
	0x1300007C,
	0x1300007D,
	0x1300007E,
	0x1300007F,
	0x13000080,
	0x13000081,
	0x13000082,
	0x13000083,
	0x13000084,
	0x13000085,
	0x13000086,
	0x13000087,
	0x13000088,
	0x13000089,
	0x1300008A,
	0x1300008B,
	0x1300008C,
	0x1300008D,
	0x1300008E,
	0x1300008F,
	0x13000090,
	0x13000091,
	0x13000092,
	0x13000093,
	0x13000094,
	0x13000095,
	0x13000096,
	0x13000097,
	0x13000098,
	0x13000099,
	0x1300009A,
	0x1200009B,
	0x1000009C,
	0x1000009D,
	0x1000009E,
	0x1000009F,
	0x100000A0,
	0x100000A1,
	0x80000A2,
	0x90000A3,
	0x90000A4,
	0x90000A5,
	0x90000A6,
	0x90000A7,
	0x90000A8,
	0x80000A9,
	0x90000AA,
	0x90000AB,
	0x90000AC,
	0x90000AD,
	0x90000AE,
	0x90000AF,
	0x90000B0,
	0x90000B1,
	0x0D0000B2,
	0x0D0000B3,
	0x0D0000B4,
	0x80000B5,
	0x80000B6,
	0x80000B7,
	0x90000B8,
	0x90000B9,
	0x0D0000BA,
	0x0D0000BB,
	0x0D0000BC,
	0x0D0000BD,
	0x0D0000BE,
	0x0D0000BF,
	0x90000C0,
	0x0C0000C1,
	0x90000C2,
	0x90000C3,
	0x90000C4,
	0x0D0000C5,
	0x90000C6,
	0x90000C7,
	0x90000C8,
	0x90000C9,
	0x130000CA,
	0x130000CB,
	0x130000CC,
	0x100000CD,
	0x100000CE,
	0x100000CF,
	0x100000D0,
	0x100000D1,
	0x100000D2,
	0x400000D3,
	0x120000D4,
	0x90000D5,
	0x90000D6,
	0x90000D7,
	0x90000D8,
	0x90000D9,
	0x90000DA,
	0x90000DB,
	0x90000DC,
	0x90000DD,
	0x90000DE,
	0x120000DF,
	0x400000E0,
	0x400000E1,
	0x100000E2,
	0x100000E3,
	0x400000E4,
	0x400000E5,
	0x400000E6,
	0x90000E7,
	0x800000E8,
	0x800000E9,
	0x430000EA,
	0x430000EB,
	0x430000EC,
	0x430000ED,
	0x430000EE,
	0x430000EF,
	0x430000F0,
	0x430000F1,
	0x430000F2,
	0x430000F3,
	0x430000F4,
	0x430000F5,
	0x430000F6,
	0x430000F7,
	0x430000F8,
	0x420000F9,
	0x430000FA,
	0x430000FB,
	0x430000FC,
	0x430000FD,
	0x90000FE,
	0x90000FF,
	0x9000100,
	0x9000101,
	0x9000102,
	0x9000103,
	0x9000104,
	0x9000105,
	0x9000106,
	0x9000107,
	0x9000108,
	0x9000109,
	0x900010A,
	0x900010B,
	0x900010C,
	0x900010D,
	0x1000010E,
	0x1000010F,
	0x10000110,
	0x10000111,
	0x9000112,
	0x9000113,
	0x9000114,
	0x9000115,
	0x9000116,
	0x9000117,
	0x43000118,
	0x13000119,
	0x4300011A,
	0x4300011B,
	0x4300011C,
	0x900011D,
	0x1000011E,
	0x1000011F,
	0x10000120,
	0x10000121,
	0x10000122,
	0x10000123,
	0x10000124,
	0x10000125,
	0x10000126,
	0x10000127,
	0x10000128,
	0x10000129,
	0x1000012A,
	0x1000012B,
	0x1000012C,
	0x1000012D,
	0x1000012E,
	0x1000012F,
	0x10000130,
	0x10000131,
	0x10000132,
	0x10000133,
	0x10000134,
	0x13000135,
	0x40000136,
	0x40000137,
	0x40000138,
	0x40000139,
	0x1000013A,
	0x8000013B,
	0x8000013C,
	0x4300013D,
	0x4300013E,
	0x4300013F,
	0x43000140,
	0x43000141,
	0x43000142,
	0x43000143,
	0x43000144,
	0x43000145,
	0x43000146,
	0x43000147,
	0x43000148,
	0x43000149,
	0x1300014A,
	0x1300014B,
	0x1300014C,
	0x1300014D,
	0x1300014E,
	0x1300014F,
	0x13000150,
	0x13000151,
	0x13000152,
	0x10000153,
	0x9000154,
	0x9000155,
	0x9000156,
	0x9000157,
	0x9000158,
	0x9000159,
	0x900015A,
	0x900015B,
	0x900015C,
	0x900015D,
	0x900015E,
	0x900015F,
	0x9000160,
	0x9000161,
	0x9000162,
	0x9000163,
	0x9000164,
	0x10000165,
	0x10000166,
	0x10000167,
	0x9000168,
	0x9000169,
	0x900016A,
	0x900016B,
	0x900016C,
	0x900016D,
	0x900016E,
	0x900016F,
	0x9000170,
	0x10000171,
	0x10000172,
	0x10000173,
	0x10000174,
	0x10000175,
	0x10000176,
	0x10000177,
	0x10000178,
	0x10000179,
	0x1000017A,
	0x1000017B,
	0x1000017C,
	0x1000017D,
	0x1000017E,
	0x1000017F,
	0x10000180,
	0x10000181,
	0x10000182,
	0x10000183,
	0x10000184,
	0x10000185,
	0x10000186,
	0x10000187,
	0x10000188,
	0x10000189,
	0x1000018A,
	0x1000018B,
	0x1000018C,
	0x1000018D,
	0x1000018E,
	0x1000018F,
	0x10000190,
	0x10000191,
	0x10000192,
	0x10000193,
	0x10000194,
	0x10000195,
	0x10000196,
	0x10000197,
	0x10000198,
	0x10000199,
	0x1000019A,
	0x1000019B,
};

const DWORD NUM_COMMAND_IDS = sizeof(command_ids) / sizeof(DWORD); // 408 entries

DWORD GetCommandID(WORD index)
{
	if (index >= NUM_COMMAND_IDS)
		return command_ids[0];

	return command_ids[index];
}

DWORD OldToNewCommandID(DWORD command_id)
{
	WORD index = (WORD)(command_id & 0xFFFF);

	if (index >= 0x115)
	{
		index += 3;
	}

	return GetCommandID(index);
}

void ActionNode::Pack(BinaryWriter *pWriter)
{
	pWriter->Write<WORD>((WORD)(action & 0xFFFF));
	WORD data = (stamp & 0x7FFF) | (autonomous ? 0x8000 : 0);
	pWriter->Write<WORD>(data);

	pWriter->Write<float>(speed);
}

bool ActionNode::UnPack(BinaryReader *pReader)
{
	WORD commandIndex;
	WORD data;

	commandIndex = pReader->Read<WORD>();
	data = pReader->Read<WORD>();
	speed = pReader->Read<float>();
	action = GetCommandID(commandIndex);

	stamp = data & 0x7FFF;
	autonomous = (data >> 15) & 1;
	return true;
}

RawMotionState::~RawMotionState()
{
	Destroy();
}

void RawMotionState::Destroy()
{
	actions.clear();
}

void RawMotionState::Pack(BinaryWriter *pWriter)
{
	BinaryWriter content;

	RawMotionState::PackBitfield flags;
	flags.bitfield = 0;

	if (current_holdkey)
	{
		flags.current_holdkey = 1;
		content.Write<DWORD>(current_holdkey);
	}
	if (current_style != 0x8000003D)
	{
		flags.current_style = 1;
		content.Write<DWORD>(current_style);
	}
	if (forward_command != 0x41000003)
	{
		flags.forward_command = 1;
		content.Write<DWORD>(forward_command);
	}
	if (forward_holdkey)
	{
		flags.forward_holdkey = 1;
		content.Write<DWORD>(forward_holdkey);
	}
	if (forward_speed != 1.0f)
	{
		flags.forward_speed = 1;
		content.Write<float>(forward_speed);
	}
	if (sidestep_command)
	{
		flags.sidestep_command = 1;
		content.Write<DWORD>(sidestep_command);
	}
	if (sidestep_holdkey)
	{
		flags.sidestep_holdkey = 1;
		content.Write<DWORD>(sidestep_holdkey);
	}
	if (sidestep_speed)
	{
		flags.sidestep_speed = 1;
		content.Write<float>(sidestep_speed);
	}
	if (turn_command)
	{
		flags.turn_command = 1;
		content.Write<DWORD>(turn_command);
	}
	if (turn_holdkey)
	{
		flags.turn_holdkey = 1;
		content.Write<DWORD>(turn_holdkey);
	}
	if (turn_speed)
	{
		flags.turn_speed = 1;
		content.Write<float>(turn_speed);
	}

	int numActions = actions.size() & 0x1F;
	flags.num_actions = numActions;

	for (auto &action : actions)
	{
		if (!numActions) // no space left
			break;

		action.Pack(&content);
		numActions--;
	}

	pWriter->Write<DWORD>(flags.bitfield);
	pWriter->Write(&content);
}

bool RawMotionState::UnPack(BinaryReader *pReader)
{
	Destroy();

	RawMotionState::PackBitfield flags;
	flags.bitfield = pReader->Read<DWORD>();

	if (flags.current_holdkey)
		current_holdkey = (HoldKey)pReader->Read<DWORD>();
	else
		current_holdkey = HoldKey_None;

	if (flags.current_style)
		current_style = pReader->Read<DWORD>();
	else
		current_style = 0x8000003D;

	if (flags.forward_command)
		forward_command = pReader->Read<DWORD>();
	else
		forward_command = 0x41000003;

	if (flags.forward_holdkey)
		forward_holdkey = (HoldKey)pReader->Read<DWORD>();
	else
		forward_holdkey = HoldKey_Invalid;

	if (flags.forward_speed)
		forward_speed = pReader->Read<float>();
	else
		forward_speed = 1.0f;

	if (flags.sidestep_command)
		sidestep_command = pReader->Read<DWORD>();
	else
		sidestep_command = 0;

	if (flags.sidestep_holdkey)
		sidestep_holdkey = (HoldKey)pReader->Read<DWORD>();
	else
		sidestep_holdkey = HoldKey_Invalid;

	if (flags.sidestep_speed)
		sidestep_speed = pReader->Read<float>();
	else
		sidestep_speed = 1.0f;

	if (flags.turn_command)
		turn_command = pReader->Read<DWORD>();
	else
		turn_command = 0;

	if (flags.turn_holdkey)
		turn_holdkey = (HoldKey)pReader->Read<DWORD>();
	else
		turn_holdkey = HoldKey_Invalid;

	if (flags.turn_speed)
		turn_speed = pReader->Read<float>();
	else
		turn_speed = 1.0f;

	for (int i = 0; i < flags.num_actions; i++)
	{
		ActionNode node;
		node.UnPack(pReader);
		actions.push_back(node);
	}

	return TRUE;
}

void RawMotionState::AddAction(const ActionNode &node)
{
	actions.push_back(node);
}

DWORD RawMotionState::RemoveAction()
{
	std::list<ActionNode>::iterator i = actions.begin();

	if (i != actions.end())
	{
		DWORD action = i->action;
		actions.erase(i);

		return action;
	}

	return 0;
}

void RawMotionState::ApplyMotion(DWORD motion, MovementParameters *params)
{
	switch (motion)
	{
	case 0x6500000D:
	case 0x6500000E:
		turn_command = motion;

		if (params->set_hold_key)
		{
			turn_holdkey = HoldKey_Invalid;
			turn_speed = params->speed;
		}
		else
		{
			turn_holdkey = params->hold_key_to_apply;
			turn_speed = params->speed;
		}

		break;

	case 0x6500000F:
	case 0x65000010:
		sidestep_command = motion;

		if (params->set_hold_key)
		{
			sidestep_holdkey = HoldKey_Invalid;
			sidestep_speed = params->speed;
		}
		else
		{
			sidestep_holdkey = params->hold_key_to_apply;
			sidestep_speed = params->speed;
		}

		break;

	default:
		if (motion & 0x40000000)
		{
			if (motion != 0x44000007)
			{
				forward_command = motion;

				if (params->set_hold_key)
				{
					forward_holdkey = HoldKey_Invalid;
					forward_speed = params->speed;
				}
				else
				{
					forward_holdkey = params->hold_key_to_apply;
					forward_speed = params->speed;
				}
			}
		}
		else if (motion & 0x80000000)
		{
			if (current_style != motion)
			{
				forward_command = 0x41000003;
				current_style = motion;
			}
		}
		else if (motion & 0x10000000)
		{
			AddAction(ActionNode(motion, params->speed, params->action_stamp, params->autonomous));
		}

		break;
	}
}

void RawMotionState::RemoveMotion(DWORD motion)
{
	switch (motion)
	{
	case 0x6500000D:
	case 0x6500000E:
		turn_command = 0;
		break;

	case 0x6500000F:
	case 0x65000010:
		sidestep_command = 0;
		break;

	default:
		if (motion & 0x40000000)
		{
			if (motion == forward_command)
			{
				forward_command = 0x41000003;
				forward_speed = 1.0f;
			}
		}
		else if (motion & 0x80000000)
		{
			if (motion == current_style)
			{
				current_style = 0x8000003D;
			}
		}

		break;
	}
}


InterpretedMotionState::InterpretedMotionState()
{
	current_style = 0x8000003D;
	forward_command = 0x41000003;
	forward_speed = 1.0f;
	sidestep_command = 0;
	sidestep_speed = 1.0f;
	turn_command = 0;
	turn_speed = 1.0f;
}

InterpretedMotionState::~InterpretedMotionState()
{
	Destroy();
}

void InterpretedMotionState::Destroy()
{
	actions.clear();
}

void InterpretedMotionState::copy_movement_from(const InterpretedMotionState &other)
{
	current_style = other.current_style;
	forward_command = other.forward_command;
	forward_speed = other.forward_speed;
	sidestep_command = other.sidestep_command;
	sidestep_speed = other.sidestep_speed;
	turn_command = other.turn_command;
	turn_speed = other.turn_speed;

	// doesn't copy actions
}

void InterpretedMotionState::ApplyMotion(DWORD motion, MovementParameters *params)
{
	switch (motion)
	{
	case 0x6500000D:
		turn_command = 0x6500000D;
		turn_speed = params->speed;
		break;

	case 0x6500000F:
		sidestep_command = 0x6500000F;
		sidestep_speed = params->speed;
		break;

	default:
		if (motion & 0x40000000) // CM_SubState
		{
			forward_command = motion;
			forward_speed = params->speed;
		}
		else if (motion & 0x80000000) // CM_Style
		{
			forward_command = 0x41000003;
			current_style = motion;
		}
		else if (motion & 0x10000000)
		{
			AddAction(ActionNode(motion, params->speed, params->action_stamp, params->autonomous));
		}

		break;
	}
}

void InterpretedMotionState::RemoveMotion(DWORD motion)
{
	switch (motion)
	{
	case 0x6500000D:
		turn_command = 0;
		break;
	case 0x6500000F:
		sidestep_command = 0;
		break;

	default:

		if (motion & 0x40000000)
		{
			if (forward_command == motion)
			{
				forward_command = 0x41000003;
				forward_speed = 1.0f;
			}
		}
		else if (motion & 0x80000000)
		{
			if (current_style == motion)
			{
				current_style = 0x8000003D;
			}
		}

		break;
	}
}

DWORD InterpretedMotionState::GetNumActions()
{
	return (DWORD) actions.size();
}

void InterpretedMotionState::AddAction(const ActionNode &action)
{
	actions.push_back(action);
}

DWORD InterpretedMotionState::RemoveAction()
{
	std::list<ActionNode>::iterator i = actions.begin();

	if (i != actions.end())
	{
		DWORD action = i->action;
		actions.erase(i);

		return action;
	}

	return 0;
}

void InterpretedMotionState::Pack(BinaryWriter *pWriter)
{
	BinaryWriter content;
	PackBitfield flags;
	flags.bitfield = 0;

	if (current_style != 0x8000003D)
	{
		flags.current_style = 1;
		content.Write<WORD>((WORD)(current_style & 0xFFFF));
	}

	if (forward_command != 0x41000003)
	{
		flags.forward_command = 1;
		content.Write<WORD>((WORD)(forward_command & 0xFFFF));
	}

	if (sidestep_command != 0)
	{
		flags.sidestep_command = 1;
		content.Write<WORD>((WORD)(sidestep_command & 0xFFFF));
	}

	if (turn_command != 0)
	{
		flags.turn_command = 1;
		content.Write<WORD>((WORD)(turn_command & 0xFFFF));
	}

	if (forward_speed != 1.0f)
	{
		flags.forward_speed = 1;
		content.Write<float>(forward_speed);
	}

	if (sidestep_speed != 1.0f)
	{
		flags.sidestep_speed = 1;
		content.Write<float>(sidestep_speed);
	}

	if (turn_speed != 1.0f)
	{
		flags.turn_speed = 1;
		content.Write<float>(turn_speed);
	}

	int numActions = actions.size() & 0x1F;
	flags.num_actions = numActions;

	for (auto &action : actions)
	{
		if (!numActions) // no space left
			break;

		action.Pack(&content);
		numActions--;
	}

	pWriter->Write<DWORD>(flags.bitfield);
	pWriter->Write(&content);
	pWriter->Align();
}

bool InterpretedMotionState::UnPack(BinaryReader *pReader)
{
	Destroy();

	PackBitfield flags;
	flags.bitfield = pReader->Read<DWORD>();

	if (flags.current_style)
		current_style = GetCommandID(pReader->Read<WORD>());
	else
		current_style = 0x8000003D;

	if (flags.forward_command)
		forward_command = GetCommandID(pReader->Read<WORD>());
	else
		forward_command = 0x41000003;

	if (flags.sidestep_command)
		sidestep_command = GetCommandID(pReader->Read<WORD>());
	else
		sidestep_command = 0;

	if (flags.turn_command)
		turn_command = GetCommandID(pReader->Read<WORD>());
	else
		turn_command = 0;

	if (flags.forward_speed)
		forward_speed = pReader->Read<float>();
	else
		forward_speed = 1.0f;

	if (flags.sidestep_speed)
		sidestep_speed = pReader->Read<float>();
	else
		sidestep_speed = 1.0f;

	if (flags.turn_speed)
		turn_speed = pReader->Read<float>();
	else
		turn_speed = 1.0f;

	for (int i = 0; i < flags.num_actions; i++)
	{
		ActionNode node;
		node.UnPack(pReader);
		actions.push_back(node);
	}

	pReader->ReadAlign();
	return TRUE;
}

MoveToStatePack::~MoveToStatePack()
{
}

void MoveToStatePack::Pack(BinaryWriter *pWriter)
{
	raw_motion_state.Pack(pWriter);
	position.Pack(pWriter);

	pWriter->Write<short>(instance_timestamp);
	pWriter->Write<short>(server_control_timestamp);
	pWriter->Write<short>(teleport_timestamp);
	pWriter->Write<short>(force_position_ts);

	BYTE flags = 0;
	flags |= contact ? 1 : 0;
	flags |= longjump_mode ? 2 : 0;
	pWriter->Write<BYTE>(flags);

	pWriter->Align();
}

bool MoveToStatePack::UnPack(BinaryReader *pReader)
{
	raw_motion_state.UnPack(pReader);
	position.UnPack(pReader);

	instance_timestamp = pReader->Read<short>();
	server_control_timestamp = pReader->Read<short>();
	teleport_timestamp = pReader->Read<short>();
	force_position_ts = pReader->Read<short>();

	BYTE flags = pReader->Read<BYTE>();

	contact = flags & 1;
	longjump_mode = flags & 2;

	pReader->ReadAlign();
	return true;
}

