
#pragma once

class CGameMode;

class CGameMode
{
public:
	CGameMode();
	virtual ~CGameMode();

	virtual const char *GetName() = 0;
	virtual void Think() = 0;

	virtual void OnRemoveEntity(CWeenieObject *pEntity) = 0;
	virtual void OnTargetAttacked(CWeenieObject *pTarget, CWeenieObject *pSource) = 0;
};

class CGameMode_Tag : public CGameMode
{
public:
	CGameMode_Tag();
	virtual ~CGameMode_Tag() override;

	virtual const char *GetName();
	virtual void Think() override;

	virtual void OnRemoveEntity(CWeenieObject *pEntity) override;
	virtual void OnTargetAttacked(CWeenieObject *pTarget, CWeenieObject *pSource) override;

protected:
	void SelectPlayer(CPlayerWeenie *pPlayer);
	void UnselectPlayer();

	CPlayerWeenie *m_pSelectedPlayer;
};
