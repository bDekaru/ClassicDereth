#include <StdAfx.h>
#include "WeenieObject.h"
#include "PhysicsObj.h"
#include "ObjectMsgs.h"
#include "World.h"
#include "Lifestone.h"
#include "UseManager.h"
#include "SpellcastingManager.h"
#include "AttackManager.h"
#include "WeenieFactory.h"

#include "BinaryWriter.h"
#include "ChatMsgs.h"
#include "HouseManager.h"
#include "Player.h"
#include "Client.h"
#include "ClientCommands.h"
#include "MonsterAI.h"
#include "AllegianceManager.h"
#include "SpellcastingManager.h"
#include "GameEventManager.h"
#include "EmoteManager.h"
#include "WorldLandBlock.h"
#include "InferredPortalData.h"
#include "MovementManager.h"
#include "Movement.h"
#include "WClassID.h"
#include "InferredPortalData.h"
#include "Config.h"
#include "House.h"
#include "Ammunition.h"



CWeenieObject::CWeenieObject()
{
	SetID(0);

	SetName("Weenie");

	m_Qualities.SetBool(ATTACKABLE_BOOL, TRUE);
	m_Qualities.SetBool(STUCK_BOOL, FALSE);

	SetSetupID(0x0200026B);

	m_Qualities.id = 0x0001;
	SetIcon(0x06001036);
	SetItemType(TYPE_GEM);

	weenie_obj = this;

	_last_update_pos = Timer::cur_time;

	InitPhysicsTemporary();
}

CWeenieObject::~CWeenieObject()
{
	CleanupPhysicsTemporary();

	SafeDelete(m_UseManager);
	SafeDelete(m_SpellcastingManager);
	SafeDelete(m_EmoteManager);
	SafeDelete(m_AttackManager);

	if (_combatTable)
	{
		CombatManeuverTable::Release(_combatTable);
		_combatTable = NULL;
	}
}

void CWeenieObject::InitPhysicsObj()
{
	if (_phys_obj)
	{
		// already initialized
		return;
	}

	uint32_t setupID = 0;

	m_Qualities.InqDataID(SETUP_DID, setupID);

#if 0
	_phys_obj = CPhysicsObj::makeObject(setupID, GetID(), TRUE);

	if (!_phys_obj)
		return;
#else
	_phys_obj = this;

	int physicsState = 0;
	if (m_Qualities.InqInt(PHYSICS_STATE_INT, physicsState))
	{
		m_PhysicsState = physicsState;
	}

	if (!InitObjectBegin(GetID(), TRUE))
	{
		SERVER_ERROR << "Failed creating physics object!";
	}



	BOOL bCreateAtAll = FALSE;
	BOOL bCreateParts = FALSE;

#if PHATSDK_IS_SERVER
	uint32_t dataType = setupID & 0xFF000000;

	if (dataType == 0x01000000)
	{
		CGfxObj *object = CGfxObj::Get(setupID);

		if (object)
		{
			bCreateAtAll = TRUE;

			if (object->physics_bsp)
			{
				bCreateParts = TRUE;
			}

			CGfxObj::Release(object);
		}
	}
	else if (dataType == 0x02000000)
	{
		CSetup *object = CSetup::Get(setupID);

		if (object)
		{
			bCreateAtAll = TRUE;

			if (object->has_physics_bsp)
			{
				bCreateParts = TRUE;
			}

			CSetup::Release(object);
		}
	}
#else
	bCreateParts = TRUE;
#endif

	if (!InitPartArrayObject(setupID, bCreateParts))
	{
		SERVER_ERROR << "Failed creating parts array for physics object with setup" << setupID;
	}

	InitObjectEnd(); //  SetPlacementFrameInternal(0x65);
#endif

	uint32_t motionTableDID = 0;


	if (m_Qualities.InqDataID(MOTION_TABLE_DID, motionTableDID) && motionTableDID)
	{
		_phys_obj->SetMotionTableID(motionTableDID);
	}

	/*
	int physicsState = 0;
	if (m_Qualities.InqInt(PHYSICS_STATE_INT, physicsState))
	{
	_phys_obj->set_state(physicsState, FALSE);
	CacheHasPhysicsBSP();
	}
	*/

	CacheHasPhysicsBSP();

	Position initialPosition;
	if (AsPlayer() && m_Qualities.InqPosition(LOCATION_POSITION, initialPosition) && initialPosition.objcell_id)
	{
		m_Position = initialPosition;
	}
	else if (m_Qualities.InqPosition(INSTANTIATION_POSITION, initialPosition))
	{
		m_Position = initialPosition;
	}

	double defaultScale = 1.0;
	if (m_Qualities.InqFloat(DEFAULT_SCALE_FLOAT, defaultScale))
	{
		_phys_obj->SetScaleStatic(defaultScale);
	}

	double fTranslucency = 1.0;
	if (m_Qualities.InqFloat(TRANSLUCENCY_FLOAT, fTranslucency))
	{
		_phys_obj->SetTranslucencyInternal(fTranslucency);
	}

	double fFriction = 0.95f;
	if (m_Qualities.InqFloat(FRICTION_FLOAT, fFriction))
	{
		_phys_obj->m_fFriction = fFriction;
	}

	double fElasticity = 0.05f;
	if (m_Qualities.InqFloat(ELASTICITY_FLOAT, fElasticity))
	{
		_phys_obj->m_fElasticity = fElasticity;
	}

	int placement_id = 0;
	if (m_Qualities.InqInt(PLACEMENT_POSITION_INT, placement_id, TRUE))
	{
		_phys_obj->SetPlacementFrame(placement_id, FALSE);
	}

	if (uint32_t combatTable = m_Qualities.GetDID(COMBAT_TABLE_DID, 0))
	{
		_combatTable = CombatManeuverTable::Get(combatTable);
	}
}

double CWeenieObject::GetHealthPercent()
{
	unsigned int maxHealth = GetMaxHealth();
	if (!maxHealth)
		return 0.0f;

	return GetHealth() / (double)maxHealth;
}

