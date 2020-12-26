#include <StdAfx.h>
#include "WeenieObject.h"
#include "AllegianceManager.h"
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
#include <chrono>

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
	m_Qualities.SetFloat(HEARTBEAT_INTERVAL_FLOAT, DEFAULT_HEARTBEAT_INTERVAL);
	_nextHeartBeat = Timer::cur_time + DEFAULT_HEARTBEAT_INTERVAL;
}

CMonsterWeenie::~CMonsterWeenie()
{
	SafeDelete(m_MonsterAI);
}

void CMonsterWeenie::ApplyQualityOverrides()
{
	int group = 1;
	int gender = 1;

	bool persistantAppearance = false;

	int attackable;
	if (m_Qualities.InqBool(ATTACKABLE_BOOL, attackable))
	{
		//use this to determine if we're a town NPC or an humanoid monster. We don't want to persist humanoid monster's appearance or they will all look the same!
		if(!attackable)
			persistantAppearance = true;
	}

	uint32_t seedBackup = Random::GetSeed();
	if (persistantAppearance)
	{
		//let's change our pseudo random seed to be based on our WCID so we mantain the same appearance between sessions.
		//One downside of this is that all instances of the same WCID have the same appearance, this affects town criers and collectors and would affect humanoid monsters(like bandits) if it were not for the persistantAppearance bool that is set above.
		Random::SetSeed(m_Qualities.GetID());
	}

	int species = 0;
	if (!m_Qualities.InqInt(CREATURE_TYPE_INT, species) || species != CreatureType::human)
		return;

	if (!m_Qualities.InqInt(HERITAGE_GROUP_INT, group))
	{
		std::string heritageString;
		if (m_Qualities.InqString(HERITAGE_GROUP_STRING, heritageString))
		{
			if (heritageString == "Aluvian")
				group = 1;
			else if (heritageString == "Gharu'ndim")
				group = 2;
			else if (heritageString == "Sho")
				group = 3;
			else if (heritageString == "Viamontian")
				group = 4;
		}
		else
			group = Random::GenInt(1, 3); //if the max value was set to 4 it would also allow Viamontians but leave those out as generaly they stick to Sanamar.
		m_Qualities.SetInt(HERITAGE_GROUP_INT, group);
	}

	if (!m_Qualities.InqInt(GENDER_INT, gender))
	{
		std::string sex;
		if (!m_Qualities.InqString(SEX_STRING, sex))
			return;

		if (sex == "Male") gender = 1;
		else if (sex == "Female") gender = 2;

		m_Qualities.SetInt(GENDER_INT, gender);
	}

	HeritageGroup_CG *heritage = CachedCharGenData->mHeritageGroupList.lookup(group);
	Sex_CG *sex = heritage->mGenderList.lookup(gender);

	uint32_t tmp = 0;
	uint32_t tmp_default = 0;

	int max = 0;
	int idx = 0;
	ObjDesc *desc = nullptr;
	HairStyle_CG *hair = nullptr;

	//skin color
	if (!m_Qualities.InqDataID(SKIN_PALETTE_DID, tmp))
	{

		PalSet *ps;
		if (ps = PalSet::Get(sex->skinPalSet))
		{
			uint32_t skinPalette = ps->GetPaletteID(Random::GenFloat());
			m_Qualities.SetDataID(SKIN_PALETTE_DID, skinPalette);
			PalSet::Release(ps);
		}
	}

	// hair/head
	if (m_Qualities.InqDataID(HEAD_OBJECT_DID, tmp))
	{
		hair = sex->FindHairStyle(tmp);
	}
	else
	{
		max = 8; //Changed to limit NPC hair styles to classic ones. previously sex->mHairStyleList.num_used - 1;
		idx = Random::GenInt(0, max);
		hair = &(sex->mHairStyleList.array_data[idx]);

		AnimPartChange *change = hair->objDesc.GetAnimPartChange(Head);

		if (change)
			m_Qualities.SetDataID(HEAD_OBJECT_DID, change->part_id);
	}
	if (!m_Qualities.InqDataID(HAIR_PALETTE_DID, tmp))
	{
		int hairColor = Random::GenInt(0, (int)sex->mHairColorList.num_used - 1); 
		PalSet *ps;
		if (ps = PalSet::Get(sex->mHairColorList.array_data[hairColor]))
		{
			uint32_t hairPalette = ps->GetPaletteID(Random::GenFloat());
			m_Qualities.SetDataID(HAIR_PALETTE_DID, hairPalette);
			PalSet::Release(ps);
		}
	}

	// eyes
	if (!m_Qualities.InqDataID(DEFAULT_EYES_TEXTURE_DID, tmp_default))
	{
		max = sex->mEyeStripList.num_used - 1;
		idx = Random::GenInt(0, max);
		EyesStrip_CG *eyes = &(sex->mEyeStripList.array_data[idx]);

		desc = hair->bald ? &eyes->objDesc_Bald : &eyes->objDesc;

		if (desc->firstTMChange)
		{
			m_Qualities.SetDataID(DEFAULT_EYES_TEXTURE_DID, desc->firstTMChange->old_tex_id);
			m_Qualities.SetDataID(EYES_TEXTURE_DID, desc->firstTMChange->new_tex_id);
		}

		//if (desc->paletteID)
		//{
		//	
		//}
	}
	if (!m_Qualities.InqDataID(EYES_PALETTE_DID, tmp))
	{
		int eyeColor = Random::GenInt(0, (int)sex->mEyeColorList.num_used - 1);
		m_Qualities.SetDataID(EYES_PALETTE_DID, sex->mEyeColorList.array_data[eyeColor]);
	}

	// nose
	if (!m_Qualities.InqDataID(DEFAULT_NOSE_TEXTURE_DID, tmp_default))
	{
		max = sex->mNoseStripList.num_used - 1;
		idx = Random::GenInt(0, max);
		FaceStrip_CG *nose = &(sex->mNoseStripList.array_data[idx]);

		desc = &nose->objDesc;

		if (desc->firstTMChange)
		{
			m_Qualities.SetDataID(DEFAULT_NOSE_TEXTURE_DID, desc->firstTMChange->old_tex_id);
			m_Qualities.SetDataID(NOSE_TEXTURE_DID, desc->firstTMChange->new_tex_id);
		}
	}

	// mouth
	if (!m_Qualities.InqDataID(DEFAULT_MOUTH_TEXTURE_DID, tmp_default))
	{
		max = sex->mMouthStripList.num_used - 1;
		idx = Random::GenInt(0, max);
		FaceStrip_CG *mouth = &(sex->mMouthStripList.array_data[idx]);

		desc = &mouth->objDesc;

		if (desc->firstTMChange)
		{
			m_Qualities.SetDataID(DEFAULT_MOUTH_TEXTURE_DID, desc->firstTMChange->old_tex_id);
			m_Qualities.SetDataID(MOUTH_TEXTURE_DID, desc->firstTMChange->new_tex_id);
		}
	}

	//uint32_t skin_palette_id;
	//if (m_Qualities.InqDataID(SKIN_PALETTE_DID, skin_palette_id))
	//	objDesc.AddSubpalette(new Subpalette(skin_palette_id, 0 << 3, 0x18 << 3));

	//uint32_t hair_palette_id;
	//if (m_Qualities.InqDataID(HAIR_PALETTE_DID, hair_palette_id))
	//	objDesc.AddSubpalette(new Subpalette(hair_palette_id, 0x18 << 3, 0x8 << 3));

	//uint32_t eye_palette_id;
	//if (m_Qualities.InqDataID(EYES_PALETTE_DID, eye_palette_id))
	//	objDesc.AddSubpalette(new Subpalette(eye_palette_id, 0x20 << 3, 0x8 << 3));

	Random::SetSeed(seedBackup); //and return our seed to its original value.
}

