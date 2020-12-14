
#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CPortalUseEvent : public CUseEventData
{
public:
	virtual void OnReadyToUse() override;
};

class CPortal : public CWeenieObject
{
public:
	CPortal();
	virtual ~CPortal();

	virtual class CPortal *AsPortal() override { return this; }

	virtual void PostSpawn() override;

	virtual int Use(CPlayerWeenie *) override;
	virtual void Tick() override;

	virtual int DoCollision(const class ObjCollisionProfile &prof) override;

	void CheckedTeleport(CWeenieObject *pOther);
	void Teleport(CWeenieObject *pTarget);

#if 0 // deprecated
	void ProximityThink();
#endif

	virtual bool GetDestination(Position &position);

private:
#if 0 // deprecated
	double m_fLastCacheClear;
	std::set<uint32_t> m_RecentlyTeleported;
#endif
};