unsigned int CWeenieObject::GetHealth()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(HEALTH_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxHealth()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

void CWeenieObject::SetHealth(unsigned int value, bool bSendUpdate)
{
	if (value > GetMaxHealth())
		value = GetMaxHealth();

	m_Qualities.SetAttribute2nd(HEALTH_ATTRIBUTE_2ND, value);

	if (bSendUpdate)
		NotifyAttribute2ndStatUpdated(HEALTH_ATTRIBUTE_2ND);
}

unsigned int CWeenieObject::GetStamina()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(STAMINA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxStamina()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(MAX_STAMINA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

void CWeenieObject::SetStamina(unsigned int value, bool bSendUpdate)
{
	if (value > GetMaxStamina())
		value = GetMaxStamina();

	m_Qualities.SetAttribute2nd(STAMINA_ATTRIBUTE_2ND, value);

	if (bSendUpdate)
		NotifyAttribute2ndStatUpdated(STAMINA_ATTRIBUTE_2ND);
}

unsigned int CWeenieObject::GetMana()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(MANA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxMana()
{
	uint32_t value = 0;
	m_Qualities.InqAttribute2nd(MAX_MANA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

void CWeenieObject::SetMana(unsigned int value, bool bSendUpdate)
{
	if (value > GetMaxMana())
		value = GetMaxMana();

	m_Qualities.SetAttribute2nd(MANA_ATTRIBUTE_2ND, value);

	if (bSendUpdate)
		NotifyAttribute2ndStatUpdated(MANA_ATTRIBUTE_2ND);
}

void CWeenieObject::SetMaxVitals(bool bSendUpdate)
{
	SetHealth(GetMaxHealth(), bSendUpdate);
	SetStamina(GetMaxStamina(), bSendUpdate);
	SetMana(GetMaxMana(), bSendUpdate);
}

int CWeenieObject::AdjustHealth(int amount, bool useRatings)
{
	int ratingAdj = 0;
	// TODO: Special cases that are not affected by ratings, such as Surge of Regeneration.
	if (useRatings)
	{
		if (amount > 0) // Heals
		{
			ratingAdj += GetRating(HEALING_BOOST_RATING_INT);
			ratingAdj -= GetRating(HEALING_RESIST_RATING_INT);
		}
		else // Drains
			ratingAdj -= GetRating(LIFE_RESIST_RATING_INT);

		amount *= GetRatingMod(ratingAdj);
	}

	// amount can be negative or positive
	int maxHealth = GetMaxHealth();
	int oldHealth = GetHealth();
	int newHealth = oldHealth + amount;

	if (newHealth <= 0)
		newHealth = 0;
	else if (newHealth >= maxHealth)
		newHealth = maxHealth;

	int adjustment = newHealth - oldHealth;
	SetHealth(newHealth);

	return adjustment;
}

int CWeenieObject::AdjustStamina(int amount)
{
	// amount can be negative or positive
	int maxStamina = GetMaxStamina();
	int oldStamina = GetStamina();
	int newStamina = oldStamina + amount;

	if (newStamina <= 0)
		newStamina = 0;
	else if (newStamina >= maxStamina)
		newStamina = maxStamina;

	int adjustment = newStamina - oldStamina;
	SetStamina(newStamina);

	return adjustment;
}

int CWeenieObject::AdjustMana(int amount)
{
	// amount can be negative or positive
	int maxMana = GetMaxMana();
	int oldMana = GetMana();
	int newMana = oldMana + amount;

	if (newMana <= 0)
		newMana = 0;
	else if (newMana >= maxMana)
		newMana = maxMana;

	int adjustment = newMana - oldMana;
	SetMana(newMana);

	return adjustment;
}

bool CWeenieObject::IsDead()
{
	if (GetHealth() > 0)
		return false;

	// we'll consider objects with no health are considered invincible for now
	if (!GetMaxHealth())
		return false;

	return true;
}

bool CWeenieObject::IsInPortalSpace()
{
	if (InqFloatQuality(LAST_TELEPORT_START_TIMESTAMP_FLOAT, 0))
		return true;

	return false;
}

void CWeenieObject::OnMotionDone(uint32_t motion, BOOL success)
{
	if (m_UseManager)
		m_UseManager->OnMotionDone(motion, success);

	if (m_SpellcastingManager)
		m_SpellcastingManager->HandleMotionDone(motion, success);

	if (m_AttackManager)
		m_AttackManager->OnMotionDone(motion, success);

	if (motion == Motion_Reload)
	{
		switch (get_minterp()->InqStyle())
		{
		case Motion_ThrownWeaponCombat:
		case Motion_AtlatlCombat:
		case Motion_CrossbowCombat:
		case Motion_BowCombat:
		{
			CWeenieObject *ammo;
			if (get_minterp()->InqStyle() == Motion_ThrownWeaponCombat)
				ammo = GetWieldedCombat(COMBAT_USE_MISSILE); //for thrown weapons the ammo is the weapon itself.
			else
				ammo = GetWieldedCombat(COMBAT_USE_AMMO);

			if (ammo)
			{
				if (ammo->parent != this)
				{
					ammo->m_Qualities.SetInt(PARENT_LOCATION_INT, PARENT_ENUM::PARENT_RIGHT_HAND);
					ammo->set_parent(this, PARENT_ENUM::PARENT_RIGHT_HAND);
					ammo->SetPlacementFrame(Placement::RightHandCombat, FALSE);

					if (m_bWorldIsAware)
					{
						if (CWeenieObject *owner = GetWorldTopLevelOwner())
						{
							if (owner->GetBlock())
							{
								owner->GetBlock()->ExchangePVS(ammo, 0);
							}
						}

						/*
						BinaryWriter *writer = item->CreateMessage();
						g_pWorld->BroadcastPVS(dwCell, writer->GetData(), writer->GetSize(), OBJECT_MSG, 0, FALSE);
						delete writer;
						*/

						BinaryWriter Blah;
						Blah.Write<uint32_t>(0xF749);
						Blah.Write<uint32_t>(GetID());
						Blah.Write<uint32_t>(ammo->GetID());
						Blah.Write<uint32_t>(PARENT_ENUM::PARENT_RIGHT_HAND);
						Blah.Write<uint32_t>(Placement::RightHandCombat);
						Blah.Write<WORD>(GetPhysicsObj()->_instance_timestamp);
						Blah.Write<WORD>(++ammo->_position_timestamp);
						g_pWorld->BroadcastPVS(GetLandcell(), Blah.GetData(), Blah.GetSize(), OBJECT_MSG, false, false, true);
					}
				}
			}

			break;
		}
		}

		DoForcedStopCompletely();
	}

	CWeenieObject *ammo;
	if (get_minterp()->InqStyle() == Motion_ThrownWeaponCombat)
		ammo = GetWieldedCombat(COMBAT_USE_MISSILE); //for thrown weapons the ammo is the weapon itself.
	else
		ammo = GetWieldedCombat(COMBAT_USE_AMMO);

	if (ammo)
	{
		if (!HasInterpActions() && get_minterp()->interpreted_state.forward_command != Motion_Reload && !IsDead())
		{
			switch (get_minterp()->InqStyle())
			{
			default:
			case Motion_NonCombat:
			case Motion_SwordCombat:
			case Motion_HandCombat:
			case Motion_SwordShieldCombat:
			case Motion_Magic:
			{
				if (ammo->parent == this)
				{
					if (m_bWorldIsAware)
					{
						ammo->_position_timestamp++;

						BinaryWriter Blah;
						Blah.Write<uint32_t>(0xF74A);
						Blah.Write<uint32_t>(ammo->GetID());
						Blah.Write<WORD>(ammo->_instance_timestamp);
						Blah.Write<WORD>(ammo->_position_timestamp);
						g_pWorld->BroadcastPVS(GetLandcell(), Blah.GetData(), Blah.GetSize());
					}

					ammo->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
					ammo->unset_parent();
					ammo->leave_world();
				}

				break;
			}

			case Motion_ThrownWeaponCombat:
			case Motion_AtlatlCombat:
			case Motion_CrossbowCombat:
			case Motion_BowCombat:
			{
				if (ammo->parent != this)
				{
					MovementParameters params;
					get_minterp()->DoMotion(Motion_Reload, &params);

					_server_control_timestamp++;
					last_move_was_autonomous = false;

					Animation_Update();
				}

				break;
			}
			}
		}
	}
}

void CWeenieObject::OnDeath(uint32_t killer_id)
{
	if (CWeenieObject *pKiller = g_pWorld->FindObject(killer_id))
	{
		pKiller->ChanceExecuteEmoteSet(GetID(), KillTaunt_EmoteCategory);
	}

	if (m_UseManager)
		m_UseManager->OnDeath(killer_id);

	if (m_SpellcastingManager)
		m_SpellcastingManager->OnDeath(killer_id);

	//if (m_EmoteManager)
	//	m_EmoteManager->OnDeath(killer_id);
	MakeEmoteManager()->OnDeath(killer_id);

	if (m_AttackManager)
		m_AttackManager->OnDeath(killer_id);

	//ChanceExecuteEmoteSet(killer_id, Death_EmoteCategory);

	m_Qualities.SetInt(NETHER_OVER_TIME_INT, 0);
	NotifyIntStatUpdated(NETHER_OVER_TIME_INT);
	m_Qualities.SetInt(DAMAGE_OVER_TIME_INT, 0);
	NotifyIntStatUpdated(DAMAGE_OVER_TIME_INT);

	bool isPkDeath = false;
	CWeenieObject* killer = g_pWorld->FindPlayer(killer_id);
	if (killer && killer->AsPlayer() && !killer->IsPKLite())
		isPkDeath = true;

	bool spellsLastBeyondDeath = m_Qualities.GetInt(AUGMENTATION_SPELLS_REMAIN_PAST_DEATH_INT, 0) > 0;
	

	if (m_Qualities._enchantment_reg)
	{
		PackableList<uint32_t> removed;

		if (m_Qualities._enchantment_reg->_add_list)
		{
			for (auto it = m_Qualities._enchantment_reg->_add_list->begin(); it != m_Qualities._enchantment_reg->_add_list->end();)
			{
				if (it->_duration == -1.0)
				{
					it++;
				}
				else
				{
					const CSpellBase* spellBase = MagicSystem::GetSpellTable()->GetSpellBase(it->_id);
					if (spellBase)
					{
						bool isSpellBeneficial = spellBase->_bitfield & Beneficial_SpellIndex;
						if ((it->_duration == -1.0) || (it->_duration > 0 && spellsLastBeyondDeath && isSpellBeneficial && !isPkDeath))
						{
							it++;
						}
						else
						{
							removed.push_back(it->_id);
							it = m_Qualities._enchantment_reg->_add_list->erase(it);
						}
					}
					else
					{
						SERVER_ERROR << "OnDeath: Enchantment AddList - Unable to get spell base for spell ID:" << it->_id;
						removed.push_back(it->_id);
						it = m_Qualities._enchantment_reg->_add_list->erase(it);
					}
				}
			}
		}

		if (m_Qualities._enchantment_reg->_mult_list)
		{
			for (auto it = m_Qualities._enchantment_reg->_mult_list->begin(); it != m_Qualities._enchantment_reg->_mult_list->end();)
			{
				if (it->_duration == -1.0)
				{
					it++;
				}
				else
				{
					const CSpellBase* spellBase = MagicSystem::GetSpellTable()->GetSpellBase(it->_id);
					if (spellBase)
					{
						bool isSpellBeneficial = spellBase->_bitfield & Beneficial_SpellIndex;
						if ((it->_duration == -1.0) || (it->_duration > 0 && spellsLastBeyondDeath && isSpellBeneficial && !isPkDeath))
						{
							it++;
						}
						else
						{
							removed.push_back(it->_id);
							it = m_Qualities._enchantment_reg->_mult_list->erase(it);
						}
					}
					else
					{
						SERVER_ERROR << "OnDeath: Enchantment MultiList - Unable to get spell base for spell ID:" << it->_id;
						removed.push_back(it->_id);
						it = m_Qualities._enchantment_reg->_mult_list->erase(it);
					}
				}
			}
		}

		if (removed.size())
		{
			// m_Qualities._enchantment_reg->PurgeEnchantments();

			BinaryWriter expireMessage;
			expireMessage.Write<uint32_t>(0x2C8);
			removed.Pack(&expireMessage);

			SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
		}
	}

	m_Qualities.SetInt(NUM_DEATHS_INT, InqIntQuality(NUM_DEATHS_INT, 0) + 1);
	NotifyIntStatUpdated(NUM_DEATHS_INT);
}

void CWeenieObject::Revive()
{
	last_move_was_autonomous = false;
	StopCompletely(0);
	_server_control_timestamp++;

	ChangeCombatMode(NONCOMBAT_COMBAT_MODE, false);

	TeleportToSpawn();

	assert(get_minterp()->interpreted_state.forward_command != GetCommandID(0x11));

	Animation_Update();

	SetHealth((int)(GetMaxHealth() * InqFloatQuality(HEALTH_UPON_RESURRECTION_FLOAT, 1.0f)));
	SetStamina((int)(GetMaxStamina() * InqFloatQuality(STAMINA_UPON_RESURRECTION_FLOAT, 1.0f)));
	SetMana((int)(GetMaxMana() * InqFloatQuality(MANA_UPON_RESURRECTION_FLOAT, 1.0f)));

	m_Qualities.SetBool(UNDER_LIFESTONE_PROTECTION_BOOL, 1);
	m_Qualities.SetFloat(LIFESTONE_PROTECTION_TIMESTAMP_FLOAT, (Timer::cur_time + 60.0));

	fellowship_ptr_t fs = GetFellowship();
	if (fs)
	{
		fs->VitalsUpdate(id);
	}
}

bool CWeenieObject::TeleportToSpawn()
{
	extern bool g_bReviveOverride;
	extern Position g_RevivePosition;

	if (g_bReviveOverride)
	{
		Movement_Teleport(
			g_RevivePosition
			/*Position(0x7B610007, Vector(23.343912f, 159.351471f, 10.004999f), Quaternion(-0.171325f, 0.000000f, 0.000000f, 0.985215f))*/
			/*
			Position(0x7C65002B,
			Vector(141.572693f, 59.415398f, 9.802724f),
			Quaternion(0.907020f, 0.000000f, 0.000000f, 0.421088f))
			*/,
			true);
		return true;
	}

	if (TeleportToLifestone())
		return true;

	Position initialPos;
	if (m_Qualities.InqPosition(INSTANTIATION_POSITION, initialPos) && initialPos.objcell_id)
	{
		Movement_Teleport(initialPos, true);
		return true;
	}

	return false;
}

void CWeenieObject::Attach(CWorldLandBlock *pBlock)
{
	m_pBlock = pBlock;
}

void CWeenieObject::Detach()
{
	m_pBlock = NULL;
}

CWorldLandBlock *CWeenieObject::GetBlock()
{
	return m_pBlock;
}

void CWeenieObject::ReleaseFromBlock()
{
	if (CWorldLandBlock *pBlock = GetBlock())
		pBlock->Release(this);
}

void CWeenieObject::TryIdentify(CWeenieObject *source)
{
	bool success = false;
	bool bShowLevel = false;

	if (source == this)
	{
		success = true;
	}
	else if (AsPlayer() && source->AsPlayer())
	{
		if (!(AsPlayer()->GetCharacterOptions() & UseDeception_CharacterOption))
			success = true;
	}

	if (!success)
	{
		uint32_t deceptionSkill = 0;

		if (InqType() & TYPE_ITEM)
		{
			deceptionSkill = (uint32_t)m_Qualities.GetInt(RESIST_ITEM_APPRAISAL_INT, 0);
		}
		else
		{
			m_Qualities.InqSkill(DECEPTION_SKILL, deceptionSkill, FALSE);

			if (IsCreature() && !_IsPlayer())
			{
				uint32_t focus = 0, self = 0;
				m_Qualities.InqAttribute(FOCUS_ATTRIBUTE, focus, FALSE);
				m_Qualities.InqAttribute(SELF_ATTRIBUTE, self, FALSE);
				deceptionSkill += (focus + self) / 4;
			}

			float dist = DistanceTo(source, true);
			if (dist < 60.0)
			{
				dist = max(0.0f, dist);
				deceptionSkill *= 0.75 + ((dist / 60.0) * 0.25);
			}
		}

		if (deceptionSkill > 0)
		{
			STypeSkill sourceSkillType = STypeSkill::UNDEF_SKILL;

			uint32_t itemType = InqType();

			if (_IsPlayer())
			{
				sourceSkillType = STypeSkill::PERSONAL_APPRAISAL_SKILL;
			}
			else if (itemType & TYPE_CREATURE)
			{
				sourceSkillType = STypeSkill::CREATURE_APPRAISAL_SKILL;
			}
			else if (itemType & TYPE_VESTEMENTS)
			{
				sourceSkillType = STypeSkill::ARMOR_APPRAISAL_SKILL;
			}
			else if (itemType & TYPE_WEAPON_OR_CASTER)
			{
				sourceSkillType = STypeSkill::WEAPON_APPRAISAL_SKILL;
			}
			else if (itemType & TYPE_ITEM)
			{
				sourceSkillType = STypeSkill::ITEM_APPRAISAL_SKILL;

				if (m_Qualities._spell_book && !m_Qualities._spell_book->_spellbook.empty())
				{
					sourceSkillType = STypeSkill::MAGIC_ITEM_APPRAISAL_SKILL;
				}
			}

			if (sourceSkillType != STypeSkill::UNDEF_SKILL)
			{
				uint32_t appraiseSkill = 0;
				source->m_Qualities.InqSkill(sourceSkillType, appraiseSkill, FALSE);

				if (IsCreature())
					appraiseSkill += 50;

				if (AppraisalSkillCheck(appraiseSkill + 50, deceptionSkill))
				{
					bShowLevel = true;

					if (AppraisalSkillCheck(appraiseSkill, deceptionSkill))
					{
						success = true;
					}
				}
			}
			else
			{
				success = true;
			}
		}
		else
		{
			success = true;
		}
	}

	OnIdentifyAttempted(source);

	if (success)
	{
		Identify(source);
	}
	else
	{
		source->SendNetMessage(IdentifyObjectFail(this, bShowLevel), PRIVATE_MSG, TRUE, TRUE);

		if (AsPlayer())
		{
			SendText(csprintf("%s tried and failed to assess you!", source->GetName().c_str()), LTT_APPRAISAL_CHANNEL);
		}
	}
}

void CWeenieObject::Identify(CWeenieObject *source, uint32_t overrideId)
{
	source->SendNetMessage(IdentifyObject(source, this, overrideId), PRIVATE_MSG, TRUE, TRUE);
}

float CWeenieObject::DistanceTo(CWeenieObject *other, bool bUseSpheres)
{
	if (!_phys_obj || !other || !other->_phys_obj)
		return FLT_MAX;

	if (this == other)
		return 0.0;

	float dist = _phys_obj->DistanceTo(other->_phys_obj);

	if (bUseSpheres) // && InValidCell() && other->InValidCell())
	{
		dist -= GetRadius() + other->GetRadius();
		if (dist < F_EPSILON)
			dist = 0.0;
	}

	return dist;
}

float CWeenieObject::DistanceSquared(CWeenieObject *other)
{
	if (!_phys_obj || !other || !other->_phys_obj)
		return FLT_MAX;

	if (this == other)
		return 0.0;

	return _phys_obj->DistanceSquared(other->_phys_obj);
}

float CWeenieObject::HeadingTo(CWeenieObject *target, bool relative)
{
	if (!target || this == target || target->IsContained())
		return 0.0f;

	float headingToTarget = m_Position.heading_diff(target->m_Position);
	headingToTarget = fabs(headingToTarget);

	if (relative)
	{
		if (headingToTarget >= 180.0f)
			headingToTarget = 360.0f - headingToTarget;
	}
	else
		return headingToTarget;

	return headingToTarget;
}

float CWeenieObject::HeadingFrom(CWeenieObject *target, bool relative)
{
	if (target != nullptr)
		return target->HeadingTo(this, relative);
	return 0.0;
}

float CWeenieObject::HeadingTo(uint32_t targetId, bool relative)
{
	if (!targetId || targetId == GetID())
		return 0.0;

	CWeenieObject *target = g_pWorld->FindObject(targetId);
	if (!target)
		return 0.0;

	return HeadingTo(target, relative);
}

float CWeenieObject::HeadingFrom(uint32_t targetId, bool relative)
{
	if (!targetId || targetId == GetID())
		return 0.0;

	CWeenieObject *target = g_pWorld->FindObject(targetId);
	if (!target)
		return 0.0;

	return target->HeadingTo(this, relative);
}

uint32_t CWeenieObject::GetLandcell()
{
	return m_Position.objcell_id; // return _phys_obj ? _phys_obj->m_Position.objcell_id : 0;
}

void CWeenieObject::SpeakLocal(const char *text, LogTextType ltt)
{
	if (HasOwner())
		return;

	extern bool g_bSilence;
	if (g_bSilence && !IsAdmin())
	{
		return;
	}

	if (AsPlayer())
	{
		std::list<CWeenieObject *> results;
		g_pWorld->EnumNearby(GetPosition(), 30.0f, &results);

		for (auto weenie : results)
		{
			if (weenie == this)
				continue;

			weenie->ChanceExecuteEmoteSet(GetID(), HearChat_EmoteCategory);
		}
	}

	BinaryWriter *LC = LocalChat(text, GetName().c_str(), GetID(), ltt);
	g_pWorld->BroadcastPVS(GetLandcell(), LC->GetData(), LC->GetSize(), PRIVATE_MSG);
	delete LC;
}

void CWeenieObject::EmoteLocal(const char* szText)
{
	if (HasOwner())
		return;

	extern bool g_bSilence;
	if (g_bSilence && !IsAdmin())
	{
		return;
	}

	BinaryWriter *EL = EmoteChat(szText, GetName().c_str(), GetID());
	g_pWorld->BroadcastPVS(GetLandcell(), EL->GetData(), EL->GetSize(), PRIVATE_MSG);
	delete EL;
}

void CWeenieObject::ActionLocal(const char* szText)
{
	if (HasOwner())
		return;
	
	extern bool g_bSilence;
	if (g_bSilence && !IsAdmin())
	{
		return;
	}

	BinaryWriter *AL = ActionChat(szText, GetName().c_str(), GetID());
	g_pWorld->BroadcastPVS(GetLandcell(), AL->GetData(), AL->GetSize(), PRIVATE_MSG);
	delete AL;
}

EmoteManager *CWeenieObject::MakeEmoteManager()
{
	if (!m_EmoteManager)
	{
		m_EmoteManager = new EmoteManager(this);
	}

	return m_EmoteManager;
}

UseManager *CWeenieObject::MakeUseManager()
{
	if (!m_UseManager)
	{
		m_UseManager = new UseManager(this);
	}

	return m_UseManager;
}

CSpellcastingManager *CWeenieObject::MakeSpellcastingManager()
{
	if (!m_SpellcastingManager)
	{
		m_SpellcastingManager = new CSpellcastingManager(this);
	}

	return m_SpellcastingManager;
}

AttackManager *CWeenieObject::MakeAttackManager()
{
	if (!m_AttackManager)
	{
		m_AttackManager = new AttackManager(this);
	}

	return m_AttackManager;
}

BYTE CWeenieObject::GetNextStatTimestamp(StatType statType, int statIndex)
{
	uint32_t key = (statType << 16) | statIndex;

	BYTE nextStat = m_StatSequences[key] + 1;
	m_StatSequences[key] = nextStat;

	return nextStat;
}

void CWeenieObject::NotifyObjectCreated(bool bPrivate)
{
	BinaryWriter *CM = CreateMessage();
	if (CM)
	{
		if (bPrivate)
			SendNetMessage(CM, OBJECT_MSG, FALSE, FALSE);
		else
			g_pWorld->BroadcastPVS(this, CM->GetData(), CM->GetSize(), OBJECT_MSG, 0);
		delete CM;
	}
}

void CWeenieObject::NotifyObjectUpdated(bool bPrivate)
{
	BinaryWriter *UM = UpdateMessage();
	if (UM)
	{
		if (bPrivate)
			SendNetMessage(UM, OBJECT_MSG, FALSE, FALSE);
		else
			g_pWorld->BroadcastPVS(this, UM->GetData(), UM->GetSize(), OBJECT_MSG, 0);
		delete UM;
	}
}

void CWeenieObject::NotifyIntStatUpdated(STypeInt key, bool bPrivate)
{
	if (!m_bWorldIsAware && !bPrivate)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Int_StatType, key);

	int value;
	if (m_Qualities.InqInt(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2CD);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2CE);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1D1);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1D2);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifiyObjectIntStatUpdated(uint32_t objectId, STypeInt key, int value)
{
	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Int_StatType, key);

	statNotify.Write<uint32_t>(0x2CE);
	statNotify.Write<BYTE>(statTS);
	statNotify.Write<uint32_t>(objectId);
	statNotify.Write<uint32_t>(key);
	statNotify.Write<int>(value);
	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifyInt64StatUpdated(STypeInt64 key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Int64_StatType, key);

	int64_t value;
	if (m_Qualities.InqInt64(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2CF);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int64_t>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2D0);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int64_t>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2B8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2B9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyBoolStatUpdated(STypeBool key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Bool_StatType, key);

	BOOL value;
	if (m_Qualities.InqBool(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2D1);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2D2);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<int>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1D3);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1D4);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyFloatStatUpdated(STypeFloat key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Float_StatType, key);

	double value;
	if (m_Qualities.InqFloat(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2D3);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<double>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2D4);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<double>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1D5);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1D6);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyStringStatUpdated(STypeString key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(String_StatType, key);

	std::string value;
	if (m_Qualities.InqString(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2D5);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.WriteString(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2D6);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.WriteString(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1D7);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1D8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyDIDStatUpdated(STypeDID key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(DID_StatType, key);

	uint32_t value;

	if (m_Qualities.InqDataID(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2D7);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<uint32_t>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2D8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<uint32_t>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1D9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyObjectDIDStatUpdated(uint32_t objectId, STypeDID key, uint32_t value)
{
	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(DID_StatType, key);

	statNotify.Write<uint32_t>(0x2D8);
	statNotify.Write<BYTE>(statTS);
	statNotify.Write<uint32_t>(objectId);
	statNotify.Write<uint32_t>(key);
	statNotify.Write<uint32_t>(value);
	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifyIIDStatUpdated(STypeIID key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(IID_StatType, key);

	uint32_t value;
	if (m_Qualities.InqInstanceID(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2D9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			statNotify.Write<uint32_t>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			statNotify.Write<uint32_t>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1DB);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1DC);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyPositionStatUpdated(STypePosition key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Position_StatType, key);

	Position value;
	if (m_Qualities.InqPosition(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x2DB);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			value.Pack(&statNotify);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x2DC);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			value.Pack(&statNotify);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<uint32_t>(0x1DD);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
		}
		else
		{
			statNotify.Write<uint32_t>(0x1DE);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
		}
	}
}

void CWeenieObject::NotifyStackSizeUpdated(bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter msg;
	msg.Write<uint32_t>(0x197);
	msg.Write<BYTE>(GetNextStatTimestamp(Int_StatType, STACK_SIZE_INT));
	msg.Write<uint32_t>(GetID());
	msg.Write<uint32_t>(InqIntQuality(STACK_SIZE_INT, 1));
	msg.Write<uint32_t>(InqIntQuality(VALUE_INT, 0));

	if (bPrivate)
		SendNetMessage(&msg, PRIVATE_MSG, FALSE, FALSE, true);
	else
		g_pWorld->BroadcastPVS(this, msg.GetData(), msg.GetSize(), PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifyContainedItemRemoved(uint32_t objectId, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter msg;
	msg.Write<uint32_t>(0x24);
	msg.Write<uint32_t>(objectId);

	if (bPrivate)
		SendNetMessage(&msg, PRIVATE_MSG, FALSE, FALSE);
	else
		g_pWorld->BroadcastPVS(this, msg.GetData(), msg.GetSize(), PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifyObjectRemoved()
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter msg;
	msg.Write<uint32_t>(0xF747);
	msg.Write<uint32_t>(GetID());
	msg.Write<uint32_t>(_instance_timestamp);

	g_pWorld->BroadcastPVS(this, msg.GetData(), msg.GetSize());
}

void CWeenieObject::CopyIntStat(STypeInt key, CWeenieObject *from)
{
	CopyIntStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyIntStat(STypeInt key, CACQualities *from)
{
	int value = 0;
	if (from->InqInt(key, value, TRUE, TRUE))
		m_Qualities.SetInt(key, value);
}

void CWeenieObject::CopyInt64Stat(STypeInt64 key, CWeenieObject *from)
{
	CopyInt64Stat(key, &from->m_Qualities);
}

void CWeenieObject::CopyInt64Stat(STypeInt64 key, CACQualities *from)
{
	int64_t value = 0;
	if (from->InqInt64(key, value))
		m_Qualities.SetInt64(key, value);
}

void CWeenieObject::CopyBoolStat(STypeBool key, CWeenieObject *from)
{
	CopyBoolStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyBoolStat(STypeBool key, CACQualities *from)
{
	int value = 0;
	if (from->InqBool(key, value))
		m_Qualities.SetBool(key, value);
}

void CWeenieObject::CopyFloatStat(STypeFloat key, CWeenieObject *from)
{
	CopyFloatStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyFloatStat(STypeFloat key, CACQualities *from)
{
	double value = 0;
	if (from->InqFloat(key, value, TRUE))
		m_Qualities.SetFloat(key, value);
}

void CWeenieObject::CopyStringStat(STypeString key, CWeenieObject *from)
{
	CopyStringStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyStringStat(STypeString key, CACQualities *from)
{
	std::string value;
	if (from->InqString(key, value))
		m_Qualities.SetString(key, value);
}

void CWeenieObject::CopyDIDStat(STypeDID key, CWeenieObject *from)
{
	CopyDIDStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyDIDStat(STypeDID key, CACQualities *from)
{
	uint32_t value;

	if (from->InqDataID(key, value))
		m_Qualities.SetDataID(key, value);
}

void CWeenieObject::CopyPositionStat(STypePosition key, CWeenieObject *from)
{
	CopyPositionStat(key, &from->m_Qualities);
}

void CWeenieObject::CopyPositionStat(STypePosition key, CACQualities *from)
{
	Position value;
	if (from->InqPosition(key, value))
		m_Qualities.SetPosition(key, value);
}

void CWeenieObject::NotifyAttributeStatUpdated(STypeAttribute key)
{
	BinaryWriter statNotify;
	statNotify.Write<uint32_t>(0x2E3);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Attribute_StatType, key));
	statNotify.Write<uint32_t>(key);

	Attribute attrib;
	m_Qualities.InqAttribute(key, attrib);
	statNotify.Write(&attrib);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);

	updateFellowStats = true;
}

void CWeenieObject::NotifyAttribute2ndStatUpdated(STypeAttribute2nd key)
{
	BinaryWriter statNotify;
	statNotify.Write<uint32_t>(0x2E7);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Attribute_2nd_StatType, key));
	statNotify.Write<uint32_t>(key);

	SecondaryAttribute attrib2nd;
	m_Qualities.InqAttribute2nd(key, attrib2nd);
	statNotify.Write(&attrib2nd);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifySkillStatUpdated(STypeSkill key)
{
	BinaryWriter statNotify;
	statNotify.Write<uint32_t>(0x2DD);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Skill_StatType, key));
	statNotify.Write<uint32_t>(key);

	Skill skill;
	m_Qualities.InqSkill(key, skill);
	statNotify.Write(&skill);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifySkillAdvancementClassUpdated(STypeSkill key)
{
	BinaryWriter statNotify;
	statNotify.Write<uint32_t>(0x2E1);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Skill_StatType, key));
	statNotify.Write<uint32_t>(key);

	SKILL_ADVANCEMENT_CLASS sac;
	m_Qualities.InqSkillAdvancementClass(key, sac);
	statNotify.Write<int>(sac);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE, true);
}

void CWeenieObject::NotifyEnchantmentUpdated(Enchantment *enchant)
{
	BinaryWriter statNotify;
	statNotify.Write<uint32_t>(0x2C2);
	enchant->Pack(&statNotify);

	SendNetMessage(&statNotify, PRIVATE_MSG, TRUE, FALSE);
}

uint32_t CWeenieObject::GetCostToRaiseSkill(STypeSkill key)
{
	Skill skill;
	m_Qualities.InqSkill(key, skill);

	if (skill._sac < TRAINED_SKILL_ADVANCEMENT_CLASS)
	{
		SkillTable *pSkillTable = SkillSystem::GetSkillTable();

		if (!pSkillTable)
			return 0;

		const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
		if (pSkillBase != NULL)
		{
			return pSkillBase->_trained_cost;
		}
	}
	else
	{
		uint32_t maxLevel;

		switch (skill._sac)
		{
		default:
			return 0;
		case TRAINED_SKILL_ADVANCEMENT_CLASS:
			maxLevel = ExperienceSystem::GetMaxTrainedSkillLevel();
			break;
		case SPECIALIZED_SKILL_ADVANCEMENT_CLASS:
			maxLevel = ExperienceSystem::GetMaxSpecializedSkillLevel();
			break;
		}

		if (skill._level_from_pp < maxLevel)
		{
			uint32_t expRequired = ExperienceSystem::ExperienceToSkillLevel(skill._sac, skill._level_from_pp + 1);
			expRequired -= skill._pp;
			return expRequired;
		}
	}

	return 0;
}

const char* CWeenieObject::GetGenderString()
{
	int value;
	if (m_Qualities.InqInt(GENDER_INT, value))
	{
		switch (value)
		{
		case 1:
			return "Male";
		case 2:
			return "Female";
		}
	}

	return "Unknown";
}

const char* CWeenieObject::GetRaceString()
{
	return "[race]";
}

const char* CWeenieObject::GetTitleString()
{
	return "[title]";
}

uint32_t CWeenieObject::GetXPForKillLevel(int level)
{
	double xp = 380.5804 + 63.92362*level + 3.543397*pow(level, 2) - 0.05233995*pow(level, 3) + 0.0007008949*pow(level, 4);

	uint32_t xpvalout;

	if (xp < 0)
		xpvalout = 0;
	else if (xp >= UINT_MAX)
		xpvalout = UINT_MAX;
	else
		xpvalout = (uint32_t)xp;

	// xp /= 5.0; // normally things are way too easy, let's tone it down a bit

	return xpvalout;
}

void CWeenieObject::GivePerksForKill(CWeenieObject *pKilled)
{
	if (!pKilled || !pKilled->IsCreature() || pKilled->_IsPlayer())
		return;

	int level = pKilled->InqIntQuality(LEVEL_INT, 0);
	if (level <= 0)
		return;

	int baseXPForKill = 0;
	int baseLumForKill = 0;
	int64_t xpForKill = 0;
	int64_t lumForKill = 0;

	if (!pKilled->m_Qualities.InqInt(XP_OVERRIDE_INT, baseXPForKill, 0, FALSE))
		baseXPForKill = (uint32_t)GetXPForKillLevel(level);

	xpForKill = (int64_t)(baseXPForKill * g_pConfig->GetKillXPMultiplier(level));

	if (xpForKill < 0)
		xpForKill = 0;

	if (pKilled->m_Qualities.InqInt(LUMINANCE_AWARD_INT, baseLumForKill, 0, FALSE))
		lumForKill = (int64_t)(baseLumForKill * g_pConfig->GetLumXPMultiplier());

	GiveSharedXP(xpForKill, false);
	GiveSharedLum(lumForKill, false);
}

void CWeenieObject::GiveSharedXP(int64_t amount, bool showText)
{
	if (amount <= 0)
		return;

	EnchantedQualityDetails buffDetails;
	GetFloatEnchantmentDetails(GLOBAL_XP_MOD_FLOAT, 0.0, &buffDetails);

	if (buffDetails.enchantedValue > 0.0)
		amount *= buffDetails.valueIncreasingMultiplier;

	fellowship_ptr_t f = GetFellowship();

	if (f && f->_share_xp)
		f->GiveXP(this, amount, ExperienceHandlingType::DefaultXp, showText);
	else
		GiveXP(amount, ExperienceHandlingType::DefaultXp, showText);
}

void CWeenieObject::GiveXP(int64_t amount, ExperienceHandlingType flags, bool showText)
{
	if (amount <= 0)
		return;

	OnGivenXP(amount, flags);
	//return; // MOROSITY - Skipping item leveling until more work done

	// items don't get non-direct xp
	// if we can't share it, it was *probably* already shared
	// must also count towards vitae
	if (!flags_check(flags, ExperienceHandlingType::PossibleItemXp))
		return;

	int32_t style = InqIntQuality(ITEM_XP_STYLE_INT, 0);
	if (style == 0)
		return;

	bool bLeveled = false;
	uint64_t currxp = (uint64_t)InqInt64Quality(ITEM_TOTAL_XP_INT64, 0);
	uint64_t basexp = (uint64_t)InqInt64Quality(ITEM_BASE_XP_INT64, 0);
	int32_t maxlvl = InqIntQuality(ITEM_MAX_LEVEL_INT, 0);

	int32_t baseLevel = ExperienceSystem::ItemTotalXpToLevel(currxp, basexp, maxlvl, style);
	int32_t currentLevel = baseLevel;
	uint64_t xpToNextLevel = ExperienceSystem::ItemLevelToTotalXp(currentLevel + 1, basexp, maxlvl, style);
	uint64_t maxxp = ExperienceSystem::ItemLevelToTotalXp(maxlvl, basexp, maxlvl, style);

	currxp += amount;

	while (xpToNextLevel <= currxp && currentLevel < maxlvl)
	{
		currentLevel++;
		bLeveled = true;

		xpToNextLevel = ExperienceSystem::ItemLevelToTotalXp(currentLevel + 1, basexp, maxlvl, style);

		if (!xpToNextLevel)
			break;
	}
	if (currxp <= maxxp)
		m_Qualities.SetInt64(ITEM_TOTAL_XP_INT64, currxp);

	if (currxp > maxxp)
		m_Qualities.SetInt64(ITEM_TOTAL_XP_INT64, maxxp);

	if (bLeveled)
	{
		CMonsterWeenie *owner = GetWorldTopLevelOwner()->AsMonster();
		if (!owner)	// abort, something bad happened
			return;

		owner->EmitEffect(PS_AetheriaLevelUp, 1.0f);

		const char *notice = csprintf(
			"Your %s has increased in power to level %d!",
			GetName().c_str(),
			currentLevel);

		owner->SendText(notice, LTT_ADVANCEMENT);

		int setId;
		if (m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, setId))
		{
			if(owner->AsPlayer())
			{
				owner->AsPlayer()->UpdateSetSpells(setId, GetID());
				if (IsAetheria())
				{
					owner->AsPlayer()->UpdateSigilProcRate();
				}
			}
		}
	}
}

void CWeenieObject::UpdateSetLevel(int setId, int leveldiff, int sourceid)
{
	if (leveldiff == 0)
		return;

	auto sourceItem = g_pWorld->FindObject(sourceid);
	auto owner = sourceItem->GetWorldTopLevelOwner();

	if (owner->m_pendingSetChanges.count(setId))
	{
		owner->m_pendingSetChanges[setId].second += leveldiff;
	}
	else
	{
		owner->m_pendingSetChanges.insert(std::make_pair(setId, std::make_pair(sourceid, leveldiff)));
	}
}


void CWeenieObject::GiveSharedLum(int64_t amount, bool showText)
{
	if (amount <= 0)
		return;

	EnchantedQualityDetails buffDetails;
	GetFloatEnchantmentDetails(GLOBAL_XP_MOD_FLOAT, 0.0, &buffDetails);

	if (buffDetails.enchantedValue > 0.0)
		amount *= buffDetails.valueIncreasingMultiplier;

	fellowship_ptr_t f = GetFellowship();

	if (f)
		f->GiveLum(this, amount, showText);
	else
		GiveLum(amount, showText);
}

void CWeenieObject::GiveLum(int64_t amount, bool showText)
{
	if (amount <= 0)
		return;

	if (m_Qualities.GetInt(AUGMENTATION_BONUS_XP_INT, 0))
	{
		amount = uint64_t((double)amount * 1.05);
	}

	uint64_t AvailableLum = (uint64_t)InqInt64Quality(AVAILABLE_LUMINANCE_INT64, 0);
	uint64_t MaxLum = (uint64_t)InqInt64Quality(MAXIMUM_LUMINANCE_INT64, 0);
	uint64_t newAvailableLum = (uint64_t)InqInt64Quality(AVAILABLE_LUMINANCE_INT64, 0) + amount;

	if (AvailableLum < MaxLum) // Cant Earn Lum without flagging.
	{
		m_Qualities.SetInt64(AVAILABLE_LUMINANCE_INT64, min(newAvailableLum, (uint64_t)1000000)); // don't go over 1 million.
		NotifyInt64StatUpdated(AVAILABLE_LUMINANCE_INT64);
	}

	if (showText)
	{
		const char *notice = csprintf(
			"You've earned %s luminance.",
			FormatNumberString(amount).c_str());

		SendText(notice, LTT_DEFAULT);
	}
}

void CWeenieObject::TryToUnloadAllegianceXP(bool bShowText)
{
	AllegianceTreeNode *node = g_pAllegianceManager->GetTreeNode(GetID());
	if (node)
	{
		if (node->_cp_pool_to_unload > 0)
		{
			GiveXP(node->_cp_pool_to_unload, ExperienceHandlingType::NoHandling, false);

			if (bShowText)
			{
				// this was from logging in
				SendText(csprintf("Your Vassals have produced experience points for you. Taking your skills as a leader into account, you gain %s xp.",
					FormatNumberString(node->_cp_pool_to_unload).c_str()), LTT_DEFAULT);
			}

			node->_cp_pool_to_unload = 0;
		}
	}
}

uint32_t CWeenieObject::GiveAttributeXP(STypeAttribute key, uint32_t amount)
{
	Attribute attribute;
	m_Qualities.InqAttribute(key, attribute);
	attribute._cp_spent += amount;

	uint32_t oldLevel = attribute._level_from_cp;
	attribute._level_from_cp = ExperienceSystem::AttributeLevelFromExperience(attribute._cp_spent);

	m_Qualities.SetAttribute(key, attribute);

	uint32_t raised = attribute._level_from_cp - oldLevel;
	if (raised)
	{
		uint32_t newLevel;
		if (m_Qualities.InqAttribute(key, newLevel, TRUE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);
			if (attribute._level_from_cp == 190) {
				SendText(csprintf("Your base %s is now %u and has reached its upper limit!", Attribute::GetAttributeName(key), newLevel), LTT_ADVANCEMENT);
				EmitEffect(PS_WeddingBliss, 1.0f);
			}
			else
				SendText(csprintf("Your base %s is now %u!", Attribute::GetAttributeName(key), newLevel), LTT_ADVANCEMENT);
		}
	}
	NotifyAttributeStatUpdated(key);

	return raised;
}

inline const char *GetAttribute2ndName(STypeAttribute2nd key)
{
	switch (key)
	{
	case MAX_HEALTH_ATTRIBUTE_2ND:
	case HEALTH_ATTRIBUTE_2ND:
		return "Health";

	case MAX_STAMINA_ATTRIBUTE_2ND:
	case STAMINA_ATTRIBUTE_2ND:
		return "Stamina";

	case MAX_MANA_ATTRIBUTE_2ND:
	case MANA_ATTRIBUTE_2ND:
		return "Mana";

	default:
		return "";
	}
}

uint32_t CWeenieObject::GiveAttribute2ndXP(STypeAttribute2nd key, uint32_t amount)
{
	SecondaryAttribute attribute2nd;
	m_Qualities.InqAttribute2nd(key, attribute2nd);
	attribute2nd._cp_spent += amount;

	uint32_t oldLevel = attribute2nd._level_from_cp;
	attribute2nd._level_from_cp = ExperienceSystem::Attribute2ndLevelFromExperience(attribute2nd._cp_spent);

	m_Qualities.SetAttribute2nd(key, attribute2nd);

	uint32_t raised = attribute2nd._level_from_cp - oldLevel;
	if (raised)
	{
		uint32_t newLevel;
		if (m_Qualities.InqAttribute2nd(key, newLevel, TRUE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

			if (attribute2nd._level_from_cp == 196) {
				SendText(csprintf("Your base %s is now %u and has reached its upper limit!", GetAttribute2ndName(key), newLevel), LTT_ADVANCEMENT);
				EmitEffect(PS_WeddingBliss, 1.0f);
			}
			else
				SendText(csprintf("Your base %s is now %u!", GetAttribute2ndName(key), newLevel), LTT_ADVANCEMENT);
		}
	}
	NotifyAttribute2ndStatUpdated(key);
	return raised;
}

uint32_t CWeenieObject::GiveSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS sac)
{
	Skill skill;
	m_Qualities.InqSkill(key, skill);

	// before
	uint32_t oldLevel = 0;
	m_Qualities.InqSkill(key, oldLevel, TRUE);

	bool skillSpecByAug = IsSkillAugmented(key);
	if (skillSpecByAug) //If skill has been augmented for sac (tinkering skills), than we should immediately jump to specialized.
	{
		skill._sac = SPECIALIZED_SKILL_ADVANCEMENT_CLASS;
		skill._init_level = 10;
	}
	else
		skill._sac = sac;

	skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);

	// after
	uint32_t newLevel;
	m_Qualities.InqSkill(key, newLevel, TRUE);

	m_Qualities.SetSkill(key, skill);

	EmitSound(Sound_RaiseTrait, 1.0, true);

	std::string skillName = "Unknown";

	if (SkillTable *pSkillTable = SkillSystem::GetSkillTable())
	{
		const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
		if (pSkillBase)
		{
			skillName = pSkillBase->_name;
		}
	}
	if (skillSpecByAug)
		SendText(csprintf("You are now specialized in %s!", skillName.c_str()), LTT_ADVANCEMENT);
	else
		SendText(csprintf("You are now trained in %s!", skillName.c_str()), LTT_ADVANCEMENT);

	uint32_t raised = newLevel - oldLevel;
	if (raised)
	{
		SendText(csprintf("Your base %s is now %u!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
	}

	NotifySkillStatUpdated(key);
	return raised;
}

bool CWeenieObject::IsSkillAugmented(STypeSkill key)
{
	STypeInt augskillint = UNDEF_INT;
	switch (key)
	{
	case ARMOR_APPRAISAL_SKILL:
	{
		augskillint = AUGMENTATION_SPECIALIZE_ARMOR_TINKERING_INT;
		break;
	}
	case ITEM_APPRAISAL_SKILL:
	{
		augskillint = AUGMENTATION_SPECIALIZE_ITEM_TINKERING_INT;
		break;
	}
	case MAGIC_ITEM_APPRAISAL_SKILL:
	{
		augskillint = AUGMENTATION_SPECIALIZE_MAGIC_ITEM_TINKERING_INT;
		break;
	}
	case WEAPON_APPRAISAL_SKILL:
	{
		augskillint = AUGMENTATION_SPECIALIZE_WEAPON_TINKERING_INT;
		break;
	}
	//salvaging is never unspecialized so it isn't needed here as we will never be advancing it again once spec'd.
	default:
		return false;
	}

	if (InqIntQuality(augskillint, 0) == 1)
		return true;
	else
		return false;
}

uint32_t CWeenieObject::GiveSkillXP(STypeSkill key, uint32_t amount, bool silent)
{
	if (amount <= 0)
		return 0;

	Skill skill;
	m_Qualities.InqSkill(key, skill);

	if (skill._sac < TRAINED_SKILL_ADVANCEMENT_CLASS)
		return 0;

	skill._pp += amount;

	uint32_t oldLevel = skill._level_from_pp;
	skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);

	std::string skillName = "Unknown";

	if (SkillTable *pSkillTable = SkillSystem::GetSkillTable())
	{
		const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
		if (pSkillBase)
		{
			skillName = pSkillBase->_name;
		}
	}

	if (!silent)
	{
		const char *notice = csprintf(
			"You've earned %s experience in your %s skill.",
			FormatNumberString(amount).c_str(), skillName.c_str());

		SendText(notice, LTT_ADVANCEMENT);
	}

	//if ((skill._sac == TRAINED_SKILL_ADVANCEMENT_CLASS && oldLevel == 208) || (skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS && oldLevel == 226))
		//return 0; //We already check if the skill is maxed before entering the function.

	m_Qualities.SetSkill(key, skill);

	uint32_t raised = skill._level_from_pp - oldLevel;
	if (raised)
	{
		// Your base Melee Defense is now 123! color: 13

		uint32_t newLevel;
		if (m_Qualities.InqSkill(key, newLevel, TRUE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

			if ((skill._sac == TRAINED_SKILL_ADVANCEMENT_CLASS && skill._level_from_pp == 208) || (skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS && skill._level_from_pp == 226)) {
				SendText(csprintf("Your base %s is now %u and has reached its upper limit!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
				EmitEffect(PS_WeddingBliss, 1.0f);
			}
			else
				SendText(csprintf("Your base %s is now %u!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
		}

	}

	NotifySkillStatUpdated(key);

	if (key == LEADERSHIP_SKILL || key == LOYALTY_SKILL)
	{
		AllegianceTreeNode *self = g_pAllegianceManager->GetTreeNode(GetID());
		if (self)
			self->UpdateWithWeenie(this);
	}

	return raised;
}

uint32_t CWeenieObject::GiveSkillPoints(STypeSkill key, uint32_t amount)
{
	if (amount <= 0)
		return 0;

	Skill skill;
	m_Qualities.InqSkill(key, skill);

	if (skill._sac < TRAINED_SKILL_ADVANCEMENT_CLASS)
		return 0;

	skill._level_from_pp += amount;

	skill._pp = ExperienceSystem::ExperienceToSkillLevel(skill._sac, skill._level_from_pp);

	m_Qualities.SetSkill(key, skill);

	// Your base Melee Defense is now 123! color: 13

	uint32_t newLevel;
	if (m_Qualities.InqSkill(key, newLevel, TRUE))
	{
		std::string skillName = "Unknown";

		if (SkillTable *pSkillTable = SkillSystem::GetSkillTable())
		{
			const SkillBase *pSkillBase = pSkillTable->GetSkillBase(key);
			if (pSkillBase)
			{
				skillName = pSkillBase->_name;
			}
		}

		if (_phys_obj)
			_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

		if ((skill._sac == TRAINED_SKILL_ADVANCEMENT_CLASS && skill._level_from_pp == 208) || (skill._sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS && skill._level_from_pp == 226)) {
			SendText(csprintf("Your base %s is now %u and has reached its upper limit!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
			EmitEffect(PS_WeddingBliss, 1.0f);
		}
		else
			SendText(csprintf("Your base %s is now %u!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
	}

	NotifySkillStatUpdated(key);

	if (key == LEADERSHIP_SKILL || key == LOYALTY_SKILL)
	{
		AllegianceTreeNode *self = g_pAllegianceManager->GetTreeNode(GetID());
		if (self)
			self->UpdateWithWeenie(this);
	}

	return amount;
}

void CWeenieObject::GiveSkillCredit(int amount)
{
	if (amount > 0)
	{
		int newAmount = (int)(GetSkillCredits()) + amount;
		SetAvailSkillsAndNotifyPlayer(newAmount);
		SendText(csprintf("You have gained %d skill %s!", amount, amount == 1 ? "credit" : "credits"), LTT_ADVANCEMENT);
	}
}

void CWeenieObject::SetAvailSkillsAndNotifyPlayer(int amount)
{
	m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, amount);
	NotifyIntStatUpdated(AVAILABLE_SKILL_CREDITS_INT);
}

void CWeenieObject::AdjustSkillCredits(int expected, int current, bool showText)
{
	int amount = expected - current;
	if (amount == 0)
		return;

	std::string creditChange = "earned";
	if (amount < 0)
	{
		creditChange = "lost";
	}

	if (!showText)
		creditChange = "";

	SetAvailSkillsAndNotifyPlayer(max((int)(GetSkillCredits()) + amount, 0));
	SendText(csprintf("You have %s %d skill %s!", creditChange.c_str(), abs(amount), amount == 1 ? "credit" : "credits"), LTT_ADVANCEMENT);
}

uint32_t CWeenieObject::GetSkillCredits()
{
	uint32_t unassignedCredits = 0;
	m_Qualities.InqInt(AVAILABLE_SKILL_CREDITS_INT, *(int *)&unassignedCredits);
	return unassignedCredits;
}

void CWeenieObject::SendText(const char* szText, int32_t lColor)
{
	SendNetMessage(ServerText(szText, lColor), PRIVATE_MSG, FALSE, TRUE);
}

void CWeenieObject::SendTextToOverlay(const char* szText)
{
	SendNetMessage(OverlayText(szText), PRIVATE_MSG, FALSE, TRUE);
}

float Calc_BurdenMod(float flBurden)
{
	if (flBurden < 1.0f)
		return 1.0f;

	if (flBurden < 2.0f)
		return (2.0f - flBurden);

	return 0.0f;
}

float Calc_AnimSpeed(uint32_t runSkill, float fBurden)
{
	float fBurdenMod = Calc_BurdenMod(fBurden);
	float fSpeed = (float)runSkill;

	if (fSpeed == 800.0f)
		return (18.0f / 4.0f);

	fSpeed /= (fSpeed + 200.0f);
	fSpeed *= 11.0f * fBurdenMod;
	return ((fSpeed + 4.0f) / 4.0f);
}

float CWeenieObject::GetBurdenPercent()
{
	// should base this off our total burden and strength
	return 0.0f;
}

bool CWeenieObject::InqJumpVelocity(float extent, float &vz)
{
	//Client does this.. but we are the server!
	//if (IsThePlayer())
	//{
	//     etc.
	//}

	return !!m_Qualities.InqJumpVelocity(extent, vz);
}

bool CWeenieObject::InqRunRate(float &rate)
{
	//Client does this.. but we are the server!
	//if (IsThePlayer())
	//{
	//     etc.
	//}

	return !!m_Qualities.InqRunRate(rate);
}

bool CWeenieObject::CanJump(float extent)
{
	//Client does this.. but we are the server!
	//if (IsThePlayer())
	//{
	//     etc.
	//}

	return !!m_Qualities.CanJump(extent);
}

bool CWeenieObject::JumpStaminaCost(float extent, int32_t &cost)
{
	// Client does this..but we are the server!
	//if (IsThePlayer())
	//{
	//     etc.
	//}

	return !!m_Qualities.JumpStaminaCost(extent, cost);
}

void CWeenieObject::EmitEffect(uint32_t effect, float mod)
{
	if (_phys_obj)
		_phys_obj->EmitEffect(effect, mod);
}

bool CWeenieObject::TeleportToLifestone()
{
	Position lifestone;
	if (m_Qualities.InqPosition(SANCTUARY_POSITION, lifestone) && lifestone.objcell_id)
	{
		Movement_Teleport(lifestone, true);
		return true;
	}

	return false;
}

bool CWeenieObject::TeleportToHouse()
{
	if (CPlayerWeenie *player = AsPlayer())
	{
		uint32_t houseId = player->GetAccountHouseId();
		if (houseId)
		{
			CHouseData *houseData = g_pHouseManager->GetHouseData(houseId);
			if (houseData->_ownerAccount == player->GetClient()->GetAccountInfo().id)
			{
				if (houseData->_position.objcell_id)
				{
					Movement_Teleport(houseData->_position, false);
					return true;
				}
			}
		}
	}
	return false;
}

bool CWeenieObject::TeleportToMansion()
{
	AllegianceTreeNode *allegianceNode = g_pAllegianceManager->GetTreeNode(GetID());

	if (!allegianceNode)
		return false;

	uint32_t allegianceHouseId;

	CWeenieObject *monarch = g_pWorld->FindObject(allegianceNode->_monarchID);
	if (!monarch)
	{
		monarch = CWeenieObject::Load(allegianceNode->_monarchID);
		if (!monarch)
			return false;
		allegianceHouseId = monarch->InqDIDQuality(HOUSEID_DID, 0);
		delete monarch;
	}
	else
		allegianceHouseId = monarch->InqDIDQuality(HOUSEID_DID, 0);

	if (allegianceHouseId)
	{
		CHouseData *houseData = g_pHouseManager->GetHouseData(allegianceHouseId);
		if (houseData && houseData->_ownerId == allegianceNode->_monarchID && (houseData->_houseType == Villa_HouseType || houseData->_houseType == Mansion_HouseType)) //2 = villa, 3 = mansion
		{
			if (houseData->_position.objcell_id)
			{
				Movement_Teleport(houseData->_position, false);
				return true;
			}
		}
	}
	return false;
}

bool CWeenieObject::TeleportToAllegianceHometown()
{
	AllegianceTreeNode *allegianceNode = g_pAllegianceManager->GetTreeNode(GetID());

	if (!allegianceNode)
		return false;
	AllegianceInfo *allegianceInfo = g_pAllegianceManager->GetInfo(allegianceNode->_monarchID);

	if (allegianceInfo && allegianceInfo->_info.m_BindPoint.objcell_id)
	{
		Movement_Teleport(allegianceInfo->_info.m_BindPoint, false);
		return true;
	}
	return false;
}

void CWeenieObject::SendUseMessage(CWeenieObject *other, unsigned int channel)
{
	std::string useMessage;
	if (m_Qualities.InqString(USE_MESSAGE_STRING, useMessage))
	{
		other->SendNetMessage(ServerText(useMessage.c_str(), channel), PRIVATE_MSG, FALSE, TRUE);
	}
}

void CWeenieObject::ExecuteUseEvent(CUseEventData *useEvent)
{
	MakeUseManager();

	bool busy = false;
	int error = WERROR_NONE;

	CWeenieObject *target = g_pWorld->FindObject(useEvent->_target_id);

	if (IsBusyOrInAction())
	{
		busy = true;
		error = WERROR_ACTIONS_LOCKED;
	}

	if (target && target->m_EmoteManager && target->m_EmoteManager->HasQueue())
	{
		busy = true;
		error = WERROR_EXTERNAL_ACTIONS_LOCKED;
		SendText(csprintf("%s is busy.", target->GetName().c_str()), LTT_DEFAULT);
	}

	if (busy)
	{
		NotifyUseDone(error);
		if (useEvent->_give_event)
			NotifyInventoryFailedEvent(useEvent->_source_item_id, WERROR_NONE);

		delete useEvent;
		return;
	}

	useEvent->_manager = m_UseManager;
	useEvent->_weenie = this;

	m_UseManager->BeginUse(useEvent);
}

void CWeenieObject::ExecuteAttackEvent(CAttackEventData *attackEvent)
{
	if (m_AttackManager && m_AttackManager->IsAttacking())
	{
		// NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		return;
	}

	if (IsBusyOrInAction())
	{
		NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		NotifyAttackDone();
		return;
	}

	if (!m_AttackManager)
	{
		m_AttackManager = new AttackManager(this);
	}

	attackEvent->_manager = m_AttackManager;
	attackEvent->_weenie = this;

	m_AttackManager->BeginAttack(attackEvent);
}

const Position &CWeenieObject::GetPosition()
{
	/*
	if (!_phys_obj)
	{
	static Position emptyPosition;
	return emptyPosition;
	}

	return _phys_obj->m_Position;
	*/

	return m_Position;
}

void CWeenieObject::CheckForTickingDots()
{
	if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->_add_list && m_Qualities._enchantment_reg->_add_list->size() > 0)
	{
		CWeenieObject* dfSource = nullptr;
		CWeenieObject* voidSource = nullptr;
		std::vector<std::string> dotspellNames;
		std::vector<std::string> voidspellNames;
		for (auto it = m_Qualities._enchantment_reg->_add_list->begin(); it != m_Qualities._enchantment_reg->_add_list->end(); ++it)
		{
			CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
			const CSpellBase *pSpellBase = pSpellTable->GetSpellBase(it->_id);
			if (pSpellBase && (pSpellBase->_bitfield & DamageOverTime_SpellIndex))
			{
				if (it->_spell_category == DF_Bleed_Damage_SpellCategory)
				{
					dfSource = g_pWorld->FindObject(it->_caster);
					if (g_pConfig->ShowDotSpells())
						dotspellNames.push_back(pSpellBase->_name);
				}
				else if (it->_spell_category == NetherDamageOverTime_Raising_SpellCategory || it->_spell_category == NetherDamageOverTime_Raising2_SpellCategory ||
					it->_spell_category == NetherDamageOverTime_Raising3_SpellCategory)
				{
					voidSource = g_pWorld->FindObject(it->_caster);
					if (g_pConfig->ShowDotSpells())
						voidspellNames.push_back(pSpellBase->_name);
				}
			}
			if(pSpellBase && it->_spell_category == AetheriaProcDamageOverTime_Raising_SpellCategory)
			{
				if (g_pConfig->ShowDotSpells())
				{
					dfSource = g_pWorld->FindObject(it->_caster);
					if (dfSource)
					{
						CWeenieObject* owner = dfSource->GetWorldTopLevelOwner();
						dotspellNames.push_back(pSpellBase->_name);
					}
				}
			}
		}

		int dfValue = 0;
		//if (dfValue > 0 && AsPlayer())
		//	AsPlayer()->SendText(csprintf("DOT: %u", dfValue), LTT_COMBAT);
		if (m_Qualities.EnchantInt(DAMAGE_OVER_TIME_INT, dfValue, false))
		{
			DamageEventData dfDamage;
			dfDamage.baseDamage = (double)dfValue;
			dfDamage.damage_type = BASE_DAMAGE_TYPE;
			dfDamage.source = dfSource;
			dfDamage.target = this;
			dfDamage.outputDamageFinal = dfValue;
			TakeDamage(dfDamage);
			_phys_obj->EmitEffect(PS_DirtyFightingDamageOverTime, 1.0);
			if (!dfDamage.killingBlow && AsPlayer())
			{
				if (g_pConfig->ShowDotSpells())
				{
					std::string s;
					bool first = true;
					for (const auto &sname : dotspellNames)
					{
						if (!first)
							s += " and ";
						else
							first = false;
						s += sname;
					}
					AsPlayer()->SendText(csprintf("You take %u points of periodic damage from %s.", dfValue, s.c_str()), LTT_COMBAT);
				}
				else
				{
					AsPlayer()->SendText(csprintf("You take %u points of periodic damage.", dfValue), LTT_COMBAT);
				}
			}
		}

		int voidDotValue = 0;
		//if (voidDotValue > 0 && AsPlayer())
		//	AsPlayer()->SendText(csprintf("NOT: %u", voidDotValue), LTT_COMBAT);
		if (m_Qualities.EnchantInt(NETHER_OVER_TIME_INT, voidDotValue, false))
		{
			DamageEventData vdDamage;
			vdDamage.baseDamage = (double)voidDotValue;
			vdDamage.damage_type = NETHER_DAMAGE_TYPE;
			vdDamage.source = voidSource;
			vdDamage.isDot = true;
			vdDamage.target = this;
			vdDamage.outputDamageFinal = voidDotValue;
			TakeDamage(vdDamage);
			_phys_obj->EmitEffect(PS_HealthDownVoid, 1.0);
			if (!vdDamage.killingBlow && AsPlayer())
			{
				if (g_pConfig->ShowDotSpells())
				{
					std::string s;
					bool first = true;
					for (const auto &sname : voidspellNames)
					{
						if (!first)
							s += " and ";
						else
							first = false;
						s += sname;
					}
					AsPlayer()->SendText(csprintf("You take %u points of periodic nether damage from %s.", (uint32_t)((double)voidDotValue * g_pConfig->VoidDamageReduction()), s.c_str()), LTT_COMBAT);
				}
				else
				{
					AsPlayer()->SendText(csprintf("You take %u points of periodic nether damage.", (uint32_t)((double)voidDotValue * g_pConfig->VoidDamageReduction())), LTT_COMBAT);
				}
			}
		}
	}
}

void CWeenieObject::CheckForTickingHots()
{
	if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->_add_list && m_Qualities._enchantment_reg->_add_list->size() > 0)
	{
		for (auto it = m_Qualities._enchantment_reg->_add_list->begin(); it != m_Qualities._enchantment_reg->_add_list->end(); ++it)
		{
			CSpellTable *pSpellTable = MagicSystem::GetSpellTable();
			const CSpellBase *pSpellBase = pSpellTable->GetSpellBase(it->_id);
			if (pSpellBase && ((pSpellBase->_category == HealOverTime_Raising_SpellCategory) || (pSpellBase->_category == AetheriaProcHealthOverTime_Raising_SpellCategory)))
			{
				int hValue = 0;
				if (m_Qualities.EnchantInt(HEAL_OVER_TIME_INT, hValue, false))
				{
					AdjustHealth(hValue, false);
				}
			}
		}
	}
}

void CWeenieObject::CheckForExpiredEnchantments()
{
	if (m_Qualities._enchantment_reg)
	{
		PackableListWithJson<uint32_t> expired;
		m_Qualities._enchantment_reg->GetExpiredEnchantments(&expired);

		if (expired.size())
		{
			m_Qualities._enchantment_reg->RemoveEnchantments(&expired);

			if (AsPlayer())
			{
				BinaryWriter expireMessage;
				expireMessage.Write<uint32_t>(0x2C5);
				expired.Pack(&expireMessage);

				SendNetMessage(&expireMessage, PRIVATE_MSG, TRUE, FALSE);
				EmitSound(Sound_SpellExpire, 1.0f, true);
			}
			else if (CWeenieObject *topMostOwner = GetWorldTopLevelOwner())
			{
				if (topMostOwner->AsPlayer())
				{
					for (auto spellExpired : expired)
					{
						const CSpellBase *spellBase = MagicSystem::GetSpellTable()->_spellBaseHash.lookup(spellExpired);
						if (spellBase)
							topMostOwner->SendText(csprintf("The spell %s on %s has expired.", spellBase->_name.c_str(), GetName().c_str()), LTT_MAGIC);
					}
					topMostOwner->EmitSound(Sound_SpellExpire, 1.0f, true);
				}
			}
	
			CheckVitalRanges();
		}
	}
}

void CWeenieObject::WieldedTick()
{
	InventoryTick();

	CWeenieObject *wielder = GetWorldWielder();
	if (!wielder)
		return;

	if (_nextManaUse >= 0 && _nextManaUse <= Timer::cur_time)
	{
		double manaRate = 0.0f;
		int currentMana = 0;
		if (m_Qualities.InqFloat(MANA_RATE_FLOAT, manaRate, TRUE) && manaRate != 0.0 && m_Qualities.InqInt(ITEM_CUR_MANA_INT, currentMana, TRUE, FALSE))
		{
			bool hasActiveSpells = false;

			if (m_Qualities._enchantment_reg)
			{
				if (m_Qualities._enchantment_reg->_add_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_add_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}

				if (!hasActiveSpells && m_Qualities._enchantment_reg->_mult_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_mult_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}

				if (!hasActiveSpells && m_Qualities._enchantment_reg->_cooldown_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_cooldown_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}
			}

			if (wielder->m_Qualities._enchantment_reg)
			{
				if (wielder->m_Qualities._enchantment_reg->_add_list)
				{
					for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_add_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}

				if (!hasActiveSpells && wielder->m_Qualities._enchantment_reg->_mult_list)
				{
					for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_mult_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}

				if (!hasActiveSpells && wielder->m_Qualities._enchantment_reg->_cooldown_list)
				{
					for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_cooldown_list)
					{
						if (entry._caster == GetID())
						{
							hasActiveSpells = true;
							break;
						}
					}
				}
			}

			if (hasActiveSpells)
			{
				if (currentMana > 0)
				{
					currentMana--;
					m_Qualities.SetInt(ITEM_CUR_MANA_INT, currentMana);

					if (currentMana == 2)
					{
						if (wielder->AsPlayer())
							wielder->SendText(csprintf("The %s will run out of mana soon.", GetName().c_str()), LTT_MAGIC); //todo: made up message, confirm if it's correct
					}

					if (wielder->AsPlayer())
						manaRate *= wielder->m_Qualities.GetNegativeRating(LUM_AUG_ITEM_MANA_USAGE_INT, false);

					_nextManaUse = Timer::cur_time + (1 / -manaRate);
				}
				else
				{
					m_Qualities.SetInt(ITEM_CUR_MANA_INT, 0);
					_nextManaUse = -1.0;

					if (wielder != NULL)
					{
						if (wielder->AsPlayer())
						{
							wielder->SendText(csprintf("The %s has run out of mana.", GetName().c_str()), LTT_MAGIC); //todo: made up message, confirm if it's correct
							wielder->EmitSound(Sound_ItemManaDepleted, 1.0, true);
						}

						// remove any enchantments
						if (wielder->m_Qualities._enchantment_reg)
						{
							PackableListWithJson<uint32_t> spells_to_remove;

							if (wielder->m_Qualities._enchantment_reg->_add_list)
							{
								for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_add_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (wielder->m_Qualities._enchantment_reg->_mult_list)
							{
								for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_mult_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (wielder->m_Qualities._enchantment_reg->_cooldown_list)
							{
								for (const auto &entry : *wielder->m_Qualities._enchantment_reg->_cooldown_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (!spells_to_remove.empty())
							{
								wielder->m_Qualities._enchantment_reg->RemoveEnchantments(&spells_to_remove);

								if (wielder->AsPlayer())
								{
									BinaryWriter silentRemoveMessage;
									silentRemoveMessage.Write<uint32_t>(0x2C8);
									spells_to_remove.Pack(&silentRemoveMessage);

									wielder->SendNetMessage(&silentRemoveMessage, PRIVATE_MSG, TRUE, FALSE);
								}

								wielder->CheckVitalRanges();
							}
						}

						if (m_Qualities._enchantment_reg)
						{
							PackableListWithJson<uint32_t> spells_to_remove;

							if (m_Qualities._enchantment_reg->_add_list)
							{
								for (const auto &entry : *m_Qualities._enchantment_reg->_add_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (m_Qualities._enchantment_reg->_mult_list)
							{
								for (const auto &entry : *m_Qualities._enchantment_reg->_mult_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (m_Qualities._enchantment_reg->_cooldown_list)
							{
								for (const auto &entry : *m_Qualities._enchantment_reg->_cooldown_list)
								{
									if (entry._caster == GetID())
									{
										spells_to_remove.push_back(entry._id);
									}
								}
							}

							if (!spells_to_remove.empty())
							{
								m_Qualities._enchantment_reg->RemoveEnchantments(&spells_to_remove);

								CheckVitalRanges();
							}
						}
					}
				}
			}
		}
		else
			_nextManaUse = -1.0;
	}
}

void CWeenieObject::InventoryTick()
{
	if (m_EmoteManager)
		m_EmoteManager->Tick();

	CheckForExpiredEnchantments();

	Tick();
}

void CWeenieObject::Tick()
{
	if (m_AttackManager)
		m_AttackManager->Update();

	if (m_SpellcastingManager)
		m_SpellcastingManager->Update();

	if (m_EmoteManager)
		m_EmoteManager->Tick();

	if (m_UseManager)
		m_UseManager->Update();

	if (AsPlayer() && IsMovingTo(MovementTypes::MoveToObject))
	{
		CWeenieObject *target = g_pWorld->FindWithinPVS(this, movement_manager->moveto_manager->top_level_object_id);
		if (!target)
			movement_manager->CancelMoveTo(WERROR_OBJECT_GONE);
	}

	if (AsPlayer())
	{
		int allegUpdate;
		m_Qualities.InqBool(ALLEGIANCE_UPDATE_REQUEST_BOOL, allegUpdate);
		if (allegUpdate)
		{
			g_pAllegianceManager->SendAllegianceProfile(this->AsPlayer());
			m_Qualities.SetBool(ALLEGIANCE_UPDATE_REQUEST_BOOL, false);
		}
	}

	UpdateGenerator();

	if (_nextHeartBeat != -1.0 && _nextHeartBeat <= Timer::cur_time)
	{
		if (!IsDead() && !IsInPortalSpace())
		{
			if (IsCompletelyIdle()) // Don't perform idle emotes unless you are completely idle, but allow vital regeneration.
			{
				if (_nextHeartBeatEmote != -1.0 && _nextHeartBeatEmote <= Timer::cur_time)
				{
					_nextHeartBeatEmote = Timer::cur_time + Random::GenUInt(2, 15); //add a little variation to avoid synchronization.

					//_last_update_pos is the time of the last attack/movement/action, basically we don't want to do heartBeat emotes if we're active.
					if (Timer::cur_time > _last_update_pos + 30.0 && m_Qualities._emote_table)
					{
						PackableList<EmoteSet> *emoteSetList = m_Qualities._emote_table->_emote_table.lookup(HeartBeat_EmoteCategory);

						if (emoteSetList)
						{
							double dice = Random::GenFloat(0.0, 1.0);

							for (auto &emoteSet : *emoteSetList)
							{
								if (dice < emoteSet.probability)
								{
									if (movement_manager && movement_manager->motion_interpreter)
									{
										if (emoteSet.style == movement_manager->motion_interpreter->interpreted_state.current_style &&
											emoteSet.substyle == movement_manager->motion_interpreter->interpreted_state.forward_command &&
											!movement_manager->motion_interpreter->interpreted_state.turn_command &&
											!movement_manager->motion_interpreter->interpreted_state.sidestep_command)
										{
											MakeEmoteManager()->ExecuteEmoteSet(emoteSet, 0);
										}
									}

									break;
								}
							}
						}
					}
				}
			}
			else
				_nextHeartBeatEmote = Timer::cur_time + 30.0;

			CheckRegeneration(InqFloatQuality(HEALTH_RATE_FLOAT, 0.0), HEALTH_ATTRIBUTE_2ND, MAX_HEALTH_ATTRIBUTE_2ND);
			CheckRegeneration(InqFloatQuality(STAMINA_RATE_FLOAT, 0.0), STAMINA_ATTRIBUTE_2ND, MAX_STAMINA_ATTRIBUTE_2ND);
			CheckRegeneration(InqFloatQuality(MANA_RATE_FLOAT, 0.0), MANA_ATTRIBUTE_2ND, MAX_MANA_ATTRIBUTE_2ND);


		}
		CheckForTickingDots();
		CheckForTickingHots();
		CheckForExpiredEnchantments();

		bool serverGagCheck = false;
		int isSGagged = 0;
		if (m_Qualities.InqBool(IS_GAGGED_BOOL, isSGagged) && isSGagged && !serverGagCheck)
		{
			double gagDuration;
			if (m_Qualities.InqFloat(GAG_DURATION_FLOAT, gagDuration) && gagDuration > 0)
			{
				double gagTS;
				if (m_Qualities.InqFloat(GAG_TIMESTAMP_FLOAT, gagTS))
				{
					double currenTime = Time::GetTimeCurrent();
					if (currenTime > (gagTS + gagDuration))
					{
						serverGagCheck = true;
						g_pDBIO->RemoveServerGag(GetID());
						m_Qualities.SetBool(IS_GAGGED_BOOL, false);
					}
				}
			}
		}

		bool allegGagCheck = false;
		int isAGagged = 0;
		if (m_Qualities.InqBool(IS_ALLEGIANCE_GAGGED_BOOL, isAGagged) && isAGagged && !allegGagCheck)
		{
			double gagDuration;
			if (m_Qualities.InqFloat(ALLEGIANCE_GAG_DURATION_FLOAT, gagDuration) && gagDuration > 0)
			{
				double gagTS;
				if (m_Qualities.InqFloat(ALLEGIANCE_GAG_TIMESTAMP_FLOAT, gagTS))
				{
					double currenTime = Time::GetTimeCurrent();
					if (currenTime > (gagTS + gagDuration))
					{
						allegGagCheck = true;
						m_Qualities.SetBool(IS_ALLEGIANCE_GAGGED_BOOL, false);
					}
				}
			}
		}

		int lifespan = 0;
		if (m_Qualities.InqInt(LIFESPAN_INT, lifespan))
		{
			if (lifespan > 0)
			{
				int lsRemain = 0;
				if (m_Qualities.InqInt(REMAINING_LIFESPAN_INT, lsRemain) && lsRemain > 0)
				{
					m_Qualities.AdjInt(REMAINING_LIFESPAN_INT, -m_Qualities.GetFloat(HEARTBEAT_INTERVAL_FLOAT, DEFAULT_HEARTBEAT_INTERVAL));
					NotifyIntStatUpdated(REMAINING_LIFESPAN_INT);
				}
				else
				{
					CWeenieObject *owner = GetWorldTopLevelOwner();

					if (owner)
					{
						if (IsWielded())
						{
							if (owner->id == GetWielderID())
							{
								owner->DoForcedStopCompletely();
								owner->ChangeCombatMode(NONCOMBAT_COMBAT_MODE, false);
							}
						}
						Remove();
						owner->RecalculateEncumbrance();
						owner->SendText(csprintf("Its lifespan finished, your %s crumbles to dust.", GetName().c_str()), LTT_DEFAULT);
						if ((AsClothing() || IsWielded()) && m_bWorldIsAware)
							owner->UpdateModel();
					}
				}
			}
		}


		_nextHeartBeat = Timer::cur_time + m_Qualities.GetFloat(HEARTBEAT_INTERVAL_FLOAT, DEFAULT_HEARTBEAT_INTERVAL);

		m_Qualities.SetFloat(HEARTBEAT_TIMESTAMP_FLOAT, Timer::cur_time);
	}

	if (_nextReset >= 0)
	{
		if (_nextReset <= Timer::cur_time)
		{
			ResetToInitialState();
			_nextReset = -1.0;
		}
	}

	if (_timeToRot >= 0 && _timeToRot <= Timer::cur_time)
	{
		// Allow items to rot if they have a predetermined lifespan.
		if (!HasOwner() || m_Qualities.GetInt(REMAINING_LIFESPAN_INT, 0) <= 0)
		{
			if (_beganRot)
			{
				if ((_timeToRot + 2.0) <= Timer::cur_time)
				{
					MarkForDestroy();
				}
			}
			else
			{
				if(!IsContained())
					EmitEffect(PS_Destroy, 1.0f);
				_beganRot = true;
			}
		}
	}

	

	TaskRun();
}

void CWeenieObject::CheckRegeneration(double rate, STypeAttribute2nd currentAttrib, STypeAttribute2nd maxAttrib)
{
	if (rate == 0.0)
		return;

	if (AsPlayer()) //only players have natural regeneration bonuses
	{
		//some combination of strength and endurance(with endurance being more important) allows one to regenerate hit points at a 
		//faster rate the higher one's endurance is. This bonus is in addition to any regeneration spells one may have placed upon themselves. 
		//This regeneration bonus caps at around 110%. 
		if (currentAttrib == HEALTH_ATTRIBUTE_2ND)
		{
			uint32_t strength = 0;
			uint32_t endurance = 0;
			m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + (endurance * 2));
			float regenMod = 1.0 + (0.0494 * pow(strAndEnd, 1.179) / 100.0f); //formula deduced from values present in the client pdb.
			regenMod = min(max(regenMod, 1.0f), 2.1f);

			rate *= regenMod;
		}

		if (currentAttrib == HEALTH_ATTRIBUTE_2ND || currentAttrib == STAMINA_ATTRIBUTE_2ND)
		{
			if (get_minterp()->interpreted_state.current_style == Motion_NonCombat)
			{
				switch (get_minterp()->interpreted_state.forward_command)
				{
				default:
					rate *= 1; //out of combat regen
					break;
				case Motion_Crouch:
					rate *= 2;
					break;
				case Motion_Sitting:
					rate *= 2.5;
					break;
				case Motion_Sleeping:
					rate *= 3;

					if (m_Qualities.GetInt(AUGMENTATION_FASTER_REGEN_INT, 0))
						rate *= 2;

					break;
				}
			}
			else
				rate *= 0.5; //in combat regen
		}
	}
	else if (AsMonster() && !InqBoolQuality(AI_IMMOBILE_BOOL, FALSE))
	{
		// boosted out of combat regen
		// avoid if the AI is immobile (doors, crystals, etc)
		if (get_minterp()->interpreted_state.current_style == Motion_NonCombat)
		{
			rate *= 100;
		}
	}

	switch (currentAttrib)
	{
	case HEALTH_ATTRIBUTE_2ND:
		rate += accumulatedHealthRegen;
		break;
	case STAMINA_ATTRIBUTE_2ND:
		rate += accumulatedStaminaRegen;
		break;
	case MANA_ATTRIBUTE_2ND:
		rate += accumulatedManaRegen;
		break;
	}

	int regenAmount = floor(rate);
	double accumulated = rate - regenAmount;

	switch (currentAttrib)
	{
	case HEALTH_ATTRIBUTE_2ND:
		accumulatedHealthRegen = accumulated;
		break;
	case STAMINA_ATTRIBUTE_2ND:
		accumulatedStaminaRegen = accumulated;
		break;
	case MANA_ATTRIBUTE_2ND:
		accumulatedManaRegen = accumulated;
		break;
	}

	if (regenAmount > 0)
	{
		uint32_t currentVital = 0, maxVital = 0;
		if (m_Qualities.InqAttribute2nd(currentAttrib, currentVital, FALSE) && m_Qualities.InqAttribute2nd(maxAttrib, maxVital, FALSE))
		{
			if (currentVital < maxVital)
			{
				currentVital += regenAmount;

				//currentVital += (uint32_t)(ceil(rate) + F_EPSILON); //previously

				if (currentVital > maxVital)
				{
					currentVital = maxVital;
				}

				OnRegen(currentAttrib, currentVital);
			}
		}
	}
}

void CWeenieObject::SetupWeenie()
{
	//// correct the icon
	//uint32_t clothing_table_id = InqDIDQuality(CLOTHINGBASE_DID, 0);
	//uint32_t palette_template_key = InqIntQuality(PALETTE_TEMPLATE_INT, 0);

	//if (clothing_table_id && palette_template_key)
	//{
	//	if (!InqBoolQuality(IGNORE_CLO_ICONS_BOOL, FALSE))
	//	{
	//		if (ClothingTable *clothingTable = ClothingTable::Get(clothing_table_id))
	//		{
	//			if (const CloPaletteTemplate *pPaletteTemplate = clothingTable->_paletteTemplatesHash.lookup(palette_template_key))
	//			{
	//				if (pPaletteTemplate->iconID)
	//				{
	//					SetIcon(pPaletteTemplate->iconID);
	//				}
	//			}

	//			ClothingTable::Release(clothingTable);
	//		}
	//	}
	//}
}

void CWeenieObject::PostSpawn()
{
	if (g_pConfig->EverythingUnlocked())
	{
		SetLocked(FALSE);
	}

	double heartbeatInterval;
	if (m_Qualities.InqFloat(HEARTBEAT_INTERVAL_FLOAT, heartbeatInterval, TRUE))
		_nextHeartBeat = Timer::cur_time + heartbeatInterval;
	else
	{
		m_Qualities.SetFloat(HEARTBEAT_INTERVAL_FLOAT, DEFAULT_HEARTBEAT_INTERVAL);
		NotifyFloatStatUpdated(HEARTBEAT_INTERVAL_FLOAT);
		_nextHeartBeat = Timer::cur_time + DEFAULT_HEARTBEAT_INTERVAL;
	}

	if (m_Qualities._emote_table && m_Qualities._emote_table->_emote_table.lookup(HeartBeat_EmoteCategory))
		_nextHeartBeatEmote = Timer::cur_time + Random::GenUInt(2, 15); //We have heartBeat emotes so schedule them.
}

bool CWeenieObject::IsStorage()
{
	return (m_Qualities.id == W_STORAGE_CLASS) ? true : false; // "STORAGE"
}



bool CWeenieObject::CanPickup()
{
	if (IsStuck())
		return false;
	if (IsCreature())
		return false;

	return true;
}

BinaryWriter *CWeenieObject::CreateMessage()
{
	m_bWorldIsAware = true;
	return CreateObject(this);
}

BinaryWriter *CWeenieObject::UpdateMessage()
{
	return UpdateObject(this);
}

void CWeenieObject::RemovePreviousInstance()
{
	if (_instance_timestamp == 1) // not ideal, but a time saver for now
		return;

	BinaryWriter message;

	message.Write<uint32_t>(0xF747);
	message.Write<uint32_t>(GetID());
	message.Write<uint32_t>(_instance_timestamp - 1);

	g_pWorld->BroadcastPVS(this, message.GetData(), message.GetSize());
}

void CWeenieObject::UpdateModel()
{
	if (parent)
		return;

	BinaryWriter MU;

	MU.Write<uint32_t>(0xF625);
	MU.Write<uint32_t>(GetID());

	ObjDesc objDesc;
	GetObjDesc(objDesc);
	objDesc.Pack(&MU);

	MU.Write<WORD>(_instance_timestamp);
	MU.Write<WORD>(++_objdesc_timestamp);

	g_pWorld->BroadcastPVS(this, MU.GetData(), MU.GetSize(), OBJECT_MSG, false, false, true);
}

void CWeenieObject::GetObjDesc(ObjDesc &objDesc)
{
	// Generate 
	objDesc.Wipe();

	objDesc = m_ObjDescOverride;

	//if (m_bObjDescOverride)
	//{
	//	objDesc = m_ObjDescOverride;
	//	return;
	//}

	uint32_t basePaletteID;

	if (m_Qualities.InqDataID(PALETTE_BASE_DID, basePaletteID))
		objDesc.paletteID = basePaletteID;
	else
		objDesc.paletteID = 0x0400007E;

	if (objDesc.paletteID == 0x04000B75)
		objDesc.paletteID = 0x0400007E; // shadows are messed up

	uint32_t clothingBaseID;

	if (m_Qualities.InqDataID(CLOTHINGBASE_DID, clothingBaseID))
	{
		ClothingTable *clothingTable = ClothingTable::Get(clothingBaseID);

		if (clothingTable)
		{
			ShadePackage shades(InqFloatQuality(SHADE_FLOAT, 0.0));

			double shade;
			if (m_Qualities.InqFloat(SHADE2_FLOAT, shade))
				shades._val[1] = shade;
			if (m_Qualities.InqFloat(SHADE3_FLOAT, shade))
				shades._val[2] = shade;
			if (m_Qualities.InqFloat(SHADE4_FLOAT, shade))
				shades._val[3] = shade;



			uint32_t paletteTemplateID = InqIntQuality(PALETTE_TEMPLATE_INT, 0);
			uint32_t iconId = 0;
			
			clothingTable->BuildObjDesc(GetSetupID(), paletteTemplateID, &shades, &objDesc, &iconId);

			// correct the icon
			if (!InqBoolQuality(IGNORE_CLO_ICONS_BOOL, FALSE) && iconId)
			{
				SetIcon(iconId);
			}

			ClothingTable::Release(clothingTable);
		}
	}
	else
	{
		// if not using a clothing base, start objDesc from model itself
		// TODO: fix this later, quick fix to handle unequipping resetting the model parts
		if (_IsPlayer() && part_array && part_array->setup && part_array->setup->parts)
		{
			for (uint32_t i = 0; i < part_array->setup->num_parts; i++)
			{
				objDesc.AddAnimPartChange(new AnimPartChange(i, part_array->setup->parts[i]));
			}
		}
	}
}

void CWeenieObject::SetMotionTableID(uint32_t motion_table_did)
{
	m_Qualities.SetDataID(MOTION_TABLE_DID, motion_table_did);
}

void CWeenieObject::SetSetupID(uint32_t setup_did)
{
	m_Qualities.SetDataID(SETUP_DID, setup_did);
}

void CWeenieObject::SetSoundTableID(uint32_t sound_table_did)
{
	m_Qualities.SetDataID(SOUND_TABLE_DID, sound_table_did);
}

void CWeenieObject::SetPETableID(uint32_t pe_table_did)
{
	m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, pe_table_did);
}

void CWeenieObject::SetScale(float value)
{
	m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, value);
}

void CWeenieObject::SetIcon(uint32_t icon_did)
{
	m_Qualities.SetDataID(ICON_DID, icon_did);
}

void CWeenieObject::SetItemType(ITEM_TYPE type)
{
	m_Qualities.SetInt(ITEM_TYPE_INT, type);
}

void CWeenieObject::SetName(const char *name)
{
	m_Qualities.SetString(NAME_STRING, name);
}

void CWeenieObject::SetShortDescription(const char *text)
{
	m_Qualities.SetString(SHORT_DESC_STRING, text);
}

void CWeenieObject::SetLongDescription(const char *text)
{
	m_Qualities.SetString(LONG_DESC_STRING, text);
}

void CWeenieObject::SetRadarBlipColor(RadarBlipEnum color)
{
	/*
	1 = Lifestone, Pure Blue
	2 = Monster Orange
	3 = NPK White
	4 = Portal Purple
	5 = PK Red
	6 = PKLite Light Pink
	7 = Fellowship Dark Green?
	8 = NPC Light Yellow
	9 = Envoy Light Blue
	10 = Fellowship Bright Green?
	*/
	m_Qualities.SetInt(RADARBLIP_COLOR_INT, (int)color);
}

void CWeenieObject::SetInitialPhysicsState(uint32_t physics_state)
{
	m_Qualities.SetInt(PHYSICS_STATE_INT, physics_state);
}

void CWeenieObject::SetInitialPosition(const Position &position)
{
	m_Qualities.SetPosition(INSTANTIATION_POSITION, position);
}

void CWeenieObject::SetSpellID(uint32_t spell_id)
{
	m_Qualities.SetDataID(SPELL_DID, spell_id);
}


uint32_t CWeenieObject::GetValue()
{
	return m_Qualities.GetInt(VALUE_INT, 0);
}

void CWeenieObject::SetValue(uint32_t amount)
{
	m_Qualities.SetInt(VALUE_INT, amount);
}

uint32_t CWeenieObject::GetSpellID()
{
	return m_Qualities.GetDID(SPELL_DID, 0);
}

uint32_t CWeenieObject::GetIcon()
{
	uint32_t icon = 0;
	m_Qualities.InqDataID(ICON_DID, icon);
	return icon;
}

std::string CWeenieObject::GetName()
{
	if (!_IsPlayer())
	{
		if (MaterialType type = (MaterialType)InqIntQuality(MATERIAL_TYPE_INT, 0))
		{
			if (type != MaterialType::Leather_MaterialType)
			{
				std::string name;
				MaterialTypeEnumMapper::MaterialTypeToString(type, name);
				name.append(" ");
				name.append(InqStringQuality(NAME_STRING, ""));
				return name;
			}
		}
	}
	std::string name;
	m_Qualities.InqString(NAME_STRING, name);
	return name;
}

std::string CWeenieObject::GetPluralName()
{
	std::string name;
	if (!m_Qualities.InqString(PLURAL_NAME_STRING, name))
	{
		if (m_Qualities.InqString(NAME_STRING, name))
			name += "s";
	}
	return name;
}

ITEM_TYPE CWeenieObject::GetItemType()
{
	int type = ITEM_TYPE::TYPE_UNDEF;
	m_Qualities.InqInt(ITEM_TYPE_INT, type);
	return (ITEM_TYPE)type;
}

bool CWeenieObject::IsCreature()
{
	return (GetItemType() == TYPE_CREATURE);
}

BOOL CWeenieObject::DoCollision(const class EnvCollisionProfile &prof)
{
	if (_phys_obj && (_phys_obj->m_PhysicsState & SCRIPTED_COLLISION_PS))
	{
		_phys_obj->play_default_script();
	}

	if (_IsPlayer())
	{
		float jumpVelocity = 0.0f;
		weenie_obj->InqJumpVelocity(1.0, jumpVelocity);
		float overSpeed = (jumpVelocity + prof.velocity.z) + 4.5; // a little leeway

		double overSpeedRatio = -(overSpeed / jumpVelocity);

		if (overSpeedRatio > 0.00)
		{
			int damage = overSpeedRatio * 40.0; // (int)(-overSpeed * 5.0);

			if (damage > 0)
			{
				DamageEventData damageEvent;
				damageEvent.damage_type = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
				damageEvent.damage_form = DAMAGE_FORM::DF_IMPACT;
				damageEvent.damageAfterMitigation = damageEvent.damageBeforeMitigation = damage;
				damageEvent.target = this;

				// LOG(Temp, Normal, "%f %f HitGround overSpeed: %f overSpeedRatio: %f damage: %d\n",jumpVelocity, fallVelocity, overSpeed, overSpeedRatio, damage);
				if (!ImmuneToDamage(NULL))
				{
					TakeDamage(damageEvent);
				}
			}
		}
	}

	return 1;
}

BOOL CWeenieObject::DoCollision(const class AtkCollisionProfile &prof)
{
	if (_phys_obj && (_phys_obj->m_PhysicsState & SCRIPTED_COLLISION_PS))
	{
		_phys_obj->play_default_script();
	}

	return 1;
}

BOOL CWeenieObject::DoCollision(const class ObjCollisionProfile &prof)
{
	if (_phys_obj && (_phys_obj->m_PhysicsState & SCRIPTED_COLLISION_PS))
	{
		_phys_obj->play_default_script();
	}

	return 1;
}

void CWeenieObject::DoCollisionEnd(uint32_t object_id)
{
}

const char *CWeenieObject::GetLongDescription()
{
	std::string longDesc;
	m_Qualities.InqString(LONG_DESC_STRING, longDesc);
	return longDesc.c_str();
}

int CWeenieObject::InqIntQuality(STypeInt key, int defaultValue, BOOL raw)
{
	int value = defaultValue;
	m_Qualities.InqInt(key, value, raw);
	return value;
}

int64_t CWeenieObject::InqInt64Quality(STypeInt64 key, int64_t defaultValue)
{
	int64_t value = (int64_t)defaultValue;
	m_Qualities.InqInt64(key, value);
	return value;
}

BOOL CWeenieObject::InqBoolQuality(STypeBool key, BOOL defaultValue)
{
	BOOL value = defaultValue;
	m_Qualities.InqBool(key, value);
	return value;
}

double CWeenieObject::InqFloatQuality(STypeFloat key, double defaultValue, BOOL raw)
{
	double value = defaultValue;
	m_Qualities.InqFloat(key, value, raw);
	return value;
}

std::string CWeenieObject::InqStringQuality(STypeString key, std::string defaultValue)
{
	std::string value = defaultValue;
	m_Qualities.InqString(key, value);
	return value;
}

uint32_t CWeenieObject::InqDIDQuality(STypeDID key, uint32_t defaultValue)
{
	uint32_t value = defaultValue;
	m_Qualities.InqDataID(key, value);
	return value;
}

uint32_t CWeenieObject::InqIIDQuality(STypeIID key, uint32_t defaultValue)
{
	uint32_t value = defaultValue;
	m_Qualities.InqInstanceID(key, value);
	return value;
}

Position CWeenieObject::InqPositionQuality(STypePosition key, const Position &defaultValue)
{
	Position value = defaultValue;
	m_Qualities.InqPosition(key, value);
	return value;
}

DAMAGE_TYPE CWeenieObject::InqDamageType()
{
	return (DAMAGE_TYPE)InqIntQuality(DAMAGE_TYPE_INT, UNDEF_DAMAGE_TYPE);
}

bool CWeenieObject::GetFloatEnchantmentDetails(STypeFloat stype, double defaultValue, EnchantedQualityDetails *enchantmentDetails)
{
	enchantmentDetails->rawValue = InqFloatQuality(stype, defaultValue, true);
	bool returnValue = m_Qualities.GetFloatEnchantmentDetails(stype, enchantmentDetails);
	enchantmentDetails->CalculateEnchantedValue();
	return returnValue;
}

bool CWeenieObject::GetIntEnchantmentDetails(STypeInt stype, int defaultValue, EnchantedQualityDetails *enchantmentDetails)
{
	enchantmentDetails->rawValue = InqIntQuality(stype, defaultValue, true);
	bool returnValue = m_Qualities.GetIntEnchantmentDetails(stype, enchantmentDetails);
	enchantmentDetails->CalculateEnchantedValue();
	return returnValue;
}

bool CWeenieObject::GetBodyArmorEnchantmentDetails(unsigned int bodyPart, DAMAGE_TYPE damageType, EnchantedQualityDetails *enchantmentDetails)
{
	bool returnValue = m_Qualities.GetBodyArmorEnchantmentDetails(bodyPart, damageType, enchantmentDetails);
	enchantmentDetails->CalculateEnchantedValue();
	return returnValue;
}

void CWeenieObject::TryCancelAttack()
{
	int mode;
	if (m_Qualities.InqInt(COMBAT_MODE_INT, mode) && (COMBAT_MODE)mode == MAGIC_COMBAT_MODE)
		return;

	if (m_AttackManager)
		m_AttackManager->Cancel();

	if (_IsPlayer() && !IsInPeaceMode())
		AsPlayer()->m_bCancelAttack = true;
}

void CWeenieObject::HandleAttackHook(const AttackCone &cone)
{
	if (m_AttackManager)
	{
		m_AttackManager->HandleAttackHook(cone);
	}
}

ITEM_TYPE CWeenieObject::InqType()
{
	return GetItemType(); // clientside from pwd
}

int CWeenieObject::InqCollisionProfile(ObjCollisionProfile &prof)
{
	prof.wcid = m_Qualities.id;
	prof.itemType = InqType();
	prof.SetCreature(IsCreature());
	prof.SetPlayer(_IsPlayer());
	prof.SetAttackable(IsAttackable() ? TRUE : FALSE);
	prof.SetDoor(IsDoor() ? TRUE : FALSE);
	return TRUE;
}

void CWeenieObject::HitGround(float fallVelocity)
{
	/*
	if (!_IsPlayer())
	return;

	float jumpVelocity = 0.0f;
	weenie_obj->InqJumpVelocity(1.0, jumpVelocity);
	float overSpeed = (jumpVelocity + fallVelocity) + 4.5; // a little leeway

	double overSpeedRatio = -(overSpeed / jumpVelocity);

	if (overSpeedRatio > 0.00)
	{
	int damage = overSpeedRatio * 40.0; // (int)(-overSpeed * 5.0);

	if (damage <= 0)
	return;

	DamageEventData damageEvent;
	damageEvent.damage_type = DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE;
	damageEvent.damage_form = DAMAGE_FORM::DF_IMPACT;
	damageEvent.outputDamage = damageEvent.inputDamage = damage;
	damageEvent.target = this;

	// LOG(Temp, Normal, "%f %f HitGround overSpeed: %f overSpeedRatio: %f damage: %d\n",jumpVelocity, fallVelocity, overSpeed, overSpeedRatio, damage);
	if (ImmuneToDamage(NULL))
	return;

	TakeDamage(damageEvent);
	}
	*/
}

void CWeenieObject::TryCastSpell(uint32_t target_id, uint32_t spell_id)
{
	int error = 0;
	SchoolOfMagic skillNeeded = (SchoolOfMagic)MagicSystem::GetSpellTable()->GetSpellBase(spell_id)->_school;
	if ((skillNeeded == War_Magic || skillNeeded == Void_Magic) &&
		(lastUsedMagicSkill != Life_Magic && lastUsedMagicSkill != ItemEnchantment_Magic && lastUsedMagicSkill != CreatureEnchantment_Magic))
	{
		if (skillNeeded != lastUsedMagicSkill && lastUsedMagicSkill != Undef_Magic)
		{
			if (double time = m_Qualities.InqFloat(NEXT_SPELLCAST_TIMESTAMP_FLOAT, time) && time > Timer::cur_time)
			{
				error = WERROR_INTERRUPTED;
			}
			else
			{
				lastUsedMagicSkill = skillNeeded;
				m_Qualities.SetFloat(NEXT_SPELLCAST_TIMESTAMP_FLOAT, Timer::cur_time + Random::RollDice(3.0, 5.0));
			}
		}
	}

	if (!error)
		error = MakeSpellcastingManager()->TryBeginCast(target_id, spell_id);

	if (error)
	{
		NotifyUseDone(error);
	}
}

void CWeenieObject::DoForcedStopCompletely()
{
	MakeMovementManager(TRUE);
	StopCompletely(0);

	_server_control_timestamp++;
	last_move_was_autonomous = false;
	Animation_Update();
}

uint32_t CWeenieObject::DoAutonomousMotion(uint32_t motion, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	uint32_t err;

	if (params)
	{
		err = get_minterp()->DoMotion(motion, params);
	}
	else
	{
		MovementParameters defaultParams;
		defaultParams.action_stamp = ++m_wAnimSequence; // maybe check if motion is & 0x10000000
		err = get_minterp()->DoMotion(motion, &defaultParams);
	}

	if (!err)
	{
		_server_control_timestamp++;
		last_move_was_autonomous = false;
		Animation_Update();
	}

	return err;
}

uint32_t CWeenieObject::DoForcedMotion(uint32_t motion, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	uint32_t err;

	if (params)
	{
		err = get_minterp()->DoMotion(motion, params);
	}
	else
	{
		MovementParameters defaultParams;
		defaultParams.action_stamp = ++m_wAnimSequence; // maybe check if motion is & 0x10000000
		err = get_minterp()->DoMotion(motion, &defaultParams);
	}

	if (!err)
	{
		// _server_control_timestamp++;
		last_move_was_autonomous = false;
		Animation_Update();
	}

	return err;
}

bool CWeenieObject::IsCompletelyIdle()
{
	if (IsBusyOrInAction())
		return false;

	if (movement_manager)
	{
		if (movement_manager->moveto_manager)
		{
			if (movement_manager->moveto_manager->movement_type != MovementTypes::Invalid)
				return false;
		}


		if (movement_manager->motion_interpreter)
		{
			if (movement_manager->motion_interpreter->interpreted_state.sidestep_command ||
				movement_manager->motion_interpreter->interpreted_state.turn_command ||
				movement_manager->motion_interpreter->interpreted_state.forward_command != 0x41000003)
				return false;
		}
	}

	return true;
}

bool CWeenieObject::IsBusy()
{
	if (m_UseManager && m_UseManager->IsUsing())
		return true;

	if (m_SpellcastingManager && m_SpellcastingManager->m_bCasting)
		return true;

	return false;
}

bool CWeenieObject::IsMovingTo(MovementTypes key)
{
	if (!movement_manager || !movement_manager->moveto_manager)
		return false;

	if (key == Invalid)
		return movement_manager->moveto_manager->movement_type != MovementTypes::Invalid; //default returns true if we are doing any type of positional movement.
	else
		return movement_manager->moveto_manager->movement_type == key; //are we doing a specific movement type?
}

bool CWeenieObject::IsBusyOrInAction()
{
	return IsBusy() || HasInterpActions();
}

bool CWeenieObject::HasInterpActions()
{
	if (!movement_manager)
		return false;

	if (!movement_manager->motion_interpreter)
		return false;

	return movement_manager->motion_interpreter->interpreted_state.GetNumActions() > 0;
}

bool CWeenieObject::ImmuneToDamage(CWeenieObject *other)
{
	if (m_PhysicsState & (HIDDEN_PS | PARTICLE_EMITTER_PS | MISSILE_PS))
		return true;

	if (!IsCreature())
		return true;

	if (IsCreature() && !IsAttackable()) //no aoe spell collision damage on unattackable creatures (players, etc.)
		return true;

	if (IsDead())
		return true;

	return false;
}

BODY_PART_ENUM GetRandomBodyPartByDamageQuadrant(DAMAGE_QUADRANT dq)
{
	BODY_PART_ENUM result = BP_UNDEF;
	float hitLoc = Random::RollDice(0.0, 1.0);

	// do damage processing here...
	if (dq & DQ_LOW)
	{
		if (hitLoc <= 0.18)
			result = BP_UPPER_LEG;
		else if (hitLoc <= 0.78)
			result = BP_LOWER_LEG;
		else
			result = BP_FOOT;
	}
	else if (dq & DQ_MEDIUM)
	{
		if (hitLoc <= 0.17)
			result = BP_CHEST;
		else if (hitLoc <= 0.34)
			result = BP_ABDOMEN;
		else if (hitLoc <= 0.37)
			result = BP_UPPER_ARM;
		else if (hitLoc <= 0.67)
			result = BP_LOWER_ARM;
		else if (hitLoc <= 0.87)
			result = BP_HAND;
		else
			result = BP_UPPER_LEG;
	}
	else if (dq & DQ_HIGH)
	{
		if (hitLoc <= 0.33)
			result = BP_HEAD;
		else if (hitLoc <= 0.77)
			result = BP_CHEST;
		else
			result = BP_UPPER_ARM;
	}
	else
	{
		result = BP_CHEST;
	}

	return result;
}

void CWeenieObject::TryToDealDamage(DamageEventData &data)
{
	if (!data.target)
		return;

	data.target->HandleAggro(this);

	if (data.damageBeforeMitigation >= 0)
	{
		if (data.target->ImmuneToDamage(data.source))
			return;
	}

	if (data.target->AsPlayer() && data.target->AsPlayer()->m_Qualities.GetBool(UNDER_LIFESTONE_PROTECTION_BOOL, 0))
	{
		data.target->EmitEffect(PS_ShieldUpBlue, 1.0);
		data.target->NotifyWeenieError(WERROR_LIFESTONE_PROTECTION);
		return;
	}

	if (data.target->AsPlayer() && data.source->AsPlayer())
	{
		WErrorType attackerError = WERROR_NONE;
		WErrorType defenderError = WERROR_NONE;
		if ( !IsValidPkAction(false, (PKStatusEnum)m_Qualities.GetInt(PLAYER_KILLER_STATUS_INT, 1),
			(PKStatusEnum)data.target->m_Qualities.GetInt(PLAYER_KILLER_STATUS_INT, 1), attackerError, defenderError))
		{
			data.source->AsPlayer()->NotifyWeenieErrorWithString(attackerError, data.target->GetName().c_str());
			data.target->NotifyWeenieErrorWithString(defenderError, GetName().c_str());
			return;
		}
	}
	
	if (data.damage_form & DF_PHYSICAL)
	{
		std::map<int32_t, float> bodyParts;
		if (data.target->m_Qualities._body)
		{
			//todo: taking into account the rest of the quadrant data(front/sides/etc)
			for (auto &bp : data.target->m_Qualities._body->_body_part_table)
			{
				if (data.hit_quadrant == DQ_HLF && bp.second._bpsd->HLF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->HLF);
				else if (data.hit_quadrant == DQ_HLB && bp.second._bpsd->HLB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->HLB);
				else if (data.hit_quadrant == DQ_HRF && bp.second._bpsd->HRF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->HRF);
				else if (data.hit_quadrant == DQ_HRB && bp.second._bpsd->HRB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->HRB);
				else if (data.hit_quadrant == DQ_MLF && bp.second._bpsd->MLF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->MLF);
				else if (data.hit_quadrant == DQ_MLB && bp.second._bpsd->MLB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->MLB);
				else if (data.hit_quadrant == DQ_MRF && bp.second._bpsd->MRF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->MRF);
				else if (data.hit_quadrant == DQ_MRB && bp.second._bpsd->MRB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->MRB);
				else if (data.hit_quadrant == DQ_LLF && bp.second._bpsd->LLF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->LLF);
				else if (data.hit_quadrant == DQ_LLB && bp.second._bpsd->LLB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->LLB);
				else if (data.hit_quadrant == DQ_LRF && bp.second._bpsd->LRF > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->LRF);
				else if (data.hit_quadrant == DQ_LRB && bp.second._bpsd->LRB > 0)
					bodyParts.emplace(bp.first, bp.second._bpsd->LRB);
			}
		}

		data.hitPart = (BODY_PART_ENUM)BP_UNDEF;
		if (bodyParts.size())
		{
			float bodyPartSum = 0;
			float bodyPartHitRoll = Random::RollDice(0.0, 1.0);
			for (std::map<int32_t, float >::iterator it = bodyParts.begin(); it != bodyParts.end(); ++it)
			{
				if (bodyPartHitRoll <= (it->second + bodyPartSum))
				{
					data.hitPart = (BODY_PART_ENUM)it->first;
					break;
				}
				else
					bodyPartSum += it->second;
			}
			if (data.hitPart == (BODY_PART_ENUM)BP_UNDEF)
			{
				SERVER_ERROR << "Unknown body part for " << data.target->GetName();
				data.hitPart = (BODY_PART_ENUM)BP_HEAD;
			}
		}
		else
		{
			data.hitPart = GetRandomBodyPartByDamageQuadrant(data.hit_quadrant);
		}
	}

	data.target->TakeDamage(data);
}


void GetDeathMessage(DAMAGE_TYPE dt, const std::string &killerName, const std::string &victimName, std::string &killerMessage, std::string &victimMessage, std::string &otherMessage)
{
	switch (dt)
	{
	case BLUDGEON_DAMAGE_TYPE:
		switch (Random::GenInt(0, 4))
		{
		case 0:
			killerMessage = victimName + " is utterly destroyed by your attack!";
			victimMessage = "You are utterly destroyed by " + killerName + "'s attack!";
			otherMessage = victimName + " is utterly destroyed by " + killerName + "'s attack!";
			break;
		case 1:
			killerMessage = "You knock " + victimName + " into next Morningthaw!";
			victimMessage = killerName + " knocks you into next Morningthaw!";
			otherMessage = killerName + " knocks " + victimName + " into next Morningthaw!";
			break;
		case 2:
			killerMessage = "You flatten " + victimName + "'s body with the force of your assault!";
			victimMessage = killerName + " knocks you into next Morningthaw!";
			otherMessage = killerName + " knocks " + victimName + " into next Morningthaw!";
			break;
		case 3:
			killerMessage = "You beat " + victimName + " to a lifeless pulp!";
			victimMessage = killerName + " beats you to a lifeless pulp!";
			otherMessage = killerName + " beats " + victimName + " to a lifeless pulp!";
			break;
		case 4:
			killerMessage = "The thunder of crushing " + victimName + " is followed by the deafening silence of death!";
			victimMessage = killerName + "'s thunder of crushing you is followed by the deafening silence of death!";
			otherMessage = killerName + "'s thunder of crushing " + victimName + " is followed by the deafening silence of death!";
			break;
		}
		break;
	case FIRE_DAMAGE_TYPE:
		switch (Random::GenInt(0, 3))
		{
		case 0:
			killerMessage = victimName + "'s seared corpse smolders before you!";
			victimMessage = "Your seared corpse smolders before " + killerName + "!";
			otherMessage = victimName + "'s seared corpse smolders before " + killerName + "!";
			break;
		case 1:
			killerMessage = "You bring " + victimName + " to a fiery end!";
			victimMessage = killerName + " brings you to a fiery end!";
			otherMessage = killerName + " brings " + victimName + " to a fiery end!";
			break;
		case 2:
			killerMessage = "You reduced " + victimName + " to cinders!";
			victimMessage = "You are reduced to cinders by " + killerName + "!";
			otherMessage = killerName + " reduced " + victimName + " to cinders!";
			break;
		case 3:
			killerMessage = victimName + " is incinerated by your assault!";
			victimMessage = "You are incinerated by " + killerName + "'s assault!";
			otherMessage = victimName + " is incinerated by " + killerName + "'s assault!";
			break;
		}
		break;
	case PIERCE_DAMAGE_TYPE:
		switch (Random::GenInt(0, 3))
		{
		case 0:
			killerMessage = victimName + "'s perforated corpse falls before you!";
			victimMessage = "Your perforated corpse falls before " + killerName + "!";
			otherMessage = victimName + "'s perforated corpse falls before " + killerName + "!";
			break;
		case 1:
			killerMessage = victimName + "'s death is preceded by a sharp, stabbing pain!";
			victimMessage = "Your death is preceded by a sharp, stabbing pain!";
			otherMessage = victimName + "'s death is preceded by a sharp, stabbing pain courtesy of " + killerName + "!";
			break;
		case 2:
			killerMessage = victimName + " is fatally punctured!";
			victimMessage = "You were fatally punctured!";
			otherMessage = victimName + " is fatally punctured by " + killerName + "!";
			break;
		case 3:
			killerMessage = "You run " + victimName + " through!";
			victimMessage = killerName + " runs you through!";
			otherMessage = killerName + " runs " + victimName + " through!";
			break;
		}
		break;
	case SLASH_DAMAGE_TYPE:
		switch (Random::GenInt(0, 2))
		{
		case 0:
			killerMessage = "You split " + victimName + " apart!";
			victimMessage = killerName + " splits you apart!";
			otherMessage = killerName + " splits " + victimName + " apart!";
			break;
		case 1:
			killerMessage = "You cleave " + victimName + " in twain!";
			victimMessage = killerName + " cleaves you in twain!";
			otherMessage = killerName + " cleaves " + victimName + " in twain!";
			break;
		case 2:
			killerMessage = "You slay " + victimName + " viciously enough to impart death several times over!";
			victimMessage = killerName + " slays you viciously enough to impart death several times over!";
			otherMessage = killerName + " slays " + victimName + " viciously enough to impart death several times over!";
			break;
		}
		break;
	case NETHER_DAMAGE_TYPE:
		switch (Random::GenInt(0, 2))
		{
		case 0:
			killerMessage = "You reduce " + victimName + " to a drained, twisted corpse!";
			victimMessage = killerName + " reduces you to a drained, twisted corpse!";
			otherMessage = killerName + " reduces " + victimName + " to a drained, twisted corpse!";
			break;
		case 1:
			killerMessage = victimName + " is dessicated by your attack!";
			victimMessage = "You are dessicated by " + killerName + "'s attack!";
			otherMessage = victimName + " is dessicated by " + killerName + "'s attack!";
			break;
		case 2:
			killerMessage = victimName + "'s last strength withers before you!";
			victimMessage = "Your last strength withers before " + killerName + "!";
			otherMessage = victimName + "'s last strength withers before " + killerName + "!";
			break;
		}
		break;
	case ACID_DAMAGE_TYPE:
		switch (Random::GenInt(0, 2))
		{
		case 0:
			killerMessage = "You reduce " + victimName + " to a sizzling, oozing mass!";
			victimMessage = killerName + " reduces you to a sizzling, oozing mass!";
			otherMessage = killerName + " reduces " + victimName + " to a sizzling, oozing mass!";
			break;
		case 1:
			killerMessage = victimName + "'s last strength dissolves before you!";
			victimMessage = "Your last strength dissolves before " + killerName + "!";
			otherMessage = victimName + "'s last strength dissolves before " + killerName + "!";
			break;
		case 2:
			killerMessage = victimName + " is liquified by your attack!";
			victimMessage = "You are liquified by " + killerName + "'s attack!";
			otherMessage = victimName + " is liquified by " + killerName + "'s attack!";
			break;
		}
		break;

	case COLD_DAMAGE_TYPE:
		switch (Random::GenInt(0, 2))
		{
		case 0:
			killerMessage = "Your attack stops " + victimName + " cold!";
			victimMessage = killerName + "'s attack stops you cold!";
			otherMessage = killerName + " stops " + victimName + " cold!";
			break;
		case 1:
			killerMessage = "Your assault sends " + victimName + " to an icy death!";
			victimMessage = killerName + "'s assault sends you to an icy death!";
			otherMessage = killerName + "'s assault sends " + victimName + " to an icy death!";
			break;
		case 2:
			killerMessage = victimName + " suffers a frozen fate!";
			victimMessage = "You suffer a frozen fate at the hands of " + killerName + "!";
			otherMessage = victimName + " suffers a frozen fate at the hands of " + killerName + "!";
			break;
		}
		break;

	case ELECTRIC_DAMAGE_TYPE:
		switch (Random::GenInt(0, 1))
		{
		case 0:
			killerMessage = "Your lightning coruscates over " + victimName + "'s mortal remains!";
			victimMessage = killerName + "'s lightning coruscates over your mortal remains!";
			otherMessage = killerName + "'s lightning coruscates over " + victimName + "'s mortal remains!";
			break;
			/*
			case 1:
			killerMessage = "Blistered by lightning, " + victimName + " falls!";
			break;
			*/
		case 1:
			killerMessage = "Electricity tears " + victimName + " apart!";
			victimMessage = killerName + "'s electricity tears you apart!";
			otherMessage = killerName + "'s electricity tears " + victimName + " apart!";
			break;
		}
		break;
	default:
	{
		killerMessage = "You killed " + victimName + "!";
		victimMessage = "You were killed by " + killerName + "!";
		otherMessage = killerName + " slayed " + victimName + "!";
	}
	}
}

float CWeenieObject::GetArmorModForDamageType(DAMAGE_TYPE dt)
{
	switch (dt)
	{
	case SLASH_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_SLASH_FLOAT, 1.0f, FALSE);
	case PIERCE_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_PIERCE_FLOAT, 1.0f, FALSE);
	case BLUDGEON_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_BLUDGEON_FLOAT, 1.0f, FALSE);
	case FIRE_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_FIRE_FLOAT, 1.0f, FALSE);
	case COLD_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_COLD_FLOAT, 1.0f, FALSE);
	case ELECTRIC_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_ELECTRIC_FLOAT, 1.0f, FALSE);
	case ACID_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_ACID_FLOAT, 1.0f, FALSE);
	case NETHER_DAMAGE_TYPE: return InqFloatQuality(ARMOR_MOD_VS_NETHER_FLOAT, 1.0f, FALSE);
	}

	return 1.0f;
}

float CWeenieObject::GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor)
{
	float armorLevel = 0.0f;

	bool isShield = InqIntQuality(COMBAT_USE_INT, 0, TRUE) == COMBAT_USE::COMBAT_USE_SHIELD &&
		InqIntQuality(SHIELD_VALUE_INT, 0, false) > 0;
	if (isShield && (damageData.hit_quadrant & DQ_FRONT) == 0)
		return 0.0f; //shield only works if the attack was from the front.

	//bool isShieldSpeced = false;
	//SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
	//if (damageData.target->m_Qualities.InqSkillAdvancementClass(SHIELD_SKILL, sac))
	//	isShieldSpeced = sac == SPECIALIZED_SKILL_ADVANCEMENT_CLASS;


	EnchantedQualityDetails buffDetails;
	if (!isShield)
		GetIntEnchantmentDetails(ARMOR_LEVEL_INT, 0, &buffDetails);
	else
		GetIntEnchantmentDetails(SHIELD_VALUE_INT, 0, &buffDetails);

	if (bIgnoreMagicArmor)
		armorLevel = buffDetails.rawValue; //take the Raw armor value for Hollows. Debuffs should not count
	else
		armorLevel = buffDetails.enchantedValue;

	if (isShield)
	{
		//uint32_t shieldSkill;
		//damageData.target->m_Qualities.InqSkill(SHIELD_SKILL, shieldSkill, false);
		
		//if (!isShieldSpeced)
		//{
		//	if (armorLevel >= shieldSkill / 2)
		//	{
		//		armorLevel = shieldSkill / 2;
		//	}
		//}
		//else
		//{
		//	armorLevel = min((float)shieldSkill, armorLevel);
		//}
		double ignoreShieldMod = 0.0f;
		if (damageData.source)
			damageData.source->m_Qualities.InqFloat(IGNORE_SHIELD_FLOAT, ignoreShieldMod, FALSE);

		armorLevel *= (1.0 - ignoreShieldMod);
	}

	switch (damageData.damage_type)
	{
	case DAMAGE_TYPE::SLASH_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_SLASH_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::PIERCE_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_PIERCE_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_BLUDGEON_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::FIRE_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_FIRE_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::COLD_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_COLD_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::ACID_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_ACID_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::ELECTRIC_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_ELECTRIC_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	case DAMAGE_TYPE::NETHER_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_NETHER_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0)); break;
	}

	return armorLevel;
}

void CWeenieObject::TakeDamage(DamageEventData &damageData)
{
	EnchantedQualityDetails buffDetails;
	bool isEnchanted = false;

	switch (damageData.damage_type)
	{
	case SLASH_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_SLASH_FLOAT, 1.0, &buffDetails);
		break;
	case PIERCE_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_PIERCE_FLOAT, 1.0, &buffDetails);
		break;
	case BLUDGEON_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_BLUDGEON_FLOAT, 1.0, &buffDetails);
		break;
	case FIRE_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_FIRE_FLOAT, 1.0, &buffDetails);
		break;
	case COLD_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_COLD_FLOAT, 1.0, &buffDetails);
		break;
	case ACID_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_ACID_FLOAT, 1.0, &buffDetails);
		break;
	case ELECTRIC_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_ELECTRIC_FLOAT, 1.0, &buffDetails);
		break;
	case NETHER_DAMAGE_TYPE:
		isEnchanted = GetFloatEnchantmentDetails(RESIST_NETHER_FLOAT, 1.0, &buffDetails);
		break;
	case HEALTH_DAMAGE_TYPE:
		if (damageData.damageAfterMitigation > 0)
			isEnchanted = GetFloatEnchantmentDetails(RESIST_HEALTH_DRAIN_FLOAT, 1.0, &buffDetails);
		else
			isEnchanted = GetFloatEnchantmentDetails(RESIST_HEALTH_BOOST_FLOAT, 1.0, &buffDetails);
		break;
	case STAMINA_DAMAGE_TYPE:
		if (damageData.damageAfterMitigation > 0)
			isEnchanted = GetFloatEnchantmentDetails(RESIST_STAMINA_DRAIN_FLOAT, 1.0, &buffDetails);
		else
			isEnchanted = GetFloatEnchantmentDetails(RESIST_STAMINA_BOOST_FLOAT, 1.0, &buffDetails);
		break;
	case MANA_DAMAGE_TYPE:
		if (damageData.damageAfterMitigation > 0)
			isEnchanted = GetFloatEnchantmentDetails(RESIST_MANA_DRAIN_FLOAT, 1.0, &buffDetails);
		else
			isEnchanted = GetFloatEnchantmentDetails(RESIST_MANA_BOOST_FLOAT, 1.0, &buffDetails);
		break;
	case BASE_DAMAGE_TYPE:
		break;
	}

	if (damageData.isElementalRending && damageData.rendingMultiplier > buffDetails.valueIncreasingMultiplier)
	{
		//our elemental rending is better than the debuffs applied, replace debuffs with rending.
		//we don't need to check if the elemental rending is of the correct type here as the check is already done on the attacker's side
		//and the flag is only true if the damage type matches the rending element.
		buffDetails.valueIncreasingMultiplier = damageData.rendingMultiplier;
		buffDetails.CalculateEnchantedValue();
	}

	float resistanceRegular = 0;
	if (damageData.damage_type != BASE_DAMAGE_TYPE)
	{
		if (damageData.ignoreMagicResist)
			resistanceRegular = buffDetails.rawValue; //take the Raw resistance value for Hollows. Debuffs should not count
		else
			resistanceRegular = buffDetails.enchantedValue;
	}

	if (damageData.damageAfterMitigation > 0 || damageData.damage_type == HEALTH_DAMAGE_TYPE || damageData.damage_type == STAMINA_DAMAGE_TYPE || damageData.damage_type == MANA_DAMAGE_TYPE)
	{
		if (!AsPlayer() || damageData.ignoreMagicResist)
		{
			if (resistanceRegular != 0.f)
				damageData.damageAfterMitigation *= resistanceRegular;
		}
		else //only players have natural resistances.
		{
			//Some combination of strength and endurance allows one to have a level of "natural resistances" to the 7 damage types.This caps out at a 50 % resistance(the equivalent to level 5 life prots) to these damage types.This resistance is not additive to life protections : higher level life protections will overwrite these natural resistances, although life vulns will take these natural resistances into account, if the player does not have a higher level life protection cast upon them.
			//For example, a player will not get a free protective bonus from natural resistances if they have both Prot 7 and Vuln 7 cast upon them.The Prot and Vuln will cancel each other out, and since the Prot has overwritten the natural resistances, there will be no resistance bonus.
			//The abilities that Endurance or Endurance/Strength conveys are not increased by Strength or Endurance buffs.It is the raw Strength and/or Endurance scores that determine the various bonuses.
			//drain resistances(same formula as natural resistances) allows one to partially resist drain health/stamina/mana and harm attacks, up to a maximum of roughly 50%. 

			//todo: natural resistances only change when base strength or endurance changes so we could potentially pre-calculate this somewhere else.
			uint32_t strength = 0;
			uint32_t endurance = 0;
			m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + endurance);
			float resistanceNatural;
			if (strAndEnd <= 200) //formula deduced from values present in the client pdb.
				resistanceNatural = 1.0f - ((0.05 * strAndEnd) / 100.f);
			else
				resistanceNatural = 1.0f - (((0.1666667 * strAndEnd) - 23.33333) / 100.f);

			resistanceNatural = max(resistanceNatural, 0.5f);

			//natural resistances only work if they beat the buffed value before any debuffs are applied.
			if (resistanceNatural < buffDetails.enchantedValue_DecreasingOnly)
			{
				//replace our buffed value with natural resistances and recalculate.
				buffDetails.valueDecreasingMultiplier = resistanceNatural;
				buffDetails.CalculateEnchantedValue();
				damageData.damageAfterMitigation *= buffDetails.enchantedValue;
			}
			else
				damageData.damageAfterMitigation *= resistanceRegular;
		}
	}

	if (!damageData.ignoreArmorEntirely && (damageData.damage_form & DF_PHYSICAL))
	{
		// calculate armor level
		double armorLevel = GetEffectiveArmorLevel(damageData, damageData.ignoreMagicArmor);

		float damageFactor;
		if (armorLevel >= 0)
			damageFactor = 1.0 / (1.0 + (armorLevel / (190.0 / 3.0)));
		else
			damageFactor = (1.0 + (-armorLevel / (190.0 / 3.0)));

		damageData.damageAfterMitigation *= damageFactor;
	}

	if (damageData.damage_form & DF_MAGIC && damageData.isProjectileSpell)
	{
		// check for magic absorption
		CWeenieObject *shieldOrMissileWeapon = GetWieldedCombat(COMBAT_USE::COMBAT_USE_SHIELD);
		if (!shieldOrMissileWeapon)
			shieldOrMissileWeapon = GetWieldedCombat(COMBAT_USE::COMBAT_USE_MISSILE);

		if (shieldOrMissileWeapon && shieldOrMissileWeapon->GetImbueEffects() & ImbuedEffectType::IgnoreSomeMagicProjectileDamage_ImbuedEffectType)
		{
			double reduction = (0.25 * GetMagicDefense() * 0.003) - (0.25 * 0.3);

			if (reduction > 0.0)
			{
				if (reduction > 0.25)
					reduction = 0.25;

				if (damageData.source && damageData.target)
				{
					bool isPvP = damageData.source->AsPlayer() && damageData.target->AsPlayer();

					if (isPvP)
						reduction *= 0.72;

					damageData.damageAfterMitigation *= 1.0 - reduction;
				}
			}
		}

		//CWeenieObject *shield = GetWieldedCombat(COMBAT_USE::COMBAT_USE_SHIELD);

		//if (shield && !IsInPeaceMode())
		//{
		//	SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
		//	damageData.target->m_Qualities.InqSkillAdvancementClass(SHIELD_SKILL, sac);
		//	float cap = shield->InqFloatQuality(ABSORB_MAGIC_DAMAGE_FLOAT, 0.0);
		//	uint32_t shieldSkill;
		//	damageData.target->m_Qualities.InqSkill(SHIELD_SKILL, shieldSkill, true);
		//	if (cap > 0.0 && shieldSkill >= 100)
		//	{
		//		float st = sac != SPECIALIZED_SKILL_ADVANCEMENT_CLASS ? .8 : 1;

		//		float reduction = max(min(float(1 * ((cap * st * shieldSkill * 0.0030) - (cap * st * .3))), cap * st), 0.0f);

		//		damageData.damageAfterMitigation *= 1.0 - reduction;
		//	}
		//}
	}

	if (damageData.damageAfterMitigation > 0 && (damageData.damage_type < 0x80 || damageData.damage_type == NETHER_DAMAGE_TYPE)) // elemental damage types only
	{
		// calculate augmentation resistances
		if (damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_FAMILY_INT, 0) > 0)
		{
			double reduction = 0.0;

			switch (damageData.damage_type)
			{
			case BLUDGEON_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_BLUNT_INT, 0); break;
			case SLASH_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_SLASH_INT, 0); break;
			case PIERCE_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_PIERCE_INT, 0); break;
			case ACID_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_ACID_INT, 0); break;
			case FIRE_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_FIRE_INT, 0); break;
			case COLD_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_FROST_INT, 0); break;
			case ELECTRIC_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_LIGHTNING_INT, 0); break;
			case NETHER_DAMAGE_TYPE:
				reduction = 0.1 * damageData.target->InqIntQuality(AUGMENTATION_RESISTANCE_NETHER_INT, 0); break;
			default:
				break;
			}

			damageData.damageAfterMitigation *= 1.0 - reduction;
		}
	}

	if (damageData.damage_type == BASE_DAMAGE_TYPE)
		damageData.damageAfterMitigation = damageData.baseDamage;

	if (damageData.damage_type == NETHER_DAMAGE_TYPE && damageData.isDot)
	{
		int ratingTotal = 0;
		int rating = 0;
		if (damageData.target->m_Qualities.InqInt(AUGMENTATION_DAMAGE_REDUCTION_INT, rating))
		{
			ratingTotal += rating;
		}
		if (damageData.target->m_Qualities.InqInt(DOT_RESIST_RATING_INT, rating))
		{
			ratingTotal += rating;
		}
		damageData.damageAfterMitigation = damageData.baseDamage * (100.0f / (100.0f + (double)ratingTotal));
	}

	if (damageData.damage_type == NETHER_DAMAGE_TYPE && damageData.target->AsPlayer() && damageData.isDot)
	{
		damageData.damageAfterMitigation *= g_pConfig->VoidDamageReduction();
	}

	if (damageData.damageAfterMitigation < 0)
	{
		//todo: can't all damage types heal? Like hitting a fire elemental with fire should heal it instead of damage it?
		if (damageData.damage_type & (HEALTH_DAMAGE_TYPE | STAMINA_DAMAGE_TYPE | MANA_DAMAGE_TYPE))
		{
			// these types can heal
			damageData.outputDamageFinal = (int)ceil(damageData.damageAfterMitigation - F_EPSILON);
		}
		else
		{
			damageData.damageAfterMitigation = 0;
		}
	}
	else
	{
		damageData.outputDamageFinal = (int)floor(damageData.damageAfterMitigation + F_EPSILON);
	}

	if (_IsPlayer())
	{
		CWeenieObject* cloak = GetWielded(CLOAK_LOC);
		if (cloak)
		{
			double baseProcRate = g_pConfig->GetCloakBaseProcRate();
			uint32_t maxHealth;
			m_Qualities.InqAttribute2nd(HEALTH_ATTRIBUTE_2ND, maxHealth, false);
			if (damageData.outputDamageFinal / maxHealth >= .50)
			{
				baseProcRate += g_pConfig->GetCloakHalfHealthProcRate();
			}
			else if (damageData.outputDamageFinal / maxHealth >= .25)
			{
				baseProcRate += g_pConfig->GetCloakQuarterHealthProcRate();
			}
			else if (damageData.outputDamageFinal / maxHealth >= .10)
			{
				baseProcRate += g_pConfig->GetCloakTenthHealthProcRate();
			}

			int cloakLevel = cloak->GetLevel();
			if (cloakLevel > 1)
			{
				baseProcRate += (cloakLevel - 1) * g_pConfig->GetCloakPerLevelProcRate();
			}

			double luck = Random::RollDice(0.0, 1.0);
			if (luck <= baseProcRate)
			{
				int procType;
				if (cloak->m_Qualities.InqInt(CLOAK_WEAVE_PROC_INT, procType))
				{
					if (procType == 2)
					{
						int originalDamage = damageData.outputDamageFinal;
						if(damageData.isPvP)
							damageData.outputDamageFinal -= min(damageData.outputDamageFinal, 100);
						else
							damageData.outputDamageFinal -= min(damageData.outputDamageFinal, 200);
						SendText(csprintf("Your cloak reduced the damage from %u down to %u!", originalDamage, damageData.outputDamageFinal), LTT_MAGIC);
					}
					else if (procType == 1)
					{
						uint32_t procSpell = cloak->m_Qualities.GetDID(PROC_SPELL_DID, 0);
						if (procSpell)
						{
							bool spellCast = false;
							// if shroud, get current combat target and cast spell
							if (procSpell == 5754 || procSpell == 5755 || procSpell == 5756)
							{
								uint32_t target;
								if (m_Qualities.InqInstanceID(CURRENT_ENEMY_IID, target))
								{
									cloak->MakeSpellcastingManager()->CastSpellInstant(target, procSpell);
									spellCast = true;
								}
							}
							else
							{
								cloak->MakeSpellcastingManager()->CastSpellInstant(GetID(), procSpell);
								spellCast = true;
							}

							CSpellTable *spellTable = MagicSystem::GetSpellTable();
							if (spellTable && spellCast)
							{
								const CSpellBase* spell = spellTable->GetSpellBase(procSpell);
								if (spell)
								{
									SendText(csprintf("The cloak of %s weaves the magic of %s!", GetName().c_str(), spell->_name.c_str()), LTT_MAGIC_CASTING_CHANNEL);
								}
							}
						}
					}
				}
			}
		}

	}


	STypeAttribute2nd vitalAffected;
	switch (damageData.damage_type)
	{
	default: vitalAffected = HEALTH_ATTRIBUTE_2ND; break;
	case STAMINA_DAMAGE_TYPE: vitalAffected = STAMINA_ATTRIBUTE_2ND; break;
	case MANA_DAMAGE_TYPE: vitalAffected = MANA_ATTRIBUTE_2ND; break;
	}

	uint32_t vitalStartValue = 0;
	uint32_t vitalMinValue = 0;
	uint32_t vitalMaxValue = 0;
	m_Qualities.InqAttribute2nd(vitalAffected, vitalStartValue, FALSE);
	m_Qualities.InqAttribute2nd((STypeAttribute2nd)((int)vitalAffected - 1), vitalMaxValue, FALSE);

	int vitalNewValue = vitalStartValue - damageData.outputDamageFinal;
	vitalNewValue = max((int)vitalMinValue, min((int)vitalMaxValue, vitalNewValue));

	damageData.outputDamageFinal = (int)vitalStartValue - (int)vitalNewValue;
	damageData.outputDamageFinalPercent = damageData.outputDamageFinal / (double)vitalMaxValue;

	if (AsMonster())
	{
		// add to list of attackers
		AsMonster()->UpdateDamageList(damageData);
	}

	if (vitalNewValue != vitalStartValue)
	{
		m_Qualities.SetAttribute2nd(vitalAffected, vitalNewValue);
		NotifyAttribute2ndStatUpdated(vitalAffected);

		if (vitalAffected == HEALTH_ATTRIBUTE_2ND)
		{
			if (vitalNewValue <= 0)
			{
				damageData.killingBlow = true;
				OnDeath(damageData.source ? damageData.source->GetID() : 0);
			}
			else
			{
				if (damageData.damage_form & DF_IMPACT)
				{
					if (damageData.outputDamageFinalPercent >= 0.1f)
					{
						EmitSound(13, 1.0f);
					}
					else
					{
						EmitSound(14, 1.0f);
					}
				}
				else
				{
					if (damageData.outputDamageFinalPercent >= 0.1f)
					{
						if (damageData.damage_form & DF_PHYSICAL)
						{
							switch (damageData.damage_type)
							{
							case DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE:
								EmitSound(Random::GenInt(13, 14), 1.0f);
								break;

							default:
								EmitSound(12, 1.0f);
								break;
							}
						}
					}
				}


			}
		}
	}

	if (damageData.killingBlow && (damageData.source && damageData.source->IsCreature()))
	{
		std::string kmsg, vmsg, omsg;
		GetDeathMessage(
			damageData.damage_type,
			damageData.GetSourceName(),
			damageData.GetTargetName(),
			damageData.killer_msg,
			damageData.victim_msg,
			damageData.other_msg);
	}
	else
	{
		damageData.killer_msg = "You killed " + damageData.GetTargetName() + "!";
		damageData.victim_msg = "You died!";
		damageData.other_msg = damageData.GetTargetName() + " died!";
	}

	// notify the victim they took damage
	OnTookDamage(damageData);

	// notify the atttacker they did damage
	if (damageData.source)
	{
		damageData.source->OnDealtDamage(damageData);

		// Monster::OnDeath grants Xp
		//if (damageData.killingBlow)
		//{
		//	damageData.source->GivePerksForKill(this);
		//}
	}
}