void CMonsterWeenie::PreSpawnCreate()
{
	if (m_Qualities._create_list)
		g_pWeenieFactory->AddFromCreateList(this, m_Qualities._create_list, (DestinationType)(Wield_DestinationType | WieldTreasure_DestinationType));

	if (uint32_t wieldedTreasureType = InqDIDQuality(WIELDED_TREASURE_TYPE_DID, 0))
		g_pWeenieFactory->GenerateFromTypeOrWcid(this, DestinationType::WieldTreasure_DestinationType, wieldedTreasureType);

	if (m_Qualities.GetDID(PHYSICS_SCRIPT_DID, 0))
	{
		// Add one here or else the wrong script will play. e.g. Frost Breath for Olthoi and Sewer Rats instead of Acid Breath.
		m_DefaultScript = m_Qualities.GetDID(PHYSICS_SCRIPT_DID, 0);
		m_DefaultScriptIntensity = m_Qualities.GetFloat(PHYSICS_SCRIPT_INTENSITY_FLOAT, 1.0);
	}
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

CWeenieObject *CMonsterWeenie::SpawnWielded(uint32_t index, SmartArray<Style_CG> possibleStyles, uint32_t color, SmartArray<uint32_t> validColors, double shade)
{
	index = max(min(index, (uint32_t)possibleStyles.num_used - 1), (uint32_t)0);
	shade = max(min(shade, 1.0), 0.0);

	uint32_t wcid = possibleStyles.array_data[index].weenieDefault;
	uint32_t clothingTable = possibleStyles.array_data[index].clothingTable;

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

CWeenieObject *CMonsterWeenie::SpawnWielded(uint32_t wcid, int ptid, float shade)
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

	item->SetID(g_pWorld->GenerateGUID(m_Qualities.GetInt(HERITAGE_GROUP_INT, 0) == 0 ? eEphemeral : eDynamicGUID));

	if (!g_pWorld->CreateEntity(item, false))
	{
		//delete item;
		return NULL;
	}

	if (!FinishMoveItemToWield(item, item->m_Qualities.GetInt(LOCATIONS_INT, 0)))
	{
		g_pWorld->RemoveEntity(item);
		item = NULL;
	}

	return item;
}

CWeenieObject *CMonsterWeenie::FindValidNearbyItem(uint32_t itemId, float maxDistance)
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
			item = g_pWorld->FindObject(itemId, false, true);

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

CContainerWeenie *CMonsterWeenie::FindValidNearbyContainer(uint32_t containerId, float maxDistance)
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

			if (!object)
			{
				NotifyInventoryFailedEvent(containerId, WERROR_OBJECT_GONE);
				return NULL;
			}

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

bool CMonsterWeenie::GetEquipPlacementAndHoldLocation(CWeenieObject *item, uint32_t location, uint32_t *pPlacementFrame, uint32_t *pHoldLocation)
{
	if (location & (MELEE_WEAPON_LOC | TWO_HANDED_LOC))
	{
		int leftTethered = 0;
//		if (item->m_Qualities.InqBool(AUTOWIELD_LEFT_BOOL, leftTethered) && leftTethered == 1)
//		{
//			*pPlacementFrame = Placement::LeftHand;
//			*pHoldLocation = PARENT_LEFT_HAND;
//		}
//		else
		{
			*pPlacementFrame = Placement::RightHandCombat;
			*pHoldLocation = PARENT_RIGHT_HAND;
		}
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
		if (item->InqIntQuality(LOCATIONS_INT, 0, TRUE) != SHIELD_LOC)
		{
			int combatStyle = item->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, 0);

			*pPlacementFrame = Placement::LeftWeapon;
			*pHoldLocation = (combatStyle == Unarmed_CombatStyle) ? PARENT_LEFT_HAND : PARENT_LEFT_WEAPON;
		}
		else
		{
			*pPlacementFrame = Placement::Shield;
			*pHoldLocation = PARENT_SHIELD;
		}
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

	for (uint32_t i = 0; i < 32; i++)
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
		uint32_t skillLevel = 0;
		if (!wielder->m_Qualities.InqSkill((STypeSkill)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_SKILL_TOO_LOW;
		break;
	}
	case 2: // base skill
	{
		uint32_t skillLevel = 0;
		if (!wielder->m_Qualities.InqSkill((STypeSkill)skillType, skillLevel, TRUE) || (int)skillLevel < wieldDifficulty)
			return WERROR_SKILL_TOO_LOW;
		break;
	}
	case 3: // attribute
	{
		uint32_t skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute((STypeAttribute)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 4: // base attribute
	{
		uint32_t skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute((STypeAttribute)skillType, skillLevel, TRUE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 5: // attribute 2nd 
	{
		uint32_t skillLevel = 0;
		if (!wielder->m_Qualities.InqAttribute2nd((STypeAttribute2nd)skillType, skillLevel, FALSE) || (int)skillLevel < wieldDifficulty)
			return WERROR_ACTIVATION_SKILL_TOO_LOW;
		break;
	}
	case 6: // attribute 2nd base
	{
		uint32_t skillLevel = 0;
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

bool CMonsterWeenie::MoveItemToContainer(uint32_t sourceItemId, uint32_t targetContainerId, uint32_t targetSlot, bool animationDone)
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
	if (!targetContainer || targetContainer->IsCorpse() || !targetContainer->Container_CanStore(sourceItem))
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_OBJECT_GONE);
		return false;
	}

	if (sourceItem->IsAttunedOrContainsAttuned() && targetContainer->GetTopLevelID() != GetID())
	{
		NotifyInventoryFailedEvent(sourceItem->id, WERROR_ATTUNED_ITEM);
		return false;
	}

	uint32_t freezer = sourceItem->InqIIDQuality(FREEZER_IID, 0);
	if (freezer != 0 && freezer != GetID())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_FROZEN);
		return false;
	}

	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, GetID());

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

void CMonsterWeenie::FinishMoveItemToContainer(CWeenieObject *sourceItem, CContainerWeenie *targetContainer, uint32_t targetSlot, bool bSendEvent, bool silent)
{
	// Scenarios to consider:
	// 1. Item being stored is equipped.
	// 2. Item being stored is on the GROUND!
	// 3. Item being stored is already in the inventory.
	// 4. Item being stored is in an external container (chest) or being moved to an external container (chest)

	bool wasWielded = sourceItem->IsWielded();
	bool isPickup = sourceItem->GetWorldTopLevelOwner() != this;

	uint32_t dwCell = GetLandcell();

	//if (!sourceItem->HasOwner())
	//{
	if (CWeenieObject *generator = g_pWorld->FindObject(sourceItem->InqIIDQuality(GENERATOR_IID, 0)))
		generator->NotifyGeneratedPickedUp(sourceItem);
	//}

	// Take it out of whatever slot it's in.
	//if (sourceItem->m_Qualities.m_WeenieType == WeenieType::Caster_WeenieType || sourceItem->m_Qualities.m_WeenieType == WeenieType::Clothing_WeenieType ||
	//	sourceItem->m_Qualities.m_WeenieType == WeenieType::MeleeWeapon_WeenieType || sourceItem->m_Qualities.m_WeenieType == WeenieType::MissileLauncher_WeenieType ||
	//	sourceItem->m_Qualities.m_WeenieType == WeenieType::Missile_WeenieType)
	//{
	//}
	if (sourceItem->GetItemType() & (TYPE_VESTEMENTS | TYPE_WEAPON_OR_CASTER))
	{
		sourceItem->ReleaseFromAnyWeenieParent(true, true);
	}
	else
	{
		sourceItem->ReleaseFromAnyWeenieParent(false, true);
	}
	sourceItem->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
	sourceItem->SetWeenieContainer(targetContainer->GetID());
	sourceItem->ReleaseFromBlock();
	if(AsPlayer())
		sourceItem->m_Qualities.SetInstanceID(OWNER_IID, GetID());

	//store the current motion here for use later.
	uint32_t oldmotion = get_minterp()->InqStyle();

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

	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, 0);

	if (AsPlayer() && wasWielded)
	{
		int setId = 0;
		if (sourceItem->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, setId))
		{
			AsPlayer()->UpdateSetSpells(setId, sourceItem->GetID());
		}
		if (sourceItem->IsAetheria())
		{
			AsPlayer()->UpdateSigilProcRate();
		}
	}
	

	if (targetContainer->GetWorldTopLevelOwner() != this)
		RecalculateEncumbrance();

	if (sourceItem->GetItemType() & (TYPE_ARMOR | TYPE_CLOTHING))
	{
		if (m_Qualities.GetInt(HERITAGE_GROUP_INT, 1) != Gearknight_HeritageGroup) // TODO: Update JUST cloak on gearknight unequip rather than whole model.
			UpdateModel();
	}

	// Checks the old motion vs Motion_NonCombat to prevent getting stuck while adjusting to the new combat style.
	if (wasWielded && oldmotion != Motion_NonCombat)
		AdjustToNewCombatMode();

	if (AsPlayer() && sourceItem->m_Qualities.id == W_COINSTACK_CLASS)
		RecalculateCoinAmount(W_COINSTACK_CLASS);

	if (!sourceItem->m_Qualities.GetInt(LIFESPAN_INT, 0))
	{
		sourceItem->m_Qualities.RemoveFloat(TIME_TO_ROT_FLOAT);
		sourceItem->_timeToRot = -1.0;
		sourceItem->_beganRot = false;
	}
}

bool CMonsterWeenie::MoveItemTo3D(uint32_t sourceItemId, bool animationDone)
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
	
	uint32_t freezer = sourceItem->InqIIDQuality(FREEZER_IID, 0);
	if (freezer != 0 && freezer != GetID())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_FROZEN);
		return false;
	}
	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, GetID());

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

	//store the current motion here for use later.
	uint32_t oldmotion = get_minterp()->InqStyle();

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

	if (!sourceItem->m_Qualities.m_PositionStats)
		sourceItem->m_Qualities.SetPosition(LOCATION_POSITION, m_Position);

	sourceItem->SetPlacementFrame(0x65, FALSE);

	//if (!pItem->HasAnims())
	//	pItem->SetPlacementFrame(pItem->CanPickup() ? 0x65 : 0, 1);

	if (sourceItem->AsClothing())
	{
		if (m_Qualities.GetInt(HERITAGE_GROUP_INT, 1) != Gearknight_HeritageGroup) // TODO: Update JUST cloak on gearknight unequip rather than whole model.
			UpdateModel();
	}

	g_pWorld->InsertEntity(sourceItem);
	sourceItem->Movement_Teleport(GetPosition());

	if (!sourceItem->m_Qualities.GetInt(LIFESPAN_INT, 0))
	{
		sourceItem->_timeToRot = Timer::cur_time + 300.0;
		sourceItem->_beganRot = false;
		sourceItem->m_Qualities.SetFloat(TIME_TO_ROT_FLOAT, sourceItem->_timeToRot);
	}

	RecalculateEncumbrance();

	if (AsPlayer() && sourceItem->m_Qualities.id == W_COINSTACK_CLASS)
		RecalculateCoinAmount(W_COINSTACK_CLASS);

	// Checks the old motion vs Motion_NonCombat to prevent getting stuck while adjusting to the new combat style.
	if (bWasWielded && oldmotion != Motion_NonCombat)
		AdjustToNewCombatMode();

	if (bWasWielded)
	{
		sourceItem->OnUnwield(this);
	}

	if(AsPlayer() && bWasWielded)
	{
		int setSpell;
		if (sourceItem->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, setSpell))
		{
			AsPlayer()->UpdateSetSpells(setSpell, sourceItem->GetID());
		}
		if (sourceItem->IsAetheria())
		{
			AsPlayer()->UpdateSigilProcRate();
		}
	}
	sourceItem->OnDropped(this);

	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, 0);

	sourceItem->m_bDontClear = false;
}

