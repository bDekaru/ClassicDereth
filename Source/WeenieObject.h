
#pragma once

#include "Movement.h"
#include "PhysicsObj.h"
#include "Qualities.h"

class CWorldLandBlock;

#define MAX_PLAYER_INVENTORY 102
#define MAX_PLAYER_CONTAINERS 7

#define USEDISTANCE_ANYWHERE FLT_MAX
#define USEDISTANCE_FAR 100

template<class T>
std::string FormatNumberString(T value)
{
	std::stringstream ss;
	ss.imbue(std::locale(""));
	ss << std::fixed << value;
	return ss.str();
}

enum DAMAGE_FORM // made up
{
	DF_UNDEF = 0,
	DF_MELEE = 1,
	DF_MISSILE = 2,
	DF_MAGIC = 4,
	DF_HOTSPOT = 8,
	DF_IMPACT = 0x10,

	DF_PHYSICAL = 0x3,
};

enum DAMAGE_QUADRANT // bitfield
{
	DQ_UNDEF = 0,
	DQ_HIGH = 1,
	DQ_MEDIUM = 2,
	DQ_LOW = 4,

	DQ_LEFT = 0x08,
	DQ_RIGHT = 0x10,
	DQ_FRONT = 0x20,
	DQ_BACK = 0x40,

	DQ_HEIGHT_MASK = 7,
};

enum BODY_PART_ENUM
{
	BP_UNDEF = -1,
	BP_HEAD = 0,
	BP_CHEST,
	BP_ABDOMEN,
	BP_UPPER_ARM,
	BP_LOWER_ARM,
	BP_HAND,
	BP_UPPER_LEG,
	BP_LOWER_LEG,
	BP_FOOT,
	BP_HORN,
	BP_FRONT_LEG,
	BP_FRONT_FOOT = 12,
	BP_REAR_LEG = 13,
	BP_REAR_FOOT = 15,
	BP_TORSO = 16,
	BP_TAIL = 17,
	BP_ARM = 18,
	BP_LEG = 19,
	BP_CLAW = 20,
	BP_WINGS = 21,
	BP_BREATH = 22,
	BP_TENTACLE = 23,
	BP_UPPER_TENTACLE = 24,
	BP_LOWER_TENTACLE = 25,
	BP_CLOAK = 26
};

struct DamageEventData
{
	CWeenieObject *source = NULL;
	CWeenieObject *target = NULL;
	CWeenieObject *weapon = NULL;

	DWORD attackSkill = 0;
	DWORD attackSkillLevel = 0;

	DAMAGE_FORM damage_form = DF_UNDEF;
	DAMAGE_TYPE damage_type = UNDEF_DAMAGE_TYPE;
	
	int preVarianceDamage = 0;
	double baseDamage = 0;
	double damageBeforeMitigation = 0;
	double damageAfterMitigation = 0;
	int outputDamageFinal = 0;
	float outputDamageFinalPercent = 0.0f;

	DAMAGE_QUADRANT hit_quadrant = DQ_UNDEF;
	BODY_PART_ENUM hitPart = BP_UNDEF;

	bool killingBlow = false;
	unsigned int attackConditions = 0;
	std::string killer_msg, victim_msg, other_msg;

	std::string spell_name;
	bool isProjectileSpell = false;

	double skillDamageBonus = 0.0;
	double attributeDamageBonus = 0.0;
	double slayerDamageBonus = 0.0;

	bool wasCrit = false;
	double critChance = 0.0;
	double critMultiplier = 0.0;

	bool ignoreMagicResist = false;
	bool ignoreMagicArmor = false;
	bool ignoreArmorEntirely = false;

	bool isArmorRending = false;
	bool isElementalRending = false;
	double rendingMultiplier = 0.0;

	std::string GetSourceName();
	std::string GetTargetName();
};

