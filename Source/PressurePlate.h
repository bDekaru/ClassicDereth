
#pragma once

#include "WeenieObject.h"

class CPressurePlateWeenie : public CWeenieObject
{
public:
	CPressurePlateWeenie();
	virtual ~CPressurePlateWeenie() override;

	virtual class CPressurePlateWeenie *AsPressurePlate() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual void Tick() override;

	virtual void PostSpawn() override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;

	virtual int Activate(uint32_t activator_id) override;

	double next_activation_time = 0.0f;

protected:
};

