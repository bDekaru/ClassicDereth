
#pragma once

#include "Ammunition.h"

class CMissileWeenie : public CAmmunitionWeenie
{
public:
	CMissileWeenie();
	virtual ~CMissileWeenie() override;

	virtual class CMissileWeenie *AsMissile() override { return this; }

protected:
};

