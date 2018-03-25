#pragma once

#include "UseManager.h"

class CBindStone : public CWeenieObject
{
public:
	CBindStone();
	virtual ~CBindStone() override;

	virtual class CBindStone *AsBindStone() { return this; }

	virtual int Use(CPlayerWeenie *) override;
	virtual int DoUseResponse(CWeenieObject *player) override;

protected:
};