#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CManaStoneUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
};

class CManaStoneWeenie : public CWeenieObject // CWeenieObject
{
public:
	CManaStoneWeenie();
	virtual ~CManaStoneWeenie() override;

	virtual class CManaStoneWeenie *AsManaStone() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int UseWith(CPlayerWeenie *player, CWeenieObject *with) override;
	virtual int DoUseWithResponse(CWeenieObject * player, CWeenieObject * pTarget) override;

protected:
};

