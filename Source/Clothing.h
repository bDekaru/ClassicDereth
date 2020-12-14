
#pragma once

#include "WeenieObject.h"

class CClothingWeenie : public CWeenieObject
{
public:
	CClothingWeenie();

	virtual class CClothingWeenie *AsClothing() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *other) override;

	virtual bool IsValidWieldLocation(uint32_t location) override;
	virtual bool CanEquipWith(CWeenieObject *other, uint32_t otherLocation) override;

	bool CoversBodyPart(BODY_PART_ENUM part, float *factor);
	virtual float GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor) override;

	virtual bool IsHelm() override;
	virtual bool IsCloak() override;
};
