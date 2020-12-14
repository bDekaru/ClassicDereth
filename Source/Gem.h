
#pragma once

#include "WeenieObject.h"
#include "UseManager.h"

class CGemUseEvent : public CUseEventData
{
public:
  virtual void OnReadyToUse() override;
};

class CGemWeenie : public CWeenieObject
{
public:
	CGemWeenie();
	virtual ~CGemWeenie() override;

	virtual CGemWeenie *AsGem() override { return this; }

	virtual int Use(CPlayerWeenie *pOther) override;
	virtual int DoUseResponse(CWeenieObject *player) override;
	double _nextUse = 0;
  virtual int UseWith(CPlayerWeenie* player, CWeenieObject* with) override;
  virtual int DoUseWithResponse(CWeenieObject* player, CWeenieObject* pTarget) override;

private:
  virtual int Tailor_ReduceArmor(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_KitOntoArmor(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_KitOntoWeapon(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_TempOntoArmor(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_TempOntoWeapon(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_LayerTop(CWeenieObject* source, CWeenieObject* pTarget);
  virtual int Tailor_LayerBot(CWeenieObject* source, CWeenieObject* pTarget);
};

