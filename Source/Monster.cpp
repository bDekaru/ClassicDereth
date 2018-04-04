
#include "StdAfx.h"
#include "WeenieObject.h"
#include "Monster.h"
#include "World.h"
#include "GameMode.h"
#include "Lifestone.h"
#include "ChatMsgs.h"
#include "SpellProjectile.h"
#include "MonsterAI.h"
#include "WeenieFactory.h"
#include "InferredPortalData.h"
#include "ObjectMsgs.h"
#include "EmoteManager.h"
#include "Corpse.h"
#include "AttackManager.h"
#include "WClassID.h"
#include "InferredPortalData.h"
#include "Config.h"
#include "DatabaseIO.h"
#include "InferredPortalData.h"
#include "SpellTableExtendedData.h"
#include "Door.h"
#include "Player.h"
#include "House.h"

CMonsterWeenie::CMonsterWeenie()
{
	SetItemType(TYPE_CREATURE);

	m_Qualities.SetInt(LEVEL_INT, 1);
	m_Qualities.SetAttribute(STRENGTH_ATTRIBUTE, 100);
	m_Qualities.SetAttribute(ENDURANCE_ATTRIBUTE, 100);
	m_Qualities.SetAttribute(COORDINATION_ATTRIBUTE, 100);
	m_Qualities.SetAttribute(QUICKNESS_ATTRIBUTE, 100);
	m_Qualities.SetAttribute(FOCUS_ATTRIBUTE, 100);
	m_Qualities.SetAttribute(SELF_ATTRIBUTE, 100);
	m_Qualities.SetAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, 0);
	m_Qualities.SetAttribute2nd(HEALTH_ATTRIBUTE_2ND, 50);
	m_Qualities.SetAttribute2nd(MAX_STAMINA_ATTRIBUTE_2ND, 0);
	m_Qualities.SetAttribute2nd(STAMINA_ATTRIBUTE_2ND, 100);
	m_Qualities.SetAttribute2nd(MAX_MANA_ATTRIBUTE_2ND, 0);
	m_Qualities.SetAttribute2nd(MANA_ATTRIBUTE_2ND, 100);

	m_Qualities.SetInt(SHOWABLE_ON_RADAR_INT, ShowAlways_RadarEnum);

	m_Qualities.SetBool(STUCK_BOOL, TRUE);
	m_Qualities.SetBool(ATTACKABLE_BOOL, TRUE);
	m_Qualities.SetInt(ITEM_USEABLE_INT, USEABLE_NO);
}

CMonsterWeenie::~CMonsterWeenie()
{
	SafeDelete(m_MonsterAI);
}

void CMonsterWeenie::ApplyQualityOverrides()
{
}

void CMonsterWeenie::PreSpawnCreate()
{
	if (m_Qualities._create_list)
		g_pWeenieFactory->AddFromCreateList(this, m_Qualities._create_list, (DestinationType)(Wield_DestinationType | WieldTreasure_DestinationType));

	if (DWORD wieldedTreasureType = InqDIDQuality(WIELDED_TREASURE_TYPE_DID, 0))
		g_pWeenieFactory->GenerateFromTypeOrWcid(this, DestinationType::WieldTreasure_DestinationType, wieldedTreasureType);
}

void CMonsterWeenie::PostSpawn()
{
	CWeenieObject::PostSpawn();

	// check if we need to create a monster AI
	if (IsCreature() && !AsPlayer() && !IsVendor() && m_Qualities.GetInt(PLAYER_KILLER_STATUS_INT, 0) != RubberGlue_PKStatus)
	{
		m_MonsterAI = new MonsterAIManager(this, m_Position);
	}

	if (!_IsPlayer())
	{
		EmitEffect(PS_Create, 1.0f);
	}
}

CWeenieObject *CMonsterWeenie::SpawnWielded(DWORD index, SmartArray<Style_CG> possibleStyles, DWORD color, SmartArray<DWORD> validColors, long double shade)
{
	index = max(min(index, possibleStyles.num_used - 1), 0);
	shade = max(min(shade, 1), 0);

	DWORD wcid = possibleStyles.array_data[index].weenieDefault;
	DWORD clothingTable = possibleStyles.array_data[index].clothingTable;

	bool validColor = false;
	for (int i = 0; i < validColors.num_used; i++)
	{
		if (color == validColors.array_data[i])
		{
			validColor = true;
			break;
		}
	}
	if (!validColor)
		color = validColors.array_data[0];

	CWeenieObject *weenie = SpawnWielded(wcid, color, shade);
	weenie->m_Qualities.SetDataID(CLOTHINGBASE_DID, clothingTable);

	return weenie;
}

CWeenieObject *CMonsterWeenie::SpawnWielded(DWORD wcid, int ptid, float shade)
{
	CWeenieObject *item = g_pWeenieFactory->CreateWeenieByClassID(wcid, NULL, false);

	if (ptid)
		item->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);

	if (shade > 0.0)
		item->m_Qualities.SetFloat(SHADE_FLOAT, shade);

	return SpawnWielded(item);
}

CWeenieObject *CMonsterWeenie::SpawnWielded(CWeenieObject *item, bool deleteItemOnFailure)
{
	if (!item)
		return NULL;

	if (!item->IsEquippable())
	{
		LOG_PRIVATE(Temp, Warning, "Trying to SpawnWielded an unwieldable item %u\n", item->m_Qualities.id);
		delete item;

		return NULL;
	}

	item->SetID(g_pWorld->GenerateGUID(eDynamicGUID));

	if (!g_pWorld->CreateEntity(item, false))
	{
		delete item;
		return NULL;
	}

	if (!FinishMoveItemToWield(item, item->m_Qualities.GetInt(LOCATIONS_INT, 0)))
	{
		g_pWorld->RemoveEntity(item);
		item = NULL;
	}

	return item;
}

CWeenieObject *CMonsterWeenie::FindValidNearbyItem(DWORD itemId, float maxDistance)
{
	//do we have it?
	CWeenieObject *item = FindContainedItem(itemId);

	if (!item)
	{
		// maybe it's in an external container
		if (_lastOpenedRemoteContainerId != 0)
		{
			CContainerWeenie *externalContainer = FindContainer(_lastOpenedRemoteContainerId);
			if (externalContainer && externalContainer->_openedById == GetID())
			{
				item = externalContainer->FindContainedItem(itemId);
			}
		}

		if (!item)
		{
			// maybe it's on the ground
			item = g_pWorld->FindObject(itemId);

			if (!item || item->HasOwner() || !item->InValidCell())
			{
				NotifyInventoryFailedEvent(itemId, WERROR_OBJECT_GONE);
				return NULL;
			}

			if (DistanceTo(item, true) > maxDistance)
			{
				NotifyInventoryFailedEvent(itemId, WERROR_TOO_FAR);
				return NULL;;
			}
		}
	}

	if (!item->CanPickup())
	{
		NotifyInventoryFailedEvent(itemId, WERROR_NONE);
		return NULL;
	}

	return item;
}

CContainerWeenie *CMonsterWeenie::FindValidNearbyContainer(DWORD containerId, float maxDistance)
{
	//do we have it?
	CContainerWeenie *container = FindContainer(containerId);

	if (!container)
	{
		// maybe it's in the external container
		if (_lastOpenedRemoteContainerId != 0)
		{
			CContainerWeenie *externalContainer = FindContainer(_lastOpenedRemoteContainerId);
			if (externalContainer && externalContainer->_openedById == GetID())
				container = externalContainer->FindContainer(containerId);
		}

		if (!container)
		{
			// maybe it's on the ground
			CWeenieObject *object = g_pWorld->FindObject(containerId);
			container = object->AsContainer();

			if (!container || container->HasOwner() || !container->InValidCell())
			{
				NotifyInventoryFailedEvent(containerId, WERROR_OBJECT_GONE);
				return NULL;
			}

			if (DistanceTo(container, true) > maxDistance)
			{
				NotifyInventoryFailedEvent(containerId, WERROR_TOO_FAR);
				return NULL;;
			}
		}
	}

	if (container->GetID() != GetID() && container->IsCreature())
	{
		NotifyInventoryFailedEvent(containerId, WERROR_NONE);
		return NULL;
	}

	return container;
}

