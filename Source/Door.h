
#pragma once

class CBaseDoor : public CWeenieObject
{
public:
	CBaseDoor();
	virtual ~CBaseDoor() override;
	
	virtual class CBaseDoor *AsDoor() override { return this; }

	virtual int Activate(uint32_t activator_id) override;
	virtual int Use(CPlayerWeenie *) override;

	virtual void PostSpawn() override;
	virtual void ResetToInitialState() override;

	void CloseDoor();
	void OpenDoor();
	
	bool IsClosed();

protected:
	bool m_bOpen = false;
	bool m_bInitiallyLocked = false;
};