bool CMonsterWeenie::MoveItemToWield(uint32_t sourceItemId, uint32_t targetLoc, bool animationDone)
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

	uint32_t freezer = sourceItem->InqIIDQuality(FREEZER_IID, 0);
	if (freezer != 0 && freezer != GetID())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_FROZEN);
		return false;
	}
	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, GetID());

	if (sourceItem->IsAetheria())
	{
		int aetheriaUnlock;
		if (m_Qualities.InqInt(AETHERIA_BITFIELD_INT, aetheriaUnlock))
		{
			int aetheriaLevel;
			if (sourceItem->m_Qualities.InqInt(WIELD_DIFFICULTY_INT, aetheriaLevel))
			{
				if (aetheriaLevel == 75 && !(aetheriaUnlock & 0x1))
				{
					NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
					return false;
				}
				else if (aetheriaLevel == 150 && !(aetheriaUnlock & 0x2))
				{
					NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
					return false;
				}
				else if (aetheriaLevel == 225 && !(aetheriaUnlock & 0x4))
				{
					NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
					return false;
				}
			}
		}
		else
		{
			NotifyInventoryFailedEvent(sourceItemId, WERROR_BE_WIELDED_FAILURE);
			return false;
		}
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

bool CMonsterWeenie::FinishMoveItemToWield(CWeenieObject *sourceItem, uint32_t targetLoc)
{
	// Scenarios to consider:
	// 1. Item being equipped from the GROUND!
	// 2. Item being equipped from different equip slot.
	// 3. Item being equipped from the player's inventory.

	bool isPickup = sourceItem->GetWorldTopLevelOwner() != this;

	if (AsPlayer())
	{
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

		if (sourceItem->InqIIDQuality(ALLOWED_WIELDER_IID, 0) && sourceItem->InqIIDQuality(ALLOWED_WIELDER_IID, 0) != GetID())
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_ATTUNED_ITEM);
			return false;
		}

		if (sourceItem->InqIntQuality(HERITAGE_SPECIFIC_ARMOR_INT, 0) && sourceItem->InqIntQuality(HERITAGE_SPECIFIC_ARMOR_INT, 0) != m_Qualities.GetInt(HERITAGE_GROUP_INT, 1)) // Other heritages cannot wear gearknight armor.
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_SPECIFIC_ARMOR_REQUIRES_HERITAGE);
			return false;
		}

		if (m_Qualities.GetInt(HERITAGE_GROUP_INT, 1) == Gearknight_HeritageGroup && !sourceItem->InqIntQuality(HERITAGE_SPECIFIC_ARMOR_INT, 0) && sourceItem->m_Qualities.GetInt(LOCATIONS_INT, 0) < 0x8000) // Only restrict Armor, not weapons, cloaks, jewelry, trinkets, etc.
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_HERITAGE_REQUIRES_SPECIFIC_ARMOR);
			return false;
		}
	}

	if (sourceItem->InqIntQuality(ITEM_TYPE_INT, 0) == TYPE_ARMOR && sourceItem->InqIntQuality(LOCATIONS_INT, 0) == SHIELD_LOC)
	{
		sourceItem->m_Qualities.SetInt(SHIELD_VALUE_INT, sourceItem->InqIntQuality(ARMOR_LEVEL_INT, 0));
	}

	//if (!sourceItem->HasOwner())
	//{
	if (CWeenieObject *generator = g_pWorld->FindObject(sourceItem->InqIIDQuality(GENERATOR_IID, 0)))
		generator->NotifyGeneratedPickedUp(sourceItem);
	//}

	uint32_t cell_id = m_Position.objcell_id;

	// Take it out of whatever slot it may be in
	sourceItem->ReleaseFromAnyWeenieParent(false, false);

	uint32_t placement_id, child_location_id;
	bool bShouldPlace = GetEquipPlacementAndHoldLocation(sourceItem, targetLoc, &placement_id, &child_location_id);

	sourceItem->SetWielderID(GetID());
	sourceItem->SetWieldedLocation(targetLoc);
	sourceItem->ReleaseFromBlock();

	//store the current motion here for use later.
	uint32_t oldmotion = get_minterp()->InqStyle();

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
		sourceItem->NotifyIntStatUpdated(PARENT_LOCATION_INT, false);
		sourceItem->NotifyIntStatUpdated(PLACEMENT_POSITION_INT, false);
	}

	if (sourceItem->AsClothing() && m_bWorldIsAware)
	{
		if (m_Qualities.GetInt(HERITAGE_GROUP_INT, 1) != Gearknight_HeritageGroup) // TODO: Update JUST cloak on gearknight equip rather than whole model.
			UpdateModel();
	}

	if (!sourceItem->m_Qualities.GetInt(LIFESPAN_INT, 0))
	{
		// Remove all loot items with wcid > g_pConfig->WcidForPurge()
		if (g_pConfig->InventoryPurgeOnLogin())
		{
			if (sourceItem->m_Qualities.id > g_pConfig->WcidForPurge() && sourceItem->m_Qualities.GetInt(ITEM_WORKMANSHIP_INT, 0))
			{
				sourceItem->_timeToRot = Timer::cur_time;
				sourceItem->_beganRot = true;
			}
		}
		else
		{
			sourceItem->m_Qualities.RemoveFloat(TIME_TO_ROT_FLOAT);
			sourceItem->_timeToRot = -1.0;
			sourceItem->_beganRot = false;
		}
	}

	// Checks the old motion vs Motion_NonCombat to prevent getting stuck while adjusting to the new combat style.
	if (oldmotion != Motion_NonCombat)
		AdjustToNewCombatMode();

	// apply enchantments
	if (sourceItem->m_Qualities._spell_book && _IsPlayer())
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
			uint32_t skillLevel = 0;
			if (!m_Qualities.InqSkill(ARCANE_LORE_SKILL, skillLevel, FALSE) || (int)skillLevel < difficulty)
			{
				bShouldCast = false;

				NotifyWeenieError(WERROR_ACTIVATION_ARCANE_LORE_TOO_LOW);
			}
		}

		if (bShouldCast)
		{
			difficulty = 0;
			uint32_t skillActivationTypeDID = 0;


			if (sourceItem->m_Qualities.InqInt(ITEM_SKILL_LEVEL_LIMIT_INT, difficulty, TRUE, FALSE) && sourceItem->m_Qualities.InqDataID(ITEM_SKILL_LIMIT_DID, skillActivationTypeDID))
			{
				STypeSkill skillActivationType = (STypeSkill)skillActivationTypeDID;

				uint32_t skillLevel = 0;
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
			uint32_t serial = 0;

			serial |= ((uint32_t)GetEnchantmentSerialByteForMask(sourceItem->InqIntQuality(LOCATIONS_INT, 0, TRUE)) << (uint32_t)0);
			serial |= ((uint32_t)GetEnchantmentSerialByteForMask(sourceItem->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE)) << (uint32_t)8);

			uint32_t procspellid = sourceItem->InqDIDQuality(PROC_SPELL_DID, 0);
			uint32_t spellid = sourceItem->InqDIDQuality(SPELL_DID, 0);
			for (auto &spellPage : sourceItem->m_Qualities._spell_book->_spellbook)
			{
				//ignore the spell and don't cast if it's the Proc_spell_DID or the spell_DID
				if (spellid != spellPage.first && procspellid != spellPage.first)
					sourceItem->MakeSpellcastingManager()->CastSpellEquipped(GetID(), spellPage.first, (WORD)serial);
			}
		}
	}

	int setSpell;
	if (AsPlayer() && sourceItem->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, setSpell))
	{
		AsPlayer()->UpdateSetSpells(setSpell, sourceItem->GetID());

		if (sourceItem->IsAetheria())
		{
			if (AsPlayer())
			{
				AsPlayer()->UpdateSigilProcRate();
			}
		}
	}

	int gearRating;
	for (int i = 370; i < 380; i++)
	{
		if (sourceItem->m_Qualities.InqInt((STypeInt)i, gearRating))
		{
			UpdateRatingFromGear((STypeInt)i, gearRating);
		}
	}

	// Damage Rating for Weapon Mastery. Does not apply to quest weapons and loot without WEAPON_TYPE_INT
	if (sourceItem->m_Qualities.GetInt(WEAPON_TYPE_INT, 0))
	{
		int ratingVal;
		if (m_Qualities.InqInt(DAMAGE_RATING_INT, ratingVal, true))
		{
			m_Qualities.SetInt(DAMAGE_RATING_INT, ratingVal + 5);
			NotifyIntStatUpdated(DAMAGE_RATING_INT);
		}
	}
	sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, GetID());
	
	if (isPickup)
		sourceItem->OnPickedUp(this);
	sourceItem->OnWield(this);
	return true;
}


