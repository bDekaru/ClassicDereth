
#include "StdAfx.h"
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

	DWORD setupID = 0;
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
		LOG(Temp, Warning, "Failed creating physics object!\n");
	}



	BOOL bCreateAtAll = FALSE;
	BOOL bCreateParts = FALSE;

#if PHATSDK_IS_SERVER
	DWORD dataType = setupID & 0xFF000000;

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
		LOG(Temp, Warning, "Failed creating parts array for physics object with setup 0x%08X!\n", setupID);
	}

	InitObjectEnd(); //  SetPlacementFrameInternal(0x65);
#endif

	DWORD motionTableDID = 0;
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

	if (DWORD combatTable = m_Qualities.GetDID(COMBAT_TABLE_DID, 0))
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
	DWORD value = 0;
	m_Qualities.InqAttribute2nd(HEALTH_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxHealth()
{
	DWORD value = 0;
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
	DWORD value = 0;
	m_Qualities.InqAttribute2nd(STAMINA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxStamina()
{
	DWORD value = 0;
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
	DWORD value = 0;
	m_Qualities.InqAttribute2nd(MANA_ATTRIBUTE_2ND, value, FALSE);
	return value;
}

unsigned int CWeenieObject::GetMaxMana()
{
	DWORD value = 0;
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

int CWeenieObject::AdjustHealth(int amount)
{
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
	return false;
}

void CWeenieObject::OnMotionDone(DWORD motion, BOOL success)
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
							Blah.Write<DWORD>(0xF749);
							Blah.Write<DWORD>(GetID());
							Blah.Write<DWORD>(ammo->GetID());
							Blah.Write<DWORD>(PARENT_ENUM::PARENT_RIGHT_HAND);
							Blah.Write<DWORD>(Placement::RightHandCombat);
							Blah.Write<WORD>(GetPhysicsObj()->_instance_timestamp);
							Blah.Write<WORD>(++ammo->_position_timestamp);
							g_pWorld->BroadcastPVS(GetLandcell(), Blah.GetData(), Blah.GetSize());
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
							Blah.Write<DWORD>(0xF74A);
							Blah.Write<DWORD>(ammo->GetID());
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

void CWeenieObject::OnDeath(DWORD killer_id)
{
	if (CWeenieObject *pKiller = g_pWorld->FindObject(killer_id))
	{
		pKiller->ChanceExecuteEmoteSet(GetID(), KillTaunt_EmoteCategory);
	}

	if (m_UseManager)
		m_UseManager->OnDeath(killer_id);

	if (m_SpellcastingManager)
		m_SpellcastingManager->OnDeath(killer_id);

	if (m_EmoteManager)
		m_EmoteManager->OnDeath(killer_id);

	if (m_AttackManager)
		m_AttackManager->OnDeath(killer_id);

	ChanceExecuteEmoteSet(killer_id, Death_EmoteCategory);

	if (m_Qualities._enchantment_reg)
	{
		PackableList<DWORD> removed;

		if (m_Qualities._enchantment_reg->_add_list)
		{
			for (auto it = m_Qualities._enchantment_reg->_add_list->begin(); it != m_Qualities._enchantment_reg->_add_list->end();)
			{
				if (it->_duration != -1.0)
				{
					removed.push_back(it->_id);
					it = m_Qualities._enchantment_reg->_add_list->erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		if (m_Qualities._enchantment_reg->_mult_list)
		{
			for (auto it = m_Qualities._enchantment_reg->_mult_list->begin(); it != m_Qualities._enchantment_reg->_mult_list->end();)
			{
				if (it->_duration != -1.0)
				{
					removed.push_back(it->_id);
					it = m_Qualities._enchantment_reg->_mult_list->erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		if (removed.size())
		{
			// m_Qualities._enchantment_reg->PurgeEnchantments();

			BinaryWriter expireMessage;
			expireMessage.Write<DWORD>(0x2C8);
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

	SetHealth( (int)(GetMaxHealth() * InqFloatQuality(HEALTH_UPON_RESURRECTION_FLOAT, 1.0f)) );
	SetStamina( (int)(GetMaxStamina() * InqFloatQuality(STAMINA_UPON_RESURRECTION_FLOAT, 1.0f)) );
	SetMana( (int)(GetMaxMana() * InqFloatQuality(MANA_UPON_RESURRECTION_FLOAT, 1.0f)) );
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
		DWORD deceptionSkill = 0;
		
		if (InqType() & TYPE_ITEM)
		{
			deceptionSkill = (DWORD) m_Qualities.GetInt(RESIST_ITEM_APPRAISAL_INT, 0);
		}
		else
		{
			m_Qualities.InqSkill(DECEPTION_SKILL, deceptionSkill, FALSE);

			if (IsCreature() && !_IsPlayer())
			{
				DWORD focus = 0, self = 0;
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
		
			DWORD itemType = InqType();

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
				DWORD appraiseSkill = 0;
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

void CWeenieObject::Identify(CWeenieObject *source, DWORD overrideId)
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
	if (this == target || target->IsContained())
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
	if (target != NULL)
		return target->HeadingTo(this);
	return 0.0;
}

float CWeenieObject::HeadingTo(DWORD targetId, bool relative)
{
	if (!targetId || targetId == GetID())
		return 0.0;

	CWeenieObject *target = g_pWorld->FindObject(targetId);
	if (!target)
		return 0.0;

	return HeadingTo(target, relative);
}

float CWeenieObject::HeadingFrom(DWORD targetId, bool relative)
{
	if (!targetId || targetId == GetID())
		return 0.0;

	CWeenieObject *target = g_pWorld->FindObject(targetId);
	if (!target)
		return 0.0;

	return target->HeadingTo(this, relative);
}

DWORD CWeenieObject::GetLandcell()
{
	return m_Position.objcell_id; // return _phys_obj ? _phys_obj->m_Position.objcell_id : 0;
}

void CWeenieObject::EnsureLink(CWeenieObject *source)
{
	if (m_Qualities._generator_table)
	{
		source->m_Qualities.SetInstanceID(GENERATOR_IID, GetID());
	}
	else
	{
		// probably should check if it's zero or not
		source->m_Qualities.SetInstanceID(ACTIVATION_TARGET_IID, GetID());
	}
}

void CWeenieObject::NotifyGeneratedDeath(CWeenieObject *weenie)
{
	OnGeneratedDeath(weenie);
}

void CWeenieObject::OnGeneratedDeath(CWeenieObject *weenie)
{
	if (!weenie || !m_Qualities._generator_registry)
		return;

	DWORD weenie_id = weenie->GetID();

	GeneratorRegistryNode *node = m_Qualities._generator_registry->_registry.lookup(weenie_id);

	if (node)
	{
		GeneratorRegistryNode oldNode = *node;
		m_Qualities._generator_registry->_registry.remove(weenie_id);

		double delay = -1.0;
		if (m_Qualities._generator_table)
		{
			if (InqIIDQuality(GENERATOR_IID, 0)) // we have a generator
			{
				std::vector<CWeenieObject *> rotList;
				bool hasValidChildren = false;
				if (m_Qualities._generator_registry->_registry.size() > 0)
				{
					for each(auto entry in m_Qualities._generator_registry->_registry)
					{
						CWeenieObject *weenie = g_pWorld->FindObject(entry.second.m_objectId);

						if (weenie && (weenie->IsCreature() || !weenie->IsStuck())) //stuck objects(chests and decorations usually) do not prevent the generator from being finished.
							hasValidChildren = true;
						else if (weenie)
							rotList.push_back(weenie);
					}
				}

				if (!hasValidChildren)
				{
					//we're the child of a generator and all our children have been destroyed/picked up.
					//so we're done and should cease to exist.
					MarkForDestroy();

					//make the leftovers rot.
					for each (auto entry in rotList)
					{
						entry->m_Qualities.SetInstanceID(GENERATOR_IID, 0);
						entry->_timeToRot = Timer::cur_time + 300.0; //in 5 minutes
						entry->_beganRot = false;
						entry->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _timeToRot);
					}
					return;
				}
			}

			// find this profile slot
			for (auto &profile : m_Qualities._generator_table->_profile_list)
			{
				if (profile.slot == oldNode.slot &&
					(profile.whenCreate == RegenerationType::Death_RegenerationType ||
					 profile.whenCreate == RegenerationType::Destruction_RegenerationType))
				{
					delay = profile.delay * g_pConfig->RespawnTimeMultiplier();
					break;
				}
			}
		}

		if (delay >= 0)
		{
			if (!m_Qualities._generator_queue)
				m_Qualities._generator_queue = new GeneratorQueue();

			GeneratorQueueNode queueNode;
			queueNode.slot = oldNode.slot;
			queueNode.when = Timer::cur_time + delay;
			m_Qualities._generator_queue->_queue.push_back(queueNode);

			if (_nextRegen < 0.0)
				_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
		}
	}
}

void CWeenieObject::NotifyGeneratedPickedUp(CWeenieObject *weenie)
{
	OnGeneratedPickedUp(weenie);
	weenie->m_Qualities.RemoveInstanceID(GENERATOR_IID);
}

void CWeenieObject::OnGeneratedPickedUp(CWeenieObject *weenie)
{
	if (!weenie || !m_Qualities._generator_registry)
		return;

	DWORD weenie_id = weenie->GetID();

	GeneratorRegistryNode *node = m_Qualities._generator_registry->_registry.lookup(weenie_id);

	if (node)
	{
		GeneratorRegistryNode oldNode = *node;
		m_Qualities._generator_registry->_registry.remove(weenie_id);

		double delay = -1.0;
		if (m_Qualities._generator_table)
		{
			if (InqIIDQuality(GENERATOR_IID, 0)) // we have a generator
			{
				std::vector<CWeenieObject *> rotList;
				bool hasValidChildren = false;
				if (m_Qualities._generator_registry->_registry.size() > 0)
				{
					for each(auto entry in m_Qualities._generator_registry->_registry)
					{
						CWeenieObject *weenie = g_pWorld->FindObject(entry.second.m_objectId);

						if (weenie && (weenie->IsCreature() || !weenie->IsStuck())) //stuck objects(chests and decorations usually) do not prevent the generator from being finished.
							hasValidChildren = true;
						else if (weenie)
							rotList.push_back(weenie);
					}
				}

				if (!hasValidChildren)
				{
					//we're the child of a generator and all our children have been destroyed/picked up.
					//so we're done and should cease to exist.
					MarkForDestroy();

					//make the leftovers rot.
					for each (auto entry in rotList)
					{
						entry->m_Qualities.SetInstanceID(GENERATOR_IID, 0);
						entry->_timeToRot = Timer::cur_time + 300.0; //in 5 minutes
						entry->_beganRot = false;
						entry->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, _timeToRot);
					}
					return;
				}
			}

			// find this profile slot
			for (auto &profile : m_Qualities._generator_table->_profile_list)
			{
				if (profile.slot == oldNode.slot && profile.whenCreate == RegenerationType::PickUp_RegenerationType || profile.whenCreate == Destruction_RegenerationType)
				{
					delay = profile.delay * g_pConfig->RespawnTimeMultiplier();
					break;
				}
			}
		}

		if (delay >= 0)
		{
			if (!m_Qualities._generator_queue)
				m_Qualities._generator_queue = new GeneratorQueue();

			GeneratorQueueNode queueNode;
			queueNode.slot = oldNode.slot;
			queueNode.when = Timer::cur_time + delay;
			m_Qualities._generator_queue->_queue.push_back(queueNode);

			if (_nextRegen < 0.0)
				_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
		}
	}
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
	DWORD key = (statType << 16) | statIndex;

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
			SendNetMessage(&CM, OBJECT_MSG, FALSE, FALSE);
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
			SendNetMessage(&UM, OBJECT_MSG, FALSE, FALSE);
		else
			g_pWorld->BroadcastPVS(this, UM->GetData(), UM->GetSize(), OBJECT_MSG, 0);
		delete UM;
	}
}

void CWeenieObject::NotifyIntStatUpdated(STypeInt key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Int_StatType, key);

	int value;
	if (m_Qualities.InqInt(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x2CD);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<int>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2CE);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<int>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1D1);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1D2);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
}

void CWeenieObject::NotifyInt64StatUpdated(STypeInt64 key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(Int64_StatType, key);

	__int64 value;
	if (m_Qualities.InqInt64(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x2CF);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<__int64>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2D0);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<__int64>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x2B8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2B9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
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
			statNotify.Write<DWORD>(0x2D1);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<int>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2D2);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<int>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1D3);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1D4);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
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
			statNotify.Write<DWORD>(0x2D3);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<double>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2D4);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<double>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1D5);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1D6);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
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
			statNotify.Write<DWORD>(0x2D5);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.WriteString(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2D6);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.WriteString(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1D7);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1D8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
}

void CWeenieObject::NotifyDIDStatUpdated(STypeDID key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(DID_StatType, key);

	DWORD value;
	if (m_Qualities.InqDataID(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x2D7);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<DWORD>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2D8);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<DWORD>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1D9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
}

void CWeenieObject::NotifyIIDStatUpdated(STypeIID key, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter statNotify;
	BYTE statTS = GetNextStatTimestamp(IID_StatType, key);

	DWORD value;
	if (m_Qualities.InqInstanceID(key, value))
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x2D9);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			statNotify.Write<DWORD>(value);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			statNotify.Write<DWORD>(value);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1DB);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1DC);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
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
			statNotify.Write<DWORD>(0x2DB);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			value.Pack(&statNotify);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x2DC);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			value.Pack(&statNotify);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
	else
	{
		if (bPrivate)
		{
			statNotify.Write<DWORD>(0x1DD);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(key);
			SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
		}
		else
		{
			statNotify.Write<DWORD>(0x1DE);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(key);
			g_pWorld->BroadcastPVS(this, statNotify.GetData(), statNotify.GetSize(), PRIVATE_MSG, FALSE, FALSE);
		}
	}
}