class CWeenieObject : public CPhysicsObj
{
private:
	double accumulatedHealthRegen = 0.0;
	double accumulatedStaminaRegen = 0.0;
	double accumulatedManaRegen = 0.0;
public:
	CWeenieObject();
	virtual ~CWeenieObject();

	// Control handling
	void Attach(CWorldLandBlock *pBlock);
	void Detach();
	CWorldLandBlock *GetBlock();
	void ReleaseFromBlock();

	// BOOL Think();
	// virtual BOOL DefaultThink() { return FALSE; }
	// void MakeLive();

	bool IsContainedWithinViewable(DWORD object_id);

	virtual bool ShouldSave();
	virtual void SaveEx(class CWeenieSave &save);
	virtual bool Save();
	virtual void LoadEx(class CWeenieSave &save);
	virtual bool Load();
	static CWeenieObject *Load(DWORD weenie_id);

	DWORD GetTopLevelID();

	virtual void MakeAware(CWeenieObject *, bool bForceUpdate = false) { }

	// Returns WERROR code
	virtual int UseChecked(CPlayerWeenie *);

	virtual class CAmmunitionWeenie *AsAmmunition() { return NULL; }
	virtual class CAttributeTransferDeviceWeenie *AsAttributeTransferDevice() { return NULL; }
	virtual class CBookWeenie *AsBook() { return NULL; }
	virtual class CBootSpotWeenie *AsBootSpot() { return NULL; }
	virtual class CCasterWeenie *AsCaster() { return NULL; }
	virtual class CChestWeenie *AsChest() { return NULL; }
	virtual class CClothingWeenie *AsClothing() { return NULL; }
	virtual class CContainerWeenie *AsContainer() { return NULL; }
	virtual class CCorpseWeenie *AsCorpse() { return NULL; }
	virtual class CDeedWeenie *AsDeed() { return NULL; }
	virtual class CBaseDoor *AsDoor() { return NULL; }
	virtual class CFoodWeenie *AsFood() { return NULL; }
	virtual class CGemWeenie *AsGem() { return NULL; }
	virtual class CHealerWeenie *AsHealer() { return NULL; }
	virtual class CHotSpotWeenie *AsHotSpot() { return NULL; }
	virtual class CHookWeenie *AsHook() { return NULL; }
	virtual class CHouseWeenie *AsHouse() { return NULL; }
	virtual class CHousePortalWeenie *AsHousePortal() { return NULL; }
	virtual class CKeyWeenie *AsKey() { return NULL; }
	virtual class CBaseLifestone *AsLifestone() { return NULL; }
	virtual class CBindStone *AsBindStone() { return NULL; }
	virtual class CLockpickWeenie *AsLockpick() { return NULL; }
	virtual class CMeleeWeaponWeenie *AsMeleeWeapon() { return NULL; }
	virtual class CMissileWeenie *AsMissile() { return NULL; }
	virtual class CMissileLauncherWeenie *AsMissileLauncher() { return NULL; }
	virtual class CMonsterWeenie *AsMonster() { return NULL; }
	virtual class CPKModifierWeenie *AsPKModifier() { return NULL; }
	virtual class CPlayerWeenie *AsPlayer() { return NULL; }
	virtual class CPortal *AsPortal() { return NULL; }
	virtual class CPressurePlateWeenie *AsPressurePlate() { return NULL; }
	virtual class CScrollWeenie *AsScroll() { return NULL; }
	virtual class CSkillAlterationDeviceWeenie *AsSkillAlterationDevice() { return NULL; }
	virtual class CSlumLordWeenie *AsSlumLord() { return NULL; }
	virtual class CSpellProjectile *AsSpellProjectile() { return NULL; }
	virtual class CStorageWeenie *AsStorage() { return NULL; }
	virtual class CSwitchWeenie *AsSwitch() { return NULL; }
	virtual class CTownCrier *AsTownCrier() { return NULL; }
	virtual class CVendor *AsVendor() { return NULL; }