bool CMonsterWeenie::GetEquipPlacementAndHoldLocation(CWeenieObject *item, DWORD location, DWORD *pPlacementFrame, DWORD *pHoldLocation)
{
	if (location & MELEE_WEAPON_LOC)
	{
		*pPlacementFrame = Placement::RightHandCombat;
		*pHoldLocation = PARENT_RIGHT_HAND;
	}
	else if (location & MISSILE_WEAPON_LOC)
	{
		int combatStyle = item->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0);
		if (combatStyle == ThrownWeapon_CombatStyle || combatStyle == Atlatl_CombatStyle)
		{
			*pPlacementFrame = Placement::RightHandCombat;
			*pHoldLocation = PARENT_RIGHT_HAND;
		}
		else
		{
			*pPlacementFrame = Placement::LeftHand;
			*pHoldLocation = PARENT_LEFT_HAND;
		}
	}
	else if (location & SHIELD_LOC)
	{
		*pPlacementFrame = Placement::Shield;
		*pHoldLocation = PARENT_SHIELD;
	}
	else if (location & HELD_LOC)
	{
		*pPlacementFrame = Placement::RightHandCombat;
		*pHoldLocation = PARENT_RIGHT_HAND;
	}
	else if (location & MISSILE_AMMO_LOC)
	{
		//*pPlacementFrame = Placement::RightHandCombat;
		//*pHoldLocation = PARENT_RIGHT_HAND;

		*pPlacementFrame = 0;
		*pHoldLocation = 0;
	}
	else if (location & (ARMOR_LOC | CLOTHING_LOC))
	{
		*pPlacementFrame = 0;
		*pHoldLocation = 0;
		return false;
	}
	else
	{
		// LOG(Temp, Normal, "Trying to place %s but don't know how with coverage 0x%08X\n", item->GetName().c_str(), location);
		*pPlacementFrame = 0;
		*pHoldLocation = 0;
		return false;
	}

	return true;
}


BYTE CMonsterWeenie::GetEnchantmentSerialByteForMask(int priority)
{
	if (!priority)
		return 0;

	for (DWORD i = 0; i < 32; i++)
	{
		if (priority & 1)
			return i + 1;

		priority >>= 1;
	}

	return 0;
}