void CMonsterWeenie::UpdateRatingFromGear(STypeInt rating, int gearRating)
{
	m_Qualities.AdjInt(rating, gearRating);
	NotifyIntStatUpdated(rating);
}

bool CMonsterWeenie::MergeItem(uint32_t sourceItemId, uint32_t targetItemId, uint32_t amountToTransfer, bool animationDone)
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

	uint32_t freezer = sourceItem->InqIIDQuality(FREEZER_IID, 0);
	if (freezer != 0 && freezer != GetID())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_FROZEN);
		return false;
	}

	if (!animationDone && (!FindContainedItem(sourceItemId) || !FindContainedItem(targetItemId)))
	{
		sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, GetID());
		//from ground or other container or to ground or other container.
		CStackMergeInventoryUseEvent *mergeEvent = new CStackMergeInventoryUseEvent();
		if (sourceItem->IsContained() || sourceItem->IsWielded())
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

		uint32_t sourceCurrentAmount = sourceItem->InqIntQuality(STACK_SIZE_INT, 1);
		if (amountToTransfer > sourceCurrentAmount)
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_NONE);
			return false;
		}

		uint32_t targetCurrentAmount = targetItem->InqIntQuality(STACK_SIZE_INT, 1);
		uint32_t targetMaxStackSize = targetItem->InqIntQuality(MAX_STACK_SIZE_INT, 1);

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

		uint32_t sourceItemNewStackSize = sourceCurrentAmount - amountToTransfer;
		if (sourceItemNewStackSize > 0) //partial stack movement
		{
			//the client seems to forget about the source item when merging using the "place selected object in inventory" keybind
			//to split from a stack so we have to remind it. I don't know how we can determine when the client uses the keybind or dragging
			//so just do it regarless.
			MakeAware(sourceItem, true);
			sourceItem->SetStackSize(sourceItemNewStackSize);
		}
		else //full stack movement
		{
			sourceItem->Remove();
		}
		sourceItem->m_Qualities.SetInstanceID(FREEZER_IID, 0);

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
			RecalculateCoinAmount(W_COINSTACK_CLASS);

	}

	return true;
}

bool CMonsterWeenie::SplitItemToContainer(uint32_t sourceItemId, uint32_t targetContainerId, uint32_t targetSlot, uint32_t amountToTransfer, bool animationDone)
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

	if (sourceItem->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0) < 1)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
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
		MakeAware(newStackItem, true);

		//and set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);
	}

	return true;
}

bool CMonsterWeenie::SplitItemto3D(uint32_t sourceItemId, uint32_t amountToTransfer, bool animationDone)
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

	if (sourceItem->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0) < 1)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return false;
	}

	if (sourceItem->IsAttunedOrContainsAttuned())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
		return false;
	}

	//// Temporarily disallow dropping stackable items.
	//if (sourceItem->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0) > 1)
	//{
	//	NotifyInventoryFailedEvent(sourceItemId, WERROR_ATTUNED_ITEM);
	//	return false;
	//}

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
		MakeAware(newStackItem, true);

		// set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);
		if (newStackItem->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0) > 1)
			newStackItem->m_Qualities.SetInstanceID(OWNER_IID, GetID());
		// and now move on to dropping it
		CDropInventoryUseEvent *dropEvent = new CDropInventoryUseEvent();
		dropEvent->_target_id = newStackItem->GetID();
		ExecuteUseEvent(dropEvent);
	}

	return true;
}

bool CMonsterWeenie::SplitItemToWield(uint32_t sourceItemId, uint32_t targetLoc, uint32_t amountToTransfer, bool animationDone)
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

	if (sourceItem->m_Qualities.GetInt(MAX_STACK_SIZE_INT, 0) < 1)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
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
		MakeAware(newStackItem, true);

		//and set original item amount to what remains.
		sourceItem->SetStackSize(totalAmount - amountToTransfer);
	}

	return true;
}

