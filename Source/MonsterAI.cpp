
#include "StdAfx.h"
#include "MonsterAI.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "World.h"
#include "SpellcastingManager.h"
#include "EmoteManager.h"

#define DEFAULT_AWARENESS_RANGE 40.0

MonsterAIManager::MonsterAIManager(CMonsterWeenie *pWeenie, const Position &HomePos)
{
	m_pWeenie = pWeenie;
	_toleranceType = pWeenie->InqIntQuality(TOLERANCE_INT, 0, TRUE);
	_aiOptions = pWeenie->InqIntQuality(AI_OPTIONS_INT, 0, TRUE);
	_cachedVisualAwarenessRange = m_pWeenie->InqFloatQuality(VISUAL_AWARENESS_RANGE_FLOAT, DEFAULT_AWARENESS_RANGE);

	_meleeWeapon = m_pWeenie->GetWieldedCombat(COMBAT_USE_MELEE);
	_missileWeapon = m_pWeenie->GetWieldedCombat(COMBAT_USE_MISSILE);
	_shield = m_pWeenie->GetWieldedCombat(COMBAT_USE_SHIELD);

	SKILL_ADVANCEMENT_CLASS unarmedSkill;
	m_pWeenie->m_Qualities.InqSkillAdvancementClass(UNARMED_COMBAT_SKILL, unarmedSkill);
	_hasUnarmedSkill = (unarmedSkill > UNTRAINED_SKILL_ADVANCEMENT_CLASS);

	if (_meleeWeapon != NULL && _missileWeapon != NULL) //if we have both melee and missile weapons, favor missile
	{
		m_pWeenie->FinishMoveItemToContainer(_meleeWeapon, m_pWeenie, 0, true);
		if (_shield && _missileWeapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) != ThrownWeapon_CombatStyle)
			m_pWeenie->FinishMoveItemToContainer(_shield, m_pWeenie, 0, true);
		else
			_currentShield = _shield;
		_currentWeapon = _missileWeapon;
	}
	else if (_missileWeapon != NULL)
	{
		_currentWeapon = _missileWeapon;
		if (_shield && _missileWeapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) != ThrownWeapon_CombatStyle)
			m_pWeenie->FinishMoveItemToContainer(_shield, m_pWeenie, 0, true);
		else
			_currentShield = _shield;
	}
	else if (_meleeWeapon != NULL)
	{
		_currentWeapon = _meleeWeapon;
		_currentShield = _shield;
	}

	if (pWeenie->m_Qualities._emote_table && pWeenie->m_Qualities._emote_table->_emote_table.lookup(Taunt_EmoteCategory))
		_nextTaunt = Timer::cur_time + Random::GenUInt(10, 30); //We have taunts so schedule them.

	// List of qualities that could be used to affect the AI behavior:
	// AI_CP_THRESHOLD_INT
	// AI_PP_THRESHOLD_INT
	// AI_ADVANCEMENT_STRATEGY_INT

	// AI_ALLOWED_COMBAT_STYLE_INT
	// COMBAT_TACTIC_INT
	// TARGETING_TACTIC_INT
	// HOMESICK_TARGETING_TACTIC_INT

	// AI_OPTIONS_INT
	// FRIEND_TYPE_INT
	// FOE_TYPE_INT

	// ATTACKER_AI_BOOL
	// AI_USES_MANA_BOOL
	// AI_USE_HUMAN_MAGIC_ANIMATIONS_BOOL
	// AI_IMMOBILE_BOOL
	// AI_ALLOW_TRADE_BOOL
	// AI_ACCEPT_EVERYTHING_BOOL
	// AI_USE_MAGIC_DELAY_FLOAT

	// AI_ACQUIRE_HEALTH_FLOAT
	// AI_ACQUIRE_STAMINA_FLOAT
	// AI_ACQUIRE_MANA_FLOAT

	// AI_COUNTERACT_ENCHANTMENT_FLOAT
	// AI_DISPEL_ENCHANTMENT_FLOAT

	// AI_TARGETED_DETECTION_RADIUS_FLOAT

	// SetHomePosition(HomePos); don't use preset home position
}

MonsterAIManager::~MonsterAIManager()
{
}

void MonsterAIManager::SetHomePosition(const Position &pos)
{
	m_HomePosition = pos;
}

