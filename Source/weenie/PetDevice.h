#pragma once

#include "WeenieObject.h"
#include "Player.h"

// PetDevice_WeenieType (70)
class PetDevice : public CWeenieObject
{
public:
	PetDevice();
	virtual ~PetDevice() override;

	virtual void LoadEx(class CWeenieSave &save) override;

	virtual int Use(CPlayerWeenie *player) override;
	virtual int DoUseResponse(CWeenieObject *player) override;

	virtual void InventoryTick() override;
	virtual void Tick() override;

private:
	void SpawnPet(CPlayerWeenie *player);
	void DespawnPet(CPlayerWeenie *player, uint32_t petId);
};

// Pet_WeenieType (69)
class PetPassive : public CMonsterWeenie
{
public:
	PetPassive();
	virtual ~PetPassive() override;

	virtual bool IsCreature() override { return true; }
	
	virtual void PostSpawn() override;
	virtual void Tick() override;
	virtual void OnDeath(uint32_t killer_id) override;
};

// CombatPet_WeenieType (71)
class PetCombat : public PetPassive
{
public:
	PetCombat();
	virtual ~PetCombat() override;

	//virtual void PostSpawn() override;
	virtual bool CanTarget(CWeenieObject* target) override;

	virtual void Tick() override;
};

