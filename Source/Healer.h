#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CHealerUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;
};

class CHealerWeenie : public CWeenieObject // CWeenieObject
{
public:
	CHealerWeenie();
	virtual ~CHealerWeenie() override;

	virtual class CHealerWeenie *AsHealer() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int UseWith(CPlayerWeenie *player, CWeenieObject *with) override;

protected:
};

