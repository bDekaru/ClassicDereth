
#include <StdAfx.h>
#include "Gem.h"
#include "UseManager.h"
#include "Player.h"
#include "SpellcastingManager.h"
#include "World.h"
#include "WeenieFactory.h"

CGemWeenie::CGemWeenie()
{
}

CGemWeenie::~CGemWeenie()
{
}

int CGemWeenie::Use(CPlayerWeenie *player)
{
	if (!player->FindContainedItem(GetID()))
	{
		player->NotifyUseDone(WERROR_NONE);
		return WERROR_NONE;
	}

	if (player->IsInPortalSpace())
	{
		player->NotifyUseDone(WERROR_ACTIONS_LOCKED);
		return WERROR_NONE;
	}

	if (InqDIDQuality(SPELL_DID, 0) && _nextUse > Timer::cur_time) // If this gem casts a spell and the _nextUse has not elapsed yet then cancel the use.
	{
		player->NotifyUseDone(WERROR_NONE);
		player->SendText("You can't do that yet!", LTT_ERROR);
		return WERROR_NONE;
	}

	if (InqBoolQuality(RARE_USES_TIMER_BOOL, 0) && player->_nextRareUse > Timer::cur_time) // If this is a rare gem and the _nextRareUse has not elapsed yet then cancel the use.
	{
		player->NotifyUseDone(WERROR_NONE);
		player->SendText("You can't do that yet!", LTT_ERROR); // TODO: is this the correct text? Was it sent to your window? Did it notify you how much time you had left?
		return WERROR_NONE;
	}

	CGenericUseEvent *useEvent = new CGenericUseEvent;
	useEvent->_target_id = GetID();
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

int CGemWeenie::DoUseResponse(CWeenieObject *player)
{
	if (InqIntQuality(ITEM_TYPE_INT, 0) == TYPE_FOOD)
	{
		player->DoForcedMotion(Motion_Eat);
		player->DoForcedMotion(Motion_Ready);
	}

	if (uint32_t spell_did = InqDIDQuality(SPELL_DID, 0))
	{
		MakeSpellcastingManager()->CastSpellInstant(player->GetID(), spell_did);

		// If this is a rare gem then set _nextRareUse on the player and broadcast the use locally, otherwise set the _nextUse on the gem.
		if (InqBoolQuality(RARE_USES_TIMER_BOOL, 0))
		{
			player->AsPlayer()->_nextRareUse = Timer::cur_time + 180.0;
			std::string text = csprintf("%s used the rare item %s", player->GetName().c_str(), GetName().c_str());
			if (!text.empty())
			{
				g_pWorld->BroadcastLocal(player->GetLandcell(), text);
			}
		}
		else
			_nextUse = Timer::cur_time + InqFloatQuality(COOLDOWN_DURATION_FLOAT, 0, FALSE);
	}

	DecrementStackOrStructureNum();

	return CWeenieObject::DoUseResponse(player);
}


//copied wholesale from manastone.cpp
int CGemWeenie::UseWith(CPlayerWeenie *player, CWeenieObject *with)
{
	CGemUseEvent *useEvent = new CGemUseEvent;
	useEvent->_target_id = with->GetID();
	useEvent->_tool_id = GetID();
	useEvent->_max_use_distance = 0.0;
	player->ExecuteUseEvent(useEvent);

	return WERROR_NONE;
}

//copied wholesale from manastone.cpp
void CGemUseEvent::OnReadyToUse()
{

	CWeenieObject* target = GetTarget();
	if (!target && _target_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	CWeenieObject *tool = GetTool();
	if (!tool && _tool_id)
	{
		Cancel(WERROR_OBJECT_GONE);
		return;
	}

	int error = WERROR_NONE;

	if (target)
	{
		error = tool->DoUseWithResponse(_weenie, target);
	}

	Done(error);
}

int CGemWeenie::DoUseWithResponse(CWeenieObject* source, CWeenieObject* pTarget)
{
	switch (m_Qualities.id)
	{
	case 42622://Armor Main Reduction Tool
	case 44879://Armor Lower Reduction Tool
	case 44880://Armor Middle Reduction Tool
	{
		return Tailor_ReduceArmor(source, pTarget);
	}

	case 41956://Armor Tailoring Kit
	{
		return Tailor_KitOntoArmor(source, pTarget);
	}

	case 51445://Weapon Tailoring Kit
	{
		return Tailor_KitOntoWeapon(source, pTarget);
	}

	case 42724://Armor Layering Tool(Top)
	{
		return Tailor_LayerTop(source, pTarget);
	}

	case 42726://Armor Layering Tool(Bottom)
	{
		return Tailor_LayerBot(source, pTarget);
	}

	//tked armor intermediaries
	case 42414://helm
	case 42407://gauntlets
	case 42422://boots
	case 42403://breastplate
	case 42409://girth
	case 42418://pauldrons
	case 42421://vambraces
	case 42411://tassets
	case 42416://greaves
	case 42417://lower-body multislot
	case 42405://upper-body multislot
	case 44863://clothing or shield
	{
		return Tailor_TempOntoArmor(source, pTarget);
	}

	case 51451://tked weapon
	{
		return Tailor_TempOntoWeapon(source, pTarget);
	}
	default:
	{
		//return WERROR_CRAFT_FAILED_REQUIREMENTS;
		return CWeenieObject::DoUseWithResponse(source, pTarget);
	}
	}

	//return WERROR_NONE;
}

int CGemWeenie::Tailor_ReduceArmor(CWeenieObject* source, CWeenieObject* pTarget)
{
	//if retained item->bail
	if (pTarget->m_Qualities.GetBool(RETAINED_BOOL, bool())) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	if (!source->FindContainedItem(pTarget->id)) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;
	if (pTarget->IsEquipped()) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;

	//quest item->bail
	if (!pTarget->m_Qualities.GetInt(ITEM_WORKMANSHIP_INT, int())) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	int locations = pTarget->m_Qualities.GetInt(LOCATIONS_INT, int());
	int priorities = 0;

	switch (m_Qualities.id)
	{
	case 42622://Armor Main Reduction Tool
	{
		//coat->bp //shirt->bp //cuirass->bp
		if (locations == CHEST_UPPERARM_LOWERARM_LOC || locations == CHEST_UPPERARM_LOWERARM_ABS_LOC || locations == CHEST_ABS_LOC
			|| locations == CHEST_UPPERARM_ABS_LOC || locations == CHEST_UPPERARM_LOC)
		{
			locations = CHEST_ARMOR_LOC;
			priorities = CHEST_ARMOR_CLOTHING_PRIORITY;
		}
		//sleeves->paulds
		else if (locations == UPPERARM_LOWERARM_LOC)
		{
			locations = UPPER_ARM_ARMOR_LOC;
			priorities = UPPER_ARM_ARMOR_CLOTHING_PRIORITY;
		}
		//legs->girth // shorts->girth
		else if (locations == ABS_UPPERLEG_LOWERLEG_LOC || locations == ABS_UPPERLEG_LOC)
		{
			locations = ABDOMEN_ARMOR_LOC;
			priorities = ABDOMEN_ARMOR_CLOTHING_PRIORITY;
		}
		break;
	}

	case 44879://Armor Lower Reduction Tool
	{
		//sleeves->bracers
		if (locations == UPPERARM_LOWERARM_LOC)
		{
			locations = LOWER_ARM_ARMOR_LOC;
			priorities = LOWER_ARM_ARMOR_CLOTHING_PRIORITY;
		}
		//legs->greaves //chaps->greaves
		else if (locations == ABS_UPPERLEG_LOWERLEG_LOC || locations == UPPERLEG_LOWERLEG_LOC)
		{
			locations = LOWER_LEG_ARMOR_LOC;
			priorities = LOWER_LEG_ARMOR_CLOTHING_PRIORITY;
		}
		// Viamontian Laced Boots -> boots
		else if (locations == LOWERLEG_FOOT_LOC)
		{
			locations = FOOT_WEAR_LOC;
			priorities = FOOT_WEAR_CLOTHING_PRIORITY;
		}
		break;
	}

	case 44880://Armor Middle Reduction Tool
	{
		//legs->tassets //chaps->tassets //shorts->tassets
		if (locations == ABS_UPPERLEG_LOWERLEG_LOC || locations == UPPERLEG_LOWERLEG_LOC || locations == ABS_UPPERLEG_LOC)
		{
			locations = UPPER_LEG_ARMOR_LOC;
			priorities = UPPER_LEG_ARMOR_CLOTHING_PRIORITY;
		}
		break;
	}
	}

	//no successful reduction
	if (priorities == 0) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	//successful reduction
	pTarget->m_Qualities.SetInt(LOCATIONS_INT, locations);
	pTarget->NotifyIntStatUpdated(LOCATIONS_INT, false);
	pTarget->m_Qualities.SetInt(CLOTHING_PRIORITY_INT, priorities);
	pTarget->NotifyIntStatUpdated(CLOTHING_PRIORITY_INT, false);

	//remove kit
	Remove();

	return WERROR_NONE;
}

int CGemWeenie::Tailor_KitOntoArmor(CWeenieObject* source, CWeenieObject* pTarget)
{
	if (pTarget->m_Qualities.GetBool(RETAINED_BOOL, false))
	{
		source->AsPlayer()->SendText("You must use Sandstone Salvage to remove the retained property before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	if (!source->FindContainedItem(pTarget->id)) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;
	if (pTarget->IsEquipped()) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	int locations = pTarget->m_Qualities.GetInt(LOCATIONS_INT, int());
	CWeenieObject* weenie = NULL;

	switch (locations)
	{
	case HEAD_WEAR_LOC: //42414
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42414, NULL);
		break;
	}
	case HAND_WEAR_LOC: //42407
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42407, NULL);
		break;
	}
	case FOOT_LOWERLEG_BOOTS_LOC: // Stupid boots...
	case FOOT_WEAR_LOC: //42422
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42422, NULL);
		break;
	}
	case CHEST_ARMOR_LOC: //42403
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42403, NULL);
		break;
	}
	case ABDOMEN_ARMOR_LOC: //42409
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42409, NULL);
		break;
	}
	case UPPER_ARM_ARMOR_LOC: //42418
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42418, NULL);
		break;
	}
	case LOWER_ARM_ARMOR_LOC: //42421
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42421, NULL);
		break;
	}
	case UPPER_LEG_ARMOR_LOC: //42411
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42411, NULL);
		break;
	}
	case LOWER_LEG_ARMOR_LOC: //42416
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42416, NULL);
		break;
	}

	case 6656: //SHORT_COAT_ARMOR_LOC //42405
	case 1536: //CUIRASS_ARMOR_LOC
	case 7680: //LONG_COAT_ARMOR_LOC
	case 3584: //LONG_SHIRT_ARMOR_LOC
	case 2560: //SHORT_SHIRT_ARMOR_LOC
	case 6144: //SLEEVES_ARMOR_LOC
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42405, NULL);
		break;
	}

	case 25600: //LEGGINGS_ARMOR_LOC //42417
	case 24576: //CHAPS_ARMOR_LOC
	case 9216: //SHORTS_ARMOR_SLOT
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(42417, NULL);
		break;
	}

	case CLOAK_LOC: //44863
	case SHIELD_LOC:
	case 32512: //ROBE_ARMOR_SLOT
	case 32513: //HOODED_ROBE_ARMOR_SLOT
	case 26: //FC_UPPER_UNDIES_SLOT
	case 196: //FC_LOWER_UNDIES_SLOT
	case 222: //FC_RAIMENT_UNDIES_SLOT
	{
		weenie = g_pWeenieFactory->CreateWeenieByClassID(44863, NULL);
		break;
	}
	default:
	{
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}
	}

	weenie->m_Qualities.SetString(NAME_STRING, pTarget->m_Qualities.GetString(NAME_STRING, std::string()));

	weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, pTarget->m_Qualities.GetInt(PALETTE_TEMPLATE_INT, int()));
	weenie->m_Qualities.SetInt(CLOTHING_PRIORITY_INT, pTarget->m_Qualities.GetInt(CLOTHING_PRIORITY_INT, int()));
	weenie->m_Qualities.SetInt(ENCUMB_VAL_INT, pTarget->m_Qualities.GetInt(ENCUMB_VAL_INT, int()));
	weenie->m_Qualities.SetInt(UI_EFFECTS_INT, pTarget->m_Qualities.GetInt(UI_EFFECTS_INT, int()));
	weenie->m_Qualities.SetInt(VALUE_INT, pTarget->m_Qualities.GetInt(VALUE_INT, int()));
	weenie->m_Qualities.SetInt(MATERIAL_TYPE_INT, pTarget->m_Qualities.GetInt(MATERIAL_TYPE_INT, int()));
	weenie->m_Qualities.SetInt(TARGET_TYPE_INT, pTarget->m_Qualities.GetInt(ITEM_TYPE_INT, int()));

	// clear shades
	weenie->m_Qualities.RemoveFloat(SHADE_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE2_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE3_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE4_FLOAT);

	double shade = 0;
	if (pTarget->m_Qualities.InqFloat(SHADE_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE2_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE2_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE3_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE3_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE4_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE4_FLOAT, shade);

	weenie->m_Qualities.SetDataID(SETUP_DID, pTarget->m_Qualities.GetDID(SETUP_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(PALETTE_BASE_DID, pTarget->m_Qualities.GetDID(PALETTE_BASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(CLOTHINGBASE_DID, pTarget->m_Qualities.GetDID(CLOTHINGBASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(ICON_DID, pTarget->m_Qualities.GetDID(ICON_DID, uint32_t()));

	weenie->m_ObjDescOverride.Clear();

	DecrementStackOrStructureNum();
	pTarget->Remove();

	source->AsPlayer()->SpawnInContainer(weenie);
	return WERROR_NONE;
}

int CGemWeenie::Tailor_KitOntoWeapon(CWeenieObject* source, CWeenieObject* pTarget)
{
	if (!source->FindContainedItem(pTarget->id)) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;
	if (pTarget->IsEquipped()) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	if (pTarget->m_Qualities.GetBool(RETAINED_BOOL, false))
	{
		source->AsPlayer()->SendText("You must use Sandstone Salvage to remove the retained property before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	//no fucking goblets lime godamnit
	if (pTarget->m_Qualities.GetInt(ITEM_TYPE_INT, int()) == TYPE_MISSILE_WEAPON
		&& pTarget->m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, int()) == ThrownWeapon_CombatStyle) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	CWeenieObject* weenie = NULL;

	weenie = g_pWeenieFactory->CreateWeenieByClassID(51451, NULL);

	weenie->m_Qualities.SetString(NAME_STRING, pTarget->m_Qualities.GetString(NAME_STRING, std::string()));

	switch (pTarget->m_Qualities.GetInt(ITEM_TYPE_INT, int()))
	{
	case TYPE_MELEE_WEAPON:
	{
		weenie->m_Qualities.SetInt(TARGET_TYPE_INT, TYPE_MELEE_WEAPON);
		weenie->m_Qualities.SetInt(WEAPON_TYPE_INT, pTarget->m_Qualities.GetInt(WEAPON_TYPE_INT, int()));
		break;
	}
	case TYPE_MISSILE_WEAPON:
	{
		weenie->m_Qualities.SetInt(TARGET_TYPE_INT, TYPE_MISSILE_WEAPON);
		weenie->m_Qualities.SetInt(DEFAULT_COMBAT_STYLE_INT, pTarget->m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, int()));
		break;
	}
	case TYPE_CASTER:
	{
		weenie->m_Qualities.SetInt(TARGET_TYPE_INT, TYPE_CASTER);
		break;
	}
	}

	weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, pTarget->m_Qualities.GetInt(PALETTE_TEMPLATE_INT, int()));
	weenie->m_Qualities.SetInt(ENCUMB_VAL_INT, pTarget->m_Qualities.GetInt(ENCUMB_VAL_INT, int()));
	weenie->m_Qualities.SetInt(UI_EFFECTS_INT, pTarget->m_Qualities.GetInt(UI_EFFECTS_INT, int()));
	weenie->m_Qualities.SetInt(VALUE_INT, pTarget->m_Qualities.GetInt(VALUE_INT, int()));
	weenie->m_Qualities.SetInt(DAMAGE_TYPE_INT, pTarget->m_Qualities.GetInt(DAMAGE_TYPE_INT, int()));
	weenie->m_Qualities.SetInt(MATERIAL_TYPE_INT, pTarget->m_Qualities.GetInt(MATERIAL_TYPE_INT, int()));

	// clear shades
	weenie->m_Qualities.RemoveFloat(SHADE_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE2_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE3_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE4_FLOAT);

	double shade = 0;
	if (pTarget->m_Qualities.InqFloat(SHADE_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE2_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE2_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE3_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE3_FLOAT, shade);
	if (pTarget->m_Qualities.InqFloat(SHADE4_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE4_FLOAT, shade);

	weenie->m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, pTarget->m_Qualities.GetFloat(DEFAULT_SCALE_FLOAT, float()));

	weenie->m_Qualities.SetDataID(SETUP_DID, pTarget->m_Qualities.GetDID(SETUP_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(PALETTE_BASE_DID, pTarget->m_Qualities.GetDID(PALETTE_BASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(CLOTHINGBASE_DID, pTarget->m_Qualities.GetDID(CLOTHINGBASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(ICON_DID, pTarget->m_Qualities.GetDID(ICON_DID, uint32_t()));

	DecrementStackOrStructureNum();
	pTarget->Remove();

	source->AsPlayer()->SpawnInContainer(weenie);
	return WERROR_NONE;
}

int CGemWeenie::Tailor_TempOntoArmor(CWeenieObject* source, CWeenieObject* pTarget)
{
	//if retained item->bail
	if (pTarget->m_Qualities.GetBool(RETAINED_BOOL, false))
	{
		source->AsPlayer()->SendText("You must use Sandstone Salvage to remove the retained property before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	if (!source->FindContainedItem(pTarget->id)) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;
	if (pTarget->IsEquipped())
	{
		source->AsPlayer()->SendText("You must unequip this armor before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	int shieldLoc = pTarget->m_Qualities.GetInt(LOCATIONS_INT, 0);
	//wrong coverage->bail
	if (m_Qualities.GetInt(CLOTHING_PRIORITY_INT, int()) != pTarget->m_Qualities.GetInt(CLOTHING_PRIORITY_INT, int()) && shieldLoc != 2097152) return WERROR_CRAFT_FAILED_REQUIREMENTS;

	CWeenieObject* weenie = g_pWeenieFactory->CloneWeenie(pTarget);

	weenie->m_Qualities.SetString(NAME_STRING, m_Qualities.GetString(NAME_STRING, std::string()));

	weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, m_Qualities.GetInt(PALETTE_TEMPLATE_INT, int()));
	weenie->m_Qualities.SetInt(UI_EFFECTS_INT, m_Qualities.GetInt(UI_EFFECTS_INT, int()));
	weenie->m_Qualities.SetInt(MATERIAL_TYPE_INT, m_Qualities.GetInt(MATERIAL_TYPE_INT, int()));

	// clear shades
	weenie->m_Qualities.RemoveFloat(SHADE_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE2_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE3_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE4_FLOAT);

	double shade = 0;
	if (m_Qualities.InqFloat(SHADE_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE2_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE2_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE3_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE3_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE4_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE4_FLOAT, shade);

	weenie->m_Qualities.SetDataID(SETUP_DID, m_Qualities.GetDID(SETUP_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(PALETTE_BASE_DID, m_Qualities.GetDID(PALETTE_BASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(CLOTHINGBASE_DID, m_Qualities.GetDID(CLOTHINGBASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(ICON_DID, m_Qualities.GetDID(ICON_DID, uint32_t()));

	weenie->m_ObjDescOverride.Clear();

	pTarget->Remove();
	Remove();

	source->AsPlayer()->SpawnInContainer(weenie);

	return WERROR_NONE;
}

int CGemWeenie::Tailor_TempOntoWeapon(CWeenieObject* source, CWeenieObject* pTarget)
{
	//if retained item->bail
	if (pTarget->m_Qualities.GetBool(RETAINED_BOOL, bool()))
	{
		source->AsPlayer()->SendText("You must use Sandstone Salvage to remove the retained property before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	if (!source->FindContainedItem(pTarget->id)) return WERROR_ILLEGAL_INVENTORY_TRANSACTION;
	if (pTarget->IsEquipped())
	{
		source->AsPlayer()->SendText("You must unequip this weapon before tailoring.", LTT_CRAFT);
		return WERROR_CRAFT_FAILED_REQUIREMENTS;
	}

	switch (m_Qualities.GetInt(TARGET_TYPE_INT, int()))
	{
	case TYPE_MELEE_WEAPON:
	{
		//if weap type mismatch->bail
		if (m_Qualities.GetInt(WEAPON_TYPE_INT, int()) != pTarget->m_Qualities.GetInt(WEAPON_TYPE_INT, int()))
		{
			source->AsPlayer()->SendText("Weapon types do not match.", LTT_CRAFT);
			return WERROR_CRAFT_FAILED_REQUIREMENTS;
		}

		//if ele mismatch->bail
		if (m_Qualities.GetInt(DAMAGE_TYPE_INT, int()) != pTarget->m_Qualities.GetInt(DAMAGE_TYPE_INT, int()))
		{
			source->AsPlayer()->SendText("Damage types do not match.", LTT_CRAFT);
			return WERROR_CRAFT_FAILED_REQUIREMENTS;
		}

		break;
	}
	case TYPE_MISSILE_WEAPON:
	{
		//if weap type mismatch->bail (MISSILE WEAPONS ARE WEIRD)
		if (m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, int()) != pTarget->m_Qualities.GetInt(DEFAULT_COMBAT_STYLE_INT, int()))
		{
			source->AsPlayer()->SendText("Weapon types do not match.", LTT_CRAFT);
			return WERROR_CRAFT_FAILED_REQUIREMENTS;
		}

		//if ele mismatch->bail (allow neutral to overwrite ele)
		if (m_Qualities.GetInt(DAMAGE_TYPE_INT, int()) != 0
			&& m_Qualities.GetInt(DAMAGE_TYPE_INT, int()) != pTarget->m_Qualities.GetInt(DAMAGE_TYPE_INT, int()))
		{
			source->AsPlayer()->SendText("Damage types do not match.", LTT_CRAFT);
			return WERROR_CRAFT_FAILED_REQUIREMENTS;
		}

		break;
	}
	case TYPE_CASTER:
	{
		//if ele mismatch->bail (allow neutral to overwrite ele)
		if (m_Qualities.GetInt(DAMAGE_TYPE_INT, int()) != 0
			&& m_Qualities.GetInt(DAMAGE_TYPE_INT, int()) != pTarget->m_Qualities.GetInt(DAMAGE_TYPE_INT, int()))
		{
			source->AsPlayer()->SendText("Damage types do not match.", LTT_CRAFT);
			return WERROR_CRAFT_FAILED_REQUIREMENTS;
		}

		break;
	}
	}

	CWeenieObject* weenie = g_pWeenieFactory->CloneWeenie(pTarget);

	weenie->m_Qualities.SetString(NAME_STRING, m_Qualities.GetString(NAME_STRING, std::string()));

	weenie->m_Qualities.SetInt(PALETTE_TEMPLATE_INT, m_Qualities.GetInt(PALETTE_TEMPLATE_INT, int()));
	weenie->m_Qualities.SetInt(UI_EFFECTS_INT, m_Qualities.GetInt(UI_EFFECTS_INT, int()));
	weenie->m_Qualities.SetInt(MATERIAL_TYPE_INT, m_Qualities.GetInt(MATERIAL_TYPE_INT, int()));

	// clear shades
	weenie->m_Qualities.RemoveFloat(SHADE_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE2_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE3_FLOAT);
	weenie->m_Qualities.RemoveFloat(SHADE4_FLOAT);

	double shade = 0;
	if (m_Qualities.InqFloat(SHADE_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE2_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE2_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE3_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE3_FLOAT, shade);
	if (m_Qualities.InqFloat(SHADE4_FLOAT, shade, TRUE))
		weenie->m_Qualities.SetFloat(SHADE4_FLOAT, shade);

	weenie->m_Qualities.SetFloat(DEFAULT_SCALE_FLOAT, m_Qualities.GetFloat(DEFAULT_SCALE_FLOAT, float()));

	weenie->m_Qualities.SetDataID(SETUP_DID, m_Qualities.GetDID(SETUP_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(PALETTE_BASE_DID, m_Qualities.GetDID(PALETTE_BASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(CLOTHINGBASE_DID, m_Qualities.GetDID(CLOTHINGBASE_DID, uint32_t()));
	weenie->m_Qualities.SetDataID(ICON_DID, m_Qualities.GetDID(ICON_DID, uint32_t()));

	pTarget->Remove();
	Remove();

	source->AsPlayer()->SpawnInContainer(weenie);
	return WERROR_NONE;
}

int CGemWeenie::Tailor_LayerTop(CWeenieObject* source, CWeenieObject* pTarget)
{
	pTarget->m_Qualities.SetBool(TOP_LAYER_PRIORITY_BOOL, 1);
	pTarget->NotifyBoolStatUpdated(TOP_LAYER_PRIORITY_BOOL);
	DecrementStackOrStructureNum();
	source->SendText("You modify your armor", LogTextType::LTT_CRAFT);
	return WERROR_NONE;
}

int CGemWeenie::Tailor_LayerBot(CWeenieObject* source, CWeenieObject* pTarget)
{
	pTarget->m_Qualities.SetBool(TOP_LAYER_PRIORITY_BOOL, 0);
	pTarget->NotifyBoolStatUpdated(TOP_LAYER_PRIORITY_BOOL);
	DecrementStackOrStructureNum();
	source->SendText("You modify your armor", LogTextType::LTT_CRAFT);
	return WERROR_NONE;
}

