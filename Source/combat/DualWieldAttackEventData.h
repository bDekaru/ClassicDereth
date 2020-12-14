#pragma once

#include "GameEnums.h"
#include "combat/AttackEventData.h"
#include "combat/MeleeAttackEventData.h"

class CDualWieldAttackEvent : public CMeleeAttackEvent
{
public:
	CDualWieldAttackEvent() :
		CMeleeAttackEvent(COMBAT_USE_MELEE),
		_left_hand(0), _main_attack_motion(0), _offhand_attack_motion(0)
	{ }

	virtual void Setup() override;

	//virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(uint32_t motion) override;
	virtual float AttackTimeMod() override { return 0.8f; }
	virtual float AttackSpeedMod() override { return 1.2f; }
	//void Finish();

	//virtual void HandleAttackHook(const AttackCone &cone) override;
	//virtual float CalculateDef() override;

	//void HandlePerformAttack(CWeenieObject *target, DamageEventData dmgEvent);

protected:
	virtual void CalculateAtt(CWeenieObject *weapon, STypeSkill& weaponSkill, uint32_t& weaponSkillLevel) override;

	int _left_hand;

	int _main_attack_motion;
	int _offhand_attack_motion;
};
