
#pragma once

#include "WeenieObject.h"

class CKeyWeenie : public CWeenieObject
{
public:
	CKeyWeenie();
	virtual ~CKeyWeenie() override;

	virtual CKeyWeenie *AsKey() override { return this; }

	virtual int UseWith(CPlayerWeenie *player, CWeenieObject *with) override;
	virtual int DoUseWithResponse(CWeenieObject *player, CWeenieObject *with) override;
};