int CMonsterWeenie::CheckWieldRequirements(CWeenieObject *item, CWeenieObject *wielder, STypeInt requirementStat, STypeInt skillStat, STypeInt difficultyStat)
{
	int requirementType = item->InqIntQuality(requirementStat, 0, TRUE);

	if (!requirementType)
		return WERROR_NONE;

	int skillType = item->InqIntQuality(skillStat, 0, TRUE);
	int wieldDifficulty = item->InqIntQuality(difficultyStat, 0, TRUE);

	switch (requirementType)
	{
	case 1: // skill
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqSkill((STypeSkill)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 2: // base skill
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqSkill((STypeSkill)skillType, skillLevel, TRUE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 3: // attribute
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute((STypeAttribute)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 4: // base attribute
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute((STypeAttribute)skillType, skillLevel, TRUE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 5: // attribute 2nd 
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute2nd((STypeAttribute2nd)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 6: // attribute 2nd base
	{
		DWORD skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute2nd((STypeAttribute2nd)skillType, skillLevel, TRUE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 7: // level
	{
		if (wielder->InqIntQuality(LEVEL_INT, 1, TRUE) < wieldDifficulty)
			return WERROR_LEVEL_TOO_LOW;
		break;
	}
	case 8: // skill advancement class
	{
		SKILL_ADVANCEMENT_CLASS sac = SKILL_ADVANCEMENT_CLASS::UNDEF_SKILL_ADVANCEMENT_CLASS;
		if (!wielder->m_Qualities.InqSkillAdvancementClass((STypeSkill)skillType, sac) || (int)sac < wieldDifficulty)
			return WERROR_ACTIVATION_NOT_SPECIALIZED;
		break;
	}
	case 9: // faction
	{
		return WERROR_NONE; //todo
	}
	case 10: // unknown
	{
		return WERROR_NONE; //todo
	}
	case 11: // type
	{
		if (wielder->InqIntQuality(CREATURE_TYPE_INT, 0, TRUE) != wieldDifficulty)
			return WERROR_ACTIVATION_WRONG_RACE;
		break;
	}
	case 12: // race
	{
		if (wielder->InqIntQuality(HERITAGE_GROUP_INT, 0, TRUE) != wieldDifficulty)
			return WERROR_ACTIVATION_WRONG_RACE;
		break;
	}
	}
	return WERROR_NONE;
}

bool CMonsterWeenie::MoveItemToContainer(DWORD sourceItemId, DWORD targetContainerId, DWORD targetSlot, bool animationDone)
{
	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	CContainerWeenie *targetContainer = FindValidNearbyContainer(targetContainerId, USEDISTANCE_FAR);
	if (!targetContainer || !targetContainer->Container_CanStore(sourceItem))
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (sourceItem->IsAttunedOrContainsAttuned() && targetContainer->GetTopLevelID() != GetID())
	{
		NotifyInventoryFailedEvent(sourceItem->id, WERROR_ATTUNED_ITEM);
		return false;
	}

	bool isExternalTargetContainer = (targetContainer->GetTopLevelID() != GetID());

	if (!animationDone && (!FindContainedItem(sourceItem->GetID()) || isExternalTargetContainer))
	{
		//from ground or external container or to external container.
		CPickupInventoryUseEvent *pickupEvent = new CPickupInventoryUseEvent();
		pickupEvent->_target_id = sourceItemId;
		pickupEvent->_source_item_id = sourceItemId;
		pickupEvent->_target_container_id = targetContainerId;
		pickupEvent->_target_slot = targetSlot;
		ExecuteUseEvent(pickupEvent);
	}
	else
	{
		//from our inventory or have already done the animation.
		FinishMoveItemToContainer(sourceItem, targetContainer, targetSlot);
	}

	return true;
}

void CMonsterWeenie::FinishMoveItemToContainer(CWeenieObject *sourceItem, CContainerWeenie *targetContainer, DWORD targetSlot, bool bSendEvent, bool silent)
{
	// Scenarios to consider:
	// 1. Item being stored is equipped.
	// 2. Item being stored is on the GROUND!
	// 3. Item being stored is already in the inventory.
	// 4. Item being stored is in an external container (chest) or being moved to an external container (chest)

	bool wasWielded = sourceItem->IsWielded();
	bool isPickup = sourceItem->GetWorldTopLevelOwner() != this;

	DWORD dwCell = GetLandcell();

	//if (!sourceItem->HasOwner())
	//{
		if (CWeenieObject *generator = g_pWorld->FindObject(sourceItem->InqIIDQuality(GENERATOR_IID, 0)))
			generator->NotifyGeneratedPickedUp(sourceItem);
	//}

	// Take it out of whatever slot it's in.
	sourceItem->ReleaseFromAnyWeenieParent(false, true);
	sourceItem->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
	sourceItem->SetWeenieContainer(targetContainer->GetID());
	sourceItem->ReleaseFromBlock();

	// The container will auto-correct this slot into a valid range.
	targetSlot = targetContainer->Container_InsertInventoryItem(dwCell, sourceItem, targetSlot);

	if (!silent)
	{
		if (wasWielded)
			EmitSound(Sound_UnwieldObject, 1.0f);
		else
			EmitSound(Sound_PickUpItem, 1.0f);
	}

	if (isPickup)
		sourceItem->OnPickedUp(this);

	if (bSendEvent)
	{
		if (sourceItem->AsContainer())
			sourceItem->AsContainer()->MakeAwareViewContent(this);

		SendNetMessage(InventoryMove(sourceItem->GetID(), targetContainer->GetID(), targetSlot, sourceItem->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);
	}

	if (targetContainer->GetWorldTopLevelOwner() != this)
		RecalculateEncumbrance();

	if (sourceItem->GetItemType() & (TYPE_ARMOR | TYPE_CLOTHING))
		UpdateModel();

	if (wasWielded && get_minterp()->InqStyle() != Motion_NonCombat)
		AdjustToNewCombatMode();

	if (AsPlayer() && sourceItem->m_Qualities.id == W_COINSTACK_CLASS)
		RecalculateCoinAmount();

	sourceItem->m_Qualities.RemoveFloat(TIME_TO_ROT_FLOAT);
	sourceItem->_timeToRot = -1.0;
	sourceItem->_beganRot = false;
}

bool CMonsterWeenie::MoveItemTo3D(DWORD sourceItemId, bool animationDone)
{
	// Scenarios to consider:
	// 1. Item being stored is equipped.
	// 3. Item being stored is in the inventory.

	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (sourceItem->IsAttunedOrContainsAttuned())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
		return false;
	}

	if (!animationDone)
	{
		CDropInventoryUseEvent *dropEvent = new CDropInventoryUseEvent();
		dropEvent->_target_id = sourceItemId;
		ExecuteUseEvent(dropEvent);
	}
	else
	{
		FinishMoveItemTo3D(sourceItem);
	}

	return true;
}

void CMonsterWeenie::FinishMoveItemTo3D(CWeenieObject *sourceItem)
{
	// Scenarios to consider:
	// 1. Item being dropped is equipped.
	// 3. Item being dropped is in the inventory.

	BOOL bWasWielded = sourceItem->IsWielded();

	// Take it out of whatever slot it's in.
	sourceItem->ReleaseFromAnyWeenieParent(false, true);
	sourceItem->SetWeenieContainer(0);
	sourceItem->SetWielderID(0);
	sourceItem->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);

	EmitSound(Sound_DropItem, 1.0f);
	SendNetMessage(InventoryDrop(sourceItem->GetID()), PRIVATE_MSG, TRUE);

	sourceItem->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
	sourceItem->unset_parent();
	sourceItem->enter_world(&m_Position);

	sourceItem->SetPlacementFrame(0x65, FALSE);

	//if (!pItem->HasAnims())
	//	pItem->SetPlacementFrame(pItem->CanPickup() ? 0x65 : 0, 1);

	if (sourceItem->AsClothing())
		UpdateModel();

	g_pWorld->InsertEntity(sourceItem);
	sourceItem->Movement_Teleport(GetPosition());

	sourceItem->_timeToRot = Timer::cur_time + 300.0;
	sourceItem->_beganRot = false;
	sourceItem->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, sourceItem->_timeToRot);

	RecalculateEncumbrance();

	if (AsPlayer() && sourceItem->m_Qualities.id == W_COINSTACK_CLASS)
		RecalculateCoinAmount();

	if (bWasWielded && get_minterp()->InqStyle() != Motion_NonCombat)
		AdjustToNewCombatMode();

	if(bWasWielded)
		sourceItem->OnUnwield(this);
	sourceItem->OnDropped(this);
}

bool CMonsterWeenie::MoveItemToWield(DWORD sourceItemId, DWORD targetLoc, bool animationDone)
{
	// Scenarios to consider:
	// 1. Item being equipped from the GROUND!
	// 2. Item being equipped from different equip slot.
	// 3. Item being equipped from the player's inventory.

	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (!Container_CanEquip(sourceItem, targetLoc))
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
		return false;
	}

	if (!animationDone && !FindContainedItem(sourceItem->GetID()))
	{
		//from ground or external container.
		CMoveToWieldInventoryUseEvent *pickupEvent = new CMoveToWieldInventoryUseEvent();
		pickupEvent->_target_id = sourceItemId;
		pickupEvent->_sourceItemId = sourceItemId;
		pickupEvent->_targetLoc = targetLoc;
		ExecuteUseEvent(pickupEvent);
	}
	else
	{
		//from our inventory or have already done the animation.
		return FinishMoveItemToWield(sourceItem, targetLoc);
	}

	return true;
}

bool CMonsterWeenie::FinishMoveItemToWield(CWeenieObject *sourceItem, DWORD targetLoc)
{
	// Scenarios to consider:
	// 1. Item being equipped from the GROUND!
	// 2. Item being equipped from different equip slot.
	// 3. Item being equipped from the player's inventory.

	bool isPickup = sourceItem->GetWorldTopLevelOwner() != this;

	int error = CheckWieldRequirements(sourceItem, this, WIELD_REQUIREMENTS_INT, WIELD_SKILLTYPE_INT, WIELD_DIFFICULTY_INT);
	if (error != WERROR_NONE)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), error);
		return false;
	}

	error = CheckWieldRequirements(sourceItem, this, WIELD_REQUIREMENTS_2_INT, WIELD_SKILLTYPE_2_INT, WIELD_DIFFICULTY_2_INT);
	if (error != WERROR_NONE)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), error);
		return false;
	}

	error = CheckWieldRequirements(sourceItem, this, WIELD_REQUIREMENTS_3_INT, WIELD_SKILLTYPE_3_INT, WIELD_DIFFICULTY_3_INT);
	if (error != WERROR_NONE)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), error);
		return false;
	}

	error = CheckWieldRequirements(sourceItem, this, WIELD_REQUIREMENTS_4_INT, WIELD_SKILLTYPE_4_INT, WIELD_DIFFICULTY_4_INT);
	if (error != WERROR_NONE)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), error);
		return false;
	}

	//if (!sourceItem->HasOwner())
	//{
		if (CWeenieObject *generator = g_pWorld->FindObject(sourceItem->InqIIDQuality(GENERATOR_IID, 0)))
			generator->NotifyGeneratedPickedUp(sourceItem);
	//}

	DWORD cell_id = m_Position.objcell_id;

	// Take it out of whatever slot it may be in
	sourceItem->ReleaseFromAnyWeenieParent(false, false);

	DWORD placement_id, child_location_id;
	bool bShouldPlace = GetEquipPlacementAndHoldLocation(sourceItem, targetLoc, &placement_id, &child_location_id);

	sourceItem->SetWielderID(GetID());
	sourceItem->SetWieldedLocation(targetLoc);
	sourceItem->ReleaseFromBlock();

	// The container will auto-correct this slot into a valid range.
	Container_EquipItem(cell_id, sourceItem, targetLoc, child_location_id, placement_id);

	if (_IsPlayer())
	{
		SendNetMessage(InventoryEquip(sourceItem->GetID(), targetLoc), PRIVATE_MSG, TRUE);
		EmitSound(Sound_WieldObject, 1.0f);
	}

	if (m_bWorldIsAware)
	{
		sourceItem->NotifyIIDStatUpdated(CONTAINER_IID, false);
		sourceItem->NotifyIIDStatUpdated(WIELDER_IID, false);
		sourceItem->NotifyIntStatUpdated(CURRENT_WIELDED_LOCATION_INT, false);
	}

	if (sourceItem->AsClothing() && m_bWorldIsAware)
		UpdateModel();

	sourceItem->m_Qualities.RemoveFloat(TIME_TO_ROT_FLOAT);
	sourceItem->_timeToRot = -1.0;
	sourceItem->_beganRot = false;

	if (get_minterp()->InqStyle() != Motion_NonCombat)
		AdjustToNewCombatMode();

	// apply enchantments
	if (sourceItem->m_Qualities._spell_book)
	{
		bool bShouldCast = true;

		std::string name;
		if (sourceItem->m_Qualities.InqString(CRAFTSMAN_NAME_STRING, name))
		{
			if (!name.empty() && name != InqStringQuality(NAME_STRING, ""))
			{
				bShouldCast = false;

				NotifyWeenieErrorWithString(WERROR_ACTIVATION_NOT_CRAFTSMAN, name.c_str());
			}
		}

		int difficulty;
		difficulty = 0;
		if (sourceItem->m_Qualities.InqInt(ITEM_DIFFICULTY_INT, difficulty, TRUE, FALSE))
		{
			DWORD skillLevel = 0;
			if (!m_Qualities.InqSkill(ARCANE_LORE_SKILL, skillLevel, FALSE) || (int)skillLevel < difficulty)
			{
				bShouldCast = false;

				NotifyWeenieError(WERROR_ACTIVATION_ARCANE_LORE_TOO_LOW);
			}
		}

		if (bShouldCast)
		{
			difficulty = 0;
			DWORD skillActivationTypeDID = 0;
			if (sourceItem->m_Qualities.InqInt(ITEM_SKILL_LEVEL_LIMIT_INT, difficulty, TRUE, FALSE) && sourceItem->m_Qualities.InqDataID(ITEM_SKILL_LIMIT_DID, skillActivationTypeDID))
			{
				STypeSkill skillActivationType = (STypeSkill)skillActivationTypeDID;

				DWORD skillLevel = 0;
				if (!m_Qualities.InqSkill(skillActivationType, skillLevel, FALSE) || (int)skillLevel < difficulty)
				{
					bShouldCast = false;

					NotifyWeenieErrorWithString(WERROR_ACTIVATION_SKILL_TOO_LOW, CachedSkillTable->GetSkillName(skillActivationType).c_str());
				}
			}
		}

		if (bShouldCast && sourceItem->InqIntQuality(ITEM_ALLEGIANCE_RANK_LIMIT_INT, 0) > InqIntQuality(ALLEGIANCE_RANK_INT, 0))
		{
			bShouldCast = false;
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_ACTIVATION_RANK_TOO_LOW);
		}

		if (bShouldCast)
		{
			int heritageRequirement = sourceItem->InqIntQuality(HERITAGE_GROUP_INT, -1);
			if (heritageRequirement != -1 && heritageRequirement != InqIntQuality(HERITAGE_GROUP_INT, 0))
			{
				bShouldCast = false;
				std::string heritageString = sourceItem->InqStringQuality(ITEM_HERITAGE_GROUP_RESTRICTION_STRING, "of the correct heritage");
				NotifyWeenieErrorWithString(WERROR_ACTIVATION_WRONG_RACE, heritageString.c_str());
			}
		}

		int currentMana = 0;
		if (bShouldCast && sourceItem->m_Qualities.InqInt(ITEM_CUR_MANA_INT, currentMana, TRUE, FALSE))
		{
			if (currentMana == 0)
			{
				bShouldCast = false;
				NotifyWeenieError(WERROR_ACTIVATION_NOT_ENOUGH_MANA);
			}
			else
				sourceItem->_nextManaUse = Timer::cur_time;
		}

		if (bShouldCast)
		{
			DWORD serial = 0;

			serial |= ((DWORD)GetEnchantmentSerialByteForMask(sourceItem->InqIntQuality(LOCATIONS_INT, 0, TRUE)) << (DWORD)0);
			serial |= ((DWORD)GetEnchantmentSerialByteForMask(sourceItem->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE)) << (DWORD)8);

			for (auto &spellPage : sourceItem->m_Qualities._spell_book->_spellbook)
			{
				sourceItem->MakeSpellcastingManager()->CastSpellEquipped(GetID(), spellPage.first, (WORD)serial);
			}
		}
	}

	if(isPickup)
		sourceItem->OnPickedUp(this);
	sourceItem->OnWield(this);
	return true;
}