void CWeenieObject::NotifyStackSizeUpdated(bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter msg;
	msg.Write<DWORD>(0x197);
	msg.Write<BYTE>(GetNextStatTimestamp(Int_StatType, STACK_SIZE_INT));
	msg.Write<DWORD>(GetID());
	msg.Write<DWORD>(InqIntQuality(STACK_SIZE_INT, 1));
	msg.Write<DWORD>(InqIntQuality(VALUE_INT, 0));

	if (bPrivate)
		SendNetMessage(&msg, PRIVATE_MSG, FALSE, FALSE);
	else
		g_pWorld->BroadcastPVS(this, msg.GetData(), msg.GetSize(), PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifyContainedItemRemoved(DWORD objectId, bool bPrivate)
{
	if (!m_bWorldIsAware)
		return;

	BinaryWriter msg;
	msg.Write<DWORD>(0x24);
	msg.Write<DWORD>(objectId);

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
	msg.Write<DWORD>(0xF747);
	msg.Write<DWORD>(GetID());
	msg.Write<DWORD>(_instance_timestamp);

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
	__int64 value = 0;
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
	DWORD value;
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
	statNotify.Write<DWORD>(0x2E3);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Attribute_StatType, key));
	statNotify.Write<DWORD>(key);

	Attribute attrib;
	m_Qualities.InqAttribute(key, attrib);
	statNotify.Write(&attrib);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifyAttribute2ndStatUpdated(STypeAttribute2nd key)
{
	BinaryWriter statNotify;
	statNotify.Write<DWORD>(0x2E7);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Attribute_2nd_StatType, key));
	statNotify.Write<DWORD>(key);

	SecondaryAttribute attrib2nd;
	m_Qualities.InqAttribute2nd(key, attrib2nd);
	statNotify.Write(&attrib2nd);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifySkillStatUpdated(STypeSkill key)
{
	BinaryWriter statNotify;
	statNotify.Write<DWORD>(0x2DD);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Skill_StatType, key));
	statNotify.Write<DWORD>(key);

	Skill skill;
	m_Qualities.InqSkill(key, skill);
	statNotify.Write(&skill);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifySkillAdvancementClassUpdated(STypeSkill key)
{
	BinaryWriter statNotify;
	statNotify.Write<DWORD>(0x2E1);
	statNotify.Write<BYTE>(GetNextStatTimestamp(Skill_StatType, key));
	statNotify.Write<DWORD>(key);

	SKILL_ADVANCEMENT_CLASS sac;
	m_Qualities.InqSkillAdvancementClass(key, sac);
	statNotify.Write<int>(sac);

	SendNetMessage(&statNotify, PRIVATE_MSG, FALSE, FALSE);
}

void CWeenieObject::NotifyEnchantmentUpdated(Enchantment *enchant)
{
	BinaryWriter statNotify;
	statNotify.Write<DWORD>(0x2C2);
	enchant->Pack(&statNotify);

	SendNetMessage(&statNotify, PRIVATE_MSG, TRUE, FALSE);
}

DWORD CWeenieObject::GetCostToRaiseSkill(STypeSkill key)
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
		DWORD maxLevel;

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
			DWORD expRequired = ExperienceSystem::ExperienceToSkillLevel(skill._sac, skill._level_from_pp + 1);
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

DWORD GetXPForKillLevel(int level)
{
	double xp = 380.5804 + 63.92362*level + 3.543397*pow(level, 2) - 0.05233995*pow(level, 3) + 0.0007008949*pow(level, 4);

	DWORD xpvalout;

	if (xp < 0)
		xpvalout = 0;
	else if (xp >= UINT_MAX)
		xpvalout = UINT_MAX;
	else
		xpvalout = (DWORD) xp;

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

	int xpForKill = 0;
	
	if (!pKilled->m_Qualities.InqInt(XP_OVERRIDE_INT, xpForKill, 0, FALSE))
		xpForKill = (int) GetXPForKillLevel(level);

	xpForKill = (int)(xpForKill * g_pConfig->KillXPMultiplier(level));

	if (xpForKill < 0)
		xpForKill = 0;

	GiveSharedXP(xpForKill, false);
}

void CWeenieObject::GiveSharedXP(long long amount, bool showText)
{
	if (amount <= 0)
		return;

	Fellowship *f = GetFellowship();

	if (f)
		f->GiveXP(this, amount, showText);
	else
		GiveXP(amount, showText, false);
}

void CWeenieObject::GiveXP(long long amount, bool showText, bool allegianceXP)
{
	if (amount <= 0)
		return;

	OnGivenXP(amount, allegianceXP);

	unsigned __int64 newAvailableXP = (unsigned __int64)InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + amount;
	unsigned __int64 newTotalXP = (unsigned __int64)InqInt64Quality(TOTAL_EXPERIENCE_INT64, 0) + amount;

	int currentLevel = InqIntQuality(LEVEL_INT, 1);

	DWORD skillCredits = 0;
	bool bLeveled = false;
	DWORD64 xpToNextLevel = ExperienceSystem::ExperienceToLevel(currentLevel + 1);
	while  (xpToNextLevel <= newTotalXP && currentLevel < ExperienceSystem::GetMaxLevel())
	{
		currentLevel++;
		skillCredits += ExperienceSystem::GetCreditsForLevel(currentLevel);
		bLeveled = true;

		xpToNextLevel = ExperienceSystem::ExperienceToLevel(currentLevel + 1);

		if (!xpToNextLevel)
			break;
	}

	m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, newAvailableXP);
	NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);

	m_Qualities.SetInt64(TOTAL_EXPERIENCE_INT64, newTotalXP);
	NotifyInt64StatUpdated(TOTAL_EXPERIENCE_INT64);

	GiveSkillCredits(skillCredits, false);

	if (bLeveled)
	{
		m_Qualities.SetInt(LEVEL_INT, currentLevel);
		NotifyIntStatUpdated(LEVEL_INT);

		EmitEffect(PS_LevelUp, 1.0f);

		if (currentLevel == 275)
		{
			EmitEffect(PS_WeddingBliss, 1.0f);
		}

		const char *notice = csprintf(
			"You are now level %s! You have %s experience points and %s skill credits available to raise skills and attributes.",
			FormatNumberString(currentLevel).c_str(),
			FormatNumberString(newAvailableXP).c_str(),
			FormatNumberString(InqIntQuality(AVAILABLE_SKILL_CREDITS_INT, 0)).c_str());

		SendText(notice, LTT_ADVANCEMENT);

		// restore vitals
		if (!IsDead())
		{
			SetMaxVitals(true);
		}
	}

	if (showText)
	{
		const char *notice = csprintf(
			"You've earned %s experience.",
			FormatNumberString(amount).c_str());

		SendText(notice, LTT_ADVANCEMENT);
	}

	if (!allegianceXP)
	{
		// let allegiance manager pass it up
		g_pAllegianceManager->HandleAllegiancePassup(GetID(), amount, true);
	}
}

