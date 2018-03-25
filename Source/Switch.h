
#pragma once

#include "WeenieObject.h"

class CSwitchWeenie : public CWeenieObject
{
public:
	CSwitchWeenie();
	virtual ~CSwitchWeenie() override;

	virtual class CSwitchWeenie *AsSwitch() { return this; }

	virtual void ApplyQualityOverrides() override;

	virtual int Activate(DWORD activator_id) override;
	virtual int Use(CPlayerWeenie *) override;

	void PlaySwitchMotion();

	double m_fNextSwitchActivation = 0.0;
};