bool CMonsterWeenie::MergeItem(DWORD sourceItemId, DWORD targetItemId, DWORD amountToTransfer, bool animationDone)
{
	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	CWeenieObject *targetItem = FindValidNearbyItem(targetItemId, USEDISTANCE_FAR);
	if (!targetItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (!animationDone && (!FindContainedItem(sourceItemId) || !FindContainedItem(targetItemId)))
	{
		//from ground or other container or to ground or other container.
		CStackMergeInventoryUseEvent *mergeEvent = new CStackMergeInventoryUseEvent();
		if(sourceItem->IsContained() || sourceItem->IsWielded())
			mergeEvent->_target_id = targetItemId;
		else
			mergeEvent->_target_id = sourceItemId;
		mergeEvent->_sourceItemId = sourceItemId;
		mergeEvent->_targetItemId = targetItemId;
		mergeEvent->_amountToTransfer = amountToTransfer;
		ExecuteUseEvent(mergeEvent);
	}
	else
	{
		//check if the items is still in range.
		if (FindValidNearbyItem(sourceItemId, 5.0) == NULL || FindValidNearbyItem(targetItemId, 5.0) == NULL)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_TOO_FAR);
			return false;
		}

		if (sourceItem->m_Qualities.id != targetItem->m_Qualities.id)
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_NONE);
			return false;
		}

		DWORD sourceCurrentAmount = sourceItem->InqIntQuality(STACK_SIZE_INT, 1);
		if (amountToTransfer > sourceCurrentAmount)
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_NONE);
			return false;
		}

		DWORD targetCurrentAmount = targetItem->InqIntQuality(STACK_SIZE_INT, 1);
		DWORD targetMaxStackSize = targetItem->InqIntQuality(MAX_STACK_SIZE_INT, 1);

		int maxTransferAmount = targetMaxStackSize - targetCurrentAmount;
		if (maxTransferAmount <= 0)
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_NONE);
			return false;
		}

		if (maxTransferAmount < amountToTransfer)
			amountToTransfer = maxTransferAmount;
		targetItem->SetStackSize(targetCurrentAmount + amountToTransfer);

		bool wasSourceItemWielded = sourceItem->IsWielded();
		bool wasTargetItemWielded = targetItem->IsWielded();

		DWORD sourceItemNewStackSize = sourceCurrentAmount - amountToTransfer;
		if (sourceItemNewStackSize > 0) //partial stack movement
		{
			//the client seems to forget about the source item when merging using the "place selected object in inventory" keybind
			//to split from a stack so we have to remind it. I don't know how we can determine when the client uses the keybind or dragging
			//so just do it regarless.
			MakeAware(sourceItem, true);
			sourceItem->SetStackSize(sourceItemNewStackSize);
		}
		else //full stack movement
			sourceItem->Remove();

		if (wasSourceItemWielded)
			EmitSound(Sound_UnwieldObject, 1.0f);
		else if (wasTargetItemWielded)
			EmitSound(Sound_WieldObject, 1.0f);
		else if (!FindContainedItem(targetItemId)) //Todo: does placing items in external containers make the drop or pickup sound?
			EmitSound(Sound_DropItem, 1.0f);
		else
			EmitSound(Sound_PickUpItem, 1.0f);

		RecalculateEncumbrance();

		if (AsPlayer() && sourceItem->m_Qualities.id == W_COINSTACK_CLASS)
			RecalculateCoinAmount();
	}

	return true;
}

bool CMonsterWeenie::SplitItemToContainer(DWORD sourceItemId, DWORD targetContainerId, DWORD targetSlot, DWORD amountToTransfer, bool animationDone)
{
	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	CContainerWeenie *targetContainer = FindValidNearbyContainer(targetContainerId, USEDISTANCE_FAR);
	if (!targetContainer || !targetContainer->Container_CanStore(sourceItem))
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	bool isExternalTargetContainer = (targetContainer->GetTopLevelID() != GetID());

	if (isExternalTargetContainer && sourceItem->IsAttunedOrContainsAttuned())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
		return false;
	}

	if (!animationDone && (!FindContainedItem(sourceItemId) || isExternalTargetContainer))
	{
		//from ground or external container or to external container
		CStackSplitToContainerInventoryUseEvent *mergeEvent = new CStackSplitToContainerInventoryUseEvent();
		if (sourceItem->IsContained() || sourceItem->IsWielded())
			mergeEvent->_target_id = targetContainerId;
		else
			mergeEvent->_target_id = sourceItemId;
		mergeEvent->_sourceItemId = sourceItemId;
		mergeEvent->_targetContainerId = targetContainerId;
		mergeEvent->_targetSlot = targetSlot;
		mergeEvent->_amountToTransfer = amountToTransfer;
		ExecuteUseEvent(mergeEvent);
	}
	else
	{
		//check if items are still in range.
		if (FindValidNearbyItem(sourceItemId, 5.0) == NULL || FindValidNearbyContainer(targetContainerId, 5.0) == NULL)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_TOO_FAR);
			return false;
		}

		int totalAmount = sourceItem->InqIntQuality(STACK_SIZE_INT, 1, TRUE);

		if (amountToTransfer >= totalAmount)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}

		CWeenieObject *newStackItem = g_pWeenieFactory->CloneWeenie(sourceItem);
		if (!newStackItem)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}
		newStackItem->SetStackSize(amountToTransfer);
		newStackItem->SetID(g_pWorld->GenerateGUID(eDynamicGUID));

		if (!g_pWorld->CreateEntity(newStackItem))
		{
			delete newStackItem;
			return false;
		}

		FinishMoveItemToContainer(newStackItem, targetContainer, targetSlot);
		MakeAware(newStackItem);

		//and set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);
	}

	return true;
}

bool CMonsterWeenie::SplitItemto3D(DWORD sourceItemId, DWORD amountToTransfer, bool animationDone)
{
	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (sourceItem->IsAttunedOrContainsAttuned())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
		return false;
	}

	if (!animationDone && !FindContainedItem(sourceItem->GetID()))
	{
		//from ground or other container...
		CStackSplitTo3DInventoryUseEvent *splitEvent = new CStackSplitTo3DInventoryUseEvent();
		splitEvent->_target_id = sourceItemId;
		splitEvent->_sourceItemId = sourceItemId;
		splitEvent->_amountToTransfer = amountToTransfer;
		ExecuteUseEvent(splitEvent);
	}
	else
	{
		//We do this by first splitting the item to our main pack and then dropping it. We could change this to do it directly if this causes any issues.
		int totalAmount = sourceItem->InqIntQuality(STACK_SIZE_INT, 1, TRUE);

		if (amountToTransfer >= totalAmount)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}

		CWeenieObject *newStackItem = g_pWeenieFactory->CloneWeenie(sourceItem);
		if (!newStackItem)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}
		newStackItem->SetStackSize(amountToTransfer);
		newStackItem->SetID(g_pWorld->GenerateGUID(eDynamicGUID));

		if (!g_pWorld->CreateEntity(newStackItem))
		{
			delete newStackItem;
			return false;
		}

		// move the new stack to our main pack in preparation for dropping it.
		FinishMoveItemToContainer(newStackItem, this, 0);
		MakeAware(newStackItem);

		// set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);

		// and now move on to dropping it
		CDropInventoryUseEvent *dropEvent = new CDropInventoryUseEvent();
		dropEvent->_target_id = newStackItem->GetID();
		ExecuteUseEvent(dropEvent);
	}

	return true;
}

bool CMonsterWeenie::SplitItemToWield(DWORD sourceItemId, DWORD targetLoc, DWORD amountToTransfer, bool animationDone)
{
	if (!animationDone && IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	CWeenieObject *sourceItem = FindValidNearbyItem(sourceItemId, USEDISTANCE_FAR);
	if (!sourceItem)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (!Container_CanEquip(sourceItem, targetLoc))
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
		return false;
	}

	if (!animationDone && !FindContainedItem(sourceItem->GetID()))
	{
		//from ground or other container...
		CStackSplitToWieldInventoryUseEvent *stackSplitEvent = new CStackSplitToWieldInventoryUseEvent();
		stackSplitEvent->_target_id = sourceItemId;
		stackSplitEvent->_sourceItemId = sourceItemId;
		stackSplitEvent->_targetLoc = targetLoc;
		stackSplitEvent->_amountToTransfer = amountToTransfer;
		ExecuteUseEvent(stackSplitEvent);
	}
	else
	{
		int totalAmount = sourceItem->InqIntQuality(STACK_SIZE_INT, 1, TRUE);

		if (amountToTransfer >= totalAmount)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}

		CWeenieObject *newStackItem = g_pWeenieFactory->CloneWeenie(sourceItem);
		if (!newStackItem)
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
			return false;
		}
		newStackItem->SetStackSize(amountToTransfer);

		newStackItem->SetID(g_pWorld->GenerateGUID(eDynamicGUID));
		if (!g_pWorld->CreateEntity(newStackItem))
		{
			NotifyInventoryFailedEvent(newStackItem->GetID(), WERROR_OBJECT_GONE);
			return false;
		}

		if (!FinishMoveItemToWield(newStackItem, targetLoc))
		{
			g_pWorld->RemoveEntity(newStackItem);
			return false; //no need for notifications here as FinishMoveItemToWield takes care of that.
		}
		MakeAware(newStackItem);

		//and set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);
	}

	return true;
}

