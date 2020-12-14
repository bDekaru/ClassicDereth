
#pragma once

#ifdef PHATSDK_USE_WEENIE_STUB
class CWeenieObject
{
public:
	virtual BOOL _IsPlayer() { return FALSE; } // 0x10
	virtual bool IsThePlayer() { return FALSE; } // 0x14

	virtual BOOL IsPK() { return FALSE; } // 0x20
	virtual BOOL IsPKLite() { return FALSE; } // 0x24
	virtual BOOL IsImpenetrable() { return FALSE; } // 0x28
	virtual bool IsCreature() { return true; } // 0x2C
	virtual bool InqJumpVelocity(float extent, float &vz) { vz = sqrt(MovementSystem::GetJumpHeight(1.0f, 100, 1.0f, 1.0f) * 19.6); return true; } // 0x30
	virtual bool InqRunRate(float &rate) { return false; } // 0x34
	virtual bool CanJump(float extent) { return true; }
	virtual bool JumpStaminaCost(float, int32_t &val) { val = 0; return true; } // 0x40

	virtual int DoCollision(const class EnvCollisionProfile &prof) { return 0; }
	virtual int DoCollision(const class ObjCollisionProfile &prof) { return 0; }
	virtual int DoCollision(const class AtkCollisionProfile &prof) { return 0; }
	virtual void DoCollisionEnd(uint32_t object_id) { }

	virtual int InqCollisionProfile(ObjCollisionProfile &prof) { return 0; } // 0x5C

	virtual bool IsStorage() { return false; }
	virtual bool IsCorpse() { return false; }

	virtual uint32_t GetPhysicsTargetID() { return 0; }
	virtual void HandleMoveToDone(uint32_t error) { }

	virtual void HitGround(float zv) { }
};
#endif