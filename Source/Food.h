#pragma once

#include "WeenieObject.h"

class CFoodWeenie : public CWeenieObject // CWeenieObject
{
public:
	CFoodWeenie();
	virtual ~CFoodWeenie() override;

	virtual class CFoodWeenie *AsFood() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *) override;
	virtual int DoUseResponse(CWeenieObject *other) override;

protected:
};

