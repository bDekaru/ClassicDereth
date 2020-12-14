
#pragma once

#include "combat/AttackEventData.h"

//#define CLEAVING_ATTACK_ANGLE 178

class AttackManager
{
public:
	AttackManager(class CWeenieObject *weenie);
	~AttackManager();

	void Update();
	void Cancel();

	void BeginMeleeAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, float chase_distance = 15.0f, uint32_t motion = 0);
	void BeginMissileAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion = 0);

	void BeginAttack(CAttackEventData *data);

	void OnAttackCancelled(uint32_t error = 0);
	void OnAttackDone(uint32_t error = 0);

	bool RepeatAttacks();

	void OnDeath(uint32_t killer_id);
	void HandleMoveToDone(uint32_t error);
	void HandleAttackHook(const AttackCone &cone);
	void OnMotionDone(uint32_t motion, BOOL success);

	bool IsAttacking();

	void MarkForCleanup(CAttackEventData *data);

	// This is not actually used by the system. Revisit later.
	float GetDefenseMod()
	{
		if (_attackData)
			return _attackData->CalculateDef();
		return 1.0f;
	}

private:
	class CWeenieObject *_weenie = NULL;

	double _next_allowed_attack = 0.0;
	CAttackEventData *_attackData = NULL;
	CAttackEventData *_queuedAttackData = NULL;
	CAttackEventData *_cleanupData = NULL;
};