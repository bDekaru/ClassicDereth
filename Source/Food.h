#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CFoodUseEvent : public CGenericUseEvent
{
public:
	virtual void OnUseAnimSuccess(uint32_t motion) override;
};

class CFoodWeenie : public CWeenieObject // CWeenieObject
{
public:
	CFoodWeenie();
	virtual ~CFoodWeenie() override;

	virtual class CFoodWeenie *AsFood() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *) override;
	virtual int DoUseResponse(CWeenieObject *other) override;

protected:
};

