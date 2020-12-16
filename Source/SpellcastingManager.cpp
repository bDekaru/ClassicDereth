#include <StdAfx.h>
#include "SpellcastingManager.h"
#include "WeenieObject.h"
#include "World.h"
#include "SpellProjectile.h"
#include "WeenieFactory.h"
#include "InferredPortalData.h"
#include "WClassID.h"
#include "Container.h"
#include "Player.h"
#include "Client.h"
#include "Config.h"
#include "CombatFormulas.h"
#include "RandomRange.h"
#include "Random.h"
#include <random>

const float MAX_HEADING_TO_TARGET_FOR_CAST = 45.0f;
const float MAX_TURN_TIME_FOR_CAST = 9999.0f;
const float MAX_MOTION_TIME_FOR_CAST = 4.0f;
const float MAX_PROJECTILE_CAST_RANGE = 100.0;
const float CAST_UPDATE_RATE = 1.25f;


CSpellcastingManager::CSpellcastingManager(CWeenieObject *pWeenie)
{
	m_pWeenie = pWeenie;
}

CSpellcastingManager::~CSpellcastingManager()
{

}

void CSpellcastingManager::EndCast(int error)
{
	m_pWeenie->DoForcedStopCompletely();
	m_pWeenie->NotifyUseDone(error);
	m_bCasting = false;
	m_SpellCastData = SpellCastData();
	m_PendingMotions.clear();
	m_bTurningToObject = false;
	m_bTurned = false;
}

bool CSpellcastingManager::ResolveSpellBeingCasted()
{
	if (!m_SpellCastData.spell_id)
		return false;

	CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
	if (!pSpellTable)
		return false;

	m_SpellCastData.spell = pSpellTable->GetSpellBase(m_SpellCastData.spell_id);
	if (!m_SpellCastData.spell)
		return false;

	CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx();
	if (pSpellTableEx)
		m_SpellCastData.spellEx = pSpellTableEx->GetSpellBase(m_SpellCastData.spell_id);

	m_SpellCastData.spell_formula = m_SpellCastData.spell->InqSpellFormula();
	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();
	m_SpellCastData.current_skill = DetermineSkillLevelForSpell();
	if (m_SpellCastData.spellEx->_category >= 683 && m_SpellCastData.spellEx->_category <= 686)
	{
		m_SpellCastData.current_skill = 1;
	}
	m_SpellCastData.max_range = DetermineSpellRange();

	if (m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex)
	{
		m_SpellCastData.target_id = m_SpellCastData.source_id;
	}

	if (m_SpellCastData.equipped && m_SpellCastData.spell->InqTargetType() == ITEM_TYPE::TYPE_ITEM_ENCHANTABLE_TARGET)
	{
		m_SpellCastData.target_id = m_SpellCastData.caster_id;
	}

	// item enchantment
	if (m_SpellCastData.equipped && (m_SpellCastData.spell->_school == 3) && (m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex))
	{
		if (m_SpellCastData.spell->_category == 152 || m_SpellCastData.spell->_category == 154 || m_SpellCastData.spell->_category == 156 || m_SpellCastData.spell->_category == 158 || m_SpellCastData.spell->_category == 195 || m_SpellCastData.spell->_category == 695)
		{
			m_SpellCastData.target_id = m_SpellCastData.source_id;
		}
		else {
			m_SpellCastData.target_id = m_SpellCastData.caster_id;
		}
	}
	return true;
}

std::string appendSpellText(std::string text, std::string newText, SpellComponentType componentType, SpellComponentType lastComponentType)
{
	if (!newText.size())
		return text;

	switch (componentType)
	{
	default:
	case SpellComponentType::Undef_SpellComponentType:
	case SpellComponentType::Action_SpellComponentType:
	case SpellComponentType::ConceptPrefix_SpellComponentType:
	{
		if (!text.empty())
			text += " ";
		text += newText;
		break;
	}
	case SpellComponentType::ConceptSuffix_SpellComponentType:
	{
		switch (lastComponentType)
		{
		default:
		case SpellComponentType::Undef_SpellComponentType:
		case SpellComponentType::Action_SpellComponentType:
			if (!text.empty())
				text += " ";
			text += newText;
			break;

		case SpellComponentType::ConceptPrefix_SpellComponentType:
		case SpellComponentType::ConceptSuffix_SpellComponentType:
			newText[0] = ::tolower(newText[0]);
			text += newText;
			break;
		}

		break;
	}
	}

	return text;
}

bool CSpellcastingManager::AddMotionsForSpell()
{
	SpellComponentTable *pSpellComponents = MagicSystem::GetSpellComponentTable();
	if (!pSpellComponents)
		return false;

	std::string spellWords;
	SpellComponentType lastComponentType = Undef_SpellComponentType;

	bool firstMotion = true;

	for (uint32_t i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
		uint32_t comp = m_SpellCastData.spell_formula._comps[i];
		if (!comp)
			break;

		const SpellComponentBase *pSpellComponent = pSpellComponents->InqSpellComponentBase(comp);
		if (!pSpellComponent)
			return false;

		if (pSpellComponent->_category == Scarab_SpellComponentCategory && m_SpellCastData.spell->_bitfield & FastCast_SpellIndex)
			continue;

		uint32_t gesture = pSpellComponent->_gesture;
		if (gesture && (gesture & 0xFFFF) != 0)
		{
			if (gesture == 0x1000012F)
				gesture = 0x13000132; // level 7's are wrong for some reason

			// m_pWeenie->SendText(csprintf("Component \"%s\": %s %f %f 0x%08X (%u)", pSpellComponent->_name.c_str(), pSpellComponent->_text.c_str(), pSpellComponent->_time, pSpellComponent->_CDM, pSpellComponent->_gesture, pSpellComponent->_gesture & 0xFFFF), 1);
			m_PendingMotions.push_back(SpellCastingMotion(gesture, 2.0f, firstMotion, firstMotion, pSpellComponent->_time));
			firstMotion = false;
		}

		if (!pSpellComponent->_text.empty())
		{
			spellWords = appendSpellText(spellWords, pSpellComponent->_text, pSpellComponent->_type, lastComponentType);
			lastComponentType = pSpellComponent->_type;
		}
	}

	if (!spellWords.empty())
	{
		m_pWeenie->SpeakLocal(spellWords.c_str(), LTT_MAGIC_CASTING_CHANNEL);
	}

	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();
	return true;
}

void CSpellcastingManager::BeginCast()
{
	m_bCasting = true;
	m_PendingMotions.clear();
	m_bTurningToObject = false;
	m_bTurned = false;
	
	AddMotionsForSpell();

	BeginNextMotion();
}

CWeenieObject *CSpellcastingManager::GetCastTarget()
{
	if (!m_bCasting || !m_SpellCastData.target_id)
		return NULL;

	return g_pWorld->FindObject(m_SpellCastData.target_id);
}

CWeenieObject *CSpellcastingManager::GetCastCaster()
{
	// could be a contained item casting a spell on the player
	return g_pWorld->FindObject(m_SpellCastData.caster_id);
}

CWeenieObject *CSpellcastingManager::GetCastSource()
{
	return g_pWorld->FindObject(m_SpellCastData.source_id);
}

float CSpellcastingManager::HeadingToTarget()
{
	if (!m_SpellCastData.target_id)
	{
		// Untargeted spell
		return 0.0;
	}

	CWeenieObject *pTarget = GetCastTarget();
	if (!pTarget)
	{
		// Don't know where the target is
		EndCast(WERROR_OBJECT_GONE);
		return 0.0;
	}

	if (pTarget->parent)
	{
		// Target has a parent (in inventory?)
		// This needs additional logic
		return 0.0;
	}

	// Return difference in heading.
	float fHeadingToTarget = m_pWeenie->m_Position.heading_diff(pTarget->m_Position);

	fHeadingToTarget = fabs(fHeadingToTarget);

	if (fHeadingToTarget >= 180.0)
		fHeadingToTarget = 360.0 - fHeadingToTarget;

	return fHeadingToTarget;
}

bool CSpellcastingManager::MotionRequiresHeading()
{
	if (!m_SpellCastData.target_id || m_SpellCastData.target_id == m_SpellCastData.caster_id || m_SpellCastData.target_id == m_SpellCastData.source_id)
		return false;

	if (CWeenieObject *target = g_pWorld->FindObject(m_SpellCastData.target_id))
	{
		if (CWeenieObject *owner = target->GetWorldTopLevelOwner())
		{
			if (owner->GetID() == m_SpellCastData.source_id)
				return false;
		}
	}

	if (m_PendingMotions.empty())
		return true; // last motion requires heading

	return m_PendingMotions.begin()->requiresHeading;
}

void CSpellcastingManager::BeginNextMotion()
{
	if (m_pWeenie->IsDead())
	{
		EndCast(0);
		return;
	}

	bool bNeedsTurnToObject = false;

	if (MotionRequiresHeading() && HeadingToTarget() > MAX_HEADING_TO_TARGET_FOR_CAST)
	{
		bNeedsTurnToObject = true;
	}

	if (!m_bCasting)
	{
		// Must have errored...
		return;
	}

	if (bNeedsTurnToObject)
	{
		m_bTurningToObject = true;

		MovementParameters params;
		params.speed = 1.0f;
		params.action_stamp = ++m_pWeenie->m_wAnimSequence;
		params.modify_interpreted_state = 0;
		m_pWeenie->last_move_was_autonomous = false;

		m_pWeenie->cancel_moveto();
		m_pWeenie->TurnToObject(m_SpellCastData.target_id, &params);

		m_SpellCastData.cast_timeout = Timer::cur_time + MAX_TURN_TIME_FOR_CAST;
	}
	else
	{
		m_bTurningToObject = false;

		if (m_PendingMotions.empty())
		{
			int bitfield = m_pWeenie->m_SpellcastingManager->m_SpellCastData.spell->_bitfield;
			if ((bitfield & Resistable_SpellIndex) && (bitfield & PKSensitive_SpellIndex) && (bitfield & FastCast_SpellIndex)) //streaks
			{
				if (m_pWeenie->m_Qualities.GetFloat(NEXT_SPELLCAST_TIMESTAMP_FLOAT, 0.0) <= Timer::cur_time)
				{
					int error = LaunchSpellEffect(false);
					EndCast(error);
					m_pWeenie->m_Qualities.SetFloat(NEXT_SPELLCAST_TIMESTAMP_FLOAT, Timer::cur_time + 2.0);
				}
				else
					EndCast(WERROR_ACTIONS_LOCKED);
			}
			else //everything else
			{
				int error = LaunchSpellEffect(false);
				EndCast(error);
			}
		}
		else
		{
			m_SpellCastData.cast_timeout = Timer::cur_time + MAX_MOTION_TIME_FOR_CAST;

			m_pWeenie->StopCompletely(0);

			SpellCastingMotion &cast_motion = *m_PendingMotions.begin();

			MovementParameters params;
			params.action_stamp = ++m_pWeenie->m_wAnimSequence;
			params.speed = cast_motion.speed;

			MovementStruct mvs;
			mvs.type = MovementTypes::RawCommand;
			mvs.motion = cast_motion.motion;
			mvs.params = &params;

			m_pWeenie->last_move_was_autonomous = false;

			int errorCode;
			if (!(errorCode = m_pWeenie->PerformMovement(mvs)))
			{
				m_fNextCastTime = Timer::cur_time + (cast_motion.min_time * 0.5f);
				m_pWeenie->_server_control_timestamp++;
				m_pWeenie->Animation_Update();
			}
			else
			{
				// m_pWeenie->PerformMovement(mvs);
				EndCast(WERROR_MAGIC_GENERAL_FAILURE);
			}
		}
	}
}

Position CSpellcastingManager::GetSpellProjectileSpawnPosition(CSpellProjectile *pProjectile, CWeenieObject *pTarget, float *pDistToTarget, double dDir, bool bRing)
{
	bool bArc = pProjectile->InqBoolQuality(GRAVITY_STATUS_BOOL, FALSE) ? true : false;

	CWeenieObject *pSource = GetCastSource();
	Position spawnPosition = pSource->m_Position.add_offset(Vector(0, 0, pSource->GetHeight() * (bArc ? 1.0 : (2.0 / 3.0))));

	Vector targetOffset;

	if (pTarget == pSource)
	{
		// rotate by dDir
		double cs = cos(dDir);
		double sn = sin(dDir);

		double x = -1000 * sn;
		double y = 1000 * cs;

		float z = pSource->GetHeight() * (2.0 / 3.0);
		if (bRing)
			z *= 1.5;

		targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(pTarget->m_Position.localtoglobalvec(Vector(x, y, z))));
	}
	else
	{
		targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (bArc ? (5.0 / 6.0) : (2.0 / 3.0)))));
		//targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (bArc ? (5.0 / 6.0) : (0.5)))));
		// rotate by dDir
		double cs = cos(dDir);
		double sn = sin(dDir);

		targetOffset.x = targetOffset.x * cs - targetOffset.y * sn;
		targetOffset.y = targetOffset.x * sn + targetOffset.y * cs;
	}

	Vector targetDir = targetOffset;

	if (targetDir.normalize_check_small())
	{
		targetDir = spawnPosition.frame.get_vector_heading();

		// spawnPosition.frame.m_origin += targetDir * minSpawnDist;
		spawnPosition.frame.set_vector_heading(targetDir);

		*pDistToTarget = 0.0f;
	}
	else
	{
		double minSpawnDist = (pSource->GetRadius() + pProjectile->GetRadius()) + 0.1f;

		if (bRing)
			minSpawnDist += 0.5f; //this will influence ring projectile start position. 1.0 will cause small targets to be inside the spawn radius at point blank.

		spawnPosition.frame.m_origin += targetDir * minSpawnDist;
		spawnPosition.frame.set_vector_heading(targetDir);

		*pDistToTarget = targetOffset.magnitude();
	}

	return spawnPosition;
}

Vector CSpellcastingManager::GetSpellProjectileSpawnVelocity(Position *pSpawnPosition, CWeenieObject *pTarget, float speed, bool tracked, bool gravity, Vector *pTargetDir, double dDir, bool bRing)
{
	Vector targetOffset;
	double targetDist;

	CWeenieObject *pSource = GetCastSource();
	if (pTarget == pSource)
	{
		// rotate by dDir
		double cs = cos(dDir);
		double sn = sin(dDir);

		double x = -1000 * sn;
		double y = 1000 * cs;

		float z = pTarget->GetHeight() * (2.0 / 3.0);
		if (bRing)
			z *= 1.5;

		targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(pTarget->m_Position.localtoglobalvec(Vector(x, y, z))));
		tracked = false;
	}
	else
	{
		targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (2.0 / 3.0))));

		// rotate by dDir
		double cs = cos(dDir);
		double sn = sin(dDir);

		targetOffset.x = targetOffset.x * cs - targetOffset.y * sn;
		targetOffset.y = targetOffset.x * sn + targetOffset.y * cs;
	}
	//targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * 0.5f)));

	targetDist = targetOffset.magnitude();

	Vector v;

	if (!tracked)
	{
		double t = targetDist / speed;
		v = targetOffset / t;

		if (gravity)
			v.z += (9.8*t) / 2.0f;

		Vector targetDir = v;
		targetDir.normalize();

		if (pTargetDir)
			*pTargetDir = targetDir;
	}
	else
	{
		Vector P0 = targetOffset;
		Vector P1(0, 0, 0);

		float s0 = pTarget->get_velocity().magnitude();
		Vector V0 = pTarget->get_velocity();
		if (V0.normalize_check_small())
			V0 = Vector(0, 0, 0);

		float s1 = speed;

		double a = (V0.x * V0.x) + (V0.y * V0.y) - (s1 * s1);
		double b = 2 * ((P0.x * V0.x) + (P0.y * V0.y) - (P1.x * V0.x) - (P1.y * V0.y));
		double c = (P0.x * P0.x) + (P0.y * P0.y) + (P1.x * P1.x) + (P1.y * P1.y) - (2 * P1.x * P0.x) - (2 * P1.y * P0.y);

		double t1 = (-b + sqrt((b * b) - (4 * a * c))) / (2 * a);
		double t2 = (-b - sqrt((b * b) - (4 * a * c))) / (2 * a);

		if (t1 < 0)
			t1 = FLT_MAX;
		if (t2 < 0)
			t2 = FLT_MAX;

		double t = min(t1, t2);
		if (t >= 100.0)
		{
			return GetSpellProjectileSpawnVelocity(pSpawnPosition, pTarget, speed, false, gravity, pTargetDir, dDir, bRing);
		}

		v.x = (P0.x + (t * s0 * V0.x)) / (t); // * s1);
		v.y = (P0.y + (t * s0 * V0.y)) / (t); // * s1);
		v.z = (P0.z + (t * s0 * V0.z)) / (t); // * s1);

		if (gravity)
		{
			v.z += (9.8*t) / 2.0f;
		}

		if (pTargetDir)
		{
			Vector targetDir = v;
			if (targetDir.normalize_check_small())
				targetDir = Vector(0, 0, 0);

			*pTargetDir = targetDir;
		}
	}

	return v;
}