	virtual bool IsAdvocate() { return false; }
	virtual bool IsSentinel() { return false; }
	virtual bool IsAdmin() { return false; }
	bool IsLocked();

	void SetLocked(BOOL locked);
	virtual void ResetToInitialState() { }

	virtual void RecalculateEncumbrance();

	// The use functionality, minus the pre-checks
	virtual int Activate(DWORD activator_id);
	virtual int Use(CPlayerWeenie *);
	virtual int UseWith(CPlayerWeenie *player, CWeenieObject *with);
	virtual int DoUseResponse(CWeenieObject *player) { return WERROR_NONE; }
	virtual int DoUseWithResponse(CWeenieObject *player, CWeenieObject *with);
	void DoActivationEmote(DWORD activator_id);
	void DoUseEmote(CWeenieObject *other);
	void ChanceExecuteEmoteSet(DWORD other_id, EmoteCategory category);
	bool IsExecutingEmote();

	virtual void EnsureLink(CWeenieObject *source);
	virtual void NotifyGeneratedDeath(CWeenieObject *weenie);
	virtual void OnGeneratedDeath(CWeenieObject *weenie);
	virtual void NotifyGeneratedPickedUp(CWeenieObject *weenie);
	virtual void OnGeneratedPickedUp(CWeenieObject *weenie);

	virtual void TryIdentify(CWeenieObject *other);
	virtual void Identify(CWeenieObject *other, DWORD overrideId = 0);

	virtual void MarkForDestroy() { m_bDestroyMe = true; }
	virtual bool ShouldDestroy() { return m_bDestroyMe; }
	virtual void Remove();
	virtual void DebugValidate();

	virtual void ApplyQualityOverrides() { }
	virtual void SetupWeenie();
	virtual void PreSpawnCreate() { }
	virtual void PostSpawn();

	virtual void InitPhysicsObj();
	void CleanupPhysics();

	int GetStructureNum();
	int GetStackOrStructureNum();
	void DecrementStackOrStructureNum(int amount = 1, bool bDestroyOnZero = true);
	void DecrementStackNum(int amount = 1, bool bDestroyOnZero = true);
	void DecrementStructureNum(int amount = 1, bool bDestroyOnZero = true);
	void SetStackSize(DWORD stackSize);

	void CheckDeath(CWeenieObject *source, DAMAGE_TYPE dt);

	// Actions
	virtual void SpeakLocal(const char *text, LogTextType ltt = LTT_SPEECH);
	virtual void EmoteLocal(const char *text);
	virtual void ActionLocal(const char *text);
	
	// REAL WEENIE VIRTUAL FUNCTIONS

	virtual BOOL _IsPlayer() { return AsPlayer() != NULL; /*m_Qualities.GetID() == 1;*/ } // 0x10
	// virtual bool IsThePlayer(); // 0x14
	virtual ITEM_TYPE InqType(); // 0x18

	virtual BOOL IsPK(); // 0x20
	virtual BOOL IsPKLite(); // 0x24
	virtual BOOL IsImpenetrable(); // 0x28
	virtual bool IsCreature(); // 0x2C
	virtual bool InqJumpVelocity(float extent, float &vz); // 0x30
	virtual bool InqRunRate(float &rate); // 0x34
	virtual bool CanJump(float extent);
	virtual bool JumpStaminaCost(float, long &); // 0x40

	virtual int DoCollision(const class EnvCollisionProfile &prof);
	virtual int DoCollision(const class ObjCollisionProfile &prof);
	virtual int DoCollision(const class AtkCollisionProfile &prof);
	virtual void DoCollisionEnd(DWORD object_id);

	virtual int InqCollisionProfile(ObjCollisionProfile &prof); // 0x5C

	virtual CWeenieObject *FindContainedItem(DWORD object_id) { return NULL; }

	virtual void BeginLogout() { }

	// END OF REAL WEENIE VIRTUAL FUNCTIONS

	virtual void HitGround(float zv);

