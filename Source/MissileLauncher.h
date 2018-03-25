
#pragma once

#include "WeenieObject.h"

class CMissileLauncherWeenie : public CWeenieObject
{
public:
	CMissileLauncherWeenie();

	virtual class CMissileLauncherWeenie *AsMissileLauncher() { return this; }

	virtual COMBAT_MODE GetEquippedCombatMode() override { return COMBAT_MODE::MISSILE_COMBAT_MODE; }
};