bool CSpellcastingManager::LaunchProjectileSpell(ProjectileSpellEx *meta)
{
	if (!meta)
		return false;

	CWeenieObject *pTarget = GetCastTarget();
	if (!pTarget || !pTarget->InValidCell())
	{
		// Don't know where the target is
		return false;
	}

	int numX = (int)(meta->_dims.x + 0.5f);
	int numY = (int)(meta->_dims.y + 0.5f);
	int numZ = (int)(meta->_dims.z + 0.5f);

	CWeenieObject *pSource = GetCastSource();

	if (!pSource)
	{
		return false;
	}

	bool isLifeProjectile = false;
	double selfDrainedAmount = 0;
	float selfDrainedDamageRatio = 0;
	if (meta->AsLifeProjectileSpell())
	{
		isLifeProjectile = true;
		ProjectileLifeSpellEx *lifeProjectile = meta->AsLifeProjectileSpell();

		DAMAGE_TYPE damageType = static_cast<DAMAGE_TYPE>(lifeProjectile->_etype);
		selfDrainedDamageRatio = lifeProjectile->_damage_ratio;
		float drainPercentage = lifeProjectile->_drain_percentage;

		switch (damageType)
		{
		case HEALTH_DAMAGE_TYPE:
		{
			int amount = round((float)pSource->GetHealth() * drainPercentage);
			if (amount >= pSource->GetHealth())
			{
				selfDrainedAmount = pSource->GetHealth() - 1;
				pSource->SetHealth(1);
			}
			else
				selfDrainedAmount = abs(pSource->AdjustHealth(-amount, false));
			break;
		}
		case STAMINA_DAMAGE_TYPE:
		{
			int amount = round((float)pSource->GetStamina() * drainPercentage);
			selfDrainedAmount = abs(pSource->AdjustStamina(-amount));
			break;
		}
		case MANA_DAMAGE_TYPE:
		{
			int amount = round((float)pSource->GetMana() * drainPercentage);
			selfDrainedAmount = abs(pSource->AdjustMana(-amount));
			break;
		}
		}
	}

	bool bAngled = meta->_spreadAngle > 0 && numX > 1;

	for (int x = 0; x < numX; x++)
	{
		for (int y = 0; y < numY; y++)
		{
			for (int z = 0; z < numZ; z++)
			{
				//uint32_t damage = Random::GenUInt(meta->_baseIntensity, meta->_baseIntensity + meta->_variance);

				int target = m_SpellCastData.target_id;

				if (numX > 1 || numY > 1 || numZ > 1)
				{
					// volleys, blasts, rings aren't limited to one target
					target = 0;
				}

				CSpellProjectile *pProjectile = new CSpellProjectile(m_SpellCastData, target);

				// create the initial object
				float distToTarget;

				// spawn default object properties
				g_pWeenieFactory->ApplyWeenieDefaults(pProjectile, meta->_wcid);

				pProjectile->m_Qualities.SetInt(DAMAGE_TYPE_INT, meta->_etype);

				// set spell id (is this even needed?)
				pProjectile->SetSpellID(m_SpellCastData.spell_id);
				pProjectile->SetInitialPosition(GetCastSource()->m_Position);
				pProjectile->InitPhysicsObj();

				pProjectile->m_PhysicsState |= MISSILE_PS | REPORT_COLLISIONS_PS;
				pProjectile->m_PhysicsState &= ~IGNORE_COLLISIONS_PS;

				double maxVelocity = 5.0;
				pProjectile->m_Qualities.InqFloat(MAXIMUM_VELOCITY_FLOAT, maxVelocity);

				bool bGravity = false;
				if (pProjectile->InqBoolQuality(GRAVITY_STATUS_BOOL, FALSE))
				{
					pProjectile->m_PhysicsState |= GRAVITY_PS;
					bGravity = true;
				}

				bool bTracking = !meta->_bNonTracking;

				// angle at which to spawn from the caster
				double theta = 0;
				bool bRing = false;
				if (bAngled)
				{
					double xRatio = x / (double)numX;
					xRatio -= (numX - 1.0) / (double)(2 * numX);

					theta = DEG2RAD(xRatio * meta->_spreadAngle);

					// if a ring we want it to start a little further out
					if (meta->_spreadAngle > 180)
					{
						bRing = true;
					}
				}

				Position projSpawnPos = GetSpellProjectileSpawnPosition(pProjectile, pTarget, &distToTarget, theta, bRing);


				if (bRing)
				{
					// adjust for player casting position
					Vector ringOffset = pSource->m_Position.localtoglobalvec(Vector(0, 0.1f, 0));
					projSpawnPos = projSpawnPos.add_offset(ringOffset);
				}

				// overall offset
				Vector createOffset = projSpawnPos.localtoglobalvec(meta->_createOffset);
				projSpawnPos = projSpawnPos.add_offset(
					Vector(
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.x * meta->_padding.x,
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.y * meta->_padding.y,
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.z * meta->_padding.z));
				projSpawnPos = projSpawnPos.add_offset(createOffset);

				Vector spawnVelocity = GetSpellProjectileSpawnVelocity(&projSpawnPos, pTarget, maxVelocity, bTracking, bGravity, NULL, theta, bRing);

				// individual offset
				if (!bAngled)
				{
					Vector sizePerProjectile = meta->_padding; // * radius;

					Vector projectileGroupOffset = Vector(x, y, z);
					projectileGroupOffset.x *= sizePerProjectile.x;
					projectileGroupOffset.y *= sizePerProjectile.y;
					projectileGroupOffset.z *= sizePerProjectile.z;

					projectileGroupOffset.x -= sizePerProjectile.x * ((meta->_dims.x - 1.0) / 2.0);
					projectileGroupOffset.y -= sizePerProjectile.y * ((meta->_dims.y - 1.0) / 2.0);
					projectileGroupOffset.z -= sizePerProjectile.z * ((meta->_dims.z - 1.0) / 2.0);

					projectileGroupOffset = projSpawnPos.localtoglobalvec(projectileGroupOffset);
					projSpawnPos = projSpawnPos.add_offset(projectileGroupOffset);
				}


				pProjectile->m_Position = projSpawnPos;

				pProjectile->set_velocity(spawnVelocity, 0);

				pProjectile->m_PhysicsState |= INELASTIC_PS | SCRIPTED_COLLISION_PS | LIGHTING_ON_PS;

				// pProjectile->m_PhysicsState |= ETHEREAL_PS;




				if (isLifeProjectile)
				{
					pProjectile->makeLifeProjectile(selfDrainedAmount, selfDrainedDamageRatio);
				}

				if (meta->IsProjectileEnchantSpell())
				{
					pProjectile->makeEnchantProjectile();
				}

				// insert the object into the world
				if (g_pWorld->CreateEntity(pProjectile))
				{

					/*
					LOG(Temp, Normal, "Projectile Offset @ %.3f %.3f %.3f %f\n",
						createOffset.x + projectileGroupOffset.x,
						createOffset.y + projectileGroupOffset.y,
						createOffset.z + projectileGroupOffset.z, pProjectile->m_velocityVector.magnitude());
						*/
				}
			}
		}
	}
	// LOG(Temp, Normal, "-\n");

	return true;
}

void CSpellcastingManager::PerformCastParticleEffects()
{
	if (!m_bCasting)
		return;

	// self effect
	if (m_SpellCastData.spell->_caster_effect)
	{
		GetCastSource()->EmitEffect(m_SpellCastData.spell->_caster_effect, max(0.0, min(1.0, (m_SpellCastData.power_level_of_power_component - 1.0) / 7.0)));
	}

	// target effect
	if (m_SpellCastData.spell->_target_effect)
	{
		if ((m_SpellCastData.spell->_category == 162 || m_SpellCastData.spell->_category == 164 || m_SpellCastData.spell->_category == 166 || m_SpellCastData.spell->_category == 168 ||
			m_SpellCastData.spell->_category == 170 || m_SpellCastData.spell->_category == 172 || m_SpellCastData.spell->_category == 174 || m_SpellCastData.spell->_category == 160) &&
			(m_SpellCastData.source_id == m_SpellCastData.target_id))
		{
			return;
		}
		else
		{
			if (CWeenieObject *pTarget = GetCastTarget())
			{
				pTarget->EmitEffect(m_SpellCastData.spell->_target_effect, max(0.0, min(1.0, (m_SpellCastData.power_level_of_power_component - 1.0) / 7.0)));
			}
		}
	}

}

void CSpellcastingManager::PerformFellowCastParticleEffects(fellowship_ptr_t &fellow)
{
	if (!m_bCasting)
		return;

	// self effect
	if (m_SpellCastData.spell->_caster_effect)
	{
		GetCastSource()->EmitEffect(m_SpellCastData.spell->_caster_effect, max(0.0, min(1.0, (m_SpellCastData.power_level_of_power_component - 1.0) / 7.0)));
	}

	// target effect
	if (m_SpellCastData.spell->_target_effect)
	{
		for (auto &entry : fellow->_fellowship_table)
		{
			if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
			{
				if (member)
					member->EmitEffect(m_SpellCastData.spell->_target_effect, max(0.0, min(1.0, (m_SpellCastData.power_level_of_power_component - 1.0) / 7.0)));
			}
		}
	}
}

void CSpellcastingManager::BeginPortalSend(const Position &targetPos)
{
	if (CPlayerWeenie *player = m_pWeenie->AsPlayer())
	{
		if (!player->IsRecalling())
		{
			player->BeginRecall(targetPos);
		}
	}
	else if (m_pWeenie->HasOwner())
	{
		if (CPlayerWeenie *gemtarget = m_pWeenie->GetWorldTopLevelOwner()->AsPlayer())
		{
			if (!gemtarget->IsRecalling())
			{
				gemtarget->BeginRecall(targetPos);
			}
		}
	}
	else
	{
		m_pWeenie->Movement_Teleport(targetPos, false);
	}
}