	DWORD GetLandcell();
	const Position &GetPosition();
	
	bool IsCorpse() { return m_Qualities.m_WeenieType == Corpse_WeenieType; }
	bool IsDoor() { return m_Qualities.m_WeenieType == Door_WeenieType; }
	bool IsInscribable() { return InqBoolQuality(INSCRIBABLE_BOOL, FALSE) ? true : false; }
	bool IsLifestone() { return m_Qualities.m_WeenieType == LifeStone_WeenieType; }
	bool IsVendor() { return m_Qualities.m_WeenieType == Vendor_WeenieType; }
	bool IsStuck() { return InqBoolQuality(STUCK_BOOL, FALSE) ? true : false; }
	bool IsPortal() { return m_Qualities.m_WeenieType == Portal_WeenieType; }
	bool IsAttackable() { return InqBoolQuality(ATTACKABLE_BOOL, TRUE) ? true : false; }
	bool IsContainer() { return InqType() & TYPE_CONTAINER ? true : false; }

	virtual bool RequiresPackSlot() { return InqBoolQuality(REQUIRES_BACKPACK_SLOT_BOOL, FALSE) ? true : false; }

	BOOL IsContained() { return GetContainerID() != 0 ? true : false; }
	BOOL IsEquipped() { return InqIntQuality(CURRENT_WIELDED_LOCATION_INT, 0) != 0 ? true : false; }
	BOOL IsEquippable() { return InqIntQuality(LOCATIONS_INT, 0) != 0 ? true : false; }
	BOOL IsWielded() { return GetWielderID() != 0 ? true : false; }

	DWORD GetContainerID() { return InqIIDQuality(CONTAINER_IID, 0); }
	DWORD GetWielderID() { return InqIIDQuality(WIELDER_IID, 0); }

	virtual CWeenieObject *GetWieldedCaster() { return NULL; }
	DWORD GetWieldedCasterID() { CWeenieObject *caster = GetWieldedCaster(); return caster ? caster->GetID() : 0; }

	bool IsGeneratorSlotReady(int slot);
	void InitCreateGenerator();

	bool IsStorage();
	bool CanPickup(); // custom
		
	virtual void SendNetMessage(void *_data, DWORD _len, WORD _group, BOOL _event = 0);
	virtual void SendNetMessage(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1);
	virtual void SendNetMessageToTopMost(void *_data, DWORD _len, WORD _group, BOOL _event = 0);
	virtual void SendNetMessageToTopMost(BinaryWriter *_food, WORD _group, BOOL _event = 0, BOOL del = 1);

	void SendText(const char* szText, long lColor);

	float DistanceTo(CWeenieObject *other, bool bUseSpheres = false);
	float DistanceSquared(CWeenieObject *other);
	float HeadingTo(CWeenieObject *target, bool absolute = true);
	float HeadingFrom(CWeenieObject *target, bool absolute = true);
	float HeadingTo(DWORD targetId, bool relative = true);
	float HeadingFrom(DWORD targetId, bool relative = true);

	float GetBurdenPercent();

	void GivePerksForKill(CWeenieObject *pKilled);
	void GiveSharedXP(long long amount, bool showText);
	void GiveXP(long long amount, bool showText = false, bool allegianceXP = false);
	virtual void OnGivenXP(long long amount, bool allegianceXP) { }
	DWORD GiveAttributeXP(STypeAttribute key, DWORD amount);
	DWORD GiveAttribute2ndXP(STypeAttribute2nd key, DWORD amount);
	DWORD GiveSkillXP(STypeSkill key, DWORD amount, bool silent = true);
	DWORD GiveSkillPoints(STypeSkill key, DWORD amount);
	DWORD GiveSkillAdvancementClass(STypeSkill key, SKILL_ADVANCEMENT_CLASS sac);
	void GiveSkillCredits(DWORD amount, bool showText);

	void TryToUnloadAllegianceXP(bool bShowText);

