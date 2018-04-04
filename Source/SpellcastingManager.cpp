
#include "StdAfx.h"
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

const float MAX_HEADING_TO_TARGET_FOR_CAST = 45.0f;
const float MAX_TURN_TIME_FOR_CAST = 8.0f;
const float MAX_MOTION_TIME_FOR_CAST = 4.0f;
const float MAX_PROJECTILE_CAST_RANGE = 100.0;

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
		m_SpellCastData.target_id = m_SpellCastData.caster_id;
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

	for (DWORD i = 0; i < SPELLFORMULA_MAX_COMPS; i++)
	{
		DWORD comp = m_SpellCastData.spell_formula._comps[i];
		if (!comp)
			break;

		const SpellComponentBase *pSpellComponent = pSpellComponents->InqSpellComponentBase(comp);
		if (!pSpellComponent)
			return false;

		if (pSpellComponent->_category == Scarab_SpellComponentCategory && m_SpellCastData.spell->_bitfield & FastCast_SpellIndex)
			continue;

		DWORD gesture = pSpellComponent->_gesture;
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
			int error = LaunchSpellEffect();
			EndCast(error);
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

Position CSpellcastingManager::GetSpellProjectileSpawnPosition(CSpellProjectile *pProjectile, CWeenieObject *pTarget, float *pDistToTarget)
{
	bool bArc = pProjectile->InqBoolQuality(GRAVITY_STATUS_BOOL, FALSE) ? true : false;

	CWeenieObject *pSource = GetCastSource();
	Position spawnPosition = pSource->m_Position.add_offset(Vector(0, 0, pSource->GetHeight() * (bArc ? 1.0 : (2.0 / 3.0))));

	Vector targetOffset;

	if (pTarget == pSource)
	{
		targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(pTarget->m_Position.localtoglobalvec(Vector(0, 2000, pSource->GetHeight() * (2.0 / 3.0)))));
	}
	else
	{
		targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (bArc ? (5.0 / 6.0) : (2.0 / 3.0)))));
		//targetOffset = spawnPosition.get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (bArc ? (5.0 / 6.0) : (0.5)))));
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
		float minSpawnDist = (pSource->GetRadius() + pProjectile->GetRadius()) + 0.1f;

		spawnPosition.frame.m_origin += targetDir * minSpawnDist;
		spawnPosition.frame.set_vector_heading(targetDir);

		*pDistToTarget = targetOffset.magnitude();
	}

	return spawnPosition;
}

Vector CSpellcastingManager::GetSpellProjectileSpawnVelocity(Position *pSpawnPosition, CWeenieObject *pTarget, float speed, bool tracked, bool gravity, Vector *pTargetDir)
{
	Vector targetOffset;
	double targetDist;

	CWeenieObject *pSource = GetCastSource();
	if (pTarget == pSource)
	{
		targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(pTarget->m_Position.localtoglobalvec(Vector(0, 2000, pTarget->GetHeight() * (2.0 / 3.0)))));
		tracked = false;
	}
	else
		targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * (2.0 / 3.0))));
		//targetOffset = pSpawnPosition->get_offset(pTarget->m_Position.add_offset(Vector(0, 0, pTarget->GetHeight() * 0.5f)));

	targetDist = targetOffset.magnitude();

	if (!tracked)
	{
		double t = targetDist / speed;
		Vector v = targetOffset / t;

		if (gravity)
			v.z += (9.8*t) / 2.0f;
		
		Vector targetDir = v;
		targetDir.normalize();

		if (pTargetDir)
			*pTargetDir = targetDir;

		return v;
	}

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
		return GetSpellProjectileSpawnVelocity(pSpawnPosition, pTarget, speed, false, gravity, pTargetDir);
	}

	Vector v;
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

	return v;
}

const double MIN_BOLT_VELOCITY = 5.0;
const double MAX_BOLT_VELOCITY = 20.0;
double CalculateBoltVelocityByDistance(double distance)
{
	return MIN_BOLT_VELOCITY + ((MAX_BOLT_VELOCITY-MIN_BOLT_VELOCITY) * min(1.0, max(0.0, (distance / 15.0))));
}