void MonsterAIManager::Update()
{
	if (!m_HomePosition.objcell_id)
	{
		// make sure we set a home position
		if (!(m_pWeenie->transient_state & ON_WALKABLE_TS))
			return;

		SetHomePosition(m_pWeenie->m_Position);
	}

	switch (m_State)
	{
	case MonsterAIState::Idle:
		UpdateIdle();
		break;

	case MonsterAIState::MeleeModeAttack:
		UpdateMeleeModeAttack();
		break;

	case MonsterAIState::MissileModeAttack:
		UpdateMissileModeAttack();
		break;

	case MonsterAIState::ReturningToSpawn:
		UpdateReturningToSpawn();
		break;

	case MonsterAIState::SeekNewTarget:
		UpdateSeekNewTarget();
		break;
	}
}

void MonsterAIManager::SwitchState(int state)
{
	if (state == m_State)
		return;

	switch (state)
	{
	case MonsterAIState::Idle:
		EndIdle();
		break;

	case MonsterAIState::MeleeModeAttack:
		EndMeleeModeAttack();
		break;

	case MonsterAIState::MissileModeAttack:
		EndMissileModeAttack();
		break;

	case MonsterAIState::ReturningToSpawn:
		EndReturningToSpawn();
		break;

	case MonsterAIState::SeekNewTarget:
		EndSeekNewTarget();
		break;
	}

	EnterState(state);
}

void MonsterAIManager::EnterState(int state)
{
	m_State = state;

	switch (state)
	{
	case MonsterAIState::Idle:
		BeginIdle();
		break;

	case MonsterAIState::MeleeModeAttack:
		BeginMeleeModeAttack();
		break;

	case MonsterAIState::MissileModeAttack:
		BeginMissileModeAttack();
		break;

	case MonsterAIState::ReturningToSpawn:
		BeginReturningToSpawn();
		break;

	case MonsterAIState::SeekNewTarget:
		BeginSeekNewTarget();
		break;
	}
}

void MonsterAIManager::BeginIdle()
{
	m_fNextPVSCheck = Timer::cur_time;
	m_pWeenie->ChangeCombatMode(COMBAT_MODE::NONCOMBAT_COMBAT_MODE, false);
}

void MonsterAIManager::EndIdle()
{
}

void MonsterAIManager::UpdateIdle()
{
	if (_toleranceType == TolerateNothing)
	{
		SeekTarget();
	}
}

bool MonsterAIManager::SeekTarget()
{
	if (m_fNextPVSCheck <= Timer::cur_time)
	{
		m_fNextPVSCheck = Timer::cur_time + 2.0f;

		std::list<CWeenieObject *> results;
		g_pWorld->EnumNearbyPlayers(m_pWeenie, _cachedVisualAwarenessRange, &results); // m_HomePosition

		std::list<CWeenieObject *> validTargets;

		CWeenieObject *pClosestWeenie = NULL;
		double fClosestWeenieDist = FLT_MAX;

		for (auto weenie : results)
		{
			if (weenie == m_pWeenie)
				continue;

			if (!weenie->_IsPlayer()) // only attack players
				continue;

			if (!weenie->IsAttackable())
				continue;

			if (weenie->ImmuneToDamage(m_pWeenie)) // only attackable players (not dead, not in portal space, etc.
				continue;

			validTargets.push_back(weenie);

			/*
			double fWeenieDist = m_pWeenie->DistanceTo(weenie);
			if (pClosestWeenie && fWeenieDist >= fClosestWeenieDist)
			continue;

			pClosestWeenie = weenie;
			fClosestWeenieDist = fWeenieDist;
			*/
		}

		/*
		if (pClosestWeenie)
		SetNewTarget(pClosestWeenie);
		*/

		if (!validTargets.empty())
		{
			// Random target
			std::list<CWeenieObject *>::iterator i = validTargets.begin();
			std::advance(i, Random::GenInt(0, (unsigned int)(validTargets.size() - 1)));
			SetNewTarget(*i);
			return true;
		}
	}

	return false;
}

void MonsterAIManager::SetNewTarget(CWeenieObject *pTarget)
{
	m_TargetID = pTarget->GetID();

	if (_missileWeapon != NULL)
	{
		double fTargetDist = m_pWeenie->DistanceTo(pTarget, true);
		if(fTargetDist > 5 || (_meleeWeapon == NULL && !_hasUnarmedSkill))
			SwitchState(MissileModeAttack);
		else
			SwitchState(MeleeModeAttack);
	}
	else
		SwitchState(MeleeModeAttack);

	m_pWeenie->ChanceExecuteEmoteSet(pTarget->GetID(), NewEnemy_EmoteCategory);
}

