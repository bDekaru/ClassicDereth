
#pragma once

#include "WeenieObject.h"

class CPKModifierWeenie : public CWeenieObject
{
public:
	CPKModifierWeenie();
	virtual ~CPKModifierWeenie() override;

	virtual class CPKModifierWeenie *AsPKModifier() override { return this; }

	virtual int Use(CPlayerWeenie *player) override;
	virtual int DoUseResponse(CWeenieObject *player) override;
};
