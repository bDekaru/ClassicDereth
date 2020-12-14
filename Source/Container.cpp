
#include <StdAfx.h>
#include "Container.h"
#include "World.h"
#include "ObjectMsgs.h"
#include "ChatMsgs.h"
#include "Player.h"
#include "WeenieFactory.h"
#include "WorldLandBlock.h"
#include "Config.h"
#include "SpellcastingManager.h"

CContainerWeenie::CContainerWeenie()
{
	for (uint32_t i = 0; i < MAX_WIELDED_COMBAT; i++)
		m_WieldedCombat[i] = NULL;
}

CContainerWeenie::~CContainerWeenie()
{
	for (uint32_t i = 0; i < MAX_WIELDED_COMBAT; i++)
		m_WieldedCombat[i] = NULL;

	for (auto item : m_Wielded)
	{
		g_pWorld->RemoveEntity(item);
	}

	m_Wielded.clear();

	for (auto item : m_Items)
	{
		g_pWorld->RemoveEntity(item);
	}

	m_Items.clear();

	for (auto container : m_Packs)
	{
		g_pWorld->RemoveEntity(container);
	}

	m_Packs.clear();
}

void CContainerWeenie::ApplyQualityOverrides()
{
	CWeenieObject::ApplyQualityOverrides();

	if (m_Qualities.GetInt(ITEM_TYPE_INT, 0) & TYPE_CONTAINER)
	{
		if (GetItemsCapacity() < 0)
		{
			m_Qualities.SetInt(ITEMS_CAPACITY_INT, 120);
		}

		if (GetContainersCapacity() < 0)
		{
			m_Qualities.SetInt(CONTAINERS_CAPACITY_INT, 10);
		}
	}
}

void CContainerWeenie::PostSpawn()
{
	CWeenieObject::PostSpawn();

	m_bInitiallyLocked = IsLocked();
}

int CContainerWeenie::GetItemsCapacity()
{
	return InqIntQuality(ITEMS_CAPACITY_INT, 0);
}

int CContainerWeenie::GetContainersCapacity()
{
	return InqIntQuality(CONTAINERS_CAPACITY_INT, 0);
}

CContainerWeenie *CContainerWeenie::FindContainer(uint32_t container_id)
{
	if (GetID() == container_id)
		return this;

	for (auto pack : m_Packs)
	{
		if (pack->GetID() == container_id)
		{
			if (CContainerWeenie *packContainer = pack->AsContainer())
			{
				return packContainer;
			}
		}
	}

	if (CWeenieObject *externalObject = g_pWorld->FindObject(container_id))
	{
		if (CContainerWeenie *externalContainer = externalObject->AsContainer())
		{
			if (externalContainer->_openedById == GetTopLevelID())
			{
				return externalContainer;
			}
		}
	}

	return NULL;
}

CWeenieObject *CContainerWeenie::GetWieldedCombat(COMBAT_USE combatUse)
{
	// The first entry is "Undef" so we omit that.
	int index = combatUse - 1;

	if (index < 0 || index > MAX_WIELDED_COMBAT)
		return NULL;

	// OFFHAND is the shield slot
	if (combatUse == COMBAT_USE_OFFHAND)
		index = COMBAT_USE_SHIELD - 1;

	return m_WieldedCombat[index];
}

void CContainerWeenie::SetWieldedCombat(CWeenieObject *wielded, COMBAT_USE combatUse)
{
	// The first entry is "Undef" so we omit that.
	int index = combatUse - 1;

	if (index < 0 || index > MAX_WIELDED_COMBAT)
		return;

	// OFFHAND is the shield slot
	if (combatUse == COMBAT_USE_OFFHAND)
		index = COMBAT_USE_SHIELD - 1;

	m_WieldedCombat[index] = wielded;
}

bool CContainerWeenie::HasWielded()
{
	CWeenieObject* wielded = GetWieldedCaster();
	if (wielded)
		return true;
	wielded = GetWieldedMelee();
	if (wielded)
		return true;
	wielded = GetWieldedMissile();
	if (wielded)
		return true;
	wielded = GetWieldedTwoHanded();
	if (wielded)
		return true;
	wielded = GetWieldedShield();
	if (wielded)
		return true;
	return false;
}

CWeenieObject *CContainerWeenie::GetWieldedMelee()
{
	return GetWieldedCombat(COMBAT_USE_MELEE);
}

CWeenieObject *CContainerWeenie::GetWieldedMissile()
{
	return GetWieldedCombat(COMBAT_USE_MISSILE);
}

CWeenieObject *CContainerWeenie::GetWieldedAmmo()
{
	return GetWieldedCombat(COMBAT_USE_AMMO);
}

CWeenieObject *CContainerWeenie::GetWieldedShield()
{
	return GetWieldedCombat(COMBAT_USE_SHIELD);
}

CWeenieObject *CContainerWeenie::GetWieldedTwoHanded()
{
	return GetWieldedCombat(COMBAT_USE_TWO_HANDED);
}

CWeenieObject *CContainerWeenie::GetWieldedCaster()
{
	for (auto item : m_Wielded)
	{
		if (item->AsCaster())
			return item;
	}

	return NULL;
}

void CContainerWeenie::Container_GetWieldedByMask(std::list<CWeenieObject *> &wielded, uint32_t inv_loc_mask)
{
	for (auto item : m_Wielded)
	{
		if (item->InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0, TRUE) & inv_loc_mask)
			wielded.push_back(item);
	}
}

CWeenieObject *CContainerWeenie::GetWielded(INVENTORY_LOC slot)
{
	for (auto item : m_Wielded)
	{
		if (item->InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0, TRUE) == slot)
			return item;
	}

	return NULL;
}

