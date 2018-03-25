
#pragma once

#include "WeenieObject.h"

class CGemWeenie : public CWeenieObject
{
public:
	CGemWeenie();
	virtual ~CGemWeenie() override;

	virtual CGemWeenie *AsGem() override { return this; }

	virtual int Use(CPlayerWeenie *pOther) override;
	virtual int DoUseResponse(CWeenieObject *player) override;
};