	void EmitEffect(DWORD effect, float mod);

	virtual void OnMotionDone(DWORD motion, BOOL success);
	virtual void OnDeath(DWORD killer_id);

	virtual bool IsDead();
	virtual bool IsInPortalSpace();
	double GetHealthPercent();
	unsigned int GetHealth();
	unsigned int GetMaxHealth();
	void SetHealth(unsigned int value, bool bSendUpdate = true);
	unsigned int GetStamina();
	unsigned int GetMaxStamina();
	void SetStamina(unsigned int value, bool bSendUpdate = true);
	unsigned int GetMana();
	unsigned int GetMaxMana();
	void SetMana(unsigned int value, bool bSendUpdate = true);
	void SetMaxVitals(bool bSendUpdate = true);

	int AdjustHealth(int amount);
	int AdjustStamina(int amount);
	int AdjustMana(int amount);

	void Revive();
	bool m_bReviveAfterAnim = false;

	bool TeleportToSpawn(); // on death
 	bool TeleportToLifestone(); // on lifestone recall
	bool TeleportToHouse(); // on house recall
	bool TeleportToMansion(); // on mansion recall
	bool TeleportToAllegianceHometown();

	void ExecuteUseEvent(class CUseEventData *useEvent);
	void ExecuteAttackEvent(class CAttackEventData *attackEvent);

	void SendUseMessage(CWeenieObject *other, unsigned int channel);

	BinaryWriter *CreateMessage();
	BinaryWriter *UpdateMessage();
	void RemovePreviousInstance();
	void UpdateModel();
	virtual void GetObjDesc(ObjDesc &objDesc);
	virtual bool IsHelm() { return false; }
	virtual bool ShowHelm() { return true; }

	void SetMotionTableID(DWORD motion_table_did);
	void SetSetupID(DWORD setup_did);
	void SetSoundTableID(DWORD sound_table_did);
	void SetPETableID(DWORD pe_table_did);
	void SetIcon(DWORD icon_did);
	void SetScale(float value);
	void SetItemType(ITEM_TYPE type);
	void SetName(const char *name);
	void SetRadarBlipColor(RadarBlipEnum color);
	void SetShortDescription(const char *text);
	void SetLongDescription(const char *text);
	void SetInitialPhysicsState(DWORD physics_state);
	void SetInitialPosition(const Position &position);
	void SetSpellID(DWORD spell_id);

	DWORD GetIcon();
	std::string GetName();
	std::string GetPluralName();
	ITEM_TYPE GetItemType();
	const char *GetLongDescription();
	DWORD GetSpellID();

	virtual DWORD RecalculateCoinAmount() { return 0; };
	virtual DWORD ConsumeCoin(int amountToConsume) { return 0; };
	void SetValue(DWORD amount);
	DWORD GetValue();

	int InqIntQuality(STypeInt key, int defaultValue, BOOL raw = FALSE);
	long long InqInt64Quality(STypeInt64 key, long long defaultValue);
	BOOL InqBoolQuality(STypeBool key, BOOL defaultValue);
	double InqFloatQuality(STypeFloat key, double defaultValue, BOOL raw = FALSE);
	std::string InqStringQuality(STypeString key, std::string defaultValue);
	DWORD InqDIDQuality(STypeDID key, DWORD defaultValue);
	DWORD InqIIDQuality(STypeIID key, DWORD defaultValue);
	Position InqPositionQuality(STypePosition key, const Position &defaultValue);

	DAMAGE_TYPE InqDamageType();

	bool GetFloatEnchantmentDetails(STypeFloat stype, double defaultValue, EnchantedQualityDetails *enchantmentDetails);
	bool GetIntEnchantmentDetails(STypeInt stype, int defaultValue, EnchantedQualityDetails *enchantmentDetails);

	void TryCancelAttack();

	float GetArmorModForDamageType(DAMAGE_TYPE dt);

	bool m_bDestroyMe = false;

