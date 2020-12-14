
#pragma once

#include "WeenieObject.h"

class CAmmunitionWeenie : public CWeenieObject
{
public:
	CAmmunitionWeenie();
	virtual ~CAmmunitionWeenie() override;

	virtual class CAmmunitionWeenie *AsAmmunition() override { return this; }

	virtual void ApplyQualityOverrides() override;

	virtual void PostSpawn() override;
	virtual int DoCollision(const class EnvCollisionProfile &prof) override;
	virtual int DoCollision(const class AtkCollisionProfile &prof) override;
	virtual int DoCollision(const class ObjCollisionProfile &prof) override;
	virtual void DoCollisionEnd(uint32_t object_id) override;

	virtual uint32_t GetPhysicsTargetID() override { return _targetID; }

	void MakeIntoMissile();
	void MakeIntoAmmo();

	void HandleNonTargetCollision();
	void HandleTargetCollision();

	uint32_t _sourceID = 0;
	uint32_t _launcherID = 0;
	uint32_t _targetID = 0;

	float _attackPower = 0.0f;
	STypeSkill _weaponSkill = UNDEF_SKILL;
	uint32_t _weaponSkillLevel = 0;
};

