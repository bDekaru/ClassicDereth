
#pragma once

#include "Container.h"

class CChestWeenie : public CContainerWeenie
{
public:
	CChestWeenie();
	virtual ~CChestWeenie() override;

	virtual class CChestWeenie *AsChest() override { return this; }

	virtual void PostSpawn() override;

	virtual void OnContainerOpened(CWeenieObject *other) override;
	virtual void OnContainerClosed(CWeenieObject *other) override;

	bool IsClosed() {
		return get_minterp()->interpreted_state.forward_command != Motion_On;
	}
};
