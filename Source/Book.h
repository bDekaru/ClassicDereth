#pragma once

#include "WeenieObject.h"

class CBookWeenie : public CWeenieObject
{
public:
	CBookWeenie();
	virtual ~CBookWeenie() override;

	virtual class CBookWeenie *AsBook() override { return this; }

	virtual void ApplyQualityOverrides() override;
	virtual int Use(CPlayerWeenie *) override;

protected:
};

