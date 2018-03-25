
#pragma once

#include "WeenieObject.h"

class CMeleeWeaponWeenie : public CWeenieObject
{
public:
	CMeleeWeaponWeenie();

	virtual class CMeleeWeaponWeenie *AsMeleeWeapon() { return this; }

	virtual COMBAT_MODE GetEquippedCombatMode() override { return COMBAT_MODE::MELEE_COMBAT_MODE; }
};