const char *damageFormString(DAMAGE_FORM df)
{
	switch (df)
	{
	case DF_MELEE: return "melee";
	case DF_MISSILE: return "missile";
	case DF_MAGIC: return "magic";
	}

	return "unknown form";
}

const char *damageQuadrantString(DAMAGE_QUADRANT dq)
{
	if (dq & DQ_LOW)
		return "low";
	if (dq & DQ_MEDIUM)
		return "medium";
	if (dq & DQ_HIGH)
		return "high";

	return "*";
}

const char *damageTypeString(DAMAGE_TYPE dt)
{
	switch (dt)
	{
	case BLUDGEON_DAMAGE_TYPE: return "bludgeon";
	case PIERCE_DAMAGE_TYPE: return "pierce";
	case SLASH_DAMAGE_TYPE: return "slash";
	case FIRE_DAMAGE_TYPE: return "fire";
	case COLD_DAMAGE_TYPE: return "cold";
	case ELECTRIC_DAMAGE_TYPE: return "electric";
	case ACID_DAMAGE_TYPE: return "acid";
	}

	return "unknown type";
}

std::string DamageEventData::GetSourceName()
{
	return source ? source->GetName() : "A mysterious source";
}

std::string DamageEventData::GetTargetName()
{
	return target ? target->GetName() : "A mysterious victim";
}