void CContainerWeenie::ReleaseContainedItemRecursive(CWeenieObject *item)
{
	if (!item)
		return;

	for (uint32_t i = 0; i < MAX_WIELDED_COMBAT; i++)
	{
		if (item == m_WieldedCombat[i])
			m_WieldedCombat[i] = NULL;
	}

	for (std::vector<CWeenieObject *>::iterator equipmentIterator = m_Wielded.begin(); equipmentIterator != m_Wielded.end();)
	{
		if (*equipmentIterator != item)
		{
			equipmentIterator++;
			continue;
		}

		equipmentIterator = m_Wielded.erase(equipmentIterator);
	}

	for (std::vector<CWeenieObject *>::iterator itemIterator = m_Items.begin(); itemIterator != m_Items.end();)
	{
		if (*itemIterator != item)
		{
			itemIterator++;
			continue;
		}
		
		itemIterator = m_Items.erase(itemIterator);
	}

	for (std::vector<CWeenieObject *>::iterator packIterator = m_Packs.begin(); packIterator != m_Packs.end();)
	{
		CWeenieObject *pack = *packIterator;

		if (pack != item)
		{
			pack->ReleaseContainedItemRecursive(item);
			packIterator++;
			continue;
		}

		packIterator = m_Packs.erase(packIterator);
	}

	if (item->GetContainerID() == GetID())
	{
		item->m_Qualities.SetInstanceID(CONTAINER_IID, 0);
	}

	if (item->GetWielderID() == GetID())
	{
		item->m_Qualities.SetInstanceID(WIELDER_IID, 0);
	}

	item->RecacheHasOwner();
}

BOOL CContainerWeenie::Container_CanEquip(CWeenieObject *item, uint32_t location)
{
	if (!item)
		return FALSE;

	int possible = item->InqIntQuality(LOCATIONS_INT, 0, TRUE);

	// weapons (right-hand) can go in the shield (left-hand) slot, too
	//if ((possible & location) == 0 && !(location == SHIELD_LOC && possible == MELEE_WEAPON_LOC))
	//	return FALSE;

	// Don't check Valid Wield Location for Dual Wield
	//if (!(location == SHIELD_LOC && possible == MELEE_WEAPON_LOC))
	//{
	//	if (!item->IsValidWieldLocation(location))
	//		return FALSE;
	//}

	for (auto wielded : m_Wielded)
	{
		if (wielded == item)
			return TRUE;

		if (!wielded->CanEquipWith(item, location))
			return FALSE;
	}

	return TRUE;
}

