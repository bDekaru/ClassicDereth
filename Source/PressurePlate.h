
#pragma once

#include "WeenieObject.h"

class CPressurePlateWeenie : public CWeenieObject
{
public:
	CPressurePlateWeenie();
	virtual ~CPressurePlateWeenie() override;

	virtual class CPressurePlateWeenie *AsPressurePlate() { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual void Tick() override;

	virtual void PostSpawn() override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;

	virtual int Activate(DWORD activator_id) override;

protected:
};