void CWeenieObject::TryToUnloadAllegianceXP(bool bShowText)
{
	AllegianceTreeNode *node = g_pAllegianceManager->GetTreeNode(GetID());
	if (node)
	{
		if (node->_cp_pool_to_unload > 0)
		{
			GiveXP(node->_cp_pool_to_unload, false, true);

			if (bShowText)
			{
				// this was from logging in
				SendText(csprintf("Your Vassals have produced experience points for you.\nTaking your skills as a leader into account, you gain %s xp.",
					FormatNumberString(node->_cp_pool_to_unload).c_str()), LTT_DEFAULT);
			}

			node->_cp_pool_to_unload = 0;
		}
	}
}

DWORD CWeenieObject::GiveAttributeXP(STypeAttribute key, DWORD amount)
{
	Attribute attribute;
	m_Qualities.InqAttribute(key, attribute);
	attribute._cp_spent += amount;

	DWORD oldLevel = attribute._level_from_cp;
	attribute._level_from_cp = ExperienceSystem::AttributeLevelFromExperience(attribute._cp_spent);

	m_Qualities.SetAttribute(key, attribute);

	DWORD raised = attribute._level_from_cp - oldLevel;
	if (raised)
	{
		DWORD newLevel;
		if (m_Qualities.InqAttribute(key, newLevel, TRUE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

			SendText(csprintf("Your base %s is now %u!", Attribute::GetAttributeName(key), newLevel), LTT_ADVANCEMENT);
		}
	}

	NotifyAttributeStatUpdated(key);

	return raised;
}

const char *GetAttribute2ndName(STypeAttribute2nd key)
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

DWORD CWeenieObject::GiveAttribute2ndXP(STypeAttribute2nd key, DWORD amount)
{
	SecondaryAttribute attribute2nd;
	m_Qualities.InqAttribute2nd(key, attribute2nd);
	attribute2nd._cp_spent += amount;

	DWORD oldLevel = attribute2nd._level_from_cp;
	attribute2nd._level_from_cp = ExperienceSystem::Attribute2ndLevelFromExperience(attribute2nd._cp_spent);

	m_Qualities.SetAttribute2nd(key, attribute2nd);

	DWORD raised = attribute2nd._level_from_cp - oldLevel;
	if (raised)
	{
		DWORD newLevel;
		if (m_Qualities.InqAttribute2nd(key, newLevel, FALSE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

			SendText(csprintf("Your base %s is now %u!", GetAttribute2ndName(key), newLevel), LTT_ADVANCEMENT);
		}
	}

	NotifyAttribute2ndStatUpdated(key);
	return raised;
}

DWORD CWeenieObject::GiveSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS sac)
{
	Skill skill;
	m_Qualities.InqSkill(key, skill);

	// before
	DWORD oldLevel = 0;
	m_Qualities.InqSkill(key, oldLevel, TRUE);

	skill._sac = sac;
	skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);

	// after
	DWORD newLevel;
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

	SendText(csprintf("You are now trained in %s!", skillName.c_str()), LTT_ADVANCEMENT);

	DWORD raised = newLevel - oldLevel;
	if (raised)
	{
		SendText(csprintf("Your base %s is now %u!", skillName.c_str(), newLevel), LTT_ADVANCEMENT);
	}

	NotifySkillStatUpdated(key);
	return raised;
}

