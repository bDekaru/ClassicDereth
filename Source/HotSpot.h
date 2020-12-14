
#pragma once

#include "WeenieObject.h"

class CHotSpotWeenie : public CWeenieObject
{
public:
	CHotSpotWeenie();
	virtual ~CHotSpotWeenie() override;

	virtual class CHotSpotWeenie *AsHotSpot() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual void Tick() override;

	virtual void PostSpawn() override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;
	virtual void DoCollisionEnd(uint32_t object_id) override;

protected:
	void SetNextCycleTime();
	void DoCycle();
	void DoCycleDamage(CWeenieObject *other);

	double m_fNextCycleTime = 0.0;
	std::set<uint32_t> m_ContactedWeenies;
};

