#pragma once
#define CLEAVING_ATTACK_ANGLE 178

const double DISTANCE_REQUIRED_FOR_MELEE_ATTACK = 1.0;
const double MAX_MELEE_ATTACK_CONE_ANGLE = 15.0;
const double MAX_MISSILE_ATTACK_CONE_ANGLE = 3.0;

#define MISSILE_SLOW_SPEED 0.9f
#define MISSILE_FAST_SPEED 1.25f

class CAttackEventData
{
public:
	CAttackEventData() noexcept;
	virtual ~CAttackEventData() = default;

	virtual void Setup();
	void Begin();

	virtual void PostCharge();
	void CheckTimeout();
	void CancelMoveTo();

	bool IsValidTarget();

	virtual void OnMotionDone(uint32_t motion, BOOL success);
	virtual void OnAttackAnimSuccess(uint32_t motion);

	void ExecuteAnimation(uint32_t motion, MovementParameters *params = NULL);
	double DistanceToTarget();
	double HeadingToTarget(bool relative = true);
	bool InAttackRange();
	bool InAttackCone();

	CWeenieObject *GetTarget();
	void MoveToAttack();
	void TurnToAttack();
	virtual void HandleMoveToDone(uint32_t error);
	virtual void HandleAttackHook(const AttackCone &cone) { }
	virtual void OnReadyToAttack() = 0;

	virtual void Update();
	virtual void Cancel(uint32_t error = 0);
	virtual void Done(uint32_t error = 0);

	// This is not actually used by the system. Revisit later.
	virtual float CalculateDef() { return 1.0f; }

	virtual float AttackTimeMod() { return 1.0f; }
	virtual float AttackSpeedMod() { return 1.0f; }
	virtual bool ShouldNotifyAttackDone() { return true; }

	virtual class CMeleeAttackEvent *AsMeleeAttackEvent() { return NULL; }
	virtual class CMissileAttackEvent *AsMissileAttackEvent() { return NULL; }

	class AttackManager *_manager = NULL;
	class CWeenieObject *_weenie = NULL;

	uint32_t _target_id = 0;
	bool _move_to = false;
	bool _turn_to = false;
	bool _use_sticky = true;
	double _max_attack_distance = FLT_MAX;
	double _max_attack_angle = FLT_MAX;
	double _timeout = FLT_MAX;
	uint32_t _active_attack_anim = 0;
	ATTACK_HEIGHT _attack_height = ATTACK_HEIGHT::UNDEF_ATTACK_HEIGHT;
	float _attack_power = 0.0f;
	float _attack_speed = 1.5f;
	float _fail_distance = 15.0f;
	double _attack_charge_time = -1.0f;

	bool m_bCanCharge = false;
};