DWORD CWeenieObject::GiveSkillXP(STypeSkill key, DWORD amount, bool silent)
{
	if (amount <= 0)
		return 0;

	Skill skill;
	m_Qualities.InqSkill(key, skill);

	if (skill._sac < TRAINED_SKILL_ADVANCEMENT_CLASS)
		return 0;

	skill._pp += amount;

	DWORD oldLevel = skill._level_from_pp;
	skill._level_from_pp = ExperienceSystem::SkillLevelFromExperience(skill._sac, skill._pp);

	m_Qualities.SetSkill(key, skill);

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

	DWORD raised = skill._level_from_pp - oldLevel;
	if (raised)
	{
		// Your base Melee Defense is now 123! color: 13

		DWORD newLevel;
		if (m_Qualities.InqSkill(key, newLevel, TRUE))
		{
			if (_phys_obj)
				_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

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

DWORD CWeenieObject::GiveSkillPoints(STypeSkill key, DWORD amount)
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

	DWORD newLevel;
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

void CWeenieObject::GiveSkillCredits(DWORD amount, bool showText)
{
	if (amount <= 0)
		return;

	if (!amount)
		return;

	DWORD unassignedCredits = 0;
	m_Qualities.InqInt(AVAILABLE_SKILL_CREDITS_INT, *(int *)&unassignedCredits);	
	m_Qualities.SetInt(AVAILABLE_SKILL_CREDITS_INT, unassignedCredits + amount);
	NotifyIntStatUpdated(AVAILABLE_SKILL_CREDITS_INT);

	if (showText)
	{
		if (_phys_obj)
			_phys_obj->EmitSound(Sound_RaiseTrait, 1.0, true);

		SendText(csprintf("You have earned %u skill %s!", amount, amount == 1 ? "credit" : "credits"), LTT_ADVANCEMENT);
	}
}

void CWeenieObject::SendText(const char* szText, long lColor)
{
	SendNetMessage(ServerText(szText, lColor), PRIVATE_MSG, FALSE, TRUE);
}

float Calc_BurdenMod(float flBurden)
{
	if (flBurden < 1.0f)
		return 1.0f;

	if (flBurden < 2.0f)
		return (2.0f - flBurden);

	return 0.0f;
}

float Calc_AnimSpeed(DWORD runSkill, float fBurden)
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

bool CWeenieObject::JumpStaminaCost(float extent, long &cost)
{
	// Client does this..but we are the server!
	//if (IsThePlayer())
	//{
	//     etc.
	//}

	return !!m_Qualities.JumpStaminaCost(extent, cost);
}

void CWeenieObject::EmitEffect(DWORD effect, float mod)
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
		DWORD houseId = player->GetAccountHouseId();
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

	DWORD allegianceHouseId;

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
		if (houseData && houseData->_ownerId == allegianceNode->_monarchID && (houseData->_houseType == 2 || houseData->_houseType == 3)) //2 = villa, 3 = mansion
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
	if (m_UseManager && m_UseManager->IsUsing())
	{
		NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		return;
	}

	if (IsBusyOrInAction())
	{
		NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		NotifyUseDone(WERROR_NONE);
		return;
	}

	if (!m_UseManager)
	{
		m_UseManager = new UseManager(this);
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

void CWeenieObject::CheckForExpiredEnchantments()
{
	if (m_Qualities._enchantment_reg)
	{
		PackableListWithJson<DWORD> expired;
		m_Qualities._enchantment_reg->GetExpiredEnchantments(&expired);

		if (expired.size())
		{
			m_Qualities._enchantment_reg->RemoveEnchantments(&expired);

			if (AsPlayer())
			{
				BinaryWriter expireMessage;
				expireMessage.Write<DWORD>(0x2C5);
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
						{
							topMostOwner->SendText(csprintf("The spell %s on %s has expired.", spellBase->_name.c_str(), GetName().c_str()), LTT_MAGIC);
						}
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

					_nextManaUse = Timer::cur_time + (-manaRate * 1000);
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
							PackableListWithJson<DWORD> spells_to_remove;

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
									silentRemoveMessage.Write<DWORD>(0x2C8);
									spells_to_remove.Pack(&silentRemoveMessage);

									wielder->SendNetMessage(&silentRemoveMessage, PRIVATE_MSG, TRUE, FALSE);
								}

								wielder->CheckVitalRanges();
							}
						}

						if (m_Qualities._enchantment_reg)
						{
							PackableListWithJson<DWORD> spells_to_remove;

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
}

void CWeenieObject::Tick()
{
	if (m_AttackManager)
		m_AttackManager->Update();

	if (m_SpellcastingManager)
		m_SpellcastingManager->Update();

	if (m_EmoteManager)
		m_EmoteManager->Tick();

	CheckForExpiredEnchantments();

	if (_nextRegen >= 0 && _nextRegen <= Timer::cur_time)
	{
		if (m_Qualities._generator_queue)
		{
			PackableList<GeneratorQueueNode> &queue = m_Qualities._generator_queue->_queue;
			for (auto entry = queue.begin(); entry != queue.end();)
			{
				if (entry->when <= Timer::cur_time)
				{
					double regenInterval = 0.0;
					if (m_Qualities.InqFloat(REGENERATION_INTERVAL_FLOAT, regenInterval) && regenInterval > 0.0)
					{
						if (m_Qualities._generator_table)
						{
							for (auto &profile : m_Qualities._generator_table->_profile_list)
							{
								if (profile.slot == entry->slot)
								{
									g_pWeenieFactory->GenerateFromTypeOrWcid(this, &profile);
									break;
								}
							}
						}
					}

					//ChanceCreateGeneratorSlot(entry->slot);
					entry = queue.erase(entry);
					continue;
				}

				entry++;
			}
		}

		g_pWeenieFactory->AddFromGeneratorTable(this, false);

		int numSpawned = m_Qualities._generator_registry ? (DWORD)m_Qualities._generator_registry->_registry.size() : 0;
		if ((!m_Qualities._generator_queue || m_Qualities._generator_queue->_queue.empty()) && numSpawned >= InqIntQuality(MAX_GENERATED_OBJECTS_INT, 0, TRUE))
			_nextRegen = -1.0;
		else
			_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, -1.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
	}

	if (_nextHeartBeat != -1.0 && _nextHeartBeat <= Timer::cur_time)
	{
		if (!IsDead() && !IsInPortalSpace())
		{
			if (_nextHeartBeatEmote != -1.0 && _nextHeartBeatEmote <= Timer::cur_time)
			{
				_nextHeartBeatEmote = Timer::cur_time + Random::GenUInt(2, 15); //add a little variation to avoid synchronization.

				//_last_update_pos is the time of the last attack/movement/action, basically we don't want to do heartBeat emotes if we're active.
				if (Timer::cur_time > _last_update_pos + 10.0 && m_Qualities._emote_table)
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

			CheckRegeneration(InqFloatQuality(HEALTH_RATE_FLOAT, 0.0), HEALTH_ATTRIBUTE_2ND, MAX_HEALTH_ATTRIBUTE_2ND);
			CheckRegeneration(InqFloatQuality(STAMINA_RATE_FLOAT, 0.0), STAMINA_ATTRIBUTE_2ND, MAX_STAMINA_ATTRIBUTE_2ND);
			CheckRegeneration(InqFloatQuality(MANA_RATE_FLOAT, 0.0), MANA_ATTRIBUTE_2ND, MAX_MANA_ATTRIBUTE_2ND);
		}

		double heartbeatInterval;
		if (m_Qualities.InqFloat(HEARTBEAT_INTERVAL_FLOAT, heartbeatInterval, TRUE))
			_nextHeartBeat = Timer::cur_time + heartbeatInterval;
		else
			_nextHeartBeat = -1.0;

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
		if (!HasOwner())
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
				EmitEffect(PS_Destroy, 1.0f);
				_beganRot = true;
			}
		}
	}
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
		if(currentAttrib == HEALTH_ATTRIBUTE_2ND)
		{
			DWORD strength = 0;
			DWORD endurance = 0;
			m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + (endurance * 2));
			float regenMod = 1.0 + (0.0494 * pow(strAndEnd, 1.179) / 100.0f); //formula deduced from values present in the client pdb.
			regenMod = min(max(regenMod, 1.0), 2.1);

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
					break;
				}
			}
			else
				rate *= 0.5; //in combat regen
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
		DWORD currentVital = 0, maxVital = 0;
		if (m_Qualities.InqAttribute2nd(currentAttrib, currentVital, FALSE) && m_Qualities.InqAttribute2nd(maxAttrib, maxVital, FALSE))
		{
			if (currentVital < maxVital)
			{
				currentVital += regenAmount;

				//currentVital += (DWORD)(ceil(rate) + F_EPSILON); //previously

				if (currentVital > maxVital)
				{
					currentVital = maxVital;
				}

				m_Qualities.SetAttribute2nd(currentAttrib, currentVital);

				if (AsPlayer())
				{
					NotifyAttribute2ndStatUpdated(currentAttrib);
				}
			}
		}
	}
}

void CWeenieObject::SetupWeenie()
{
	// correct the icon
	DWORD clothing_table_id = InqDIDQuality(CLOTHINGBASE_DID, 0);
	DWORD palette_template_key = InqIntQuality(PALETTE_TEMPLATE_INT, 0);

	if (clothing_table_id && palette_template_key)
	{
		if (!InqBoolQuality(IGNORE_CLO_ICONS_BOOL, FALSE))
		{
			if (ClothingTable *clothingTable = ClothingTable::Get(clothing_table_id))
			{
				if (const CloPaletteTemplate *pPaletteTemplate = clothingTable->_paletteTemplatesHash.lookup(palette_template_key))
				{
					if (pPaletteTemplate->iconID)
					{
						SetIcon(pPaletteTemplate->iconID);
					}
				}

				ClothingTable::Release(clothingTable);
			}
		}
	}
}

void CWeenieObject::PostSpawn()
{
	if (g_pConfig->EverythingUnlocked())
	{
		SetLocked(FALSE);
	}

	InitCreateGenerator();

	double heartbeatInterval;
	if (m_Qualities.InqFloat(HEARTBEAT_INTERVAL_FLOAT, heartbeatInterval, TRUE))
		_nextHeartBeat = Timer::cur_time + heartbeatInterval;

	if (m_Qualities._emote_table && m_Qualities._emote_table->_emote_table.lookup(HeartBeat_EmoteCategory))
		_nextHeartBeatEmote = Timer::cur_time + Random::GenUInt(2, 15); //We have heartBeat emotes so schedule them.
}

bool CWeenieObject::IsGeneratorSlotReady(int slot)
{
	if (m_Qualities._generator_registry)
	{
		for (auto &entry : m_Qualities._generator_registry->_registry)
		{
			if (entry.second.slot == slot)
			{
				return false;
			}
		}
	}

	if (m_Qualities._generator_queue)
	{
		for (auto &entry : m_Qualities._generator_queue->_queue)
		{
			if (entry.slot == slot && entry.when > Timer::cur_time)
			{
				return false;
			}
		}
	}

	return true;
}

void CWeenieObject::InitCreateGenerator()
{
	std::string eventString;
	if (m_Qualities.InqString(GENERATOR_EVENT_STRING, eventString))
	{
		if(g_pGameEventManager->IsEventStarted(eventString.c_str()))
			g_pWeenieFactory->AddFromGeneratorTable(this, true);
	}
	else
		g_pWeenieFactory->AddFromGeneratorTable(this, true);
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

	message.Write<DWORD>(0xF747);
	message.Write<DWORD>(GetID());
	message.Write<DWORD>(_instance_timestamp - 1);

	g_pWorld->BroadcastPVS(this, message.GetData(), message.GetSize());
}

void CWeenieObject::UpdateModel()
{
	if (parent)
		return;

	BinaryWriter MU;

	MU.Write<DWORD>(0xF625);
	MU.Write<DWORD>(GetID());

	ObjDesc objDesc;
	GetObjDesc(objDesc);
	objDesc.Pack(&MU);

	MU.Write<WORD>(_instance_timestamp);
	MU.Write<WORD>(++_objdesc_timestamp);

	g_pWorld->BroadcastPVS(this, MU.GetData(), MU.GetSize());
}

void CWeenieObject::GetObjDesc(ObjDesc &objDesc)
{
	// Generate 
	objDesc.Wipe();
	
	if (m_bObjDescOverride)
	{
		objDesc = m_ObjDescOverride;
		return;
	}

	DWORD basePaletteID;
	if (m_Qualities.InqDataID(PALETTE_BASE_DID, basePaletteID))
		objDesc.paletteID = basePaletteID;
	else
		objDesc.paletteID = 0x0400007E;

	if (objDesc.paletteID == 0x04000B75)
		objDesc.paletteID = 0x0400007E; // shadows are messed up

	DWORD clothingBaseID;
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

			clothingTable->BuildObjDesc(GetSetupID(), InqIntQuality(PALETTE_TEMPLATE_INT, 0), &shades, &objDesc);
			ClothingTable::Release(clothingTable);
		}
	}
	else
	{
		// if not using a clothing base, start objDesc from model itself
		// TODO: fix this later, quick fix to handle unequipping resetting the model parts
		if (_IsPlayer() && part_array && part_array->setup)
		{
			for (DWORD i = 0; i < part_array->setup->num_parts; i++)
			{
				if (part_array->setup && part_array->setup->parts && part_array->setup->num_parts > 0)
					objDesc.AddAnimPartChange(new AnimPartChange(i, part_array->setup->parts[i]));
			}
		}
	}
}

void CWeenieObject::SetMotionTableID(DWORD motion_table_did)
{
	m_Qualities.SetDataID(MOTION_TABLE_DID, motion_table_did);
}

void CWeenieObject::SetSetupID(DWORD setup_did)
{
	m_Qualities.SetDataID(SETUP_DID, setup_did);
}

void CWeenieObject::SetSoundTableID(DWORD sound_table_did)
{
	m_Qualities.SetDataID(SOUND_TABLE_DID, sound_table_did);
}

void CWeenieObject::SetPETableID(DWORD pe_table_did)
{
	m_Qualities.SetDataID(PHYSICS_EFFECT_TABLE_DID, pe_table_did);
}

void CWeenieObject::SetScale(float value)
{
	m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, value);
}

void CWeenieObject::SetIcon(DWORD icon_did)
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
	m_Qualities.SetInt(RADARBLIP_COLOR_INT, (int) color);
}

void CWeenieObject::SetInitialPhysicsState(DWORD physics_state)
{
	m_Qualities.SetInt(PHYSICS_STATE_INT, physics_state);
}

void CWeenieObject::SetInitialPosition(const Position &position)
{
	m_Qualities.SetPosition(INSTANTIATION_POSITION, position);
}

void CWeenieObject::SetSpellID(DWORD spell_id)
{
	m_Qualities.SetDataID(SPELL_DID, spell_id);
}


DWORD CWeenieObject::GetValue()
{
	return m_Qualities.GetInt(VALUE_INT, 0);
}

void CWeenieObject::SetValue(DWORD amount)
{
	m_Qualities.SetInt(VALUE_INT, amount);
}

DWORD CWeenieObject::GetSpellID()
{
	return m_Qualities.GetDID(SPELL_DID, 0);
}

DWORD CWeenieObject::GetIcon()
{
	DWORD icon = 0;
	m_Qualities.InqDataID(ICON_DID, icon);
	return icon;
}