void CWeenieObject::NotifyDeathMessage(uint32_t killer_id, const char *message)
{
	BinaryWriter deathMsg;
	deathMsg.Write<uint32_t>(0x19E);
	deathMsg.WriteString(message);
	deathMsg.Write<uint32_t>(GetID());
	deathMsg.Write<uint32_t>(killer_id);

	if (_IsPlayer() && (g_pConfig->ShowDeathMessagesGlobally() || (g_pConfig->ShowPlayerDeathMessagesGlobally() && g_pWorld->FindPlayer(killer_id) != NULL)))
	{
		g_pWorld->BroadcastGlobal(&deathMsg, PRIVATE_MSG, 0, FALSE, FALSE);
	}
	else
	{
		g_pWorld->BroadcastPVS(GetLandcell(), deathMsg.GetData(), deathMsg.GetSize(), PRIVATE_MSG);
	}
}

PScriptType CWeenieObject::GetScriptByHitLoc(DamageEventData &damageData)
{
	PScriptType ps = PS_Invalid;
	switch (damageData.hit_quadrant)
	{
	case DQ_HLF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterUpLeftFront : PS_SparkUpLeftFront; break;
	case DQ_MLF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterMidLeftFront : PS_SparkMidLeftFront; break;
	case DQ_LLF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterLowLeftFront : PS_SparkLowLeftFront; break;
	case DQ_HRF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterUpRightFront : PS_SparkUpRightFront; break;
	case DQ_MRF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterMidRightFront : PS_SparkMidRightFront; break;
	case DQ_LRF: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterLowRightFront : PS_SparkLowRightFront; break;
	case DQ_HLB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterUpLeftBack : PS_SparkUpLeftBack; break;
	case DQ_MLB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterMidLeftBack : PS_SparkMidLeftBack; break;
	case DQ_LLB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterLowLeftBack : PS_SparkLowLeftBack; break;
	case DQ_HRB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterUpRightBack : PS_SparkUpRightBack; break;
	case DQ_MRB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterMidRightBack : PS_SparkMidRightBack; break;
	case DQ_LRB: ps = damageData.outputDamageFinalPercent > 0.05f ? PS_SplatterLowRightBack : PS_SparkLowRightBack; break;
	}
	return ps;
}

