#pragma once

#include "UseManager.h"

class CBaseLifestone : public CWeenieObject
{
public:
	CBaseLifestone();
	virtual ~CBaseLifestone() override;

	virtual class CBaseLifestone *AsLifestone() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *) override;
	virtual int DoUseResponse(CWeenieObject *player) override;

protected:
};