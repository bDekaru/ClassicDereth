
#pragma once

#include "WeenieObject.h"

class CAttributeTransferDeviceWeenie : public CWeenieObject
{
public:
	CAttributeTransferDeviceWeenie();
	virtual ~CAttributeTransferDeviceWeenie() override;

	virtual CAttributeTransferDeviceWeenie *AsAttributeTransferDevice() override { return this; }

	virtual int Use(CPlayerWeenie *pOther) override;
};

