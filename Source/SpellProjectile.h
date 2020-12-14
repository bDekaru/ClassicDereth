
#pragma once

#include "WeenieObject.h"
#include "SpellcastingManager.h"

class CSpellProjectile : public CWeenieObject
{
public:
	CSpellProjectile(const SpellCastData &scd, uint32_t target_id);//, unsigned int damage);
	virtual ~CSpellProjectile();

	virtual class CSpellProjectile *AsSpellProjectile() override { return this; }

	virtual void Tick() override;
	virtual void PostSpawn() override;
	virtual int DoCollision(const class EnvCollisionProfile &prof) override;
	virtual int DoCollision(const class AtkCollisionProfile &prof) override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;
	virtual void DoCollisionEnd(uint32_t object_id) override;

	virtual uint32_t GetPhysicsTargetID() override { return m_TargetID; }

	void makeLifeProjectile(int selfDrainedAmount, float selfDrainedDamageRatio);
	void makeEnchantProjectile() { isEnchantProjectile = true; };

public:
	uint32_t m_SourceID;
	uint32_t m_TargetID;
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

	bool isEnchantProjectile = false;
};