void CContainerWeenie::Container_EquipItem(uint32_t dwCell, CWeenieObject *item, uint32_t inv_loc, uint32_t child_location, uint32_t placement)
{
	int combatUse = item->InqIntQuality(COMBAT_USE_INT, 0, TRUE);
	if (combatUse)
	{
//		if (combatUse == COMBAT_USE_MELEE && placement == LeftWeapon)
//			combatUse = COMBAT_USE_OFFHAND;
		SetWieldedCombat(item, (COMBAT_USE)combatUse);
	}

	bool bAlreadyEquipped = false;
	for (auto entry : m_Wielded)
	{
		if (entry == item)
		{
			bAlreadyEquipped = true;
			break;
		}
	}
	if (!bAlreadyEquipped)
	{
		m_Wielded.push_back(item);
	}

	if (child_location && placement)
	{
		item->m_Qualities.SetInt(PARENT_LOCATION_INT, child_location);
		item->set_parent(this, child_location);
		item->SetPlacementFrame(placement, FALSE);

		if (m_bWorldIsAware)
		{
			if (CWeenieObject *owner = GetWorldTopLevelOwner())
			{
				if (owner->GetBlock())
				{
					owner->GetBlock()->ExchangePVS(item, 0);
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
			Blah.Write<uint32_t>(item->GetID());
			Blah.Write<uint32_t>(child_location);
			Blah.Write<uint32_t>(placement);
			Blah.Write<WORD>(GetPhysicsObj()->_instance_timestamp);
			Blah.Write<WORD>(++item->_position_timestamp);
			g_pWorld->BroadcastPVS(dwCell, Blah.GetData(), Blah.GetSize(), OBJECT_MSG, false, false, true);
		}
	}
	else
	{
		if (m_bWorldIsAware)
		{
			item->_position_timestamp++;

			BinaryWriter Blah;
			Blah.Write<uint32_t>(0xF74A);
			Blah.Write<uint32_t>(item->GetID());
			Blah.Write<WORD>(item->_instance_timestamp);
			Blah.Write<WORD>(item->_position_timestamp);
			g_pWorld->BroadcastPVS(dwCell, Blah.GetData(), Blah.GetSize());
		}
	}
}

CWeenieObject *CContainerWeenie::FindContainedItem(uint32_t object_id)
{
	for (auto item : m_Wielded)
	{
		if (item->GetID() == object_id)
			return item;
	}

	for (auto item : m_Items)
	{
		if (item->GetID() == object_id)
			return item;
	}

	for (auto item : m_Packs)
	{
		if (item->GetID() == object_id)
			return item;
		
		if (auto subitem = item->FindContainedItem(object_id))
			return subitem;
	}

	return NULL;
}

uint32_t CContainerWeenie::Container_GetNumFreeMainPackSlots()
{
	return (uint32_t) max(0, GetItemsCapacity() - (signed)m_Items.size());
}

BOOL CContainerWeenie::Container_CanStore(CWeenieObject *pItem)
{
	return Container_CanStore(pItem, pItem->RequiresPackSlot());
}

BOOL CContainerWeenie::IsItemsCapacityFull()
{
	int capacity = GetItemsCapacity();

	if (capacity >= 0)
	{
		if (m_Items.size() < capacity)
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

BOOL CContainerWeenie::IsContainersCapacityFull()
{
	int capacity = GetContainersCapacity();

	if (capacity >= 0)
	{
		if (m_Packs.size() < capacity)
			return FALSE;

		return TRUE;
	}

	return TRUE;
}

BOOL CContainerWeenie::Container_CanStore(CWeenieObject *pItem, bool bPackSlot)
{
	// TODO handle: pItem->InqBoolQuality(REQUIRES_BACKPACK_SLOT_BOOL, FALSE)

	if (bPackSlot)
	{
		if (!pItem->RequiresPackSlot())
			return FALSE;
		
		int capacity = GetContainersCapacity();

		if (capacity >= 0)
		{
			if (m_Packs.size() < capacity)
				return TRUE;

			for (auto container : m_Packs)
			{
				if (container == pItem)
					return TRUE;
			}	
		}
		return FALSE;
		//else
		//{
		//	if (InqBoolQuality(AI_ACCEPT_EVERYTHING_BOOL, FALSE))
		//		return TRUE;

		//	// check emote item acceptance here

		//	return FALSE;
		//}
	}
	else
	{
		if (pItem->RequiresPackSlot())
			return FALSE;

		int capacity = GetItemsCapacity();

		if (capacity >= 0)
		{
			if (m_Items.size() < capacity)
				return TRUE;

			for (auto container : m_Items)
			{
				if (container == pItem)
					return TRUE;
			}
		}
		return FALSE;
		//else
		//{
		//	if (InqBoolQuality(AI_ACCEPT_EVERYTHING_BOOL, FALSE))
		//		return TRUE;

		//	// check Give and Refuse emote item acceptance here
		//	if (m_Qualities._emote_table)
		//	{
		//		if (HasEmoteForID(Give_EmoteCategory, pItem->m_Qualities.id) || HasEmoteForID(Refuse_EmoteCategory, pItem->m_Qualities.id))
		//			return TRUE;
		//	}

		//	return FALSE;
		//}
	}
}

void CContainerWeenie::Container_DeleteItem(uint32_t item_id)
{
	CWeenieObject *item = FindContainedItem(item_id);
	if (!item)
		return;

	bool bWielded = item->IsWielded() ? true : false;

	// take it out of whatever slot it is in
	ReleaseContainedItemRecursive(item);

	item->SetWeenieContainer(0);
	item->SetWielderID(0);
	item->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
	
	if (bWielded && item->AsClothing())
	{
		if (m_Qualities.GetInt(HERITAGE_GROUP_INT, 1) != Gearknight_HeritageGroup) // TODO: Update JUST cloak on gearknight unequip rather than whole model.
			UpdateModel();
	}

	uint32_t RemoveObject[3];
	RemoveObject[0] = 0xF747;
	RemoveObject[1] = item->GetID();
	RemoveObject[2] = item->_instance_timestamp;
	g_pWorld->BroadcastPVS(this, RemoveObject, sizeof(RemoveObject));

	g_pWorld->RemoveEntity(item);
}

uint32_t CContainerWeenie::Container_InsertInventoryItem(uint32_t dwCell, CWeenieObject *item, uint32_t slot)
{
	// You should check if the inventory is full before calling this.
	if (!item->RequiresPackSlot())
	{
		if (slot > (uint32_t) m_Items.size())
			slot = (uint32_t) m_Items.size();

		m_Items.insert(m_Items.begin() + slot, item);
	}
	else
	{
		if (slot > (uint32_t) m_Packs.size())
			slot = (uint32_t) m_Packs.size();

		m_Packs.insert(m_Packs.begin() + slot, item);
	}

	if (dwCell && m_bWorldIsAware)
	{
		item->_position_timestamp++;

		BinaryWriter Blah;
		Blah.Write<uint32_t>(0xF74A);
		Blah.Write<uint32_t>(item->GetID());
		Blah.Write<WORD>(item->_instance_timestamp);
		Blah.Write<WORD>(item->_position_timestamp);
		g_pWorld->BroadcastPVS(dwCell, Blah.GetData(), Blah.GetSize());
	}

	item->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
	item->NotifyIntStatUpdated(PARENT_LOCATION_INT, 0);
	item->unset_parent();
	item->leave_world();

	RecalculateEncumbrance();

	return slot;
}

bool CContainerWeenie::SpawnTreasureInContainer(eTreasureCategory category, int tier, int workmanship)
{
	CWeenieObject *treasure = g_pTreasureFactory->GenerateTreasure(tier, category);

	if (!treasure)
		return false;

	if (workmanship > 0)
	{
		workmanship = min(max(workmanship, 1), 10);
		treasure->m_Qualities.SetInt(ITEM_WORKMANSHIP_INT, workmanship);
	}
	else if (workmanship == 0)
		treasure->m_Qualities.RemoveInt(ITEM_WORKMANSHIP_INT);

	return SpawnInContainer(treasure);
}

bool CContainerWeenie::SpawnInContainer(uint32_t wcid, int amount, int ptid, float shade, bool sendEnvent)
{
	if (amount < 1)
		return false;

	CWeenieObject *item = g_pWeenieFactory->CreateWeenieByClassID(wcid, NULL, false);

	if (!item)
		return false;

	if (ptid)
		item->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, ptid);

	if (shade > 0.0)
		item->m_Qualities.SetFloat(SHADE_FLOAT, shade);

	int maxStackSize = item->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 1);
	
	if (maxStackSize > 1)
	{
		//If we're stackable, first let's try stacking to existing items.
		for (auto possibleMatch : m_Items)
		{
			if (item->m_Qualities.id != possibleMatch->m_Qualities.id)
				continue;
	
			if (ptid != 0 && possibleMatch->InqIntQuality(PALETTE_TEMPLATE_INT, 0) != ptid)
				continue;
	
			if (shade >= 0.0 && possibleMatch->InqFloatQuality(SHADE_FLOAT, 0) != shade)
				continue;
	
			int possibleMatchStackSize = possibleMatch->InqIntQuality(STACK_SIZE_INT, 1);
			if (possibleMatchStackSize < maxStackSize)
			{
				//we have room.
				int roomFor = maxStackSize - possibleMatchStackSize;
				if (roomFor >= amount)
				{
					//room for everything!
					possibleMatch->SetStackSize(possibleMatchStackSize + amount);
					delete item;
					return true;
				}
				else
				{
					//room for some.
					amount -= roomFor;
					possibleMatch->SetStackSize(maxStackSize);
				}
			}
		}
	}

	//We're done stacking and we still have enough for a new item.

	uint32_t totalSlotsRequired = 0;
	if (maxStackSize < 1)
		maxStackSize = 1;
	totalSlotsRequired = amount / maxStackSize;

	if (Container_GetNumFreeMainPackSlots() < totalSlotsRequired)
		return false;

	if (amount > 1)
	{
		int maxStackSize = item->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 1);
		if (amount <= maxStackSize)
			item->SetStackSize(amount);
		else
		{
			int amountOfStacks = amount / maxStackSize;
			int restStackSize = amount % maxStackSize;
			for (int i = 0; i < amountOfStacks; i++)
				SpawnInContainer(wcid, maxStackSize, ptid, shade);
			if (restStackSize > 0)
				item->SetStackSize(restStackSize);
			else
			{
				delete item;
				return true;
			}
		}
	}

	SpawnInContainer(item, sendEnvent);
	return true;
}

bool CContainerWeenie::SpawnCloneInContainer(CWeenieObject *itemToClone, int amount, bool sendEnvent)
{
	if (!itemToClone)
		return false;

	if (amount < 1)
		return false;

	int maxStackSize = itemToClone->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 1);
	int amountOfStacks = 0;
	int restStackSize = 0;
	
	if (amount <= maxStackSize)
	{
		restStackSize = amount;
	}
	else
	{
		amountOfStacks = amount / maxStackSize;
		restStackSize =  amount % maxStackSize;
	}
	int numStacks = amountOfStacks + (restStackSize > 0 ? 1 : 0);

	for (int i = numStacks; i > 0 ; i--)
	{
		CWeenieObject *item = g_pWeenieFactory->CloneWeenie(itemToClone);
		if (!item)
			return false;
	
		if ((i == 1) && (restStackSize > 0))
		{
			item->SetStackSize(restStackSize);
		}
		else
		{
			item->SetStackSize(maxStackSize);
		}
			
		if (!SpawnInContainer(item, sendEnvent, false))
		{
			CWeenieObject *owner = this->GetWorldTopLevelOwner();
			if (owner)
			{
				item->SetInitialPosition(owner->m_Position);

				if (g_pWorld->CreateEntity(item))
				{
					item->_timeToRot = Timer::cur_time + 300.0;
					item->_beganRot = false;
					item->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, item->_timeToRot);
				}
								
			}
			else
			{
				delete item;
				continue;
			}
		}
	}
	return true;
}