bool CWeenieObject::IsValidPkAction(bool helpful, PKStatusEnum attacker, PKStatusEnum defender, WErrorType & attackerError, WErrorType & defenderError)
{
	bool validAction = false;
	if (!helpful)
	{
		if (attacker == Protected_PKStatus)
		{
			attackerError = WERROR_PK_PROTECTED_ATTACKER;
			defenderError = WERROR_PK_PROTECTED_ATTACKER_PASSIVE;
		}
		else if (attacker == NPK_PKStatus && defender == PK_PKStatus)
		{
			attackerError = WERROR_PK_NPK_ATTACKER;
			defenderError = WERROR_PK_NPK_ATTACKER_PASSIVE;
		}
		else if (attacker == PK_PKStatus && defender == NPK_PKStatus)
		{
			attackerError = WERROR_PK_NPK_TARGET;
			defenderError = WERROR_PK_NPK_TARGET_PASSIVE;
		}
		else if ((attacker == PK_PKStatus && defender == PKLite_PKStatus) || (attacker == PKLite_PKStatus && defender == PK_PKStatus))
		{
			attackerError = WERROR_PK_WRONG_KIND;
			defenderError = WERROR_PK_WRONG_KIND_PASSIVE;
		}
		else if (attacker == PKLite_PKStatus && defender == NPK_PKStatus)
		{
			attackerError = WERROR_PK_NPK_TARGET;
			defenderError = WERROR_PK_NPK_TARGET_PASSIVE;
		}
		else if (attacker == NPK_PKStatus && defender == PKLite_PKStatus)
		{
			attackerError = WERROR_PK_NPK_ATTACKER;
			defenderError = WERROR_PK_NPK_TARGET;
		}
		else
			validAction = true;
	}
	else
	{
		if (attacker == NPK_PKStatus && defender == PK_PKStatus)
		{
			attackerError = WERROR_PK_NPK_ATTACKER;
			defenderError = WERROR_PK_NPK_ATTACKER_PASSIVE;
		}
		else if (attacker == PK_PKStatus && defender == NPK_PKStatus)
		{
			attackerError = WERROR_PK_NPK_TARGET;
			defenderError = WERROR_PK_NPK_TARGET_PASSIVE;
		}
		else if ((attacker == PK_PKStatus && defender == PKLite_PKStatus) || (attacker == PKLite_PKStatus && defender == PK_PKStatus))
		{
			attackerError = WERROR_PK_WRONG_KIND;
			defenderError = WERROR_PK_WRONG_KIND_PASSIVE;
		}
		else if (attacker == NPK_PKStatus && defender == PKLite_PKStatus)
		{
			attackerError = WERROR_PK_NPK_ATTACKER;
			defenderError = WERROR_PK_NPK_ATTACKER_PASSIVE;
		}
		else
			validAction = true;
	}

	return validAction;
}

