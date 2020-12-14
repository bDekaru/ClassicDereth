#pragma once

#include "combat/AttackEventData.h"

class CMissileAttackEvent : public CAttackEventData
{
public:
	virtual void Setup() override;
	virtual void PostCharge() override;
	virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(uint32_t motion) override;
	void Finish();

	virtual void HandleAttackHook(const AttackCone &cone) override;

	void FireMissile();

	void CalculateAttackMotion();
	bool CalculateTargetPosition();
	bool CalculateSpawnPosition(float missileRadius);
	bool CalculateMissileVelocity(bool track = true, bool gravity = true, float speed = 20.0f);

	float CalculateDef() override;

	virtual bool ShouldNotifyAttackDone() override { return false; }

	virtual class CMissileAttackEvent *AsMissileAttackEvent() override { return this; }

	uint32_t _do_attack_animation = 0;

	Position _missile_spawn_position;
	Position _missile_target_position;
	Vector _missile_velocity;
	float _missile_dist_to_target = 0.0f;
	bool m_bTurned = false;
};