CWeenieObject *MonsterAIManager::GetTargetWeenie()
{
	return g_pWorld->FindObject(m_TargetID);
}

float MonsterAIManager::DistanceToHome()
{
	if (!m_HomePosition.objcell_id)
		return FLT_MAX;

	return m_HomePosition.distance(m_pWeenie->m_Position);
}

bool MonsterAIManager::ShouldSeekNewTarget()
{
	if (DistanceToHome() >= m_fMaxHomeRange)
		return false;

	return true;
}

bool MonsterAIManager::RollDiceCastSpell()
{
	if (m_fNextCastTime > Timer::cur_time)
	{
		return false;
	}

	if (m_pWeenie->m_Qualities._spell_book)
	{
		/* not correct, these must be independent events (look at wisps)
		float dice = Random::RollDice(0.0f, 1.0f);

		auto spellIterator = m_pWeenie->m_Qualities._spell_book->_spellbook.begin();

		while (spellIterator != m_pWeenie->m_Qualities._spell_book->_spellbook.end())
		{
			float likelihood = spellIterator->second._casting_likelihood;

			if (dice <= likelihood)
			{
				return DoCastSpell(spellIterator->first);
			}

			dice -= likelihood;
			spellIterator++;
		}
		*/
		
		auto spellIterator = m_pWeenie->m_Qualities._spell_book->_spellbook.begin();

		while (spellIterator != m_pWeenie->m_Qualities._spell_book->_spellbook.end())
		{
			float dice = Random::RollDice(0.0f, 1.0f);
			float likelihood = spellIterator->second._casting_likelihood;

			if (dice <= likelihood)
			{
				m_fNextCastTime = Timer::cur_time + m_pWeenie->m_Qualities.GetFloat(AI_USE_MAGIC_DELAY_FLOAT, 0.0);
				return DoCastSpell(spellIterator->first);
			}

			spellIterator++;
		}
	}

	return false;
}

bool MonsterAIManager::DoCastSpell(DWORD spell_id)
{	
	CWeenieObject *pTarget = GetTargetWeenie();
	m_pWeenie->MakeSpellcastingManager()->CreatureBeginCast(pTarget ? pTarget->GetID() : 0, spell_id);
	return true;
}

bool MonsterAIManager::DoMeleeAttack()
{
	DWORD motion = 0;
	ATTACK_HEIGHT height = ATTACK_HEIGHT::UNDEF_ATTACK_HEIGHT;
	float power = 0.0f;
	GenerateRandomAttack(&motion, &height, &power);
	if (!motion)
	{
		return false;
	}

	CWeenieObject *pTarget = GetTargetWeenie();
	if (!pTarget)
	{
		return false;
	}

	m_pWeenie->TryMeleeAttack(pTarget->GetID(), height, power, motion);

	m_fNextAttackTime = Timer::cur_time + 2.0f;
	m_fNextChaseTime = Timer::cur_time; // chase again anytime
	m_fMinCombatStateTime = Timer::cur_time + m_fMinCombatStateDuration;

	return true;
}