int CSpellcastingManager::LaunchSpellEffect(bool bFizzled, bool silent)
{
	if (bFizzled)
		return WERROR_NONE;

	int targetError = 0;
	if(!silent)
		targetError = CheckTargetValidity();

	if (targetError && m_SpellCastData.range_check)
	{
		switch (targetError)
		{
		case WERROR_MISSILE_OUT_OF_RANGE:
		{
			CWeenieObject *pTarget = GetCastTarget();
			if (pTarget)
				pTarget->SendText(csprintf("%s tried to cast a spell on you, but was too far away!", m_pWeenie->GetName().c_str()), LTT_MAGIC);

			m_pWeenie->SendText("That target is too far away!", LTT_MAGIC);
			break;
		}
		}

		return targetError;
	}


	// cast source is assumed to be valid at this point
	CPlayerWeenie* player = m_pWeenie->AsPlayer();
	if (player && m_SpellCastData.uses_mana)
	{
		double chance = GetMagicSkillChance(m_SpellCastData.current_skill, m_SpellCastData.spell->_power);
		if (chance < Random::RollDice(0.0, 1.0))
		{
			// fizzle
			m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
			m_pWeenie->AdjustMana(-5);

			bFizzled = true;
		}
		else if (!bFizzled && m_pWeenie->m_Position.distance(m_SpellCastData.initial_cast_position) >= 6.0)
		{
			// fizzle
			m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
			m_pWeenie->AdjustMana(-5);
			m_pWeenie->SendText("Your movement disrupted spell casting!", LTT_MAGIC);
			m_pWeenie->SendText("Your movement disrupted spell casting!", LTT_ERROR);

			return WERROR_NONE;
		}

		if (!m_UsedComponents.empty()) //we used components, so check if any need burning
		{
			//Each spell and each component type has a burn rate, where the lower level spells typically burn less.
			//This rate is increased when fizzling. Your magic skill is not a factor except indirectly through fizzling.

			SpellComponentTable *pSpellComponents = MagicSystem::GetSpellComponentTable();
			float spellComponentLossMod = m_SpellCastData.spell->_component_loss;
			if (bFizzled)
				spellComponentLossMod *= 1.5; //made up value: 50% extra chance of burning components on fizzles.

			if (pSpellComponents)
			{
				std::string componentsConsumedString = "";
				for (std::map<uint32_t, uint32_t>::iterator iter = m_UsedComponents.begin(); iter != m_UsedComponents.end(); ++iter)
				{
					CWeenieObject *component = g_pWorld->FindObject(iter->first);
					if (!component) // where did it go? force fizzle.
						return WERROR_MAGIC_FIZZLE;

					int compId = component->InqDIDQuality(SPELL_COMPONENT_DID, 0);
					int spellPower = m_SpellCastData.spell->_power;
					int currentSkill = m_SpellCastData.current_skill;

					const SpellComponentBase *componentBase = pSpellComponents->InqSpellComponentBase(compId);
					float burnChance = componentBase->_CDM * spellComponentLossMod;
					burnChance *= min(1.0, (double)spellPower / (double)currentSkill);
					if (Random::RollDice(0.0, 1.0) < burnChance)
					{
						for (int i = 0; i < getRandomNumber(1, iter->second, eRandomFormula::favorMid, 1.5, 0); ++i)
						{
							component->DecrementStackOrStructureNum();
							if (componentsConsumedString.length() > 0)
								componentsConsumedString.append(", ");
							componentsConsumedString.append(componentBase->_name);
						}
					}
				}
				m_UsedComponents.clear();

				if (componentsConsumedString.length() > 0)
					m_pWeenie->SendText(csprintf("The spell consumed the following components: %s", componentsConsumedString.c_str()), LTT_MAGIC);
			}
		}

		if (bFizzled)
			return WERROR_MAGIC_FIZZLE;

		//Give skill XP for casting spell
		if(m_SpellCastData.spell->_power > 0) {
			player->MaybeGiveSkillUsageXP((STypeSkill)m_SpellCastData.spell->InqSkillForSpell(), m_SpellCastData.spell->_power);

			//Give skill XP for mana conversion
			int manaConversionDifficulty = round((((float)50 + m_SpellCastData.spell->_power / 50.0) + 1) * 25.0);
			player->MaybeGiveSkillUsageXP(STypeSkill::MANA_CONVERSION_SKILL, manaConversionDifficulty);
		}


		m_pWeenie->AdjustMana(-GenerateManaCost());
	}
	else if ((m_pWeenie->AsMeleeWeapon() || m_pWeenie->AsMissileLauncher()) && m_SpellCastData.uses_mana)
	{
		// Drain mana from weapon on Cast on Strike
		m_pWeenie->m_Qualities.SetInt(ITEM_CUR_MANA_INT, m_pWeenie->InqIntQuality(ITEM_CUR_MANA_INT, 0, 0) - m_SpellCastData.spell->_base_mana);
	}

	CWeenieObject *target = GetCastTarget();
	if (target && target->AsPlayer() && m_pWeenie->AsPlayer() && !VerifyPkAction(target))
		return WERROR_NONE;

	bool bSpellPerformed = false;

	if (m_SpellCastData.spellEx && m_SpellCastData.spellEx->_meta_spell._spell)
	{
		switch (m_SpellCastData.spellEx->_meta_spell._sp_type)
		{
		case SpellType::Transfer_SpellType:
		{
			TransferSpellEx *meta = (TransferSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
			bSpellPerformed = DoTransferSpell(GetCastTarget(), meta);
			break;
		}
		case SpellType::Boost_SpellType:
		{
			//bSpellPerformed = AdjustVital(GetCastTarget(), Random::GenInt(boostMin, boostMax), meta->_dt);
			bSpellPerformed = AdjustVital(GetCastTarget());
			break;
		}
		case SpellType::FellowBoost_SpellType:
		{
			FellowshipBoostSpellEx *meta = (FellowshipBoostSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			if (!target->HasFellowship())
				break;

			else
			{
				fellowship_ptr_t fellow = target->GetFellowship();
				CWorldLandBlock *block = target->GetBlock();
				for (auto &entry : fellow->_fellowship_table)
				{
					if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
					{
						if (member->GetBlock() == block)
						{
							if (!VerifyPkAction(member))
								return WERROR_NONE;
							else
								bSpellPerformed = AdjustVital(member);
						}
					}
				}
				PerformFellowCastParticleEffects(fellow);
			}

			break;
		}
		case SpellType::Dispel_SpellType:
		{
			DispelSpellEx *meta = (DispelSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			int minNum = (int)((meta->_number * meta->_number_variance) + F_EPSILON);
			int maxNum = (int)((meta->_number * (1.0 / meta->_number_variance)) + F_EPSILON);

			int numToDispel = Random::GenInt(minNum, maxNum);

			if (target)
			{
				if (target->AsPlayer() && target->AsPlayer()->CheckPKActivity())
				{
					if (m_pWeenie->AsPlayer() && m_pWeenie->GetID() == target->GetID())
					{
						if (m_pWeenie->AsPlayer())
							m_pWeenie->AsPlayer()->SendText("You have been involved in Player Killer combat too recently!", LTT_MAGIC);
						break;
					}
					if (target->AsPlayer() && target->AsPlayer()->CheckPKActivity())
					{
						if (m_pWeenie->AsPlayer())
							m_pWeenie->AsPlayer()->SendText(csprintf("%s has been involved in Player Killer combat too recently!", target->GetName().c_str()), LTT_MAGIC);
						break;
					}
				}
				if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
				{
					if (target->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
					{
						break;
					}
				}

				PackableListWithJson<uint32_t> possibleToDispel;

				if (target->m_Qualities._enchantment_reg)
				{
					if (target->m_Qualities._enchantment_reg->_add_list)
					{
						for (auto &entry : *target->m_Qualities._enchantment_reg->_add_list)
						{
							if (entry._power_level > meta->_max_power)
								continue;
							if (entry._power_level < meta->_min_power)
								continue;
							if (entry._duration <= 0.0)
								continue;
							if ((entry._id & 0xFFFF) == 666) // vitae
								continue;
							if (entry._id == 3204 && g_pConfig->AllowCoalDispel())
								continue;

							if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
							{
								if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry._id & 0xFFFF))
								{
									if (meta->_school && meta->_school != spellBaseEx->_school)
										continue;

									switch (meta->_align)
									{
									case 0: // neutral
									{
										break;
									}
									case 1: // good only
									{
										if (!(spellBaseEx->_bitfield & Beneficial_SpellIndex))
											continue;

										break;
									}
									case 2: // bad only
									{
										if (spellBaseEx->_bitfield & Beneficial_SpellIndex)
											continue;

										break;
									}
									}

									possibleToDispel.push_back(entry._id);
								}
							}
						}
					}

					if (target->m_Qualities._enchantment_reg->_mult_list)
					{
						for (auto &entry : *target->m_Qualities._enchantment_reg->_mult_list)
						{
							if (entry._power_level > meta->_max_power)
								continue;
							if (entry._power_level < meta->_min_power)
								continue;
							if (entry._duration <= 0.0)
								continue;
							if ((entry._id & 0xFFFF) == 666) // vitae
								continue;
							if (entry._id == 3204 && g_pConfig->AllowCoalDispel())
								continue;

							if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
							{
								if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry._id & 0xFFFF))
								{
									if (meta->_school && meta->_school != spellBaseEx->_school)
										continue;

									switch (meta->_align)
									{
									case 0: // neutral
									{
										break;
									}
									case 1: // good only
									{
										if (!(spellBaseEx->_bitfield & Beneficial_SpellIndex))
											continue;

										break;
									}
									case 2: // bad only
									{
										if (spellBaseEx->_bitfield & Beneficial_SpellIndex)
											continue;

										break;
									}
									}

									possibleToDispel.push_back(entry._id);
								}
							}
						}
					}
				}

				PackableListWithJson<uint32_t> listToDispel;

				if (meta->_number < 0)
				{
					// dispel all
					listToDispel = possibleToDispel;
				}
				else
				{
					while (numToDispel > 0 && !possibleToDispel.empty())
					{
						std::list<uint32_t>::iterator randomEntry = possibleToDispel.begin();
						std::advance(randomEntry, Random::GenUInt(0, (uint32_t)(possibleToDispel.size() - 1)));

						listToDispel.push_back(*randomEntry);
						possibleToDispel.erase(randomEntry);

						numToDispel--;
					}
				}

				std::string spellNames;
				for (auto entry : listToDispel)
				{
					if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
					{
						if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry & 0xFFFF))
						{
							if (spellNames.empty())
							{
								spellNames = spellBaseEx->_name;
							}
							else
							{
								spellNames += ", ";
								spellNames += spellBaseEx->_name;
							}
						}
					}
				}

				// "You cast Incantation of Nullify All Magic Self on yourself and dispel: ..."
				// "Tusker's Friend casts Nullify All Magic Other on you, but the dispel fails."

				if (listToDispel.size() > 0)
				{
					if (target == m_pWeenie)
					{
						m_pWeenie->SendText(csprintf("You cast %s on yourself and dispel: %s", m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
					}
					else
					{
						m_pWeenie->SendText(csprintf("You cast %s on %s and dispel: %s", m_SpellCastData.spell->_name.c_str(), target->GetName().c_str(), spellNames.c_str()), LTT_MAGIC);
						target->SendText(csprintf("%s casts %s on you and dispels: %s", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
					}

					if (target->m_Qualities._enchantment_reg)
					{
						target->m_Qualities._enchantment_reg->RemoveEnchantments(&listToDispel);

						BinaryWriter expireMessage;
						expireMessage.Write<uint32_t>(0x2C8);
						listToDispel.Pack(&expireMessage);
						target->SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
					}
				}
				else
				{
					if (target == m_pWeenie)
					{
						m_pWeenie->SendText(csprintf("You cast %s on yourself, but the dispel fails.", m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
					}
					else
					{
						m_pWeenie->SendText(csprintf("You cast %s on %s, but the dispel fails.", m_SpellCastData.spell->_name.c_str(), target->GetName().c_str()), LTT_MAGIC);
						target->SendText(csprintf("%s casts %s on you, but the dispel fails.", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
					}
				}

				bSpellPerformed = true;
			}

			break;
		}
		case SpellType::FellowDispel_SpellType:
		{
			FellowshipDispelSpellEx *meta = (FellowshipDispelSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			int minNum = (int)((meta->_number * meta->_number_variance) + F_EPSILON);
			int maxNum = (int)((meta->_number * (1.0 / meta->_number_variance)) + F_EPSILON);

			int numToDispel = Random::GenInt(minNum, maxNum);

			if (!target->HasFellowship())
				break;

			else
			{
				fellowship_ptr_t fellow = target->GetFellowship();
				CWorldLandBlock *block = target->GetBlock();
				for (auto &entry : fellow->_fellowship_table)
				{
					if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
					{
						if (member->GetBlock() == block)
						{
							if (member && VerifyPkAction(member))
							{
								if (member->AsPlayer() && member->AsPlayer()->CheckPKActivity())
								{
									if (m_pWeenie->AsPlayer() && m_pWeenie->GetID() == member->GetID())
									{
										m_pWeenie->AsPlayer()->SendText("You have been involved in Player Killer combat too recently!", LTT_MAGIC);
										break;
									}
									if (member->AsPlayer() && member->AsPlayer()->CheckPKActivity())
									{
										m_pWeenie->AsPlayer()->SendText(csprintf("%s has been involved in Player Killer combat too recently!", member->GetName().c_str()), LTT_MAGIC);
										break;
									}
								}

								if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
								{
									if (member->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
									{
										break;
									}
								}

								PackableListWithJson<uint32_t> possibleToDispel;

								if (member->m_Qualities._enchantment_reg)
								{
									if (member->m_Qualities._enchantment_reg->_add_list)
									{
										for (auto &entry : *member->m_Qualities._enchantment_reg->_add_list)
										{
											if (entry._power_level > meta->_max_power)
												continue;
											if (entry._power_level < meta->_min_power)
												continue;
											if (entry._duration <= 0.0)
												continue;
											if ((entry._id & 0xFFFF) == 666) // vitae
												continue;
											if (entry._id == 3204 && g_pConfig->AllowCoalDispel())
												continue;

											if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
											{
												if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry._id & 0xFFFF))
												{
													if (meta->_school && meta->_school != spellBaseEx->_school)
														continue;

													switch (meta->_align)
													{
													case 0: // neutral
													{
														break;
													}
													case 1: // good only
													{
														if (!(spellBaseEx->_bitfield & Beneficial_SpellIndex))
															continue;

														break;
													}
													case 2: // bad only
													{
														if (spellBaseEx->_bitfield & Beneficial_SpellIndex)
															continue;

														break;
													}
													}

													possibleToDispel.push_back(entry._id);
												}
											}
										}
									}

									if (member->m_Qualities._enchantment_reg->_mult_list)
									{
										for (auto &entry : *member->m_Qualities._enchantment_reg->_mult_list)
										{
											if (entry._power_level > meta->_max_power)
												continue;
											if (entry._power_level < meta->_min_power)
												continue;
											if (entry._duration <= 0.0)
												continue;
											if ((entry._id & 0xFFFF) == 666) // vitae
												continue;
											if (entry._id == 3204 && g_pConfig->AllowCoalDispel())
												continue;

											if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
											{
												if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry._id & 0xFFFF))
												{
													if (meta->_school && meta->_school != spellBaseEx->_school)
														continue;

													switch (meta->_align)
													{
													case 0: // neutral
													{
														break;
													}
													case 1: // good only
													{
														if (!(spellBaseEx->_bitfield & Beneficial_SpellIndex))
															continue;

														break;
													}
													case 2: // bad only
													{
														if (spellBaseEx->_bitfield & Beneficial_SpellIndex)
															continue;

														break;
													}
													}

													possibleToDispel.push_back(entry._id);
												}
											}
										}
									}
								}

								PackableListWithJson<uint32_t> listToDispel;

								if (meta->_number < 0)
								{
									// dispel all
									listToDispel = possibleToDispel;
								}
								else
								{
									while (numToDispel > 0 && !possibleToDispel.empty())
									{
										std::list<uint32_t>::iterator randomEntry = possibleToDispel.begin();
										std::advance(randomEntry, Random::GenUInt(0, (uint32_t)(possibleToDispel.size() - 1)));

										listToDispel.push_back(*randomEntry);
										possibleToDispel.erase(randomEntry);

										numToDispel--;
									}
								}

								std::string spellNames;
								for (auto entry : listToDispel)
								{
									if (CSpellTableEx *pSpellTableEx = g_pPortalDataEx->GetSpellTableEx())
									{
										if (const CSpellBaseEx *spellBaseEx = pSpellTableEx->GetSpellBase(entry & 0xFFFF))
										{
											if (spellNames.empty())
											{
												spellNames = spellBaseEx->_name;
											}
											else
											{
												spellNames += ", ";
												spellNames += spellBaseEx->_name;
											}
										}
									}
								}

								// "You cast Incantation of Nullify All Magic Self on yourself and dispel: ..."
								// "Tusker's Friend casts Nullify All Magic Other on you, but the dispel fails."

								if (listToDispel.size() > 0)
								{
									if (member == m_pWeenie)
									{
										m_pWeenie->SendText(csprintf("You cast %s on yourself and dispel: %s", m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
									}
									else
									{
										m_pWeenie->SendText(csprintf("You cast %s on %s and dispel: %s", m_SpellCastData.spell->_name.c_str(), member->GetName().c_str(), spellNames.c_str()), LTT_MAGIC);
										member->SendText(csprintf("%s casts %s on you and dispels: %s", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
									}

									if (member->m_Qualities._enchantment_reg)
									{
										member->m_Qualities._enchantment_reg->RemoveEnchantments(&listToDispel);

										BinaryWriter expireMessage;
										expireMessage.Write<uint32_t>(0x2C8);
										listToDispel.Pack(&expireMessage);
										member->SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
									}
								}
								else
								{
									if (member == m_pWeenie)
									{
										m_pWeenie->SendText(csprintf("You cast %s on yourself, but the dispel fails.", m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
									}
									else
									{
										m_pWeenie->SendText(csprintf("You cast %s on %s, but the dispel fails.", m_SpellCastData.spell->_name.c_str(), member->GetName().c_str()), LTT_MAGIC);
										member->SendText(csprintf("%s casts %s on you, but the dispel fails.", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
									}
								}

								bSpellPerformed = true;
							}
						}
					}
				}
				PerformFellowCastParticleEffects(fellow);
			}
			break;
		}
		case SpellType::PortalLink_SpellType:
		{
			//if (m_pWeenie->HasOwner())
				//break;

			PortalLinkSpellEx *meta = (PortalLinkSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			if (target)
			{
				// portal bitmask, 0x10 = cannot be summoned
				// portal bitmask, 0x20 = cannot be linked/recalled

				int minLevel = target->InqIntQuality(MIN_LEVEL_INT, 0);
				int maxLevel = target->InqIntQuality(MAX_LEVEL_INT, 0);
				uint32_t origPortID = target->InqDIDQuality(ORIGINAL_PORTAL_DID, 0);

				int currentLevel = m_pWeenie->InqIntQuality(LEVEL_INT, 1);

				if (minLevel && currentLevel < minLevel)
				{
					m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LEVEL_TOO_LOW);
					break;
				}
				else if (maxLevel && currentLevel > maxLevel)
				{
					m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LEVEL_TOO_HIGH);
					break;
				}
				else if (origPortID == W_PORTALGATEWAY_CLASS) // Don't tie to portals with no original portal set.
				{
					m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LINK_FAILED);
					break;
				}

				switch (meta->_index)
				{
				case 1:
					if (target->m_Qualities.m_WeenieType == LifeStone_WeenieType)
					{
						m_pWeenie->m_Qualities.SetPosition(LINKED_LIFESTONE_POSITION, m_pWeenie->m_Position);
						bSpellPerformed = true;
						m_pWeenie->NotifyWeenieError(WERROR_LIFESTONE_LINK_SUCCESS);
					}
					else
					{
						m_pWeenie->NotifyWeenieError(WERROR_LIFESTONE_LINK_FAILED);
					}
					break;

				case 2: // primary portal tie
					if (target->m_Qualities.m_WeenieType == Portal_WeenieType &&
						!(target->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x20))
					{
						if (origPortID > 0)
						{
							// This is a summoned portal with an original portal ID
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_ONE_DID, origPortID);
							m_pWeenie->m_Qualities.SetPosition(LINKED_PORTAL_ONE_POSITION, target->InqPositionQuality(DESTINATION_POSITION, Position()));

						}
						else
						{
							// This is not a summoned portal
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_ONE_DID, target->m_Qualities.id);
							m_pWeenie->m_Qualities.SetPosition(LINKED_PORTAL_ONE_POSITION, target->InqPositionQuality(DESTINATION_POSITION, Position()));
						}
						bSpellPerformed = true;
						m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LINK_SUCCESS);
					}
					else
					{
						m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LINK_FAILED);
					}

					break;

				case 3: // secondary portal tie
					if (target->m_Qualities.m_WeenieType == Portal_WeenieType &&
						!(target->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x20))
					{
						if (origPortID > 0)
						{
							// This is a summoned portal with an original portal ID
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_TWO_DID, origPortID);
							m_pWeenie->m_Qualities.SetPosition(LINKED_PORTAL_TWO_POSITION, target->InqPositionQuality(DESTINATION_POSITION, Position()));

						}
						else
						{
							// This is not a summoned portal
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_TWO_DID, target->m_Qualities.id);
							m_pWeenie->m_Qualities.SetPosition(LINKED_PORTAL_TWO_POSITION, target->InqPositionQuality(DESTINATION_POSITION, Position()));
						}
						bSpellPerformed = true;
						m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LINK_SUCCESS);
					}
					else
					{
						m_pWeenie->NotifyWeenieError(WERROR_PORTAL_LINK_FAILED);
					}

					break;
				}
			}

			if (m_pWeenie->IsAdmin())
			{
				// lifestone tie = 1
				// primary portal tie = 2
				// secondary portal tie = 3
				// m_pWeenie->SendText(csprintf("Index: %d", meta->_index), LTT_DEFAULT);
			}

			break;
		}
		case SpellType::PortalRecall_SpellType:
		{
			/*if (m_pWeenie->HasOwner()) //Needed to comment out for Rare gems.
				break;*/

			if (m_pWeenie->AsPlayer() && m_pWeenie->AsPlayer()->CheckPKActivity())
			{
				m_pWeenie->NotifyWeenieError(WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY);
				break;
			}

			PortalRecallSpellEx *meta = (PortalRecallSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			switch (meta->_index)
			{
			case 1: // lifestone sending
			{
				Position lifestone;
				if (m_pWeenie->m_Qualities.InqPosition(SANCTUARY_POSITION, lifestone) && lifestone.objcell_id != 0)
				{
					BeginPortalSend(lifestone);
					bSpellPerformed = true;
				}
				else
				{
					m_pWeenie->NotifyWeenieError(WERROR_LIFESTONE_RECALL_NO_LINK);
				}
				break;
			}
			case 2: // lifestone recall
			{
				Position lifestone;
				if (m_pWeenie->m_Qualities.InqPosition(LINKED_LIFESTONE_POSITION, lifestone) && lifestone.objcell_id != 0)
				{
					BeginPortalSend(lifestone);
					bSpellPerformed = true;
				}
				else
				{
					m_pWeenie->NotifyWeenieError(WERROR_LIFESTONE_RECALL_NO_LINK);
				}
				break;
			}

			case 3: // portal recall
			{
				WErrorType portalError = CanPortalRecall(m_pWeenie->m_Qualities.GetDID(LAST_PORTAL_DID, 0));
				if (portalError != WERROR_NONE)
				{
					m_pWeenie->NotifyWeenieError(portalError);
					break;
				}
				else
					bSpellPerformed = true;
				break;
			}
			case 4: // primary portal recall
			{
				WErrorType portalError = CanPortalRecall(m_pWeenie->m_Qualities.GetDID(LINKED_PORTAL_ONE_DID, 0));
				if (portalError != WERROR_NONE)
				{
					m_pWeenie->NotifyWeenieError(portalError);
					break;
				}
				else
					bSpellPerformed = true;
				break;
			}
			case 5: // secondary portal recall
			{
				WErrorType portalError = CanPortalRecall(m_pWeenie->m_Qualities.GetDID(LINKED_PORTAL_TWO_DID, 0));
				if (portalError != WERROR_NONE)
				{
					m_pWeenie->NotifyWeenieError(portalError);
					break;
				}
				else
					bSpellPerformed = true;
				break;
			}
			}
			break;
		}
		case SpellType::Enchantment_SpellType:
		{
			EnchantmentSpellEx *meta = (EnchantmentSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			Enchantment enchant;

			enchant._id = meta->_spell_id | ((uint32_t)m_SpellCastData.serial << (uint32_t)16);
			enchant.m_SpellSetID = m_SpellCastData.set_id;
			enchant._spell_category = m_SpellCastData.spellEx->_category; // meta->_spellCategory;
			enchant._power_level = m_SpellCastData.spell->_power;
			enchant._start_time = Timer::cur_time;
			double buffDuration = 1.0;
			int augDurationCount = 1;
			if (m_pWeenie->m_Qualities.InqInt(AUGMENTATION_INCREASED_SPELL_DURATION_INT, augDurationCount))
			{
				buffDuration = ((double)augDurationCount * 0.2) + 1;
			}
			if (m_SpellCastData.spell->_bitfield & DamageOverTime_SpellIndex)
			{
				buffDuration = 1;
			}
			enchant._duration = m_SpellCastData.equipped ? -1.0 : meta->_duration * buffDuration;
			enchant._caster = m_pWeenie->GetID();
			enchant._degrade_modifier = meta->_degrade_modifier;
			enchant._degrade_limit = meta->_degrade_limit;
			enchant._last_time_degraded = -1.0;
			enchant._smod = meta->_smod;
			enchant._dtype = meta->_elementalDamageType;


			if (silent)
			{
				m_pWeenie->m_Qualities.UpdateEnchantment(&enchant);
			}


			std::list<CWeenieObject *> targets;

			if (target)
			{

				if (m_SpellCastData.spell->InqTargetType() != ITEM_TYPE::TYPE_ITEM_ENCHANTABLE_TARGET)
				{
					targets.push_back(target);
				}
				else
				{
					CContainerWeenie *container = target->AsContainer();
					if (container && !container->HasOwner())
					{
						for (auto wielded : container->m_Wielded)
						{
							if (wielded->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
							{
								if (target == m_pWeenie || wielded->parent || m_pWeenie->GetWorldTopLevelOwner() == target->GetWorldTopLevelOwner()) // for other targets, only physically wielded allowed, TopLevelOwner for buff gems.
								{
									targets.push_back(wielded);
								}
							}
						}
					}
					else
					{
						if (target->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
						{
							if (target == m_pWeenie || target->parent || !target->HasOwner() || target->GetWorldTopLevelOwner() == m_pWeenie) // for other targets, only physically wielded allowed
							{
								targets.push_back(target);
							}
						}
					}
				}

				bool isItemFirstBuff = true;
				for (auto castTarget : targets)
				{
					// You cast Harlune's Blessing on yourself, refreshing Harlune's Blessing
					// You cast Impenetrability III on Pathwarden Robe, surpassing Impenetrability II

					CWeenieObject *topLevelOwner = castTarget->GetWorldTopLevelOwner();

					if (castTarget->InqIntQuality(MAX_STACK_SIZE_INT, 1) > 1) //do not allow enchanting stackable items(ammunition)
					{
						m_pWeenie->NotifyWeenieError(WERROR_MAGIC_BAD_TARGET_TYPE);
						continue;
					}

					int text_option = 0; // 0 = normal cast text, 1 = refreshes text, 2 = surpassed by text, 3 = surpasses text
					std::string existing_spell_name = "";
					if (CEnchantmentRegistry* enchant_reg = castTarget->m_Qualities._enchantment_reg)
					{
						if (Enchantment* highest_enchant = enchant_reg->GetHighestEnchantOfCategory(enchant._spell_category, enchant._smod.type)) // check if spell of this category already exists
						{
							if ((highest_enchant->_id & 0xFFFF) == enchant._id)
								text_option = 1;

							else
							{
								// if the current highest enchant is more powerful than the one being cast, use surpassed by text (2), otherwise use surpasses text (3)
								text_option = highest_enchant->_power_level > enchant._power_level ? 2 : 3;

								CSpellTable* st = MagicSystem::GetSpellTable();
								existing_spell_name = st->GetSpellBase((highest_enchant->_id & 0xFFFF))->_name;
							}
						}
					}

					bool isPvP = false;
					if (m_pWeenie != castTarget)
					{
						if (m_SpellCastData.spell->_bitfield & Resistable_SpellIndex)
						{
							if (topLevelOwner->TryMagicResist(m_SpellCastData.current_skill))
							{
								topLevelOwner->EmitSound(Sound_ResistSpell, 1.0f, false);
								topLevelOwner->SendText(csprintf("You resist the spell cast by %s", m_pWeenie->GetName().c_str()), LTT_MAGIC);
								m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
								topLevelOwner->OnResistSpell(m_pWeenie);

								// Cast on Strike
								if (m_pWeenie != m_pWeenie->GetWorldTopLevelOwner())
								{
									m_pWeenie->GetWorldTopLevelOwner()->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
								}

								continue;
							}
						}

						if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
						{
							if (m_pWeenie && castTarget && m_pWeenie->AsPlayer() && (castTarget->AsPlayer() || topLevelOwner->AsPlayer()))
							{
								// Only Update PK Activity if both target and caster are PK, Prevents PKs from tagging NPKs for PK activity.
								if (m_pWeenie->IsPK() && (castTarget->IsPK() || topLevelOwner->AsPlayer()))
								{
									m_pWeenie->AsPlayer()->UpdatePKActivity();
									if (castTarget->AsPlayer())
										castTarget->AsPlayer()->UpdatePKActivity();
									else
										topLevelOwner->AsPlayer()->UpdatePKActivity();
									isPvP = true;
								}

							}

							if (castTarget->AsPlayer() && castTarget->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
							{
								continue;
							}
						}

						uint32_t resistMagic = castTarget->InqIntQuality(RESIST_MAGIC_INT, 0, FALSE);
						if (resistMagic > 0 && !(m_SpellCastData.spellEx->_bitfield & DamageOverTime_SpellIndex))
						{
							if (resistMagic >= 9999 && topLevelOwner->GetID() == m_pWeenie->GetID())
								continue;

							if (resistMagic >= 9999 || ::TryMagicResist(m_SpellCastData.current_skill, (uint32_t)resistMagic))
							{
								castTarget->EmitSound(Sound_ResistSpell, 1.0f, false);

								if (m_pWeenie != topLevelOwner)
								{
									topLevelOwner->SendText(csprintf("%s resists the spell cast by %s", m_pWeenie->GetName().c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
								}

								m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
								castTarget->OnResistSpell(m_pWeenie);
								continue;
							}
						}
						if (bool resistProjectileMagic = castTarget->InqBoolQuality(NON_PROJECTILE_MAGIC_IMMUNE_BOOL, 0) && !(m_SpellCastData.spellEx->_bitfield & DamageOverTime_SpellIndex))
						{
							if (resistProjectileMagic == 1 || ::TryMagicResist(m_SpellCastData.current_skill, (uint32_t)resistProjectileMagic))
							{
								castTarget->EmitSound(Sound_ResistSpell, 1.0f, false);

								if (m_pWeenie != topLevelOwner)
								{
									topLevelOwner->SendText(csprintf("%s resists the spell cast by %s", m_pWeenie->GetName().c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
								}

								m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
								castTarget->OnResistSpell(m_pWeenie);
								continue;
							}
						}
					}

					topLevelOwner->HandleAggro(m_pWeenie);


					if (castTarget && (m_SpellCastData.spell->_bitfield & DamageOverTime_SpellIndex))
					{
						enchant._duration += 0.25; // Handle extra time so all ticks happen
					}

					// PVP Item debuffs only last 30 sec
					if (isPvP && enchant._spell_category == 153 || enchant._spell_category == 155 || enchant._spell_category == 157 || enchant._spell_category == 159
						|| enchant._spell_category == 161)
						enchant._duration = 30.0;

					castTarget->m_Qualities.UpdateEnchantment(&enchant);
					castTarget->NotifyEnchantmentUpdated(&enchant);

					castTarget->CheckVitalRanges();

					if (IsArmorBuff(enchant._spell_category))
					{
						if (!isItemFirstBuff)
						{
							m_pWeenie->AdjustMana(-GenerateManaCost(true));
						}
						isItemFirstBuff = false;
					}


					const char* spell_name = m_SpellCastData.spell->_name.c_str();
					if (!silent)
					{
						if (m_pWeenie == castTarget)
						{
							switch (text_option)
							{
							default:
							case 0:
								m_pWeenie->SendText(csprintf("You cast %s on yourself", spell_name), LTT_MAGIC);
								break;
							case 1:
								m_pWeenie->SendText(csprintf("You cast %s on yourself, refreshing %s", spell_name, spell_name), LTT_MAGIC);
								break;
							case 2:
								m_pWeenie->SendText(csprintf("You cast %s on yourself, but it is surpassed by %s", spell_name, existing_spell_name.c_str()), LTT_MAGIC);
								break;
							case 3:
								m_pWeenie->SendText(csprintf("You cast %s on yourself, surpassing %s", spell_name, existing_spell_name.c_str()), LTT_MAGIC);
							}
						}
						else
						{
							if ((m_SpellCastData.spellEx->_category >= 683 && m_SpellCastData.spellEx->_category <= 686) && (m_pWeenie->AsMeleeWeapon() || m_pWeenie->AsMissileLauncher()))
							{
								//<player> delivers a Traumatic Assault to Hea Apostate Shock Trooper!
								m_pWeenie->GetWorldTopLevelOwner()->AsPlayer()->SendText(csprintf("Dirty Fighting! You deliver a %s to %s.", spell_name, castTarget->GetName().c_str()), LTT_COMBAT);
								if (m_pWeenie != topLevelOwner)
									topLevelOwner->SendText(csprintf("Dirty Fighting! %s delivers a %s to %s", m_pWeenie->GetWorldOwner()->GetName().c_str(), spell_name, castTarget == topLevelOwner ? "you" : castTarget->GetName().c_str()), LTT_COMBAT);
							}
							else
							{
								if (m_pWeenie->IsAetheria())
									text_option = 4;

								switch (text_option)
								{
									//default:
								case 0:
									m_pWeenie->SendText(csprintf("You cast %s on %s", spell_name, castTarget->GetName().c_str()), LTT_MAGIC);
									if (m_pWeenie != topLevelOwner)
										topLevelOwner->SendText(csprintf("%s cast %s on %s", m_pWeenie->GetName().c_str(), spell_name, castTarget == topLevelOwner ? "you" : castTarget->GetName().c_str()), LTT_MAGIC);
									break;
								case 1:
									m_pWeenie->SendText(csprintf("You cast %s on %s, refreshing %s", spell_name, castTarget->GetName().c_str(), spell_name), LTT_MAGIC);
									if (m_pWeenie != topLevelOwner)
										topLevelOwner->SendText(csprintf("%s cast %s on %s, refreshing %s", m_pWeenie->GetName().c_str(), spell_name, castTarget == topLevelOwner ? "you" : castTarget->GetName().c_str(), spell_name), LTT_MAGIC);
									break;
								case 2:
									m_pWeenie->SendText(csprintf("You cast %s on %s, but it is surpassed by %s", spell_name, castTarget->GetName().c_str(), existing_spell_name.c_str()), LTT_MAGIC);
									if (m_pWeenie != topLevelOwner)
										topLevelOwner->SendText(csprintf("%s cast %s on %s, but it is surpassed by %s", m_pWeenie->GetName().c_str(), spell_name, castTarget == topLevelOwner ? "you" : castTarget->GetName().c_str(), existing_spell_name.c_str()), LTT_MAGIC);
									break;
								case 3:
									m_pWeenie->SendText(csprintf("You cast %s on %s, surpassing %s", spell_name, castTarget->GetName().c_str(), existing_spell_name.c_str()), LTT_MAGIC);
									if (m_pWeenie != topLevelOwner)
										topLevelOwner->SendText(csprintf("%s cast %s on %s, surpassing %s", m_pWeenie->GetName().c_str(), spell_name, castTarget == topLevelOwner ? "you" : castTarget->GetName().c_str(), existing_spell_name.c_str()), LTT_MAGIC);
									break;
								case 4:
									CWeenieObject* aetheriaOwner = m_pWeenie->GetWorldTopLevelOwner();
									aetheriaOwner->SendText(csprintf("Aetheria cast %s on you.", spell_name), LTT_MAGIC);
								}
							}
						}
					}

					bSpellPerformed = true;
				}
			}

			break;
		}
		case SpellType::FellowEnchantment_SpellType:
		{
			FellowshipEnchantmentSpellEx *meta = (FellowshipEnchantmentSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			Enchantment enchant;
			enchant._id = meta->_spell_id | ((uint32_t)m_SpellCastData.serial << (uint32_t)16);
			enchant.m_SpellSetID = 0; // ???
			enchant._spell_category = m_SpellCastData.spell->_category; // meta->_spellCategory;
			enchant._power_level = m_SpellCastData.spell->_power;
			enchant._start_time = Timer::cur_time;
			enchant._duration = m_SpellCastData.equipped ? -1.0 : meta->_duration;
			enchant._caster = m_pWeenie->GetID();
			enchant._degrade_modifier = meta->_degrade_modifier;
			enchant._degrade_limit = meta->_degrade_limit;
			enchant._last_time_degraded = -1.0;
			enchant._smod = meta->_smod;

			if (!target->HasFellowship())
				break;

			// NPKs should not be able to buff PKs
			if (m_pWeenie && target && m_pWeenie->IsPK() && target->IsPK())
			{
				bSpellPerformed = false;
				break;
			}
			else
			{
				fellowship_ptr_t fellow = target->GetFellowship();
				CWorldLandBlock *block = target->GetBlock();
				for (auto &entry : fellow->_fellowship_table)
				{
					if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
					{
						if (member->GetBlock() == block && VerifyPkAction(member))
						{
							std::list<CWeenieObject *> targets;

							if (member)
							{
								if (m_SpellCastData.spell->InqTargetType() != ITEM_TYPE::TYPE_ITEM_ENCHANTABLE_TARGET)
								{
									targets.push_back(member);
								}
								else
								{
									CContainerWeenie *container = member->AsContainer();
									if (container && !container->HasOwner())
									{
										for (auto wielded : container->m_Wielded)
										{
											if (wielded->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
											{
												if (member == m_pWeenie || wielded->parent) // for other targets, only physically wielded allowed
												{
													targets.push_back(wielded);
												}
											}
										}
									}
									else
									{
										if (member->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
										{
											if (member == m_pWeenie || member->parent || !member->HasOwner() || member->GetWorldTopLevelOwner() == m_pWeenie) // for other targets, only physically wielded allowed
											{
												targets.push_back(member);
											}
										}
									}
								}

								for (auto castTarget : targets)
								{
									// You cast Harlune's Blessing on yourself, refreshing Harlune's Blessing
									// You cast Impenetrability III on Pathwarden Robe, surpassing Impenetrability II

									CWeenieObject *topLevelOwner = castTarget->GetWorldTopLevelOwner();

									if (castTarget->InqIntQuality(MAX_STACK_SIZE_INT, 1) > 1) //do not allow enchanting stackable items(ammunition)
									{
										m_pWeenie->NotifyWeenieError(WERROR_MAGIC_BAD_TARGET_TYPE);
										continue;
									}

									bool bAlreadyExisted = false;
									if (castTarget->m_Qualities._enchantment_reg && castTarget->m_Qualities._enchantment_reg->IsEnchanted(enchant._id))
										bAlreadyExisted = true;

									if (m_pWeenie != castTarget)
									{
										if (m_SpellCastData.spell->_bitfield & Resistable_SpellIndex)
										{
											if (topLevelOwner->TryMagicResist(m_SpellCastData.current_skill))
											{
												topLevelOwner->EmitSound(Sound_ResistSpell, 1.0f, false);
												topLevelOwner->SendText(csprintf("You resist the spell cast by %s", m_pWeenie->GetName().c_str()), LTT_MAGIC);
												m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
												topLevelOwner->OnResistSpell(m_pWeenie);
												continue;
											}
										}

										if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
										{
											// Only Update PK Activity if both target and caster are PK, Prevents PKs from tagging NPKs for PK activity.
											if (m_pWeenie->IsPK() && castTarget->IsPK())
											{
												m_pWeenie->AsPlayer()->UpdatePKActivity();
												castTarget->AsPlayer()->UpdatePKActivity();
											}

											if (castTarget->AsPlayer() && castTarget->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
											{
												continue;
											}
										}

										if (int resistMagic = castTarget->InqIntQuality(RESIST_MAGIC_INT, 0, FALSE))
										{
											if (resistMagic >= 9999 && topLevelOwner == m_pWeenie)
												continue;

											if (resistMagic >= 9999 || ::TryMagicResist(m_SpellCastData.current_skill, (uint32_t)resistMagic))
											{
												castTarget->EmitSound(Sound_ResistSpell, 1.0f, false);

												if (m_pWeenie != topLevelOwner)
												{
													topLevelOwner->SendText(csprintf("%s resists the spell cast by %s", m_pWeenie->GetName().c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
												}

												m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
												castTarget->OnResistSpell(m_pWeenie);
												continue;
											}
										}
										if (bool resistProjectileMagic = castTarget->InqBoolQuality(NON_PROJECTILE_MAGIC_IMMUNE_BOOL, 0))
										{
											if (resistProjectileMagic == 1 || ::TryMagicResist(m_SpellCastData.current_skill, (uint32_t)resistProjectileMagic))
											{
												castTarget->EmitSound(Sound_ResistSpell, 1.0f, false);

												if (m_pWeenie != topLevelOwner)
												{
													topLevelOwner->SendText(csprintf("%s resists the spell cast by %s", m_pWeenie->GetName().c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
												}

												m_pWeenie->SendText(csprintf("%s resists your spell", castTarget->GetName().c_str()), LTT_MAGIC);
												castTarget->OnResistSpell(m_pWeenie);
												continue;
											}
										}
									}

									topLevelOwner->HandleAggro(m_pWeenie);

									castTarget->m_Qualities.UpdateEnchantment(&enchant);
									castTarget->NotifyEnchantmentUpdated(&enchant);

									castTarget->CheckVitalRanges();

									if (m_pWeenie == castTarget)
									{
										m_pWeenie->SendText(csprintf("You cast %s on yourself", m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
									}
									else
									{
										m_pWeenie->SendText(csprintf("You cast %s on %s", m_SpellCastData.spell->_name.c_str(), castTarget->GetName().c_str()), LTT_MAGIC);

										if (m_pWeenie != topLevelOwner)
										{
											if (castTarget == topLevelOwner)
												castTarget->SendText(csprintf("%s cast %s on you", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
											else
												topLevelOwner->SendText(csprintf("%s cast %s on %s", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), castTarget->GetName().c_str()), LTT_MAGIC);
										}
									}

									bSpellPerformed = true;

								}
							}

						}
					}
				}
				PerformFellowCastParticleEffects(fellow);
			}

			break;
		}
		case SpellType::PortalSummon_SpellType:
		{
			if (m_pWeenie && m_pWeenie->AsPlayer() && m_pWeenie->AsPlayer()->CheckPKActivity())
			{
				m_pWeenie->NotifyWeenieError(WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY);
				break;
			}

			PortalSummonSpellEx *meta = (PortalSummonSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
			if (!meta && (meta->_link == 1 || meta->_link == 2))
				break;

			Position spawnPos;
			if (!m_pWeenie->m_Qualities.InqPosition(PORTAL_SUMMON_LOC_POSITION, spawnPos))
			{
				spawnPos = GetCastSource()->m_Position;
				spawnPos.frame.m_origin += spawnPos.localtoglobalvec(Vector(0, 7, 0));
			}

			uint32_t portalDID = 0;

			Position dummyPos;
			bool bNoLink = false;
			Position portalPos;

			switch (meta->_link)
			{
			case 1:
				portalDID = m_pWeenie->InqDIDQuality(LINKED_PORTAL_ONE_DID, 0);
				portalPos = m_pWeenie->InqPositionQuality(LINKED_PORTAL_ONE_POSITION, Position());

				if (!portalDID && !m_pWeenie->m_Qualities.InqPosition(LINKED_PORTAL_ONE_POSITION, dummyPos))
				{
					m_pWeenie->NotifyWeenieError(WERROR_PORTAL_SUMMON_NO_LINK);
					bNoLink = true;
				}

				break;

			case 2:
				portalDID = m_pWeenie->InqDIDQuality(LINKED_PORTAL_TWO_DID, 0);
				portalPos = m_pWeenie->InqPositionQuality(LINKED_PORTAL_TWO_POSITION, Position());

				if (!portalDID && !m_pWeenie->m_Qualities.InqPosition(LINKED_PORTAL_TWO_POSITION, dummyPos))
				{
					m_pWeenie->NotifyWeenieError(WERROR_PORTAL_SUMMON_NO_LINK);
					bNoLink = true;
				}

				break;
			}

			if (bNoLink)
			{
				break;
			}

			CWeenieDefaults *portalDefaults = NULL;

			bool canFlagForQuest = false;
			if (portalDID)
			{
				portalDefaults = g_pWeenieFactory->GetWeenieDefaults(portalDID);

				if (portalDefaults)
				{
					int minLevel = 0;
					int maxLevel = 0;
					portalDefaults->m_Qualities.InqInt(MIN_LEVEL_INT, minLevel);
					portalDefaults->m_Qualities.InqInt(MAX_LEVEL_INT, maxLevel);

					if (m_pWeenie->AsPlayer())
					{
						// TODO BAD LOGIC
						int currentLevel = m_pWeenie->InqIntQuality(LEVEL_INT, 1);
						if (minLevel && currentLevel < minLevel)
						{
							m_pWeenie->SendText("You are not powerful enough to summon this portal yet.", LTT_MAGIC);
							break;
						}
						else if (maxLevel && currentLevel > maxLevel)
						{
							m_pWeenie->SendText("You are too powerful to summon this portal.", LTT_MAGIC);
							break;
						}
						// TODO END BAD LOGIC

						if ((portalDefaults->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x10))
						{
							m_pWeenie->NotifyWeenieError(WERROR_PORTAL_NOT_SUMMONABLE);
							break;
						}
					}
					else
						canFlagForQuest = true;
				}
			}

			CWeenieObject *weenie = g_pWeenieFactory->CreateWeenieByClassID(W_PORTALGATEWAY_CLASS, &spawnPos, false);

			if (weenie)
			{
				weenie->_timeToRot = Timer::cur_time + meta->_portal_lifetime;
				weenie->_beganRot = false;
				weenie->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, weenie->_timeToRot);

				bool bHasDestination = false;

				if (portalDefaults)
				{
					weenie->CopyPositionStat(DESTINATION_POSITION, &portalDefaults->m_Qualities);
					weenie->CopyIntStat(MIN_LEVEL_INT, &portalDefaults->m_Qualities);
					weenie->CopyIntStat(MAX_LEVEL_INT, &portalDefaults->m_Qualities);
					weenie->CopyIntStat(PORTAL_BITMASK_INT, &portalDefaults->m_Qualities);
					weenie->CopyStringStat(QUEST_RESTRICTION_STRING, &portalDefaults->m_Qualities);

					switch (meta->_link)
					{
					case 1:
						weenie->m_Qualities.SetDataID(ORIGINAL_PORTAL_DID, m_pWeenie->InqDIDQuality(LINKED_PORTAL_ONE_DID, 0));
						break;

					case 2:
						weenie->m_Qualities.SetDataID(ORIGINAL_PORTAL_DID, m_pWeenie->InqDIDQuality(LINKED_PORTAL_TWO_DID, 0));
						break;
					}

					if (canFlagForQuest)
						weenie->CopyStringStat(QUEST_STRING, &portalDefaults->m_Qualities);
					if (portalDefaults->m_Qualities.GetBool(PORTAL_SHOW_DESTINATION_BOOL, 0))
					{
						weenie->CopyBoolStat(PORTAL_SHOW_DESTINATION_BOOL, &portalDefaults->m_Qualities);
						weenie->CopyStringStat(APPRAISAL_PORTAL_DESTINATION_STRING, &portalDefaults->m_Qualities);
					}
				}
				else
				{
					switch (meta->_link)
					{
					case 1:
						weenie->m_Qualities.SetPosition(DESTINATION_POSITION, m_pWeenie->InqPositionQuality(LINKED_PORTAL_ONE_POSITION, Position()));
						weenie->m_Qualities.SetDataID(ORIGINAL_PORTAL_DID, m_pWeenie->InqDIDQuality(LINKED_PORTAL_ONE_DID, 0));
						break;

					case 2:
						weenie->m_Qualities.SetPosition(DESTINATION_POSITION, m_pWeenie->InqPositionQuality(LINKED_PORTAL_TWO_POSITION, Position()));
						weenie->m_Qualities.SetDataID(ORIGINAL_PORTAL_DID, m_pWeenie->InqDIDQuality(LINKED_PORTAL_TWO_DID, 0));
						break;
					}
				}

				g_pWorld->CreateEntity(weenie);

				bSpellPerformed = true;
			}

			break;
		}
		case SpellType::PortalSending_SpellType:
		{
			PortalSendingSpellEx *meta = (PortalSendingSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			if (m_pWeenie->AsPlayer() && m_pWeenie->AsPlayer()->CheckPKActivity() || m_pWeenie->HasOwner() && target->AsPlayer() && target->AsPlayer()->CheckPKActivity())
			{
				m_pWeenie->NotifyWeenieError(WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY);
				break;
			}

			if (target)
			{
				std::string spellName = m_SpellCastData.spellEx->_name; //need to set this now because movement_teleport will terminate m_SpellCastData.
				PerformCastParticleEffects(); // perform particle effects early because teleporting will cancel it
				

				if ((m_pWeenie != target) && (m_pWeenie->m_Qualities.m_WeenieType != Gem_WeenieType))
				{
					target->Movement_Teleport(meta->_pos, false);
					target->SendText(csprintf("%s teleports you with %s.", m_pWeenie->GetName().c_str(), spellName.c_str()), LTT_MAGIC);
				}
				else
				{
					
					BeginPortalSend(meta->_pos);
					target->SendText("You have been teleported.", LTT_MAGIC);
				}
			}

			break;
		}
		case SpellType::FellowPortalSending_SpellType:
		{
			FellowshipPortalSendingSpellEx *meta = (FellowshipPortalSendingSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

			if (!target->HasFellowship())
				break;

			else
			{
				fellowship_ptr_t fellow = target->GetFellowship();
				CWorldLandBlock *block = target->GetBlock();
				for (auto &entry : fellow->_fellowship_table)
				{
					if (CWeenieObject *member = g_pWorld->FindPlayer(entry.first))
					{
						if (member->GetBlock() == block && VerifyPkAction(member))
						{
							member->Movement_Teleport(meta->_pos, false);
							member->SendText("You have been teleported.", LTT_MAGIC);
						}
					}
				}
				PerformFellowCastParticleEffects(fellow);
			}

			break;
		}
		case SpellType::LifeProjectile_SpellType:
		case SpellType::Projectile_SpellType:
		{
			ProjectileSpellEx *meta = (ProjectileSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
			bSpellPerformed = LaunchProjectileSpell(meta);

			break;
		}
		case SpellType::EnchantmentProjectile_SpellType:
			ProjectileEnchantmentSpellEx *meta = (ProjectileEnchantmentSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
			bSpellPerformed = LaunchProjectileSpell(meta);
			break;
		}

	}

	if (bSpellPerformed)
	{
		PerformCastParticleEffects();
	}

	return WERROR_NONE;
}

bool CSpellcastingManager::VerifyPkAction(CWeenieObject * target)
{
	bool isSpellHelpful = m_SpellCastData.spellEx->_bitfield & Beneficial_SpellIndex;
	WErrorType attackerError = WERROR_NONE;
	WErrorType defenderError = WERROR_NONE;
	if (target && target->GetID() != m_pWeenie->GetID() && !m_pWeenie->IsValidPkAction(isSpellHelpful, (PKStatusEnum)m_pWeenie->m_Qualities.GetInt(PLAYER_KILLER_STATUS_INT, 1),
		(PKStatusEnum)target->m_Qualities.GetInt(PLAYER_KILLER_STATUS_INT, 1), attackerError, defenderError))
	{
		m_pWeenie->NotifyWeenieErrorWithString(attackerError, target->GetName().c_str());
		target->NotifyWeenieErrorWithString(defenderError, m_pWeenie->GetName().c_str());
		return false;
	}
	return true;
}

WErrorType CSpellcastingManager::CanPortalRecall(uint32_t portalDID, uint32_t linkedPortalEnum)
{
	WErrorType error = WERROR_NONE;

	if (portalDID == 0)
		return WERROR_PORTAL_RECALL_NO_LINK;

	if (portalDID == W_PORTALGATEWAY_CLASS && linkedPortalEnum == 0)
		return WERROR_PORTAL_RECALL_FAILED;

	CWeenieDefaults *portalDefaults = g_pWeenieFactory->GetWeenieDefaults(portalDID);

	if (portalDefaults)
	{
		if (m_pWeenie->InqIntQuality(LEVEL_INT, 1) < portalDefaults->m_Qualities.GetInt(MIN_LEVEL_INT, 1))
			return WERROR_PORTAL_LEVEL_TOO_LOW;

		if (m_pWeenie->InqIntQuality(LEVEL_INT, 1) > portalDefaults->m_Qualities.GetInt(MAX_LEVEL_INT, 275))
			return WERROR_PORTAL_LEVEL_TOO_HIGH;

		std::string restriction;
		if (portalDefaults->m_Qualities.InqString(QUEST_RESTRICTION_STRING, restriction))
		{
			if (CPlayerWeenie *player = m_pWeenie->AsPlayer())
			{
				if (!player->InqQuest(restriction.c_str()))
				{
					return WERROR_PORTAL_NOT_RECALLABLE;
				}
			}
		}

		if (linkedPortalEnum)
		{
			portalDefaults->m_Qualities.SetPosition(DESTINATION_POSITION, m_pWeenie->InqPositionQuality((STypePosition)linkedPortalEnum, Position()));
		}

		Position portalDest;
		if (!portalDefaults->m_Qualities.InqPosition(DESTINATION_POSITION, portalDest) || portalDest.objcell_id == 0)
		{
			return WERROR_PORTAL_RECALL_FAILED;
		}
		BeginPortalSend(portalDest);
	}
	else
	{
		return WERROR_PORTAL_RECALL_FAILED;
	}

	return error;
}


// You gain 37 points of health due to casting Drain Health Other I on Gelidite Lord
void CSpellcastingManager::TransferVitalPercent(CWeenieObject *target, float drainPercent, float infusePercent, STypeAttribute2nd attribute)
{
	if (!target || target->IsDead())
		return;

	bool reversed = (m_SpellCastData.spell->_bitfield & Reversed_SpellIndex) ? true : false;

	int targetAdjust;
	int selfAdjust;

	if (reversed)
	{
		// draining target, infusing self
		targetAdjust = -(int)ceil(target->GetHealth() * drainPercent);
		if (targetAdjust < -50)
			targetAdjust = -50;

		selfAdjust = (int)floor(-targetAdjust * infusePercent);
	}
	else
	{
		// draining self, infusing target
		selfAdjust = -(int)ceil(m_pWeenie->GetHealth() * drainPercent);
		targetAdjust = (int)floor(-selfAdjust * infusePercent);
	}

	if (targetAdjust < 0 && target->ImmuneToDamage(m_pWeenie))
	{
		// send notice they are immune?
		return;
	}

	const char *vitalName;
	switch (attribute)
	{
	case HEALTH_ATTRIBUTE_2ND:
	{
		selfAdjust = m_pWeenie->AdjustHealth(selfAdjust);
		targetAdjust = target->AdjustHealth(targetAdjust);
		vitalName = "health";
		break;
	}
	case STAMINA_ATTRIBUTE_2ND:
	{
		selfAdjust = m_pWeenie->AdjustStamina(selfAdjust);
		targetAdjust = target->AdjustStamina(targetAdjust);
		vitalName = "stamina";
		break;
	}
	case MANA_ATTRIBUTE_2ND:
	{
		selfAdjust = m_pWeenie->AdjustMana(selfAdjust);
		targetAdjust = target->AdjustMana(targetAdjust);
		vitalName = "mana";
		break;
	}
	default:
		return;
	}

	SendTransferVitalPercentText(target, reversed ? targetAdjust : selfAdjust, reversed ? selfAdjust : targetAdjust, reversed, vitalName);

	if (target->IsDead())
	{
		if (m_pWeenie == target)
		{
			target->NotifyVictimEvent("You died!");

			if (target->_IsPlayer())
			{
				target->NotifyDeathMessage(m_pWeenie->GetID(), csprintf("%s died!", target->GetName().c_str()));
			}
		}
		else
		{
			target->NotifyVictimEvent(csprintf("You were killed by %s!", m_pWeenie->GetName().c_str()));
			m_pWeenie->NotifyKillerEvent(csprintf("You killed %s!", target->GetName().c_str()));

			if (target->_IsPlayer())
			{
				target->NotifyDeathMessage(m_pWeenie->GetID(), csprintf("%s killed %s!", m_pWeenie->GetName().c_str(), target->GetName().c_str()));
			}
		}

		target->OnDeath(m_pWeenie->GetID());
	}

	if (m_pWeenie != target && m_pWeenie->IsDead())
	{
		m_pWeenie->NotifyVictimEvent("You died!");

		if (m_pWeenie->_IsPlayer())
		{
			m_pWeenie->NotifyDeathMessage(m_pWeenie->GetID(), csprintf("%s died!", m_pWeenie->GetName().c_str()));
		}

		m_pWeenie->OnDeath(m_pWeenie->GetID());
	}
}

void CSpellcastingManager::SendTransferVitalPercentText(CWeenieObject *target, int drained, int infused, bool reversed, const char *vitalName)
{
	// You gain 37 points of health due to casting Drain Health Other I on Gelidite Lord

	m_pWeenie->SendText(csprintf("You %s %d points of %s due to casting %s on %s",
		reversed ? "gain" : "lose", reversed ? infused : -drained, vitalName, m_SpellCastData.spell->_name.c_str(), target->GetName().c_str()), LTT_MAGIC);

	if (target != m_pWeenie)
	{
		target->SendText(csprintf("You %s %d points of %s due to %s casting %s on you",
			!reversed ? "gain" : "lose", !reversed ? infused : -drained, vitalName, m_SpellCastData.spell->_name.c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
	}
}

inline std::string GetAttribute2ndName(STypeAttribute2nd attribute2nd)
{
	switch (attribute2nd)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
		return "maximum health";
	case HEALTH_ATTRIBUTE_2ND:
		return "health";
	case MAX_STAMINA_ATTRIBUTE_2ND:
		return "maximum stamina";
	case STAMINA_ATTRIBUTE_2ND:
		return "stamina";
	case MAX_MANA_ATTRIBUTE_2ND:
		return "maximum mana";
	case MANA_ATTRIBUTE_2ND:
		return "mana";
	}

	return "";
}

bool CSpellcastingManager::DoTransferSpell(CWeenieObject *other, const TransferSpellEx *meta)
{
	// Calculate source amount
	CWeenieObject *source = NULL;
	if (meta->_bitfield & TransferSpellEx::SourceSelf)
		source = GetCastSource();
	else if (meta->_bitfield & TransferSpellEx::SourceOther)
		source = other;

	CWeenieObject *dest = NULL;
	if (meta->_bitfield & TransferSpellEx::DestinationSelf)
		dest = GetCastSource();
	else if (meta->_bitfield & TransferSpellEx::DestinationOther)
		dest = other;

	if (!source || !dest)
		return false;

	double drainResistMod = 1.0;
	double boostResistMod = 1.0;

	if (m_pWeenie != source)
	{
		// negative spell
		if (source->ImmuneToDamage(m_pWeenie))
		{
			return false;
		}

		// NPKs should not be able to heal PKs
		if (m_pWeenie && other && m_pWeenie->AsPlayer() && other->AsPlayer())
		{
			if (!m_pWeenie->IsPK() && other->IsPK())
			{
				return false;
			}
		}

		// try to resist
		if (m_SpellCastData.spell->_bitfield & Resistable_SpellIndex)
		{
			if (source->TryMagicResist(m_SpellCastData.current_skill))
			{
				source->EmitSound(Sound_ResistSpell, 1.0f, false);
				source->SendText(csprintf("You resist the spell cast by %s", m_pWeenie->GetName().c_str()), LTT_MAGIC);
				m_pWeenie->SendText(csprintf("%s resists your spell", source->GetName().c_str()), LTT_MAGIC);
				source->OnResistSpell(m_pWeenie);
				return false;
			}
		}

		source->HandleAggro(m_pWeenie);

		switch (meta->_src)
		{
		case HEALTH_ATTRIBUTE_2ND:
			source->m_Qualities.InqFloat(RESIST_HEALTH_DRAIN_FLOAT, drainResistMod);
			break;
		case STAMINA_ATTRIBUTE_2ND:
			source->m_Qualities.InqFloat(RESIST_STAMINA_DRAIN_FLOAT, drainResistMod);
			break;
		case MANA_ATTRIBUTE_2ND:
			source->m_Qualities.InqFloat(RESIST_MANA_DRAIN_FLOAT, drainResistMod);
			break;
		}

		if (source->AsPlayer()) //only players have natural resistances.
		{
			//Some combination of strength and endurance allows one to have a level of "natural resistances" to the 7 damage types.This caps out at a 50 % resistance(the equivalent to level 5 life prots) to these damage types.This resistance is not additive to life protections : higher level life protections will overwrite these natural resistances, although life vulns will take these natural resistances into account, if the player does not have a higher level life protection cast upon them.
			//For example, a player will not get a free protective bonus from natural resistances if they have both Prot 7 and Vuln 7 cast upon them.The Prot and Vuln will cancel each other out, and since the Prot has overwritten the natural resistances, there will be no resistance bonus.
			//The abilities that Endurance or Endurance/Strength conveys are not increased by Strength or Endurance buffs.It is the raw Strength and/or Endurance scores that determine the various bonuses.
			//drain resistances(same formula as natural resistances) allows one to partially resist drain health/stamina/mana and harm attacks, up to a maximum of roughly 50%. 

			//todo: natural resistances only change when base strength or endurance changes so we could potentially pre-calculate this somewhere else.
			uint32_t strength = 0;
			uint32_t endurance = 0;
			source->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			source->m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + endurance);
			float resistanceNatural;
			if (strAndEnd <= 200) //formula deduced from values present in the client pdb.
				resistanceNatural = 1.0f - ((0.05 * strAndEnd) / 100.f);
			else
				resistanceNatural = 1.0f - (((0.1666667 * strAndEnd) - 23.33333) / 100.f);

			resistanceNatural = max(resistanceNatural, 0.5f);

			if (resistanceNatural < drainResistMod)
				drainResistMod = resistanceNatural;
		}
	}

	if (m_pWeenie != dest)
	{
		switch (meta->_src)
		{
		case HEALTH_ATTRIBUTE_2ND:
			dest->m_Qualities.InqFloat(RESIST_HEALTH_BOOST_FLOAT, boostResistMod);
			break;
		case STAMINA_ATTRIBUTE_2ND:
			dest->m_Qualities.InqFloat(RESIST_STAMINA_DRAIN_FLOAT, boostResistMod);
			break;
		case MANA_ATTRIBUTE_2ND:
			dest->m_Qualities.InqFloat(RESIST_MANA_DRAIN_FLOAT, boostResistMod);
			break;
		}
	}



	uint32_t sourceStartValue = 0;
	uint32_t sourceMinValue = 0;
	uint32_t sourceMaxValue = 0;
	source->m_Qualities.InqAttribute2nd(meta->_src, sourceStartValue, FALSE);
	source->m_Qualities.InqAttribute2nd((STypeAttribute2nd)((int)meta->_src - 1), sourceMaxValue, FALSE);

	uint32_t destStartValue = 0;
	uint32_t destMinValue = 0;
	uint32_t destMaxValue = 0;
	dest->m_Qualities.InqAttribute2nd(meta->_dest, destStartValue, FALSE);
	dest->m_Qualities.InqAttribute2nd((STypeAttribute2nd)((int)meta->_dest - 1), destMaxValue, FALSE);

	int sourceTakeAmount = (int)(sourceStartValue * meta->_proportion * drainResistMod);
	if (meta->_transferCap && sourceTakeAmount > meta->_transferCap)
		sourceTakeAmount = meta->_transferCap;

	int destGiveAmount = (int)(sourceTakeAmount * (1.0 - meta->_lossPercent) * boostResistMod);

	int destResultValue = destStartValue + destGiveAmount;
	if (destResultValue > destMaxValue)
	{
		destResultValue = destMaxValue;
		destGiveAmount = destMaxValue - destStartValue;
		sourceTakeAmount = (int)(destGiveAmount / (1.0 - meta->_lossPercent));
	}
	int sourceResultValue = sourceStartValue - sourceTakeAmount;

	if (sourceResultValue != sourceStartValue)
	{
		source->m_Qualities.SetAttribute2nd(meta->_src, sourceResultValue);
		source->NotifyAttribute2ndStatUpdated(meta->_src);
	}

	if (destResultValue != destStartValue)
	{
		dest->m_Qualities.SetAttribute2nd(meta->_dest, destResultValue);
		dest->NotifyAttribute2ndStatUpdated(meta->_dest);
	}

	if (source == dest)
	{
		// You cast Stamina to Mana Self III on yourself and lose 78 points of stamina and also gain 86 points of mana
		if (m_pWeenie == source)
		{
			source->SendText(csprintf(
				"You cast %s on yourself and lose %d points of %s and also gain %d points of %s",
				m_SpellCastData.spell->_name.c_str(), sourceTakeAmount, GetAttribute2ndName(meta->_src).c_str(), destGiveAmount, GetAttribute2ndName(meta->_dest).c_str()
			), LTT_MAGIC);
		}
		else
		{
			source->SendText(csprintf(
				"You lose %d points of %s and also gain %d points of %s due to %s casting %s on you",
				sourceTakeAmount, GetAttribute2ndName(meta->_src).c_str(), destGiveAmount, GetAttribute2ndName(meta->_dest).c_str(), m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()
			), LTT_MAGIC);
		}
	}
	else
	{
		if (m_pWeenie == source)
		{
			source->SendText(csprintf("You lose %d points of %s due to casting %s on %s",
				sourceTakeAmount, GetAttribute2ndName(meta->_src).c_str(), m_SpellCastData.spell->_name.c_str(), dest->GetName().c_str()), LTT_MAGIC);
			dest->SendText(csprintf("You gain %d points of %s due to %s casting %s on you",
				destGiveAmount, GetAttribute2ndName(meta->_dest).c_str(), m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
		}
		else if (m_pWeenie == dest)
		{
			dest->SendText(csprintf("You gain %d points of %s due to casting %s on %s",
				destGiveAmount, GetAttribute2ndName(meta->_dest).c_str(), m_SpellCastData.spell->_name.c_str(), source->GetName().c_str()), LTT_MAGIC);
			source->SendText(csprintf("You lose %d points of %s due to %s casting %s on you",
				sourceTakeAmount, GetAttribute2ndName(meta->_src).c_str(), m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);

			if (dest->AsPlayer())
			{
				// update the target's health on the casting player asap
				if (source->AsPlayer())
					((CPlayerWeenie*)source)->RefreshTargetHealth();
			}
			if (source->AsPlayer())
			{
				// update the target's health on the casting player asap
				if (dest->AsPlayer())
					((CPlayerWeenie*)dest)->RefreshTargetHealth();
			}
		}
		else
		{
			// should never happen
		}
	}

	if (meta->_src == HEALTH_ATTRIBUTE_2ND)
		source->CheckDeath(m_pWeenie, DAMAGE_TYPE::HEALTH_DAMAGE_TYPE);

	return true;
}

bool CSpellcastingManager::AdjustVital(CWeenieObject *target)
{
	if (!target || target->IsDead())
		return false;

	// NPKs should not be able to heal PKs
	if (m_pWeenie && target && m_pWeenie->AsPlayer() && target->AsPlayer())
	{
		if (!m_pWeenie->IsPK() && target->IsPK())
		{
			return false;
		}
	}

	BoostSpellEx *meta = (BoostSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

	bool isDamage = (meta->_boost < 0);
	int boostMin = abs(meta->_boost);
	int boostMax = abs(meta->_boost + meta->_boostVariance);

	double preVarianceDamage = boostMax;
	double damageOrHealAmount = Random::RollDice(boostMin, boostMax);

	CWeenieObject *wand = g_pWorld->FindObject(m_SpellCastData.wand_id);
	if (wand)
	{
		double elementalDamageMod = wand->InqDamageType() == meta->_dt ? wand->InqFloatQuality(ELEMENTAL_DAMAGE_MOD_FLOAT, 1.0) : 1.0;
		if (m_pWeenie->AsPlayer() && target->AsPlayer()) //pvp
			elementalDamageMod = ((elementalDamageMod - 1.0) / 2.0) + 1.0;
		damageOrHealAmount *= elementalDamageMod;
	}

	// negative spell
	if (isDamage)
	{
		if (m_pWeenie && target && m_pWeenie->AsPlayer() && target->AsPlayer())
		{
			if (m_pWeenie->IsPK() && target->IsPK())
			{
				m_pWeenie->AsPlayer()->UpdatePKActivity();
				target->AsPlayer()->UpdatePKActivity();
			}
		}

		// try to resist
		if (m_SpellCastData.spell->_bitfield & Resistable_SpellIndex)
		{
			if (m_pWeenie != target)
			{
				if (target->TryMagicResist(m_SpellCastData.current_skill))
				{
					target->EmitSound(Sound_ResistSpell, 1.0f, false);

					if (m_pWeenie)
					{
						target->SendText(csprintf("You resist the spell cast by %s", m_pWeenie->GetName().c_str()), LTT_MAGIC);
						m_pWeenie->SendText(csprintf("%s resists your spell", target->GetName().c_str()), LTT_MAGIC);
						target->OnResistSpell(m_pWeenie);
					}
					return false;
				}
			}
		}

		DamageEventData dmgEvent;
		dmgEvent.source = m_pWeenie;
		dmgEvent.target = target;
		dmgEvent.weapon = wand;
		dmgEvent.damage_form = DF_MAGIC;
		dmgEvent.damage_type = meta->_dt;
		dmgEvent.hit_quadrant = DAMAGE_QUADRANT::DQ_UNDEF; //should spells have hit quadrants?
		dmgEvent.attackSkill = m_SpellCastData.spell->InqSkillForSpell();
		dmgEvent.attackSkillLevel = m_SpellCastData.current_skill;
		dmgEvent.preVarianceDamage = preVarianceDamage;
		dmgEvent.baseDamage = damageOrHealAmount;

		dmgEvent.isProjectileSpell = false;
		dmgEvent.spell_name = m_SpellCastData.spell->_name;

		CalculateCriticalHitData(&dmgEvent, &m_SpellCastData);
		dmgEvent.wasCrit = (Random::GenFloat(0.0, 1.0) < dmgEvent.critChance) ? true : false;

		CalculateDamage(&dmgEvent, &m_SpellCastData);

		m_pWeenie->TryToDealDamage(dmgEvent);

		return true;
	}
	else
	{
		const char *vitalName;

		switch (meta->_dt)
		{
		case HEALTH_DAMAGE_TYPE:
		{
			damageOrHealAmount = target->AdjustHealth(damageOrHealAmount);
			vitalName = "health";
			break;
		}
		case STAMINA_DAMAGE_TYPE:
		{
			damageOrHealAmount = target->AdjustStamina(damageOrHealAmount);
			vitalName = "stamina";
			break;
		}
		case MANA_DAMAGE_TYPE:
		{
			damageOrHealAmount = target->AdjustMana(damageOrHealAmount);
			vitalName = "mana";
			break;
		}
		default:
			return false;
		}

		SendAdjustVitalText(target, damageOrHealAmount, vitalName);

		target->CheckDeath(GetCastSource(), meta->_dt);

		return true;
	}
}

void CSpellcastingManager::SendAdjustVitalText(CWeenieObject *target, int amount, const char *vitalName)
{
	bool bRestore = true;
	if (amount < 0 || (amount == 0 && !(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex)))
	{
		bRestore = false;
		amount = -amount;
	}

	if (m_pWeenie != target)
	{
		m_pWeenie->SendText(csprintf("With %s you %s %d points of %s %s %s.",
			m_SpellCastData.spell->_name.c_str(), bRestore ? "restore" : "drain", amount, vitalName, bRestore ? "to" : "from", target->GetName().c_str()), LTT_MAGIC);

		target->SendText(csprintf("%s casts %s and %s %d points of your %s.",
			m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), bRestore ? "restores" : "drains", amount, vitalName), LTT_MAGIC);

		if (vitalName == "health")
		{
			if (m_pWeenie->AsPlayer())
			{
				// update the target's health on the casting player asap
				((CPlayerWeenie*)m_pWeenie)->RefreshTargetHealth();
			}
		}
	}
	else
	{
		m_pWeenie->SendText(csprintf("You cast %s and %s %d points of your %s.",
			m_SpellCastData.spell->_name.c_str(), bRestore ? "restore" : "drain", amount, vitalName), LTT_MAGIC);
	}
}

int CSpellcastingManager::CreatureBeginCast(uint32_t target_id, uint32_t spell_id)
{
	if (m_bCasting)
		return WERROR_ACTIONS_LOCKED;

	m_SpellCastData = SpellCastData();
	m_SpellCastData.caster_id = m_pWeenie->GetID();
	m_SpellCastData.source_id = m_pWeenie->GetTopLevelID();
	m_SpellCastData.target_id = target_id;
	m_SpellCastData.spell_id = spell_id;
	m_SpellCastData.wand_id = m_pWeenie->GetWieldedCasterID();

	m_SpellCastData.cast_timeout = Timer::cur_time + 10.0f;
	m_SpellCastData.initial_cast_position = m_pWeenie->m_Position;

	if (!ResolveSpellBeingCasted())
	{
		SERVER_INFO << "Player trying to cast unknown spell?";
		return WERROR_MAGIC_GENERAL_FAILURE;
	}

	// if the spell uses mana and we use mana then check we've got enough
	if (m_SpellCastData.uses_mana && m_pWeenie->InqBoolQuality(AI_USES_MANA_BOOL, 1) && m_pWeenie->GetMana() < m_SpellCastData.spell->_base_mana)
	{
		return WERROR_MAGIC_INSUFFICIENT_MANA;
	}

	// if we're at max hp, no need to cast self heals
	if (m_SpellCastData.spell->_category == 67 && (m_pWeenie->GetHealth() == m_pWeenie->GetMaxHealth()))
	{
		return WERROR_NONE;
	}

	m_bCasting = true;
	m_PendingMotions.clear();
	m_bTurningToObject = false;

	m_PendingMotions.push_back(SpellCastingMotion(Motion_CastSpell, 2.0f, true, true, 2.0f));
	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();

	// if we use our mana pool then adjust it for the spell cost
	if (m_SpellCastData.uses_mana && m_pWeenie->InqBoolQuality(AI_USES_MANA_BOOL, 1))
	{
		m_pWeenie->AdjustMana(-(m_SpellCastData.spell->_base_mana)); //should this use GetManaCost to adjust for skill/mana c?
	}

	BeginNextMotion();

	return WERROR_NONE;
}


int CSpellcastingManager::CastSpellInstant(uint32_t target_id, uint32_t spell_id)
{
	int error = WERROR_NONE;

	// incase we are interrupting a cast
	SpellCastData oldData;
	if (m_bCasting)
		oldData = m_SpellCastData;
	bool bOldCasting = m_bCasting;

	m_bCasting = true;
	m_SpellCastData = SpellCastData();
	m_SpellCastData.caster_id = m_pWeenie->GetID();
	m_SpellCastData.source_id = m_pWeenie->GetTopLevelID();
	m_SpellCastData.target_id = target_id;
	m_SpellCastData.spell_id = spell_id;
	m_SpellCastData.wand_id = m_pWeenie->GetWieldedCasterID();
	m_SpellCastData.cast_timeout = Timer::cur_time + 10.0f;
	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();
	m_SpellCastData.initial_cast_position = m_pWeenie->m_Position;
	m_SpellCastData.uses_mana = false;

	if (ResolveSpellBeingCasted())
	{
		LaunchSpellEffect(false);
	}
	else
	{
		error = WERROR_MAGIC_GENERAL_FAILURE;
	}

	if (bOldCasting)
		m_SpellCastData = oldData;
	m_bCasting = bOldCasting;

	return error;
}


int CSpellcastingManager::CastSpellEquipped(uint32_t target_id, uint32_t spell_id, WORD serial, uint32_t set_id, bool silent)
{
	int error = WERROR_NONE;

	// incase we are interrupting a cast
	SpellCastData oldData = m_SpellCastData;
	bool bOldCasting = m_bCasting;

	m_bCasting = true;
	m_SpellCastData = SpellCastData();
	m_SpellCastData.caster_id = m_pWeenie->GetID();
	m_SpellCastData.source_id = m_pWeenie->GetTopLevelID();
	m_SpellCastData.target_id = target_id;
	m_SpellCastData.spell_id = spell_id;
	m_SpellCastData.wand_id = m_pWeenie->GetWieldedCasterID();
	m_SpellCastData.cast_timeout = Timer::cur_time + 10.0f;
	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();
	m_SpellCastData.initial_cast_position = m_pWeenie->m_Position;
	m_SpellCastData.uses_mana = false;
	m_SpellCastData.equipped = true;
	m_SpellCastData.serial = serial;
	m_SpellCastData.set_id = set_id;

	if (ResolveSpellBeingCasted())
	{
		LaunchSpellEffect(false, silent);
	}
	else
	{
		error = WERROR_MAGIC_GENERAL_FAILURE;
	}

	m_SpellCastData = oldData;
	m_bCasting = bOldCasting;

	return error;
}

uint32_t CSpellcastingManager::DetermineSkillLevelForSpell()
{
	int spellcraft = 0;
	CWeenieObject *source = g_pWorld->FindObject(m_SpellCastData.caster_id);

	if (!source)
		return 0;

	if (source->m_Qualities.InqInt(ITEM_SPELLCRAFT_INT, spellcraft, FALSE, FALSE))
		return (uint32_t)spellcraft;

	if (!m_SpellCastData.spell)
		return 0;

	uint32_t skillLevel = 0;
	STypeSkill skill = m_SpellCastData.spell->InqSkillForSpell();

	if (skill)
	{
		m_pWeenie->InqSkill(skill, skillLevel, FALSE);
	}
	else
	{
		uint32_t creatureEnch = 0, itemEnch = 0, life = 0, war = 0, voidMagic = 0;
		m_pWeenie->InqSkill(CREATURE_ENCHANTMENT_SKILL, creatureEnch, FALSE);
		m_pWeenie->InqSkill(ITEM_ENCHANTMENT_SKILL, itemEnch, FALSE);
		m_pWeenie->InqSkill(LIFE_MAGIC_SKILL, life, FALSE);
		m_pWeenie->InqSkill(WAR_MAGIC_SKILL, war, FALSE);
		m_pWeenie->InqSkill(VOID_MAGIC_SKILL, voidMagic, FALSE);

		uint32_t highestSkill = 0;

		highestSkill = max(highestSkill, creatureEnch);
		highestSkill = max(highestSkill, itemEnch);
		highestSkill = max(highestSkill, life);
		highestSkill = max(highestSkill, war);
		highestSkill = max(highestSkill, voidMagic);

		skillLevel = highestSkill;
	}
	return skillLevel;
}

double CSpellcastingManager::DetermineSpellRange()
{
	if (!m_SpellCastData.spell)
		return 0.0;

	double range = 0.0;

	if (m_SpellCastData.spellEx)
		range = m_SpellCastData.spellEx->_base_range_constant + m_SpellCastData.spellEx->_base_range_mod * m_SpellCastData.current_skill;
	else
		range = m_SpellCastData.spell->_base_range_constant + m_SpellCastData.spell->_base_range_mod * m_SpellCastData.current_skill;

	const float RADAR_OUTDOOR_RADIUS = 75.0f;
	if (range > RADAR_OUTDOOR_RADIUS)
		range = RADAR_OUTDOOR_RADIUS;

	return range;
}

int CSpellcastingManager::CheckTargetValidity()
{
	if (!m_SpellCastData.spell)
		return WERROR_MAGIC_GENERAL_FAILURE;

	CWeenieObject *pCastSource = GetCastSource();
	if (!pCastSource)
		return WERROR_OBJECT_GONE;

	CWeenieObject *pTarget = g_pWorld->FindWithinPVS(pCastSource, m_SpellCastData.target_id, true); //we can cast on items owned by others!

	if (!(m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex))
	{
		if (!pTarget)
			return WERROR_OBJECT_GONE;

		if (!pTarget->HasOwner() || !pCastSource->FindContainedItem(pTarget->GetID()))
		{
			if (pCastSource && m_SpellCastData.range_check)
			{
				if (pCastSource->DistanceTo(pTarget, true) > m_SpellCastData.max_range)
					return WERROR_MISSILE_OUT_OF_RANGE;
			}
		}
	}

	int targetType = m_SpellCastData.spell->InqTargetType();
	if (targetType != ITEM_TYPE::TYPE_UNDEF)
	{
		if (!pTarget)
			return WERROR_OBJECT_GONE;

		if (targetType == ITEM_TYPE::TYPE_ITEM_ENCHANTABLE_TARGET && !m_SpellCastData.equipped)
		{
			targetType |= TYPE_CREATURE;
		}

		if (!(pTarget->GetItemType() & targetType))
		{
			if (m_SpellCastData.equipped && (m_SpellCastData.spell->_school == ItemEnchantment_Magic) && (m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex))
			{
			}
			else
			{
				return WERROR_MAGIC_BAD_TARGET_TYPE;
			}
		}
	}

	if (pTarget->IsCreature())
	{
		if (pTarget->IsVendor())
			return WERROR_MAGIC_BAD_TARGET_TYPE;
	}

	if (!(m_SpellCastData.spell->_bitfield & (Beneficial_SpellIndex | NotResearchable_SpellIndex | SelfTargeted_SpellIndex))) //Not researchable bitmask to allow sending spells to function properly. SelfTargeted to allow portal summon gems to function.
	{
		if (!pTarget)
			return WERROR_OBJECT_GONE;

		if (pTarget->IsCreature() && !pTarget->IsAttackable() && (pTarget != m_pWeenie))//no harmful spells on unattackable players/admins/sentinels. Exclude self to cast AoE while unattackable.
		{
			if (pCastSource && m_SpellCastData.range_check)
			{
				if (pCastSource->DistanceTo(pTarget, true) > m_SpellCastData.max_range)
					return WERROR_MISSILE_OUT_OF_RANGE;
			}
			return WERROR_PK_PROTECTED_TARGET;
		}
	}

	if (m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex)
	{
		if (!pTarget)
			return WERROR_OBJECT_GONE;

		if (pTarget->IsCreature() && !pTarget->AsPlayer() && m_pWeenie->AsPlayer())//no buffing mobs if we are a player.
		{
			if (pCastSource && m_SpellCastData.range_check)
			{
				if (pCastSource->DistanceTo(pTarget, true) > m_SpellCastData.max_range)
					return WERROR_MISSILE_OUT_OF_RANGE;
			}
			return  WERROR_MAGIC_BAD_TARGET_TYPE;
		}

	}

	return WERROR_NONE;
}

int CSpellcastingManager::GenerateManaCost(bool useMod)
{
	uint32_t manaConvSkill = m_pWeenie->GetEffectiveManaConversionSkill();

	// not all level spells are the same.  some should be harder to save on.  power seems the balancer
	int difficulty = 50 + m_SpellCastData.spell->_power;

	return GetManaCost(m_SpellCastData.current_skill, difficulty, useMod ? m_SpellCastData.spell->_mana_mod : m_SpellCastData.spell->_base_mana, manaConvSkill);
}

bool CSpellcastingManager::IsArmorBuff(uint32_t category)
{
	if (category == Armor_Value_Raising_SpellCategory || category == Acid_Resistance_Raising_SpellCategory || category == Bludgeon_Resistance_Raising_SpellCategory
		|| category == Cold_Resistance_Raising_SpellCategory || category == Electric_Resistance_Raising_SpellCategory ||
		category == Fire_Resistance_Raising_SpellCategory || category == Pierce_Resistance_Raising_SpellCategory || category == Slash_Resistance_Raising_SpellCategory)
		return true;
	return false;
}

int CSpellcastingManager::TryBeginCast(uint32_t target_id, uint32_t spell_id)
{
	if (m_bCasting)
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Actions Locked, Casting = true, Turning = %s", m_bTurningToObject ? "true" : "false"), LTT_ALL_CHANNELS);
		return WERROR_ACTIONS_LOCKED;
	}
	if (m_pWeenie->IsInPortalSpace())
		return WERROR_ACTIONS_LOCKED;

	if (m_pWeenie->get_minterp()->interpreted_state.actions.size())
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Actions Locked, Interp state has %d actions.", m_pWeenie->get_minterp()->interpreted_state.actions.size()), LTT_ALL_CHANNELS);
		return WERROR_ACTIONS_LOCKED;
	}

	//if (m_pWeenie->get_minterp()->interpreted_state.forward_command != 0x41000003)
	//{
		// m_pWeenie->SendText(csprintf("DEBUG: Unprepared, Interp state forward command is 0x%08X, not idle.", m_pWeenie->get_minterp()->interpreted_state.forward_command), LTT_ALL_CHANNELS);
	//	return WERROR_MAGIC_UNPREPARED;
//	}

	if (!m_pWeenie->m_Qualities.IsSpellKnown(spell_id))
	{
		// Don't return the unlearned spell error for Cast on Strike
		if (!(m_pWeenie->AsMeleeWeapon() || m_pWeenie->AsMissileLauncher()) && spell_id != m_pWeenie->InqDIDQuality(PROC_SPELL_DID, 0))
		{
			return WERROR_MAGIC_UNLEARNED_SPELL;
		}
	}

	if (Timer::cur_time < m_fNextCastTime)
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Actions Locked, Can't cast for %.3f more seconds.", Timer::cur_time - m_fNextCastTime), LTT_ALL_CHANNELS);
		return WERROR_ACTIONS_LOCKED;
	}

	m_SpellCastData = SpellCastData(); // reset
	m_SpellCastData.caster_id = m_pWeenie->GetID();
	m_SpellCastData.source_id = m_pWeenie->GetTopLevelID();
	m_SpellCastData.target_id = target_id;
	m_SpellCastData.spell_id = spell_id;
	m_SpellCastData.wand_id = m_pWeenie->GetWieldedCasterID();
	m_SpellCastData.cast_timeout = Timer::cur_time + 10.0f;
	m_SpellCastData.initial_cast_position = m_pWeenie->m_Position;

	if (!ResolveSpellBeingCasted())
	{
		SERVER_INFO << "Player trying to cast unknown spell?";
		return WERROR_MAGIC_GENERAL_FAILURE;
	}

	if (!m_SpellCastData.current_skill)
	{
		m_pWeenie->SendText("You are not trained in that skill!", LTT_MAGIC);
		return WERROR_MAGIC_INVALID_SPELL_TYPE;
	}

	// m_pWeenie->SendText(csprintf("Casting %d", m_SpellCastData.spell_id), LTT_DEFAULT);

	int targetError = CheckTargetValidity();
	if (targetError)
	{
		CWeenieObject * pTarget = g_pWorld->FindObject(m_SpellCastData.target_id);
		switch (targetError)
		{
		case WERROR_PK_PROTECTED_TARGET:
		{
			if (pTarget)
			{
				auto targetName = pTarget->GetName();
				pTarget->SendText(csprintf("%s fails to affect you because you cannot be harmed!", m_pWeenie->GetName().c_str()), LTT_MAGIC);
				m_pWeenie->SendText(csprintf("You fail to affect %s because %s cannot be harmed!", targetName.c_str(), targetName.c_str()), LTT_MAGIC);
			}
			return WERROR_NONE;
		}
		case WERROR_MAGIC_BAD_TARGET_TYPE:
		{
			if (pTarget)
			{
				m_pWeenie->EmitSound(Sound_UI_GeneralError, 1.0f, true);
				m_pWeenie->NotifyWeenieErrorWithString(WERROR_MAGIC_BAD_TARGET_TYPE, pTarget->GetName().c_str());
			}
			return WERROR_NONE;
		}
		}

		return targetError;
	}

	//Components
	CContainerWeenie *caster = m_pWeenie->AsContainer();
	if (caster != NULL && caster->InqBoolQuality(SPELL_COMPONENTS_REQUIRED_BOOL, FALSE) == TRUE)
	{
		bool foci;

		if (g_pConfig->IsSpellFociEnabled())
		{
			switch (m_SpellCastData.spell->InqSkillForSpell())
			{
			case CREATURE_ENCHANTMENT_SKILL: foci = FindFociInContainer(caster, W_PACKCREATUREESSENCE_CLASS) || caster->InqIntQuality(AUGMENTATION_INFUSED_CREATURE_MAGIC_INT, 0); break;
			case ITEM_ENCHANTMENT_SKILL: foci = FindFociInContainer(caster, W_PACKITEMESSENCE_CLASS) || caster->InqIntQuality(AUGMENTATION_INFUSED_ITEM_MAGIC_INT, 0); break;
			case LIFE_MAGIC_SKILL: foci = FindFociInContainer(caster, W_PACKLIFEESSENCE_CLASS) || caster->InqIntQuality(AUGMENTATION_INFUSED_LIFE_MAGIC_INT, 0); break;
			case WAR_MAGIC_SKILL: foci = FindFociInContainer(caster, W_PACKWARESSENCE_CLASS) || caster->InqIntQuality(AUGMENTATION_INFUSED_WAR_MAGIC_INT, 0); break;
			case VOID_MAGIC_SKILL: foci = FindFociInContainer(caster, W_FOCIOFSHADOW_CLASS) || caster->InqIntQuality(AUGMENTATION_INFUSED_VOID_MAGIC_INT, 0); break;
			}
		}

		SpellFormula randomizedComponents;
		randomizedComponents.CopyFrom(m_SpellCastData.spell->InqSpellFormula());

		CPlayerWeenie *player = m_pWeenie->AsPlayer();
		if (player)
			randomizedComponents.RandomizeForName(player->GetClient()->GetAccount(), m_SpellCastData.spell->_formula_version);

		std::map<uint32_t, uint32_t> componentAmounts;
		for (uint32_t componentId : randomizedComponents._comps)
		{
			if (componentId == 0)
				continue;

			if (foci != FALSE)
			{
				SpellComponentTable *pSpellComponents = MagicSystem::GetSpellComponentTable();
				const SpellComponentBase *componentBase = pSpellComponents->InqSpellComponentBase(componentId);
				switch (componentBase->_type)
				{
				case SpellComponentType::Power_SpellComponentType: //scarabs
					break;
				case SpellComponentType::Accent_SpellComponentType: //tapers
					componentId = 188; //turn tapers into prismatic tapers.
					break;
				default:
					continue;
				}
			}

			componentAmounts[componentId]++;
		}
		if (foci != FALSE)
			componentAmounts[188]++; //if using foci, add one more prismatic taper.

		m_UsedComponents.clear(); // clear the list of left overs from previous interrupted spell attempts.
		for (std::map<uint32_t, uint32_t>::iterator iter = componentAmounts.begin(); iter != componentAmounts.end(); ++iter)
		{
			uint32_t compId = iter->first;
			uint32_t amount = iter->second;

			std::map<uint32_t, uint32_t> components = FindComponentInContainer(caster, compId, amount);

			if (!components.empty())
			{
				for (std::map<uint32_t, uint32_t>::iterator iter = components.begin(); iter != components.end(); ++iter)
					m_UsedComponents[iter->first] += iter->second;
			}
			else
				return WERROR_MAGIC_MISSING_COMPONENTS;
		}
	}

	if ((m_pWeenie->AsMeleeWeapon() || m_pWeenie->AsMissileLauncher()) && (m_SpellCastData.spellEx->_category >= DF_Attack_Skill_Debuff_SpellCategory
		&& m_SpellCastData.spellEx->_category <= DF_Healing_Debuff_SpellCategory))
	{
		m_bCasting = true;
		m_SpellCastData.current_skill = 0;
		m_SpellCastData.uses_mana = false;
		int error = LaunchSpellEffect(FALSE);
		EndCast(error);
		return WERROR_NONE;
	}

	if (!(m_pWeenie->AsMeleeWeapon() || m_pWeenie->AsMissileLauncher()))
	{
		if (m_pWeenie->GetMana() < GenerateManaCost())
			return WERROR_MAGIC_INSUFFICIENT_MANA;
	}
	else if (m_pWeenie->InqIntQuality(ITEM_CUR_MANA_INT, 0, 0) < GenerateManaCost())
		return WERROR_MAGIC_INSUFFICIENT_MANA;
	else
	{
		m_bCasting = true;
		m_SpellCastData.current_skill = m_pWeenie->InqIntQuality(ITEM_SPELLCRAFT_INT, 0, 0);
		int error = LaunchSpellEffect(FALSE);
		EndCast(error);
		return WERROR_NONE;
	}

	if (m_pWeenie->AsPlayer())
		m_pWeenie->AsPlayer()->CancelLifestoneProtection();

	BeginCast();
	return WERROR_NONE;
}

std::map<uint32_t, uint32_t> CSpellcastingManager::FindComponentInContainer(CContainerWeenie *container, unsigned int componentId, int amountNeeded)
{
	std::map<uint32_t, uint32_t> foundItems;
	int amountLeftToFind = amountNeeded;
	for (auto item : container->m_Items)
	{
		if (item->InqDIDQuality(SPELL_COMPONENT_DID, 0) == componentId)
		{
			int amount = item->InqIntQuality(STACK_SIZE_INT, 1);
			if (amount > amountLeftToFind)
			{
				amount = amountLeftToFind;
				amountLeftToFind = 0;
			}
			else
				amountLeftToFind -= amount;
			foundItems.emplace(item->GetID(), amount);

			if (amountLeftToFind == 0)
				return foundItems;
		}
	}

	for (auto packSlot : container->m_Packs)
	{
		CContainerWeenie *pack = packSlot->AsContainer();
		if (pack != NULL)
		{
			for (auto item : pack->m_Items)
			{
				if (item->InqDIDQuality(SPELL_COMPONENT_DID, 0) == componentId)
				{
					int amount = item->InqIntQuality(STACK_SIZE_INT, 1);
					if (amount > amountLeftToFind)
					{
						amount = amountLeftToFind;
						amountLeftToFind = 0;
					}
					else
						amountLeftToFind -= amount;
					foundItems.emplace(item->GetID(), amount);

					if (amountLeftToFind == 0)
						return foundItems;
				}
			}
		}
	}

	//we didnt find everything, so empty our list and return
	foundItems.clear();
	return foundItems;
}

CWeenieObject *CSpellcastingManager::FindFociInContainer(CContainerWeenie *container, uint32_t fociWcid)
{
	for (auto pack : container->m_Packs)
	{
		if (pack->m_Qualities.id == fociWcid)
			return pack;
	}

	return NULL;
}

void CSpellcastingManager::Update()
{
	if (!m_bCasting)
	{
		return;
	}

	if (m_SpellCastData.cast_timeout <= Timer::cur_time)
	{
		EndCast(WERROR_MAGIC_GENERAL_FAILURE);
		m_bCasting = false;
	}

	if (m_pWeenie->AsPlayer() && m_pWeenie->IsInPeaceMode())
	{
		// fizzle if you're no longer in combat mode.
		m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
		m_pWeenie->AdjustMana(-5);

		int error = LaunchSpellEffect(TRUE);
		EndCast(error);
	}

	if ((m_SpellCastData.next_update <= Timer::cur_time) && m_bTurned && m_pWeenie->m_Position.distance(m_SpellCastData.initial_cast_position) >= 6.0)
	{
		// fizzle
		m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
		m_pWeenie->AdjustMana(-5);
		m_pWeenie->SendText("Your movement disrupted spell casting!", LTT_MAGIC);
		m_pWeenie->SendText("Your movement disrupted spell casting!", LTT_ERROR);

		int error = LaunchSpellEffect(TRUE);
		EndCast(error);
	}

	if (m_bTurningToObject)
	{
		if (m_pWeenie->movement_manager->moveto_manager->movement_type != MovementTypes::TurnToObject)
		{
			if (HeadingToTarget() <= MAX_HEADING_TO_TARGET_FOR_CAST)
			{
				// if (!m_pWeenie->get_minterp()->interpreted_state.turn_command)
				{
					BeginNextMotion();
				}
			}
			else
			{
				if ((m_SpellCastData.next_update <= Timer::cur_time) && !m_pWeenie->get_minterp()->interpreted_state.turn_command)
				{
					MovementParameters params;
					params.speed = 1.0f;
					params.action_stamp = ++m_pWeenie->m_wAnimSequence;
					m_pWeenie->last_move_was_autonomous = false;
					m_pWeenie->TurnToObject(m_SpellCastData.target_id, &params);
					m_bTurned = true;
					m_SpellCastData.next_update = Timer::cur_time + CAST_UPDATE_RATE;
					//m_bTurningToObject = true;
					//m_fCastTimeout = Timer::cur_time + MAX_TURN_TIME_FOR_CAST;
				}
			}
		}
		else
		{
			/*
			CWeenieObject *pTarget = g_pWorld->FindWithinPVS(m_pWeenie, m_TargetID);
			if (pTarget)
			{
				LOG(Temp, Normal, "%f (%f %f) %08X %f %f %f\n",
					HeadingToTarget(),
					m_pWeenie->m_Position.heading(pTarget->m_Position),
					m_pWeenie->m_Position.frame.get_heading(),
					m_pWeenie->get_minterp()->interpreted_state.turn_command,
					m_pWeenie->movement_manager->moveto_manager->sought_position.frame.get_heading(),
					m_pWeenie->get_heading(),
					m_pWeenie->get_heading() - m_pWeenie->movement_manager->moveto_manager->sought_position.frame.get_heading());
			}
			*/
		}
	}
}

void CSpellcastingManager::Cancel()
{
	if (!m_bCasting)
	{
		return;
	}

	EndCast(WErrorType::WERROR_MAGIC_UNPREPARED);
	m_bCasting = false;
}

void CSpellcastingManager::OnDeath(uint32_t killer_id)
{
	Cancel();
}

void CSpellcastingManager::HandleMotionDone(uint32_t motion, BOOL success)
{
	if (!m_bCasting)
		return;

	if (m_bTurningToObject)
		return;

	if (m_PendingMotions.empty() || m_PendingMotions.begin()->motion != motion)
		return;

	if (!success)
	{
		EndCast(WERROR_MAGIC_GENERAL_FAILURE);
		return;
	}

	m_PendingMotions.pop_front();
	BeginNextMotion();
}