std::string CWeenieObject::GetName()
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

void CWeenieObject::DoCollisionEnd(DWORD object_id)
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

long long CWeenieObject::InqInt64Quality(STypeInt64 key, long long defaultValue)
{
	long long value = defaultValue;
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

DWORD CWeenieObject::InqDIDQuality(STypeDID key, DWORD defaultValue)
{
	DWORD value = defaultValue;
	m_Qualities.InqDataID(key, value);
	return value;
}

DWORD CWeenieObject::InqIIDQuality(STypeIID key, DWORD defaultValue)
{
	DWORD value = defaultValue;
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

void CWeenieObject::TryCancelAttack()
{
	if (m_AttackManager)
		m_AttackManager->Cancel();
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

void CWeenieObject::TryCastSpell(DWORD target_id, DWORD spell_id)
{
	int error = MakeSpellcastingManager()->TryBeginCast(target_id, spell_id);

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

DWORD CWeenieObject::DoAutonomousMotion(DWORD motion, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	DWORD err;

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

DWORD CWeenieObject::DoForcedMotion(DWORD motion, MovementParameters *params)
{
	MakeMovementManager(TRUE);

	DWORD err;

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
	}

	if (movement_manager->motion_interpreter)
	{
		if (movement_manager->motion_interpreter->interpreted_state.sidestep_command ||
			movement_manager->motion_interpreter->interpreted_state.turn_command ||
			movement_manager->motion_interpreter->interpreted_state.forward_command != 0x41000003)
			return false;
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

bool CWeenieObject::IsMovingTo()
{
	if (!movement_manager || !movement_manager->moveto_manager)
		return false;

	return movement_manager->moveto_manager->movement_type != MovementTypes::Invalid;
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
	if (m_PhysicsState & (HIDDEN_PS|PARTICLE_EMITTER_PS|MISSILE_PS))
		return true;

	if (!IsCreature())
		return true;

	if (IsDead())
		return true;

	return false;
}

BODY_PART_ENUM GetRandomBodyPartByDamageQuadrant(DAMAGE_QUADRANT dq)
{
	BODY_PART_ENUM result = BP_UNDEF;

	// do damage processing here...
	if (dq & DQ_LOW)
	{
		switch (Random::GenInt(0, 4))
		{
		case 0: result = BP_FOOT; break;
		case 1: result = BP_LOWER_LEG; break;
		case 2: result = BP_UPPER_LEG; break;
		case 3: result = BP_HAND; break;
		case 4: result = BP_ABDOMEN; break;
		}
	}
	else if (dq & DQ_MEDIUM)
	{
		switch (Random::GenInt(0, 6))
		{
		case 0: result = BP_UPPER_LEG; break;
		case 1: result = BP_ABDOMEN; break;
		case 2: result = BP_CHEST; break;
		case 3: result = BP_HAND; break;
		case 5: result = BP_LOWER_ARM; break;
		case 6: result = BP_UPPER_ARM; break;
		}
	}
	else if (dq & DQ_HIGH)
	{
		switch (Random::GenInt(0, 5))
		{
		case 0: result = BP_CHEST; break;
		case 1: result = BP_HAND; break;
		case 2: result = BP_CHEST; break;
		case 3: result = BP_LOWER_ARM; break;
		case 4: result = BP_UPPER_ARM; break;
		case 5: result = BP_HEAD; break;
		}
	}
	else
	{
		switch (Random::GenInt(0, 9))
		{
		case 0: result = BP_CHEST; break;
		case 1: result = BP_HAND; break;
		case 2: result = BP_CHEST; break;
		case 3: result = BP_LOWER_ARM; break;
		case 4: result = BP_UPPER_ARM; break;
		case 5: result = BP_HEAD; break;
		case 6: result = BP_UPPER_LEG; break;
		case 7: result = BP_LOWER_LEG; break;
		case 8: result = BP_FOOT; break;
		case 9: result = BP_ABDOMEN; break;
		}
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

	if (data.damage_form & DF_PHYSICAL)
	{
		std::list<long> bodyParts;
		if (data.target->m_Qualities._body)
		{
			int hitHeight;
			if (data.hit_quadrant & DQ_HIGH)
				hitHeight = BODY_HEIGHT::HIGH_BODY_HEIGHT;
			else if (data.hit_quadrant & DQ_MEDIUM)
				hitHeight = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;
			else if (data.hit_quadrant & DQ_LOW)
				hitHeight = BODY_HEIGHT::LOW_BODY_HEIGHT;
			else
				hitHeight = BODY_HEIGHT::MEDIUM_BODY_HEIGHT;

			//todo: taking into account the rest of the quadrant data(front/sides/etc)

			for (auto &bp : data.target->m_Qualities._body->_body_part_table)
			{
				if (bp.second._bh == hitHeight)
					bodyParts.push_back(bp.first);
			}
		}

		if (bodyParts.size())
		{
			std::list<long>::iterator randomBodyPartIterator = bodyParts.begin();
			std::advance(randomBodyPartIterator, Random::GenInt(0, (int)(bodyParts.size() - 1)));
			data.hitPart = (BODY_PART_ENUM)*randomBodyPartIterator;
		}
		else
			data.hitPart = GetRandomBodyPartByDamageQuadrant(data.hit_quadrant);
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
			killerMessage = victimName + "is liquified by your attack!";
			victimMessage = "You are liquified by " + killerName + "'s attack!";
			otherMessage = victimName + "is liquified by " + killerName + "'s attack!";
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

	bool isShield = InqIntQuality(COMBAT_USE_INT, 0, TRUE) == COMBAT_USE::COMBAT_USE_SHIELD;
	if (isShield && (damageData.hit_quadrant & DQ_FRONT) == 0)
		return 0.0f; //shield only works if the attack was from the front.

	EnchantedQualityDetails buffDetails;
	GetIntEnchantmentDetails(ARMOR_LEVEL_INT, 0, &buffDetails);

	if (damageData.isArmorRending && damageData.rendingMultiplier < buffDetails.valueDecreasingMultiplier)
	{
		//our armor rending is better than the debuffs applied, replace debuffs with rending.
		buffDetails.valueDecreasingMultiplier = damageData.rendingMultiplier;
		buffDetails.CalculateEnchantedValue();
	}

	if (bIgnoreMagicArmor)
		armorLevel = buffDetails.enchantedValue_DecreasingOnly; //debuffs still count
	else
		armorLevel = buffDetails.enchantedValue;

	switch (damageData.damage_type)
	{
	case DAMAGE_TYPE::SLASH_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_SLASH_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::PIERCE_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_PIERCE_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::BLUDGEON_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_BLUDGEON_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::FIRE_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_FIRE_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::COLD_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_COLD_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::ACID_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_ACID_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::ELECTRIC_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_ELECTRIC_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	case DAMAGE_TYPE::NETHER_DAMAGE_TYPE: armorLevel *= max(0.0, min(InqFloatQuality(ARMOR_MOD_VS_NETHER_FLOAT, 1.0f, bIgnoreMagicArmor), 2.0f)); break;
	}

	//todo: what about IGNORE_SHIELDS_BY_SKILL_BOOL?
	if (isShield)
	{
		double ignoreShieldMod = 0.0f;
		if (damageData.source)
			damageData.source->m_Qualities.InqFloat(IGNORE_SHIELD_FLOAT, ignoreShieldMod, FALSE);

		armorLevel *= (1.0 - ignoreShieldMod);
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
	case HEALTH_DAMAGE_TYPE:
		if(damageData.damageAfterMitigation > 0)
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
	}

	if (damageData.isElementalRending && damageData.rendingMultiplier > buffDetails.valueIncreasingMultiplier)
	{
		//our elemental rending is better than the debuffs applied, replace debuffs with rending.
		//we don't need to check if the elemental rending is of the correct type here as the check is already done on the attacker's side
		//and the flag is only true if the damage type matches the rending element.
		buffDetails.valueIncreasingMultiplier = damageData.rendingMultiplier;
		buffDetails.CalculateEnchantedValue();
	}

	float resistanceRegular = buffDetails.enchantedValue;
	if (damageData.ignoreMagicResist)
		resistanceRegular = buffDetails.enchantedValue_IncreasingOnly; //debuffs still count

	if (damageData.damageAfterMitigation > 0 || damageData.damage_type == HEALTH_DAMAGE_TYPE || damageData.damage_type == STAMINA_DAMAGE_TYPE || damageData.damage_type == MANA_DAMAGE_TYPE)
	{
		if (!AsPlayer() || damageData.ignoreMagicResist)
			damageData.damageAfterMitigation *= resistanceRegular;
		else //only players have natural resistances.
		{
			//Some combination of strength and endurance allows one to have a level of "natural resistances" to the 7 damage types.This caps out at a 50 % resistance(the equivalent to level 5 life prots) to these damage types.This resistance is not additive to life protections : higher level life protections will overwrite these natural resistances, although life vulns will take these natural resistances into account, if the player does not have a higher level life protection cast upon them.
			//For example, a player will not get a free protective bonus from natural resistances if they have both Prot 7 and Vuln 7 cast upon them.The Prot and Vuln will cancel each other out, and since the Prot has overwritten the natural resistances, there will be no resistance bonus.
			//The abilities that Endurance or Endurance/Strength conveys are not increased by Strength or Endurance buffs.It is the raw Strength and/or Endurance scores that determine the various bonuses.
			//drain resistances(same formula as natural resistances) allows one to partially resist drain health/stamina/mana and harm attacks, up to a maximum of roughly 50%. 

			//todo: natural resistances only change when base strength or endurance changes so we could potentially pre-calculate this somewhere else.
			DWORD strength = 0;
			DWORD endurance = 0;
			m_Qualities.InqAttribute(STRENGTH_ATTRIBUTE, strength, true);
			m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
			float strAndEnd = (float)(strength + endurance);
			float resistanceNatural;
			if (strAndEnd <= 200) //formula deduced from values present in the client pdb.
				resistanceNatural = 1.0f - ((0.05 * strAndEnd) / 100.f);
			else
				resistanceNatural = 1.0f - (((0.1666667 * strAndEnd) - 23.33333) / 100.f);

			resistanceNatural = max(resistanceNatural, 0.5);

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

		if (shieldOrMissileWeapon)
		{
			if (shieldOrMissileWeapon->GetImbueEffects() & ImbuedEffectType::IgnoreSomeMagicProjectileDamage_ImbuedEffectType)
			{
				// not worrying about if trained or specailized or using shield skill vs magic defense skill for now
				// RED = 100 % * ((CAP * ST * SKILL * 0.0030) - (CAP * ST *.3))

				double reduction = (0.25 * GetMagicDefense() * 0.003) - (0.25 * 0.3);

				if (reduction > 0.0)
				{
					if (reduction > 0.25)
						reduction = 0.25;

					damageData.damageAfterMitigation *= 1.0 - reduction;
				}
			}
		}
	}

	if (damageData.damageAfterMitigation < 0)
	{
		//todo: can't all damage types heal? Like hitting a fire elemental with fire should heal it instead of damage it?
		if (damageData.damage_type & (HEALTH_DAMAGE_TYPE|STAMINA_DAMAGE_TYPE|MANA_DAMAGE_TYPE))
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

	STypeAttribute2nd vitalAffected;
	switch (damageData.damage_type)
	{
	default: vitalAffected = HEALTH_ATTRIBUTE_2ND; break;
	case STAMINA_DAMAGE_TYPE: vitalAffected = STAMINA_ATTRIBUTE_2ND; break;
	case MANA_DAMAGE_TYPE: vitalAffected = MANA_ATTRIBUTE_2ND; break;
	}

	DWORD vitalStartValue = 0;
	DWORD vitalMinValue = 0;
	DWORD vitalMaxValue = 0;
	m_Qualities.InqAttribute2nd(vitalAffected, vitalStartValue, FALSE);
	m_Qualities.InqAttribute2nd((STypeAttribute2nd)((int)vitalAffected - 1), vitalMaxValue, FALSE);

	int vitalNewValue = vitalStartValue - damageData.outputDamageFinal;
	vitalNewValue = max((int)vitalMinValue, min((int)vitalMaxValue, vitalNewValue));

	damageData.outputDamageFinal = (int)vitalStartValue - (int)vitalNewValue;
	damageData.outputDamageFinalPercent = damageData.outputDamageFinal / (double)vitalMaxValue;

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

	/*
	if (damageData.outputDamageFinal >= 0)
	{
		switch (damageData.damage_type)
		{
		default:
			{
				if (damageData.outputDamageFinal >= GetHealth())
				{
					damageData.killingBlow = true;
					damageData.outputDamageFinal = GetHealth();
					damageData.outputDamageFinalPercent = ((float)damageData.outputDamageFinal / GetMaxHealth());
					SetHealth(0);
					OnDeath(damageData.source ? damageData.source->GetID() : 0);
				}
				else
				{
					damageData.outputDamageFinalPercent = ((float)damageData.outputDamageFinal / GetMaxHealth());
					SetHealth(GetHealth() - damageData.outputDamageFinal);

					if (damageData.outputDamageFinalPercent >= 0.1f)
					{
						if (_IsPlayer())
						{
							EmitSound(Random::GenInt(12, 14), 1.0f);
						}
					}
				}
				break;
			}
		}
	}
	*/

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

		if (damageData.killingBlow)
		{
			damageData.source->GivePerksForKill(this);
		}
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

void CWeenieObject::NotifyDeathMessage(DWORD killer_id, const char *message)
{
	BinaryWriter deathMsg;
	deathMsg.Write<DWORD>(0x19E);
	deathMsg.WriteString(message);
	deathMsg.Write<DWORD>(GetID());
	deathMsg.Write<DWORD>(killer_id);

	if (_IsPlayer() && (g_pConfig->ShowDeathMessagesGlobally() || (g_pConfig->ShowPlayerDeathMessagesGlobally() && g_pWorld->FindPlayer(killer_id) != NULL)))
	{
		g_pWorld->BroadcastGlobal(&deathMsg, PRIVATE_MSG, 0, FALSE, FALSE);
	}
	else
	{
		g_pWorld->BroadcastPVS(GetLandcell(), deathMsg.GetData(), deathMsg.GetSize(), PRIVATE_MSG);
	}
}

void CWeenieObject::OnTookDamage(DamageEventData &data)
{
	if (data.damage_form & DF_PHYSICAL && data.outputDamageFinal >= 0)
	{
		if (data.hit_quadrant & DQ_LOW)
			EmitEffect(Random::GenInt(91, 94), 1.0f);

		if (!(data.hit_quadrant & DQ_HEIGHT_MASK) || data.hit_quadrant & DQ_MEDIUM)
			EmitEffect(Random::GenInt(95, 98), 1.0f);

		if (data.hit_quadrant & DQ_HIGH)
			EmitEffect(Random::GenInt(99, 102), 1.0f);
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
				SendText(csprintf("%s %s you for %d points with %s.", data.GetSourceName().c_str(), plural_adj.c_str(), max(0, data.outputDamageFinal), data.spell_name.c_str()), LTT_MAGIC);
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
					char damageNumText[32];
					_itoa(abs(data.outputDamageFinal), damageNumText, 10);
					ReplaceString(activationTalkString, "%i", damageNumText);
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
			NotifyAttackerEvent(
				data.GetTargetName().c_str(),
				data.damage_type,
				data.outputDamageFinalPercent,
				data.outputDamageFinal,
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
				SendText(csprintf("%sYou %s %s for %d points with %s.", data.wasCrit ? "Critical hit! " : "", single_adj.c_str(), data.GetTargetName().c_str(), data.outputDamageFinal, data.spell_name.c_str()), LTT_MAGIC);
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

bool CWeenieObject::IsContainedWithinViewable(DWORD object_id)
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

	return false;
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

	double elapsed = watch.GetElapsed();
	if (elapsed >= 0.1)
	{
		LOG_PRIVATE(Temp, Warning, "Took %f seconds to save %s\n", elapsed, GetName().c_str());
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

CWeenieObject *CWeenieObject::Load(DWORD weenie_id)
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

DWORD CWeenieObject::GetTopLevelID()
{
	DWORD container_id = InqIIDQuality(CONTAINER_IID, 0);
	if (container_id)
	{
		if (CWeenieObject *pContainer = g_pWorld->FindObject(container_id))
		{
			return pContainer->GetTopLevelID();
		}
		else
		{
			LOG(Temp, Error, "Could not find parent container weenie 0x%08X for 0x%08X in GetTopLevelID()\n", container_id, GetID());
		}
	}

	DWORD wielder_id = InqIIDQuality(WIELDER_IID, 0);
	if (wielder_id)
	{
		if (CWeenieObject *pWielder = g_pWorld->FindObject(wielder_id))
		{
			return pWielder->GetTopLevelID();
		}
		else
		{
			LOG(Temp, Error, "Could not find parent wielder weenie 0x%08X for 0x%08X in GetTopLevelID()\n", wielder_id, GetID());
		}
	}

	return GetID();
}

int CWeenieObject::UseChecked(CPlayerWeenie *other)
{
	if (IsBusyOrInAction() || IsDead())
		return WERROR_ACTIONS_LOCKED;

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

int CWeenieObject::Activate(DWORD activator_id)
{
	if (m_Qualities._generator_registry)
	{
		GeneratorRegistryNode *node = m_Qualities._generator_registry->_registry.lookup(activator_id);

		if (node)
		{
			m_Qualities._generator_registry->_registry.remove(activator_id);

			// find this profile slot
			double delay = -1.0;
			if (m_Qualities._generator_table)
			{
				for (auto &profile : m_Qualities._generator_table->_profile_list)
				{
					if (profile.slot == node->slot)
					{
						delay = profile.delay * g_pConfig->RespawnTimeMultiplier();;
						break;
					}
				}
			}

			if (delay >= 0)
			{
				if (!m_Qualities._generator_queue)
					m_Qualities._generator_queue = new GeneratorQueue();

				GeneratorQueueNode queueNode;
				queueNode.slot = node->slot;
				queueNode.when = Timer::cur_time + delay;
				m_Qualities._generator_queue->_queue.push_back(queueNode);

				if (_nextRegen < 0.0)
					_nextRegen = Timer::cur_time + (InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0.0, TRUE) * g_pConfig->RespawnTimeMultiplier());
			}
		}
	}

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
	if (m_EmoteManager && m_EmoteManager->IsExecutingAlready())
	{
		return true;
	}

	return false;
}

void CWeenieObject::ChanceExecuteEmoteSet(DWORD other_id, EmoteCategory category)
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

void CWeenieObject::DoActivationEmote(DWORD activator_id)
{
	ChanceExecuteEmoteSet(activator_id, Activation_EmoteCategory);
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

void CWeenieObject::HandleMoveToDone(DWORD error)
{
	if (m_UseManager)
	{
		m_UseManager->HandleMoveToDone(error);
	}
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

bool CWeenieObject::HasFellowship()
{
	std::string dummy;
	if (m_Qualities.InqString(FELLOWSHIP_STRING, dummy))
		return true;

	return false;
}

void CWeenieObject::LeaveFellowship()
{
	std::string fellowshipName;
	if (m_Qualities.InqString(FELLOWSHIP_STRING, fellowshipName))
	{
		g_pFellowshipManager->Quit(fellowshipName, GetID());
		m_Qualities.RemoveString(FELLOWSHIP_STRING);
	}
}

Fellowship *CWeenieObject::GetFellowship()
{
	std::string fellowshipName;
	if (m_Qualities.InqString(FELLOWSHIP_STRING, fellowshipName))
	{
		return g_pFellowshipManager->GetFellowship(fellowshipName);
	}

	return NULL;
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
		if(InqIntQuality(DEFAULT_COMBAT_STYLE_INT, CombatStyle::Undef_CombatStyle) & Magic_CombatStyle)
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
	DWORD top_level_id = GetTopLevelID();

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
			statNotify.Write<DWORD>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(CONTAINER_IID);
			statNotify.Write<DWORD>(0);
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

		/*
		if (bBroadcastContainerChange)
		{
			NotifyIIDStatUpdated(CONTAINER_IID, false);
		}
		*/
	}

	if (IsWielded())
	{
		if (m_bWorldIsAware && bBroadcastEquipmentChange)
		{
			BinaryWriter statNotify;
			BYTE statTS = GetNextStatTimestamp(IID_StatType, WIELDER_IID);
			statNotify.Write<DWORD>(0x2DA);
			statNotify.Write<BYTE>(statTS);
			statNotify.Write<DWORD>(GetID());
			statNotify.Write<DWORD>(WIELDER_IID);
			statNotify.Write<DWORD>(0);
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
				PackableListWithJson<DWORD> spells_to_remove;

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
						silentRemoveMessage.Write<DWORD>(0x2C8);
						spells_to_remove.Pack(&silentRemoveMessage);

						wielderObj->SendNetMessage(&silentRemoveMessage, PRIVATE_MSG, TRUE, FALSE);
					}

					wielderObj->CheckVitalRanges();
				}
			}

			if (m_Qualities._enchantment_reg)
			{
				PackableListWithJson<DWORD> spells_to_remove;

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
	
		/*
		if (bBroadcastEquipmentChange)
		{
			NotifyIIDStatUpdated(WIELDER_IID, false);
		}
		*/
	}

	_cachedHasOwner = false;
}

CWeenieObject *CWeenieObject::FindContained(DWORD object_id)
{
	return NULL;
}

void CWeenieObject::SetWeenieContainer(DWORD container_id)
{
	m_Qualities.SetInstanceID(CONTAINER_IID, container_id);
	NotifyIIDStatUpdated(CONTAINER_IID, false);

	RecacheHasOwner();
}

void CWeenieObject::SetWielderID(DWORD wielder_id)
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

void CWeenieObject::SetWieldedLocation(DWORD inv_loc)
{
	m_Qualities.SetInt(CURRENT_WIELDED_LOCATION_INT, inv_loc);
	NotifyIntStatUpdated(CURRENT_WIELDED_LOCATION_INT, false);
}

bool CWeenieObject::IsValidWieldLocation(DWORD location)
{
	if (InqIntQuality(LOCATIONS_INT, 0, TRUE) & location)
		return true;

	return false;
}

bool CWeenieObject::CanEquipWith(CWeenieObject *other, DWORD otherLocation)
{
	if (InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0, TRUE) & otherLocation)
	{
		return false;
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
			if (amount > 1)
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

	return WERROR_NONE;
}

void CWeenieObject::SimulateGiveObject(class CContainerWeenie *target_container, DWORD wcid, int amount, int ptid, float shade, int bondedType)
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

	if(bondedType != 0)
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
			if (amount > 1)
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

void CWeenieObject::SendNetMessage(void *_data, DWORD _len, WORD _group, BOOL _event)
{
}

void CWeenieObject::SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event, BOOL should_delete)
{
	if (should_delete)
	{
		delete _food;
	}
}

void CWeenieObject::SendNetMessageToTopMost(void *_data, DWORD _len, WORD _group, BOOL _event)
{
	if (AsPlayer())
	{
		// obviously we are the top most
		SendNetMessage(_data, _len, _group, _event);
		return;
	}

	if (CWeenieObject *topMost = GetWorldTopLevelOwner())
	{
		// we are wielded or contained, send to topmost owner
		topMost->SendNetMessage(_data, _len, _group, _event);
		return;
	}
}

void CWeenieObject::SendNetMessageToTopMost(BinaryWriter *_food, WORD _group, BOOL _event, BOOL should_delete)
{
	if (AsPlayer())
	{
		// obviously we are the top most
		SendNetMessage(_food, _group, _event, should_delete);
		return;
	}

	if (CWeenieObject *topMost = GetWorldTopLevelOwner())
	{
		// we are wielded or contained, send to topmost owner
		topMost->SendNetMessage(_food, _group, _event, should_delete);
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

DWORD CWeenieObject::GetMagicDefense()
{
	DWORD defenseSkill = 0;
	InqSkill(MAGIC_DEFENSE_SKILL, defenseSkill, FALSE);
	return defenseSkill;
}

bool CWeenieObject::TryMagicResist(DWORD magicSkill)
{
	DWORD defenseSkill = GetMagicDefense();

	double defenseMod = GetMagicDefenseModUsingWielded();
	if (AsContainer())
	{
		for (auto item : AsContainer()->m_Wielded)
		{
			if (item->GetImbueEffects() & MagicDefense_ImbuedEffectType)
				defenseMod += 0.01;
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
	double mod = InqFloatQuality(WEAPON_DEFENSE_FLOAT, 1.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_DEFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= wielder->GetMeleeDefenseMod();
	}

	return mod;
}

double CWeenieObject::GetMissileDefenseModUsingWielded()
{
	return GetMissileDefenseMod();
}

double CWeenieObject::GetMissileDefenseMod()
{
	double mod = InqFloatQuality(WEAPON_MISSILE_DEFENSE_FLOAT, 1.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_MISSILE_DEFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= wielder->GetMissileDefenseMod();
	}

	return mod;
}

double CWeenieObject::GetMagicDefenseModUsingWielded()
{
	return GetMagicDefenseMod();
}

double CWeenieObject::GetMagicDefenseMod()
{
	double mod = InqFloatQuality(WEAPON_MAGIC_DEFENSE_FLOAT, 1.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_MAGIC_DEFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= wielder->GetMagicDefenseMod();
	}

	return mod;
}

double CWeenieObject::GetOffenseMod()
{
	double mod = InqFloatQuality(WEAPON_OFFENSE_FLOAT, 1.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(WEAPON_OFFENSE_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= wielder->GetOffenseMod();
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
	DWORD imbueEffects = GetImbueEffects();

	if (imbueEffects & effect)
		return; //it's already present.

	imbueEffects |= effect;
	m_Qualities.SetInt(IMBUED_EFFECT_INT, (int)effect);

	//imbue effects 2 to 5 are not used by our dataset.
	//if (!InqIntQuality(IMBUED_EFFECT_INT, 0, FALSE))
	//	m_Qualities.SetInt(IMBUED_EFFECT_INT, (int)effect);
	//else if (!InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE))
	//	m_Qualities.SetInt(IMBUED_EFFECT_2_INT, (int)effect);
	//else if (!InqIntQuality(IMBUED_EFFECT_3_INT, 0, FALSE))
	//	m_Qualities.SetInt(IMBUED_EFFECT_3_INT, (int)effect);
	//else if (!InqIntQuality(IMBUED_EFFECT_4_INT, 0, FALSE))
	//	m_Qualities.SetInt(IMBUED_EFFECT_4_INT, (int)effect);
	//else if (!InqIntQuality(IMBUED_EFFECT_5_INT, 0, FALSE))
	//	m_Qualities.SetInt(IMBUED_EFFECT_5_INT, (int)effect);
	//else
	//	return false;
	//return true;
}

DWORD CWeenieObject::GetImbueEffects()
{
	return InqIntQuality(IMBUED_EFFECT_INT, 0, FALSE);

	//imbue effects 2 to 5 are not used by our dataset.
	//DWORD imbuedEffect = 0;

	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_INT, 0, FALSE);
	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE);
	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_2_INT, 0, FALSE);
	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_3_INT, 0, FALSE);
	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_4_INT, 0, FALSE);
	//imbuedEffect |= (DWORD) InqIntQuality(IMBUED_EFFECT_5_INT, 0, FALSE);

	//return imbuedEffect;
}

double CWeenieObject::GetManaConversionMod()
{
	double mod = InqFloatQuality(MANA_CONVERSION_MOD_FLOAT, 0.0, FALSE);

	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantFloat(MANA_CONVERSION_MOD_FLOAT, &mod);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		mod *= (1.0 + wielder->GetManaConversionMod());
	}

	return mod;
}

DWORD CWeenieObject::GetEffectiveManaConversionSkill()
{
	SKILL_ADVANCEMENT_CLASS sac;
	DWORD manaConvSkill = 0;

	//mana conversion is an unuseable untrained skill, which means it's always 0 if you don't have it at least trained.
	if (!m_Qualities.InqSkillAdvancementClass(MANA_CONVERSION_SKILL, sac) || sac >= TRAINED_SKILL_ADVANCEMENT_CLASS)
		InqSkill(MANA_CONVERSION_SKILL, manaConvSkill, FALSE);

	if (CWeenieObject *wand = GetWieldedCaster())
	{
		double manaMod = wand->GetManaConversionMod();

		if (manaMod > 0.0)
		{
			manaConvSkill = (DWORD)max(0, ((int)manaConvSkill * (1.0 + manaMod)));
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

int CWeenieObject::GetAttackTimeUsingWielded()
{
	return GetAttackTime();
}

int CWeenieObject::GetAttackDamage()
{
	int damage = InqIntQuality(DAMAGE_INT, 0, TRUE);
	
	if (m_Qualities._enchantment_reg)
		m_Qualities._enchantment_reg->EnchantInt(DAMAGE_INT, &damage, FALSE);

	CWeenieObject *wielder = GetWorldWielder();
	if (wielder && InqIntQuality(RESIST_MAGIC_INT, 0, FALSE) < 9999)
	{
		damage += wielder->GetAttackDamage();
	}

	return damage;
}

bool CWeenieObject::IsInPeaceMode()
{
	return get_minterp()->interpreted_state.current_style == Motion_NonCombat;
}

bool CWeenieObject::TryMeleeEvade(DWORD attackSkill)
{
	if (GetStamina() < 1) // when we're out of stamina our defense skill is lowered to 0.
		return false;

	DWORD defenseSkill = 0;
	InqSkill(MELEE_DEFENSE_SKILL, defenseSkill, FALSE);

	double defenseMod = GetMeleeDefenseModUsingWielded();

	bool inCombatMode = true;
	if (get_minterp()->interpreted_state.current_style == Motion_NonCombat)
	{
		inCombatMode = false;
		switch (get_minterp()->interpreted_state.forward_command)
		{
		default:
			defenseMod *= 0.8;
			break;
		case Motion_Crouch:
			defenseMod *= 0.5;
			break;
		case Motion_Sitting:
			defenseMod *= 0.25;
			break;
		case Motion_Sleeping:
			defenseMod *= 0.1;
			break;
		}
	}

	defenseSkill = (int)round((double)defenseSkill * defenseMod);

	bool success = ::TryMeleeEvade(attackSkill, defenseSkill);

	if (inCombatMode)
	{
		if (success)
		{
			if (AsPlayer())
			{
				//the higher a player's Endurance, the more likely they are not to use a point of stamina to successfully evade a missile or melee attack.
				//A player is required to have Melee Defense for melee attacks or Missile Defense for missile attacks trained or specialized in order 
				//for this specific ability to work. This benefit is tied to Endurance only, and it caps out at around a 75% chance to avoid losing
				//a point of stamina per successful evasion. 
				SKILL_ADVANCEMENT_CLASS defenseSkillSAC = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
				m_Qualities.InqSkillAdvancementClass(STypeSkill::MELEE_DEFENSE_SKILL, defenseSkillSAC);

				if (defenseSkillSAC >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
				{
					DWORD endurance = 0;
					m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
					float noStaminaUseChance = ((float)endurance - 100.0) / 400.0; //made up formula: 75% reduction at 400 endurance.
					noStaminaUseChance = min(max(noStaminaUseChance, 0.0), 0.75);
					if (Random::RollDice(0.0, 1.0) > noStaminaUseChance)
						AdjustStamina(-1); // failed the roll, use stamina.
				}
				else
					AdjustStamina(-1); // defense skill not trained/specialized, use stamina.
			}
			else
				AdjustStamina(-1);
		}
		else
			AdjustStamina(-1); // while in combat all evasion failures consume 1 stamina.
	}

	return success;
}

bool CWeenieObject::TryMissileEvade(DWORD attackSkill)
{
	if (GetStamina() < 1) // when we're out of stamine our defense skill is lowered to 0.
		return false;

	DWORD defenseSkill = 0;
	InqSkill(MISSILE_DEFENSE_SKILL, defenseSkill, FALSE);

	double defenseMod = GetMissileDefenseModUsingWielded();
	if (AsContainer())
	{
		for (auto item : AsContainer()->m_Wielded)
		{
			if (item->GetImbueEffects() & MissileDefense_ImbuedEffectType)
				defenseMod += 0.01;
		}
	}

	bool inCombatMode = true;
	if (get_minterp()->interpreted_state.current_style == Motion_NonCombat)
	{
		inCombatMode = false;
		switch (get_minterp()->interpreted_state.forward_command)
		{
		default:
			defenseMod *= 0.8;
			break;
		case Motion_Crouch:
			defenseMod *= 0.5;
			break;
		case Motion_Sitting:
			defenseMod *= 0.25;
			break;
		case Motion_Sleeping:
			defenseMod *= 0.1;
			break;
		}
	}

	defenseSkill = (int)round((double)defenseSkill * defenseMod);

	bool success = ::TryMissileEvade(attackSkill, defenseSkill);

	if (inCombatMode)
	{
		if (success)
		{
			//the higher a player's Endurance, the more likely they are not to use a point of stamina to successfully evade a missile or melee attack.
			//A player is required to have Melee Defense for melee attacks or Missile Defense for missile attacks trained or specialized in order 
			//for this specific ability to work. This benefit is tied to Endurance only, and it caps out at around a 75% chance to avoid losing
			//a point of stamina per successful evasion. 
			SKILL_ADVANCEMENT_CLASS defenseSkillSAC = SKILL_ADVANCEMENT_CLASS::UNTRAINED_SKILL_ADVANCEMENT_CLASS;
			m_Qualities.InqSkillAdvancementClass(STypeSkill::MISSILE_DEFENSE_SKILL, defenseSkillSAC);

			if (defenseSkillSAC >= SKILL_ADVANCEMENT_CLASS::TRAINED_SKILL_ADVANCEMENT_CLASS)
			{
				DWORD endurance = 0;
				m_Qualities.InqAttribute(ENDURANCE_ATTRIBUTE, endurance, true);
				float noStaminaUseChance = ((float)endurance - 100.0) / 400.0; //made up formula: 75% reduction at 400 endurance.
				noStaminaUseChance = min(max(noStaminaUseChance, 0.0), 0.75);
				if (Random::RollDice(0.0, 1.0) > noStaminaUseChance)
					AdjustStamina(-1); // failed the roll, use stamina.
			}
			else
				AdjustStamina(-1); // defense skill not trained/specialized, use stamina.
		}
		else
			AdjustStamina(-1); // while in combat all evasion failures consume 1 stamina.
	}

	return success;
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

	if (m_EmoteManager)
		m_EmoteManager->Cancel();

	if (m_AttackManager)
		m_AttackManager->Cancel();
}

void CWeenieObject::Movement_Teleport(const Position &position, bool bWasDeath)
{
	assert(position.objcell_id);
	assert(!parent);

	OnTeleported();

	last_move_was_autonomous = false;

	DWORD dwOldLastCell = GetLandcell();

	if (weenie_obj && weenie_obj->_IsPlayer())
		EnterPortal(dwOldLastCell);

	SetPositionStruct sps;
	sps.pos = position;
	sps.SetFlags(SEND_POSITION_EVENT_SPF | SLIDE_SPF | PLACEMENT_SPF | TELEPORT_SPF);
	SetPosition(sps);

	_teleport_timestamp += 2;

	if (bWasDeath)
	{
		// Send position and movement -- only seen this happen on death so far
		BinaryWriter positionAndMovement;
		positionAndMovement.Write<DWORD>(0xF619);
		positionAndMovement.Write<DWORD>(GetID());

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

void CWeenieObject::TryMeleeAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion)
{
}

void CWeenieObject::TryMissileAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion)
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
	if (m_Qualities.InqInt(STRUCTURE_INT, dummy, TRUE))
		DecrementStructureNum(amount, bDestroyOnZero);
	else
		DecrementStackNum(amount, bDestroyOnZero);
}

void CWeenieObject::DecrementStackNum(int amount, bool bDestroyOnZero)
{
	int usesLeft = InqIntQuality(STACK_SIZE_INT, 1);

	usesLeft -= amount;

	if (usesLeft <= 0)
	{
		if(bDestroyOnZero)
			Remove();
		else
			SetStackSize(0);
	}
	else
		SetStackSize(usesLeft);
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

void CWeenieObject::SetStackSize(DWORD stackSize)
{
	if (!m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0))
	{
		// not a stackable... ??
		return;
	}

	m_Qualities.SetInt(STACK_SIZE_INT, stackSize);
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(STACK_SIZE_INT, false);

	m_Qualities.SetInt(ENCUMB_VAL_INT, stackSize * InqIntQuality(STACK_UNIT_ENCUMB_INT, 0));
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(ENCUMB_VAL_INT, false);

	m_Qualities.SetInt(VALUE_INT, stackSize * InqIntQuality(STACK_UNIT_VALUE_INT, 0));
	//if (m_bWorldIsAware)
	//	NotifyIntStatUpdated(VALUE_INT, false);

	if (m_bWorldIsAware)
		NotifyStackSizeUpdated(false);

	if (CWeenieObject *owner = GetWorldTopLevelOwner())
	{
		owner->RecalculateEncumbrance();
		if (owner->AsPlayer() && m_Qualities.id == W_COINSTACK_CLASS)
			owner->RecalculateCoinAmount();
	}
}

bool CWeenieObject::IsAvatarJumpsuit()
{
	return m_Qualities.id >= 7000000 && m_Qualities.id <= 7010000;
}

bool CWeenieObject::LearnSpell(DWORD spell_id, bool showTextAndEffect)
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
				addSpellToSpellbook.Write<DWORD>(0x2C1);
				addSpellToSpellbook.Write<DWORD>(spell_id);
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
	MarkForDestroy();
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

BOOL CWeenieObject::InqSkill(STypeSkill key, DWORD &value, BOOL raw)
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

void CWeenieObject::CheckEventState()
{
	if (!m_Qualities._generator_table)
	{
		return;
	}

	std::string eventName;
	if (m_Qualities.InqString(GENERATOR_EVENT_STRING, eventName))
	{
		if (g_pGameEventManager->IsEventStarted(eventName.c_str()))
		{
			HandleEventActive();
		}
		else
		{
			HandleEventInactive();
		}
	}
}

void CWeenieObject::HandleEventActive()
{
	// if we have any spawns, we shouldn't be needing activation
	if (m_Qualities._generator_registry && !m_Qualities._generator_registry->_registry.empty())
		return;
	if (m_Qualities._generator_queue && !m_Qualities._generator_queue->_queue.empty())
		return;

	g_pWeenieFactory->AddFromGeneratorTable(this, false);
}

void CWeenieObject::HandleEventInactive()
{
	if (m_Qualities._generator_registry)
	{
		while (!m_Qualities._generator_registry->_registry.empty())
		{
			DWORD weenie_id = m_Qualities._generator_registry->_registry.begin()->first;
			
			if (CWeenieObject *spawned_weenie = g_pWorld->FindObject(weenie_id))
			{
				spawned_weenie->MarkForDestroy();
			}

			// make sure it's gone (it should be already.)
			m_Qualities._generator_registry->_registry.erase(weenie_id);
		}
	}

	if (m_Qualities._generator_queue)
	{
		m_Qualities._generator_queue->_queue.clear();
	}
}
