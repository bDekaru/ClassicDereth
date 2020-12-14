
#pragma once

#include "WeenieObject.h"

class CCasterWeenie : public CWeenieObject
{
public:
	CCasterWeenie();

	virtual class CCasterWeenie *AsCaster() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *other) override;

	virtual COMBAT_MODE GetEquippedCombatMode() override { return COMBAT_MODE::MAGIC_COMBAT_MODE; }
};
