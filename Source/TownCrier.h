
#pragma once

#include "Monster.h"

class CTownCrier : public CMonsterWeenie
{
public:
	CTownCrier();
	virtual ~CTownCrier() override;

	virtual class CTownCrier *AsTownCrier() override { return this; }

	virtual int DoUseResponse(CWeenieObject *player) override;
	virtual int Use(CPlayerWeenie *other) override;
	virtual void HandleMoveToDone(uint32_t error) override;

	virtual uint32_t OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, uint32_t desired_slot) override;

	std::string GetNewsText(bool paid);
};
