
#pragma once

#include "Movement.h"
#include "MovementManager.h"

class CWeenieObject;
class CSpellProjectile;

enum ProjectileType
{
	Undef = 0,
	Bolt,
	Streak,
	BoltGravity,
	Ring,
	Grenade
};

struct SpellCastData
{
	DWORD caster_id = 0; // not necessarily the same as m_pWeenie (e.g. a gem casting a spell, the player is source)
	DWORD source_id = 0; // should always match m_pWeenie's ID
	DWORD target_id = 0;
	DWORD spell_id = 0;
	DWORD wand_id = 0; // the CCasterWeenie used to cast the spell if there was one....
	double cast_timeout = FLT_MAX;
	Position initial_cast_position;
	DWORD power = 0; // the magic power level
	DWORD power_level_of_power_component = 0; // the scarab level basically
	float max_range = FLT_MAX;
	DWORD current_skill = 0;
	const class CSpellBase *spell = NULL;
	const class CSpellBaseEx *spellEx = NULL;
	SpellFormula spell_formula;
	ProjectileType proj_type = ProjectileType::Undef;
	bool range_check = true;
	bool uses_mana = true;
	bool equipped = false;
	WORD serial = 0;
};

class CSpellcastingManager
{
public:
	struct SpellCastingMotion
	{
		SpellCastingMotion(DWORD _motion, float _speed, bool _turns, bool _requiresHeading, float _min_time)
		{
			motion = _motion;
			speed = _speed;
			turns = _turns;
			requiresHeading = _requiresHeading;
			min_time = _min_time;
		}

		DWORD motion;
		float speed;
		bool turns;
		bool requiresHeading;
		float min_time;
	};

	CSpellcastingManager(class CWeenieObject *pWeenie);
	virtual ~CSpellcastingManager();

	int CreatureBeginCast(DWORD target_id, DWORD spell_id);
	int CastSpellInstant(DWORD target_id, DWORD spell_id);
	int CastSpellEquipped(DWORD target_id, DWORD spell_id, WORD serial);
	int TryBeginCast(DWORD target_id, DWORD spell_id);
	void BeginCast();
	void EndCast(int error);
	bool MotionRequiresHeading();
	bool AddMotionsForSpell();
	bool ResolveSpellBeingCasted();
	CWeenieObject *GetCastTarget();
	CWeenieObject *GetCastCaster();
	CWeenieObject *GetCastSource();
	float HeadingToTarget();
	void BeginNextMotion();
	int LaunchBoltProjectile(DWORD wcid);
	int LaunchRingProjectiles(DWORD wcid);
	void PerformCastParticleEffects();
	int LaunchSpellEffect();
	bool DoTransferSpell(CWeenieObject *other, const TransferSpellEx *meta);
	bool AdjustVital(CWeenieObject *target);
	void SendAdjustVitalText(CWeenieObject *target, int amount, const char *vitalName);
	void TransferVitalPercent(CWeenieObject *target, float drainPercent, float infusePercent, STypeAttribute2nd attribute);
	void SendTransferVitalPercentText(CWeenieObject *target, int drained, int infused, bool reversed, const char *vitalName);
	//void SendAdjustedVitalText(CWeenieObject *target, unsigned int amount, STypeAttribute2nd attribute, bool beneficial);
	Position GetSpellProjectileSpawnPosition(CSpellProjectile *pProjectile, CWeenieObject *pTarget, float *pDistToTarget);
	Vector GetSpellProjectileSpawnVelocity(Position *pSpawnPosition, CWeenieObject *pTarget, float speed, bool tracked, bool gravity, Vector *pTargetDir);
	void Update();

	DWORD DetermineSkillLevelForSpell();
	double DetermineSpellRange();
	int CheckTargetValidity();
	int GenerateManaCost();

	bool LaunchProjectileSpell(class ProjectileSpellEx *meta);

	void BeginPortalSend(const Position &target);

	void Cancel();
	void OnDeath(DWORD killer_id);
	void HandleMotionDone(DWORD motion, BOOL success);

	std::map<DWORD, DWORD> FindComponentInContainer(CContainerWeenie *container, unsigned int componentId, int amountNeeded);
	CWeenieObject *FindFociInContainer(CContainerWeenie *container, DWORD fociWcid);

	CWeenieObject *m_pWeenie;
	bool m_bCasting = false;
	double m_fNextCastTime = 0.0;

	SpellCastData m_SpellCastData;

	std::list<SpellCastingMotion> m_PendingMotions;
	std::map<DWORD, DWORD> m_UsedComponents;
	bool m_bTurningToObject = false;
};