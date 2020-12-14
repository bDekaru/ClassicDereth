#pragma once

#include "combat/AttackEventData.h"

class CMeleeAttackEvent : public CAttackEventData
{
public:
	CMeleeAttackEvent() : _combat_style(COMBAT_USE_MELEE) { }

	virtual void Setup() override;

	virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(uint32_t motion) override;
	void Finish();

	virtual void HandleAttackHook(const AttackCone &cone) override;
	void HandlePerformAttack(CWeenieObject *target, DamageEventData dmgEvent);

	//virtual float CalculateDef() override;

	virtual class CMeleeAttackEvent *AsMeleeAttackEvent() override { return this; }

	uint32_t _do_attack_animation = 0;

protected:
	CMeleeAttackEvent(COMBAT_USE style) : _combat_style(style) { }

	virtual void CalculateAtt(CWeenieObject *weapon, STypeSkill& weaponSkill, uint32_t& weaponSkillLevel);

	COMBAT_USE _combat_style;
};

class CTwoHandAttackEvent : public CMeleeAttackEvent
{
public:
	CTwoHandAttackEvent() : CMeleeAttackEvent(COMBAT_USE_TWO_HANDED) { }
};

//class CAIMeleeAttackEvent : public CAttackEventData
//{
//public:
//	virtual void Setup() override;
//
//	virtual void OnReadyToAttack() override;
//	virtual void OnAttackAnimSuccess(uint32_t motion) override;
//	void Finish();
//
//	virtual void HandleAttackHook(const AttackCone &cone) override;
//
//	uint32_t _do_attack_animation = 0;
//};