	virtual void WieldedTick();
	virtual void InventoryTick();
	virtual void Tick();
	
	void CheckForExpiredEnchantments();

	class EmoteManager *m_EmoteManager = NULL;
	class UseManager *m_UseManager = NULL;
	class CSpellcastingManager *m_SpellcastingManager = NULL;
	class AttackManager *m_AttackManager = NULL;
	
	EmoteManager *MakeEmoteManager();
	UseManager *MakeUseManager();
	CSpellcastingManager *MakeSpellcastingManager();
	AttackManager *MakeAttackManager();

	bool m_bObjDescOverride = false;
	ObjDesc m_ObjDescOverride;

	CACQualities m_Qualities; // m_pQualities is 0x14C

	void NotifyObjectCreated(bool bPrivate = true);
	void NotifyObjectUpdated(bool bPrivate = true);
	void NotifyIntStatUpdated(STypeInt key, bool bPrivate = true);
	void NotifyInt64StatUpdated(STypeInt64 key, bool bPrivate = true);
	void NotifyBoolStatUpdated(STypeBool key, bool bPrivate = true);
	void NotifyFloatStatUpdated(STypeFloat key, bool bPrivate = true);
	void NotifyStringStatUpdated(STypeString key, bool bPrivate = true);
	void NotifyDIDStatUpdated(STypeDID key, bool bPrivate = true);
	void NotifyIIDStatUpdated(STypeIID key, bool bPrivate = true);
	void NotifyPositionStatUpdated(STypePosition key, bool bPrivate = true);
	void NotifyStackSizeUpdated(bool bPrivate = true);
	void NotifyContainedItemRemoved(DWORD objectId, bool bPrivate = true);
	void NotifyObjectRemoved();

	void NotifyAttributeStatUpdated(STypeAttribute key);
	void NotifyAttribute2ndStatUpdated(STypeAttribute2nd key);
	void NotifySkillStatUpdated(STypeSkill key);
	void NotifySkillAdvancementClassUpdated(STypeSkill key);
	void NotifyEnchantmentUpdated(Enchantment *enchant);

	virtual void NotifyAttackDone(int error = WERROR_NONE) { }
	virtual void NotifyCommenceAttack() { }
	virtual void NotifyUseDone(int error = WERROR_NONE) { }
	virtual void NotifyWeenieError(int error) { }
	virtual void NotifyWeenieErrorWithString(int error, const char *text) { }
	virtual void NotifyInventoryFailedEvent(DWORD object_id, int error) { }

	void CopyIntStat(STypeInt key, CWeenieObject *from);
	void CopyInt64Stat(STypeInt64 key, CWeenieObject *from);
	void CopyBoolStat(STypeBool key, CWeenieObject *from);
	void CopyFloatStat(STypeFloat key, CWeenieObject *from);
	void CopyStringStat(STypeString key, CWeenieObject *from);
	void CopyDIDStat(STypeDID key, CWeenieObject *from);
	void CopyPositionStat(STypePosition key, CWeenieObject *from);

	void CopyIntStat(STypeInt key, CACQualities *from);
	void CopyInt64Stat(STypeInt64 key, CACQualities *from);
	void CopyBoolStat(STypeBool key, CACQualities *from);
	void CopyFloatStat(STypeFloat key, CACQualities *from);
	void CopyStringStat(STypeString key, CACQualities *from);
	void CopyDIDStat(STypeDID key, CACQualities *from);
	void CopyPositionStat(STypePosition key, CACQualities *from);

	DWORD GetCostToRaiseSkill(STypeSkill skill);

	virtual void TryCastSpell(DWORD target_id, DWORD spell_id);

	const char *GetGenderString();
	const char *GetRaceString();
	const char *GetTitleString();

	virtual void HandleAggro(CWeenieObject *attacker);
	virtual void OnIdentifyAttempted(CWeenieObject *other) { };
	virtual void OnResistSpell(CWeenieObject *attacker) { };
	virtual void OnEvadeAttack(CWeenieObject *attacker) { };

