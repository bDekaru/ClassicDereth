
#pragma once


class CAttackEventData
{
public:
	CAttackEventData();

	virtual void Setup();
	void Begin();

	void PostCharge();
	void CheckTimeout();
	void CancelMoveTo();

	bool IsValidTarget();

	virtual void OnMotionDone(DWORD motion, BOOL success);
	virtual void OnAttackAnimSuccess(DWORD motion);

	void ExecuteAnimation(DWORD motion, MovementParameters *params = NULL);
	double DistanceToTarget();
	double HeadingToTarget(bool relative = true);
	bool InAttackRange();
	bool InAttackCone();

	CWeenieObject *GetTarget();
	void MoveToAttack();

	virtual void HandleMoveToDone(DWORD error);
	virtual void HandleAttackHook(const AttackCone &cone) { }
	virtual void OnReadyToAttack() = 0;

	virtual void Update();
	virtual void Cancel(DWORD error = 0);
	virtual void Done(DWORD error = 0);

	virtual class CMeleeAttackEvent *AsMeleeAttackEvent() { return NULL; }
	virtual class CMissileAttackEvent *AsMissileAttackEvent() { return NULL; }

	class AttackManager *_manager = NULL;
	class CWeenieObject *_weenie = NULL;

	DWORD _target_id = 0;
	bool _move_to = false;
	bool _turn_to = false;
	bool _use_sticky = true;
	double _max_attack_distance = FLT_MAX;
	double _max_attack_angle = FLT_MAX;
	double _timeout = FLT_MAX;
	DWORD _active_attack_anim = 0;
	ATTACK_HEIGHT _attack_height = ATTACK_HEIGHT::UNDEF_ATTACK_HEIGHT;
	float _attack_power = 0.0f;
	float _attack_speed = 1.5f;
	float _fail_distance = 15.0f;
	double _attack_charge_time = -1.0f;
};

class CMeleeAttackEvent : public CAttackEventData
{
public:
	virtual void Setup() override;

	virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(DWORD motion) override;
	void Finish();

	virtual void HandleAttackHook(const AttackCone &cone) override;

	virtual class CMeleeAttackEvent *AsMeleeAttackEvent() { return NULL; }
	
	DWORD _do_attack_animation = 0;
};

class CAIMeleeAttackEvent : public CAttackEventData
{
public:
	virtual void Setup() override;

	virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(DWORD motion) override;
	void Finish();

	virtual void HandleAttackHook(const AttackCone &cone) override;

	DWORD _do_attack_animation = 0;
};

class CMissileAttackEvent : public CAttackEventData
{
public:
	virtual void Setup() override;

	virtual void OnReadyToAttack() override;
	virtual void OnAttackAnimSuccess(DWORD motion) override;
	void Finish();

	virtual void HandleAttackHook(const AttackCone &cone) override;

	void FireMissile();

	bool CalculateTargetPosition();
	bool CalculateSpawnPosition(float missileRadius);	
	bool CalculateMissileVelocity(bool track = true, bool gravity = true, float speed = 20.0f);

	virtual class CMissileAttackEvent *AsMissileAttackEvent() { return this; }

	DWORD _do_attack_animation = 0;

	Position _missile_spawn_position;
	Position _missile_target_position;
	Vector _missile_velocity;
	float _missile_dist_to_target = 0.0f;
};

class AttackManager
{
public:
	AttackManager(class CWeenieObject *weenie);
	~AttackManager();

	void Update();
	void Cancel();

	void BeginMeleeAttack(DWORD target_id, ATTACK_HEIGHT height, float power, float chase_distance = 15.0f, DWORD motion = 0);
	void BeginMissileAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion = 0);

	void BeginAttack(CAttackEventData *data);

	void OnAttackCancelled(DWORD error = 0);
	void OnAttackDone(DWORD error = 0);

	bool RepeatAttacks();

	void OnDeath(DWORD killer_id);
	void HandleMoveToDone(DWORD error);
	void HandleAttackHook(const AttackCone &cone);
	void OnMotionDone(DWORD motion, BOOL success);

	bool IsAttacking();

	void MarkForCleanup(CAttackEventData *data);

private:
	class CWeenieObject *_weenie = NULL;

	double _next_allowed_attack = 0.0;
	CAttackEventData *_attackData = NULL;
	CAttackEventData *_queuedAttackData = NULL;
	CAttackEventData *_cleanupData = NULL;
};