void CMonsterWeenie::GiveItem(DWORD targetContainerId, DWORD sourceItemId, DWORD transferAmount)
{
	if (IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return;
	}

	CWeenieObject *target = g_pWorld->FindObject(targetContainerId);
	CWeenieObject *sourceItem = FindContainedItem(sourceItemId);

	if (!sourceItem || !target)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return;
	}

	if (target->IsContained())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return;
	}

	if (sourceItem == target || !target->AsContainer())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	if (sourceItem->IsAttunedOrContainsAttuned() && target->AsPlayer())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
		return;
	}

	if (DistanceTo(target, true) > 1.0)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_TOO_FAR);
		return;
	}

	FinishGiveItem(target->AsContainer(), sourceItem, transferAmount);
}

void CMonsterWeenie::FinishGiveItem(CContainerWeenie *targetContainer, CWeenieObject *sourceItem, DWORD amountToTransfer)
{
	if (amountToTransfer <= 0 || amountToTransfer >= 100000)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	// for now we won't support giving items that are currently equipped
	if (sourceItem->IsEquipped() || sourceItem->parent)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	if (!targetContainer->Container_CanStore(sourceItem))
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	CWeenieObject *targetTopLevel = targetContainer->GetWorldTopLevelOwner();

	if (targetTopLevel != sourceItem->GetWorldTopLevelOwner() && targetTopLevel && targetTopLevel->IsExecutingEmote())
	{
		SendText(csprintf("%s is busy.", targetTopLevel->GetName().c_str()), LTT_DEFAULT);
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	if (CPlayerWeenie *player = targetContainer->AsPlayer())
	{
		if (!(player->GetCharacterOptions() & AllowGive_CharacterOption))
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
			return;
		}
	}

	int currentStackSize = sourceItem->InqIntQuality(STACK_SIZE_INT, 1, TRUE);
	if (amountToTransfer > currentStackSize)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	CWeenieObject *newStackItem;
	if (amountToTransfer < currentStackSize)
	{
		// partial stack, make a new object
		newStackItem = g_pWeenieFactory->CloneWeenie(sourceItem);
		newStackItem->SetID(g_pWorld->GenerateGUID(eDynamicGUID));

		if (!g_pWorld->CreateEntity(newStackItem, false))
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
			return;
		}

		sourceItem->SetStackSize(currentStackSize - amountToTransfer);
		newStackItem->SetStackSize(amountToTransfer);
	}
	else
	{
		newStackItem = sourceItem;
		sourceItem->ReleaseFromAnyWeenieParent();
	}

	newStackItem->m_Qualities.SetInstanceID(CONTAINER_IID, targetContainer->GetID());
	newStackItem->_cachedHasOwner = true;

	CWeenieObject *topLevelOwnerObj = newStackItem->GetWorldTopLevelOwner();
	assert(topLevelOwnerObj);
	assert(topLevelOwnerObj->AsContainer());

	if (topLevelOwnerObj)
	{
		if (CContainerWeenie *topLevelOwner = topLevelOwnerObj->AsContainer())
		{
			if (newStackItem == sourceItem)
			{
				SendNetMessage(InventoryMove(sourceItem->GetID(), targetContainer->GetID(), 0, sourceItem->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);
			}

			if (!InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE) && !topLevelOwner->InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE))
			{
				targetContainer->EmitSound(Sound_ReceiveItem, 1.0f);

				if (amountToTransfer > 1)
				{
					SendText(csprintf("You give %s %s %s.", topLevelOwner->GetName().c_str(), FormatNumberString(amountToTransfer).c_str(), newStackItem->GetPluralName().c_str()), LTT_DEFAULT);
					topLevelOwner->SendText(csprintf("%s gives you %s %s.", GetName().c_str(), FormatNumberString(amountToTransfer).c_str(), newStackItem->GetPluralName().c_str()), LTT_DEFAULT);
				}
				else
				{
					SendText(csprintf("You give %s %s.", topLevelOwner->GetName().c_str(), newStackItem->GetName().c_str()), LTT_DEFAULT);
					topLevelOwner->SendText(csprintf("%s gives you %s.", GetName().c_str(), newStackItem->GetName().c_str()), LTT_DEFAULT);
				}
			}

			topLevelOwner->MakeAware(newStackItem, true);

			if (newStackItem->AsContainer())
			{
				newStackItem->AsContainer()->MakeAwareViewContent(topLevelOwner);
			}

			if (AsPlayer() && newStackItem->m_Qualities.id == W_COINSTACK_CLASS)
				RecalculateCoinAmount();

			topLevelOwner->OnReceiveInventoryItem(this, newStackItem, 0);
			topLevelOwner->DebugValidate();
		}
	}

	DebugValidate();

	RecalculateEncumbrance();
}

void CMonsterWeenie::Tick()
{
	CContainerWeenie::Tick();

	Movement_Think();

	if (m_MonsterAI)
		m_MonsterAI->Update();
}

void CMonsterWeenie::OnDealtDamage(DamageEventData &damageData)
{
	CWeenieObject::OnDealtDamage(damageData);

	if (m_MonsterAI)
		m_MonsterAI->OnDealtDamage(damageData);
}

void CMonsterWeenie::OnTookDamage(DamageEventData &damageData)
{
	CWeenieObject::OnTookDamage(damageData);

	//if (IsDead())
	//	return;

	if (m_MonsterAI)
		m_MonsterAI->OnTookDamage(damageData);
}

void CMonsterWeenie::OnIdentifyAttempted(CWeenieObject *other)
{
	if (m_MonsterAI)
		m_MonsterAI->OnIdentifyAttempted(other);
}

void CMonsterWeenie::OnResistSpell(CWeenieObject *attacker)
{
	HandleAggro(attacker);

	if (m_MonsterAI)
		m_MonsterAI->OnResistSpell(attacker);
}

void CMonsterWeenie::OnEvadeAttack(CWeenieObject *attacker)
{
	HandleAggro(attacker);

	if (m_MonsterAI)
		m_MonsterAI->OnEvadeAttack(attacker);
}

void CMonsterWeenie::HandleAggro(CWeenieObject *attacker)
{
	if (m_MonsterAI)
		m_MonsterAI->HandleAggro(attacker);
}

/*
CBaelZharon::CBaelZharon()
{
	SetName("Bael'Zharon");
	SetSetupID(0x0200099E);
	SetScale(1.8f);

	m_ObjDesc.paletteID = 0x04001071;
	m_ObjDesc.AddSubpalette(new Subpalette(0x04001072, 0, 0));

	m_fTickFrequency = -1.0f;
}
*/

