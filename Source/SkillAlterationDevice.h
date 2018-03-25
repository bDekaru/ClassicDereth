
#pragma once

#include "WeenieObject.h"

class CSkillAlterationDeviceWeenie : public CWeenieObject
{
public:
	CSkillAlterationDeviceWeenie();
	virtual ~CSkillAlterationDeviceWeenie() override;

	virtual CSkillAlterationDeviceWeenie *AsSkillAlterationDevice() override { return this; }

	virtual int Use(CPlayerWeenie *pOther) override;
};