void MonsterAIManager::GenerateRandomAttack(DWORD *motion, ATTACK_HEIGHT *height, float *power, CWeenieObject *weapon)
{
	*motion = 0;
	*height = ATTACK_HEIGHT::UNDEF_ATTACK_HEIGHT;
	*power = Random::GenFloat(0, 1);

	if (m_pWeenie->_combatTable)
	{
		if(weapon == NULL)
			weapon = m_pWeenie->GetWieldedCombat(COMBAT_USE_MELEE);
		if (weapon)
		{
			AttackType attackType = (AttackType)weapon->InqIntQuality(ATTACK_TYPE_INT, 0);

			if (attackType == (Thrust_AttackType | Slash_AttackType))
			{
				if (*power >= 0.75f)
					attackType = Slash_AttackType;
				else
					attackType = Thrust_AttackType;
			}

			CombatManeuver *combatManeuver;
			
			// some monster have undef'd attack heights (hollow?) which is index 0
			combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, (ATTACK_HEIGHT)Random::GenUInt(0, 3));

			if (!combatManeuver)
			{
				// and some don't
				combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, (ATTACK_HEIGHT)Random::GenUInt(1, 3));

				if (!combatManeuver)
				{
					combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::HIGH_ATTACK_HEIGHT);
				
					if (!combatManeuver)
					{
						combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::MEDIUM_ATTACK_HEIGHT);
				
						if (!combatManeuver)
						{
							combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::LOW_ATTACK_HEIGHT);
						}
					}
				}
			}

			if (combatManeuver)
			{
				*motion = combatManeuver->motion;
				*height = combatManeuver->attack_height;
			}
		}
		else
		{
			AttackType attackType;
			
			if (*power >= 0.75f)
			{
				attackType = Kick_AttackType;
			}
			else
			{
				attackType = Punch_AttackType;
			}

			CombatManeuver *combatManeuver;

			// some monster have undef'd attack heights (hollow?) which is index 0
			combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, (ATTACK_HEIGHT)Random::GenUInt(0, 3));

			if (!combatManeuver)
			{
				// and some don't
				combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, (ATTACK_HEIGHT)Random::GenUInt(1, 3));

				if (!combatManeuver)
				{
					combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::HIGH_ATTACK_HEIGHT);

					if (!combatManeuver)
					{
						combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::MEDIUM_ATTACK_HEIGHT);

						if (!combatManeuver)
						{
							combatManeuver = m_pWeenie->_combatTable->TryGetCombatManuever(m_pWeenie->get_minterp()->InqStyle(), attackType, ATTACK_HEIGHT::LOW_ATTACK_HEIGHT);
						}
					}
				}
			}

			if (combatManeuver)
			{
				*motion = combatManeuver->motion;
				*height = combatManeuver->attack_height;
			}
		}
	}

	if (!*motion)
	{
		*motion = Motion_AttackHigh1;
	}
}

void MonsterAIManager::BeginReturningToSpawn()
{
	// m_pWeenie->DoForcedStopCompletely();

	MovementParameters params;
	params.can_walk = 0;

	MovementStruct mvs;
	mvs.type = MovementTypes::MoveToPosition;	
	mvs.pos = m_HomePosition;
	mvs.params = &params;

	m_pWeenie->movement_manager->PerformMovement(mvs);

	m_fReturnTimeoutTime = Timer::cur_time + m_fReturnTimeout;

	m_pWeenie->ChanceExecuteEmoteSet(m_TargetID, Homesick_EmoteCategory);
}

void MonsterAIManager::EndReturningToSpawn()
{
}

void MonsterAIManager::UpdateReturningToSpawn()
{
	float fDistToHome = m_HomePosition.distance(m_pWeenie->m_Position);

	if (fDistToHome < 5.0f)
	{
		SwitchState(Idle);
		return;
	}
	
	if (m_fReturnTimeoutTime <= Timer::cur_time)
	{
		// teleport back to spawn
		m_pWeenie->Movement_Teleport(m_HomePosition);

		SwitchState(Idle);
		return;
	}
}

void MonsterAIManager::OnDeath()
{
}

bool MonsterAIManager::IsValidTarget(CWeenieObject *pWeenie)
{
	if (!pWeenie)
		return false;

	if (pWeenie == m_pWeenie)
		return false;

	if (!pWeenie->_IsPlayer()) // only attack players
		return false;

	if (pWeenie->ImmuneToDamage(m_pWeenie)) // only attackable players (not dead, not in portal space, etc.
		return false;

	return true;
}

void MonsterAIManager::AlertIdleFriendsToAggro(CWeenieObject *pAttacker)
{
	std::list<CWeenieObject *> results;
	g_pWorld->EnumNearby(m_pWeenie, 20.0f, &results);

	int ourType = m_pWeenie->InqIntQuality(CREATURE_TYPE_INT, 0);
	int ourFriendType = m_pWeenie->InqIntQuality(FRIEND_TYPE_INT, 0);
	
	for (auto weenie : results)
	{
		if (weenie == m_pWeenie)
			continue;

		if (!weenie->IsCreature())
			continue;

		CMonsterWeenie *creature = (CMonsterWeenie *)weenie;
		if (!creature->m_MonsterAI)
			continue;

		switch (creature->m_MonsterAI->m_State)
		{
		case Idle:
		case ReturningToSpawn:
		case SeekNewTarget:
			break;

		default:
			continue;
		}

		if (!creature->m_MonsterAI->IsValidTarget(pAttacker))
			continue;

		if (creature->m_MonsterAI->_toleranceType == TolerateEverything)
			continue;

		if (creature->m_MonsterAI->_toleranceType == TolerateUnlessAttacked && _aiOptions != 1) // _aiOptions = 1 creatures do not tolerate their own kind/friends being attacked.
			continue;

		int theirType = creature->InqIntQuality(CREATURE_TYPE_INT, 0);
		int theirFriendType = creature->InqIntQuality(FRIEND_TYPE_INT, 0);

		if (ourType > 0 && theirType > 0)
		{
			if (ourType != theirType && ourType != theirFriendType && theirType != ourFriendType) //we only help our own kind or friends
				continue;
		}

		creature->m_MonsterAI->m_fAggroTime = Timer::cur_time + 10.0;
		creature->m_MonsterAI->SetNewTarget(pAttacker);
	}
}