#if 0
CTargetDrudge::CTargetDrudge()
{
	SetSetupID(0x02000034);
	SetScale(0.95f);
	SetName("Oak Target Drudge");
	SetMotionTableID(0x0900008A);
	SetPETableID(0x3400006B);
	SetSoundTableID(0x20000051);

	m_miBaseModel.SetBasePalette(0x01B9);
	m_miBaseModel.ReplacePalette(0x08B4, 0x00, 0x00);
	m_miBaseModel.ReplaceTexture(0x00, 0x0036, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x01, 0x0031, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x02, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x03, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x04, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x05, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x06, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x07, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x08, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x09, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0A, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0B, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0C, 0x0030, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0D, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0E, 0x0D33, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x0E, 0x0EE8, 0x0EE8);
	m_miBaseModel.ReplaceTexture(0x0F, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceTexture(0x10, 0x0035, 0x0D33);
	m_miBaseModel.ReplaceModel(0x00, 0x005D);
	m_miBaseModel.ReplaceModel(0x01, 0x005E);
	m_miBaseModel.ReplaceModel(0x02, 0x006E);
	m_miBaseModel.ReplaceModel(0x03, 0x0064);
	m_miBaseModel.ReplaceModel(0x04, 0x18D9);
	m_miBaseModel.ReplaceModel(0x05, 0x006F);
	m_miBaseModel.ReplaceModel(0x06, 0x0316);
	m_miBaseModel.ReplaceModel(0x07, 0x18D9);
	m_miBaseModel.ReplaceModel(0x08, 0x006D);
	m_miBaseModel.ReplaceModel(0x09, 0x006B);
	m_miBaseModel.ReplaceModel(0x0A, 0x005F);
	m_miBaseModel.ReplaceModel(0x0B, 0x006C);
	m_miBaseModel.ReplaceModel(0x0C, 0x0068);
	m_miBaseModel.ReplaceModel(0x0D, 0x0060);
	m_miBaseModel.ReplaceModel(0x0E, 0x18D7);
	m_miBaseModel.ReplaceModel(0x0F, 0x0067);
	m_miBaseModel.ReplaceModel(0x10, 0x0060);
}
#endif

bool CMonsterWeenie::IsAttackMotion(DWORD motion)
{
	switch ((WORD)motion)
	{
	case 0x62:
	case 0x63:
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	case 0x68:
	case 0x69:
	case 0x6A:
		return true;
	}

	return false;
}

void CMonsterWeenie::OnMotionDone(DWORD motion, BOOL success)
{
	CWeenieObject::OnMotionDone(motion, success);

	if (motion == 0x40000011)
	{
		OnDeathAnimComplete();
	}

	if (m_MotionUseData.m_MotionUseType != MUT_UNDEF)
	{
		bool bUseSuccess = success && !IsDead();

		if (motion == m_MotionUseData.m_MotionUseMotionID)
		{
			switch (m_MotionUseData.m_MotionUseType)
			{
			case MUT_CONSUME_FOOD:
				{
					bool bConsumed = false;
					MotionUseData useData = m_MotionUseData;

					if (bUseSuccess)
					{
						m_MotionUseData.Reset(); // necessary so this doesn't become infinitely recursive

						DoForcedStopCompletely();

						CWeenieObject *pItem = FindContainedItem(useData.m_MotionUseTarget);

						if (pItem)
						{
							ReleaseContainedItemRecursive(pItem);

							bConsumed = true;

							if (DWORD use_sound_did = pItem->InqDIDQuality(USE_SOUND_DID, 0))
								EmitSound(use_sound_did, 1.0f);

							DWORD boost_stat = pItem->InqIntQuality(BOOSTER_ENUM_INT, 0);
							DWORD boost_value = pItem->InqIntQuality(BOOST_VALUE_INT, 0);

							switch (boost_stat)
							{
							case HEALTH_ATTRIBUTE_2ND:
							case STAMINA_ATTRIBUTE_2ND:
							case MANA_ATTRIBUTE_2ND:
								{
									STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
									STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

									DWORD statValue = 0, maxStatValue = 0;
									m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
									m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);

									DWORD newStatValue = min(statValue + boost_value, maxStatValue);

									int statChange = newStatValue - statValue;
									if (statChange)
									{
										m_Qualities.SetAttribute2nd(statType, newStatValue);
										NotifyAttribute2ndStatUpdated(statType);
									}

									const char *vitalName = "";
									switch (boost_stat)
									{
									case HEALTH_ATTRIBUTE_2ND: vitalName = "health"; break;
									case STAMINA_ATTRIBUTE_2ND: vitalName = "stamina"; break;
									case MANA_ATTRIBUTE_2ND: vitalName = "mana"; break;
									}

									SendText(csprintf("The %s restores %d points of your %s.", pItem->GetName().c_str(), max(0, statChange), vitalName), LTT_DEFAULT);
									break;
								}
							}

							NotifyContainedItemRemoved(pItem->id);

							pItem->MarkForDestroy();
						}
					}

					if (!bConsumed)
						NotifyInventoryFailedEvent(useData.m_MotionUseTarget, 0);

					break;
				}
			}

			m_MotionUseData.Reset();
		}
	}
}

CCorpseWeenie *CMonsterWeenie::CreateCorpse(bool visible)
{
	if (!InValidCell())
		return NULL;

	// spawn corpse
	CCorpseWeenie *pCorpse = (CCorpseWeenie *) g_pWeenieFactory->CreateWeenieByClassID(W_CORPSE_CLASS);

	pCorpse->CopyDIDStat(SETUP_DID, this);
	pCorpse->CopyDIDStat(MOTION_TABLE_DID, this);
	// pCorpse->CopyDIDStat(SOUND_TABLE_DID, this);
	// pCorpse->CopyDIDStat(PHYSICS_EFFECT_TABLE_DID, this);
	pCorpse->CopyFloatStat(DEFAULT_SCALE_FLOAT, this);
	pCorpse->CopyFloatStat(TRANSLUCENCY_FLOAT, this);

	pCorpse->CopyIntStat(LEVEL_INT, this); //copy the level so the treasure generator can have access to that value.

	ObjDesc desc;
	GetObjDesc(desc);
	pCorpse->SetObjDesc(desc);

	pCorpse->SetInitialPosition(m_Position);
	pCorpse->SetName(csprintf("Corpse of %s", GetName().c_str()));
	pCorpse->InitPhysicsObj();

	pCorpse->m_bDontClear = false;

	pCorpse->m_Qualities.SetString(LONG_DESC_STRING, csprintf("Killed by %s.", m_DeathKillerNameForCorpse.empty() ? "a mysterious source" : m_DeathKillerNameForCorpse.c_str()));
	pCorpse->m_Qualities.SetInstanceID(KILLER_IID, m_DeathKillerIDForCorpse);
	pCorpse->m_Qualities.SetInstanceID(VICTIM_IID, GetID());

	pCorpse->MakeMovementManager(TRUE);

	MovementParameters params;
	params.autonomous = 0;
	pCorpse->last_move_was_autonomous = false;
	pCorpse->DoMotion(GetCommandID(17), &params, 0);

	if (!visible)
		pCorpse->m_Qualities.SetBool(VISIBILITY_BOOL, false);

	if (!g_pWorld->CreateEntity(pCorpse))
		pCorpse = NULL;

	m_DeathKillerIDForCorpse = 0;
	m_DeathKillerNameForCorpse.clear();

	return pCorpse;
}

void CMonsterWeenie::DropAllLoot(CCorpseWeenie *pCorpse)
{
}

void CMonsterWeenie::GenerateDeathLoot(CCorpseWeenie *pCorpse)
{
	if (m_Qualities._create_list)
		g_pWeenieFactory->AddFromCreateList(pCorpse, m_Qualities._create_list, (DestinationType)(Contain_DestinationType | Treasure_DestinationType));

	if (DWORD deathTreasureType = InqDIDQuality(DEATH_TREASURE_TYPE_DID, 0))
		g_pWeenieFactory->GenerateFromTypeOrWcid(pCorpse, DestinationType::ContainTreasure_DestinationType, deathTreasureType);

	std::list<CWeenieObject *> removeList;

	for each(auto item in pCorpse->m_Items)
	{
		if (item->IsDestroyedOnDeath())
			removeList.push_back(item);
	}

	for (auto item : removeList)
		item->Remove();
}

void CMonsterWeenie::OnDeathAnimComplete()
{
	if (!m_bWaitingForDeathToFinish)
		return;

	m_bWaitingForDeathToFinish = false;

	if (!_IsPlayer())
	{
		MarkForDestroy();
	}

	// create corpse
	CPlayerWeenie *player = AsPlayer();
	if (!player)
	{
		CCorpseWeenie *pCorpse = CreateCorpse();

		if (pCorpse)
			GenerateDeathLoot(pCorpse);
	}
	else if(player->_pendingCorpse)
	{
		//make the player corpse visible.
		player->_pendingCorpse->m_Qualities.RemoveBool(VISIBILITY_BOOL);
		player->_pendingCorpse->NotifyBoolStatUpdated(VISIBILITY_BOOL, false);
		player->_pendingCorpse->NotifyObjectCreated(false);
		player->_pendingCorpse = NULL;
	}
}

void CMonsterWeenie::OnDeath(DWORD killer_id)
{
	CWeenieObject::OnDeath(killer_id);
	
	m_DeathKillerIDForCorpse = killer_id;
	if (!g_pWorld->FindObjectName(killer_id, m_DeathKillerNameForCorpse))
		m_DeathKillerNameForCorpse.clear();

	MakeMovementManager(TRUE);
	StopCompletely(0);

	bool bHardcoreDeath = false;

	if (g_pConfig->HardcoreMode() && _IsPlayer())
	{
		if (CWeenieObject *pKiller = g_pWorld->FindObject(killer_id))
		{
			if (!g_pConfig->HardcoreModePlayersOnly() || pKiller->_IsPlayer())
			{
				bHardcoreDeath = true;
			}
		}
	}

	if (!bHardcoreDeath)
	{
		MovementStruct mvs;
		MovementParameters params;

		mvs.type = RawCommand;
		mvs.motion = Motion_Dead;
		mvs.params = &params;
		params.action_stamp = ++m_wAnimSequence;
		last_move_was_autonomous = 0;

		m_bWaitingForDeathToFinish = true;
		if (movement_manager->PerformMovement(mvs))
		{
			// animation failed for some reason
			OnDeathAnimComplete();
		}
		else
		{
			Animation_Update();
		}
	}
	else
	{
		if (!m_DeathKillerNameForCorpse.empty())
		{
			g_pWorld->BroadcastGlobal(ServerText(csprintf("%s has been defeated by %s.", GetName().c_str(), m_DeathKillerNameForCorpse.c_str()), LTT_DEFAULT), PRIVATE_MSG);
		}
		else
		{
			g_pWorld->BroadcastGlobal(ServerText(csprintf("%s has been defeated.", GetName().c_str()), LTT_DEFAULT), PRIVATE_MSG);
		}

		if (CCorpseWeenie *pCorpse = CreateCorpse())
		{
			DropAllLoot(pCorpse);
		}

		g_pDBIO->DeleteCharacter(GetID());

		BeginLogout();
	}
}

