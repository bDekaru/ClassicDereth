
#pragma once

class CombatManeuver : public PackObj
{
public:
	CombatManeuver();
	virtual ~CombatManeuver();

	DECLARE_PACKABLE();

	unsigned int style;
	ATTACK_HEIGHT attack_height;
	AttackType attack_type;
	unsigned int min_skill_level;
	unsigned int motion;
};

class CombatManeuverTable : public PackObj, public DBObj
{
public:
	CombatManeuverTable();
	virtual ~CombatManeuverTable();

	DECLARE_DBOBJ(CombatManeuverTable);
	DECLARE_PACKABLE();
	DECLARE_LEGACY_PACK_MIGRATOR();

	void Clear();

	CombatManeuver *TryGetCombatManuever(uint32_t style, AttackType at, ATTACK_HEIGHT height); // custom

	//void MeleeAttackToMotion();
	//void MotionToDamageType();
	//void DetermineDamageTypePrecedence();
	//void FiringAngleToMotion();
	uint32_t CombatStyleToMotion(CombatStyle style);
	COMBAT_MODE CombatStyleToCombatMode(CombatStyle style);

	unsigned int _num_combat_maneuvers = 0;
	CombatManeuver *_cmt = NULL;
};

class CombatSystem
{
public:
	static BOOL InqCombatHitAdjectives(DAMAGE_TYPE damage_type, const double php, std::string &single_adj, std::string &plural_adj);
};