void CMonsterWeenie::GiveItem(uint32_t targetContainerId, uint32_t sourceItemId, uint32_t transferAmount)
{

	if (sourceItemId == id)
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_NONE);
		return;
	}

	if (IsExecutingEmote())
	{
		SendText(csprintf("%s is busy.", GetName().c_str()), LTT_DEFAULT);
		NotifyInventoryFailedEvent(sourceItemId, WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	if (IsBusyOrInAction())
	{
		NotifyInventoryFailedEvent(sourceItemId, WERROR_ACTIONS_LOCKED);
		return;
	}
	CWeenieObject *target = g_pWorld->FindWithinPVS(this, targetContainerId);
	CWeenieObject *sourceItem = FindContainedItem(sourceItemId);

	if (!sourceItem || !target)
	{
		if (!sourceItem)
		{
			CWeenieObject *hackItem = g_pWorld->FindObject(sourceItemId);
			if (hackItem && target)
				SERVER_ERROR << GetName() << "tried to give" << hackItem->GetName() << "to" << target->GetName();
		}

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

	CGiveEvent *giveEvent = new CGiveEvent;
	giveEvent->_target_id = targetContainerId;
	giveEvent->_source_item_id = sourceItemId;
	giveEvent->_transfer_amount = transferAmount;
	giveEvent->_max_use_distance = 1.0;
	giveEvent->_give_event = true;
	this->ExecuteUseEvent(giveEvent); //handles moving if needed and does all error checking during this process. At the completion of AnimSuccess FinishGiveItem is called.
}

void CMonsterWeenie::FinishGiveItem(CContainerWeenie *targetContainer, CWeenieObject *sourceItem, uint32_t amountToTransfer)
{
	if (amountToTransfer <= 0 || amountToTransfer >= 100000)
	{
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}


	//if (!targetContainer->Container_CanStore(sourceItem))
	//{
	//	NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
	//	return;
	//}

	CWeenieObject *targetTopLevel = targetContainer->GetWorldTopLevelOwner();

	if (targetTopLevel != sourceItem->GetWorldTopLevelOwner() && targetTopLevel && targetTopLevel->IsExecutingEmote())
	{
		SendText(csprintf("%s is busy.", targetTopLevel->GetName().c_str()), LTT_DEFAULT);
		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
		return;
	}

	//if (CPlayerWeenie *player = targetContainer->AsPlayer())
	//{
	//	if (!(player->GetCharacterOptions() & AllowGive_CharacterOption))
	//	{
	//		NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
	//		return;
	//	}
	//}

	//NOTE: A WCID may only exist in an emote table as refused or accepted (which makes sense logically). You either want it or you don't. Make up your mind.
	if (targetTopLevel->m_Qualities._emote_table && targetTopLevel->HasEmoteForID(Refuse_EmoteCategory, sourceItem->m_Qualities.id))
	{
		//DO REFUSE EMOTE STUFF
		CWeenieObject* topLevelOwner = sourceItem->GetWorldTopLevelOwner();

		if (topLevelOwner)
		{
			if (!InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE) && !topLevelOwner->InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE))
				topLevelOwner->SendText(csprintf("You allow %s to examine your %s.", targetTopLevel->GetName().c_str(), sourceItem->GetName().c_str()), LTT_DEFAULT);
			
			auto emoteCategory = targetTopLevel->m_Qualities._emote_table->_emote_table.lookup(Refuse_EmoteCategory);
			if (emoteCategory)
			{
				double dice = Random::GenFloat(0.0, 1.0);
				double lastProbability = -1.0;

				for (auto& emoteSet : *emoteCategory)
				{
					if (emoteSet.classID == sourceItem->m_Qualities.id)
					{
						if (dice >= emoteSet.probability)
							continue;

						if (lastProbability < 0.0 || lastProbability == emoteSet.probability)
						{
							targetTopLevel->MakeEmoteManager()->ExecuteEmoteSet(emoteSet, topLevelOwner->GetID());

							lastProbability = emoteSet.probability;

							//There is no point in going through the motions of transfering this object. Fail silently as we triggered what we want.
							NotifyInventoryFailedEvent(sourceItem->GetID(), 0);
							return;

						}
					}
				}
			}
		}
	}

	if (targetTopLevel->AsMonster() && targetTopLevel->AsMonster()->CanAcceptGive(sourceItem))
	{
		//DO GIVE STUFF HERE
		BOOL bWasWielded = sourceItem->IsWielded();

		int currentStackSize = sourceItem->InqIntQuality(STACK_SIZE_INT, 1, TRUE);
		if (amountToTransfer > currentStackSize)
		{
			NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
			return;
		}

		CWeenieObject* newStackItem;
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
			sourceItem->SetWeenieContainer(0);
			sourceItem->SetWielderID(0);
			sourceItem->SetWieldedLocation(INVENTORY_LOC::NONE_LOC);
		}

		if (AsPlayer() && bWasWielded)
		{
			int setSpell;
			if (sourceItem->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, setSpell))
			{
				AsPlayer()->UpdateSetSpells(setSpell, sourceItem->GetID());
			}
			if (sourceItem->IsAetheria())
			{
				AsPlayer()->UpdateSigilProcRate();
			}
		}

		// Take it out of whatever slot it's in.


		newStackItem->m_Qualities.SetInstanceID(CONTAINER_IID, targetContainer->GetID());
		newStackItem->_cachedHasOwner = true;

		CWeenieObject* topLevelOwnerObj = newStackItem->GetWorldTopLevelOwner();
		assert(topLevelOwnerObj);
		assert(topLevelOwnerObj->AsContainer());

		if (topLevelOwnerObj)
		{
			if (CContainerWeenie* topLevelOwner = topLevelOwnerObj->AsContainer())
			{
				if (newStackItem == sourceItem)
				{
					SendNetMessage(InventoryMove(sourceItem->GetID(), targetContainer->GetID(), 0, sourceItem->RequiresPackSlot() ? 1 : 0), PRIVATE_MSG, TRUE);
				}

				if (!InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE) && !topLevelOwner->InqBoolQuality(NPC_INTERACTS_SILENTLY_BOOL, FALSE))
				{
					targetContainer->EmitSound(Sound_ReceiveItem, 1.0f);

					if (amountToTransfer > 1 && !(topLevelOwner->m_Qualities._emote_table && topLevelOwner->HasEmoteForID(Refuse_EmoteCategory, newStackItem->m_Qualities.id)))
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
					RecalculateCoinAmount(W_COINSTACK_CLASS);

				topLevelOwner->OnReceiveInventoryItem(this, newStackItem, 0);
				//topLevelOwner->DebugValidate();
				//DebugValidate();
				if (AsPlayer())
					RecalculateEncumbrance();

				return;
			}
		}
		if (AsPlayer())
			RecalculateEncumbrance();
	}

	NotifyInventoryFailedEvent(sourceItem->GetID(), WERROR_GIVE_NOT_ALLOWED);
	return;
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

	if (m_MonsterAI)
		m_MonsterAI->OnTookDamage(damageData);
}

void CMonsterWeenie::UpdateDamageList(DamageEventData &damageData)
{
	if (damageData.source && damageData.outputDamageFinal > 0 && damageData.damage_type != STAMINA_DAMAGE_TYPE && damageData.damage_type != MANA_DAMAGE_TYPE)
	{
		uint32_t source = damageData.source->GetID();
		std::map<uint32_t, int>::iterator itr = m_aDamageSources.find(source);
		std::map<uint32_t, int>::iterator itr_high = m_aDamageSources.find(m_highestDamageSource);

		if (itr == m_aDamageSources.end())
		{
			itr = m_aDamageSources.emplace(std::make_pair(source, 0)).first;
		}

		uint32_t oid = 0;// InqIIDQuality(PET_OWNER_IID, 0);
		if (damageData.source && damageData.source->m_Qualities.InqInstanceID(PET_OWNER_IID, oid))
		{
			itr = m_aDamageSources.find(oid);
			if (itr == m_aDamageSources.end())
			{
				itr = m_aDamageSources.emplace(std::make_pair(oid, 0)).first;
			}
			source = oid;
		}

		//m_aDamageSources[source] += damageData.outputDamageFinal;
		itr->second += damageData.outputDamageFinal;
		m_totalDamageTaken += damageData.outputDamageFinal;

		if (m_highestDamageSource == 0 || itr->second > itr_high->second)
		{
			m_highestDamageSource = source;

			if (monster_brawl && m_MonsterAI && !(InqIntQuality(CREATURE_TYPE_INT, 0) == damageData.source->InqIntQuality(CREATURE_TYPE_INT, 0)))
			{
				m_MonsterAI->SetNewTarget(damageData.source);
				return;
			}

			if (m_MonsterAI && m_MonsterAI->_toleranceType != TolerateEverything && damageData.source->AsPlayer())  // TODO: maths to determine when to setnewtarget not just per attack as it would allow ping-ponging mobs
			{
				m_MonsterAI->m_fAggroTime = Timer::cur_time + 10.0f;
				m_MonsterAI->SetNewTarget(damageData.source);
			}
		}

	}
}

void CMonsterWeenie::OnRegen(STypeAttribute2nd currentAttrib, int newAmount)
{
	CWeenieObject::OnRegen(currentAttrib, newAmount);

	if (currentAttrib == HEALTH_ATTRIBUTE_2ND)
	{
		uint32_t maxHealth = 0;
		m_Qualities.InqAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, maxHealth, FALSE);

		if (maxHealth == newAmount)
		{
			// reset damage sources
			m_aDamageSources.clear();
			m_highestDamageSource = 0;
			m_totalDamageTaken = 0;

		}
	}
}

void CMonsterWeenie::GivePerksForKill(CWeenieObject *pKilled)
{
	// Prevent CWeenieObject::GivePerksForKill from running
}

