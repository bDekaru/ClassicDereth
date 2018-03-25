
#pragma once

#include "WeenieObject.h"

class CClothingWeenie : public CWeenieObject
{
public:
	CClothingWeenie();

	virtual class CClothingWeenie *AsClothing() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *other) override;

	virtual bool IsValidWieldLocation(DWORD location) override;
	virtual bool CanEquipWith(CWeenieObject *other, DWORD otherLocation) override;

	bool CoversBodyPart(BODY_PART_ENUM part, float *factor);
	virtual float GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor) override;

	virtual bool IsHelm() override;
};
