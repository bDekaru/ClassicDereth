#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CScrollUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(uint32_t motion) override;

	uint32_t _spell_id = 0;
};

class CScrollWeenie : public CWeenieObject // CWeenieObject
{
public:
	CScrollWeenie();
	virtual ~CScrollWeenie() override;

	virtual class CScrollWeenie *AsScroll() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *player) override;

	const CSpellBase *GetSpellBase();

protected:
};