	virtual void OnWield(CWeenieObject *wielder);
	virtual void OnUnwield(CWeenieObject *wielder);

	virtual void OnPickedUp(CWeenieObject *pickedUpBy);
	virtual void OnDropped(CWeenieObject *droppedBy);

	virtual void OnTeleported();
	void Movement_Teleport(const Position &position, bool bWasDeath = false);

	bool m_bDontClear = true;

	DWORD m_dwEquipSlot = 0;
	// DWORD m_dwCoverage1; // LOCATIONS_INT
	// DWORD m_dwCoverage2; // CURRENT_WIELDED_LOCATION_INT
	// DWORD m_dwCoverage3; // CLOTHING_PRIORITY_INT

	DWORD m_dwLastSpawnedCreatureID = 0;

	DWORD m_LastUsedBy = 0;
	double m_LastUsed = 0.0;

	CPhysicsObj *_phys_obj = NULL;
	class CPhysicsObj *GetPhysicsObj() { return _phys_obj; }

	virtual void HandleAttackHook(const AttackCone &cone);

	void DoForcedStopCompletely();
	DWORD DoAutonomousMotion(DWORD motion, MovementParameters *params = NULL);
	DWORD DoForcedMotion(DWORD motion, MovementParameters *params = NULL);
	
	virtual bool ImmuneToDamage(class CWeenieObject *other);
	virtual bool IsBusy();
	bool IsMovingTo();
	bool IsCompletelyIdle();
	bool HasInterpActions();
	bool IsBusyOrInAction();

	virtual DWORD GetPhysicsTargetID() { return 0; }
	virtual void HandleMoveToDone(DWORD error);

	virtual void TryToDealDamage(DamageEventData &damageData);
	virtual void TakeDamage(DamageEventData &damageData);
	virtual void OnTookDamage(DamageEventData &damageData);
	virtual void OnDealtDamage(DamageEventData &damageData);

	virtual float GetEffectiveArmorLevel(DamageEventData &damageData, bool bIgnoreMagicArmor);

