
#pragma once

#include "LegacyPackObj.h"
#include "MathLib.h"
#include "Frame.h"
#include "ObjDesc.h"

class CAnimHook
{
public:
	CAnimHook();
	virtual ~CAnimHook();

	static CAnimHook *UnPackHook(BYTE** ppData, ULONG iSize);

	virtual void Execute(CPhysicsObj *pOwner) = 0;
	virtual int GetType() = 0;
	virtual void GetDataRecursion(LPVOID);
	virtual ULONG pack_size() = 0;
	virtual ULONG Pack(BYTE** ppData, ULONG iSize) = 0;
	virtual BOOL UnPack(BYTE** ppData, ULONG iSize) = 0;

	// Hooks will form a list.
	void add_to_list(CAnimHook** pList);

	enum AnimHookDir
	{
		UNKNOWN_ANIMHOOK = 0xFFFFFFFE,
		BACKWARD_ANIMHOOK = 0xFFFFFFFF,
		BOTH_ANIMHOOK = 0x0,
		FORWARD_ANIMHOOK = 0x1,
		FORCE_AnimHookDir_32_BIT = 0x7FFFFFFF,
	};

	CAnimHook * next_hook; // 0x04
	AnimHookDir direction_; // 0x08
};

class AnimFrame
{
public:
	AnimFrame();
	~AnimFrame();

	void Destroy();
	BOOL UnPack(uint32_t ObjCount, BYTE **ppData, ULONG iSize);

	AFrame *frame;
	uint32_t num_frame_hooks;
	CAnimHook *hooks; // linked-list of hookable events
	uint32_t num_parts;
};

/*
class AnimPartChange : public LegacyPackObj
{
public:
	AnimPartChange();

	BOOL UnPack(BYTE **ppData, ULONG iSize);
	BOOL replaces(AnimPartChange *pChange);

	uint32_t part_index;
	uint32_t part_id;
	AnimPartChange *prev;
	AnimPartChange *next;
};
*/

class NOOPHook : public CAnimHook
{
public:
	NOOPHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);
};

class NoDrawHook : public CAnimHook
{
public:
	NoDrawHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	BOOL _no_draw;
};

class DefaultScriptHook : public CAnimHook
{
public:
	DefaultScriptHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);
};

class DefaultScriptPartHook : public CAnimHook
{
public:
	DefaultScriptPartHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t _part_index;
};

class SoundHook : public CAnimHook
{
public:
	SoundHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	void GetDataRecursion(LPVOID);
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t gid_;
};

class SoundTableHook : public CAnimHook
{
public:
	SoundTableHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t sound_type_;
};

class AnimDoneHook : public CAnimHook
{
public:
	AnimDoneHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);
};

class ReplaceObjectHook : public CAnimHook
{
public:
	ReplaceObjectHook();
	~ReplaceObjectHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	void GetDataRecursion(LPVOID);
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	// This should get the job done.
	AnimPartChange ap_change;
};

class TransparentHook : public CAnimHook
{
public:
	TransparentHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	float start;
	float end;
	float time;
};

class TransparentPartHook : public CAnimHook
{
public:
	TransparentPartHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t part;
	float start;
	float end;
	float time;
};

class LuminousPartHook : public CAnimHook
{
public:
	LuminousPartHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t part;
	float start;
	float end;
	float time;
};

class LuminousHook : public CAnimHook
{
public:
	LuminousHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	float start;
	float end;
	float time;
};

class DiffusePartHook : public CAnimHook
{
public:
	DiffusePartHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t part;
	float start;
	float end;
	float time;
};

class DiffuseHook : public CAnimHook
{
public:
	DiffuseHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	float start;
	float end;
	float time;
};

class ScaleHook : public CAnimHook
{
public:
	ScaleHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	float end;
	float time;
};

class CreateParticleHook : public CAnimHook
{
public:
	CreateParticleHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	void GetDataRecursion(LPVOID);
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t emitter_info_id;
	uint32_t part_index;
	Frame offset;
	uint32_t emitter_id;
};

class CreateBlockingParticleHook : public CreateParticleHook
{
public:
	CreateBlockingParticleHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
};

class DestroyParticleHook : public CAnimHook
{
public:
	DestroyParticleHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t emitter_id;
};

class StopParticleHook : public CAnimHook
{
public:
	StopParticleHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t emitter_id;
};

class CallPESHook : public CAnimHook
{
public:
	CallPESHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	void GetDataRecursion(LPVOID);
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t pes;
	float pause;
};

class SoundTweakedHook : public CAnimHook
{
public:
	SoundTweakedHook();
	~SoundTweakedHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	void GetDataRecursion(LPVOID);
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t gid_;
	float prio;
	float prob;
	float vol;
};

class SetOmegaHook : public CAnimHook
{
public:
	SetOmegaHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	Vector axis;
};

class TextureVelocityHook : public CAnimHook
{
public:
	TextureVelocityHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	float u_speed;
	float v_speed;
};

class TextureVelocityPartHook : public CAnimHook
{
public:
	TextureVelocityPartHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t part_index;
	float u_speed;
	float v_speed;
};

class SetLightHook : public CAnimHook
{
public:
	SetLightHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t _lights_on;
};

// Need to be moved elsewhere!
class AttackCone
{
public:
	AttackCone();

	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t part_index;
	Vec2D left;
	Vec2D right;
	float radius;
	float height;
};

class AttackHook : public CAnimHook
{
public:
	AttackHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	AttackCone m_cone;
};

class EtherealHook : public CAnimHook
{
public:
	EtherealHook();

	void Execute(CPhysicsObj *pOwner);
	int GetType();
	ULONG pack_size();
	ULONG Pack(BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	uint32_t ethereal;
};
