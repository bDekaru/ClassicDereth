#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CScrollUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
	virtual void OnUseAnimSuccess(DWORD motion) override;

	DWORD _spell_id = 0;
};

class CScrollWeenie : public CWeenieObject // CWeenieObject
{
public:
	CScrollWeenie();
	virtual ~CScrollWeenie() override;

	virtual class CScrollWeenie *AsScroll() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *player) override;

	const CSpellBase *GetSpellBase();

protected:
};