int CSpellcastingManager::LaunchRingProjectiles(DWORD wcid)
{
	//DWORD damage = 0;

	//switch (m_SpellCastData.power_level_of_power_component)
	//{
	//case 1: damage = Random::GenUInt(16, 31); break;
	//case 2: damage = Random::GenUInt(18, 35); break;
	//case 3: damage = Random::GenUInt(21, 42); break;
	//case 4: damage = Random::GenUInt(25, 50); break;
	//case 5: damage = Random::GenUInt(29, 59); break;
	//case 6: damage = Random::GenUInt(36, 71); break;
	//case 7:
	//case 8: damage = Random::GenUInt(42, 84); break;
	//case 9:
	//case 10: damage = Random::GenUInt(47, 94); break;
	//}

	float velocity = 1.0f;
	bool tracking = true;
	bool gravity = false;

	const int NUM_RING_PROJECTILES = 4;

	for (int i = 0; i < NUM_RING_PROJECTILES; i++)
	{
		CSpellProjectile *pProjectile = new CSpellProjectile(m_SpellCastData, m_SpellCastData.target_id);//, damage);

		// spawn default object properties
		g_pWeenieFactory->ApplyWeenieDefaults(pProjectile, wcid);

		// set spell id (is this even needed?)
		pProjectile->SetSpellID(m_SpellCastData.spell_id);

		double rads = ((i * 2) / (double)NUM_RING_PROJECTILES) * PI;
		Vector dir(sin(rads), cos(rads), 0.0f);

		float velocity = 2.0f;

		// create the initial object
		Position projSpawnPos = GetCastSource()->m_Position;
		projSpawnPos.frame.set_heading(rads);

		pProjectile->SetInitialPosition(projSpawnPos);		
		pProjectile->InitPhysicsObj();

		pProjectile->m_Position = pProjectile->m_Position.add_offset(Vector(
			(GetCastSource()->GetRadius() + pProjectile->GetRadius() + F_EPSILON) * dir.x,
			(GetCastSource()->GetRadius() + pProjectile->GetRadius() + F_EPSILON) * dir.y, /*1.223465f*/0.8f));

		/*
		m_pWeenie->SendText(
			csprintf("DEBUG: WeenieHeight/Radius: %f %f ProjHeight/Radius: %f %f",
				m_pWeenie->GetHeight(), m_pWeenie->GetRadius(), pProjectile->GetHeight(), pProjectile->GetRadius()), LTT_ALL_CHANNELS);
				*/

		// set velocity
		pProjectile->set_velocity(dir * velocity, 0);

		// insert the object into the world
		g_pWorld->CreateEntity(pProjectile);

		// launch particle effect
		pProjectile->EmitEffect(PS_Launch, 0.4f);
	}

	return WERROR_NONE;
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

	for (int x = 0; x < numX; x++)
	{
		for (int y = 0; y < numY; y++)
		{
			for (int z = 0; z < numZ; z++)
			{
				//DWORD damage = Random::GenUInt(meta->_baseIntensity, meta->_baseIntensity + meta->_variance);

				CSpellProjectile *pProjectile = new CSpellProjectile( // revise me
					m_SpellCastData, pTarget == pSource ? 0 : m_SpellCastData.target_id);//, damage);

				// create the initial object
				float distToTarget;
				
				// spawn default object properties
				g_pWeenieFactory->ApplyWeenieDefaults(pProjectile, meta->_wcid);
				
				pProjectile->m_Qualities.SetInt(DAMAGE_TYPE_INT, meta->_etype);

				// set spell id (is this even needed?)
				pProjectile->SetSpellID(m_SpellCastData.spell_id);
				pProjectile->SetInitialPosition(GetCastSource()->m_Position);
				pProjectile->InitPhysicsObj();

				pProjectile->m_PhysicsState |= MISSILE_PS|REPORT_COLLISIONS_PS;
				pProjectile->m_PhysicsState &= ~IGNORE_COLLISIONS_PS;
				
				Position projSpawnPos = GetSpellProjectileSpawnPosition(pProjectile, pTarget, &distToTarget);

				Vector createOffset = projSpawnPos.localtoglobalvec(meta->_createOffset);

				projSpawnPos = projSpawnPos.add_offset(
					Vector(
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.x * meta->_padding.x,
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.y * meta->_padding.y,
						Random::GenFloat(-1.0, 1.0) * meta->_peturbation.z * meta->_padding.z));

				projSpawnPos = projSpawnPos.add_offset(createOffset);

				Position spawnSourcePosition = projSpawnPos;

				float radius = pProjectile->GetRadius();

				Vector sizePerProjectile = meta->_padding; // * radius;
				
				Vector projectileGroupOffset(x, y, z);
				projectileGroupOffset.x *= sizePerProjectile.x;
				projectileGroupOffset.y *= sizePerProjectile.y;
				projectileGroupOffset.z *= sizePerProjectile.z;

				projectileGroupOffset.x -= sizePerProjectile.x * ((meta->_dims.x - 1.0) / 2.0);
				projectileGroupOffset.y -= sizePerProjectile.y * ((meta->_dims.y - 1.0) / 2.0);
				projectileGroupOffset.z -= sizePerProjectile.z * ((meta->_dims.z - 1.0) / 2.0);

				projectileGroupOffset = projSpawnPos.localtoglobalvec(projectileGroupOffset);
				projSpawnPos = projSpawnPos.add_offset(projectileGroupOffset);

				// set spell id (is this even needed?)
				pProjectile->m_Position = projSpawnPos;

				double maxVelocity = 5.0;

				pProjectile->m_Qualities.InqFloat(MAXIMUM_VELOCITY_FLOAT, maxVelocity);

				bool bGravity = false;
				bool bTracking = !meta->_bNonTracking;

				if (pProjectile->InqBoolQuality(GRAVITY_STATUS_BOOL, FALSE))
				{
					pProjectile->m_PhysicsState |= GRAVITY_PS;
					bGravity = true;
				}

				if (pProjectile->InqBoolQuality(INELASTIC_BOOL, FALSE))
					pProjectile->m_PhysicsState |= INELASTIC_PS;
				if (pProjectile->InqBoolQuality(SCRIPTED_COLLISION_BOOL, FALSE))
					pProjectile->m_PhysicsState |= SCRIPTED_COLLISION_PS;
				if (pProjectile->InqBoolQuality(LIGHTS_STATUS_BOOL, FALSE))
					pProjectile->m_PhysicsState |= LIGHTING_ON_PS;

				// pProjectile->m_PhysicsState |= ETHEREAL_PS;

				// set velocity
				Vector spawnVelocity = GetSpellProjectileSpawnVelocity(&spawnSourcePosition, pTarget, maxVelocity, bTracking, bGravity, NULL);

				if (meta->_spreadAngle > 0)
				{
					if (numX > 1)
					{
						double xRatio = x / (double)(numX - 1);
						double theta = DEG2RAD((xRatio - 0.5) * meta->_spreadAngle);
						double cs = cos(theta);
						double sn = sin(theta);
						double px = spawnVelocity.x * cs - spawnVelocity.y * sn;
						double py = spawnVelocity.x * sn + spawnVelocity.y * cs;

						spawnVelocity.x = px;
						spawnVelocity.y = py;
					}
				}

				pProjectile->set_velocity(spawnVelocity, 0);

				// insert the object into the world
				if (g_pWorld->CreateEntity(pProjectile))
				{
					// launch particle effect
					pProjectile->EmitEffect(4, 0.8f);

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
		if (CWeenieObject *pTarget = GetCastTarget())
			pTarget->EmitEffect(m_SpellCastData.spell->_target_effect, max(0.0, min(1.0, (m_SpellCastData.power_level_of_power_component - 1.0) / 7.0)));
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
	else
	{
		m_pWeenie->Movement_Teleport(targetPos, false);
	}
}

int CSpellcastingManager::LaunchSpellEffect()
{
	if (int targetError = CheckTargetValidity() && m_SpellCastData.range_check)
	{
		switch (targetError)
		{
		case WERROR_MAGIC_TARGET_OUT_OF_RANGE:
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

	if (m_pWeenie->AsPlayer() && m_SpellCastData.uses_mana)
	{
		bool fizzled = false;
		double chance = GetMagicSkillChance(m_SpellCastData.current_skill, m_SpellCastData.spell->_power);
		if (chance < Random::RollDice(0.0, 1.0))
		{
			// fizzle
			m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
			m_pWeenie->AdjustMana(-5);

			fizzled = true;
		}
		else if (m_pWeenie->m_Position.distance(m_SpellCastData.initial_cast_position) >= 6.0)
		{
			// fizzle
			m_pWeenie->EmitEffect(PS_Fizzle, 0.542734265f);
			m_pWeenie->AdjustMana(-5);
			m_pWeenie->SendText("Your movement disrupted spell casting!", LTT_MAGIC);

			fizzled = true;
		}

		if (!m_UsedComponents.empty()) //we used components, so check if any need burning
		{
			//Each spell and each component type has a burn rate, where the lower level spells typically burn less.
			//This rate is increased when fizzling. Your magic skill is not a factor except indirectly through fizzling.
				
			SpellComponentTable *pSpellComponents = MagicSystem::GetSpellComponentTable();
			float spellComponentLossMod = m_SpellCastData.spell->_component_loss;
			if (fizzled)
				spellComponentLossMod *= 1.5; //made up value: 50% extra chance of burning components on fizzles.

			if (pSpellComponents)
			{
				std::string componentsConsumedString = "";
				for (std::map<DWORD, DWORD>::iterator iter = m_UsedComponents.begin(); iter != m_UsedComponents.end(); ++iter)
				{
					CWeenieObject *component = g_pWorld->FindObject(iter->first);
					if(!component) // where did it go? force fizzle.
						return WERROR_MAGIC_FIZZLE;

					int compId = component->InqDIDQuality(SPELL_COMPONENT_DID, 0);
					const SpellComponentBase *componentBase = pSpellComponents->InqSpellComponentBase(compId);
					float burnChance = componentBase->_CDM * spellComponentLossMod;
					for (int i = 0; i < iter->second; i++) // one chance to burn for every instance of the component in the spell formula
					{
						if (Random::RollDice(0.0, 1.0) < burnChance)
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

		if(fizzled)
			return WERROR_MAGIC_FIZZLE;

		m_pWeenie->AdjustMana(-GenerateManaCost());
	}

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
		case SpellType::Dispel_SpellType:
			{
				DispelSpellEx *meta = (DispelSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

				int minNum = (int) ((meta->_number * meta->_number_variance) + F_EPSILON);
				int maxNum = (int) ((meta->_number * (1.0 / meta->_number_variance)) + F_EPSILON);

				int numToDispel = Random::GenInt(minNum, maxNum);

				CWeenieObject *pTarget = GetCastTarget();
				if (pTarget)
				{
					if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
					{
						if (pTarget->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
						{
							break;
						}
					}

					PackableListWithJson<DWORD> possibleToDispel;

					if (pTarget->m_Qualities._enchantment_reg)
					{
						if (pTarget->m_Qualities._enchantment_reg->_add_list)
						{
							for (auto &entry : *pTarget->m_Qualities._enchantment_reg->_add_list)
							{
								if (entry._power_level > meta->_max_power)
									continue;
								if (entry._power_level < meta->_min_power)
									continue;
								if (entry._duration <= 0.0)
									continue;
								if ((entry._id & 0xFFFF) == 666) // vitae
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

						if (pTarget->m_Qualities._enchantment_reg->_mult_list)
						{
							for (auto &entry : *pTarget->m_Qualities._enchantment_reg->_mult_list)
							{
								if (entry._power_level > meta->_max_power)
									continue;
								if (entry._power_level < meta->_min_power)
									continue;
								if (entry._duration <= 0.0)
									continue;
								if ((entry._id & 0xFFFF) == 666) // vitae
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
					
					PackableListWithJson<DWORD> listToDispel;

					if (meta->_number < 0)
					{
						// dispel all
						listToDispel = possibleToDispel;
					}
					else
					{
						while (numToDispel > 0 && !possibleToDispel.empty())
						{
							std::list<DWORD>::iterator randomEntry = possibleToDispel.begin();
							std::advance(randomEntry, Random::GenUInt(0, (DWORD)(possibleToDispel.size() - 1)));

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
						if (pTarget == m_pWeenie)
						{
							m_pWeenie->SendText(csprintf("You cast %s on yourself and dispel: %s", m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
						}
						else
						{
							m_pWeenie->SendText(csprintf("You cast %s on %s and dispel: %s", m_SpellCastData.spell->_name.c_str(), pTarget->GetName().c_str(), spellNames.c_str()), LTT_MAGIC);
							pTarget->SendText(csprintf("%s casts %s on you and dispels: %s", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), spellNames.c_str()), LTT_MAGIC);
						}

						if (pTarget->m_Qualities._enchantment_reg)
						{
							pTarget->m_Qualities._enchantment_reg->RemoveEnchantments(&listToDispel);

							BinaryWriter expireMessage;
							expireMessage.Write<DWORD>(0x2C8);
							listToDispel.Pack(&expireMessage);
							pTarget->SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
						}
					}
					else
					{
						if (pTarget == m_pWeenie)
						{
							m_pWeenie->SendText(csprintf("You cast %s on yourself, but the dispel fails.", m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
						}
						else
						{
							m_pWeenie->SendText(csprintf("You cast %s on %s, but the dispel fails.", m_SpellCastData.spell->_name.c_str(), pTarget->GetName().c_str()), LTT_MAGIC);
							pTarget->SendText(csprintf("%s casts %s on you, but the dispel fails.", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
						}
					}

					bSpellPerformed = true;
				}

				break;
			}
		case SpellType::PortalLink_SpellType:
			{
				if (m_pWeenie->HasOwner())
					break;

				PortalLinkSpellEx *meta = (PortalLinkSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

				CWeenieObject *pTarget = GetCastTarget();
				if (pTarget)
				{
					// portal bitmask, 0x10 = cannot be summoned
					// portal bitmask, 0x20 = cannot be linked/recalled

					int minLevel = pTarget->InqIntQuality(MIN_LEVEL_INT, 0);
					int maxLevel = pTarget->InqIntQuality(MAX_LEVEL_INT, 0);

					int currentLevel = m_pWeenie->InqIntQuality(LEVEL_INT, 1);

					if (minLevel && currentLevel < minLevel)
					{
						m_pWeenie->SendText("You are not powerful enough to tie to this portal yet.", LTT_MAGIC);
						break;
					}
					else if (maxLevel && currentLevel > maxLevel)
					{
						m_pWeenie->SendText("You are too powerful to tie to this portal.", LTT_MAGIC);
						break;
					}

					switch (meta->_index)
					{
					case 1:
						if (pTarget->m_Qualities.m_WeenieType == LifeStone_WeenieType)
						{
							m_pWeenie->m_Qualities.SetPosition(LINKED_LIFESTONE_POSITION, m_pWeenie->m_Position);
							bSpellPerformed = true;
						}
						else
						{
							m_pWeenie->SendText("You cannot link that.", LTT_MAGIC);
						}
						break;

					case 2: // primary portal tie
						if (pTarget->m_Qualities.m_WeenieType == Portal_WeenieType &&
							!(pTarget->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x20))
						{
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_ONE_DID, pTarget->m_Qualities.id);
							bSpellPerformed = true;
						}
						else
						{
							m_pWeenie->SendText("You cannot link that portal.", LTT_MAGIC);
						}

						break;

					case 3: // secondary portal tie
						if (pTarget->m_Qualities.m_WeenieType == Portal_WeenieType &&
							!(pTarget->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x20))
						{
							m_pWeenie->m_Qualities.SetDataID(LINKED_PORTAL_TWO_DID, pTarget->m_Qualities.id);
							bSpellPerformed = true;
						}
						else
						{
							m_pWeenie->SendText("You cannot link that portal.", LTT_MAGIC);
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
				if (m_pWeenie->HasOwner())
					break;

				PortalRecallSpellEx *meta = (PortalRecallSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

				switch (meta->_index)
				{
				case 2: // lifestone recall
					{
						Position lifestone;
						if (m_pWeenie->m_Qualities.InqPosition(LINKED_LIFESTONE_POSITION, lifestone) && lifestone.objcell_id != 0)
						{
							BeginPortalSend(lifestone);
						}
						else
						{
							m_pWeenie->SendText("You have no lifestone tied.", LTT_MAGIC);
						}

						bSpellPerformed = true;
						break;
					}

				case 3: // portal recall
					{
						Position lastPortalPos;
						if (m_pWeenie->m_Qualities.InqPosition(LAST_PORTAL_POSITION, lastPortalPos) && lastPortalPos.objcell_id != 0)
						{
							BeginPortalSend(lastPortalPos);
						}
						else
						{
							m_pWeenie->SendText("You have not used a recallable portal yet.", LTT_MAGIC);
						}

						bSpellPerformed = true;
						break;
					}

				case 4: // primary portal recall
					{
						DWORD portalDID = 0;
						if (m_pWeenie->m_Qualities.InqDataID(LINKED_PORTAL_ONE_DID, portalDID) && portalDID != 0)
						{
							CWeenieDefaults *portalDefaults = NULL;

							if (portalDID)
							{
								portalDefaults = g_pWeenieFactory->GetWeenieDefaults(portalDID);
							}

							int minLevel = 0;
							int maxLevel = 0;
							portalDefaults->m_Qualities.InqInt(MIN_LEVEL_INT, minLevel);
							portalDefaults->m_Qualities.InqInt(MAX_LEVEL_INT, maxLevel);

							int currentLevel = m_pWeenie->InqIntQuality(LEVEL_INT, 1);

							if (minLevel && currentLevel < minLevel)
							{
								m_pWeenie->SendText("You are not powerful enough to recall this portal yet.", LTT_MAGIC);
								break;
							}
							else if (maxLevel && currentLevel > maxLevel)
							{
								m_pWeenie->SendText("You are too powerful to recall this portal.", LTT_MAGIC);
								break;
							}

							std::string restriction;
							if (portalDefaults->m_Qualities.InqString(QUEST_RESTRICTION_STRING, restriction))
							{
								if (CPlayerWeenie *player = m_pWeenie->AsPlayer())
								{
									if (!player->InqQuest(restriction.c_str()))
									{
										m_pWeenie->SendText("You try to recall the portal but there is no effect.", LTT_MAGIC);
										break;
									}
								}
							}

							Position portalDest;
							if (portalDefaults->m_Qualities.InqPosition(DESTINATION_POSITION, portalDest) && portalDest.objcell_id != 0)
							{
								BeginPortalSend(portalDest);
							}
							else
							{
								m_pWeenie->SendText("The primary portal you have tied has no destination set.", LTT_MAGIC);
							}
						}
						else
						{
							m_pWeenie->SendText("You have no primary portal tied.", LTT_MAGIC);
						}

						bSpellPerformed = true;
						break;
					}

					break;

				case 5: // secondary portal recall
					{
						DWORD portalDID = 0;
						if (m_pWeenie->m_Qualities.InqDataID(LINKED_PORTAL_TWO_DID, portalDID) && portalDID != 0)
						{
							CWeenieDefaults *portalDefaults = NULL;

							if (portalDID)
							{
								portalDefaults = g_pWeenieFactory->GetWeenieDefaults(portalDID);
							}

							int minLevel = 0;
							int maxLevel = 0;
							portalDefaults->m_Qualities.InqInt(MIN_LEVEL_INT, minLevel);
							portalDefaults->m_Qualities.InqInt(MAX_LEVEL_INT, maxLevel);

							int currentLevel = m_pWeenie->InqIntQuality(LEVEL_INT, 1);

							if (minLevel && currentLevel < minLevel)
							{
								m_pWeenie->SendText("You are not powerful enough to recall this portal yet.", LTT_MAGIC);
								break;
							}
							else if (maxLevel && currentLevel > maxLevel)
							{
								m_pWeenie->SendText("You are too powerful to recall this portal.", LTT_MAGIC);
								break;
							}

							std::string restriction;
							if (portalDefaults->m_Qualities.InqString(QUEST_RESTRICTION_STRING, restriction))
							{
								if (CPlayerWeenie *player = m_pWeenie->AsPlayer())
								{
									if (!player->InqQuest(restriction.c_str()))
									{
										m_pWeenie->SendText("You try to recall the portal but there is no effect.", LTT_MAGIC);
										break;
									}
								}
							}

							Position portalDest;
							if (portalDefaults->m_Qualities.InqPosition(DESTINATION_POSITION, portalDest) && portalDest.objcell_id != 0)
							{
								BeginPortalSend(portalDest);
							}
							else
							{
								m_pWeenie->SendText("The secondary portal you have tied has no destination set.", LTT_MAGIC);
							}
						}
						else
						{
							m_pWeenie->SendText("You have no secondary portal tied.", LTT_MAGIC);
						}

						bSpellPerformed = true;
						break;
					}

					break;
				}

				if (m_pWeenie->IsAdmin())
				{
					// lifestone recall = 2
					// portal recall = 3
					// primary portal recall = 4
					// secondary portal recall = 5
					// m_pWeenie->SendText(csprintf("Index: %d", meta->_index), LTT_DEFAULT);
				}

				bSpellPerformed = true;
				break;
			}
		case SpellType::Enchantment_SpellType:
			{
				EnchantmentSpellEx *meta = (EnchantmentSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;

				Enchantment enchant;
				enchant._id = meta->_spell_id | ((DWORD)m_SpellCastData.serial << (DWORD)16);
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

				std::list<CWeenieObject *> targets;
				
				if (CWeenieObject *castTarget = GetCastTarget())
				{
					if (m_SpellCastData.spell->InqTargetType() != ITEM_TYPE::TYPE_ITEM_ENCHANTABLE_TARGET)
					{
						targets.push_back(castTarget);
					}
					else
					{
						CContainerWeenie *container = castTarget->AsContainer();
						if (container && !container->HasOwner())
						{
							for (auto wielded : container->m_Wielded)
							{
								if (wielded->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
								{
									if (castTarget == m_pWeenie || wielded->parent) // for other targets, only physically wielded allowed
									{
										targets.push_back(wielded);
									}
								}
							}
						}
						else
						{
							if (castTarget->GetItemType() & m_SpellCastData.spell->_non_component_target_type)
							{
								if (castTarget == m_pWeenie || castTarget->parent || !castTarget->HasOwner() || castTarget->GetWorldTopLevelOwner() == m_pWeenie) // for other targets, only physically wielded allowed
								{
									targets.push_back(castTarget);
								}
							}
						}
					}

					for (auto target : targets)
					{
						// You cast Harlune's Blessing on yourself, refreshing Harlune's Blessing
						// You cast Impenetrability III on Pathwarden Robe, surpassing Impenetrability II

						CWeenieObject *topLevelOwner = target->GetWorldTopLevelOwner();

						if (target->InqIntQuality(MAX_STACK_SIZE_INT, 1) > 1) //do not allow enchanting stackable items(ammunition)
						{
							m_pWeenie->SendText(csprintf("The %s can't be enchanted.", target->GetName().c_str()), LTT_MAGIC);
							continue;
						}

						bool bAlreadyExisted = false;
						if (target->m_Qualities._enchantment_reg && target->m_Qualities._enchantment_reg->IsEnchanted(enchant._id))
							bAlreadyExisted = true;

						if (m_pWeenie != target)
						{
							if (!(m_SpellCastData.spell->_bitfield & Beneficial_SpellIndex))
							{
								if (target->AsPlayer() && target->GetWorldTopLevelOwner()->ImmuneToDamage(m_pWeenie))
								{
									continue;
								}
							}

							if (m_SpellCastData.spell->_bitfield & Resistable_SpellIndex)
							{
								if (topLevelOwner->TryMagicResist(m_SpellCastData.current_skill))
								{
									topLevelOwner->EmitSound(Sound_ResistSpell, 1.0f, false);
									topLevelOwner->SendText(csprintf("You resist the spell cast by %s", m_pWeenie->GetName().c_str()), LTT_MAGIC);
									m_pWeenie->SendText(csprintf("%s resists your spell", target->GetName().c_str()), LTT_MAGIC);
									topLevelOwner->OnResistSpell(m_pWeenie);
									continue;
								}
							}

							if (int resistMagic = target->InqIntQuality(RESIST_MAGIC_INT, 0, FALSE))
							{
								if (resistMagic >= 9999 || ::TryMagicResist(m_SpellCastData.current_skill, (DWORD) resistMagic))
								{
									target->EmitSound(Sound_ResistSpell, 1.0f, false);

									if (m_pWeenie != topLevelOwner)
									{
										topLevelOwner->SendText(csprintf("%s resists the spell cast by %s", m_pWeenie->GetName().c_str(), m_pWeenie->GetName().c_str()), LTT_MAGIC);
									}

									m_pWeenie->SendText(csprintf("%s resists your spell", target->GetName().c_str()), LTT_MAGIC);
									target->OnResistSpell(m_pWeenie);
									continue;
								}
							}
						}

						topLevelOwner->HandleAggro(m_pWeenie);

						target->m_Qualities.UpdateEnchantment(&enchant);
						target->NotifyEnchantmentUpdated(&enchant);

						target->CheckVitalRanges();

						if (m_pWeenie == target)
						{
							m_pWeenie->SendText(csprintf("You cast %s on yourself", m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
						}
						else
						{
							m_pWeenie->SendText(csprintf("You cast %s on %s", m_SpellCastData.spell->_name.c_str(), target->GetName().c_str()), LTT_MAGIC);

							if (m_pWeenie != topLevelOwner)
							{
								if (target == topLevelOwner)
									target->SendText(csprintf("%s cast %s on you", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str()), LTT_MAGIC);
								else
									topLevelOwner->SendText(csprintf("%s cast %s on %s", m_pWeenie->GetName().c_str(), m_SpellCastData.spell->_name.c_str(), target->GetName().c_str()), LTT_MAGIC);
							}
						}

						bSpellPerformed = true;
					}
				}

				break;
			}

		case SpellType::PortalSummon_SpellType:
			{
				PortalSummonSpellEx *meta = (PortalSummonSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
				if (!meta && (meta->_link == 1 || meta->_link == 2))
					break;

				Position spawnPos;
				if (!m_pWeenie->m_Qualities.InqPosition(PORTAL_SUMMON_LOC_POSITION, spawnPos))
				{
					spawnPos = GetCastSource()->m_Position;
					spawnPos.frame.m_origin += spawnPos.localtoglobalvec(Vector(0, 7, 0));
				}

				DWORD portalDID = 0;

				Position dummyPos;
				bool bNoLink = false;

				switch (meta->_link)
				{
				case 1:
					portalDID = m_pWeenie->InqDIDQuality(LINKED_PORTAL_ONE_DID, 0);

					if (!portalDID && !m_pWeenie->m_Qualities.InqPosition(LINKED_PORTAL_ONE_POSITION, dummyPos))
					{
						m_pWeenie->SendText("You do not have a primary portal tied.", LTT_MAGIC);
						bNoLink = true;
					}

					break;

				case 2:
					portalDID = m_pWeenie->InqDIDQuality(LINKED_PORTAL_TWO_DID, 0);

					if (!portalDID && !m_pWeenie->m_Qualities.InqPosition(LINKED_PORTAL_TWO_POSITION, dummyPos))
					{
						m_pWeenie->SendText("You do not have a secondary portal tied.", LTT_MAGIC);
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

					int minLevel = 0;
					int maxLevel = 0;
					portalDefaults->m_Qualities.InqInt(MIN_LEVEL_INT, minLevel);
					portalDefaults->m_Qualities.InqInt(MAX_LEVEL_INT, maxLevel);

					if (m_pWeenie->AsPlayer())
					{
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

						if ((portalDefaults->m_Qualities.GetInt(PORTAL_BITMASK_INT, 0) & 0x10))
						{
							m_pWeenie->SendText("That portal may not be summoned.", LTT_MAGIC);
							break;
						}
					}
					else
						canFlagForQuest = true;
				}
				
				CWeenieObject *weenie =  g_pWeenieFactory->CreateWeenieByClassID(W_PORTALGATEWAY_CLASS, &spawnPos, false);

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
						if(canFlagForQuest)
							weenie->CopyStringStat(QUEST_STRING, &portalDefaults->m_Qualities);
					}
					else
					{
						switch (meta->_link)
						{
						case 1:
							weenie->m_Qualities.SetPosition(DESTINATION_POSITION, m_pWeenie->InqPositionQuality(LINKED_PORTAL_ONE_POSITION, Position()));
							break;

						case 2:
							weenie->m_Qualities.SetPosition(DESTINATION_POSITION, m_pWeenie->InqPositionQuality(LINKED_PORTAL_TWO_POSITION, Position()));
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

				CWeenieObject *target = GetCastTarget();

				if (target)
				{
					PerformCastParticleEffects(); // perform particle effects early because teleporting will cancel it
					target->Movement_Teleport(meta->_pos, false);
					target->SendText("You have been teleported.", LTT_MAGIC);
				}

				break;
			}

		case SpellType::LifeProjectile_SpellType:
		case SpellType::Projectile_SpellType:
			{
				ProjectileSpellEx *meta = (ProjectileSpellEx *)m_SpellCastData.spellEx->_meta_spell._spell;
				bSpellPerformed = LaunchProjectileSpell(meta);
				//bSpellPerformed = LaunchRingProjectiles(meta->_wcid);
				
				break;
			}
		}
	}

	if (bSpellPerformed)
	{
		PerformCastParticleEffects();
	}

	return WERROR_NONE;
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

std::string GetAttribute2ndName(STypeAttribute2nd attribute2nd)
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

		if(source->AsPlayer()) //only players have natural resistances.
		{
			//Some combination of strength and endurance allows one to have a level of "natural resistances" to the 7 damage types.This caps out at a 50 % resistance(the equivalent to level 5 life prots) to these damage types.This resistance is not additive to life protections : higher level life protections will overwrite these natural resistances, although life vulns will take these natural resistances into account, if the player does not have a higher level life protection cast upon them.
			//For example, a player will not get a free protective bonus from natural resistances if they have both Prot 7 and Vuln 7 cast upon them.The Prot and Vuln will cancel each other out, and since the Prot has overwritten the natural resistances, there will be no resistance bonus.
			//The abilities that Endurance or Endurance/Strength conveys are not increased by Strength or Endurance buffs.It is the raw Strength and/or Endurance scores that determine the various bonuses.
			//drain resistances(same formula as natural resistances) allows one to partially resist drain health/stamina/mana and harm attacks, up to a maximum of roughly 50%. 

			//todo: natural resistances only change when base strength or endurance changes so we could potentially pre-calculate this somewhere else.
			DWORD strength = 0;
			DWORD endurance = 0;
			source->m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			source->m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + endurance);
			float resistanceNatural;
			if (strAndEnd <= 200) //formula deduced from values present in the client pdb.
				resistanceNatural = 1.0f - ((0.05 * strAndEnd) / 100.f);
			else
				resistanceNatural = 1.0f - (((0.1666667 * strAndEnd) - 23.33333) / 100.f);

			resistanceNatural = max(resistanceNatural, 0.5);

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



	DWORD sourceStartValue = 0;
	DWORD sourceMinValue = 0;
	DWORD sourceMaxValue = 0;
	source->m_Qualities.InqAttribute2nd(meta->_src, sourceStartValue, FALSE);
	source->m_Qualities.InqAttribute2nd((STypeAttribute2nd)((int)meta->_src - 1), sourceMaxValue, FALSE);

	DWORD destStartValue = 0;
	DWORD destMinValue = 0;
	DWORD destMaxValue = 0;
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
	}
	else
	{
		m_pWeenie->SendText(csprintf("You cast %s and %s %d points of your %s.",
			m_SpellCastData.spell->_name.c_str(), bRestore ? "restore" : "drain", amount, vitalName), LTT_MAGIC);
	}
}

int CSpellcastingManager::CreatureBeginCast(DWORD target_id, DWORD spell_id)
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
		LOG(Temp, Normal, "Player trying to cast unknown spell?\n");
		return WERROR_MAGIC_GENERAL_FAILURE;
	}

	m_bCasting = true;
	m_PendingMotions.clear();
	m_bTurningToObject = false;
	
	m_PendingMotions.push_back(SpellCastingMotion(Motion_CastSpell, 2.0f, true, true, 2.0f));
	m_SpellCastData.power_level_of_power_component = m_SpellCastData.spell_formula.GetPowerLevelOfPowerComponent();

	BeginNextMotion();

	return WERROR_NONE;
}


int CSpellcastingManager::CastSpellInstant(DWORD target_id, DWORD spell_id)
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

	if (ResolveSpellBeingCasted())
	{
		LaunchSpellEffect();		
	}
	else
	{
		error = WERROR_MAGIC_GENERAL_FAILURE;
	}

	m_SpellCastData = oldData;
	m_bCasting = bOldCasting;

	return error;
}


int CSpellcastingManager::CastSpellEquipped(DWORD target_id, DWORD spell_id, WORD serial)
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

	if (ResolveSpellBeingCasted())
	{
		LaunchSpellEffect();
	}
	else
	{
		error = WERROR_MAGIC_GENERAL_FAILURE;
	}

	m_SpellCastData = oldData;
	m_bCasting = bOldCasting;

	return error;
}

DWORD CSpellcastingManager::DetermineSkillLevelForSpell()
{
	int spellcraft = 0;
	if (m_pWeenie->m_Qualities.InqInt(ITEM_SPELLCRAFT_INT, spellcraft, FALSE, FALSE))
		return (DWORD)spellcraft;

	if (!m_SpellCastData.spell)
		return 0;

	DWORD skillLevel = 0;
	STypeSkill skill = m_SpellCastData.spell->InqSkillForSpell();

	if (skill)
	{
		m_pWeenie->InqSkill(skill, skillLevel, FALSE);
	}
	else
	{
		DWORD creatureEnch = 0, itemEnch = 0, life = 0, war = 0, voidMagic = 0;
		m_pWeenie->InqSkill(CREATURE_ENCHANTMENT_SKILL, creatureEnch, FALSE);
		m_pWeenie->InqSkill(ITEM_ENCHANTMENT_SKILL, itemEnch, FALSE);
		m_pWeenie->InqSkill(LIFE_MAGIC_SKILL, life, FALSE);
		m_pWeenie->InqSkill(WAR_MAGIC_SKILL, war, FALSE);
		m_pWeenie->InqSkill(VOID_MAGIC_SKILL, voidMagic, FALSE);

		DWORD highestSkill = 0;

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

	double range = m_SpellCastData.spell->_base_range_constant + m_SpellCastData.spell->_base_range_mod * m_SpellCastData.current_skill;

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

	CWeenieObject *pTarget = g_pWorld->FindWithinPVS(pCastSource, m_SpellCastData.target_id);

	if (!(m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex))
	{
		if (!pTarget)
			return WERROR_OBJECT_GONE;

		if (!pTarget->HasOwner() || !pCastSource->FindContainedItem(pTarget->GetID()))
		{
			if (pCastSource && m_SpellCastData.range_check)
			{
				if (pCastSource->DistanceTo(pTarget, true) > m_SpellCastData.max_range)
					return WERROR_MAGIC_TARGET_OUT_OF_RANGE;
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
			if (m_SpellCastData.equipped && (m_SpellCastData.spell->_school == 3) && (m_SpellCastData.spell->_bitfield & SelfTargeted_SpellIndex))
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

	return WERROR_NONE;
}

int CSpellcastingManager::GenerateManaCost()
{
	DWORD manaConvSkill = m_pWeenie->GetEffectiveManaConversionSkill();

	return GetManaCost(m_SpellCastData.current_skill, m_SpellCastData.spell->_power, m_SpellCastData.spell->_base_mana, manaConvSkill);
}

int CSpellcastingManager::TryBeginCast(DWORD target_id, DWORD spell_id)
{
	if (m_bCasting)
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Actions Locked, Casting = true, Turning = %s", m_bTurningToObject ? "true" : "false"), LTT_ALL_CHANNELS);
		return WERROR_ACTIONS_LOCKED;
	}

	if (m_pWeenie->get_minterp()->interpreted_state.actions.size())
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Actions Locked, Interp state has %d actions.", m_pWeenie->get_minterp()->interpreted_state.actions.size()), LTT_ALL_CHANNELS);
		return WERROR_ACTIONS_LOCKED;
	}

	if (m_pWeenie->get_minterp()->interpreted_state.forward_command != 0x41000003)
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Unprepared, Interp state forward command is 0x%08X, not idle.", m_pWeenie->get_minterp()->interpreted_state.forward_command), LTT_ALL_CHANNELS);
		return WERROR_MAGIC_UNPREPARED;
	}

	if (!m_pWeenie->m_Qualities.IsSpellKnown(spell_id))
	{
		// m_pWeenie->SendText(csprintf("DEBUG: Unlearned spell %d.", spell_id), LTT_ALL_CHANNELS);
		return WERROR_MAGIC_UNLEARNED_SPELL;
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
		LOG(Temp, Normal, "Player trying to cast unknown spell?\n"); 
		return WERROR_MAGIC_GENERAL_FAILURE;
	}

	if (!m_SpellCastData.current_skill)
	{
		m_pWeenie->SendText("You are not trained in that skill!", LTT_MAGIC);
		return WERROR_MAGIC_INVALID_SPELL_TYPE;
	}

	// m_pWeenie->SendText(csprintf("Casting %d", m_SpellCastData.spell_id), LTT_DEFAULT);

	if (int targetError = CheckTargetValidity())
		return targetError;

	//Components
	CContainerWeenie *caster = m_pWeenie->AsContainer();
	if (caster != NULL && caster->InqBoolQuality(SPELL_COMPONENTS_REQUIRED_BOOL, FALSE) == TRUE)
	{
		CWeenieObject *foci = NULL;

		if (g_pConfig->IsSpellFociEnabled())
		{
			switch (m_SpellCastData.spell->InqSkillForSpell())
			{
			case CREATURE_ENCHANTMENT_SKILL: foci = FindFociInContainer(caster, W_PACKCREATUREESSENCE_CLASS); break;
			case ITEM_ENCHANTMENT_SKILL: foci = FindFociInContainer(caster, W_PACKITEMESSENCE_CLASS); break;
			case LIFE_MAGIC_SKILL: foci = FindFociInContainer(caster, W_PACKLIFEESSENCE_CLASS); break;
			case WAR_MAGIC_SKILL: foci = FindFociInContainer(caster, W_PACKWARESSENCE_CLASS); break;
			}
		}
		
		SpellFormula randomizedComponents;
		randomizedComponents.CopyFrom(m_SpellCastData.spell->InqSpellFormula());

		CPlayerWeenie *player = m_pWeenie->AsPlayer();
		if (player)
			randomizedComponents.RandomizeForName(player->GetClient()->GetAccount(), m_SpellCastData.spell->_formula_version);

		std::map<DWORD, DWORD> componentAmounts;
		for each (DWORD componentId in randomizedComponents._comps)
		{
			if (componentId == 0)
				continue;

			if (foci != NULL)
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
		if (foci != NULL)
			componentAmounts[188]++; //if using foci, add one more prismatic taper.

		m_UsedComponents.clear(); // clear the list of left overs from previous interrupted spell attempts.
		for (std::map<DWORD, DWORD>::iterator iter = componentAmounts.begin(); iter != componentAmounts.end(); ++iter)
		{
			DWORD compId = iter->first;
			DWORD amount = iter->second;

			std::map<DWORD, DWORD> components = FindComponentInContainer(caster, compId, amount);

			if (!components.empty())
			{
				for (std::map<DWORD, DWORD>::iterator iter = components.begin(); iter != components.end(); ++iter)
					m_UsedComponents[iter->first] += iter->second;
			}
			else
				return WERROR_MAGIC_MISSING_COMPONENTS;
		}
	}

	if (m_pWeenie->GetMana() < GenerateManaCost())
		return WERROR_MAGIC_INSUFFICIENT_MANA;

	BeginCast();
	return WERROR_NONE;
}

std::map<DWORD, DWORD> CSpellcastingManager::FindComponentInContainer(CContainerWeenie *container, unsigned int componentId, int amountNeeded)
{
	std::map<DWORD, DWORD> foundItems;
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

CWeenieObject *CSpellcastingManager::FindFociInContainer(CContainerWeenie *container, DWORD fociWcid)
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
				if (!m_pWeenie->get_minterp()->interpreted_state.turn_command)
				{
					MovementParameters params;
					params.speed = 1.0f;
					params.action_stamp = ++m_pWeenie->m_wAnimSequence;
					m_pWeenie->last_move_was_autonomous = false;
					m_pWeenie->TurnToObject(m_SpellCastData.target_id, &params);
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

void CSpellcastingManager::OnDeath(DWORD killer_id)
{
	Cancel();
}

void CSpellcastingManager::HandleMotionDone(DWORD motion, BOOL success)
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