	void NotifyDeathMessage(DWORD killer_id, const char *message);
	virtual void NotifyAttackerEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int health, unsigned int crit, unsigned int attackConditions) { }
	virtual void NotifyDefenderEvent(const char *name, unsigned int dmgType, float healthPercent, unsigned int health, BODY_PART_ENUM hitPart, unsigned int crit, unsigned int attackConditions) { }
	virtual void NotifyKillerEvent(const char *text) { }
	virtual void NotifyVictimEvent(const char *text) { }

	BYTE GetNextStatTimestamp(StatType statType, int statIndex);

	bool HasFellowship();
	void LeaveFellowship();
	class Fellowship *GetFellowship();

	virtual COMBAT_MODE GetEquippedCombatMode();

	double _nextHeartBeat = -1.0;
	double _nextHeartBeatEmote = -1.0;
	CombatManeuverTable *_combatTable = NULL;

	CWeenieObject *GetWorldContainer();
	CWeenieObject *GetWorldWielder();
	CWeenieObject *GetWorldOwner();
	CWeenieObject *GetWorldTopLevelOwner();
	class CContainerWeenie *GetWorldTopLevelContainer();

	virtual void ChangeCombatMode(COMBAT_MODE mode, bool playerRequested) { }

	void ReleaseFromAnyWeenieParent(bool bBroadcastContainerChange = true, bool bBroadcastEquipmentChange = true);
	
	void SetWeenieContainer(DWORD container_id);
	void SetWielderID(DWORD wielder_id);
	void SetWieldedLocation(DWORD location);

	bool HasOwner();
	bool CachedHasOwner();

	void RecacheHasOwner();

	virtual CWeenieObject *FindContained(DWORD object_id);
	virtual bool IsValidWieldLocation(DWORD location);
	virtual bool CanEquipWith(CWeenieObject *other, DWORD otherLocation);
	virtual void SimulateGiveObject(class CContainerWeenie *target, DWORD wcid, int amount = 1, int ptid = 0, float shade = 0, int bondedType = 0);
	virtual int SimulateGiveObject(class CContainerWeenie *target, CWeenieObject *object_weenie);
	virtual int CraftObject(CContainerWeenie *target_container, CWeenieObject *object_weenie);
	virtual void ReleaseContainedItemRecursive(CWeenieObject *object_weenie) { }
	virtual CWeenieObject *GetWieldedCombat(COMBAT_USE combatUse) { return NULL; }
	virtual CWeenieObject *GetWielded(INVENTORY_LOC slot) { return NULL; }

	virtual double GetMeleeDefenseMod();
	virtual double GetMissileDefenseMod();
	virtual double GetMagicDefenseMod();

	virtual double GetMeleeDefenseModUsingWielded();
	virtual double GetMissileDefenseModUsingWielded();
	virtual double GetMagicDefenseModUsingWielded();

	virtual double GetOffenseMod();
	virtual int GetAttackTime();
	virtual int GetAttackTimeUsingWielded();
	virtual int GetAttackDamage();

	double GetCrushingBlowMultiplier();
	double GetBitingStrikeFrequency();

	DWORD GetImbueEffects();
	void AddImbueEffect(ImbuedEffectType effect);

	bool IsArmorRending();

	virtual double GetManaConversionMod();

	DWORD GetEffectiveManaConversionSkill();

	bool IsInPeaceMode();

	DWORD GetMagicDefense();

	bool TryMagicResist(DWORD magicSkill);
	bool TryMeleeEvade(DWORD attackSkill);
	bool TryMissileEvade(DWORD attackSkill);

	// Entry to AttackManager
	bool IsAttacking();
	virtual void TryMeleeAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion = 0);
	virtual void TryMissileAttack(DWORD target_id, ATTACK_HEIGHT height, float power, DWORD motion = 0);

	bool IsAvatarJumpsuit();

	virtual bool InqQuest(const char *questName) { return false; }
	virtual int InqTimeUntilOkayToComplete(const char *questName) { return 0; }
	virtual unsigned int InqQuestSolves(const char *questName) { return 0; }
	virtual bool UpdateQuest(const char *questName) { return false; }
	virtual void StampQuest(const char *questName) { }
	virtual void IncrementQuest(const char *questName) { }
	virtual void DecrementQuest(const char *questName) { }
	virtual void EraseQuest(const char *questName) { }

	bool LearnSpell(DWORD spell_id, bool showTextAndEffect);

	BYTE _stackSequence = 0;

	DWORD _lastOpenedRemoteContainerId = 0;
	
	double _nextReset = -1.0;
	double _nextRegen = -1.0;
	double _timeToRot = -1.0;
	bool _beganRot = false;
	double _nextManaUse = -1.0;

	bool _cachedHasOwner = false;

	BOOL InqSkill(STypeSkill key, DWORD &value, BOOL raw);

	inline bool IsWorldAware() { return m_bWorldIsAware; }

	virtual bool IsAttunedOrContainsAttuned();
	virtual bool HasContainerContents() { return false; }

	bool IsBonded();
	bool IsDroppedOnDeath();
	bool IsDestroyedOnDeath();

	void CheckVitalRanges();

	void CheckRegeneration(double rate, STypeAttribute2nd currentAttrib, STypeAttribute2nd maxAttrib);

	void CheckEventState();
	void HandleEventActive();
	void HandleEventInactive();

	double _blockNewAttacksUntil = -1.0;

protected:
	CWorldLandBlock *m_pBlock = NULL;

	std::unordered_map<DWORD, BYTE> m_StatSequences;

	// whether or not anyone has ever been made aware of this objects existence
	bool m_bWorldIsAware = false;
};