void CWeenieObject::OnTookDamage(DamageEventData &data)
{
	if (data.damage_form & DF_PHYSICAL && data.outputDamageFinal >= 0)
	{
		EmitEffect(GetScriptByHitLoc(data), data.outputDamageFinalPercent);
	}

	if (data.killingBlow)
	{
		if (data.source)
		{
			NotifyVictimEvent(data.victim_msg.c_str());

			if (_IsPlayer())
				NotifyDeathMessage(data.source->GetID(), data.other_msg.c_str());
		}
		else
		{
			NotifyVictimEvent("You died!");

			if (_IsPlayer())
				NotifyDeathMessage(0, csprintf("%s died!", data.GetTargetName().c_str()));
		}
	}
	else
	{
		if (data.damage_form & DF_PHYSICAL)
		{
			NotifyDefenderEvent(
				data.GetSourceName().c_str(),
				data.damage_type,
				data.outputDamageFinalPercent,
				data.outputDamageFinal,
				data.hitPart,
				data.wasCrit,
				data.attackConditions);
		}
		else if (data.damage_form & DF_MAGIC)
		{
			std::string single_adj, plural_adj;
			if (CombatSystem::InqCombatHitAdjectives(data.damage_type, data.outputDamageFinalPercent, single_adj, plural_adj))
			{
				// Killer Phyntos Soldier sears you for 46 points with Incantation of Acid Stream.
				// You chill Killer Phyntos Swarm for 212 points with Halo of Frost II.
				switch (data.damage_type)
				{
				default:
					SendText(csprintf("%s %s you for %d points with %s.", data.GetSourceName().c_str(), plural_adj.c_str(), max(0, data.outputDamageFinal), data.spell_name.c_str()), LTT_MAGIC);
					break;
				case DAMAGE_TYPE::HEALTH_DAMAGE_TYPE:
				case DAMAGE_TYPE::STAMINA_DAMAGE_TYPE:
				case DAMAGE_TYPE::MANA_DAMAGE_TYPE:
					std::string vitalName;
					switch (data.damage_type)
					{
					case HEALTH_DAMAGE_TYPE:
						vitalName = "health";
						break;
					case STAMINA_DAMAGE_TYPE:
						vitalName = "stamina";
						break;
					case MANA_DAMAGE_TYPE:
						vitalName = "mana";
						break;
					}
					bool isRestore = (data.outputDamageFinal < 0);
					SendText(csprintf("%s casts %s and %s %d points of your %s.", data.GetSourceName().c_str(), data.spell_name.c_str(), isRestore ? "restores" : "drains", abs(data.outputDamageFinal), vitalName.c_str()), LTT_MAGIC);
					break;
				}
			}
		}
		else if (data.damage_form & DF_IMPACT)
		{
			std::string single_adj;
			if (data.outputDamageFinalPercent > 0.5)
				single_adj = "massive";
			else if (data.outputDamageFinalPercent > 0.25)
				single_adj = "crushing";
			else if (data.outputDamageFinalPercent > 0.1)
				single_adj = "heavy";
			else
				single_adj = "minor";

			SendText(csprintf("You suffer %d points of %s impact damage.", data.outputDamageFinal, single_adj.c_str()), LTT_COMBAT);
		}
		else if (data.damage_form & DF_HOTSPOT)
		{
			if (data.source)
			{
				std::string activationTalkString;
				if (data.source->m_Qualities.InqString(ACTIVATION_TALK_STRING, activationTalkString))
				{
					//char damageNumText[32];
					//itoa(abs(data.outputDamageFinal), damageNumText, 10);
					//ReplaceString(activationTalkString, "%i", damageNumText);
					std::string damageNum = std::to_string(abs(data.outputDamageFinal));
					ReplaceString(activationTalkString, "%i", damageNum);
					SendText(activationTalkString.c_str(), LTT_DEFAULT);
				}
			}
		}
	}
}

void CWeenieObject::OnDealtDamage(DamageEventData &data)
{
	if (!data.target)
		return;

	if (data.killingBlow)
	{
		if (data.target)
		{
			NotifyKillerEvent(data.killer_msg.c_str());
		}
		else
		{
			// NotifyKillerEvent(csprintf("You killed %s!", data.GetTargetName().c_str()));
		}
	}
	else
	{
		if (data.damage_form & DF_PHYSICAL)
		{
			if (m_Qualities.m_WeenieType == WeenieType::CombatPet_WeenieType && AsMonster()->ShowCombatDamage())
			{
				CPlayerWeenie* owner = g_pWorld->FindPlayer(m_Qualities.GetIID(PET_OWNER_IID, 0));
				if (owner && owner->AsPlayer())
				{
					owner->AsPlayer()->SendText(csprintf("Your pet attacked %s for %u damage.", data.GetTargetName().c_str(),
						data.outputDamageFinal), LTT_COMBAT);
				}
			}
			else
			{
				NotifyAttackerEvent(
					data.GetTargetName().c_str(),
					data.damage_type,
					data.outputDamageFinalPercent,
					data.outputDamageFinal,
					data.wasCrit,
					data.attackConditions);
			}
		}
		else if (data.damage_form & DF_MAGIC)
		{
			std::string single_adj, plural_adj;
			if (CombatSystem::InqCombatHitAdjectives(data.damage_type, data.outputDamageFinalPercent, single_adj, plural_adj))
			{
				// Killer Phyntos Soldier sears you for 46 points with Incantation of Acid Stream.
				// You chill Killer Phyntos Swarm for 212 points with Halo of Frost II.
				switch (data.damage_type)
				{
				default:
					SendText(csprintf("%s%sYou %s %s for %d points with %s.", data.wasCrit ? "Critical hit! " : "", data.isSneakAttack ? "Sneak Attack! " : "", single_adj.c_str(), data.GetTargetName().c_str(), data.outputDamageFinal, data.spell_name.c_str()), LTT_MAGIC);
					break;
				case DAMAGE_TYPE::HEALTH_DAMAGE_TYPE:
				case DAMAGE_TYPE::STAMINA_DAMAGE_TYPE:
				case DAMAGE_TYPE::MANA_DAMAGE_TYPE:
					std::string vitalName;
					switch (data.damage_type)
					{
					case HEALTH_DAMAGE_TYPE:
						vitalName = "health";
						break;
					case STAMINA_DAMAGE_TYPE:
						vitalName = "stamina";
						break;
					case MANA_DAMAGE_TYPE:
						vitalName = "mana";
						break;
					}
					bool isRestore = (data.outputDamageFinal < 0);
					if (data.target == data.source)
						SendText(csprintf("%sYou cast %s and %s %d points of your %s.", data.wasCrit ? "Critical hit! " : "", data.spell_name.c_str(), isRestore ? "restore" : "drain", abs(data.outputDamageFinal), vitalName.c_str()), LTT_MAGIC);
					else
						SendText(csprintf("%sWith %s you %s %d points of %s %s %s.", data.wasCrit ? "Critical hit! " : "", data.spell_name.c_str(), isRestore ? "restore" : "drain", abs(data.outputDamageFinal), vitalName.c_str(), isRestore ? "to" : "from", data.GetTargetName().c_str()), LTT_MAGIC);
					break;
				}
			}
		}
		else if (data.damage_form & DF_IMPACT)
		{
		}
		else if (data.damage_form & DF_HOTSPOT)
		{
		}
	}
}

void CWeenieObject::OnRegen(STypeAttribute2nd currentAttrib, int newAmount)
{

	m_Qualities.SetAttribute2nd(currentAttrib, newAmount);
}

bool CWeenieObject::IsContainedWithinViewable(uint32_t object_id)
{
	// local container
	if (FindContained(object_id))
		return true;

	// remote container
	if (CWeenieObject *externalObject = g_pWorld->FindObject(object_id))
	{
		if (CContainerWeenie *externalContainer = externalObject->GetWorldTopLevelContainer())
		{
			if (externalContainer->_openedById == GetID())
			{
				return true;
			}
		}
	}

	return false;
}

bool CWeenieObject::ShouldSave()
{
	if (_IsPlayer())
		return true;

	return m_bSaveMe;
}

bool CWeenieObject::Save()
{
	CStopWatch watch;

	if (m_Position.objcell_id)
	{
		m_Qualities.SetPosition(LOCATION_POSITION, m_Position);
	}

	CWeenieSave save;
	SaveEx(save);

	BinaryWriter writer;
	save.Pack(&writer);
	bool result = g_pDBIO->CreateOrUpdateWeenie(GetID(), GetTopLevelID(), m_Position.objcell_id >> 16, writer.GetData(), writer.GetSize());
	if (!result)
		SERVER_ERROR << "Failed to save Weenie:" << id << " Owner:" << GetTopLevelID() << " At:" << (m_Position.objcell_id >> 16);

	double elapsed = watch.GetElapsed();
	if (elapsed >= 0.1)
	{
		SERVER_WARN << csprintf("Took %f seconds to save %s\n", elapsed, GetName().c_str());
	}

	return result;
}

void CWeenieObject::SaveEx(CWeenieSave &save)
{
	save.m_SaveInstanceTS = _instance_timestamp;
	save.m_Qualities.CopyFrom(&m_Qualities);
	save.m_ObjDesc = m_ObjDescOverride;
	save.m_WornObjDesc = m_WornObjDesc;
}

void CWeenieObject::LoadEx(CWeenieSave &save)
{
	m_Qualities.CopyFrom(&save.m_Qualities);
	m_ObjDescOverride = save.m_ObjDesc;
	m_WornObjDesc = save.m_WornObjDesc;
}

bool CWeenieObject::Load()
{
#ifndef PUBLIC_BUILD
	CStopWatch watch;
#endif

	unsigned int top_level_object_id = 0;
	unsigned int block_id = 0;
	void *data = NULL;
	unsigned long data_length = 0;

	if (g_pDBIO->GetWeenie(GetID(), &top_level_object_id, &block_id, &data, &data_length))
	{
		BinaryReader reader(data, data_length);

		CWeenieSave save;
		if (save.UnPack(&reader) && !reader.GetLastError())
		{
			LoadEx(save);
			AllegianceTreeNode *node = g_pAllegianceManager->GetTreeNode(GetID());
			if (node)
				node->_level = m_Qualities.GetInt(LEVEL_INT, 1);

#ifndef PUBLIC_BUILD
			double elapsed = watch.GetElapsed();
			if (elapsed >= 0.1)
			{
				LOG_PRIVATE(Temp, Warning, "Took %f seconds to load %s\n", elapsed, GetName().c_str());
			}
#endif

			return true;
		}
		else
		{
			LOG_PRIVATE(Temp, Error, "Failed to unpack weenie 0x%08X!\n", GetID());
		}
	}
	else
	{
		LOG_PRIVATE(Temp, Error, "Failed to load weenie 0x%08X!\n", GetID());
	}

	return false;
}

CWeenieObject *CWeenieObject::Load(uint32_t weenie_id)
{
#ifndef PUBLIC_BUILD
	CStopWatch watch;
#endif

	unsigned int top_level_object_id = 0;
	unsigned int block_id = 0;
	void *data = NULL;
	unsigned long data_length = 0;

	if (g_pDBIO->GetWeenie(weenie_id, &top_level_object_id, &block_id, &data, &data_length))
	{
		BinaryReader reader(data, data_length);

		CWeenieSave save;
		if (save.UnPack(&reader) && !reader.GetLastError())
		{
			CWeenieObject *weenie = g_pWeenieFactory->CreateBaseWeenieByType(save.m_Qualities.m_WeenieType, save.m_Qualities.id);

			if (weenie)
			{
				weenie->SetID(weenie_id);
				weenie->LoadEx(save);

#ifndef PUBLIC_BUILD
				double elapsed = watch.GetElapsed();
				if (elapsed >= 0.1)
				{
					LOG_PRIVATE(Temp, Warning, "Took %f seconds to load %s\n", elapsed, weenie->GetName().c_str());
				}
#endif

				return weenie;
			}
			else
			{
				LOG_PRIVATE(Temp, Error, "Failed to load abstract weenie 0x%08X!\n", weenie_id);
			}
		}
		else
		{
			LOG_PRIVATE(Temp, Error, "Failed to unpack abstract weenie 0x%08X!\n", weenie_id);
		}
	}
	else
	{
		LOG_PRIVATE(Temp, Error, "Failed to load abstract weenie 0x%08X!\n", weenie_id);
	}

	return NULL;
}

uint32_t CWeenieObject::GetTopLevelID()
{
	uint32_t container_id = InqIIDQuality(CONTAINER_IID, 0);
	if (container_id)
	{
		if (CWeenieObject *pContainer = g_pWorld->FindObject(container_id))
		{
			return pContainer->GetTopLevelID();
		}
		else
		{
			SERVER_ERROR << "Could not find parent container weenie" << container_id << "for" << id << "in GetTopLevelID()";
		}
	}

	uint32_t wielder_id = InqIIDQuality(WIELDER_IID, 0);
	if (wielder_id)
	{
		if (CWeenieObject *pWielder = g_pWorld->FindObject(wielder_id))
		{
			return pWielder->GetTopLevelID();
		}
		else
		{
			SERVER_ERROR << "Could not find parent wielder weenie" << wielder_id << "for" << id << "in GetTopLevelID()"; ;
		}
	}

	uint32_t owner_id = InqIIDQuality(OWNER_IID, 0);
	if (owner_id)
	{
		if (CWeenieObject *pOwner = g_pWorld->FindObject(owner_id))
		{
			return pOwner->GetTopLevelID();
		}
		else
		{
			SERVER_ERROR << "Could not find parent wielder weenie" << owner_id << "for" << id << "in GetTopLevelID()"; ;
		}
	}

	return GetID();
}

int CWeenieObject::UseChecked(CPlayerWeenie *other)
{
	if (IsBusyOrInAction() || IsDead())
		return WERROR_ACTIONS_LOCKED;

	// check restrictions
	int32_t level = 0;
	int32_t level_req = 0;
	int32_t skill = 0;

	if (m_Qualities.InqInt(USE_REQUIRES_LEVEL_INT, level_req, TRUE)
		&& other->InqIntQuality(LEVEL_INT, 0, TRUE) < level_req)
		return WERROR_ITEM_INTERACTION_RESTRICTED;

	if (m_Qualities.InqInt(USE_REQUIRES_SKILL_INT, skill))
	{
		uint32_t skillLevel = 0;
		m_Qualities.InqInt(USE_REQUIRES_SKILL_LEVEL_INT, level_req);
		other->InqSkill((STypeSkill)skill, skillLevel, FALSE);

		if (skillLevel < (uint32_t)level_req)
			return WERROR_ITEM_INTERACTION_RESTRICTED;
	}

	int32_t cooldownId = 0;
	if (m_Qualities.InqInt(SHARED_COOLDOWN_INT, cooldownId, TRUE))
	{
		if (other->m_Qualities.InqCooldown(cooldownId))
			return WERROR_ACTIONS_LOCKED;

		double duration = 0.0;
		if (m_Qualities.InqFloat(COOLDOWN_DURATION_FLOAT, duration, TRUE))
		{
			Enchantment *enchant = other->m_Qualities.AddCooldown(cooldownId, duration);
			if (enchant)
				other->NotifyEnchantmentUpdated(enchant);
		}
	}

	return Use(other);
}

bool CWeenieObject::IsLocked()
{
	return m_Qualities.GetBool(LOCKED_BOOL, FALSE) ? true : false;
}

void CWeenieObject::SetLocked(BOOL locked)
{
	if (g_pConfig->EverythingUnlocked())
	{
		locked = FALSE;
	}

	if (InqBoolQuality(LOCKED_BOOL, FALSE) == locked)
	{
		return;
	}

	m_Qualities.SetBool(LOCKED_BOOL, locked);
	NotifyBoolStatUpdated(LOCKED_BOOL, false);
}

int CWeenieObject::Activate(uint32_t activator_id)
{
	GeneratorRemoveNode(activator_id, (RegenerationType)0xf);

	//activation code copied from CSwitchWeenie::Activate() seems to fix the generators not spawning mobs on Activate_EmoteType used on mobs like Scold that spawn more mobs on death.
	if (get_minterp()->interpreted_state.GetNumActions())
		return WERROR_NONE;

	int activationResponse = InqIntQuality(ACTIVATION_RESPONSE_INT, 0);

	if (uint32_t activation_target_id = InqIIDQuality(ACTIVATION_TARGET_IID, 0))
	{
		CWeenieObject *activation_target = g_pWorld->FindObject(activation_target_id);
		if (activation_target)
			activation_target->Activate(activator_id);
	}

	//if (activationResponse & Activation_CastSpell)
	{
		if (uint32_t spell_did = InqDIDQuality(SPELL_DID, 0))
		{
			MakeSpellcastingManager()->CastSpellInstant(activator_id, spell_did);
		}
	}

	if (activationResponse & Generate_ActivationResponse)
	{
		g_pWeenieFactory->AddFromGeneratorTable(this, false);
	}

	if (activationResponse & Talk_ActivationResponse)
	{
		std::string talkText;
		if (m_Qualities.InqString(ACTIVATION_TALK_STRING, talkText))
		{
			CPlayerWeenie *player = g_pWorld->FindPlayer(activator_id);
			if (player)
				player->SendText(talkText.c_str(), LTT_DEFAULT);
		}
	}

	DoActivationEmote(activator_id);

	return WERROR_NONE;
}

int CWeenieObject::DoUseResponse(CWeenieObject *player)
{
	DoLocalSignal();

	return WERROR_NONE;
}

