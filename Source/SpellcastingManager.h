
#pragma once
#include <StdAfx.h>
#include "Movement.h"
#include "MovementManager.h"
#include "PhatSDK.h"

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
	uint32_t caster_id = 0; // not necessarily the same as m_pWeenie (e.g. a gem casting a spell, the player is source)
	uint32_t source_id = 0; // should always match m_pWeenie's ID
	uint32_t target_id = 0;
	uint32_t spell_id = 0;
	uint32_t wand_id = 0; // the CCasterWeenie used to cast the spell if there was one....
	double cast_timeout = FLT_MAX;
	Position initial_cast_position;
	uint32_t power = 0; // the magic power level
	uint32_t power_level_of_power_component = 0; // the scarab level basically
	float max_range = FLT_MAX;
	uint32_t current_skill = 0;
	const class CSpellBase *spell = NULL;
	const class CSpellBaseEx *spellEx = NULL;
	SpellFormula spell_formula;
	ProjectileType proj_type = ProjectileType::Undef;
	bool range_check = true;
	bool uses_mana = true;
	bool equipped = false;
	WORD serial = 0;
	double next_update = Timer::cur_time;
	uint32_t set_id = 0;
};

class CSpellcastingManager
{
public:
	struct SpellCastingMotion
	{
		SpellCastingMotion(uint32_t _motion, float _speed, bool _turns, bool _requiresHeading, float _min_time)
		{
			motion = _motion;
			speed = _speed;
			turns = _turns;
			requiresHeading = _requiresHeading;
			min_time = _min_time;
		}

		uint32_t motion;
		float speed;
		bool turns;
		bool requiresHeading;
		float min_time;
	};

	CSpellcastingManager(class CWeenieObject *pWeenie);
	virtual ~CSpellcastingManager();

	int CreatureBeginCast(uint32_t target_id, uint32_t spell_id);
	int CastSpellInstant(uint32_t target_id, uint32_t spell_id);
	int CastSpellEquipped(uint32_t target_id, uint32_t spell_id, WORD serial, uint32_t set_id = 0, bool silent = false);
	int TryBeginCast(uint32_t target_id, uint32_t spell_id);
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
	int LaunchBoltProjectile(uint32_t wcid);
	int LaunchRingProjectiles(uint32_t wcid);
	void PerformCastParticleEffects();
	void PerformFellowCastParticleEffects(fellowship_ptr_t &fellow);
	int LaunchSpellEffect(bool bFizzled, bool silent = false);
	bool VerifyPkAction(CWeenieObject * target);
	bool DoTransferSpell(CWeenieObject *other, const TransferSpellEx *meta);
	bool AdjustVital(CWeenieObject *target);
	void SendAdjustVitalText(CWeenieObject *target, int amount, const char *vitalName);
	void TransferVitalPercent(CWeenieObject *target, float drainPercent, float infusePercent, STypeAttribute2nd attribute);
	void SendTransferVitalPercentText(CWeenieObject *target, int drained, int infused, bool reversed, const char *vitalName);
	//void SendAdjustedVitalText(CWeenieObject *target, unsigned int amount, STypeAttribute2nd attribute, bool beneficial);
	Position GetSpellProjectileSpawnPosition(CSpellProjectile *pProjectile, CWeenieObject *pTarget, float *pDistToTarget, double dDir, bool bRing);
	Vector GetSpellProjectileSpawnVelocity(Position *pSpawnPosition, CWeenieObject *pTarget, float speed, bool tracked, bool gravity, Vector *pTargetDir, double dDir, bool bRing);
	void Update();
	WErrorType CanPortalRecall(uint32_t portalDID, uint32_t linkedPortalEnum = 0);

	uint32_t DetermineSkillLevelForSpell();
	double DetermineSpellRange();
	int CheckTargetValidity();
	int GenerateManaCost(bool useMod = false);
	bool IsArmorBuff(uint32_t category);

	bool LaunchProjectileSpell(class ProjectileSpellEx *meta);

	void BeginPortalSend(const Position &target);

	void Cancel();
	void OnDeath(uint32_t killer_id);
	void HandleMotionDone(uint32_t motion, BOOL success);

	std::map<uint32_t, uint32_t> FindComponentInContainer(CContainerWeenie *container, unsigned int componentId, int amountNeeded);
	CWeenieObject *FindFociInContainer(CContainerWeenie *container, uint32_t fociWcid);

	CWeenieObject *m_pWeenie;
	bool m_bCasting = false;
	double m_fNextCastTime = 0.0;

	SpellCastData m_SpellCastData;

	std::list<SpellCastingMotion> m_PendingMotions;
	std::map<uint32_t, uint32_t> m_UsedComponents;
	bool m_bTurningToObject = false;
	bool m_bTurned = false;
};