bool CContainerWeenie::SpawnInContainer(CWeenieObject *item, bool sendEnvent, bool deleteItemOnFailure)
{
	if (item)
	{
		item->SetID(g_pWorld->GenerateGUID(eDynamicGUID));
		if (!Container_CanStore(item))
		{
			if (sendEnvent)
				NotifyInventoryFailedEvent(item->GetID(), WERROR_GIVE_NOT_ALLOWED);

			if (deleteItemOnFailure)
				delete item;
			return false;
		}

		if (!g_pWorld->CreateEntity(item))
		{
			if (sendEnvent)
				NotifyInventoryFailedEvent(item->GetID(), WERROR_GIVE_NOT_ALLOWED);
			if (deleteItemOnFailure)
				delete item;
			return false;
		}

		if (sendEnvent)
		{
			SendNetMessage(InventoryMove(item->GetID(), GetID(), 0, item->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);
			if (item->AsContainer())
				item->AsContainer()->MakeAwareViewContent(this);
			MakeAware(item, true);

			if (_openedById != 0)
			{
				CWeenieObject *openedBy = g_pWorld->FindObject(_openedById);

				if (openedBy)
				{
					openedBy->SendNetMessage(InventoryMove(item->GetID(), GetID(), 0, item->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);
					if (item->AsContainer())
						item->AsContainer()->MakeAwareViewContent(this);
					openedBy->MakeAware(item, true);
				}
			}
		}

		OnReceiveInventoryItem(this, item, 0);
		return true;
	}
	else
		return false;
}

uint32_t CContainerWeenie::OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot)
{
	if (source != this)
	{
		// By default, if we receive things just delete them... creatures can override this
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

CWeenieObject *CContainerWeenie::FindContained(uint32_t object_id)
{
	return FindContainedItem(object_id);
}

void CContainerWeenie::InitPhysicsObj()
{
	CWeenieObject::InitPhysicsObj();

	if (!_phys_obj)
		return;
	
	for (auto item : m_Wielded)
	{
#ifdef _DEBUG
		assert(item->GetWielderID() == GetID());
#endif

		int parentLocation = item->InqIntQuality(PARENT_LOCATION_INT, PARENT_ENUM::PARENT_NONE);
		if (parentLocation != PARENT_ENUM::PARENT_NONE)
		{
			item->set_parent(this, parentLocation);

			int placement = item->InqIntQuality(PLACEMENT_POSITION_INT, 0);
			assert(placement);

			item->SetPlacementFrame(placement, FALSE);
		}
	}

#ifdef _DEBUG
	for (auto item : m_Items)
	{
		assert(item->InqIntQuality(PARENT_LOCATION_INT, PARENT_ENUM::PARENT_NONE) == PARENT_ENUM::PARENT_NONE);
	}

	for (auto pack : m_Packs)
	{
		assert(pack->InqIntQuality(PARENT_LOCATION_INT, PARENT_ENUM::PARENT_NONE) == PARENT_ENUM::PARENT_NONE);
	}
#endif
}

void CContainerWeenie::SaveEx(class CWeenieSave &save)
{
	CWeenieObject::SaveEx(save);

	for (auto item : m_Wielded)
	{
		save._equipment.push_back(item->GetID());
		item->Save();
	}

	for (auto item : m_Items)
	{
		save._inventory.push_back(item->GetID());
		item->Save();
	}

	for (auto item : m_Packs)
	{
		save._packs.push_back(item->GetID());
		item->Save();
	}
}

void CContainerWeenie::LoadEx(class CWeenieSave &save)
{
	CWeenieObject::LoadEx(save);

	for (auto item : save._equipment)
	{
		CWeenieObject *weenie = CWeenieObject::Load(item);

		if (weenie)
		{
			if (weenie->RequiresPackSlot())
			{
				delete weenie;
				continue;
			}

			//assert(weenie->IsWielded());
			//assert(!weenie->IsContained());

			// make sure it has the right settings (shouldn't be necessary)
			weenie->SetWielderID(GetID());
			weenie->SetWeenieContainer(0);
			weenie->m_Qualities.RemovePosition(INSTANTIATION_POSITION);
			weenie->m_Qualities.RemovePosition(LOCATION_POSITION);

			if (g_pWorld->CreateEntity(weenie, false))
			{
				m_Wielded.push_back(weenie);

				if (int combatUse = weenie->InqIntQuality(COMBAT_USE_INT, 0, TRUE))
				{
					int frame = weenie->InqIntQuality(PLACEMENT_POSITION_INT, 0, TRUE);
					if (combatUse == COMBAT_USE_MELEE && frame == LeftWeapon)
						combatUse = COMBAT_USE_OFFHAND;
					SetWieldedCombat(weenie, (COMBAT_USE)combatUse);
				}

				assert(weenie->IsWielded());
				assert(!weenie->IsContained());
			}
			else
			{
				// remove any enchantments associated with this item that we failed to wield...
				if (m_Qualities._enchantment_reg)
				{
					PackableListWithJson<uint32_t> spells_to_remove;

					if (m_Qualities._enchantment_reg->_add_list)
					{
						for (const auto &entry : *m_Qualities._enchantment_reg->_add_list)
						{
							if (entry._caster == item)
							{
								spells_to_remove.push_back(entry._id);
							}
						}
					}

					if (m_Qualities._enchantment_reg->_mult_list)
					{
						for (const auto &entry : *m_Qualities._enchantment_reg->_mult_list)
						{
							if (entry._caster == item)
							{
								spells_to_remove.push_back(entry._id);
							}
						}
					}

					if (m_Qualities._enchantment_reg->_cooldown_list)
					{
						for (const auto &entry : *m_Qualities._enchantment_reg->_cooldown_list)
						{
							if (entry._caster == item)
							{
								spells_to_remove.push_back(entry._id);
							}
						}
					}

					if (!spells_to_remove.empty())
					{
						m_Qualities._enchantment_reg->RemoveEnchantments(&spells_to_remove);
					}
				}
			}
		}
	}

	for (auto item : save._inventory)
	{
		CWeenieObject *weenie = CWeenieObject::Load(item);

		if (weenie)
		{
			uint32_t correct_container_iid = weenie->m_Qualities.GetIID(CONTAINER_IID, 0);

			if (weenie->RequiresPackSlot() || (correct_container_iid && correct_container_iid != GetID()))
			{
				delete weenie;
				continue;
			}

			assert(!weenie->IsWielded());
			assert(weenie->IsContained());
			assert(!weenie->InqIntQuality(PARENT_LOCATION_INT, 0));

			// make sure it has the right settings (shouldn't be necessary)
			weenie->SetWielderID(0);
			weenie->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
			weenie->SetWeenieContainer(GetID());
			weenie->m_Qualities.RemovePosition(INSTANTIATION_POSITION);
			weenie->m_Qualities.RemovePosition(LOCATION_POSITION);

			if (g_pWorld->CreateEntity(weenie, false))
			{
				m_Items.push_back(weenie);

				assert(!weenie->IsWielded());
				assert(weenie->IsContained());
			}
		}
	}

	for (auto item : save._packs)
	{
		CWeenieObject *weenie = CWeenieObject::Load(item);

		if (weenie)
		{
			uint32_t correct_container_iid = weenie->m_Qualities.GetIID(CONTAINER_IID, 0);

			if (!weenie->RequiresPackSlot() || (correct_container_iid && correct_container_iid != GetID()))
			{
				delete weenie;
				continue;
			}

			assert(!weenie->IsWielded());
			assert(weenie->IsContained());
			assert(!weenie->InqIntQuality(PARENT_LOCATION_INT, 0));

			// make sure it has the right settings (shouldn't be necessary)
			weenie->SetWielderID(0);
			weenie->m_Qualities.SetInt(PARENT_LOCATION_INT, 0);
			weenie->SetWeenieContainer(GetID());
			weenie->m_Qualities.RemovePosition(INSTANTIATION_POSITION);
			weenie->m_Qualities.RemovePosition(LOCATION_POSITION);

			if (g_pWorld->CreateEntity(weenie, false))
			{
				m_Packs.push_back(weenie);

				assert(!weenie->IsWielded());
				assert(weenie->IsContained());
			}
		}
	}
}

void CContainerWeenie::MakeAwareViewContent(CWeenieObject *other)
{
	//start from the bottom of the tree so the packs have their fill bar correctly populated.
	for (auto pack : m_Packs)
	{
		if (pack->AsContainer())
			pack->AsContainer()->MakeAwareViewContent(other);
		other->MakeAware(pack, true);
	}

	for (auto item : m_Items)
	{
		other->MakeAware(item, true);
	}

	PackableList<ContentProfile> inventoryList;
	for (auto item : m_Items)
	{
		ContentProfile prof;
		prof.m_iid = item->GetID();
		prof.m_uContainerProperties = 0;
		inventoryList.push_back(prof);
	}
	for (auto pack : m_Packs)
	{
		ContentProfile prof;
		prof.m_iid = pack->GetID();
		prof.m_uContainerProperties = 1; //todo: what about foci? Do they need a different value here?
		inventoryList.push_back(prof);
	}

	BinaryWriter viewContent;
	viewContent.Write<uint32_t>(0x196);
	viewContent.Write<uint32_t>(GetID());
	inventoryList.Pack(&viewContent);
	other->SendNetMessage(&viewContent, PRIVATE_MSG, TRUE, FALSE);
}

bool CContainerWeenie::IsGroundContainer()
{
	if (HasOwner())
		return false;

	if (!m_Qualities.m_PositionStats || m_Qualities.m_PositionStats->empty())
		return false;

	return true;
}

bool CContainerWeenie::IsInOpenRange(CWeenieObject *other)
{
	if (!IsGroundContainer())
		return false;

	if (DistanceTo(other, true) >= InqFloatQuality(USE_RADIUS_FLOAT, 1.0))
		return false;

	return true;
}

void CContainerWeenie::OnContainerOpened(CWeenieObject *other)
{
	if (other && other->_lastOpenedRemoteContainerId && other->_lastOpenedRemoteContainerId != GetID())
	{
		if (CWeenieObject *otherContainerObj = g_pWorld->FindObject(other->_lastOpenedRemoteContainerId))
		{
			if (CContainerWeenie *otherContainer = otherContainerObj->AsContainer())
			{
				if (otherContainer->_openedById == other->GetID())
					otherContainer->OnContainerClosed();
			}
		}
	}

	MakeAwareViewContent(other);
	_openedById = other->GetID();
	other->_lastOpenedRemoteContainerId = GetID();
	_failedPreviousCheckToClose = false;

	uint32_t spell = 0;
	if (m_Qualities.InqDataID(SPELL_DID, spell))
		MakeSpellcastingManager()->CastSpellInstant(_openedById, spell);
}

void CContainerWeenie::OnContainerClosed(CWeenieObject *requestedBy)
{
	if (requestedBy)
	{
		BinaryWriter closeContent;
		closeContent.Write<uint32_t>(0x52);
		closeContent.Write<uint32_t>(GetID());
		requestedBy->SendNetMessage(&closeContent, PRIVATE_MSG, TRUE, FALSE);
	}

	_openedById = 0;

	if (InqStringQuality(QUEST_STRING, "") != "")
		ResetToInitialState(); //quest chests reset instantly
	else if (_nextReset < 0 || InqIIDQuality(LAST_UNLOCKER_IID, 0))
	{
		// Use the chest's built in regen or reset interval after it's been closed.
		if (double resetInterval = InqFloatQuality(RESET_INTERVAL_FLOAT, 0))
			_nextReset = Timer::cur_time + (resetInterval * g_pConfig->RespawnTimeMultiplier());
		else if (double regenInterval = InqFloatQuality(REGENERATION_INTERVAL_FLOAT, 0)) //if we don't have a reset interval, fall back to regen interval
			_nextReset = Timer::cur_time + (regenInterval * g_pConfig->RespawnTimeMultiplier());

		// Open the chest for everyone.
		m_Qualities.RemoveInstanceID(LAST_UNLOCKER_IID);
	}
}

void CContainerWeenie::NotifyGeneratedPickedUp(CWeenieObject *weenie)
{
	//container contents do not regenerate individually. Instead once the container is closed we start a reset timer.
	weenie->m_Qualities.RemoveInstanceID(GENERATOR_IID);
}

void CContainerWeenie::ResetToInitialState()
{
	m_Qualities.RemoveInstanceID(OWNER_IID);
	m_Qualities.RemoveInstanceID(LAST_UNLOCKER_IID);

	if (_openedById)
		OnContainerClosed(g_pWorld->FindObject(_openedById));

	SetLocked(m_bInitiallyLocked ? TRUE : FALSE);

	while (!m_Wielded.empty())
		Container_DeleteItem((*m_Wielded.begin())->GetID());
	while (!m_Items.empty())
		Container_DeleteItem((*m_Items.begin())->GetID());
	while (!m_Packs.empty())
		Container_DeleteItem((*m_Packs.begin())->GetID());


	if (m_Qualities._generator_table)
	{
		if (m_Qualities._generator_registry || m_Qualities._generator_queue)
		{
			for (auto &entry : m_Qualities._generator_table->_profile_list)
			{
				if (entry.whereCreate & Contain_RegenLocationType)
				{
					if (m_Qualities._generator_registry)
					{
						for (PackableHashTable<uint32_t, GeneratorRegistryNode>::iterator i = m_Qualities._generator_registry->_registry.begin(); i != m_Qualities._generator_registry->_registry.end();)
						{
							if (entry.slot == i->second.slot)
								i = m_Qualities._generator_registry->_registry.erase(i);
							else
								i++;
						}
					}

					if (m_Qualities._generator_queue)
					{
						for (auto i = m_Qualities._generator_queue->_queue.begin(); i != m_Qualities._generator_queue->_queue.end();)
						{
							if (entry.slot == i->slot)
								i = m_Qualities._generator_queue->_queue.erase(i);
							else
								i++;
						}
					}

					if (m_GeneratorSpawns.size() > 0)
						m_GeneratorSpawns.clear();
				}
			}
		}
		else
		{
			if(m_Qualities._generator_registry)
				m_Qualities._generator_registry->_registry.clear();
			if (m_Qualities._generator_queue)
				m_Qualities._generator_queue->_queue.clear();
			if (m_GeneratorSpawns.size() > 0)
				m_GeneratorSpawns.clear();
		}
	}

	InitCreateGenerator();
}

int CContainerWeenie::DoUseResponse(CWeenieObject *other)
{
	if (IsBusyOrInAction())
		return WERROR_NONE;

	if (!(GetItemType() & ITEM_TYPE::TYPE_CONTAINER))
		return WERROR_NONE;

	if (!IsGroundContainer())
		return WERROR_NONE;

	if (!IsInOpenRange(other))
		return WERROR_NONE;

	if (IsLocked())
	{
		EmitSound(Sound_OpenFailDueToLock, 1.0f);
		return WERROR_NONE;
	}

	int openError = CheckOpenContainer(other);

	if (openError)
		return WERROR_NONE;

	if (_openedById)
	{
		if (_openedById == other->GetID())
		{
			//this is actually a close container request.
			OnContainerClosed(other);
			return WERROR_NONE;
		}
		else
			return WERROR_CHEST_ALREADY_OPEN;
	}

	if (uint32_t unlocker = InqIIDQuality(LAST_UNLOCKER_IID, 0))
	{
		// Unopened and unlocked chests are open to everyone after 30 seconds.
		if (unlocker != other->GetID() && _nextReset - 570 >= Timer::cur_time)
		{
			other->SendText(csprintf("This chest is claimed by the person who unlocked it."), LTT_DEFAULT);
			return WERROR_NONE;
		}
	}

	std::string questString;
	if (m_Qualities.InqString(QUEST_STRING, questString) && !questString.empty())
	{
		if (uint32_t owner = InqIIDQuality(OWNER_IID, 0))
		{
			if (owner != other->GetID())
			{
				other->SendText(csprintf("This chest is claimed by the person who lockpicked it."), LTT_DEFAULT);
				return WERROR_NONE;
			}
		}
		else
		{
			if (other->InqQuest(questString.c_str()))
			{
				int timeTilOkay = other->InqTimeUntilOkayToComplete(questString.c_str());

				if (timeTilOkay > 0)
				{
					int secs = timeTilOkay % 60;
					timeTilOkay /= 60;

					int mins = timeTilOkay % 60;
					timeTilOkay /= 60;

					int hours = timeTilOkay % 24;
					timeTilOkay /= 24;

					int days = timeTilOkay;

					other->SendText(csprintf("You cannot open this for another %dd %dh %dm %ds.", days, hours, mins, secs), LTT_DEFAULT);
				}

				return WERROR_CHEST_USED_TOO_RECENTLY;
			}

			other->StampQuest(questString.c_str());
		}
	}

	OnContainerOpened(other);

	return CWeenieObject::DoUseResponse(other);
}

void CContainerWeenie::InventoryTick()
{
	CWeenieObject::InventoryTick();

	for (auto wielded : m_Wielded)
	{
		wielded->WieldedTick();

//#ifdef _DEBUG
//		wielded->DebugValidate();
//#endif
	}

	for (auto item : m_Items)
	{
		item->InventoryTick();

//#ifdef _DEBUG
//		item->DebugValidate();
//#endif
	}

	for (auto pack : m_Packs)
	{
		pack->InventoryTick();

//#ifdef _DEBUG
//		pack->DebugValidate();
//#endif
	}
}

void CContainerWeenie::Tick()
{
	CWeenieObject::Tick();

	if (_openedById)
	{
		CheckToClose();
	}

	if (Timer::cur_time < _nextInventoryTick)
	{
		return;
	}

	for(auto wielded : m_Wielded)
	{
		wielded->WieldedTick();

//#ifdef _DEBUG
//		wielded->DebugValidate();
//#endif
	}

	for (auto item : m_Items)
	{
		if (item)
			item->InventoryTick();

//#ifdef _DEBUG
//		item->DebugValidate();
//#endif
	}

	for (auto pack : m_Packs)
	{
		pack->InventoryTick();

//#ifdef _DEBUG
//		pack->DebugValidate();
//#endif
	}

	_nextInventoryTick = Timer::cur_time + Random::GenFloat(0.4, 0.6);
}

void CContainerWeenie::CheckToClose()
{
	if (_nextCheckToClose > Timer::cur_time)
	{
		return;
	}

	_nextCheckToClose = Timer::cur_time + 1.0;
	
	if (CWeenieObject *other = g_pWorld->FindObject(_openedById))
	{
		if (other->IsDead() || !IsInOpenRange(other))
		{
			if (_failedPreviousCheckToClose)
				OnContainerClosed();
			else
			{
				_failedPreviousCheckToClose = true; //give the client a moment to send the chest close message or we might interpret it as a open request(both use the use packet)
				_nextCheckToClose = Timer::cur_time + 2.0;
			}
		}
		else
			_failedPreviousCheckToClose = false;
	}
	else
	{
		OnContainerClosed();
	}
}

int CContainerWeenie::CheckOpenContainer(CWeenieObject *other)
{
	if (_openedById)
	{
		if (_openedById == other->GetID())
		{
			return WERROR_NONE;
		}
		
		return WERROR_CHEST_ALREADY_OPEN;
	}

	return WERROR_NONE;
}

void CContainerWeenie::HandleNoLongerViewing(CWeenieObject *other)
{
	if (!_openedById || _openedById != other->GetID())
		return;

	OnContainerClosed();
}

void CContainerWeenie::DebugValidate()
{
	CWeenieObject::DebugValidate();

#ifdef _DEBUG
	assert(!GetWielderID());
	
	for (auto wielded : m_Wielded)
	{
		assert(wielded->GetWielderID() == GetID());
		wielded->DebugValidate();
	}

	for (auto item : m_Items)
	{
		if (item->GetContainerID() != GetID())
		assert(item->GetContainerID() == GetID());
		item->DebugValidate();
	}

	for (auto pack : m_Packs)
	{
		assert(pack->GetContainerID() == GetID());
		pack->DebugValidate();
	}
#endif
}

uint32_t CContainerWeenie::RecalculateCoinAmount(int currencyid)
{
	int coinAmount = 0;
	for (auto item : m_Items)
	{
		if (item->m_Qualities.id == currencyid)
			coinAmount += item->InqIntQuality(STACK_SIZE_INT, 1, true);
	}

	for (auto pack : m_Packs)
		coinAmount += pack->RecalculateCoinAmount(currencyid);

	if (AsPlayer() && currencyid == W_COINSTACK_CLASS) {
		m_Qualities.SetInt(COIN_VALUE_INT, coinAmount);
		NotifyIntStatUpdated(COIN_VALUE_INT);
	}
	return coinAmount;
}


uint32_t CContainerWeenie::ConsumeCoin(int amountToConsume, int currencyid)
{
	if (amountToConsume < 1)
		return 0;

	if (AsPlayer()) //we don't need to recalculate this if we're a subcontainer
	{
		if (RecalculateCoinAmount(currencyid) < amountToConsume) //force recalculate our coin amount and check so we don't even try to consume if we don't have enough.
			return 0;
	}

	std::list<CWeenieObject *> removeList;

	uint32_t amountConsumed = 0;
	for (auto item : m_Items)
	{
		if (item->m_Qualities.id == currencyid)
		{
			int stackSize = item->InqIntQuality(STACK_SIZE_INT, 1, true);
			if (stackSize <= amountToConsume)
			{
				removeList.push_back(item);
				amountToConsume -= stackSize;
				amountConsumed += stackSize;
			}
			else
			{
				item->SetStackSize(stackSize - amountToConsume);
				amountConsumed += amountToConsume;
				amountToConsume = 0;
				break;
			}
		}
	}

	for (auto item : removeList)
		item->Remove();

	if (amountToConsume > 0)
	{
		for (auto pack : m_Packs)
		{
			uint32_t amountFromPack = pack->ConsumeCoin(amountToConsume, currencyid);
			amountToConsume -= amountFromPack;
			amountConsumed += amountFromPack;

			if (amountToConsume <= 0)
				break;
		}
	}

	if (AsPlayer() && currencyid == W_COINSTACK_CLASS)
		RecalculateCoinAmount(W_COINSTACK_CLASS);
	return amountConsumed;
}

void CContainerWeenie::RecalculateEncumbrance()
{
	int oldValue = InqIntQuality(ENCUMB_VAL_INT, 0);

	int newValue = 0;
	for (auto wielded : m_Wielded)
	{
		newValue += wielded->InqIntQuality(ENCUMB_VAL_INT, 0);
	}

	for (auto item : m_Items)
	{
		newValue += item->InqIntQuality(ENCUMB_VAL_INT, 0);
	}

	for (auto pack : m_Packs)
	{
		pack->RecalculateEncumbrance();
		newValue += pack->InqIntQuality(ENCUMB_VAL_INT, 0);
	}

	if (oldValue != newValue)
	{
		if (m_Qualities.m_WeenieType == Corpse_WeenieType || m_Qualities.m_WeenieType == Chest_WeenieType)
		{
			CWeenieDefaults *defaults = g_pWeenieFactory->GetWeenieDefaults(m_Qualities.id);

			if (defaults)
				newValue += defaults->m_Qualities.GetInt(ENCUMB_VAL_INT, 0);
		}

		m_Qualities.SetInt(ENCUMB_VAL_INT, newValue);
		NotifyIntStatUpdated(ENCUMB_VAL_INT, true);
	}
}

bool CContainerWeenie::IsAttunedOrContainsAttuned()
{
	for (auto wielded : m_Wielded)
	{
		if (wielded->IsAttunedOrContainsAttuned())
			return true;
	}

	for (auto item : m_Items)
	{
		if (item->IsAttunedOrContainsAttuned())
			return true;
	}

	for (auto pack : m_Packs)
	{
		if (pack->IsAttunedOrContainsAttuned())
			return true;
	}

	return CWeenieObject::IsAttunedOrContainsAttuned();
}

bool CContainerWeenie::HasContainerContents()
{
	if (!m_Wielded.empty() || !m_Items.empty() || !m_Packs.empty())
		return true;

	return CWeenieObject::HasContainerContents();
}

void CContainerWeenie::AdjustToNewCombatMode()
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, WEAPON_LOC | HELD_LOC);

	COMBAT_MODE newCombatMode = COMBAT_MODE::UNDEF_COMBAT_MODE;

	for (auto item : wielded)
	{
		newCombatMode = item->GetEquippedCombatMode();
		if (newCombatMode != COMBAT_MODE::UNDEF_COMBAT_MODE)
			break;
	}

	if (newCombatMode == COMBAT_MODE::UNDEF_COMBAT_MODE)
		newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;

	ChangeCombatMode(newCombatMode, false);
}

uint32_t CContainerWeenie::GetItemCount(int itemid)
{
	int itemAmount = 0;
	for (auto item : m_Items)
	{
		if (item->m_Qualities.id == itemid)
			itemAmount += item->InqIntQuality(STACK_SIZE_INT, 1, true);
	}

	for (auto pack : m_Packs)
		itemAmount += pack->GetItemCount(itemid);

	return itemAmount;
}

uint32_t CContainerWeenie::ConsumeItem(int amountToConsume, int itemid)
{
	if (amountToConsume < 1)
		return 0;

	if (AsPlayer()) //we don't need to recalculate this if we're a subcontainer
	{
		if (GetItemCount(itemid) < amountToConsume) //force recalculate our coin amount and check so we don't even try to consume if we don't have enough.
			return 0;
	}

	std::list<CWeenieObject *> removeList;

	uint32_t amountConsumed = 0;
	for (auto item : m_Items)
	{
		if (item->m_Qualities.id == itemid)
		{
			int stackSize = item->InqIntQuality(STACK_SIZE_INT, 1, true);
			if (stackSize <= amountToConsume)
			{
				removeList.push_back(item);
				amountToConsume -= stackSize;
				amountConsumed += stackSize;
			}
			else
			{
				item->SetStackSize(stackSize - amountToConsume);
				//amountToConsume -= stackSize;
				amountConsumed += amountToConsume;
				break;
			}
		}
	}

	for (auto item : removeList)
		item->Remove();

	if (amountToConsume > 0)
	{
		for (auto pack : m_Packs)
		{
			uint32_t amountFromPack = pack->ConsumeItem(amountToConsume, itemid);
			amountToConsume -= amountFromPack;
			amountConsumed += amountFromPack;

			if (amountToConsume <= 0)
				break;
		}
	}

	return amountConsumed;
}