int CWeenieObject::DoUseWithResponse(CWeenieObject *player, CWeenieObject *with)
{
	if (CPlayerWeenie *player_weenie = player->AsPlayer())
	{
		return player_weenie->UseEx(this, with);
	}

	return WERROR_NONE;
}

bool CWeenieObject::IsExecutingEmote()
{
	if (m_EmoteManager &&
		(m_EmoteManager->IsExecutingAlready() || m_EmoteManager->HasQueue()))
	{
		return true;
	}

	return false;
}

bool CWeenieObject::HasEmoteForID(EmoteCategory emoteCategory, uint32_t item_id)
{
	// check emote item acceptance for category
	PackableList<EmoteSet> *emoteCategoryResult = m_Qualities._emote_table->_emote_table.lookup(emoteCategory);

	if (emoteCategoryResult)
	{
		for (auto &emoteSet : *emoteCategoryResult)
		{
			if (emoteSet.classID == item_id)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CWeenieObject::ChanceExecuteEmoteSet(uint32_t other_id, EmoteCategory category)
{
	//if (IsExecutingEmote())
	//{
	//	return;
	//}

	if (m_Qualities._emote_table)
	{
		PackableList<EmoteSet> *emoteSetList = m_Qualities._emote_table->_emote_table.lookup(category);

		if (emoteSetList)
		{
			double dice = Random::GenFloat(0.0, 1.0);

			for (auto &emoteSet : *emoteSetList)
			{
				if (dice < emoteSet.probability)
				{
					MakeEmoteManager()->ExecuteEmoteSet(emoteSet, other_id);

					break;
				}

				// dice -= emoteSet.probability;
			}
		}
	}
}

void CWeenieObject::DoUseEmote(CWeenieObject *other)
{
	if (IsExecutingEmote())
	{
		if (other)
		{
			other->SendText(csprintf("%s is busy.", GetName().c_str()), LTT_DEFAULT);
		}

		return;
	}

	ChanceExecuteEmoteSet(other->GetID(), Use_EmoteCategory);
}

void CWeenieObject::DoActivationEmote(uint32_t activator_id)
{
	ChanceExecuteEmoteSet(activator_id, Activation_EmoteCategory);
}

void CWeenieObject::DoLocalSignal()
{
	std::string signal;
	if (m_Qualities.InqString(USE_SENDS_SIGNAL_STRING, signal))
	{
		DoLocalSignal(signal);
	}
}

void CWeenieObject::DoLocalSignal(const std::string &signal)
{
	CWeenieObject *source = this;
	if (IsContained())
		source = GetWorldTopLevelContainer();

	std::list<CWeenieObject*> nearby;
	g_pWorld->EnumNearby(source, 24.0f, &nearby);
	for (CWeenieObject* target : nearby)
	{
		target->NotifyLocalSignal(signal, source);
	}
}

void CWeenieObject::NotifyLocalSignal(const std::string &signal, CWeenieObject *sender)
{
	if (InqIntQuality(HEAR_LOCAL_SIGNALS_INT, 0, TRUE))
	{
		bool inrange = true;
		int radius = 0;
		if (m_Qualities.InqInt(HEAR_LOCAL_SIGNALS_RADIUS_INT, radius, TRUE))
		{
			float distance = DistanceTo(sender);
			inrange = (int)distance <= radius;
		}

		if (inrange)
			MakeEmoteManager()->ChanceExecuteEmoteSet(ReceiveLocalSignal_EmoteCategory, signal, sender->GetID());
	}
}

int CWeenieObject::Use(CPlayerWeenie *player)
{
	CGenericUseEvent *useEvent = new CGenericUseEvent();
	useEvent->_target_id = GetID();
	player->ExecuteUseEvent(useEvent);
	return WERROR_NONE;
}

int CWeenieObject::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	if (this == with)
	{
		// don't use self
		player->NotifyInventoryFailedEvent(GetID(), WERROR_NONE);
		return WERROR_NONE;
	}

	CGenericUseEvent *useEvent = new CGenericUseEvent();
	useEvent->_tool_id = GetID();
	useEvent->_target_id = with->GetID();
	useEvent->_do_use_animation = Motion_ClapHands;

	player->ExecuteUseEvent(useEvent);
	return WERROR_NONE;
}

void CWeenieObject::HandleMoveToDone(uint32_t error)
{
	/*if (m_UseManager)
	{
		m_UseManager->HandleMoveToDone(error);
	}*/
	if (m_AttackManager)
	{
		m_AttackManager->HandleMoveToDone(error);
	}
}

BOOL CWeenieObject::IsPK()
{
	return (InqIntQuality(PLAYER_KILLER_STATUS_INT, 0) & PK_PKStatus) ? true : false;
}

BOOL CWeenieObject::IsPKLite()
{
	return (InqIntQuality(PLAYER_KILLER_STATUS_INT, 0) & PKLite_PKStatus) ? true : false;
}

BOOL CWeenieObject::IsImpenetrable()
{
	return (InqIntQuality(PLAYER_KILLER_STATUS_INT, 0) & Free_PKStatus) ? true : false;
}

void CWeenieObject::LeaveFellowship()
{
	if (HasFellowship())
	{
		//_fellowship->Quit(GetID());
		_fellowship = nullptr;

		m_Qualities.RemoveString(FELLOWSHIP_STRING);
	}
}

void CWeenieObject::JoinFellowship(const fellowship_ptr_t &fellowship)
{
	// check behavior for already being in a fellow
	// just part existing until
	LeaveFellowship();

	if (fellowship)
	{
		_fellowship = fellowship;
		m_Qualities.SetString(FELLOWSHIP_STRING, _fellowship->_name);
	}
}

COMBAT_MODE CWeenieObject::GetEquippedCombatMode()
{
	switch (InqIntQuality(COMBAT_USE_INT, COMBAT_USE::COMBAT_USE_NONE))
	{
	case COMBAT_USE_SHIELD:
	case COMBAT_USE_TWO_HANDED:
	case COMBAT_USE_MELEE:
		return COMBAT_MODE::MELEE_COMBAT_MODE;
	case COMBAT_USE_MISSILE:
		return COMBAT_MODE::MISSILE_COMBAT_MODE;
	default:
		if (InqIntQuality(DEFAULT_COMBAT_STYLE_INT, CombatStyle::Undef_CombatStyle) & Magic_CombatStyle)
			return COMBAT_MODE::MAGIC_COMBAT_MODE;
		return COMBAT_MODE::UNDEF_COMBAT_MODE;
	}
}

CWeenieObject *CWeenieObject::GetWorldContainer()
{
	return g_pWorld->FindObject(GetContainerID());
}

CWeenieObject *CWeenieObject::GetWorldWielder()
{
	return g_pWorld->FindObject(GetWielderID());
}

CWeenieObject *CWeenieObject::GetWorldOwner()
{
	if (IsContained())
	{
		if (CWeenieObject *owner = GetWorldContainer())
			return owner;
	}

	if (IsWielded())
	{
		if (CWeenieObject *owner = GetWorldWielder())
			return owner;
	}

	return NULL;
}

CWeenieObject *CWeenieObject::GetWorldTopLevelOwner()
{
	return g_pWorld->FindObject(GetTopLevelID());
}

CContainerWeenie *CWeenieObject::GetWorldTopLevelContainer()
{
	uint32_t top_level_id = GetTopLevelID();

	if (top_level_id != GetID())
	{
		if (CWeenieObject *containerObj = g_pWorld->FindObject(top_level_id))
		{
			if (CContainerWeenie *container = containerObj->AsContainer())
			{
				return container;
			}
		}
	}

	return NULL;
}

void CWeenieObject::ReleaseFromAnyWeenieParent(bool bBroadcastContainerChange, bool bBroadcastEquipmentChange)
{
	if (IsContained())
	{
		if (m_bWorldIsAware && bBroadcastContainerChange)
		{
			BinaryWriter statNotify;
			BYTE statTS = GetNextStatTimestamp(IID_StatType, CONTAINER_IID);
			statNotify.Write<uint32_t>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(CONTAINER_IID);
			statNotify.Write<uint32_t>(0);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}

		if (CWeenieObject *containerObj = GetWorldContainer())
		{
			assert(containerObj->AsContainer());

			if (CContainerWeenie *container = containerObj->AsContainer())
			{
				container->ReleaseContainedItemRecursive(this);
			}
		}

		m_Qualities.SetInstanceID(CONTAINER_IID, 0);
	}

	if (IsWielded())
	{
		if (m_bWorldIsAware && bBroadcastEquipmentChange)
		{
			BinaryWriter statNotify;
			BYTE statTS = GetNextStatTimestamp(IID_StatType, WIELDER_IID);
			statNotify.Write<uint32_t>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<uint32_t>(GetID());
			statNotify.Write<uint32_t>(WIELDER_IID);
			statNotify.Write<uint32_t>(0);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}

		if (CWeenieObject *wielderObj = GetWorldWielder())
		{
			assert(wielderObj->AsContainer());

			OnUnwield(wielderObj);

			if (CContainerWeenie *wielder = wielderObj->AsContainer())
			{
				wielder->ReleaseContainedItemRecursive(this);
			}

			m_Qualities.SetInstanceID(WIELDER_IID, 0);

			// remove any enchantments
			if (wielderObj->m_Qualities._enchantment_reg)
			{
				PackableListWithJson<uint32_t> spells_to_remove;

				if (wielderObj->m_Qualities._enchantment_reg->_add_list)
				{
					for (const auto &entry : *wielderObj->m_Qualities._enchantment_reg->_add_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (wielderObj->m_Qualities._enchantment_reg->_mult_list)
				{
					for (const auto &entry : *wielderObj->m_Qualities._enchantment_reg->_mult_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (wielderObj->m_Qualities._enchantment_reg->_cooldown_list)
				{
					for (const auto &entry : *wielderObj->m_Qualities._enchantment_reg->_cooldown_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (!spells_to_remove.empty())
				{
					wielderObj->m_Qualities._enchantment_reg->RemoveEnchantments(&spells_to_remove);

					if (wielderObj->AsPlayer())
					{
						BinaryWriter silentRemoveMessage;
						silentRemoveMessage.Write<uint32_t>(0x2C8);
						spells_to_remove.Pack(&silentRemoveMessage);

						wielderObj->SendNetMessage(&silentRemoveMessage, PRIVATE_MSG, TRUE, FALSE);
					}

					wielderObj->CheckVitalRanges();
				}


			}

			if (m_Qualities._enchantment_reg)
			{
				PackableListWithJson<uint32_t> spells_to_remove;

				if (m_Qualities._enchantment_reg->_add_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_add_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (m_Qualities._enchantment_reg->_mult_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_mult_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (m_Qualities._enchantment_reg->_cooldown_list)
				{
					for (const auto &entry : *m_Qualities._enchantment_reg->_cooldown_list)
					{
						if (entry._caster == GetID())
						{
							spells_to_remove.push_back(entry._id);
						}
					}
				}

				if (!spells_to_remove.empty())
				{
					m_Qualities._enchantment_reg->RemoveEnchantments(&spells_to_remove);

					CheckVitalRanges();
				}
			}



			int gearRating;
			for (int i = 370; i < 380; i++)
			{
				if (m_Qualities.InqInt((STypeInt)i, gearRating) && gearRating > 0)
				{
					wielderObj->AsMonster()->UpdateRatingFromGear((STypeInt)i, -gearRating);
				}
			}

			// Remove Damage Rating for Weapon Mastery.
			if (m_Qualities.GetInt(WEAPON_TYPE_INT, 0))
			{
				int rating;
				if (wielderObj->m_Qualities.InqInt(DAMAGE_RATING_INT, rating, true))
				{
					wielderObj->m_Qualities.SetInt(DAMAGE_RATING_INT, max(rating - 5, 0));
					wielderObj->NotifyIntStatUpdated(DAMAGE_RATING_INT);
				}
			}
				

		}
	}

	m_Qualities.SetInstanceID(OWNER_IID, 0);

	_cachedHasOwner = false;
}

CWeenieObject *CWeenieObject::FindContained(uint32_t object_id)
{
	return NULL;
}

void CWeenieObject::SetWeenieContainer(uint32_t container_id)
{
	m_Qualities.SetInstanceID(CONTAINER_IID, container_id);
	NotifyIIDStatUpdated(CONTAINER_IID, false);

	RecacheHasOwner();
}

void CWeenieObject::SetWielderID(uint32_t wielder_id)
{
	m_Qualities.SetInstanceID(WIELDER_IID, wielder_id);
	NotifyIIDStatUpdated(WIELDER_IID, false);

	RecacheHasOwner();
}

bool CWeenieObject::HasOwner()
{
	RecacheHasOwner();
	return _cachedHasOwner;
}

bool CWeenieObject::CachedHasOwner()
{
	return _cachedHasOwner;
}

void CWeenieObject::RecacheHasOwner()
{
	_cachedHasOwner = (IsContained() || IsWielded()) ? true : false;
}

void CWeenieObject::SetWieldedLocation(uint32_t inv_loc)
{
	m_Qualities.SetInt(CURRENT_WIELDED_LOCATION_INT, inv_loc);
	NotifyIntStatUpdated(CURRENT_WIELDED_LOCATION_INT, false);
}

bool CWeenieObject::IsValidWieldLocation(uint32_t location)
{
	if (InqIntQuality(LOCATIONS_INT, 0, TRUE) & location)
		return true;

	return false;
}

bool CWeenieObject::CanEquipWith(CWeenieObject *other, uint32_t otherLocation)
{
	int loc = InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0, TRUE);
	//int allLoc = InqIntQuality(LOCATIONS_INT, 0, TRUE);

	//int otherAllLoc = other->InqIntQuality(LOCATIONS_INT, 0, TRUE);
	//int otherLoc = other->InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0, TRUE);

	//WINLOG(Object, Normal, "Can Equip %s => %s || %08x %08x => %08x %08x (%08x)\n",
	//	GetName().c_str(), other->GetName().c_str(),
	//	allLoc, loc, otherAllLoc, otherLoc, otherLocation);

	// if this object covers other location, no
	if (loc & otherLocation)
	{
		return false;
	}

	// if the other item wants to be in the left hand slot
	if (otherLocation == SHIELD_LOC)
	{
		// if this object is a bow/thrown item or two-hander
		if (loc & (HELD_LOC | MISSILE_WEAPON_LOC | TWO_HANDED_LOC))
		{
			return false;
		}
	}

	return true;
}

int CWeenieObject::SimulateGiveObject(CContainerWeenie *target_container, CWeenieObject *object_weenie)
{
	if (!target_container->Container_CanStore(object_weenie))
		return WERROR_GIVE_NOT_ALLOWED;

	ReleaseContainedItemRecursive(object_weenie);

	object_weenie->m_Qualities.SetInstanceID(CONTAINER_IID, target_container->GetID());
	object_weenie->_cachedHasOwner = true;

	SendNetMessage(InventoryMove(object_weenie->GetID(), target_container->GetID(), 0, object_weenie->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);

	target_container->MakeAware(object_weenie, true);
	target_container->OnReceiveInventoryItem(this, object_weenie, 0);

	if (!InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE) && !target_container->InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE))
	{
		target_container->EmitSound(Sound_ReceiveItem, 1.0f);

		if (IsCreature())
		{
			int amount = object_weenie->InqIntQuality(STACK_SIZE_INT, 1);
			if (amount > 1 && !(m_Qualities._emote_table && HasEmoteForID(Refuse_EmoteCategory, object_weenie->m_Qualities.id)))
			{
				SendText(csprintf("You give %s %s %s.", target_container->GetName().c_str(), FormatNumberString(amount).c_str(), object_weenie->GetPluralName().c_str()), LTT_DEFAULT);
				target_container->SendText(csprintf("%s gives you %s %s.", GetName().c_str(), FormatNumberString(amount).c_str(), object_weenie->GetPluralName().c_str()), LTT_DEFAULT);
			}
			else
			{
				// check emote Refusal here to avoid the extra "%s gives you %s." text on Refuse Emotes.
				if (m_Qualities._emote_table && HasEmoteForID(Refuse_EmoteCategory, object_weenie->m_Qualities.id))
				{
					SendText(csprintf("You allow %s to examine your %s.", target_container->GetName().c_str(), object_weenie->GetName().c_str()), LTT_DEFAULT);
				}
				else
				{
					SendText(csprintf("You give %s %s.", target_container->GetName().c_str(), object_weenie->GetName().c_str()), LTT_DEFAULT);
					target_container->SendText(csprintf("%s gives you %s.", GetName().c_str(), object_weenie->GetName().c_str()), LTT_DEFAULT);
				}
			}
		}
	}

	target_container->DebugValidate();
	DebugValidate();

	return WERROR_NONE;
}

void CWeenieObject::SimulateGiveObject(class CContainerWeenie *target_container, uint32_t wcid, int amount, int ptid, float shade, int bondedType)
{
	CWeenieObject *object_weenie = g_pWeenieFactory->CreateWeenieByClassID(wcid, NULL, false);

	if (!object_weenie)
		return;

	if (ptid)
	{
		object_weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);

		if (shade > 0.0)
			object_weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);
	}

	if (bondedType != 0)
		object_weenie->m_Qualities.SetInt(BONDED_INT, bondedType);

	if (amount > 1)
	{
		int maxStackSize = object_weenie->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 1);
		if (amount <= maxStackSize)
			object_weenie->SetStackSize(amount);
		else
		{
			int amountOfStacks = amount / maxStackSize;
			int restStackSize = amount % maxStackSize;
			for (int i = 0; i < amountOfStacks; i++)
				SimulateGiveObject(target_container, wcid, maxStackSize, ptid, shade, bondedType);
			if (restStackSize > 0)
				object_weenie->SetStackSize(restStackSize);
			else
			{
				delete object_weenie;
				return;
			}
		}
	}

	object_weenie->SetID(g_pWorld->GenerateGUID(eDynamicGUID));

	if (!g_pWorld->CreateEntity(object_weenie, false))
		return;

	if (!target_container->Container_CanStore(object_weenie))
	{
		g_pWorld->RemoveEntity(object_weenie);
		return;
	}

	ReleaseContainedItemRecursive(object_weenie);

	object_weenie->m_Qualities.SetInstanceID(CONTAINER_IID, target_container->GetID());
	object_weenie->_cachedHasOwner = true;

	SendNetMessage(InventoryMove(object_weenie->GetID(), target_container->GetID(), 0, object_weenie->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);

	target_container->MakeAware(object_weenie, true);
	target_container->OnReceiveInventoryItem(this, object_weenie, 0);

	if (!InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE) && !target_container->InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE))
	{
		target_container->EmitSound(Sound_ReceiveItem, 1.0f);

		if (IsCreature())
		{
			int amount = object_weenie->InqIntQuality(STACK_SIZE_INT, 1);
			if (amount > 1 && !(m_Qualities._emote_table && HasEmoteForID(Refuse_EmoteCategory, object_weenie->m_Qualities.id)))
			{
				SendText(csprintf("You give %s %s %s.", target_container->GetName().c_str(), FormatNumberString(amount).c_str(), object_weenie->GetPluralName().c_str()), LTT_DEFAULT);
				target_container->SendText(csprintf("%s gives you %s %s.", GetName().c_str(), FormatNumberString(amount).c_str(), object_weenie->GetPluralName().c_str()), LTT_DEFAULT);
			}
			else
			{
				SendText(csprintf("You give %s %s.", target_container->GetName().c_str(), object_weenie->GetName().c_str()), LTT_DEFAULT);
				target_container->SendText(csprintf("%s gives you %s.", GetName().c_str(), object_weenie->GetName().c_str()), LTT_DEFAULT);
			}
		}
	}

	target_container->DebugValidate();
	DebugValidate();
}

void CWeenieObject::SendNetMessage(void *_data, uint32_t _len, WORD _group, BOOL _event, bool ephemeral)
{
}

void CWeenieObject::SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event, BOOL should_delete, bool ephemeral)
{
	if (should_delete)
	{
		delete _food;
	}
}

void CWeenieObject::SendNetMessageToTopMost(void *_data, uint32_t _len, WORD _group, BOOL _event, bool ephemeral)
{
	if (AsPlayer())
	{
		// obviously we are the top most
		SendNetMessage(_data, _len, _group, _event, ephemeral);
		return;
	}

	if (CWeenieObject *topMost = GetWorldTopLevelOwner())
	{
		// we are wielded or contained, send to topmost owner
		topMost->SendNetMessage(_data, _len, _group, _event, ephemeral);
		return;
	}
}

void CWeenieObject::SendNetMessageToTopMost(BinaryWriter *_food, WORD _group, BOOL _event, BOOL should_delete, bool ephemeral)
{
	if (AsPlayer())
	{
		// obviously we are the top most
		SendNetMessage(_food, _group, _event, should_delete, ephemeral);
		return;
	}

	if (CWeenieObject *topMost = GetWorldTopLevelOwner())
	{
		// we are wielded or contained, send to topmost owner
		topMost->SendNetMessage(_food, _group, _event, should_delete, ephemeral);
		return;
	}

	if (should_delete)
	{
		delete _food;
	}
}

int CWeenieObject::CraftObject(CContainerWeenie *target_container, CWeenieObject *object_weenie)
{
	if (!target_container->Container_CanStore(object_weenie))
		return WERROR_GIVE_NOT_ALLOWED;

	object_weenie->m_Qualities.SetInstanceID(CONTAINER_IID, target_container->GetID());
	object_weenie->_cachedHasOwner = true;

	SendNetMessage(InventoryMove(object_weenie->GetID(), target_container->GetID(), 0, object_weenie->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);

	target_container->MakeAware(object_weenie, true);
	target_container->OnReceiveInventoryItem(this, object_weenie, 0);

	return WERROR_NONE;
}

void CWeenieObject::CheckDeath(CWeenieObject *source, DAMAGE_TYPE dt)
{
	if (IsDead())
	{
		if (source == this)
		{
			NotifyVictimEvent("You died!");

			if (_IsPlayer())
			{
				NotifyDeathMessage(source->GetID(), csprintf("%s died!", GetName().c_str()));
			}
		}
		else
		{
			NotifyVictimEvent(csprintf("You were killed by %s!", source->GetName().c_str()));
			source->NotifyKillerEvent(csprintf("You killed %s!", GetName().c_str()));

			if (_IsPlayer())
			{
				NotifyDeathMessage(source->GetID(), csprintf("%s killed %s!", source->GetName().c_str(), GetName().c_str()));
			}

			// notify the atttacker they did damage
			source->GivePerksForKill(this);
		}

		OnDeath(source->GetID());
	}
}

uint32_t CWeenieObject::GetMagicDefense()
{
	uint32_t defenseSkill = 0;
	InqSkill(MAGIC_DEFENSE_SKILL, defenseSkill, FALSE);
	return defenseSkill;
}

bool CWeenieObject::TryMagicResist(uint32_t magicSkill)
{
	uint32_t defenseSkill = GetMagicDefense();

	double defenseMod = GetMagicDefenseModUsingWielded();
	if (AsContainer())
	{
		for (auto item : AsContainer()->m_Wielded)
		{
			if (item->GetImbueEffects() & MagicDefense_ImbuedEffectType)
				defenseSkill += 1;
		}
	}
	defenseSkill = (int)round((double)defenseSkill * defenseMod);

	return ::TryMagicResist(magicSkill, defenseSkill);
}

double CWeenieObject::GetMeleeDefenseModUsingWielded()
{
	return GetMeleeDefenseMod();
}

double CWeenieObject::GetMeleeDefenseMod()
{
	// Don't enchant Ammunition
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
		return 1.0;

	double mod = InqFloatQuality(WEAPON_DEFENSE_FLOAT, 0.0, TRUE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_DEFENSE_FLOAT, &mod);

	if (_IsPlayer())
	{
		double mod = 0.0;
		if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->EnchantFloat(WEAPON_AURA_DEFENSE_FLOAT, &mod))
		{
			return mod;
		}
	}

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod += (wielder->GetMeleeDefenseMod());
	}
	return mod;
}

double CWeenieObject::GetMissileDefenseModUsingWielded()
{
	return GetMissileDefenseMod();
}

double CWeenieObject::GetMissileDefenseMod()
{
	double mod = InqFloatQuality(WEAPON_MISSILE_DEFENSE_FLOAT, 1.0, TRUE);

	// Don't enchant Ammunition
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
		return mod;

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_MISSILE_DEFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod += (wielder->GetMissileDefenseMod()) - 1.0;
	}

	return mod;
}

double CWeenieObject::GetMagicDefenseModUsingWielded()
{
	return GetMagicDefenseMod();
}

double CWeenieObject::GetMagicDefenseMod()
{
	double mod = InqFloatQuality(WEAPON_MAGIC_DEFENSE_FLOAT, 1.0, TRUE);

	// Don't enchant Ammunition
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
		return mod;

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_MAGIC_DEFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod += (wielder->GetMagicDefenseMod()) - 1.0;
	}

	return mod;
}

double CWeenieObject::GetOffenseMod()
{
	double mod = InqFloatQuality(WEAPON_OFFENSE_FLOAT, 0.0, TRUE);

	// Don't enchant Ammunition
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
		return mod;

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_OFFENSE_FLOAT, &mod);

	if (_IsPlayer())
	{
		mod = 0.0;
		if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->EnchantFloat(WEAPON_AURA_OFFENSE_FLOAT, &mod))
		{
			return mod;
		}
	}

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod += (wielder->GetOffenseMod());
	}

	return mod;
}

double CWeenieObject::GetCrushingBlowMultiplier()
{
	double mod = InqFloatQuality(CRITICAL_MULTIPLIER_FLOAT, 0.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(CRITICAL_MULTIPLIER_FLOAT, &mod);

	return mod;
}

double CWeenieObject::GetBitingStrikeFrequency()
{
	double mod = InqFloatQuality(CRITICAL_FREQUENCY_FLOAT, 0.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(CRITICAL_FREQUENCY_FLOAT, &mod);

	return mod;
}

void CWeenieObject::AddImbueEffect(ImbuedEffectType effect)
{
	uint32_t imbueEffects = GetImbueEffects();

	if (imbueEffects & effect)
		return; //it's already present.

	imbueEffects |= effect;

	if (!InqIntQuality(IMBUED_EFFECT_INT, 0, FALSE))
		m_Qualities.SetInt(IMBUED_EFFECT_INT, (int)effect);
	else if (!InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE))
		m_Qualities.SetInt(IMBUED_EFFECT_2_INT, (int)effect);
	else if (!InqIntQuality(IMBUED_EFFECT_3_INT, 0, FALSE))
		m_Qualities.SetInt(IMBUED_EFFECT_3_INT, (int)effect);
	else if (!InqIntQuality(IMBUED_EFFECT_4_INT, 0, FALSE))
		m_Qualities.SetInt(IMBUED_EFFECT_4_INT, (int)effect);
	else if (!InqIntQuality(IMBUED_EFFECT_5_INT, 0, FALSE))
		m_Qualities.SetInt(IMBUED_EFFECT_5_INT, (int)effect);
	else
		return;
	return;
}

bool CWeenieObject::IsCovenantArmorShield()
{
	if (m_Qualities.id >= 21156 && m_Qualities.id == 21159)
		return true;

	if (m_Qualities.id >= 40695 && m_Qualities.id == 40714)
		return true;

	return false;
}

bool CWeenieObject::IsArmor()
{
	int retval = 0;
	if (m_Qualities.InqInt(ITEM_TYPE_INT, retval) && retval == 2)
		return true;
	return false;
}


uint32_t CWeenieObject::GetImbueEffects()
{
	uint32_t imbuedEffect = 0;

	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_INT, 0, FALSE);
	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE);
	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE);
	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_3_INT, 0, FALSE);
	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_4_INT, 0, FALSE);
	imbuedEffect |= (uint32_t)InqIntQuality(IMBUED_EFFECT_5_INT, 0, FALSE);

	return imbuedEffect;
}

