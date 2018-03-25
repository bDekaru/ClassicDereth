
#pragma once

#include "WeenieObject.h"
#include "SpellcastingManager.h"

class CSpellProjectile : public CWeenieObject
{
public:
	CSpellProjectile(const SpellCastData &scd, DWORD target_id);//, unsigned int damage);
	virtual ~CSpellProjectile();

	virtual class CSpellProjectile *AsSpellProjectile() { return this; }

	virtual void Tick() override;
	virtual void PostSpawn() override;
	virtual int DoCollision(const class EnvCollisionProfile &prof) override;
	virtual int DoCollision(const class AtkCollisionProfile &prof) override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;
	virtual void DoCollisionEnd(DWORD object_id) override;

	virtual DWORD GetPhysicsTargetID() override { return m_TargetID; }

public:
	DWORD m_SourceID;
	DWORD m_TargetID;
	//unsigned int m_Damage;

private:
	void HandleExplode();

	SpellCastData m_CachedSpellCastData;
	float m_fEffectMod = 1.0f;
	double m_fSpawnTime = 0.0;
	double m_fDestroyTime = 0.0;

	bool isLifeProjectile = false;
	int selfDrainedAmount = 0;
	float selfDrainedDamageRatio = 0.0;
};



