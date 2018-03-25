
#pragma once

#include "Monster.h"

class CTownCrier : public CMonsterWeenie
{
public:
	CTownCrier();
	virtual ~CTownCrier() override;

	virtual class CTownCrier *AsTownCrier() { return this; }

	virtual int DoUseResponse(CWeenieObject *player) override;
	virtual int Use(CPlayerWeenie *other) override;
	virtual void HandleMoveToDone(DWORD error) override;

	virtual DWORD OnReceiveInventoryItem(CWeenieObject *source, CWeenieObject *item, DWORD desired_slot) override;

	std::string GetNewsText(bool paid);
};