void MonsterAIManager::OnResistSpell(CWeenieObject *attacker)
{
	if(attacker)
		m_pWeenie->ChanceExecuteEmoteSet(attacker->GetID(), ResistSpell_EmoteCategory);
}

void MonsterAIManager::OnEvadeAttack(CWeenieObject *attacker)
{

}

void MonsterAIManager::OnDealtDamage(DamageEventData &damageData)
{
	if (_nextTaunt > 0 && _nextTaunt <= Timer::cur_time)
	{
		if (damageData.target)
			m_pWeenie->ChanceExecuteEmoteSet(damageData.target->GetID(), Taunt_EmoteCategory);
		_nextTaunt = Timer::cur_time + Random::GenUInt(10, 30);
	}
}

void MonsterAIManager::OnTookDamage(DamageEventData &damageData)
{
	CWeenieObject *source = damageData.source;
	unsigned int damage = damageData.outputDamageFinal;

	if (!source)
		return;

	HandleAggro(source);

	if(damageData.wasCrit)
		m_pWeenie->ChanceExecuteEmoteSet(source->GetID(), ReceiveCritical_EmoteCategory);

	if (m_pWeenie->m_Qualities._emote_table && !m_pWeenie->IsExecutingEmote())
	{
		PackableList<EmoteSet> *emoteSetList = m_pWeenie->m_Qualities._emote_table->_emote_table.lookup(WoundedTaunt_EmoteCategory);

		if (emoteSetList)
		{
			double healthPercent = m_pWeenie->GetHealthPercent();
			if (m_fLastWoundedTauntHP > healthPercent)
			{
				double dice = Random::GenFloat(0.0, 1.0);

				for (auto &emoteSet : *emoteSetList)
				{
					// ignore probability?
					if (healthPercent >= emoteSet.minhealth && healthPercent < emoteSet.maxhealth)
					{
						m_fLastWoundedTauntHP = healthPercent;

						m_pWeenie->MakeEmoteManager()->ExecuteEmoteSet(emoteSet, source->GetID());
					}
				}
			}
		}
	}
}

void MonsterAIManager::OnIdentifyAttempted(CWeenieObject *other)
{
	if (_toleranceType != TolerateUnlessBothered)
	{
		return;
	}

	if (m_pWeenie->DistanceTo(other, true) >= 60.0)
	{
		return;
	}

	HandleAggro(other);
}

void MonsterAIManager::HandleAggro(CWeenieObject *pAttacker)
{
	if (_toleranceType == TolerateEverything)
	{
		return;
	}

	if (!m_pWeenie->IsDead())
	{
		switch (m_State)
		{
		case Idle:
		case ReturningToSpawn:
		case SeekNewTarget:
			{
				if (IsValidTarget(pAttacker))
				{
					//if (m_pWeenie->DistanceTo(pAttacker, true) <= m_pWeenie->InqFloatQuality(VISUAL_AWARENESS_RANGE_FLOAT, DEFAULT_AWARENESS_RANGE))
					//{
					SetNewTarget(pAttacker);
					//}

					m_pWeenie->ChanceExecuteEmoteSet(pAttacker->GetID(), Scream_EmoteCategory);
					m_fAggroTime = Timer::cur_time + 10.0;
				}

				break;
			}
		}
	}

	AlertIdleFriendsToAggro(pAttacker);
}

void MonsterAIManager::BeginSeekNewTarget()
{
	if (!ShouldSeekNewTarget() || !SeekTarget())
	{
		SwitchState(ReturningToSpawn);
	}
}

void MonsterAIManager::UpdateSeekNewTarget()
{
	SwitchState(ReturningToSpawn);
}