double CWeenieObject::GetManaConversionMod()
{
	double mod = InqFloatQuality(MANA_CONVERSION_MOD_FLOAT, 0.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(MANA_CONVERSION_MOD_FLOAT, &mod);

	if (_IsPlayer())
	{
		mod = 1.0;
		if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->EnchantFloat(WEAPON_AURA_MANA_CONV_FLOAT, &mod))
		{
			return mod;
		}
	}

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= wielder->GetManaConversionMod();
	}

	return mod;
}

uint32_t CWeenieObject::GetEffectiveManaConversionSkill()
{
	SKILL_ADVANCEMENT_CLASS sac;
	uint32_t manaConvSkill = 0;

	//mana conversion is an unuseable untrained skill, which means it's always 0 if you don't have it at least trained.
	if (!m_Qualities.InqSkillAdvancementClass(MANA_CONVERSION_SKILL, sac) || sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
		InqSkill(MANA_CONVERSION_SKILL, manaConvSkill, FALSE);

	if (CWeenieObject *wand = GetWieldedCaster())
	{
		double manaMod = wand->GetManaConversionMod();

		if (manaMod > 0.0)
		{
			manaConvSkill = (uint32_t)max(0, (int)(manaConvSkill * (1.0 + manaMod)));
		}
	}

	return manaConvSkill;
}

int CWeenieObject::GetAttackTime()
{
	int speed = 0;
	m_Qualities.InqInt(WEAPON_TIME_INT, speed, TRUE, TRUE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantInt(WEAPON_TIME_INT, &speed, TRUE);

	if (_IsPlayer())
	{
		speed = 0.0;
		if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->EnchantInt(WEAPON_AURA_SPEED_INT, &speed, true))
		{
			return speed;
		}
	}

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		speed += wielder->GetAttackTime();
	}

	/*
	if (speed < 0)
	speed = 0;
	if (speed > 100)
	speed = 100;
	*/

	// 0 = 2.0
	// 100 = 1.0
	// 1.0 / (1.0 + ((100 - speed) * (0.0075)))
	return speed;
}

double CWeenieObject::GetManaCon()
{
	CWeenieObject *wielder = GetWorldWielder();

	double manacon = 0;
	m_Qualities.InqFloat(MANA_CONVERSION_MOD_FLOAT, manacon, TRUE);

	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		if (wielder->m_Qualities._enchantment_reg)
			wielder->m_Qualities._enchantment_reg->EnchantFloat(MANA_CONVERSION_MOD_FLOAT, &manacon);
	}

	return manacon;
}

int CWeenieObject::GetAttackTimeUsingWielded()
{
	return GetAttackTime();
}

int CWeenieObject::GetAttackDamage(bool isAssess)
{
	int damage = InqIntQuality(DAMAGE_INT, 0, TRUE);

	// Don't enchant Ammunition instead look up the damage of the launcher
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
	{
		CAmmunitionWeenie *missile = AsAmmunition();

		if (missile && missile->_launcherID)
		{
			CWeenieObject *launcher = g_pWorld->FindObject(missile->_launcherID);
			if (launcher)
			{
				damage += launcher->GetAttackDamage();

				int launcherElement = launcher->m_Qualities.GetInt(DAMAGE_TYPE_INT, 0);
				int ammoElement = m_Qualities.GetInt(DAMAGE_TYPE_INT, 0);

				if (!isAssess)
				{
					if (ammoElement == launcherElement || ammoElement == BASE_DAMAGE_TYPE)
					{
						if (launcher->m_Qualities.GetInt(ELEMENTAL_DAMAGE_BONUS_INT, 0))
							damage += launcher->GetElementalDamageBonus();
					}
				}
				return damage;
			}
		}
	}

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantInt(DAMAGE_INT, &damage, FALSE);

	int auraDam;
	if (AsPlayer())
	{
		int auraDam = 0;
		if (m_Qualities._enchantment_reg)
			m_Qualities._enchantment_reg->EnchantInt(WEAPON_AURA_DAMAGE_INT, &auraDam, FALSE);
		damage += auraDam;
	}
	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		damage += wielder->GetAttackDamage();
	}

	return damage;
}


double CWeenieObject::GetElementalDamageMod()
{
	
	double elementdmg = InqFloatQuality(ELEMENTAL_DAMAGE_MOD_FLOAT, 0.0, true);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(ELEMENTAL_DAMAGE_MOD_FLOAT, &elementdmg);

	if (_IsPlayer())
	{
		double auraed = 0;
		if (m_Qualities._enchantment_reg && m_Qualities._enchantment_reg->EnchantFloat(WEAPON_AURA_ELEMENTAL_FLOAT, &auraed))
		{
			return auraed;
		}
	}

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		elementdmg += wielder->GetElementalDamageMod();
	}

	return elementdmg;
}


int CWeenieObject::GetElementalDamageBonus()
{
	int damageBonus = InqIntQuality(ELEMENTAL_DAMAGE_BONUS_INT, 0, TRUE);

	// Ammunition shouldn't have an elemental damage bonus.
	if (m_Qualities.m_WeenieType == Ammunition_WeenieType)
		return damageBonus;

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantInt(ELEMENTAL_DAMAGE_BONUS_INT, &damageBonus, FALSE);

	return damageBonus;
}



bool CWeenieObject::IsInPeaceMode()
{
	return get_minterp()->interpreted_state.current_style == Motion_NonCombat;
}

bool CWeenieObject::TryAttackEvade(uint32_t attackSkill, STypeSkill defSkill)
{
	if (GetStamina() < 1) // when we're out of stamina our defense skill is lowered to 0.
		return false;

	uint32_t defenseSkill = 0;
	InqSkill(defSkill, defenseSkill, FALSE);

	double defenseMod = 1.0;
	
	if (AsMonster() && AsMonster()->HasWielded())
	{
		if (defSkill == STypeSkill::MELEE_DEFENSE_SKILL)
			defenseMod = GetMeleeDefenseModUsingWielded();
		else
			defenseMod = GetMissileDefenseModUsingWielded();
	}

	defenseMod *= GetCurrentMotionMod();
	if(AsPlayer())
		defenseMod *= GetCurrentTargetMod();

	if (AsPlayer() && AsPlayer()->IsLoggingOut())
		defenseMod *= 0.25;

	defenseSkill = (int)round((double)defenseSkill * defenseMod * CalculateLoadImpactOnDefense());
	bool success = ::TryMeleeEvade(attackSkill, defenseSkill);

	if (success && _IsPlayer())
	{
		CalculateStaminaLossFromAttack(defSkill);
	}
	else
		AdjustStamina(-1); // while in combat all evasion failures consume 1 stamina.

	return success;
}

double CWeenieObject::GetCurrentMotionMod()
{
	double defenseMod = 1.0;

	if (IsInPeaceMode())
	{
		switch (get_minterp()->interpreted_state.forward_command)
		{
		case Motion_Sleeping:
			defenseMod = 0.1;
			break;
		case Motion_Falling:
		case Motion_Sitting:
			defenseMod = 0.25;
			break;
		case Motion_Crouch:
			defenseMod = 0.5;
			break;
		default:
			defenseMod = 0.8;
			break;
		}
	}
	else
	{
		switch (get_minterp()->interpreted_state.forward_command)
		{
		case Motion_Falling:
		case Motion_Sitting:
		case Motion_Crouch:
		case Motion_Sleeping:
			defenseMod = 0.4;
			break;
		}
	}
	return defenseMod;
}

double CWeenieObject::GetCurrentTargetMod()
{
	uint32_t attacker = m_Qualities.GetIID(CURRENT_ATTACKER_IID, 0);
	uint32_t target = m_Qualities.GetIID(CURRENT_COMBAT_TARGET_IID, 0);
	if (attacker && target && attacker == target)
		return 1.15;

	return 1.0;
}

void CWeenieObject::CalculateStaminaLossFromAttack(STypeSkill skill)
{
	//the higher a player's Endurance, the more likely they are not to use a point of stamina to successfully evade a missile or melee attack.
	//A player is required to have Melee Defense for melee attacks or Missile Defense for missile attacks trained or specialized in order 
	//for this specific ability to work. This benefit is tied to Endurance only, and it caps out at around a 75% chance to avoid losing
	//a point of stamina per successful evasion. 
	SKILL_ADVANCEMENT_CLASS defenseSkillSAC = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
	m_Qualities.InqSkillAdvancementClass(skill, defenseSkillSAC);

	if (get_minterp()->interpreted_state.current_style != Motion_NonCombat) //no stamina usage if we're out of combat mode.
	{
		if (defenseSkillSAC >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
		{
			uint32_t endurance = 0;
			m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);

			float noStamUseChance = 0;
			if (endurance >= 50)
			{
				noStamUseChance = ((float)(endurance * endurance) * 0.000005) + ((float)endurance * 0.00124) - 0.07; // Better curve and caps at 300 End vs 400 End
			}
			noStamUseChance = min(noStamUseChance, 0.75f);
			if (Random::RollDice(0.0, 1.0) > noStamUseChance)
				AdjustStamina(-1); // failed the roll, use stamina.
		}
		else
			AdjustStamina(-1); // defense skill not trained/specialized, use stamina.
	}
}

float CWeenieObject::CalculateLoadImpactOnDefense()
{
	float burden = 0.0f;
	m_Qualities.InqLoad(burden);
	if (burden >= 2.0)
		return 0;
	if (burden <= 1.0)
		return 1;
	float returnv = 1 - std::abs(1.0 - burden);
	return returnv;
}

void CWeenieObject::HandleAggro(CWeenieObject *attacker)
{
}

void CWeenieObject::OnWield(CWeenieObject *wielder)
{
	if (wielder)
		ChanceExecuteEmoteSet(wielder->GetID(), Wield_EmoteCategory);
}

void CWeenieObject::OnUnwield(CWeenieObject *wielder)
{
	if (wielder)
		ChanceExecuteEmoteSet(wielder->GetID(), UnWield_EmoteCategory);
}

void CWeenieObject::OnPickedUp(CWeenieObject *pickedUpBy)
{
	if (pickedUpBy)
		ChanceExecuteEmoteSet(pickedUpBy->GetID(), PickUp_EmoteCategory);
}

void CWeenieObject::OnDropped(CWeenieObject *droppedBy)
{
	if (droppedBy)
		ChanceExecuteEmoteSet(droppedBy->GetID(), Drop_EmoteCategory);
}

void CWeenieObject::OnTeleported()
{
	if (m_UseManager)
		m_UseManager->Cancel();

	if (m_SpellcastingManager)
		m_SpellcastingManager->Cancel();

	if (m_EmoteManager && weenie_obj->_IsPlayer())
		m_EmoteManager->Cancel();

	if (m_AttackManager)
		m_AttackManager->Cancel();

	if (AsPlayer())
		AsPlayer()->CancelLifestoneProtection(); //terminate lifestone protection on teleport (catch all for portal use and /recalls. Casted portals are captured in SpellCastingManager.cpp)

	if (AsPlayer())
		m_Qualities.SetFloat(LAST_TELEPORT_START_TIMESTAMP_FLOAT, Timer::cur_time + 5.0);
}

void CWeenieObject::Movement_Teleport(const Position &position, bool bWasDeath)
{
	assert(position.objcell_id);
	assert(!parent);

	OnTeleported();

	last_move_was_autonomous = false;

	uint32_t dwOldLastCell = GetLandcell();

	if (weenie_obj && weenie_obj->_IsPlayer())
		EnterPortal(dwOldLastCell);

	SetPositionStruct sps;
	sps.pos = position;
	sps.pos.frame.m_origin.z += 0.008f;
	sps.SetFlags(SEND_POSITION_EVENT_SPF | SLIDE_SPF | PLACEMENT_SPF | TELEPORT_SPF);
	SetPosition(sps);

	_teleport_timestamp += 2;

	if (bWasDeath)
	{
		// Send position and movement -- only seen this happen on death so far
		BinaryWriter positionAndMovement;
		positionAndMovement.Write<uint32_t>(0xF619);
		positionAndMovement.Write<uint32_t>(GetID());

		PositionPack position;
		position.position = m_Position;
		position.has_contact = (transient_state & CONTACT_TS && transient_state & ON_WALKABLE_TS) ? TRUE : FALSE;
		position.placement_id = GetActivePlacementFrameID();
		position.instance_timestamp = _instance_timestamp;
		position.position_timestamp = ++_position_timestamp;
		position.teleport_timestamp = _teleport_timestamp;
		// position.velocity = pEntity->m_velocityVector;
		position.force_position_timestamp = _force_position_timestamp;
		position.Pack(&positionAndMovement);

		positionAndMovement.Write<WORD>(++_movement_timestamp);
		positionAndMovement.Write<WORD>(_server_control_timestamp);
		positionAndMovement.Write<BYTE>(last_move_was_autonomous);
		positionAndMovement.Align();

		// last_move_was_autonomous = true;

		BinaryWriter *AnimInfo = Animation_GetAnimationInfo();
		positionAndMovement.Write(AnimInfo);
		delete AnimInfo;

		g_pWorld->BroadcastPVS(dwOldLastCell, positionAndMovement.GetData(), positionAndMovement.GetSize());
	}

	Movement_UpdatePos();
}

bool CWeenieObject::IsAttacking()
{
	return (m_AttackManager && m_AttackManager->IsAttacking());
}

bool CWeenieObject::TryMeleeAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion)
{
	return true;
}

void CWeenieObject::TryMissileAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion)
{
}

int CWeenieObject::GetStructureNum()
{
	int numUses = 1;
	m_Qualities.InqInt(STRUCTURE_INT, numUses, TRUE);
	return numUses;
}

int CWeenieObject::GetStackOrStructureNum()
{
	int dummy;
	if (m_Qualities.InqInt(STRUCTURE_INT, dummy, TRUE))
		return dummy;
	else
		return InqIntQuality(STACK_SIZE_INT, 1);
}

void CWeenieObject::DecrementStackOrStructureNum(int amount, bool bDestroyOnZero)
{
	int dummy;
	if (m_Qualities.GetBool(UNLIMITED_USE_BOOL, 0) == TRUE)
		return;
	if (m_Qualities.InqInt(STRUCTURE_INT, dummy, TRUE))
		DecrementStructureNum(amount, bDestroyOnZero);
	else
		DecrementStackNum(amount, bDestroyOnZero);
}

void CWeenieObject::DecrementStackNum(int amount, bool bDestroyOnZero)
{
	int usesLeft = InqIntQuality(STACK_SIZE_INT, 1);

	usesLeft -= amount;

	SetStackSize(usesLeft);

	if (usesLeft <= 0)
	{

		if (bDestroyOnZero)
			Remove();
		else
			SetStackSize(0);
	}
}

void CWeenieObject::DecrementStructureNum(int amount, bool bDestroyOnZero)
{
	int usesLeft = InqIntQuality(STRUCTURE_INT, 1);

	usesLeft -= amount;

	if (usesLeft <= 0)
	{
		m_Qualities.SetInt(STRUCTURE_INT, 0);
		if (bDestroyOnZero)
			Remove();
		else
			NotifyIntStatUpdated(STRUCTURE_INT, false);
	}
	else
	{
		m_Qualities.SetInt(STRUCTURE_INT, usesLeft);
		NotifyIntStatUpdated(STRUCTURE_INT, false);
	}
}

void CWeenieObject::SetStackSize(int stackSize)
{
	if (!m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0))
	{
		// not a stackable... ??
		return;
	}
	CWeenieDefaults *weenieDefs = g_pWeenieFactory->GetWeenieDefaults(m_Qualities.GetID());

	m_Qualities.SetInt(STACK_SIZE_INT, stackSize);
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(STACK_SIZE_INT, false);

	m_Qualities.SetInt(ENCUMB_VAL_INT, stackSize *  weenieDefs->m_Qualities.GetInt(STACK_UNIT_ENCUMB_INT, 0));
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(ENCUMB_VAL_INT, false);

	m_Qualities.SetInt(VALUE_INT, stackSize * weenieDefs->m_Qualities.GetInt(STACK_UNIT_VALUE_INT, 0));
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(VALUE_INT, false);

	if (m_bWorldIsAware)
		NotifyStackSizeUpdated(false);

	if (CWeenieObject *owner = GetWorldTopLevelOwner())
	{
		owner->RecalculateEncumbrance();
		//ToDo: figure out a method to allow this to work while not interfering with pyreal count when buying MMDs.
		if (owner->AsPlayer() && m_Qualities.id == W_COINSTACK_CLASS)
			owner->RecalculateCoinAmount(W_COINSTACK_CLASS);
	}
}

bool CWeenieObject::IsAvatarJumpsuit()
{
	return m_Qualities.id >= 7000000 && m_Qualities.id <= 7010000;
}

bool CWeenieObject::LearnSpell(uint32_t spell_id, bool showTextAndEffect)
{
	if (spell_id)
	{
		if (const CSpellBase *spell = MagicSystem::GetSpellTable()->GetSpellBase(spell_id))
		{
			if (m_Qualities._spell_book && m_Qualities._spell_book->Exists(spell_id))
			{
				if (showTextAndEffect)
				{
					SendText("You already know that spell!", LTT_DEFAULT);
				}
			}
			else
			{
				m_Qualities.AddSpell(spell_id);

				BinaryWriter addSpellToSpellbook;
				addSpellToSpellbook.Write<uint32_t>(0x2C1);
				addSpellToSpellbook.Write<uint32_t>(spell_id);
				SendNetMessage(&addSpellToSpellbook, PRIVATE_MSG, TRUE, FALSE);

				if (showTextAndEffect)
				{
					SendText(csprintf("You learn the %s spell.", spell->_name.c_str()), LTT_DEFAULT);
					EmitEffect(PS_SkillUpPurple, 1.0f);
				}
				return true;
			}
		}
	}

	return false;
}

void CWeenieObject::Remove()
{
	if (IsContained())
	{
		CWeenieObject *owner = GetWorldTopLevelOwner();
		if (owner)
		{
			owner->ReleaseContainedItemRecursive(this);
			owner->NotifyContainedItemRemoved(GetID(), false);
		}
	}
	else
		NotifyObjectRemoved();

	//if (!sourceItem->HasOwner())
	//{
	if (CWeenieObject *generator = g_pWorld->FindObject(InqIIDQuality(GENERATOR_IID, 0)))
		generator->NotifyGeneratedPickedUp(this);
	//}

	//not sure which ones of the following are necessary but for now this works.
	ReleaseFromAnyWeenieParent(false, true);
	SetWielderID(0);
	SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
	m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
	unset_parent();
	ReleaseFromBlock();
	g_pWorld->EnsureRemoved(this);
	Destroy();
}

void CWeenieObject::DebugValidate()
{
#ifdef _DEBUG
	assert(GetID());
	assert(g_pWorld->FindObject(GetID()));

	assert(!GetContainerID() || !GetWielderID());

	if (GetContainerID())
	{
		assert(g_pWorld->FindObject(GetContainerID()));
		assert(!GetWielderID());
		assert(!m_pBlock);
	}

	if (GetWielderID())
	{
		assert(g_pWorld->FindObject(GetWielderID()));
		assert(!GetContainerID());
		assert(!m_pBlock);
	}
	else
	{
		/*
		if (cell)
		{
		assert(m_pBlock);
		assert(m_pBlock->GetHeader() == (cell->id >> 16));
		}
		*/
	}

	if (!GetContainerID() && !GetWielderID())
	{
		// assert(m_pBlock);
	}

	int parentInt = InqIntQuality(PARENT_LOCATION_INT, 0);

	if (parentInt)
	{
		// assert(parent);
		assert(GetWielderID());
	}
	else
	{
		assert(!parent);
	}

	int placement = InqIntQuality(PLACEMENT_POSITION_INT, 0);

	if (!placement)
	{
		assert(!parent);
	}

#endif
}

BOOL CWeenieObject::InqSkill(STypeSkill key, uint32_t &value, BOOL raw)
{
	BOOL bResult = m_Qualities.InqSkill(key, value, raw);

	return bResult;
}

void CWeenieObject::RecalculateEncumbrance()
{
}

bool CWeenieObject::IsAttunedOrContainsAttuned()
{
	return InqIntQuality(ATTUNED_INT, 0) ? true : false;
}

bool CWeenieObject::IsBonded()
{
	return InqIntQuality(BONDED_INT, 0) == Bonded_BondedStatus ? true : false;
}

bool CWeenieObject::IsDroppedOnDeath()
{
	return InqIntQuality(BONDED_INT, 0) == Slippery_BondedStatus ? true : false;
}

bool CWeenieObject::IsDestroyedOnDeath()
{
	return InqIntQuality(BONDED_INT, 0) == Destroy_BondedStatus ? true : false;
}

void CWeenieObject::CheckVitalRanges()
{
	if (GetMaxHealth() < GetHealth())
	{
		SetHealth(GetMaxHealth());
	}

	if (GetMaxStamina() < GetStamina())
	{
		SetStamina(GetMaxStamina());
	}

	if (GetMaxMana() < GetMana())
	{
		SetMana(GetMaxMana());
	}
}

//void CWeenieObject::CheckEventState()
//{
//	if (!m_Qualities._generator_table)
//	{
//		return;
//	}
//
//	std::string eventName;
//	if (m_Qualities.InqString(GENERATOR_EVENT_STRING, eventName))
//	{
//		if (g_pGameEventManager->IsEventStarted(eventName.c_str()))
//		{
//			HandleEventActive();
//		}
//		else
//		{
//			HandleEventInactive();
//		}
//	}
//}
//
//void CWeenieObject::HandleEventActive()
//{
//	// if we have any spawns, we shouldn't be needing activation
//	if (m_Qualities._generator_registry && !m_Qualities._generator_registry->_registry.empty())
//		return;
//	if (m_Qualities._generator_queue && !m_Qualities._generator_queue->_queue.empty())
//		return;
//
//	g_pWeenieFactory->AddFromGeneratorTable(this, false);
//}
//
//void CWeenieObject::HandleEventInactive()
//{
//	if (m_Qualities._generator_registry)
//	{
//		while (!m_Qualities._generator_registry->_registry.empty())
//		{
//			uint32_t weenie_id = m_Qualities._generator_registry->_registry.begin()->first;
//
//			if (CWeenieObject *spawned_weenie = g_pWorld->FindObject(weenie_id))
//			{
//				spawned_weenie->MarkForDestroy();
//			}
//
//			// make sure it's gone (it should be already.)
//			m_Qualities._generator_registry->_registry.erase(weenie_id);
//		}
//	}
//
//	if (m_Qualities._generator_queue)
//	{
//		m_Qualities._generator_queue->_queue.clear();
//	}
//}

uint32_t CWeenieObject::GetRating(STypeInt rating)
{
	uint32_t ratingValue = 0;
	int currentRating = 0;
	switch (rating)
	{
	case DAMAGE_RATING_INT:
		if (m_Qualities.InqInt(DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_DAMAGE_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(AUGMENTATION_DAMAGE_BONUS_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case DAMAGE_RESIST_RATING_INT:
		if (m_Qualities.InqInt(DAMAGE_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_DAMAGE_RESIST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(AUGMENTATION_DAMAGE_REDUCTION_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_DAMAGE_REDUCTION_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case HEAL_OVER_TIME_INT:
		if (m_Qualities.InqInt(HEAL_OVER_TIME_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(AUGMENTATION_FASTER_REGEN_INT, currentRating, false, true) && get_minterp()->interpreted_state.forward_command == Motion_Sleeping)
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_HEALING_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case HEALING_BOOST_RATING_INT:
		if (m_Qualities.InqInt(HEALING_BOOST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_HEALING_BOOST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_HEALING_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case CRIT_RATING_INT:
		if (m_Qualities.InqInt(CRIT_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_CRIT_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(AUGMENTATION_CRITICAL_EXPERTISE_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case CRIT_DAMAGE_RATING_INT:
		if (m_Qualities.InqInt(CRIT_DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_CRIT_DAMAGE_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(AUGMENTATION_CRITICAL_POWER_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_CRIT_DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case CRIT_RESIST_RATING_INT:
		if (m_Qualities.InqInt(CRIT_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_CRIT_RESIST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case CRIT_DAMAGE_RESIST_RATING_INT:
		if (m_Qualities.InqInt(CRIT_DAMAGE_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_CRIT_DAMAGE_RESIST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(LUM_AUG_CRIT_REDUCTION_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case HEALING_RESIST_RATING_INT:
		if (m_Qualities.InqInt(HEALING_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case DAMAGE_OVER_TIME_INT:
		if (m_Qualities.InqInt(DAMAGE_OVER_TIME_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case WEAKNESS_RATING_INT:
		if (m_Qualities.InqInt(WEAKNESS_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case NETHER_OVER_TIME_INT:
		if (m_Qualities.InqInt(NETHER_OVER_TIME_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case NETHER_RESIST_RATING_INT:
		if (m_Qualities.InqInt(NETHER_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_NETHER_RESIST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case DOT_RESIST_RATING_INT:
		if (m_Qualities.InqInt(DOT_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case LIFE_RESIST_RATING_INT:
		if (m_Qualities.InqInt(LIFE_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_LIFE_RESIST_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case SNEAK_ATTACK_RATING_INT:
		if (m_Qualities.InqInt(SNEAK_ATTACK_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case RECKLESSNESS_RATING_INT:
		if (m_Qualities.InqInt(RECKLESSNESS_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case DECEPTION_RATING_INT:
		if (m_Qualities.InqInt(DECEPTION_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case PK_DAMAGE_RATING_INT:
		if (m_Qualities.InqInt(PK_DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_PK_DAMAGE_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	case PK_DAMAGE_RESIST_RATING_INT:
		if (m_Qualities.InqInt(PK_DAMAGE_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		if (m_Qualities.InqInt(GEAR_PK_DAMAGE_RESIST_RATING_INT, currentRating, false, true))
		{
			ratingValue += currentRating;
		}
		break;
	}


	return ratingValue;
}

bool CWeenieObject::GetSetSpells(int setId, int newLevel, std::vector<short>& spellsToAdd)
{
	if (!setId)
		return false;

	if (!newLevel)
		return true;

	CSpellTable *spellTable = MagicSystem::GetSpellTable();
	if (!spellTable)
		return false;

	const SpellSet *spellSet = spellTable->GetSpellSet(setId);
	if (!spellSet)
		return false;

	int maxSetLevel = (--spellSet->m_spellSetTiers.end())->first;
	if (newLevel > maxSetLevel)
		return true;

	int findLevel = newLevel;
	while (findLevel > 0)
	{
		auto newLevelFound = spellSet->m_spellSetTiers.find(findLevel);
		if (newLevelFound != spellSet->m_spellSetTiers.end())
		{
			for (uint32_t spellId : newLevelFound->second.m_tierSpellList)
			{
				spellsToAdd.push_back(spellId);
			}
			break;
		}
		findLevel--;
	}

	return true;
}

bool CWeenieObject::IsCurrency(int currencyid)
{
	switch (currencyid)// Add alt currencies to this list
	{
	case W_TRADENOTE250000_CLASS:
	case W_COINSTACK_CLASS:
	{
		return TRUE;
	}

	default:
		return FALSE; // Not a currency
	}
}

int32_t CWeenieObject::GetLevel()
{
	int32_t itemLevel = 1;
	int32_t style = InqIntQuality(ITEM_XP_STYLE_INT, 0);
	if (style != 0)
	{
		uint64_t currxp = (uint64_t)InqInt64Quality(ITEM_TOTAL_XP_INT64, 0);
		uint64_t basexp = (uint64_t)InqInt64Quality(ITEM_BASE_XP_INT64, 0);
		int32_t maxlvl = InqIntQuality(ITEM_MAX_LEVEL_INT, 0);

		itemLevel = ExperienceSystem::ItemTotalXpToLevel(currxp, basexp, maxlvl, style);
	}
	return itemLevel;
}