bool CMonsterWeenie::IsDead()
{
	if (m_bReviveAfterAnim)
		return true;

	return CWeenieObject::IsDead();
}

void CMonsterWeenie::ChangeCombatMode(COMBAT_MODE mode, bool playerRequested)
{
	COMBAT_MODE newCombatMode = (COMBAT_MODE) InqIntQuality(COMBAT_MODE_INT, COMBAT_MODE::NONCOMBAT_COMBAT_MODE, TRUE);
	DWORD new_motion_style = get_minterp()->InqStyle();

	switch (mode)
	{
	case NONCOMBAT_COMBAT_MODE:
		new_motion_style = Motion_NonCombat;
		newCombatMode = COMBAT_MODE::NONCOMBAT_COMBAT_MODE;
		break;

	case MELEE_COMBAT_MODE:
		{
			CWeenieObject *weapon = GetWieldedMelee();

			CombatStyle default_combat_style = weapon ? (CombatStyle)weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, Undef_CombatStyle) : Undef_CombatStyle;

			switch (default_combat_style)
			{
			case Undef_CombatStyle:
			case Unarmed_CombatStyle:
				new_motion_style = Motion_HandCombat;
				newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;
				break;

			case OneHanded_CombatStyle:
				{
					new_motion_style = Motion_SwordCombat;

					if (CWeenieObject *shield = GetWieldedShield())
					{
						new_motion_style = Motion_SwordShieldCombat;
					}

					newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;
					break;
				}
			}

			break;
		}

	case MISSILE_COMBAT_MODE:
		{
			CWeenieObject *weapon = GetWieldedMissile();
			CombatStyle default_combat_style = weapon ? (CombatStyle)weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, Undef_CombatStyle) : Undef_CombatStyle;

			switch (default_combat_style)
			{
			case Bow_CombatStyle:
				if (CWeenieObject *ammo = GetWieldedAmmo())
				{
					new_motion_style = Motion_BowCombat;
					newCombatMode = COMBAT_MODE::MISSILE_COMBAT_MODE;
				}
				else
				{
					 NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
					 new_motion_style = Motion_NonCombat;
					 newCombatMode = COMBAT_MODE::NONCOMBAT_COMBAT_MODE;
				}
				break;

			case Crossbow_CombatStyle:
				if (CWeenieObject *ammo = GetWieldedAmmo())
				{
					new_motion_style = Motion_CrossbowCombat;
					newCombatMode = COMBAT_MODE::MISSILE_COMBAT_MODE;
				}
				else
				{
					NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
					new_motion_style = Motion_NonCombat;
					newCombatMode = COMBAT_MODE::NONCOMBAT_COMBAT_MODE;
				}
				break;

			case ThrownWeapon_CombatStyle:
				new_motion_style = Motion_ThrownWeaponCombat;
				newCombatMode = COMBAT_MODE::MISSILE_COMBAT_MODE;
				break;

			case Atlatl_CombatStyle:
				if (CWeenieObject *ammo = GetWieldedAmmo())
				{
					new_motion_style = Motion_AtlatlCombat;
					newCombatMode = COMBAT_MODE::MISSILE_COMBAT_MODE;
				}
				else
				{
					NotifyWeenieError(WERROR_COMBAT_OUT_OF_AMMO);
					new_motion_style = Motion_NonCombat;
					newCombatMode = COMBAT_MODE::NONCOMBAT_COMBAT_MODE;
				}
				break;
			}
			break;
		}

	case MAGIC_COMBAT_MODE:
		new_motion_style = Motion_Magic;
		newCombatMode = COMBAT_MODE::MAGIC_COMBAT_MODE;
		break;
	}

	if (new_motion_style != get_minterp()->InqStyle())
	{
		MovementParameters params;
		get_minterp()->DoMotion(new_motion_style, &params);

		_server_control_timestamp++;
		last_move_was_autonomous = false;

		Animation_Update();
	}

	m_Qualities.SetInt(COMBAT_MODE_INT, newCombatMode);

	if (newCombatMode != mode || !playerRequested)
	{
		NotifyIntStatUpdated(COMBAT_MODE_INT);
	}
}

DWORD CMonsterWeenie::DoForcedUseMotion(MotionUseType useType, DWORD motion, DWORD target, DWORD childID, DWORD childLoc, MovementParameters *params)
{
	m_MotionUseData.m_MotionUseType = useType;
	m_MotionUseData.m_MotionUseMotionID = motion;
	m_MotionUseData.m_MotionUseTarget = target;
	m_MotionUseData.m_MotionUseChildID = childID;
	m_MotionUseData.m_MotionUseChildLocation = childLoc;
	return DoForcedMotion(motion, params);
}

bool CMonsterWeenie::ClothingPrioritySorter(const CWeenieObject *first, const CWeenieObject *second)
{
	return ((CWeenieObject *)first)->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE) < ((CWeenieObject *)second)->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE);
}

void CMonsterWeenie::GetObjDesc(ObjDesc &objDesc)
{
	std::list<CWeenieObject *> wieldedWearable;
	Container_GetWieldedByMask(wieldedWearable, ARMOR_LOC|CLOTHING_LOC);
	for (auto wearable : wieldedWearable)
	{
		if (wearable->IsAvatarJumpsuit())
		{
			objDesc = wearable->m_WornObjDesc;
			return;
		}
	}

	CWeenieObject::GetObjDesc(objDesc);

	DWORD head_object_id;
	if (m_Qualities.InqDataID(HEAD_OBJECT_DID, head_object_id))
		objDesc.AddAnimPartChange(new AnimPartChange(16, head_object_id));

	DWORD old_eye_texture_id, new_eye_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_EYES_TEXTURE_DID, old_eye_texture_id) && m_Qualities.InqDataID(EYES_TEXTURE_DID, new_eye_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_eye_texture_id, new_eye_texture_id));

	DWORD old_nose_texture_id, new_nose_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_NOSE_TEXTURE_DID, old_nose_texture_id) && m_Qualities.InqDataID(NOSE_TEXTURE_DID, new_nose_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_nose_texture_id, new_nose_texture_id));

	DWORD old_mouth_texture_id, new_mouth_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_MOUTH_TEXTURE_DID, old_mouth_texture_id) && m_Qualities.InqDataID(MOUTH_TEXTURE_DID, new_mouth_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_mouth_texture_id, new_mouth_texture_id));

	DWORD skin_palette_id;
	if (m_Qualities.InqDataID(SKIN_PALETTE_DID, skin_palette_id))
		objDesc.AddSubpalette(new Subpalette(skin_palette_id, 0 << 3, 0x18 << 3));

	DWORD hair_palette_id;
	if (m_Qualities.InqDataID(HAIR_PALETTE_DID, hair_palette_id))
		objDesc.AddSubpalette(new Subpalette(hair_palette_id, 0x18 << 3, 0x8 << 3));

	DWORD eye_palette_id;
	if (m_Qualities.InqDataID(EYES_PALETTE_DID, eye_palette_id))
		objDesc.AddSubpalette(new Subpalette(eye_palette_id, 0x20 << 3, 0x8 << 3));

	/*
	std::list<CWeenieObject *> wieldedClothings;
	Container_GetWieldedByMask(wieldedClothings, CLOTHING_LOC);
	wieldedClothings.sort(ClothingPrioritySorter);

	for (auto clothing : wieldedClothings)
	{
		if (clothing->IsHelm())
		{
			if (!ShowHelm())
				continue;
		}
		
		DWORD clothing_table_id = clothing->InqDIDQuality(CLOTHINGBASE_DID, 0);
		DWORD palette_template_key = clothing->InqIntQuality(PALETTE_TEMPLATE_INT, 0);

		if (clothing_table_id && !clothing->IsAvatarJumpsuit())
		{
			ClothingTable *clothingTable = ClothingTable::Get(clothing_table_id);

			if (clothingTable)
			{
				ObjDesc od;
				ShadePackage shades(clothing->InqFloatQuality(SHADE_FLOAT, 0.0));

				double shadeVal;
				if (clothing->m_Qualities.InqFloat(SHADE2_FLOAT, shadeVal))
					shades._val[1] = shadeVal;
				if (clothing->m_Qualities.InqFloat(SHADE3_FLOAT, shadeVal))
					shades._val[2] = shadeVal;
				if (clothing->m_Qualities.InqFloat(SHADE4_FLOAT, shadeVal))
					shades._val[3] = shadeVal;

				clothingTable->BuildObjDesc(GetSetupID(), palette_template_key, &shades, &od);
				objDesc += od;

				ClothingTable::Release(clothingTable);
			}
		}
		else
		{
			objDesc += clothing->m_WornObjDesc;
		}
	}
	*/

	std::list<CWeenieObject *> wieldedArmors;
	Container_GetWieldedByMask(wieldedArmors, CLOTHING_LOC|ARMOR_LOC);
	wieldedArmors.sort(ClothingPrioritySorter);

	for (auto armor : wieldedArmors)
	{
		if (armor->IsHelm())
		{
			if (!ShowHelm())
				continue;
		}

		DWORD clothing_table_id = armor->InqDIDQuality(CLOTHINGBASE_DID, 0);
		DWORD palette_template_key = armor->InqIntQuality(PALETTE_TEMPLATE_INT, 0);

		if (clothing_table_id && !armor->IsAvatarJumpsuit())
		{
			ClothingTable *clothingTable = ClothingTable::Get(clothing_table_id);

			if (clothingTable)
			{
				ObjDesc od;
				ShadePackage shades(armor->InqFloatQuality(SHADE_FLOAT, 0.0));

				double shadeVal;
				if (armor->m_Qualities.InqFloat(SHADE2_FLOAT, shadeVal))
					shades._val[1] = shadeVal;
				if (armor->m_Qualities.InqFloat(SHADE3_FLOAT, shadeVal))
					shades._val[2] = shadeVal;
				if (armor->m_Qualities.InqFloat(SHADE4_FLOAT, shadeVal))
					shades._val[3] = shadeVal;

				clothingTable->BuildObjDesc(GetSetupID(), palette_template_key, &shades, &od);
				objDesc += od;

				ClothingTable::Release(clothingTable);
			}
		}
		else
		{
			objDesc += armor->m_WornObjDesc;
		}
	}
}