void MonsterAIManager::EndSeekNewTarget()
{
}

void MonsterAIManager::BeginMeleeModeAttack()
{
	if (_shield != NULL && _currentShield == NULL)
	{
		if (m_pWeenie->FinishMoveItemToWield(_shield, SHIELD_LOC)) //make sure our shield is equipped
			_currentShield = _shield;
	}

	if (_currentWeapon != _meleeWeapon)
	{
		if (_currentWeapon)
			m_pWeenie->FinishMoveItemToContainer(_currentWeapon, m_pWeenie, 0, true);
		if (_meleeWeapon)
		{
			if (m_pWeenie->FinishMoveItemToWield(_meleeWeapon, MELEE_WEAPON_LOC))
				_currentWeapon = _meleeWeapon;
			else
				_currentWeapon = 0;
		}
	}

	m_pWeenie->ChangeCombatMode(COMBAT_MODE::MELEE_COMBAT_MODE, false);

	m_fChaseTimeoutTime = Timer::cur_time + m_fChaseTimeoutDuration;
	m_fNextAttackTime = Timer::cur_time;
	m_fNextChaseTime = Timer::cur_time;
	m_fMinCombatStateTime = Timer::cur_time + m_fMinCombatStateDuration;
	m_fMinReturnStateTime = Timer::cur_time + m_fMinReturnStateDuration;
}

void MonsterAIManager::EndMeleeModeAttack()
{
	m_pWeenie->unstick_from_object();
}

void MonsterAIManager::UpdateMeleeModeAttack()
{
	if (m_pWeenie->IsBusyOrInAction())
	{
		// still animating or busy (attacking, etc.)
		return;
	}

	// rules:
	// dont switch targets to one that is farther than visual awareness range, unless attacked
	// dont chase a target that is outside the chase range, unless attacked
	// dont chase any new target, even if attacked, outside home range

	CWeenieObject *pTarget = GetTargetWeenie();
	if (!pTarget || pTarget->IsDead() || !pTarget->IsAttackable() || pTarget->ImmuneToDamage(m_pWeenie) || m_pWeenie->DistanceTo(pTarget) >= m_fChaseRange)
	{
		if (ShouldSeekNewTarget())
		{
			SwitchState(SeekNewTarget);
		}
		else
		{
			SwitchState(ReturningToSpawn);
		}
		return;
	}

	if (DistanceToHome() >= m_fMaxHomeRange)
	{
		SwitchState(ReturningToSpawn);
		return;
	}

	double fTargetDist = m_pWeenie->DistanceTo(pTarget, true);
	if (fTargetDist >= m_pWeenie->InqFloatQuality(VISUAL_AWARENESS_RANGE_FLOAT, DEFAULT_AWARENESS_RANGE) && m_fAggroTime <= Timer::cur_time)
	{
		SwitchState(ReturningToSpawn);
		return;
	}

	if (m_fNextAttackTime > Timer::cur_time)
	{
		return;
	}

	if (!RollDiceCastSpell() && m_pWeenie->DistanceTo(pTarget) < m_fChaseRange)
	{
		// do physics attack
		DWORD motion = 0;
		ATTACK_HEIGHT height = ATTACK_HEIGHT::UNDEF_ATTACK_HEIGHT;
		float power = 0.0f;
		GenerateRandomAttack(&motion, &height, &power);
		m_pWeenie->TryMeleeAttack(pTarget->GetID(), height, power, motion);

		m_fNextAttackTime = Timer::cur_time + 2.0f;
		m_fNextChaseTime = Timer::cur_time; // chase again anytime
		m_fMinCombatStateTime = Timer::cur_time + m_fMinCombatStateDuration;
	}
}

