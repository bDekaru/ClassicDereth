
#pragma once

class CWeenieObject;
class CMonsterWeenie;

// tolerance = always attack if in range
// tolerance = 1, never attack
// tolerance = 2, attack after attacked or identified
// tolerance = 0x40, attack after provoked, only the person that attacked

enum ToleranceEnum
{
	TolerateNothing = 0,
	TolerateEverything = 1,
	TolerateUnlessBothered = 2, // ID'd or attacked
	TolerateUnlessAttacked = 0x40,
};

enum MonsterAIState
{
	Idle,
	ReturningToSpawn,
	SeekNewTarget,
	MeleeModeAttack,
	MissileModeAttack,
	MagicModeAttack
};

class MonsterAIManager
{
public:
	MonsterAIManager(CMonsterWeenie *pWeenie, const Position &HomePos);
	virtual ~MonsterAIManager();

	void SetHomePosition(const Position &pos);
	void Update();

	void SwitchState(int state);
	void EnterState(int state);

	void BeginIdle();
	void UpdateIdle();
	void EndIdle();

	void BeginReturningToSpawn();
	void UpdateReturningToSpawn();
	void EndReturningToSpawn();

	void BeginSeekNewTarget();
	void UpdateSeekNewTarget();
	void EndSeekNewTarget();

	void BeginMeleeModeAttack();
	void EndMeleeModeAttack();
	void UpdateMeleeModeAttack();

	void BeginMissileModeAttack();
	void EndMissileModeAttack();
	void UpdateMissileModeAttack();

	CWeenieObject *GetTargetWeenie();
	void SetNewTarget(CWeenieObject *pTarget);

	bool SeekTarget();

	bool IsValidTarget(CWeenieObject *pWeenie);
	void OnDeath();
	void OnDealtDamage(DamageEventData &damageData);
	void OnTookDamage(DamageEventData &damageData);
	void OnResistSpell(CWeenieObject *attacker);
	void OnEvadeAttack(CWeenieObject *attacker);

	void AlertIdleFriendsToAggro(CWeenieObject *pAttacker);

	void OnIdentifyAttempted(CWeenieObject *other);
	void HandleAggro(CWeenieObject *pAttacker);

	float DistanceToHome();
	bool ShouldSeekNewTarget();
	
	bool RollDiceCastSpell();
	bool DoCastSpell(uint32_t spell_id);
	bool DoMeleeAttack();

	void GenerateRandomAttack(uint32_t *motion, ATTACK_HEIGHT *height, float *power, CWeenieObject *weapon = NULL);
	float GetChaseDistance() { return m_fChaseRange; }

	CMonsterWeenie *m_pWeenie = NULL;

	Position m_HomePosition;
	double m_fAwarenessRange = 40.0f;
	double m_fChaseRange = 100.0f;
	double m_fMaxHomeRange = 150.0f;
	double m_fMinReturnStateDuration = 20.0;
	double m_fMinCombatStateDuration = 10.0;
	double m_fChaseTimeoutDuration = 30.0;
	double m_fMeleeAttackRange = 3.0;
	double m_fReturnTimeout = 30.0f;

	CWeenieObject *_currentWeapon = NULL;
	CWeenieObject *_currentShield = NULL;
	CWeenieObject *_shield = NULL;
	CWeenieObject *_meleeWeapon = NULL;
	CWeenieObject *_missileWeapon = NULL;
	bool _hasUnarmedSkill = false;
	double _nextTaunt = -1.0;

	int m_State = MonsterAIState::Idle;

	// to check for targets nearby
	double m_fNextPVSCheck = 0.0;

	double m_fChaseTimeoutTime = 0.0;
	double m_fNextChaseTime = 0.0;
	double m_fNextAttackTime = 0.0;
	double m_fMinReturnStateTime = 0.0;
	double m_fMinCombatStateTime = 0.0;
	uint32_t m_TargetID = 0;

	double m_fReturnTimeoutTime = 0.0;

	double m_fAggroTime = 0.0;
	double m_fNextCastTime = 0.0;

	double m_fLastWoundedTauntHP = 1.0;

	int _toleranceType = 0;
	int _aiOptions = 0;

	double _cachedVisualAwarenessRange = 0.0;
};

extern bool monster_brawl;