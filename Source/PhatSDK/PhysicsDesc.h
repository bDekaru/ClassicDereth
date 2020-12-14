
#pragma once

#include "BinaryReader.h"

// All of this is temporary to avoid re-writing the wheel for now. From aclogviewer.

class ChildInfo
{
public:
	uint32_t id;
	uint32_t location_id;

	static ChildInfo read(BinaryReader &binaryReader) {
		ChildInfo newObj;
		newObj.id = binaryReader.ReadUInt32();
		newObj.location_id = binaryReader.ReadUInt32();
		return newObj;
	}
};

class PhysicsDesc
{
public:
	enum PhysicsDescInfo {
		CSETUP = (1 << 0), // 0x1
		MTABLE = (1 << 1), // 0x2
		VELOCITY = (1 << 2), // 0x4
		ACCELERATION = (1 << 3), // 0x8
		OMEGA = (1 << 4), // 0x10
		PARENT = (1 << 5), // 0x20
		CHILDREN = (1 << 6), // 0x40
		OBJSCALE = (1 << 7), // 0x80
		FRICTION = (1 << 8), // 0x100
		ELASTICITY = (1 << 9), // 0x200
		TIMESTAMPS = (1 << 10), // 0x400
		STABLE = (1 << 11), // 0x800
		PETABLE = (1 << 12), // 0x1000
		DEFAULT_SCRIPT = (1 << 13), // 0x2000
		DEFAULT_SCRIPT_INTENSITY = (1 << 14), // 0x4000
		POSITION = (1 << 15), // 0x8000
		MOVEMENT = (1 << 16), // 0x10000
		ANIMFRAME_ID = (1 << 17), // 0x20000
		TRANSLUCENCY = (1 << 18) // 0x40000
	};

	uint32_t bitfield;
	PhysicsState state;
	BYTE *movement_buffer;
	uint32_t movement_buffer_length;
	uint32_t autonomous_movement;
	uint32_t animframe_id;
	Position pos;
	uint32_t mtable_id; // These are tag ids like animpartchange
	uint32_t stable_id;
	uint32_t phstable_id;
	uint32_t setup_id;
	uint32_t parent_id;
	uint32_t location_id;
	std::list<ChildInfo> children;
	float object_scale;
	float friction;
	float elasticity;
	float translucency;
	Vector velocity;
	Vector acceleration;
	Vector omega;
	PScriptType default_script;
	float default_script_intensity;
	uint16_t timestamps[9];

	PhysicsDesc()
	{
		state = (PhysicsState)0;
		movement_buffer = NULL;
		movement_buffer_length = 0;
		autonomous_movement = 0;
		animframe_id = 0;
		
		mtable_id = 0;
		stable_id = 0;
		phstable_id = 0;
		setup_id = 0;
		parent_id = 0;
		location_id = 0;
		object_scale = 1.0f;
		friction = 0.0f;
		elasticity = 0.0f;
		translucency = 0.0f;
		default_script = (PScriptType)0;
		default_script_intensity = 0.0f;
	}

	~PhysicsDesc()
	{
		freeData();
	}

	void freeData()
	{
		if (movement_buffer)
		{
			delete[] movement_buffer;
			movement_buffer = NULL;
		}
	}

	void Unpack(BinaryReader &binaryReader) {
		PhysicsDesc &newObj = *this;
		newObj.bitfield = binaryReader.ReadUInt32();
		newObj.state = (PhysicsState)binaryReader.ReadUInt32();

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::MOVEMENT) != 0) {
			uint32_t buff_length = binaryReader.ReadUInt32();
			if (buff_length > 0)
			{
				if (newObj.movement_buffer)
					delete[] newObj.movement_buffer;
				newObj.movement_buffer = new BYTE[buff_length];
				memcpy(newObj.movement_buffer, binaryReader.ReadArray((int)buff_length), buff_length);
				newObj.movement_buffer_length = buff_length;
			}

			newObj.autonomous_movement = binaryReader.ReadUInt32();
		}
		else if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::ANIMFRAME_ID) != 0) {
			newObj.animframe_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::POSITION) != 0) {
			newObj.pos.UnPack(&binaryReader);
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::MTABLE) != 0) {
			newObj.mtable_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::STABLE) != 0) {
			newObj.stable_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::PETABLE) != 0) {
			newObj.phstable_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::CSETUP) != 0) {
			newObj.setup_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::PARENT) != 0) {
			newObj.parent_id = binaryReader.ReadUInt32();
			newObj.location_id = binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::CHILDREN) != 0) {
			uint32_t num_children = binaryReader.ReadUInt32();
			for (uint32_t i = 0; i < num_children; ++i) {
				newObj.children.push_back(ChildInfo::read(binaryReader));
			}
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::OBJSCALE) != 0) {
			newObj.object_scale = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::FRICTION) != 0) {
			newObj.friction = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::ELASTICITY) != 0) {
			newObj.elasticity = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::TRANSLUCENCY) != 0) {
			newObj.translucency = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::VELOCITY) != 0) {
			newObj.velocity.x = binaryReader.ReadSingle();
			newObj.velocity.y = binaryReader.ReadSingle();
			newObj.velocity.z = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::ACCELERATION) != 0) {
			newObj.acceleration.x = binaryReader.ReadSingle();
			newObj.acceleration.y = binaryReader.ReadSingle();
			newObj.acceleration.z = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::OMEGA) != 0) {
			newObj.omega.x = binaryReader.ReadSingle();
			newObj.omega.y = binaryReader.ReadSingle();
			newObj.omega.z = binaryReader.ReadSingle();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::DEFAULT_SCRIPT) != 0) {
			newObj.default_script = (PScriptType)binaryReader.ReadUInt32();
		}

		if ((newObj.bitfield & (uint32_t)PhysicsDescInfo::DEFAULT_SCRIPT_INTENSITY) != 0) {
			newObj.default_script_intensity = binaryReader.ReadSingle();
		}

		for (int i = 0; i < 9; ++i) {
			newObj.timestamps[i] = binaryReader.ReadUInt16();
		}

		binaryReader.ReadAlign();
	}
};