DWORD CMonsterWeenie::OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot)
{
	if (source != this)
	{
		bool accepted = false;
		// Use our emote table to determine what to do with this item.
		if (m_Qualities._emote_table)
		{
			PackableList<EmoteSet> *emoteCategory = m_Qualities._emote_table->_emote_table.lookup(Give_EmoteCategory);

			if (emoteCategory)
			{
				double dice = Random::GenFloat(0.0, 1.0);
				double lastProbability = -1.0;

				for (auto &emoteSet : *emoteCategory)
				{
					if (emoteSet.classID == item->m_Qualities.id)
					{
						if (dice >= emoteSet.probability)
							continue;

						if (lastProbability < 0.0 || lastProbability == emoteSet.probability)
						{
							if (item->InqIntQuality(STACK_SIZE_INT, 1) > 1)
							{
								item->DecrementStackNum(1, false);
								SimulateGiveObject(source->AsContainer(), item);
							}
							else
								g_pWorld->RemoveEntity(item);

							MakeEmoteManager()->ExecuteEmoteSet(emoteSet, source->GetID());

							lastProbability = emoteSet.probability;

							return 0;
						}
					}
				}
			}
		}

		if (source)
		{
			ChanceExecuteEmoteSet(source->GetID(), Refuse_EmoteCategory);
			SimulateGiveObject(source->AsContainer(), item);
		}
		else
			g_pWorld->RemoveEntity(item);

		return 0;
	}
	else
	{
		// Unless the source is ourselves, that means the item is spawning.
		item->ReleaseFromAnyWeenieParent(false, true);
		item->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);

		item->SetWeenieContainer(GetID());
		item->ReleaseFromBlock();

		return Container_InsertInventoryItem(0, item, desired_slot);
	}
}

double CMonsterWeenie::GetMeleeDefenseModUsingWielded()
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC | HELD_LOC);

	double defenseMod = 1.0;
	for (auto item : wielded)
	{
		defenseMod *= item->GetMeleeDefenseMod();
	}

	defenseMod *= CWeenieObject::GetMeleeDefenseMod();

	Container_GetWieldedByMask(wielded, ARMOR_LOC);
	for (auto item : m_Wielded) //check all armor for appropriate imbue effects
	{
		if (item->GetImbueEffects() & MeleeDefense_ImbuedEffectType)
			defenseMod += 0.01;
	}

	return defenseMod;
}

double CMonsterWeenie::GetMissileDefenseModUsingWielded()
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC | HELD_LOC);

	double defenseMod = 1.0;
	for (auto item : wielded)
	{
		defenseMod *= item->GetMissileDefenseMod();
	}

	defenseMod *= CWeenieObject::GetMissileDefenseMod();

	Container_GetWieldedByMask(wielded, ARMOR_LOC);
	for (auto item : m_Wielded) //check all armor for appropriate imbue effects
	{
		if (item->GetImbueEffects() & MissileDefense_ImbuedEffectType)
			defenseMod += 0.01;
	}

	return defenseMod;
}

double CMonsterWeenie::GetMagicDefenseModUsingWielded()
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC | HELD_LOC);

	double defenseMod = 1.0;
	for (auto item : wielded)
	{
		defenseMod *= item->GetMagicDefenseMod();
	}

	defenseMod *= CWeenieObject::GetMagicDefenseMod();

	Container_GetWieldedByMask(wielded, ARMOR_LOC);
	for (auto item : m_Wielded) //check all armor for appropriate imbue effects
	{
		if (item->GetImbueEffects() & MagicDefense_ImbuedEffectType)
			defenseMod += 0.01;
	}

	return defenseMod;
}

int CMonsterWeenie::GetAttackTimeUsingWielded()
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC);
	
	for (auto item : wielded)
	{
		return item->GetAttackTime();
	}

	return CWeenieObject::GetAttackTime();
}

int CMonsterWeenie::GetAttackTime()
{
	/*
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC);

	int totalTime = CWeenieObject::GetAttackTime();

	for (auto item : wielded)
	{
		totalTime += item->GetAttackTime();
	}

	return totalTime;
	*/
	return CWeenieObject::GetAttackTime();
}

int CMonsterWeenie::GetAttackDamage()
{
	/*
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC);

	int damage = CWeenieObject::GetAttackDamage();

	for (auto item : wielded)
	{
		damage += item->GetAttackDamage();
	}
	*/

	return CWeenieObject::GetAttackDamage();
}

float CMonsterWeenie::GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor)
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, ARMOR_LOC|CLOTHING_LOC|SHIELD_LOC);

	EnchantedQualityDetails buffDetails;

	//body part
	GetBodyArmorEnchantmentDetails(damageData.hitPart, damageData.damage_type, &buffDetails);

	//body
	buffDetails.rawValue += CWeenieObject::GetEffectiveArmorLevel(damageData, bIgnoreMagicArmor);

	//equipment
	for (auto item : wielded)
		buffDetails.rawValue += item->GetEffectiveArmorLevel(damageData, bIgnoreMagicArmor);

	if (damageData.isArmorRending && damageData.armorRendingMultiplier < buffDetails.valueDecreasingMultiplier)
		buffDetails.valueDecreasingMultiplier = damageData.armorRendingMultiplier;
	buffDetails.CalculateEnchantedValue();

	if (bIgnoreMagicArmor)
		return buffDetails.rawValue;
	else
		return buffDetails.enchantedValue;
}

void CMonsterWeenie::TryMeleeAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion)
{
	if (IsDead())
	{
		NotifyAttackDone(WERROR_DEAD);
		return;
	}

	if (!IsAttacking() && IsBusyOrInAction() || (_blockNewAttacksUntil != -1 && _blockNewAttacksUntil > Timer::cur_time))
	{
		NotifyAttackDone(WERROR_ACTIONS_LOCKED);
		return;
	}

	if (!m_AttackManager)
	{
		m_AttackManager = new AttackManager(this);
	}

	// duplicate attack
	m_AttackManager->BeginMeleeAttack(target_id, height, power, m_MonsterAI ? m_MonsterAI->GetChaseDistance() : 15.0f, motion);

	// ensure there's no heartbeat animation
	_last_update_pos = Timer::cur_time;
}

void CMonsterWeenie::TryMissileAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion)
{
	if (IsDead())
	{
		//NotifyWeenieError(WERROR_DEAD);
		NotifyAttackDone(WERROR_DEAD);
		return;
	}

	if (!IsAttacking() && IsBusyOrInAction() || (_blockNewAttacksUntil != -1 && _blockNewAttacksUntil > Timer::cur_time))
	{
		//NotifyWeenieError(WERROR_ACTIONS_LOCKED);
		NotifyAttackDone(WERROR_ACTIONS_LOCKED);
		return;
	}

	if (!m_AttackManager)
		m_AttackManager = new AttackManager(this);

	m_AttackManager->BeginMissileAttack(target_id, height, power, motion);

	// ensure there's no heartbeat animation
	_last_update_pos = Timer::cur_time;
}

BOOL CMonsterWeenie::DoCollision(const class ObjCollisionProfile &prof)
{
	if (prof.IsDoor() && !AsPlayer())
	{
		if (CWeenieObject *weenie = g_pWorld->FindObject(prof.id))
		{
			if (CBaseDoor *door = weenie->AsDoor())
			{
				if (!door->IsLocked() && door->IsClosed())
				{
					door->OpenDoor();
				}
			}
		}
	}

	return CContainerWeenie::DoCollision(prof);
}