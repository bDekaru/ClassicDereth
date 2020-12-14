
#pragma once

#include "WeenieObject.h"

class CAugmentationDeviceWeenie : public CWeenieObject
{
public:
	CAugmentationDeviceWeenie();
	virtual ~CAugmentationDeviceWeenie() override;

	virtual CAugmentationDeviceWeenie *AsAugmentationDevice() override { return this; }

	virtual int Use(CPlayerWeenie *pOther) override;
	virtual int UseEx(CPlayerWeenie *pOther, bool bConfirmed = false);
};