void MonsterAIManager::BeginMissileModeAttack()
{
	if (_missileWeapon == 0)
	{
		SwitchState(MeleeModeAttack);
		return;
	}
	else
	{
		if (_missileWeapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0) != ThrownWeapon_CombatStyle)
		{
			CWeenieObject *equippedAmmo = m_pWeenie->GetWieldedCombat(COMBAT_USE::COMBAT_USE_AMMO);
			if (!equippedAmmo)
			{
				//we don't have ammo, disable missile mode and switch to melee.
				_missileWeapon = 0;
				SwitchState(MeleeModeAttack);
				return;
			}

			if (_currentShield != NULL)
			{
				m_pWeenie->FinishMoveItemToContainer(_currentShield, m_pWeenie, 0, true); //get rid of the shield.
				_currentShield = NULL;
			}
		}
		else if (_shield != NULL && _currentShield == NULL)
		{
			if(m_pWeenie->FinishMoveItemToWield(_shield, SHIELD_LOC)) //shields can be wielded with thrown weapons.
				_currentShield = _shield;
		}
	}

	if (_currentWeapon != _missileWeapon)
	{
		if (_currentWeapon)
		{
			m_pWeenie->FinishMoveItemToContainer(_currentWeapon, m_pWeenie, 0, true);
			if (m_pWeenie->FinishMoveItemToWield(_missileWeapon, MISSILE_WEAPON_LOC))
				_currentWeapon = _missileWeapon;
			else
			{
				_currentWeapon = 0;
				SwitchState(MeleeModeAttack);
			}
		}
	}

	m_pWeenie->ChangeCombatMode(COMBAT_MODE::MISSILE_COMBAT_MODE, false);

	m_fChaseTimeoutTime = Timer::cur_time + m_fChaseTimeoutDuration;
	m_fNextAttackTime = Timer::cur_time;
	m_fNextChaseTime = Timer::cur_time;
	m_fMinCombatStateTime = Timer::cur_time + m_fMinCombatStateDuration;
	m_fMinReturnStateTime = Timer::cur_time + m_fMinReturnStateDuration;
}

void MonsterAIManager::EndMissileModeAttack()
{
}

void MonsterAIManager::UpdateMissileModeAttack()
{
	if (m_pWeenie->IsBusyOrInAction() || m_pWeenie->motions_pending())
	{
		// still animating or busy (attacking, etc.)
		return;
	}

	// rules:
	// dont switch targets to one that is farther than visual awareness range, unless attacked
	// dont chase a target that is outside the chase range, unless attacked
	// dont chase any new target, even if attacked, outside home range

	CWeenieObject *pTarget = GetTargetWeenie();
	if (!pTarget || pTarget->IsDead() || !pTarget->IsAttackable() || pTarget->ImmuneToDamage(m_pWeenie) || m_pWeenie->DistanceTo(pTarget) >= m_fChaseRange)
	{
		if (ShouldSeekNewTarget())
		{
			SwitchState(SeekNewTarget);
		}
		else
		{
			SwitchState(ReturningToSpawn);
		}
		return;
	}

	if (DistanceToHome() >= m_fMaxHomeRange)
	{
		SwitchState(ReturningToSpawn);
		return;
	}

	float weaponMinRange = 1;
	float weaponMaxRange = 60; //todo: get the value from the weapon? Players currently have 60 as a fixed max value.

	double fTargetDist = m_pWeenie->DistanceTo(pTarget, true);
	if (fTargetDist >= max(m_pWeenie->InqFloatQuality(VISUAL_AWARENESS_RANGE_FLOAT, DEFAULT_AWARENESS_RANGE), weaponMaxRange) && m_fAggroTime <= Timer::cur_time)
	{
		SwitchState(ReturningToSpawn);
		return;
	}

	if (m_fNextAttackTime > Timer::cur_time)
	{
		return;
	}

	if (_meleeWeapon != NULL || _hasUnarmedSkill) //we also have a melee weapon(or know how to fight without one)
	{
		double roll = Random::GenFloat(0.0, 1.0);
		if (fTargetDist < weaponMinRange && roll < 0.3) //the target is too close, let's go melee
		{
			SwitchState(MeleeModeAttack);
			return;
		}

		if (roll < 0.02) //we're tired of doing missile attacks, let's switch it up
		{
			SwitchState(MeleeModeAttack);
			return;
		}
	}

	if (!RollDiceCastSpell())
	{
		if (m_pWeenie->DistanceTo(pTarget) < weaponMaxRange)
		{
			// do physics attack
			DWORD motion = 0;
			ATTACK_HEIGHT height = (ATTACK_HEIGHT)Random::GenUInt(1, 3);
			float power = Random::GenFloat(0, 1);

			m_pWeenie->TryMissileAttack(pTarget->GetID(), height, power);

			m_fNextAttackTime = Timer::cur_time + 2.0f;
			m_fNextChaseTime = Timer::cur_time; // chase again anytime
			m_fMinCombatStateTime = Timer::cur_time + m_fMinCombatStateDuration;
		}
	}
}