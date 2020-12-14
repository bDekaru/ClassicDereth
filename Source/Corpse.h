
#pragma once

#include "Container.h"

class CCorpseWeenie : public CContainerWeenie
{
public:
	CCorpseWeenie();
	virtual ~CCorpseWeenie() override;

	virtual class CCorpseWeenie *AsCorpse() override { return this; }

	virtual void Tick() override;
	virtual void ApplyQualityOverrides() override;

	void SetObjDesc(const ObjDesc &desc);
	virtual void GetObjDesc(ObjDesc &objDesc) override;

	virtual int CheckOpenContainer(CWeenieObject *other) override;

	void BeginGracefulDestroy();

	virtual void OnContainerOpened(CWeenieObject *other) override;
	virtual void OnContainerClosed(CWeenieObject *other) override;
	
	virtual void SaveEx(class CWeenieSave &save) override;
	void RemoveEx();
	virtual void LoadEx(class CWeenieSave &save) override;
	virtual bool ShouldSave() override;
	bool ContainsDecayableItems();

	bool _shouldSave = false;
	double _begin_destroy_at = FLT_MAX;

protected:
	ObjDesc _objDesc;

	bool _hasBeenOpened = false;
	bool _begun_destroy = false;
	double _mark_for_destroy_at = FLT_MAX;
};