void CMonsterWeenie::GiveXP(int64_t amount, ExperienceHandlingType flags, bool showText)
{
	if (amount <= 0)
		return;

	if (m_Qualities.GetInt(AUGMENTATION_BONUS_XP_INT, 0))
	{
		amount = uint64_t((double)amount * 1.05);
	}

	OnGivenXP(amount, flags);

	uint64_t newAvailableXP = (uint64_t)InqInt64Quality(AVAILABLE_EXPERIENCE_INT64, 0) + amount;
	uint64_t newTotalXP = (uint64_t)InqInt64Quality(TOTAL_EXPERIENCE_INT64, 0) + amount;

	int currentLevel = InqIntQuality(LEVEL_INT, 1);

	uint32_t skillCredits = 0;
	bool bLeveled = false;
	uint64_t xpToNextLevel = ExperienceSystem::ExperienceToLevel(currentLevel + 1);
	while (xpToNextLevel <= newTotalXP && currentLevel < ExperienceSystem::GetMaxLevel())
	{
		currentLevel++;
		skillCredits += ExperienceSystem::GetCreditsForLevel(currentLevel);
		bLeveled = true;

		xpToNextLevel = ExperienceSystem::ExperienceToLevel(currentLevel + 1);

		if (!xpToNextLevel)
			break;
	}
	//TODO: When luminance is introduced, move this.

	m_Qualities.SetInt64(AVAILABLE_EXPERIENCE_INT64, newAvailableXP);
	NotifyInt64StatUpdated(AVAILABLE_EXPERIENCE_INT64);

	if (currentLevel < 275) {
		m_Qualities.SetInt64(TOTAL_EXPERIENCE_INT64, newTotalXP);
		NotifyInt64StatUpdated(TOTAL_EXPERIENCE_INT64);
	}


	if (bLeveled)
	{
		GiveSkillCredit(skillCredits);

		fellowship_ptr_t fs = GetFellowship();
		if (fs)
		{
			fs->FullUpdate();
		}

		AllegianceTreeNode *node = g_pAllegianceManager->GetTreeNode(GetID());
		if (node)
			node->_level = currentLevel;
		m_Qualities.SetInt(LEVEL_INT, currentLevel);
		NotifyIntStatUpdated(LEVEL_INT);

		EmitEffect(PS_LevelUp, 1.0f);

		if (currentLevel == 275)
		{
			m_Qualities.SetInt64(TOTAL_EXPERIENCE_INT64, 191226310247);
			NotifyInt64StatUpdated(TOTAL_EXPERIENCE_INT64);
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

		SendText(notice, LTT_DEFAULT);
	}

	if (flags & ExperienceHandlingType::ShareWithAllegiance)
	{
		// let allegiance manager pass it up
		g_pAllegianceManager->HandleAllegiancePassup(GetID(), amount, true);
	}

	std::list<CWeenieObject*> equipment;
	Container_GetWieldedByMask(equipment, ALL_LOC);
	for (CWeenieObject *item : equipment)
	{
		item->GiveXP(amount, flags, false);
	}
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

bool CMonsterWeenie::IsAttackMotion(uint32_t motion)
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

void CMonsterWeenie::OnMotionDone(uint32_t motion, BOOL success)
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

						if (uint32_t use_sound_did = pItem->InqDIDQuality(USE_SOUND_DID, 0))
							EmitSound(use_sound_did, 1.0f);

						uint32_t boost_stat = pItem->InqIntQuality(BOOSTER_ENUM_INT, 0);
						uint32_t boost_value = pItem->InqIntQuality(BOOST_VALUE_INT, 0);

						switch (boost_stat)
						{
						case HEALTH_ATTRIBUTE_2ND:
						case STAMINA_ATTRIBUTE_2ND:
						case MANA_ATTRIBUTE_2ND:
						{
							STypeAttribute2nd maxStatType = (STypeAttribute2nd)(boost_stat - 1);
							STypeAttribute2nd statType = (STypeAttribute2nd)boost_stat;

							uint32_t statValue = 0, maxStatValue = 0;
							m_Qualities.InqAttribute2nd(statType, statValue, FALSE);
							m_Qualities.InqAttribute2nd(maxStatType, maxStatValue, FALSE);

							uint32_t newStatValue = min(statValue + boost_value, maxStatValue);

							int statChange = newStatValue - statValue;
							if (statChange)
							{
								if (statType == HEALTH_ATTRIBUTE_2ND)
								{
									AdjustHealth(statChange);
									NotifyAttribute2ndStatUpdated(statType);
								}
								else
								{
									m_Qualities.SetAttribute2nd(statType, newStatValue);
									NotifyAttribute2ndStatUpdated(statType);
								}
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
	CCorpseWeenie *pCorpse = (CCorpseWeenie *)g_pWeenieFactory->CreateWeenieByClassID(W_CORPSE_CLASS);

	if (m_Qualities.GetBool(TREASURE_CORPSE_BOOL, false))
	{
		pCorpse->m_Qualities.SetDataID(SETUP_DID, 33558212);
		pCorpse->m_Qualities.SetDataID(MOTION_TABLE_DID, 150995355);
		pCorpse->m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, 0.4);
		pCorpse->SetName(csprintf("Treasure of %s", GetName().c_str()));
	}
	else
	{
		pCorpse->CopyDIDStat(SETUP_DID, this);
		pCorpse->CopyFloatStat(DEFAULT_SCALE_FLOAT, this);
		pCorpse->SetName(csprintf("Corpse of %s", GetName().c_str()));
		pCorpse->CopyFloatStat(TRANSLUCENCY_FLOAT, this);
		pCorpse->CopyDIDStat(MOTION_TABLE_DID, this);
	}

	// pCorpse->CopyDIDStat(SOUND_TABLE_DID, this);
	// pCorpse->CopyDIDStat(PHYSICS_EFFECT_TABLE_DID, this);

	pCorpse->CopyIntStat(LEVEL_INT, this); //copy the level so the treasure generator can have access to that value.

	ObjDesc desc;
	GetObjDesc(desc);
	pCorpse->SetObjDesc(desc);

	auto initpos = m_Position;
	pCorpse->SetInitialPosition(m_Position);
	pCorpse->InitPhysicsObj();
	//set velocity so that corpses are affected by gravity.
	pCorpse->set_velocity(_phys_obj->m_velocityVector, 0);

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
	{
		SERVER_ERROR << "Unable to create corpse of" << GetName().c_str() << "at Landblock:" << csprintf("0x%08X", m_Position.objcell_id) <<
			"(X, Y, Z):" << m_Position.frame.m_origin.x << "," << m_Position.frame.m_origin.y << m_Position.frame.m_origin.z;
		pCorpse = NULL;
	}

	m_DeathKillerIDForCorpse = 0;
	m_DeathKillerNameForCorpse.clear();

	return pCorpse;
}

void CMonsterWeenie::DropAllLoot(CCorpseWeenie *pCorpse)
{
}

void CMonsterWeenie::GenerateDeathLoot(CCorpseWeenie *pCorpse)
{
	if (uint32_t deathTreasureType = InqDIDQuality(DEATH_TREASURE_TYPE_DID, 0))
		g_pWeenieFactory->GenerateFromTypeOrWcid(pCorpse, DestinationType::ContainTreasure_DestinationType, deathTreasureType);

	if (m_Qualities._create_list)
		g_pWeenieFactory->AddFromCreateList(pCorpse, m_Qualities._create_list, (DestinationType)(Contain_DestinationType | Treasure_DestinationType));

	std::list<CWeenieObject *> removeList;

	for (auto item : pCorpse->m_Items)
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

	CWeenieObject *killer = g_pWorld->FindObject(m_DeathKillerIDForCorpse, false);

	if (!_IsPlayer())
	{
		MarkForDestroy();
	}

	// create corpse
	CPlayerWeenie *player = AsPlayer();
	if (!player && !m_Qualities.GetBool(NO_CORPSE_BOOL, false))
	{
		CCorpseWeenie *pCorpse = CreateCorpse();

		if (pCorpse)
			GenerateDeathLoot(pCorpse);

		//if (pCorpse && m_bIsRareEligible && !g_pTreasureFactory->_TreasureProfile->rareTiers.empty() && g_pConfig->RareDropMultiplier() > 0.0)
		//{
		//	if (killer && killer->_IsPlayer())
		//		g_pWeenieFactory->GenerateRareItem(pCorpse, killer);
		//}
	}

}

void CMonsterWeenie::OnDeath(uint32_t killer_id)
{
	if (m_aDamageSources.empty()) // not sure this should ever happen
	{
		m_highestDamageSource = killer_id;
	}

	CWeenieObject::OnDeath(m_highestDamageSource);

	//CheckRareEligible(g_pWorld->FindObject(m_highestDamageSource, false));

	int level = InqIntQuality(LEVEL_INT, 0);

	int baseXPForKill = 0;
	int baseLumForKill = 0;
	int64_t xpForKill = 0;
	int64_t lumForKill = 0;

	if (level <= 0)
		baseXPForKill = 0;
	else if (!m_Qualities.InqInt(XP_OVERRIDE_INT, baseXPForKill, 0, FALSE))
		baseXPForKill = (uint32_t)GetXPForKillLevel(level);

	xpForKill = (int64_t)(baseXPForKill * g_pConfig->GetKillXPMultiplier(level));

	if (level <= 0)
		lumForKill = 0;
	else if (m_Qualities.InqInt(LUMINANCE_AWARD_INT, baseLumForKill, 0, FALSE))
		lumForKill = (int64_t)(baseLumForKill * g_pConfig->GetLumXPMultiplier());

	if (xpForKill > 0)
	{
		// hand out xp proportionally
		for (auto it = m_aDamageSources.begin(); it != m_aDamageSources.end(); ++it)
		{
			CWeenieObject *pSource = g_pWorld->FindObject(it->first);
			//uint32_t oid = 0;// InqIIDQuality(PET_OWNER_IID, 0);
			//if (pSource->m_Qualities.InqInstanceID(PET_OWNER_IID, oid))
			//{
			//	pSource = g_pWorld->FindObject(oid);
			//}

			double dPercentage = (double)it->second / m_totalDamageTaken;

			if (pSource)
			{
				pSource->GiveSharedXP(dPercentage * xpForKill, false);

				if (lumForKill > 0)
					pSource->GiveSharedLum(dPercentage * lumForKill, false);
			}
		}
	}

	m_DeathKillerIDForCorpse = m_highestDamageSource;
	if (!g_pWorld->FindObjectName(m_highestDamageSource, m_DeathKillerNameForCorpse))
		m_DeathKillerNameForCorpse = "fate";

	if (m_Qualities._generator_registry && m_Qualities.GetInt(GENERATOR_DESTRUCTION_TYPE_INT, 0) && m_Qualities.GetInt(GENERATOR_DESTRUCTION_TYPE_INT, 0) != 1)
	{
		for (auto entry : m_Qualities._generator_registry->_registry)
		{
			CWeenieObject *weenie = g_pWorld->FindObject(entry.second.m_objectId);
			if (weenie->AsMonster() != nullptr)
				weenie->AsMonster()->OnDeath(0);
		}

	}

	MakeMovementManager(TRUE);
	StopCompletely(0);

	bool bHardcoreDeath = false;

	if (g_pConfig->HardcoreMode() && _IsPlayer())
	{
		if (CWeenieObject *pKiller = g_pWorld->FindObject(m_highestDamageSource))
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
	COMBAT_MODE newCombatMode = (COMBAT_MODE)InqIntQuality(COMBAT_MODE_INT, COMBAT_MODE::NONCOMBAT_COMBAT_MODE, TRUE);
	uint32_t new_motion_style = get_minterp()->InqStyle();

	//if (_IsPlayer())
	//{
		switch (mode)
		{
		case NONCOMBAT_COMBAT_MODE:
			new_motion_style = Motion_NonCombat;
			newCombatMode = COMBAT_MODE::NONCOMBAT_COMBAT_MODE;
			break;

		case MELEE_COMBAT_MODE:
		{
			CWeenieObject *weapon = GetWieldedMelee();
			CWeenieObject *shield = GetWieldedShield();

			if (!weapon)
				weapon = GetWieldedTwoHanded();

			CombatStyle default_combat_style = weapon ? (CombatStyle)weapon->InqIntQuality(DEFAULT_COMBAT_STYLE_INT, Undef_CombatStyle) : Undef_CombatStyle;

			switch (default_combat_style)
			{
			case Undef_CombatStyle:
			case Unarmed_CombatStyle:
				new_motion_style = Motion_HandCombat;

				if (shield && shield->InqIntQuality(LOCATIONS_INT, 0, TRUE) != SHIELD_LOC)
					new_motion_style = Motion_DualWieldCombat;

				newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;
				break;

			case OneHanded_CombatStyle:
			{
				new_motion_style = Motion_SwordCombat;

				if (shield)
				{
					if (shield->InqIntQuality(LOCATIONS_INT, 0, TRUE) != SHIELD_LOC)
						new_motion_style = Motion_DualWieldCombat;
					else
						new_motion_style = Motion_SwordShieldCombat;
				}

				newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;
				break;
			}

			case TwoHanded_CombatStyle:
				new_motion_style = Motion_2HandedSwordCombat;
				newCombatMode = COMBAT_MODE::MELEE_COMBAT_MODE;
				break;
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
	//}

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

uint32_t CMonsterWeenie::DoForcedUseMotion(MotionUseType useType, uint32_t motion, uint32_t target, uint32_t childID, uint32_t childLoc, MovementParameters *params)
{
	m_MotionUseData.m_MotionUseType = useType;
	m_MotionUseData.m_MotionUseMotionID = motion;
	m_MotionUseData.m_MotionUseTarget = target;
	m_MotionUseData.m_MotionUseChildID = childID;
	m_MotionUseData.m_MotionUseChildLocation = childLoc;
	return DoForcedMotion(motion, params);
}

//thanks lime!
bool CMonsterWeenie::ClothingPrioritySorter(const CWeenieObject *first, const CWeenieObject *second)
{
	if (((CWeenieObject *)first)->InqBoolQuality(TOP_LAYER_PRIORITY_BOOL, 0))
		return FALSE;
	else if (((CWeenieObject *)second)->InqBoolQuality(TOP_LAYER_PRIORITY_BOOL, 0))
		return TRUE;
	return ((CWeenieObject *)first)->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE) < ((CWeenieObject *)second)->InqIntQuality(CLOTHING_PRIORITY_INT, 0, TRUE);
}

void CMonsterWeenie::GetObjDesc(ObjDesc &objDesc)
{
	std::list<CWeenieObject *> wieldedWearable;
	Container_GetWieldedByMask(wieldedWearable, ARMOR_LOC | CLOTHING_LOC);
	for (auto wearable : wieldedWearable)
	{
		if (wearable->IsAvatarJumpsuit())
		{
			objDesc = wearable->m_WornObjDesc;
			return;
		}
	}

	CWeenieObject::GetObjDesc(objDesc);


	uint32_t head_object_id = 0;

	if (m_Qualities.InqDataID(HEAD_OBJECT_DID, head_object_id))
		objDesc.AddAnimPartChange(new AnimPartChange(16, head_object_id));

	uint32_t setup_id = 0;
	m_Qualities.InqDataID(SETUP_DID, setup_id);

	// some heritage groups have alternate setups (undead)
	// we need to get the base gender setup or clothing tables won't work
	int group = 1;
	int gender = 1;
	if (m_Qualities.InqInt(HERITAGE_GROUP_INT, group) && m_Qualities.InqInt(GENDER_INT, gender))
	{
		HeritageGroup_CG *heritage = CachedCharGenData->mHeritageGroupList.lookup(group);
		Sex_CG *sex = heritage->mGenderList.lookup(gender);

		//objDesc += sex->objDesc;

		HairStyle_CG *hair = nullptr;

		//if (m_Qualities.InqDataID(HEAD_OBJECT_DID, head_object_id))
		//	hair = sex->FindHairStyle(head_object_id);
		hair = sex->FindHairStyleAltSetup(setup_id);

		if (hair)
		{
			//objDesc += hair->objDesc;

			//if (hair->alternateSetup == setup_id)
			setup_id = sex->setup;
		}
	}

	uint32_t old_eye_texture_id, new_eye_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_EYES_TEXTURE_DID, old_eye_texture_id) && m_Qualities.InqDataID(EYES_TEXTURE_DID, new_eye_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_eye_texture_id, new_eye_texture_id));

	uint32_t old_nose_texture_id, new_nose_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_NOSE_TEXTURE_DID, old_nose_texture_id) && m_Qualities.InqDataID(NOSE_TEXTURE_DID, new_nose_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_nose_texture_id, new_nose_texture_id));

	uint32_t old_mouth_texture_id, new_mouth_texture_id;
	if (m_Qualities.InqDataID(DEFAULT_MOUTH_TEXTURE_DID, old_mouth_texture_id) && m_Qualities.InqDataID(MOUTH_TEXTURE_DID, new_mouth_texture_id))
		objDesc.AddTextureMapChange(new TextureMapChange(16, old_mouth_texture_id, new_mouth_texture_id));

	uint32_t skin_palette_id;
	if (m_Qualities.InqDataID(SKIN_PALETTE_DID, skin_palette_id))
		objDesc.AddSubpalette(new Subpalette(skin_palette_id, 0 << 3, 0x18 << 3));

	uint32_t hair_palette_id;
	if (m_Qualities.InqDataID(HAIR_PALETTE_DID, hair_palette_id))
		objDesc.AddSubpalette(new Subpalette(hair_palette_id, 0x18 << 3, 0x8 << 3));

	uint32_t eye_palette_id;
	if (m_Qualities.InqDataID(EYES_PALETTE_DID, eye_palette_id))
		objDesc.AddSubpalette(new Subpalette(eye_palette_id, 0x20 << 3, 0x8 << 3));

	std::list<CWeenieObject *> wieldedArmors;
	Container_GetWieldedByMask(wieldedArmors, CLOTHING_LOC | ARMOR_LOC);
	wieldedArmors.sort(ClothingPrioritySorter);

	for (auto armor : wieldedArmors)
	{
		if (armor->IsHelm())
		{
			if (!ShowHelm())
				continue;
		}

		if (armor->IsCloak())
		{
			if (!ShowCloak())
				continue;
		}


		uint32_t clothing_table_id = armor->InqDIDQuality(CLOTHINGBASE_DID, 0);
		uint32_t palette_template_key = armor->InqIntQuality(PALETTE_TEMPLATE_INT, 0);

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

				if (clothingTable->BuildObjDesc(setup_id, palette_template_key, &shades, &od))
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

uint32_t CMonsterWeenie::OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot)
{
	if (source != this)
	{
		if (!InqBoolQuality(AI_ACCEPT_EVERYTHING_BOOL, false))
		{
			// Use our emote table to determine what to do with this item.
			if (m_Qualities._emote_table)
			{
				PackableList<EmoteSet>* emoteCategory = m_Qualities._emote_table->_emote_table.lookup(Give_EmoteCategory);

				if (emoteCategory)
				{
					double dice = Random::GenFloat(0.0, 1.0);
					double lastProbability = 1.1; // initialize to ensure the first check sets correctly
					EmoteSet lastEmoteSet;

					// first scan all the probability entries and find the one that is just higher than the dice roll
					// assumes the entries are not increasing in values, can be in any order
					for (auto& emoteSet : *emoteCategory)
					{
						if (emoteSet.classID == item->m_Qualities.id)
						{
							if (dice >= emoteSet.probability)  // ignore any that are less than the dice roll
								continue;
							else
								if (emoteSet.probability <= lastProbability) // ignore any that are greater than the current pick
								{
									lastProbability = emoteSet.probability;
									lastEmoteSet = emoteSet;
								}
						}
					}
					if (lastProbability <= 1.0)  // double check a probability was found
					{
						if (item->InqIntQuality(STACK_SIZE_INT, 1) > 1)
						{
							item->DecrementStackNum(1, false);
							SimulateGiveObject(source->AsContainer(), item);
						}
						else
							g_pWorld->RemoveEntity(item);

						MakeEmoteManager()->ExecuteEmoteSet(lastEmoteSet, source->GetID());
						return 0;
					}
				}
			}
		}

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
	double defenseMod = 1.0;

	// we don't have an attack manager unless we are in active combat
	//if (m_AttackManager)
	//{
	//	defenseMod *= (double)m_AttackManager->GetDefenseMod();
	//}

	CWeenieObject *main = GetWieldedCombat(COMBAT_USE_MELEE);
	CWeenieObject *left = GetWieldedCombat(COMBAT_USE_OFFHAND);
	CWeenieObject *bow = GetWieldedCombat(COMBAT_USE_MISSILE);
	CWeenieObject *both = GetWieldedCombat(COMBAT_USE_TWO_HANDED);
	CWeenieObject *caster = GetWieldedCaster();

	if (caster)
		defenseMod = caster->GetMeleeDefenseMod();
	else if (bow)
		defenseMod = bow->GetMeleeDefenseMod();
	else if (both)
		defenseMod = both->GetMeleeDefenseMod();
	else
	{
		// Calling the weapon::GetMeleeDefenseMod will include buffs, as will calling the base
		// Here we check wether or not the main hand is empty, using
		// the base defense (probably 1 + buffs) instead
		defenseMod = main ? main->GetMeleeDefenseMod() : CWeenieObject::GetMeleeDefenseMod();
		if (left)
			defenseMod = max(defenseMod, left->GetMeleeDefenseMod());
	}

	std::list<CWeenieObject *> wielded;
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

	/*	Container_GetWieldedByMask(wielded, ARMOR_LOC);
		for (auto item : m_Wielded) //check all armor for appropriate imbue effects - commented out as this is checked in TryMagicResist and should be adding 1 to skill, not 1%
		{
			if (item->GetImbueEffects() & MagicDefense_ImbuedEffectType)
				defenseMod += 0.01;
		}*/

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

int CMonsterWeenie::GetAttackDamage(bool isAssess)
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

	return CWeenieObject::GetAttackDamage(isAssess);
}

float CMonsterWeenie::GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor)
{
	std::list<CWeenieObject *> wielded;
	Container_GetWieldedByMask(wielded, ARMOR_LOC | CLOTHING_LOC | SHIELD_LOC);

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
		return buffDetails.rawValue; //Take the Raw value for Hollows. Debuffs should not count.
	else
		return buffDetails.enchantedValue;
}

bool CMonsterWeenie::TryMeleeAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion)
{
	if (IsDead())
	{
		NotifyAttackDone(WERROR_DEAD);
		return true;
	}

	if (!IsAttacking() && IsBusyOrInAction() || (_blockNewAttacksUntil != -1 && _blockNewAttacksUntil > Timer::cur_time))
	{
		NotifyAttackDone(WERROR_ACTIONS_LOCKED);
		return true;
	}

	if (AsPlayer())
	{
		double last_attack_time = 0.0f;
		if (AsPlayer()->m_Qualities.InqFloat(ATTACK_TIMESTAMP_FLOAT, last_attack_time))
		{
			if (last_attack_time > 0.0f) // player has attacked since last login
			{
				const auto current_time = Timer::cur_time;
				if (last_attack_time > current_time)
				{
					NotifyAttackDone(WERROR_BAD_PARAM);
					return true;
				}


				auto dual_wield_factor = 1.0;
				if (get_minterp()->InqStyle() == Motion_DualWieldCombat)
					dual_wield_factor = .8f;
				const auto attack_delta = ((current_time - last_attack_time) * dual_wield_factor) + .001; // floating point fudge factor
				if (power > attack_delta)
				{
					NotifyAttackDone(WERROR_BAD_PARAM);
					return false;
				}

			}
		}
	}

	if (!m_AttackManager)
	{
		m_AttackManager = new AttackManager(this);
	}

	// duplicate attack
	m_AttackManager->BeginMeleeAttack(target_id, height, power, m_MonsterAI ? m_MonsterAI->GetChaseDistance() : 15.0f, motion);

	m_LastAttackHeight = height;
	m_LastAttackPower = power;
	m_LastAttackTarget = target_id;

	// ensure there's no heartbeat animation
	_last_update_pos = Timer::cur_time;
	return true;
}

void CMonsterWeenie::TryMissileAttack(uint32_t target_id, ATTACK_HEIGHT height, float power, uint32_t motion)
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

	if (AsPlayer())
	{
		double last_attack_time = 0.0f;
		if (AsPlayer()->m_Qualities.InqFloat(ATTACK_TIMESTAMP_FLOAT, last_attack_time))
		{
			if (last_attack_time > 0.0f) // player has attacked since last login
			{
				std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
				auto duration = now.time_since_epoch();
				auto current_time = (double)std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 100;
				if (last_attack_time > current_time)
				{
					NotifyAttackDone(WERROR_BAD_PARAM);
					return;
				}

				const auto attack_delta = (current_time - last_attack_time) + .001;;
				if (power > attack_delta)
				{
					NotifyAttackDone(WERROR_BAD_PARAM);
					return;
				}
			}
		}
	}

	if (!m_AttackManager)
		m_AttackManager = new AttackManager(this);

	m_AttackManager->BeginMissileAttack(target_id, height, power, motion);

	m_LastAttackHeight = height;
	m_LastAttackPower = power;
	m_LastAttackTarget = target_id;

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

int CMonsterWeenie::AdjustHealth(int amount, bool useRatings)
{
	int adjustedAmount = CWeenieObject::AdjustHealth(amount, useRatings);

	uint32_t maxHealth = 0;
	m_Qualities.InqAttribute2nd(MAX_HEALTH_ATTRIBUTE_2ND, maxHealth, FALSE);
	uint32_t currentHealth = 0;
	m_Qualities.InqAttribute2nd(HEALTH_ATTRIBUTE_2ND, currentHealth, FALSE);

	if (maxHealth == currentHealth)
	{
		// reset damage sources
		m_aDamageSources.clear();
		m_highestDamageSource = 0;
		m_totalDamageTaken = 0;

	}

	return adjustedAmount;
}

bool CMonsterWeenie::CheckRareEligible(CWeenieObject *highestDamageDealer)
{
	if (!highestDamageDealer)
		return false;

	if (highestDamageDealer)
	{
		if (highestDamageDealer->_IsPlayer() && (m_Qualities.GetInt(LEVEL_INT, 0) >= highestDamageDealer->m_Qualities.GetInt(LEVEL_INT, 0)) || (m_Qualities.GetInt(LEVEL_INT, 0) >= 100))
			if (m_Qualities.GetDID(DEATH_TREASURE_TYPE_DID, 0))
			{
				m_bIsRareEligible = true;
				return true;
			}
	}

	return false;
}

void CMonsterWeenie::SetDisplayCombatDamage(bool show)
{
	m_bGiveCombatData = show;
}

bool CMonsterWeenie::ShowCombatDamage()
{
	return m_bGiveCombatData;
}

bool CMonsterWeenie::CanAcceptGive(CWeenieObject* item)
{
	if (AsPlayer())
	{
		if (AsPlayer()->GetCharacterOptions() & AllowGive_CharacterOption)
			return Container_CanStore(item);
	}

	if (InqBoolQuality(ALLOW_GIVE_BOOL, false))
	{
		if (InqBoolQuality(AI_ACCEPT_EVERYTHING_BOOL, false))
			return true;

		if (HasEmoteForID(Give_EmoteCategory, item->m_Qualities.id))
			return true;
	}

	return false;
}

int CMonsterWeenie::GetAetheriaSetCount(int setid)
{
	int retVal = 0;

	auto sigilBlue = GetWielded(SIGIL_ONE_LOC);
	if (sigilBlue)
	{
		int sigilBlueSet;
		if (sigilBlue->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, sigilBlueSet) && sigilBlueSet == setid)
		{
			retVal += sigilBlue->GetLevel();
		}
	}

	auto sigilYellow = GetWielded(SIGIL_TWO_LOC);
	if (sigilYellow)
	{
		int sigilYellowSet;
		if (sigilYellow->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, sigilYellowSet) && sigilYellowSet == setid)
		{
			retVal += sigilYellow->GetLevel();
		}
	}

	auto sigilRed = GetWielded(SIGIL_THREE_LOC);
	if (sigilRed)
	{
		int sigilRedSet;
		if (sigilRed->m_Qualities.InqInt(EQUIPMENT_SET_ID_INT, sigilRedSet) && sigilRedSet == setid)
		{
			retVal += sigilRed->GetLevel();
		}
	}

	return retVal;
}
