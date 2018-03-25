
#pragma once

#include "ObjCache.h"
#include "Frame.h"
#include "HashData.h"

// Declared elsewhere.
class CPhysicsObj;
class CPhysicsPart;

// Declared here.
class Particle;
class ParticleEmitter;
class ParticleEmitterInfo;
class ParticleManager;

#define MINIMUM_SCALE 0.1f
#define MAXIMUM_SCALE 10.0f

#define MINIMUM_TRANS 0.0f
#define MAXIMUM_TRANS 1.0f

#define MINUMUM_LIFESPAN 0.0f

class Particle
{
public:
	void Init(CPhysicsObj *pOwner, DWORD EmitterID, Frame *pFrame, CPhysicsPart *pPart, Vector *Offset, DWORD Info2C, BOOL Persistant,
		Vector *RandomA, Vector *RandomB, Vector *RandomC, float StartScale, float FinalScale, float StartTrans, float FinalTrans, double Lifespan);

	void Update(DWORD Info2C, BOOL Persistant, CPhysicsPart *pPartObj, Frame *pFrame);

	double m_LastUpdate; // 0x00
	double m_08; // 0x08
	double m_FrameTime; // 0x10
	Frame m_Frame; // 0x18

	Vector m_58;
	Vector m_64;
	Vector m_70;
	Vector m_7C;

	float m_StartScale; // 0x88
	float m_FinalScale; // 0x8C
	float m_StartTrans; // 0x90
	float m_FinalTrans; // 0x94

};

class ParticleEmitter
{
public:
	ParticleEmitter(CPhysicsObj *pOwner);
	~ParticleEmitter();

	static ParticleEmitter *makeParticleEmitter(CPhysicsObj *pOwner);

	void Destroy();

	BOOL SetInfo(DWORD InfoID);
	BOOL SetInfo(ParticleEmitterInfo *pInfo);
	BOOL SetParenting(DWORD b, Frame *c);
	BOOL InitEnd();

	BOOL ShouldEmitParticle();
	BOOL StopEmitter();
	void EmitParticle();
	BOOL KillParticle(long Index);
	void RecordParticleEmission();

	BOOL UpdateParticles();

	DWORD m_EmitterID; // 0x00
	CPhysicsObj *m_Owner; // 0x04
	DWORD m_08; // 0x08

	Frame m_Frame; // 0x0C (size: 0x40)

	CPhysicsObj *m_EmitterObj; // 0x4C
	ParticleEmitterInfo *m_EmitterInfo; // 0x50
	Particle *m_Particles; // 0x54
	CPhysicsPart **m_Parts58; // 0x58
	CPhysicsPart **m_Parts5C; // 0x5C
	DWORD m_60;
	float m_MaxDegradeDist; // 0x64
	double m_68;
	long m_70;
	long m_74;
	double m_78;
	Vector m_Origin; // 0x80
	BOOL m_bStopEmitting; // 0x8C
	double m_90;

	static BOOL always_use_software_particles;
};

class ParticleEmitterInfo : public DBObj
{
public:
	ParticleEmitterInfo();
	~ParticleEmitterInfo();

	static DBObj* Allocator();
	static void Destroyer(DBObj*);
	static ParticleEmitterInfo* Get(DWORD ID);
	static void Release(ParticleEmitterInfo *);

	BOOL UnPack(BYTE **ppData, ULONG iSize);

	float GetRandomStartScale();
	float GetRandomFinalScale();
	float GetRandomStartTrans();
	float GetRandomFinalTrans();
	float GetRandomLifespan();

	Vector *GetRandomA(Vector *pVector);
	Vector *GetRandomB(Vector *pVector);
	Vector *GetRandomC(Vector *pVector);
	Vector *GetRandomOffset(Vector *pVector);

	BOOL IsPersistant();
	BOOL ShouldEmitParticle(long, long, Vector *, double);

	DWORD m_28;
	DWORD m_2C;

	DWORD m_30;

	DWORD m_34;
	DWORD m_38;

	double m_40;

	long m_48;
	long m_4C;
	long m_50;

	double m_58;
	double m_60;
	double m_68;

	// Could be, sphere??
	CSphere m_70;

	Vector m_80;

	float m_8C;
	float m_90;

	Vector m_94;
	Vector m_A0;
	Vector m_AC;

	float m_B8;
	float m_BC;

	float m_C0;
	float m_C4;

	float m_C8;
	float m_CC;

	float m_D0;

	float m_D4;
	float m_D8;

	float m_DC;

	float m_E0;
	float m_E4;
};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	DWORD CreateParticleEmitter(CPhysicsObj *pOwner, DWORD a, long b, Frame *c, DWORD d);

	void UpdateParticles();

private:
	DWORD m_NoIDEmitters; // 0x00
	LongNIHash<ParticleEmitter> m_Emitters; // 0x04
};








