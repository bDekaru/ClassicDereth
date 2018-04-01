
#include "StdAfx.h"
#include "Clothing.h"

CClothingWeenie::CClothingWeenie()
{
}

void CClothingWeenie::ApplyQualityOverrides()
{
}

int CClothingWeenie::Use(CPlayerWeenie *other)
{
	return CWeenieObject::Use(other);
}

bool CClothingWeenie::IsValidWieldLocation(DWORD location)
{
	if (InqIntQuality(LOCATIONS_INT, 0, TRUE) == location)
		return true;

	return false;
}

bool CClothingWeenie::CanEquipWith(CWeenieObject *other, DWORD otherLocation)
{
	if (CWeenieObject::CanEquipWith(other, otherLocation))
		return true;

	if (other->AsClothing())
	{
		// If both are clothing, some overlaps are allowed, check the priority
		if (InqIntQuality(CLOTHING_PRIORITY_INT, -1, TRUE) & other->InqIntQuality(CLOTHING_PRIORITY_INT, -1, TRUE))
			return false;

		return true;
	}

	return false;
}

bool CClothingWeenie::CoversBodyPart(BODY_PART_ENUM part, float *factor)
{
	*factor = 1.0f;
	int coverage = InqIntQuality(LOCATIONS_INT, 0, TRUE);

	switch (part)
	{
	case BP_HEAD:
		if (coverage & INVENTORY_LOC::HEAD_WEAR_LOC)
			return true;

		return false;

	case BP_CHEST:
		if (coverage & INVENTORY_LOC::CHEST_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::CHEST_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_ABDOMEN:
		if (coverage & INVENTORY_LOC::ABDOMEN_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::ABDOMEN_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_UPPER_ARM:
		if (coverage & INVENTORY_LOC::UPPER_ARM_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::UPPER_ARM_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_LOWER_ARM:
		if (coverage & INVENTORY_LOC::LOWER_ARM_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::LOWER_ARM_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_HAND:
		if (coverage & INVENTORY_LOC::HAND_WEAR_LOC)
			return true;

		return false;

	case BP_UPPER_LEG:
		if (coverage & INVENTORY_LOC::UPPER_LEG_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::UPPER_LEG_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_LOWER_LEG:
		if (coverage & INVENTORY_LOC::LOWER_LEG_ARMOR_LOC)
			return true;
		if (coverage & INVENTORY_LOC::LOWER_LEG_WEAR_LOC)
		{
			*factor = 0.5f;
			return true;
		}

		return false;

	case BP_FOOT:
		if (coverage & INVENTORY_LOC::FOOT_WEAR_LOC)
			return true;

		return false;
	}

	return false;
}

float CClothingWeenie::GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor)
{
	float factor = 1.0;

	//Shields are being created as clothing but once the server resets they are no longer clothing.
	//until we figure out why let's have this check here to make sure they pass thru the check.
	bool isShield = InqIntQuality(COMBAT_USE_INT, 0, TRUE) == COMBAT_USE::COMBAT_USE_SHIELD;
	if (!isShield && !CoversBodyPart(damageData.hitPart, &factor))
		return 0.0f;

	return CWeenieObject::GetEffectiveArmorLevel(damageData, bIgnoreMagicArmor) * factor;
}

bool CClothingWeenie::IsHelm()
{
	return (InqIntQuality(LOCATIONS_INT, 0, TRUE) == HEAD_WEAR_LOC);
}
