
#pragma once

#include "WeenieObject.h"

class CSwitchWeenie : public CWeenieObject
{
public:
	CSwitchWeenie();
	virtual ~CSwitchWeenie() override;

	virtual class CSwitchWeenie *AsSwitch() override { return this; }

	virtual void ApplyQualityOverrides() override;

	virtual int Activate(uint32_t activator_id) override;
	virtual int Use(CPlayerWeenie *) override;

	void PlaySwitchMotion();

	double m_fNextSwitchActivation = 0.0;
};

