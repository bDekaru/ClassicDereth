
#pragma once

#include "ammunition.h"

class CMissileWeenie : public CAmmunitionWeenie
{
public:
	CMissileWeenie();
	virtual ~CMissileWeenie() override;

	virtual class CMissileWeenie *AsMissile() { return this; }

protected:
};

