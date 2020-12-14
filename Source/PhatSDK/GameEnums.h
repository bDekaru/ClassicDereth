
#pragma once

enum FriendsUpdateType
{
	FRIENDS_UPDATE = 0,
	FRIENDS_UPDATE_ADD = 1,
	FRIENDS_UPDATE_REMOVE = 2,
	FRIENDS_UPDATE_REMOVE_SILENT = 3,
	FRIENDS_UPDATE_ONLINE_STATUS = 4
};

enum BondedStatusEnum
{
	Destroy_BondedStatus = 0XFFFFFFFE,
	Slippery_BondedStatus = 0XFFFFFFFF,
	Normal_BondedStatus = 0X0,
	Bonded_BondedStatus = 0X1,
	Sticky_BondedStatus = 0X2,
	FORCE_BondedStatusEnum_32_BIT = 0X7FFFFFFF,
};

enum RadarEnum {
	Undef_RadarEnum,
	ShowNever_RadarEnum,
	ShowMovement_RadarEnum,
	ShowAttacking_RadarEnum,
	ShowAlways_RadarEnum
};

enum ITEM_USEABLE {
	USEABLE_UNDEF = 0,
	USEABLE_NO = 0x1,
	USEABLE_SELF = 0x2,
	USEABLE_WIELDED = 0x4,
	USEABLE_CONTAINED = 0x8,
	USEABLE_VIEWED = 0x10,
	USEABLE_CONTAINED_VIEWED = 0x18,
	USEABLE_REMOTE = 0x20,
	USEABLE_VIEWED_REMOTE = 0x30,
	USEABLE_CONTAINED_VIEWED_REMOTE = 0x38,
	USEABLE_NEVER_WALK = 0x40,
	USEABLE_REMOTE_NEVER_WALK = 0x60,
	USEABLE_VIEWED_REMOTE_NEVER_WALK = 0x70,
	USEABLE_CONTAINED_VIEWED_REMOTE_NEVER_WALK = 0x78,
	USEABLE_OBJSELF = 0x80,
	USEABLE_SOURCE_WIELDED_TARGET_WIELDED = 0x40004,
	USEABLE_SOURCE_WIELDED_TARGET_CONTAINED = 0x80004,
	USEABLE_SOURCE_WIELDED_TARGET_VIEWED = 0x100004,
	USEABLE_SOURCE_WIELDED_TARGET_REMOTE = 2097156,
	USEABLE_SOURCE_WIELDED_TARGET_REMOTE_NEVER_WALK = 6291460,
	USEABLE_SOURCE_CONTAINED_TARGET_WIELDED = 262152,
	USEABLE_SOURCE_CONTAINED_TARGET_CONTAINED = 524296,
	USEABLE_SOURCE_CONTAINED_TARGET_OBJSELF_OR_CONTAINED = 8912904,
	USEABLE_SOURCE_CONTAINED_TARGET_SELF_OR_CONTAINED = 655368,
	USEABLE_SOURCE_CONTAINED_TARGET_VIEWED = 1048584,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE = 2097160,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE_NEVER_WALK = 6291464,
	USEABLE_SOURCE_CONTAINED_TARGET_REMOTE_OR_SELF = 2228232,
	USEABLE_SOURCE_VIEWED_TARGET_WIELDED = 262160,
	USEABLE_SOURCE_VIEWED_TARGET_CONTAINED = 524304,
	USEABLE_SOURCE_VIEWED_TARGET_VIEWED = 1048592,
	USEABLE_SOURCE_VIEWED_TARGET_REMOTE = 2097168,
	USEABLE_SOURCE_REMOTE_TARGET_WIELDED = 262176,
	USEABLE_SOURCE_REMOTE_TARGET_CONTAINED = 524320,
	USEABLE_SOURCE_REMOTE_TARGET_VIEWED = 1048608,
	USEABLE_SOURCE_REMOTE_TARGET_REMOTE = 2097184,
	USEABLE_SOURCE_REMOTE_TARGET_REMOTE_NEVER_WALK = 6291488,
	USEABLE_SOURCE_MASK = 65535,
	USEABLE_TARGET_MASK = -65536,
};

enum ITEM_TYPE {
	TYPE_UNDEF = 0,
	TYPE_MELEE_WEAPON = (1 << 0), // 1
	TYPE_ARMOR = (1 << 1), // 2
	TYPE_CLOTHING = (1 << 2), // 4
	TYPE_JEWELRY = (1 << 3), // 8
	TYPE_CREATURE = (1 << 4), // 0x10
	TYPE_FOOD = (1 << 5), // 0x20
	TYPE_MONEY = (1 << 6), // x40
	TYPE_MISC = (1 << 7), // 0x80
	TYPE_MISSILE_WEAPON = (1 << 8), // 0x100
	TYPE_CONTAINER = (1 << 9), // 0x200
	TYPE_USELESS = (1 << 10), // 0x400
	TYPE_GEM = (1 << 11), // 0x800
	TYPE_SPELL_COMPONENTS = (1 << 12), // 0x1000
	TYPE_WRITABLE = (1 << 13),
	TYPE_KEY = (1 << 14),
	TYPE_CASTER = (1 << 15),
	TYPE_PORTAL = (1 << 16), // 0x10000
	TYPE_LOCKABLE = (1 << 17),
	TYPE_PROMISSORY_NOTE = (1 << 18),
	TYPE_MANASTONE = (1 << 19),
	TYPE_SERVICE = (1 << 20), // 0x100000
	TYPE_MAGIC_WIELDABLE = (1 << 21),
	TYPE_CRAFT_COOKING_BASE = (1 << 22),
	TYPE_CRAFT_ALCHEMY_BASE = (1 << 23),
	// NOTE: Skip 1
	TYPE_CRAFT_FLETCHING_BASE = (1 << 25), // 0x2000000
	TYPE_CRAFT_ALCHEMY_INTERMEDIATE = (1 << 26),
	TYPE_CRAFT_FLETCHING_INTERMEDIATE = (1 << 27),
	TYPE_LIFESTONE = (1 << 28), // 0x10000000
	TYPE_TINKERING_TOOL = (1 << 29),
	TYPE_TINKERING_MATERIAL = (1 << 30),
	TYPE_GAMEBOARD = (1 << 31),
	TYPE_PORTAL_MAGIC_TARGET = 268500992,
	TYPE_LOCKABLE_MAGIC_TARGET = 640,
	TYPE_VESTEMENTS = 6,
	TYPE_WEAPON = 257,
	TYPE_WEAPON_OR_CASTER = 33025,
	TYPE_ITEM = 3013615,
	TYPE_REDIRECTABLE_ITEM_ENCHANTMENT_TARGET = 33031,
	TYPE_ITEM_ENCHANTABLE_TARGET = 560015,
	TYPE_SELF = 0,
	TYPE_VENDOR_SHOPKEEP = 1208248231,
	TYPE_VENDOR_GROCER = 4481568
};

enum PhysicsState {
	STATIC_PS = (1 << 0), // 0x1
	UNUSED1_PS = (1 << 1), // 0x2
	ETHEREAL_PS = (1 << 2), // 0x4
	REPORT_COLLISIONS_PS = (1 << 3), // 0x8
	IGNORE_COLLISIONS_PS = (1 << 4), // 0x10
	NODRAW_PS = (1 << 5), // 0x20
	MISSILE_PS = (1 << 6), // 0x40
	PUSHABLE_PS = (1 << 7), // 0x80
	ALIGNPATH_PS = (1 << 8), // 0x100
	PATHCLIPPED_PS = (1 << 9), // 0x200
	GRAVITY_PS = (1 << 10), // 0x400
	LIGHTING_ON_PS = (1 << 11), // 0x800
	PARTICLE_EMITTER_PS = (1 << 12), // 0x1000
	UNNUSED2_PS = (1 << 13), // 0x2000
	HIDDEN_PS = (1 << 14), // 0x4000
	SCRIPTED_COLLISION_PS = (1 << 15), // 0x8000
	HAS_PHYSICS_BSP_PS = (1 << 16), // 0x10000
	INELASTIC_PS = (1 << 17), // 0x20000 
	HAS_DEFAULT_ANIM_PS = (1 << 18), // 0x40000
	HAS_DEFAULT_SCRIPT_PS = (1 << 19), // 0x80000
	CLOAKED_PS = (1 << 20), // 0x100000
	REPORT_COLLISIONS_AS_ENVIRONMENT_PS = (1 << 21), // 0x200000
	EDGE_SLIDE_PS = (1 << 22), // 0x400000
	SLEDDING_PS = (1 << 23), // 0x800000
	FROZEN_PS = (1 << 24) // 0x1000000
};

enum TransientState
{
	CONTACT_TS = 1, // 0x1
	ON_WALKABLE_TS = 2, // 0x2
	SLIDING_TS = 4, // 0x4
	WATER_CONTACT_TS = 8, // 0x8
	STATIONARY_FALL_TS = 16, // 0x10
	STATIONARY_STOP_TS = 32, // 0x20
	STATIONARY_STUCK_TS = 64, // 0x40
	ACTIVE_TS = 128, // 0x80
	CHECK_ETHEREAL_TS = 256 // 0x100
};

enum MovementTypes
{
	Invalid = 0,
	RawCommand = 1,
	InterpretedCommand = 2,
	StopRawCommand = 3,
	StopInterpretedCommand = 4,
	StopCompletely = 5,
	MoveToObject = 6,
	MoveToPosition = 7,
	TurnToObject = 8,
	TurnToHeading = 9
};

enum PhysicsTimeStamp
{
	POSITION_TS = 0,
	MOVEMENT_TS = 1,
	STATE_TS = 2,
	VECTOR_TS = 3,
	TELEPORT_TS = 4,
	SERVER_CONTROLLED_MOVE_TS = 5,
	FORCE_POSITION_TS = 6,
	OBJDESC_TS = 7,
	INSTANCE_TS = 8,
	NUM_PHYSICS_TS = 9
};

enum BitfieldIndex {
	BF_OPENABLE = (1 << 0), // 1
	BF_INSCRIBABLE = (1 << 1), // 2
	BF_STUCK = (1 << 2), // 4
	BF_PLAYER = (1 << 3), // 8
	BF_ATTACKABLE = (1 << 4), // 0x10
	BF_PLAYER_KILLER = (1 << 5), // 0x20
	BF_HIDDEN_ADMIN = (1 << 6), // 0x40
	BF_UI_HIDDEN = (1 << 7), // 0x80
	BF_BOOK = (1 << 8), // 0x100
	BF_VENDOR = (1 << 9), // 0x200
	BF_PKSWITCH = (1 << 10), // 0x400
	BF_NPKSWITCH = (1 << 11), // 0x800
	BF_DOOR = (1 << 12), // 0x1000
	BF_CORPSE = (1 << 13), // 0x2000
	BF_LIFESTONE = (1 << 14), // 0x4000
	BF_FOOD = (1 << 15), // 0x8000
	BF_HEALER = (1 << 16), // 0x10000
	BF_LOCKPICK = (1 << 17), // 0x20000
	BF_PORTAL = (1 << 18), // 0x40000
	BF_ADMIN = (1 << 20), // 0x100000
	BF_FREE_PKSTATUS = (1 << 21), // 0x200000
	BF_IMMUNE_CELL_RESTRICTIONS = (1 << 22), // 0x400000
	BF_REQUIRES_PACKSLOT = (1 << 23), // 0x800000
	BF_RETAINED = (1 << 24), // 0x1000000
	BF_PKLITE_PKSTATUS = (1 << 25), // 0x2000000
	BF_INCLUDES_SECOND_HEADER = (1 << 26), // 0x4000000
	BF_BINDSTONE = (1 << 27), // 0x8000000
	BF_VOLATILE_RARE = (1 << 28), // 0x10000000
	BF_WIELD_ON_USE = (1 << 29), // 0x20000000
	BF_WIELD_LEFT = (1 << 30), // 0x40000000
};

enum PhysicsDescInfo {
	CSETUP = (1 << 0), // 1
	MTABLE = (1 << 1), // 2
	VELOCITY = (1 << 2), // 4
	ACCELERATION = (1 << 3), // 8
	OMEGA = (1 << 4), // 0x10
	PARENT = (1 << 5), // 0x20
	CHILDREN = (1 << 6), // 0x40
	OBJSCALE = (1 << 7), // 0x80
	FRICTION = (1 << 8), // 0x100
	ELASTICITY = (1 << 9), // 0x200
	TIMESTAMPS = (1 << 10), // 0x400
	STABLE = (1 << 11), // 0x800
	PETABLE = (1 << 12), // 0x1000
	DEFAULT_SCRIPT = (1 << 13), // 0x2000
	DEFAULT_SCRIPT_INTENSITY = (1 << 14), // 0x4000
	POSITION = (1 << 15), // 0x8000
	MOVEMENT = (1 << 16), // 0x10000
	ANIMFRAME_ID = (1 << 17), // 0x20000
	TRANSLUCENCY = (1 << 18) // 0x40000
};

enum PublicWeenieDescPackHeader {
	PWD_Packed_None = 0,
	PWD_Packed_PluralName = (1 << 0), // 1
	PWD_Packed_ItemsCapacity = (1 << 1), // 2
	PWD_Packed_ContainersCapacity = (1 << 2), // 4
	PWD_Packed_Value = (1 << 3), // 8
	PWD_Packed_Useability = (1 << 4), // 0x10
	PWD_Packed_UseRadius = (1 << 5), // 0x20
	PWD_Packed_Monarch = (1 << 6), // 0x40
	PWD_Packed_UIEffects = (1 << 7), // 0x80
	PWD_Packed_AmmoType = (1 << 8),  // 0x100
	PWD_Packed_CombatUse = (1 << 9), // 0x200
	PWD_Packed_Structure = (1 << 10), // 0x400
	PWD_Packed_MaxStructure = (1 << 11), // 0x800
	PWD_Packed_StackSize = (1 << 12),  // 0x1000
	PWD_Packed_MaxStackSize = (1 << 13), // 0x2000
	PWD_Packed_ContainerID = (1 << 14), // 0x4000
	PWD_Packed_WielderID = (1 << 15), // 0x8000
	PWD_Packed_ValidLocations = (1 << 16),  // 0x10000
	PWD_Packed_Location = (1 << 17), // 0x20000
	PWD_Packed_Priority = (1 << 18), // 0x40000
	PWD_Packed_TargetType = (1 << 19), // 0x80000
	PWD_Packed_BlipColor = (1 << 20),  // 0x100000
	PWD_Packed_Burden = (1 << 21), // 0x200000  // NOTE: May be PWD_Packed_VendorClassID
	PWD_Packed_SpellID = (1 << 22), // 0x400000
	PWD_Packed_RadarEnum = (1 << 23), // 0x800000 // NOTE: May be PWD_Packed_RadarDistance
	PWD_Packed_Workmanship = (1 << 24), // 0x1000000
	PWD_Packed_HouseOwner = (1 << 25), // 0x2000000
	PWD_Packed_HouseRestrictions = (1 << 26), // 0x4000000
	PWD_Packed_PScript = (1 << 27), // 0x8000000
	PWD_Packed_HookType = (1 << 28), // 0x10000000
	PWD_Packed_HookItemTypes = (1 << 29), // 0x20000000
	PWD_Packed_IconOverlay = (1 << 30), // 0x40000000
	PWD_Packed_MaterialType = (1 << 31) // 0x80000000
};

enum PublicWeenieDescPackHeader2 {
	PWD2_Packed_None = 0,
	PWD2_Packed_IconUnderlay = (1 << 0),
	PWD2_Packed_CooldownID = (1 << 1),
	PWD2_Packed_CooldownDuration = (1 << 2),
	PWD2_Packed_PetOwner = (1 << 3),
};


enum PScriptType {
	PS_Invalid,
	PS_Test1,
	PS_Test2,
	PS_Test3,
	PS_Launch,
	PS_Explode,
	PS_AttribUpRed,
	PS_AttribDownRed,
	PS_AttribUpOrange,
	PS_AttribDownOrange,
	PS_AttribUpYellow,
	PS_AttribDownYellow,
	PS_AttribUpGreen,
	PS_AttribDownGreen,
	PS_AttribUpBlue,
	PS_AttribDownBlue,
	PS_AttribUpPurple,
	PS_AttribDownPurple,
	PS_SkillUpRed,
	PS_SkillDownRed,
	PS_SkillUpOrange,
	PS_SkillDownOrange,
	PS_SkillUpYellow,
	PS_SkillDownYellow,
	PS_SkillUpGreen,
	PS_SkillDownGreen,
	PS_SkillUpBlue,
	PS_SkillDownBlue,
	PS_SkillUpPurple,
	PS_SkillDownPurple,
	PS_SkillDownBlack,
	PS_HealthUpRed,
	PS_HealthDownRed,
	PS_HealthUpBlue,
	PS_HealthDownBlue,
	PS_HealthUpYellow,
	PS_HealthDownYellow,
	PS_RegenUpRed,
	PS_RegenDownREd,
	PS_RegenUpBlue,
	PS_RegenDownBlue,
	PS_RegenUpYellow,
	PS_RegenDownYellow,
	PS_ShieldUpRed,
	PS_ShieldDownRed,
	PS_ShieldUpOrange,
	PS_ShieldDownOrange,
	PS_ShieldUpYellow,
	PS_ShieldDownYellow,
	PS_ShieldUpGreen,
	PS_ShieldDownGreen,
	PS_ShieldUpBlue,
	PS_ShieldDownBlue,
	PS_ShieldUpPurple,
	PS_ShieldDownPurple,
	PS_ShieldUpGrey,
	PS_ShieldDownGrey,
	PS_EnchantUpRed,
	PS_EnchantDownRed,
	PS_EnchantUpOrange,
	PS_EnchantDownOrange,
	PS_EnchantUpYellow,
	PS_EnchantDownYellow,
	PS_EnchantUpGreen,
	PS_EnchantDownGreen,
	PS_EnchantUpBlue,
	PS_EnchantDownBlue,
	PS_EnchantUpPurple,
	PS_EnchantDownPurple,
	PS_VitaeUpWhite,
	PS_VitaeDownBlack,
	PS_VisionUpWhite,
	PS_VisionDownBlack,
	PS_SwapHealth_Red_To_Yellow,
	PS_SwapHealth_Red_To_Blue,
	PS_SwapHealth_Yellow_To_Red,
	PS_SwapHealth_Yellow_To_Blue,
	PS_SwapHealth_Blue_To_Red,
	PS_SwapHealth_Blue_To_Yellow,
	PS_TransUpWhite,
	PS_TransDownBlack,
	PS_Fizzle,
	PS_PortalEntry,
	PS_PortalExit,
	PS_BreatheFlame,
	PS_BreatheFrost,
	PS_BreatheAcid,
	PS_BreatheLightning,
	PS_Create,
	PS_Destroy,
	PS_ProjectileCollision,
	PS_SplatterLowLeftBack,
	PS_SplatterLowLeftFront,
	PS_SplatterLowRightBack,
	PS_SplatterLowRightFront,
	PS_SplatterMidLeftBack,
	PS_SplatterMidLeftFront,
	PS_SplatterMidRightBack,
	PS_SplatterMidRightFront,
	PS_SplatterUpLeftBack,
	PS_SplatterUpLeftFront,
	PS_SplatterUpRightBack,
	PS_SplatterUpRightFront,
	PS_SparkLowLeftBack,
	PS_SparkLowLeftFront,
	PS_SparkLowRightBack,
	PS_SparkLowRightFront,
	PS_SparkMidLeftBack,
	PS_SparkMidLeftFront,
	PS_SparkMidRightBack,
	PS_SparkMidRightFront,
	PS_SparkUpLeftBack,
	PS_SparkUpLeftFront,
	PS_SparkUpRightBack,
	PS_SparkUpRightFront,
	PS_PortalStorm,
	PS_Hide,
	PS_UnHide,
	PS_Hidden,
	PS_DisappearDestroy,
	SpecialState1,
	SpecialState2,
	SpecialState3,
	SpecialState4,
	SpecialState5,
	SpecialState6,
	SpecialState7,
	SpecialState8,
	SpecialState9,
	SpecialState0,
	SpecialStateRed,
	SpecialStateOrange,
	SpecialStateYellow,
	SpecialStateGreen,
	SpecialStateBlue,
	SpecialStatePurple,
	SpecialStateWhite,
	SpecialStateBlack,
	PS_LevelUp,
	PS_EnchantUpGrey,
	PS_EnchantDownGrey,
	PS_WeddingBliss,
	PS_EnchantUpWhite,
	PS_EnchantDownWhite,
	PS_CampingMastery,
	PS_CampingIneptitude,
	PS_DispelLife,
	PS_DispelCreature,
	PS_DispelAll,
	PS_BunnySmite,
	PS_BaelZharonSmite,
	PS_WeddingSteele,
	PS_RestrictionEffectBlue,
	PS_RestrictionEffectGreen,
	PS_RestrictionEffectGold,
	PS_LayingofHands,
	PS_AugmentationUseAttribute,
	PS_AugmentationUseSkill,
	PS_AugmentationUseResistances,
	PS_AugmentationUseOther,
	PS_BlackMadness,
	PS_AetheriaLevelUp,
	PS_AetheriaSurgeDestruction,
	PS_AetheriaSurgeProtection,
	PS_AetheriaSurgeRegeneration,
	PS_AetheriaSurgeAffliction,
	PS_AetheriaSurgeFestering,
	PS_HealthDownVoid,
	PS_RegenDownVoid,
	PS_SkillDownVoid,
	PS_DirtyFightingHealDebuff,
	PS_DirtyFightingAttackDebuff,
	PS_DirtyFightingDefenseDebuff,
	PS_DirtyFightingDamageOverTime,
	NUM_PSCRIPT_TYPES
};

enum AMMO_TYPE {
	AMMO_NONE,
	AMMO_ARROW,
	AMMO_BOLT,
	AMMO_ATLATL,
	AMMO_ARROW_CRYSTAL,
	AMMO_BOLT_CRYSTAL,
	AMMO_ATLATL_CRYSTAL,
	AMMO_ARROW_CHORIZITE,
	AMMO_BOLT_CHORIZITE,
	AMMO_ATLATL_CHORIZITE
};


enum MaterialType {
	Undef_MaterialType,
	Ceramic_MaterialType,
	Porcelain_MaterialType,
	Cloth_MaterialType,
	Linen_MaterialType,
	Satin_MaterialType,
	Silk_MaterialType,
	Velvet_MaterialType,
	Wool_MaterialType,
	Gem_MaterialType,
	Agate_MaterialType,
	Amber_MaterialType,
	Amethyst_MaterialType,
	Aquamarine_MaterialType,
	Azurite_MaterialType,
	Black_Garnet_MaterialType,
	Black_Opal_MaterialType,
	Bloodstone_MaterialType,
	Carnelian_MaterialType,
	Citrine_MaterialType,
	Diamond_MaterialType,
	Emerald_MaterialType,
	Fire_Opal_MaterialType,
	Green_Garnet_MaterialType,
	Green_Jade_MaterialType,
	Hematite_MaterialType,
	Imperial_Topaz_MaterialType,
	Jet_MaterialType,
	Lapis_Lazuli_MaterialType,
	Lavender_Jade_MaterialType,
	Malachite_MaterialType,
	Moonstone_MaterialType,
	Onyx_MaterialType,
	Opal_MaterialType,
	Peridot_MaterialType,
	Red_Garnet_MaterialType,
	Red_Jade_MaterialType,
	Rose_Quartz_MaterialType,
	Ruby_MaterialType,
	Sapphire_MaterialType,
	Smoky_Quartz_MaterialType,
	Sunstone_MaterialType,
	Tiger_Eye_MaterialType,
	Tourmaline_MaterialType,
	Turquoise_MaterialType,
	White_Jade_MaterialType,
	White_Quartz_MaterialType,
	White_Sapphire_MaterialType,
	Yellow_Garnet_MaterialType,
	Yellow_Topaz_MaterialType,
	Zircon_MaterialType,
	Ivory_MaterialType,
	Leather_MaterialType,
	Armoredillo_Hide_MaterialType,
	Gromnie_Hide_MaterialType,
	Reed_Shark_Hide_MaterialType,
	Metal_MaterialType,
	Brass_MaterialType,
	Bronze_MaterialType,
	Copper_MaterialType,
	Gold_MaterialType,
	Iron_MaterialType,
	Pyreal_MaterialType,
	Silver_MaterialType,
	Steel_MaterialType,
	Stone_MaterialType,
	Alabaster_MaterialType,
	Granite_MaterialType,
	Marble_MaterialType,
	Obsidian_MaterialType,
	Sandstone_MaterialType,
	Serpentine_MaterialType,
	Wood_MaterialType,
	Ebony_MaterialType,
	Mahogany_MaterialType,
	Oak_MaterialType,
	Pine_MaterialType,
	Teak_MaterialType,
	Number_MaterialType = Teak_MaterialType,
	NumMaterialTypes_MaterialType = 238
};

enum CreatureType
{
	none = 0,
	olthoi = 1,
	banderling = 2,
	drudge = 3,
	mosswart = 4,
	lugian = 5,
	tumerok = 6,
	mite = 7,
	tusker = 8,
	phyntosWasp = 9,
	rat = 10,
	auroch = 11,
	cow = 12,
	golem = 13,
	undead = 14,
	gromnie = 15,
	reedshark = 16,
	armoredillo = 17,
	fae = 18,
	virindi = 19,
	wisp = 20,
	knathtaed = 21,
	shadow = 22,
	mattekar = 23,
	mumiyah = 24,
	rabbit = 25,
	sclavus = 26,
	shallowsShark = 27,
	monouga = 28,
	zefir = 29,
	skeleton = 30,
	human = 31,
	shreth = 32,
	chittick = 33,
	moarsman = 34,
	slithis = 36,
	fireElemental = 38,
	snowman = 39,
	bunny = 41,
	lightningElemental = 42,
	grievver = 44,
	niffis = 45,
	ursuin = 46,
	crystal = 47,
	hollowMinion = 48,
	scarecrow = 49,
	idol = 50,
	empyrean = 51,
	hopeslayer = 52,
	doll = 53,
	marionette = 54,
	carenzi = 55,
	siraluun = 56,
	aunTumerok = 57,
	heaTumerok = 58,
	simulacrum = 59,
	acidElemental = 60,
	frostElemental = 61,
	elemental = 62,
	statue = 63,
	wall = 64,
	alteredHuman = 65,
	device = 66,
	harbinger = 67,
	darkSarcophagus = 68,
	chicken = 69,
	gotrokLugian = 70,
	margul = 71,
	bleachedRabbit = 72,
	nastyRabbit = 73,
	grimacingRabbit = 74,
	burun = 75,
	target = 76,
	ghost = 77,
	fiun = 78,
	eater = 79,
	penguin = 80,
	ruschk = 81,
	thrungus = 82,
	viamontianKnight = 83,
	remoran = 84,
	swarm = 85,
	moar = 86,
	enchantedArms = 87,
	sleech = 88,
	mukkir = 89,
	merwart = 90,
	food = 91,
	paradoxOlthoi = 92,
	energy = 94,
	apparition = 95,
	aerbax = 96,
	touched = 97,
	blightedMoarsman = 98,
	gearKnight = 99,
	gurog = 100,
	anekshay = 101
};

enum SKILL_ADVANCEMENT_CLASS
{
	UNDEF_SKILL_ADVANCEMENT_CLASS = 0x0,
	UNTRAINED_SKILL_ADVANCEMENT_CLASS = 0x1,
	TRAINED_SKILL_ADVANCEMENT_CLASS = 0x2,
	SPECIALIZED_SKILL_ADVANCEMENT_CLASS = 0x3,
	NUM_SKILL_ADVANCEMENT_CLASSES = 0x4,
	FORCE_SKILL_ADVANCEMENT_CLASS_32_BIT = 0x7FFFFFFF,
};

enum HoldKey
{
	HoldKey_Invalid = 0,
	HoldKey_None = 1,
	HoldKey_Run = 2,
	Num_HoldKeys = 3
};

enum TransitionState
{
	INVALID_TS = 0x0,
	OK_TS = 0x1,
	COLLIDED_TS = 0x2,
	ADJUSTED_TS = 0x3,
	SLID_TS = 0x4,
	FORCE_TransitionState_32_BIT = 0x7FFFFFFF,
};

typedef unsigned int LocationMask;
typedef unsigned int LocationId;

enum INVENTORY_LOC
{
	NONE_LOC = 0x0,
	HEAD_WEAR_LOC = 0x1,
	CHEST_WEAR_LOC = 0x2,
	ABDOMEN_WEAR_LOC = 0x4,
	UPPER_ARM_WEAR_LOC = 0x8,
	LOWER_ARM_WEAR_LOC = 0x10,
	HAND_WEAR_LOC = 0x20,
	UPPER_LEG_WEAR_LOC = 0x40,
	LOWER_LEG_WEAR_LOC = 0x80,
	FOOT_WEAR_LOC = 0x100,
	FOOT_LOWERLEG_BOOTS_LOC = 0x180,
	CHEST_ARMOR_LOC = 0x200,
	ABDOMEN_ARMOR_LOC = 0x400,
	UPPER_ARM_ARMOR_LOC = 0x800,
	LOWER_ARM_ARMOR_LOC = 0x1000,
	UPPER_LEG_ARMOR_LOC = 0x2000,
	LOWER_LEG_ARMOR_LOC = 0x4000,
	NECK_WEAR_LOC = 0x8000,
	WRIST_WEAR_LEFT_LOC = 0x10000,
	WRIST_WEAR_RIGHT_LOC = 0x20000,
	FINGER_WEAR_LEFT_LOC = 0x40000,
	FINGER_WEAR_RIGHT_LOC = 0x80000,
	MELEE_WEAPON_LOC = 0x100000,
	SHIELD_LOC = 0x200000,
	MISSILE_WEAPON_LOC = 0x400000,
	MISSILE_AMMO_LOC = 0x800000,
	HELD_LOC = 0x1000000,
	TWO_HANDED_LOC = 0x2000000,
	TRINKET_ONE_LOC = 0x4000000,
	CLOAK_LOC = 0x8000000,
	SIGIL_ONE_LOC = 0x10000000,
	SIGIL_TWO_LOC = 0x20000000,
	SIGIL_THREE_LOC = 0x40000000,
	CLOTHING_LOC = 0x80001FF,
	ARMOR_LOC = 0x7E00,
	JEWELRY_LOC = 0x7C0F8000,
	WRIST_WEAR_LOC = 0x30000,
	FINGER_WEAR_LOC = 0xC0000,
	SIGIL_LOC = 0x70000000,
	READY_SLOT_LOC = 0x3F00000,
	WEAPON_LOC = 0x2500000,
	WEAPON_READY_SLOT_LOC = 0x3500000,
	CHEST_ABS_LOC = 0x600,
	CHEST_UPPERARM_LOC = 0xA00,
	CHEST_UPPERARM_ABS_LOC = 0xE00,
	UPPERARM_LOWERARM_LOC = 0x1800,
	CHEST_UPPERARM_LOWERARM_LOC = 0x1A00,
	CHEST_UPPERARM_LOWERARM_ABS_LOC = 0x1E00,
	ABS_UPPERLEG_LOC = 0x2400,
	LOWERLEG_FOOT_LOC = 0x4100,
	UPPERLEG_LOWERLEG_LOC = 0x6000,
	ABS_UPPERLEG_LOWERLEG_LOC = 0x6400,
	ALL_LOC = 0x7FFFFFFF,
	CAN_GO_IN_READY_SLOT_LOC = 0x7FFFFFFF,
	FORCE_INVENTORY_LOC_32_BIT = 0x7FFFFFFF,
};

enum PKStatusEnum
{
	Undef_PKStatus = 0x0,
	Protected_PKStatus = 0x1,
	NPK_PKStatus = 0x2,
	PK_PKStatus = 0x4,
	Unprotected_PKStatus = 0x8,
	RubberGlue_PKStatus = 0x10,
	Free_PKStatus = 0x20,
	PKLite_PKStatus = 0x40,
	Creature_PKStatus = 0x8,
	Trap_PKStatus = 0x8,
	NPC_PKStatus = 0x1,
	Vendor_PKStatus = 0x10,
	Baelzharon_PKStatus = 0x20,
	FORCE_PKStatusEnum_32_BIT = 0x7FFFFFFF,
};

enum CommandMasks
{
	CM_Style = 0x80000000,
	CM_SubState = 0x40000000,
	CM_Modifier = 0x20000000,
	CM_Action = 0x10000000,
	CM_UI = 0x08000000,
	CM_Toggle = 0x04000000,
	CM_ChatEmote = 0x02000000,
	CM_Mappable = 0x01000000,
	CM_Command = ~(CM_Style | CM_SubState | CM_Modifier | CM_Action | CM_UI | CM_Toggle | CM_ChatEmote | CM_Mappable)
};

enum BoundingType
{
	OUTSIDE = 0x0,
	PARTIALLY_INSIDE = 0x1,
	ENTIRELY_INSIDE = 0x2,
	FORCE_BoundingType_32_BIT = 0x7FFFFFFF,
};

enum Sidedness
{
	POSITIVE = 0x0,
	NEGATIVE = 0x1,
	IN_PLANE = 0x2,
	CROSSING = 0x3,
};

enum SURFCHAR
{
	SOLID = 0x0,
	WATER = 0x1,
	FORCE_SURFCHAR_32_BIT = 0x7FFFFFFF,
};

enum eCombatMode
{
	eCombatModeUndef = 0x0,
	eCombatModeNonCombat = 0x1,
	eCombatModeMelee = 0x2,
	eCombatModeMissile = 0x4,
	eCombatModeMagic = 0x8,
};

enum COMBAT_MODE
{
	UNDEF_COMBAT_MODE = 0x0,
	NONCOMBAT_COMBAT_MODE = 0x1,
	MELEE_COMBAT_MODE = 0x2,
	MISSILE_COMBAT_MODE = 0x4,
	MAGIC_COMBAT_MODE = 0x8,
	VALID_COMBAT_MODES = 0xF,
	COMBAT_COMBAT_MODE = 0xE,
	FORCE_COMBAT_MODE_32_BIT = 0x7FFFFFFF,
};

enum ObjCollisionProfile_Bitfield
{
	Undef_OCPB = 0x0,
	Creature_OCPB = 0x1,
	Player_OCPB = 0x2,
	Attackable_OCPB = 0x4,
	Missile_OCPB = 0x8,
	Contact_OCPB = 0x10,
	MyContact_OCPB = 0x20,
	Door_OCPB = 0x40,
	Cloaked_OCPB = 0x80,
	FORCE_ObjCollisionProfile_Bitfield_32_BIT = 0x7FFFFFFF,
};

enum WErrorType
{
	WERROR_NONE = 0,
	WERROR_NOMEM,
	WERROR_BAD_PARAM,
	WERROR_DIV_ZERO,
	WERROR_SEGV,
	WERROR_UNIMPLEMENTED,
	WERROR_UNKNOWN_MESSAGE_TYPE,
	WERROR_NO_ANIMATION_TABLE,
	WERROR_NO_PHYSICS_OBJECT,
	WERROR_NO_BOOKIE_OBJECT,
	WERROR_NO_WSL_OBJECT,
	WERROR_NO_MOTION_INTERPRETER,
	WERROR_UNHANDLED_SWITCH,
	WERROR_DEFAULT_CONSTRUCTOR_CALLED,
	WERROR_INVALID_COMBAT_MANEUVER,
	WERROR_BAD_CAST,
	WERROR_MISSING_QUALITY,
	WERROR_MISSING_DATABASE_OBJECT = 18,
	WERROR_NO_CALLBACK_SET,
	WERROR_CORRUPT_QUALITY,
	WERROR_BAD_CONTEXT,
	WERROR_NO_EPHSEQ_MANAGER,
	WERROR_BAD_MOVEMENT_EVENT, // You failed to go to non-combat mode.
	WERROR_CANNOT_CREATE_NEW_OBJECT,
	WERROR_NO_CONTROLLER_OBJECT,
	WERROR_CANNOT_SEND_EVENT,
	WERROR_PHYSICS_CANT_TRANSITION,
	WERROR_PHYSICS_MAX_DISTANCE_EXCEEDED,
	WERROR_ACTIONS_LOCKED, // You're too busy!
	WERROR_EXTERNAL_ACTIONS_LOCKED, //  is too busy to accept gifts right now.\n
	WERROR_CANNOT_SEND_MESSAGE,
	WERROR_ILLEGAL_INVENTORY_TRANSACTION, // You must control both objects!
	WERROR_EXTERNAL_WEENIE_OBJECT,
	WERROR_INTERNAL_WEENIE_OBJECT,
	WERROR_MOTION_FAILURE, // Unable to move to object!
	WERROR_NO_CONTACT, // You can't jump while in the air
	WERROR_INQ_CYL_SPHERE_FAILURE,
	WERROR_BAD_COMMAND, // That is not a valid command.
	WERROR_CARRYING_ITEM,
	WERROR_FROZEN, // The item is under someone else's control!
	WERROR_STUCK, // You cannot pick that up!
	WERROR_OVERLOAD, // You are too encumbered to carry that!
	WERROR_EXTERNAL_OVERLOAD, //  cannot carry anymore.\n
	WERROR_BAD_CONTAIN,
	WERROR_BAD_PARENT,
	WERROR_BAD_DROP,
	WERROR_BAD_RELEASE,
	WERROR_MSG_BAD_MSG,
	WERROR_MSG_UNPACK_FAILED,
	WERROR_MSG_NO_MSG,
	WERROR_MSG_UNDERFLOW,
	WERROR_MSG_OVERFLOW,
	WERROR_MSG_CALLBACK_FAILED,
	WERROR_INTERRUPTED, // Action cancelled!
	WERROR_OBJECT_GONE, // Unable to move to object!
	WERROR_NO_OBJECT, // Unable to move to object!
	WERROR_CANT_GET_THERE, // Unable to move to object!
	WERROR_DEAD, // You can't do that... you're dead!
	WERROR_I_LEFT_THE_WORLD,
	WERROR_I_TELEPORTED,
	WERROR_TOO_FAR, // You charged too far!
	WERROR_STAMINA_TOO_LOW, // You are too tired to do that!
	WERROR_CANT_CROUCH_IN_COMBAT,
	WERROR_CANT_SIT_IN_COMBAT,
	WERROR_CANT_LIE_DOWN_IN_COMBAT,
	WERROR_CANT_CHAT_EMOTE_IN_COMBAT,
	WERROR_NO_MTABLE_DATA,
	WERROR_CANT_CHAT_EMOTE_NOT_STANDING,
	WERROR_TOO_MANY_ACTIONS,
	WERROR_HIDDEN,
	WERROR_GENERAL_MOVEMENT_FAILURE,
	WERROR_CANT_JUMP_POSITION, // You can't jump from this position
	WERROR_CANT_JUMP_LOAD, // You're too loaded down to jump
	WERROR_SELF_INFLICTED_DEATH, // Ack! You killed yourself!\n
	WERROR_MSG_RESPONSE_FAILURE,
	WERROR_OBJECT_IS_STATIC,
	WERROR_PK_INVALID_STATUS, // Invalid PK status!
	WERROR_PK_PROTECTED_ATTACKER, // You fail to affect %s because you cannot affect anyone!\n
	WERROR_PK_PROTECTED_TARGET, // You fail to affect %s because $s cannot be harmed!\n
	WERROR_PK_UNPROTECTED_TARGET, // You fail to affect %s because beneficial spells do not affect %s!\n
	WERROR_PK_NPK_ATTACKER, // You fail to affect %s because you are not a player killer!\n
	WERROR_PK_NPK_TARGET, // You fail to affect %s because %s is not a player killer!\n
	WERROR_PK_WRONG_KIND, // You fail to affect %s because you are not the same sort of player killer as %s!\n
	WERROR_PK_CROSS_HOUSE_BOUNDARY, // You fail to affect %s because you are acting across a house boundary!\n
	WERROR_INVALID_XP_AMOUNT = 1001,
	WERROR_INVALID_PP_CALCULATION,
	WERROR_INVALID_CP_CALCULATION,
	WERROR_UNHANDLED_STAT_ANSWER,
	WERROR_HEART_ATTACK,
	WERROR_CLOSED, // The container is closed!
	WERROR_GIVE_NOT_ALLOWED, //  is not accepting gifts right now.\n
	WERROR_CHANGE_COMBAT_MODE_FAILURE, // You failed to go to non-combat mode.
	WERROR_INVALID_INVENTORY_LOCATION,
	WERROR_FULL_INVENTORY_LOCATION,
	WERROR_CONFLICTING_INVENTORY_LOCATION,
	WERROR_ITEM_NOT_PENDING,
	WERROR_BE_WIELDED_FAILURE,
	WERROR_BE_DROPPED_FAILURE,
	WERROR_COMBAT_FATIGUE, // You are too fatigued to attack!
	WERROR_COMBAT_OUT_OF_AMMO, // You are out of ammunition!
	WERROR_COMBAT_MISFIRE, // Your missile attack misfired!
	WERROR_BAD_MISSILE_CALCULATIONS, // You've attempted an impossible spell path!
	WERROR_MAGIC_INCOMPLETE_ANIM_LIST,
	WERROR_MAGIC_INVALID_SPELL_TYPE,
	WERROR_MAGIC_INQ_POSITION_AND_VELOCITY_FAILURE,
	WERROR_MAGIC_UNLEARNED_SPELL, // You don't know that spell!
	WERROR_MAGIC_BAD_TARGET_TYPE, // Incorrect target type
	WERROR_MAGIC_MISSING_COMPONENTS, // You don't have all the components for this spell.
	WERROR_MAGIC_INSUFFICIENT_MANA, // You don't have enough Mana to cast this spell.
	WERROR_MAGIC_FIZZLE, // Your spell fizzled.\n
	WERROR_MAGIC_MISSING_TARGET, // Your spell's target is missing!
	WERROR_MAGIC_MISFIRED_PROJECTILE_SPELL, // Your projectile spell mislaunched!
	WERROR_MAGIC_SPELLBOOK_ADDSPELL_FAILURE,
	WERROR_MAGIC_TARGET_OUT_OF_RANGE,
	WERROR_MAGIC_NON_OUTDOOR_SPELL_CAST_OUTSIDE, // Your spell cannot be cast outside
	WERROR_MAGIC_NON_INDOOR_SPELL_CAST_INSIDE, // Your spell cannot be cast inside
	WERROR_MAGIC_GENERAL_FAILURE,
	WERROR_MAGIC_UNPREPARED, // You are unprepared to cast a spell
	WERROR_ALLEGIANCE_PATRON_EXISTS, // You've already sworn your Allegiance
	WERROR_ALLEGIANCE_INSUFFICIENT_CP, // You don't have enough experience available to swear Allegiance
	WERROR_ALLEGIANCE_IGNORING_REQUESTS,
	WERROR_ALLEGIANCE_SQUELCHED,
	WERROR_ALLEGIANCE_MAX_DISTANCE_EXCEEDED,
	WERROR_ALLEGIANCE_ILLEGAL_LEVEL,
	WERROR_ALLEGIANCE_BAD_CREATION,
	WERROR_ALLEGIANCE_PATRON_BUSY,
	WERROR_ALLEGIANCE_ADD_HIERARCHY_FAILURE, // %s is already one of your followers
	WERROR_ALLEGIANCE_NONEXISTENT, // You are not in an allegiance!
	WERROR_ALLEGIANCE_REMOVE_HIERARCHY_FAILURE,
	WERROR_ALLEGIANCE_MAX_VASSALS, // %s cannot have any more Vassals
	WERROR_FELLOWSHIP_IGNORING_REQUESTS,
	WERROR_FELLOWSHIP_SQUELCHED,
	WERROR_FELLOWSHIP_MAX_DISTANCE_EXCEEDED,
	WERROR_FELLOWSHIP_MEMBER,
	WERROR_FELLOWSHIP_ILLEGAL_LEVEL,
	WERROR_FELLOWSHIP_RECRUIT_BUSY,
	WERROR_FELLOWSHIP_NOT_LEADER, // You must be the leader of a Fellowship
	WERROR_FELLOWSHIP_FULL, // Your Fellowship is full
	WERROR_FELLOWSHIP_UNCLEAN_NAME, // That Fellowship name is not permitted
	WERROR_LEVEL_TOO_LOW,
	WERROR_LEVEL_TOO_HIGH,
	WERROR_CHAN_INVALID, // That channel doesn't exist.
	WERROR_CHAN_SECURITY, // You can't use that channel.
	WERROR_CHAN_ALREADY_ACTIVE, // You're already on that channel.
	WERROR_CHAN_NOT_ACTIVE, // You're not currently on that channel.
	WERROR_ATTUNED_ITEM,
	WERROR_MERGE_BAD, // You cannot merge different stacks!
	WERROR_MERGE_ENCHANTED, // You cannot merge enchanted items!
	WERROR_UNCONTROLLED_STACK, // You must control at least one stack!
	WERROR_CURRENTLY_ATTACKING,
	WERROR_MISSILE_ATTACK_NOT_OK,
	WERROR_TARGET_NOT_ACQUIRED,
	WERROR_IMPOSSIBLE_SHOT,
	WERROR_BAD_WEAPON_SKILL,
	WERROR_UNWIELD_FAILURE,
	WERROR_LAUNCH_FAILURE,
	WERROR_RELOAD_FAILURE,
	WERROR_CRAFT_UNABLE_TO_MAKE_CRAFTREQ, // Your craft attempt fails.
	WERROR_CRAFT_ANIMATION_FAILED, // Your craft attempt fails.
	WERROR_CRAFT_NO_MATCH_WITH_NUMPREOBJ, // Given that number of items, you cannot craft anything.
	WERROR_CRAFT_GENERAL_ERROR_UI_MSG, // Your craft attempt fails.
	WERROR_CRAFT_GENERAL_ERROR_NO_UI_MSG,
	WERROR_CRAFT_FAILED_REQUIREMENTS, // Either you or one of the items involved does not pass the requirements for this craft interaction.
	WERROR_CRAFT_DONT_CONTAIN_EVERYTHING, // You do not have all the neccessary items.
	WERROR_CRAFT_ALL_OBJECTS_NOT_FROZEN, // Not all the items are available.
	WERROR_CRAFT_NOT_IN_PEACE_MODE, // You must be at rest in peace mode to do trade skills.
	WERROR_CRAFT_NOT_HAVE_SKILL, // You are not trained in that trade skill.
	WERROR_HANDS_NOT_FREE, // Your hands must be free.
	WERROR_PORTAL_NOT_LINKABLE, // You cannot link to that portal!\n
	WERROR_QUEST_SOLVED_TOO_RECENTLY, // You have solved this quest too recently!\n
	WERROR_QUEST_SOLVED_MAX_TIMES, // You have solved this quest too many times!\n
	WERROR_QUEST_UNKNOWN,
	WERROR_QUEST_TABLE_CORRUPT,
	WERROR_QUEST_BAD,
	WERROR_QUEST_DUPLICATE,
	WERROR_QUEST_UNSOLVED,
	WERROR_QUEST_RESRICTION_UNSOLVED, // This item requires you to complete a specific quest before you can pick it up!\n
	WERROR_QUEST_SOLVED_TOO_LONG_AGO,
	WERROR_TRADE_IGNORING_REQUESTS = 1100,
	WERROR_TRADE_SQUELCHED,
	WERROR_TRADE_MAX_DISTANCE_EXCEEDED,
	WERROR_TRADE_ALREADY_TRADING,
	WERROR_TRADE_BUSY,
	WERROR_TRADE_CLOSED,
	WERROR_TRADE_EXPIRED,
	WERROR_TRADE_ITEM_BEING_TRADED,
	WERROR_TRADE_NON_EMPTY_CONTAINER,
	WERROR_TRADE_NONCOMBAT_MODE,
	WERROR_TRADE_INCOMPLETE,
	WERROR_TRADE_STAMP_MISMATCH,
	WERROR_TRADE_UNOPENED,
	WERROR_TRADE_EMPTY,
	WERROR_TRADE_ALREADY_ACCEPTED,
	WERROR_TRADE_OUT_OF_SYNC,
	WERROR_PORTAL_PK_NOT_ALLOWED, // Player killers may not interact with that portal!\n
	WERROR_PORTAL_NPK_NOT_ALLOWED, // Non-player killers may not interact with that portal!\n
	WERROR_HOUSE_ABANDONED, // You do not own a house!
	WERROR_HOUSE_EVICTED, // You do not own a house!
	WERROR_HOUSE_ALREADY_OWNED,
	WERROR_HOUSE_BUY_FAILED,
	WERROR_HOUSE_RENT_FAILED,
	WERROR_HOOKED,
	WERROR_MAGIC_INVALID_POSITION = 1125,
	WERROR_PORTAL_ACDM_ONLY, // You must purchase Asheron's Call: Dark Majesty to interact with that portal.\n
	WERROR_INVALID_AMMO_TYPE,
	WERROR_SKILL_TOO_LOW,
	WERROR_HOUSE_MAX_NUMBER_HOOKS_USED, // You have used all the hooks you are allowed to use for this house.\n
	WERROR_TRADE_AI_DOESNT_WANT, //  %s doesn't know what to do with that.\n
	WERROR_HOOK_HOUSE_NOTE_OWNED,
	WERROR_PORTAL_QUEST_RESTRICTED = 1140, // You must complete a quest to interact with that portal.\n
	WERROR_HOUSE_NO_ALLEGIANCE = 1150,
	WERROR_NO_HOUSE, // You must own a house to use this command.
	WERROR_HOUSE_NO_MANSION_NO_POSITION, // Your monarch does not own a mansion or a villa!
	WERROR_HOUSE_NOT_A_MANSION, // Your monarch does not own a mansion or a villa!
	WERROR_HOUSE_NOT_ALLOWED_IN, // Your monarch has closed the mansion to the Allegiance.
	WERROR_HOUSE_UNDER_MIN_LEVEL = 1160, // You must be above level %s to purchase this dwelling.\n
	WERROR_HOUSE_OVER_MAX_LEVEL, // You must be at or below level %s to purchase this dwelling.\n
	WERROR_HOUSE_NOT_A_MONARCH, // You must be a monarch to purchase this dwelling.\n
	WERROR_HOUSE_UNDER_MIN_RANK, // You must be above allegiance rank %s to purchase this dwelling.\n
	WERROR_HOUSE_OVER_MAX_RANK, // You must be at or below allegiance rank %s to purchase this dwelling.\n
	WERROR_ALLEGIANCE_DECLINED,
	WERROR_ALLEGIANCE_TIMEOUT, // Your offer of Allegiance has been ignored.
	WERROR_CONFIRMATION_IN_PROGRESS, // You are already involved in something!
	WERROR_MONARCH_ONLY, // You must be a monarch to use this command.
	WERROR_ALLEGIANCE_BOOT_EMPTY_NAME, // You must specify a character to boot.
	WERROR_ALLEGIANCE_BOOT_SELF, // You can't boot yourself!
	WERROR_NO_SUCH_CHARACTER, // That character does not exist.
	WERROR_ALLEGIANCE_TARGET_NOT_A_MEMBER, // That person is not a member of your Allegiance!
	WERROR_ALLEGIANCE_REMOVE_NO_PATRON, // No patron from which to break!
	WERROR_ALLEGIANCE_OFFLINE_DISSOLVED, // Your Allegiance has been dissolved!\n
	WERROR_ALLEGIANCE_OFFLINE_DISMISSED, // Your patron's Allegiance to you has been broken!\n
	WERROR_MOVED_TOO_FAR, // You have moved too far!
	WERROR_TELETO_INVALID_POSITION, // That is not a valid destination!
	WERROR_ACDM_ONLY, // You must purchase Asheron's Call -- Dark Majesty to use this function.
	WERROR_LIFESTONE_LINK_FAILED, // You fail to link with the lifestone!\n
	WERROR_LIFESTONE_LINK_TOO_FAR, // You wandered too far to link with the lifestone!\n
	WERROR_LIFESTONE_LINK_SUCCESS, // You successfully link with the lifestone!\n
	WERROR_LIFESTONE_RECALL_NO_LINK, // You must have linked with a lifestone in order to recall to it!\n
	WERROR_LIFESTONE_RECALL_FAILED, // You fail to recall to the lifestone!\n
	WERROR_PORTAL_LINK_FAILED, // You fail to link with the portal!\n
	WERROR_PORTAL_LINK_SUCCESS, // You successfully link with the portal!\n
	WERROR_PORTAL_RECALL_FAILED, // You fail to recall to the portal!\n
	WERROR_PORTAL_RECALL_NO_LINK, // You must have linked with a portal in order to recall to it!\n
	WERROR_PORTAL_SUMMON_FAILED, // You fail to summon the portal!\n
	WERROR_PORTAL_SUMMON_NO_LINK, // You must have linked with a portal in order to summon it!\n
	WERROR_PORTAL_TELEPORT_FAILED, // You fail to teleport!\n
	WERROR_PORTAL_TOO_RECENTLY, // You have been teleported too recently!\n
	WERROR_PORTAL_ADVOCATE_ONLY, // You must be an Advocate to interact with that portal.\n
	WERROR_PORTAL_AIS_NOT_ALLOWED,
	WERROR_PORTAL_PLAYERS_NOT_ALLOWED, // Players may not interact with that portal.\n
	WERROR_PORTAL_LEVEL_TOO_LOW, // You are not powerful enough to interact with that portal!\n
	WERROR_PORTAL_LEVEL_TOO_HIGH, // You are too powerful to interact with that portal!\n
	WERROR_PORTAL_NOT_RECALLABLE, // You cannot recall to that portal!\n
	WERROR_PORTAL_NOT_SUMMONABLE, // You cannot summon that portal!\n
	WERROR_CHEST_ALREADY_UNLOCKED, // The lock is already unlocked.
	WERROR_CHEST_NOT_LOCKABLE, // You can't lock or unlock that!
	WERROR_CHEST_ALREADY_OPEN, // You can't lock or unlock what is open!
	WERROR_CHEST_WRONG_KEY, // The key doesn't fit this lock.\n
	WERROR_CHEST_USED_TOO_RECENTLY, // The lock has been used too recently.
	WERROR_DONT_KNOW_LOCKPICKING, // You aren't trained in lockpicking!
	WERROR_ALLEGIANCE_INFO_EMPTY_NAME, // You must specify a character to query.
	WERROR_ALLEGIANCE_INFO_SELF, // Please use the allegiance panel to view your own information.
	WERROR_ALLEGIANCE_INFO_TOO_RECENT, // You have used that command too recently.
	WERROR_ABUSE_NO_SUCH_CHARACTER, // SendNotice_AbuseReportResponse
	WERROR_ABUSE_REPORTED_SELF, // SendNotice_AbuseReportResponse
	WERROR_ABUSE_COMPLAINT_HANDLED, // SendNotice_AbuseReportResponse
	WERROR_SALVAGE_DONT_OWN_TOOL = 1213, // You do not own that salvage tool!\n
	WERROR_SALVAGE_DONT_OWN_LOOT, // You do not own that item!\n
	WERROR_SALVAGE_NOT_SUITABLE, // The %s was not suitable for salvaging.
	WERROR_SALVAGE_WRONG_MATERIAL, // The %s contains the wrong material.
	WERROR_SALVAGE_CREATION_FAILED, // The material cannot be created.\n
	WERROR_SALVAGE_INVALID_LOOT_LIST, // The list of items you are attempting to salvage is invalid.\n
	WERROR_SALVAGE_TRADING_LOOT, // You cannot salvage items that you are trading!\n
	WERROR_PORTAL_HOUSE_RESTRICTED, // You must be a guest in this house to interact with that portal.\n
	WERROR_ACTIVATION_RANK_TOO_LOW, // Your Allegiance Rank is too low to use that item's magic.
	WERROR_ACTIVATION_WRONG_RACE, // You must be %s to use that item's magic.
	WERROR_ACTIVATION_ARCANE_LORE_TOO_LOW, // Your Arcane Lore skill is too low to use that item's magic.
	WERROR_ACTIVATION_NOT_ENOUGH_MANA, // That item doesn't have enough Mana.
	WERROR_ACTIVATION_SKILL_TOO_LOW, // Your %s is too low to use that item's magic.
	WERROR_ACTIVATION_NOT_CRAFTSMAN, // Only %s may use that item's magic.
	WERROR_ACTIVATION_NOT_SPECIALIZED, // You must have %s specialized to use that item's magic.
	WERROR_PORTAL_PK_ATTACKED_TOO_RECENTLY, // You have been involved in a player killer battle too recently to do that!\n
	WERROR_TRADE_AI_REFUSE_EMOTE,
	WERROR_TRADE_AI_REFUSE_EMOTE_FAILED_TOO_BUSY, //  is too busy to accept gifts right now.\n
	WERROR_TRADE_AI_TOO_MANY, //  cannot accept stacked objects. Try giving one at a time.\n
	WERROR_SKILL_ALTERATION_FAILED, // You have failed to alter your skill.\n
	WERROR_SKILL_ALTERATION_RAISE_NOT_TRAINED, // Your %s skill must be trained, not untrained or specialized, in order to be altered in this way!\n
	WERROR_SKILL_ALTERATION_RAISE_NOT_ENOUGH_CREDITS, // You do not have enough skill credits to specialize your %s skill.\n
	WERROR_SKILL_ALTERATION_WRAP_AROUND, // You have too many available experience points to be able to absorb the experience points from your %s skill. Please spend some of your experience points and try again.\n
	WERROR_SKILL_ALTERATION_LOWER_UNTRAINED, // Your %s skill is already untrained!\n
	WERROR_SKILL_ALTERATION_ILLEGAL_WIELDED_ITEMS, // You are currently wielding items which require a certain level of %s. Your %s skill cannot be lowered while you are wielding these items. Please remove these items and try again.\n
	WERROR_SKILL_ALTERATION_SPEC_SUCCEEDED, // You have succeeded in specializing your %s skill!\n
	WERROR_SKILL_ALTERATION_UNSPEC_SUCCEEDED, // You have succeeded in lowering your %s skill from specialized to trained!\n
	WERROR_SKILL_ALTERATION_UNTRAIN_SUCCEEDED, // You have succeeded in untraining your %s skill!\n
	WERROR_SKILL_ALTERATION_UNTRAIN_RACIAL_SUCCEEDED, // Although you cannot untrain your %s skill, you have succeeded in recovering all the experience you had invested in it.\n
	WERROR_SKILL_ALTERATION_RAISE_TOO_MANY_SPEC_CREDITS, // You have too many credits invested in specialized skills already! Before you can specialize your %s skill, you will need to unspecialize some other skill.\n
	WERROR_FELLOWSHIP_DECLINED,
	WERROR_FELLOWSHIP_TIMEOUT,
	WERROR_ATTRIBUTE_ALTERATION_FAILED, // You have failed to alter your attributes.\n
	WERROR_ATTRIBUTE_TRANSFER_FROM_TOO_LOW, // \n
	WERROR_ATTRIBUTE_TRANSFER_TO_TOO_HIGH, // \n
	WERROR_ATTRIBUTE_ALTERATION_ILLEGAL_WIELDED_ITEMS, // You are currently wielding items which require a certain level of skill. Your attributes cannot be transferred while you are wielding these items. Please remove these items and try again.\n
	WERROR_ATTRIBUTE_ALTERATION_SUCCEEDED, // You have succeeded in transferring your attributes!\n
	WERROR_HOUSE_DYNAMIC_HOOK_ADD, // This hook is a duplicated housing object. You may not add items to a duplicated housing object. Please empty the hook and allow it to reset.\n
	WERROR_HOUSE_WRONG_HOOK_TYPE, // That item is of the wrong type to be placed on this hook.\n
	WERROR_HOUSE_DYNAMIC_STORAGE_ADD, // This chest is a duplicated housing object. You may not add items to a duplicated housing object. Please empty everything -- including backpacks -- out of the chest and allow the chest to reset.\n
	WERROR_HOUSE_DYNAMIC_HOOK_CLOSE, // This hook was a duplicated housing object. Since it is now empty, it will be deleted momentarily. Once it is gone, it is safe to use the other, non-duplicated hook that is here.\n
	WERROR_HOUSE_DYNAMIC_STORAGE_CLOSE, // This chest was a duplicated housing object. Since it is now empty, it will be deleted momentarily. Once it is gone, it is safe to use the other, non-duplicated chest that is here.\n
	WERROR_ALLEGIANCE_OWNS_MANSION, // You cannot swear allegiance to anyone because you own a monarch-only house. Please abandon your house and try again.\n
	WERROR_HOOK_ITEM_ON_HOOK_NOT_USEABLE, // The %s cannot be used while on a hook and only the owner may open the hook.\n
	WERROR_HOOK_ITEM_ON_HOOK_NOT_USEABLE_OWNER, // The %s cannot be used while on a hook, use the '@house hooks on' command to make the hook openable.\n
	WERROR_HOOKER_NOT_USEABLE_OFF_HOOK, // The %s can only be used while on a hook.\n
	WERROR_MIDAIR, // You can't do that while in the air!
	WERROR_PK_SWITCH_RECOVERING, // You cannot modify your player killer status while you are recovering from a PK death.\n
	WERROR_PK_SWITCH_ADVOCATE, // Advocates may not change their player killer status!\n
	WERROR_PK_SWITCH_MIN_LEVEL, //Your level is too low to change your player killer status with this object.\n
	WERROR_PK_SWITCH_MAX_LEVEL, // Your level is too high to change your player killer status with this object.\n
	WERROR_PK_SWITCH_RECENT_KILL, // You feel a harsh dissonance, and you sense that an act of killing you have committed recently is interfering with the conversion.\n
	WERROR_PK_SWITCH_AUTO_PK, // Bael'Zharon's power flows through you again. You are once more a player killer.\n
	WERROR_PK_SWITCH_RESPITE, // Bael'Zharon has granted you respite after your moment of weakness. You are temporarily no longer a player killer.\n
	WERROR_PORTAL_PKLITE_NOT_ALLOWED, // Lite Player Killers may not interact with that portal!\n
	WERROR_PK_PROTECTED_ATTACKER_PASSIVE, // %s fails to affect you because $s cannot affect anyone!\n
	WERROR_PK_PROTECTED_TARGET_PASSIVE, // %s fails to affect you because you cannot be harmed!\n
	WERROR_PK_NPK_ATTACKER_PASSIVE, // %s fails to affect you because %s is not a player killer!\n
	WERROR_PK_NPK_TARGET_PASSIVE, //  fails to affect you because you are not a player killer!\n
	WERROR_PK_WRONG_KIND_PASSIVE, //  fails to affect you because you are not the same sort of player killer as 
	WERROR_PK_CROSS_HOUSE_BOUNDARY_PASSIVE, //  fails to affect you across a house boundary!\n
	WERROR_MAGIC_INVALID_TARGET, // %s is an invalid target.\n
	WERROR_MAGIC_INVALID_TARGET_PASSIVE, // You are an invalid target for the spell of %s.\n
	WERROR_HEAL_NOT_TRAINED, // You aren't trained in healing!
	WERROR_HEAL_DONT_OWN_KIT, // You don't own that healing kit!
	WERROR_HEAL_CANT_HEAL_THAT, // You can't heal that!
	WERROR_HEAL_FULL_HEALTH, // %s is already at full health!
	WERROR_HEAL_NOT_READY, // You aren't ready to heal!
	WERROR_HEAL_PLAYERS_ONLY, // You can only use Healing Kits on player characters.
	WERROR_LIFESTONE_PROTECTION, // The Lifestone's magic protects you from the attack!\n
	WERROR_PORTAL_PROTECTION, // The portal's residual energy protects you from the attack!\n
	WERROR_PK_SWITCH_PKLITE_OFF, // You are enveloped in a feeling of warmth as you are brought back into the protection of the Light. You are once again a Non-Player Killer.\n
	WERROR_DEATH_TOO_CLOSE_TO_SANCTUARY, // You're too close to your sanctuary!
	WERROR_TRADE_IN_PROGRESS, // You can't do that -- you're trading!
	WERROR_PK_SWITCH_PKLITE_ON_PK, // Only Non-Player Killers may enter PK Lite. Please see @help pklite for more details about this command.\n
	WERROR_PK_SWITCH_PKLITE_ON, // A cold wind touches your heart. You are now a Player Killer Lite.\n
	WERROR_MAGIC_NO_SUITABLE_ALTERNATE_TARGET, //  has no appropriate targets equipped for this spell.\n
	WERROR_MAGIC_NO_SUITABLE_ALTERNATE_TARGET_PASSIVE, // You have no appropriate targets equipped for %s's spell.\n
	WERROR_FELLOWSHIP_NOW_OPEN, //  is now an open fellowship; anyone may recruit new members.\n
	WERROR_FELLOWSHIP_NOW_CLOSED, //  is now a closed fellowship.\n
	WERROR_FELLOWSHIP_NEW_LEADER, //  is now the leader of this fellowship.\n
	WERROR_FELLOWSHIP_NO_LONGER_LEADER, // You have passed leadership of the fellowship to %s\n
	WERROR_FELLOWSHIP_NO_FELLOWSHIP, // You do not belong to a Fellowship.
	WERROR_HOUSE_MAX_HOOK_GROUP_REACHED, // You may not hook any more %s on your house. You already have the maximum number of %s hooked or you are not permitted to hook any on your type of house.\n
	WERROR_HOUSE_MAX_HOOK_GROUP_REACHED_SILENT,
	WERROR_HOUSE_NOW_USING_MAX_HOOKS, // You are now using the maximum number of hooks. You cannot use another hook until you take an item off one of your hooks.\n
	WERROR_HOUSE_NO_LONGER_USING_MAX_HOOKS, // You are no longer using the maximum number of hooks. You may again add items to your hooks.\n
	WERROR_HOUSE_NOW_USING_MAX_IN_HOOKGROUP, // You now have the maximum number of %s hooked. You cannot hook any additional %s until you remove one or more from your house.\n
	WERROR_HOUSE_NO_LONGER_USING_MAX_IN_HOOKGROUP, // You no longer have the maximum number of %s hooked. You may hook additional %s.\n
	WERROR_HOOK_NOT_PERMITED_TO_USE_HOOK, // You are not permitted to use that hook.\n
	WERROR_FELLOWSHIP_NOT_CLOSE_ENOUGH_LEVEL, //  is not close enough to your level.\n
	WERROR_FELLOWSHIP_LOCKED_RECRUITER, // This fellowship is locked; %s cannot be recruited into the fellowship.\n
	WERROR_FELLOWSHIP_LOCKED_RECRUITEE, // The fellowship is locked, you were not added to the fellowship.\n
	WERROR_ACTIVATION_NOT_ALLOWED_NO_NAME, // Only the original owner may use that item's magic.
	WERROR_CHAT_ENTERED_TURBINE_CHAT_ROOM, // You have entered the %s channel.\n
	WERROR_CHAT_LEFT_TURBINE_CHAT_ROOM, // You have left the %s channel.\n
	WERROR_CHAT_NOW_USING_TURBINE_CHAT,
	WERROR_ADMIN_IS_DEAF, //  will not receive your message, please use urgent assistance to speak with an in-game representative\n
	WERROR_ADMIN_DEAF_TO_MESSAGE, // Message Blocked: %s
	WERROR_LOUD_LIST_IS_FULL, // // You cannot add anymore people to the list of players that you can hear.\n
	WERROR_CHARACTER_ADDED_LOUD_LIST, //  has been added to the list of people you can hear.\n
	WERROR_CHARACTER_REMOVED_LOUD_LIST, //  has been removed from the list of people you can hear.\n
	WERROR_DEAF_MODE_ON, // You are now deaf to player's screams.\n
	WERROR_DEAF_MODE_OFF, // You can hear all players once again.\n
	WERROR_FAILED_MUTE, // You fail to remove %s from your loud list.\n
	WERROR_CRAFT_CHICKEN_OUT_MSG, // You chicken out.
	WERROR_CRAFT_NO_CHANCE, // You cannot posssibly succeed.
	WERROR_FELLOWSHIP_FELLOW_LOCKED_CAN_NOT_OPEN, // The fellowship is locked; you cannot open locked fellowships.\n
	WERROR_TRADE_COMPLETE, // Trade Complete!
	WERROR_SALVAGE_NOT_A_TOOL, // That is not a salvaging tool.
	WERROR_CHARACTER_NOT_AVAILABLE, // That person is not available now.
	WERROR_SNOOP_STARTED, // You are now snooping on %s.\n
	WERROR_SNOOP_STOPED, // You are no longer snooping on %s.\n
	WERROR_SNOOP_FAILED, // You fail to snoop on %s.\n
	WERROR_SNOOP_UNAUTHORIZED, // %s attempted to snoop on you.\n
	WERROR_SNOOP_ALREADY_SNOOPED_ON, // %s is already being snooped on, only one person may snoop on another at a time.\n
	WERROR_CHARACTER_IN_LIMBO_MSG_NOT_RECEIVED, // %s is in limbo and cannot receive your message.\n
	WERROR_HOUSE_PURCHASE_TOO_SOON, // You must wait 30 days after purchasing a house before you may purchase another with any character on the same account. This applies to all housing except apartments.\n
	WERROR_ALLEGIANCE_I_AM_BOOTED_FROM_CHAT, // You have been booted from your allegiance chat room. Use "@allegiance chat on" to rejoin. (%s).\n
	WERROR_ALLEGIANCE_TARGET_BOOTED_FROM_CHAT, // %s has been booted from the allegiance chat room.\n
	WERROR_ALLEGIANCE_NOT_AUTHORIZED, // You do not have the authority within your allegiance to do that.\n
	WERROR_ALLEGIANCE_CHAR_ALREADY_BANNED, // The account of %s is already banned from the allegiance.\n
	WERROR_ALLEGIANCE_CHAR_NOT_BANNED, // The account of %s is not banned from the allegiance.\n
	WERROR_ALLEGIANCE_CHAR_NOT_UNBANNED, // The account of %s was not unbanned from the allegiance.\n
	WERROR_ALLEGIANCE_CHAR_BANNED_SUCCESSFULLY, // The account of %s has been banned from the allegiance.\n
	WERROR_ALLEGIANCE_CHAR_UNBANNED_SUCCESSFULLY, // The account of %s is no longer banned from the allegiance.\n
	WERROR_ALLEGIANCE_LIST_BANNED_CHARACTERS, // Banned Characters: 
	WERROR_ALLEGIANCE_BANNED = 1342, // %s is banned from the allegiance!\n
	WERROR_ALLEGIANCE_YOU_ARE_BANNED, // You are banned from %s's allegiance!\n
	WERROR_ALLEGIANCE_BANNED_LIST_FULL, // You have the maximum number of accounts banned.!\n
	WERROR_ALLEGIANCE_OFFICER_SET, // %s is now an allegiance officer.\n
	WERROR_ALLEGIANCE_OFFICER_NOT_SET, // An unspecified error occurred while attempting to set %s as an allegiance officer.\n
	WERROR_ALLEGIANCE_OFFICER_REMOVED, // %s is no longer an allegiance officer.\n
	WERROR_ALLEGIANCE_OFFICER_NOT_REMOVED, // An unspecified error occurred while attempting to remove %s as an allegiance officer.\n
	WERROR_ALLEGIANCE_OFFICER_FULL, // You already have the maximum number of allegiance officers. You must remove some before you add any more.\n
	WERROR_ALLEGIANCE_OFFICERS_CLEARED, // Your allegiance officers have been cleared.\n
	WERROR_CHAT_MUST_WAIT_TO_COMMUNICATE, // You must wait %s before communicating again!\n
	WERROR_CHAT_NO_JOIN_CHANNEL_WHILE_GAGGED, // You cannot join any chat channels while gagged.\n
	WERROR_ALLEGIANCE_YOU_ARE_NOW_AN_OFFICER, // Your allegiance officer status has been modified. You now hold the position of: %s.\n
	WERROR_ALLEGIANCE_YOU_ARE_NO_LONGER_AN_OFFICER, // You are no longer an allegiance officer.\n
	WERROR_ALLEGIANCE_OFFICER_ALREADY_OFFICER, // %s is already an allegiance officer of that level.\n
	WERROR_ALLEGIANCE_HOMETOWN_NOT_SET, // Your allegiance does not have a hometown.\n
	WERROR_ALREADY_BEING_USED, // The %s is currently in use.\n
	WERROR_HOOK_EMPTY_NOT_OWNER, // The hook does not contain a usable item. You cannot open the hook because you do not own the house to which it belongs.\n
	WERROR_HOOK_EMPTY_OWNER, // The hook does not contain a usable item. Use the '@house hooks on'command to make the hook openable.\n
	WERROR_MISSILE_OUT_OF_RANGE, // Out of Range!
	WERROR_CHAT_NOT_LISTENING_TO_CHANNEL, // You are not listening to the %s channel!\n
	WERROR_ACTOD_ONLY, // You must purchase Asheron's Call -- Throne of Destiny to use this function.
	WERROR_ITEM_ACTOD_ONLY, // You must purchase Asheron's Call -- Throne of Destiny to use this item.
	WERROR_PORTAL_ACTOD_ONLY, // You must purchase Asheron's Call -- Throne of Destiny to use this portal.
	WERROR_QUEST_ACTOD_ONLY, // You must purchase Asheron's Call -- Throne of Destiny to access this quest.
	WERROR_AUGMENTATION_FAILED, // You have failed to complete the augmentation.\n
	WERROR_AUGMENTATION_TOO_MANY_TIMES, // You have used this augmentation too many times already.\n
	WERROR_AUGMENTATION_FAMILY_TOO_MANY_TIMES, // You have used augmentations of this type too many times already.\n
	WERROR_AUGMENTATION_NOT_ENOUGH_XP, // You do not have enough unspent experience available to purchase this augmentation.\n
	WERROR_AUGMENTATION_SKILL_NOT_TRAINED, // %s\n
	WERROR_AUGMENTATION_SUCCEEDED, // Congratulations! You have succeeded in acquiring the %s augmentation.\n
	WERROR_SKILL_ALTERATION_UNTRAIN_AUGMENTED_SUCCEEDED, // Although your augmentation will not allow you to untrain your %s skill, you have succeeded in recovering all the experience you had invested in it.\n
	WERROR_PORTAL_RECALLS_DISABLED, // You must exit the Training Academy before that command will be available to you.\n
	WERROR_AFK, // {arbitrary string sent by server}
	WERROR_PK_ONLY, // Only Player Killer characters may use this command!\n
	WERROR_PKL_ONLY, // Only Player Killer Lite characters may use this command!\n
	WERROR_FRIENDS_EXCEEDED_MAX, // You may only have a maximum of 50 friends at once. If you wish to add more friends, you must first remove some.
	WERROR_FRIENDS_ALREADY_FRIEND, // %s is already on your friends list!\n
	WERROR_FRIENDS_NOT_FRIEND, // That character is not on your friends list!\n
	WERROR_HOUSE_NOT_OWNER, // Only the character who owns the house may use this command.
	WERROR_ALLEGIANCE_NAME_EMPTY, // That allegiance name is invalid because it is empty. Please use the @allegiance name clear command to clear your allegiance name.\n
	WERROR_ALLEGIANCE_NAME_TOO_LONG, // That allegiance name is too long. Please choose another name.\n
	WERROR_ALLEGIANCE_NAME_BAD_CHARACTER, // That allegiance name contains illegal characters. Please choose another name using only letters, spaces, - and '.\n
	WERROR_ALLEGIANCE_NAME_NOT_APPROPRIATE, // That allegiance name is not appropriate. Please choose another name.\n
	WERROR_ALLEGIANCE_NAME_IN_USE, // That allegiance name is already in use. Please choose another name.\n
	WERROR_ALLEGIANCE_NAME_TIMER, // You may only change your allegiance name once every 24 hours. You may change your allegiance name again in %s.\n
	WERROR_ALLEGIANCE_NAME_CLEARED, // Your allegiance name has been cleared.\n
	WERROR_ALLEGIANCE_NAME_SAME_NAME, // That is already the name of your allegiance!\n
	WERROR_ALLEGIANCE_OFFICER_ALREADY_MONARCH, // %s is the monarch and cannot be promoted or demoted.\n
	WERROR_ALLEGIANCE_OFFICER_TITLE_SET, // That level of allegiance officer is now known as: %s.\n
	WERROR_ALLEGIANCE_OFFICER_INVALID_LEVEL, // That is an invalid officer level.\n
	WERROR_ALLEGIANCE_OFFICER_TITLE_NOT_APPROPRIATE, // That allegiance officer title is not appropriate.\n
	WERROR_ALLEGIANCE_OFFICER_TITLE_TOO_LONG, // That allegiance officer title name is too long. Please choose another name.\n
	WERROR_ALLEGIANCE_OFFICER_TITLE_CLEARED, // All of your allegiance officer titles have been cleared.\n
	WERROR_ALLEGIANCE_OFFICER_TITLE_BAD_CHARACTER, // That allegiance officer title contains illegal characters. Please choose another name using only letters, spaces, - and '.\n
	WERROR_ALLEGIANCE_LOCK_DISPLAY, // Your allegiance is currently: %s.\n
	WERROR_ALLEGIANCE_LOCK_SET, // Your allegiance is now: %s.\n
	WERROR_ALLEGIANCE_LOCK_PREVENTS_PATRON, // You may not accept the offer of allegiance from %s because your allegiance is locked.\n
	WERROR_ALLEGIANCE_LOCK_PREVENTS_VASSAL, // You may not swear allegiance at this time because the allegiance of %s is locked.\n
	WERROR_ALLEGIANCE_LOCK_APPROVED_DISPLAY, // You have pre-approved %s to join your allegiance.\n
	WERROR_ALLEGIANCE_LOCK_NO_APPROVED, //You have not pre-approved any vassals to join your allegiance.\n
	WERROR_ALLEGIANCE_TARGET_ALREADY_A_MEMBER, // %s is already a member of your allegiance!\n
	WERROR_ALLEGIANCE_APPROVED_SET, // %s has been pre-approved to join your allegiance.\n
	WERROR_ALLEGIANCE_APPROVED_CLEARED, // You have cleared the pre-approved vassal for your allegiance.\n
	WERROR_ALLEGIANCE_GAG_ALREADY, // That character is already gagged!\n
	WERROR_ALLEGIANCE_GAG_NOT_ALREADY, // That character is not currently gagged!\n
	WERROR_ALLEGIANCE_GAG_TARGET, // Your allegiance chat privileges have been temporarily removed by %s. Until they are restored, you may not view or speak in the allegiance chat channel.
	WERROR_ALLEGIANCE_GAG_OFFICER, // %s is now temporarily unable to view or speak in allegiance chat. The gag will run out in 5 minutes, or %s may be explicitly ungagged before then.
	WERROR_ALLEGIANCE_GAG_AUTO_UNGAG, // Your allegiance chat privileges have been restored.\n
	WERROR_ALLEGIANCE_GAG_UNGAG_TARGET, // Your allegiance chat privileges have been restored by %s.
	WERROR_ALLEGIANCE_GAG_UNGAG_OFFICER, // You have restored allegiance chat privileges to %s.
	WERROR_TOO_MANY_UNIQUE_ITEMS, // You cannot pick up more of that item!
	WERROR_HERITAGE_REQUIRES_SPECIFIC_ARMOR, // You are restricted to clothes and armor created for your race.
	WERROR_SPECIFIC_ARMOR_REQUIRES_HERITAGE, // That item was specifically created for another race.
	WERROR_NOT_OLTHOI_INTERACTION, // Olthoi cannot interact with that!\n
	WERROR_NOT_OLTHOI_LIFESTONE, // Olthoi cannot use regular lifestones! Asheron would not allow it!\n
	WERROR_NOT_OLTHOI_VENDOR, // The vendor looks at you in horror!\n
	WERROR_NOT_OLTHOI_NPC, // %s cowers from you!\n
	WERROR_NO_OLTHOI_FELLOWSHIP, // As a mindless engine of destruction an Olthoi cannot join a fellowship!\n
	WERROR_NO_OLTHOI_ALLEGIANCE, // The Olthoi only have an allegiance to the Olthoi Queen!\n
	WERROR_ITEM_INTERACTION_RESTRICTED, // You cannot use that item!\n
	WERROR_PERSON_INTERACTION_RESTRICTED, // This person will not interact with you!\n
	WERROR_PORTAL_ONLY_OLTHOI_PK, // Only Olthoi may pass through this portal!\n
	WERROR_PORTAL_NO_OLTHOI_PK, // Olthoi may not pass through this portal!\n
	WERROR_PORTAL_NO_VITAE, // You may not pass through this portal while Vitae weakens you!\n
	WERROR_PORTAL_NO_NEW_ACCOUNTS, // This character must be two weeks old or have been created on an account at least two weeks old to use this portal!\n
	WERROR_BAD_OLTHOI_RECALL, // Olthoi characters can only use Lifestone and PK Arena recalls!\n
	WERROR_CONTRACT_ERROR
};

enum SoundType
{
	Sound_Invalid = 0x0,
	Sound_Speak1 = 0x1,
	Sound_Random = 0x2,
	Sound_Attack1 = 0x3,
	Sound_Attack2 = 0x4,
	Sound_Attack3 = 0x5,
	Sound_SpecialAttack1 = 0x6,
	Sound_SpecialAttack2 = 0x7,
	Sound_SpecialAttack3 = 0x8,
	Sound_Damage1 = 0x9,
	Sound_Damage2 = 0xA,
	Sound_Damage3 = 0xB,
	Sound_Wound1 = 0xC,
	Sound_Wound2 = 0xD,
	Sound_Wound3 = 0xE,
	Sound_Death1 = 0xF,
	Sound_Death2 = 0x10,
	Sound_Death3 = 0x11,
	Sound_Grunt1 = 0x12,
	Sound_Grunt2 = 0x13,
	Sound_Grunt3 = 0x14,
	Sound_Oh1 = 0x15,
	Sound_Oh2 = 0x16,
	Sound_Oh3 = 0x17,
	Sound_Heave1 = 0x18,
	Sound_Heave2 = 0x19,
	Sound_Heave3 = 0x1A,
	Sound_Knockdown1 = 0x1B,
	Sound_Knockdown2 = 0x1C,
	Sound_Knockdown3 = 0x1D,
	Sound_Swoosh1 = 0x1E,
	Sound_Swoosh2 = 0x1F,
	Sound_Swoosh3 = 0x20,
	Sound_Thump1 = 0x21,
	Sound_Smash1 = 0x22,
	Sound_Scratch1 = 0x23,
	Sound_Spear = 0x24,
	Sound_Sling = 0x25,
	Sound_Dagger = 0x26,
	Sound_ArrowWhiz1 = 0x27,
	Sound_ArrowWhiz2 = 0x28,
	Sound_CrossbowPull = 0x29,
	Sound_CrossbowRelease = 0x2A,
	Sound_BowPull = 0x2B,
	Sound_BowRelease = 0x2C,
	Sound_ThrownWeaponRelease1 = 0x2D,
	Sound_ArrowLand = 0x2E,
	Sound_Collision = 0x2F,
	Sound_HitFlesh1 = 0x30,
	Sound_HitLeather1 = 0x31,
	Sound_HitChain1 = 0x32,
	Sound_HitPlate1 = 0x33,
	Sound_HitMissile1 = 0x34,
	Sound_HitMissile2 = 0x35,
	Sound_HitMissile3 = 0x36,
	Sound_Footstep1 = 0x37,
	Sound_Footstep2 = 0x38,
	Sound_Walk1 = 0x39,
	Sound_Dance1 = 0x3A,
	Sound_Dance2 = 0x3B,
	Sound_Dance3 = 0x3C,
	Sound_Hidden1 = 0x3D,
	Sound_Hidden2 = 0x3E,
	Sound_Hidden3 = 0x3F,
	Sound_Eat1 = 0x40,
	Sound_Drink1 = 0x41,
	Sound_Open = 0x42,
	Sound_Close = 0x43,
	Sound_OpenSlam = 0x44,
	Sound_CloseSlam = 0x45,
	Sound_Ambient1 = 0x46,
	Sound_Ambient2 = 0x47,
	Sound_Ambient3 = 0x48,
	Sound_Ambient4 = 0x49,
	Sound_Ambient5 = 0x4A,
	Sound_Ambient6 = 0x4B,
	Sound_Ambient7 = 0x4C,
	Sound_Ambient8 = 0x4D,
	Sound_Waterfall = 0x4E,
	Sound_LogOut = 0x4F,
	Sound_LogIn = 0x50,
	Sound_LifestoneOn = 0x51,
	Sound_AttribUp = 0x52,
	Sound_AttribDown = 0x53,
	Sound_SkillUp = 0x54,
	Sound_SkillDown = 0x55,
	Sound_HealthUp = 0x56,
	Sound_HealthDown = 0x57,
	Sound_ShieldUp = 0x58,
	Sound_ShieldDown = 0x59,
	Sound_EnchantUp = 0x5A,
	Sound_EnchantDown = 0x5B,
	Sound_VisionUp = 0x5C,
	Sound_VisionDown = 0x5D,
	Sound_Fizzle = 0x5E,
	Sound_Launch = 0x5F,
	Sound_Explode = 0x60,
	Sound_TransUp = 0x61,
	Sound_TransDown = 0x62,
	Sound_BreatheFlaem = 0x63,
	Sound_BreatheAcid = 0x64,
	Sound_BreatheFrost = 0x65,
	Sound_BreatheLightning = 0x66,
	Sound_Create = 0x67,
	Sound_Destroy = 0x68,
	Sound_Lockpicking = 0x69,
	Sound_UI_EnterPortal = 0x6A,
	Sound_UI_ExitPortal = 0x6B,
	Sound_UI_GeneralQuery = 0x6C,
	Sound_UI_GeneralError = 0x6D,
	Sound_UI_TransientMessage = 0x6E,
	Sound_UI_IconPickUp = 0x6F,
	Sound_UI_IconSuccessfulDrop = 0x70,
	Sound_UI_IconInvalid_Drop = 0x71,
	Sound_UI_ButtonPress = 0x72,
	Sound_UI_GrabSlider = 0x73,
	Sound_UI_ReleaseSlider = 0x74,
	Sound_UI_NewTargetSelected = 0x75,
	Sound_UI_Roar = 0x76,
	Sound_UI_Bell = 0x77,
	Sound_UI_Chant1 = 0x78,
	Sound_UI_Chant2 = 0x79,
	Sound_UI_DarkWhispers1 = 0x7A,
	Sound_UI_DarkWhispers2 = 0x7B,
	Sound_UI_DarkLaugh = 0x7C,
	Sound_UI_DarkWind = 0x7D,
	Sound_UI_DarkSpeech = 0x7E,
	Sound_UI_Drums = 0x7F,
	Sound_UI_GhostSpeak = 0x80,
	Sound_UI_Breathing = 0x81,
	Sound_UI_Howl = 0x82,
	Sound_UI_LostSouls = 0x83,
	Sound_UI_Squeal = 0x84,
	Sound_UI_Thunder1 = 0x85,
	Sound_UI_Thunder2 = 0x86,
	Sound_UI_Thunder3 = 0x87,
	Sound_UI_Thunder4 = 0x88,
	Sound_UI_Thunder5 = 0x89,
	Sound_UI_Thunder6 = 0x8A,
	Sound_RaiseTrait = 0x8B,
	Sound_WieldObject = 0x8C,
	Sound_UnwieldObject = 0x8D,
	Sound_ReceiveItem = 0x8E,
	Sound_PickUpItem = 0x8F,
	Sound_DropItem = 0x90,
	Sound_ResistSpell = 0x91,
	Sound_PicklockFail = 0x92,
	Sound_LockSuccess = 0x93,
	Sound_OpenFailDueToLock = 0x94,
	Sound_TriggerActivated = 0x95,
	Sound_SpellExpire = 0x96,
	Sound_ItemManaDepleted = 0x97,
	Sound_TriggerActivated1 = 0x98,
	Sound_TriggerActivated2 = 0x99,
	Sound_TriggerActivated3 = 0x9A,
	Sound_TriggerActivated4 = 0x9B,
	Sound_TriggerActivated5 = 0x9C,
	Sound_TriggerActivated6 = 0x9D,
	Sound_TriggerActivated7 = 0x9E,
	Sound_TriggerActivated8 = 0x9F,
	Sound_TriggerActivated9 = 0xA0,
	Sound_TriggerActivated10 = 0xA1,
	Sound_TriggerActivated11 = 0xA2,
	Sound_TriggerActivated12 = 0xA3,
	Sound_TriggerActivated13 = 0xA4,
	Sound_TriggerActivated14 = 0xA5,
	Sound_TriggerActivated15 = 0xA6,
	Sound_TriggerActivated16 = 0xA7,
	Sound_TriggerActivated17 = 0xA8,
	Sound_TriggerActivated18 = 0xA9,
	Sound_TriggerActivated19 = 0xAA,
	Sound_TriggerActivated20 = 0xAB,
	Sound_TriggerActivated21 = 0xAC,
	Sound_TriggerActivated22 = 0xAD,
	Sound_TriggerActivated23 = 0xAE,
	Sound_TriggerActivated24 = 0xAF,
	Sound_TriggerActivated25 = 0xB0,
	Sound_TriggerActivated26 = 0xB1,
	Sound_TriggerActivated27 = 0xB2,
	Sound_TriggerActivated28 = 0xB3,
	Sound_TriggerActivated29 = 0xB4,
	Sound_TriggerActivated30 = 0xB5,
	Sound_TriggerActivated31 = 0xB6,
	Sound_TriggerActivated32 = 0xB7,
	Sound_TriggerActivated33 = 0xB8,
	Sound_TriggerActivated34 = 0xB9,
	Sound_TriggerActivated35 = 0xBA,
	Sound_TriggerActivated36 = 0xBB,
	Sound_TriggerActivated37 = 0xBC,
	Sound_TriggerActivated38 = 0xBD,
	Sound_TriggerActivated39 = 0xBE,
	Sound_TriggerActivated40 = 0xBF,
	Sound_TriggerActivated41 = 0xC0,
	Sound_TriggerActivated42 = 0xC1,
	Sound_TriggerActivated43 = 0xC2,
	Sound_TriggerActivated44 = 0xC3,
	Sound_TriggerActivated45 = 0xC4,
	Sound_TriggerActivated46 = 0xC5,
	Sound_TriggerActivated47 = 0xC6,
	Sound_TriggerActivated48 = 0xC7,
	Sound_TriggerActivated49 = 0xC8,
	Sound_TriggerActivated50 = 0xC9,
	Sound_HealthDownVoid = 0xCA,
	Sound_RegenDownVoid = 0xCB,
	Sound_SkillDownVoid = 0xCC,
	NUM_SOUND_TYPES = 0xCD,
	FORCE_SoundType_32_BIT = 0x7FFFFFFF,
};

enum DAMAGE_TYPE
{
	UNDEF_DAMAGE_TYPE = 0,
	SLASH_DAMAGE_TYPE = 1,
	PIERCE_DAMAGE_TYPE = 2,
	BLUDGEON_DAMAGE_TYPE = 4,
	COLD_DAMAGE_TYPE = 8,
	FIRE_DAMAGE_TYPE = 0x10,
	ACID_DAMAGE_TYPE = 0x20,
	ELECTRIC_DAMAGE_TYPE = 0x40,
	HEALTH_DAMAGE_TYPE = 0x80,
	STAMINA_DAMAGE_TYPE = 0x100,
	MANA_DAMAGE_TYPE = 0x200,
	NETHER_DAMAGE_TYPE = 0x400,
	BASE_DAMAGE_TYPE = 0x10000000
};

enum WeenieType
{
	Undef_WeenieType,
	Generic_WeenieType,
	Clothing_WeenieType,
	MissileLauncher_WeenieType,
	Missile_WeenieType,
	Ammunition_WeenieType,
	MeleeWeapon_WeenieType,
	Portal_WeenieType,
	Book_WeenieType,
	Coin_WeenieType,
	Creature_WeenieType,
	Admin_WeenieType,
	Vendor_WeenieType,
	HotSpot_WeenieType,
	Corpse_WeenieType,
	Cow_WeenieType,
	AI_WeenieType,
	Machine_WeenieType,
	Food_WeenieType,
	Door_WeenieType,
	Chest_WeenieType,
	Container_WeenieType,
	Key_WeenieType,
	Lockpick_WeenieType,
	PressurePlate_WeenieType,
	LifeStone_WeenieType,
	Switch_WeenieType,
	PKModifier_WeenieType,
	Healer_WeenieType,
	LightSource_WeenieType,
	Allegiance_WeenieType,
	Type32_WeenieType, // unknown
	SpellComponent_WeenieType,
	ProjectileSpell_WeenieType,
	Scroll_WeenieType,
	Caster_WeenieType,
	Channel_WeenieType,
	ManaStone_WeenieType,
	Gem_WeenieType,
	AdvocateFane_WeenieType,
	AdvocateItem_WeenieType,
	Sentinel_WeenieType,
	GSpellEconomy_WeenieType,
	LSpellEconomy_WeenieType,
	CraftTool_WeenieType,
	LScoreKeeper_WeenieType,
	GScoreKeeper_WeenieType,
	GScoreGatherer_WeenieType,
	ScoreBook_WeenieType,
	EventCoordinator_WeenieType,
	Entity_WeenieType,
	Stackable_WeenieType,
	HUD_WeenieType,
	House_WeenieType,
	Deed_WeenieType,
	SlumLord_WeenieType,
	Hook_WeenieType,
	Storage_WeenieType,
	BootSpot_WeenieType,
	HousePortal_WeenieType,
	Game_WeenieType,
	GamePiece_WeenieType,
	SkillAlterationDevice_WeenieType,
	AttributeTransferDevice_WeenieType,
	Hooker_WeenieType,
	AllegianceBindstone_WeenieType,
	InGameStatKeeper_WeenieType,
	AugmentationDevice_WeenieType,
	SocialManager_WeenieType,
	Pet_WeenieType,
	PetDevice_WeenieType,
	CombatPet_WeenieType
};

enum SpellType
{
	Undef_SpellType = 0x0,
	Enchantment_SpellType = 0x1,
	Projectile_SpellType = 0x2,
	Boost_SpellType = 0x3,
	Transfer_SpellType = 0x4,
	PortalLink_SpellType = 0x5,
	PortalRecall_SpellType = 0x6,
	PortalSummon_SpellType = 0x7,
	PortalSending_SpellType = 0x8,
	Dispel_SpellType = 0x9,
	LifeProjectile_SpellType = 0xA,
	FellowBoost_SpellType = 0xB,
	FellowEnchantment_SpellType = 0xC,
	FellowPortalSending_SpellType = 0xD,
	FellowDispel_SpellType = 0xE,
	EnchantmentProjectile_SpellType = 0xF,
	FORCE_SpellType_32_BIT = 0x7FFFFFFF,
};

enum SpellComponentCategory
{
	Scarab_SpellComponentCategory = 0x0,
	Herb_SpellComponentCategory = 0x1,
	PowderedGem_SpellComponentCategory = 0x2,
	AlchemicalSubstance_SpellComponentCategory = 0x3,
	Talisman_SpellComponentCategory = 0x4,
	Taper_SpellComponentCategory = 0x5,
	Pea_SpellComponentCategory = 0x6,
	Num_SpellComponentCategories = 0x7,
	Undef_SpellComponentCategory = 0x8,
	FORCE_SpellComponentCategory_32_BIT = 0x7FFFFFFF,
};

enum SpellComponentType
{
	Undef_SpellComponentType = 0x0,
	Power_SpellComponentType = 0x1,
	Action_SpellComponentType = 0x2,
	ConceptPrefix_SpellComponentType = 0x3,
	ConceptSuffix_SpellComponentType = 0x4,
	Target_SpellComponentType = 0x5,
	Accent_SpellComponentType = 0x6,
	Pea_SpellComponentType = 0x7,
	FORCE_SpellComponentType_32_BIT = 0x7FFFFFFF,
};

enum SpellIndex
{
	Undef_SpellIndex = 0x0,
	Resistable_SpellIndex = 0x1,
	PKSensitive_SpellIndex = 0x2,
	Beneficial_SpellIndex = 0x4,
	SelfTargeted_SpellIndex = 0x8,
	Reversed_SpellIndex = 0x10,
	NotIndoor_SpellIndex = 0x20,
	NotOutdoor_SpellIndex = 0x40,
	NotResearchable_SpellIndex = 0x80,
	Projectile_SpellIndex = 0x100,
	CreatureSpell_SpellIndex = 0x200,
	ExcludedFromItemDescriptions_SpellIndex = 0x400,
	IgnoresManaConversion_SpellIndex = 0x800,
	NonTrackingProjectile_SpellIndex = 0x1000,
	FellowshipSpell_SpellIndex = 0x2000,
	FastCast_SpellIndex = 0x4000,
	IndoorLongRange_SpellIndex = 0x8000,
	DamageOverTime_SpellIndex = 0x10000,
	FORCE_SpellIndex_32_BIT = 0x7FFFFFFF,
};

enum LogTextType
{
	LTT_DEFAULT = 0,
	LTT_ALL_CHANNELS,
	LTT_SPEECH,
	LTT_SPEECH_DIRECT,
	LTT_SPEECH_DIRECT_SEND,
	LTT_SYSTEM_EVENT,
	LTT_COMBAT,
	LTT_MAGIC,
	LTT_CHANNEL,
	LTT_CHANNEL_SEND,
	LTT_SOCIAL_CHANNEL,
	LTT_SOCIAL_CHANNEL_SEND,
	LTT_EMOTE,
	LTT_ADVANCEMENT,
	LTT_ABUSE_CHANNEL,
	LTT_HELP_CHANNEL,
	LTT_APPRAISAL_CHANNEL,
	LTT_MAGIC_CASTING_CHANNEL,
	LTT_ALLEGIENCE_CHANNEL,
	LTT_FELLOWSHIP_CHANNEL,
	LTT_WORLD_BROADCAST,
	LTT_COMBAT_ENEMY,
	LTT_COMBAT_SELF,
	LTT_RECALL,
	LTT_CRAFT,
	LTT_SALVAGING,
	LTT_ERROR,
	LTT_GENERAL_CHANNEL,
	LTT_TRADE_CHANNEL,
	LTT_LFG_CHANNEL,
	LTT_ROLEPLAY_CHANNEL,
	LTT_SPEECH_DIRECT_ADMIN,
	LTT_SOCIETY_CHANNEL,
	LTT_OLTHOI_CHANNEL,
	LTT_TOTAL_NUM_CHANNELS
};

enum COMBAT_USE
{
	COMBAT_USE_NONE = 0x0,
	COMBAT_USE_MELEE = 0x1,
	COMBAT_USE_MISSILE = 0x2,
	COMBAT_USE_AMMO = 0x3,
	COMBAT_USE_SHIELD = 0x4,
	COMBAT_USE_TWO_HANDED = 0x5,
	COMBAT_USE_OFFHAND = 0x6,
	FORCE_COMBAT_USE_32_BIT = 0x7FFFFFFF,
};

enum UI_EFFECT_TYPE
{
	UI_EFFECT_UNDEF = 0x0,
	UI_EFFECT_MAGICAL = 0x1,
	UI_EFFECT_POISONED = 0x2,
	UI_EFFECT_BOOST_HEALTH = 0x4,
	UI_EFFECT_BOOST_MANA = 0x8,
	UI_EFFECT_BOOST_STAMINA = 0x10,
	UI_EFFECT_FIRE = 0x20,
	UI_EFFECT_LIGHTNING = 0x40,
	UI_EFFECT_FROST = 0x80,
	UI_EFFECT_ACID = 0x100,
	UI_EFFECT_BLUDGEONING = 0x200,
	UI_EFFECT_SLASHING = 0x400,
	UI_EFFECT_PIERCING = 0x800,
	UI_EFFECT_NETHER = 0x1000,
	FORCE_UI_EFFECT_TYPE_32_BIT = 0x7FFFFFFF,
};

enum PARENT_ENUM // not sure about the name
{
	PARENT_NONE = 0,
	PARENT_RIGHT_HAND,
	PARENT_LEFT_HAND,
	PARENT_SHIELD,
	PARENT_BELT,
	PARENT_QUIVER,
	PARENT_HERALDRY,
	PARENT_MOUTH,
	PARENT_LEFT_WEAPON,
	PARENT_LEFT_UNARMED
};

enum CLOTHING_PRIORITY // custom
{
	UNDEF_CLOTHING_PRIORITY = 0,
	UPPER_LEG_WEAR_CLOTHING_PRIORITY = 0x2,
	LOWER_LEG_WEAR_CLOTHING_PRIORITY = 0x4,
	CHEST_WEAR_CLOTHING_PRIORITY = 0x8,
	ABDOMEN_WEAR_CLOTHING_PRIORITY = 0x10,
	UPPER_ARM_WEAR_CLOTHING_PRIORITY = 0x20,
	LOWER_ARM_WEAR_CLOTHING_PRIORITY = 0x40,
	UPPER_LEG_ARMOR_CLOTHING_PRIORITY = 0x100,
	LOWER_LEG_ARMOR_CLOTHING_PRIORITY = 0x200,
	CHEST_ARMOR_CLOTHING_PRIORITY = 0x400,
	ABDOMEN_ARMOR_CLOTHING_PRIORITY = 0x800,
	UPPER_ARM_ARMOR_CLOTHING_PRIORITY = 0x1000,
	LOWER_ARM_ARMOR_CLOTHING_PRIORITY = 0x2000,
	HEAD_WEAR_CLOTHING_PRIORITY = 0x4000,
	HAND_WEAR_CLOTHING_PRIORITY = 0x8000,
	FOOT_WEAR_CLOTHING_PRIORITY = 0x10000
};

enum InventoryRequest
{
	IR_NONE = 0x0,
	IR_MERGE = 0x1,
	IR_SPLIT = 0x2,
	IR_MOVE = 0x3,
	IR_PICK_UP = 0x4,
	IR_PUT_IN_CONTAINER = 0x5,
	IR_DROP = 0x6,
	IR_WIELD = 0x7,
	IR_VIEW_AS_GROUND_CONTAINER = 0x8,
	IR_GIVE = 0x9,
	IR_SHOP_EVENT = 0xA,
	FORCE_InventoryRequest_32_BIT = 0x7FFFFFFF,
};

enum PALETTE_TEMPLATE_ID
{
	UNDEF_PALETTE_TEMPLATE = 0,
	AQUABLUE_PALETTE_TEMPLATE = 1,
	BLUE_PALETTE_TEMPLATE = 2,
	BLUEPURPLE_PALETTE_TEMPLATE = 3,
	BROWN_PALETTE_TEMPLATE = 4,
	DARKBLUE_PALETTE_TEMPLATE = 5,
	DEEPBROWN_PALETTE_TEMPLATE = 6,
	DEEPGREEN_PALETTE_TEMPLATE = 7,
	GREEN_PALETTE_TEMPLATE = 8,
	GREY_PALETTE_TEMPLATE = 9,
	LIGHTBLUE_PALETTE_TEMPLATE = 10,
	MAROON_PALETTE_TEMPLATE = 11,
	NAVY_PALETTE_TEMPLATE = 12,
	PURPLE_PALETTE_TEMPLATE = 13,
	RED_PALETTE_TEMPLATE = 14,
	REDPURPLE_PALETTE_TEMPLATE = 15,
	ROSE_PALETTE_TEMPLATE = 16,
	YELLOW_PALETTE_TEMPLATE = 17,
	YELLOWBROWN_PALETTE_TEMPLATE = 18,
	COPPER_PALETTE_TEMPLATE = 19,
	SILVER_PALETTE_TEMPLATE = 20,
	GOLD_PALETTE_TEMPLATE = 21,
	AQUA_PALETTE_TEMPLATE = 22,
	DARKAQUAMETAL_PALETTE_TEMPLATE = 23,
	DARKBLUEMETAL_PALETTE_TEMPLATE = 24,
	DARKCOPPERMETAL_PALETTE_TEMPLATE = 25,
	DARKGOLDMETAL_PALETTE_TEMPLATE = 26,
	DARKGREENMETAL_PALETTE_TEMPLATE = 27,
	DARKPURPLEMETAL_PALETTE_TEMPLATE = 28,
	DARKREDMETAL_PALETTE_TEMPLATE = 29,
	DARKSILVERMETAL_PALETTE_TEMPLATE = 30,
	LIGHTAQUAMETAL_PALETTE_TEMPLATE = 31,
	LIGHTBLUEMETAL_PALETTE_TEMPLATE = 32,
	LIGHTCOPPERMETAL_PALETTE_TEMPLATE = 33,
	LIGHTGOLDMETAL_PALETTE_TEMPLATE = 34,
	LIGHTGREENMETAL_PALETTE_TEMPLATE = 35,
	LIGHTPURPLEMETAL_PALETTE_TEMPLATE = 36,
	LIGHTREDMETAL_PALETTE_TEMPLATE = 37,
	LIGHTSILVERMETAL_PALETTE_TEMPLATE = 38,
	BLACK_PALETTE_TEMPLATE = 39,
	BRONZE_PALETTE_TEMPLATE = 40,
	SANDYYELLOW_PALETTE_TEMPLATE = 41,
	DARKBROWN_PALETTE_TEMPLATE = 42,
	LIGHTBROWN_PALETTE_TEMPLATE = 43,
	TANRED_PALETTE_TEMPLATE = 44,
	PALEGREEN_PALETTE_TEMPLATE = 45,
	TAN_PALETTE_TEMPLATE = 46,
	PASTYYELLOW_PALETTE_TEMPLATE = 47,
	SNOWYWHITE_PALETTE_TEMPLATE = 48,
	RUDDYYELLOW_PALETTE_TEMPLATE = 49,
	RUDDIERYELLOW_PALETTE_TEMPLATE = 50,
	MIDGREY_PALETTE_TEMPLATE = 51,
	DARKGREY_PALETTE_TEMPLATE = 52,
	BLUEDULLSILVER_PALETTE_TEMPLATE = 53,
	YELLOWPALESILVER_PALETTE_TEMPLATE = 54,
	BROWNBLUEDARK_PALETTE_TEMPLATE = 55,
	BROWNBLUEMED_PALETTE_TEMPLATE = 56,
	GREENSILVER_PALETTE_TEMPLATE = 57,
	BROWNGREEN_PALETTE_TEMPLATE = 58,
	YELLOWGREEN_PALETTE_TEMPLATE = 59,
	PALEPURPLE_PALETTE_TEMPLATE = 60,
	WHITE_PALETTE_TEMPLATE = 61,
	REDBROWN_PALETTE_TEMPLATE = 62,
	GREENBROWN_PALETTE_TEMPLATE = 63,
	ORANGEBROWN_PALETTE_TEMPLATE = 64,
	PALEGREENBROWN_PALETTE_TEMPLATE = 65,
	PALEORANGE_PALETTE_TEMPLATE = 66,
	GREENSLIME_PALETTE_TEMPLATE = 67,
	BLUESLIME_PALETTE_TEMPLATE = 68,
	YELLOWSLIME_PALETTE_TEMPLATE = 69,
	PURPLESLIME_PALETTE_TEMPLATE = 70,
	DULLRED_PALETTE_TEMPLATE = 71,
	GREYWHITE_PALETTE_TEMPLATE = 72,
	MEDIUMGREY_PALETTE_TEMPLATE = 73,
	DULLGREEN_PALETTE_TEMPLATE = 74,
	OLIVEGREEN_PALETTE_TEMPLATE = 75,
	ORANGE_PALETTE_TEMPLATE = 76,
	BLUEGREEN_PALETTE_TEMPLATE = 77,
	OLIVE_PALETTE_TEMPLATE = 78,
	LEAD_PALETTE_TEMPLATE = 79,
	IRON_PALETTE_TEMPLATE = 80,
	LITEGREEN_PALETTE_TEMPLATE = 81,
	PINKPURPLE_PALETTE_TEMPLATE = 82,
	AMBER_PALETTE_TEMPLATE = 83,
	DYEDARKGREEN_PALETTE_TEMPLATE = 84,
	DYEDARKRED_PALETTE_TEMPLATE = 85,
	DYEDARKYELLOW_PALETTE_TEMPLATE = 86,
	DYEBOTCHED_PALETTE_TEMPLATE = 87,
	DYEWINTERBLUE_PALETTE_TEMPLATE = 88,
	DYEWINTERGREEN_PALETTE_TEMPLATE = 89,
	DYEWINTERSILVER_PALETTE_TEMPLATE = 90,
	DYESPRINGBLUE_PALETTE_TEMPLATE = 91,
	DYESPRINGPURPLE_PALETTE_TEMPLATE = 92,
	DYESPRINGBLACK_PALETTE_TEMPLATE = 93
};

enum CG_VERIFICATION_RESPONSE
{
	UNDEF_CG_VERIFICATION_RESPONSE = 0,
	CG_VERIFICATION_RESPONSE_OK,
	CG_VERIFICATION_RESPONSE_PENDING,
	CG_VERIFICATION_RESPONSE_NAME_IN_USE,
	CG_VERIFICATION_RESPONSE_NAME_BANNED,
	CG_VERIFICATION_RESPONSE_CORRUPT,
	CG_VERIFICATION_RESPONSE_DATABASE_DOWN,
	CG_VERIFICATION_RESPONSE_ADMIN_PRIVILEGE_DENIED,
	NUM_CG_VERIFICATION_RESPONSES
};

enum Gender
{
	Invalid_Gender = 0,
	Male_Gender,
	Female_Gender
};

enum HeritageGroup
{
	Invalid_HeritageGroup = 0,
	Aluvian_HeritageGroup,
	Gharundim_HeritageGroup,
	Sho_HeritageGroup,
	Viamontian_HeritageGroup,
	Shadowbound_HeritageGroup,
	Gearknight_HeritageGroup,
	Tumerok_HeritageGroup,
	Lugian_HeritageGroup,
	Empyrean_HeritageGroup,
	Penumbraen_HeritageGroup,
	Undead_HeritageGroup,
	Olthoi_HeritageGroup,
	OlthoiAcid_HeritageGroup
};

enum BODY_HEIGHT
{
	UNDEF_BODY_HEIGHT = 0x0,
	HIGH_BODY_HEIGHT = 0x1,
	MEDIUM_BODY_HEIGHT = 0x2,
	LOW_BODY_HEIGHT = 0x3,
	NUM_BODY_HEIGHTS = 0x4,
	FORCE_BODY_HEIGHT32_BIT = 0x7FFFFFFF,
};

enum EmoteCategory
{
	Invalid_EmoteCategory = 0,
	Refuse_EmoteCategory = 1,
	Vendor_EmoteCategory = 2,
	Death_EmoteCategory = 3,
	Portal_EmoteCategory = 4,
	HeartBeat_EmoteCategory = 5,
	Give_EmoteCategory = 6,
	Use_EmoteCategory = 7,
	Activation_EmoteCategory = 8,
	Generation_EmoteCategory = 9,
	PickUp_EmoteCategory = 10,
	Drop_EmoteCategory = 11,
	QuestSuccess_EmoteCategory = 12,
	QuestFailure_EmoteCategory = 13,
	Taunt_EmoteCategory = 14,
	WoundedTaunt_EmoteCategory = 15,
	KillTaunt_EmoteCategory = 16,
	NewEnemy_EmoteCategory = 17,
	Scream_EmoteCategory = 18,
	Homesick_EmoteCategory = 19,
	ReceiveCritical_EmoteCategory = 20,
	ResistSpell_EmoteCategory = 21,
	TestSuccess_EmoteCategory = 22,
	TestFailure_EmoteCategory = 23,
	HearChat_EmoteCategory = 24,
	Wield_EmoteCategory = 25,
	UnWield_EmoteCategory = 26,
	EventSuccess_EmoteCategory = 27,
	EventFailure_EmoteCategory = 28,
	TestNoQuality_EmoteCategory = 29,
	QuestNoFellow_EmoteCategory = 30,
	TestNoFellow_EmoteCategory = 31,
	GotoSet_EmoteCategory = 32,
	NumFellowsSuccess_EmoteCategory = 33,
	NumFellowsFailure_EmoteCategory = 34,
	NumCharacterTitlesSuccess_EmoteCategory = 35,
	NumCharacterTitlesFailure_EmoteCategory = 36,
	ReceiveLocalSignal_EmoteCategory = 37,
	ReceiveTalkDirect_EmoteCategory = 38,
};

enum VendorTypeEmote // Custom
{
	Undef_VendorTypeEmote = 0,
	Open_VendorTypeEmote = 1,
	Close_VendorTypeEmote = 2,
	Sell_VendorTypeEmote = 3,
	Buy_VendorTypeEmote = 4,
	Heartbeat_VendorTypeEmote = 5
};

enum Command : uint32_t
{
	Command_Invalid = 0x00000000,
	Motion_Invalid = 0x80000000,
	Motion_HoldRun = 0x85000001,
	Motion_HoldSidestep = 0x85000002,
	Motion_Ready = 0x41000003,
	Motion_Stop = 0x40000004,
	Motion_WalkForward = 0x45000005,
	Motion_WalkBackwards = 0x45000006,
	Motion_RunForward = 0x44000007,
	Motion_Fallen = 0x40000008,
	Motion_Interpolating = 0x40000009,
	Motion_Hover = 0x4000000a,
	Motion_On = 0x4000000b,
	Motion_Off = 0x4000000c,
	Motion_TurnRight = 0x6500000d,
	Motion_TurnLeft = 0x6500000e,
	Motion_SideStepRight = 0x6500000f,
	Motion_SideStepLeft = 0x65000010,
	Motion_Dead = 0x40000011,
	Motion_Crouch = 0x41000012,
	Motion_Sitting = 0x41000013,
	Motion_Sleeping = 0x41000014,
	Motion_Falling = 0x40000015,
	Motion_Reload = 0x40000016,
	Motion_Unload = 0x40000017,
	Motion_Pickup = 0x40000018,
	Motion_StoreInBackpack = 0x40000019,
	Motion_Eat = 0x4000001a,
	Motion_Drink = 0x4000001b,
	Motion_Reading = 0x4000001c,
	Motion_JumpCharging = 0x4000001d,
	Motion_AimLevel = 0x4000001e,
	Motion_AimHigh15 = 0x4000001f,
	Motion_AimHigh30 = 0x40000020,
	Motion_AimHigh45 = 0x40000021,
	Motion_AimHigh60 = 0x40000022,
	Motion_AimHigh75 = 0x40000023,
	Motion_AimHigh90 = 0x40000024,
	Motion_AimLow15 = 0x40000025,
	Motion_AimLow30 = 0x40000026,
	Motion_AimLow45 = 0x40000027,
	Motion_AimLow60 = 0x40000028,
	Motion_AimLow75 = 0x40000029,
	Motion_AimLow90 = 0x4000002a,
	Motion_MagicBlast = 0x4000002b,
	Motion_MagicSelfHead = 0x4000002c,
	Motion_MagicSelfHeart = 0x4000002d,
	Motion_MagicBonus = 0x4000002e,
	Motion_MagicClap = 0x4000002f,
	Motion_MagicHarm = 0x40000030,
	Motion_MagicHeal = 0x40000031,
	Motion_MagicThrowMissile = 0x40000032,
	Motion_MagicRecoilMissile = 0x40000033,
	Motion_MagicPenalty = 0x40000034,
	Motion_MagicTransfer = 0x40000035,
	Motion_MagicVision = 0x40000036,
	Motion_MagicEnchantItem = 0x40000037,
	Motion_MagicPortal = 0x40000038,
	Motion_MagicPray = 0x40000039,
	Motion_StopTurning = 0x2000003a,
	Motion_Jump = 0x2500003b,
	Motion_HandCombat = 0x8000003c,
	Motion_NonCombat = 0x8000003d,
	Motion_SwordCombat = 0x8000003e,
	Motion_BowCombat = 0x8000003f,
	Motion_SwordShieldCombat = 0x80000040,
	Motion_CrossbowCombat = 0x80000041,
	Motion_UnusedCombat = 0x80000042,
	Motion_SlingCombat = 0x80000043,
	Motion_2HandedSwordCombat = 0x80000044,
	Motion_2HandedStaffCombat = 0x80000045,
	Motion_DualWieldCombat = 0x80000046,
	Motion_ThrownWeaponCombat = 0x80000047,
	Motion_Graze = 0x80000048,
	Motion_Magic = 0x80000049,
	Motion_Hop = 0x1000004a,
	Motion_Jumpup = 0x1000004b,
	Motion_Cheer = 0x1300004c,
	Motion_ChestBeat = 0x1000004d,
	Motion_TippedLeft = 0x1000004e,
	Motion_TippedRight = 0x1000004f,
	Motion_FallDown = 0x10000050,
	Motion_Twitch1 = 0x10000051,
	Motion_Twitch2 = 0x10000052,
	Motion_Twitch3 = 0x10000053,
	Motion_Twitch4 = 0x10000054,
	Motion_StaggerBackward = 0x10000055,
	Motion_StaggerForward = 0x10000056,
	Motion_Sanctuary = 0x10000057,
	Motion_ThrustMed = 0x10000058,
	Motion_ThrustLow = 0x10000059,
	Motion_ThrustHigh = 0x1000005a,
	Motion_SlashHigh = 0x1000005b,
	Motion_SlashMed = 0x1000005c,
	Motion_SlashLow = 0x1000005d,
	Motion_BackhandHigh = 0x1000005e,
	Motion_BackhandMed = 0x1000005f,
	Motion_BackhandLow = 0x10000060,
	Motion_Shoot = 0x10000061,
	Motion_AttackHigh1 = 0x10000062,
	Motion_AttackMed1 = 0x10000063,
	Motion_AttackLow1 = 0x10000064,
	Motion_AttackHigh2 = 0x10000065,
	Motion_AttackMed2 = 0x10000066,
	Motion_AttackLow2 = 0x10000067,
	Motion_AttackHigh3 = 0x10000068,
	Motion_AttackMed3 = 0x10000069,
	Motion_AttackLow3 = 0x1000006a,
	Motion_HeadThrow = 0x1000006b,
	Motion_FistSlam = 0x1000006c,
	Motion_BreatheFlame_ = 0x1000006d,
	Motion_SpinAttack = 0x1000006e,
	Motion_MagicPowerUp01 = 0x1000006f,
	Motion_MagicPowerUp02 = 0x10000070,
	Motion_MagicPowerUp03 = 0x10000071,
	Motion_MagicPowerUp04 = 0x10000072,
	Motion_MagicPowerUp05 = 0x10000073,
	Motion_MagicPowerUp06 = 0x10000074,
	Motion_MagicPowerUp07 = 0x10000075,
	Motion_MagicPowerUp08 = 0x10000076,
	Motion_MagicPowerUp09 = 0x10000077,
	Motion_MagicPowerUp10 = 0x10000078,
	Motion_ShakeFist = 0x13000079,
	Motion_Beckon = 0x1300007a,
	Motion_BeSeeingYou = 0x1300007b,
	Motion_BlowKiss = 0x1300007c,
	Motion_BowDeep = 0x1300007d,
	Motion_ClapHands = 0x1300007e,
	Motion_Cry = 0x1300007f,
	Motion_Laugh = 0x13000080,
	Motion_MimeEat = 0x13000081,
	Motion_MimeDrink = 0x13000082,
	Motion_Nod = 0x13000083,
	Motion_Point = 0x13000084,
	Motion_ShakeHead = 0x13000085,
	Motion_Shrug = 0x13000086,
	Motion_Wave = 0x13000087,
	Motion_Akimbo = 0x13000088,
	Motion_HeartyLaugh = 0x13000089,
	Motion_Salute = 0x1300008a,
	Motion_ScratchHead = 0x1300008b,
	Motion_SmackHead = 0x1300008c,
	Motion_TapFoot = 0x1300008d,
	Motion_WaveHigh = 0x1300008e,
	Motion_WaveLow = 0x1300008f,
	Motion_YawnStretch = 0x13000090,
	Motion_Cringe = 0x13000091,
	Motion_Kneel = 0x13000092,
	Motion_Plead = 0x13000093,
	Motion_Shiver = 0x13000094,
	Motion_Shoo = 0x13000095,
	Motion_Slouch = 0x13000096,
	Motion_Spit = 0x13000097,
	Motion_Surrender = 0x13000098,
	Motion_Woah = 0x13000099,
	Motion_Winded = 0x1300009a,
	Motion_YMCA = 0x1200009b,
	Motion_EnterGame = 0x1000009c,
	Motion_ExitGame = 0x1000009d,
	Motion_OnCreation = 0x1000009e,
	Motion_OnDestruction = 0x1000009f,
	Motion_EnterPortal = 0x100000a0,
	Motion_ExitPortal = 0x100000a1,
	Command_Cancel = 0x80000a2,
	Command_UseSelected = 0x90000a3,
	Command_AutosortSelected = 0x90000a4,
	Command_DropSelected = 0x90000a5,
	Command_GiveSelected = 0x90000a6,
	Command_SplitSelected = 0x90000a7,
	Command_ExamineSelected = 0x90000a8,
	Command_CreateShortcutToSelected = 0x80000a9,
	Command_PreviousCompassItem = 0x90000aa,
	Command_NextCompassItem = 0x90000ab,
	Command_ClosestCompassItem = 0x90000ac,
	Command_PreviousSelection = 0x90000ad,
	Command_LastAttacker = 0x90000ae,
	Command_PreviousFellow = 0x90000af,
	Command_NextFellow = 0x90000b0,
	Command_ToggleCombat = 0x90000b1,
	Command_HighAttack = 0xd0000b2,
	Command_MediumAttack = 0xd0000b3,
	Command_LowAttack = 0xd0000b4,
	Command_EnterChat = 0x80000b5,
	Command_ToggleChat = 0x80000b6,
	Command_SavePosition = 0x80000b7,
	Command_OptionsPanel = 0x90000b8,
	Command_ResetView = 0x90000b9,
	Command_CameraLeftRotate = 0xd0000ba,
	Command_CameraRightRotate = 0xd0000bb,
	Command_CameraRaise = 0xd0000bc,
	Command_CameraLower = 0xd0000bd,
	Command_CameraCloser = 0xd0000be,
	Command_CameraFarther = 0xd0000bf,
	Command_FloorView = 0x90000c0,
	Command_MouseLook = 0xc0000c1,
	Command_PreviousItem = 0x90000c2,
	Command_NextItem = 0x90000c3,
	Command_ClosestItem = 0x90000c4,
	Command_ShiftView = 0xd0000c5,
	Command_MapView = 0x90000c6,
	Command_AutoRun = 0x90000c7,
	Command_DecreasePowerSetting = 0x90000c8,
	Command_IncreasePowerSetting = 0x90000c9,
	Motion_Pray = 0x130000ca,
	Motion_Mock = 0x130000cb,
	Motion_Teapot = 0x130000cc,
	Motion_SpecialAttack1 = 0x100000cd,
	Motion_SpecialAttack2 = 0x100000ce,
	Motion_SpecialAttack3 = 0x100000cf,
	Motion_MissileAttack1 = 0x100000d0,
	Motion_MissileAttack2 = 0x100000d1,
	Motion_MissileAttack3 = 0x100000d2,
	Motion_CastSpell = 0x400000d3,
	Motion_Flatulence = 0x120000d4,
	Command_FirstPersonView = 0x90000d5,
	Command_AllegiancePanel = 0x90000d6,
	Command_FellowshipPanel = 0x90000d7,
	Command_SpellbookPanel = 0x90000d8,
	Command_SpellComponentsPanel = 0x90000d9,
	Command_HousePanel = 0x90000da,
	Command_AttributesPanel = 0x90000db,
	Command_SkillsPanel = 0x90000dc,
	Command_MapPanel = 0x90000dd,
	Command_InventoryPanel = 0x90000de,
	Motion_Demonet = 0x120000df,
	Motion_UseMagicStaff = 0x400000e0,
	Motion_UseMagicWand = 0x400000e1,
	Motion_Blink = 0x100000e2,
	Motion_Bite = 0x100000e3,
	Motion_TwitchSubstate1 = 0x400000e4,
	Motion_TwitchSubstate2 = 0x400000e5,
	Motion_TwitchSubstate3 = 0x400000e6,
	Command_CaptureScreenshotToFile = 0x90000e7,
	Motion_BowNoAmmo = 0x800000e8,
	Motion_CrossBowNoAmmo = 0x800000e9,
	Motion_ShakeFistState = 0x430000ea,
	Motion_PrayState = 0x430000eb,
	Motion_BowDeepState = 0x430000ec,
	Motion_ClapHandsState = 0x430000ed,
	Motion_CrossArmsState = 0x430000ee,
	Motion_ShiverState = 0x430000ef,
	Motion_PointState = 0x430000f0,
	Motion_WaveState = 0x430000f1,
	Motion_AkimboState = 0x430000f2,
	Motion_SaluteState = 0x430000f3,
	Motion_ScratchHeadState = 0x430000f4,
	Motion_TapFootState = 0x430000f5,
	Motion_LeanState = 0x430000f6,
	Motion_KneelState = 0x430000f7,
	Motion_PleadState = 0x430000f8,
	Motion_ATOYOT = 0x420000f9,
	Motion_SlouchState = 0x430000fa,
	Motion_SurrenderState = 0x430000fb,
	Motion_WoahState = 0x430000fc,
	Motion_WindedState = 0x430000fd,
	Command_AutoCreateShortcuts = 0x90000fe,
	Command_AutoRepeatAttacks = 0x90000ff,
	Command_AutoTarget = 0x9000100,
	Command_AdvancedCombatInterface = 0x9000101,
	Command_IgnoreAllegianceRequests = 0x9000102,
	Command_IgnoreFellowshipRequests = 0x9000103,
	Command_InvertMouseLook = 0x9000104,
	Command_LetPlayersGiveYouItems = 0x9000105,
	Command_AutoTrackCombatTargets = 0x9000106,
	Command_DisplayTooltips = 0x9000107,
	Command_AttemptToDeceivePlayers = 0x9000108,
	Command_RunAsDefaultMovement = 0x9000109,
	Command_StayInChatModeAfterSend = 0x900010a,
	Command_RightClickToMouseLook = 0x900010b,
	Command_VividTargetIndicator = 0x900010c,
	Command_SelectSelf = 0x900010d,
	Motion_SkillHealSelf = 0x1000010e,
	Command_NextMonster = 0x900010f,
	Command_PreviousMonster = 0x9000110,
	Command_ClosestMonster = 0x9000111,
	Command_NextPlayer = 0x9000112,
	Command_PreviousPlayer = 0x9000113,
	Command_ClosestPlayer = 0x9000114,

	// new
	Command_Unk115 = 0x9000115,
	Command_Unk116 = 0x9000116,
	Command_Unk117 = 0x9000117,

	Motion_SnowAngelState = 0x43000118,
	Motion_WarmHands = 0x13000119,
	Motion_CurtseyState = 0x4300011a,
	Motion_AFKState = 0x4300011b,
	Motion_MeditateState = 0x4300011c,
	Command_TradePanel = 0x900011d,
	Motion_LogOut = 0x1000011e,
	Motion_DoubleSlashLow = 0x1000011f,
	Motion_DoubleSlashMed = 0x10000120,
	Motion_DoubleSlashHigh = 0x10000121,
	Motion_TripleSlashLow = 0x10000122,
	Motion_TripleSlashMed = 0x10000123,
	Motion_TripleSlashHigh = 0x10000124,
	Motion_DoubleThrustLow = 0x10000125,
	Motion_DoubleThrustMed = 0x10000126,
	Motion_DoubleThrustHigh = 0x10000127,
	Motion_TripleThrustLow = 0x10000128,
	Motion_TripleThrustMed = 0x10000129,
	Motion_TripleThrustHigh = 0x1000012a,
	Motion_MagicPowerUp01Purple = 0x1000012b,
	Motion_MagicPowerUp02Purple = 0x1000012c,
	Motion_MagicPowerUp03Purple = 0x1000012d,
	Motion_MagicPowerUp04Purple = 0x1000012e,
	Motion_MagicPowerUp05Purple = 0x1000012f,
	Motion_MagicPowerUp06Purple = 0x10000130,
	Motion_MagicPowerUp07Purple = 0x10000131,
	Motion_MagicPowerUp08Purple = 0x10000132,
	Motion_MagicPowerUp09Purple = 0x10000133,
	Motion_MagicPowerUp10Purple = 0x10000134,
	Motion_Helper = 0x13000135,
	Motion_Pickup5 = 0x40000136,
	Motion_Pickup10 = 0x40000137,
	Motion_Pickup15 = 0x40000138,
	Motion_Pickup20 = 0x40000139,
	Motion_HouseRecall = 0x1000013a,
	Motion_AtlatlCombat = 0x8000013b,
	Motion_ThrownShieldCombat = 0x8000013c,
	Motion_SitState = 0x4300013d,
	Motion_SitCrossleggedState = 0x4300013e,
	Motion_SitBackState = 0x4300013f,
	Motion_PointLeftState = 0x43000140,
	Motion_PointRightState = 0x43000141,
	Motion_TalktotheHandState = 0x43000142,
	Motion_PointDownState = 0x43000143,
	Motion_DrudgeDanceState = 0x43000144,
	Motion_PossumState = 0x43000145,
	Motion_ReadState = 0x43000146,
	Motion_ThinkerState = 0x43000147,
	Motion_HaveASeatState = 0x43000148,
	Motion_AtEaseState = 0x43000149,
	Motion_NudgeLeft = 0x1300014a,
	Motion_NudgeRight = 0x1300014b,
	Motion_PointLeft = 0x1300014c,
	Motion_PointRight = 0x1300014d,
	Motion_PointDown = 0x1300014e,
	Motion_Knock = 0x1300014f,
	Motion_ScanHorizon = 0x13000150,
	Motion_DrudgeDance = 0x13000151,
	Motion_HaveASeat = 0x13000152,
	Motion_LifestoneRecall = 0x10000153,
	Command_CharacterOptionsPanel = 0x9000154,
	Command_SoundAndGraphicsPanel = 0x9000155,
	Command_HelpfulSpellsPanel = 0x9000156,
	Command_HarmfulSpellsPanel = 0x9000157,
	Command_CharacterInformationPanel = 0x9000158,
	Command_LinkStatusPanel = 0x9000159,
	Command_VitaePanel = 0x900015a,
	Command_ShareFellowshipXP = 0x900015b,
	Command_ShareFellowshipLoot = 0x900015c,
	Command_AcceptCorpseLooting = 0x900015d,
	Command_IgnoreTradeRequests = 0x900015e,
	Command_DisableWeather = 0x900015f,
	Command_DisableHouseEffect = 0x9000160,
	Command_SideBySideVitals = 0x9000161,
	Command_ShowRadarCoordinates = 0x9000162,
	Command_ShowSpellDurations = 0x9000163,
	Command_MuteOnLosingFocus = 0x9000164,
	Motion_Fishing = 0x10000165,
	Motion_MarketplaceRecall = 0x10000166,
	Motion_EnterPKLite = 0x10000167,
	Command_AllegianceChat = 0x9000168,
	Command_AutomaticallyAcceptFellowshipRequests = 0x9000169,
	Command_Reply = 0x900016a,
	Command_MonarchReply = 0x900016b,
	Command_PatronReply = 0x900016c,
	Command_ToggleCraftingChanceOfSuccessDialog = 0x900016d,
	Command_UseClosestUnopenedCorpse = 0x900016e,
	Command_UseNextUnopenedCorpse = 0x900016f,
	Command_IssueSlashCommand = 0x9000170,
	Motion_AllegianceHometownRecall = 0x10000171,
	Motion_PKArenaRecall = 0x10000172,
	Motion_OffhandSlashHigh = 0x10000173,
	Motion_OffhandSlashMed = 0x10000174,
	Motion_OffhandSlashLow = 0x10000175,
	Motion_OffhandThrustHigh = 0x10000176,
	Motion_OffhandThrustMed = 0x10000177,
	Motion_OffhandThrustLow = 0x10000178,
	Motion_OffhandDoubleSlashLow = 0x10000179,
	Motion_OffhandDoubleSlashMed = 0x1000017a,
	Motion_OffhandDoubleSlashHigh = 0x1000017b,
	Motion_OffhandTripleSlashLow = 0x1000017c,
	Motion_OffhandTripleSlashMed = 0x1000017d,
	Motion_OffhandTripleSlashHigh = 0x1000017e,
	Motion_OffhandDoubleThrustLow = 0x1000017f,
	Motion_OffhandDoubleThrustMed = 0x10000180,
	Motion_OffhandDoubleThrustHigh = 0x10000181,
	Motion_OffhandTripleThrustLow = 0x10000182,
	Motion_OffhandTripleThrustMed = 0x10000183,
	Motion_OffhandTripleThrustHigh = 0x10000184,
	Motion_OffhandKick = 0x10000185,
	Motion_AttackHigh4 = 0x10000186,
	Motion_AttackMed4 = 0x10000187,
	Motion_AttackLow4 = 0x10000188,
	Motion_AttackHigh5 = 0x10000189,
	Motion_AttackMed5 = 0x1000018a,
	Motion_AttackLow5 = 0x1000018b,
	Motion_AttackHigh6 = 0x1000018c,
	Motion_AttackMed6 = 0x1000018d,
	Motion_AttackLow6 = 0x1000018e,
	Motion_PunchFastHigh = 0x1000018f,
	Motion_PunchFastMed = 0x10000190,
	Motion_PunchFastLow = 0x10000191,
	Motion_PunchSlowHigh = 0x10000192,
	Motion_PunchSlowMed = 0x10000193,
	Motion_PunchSlowLow = 0x10000194,
	Motion_OffhandPunchFastHigh = 0x10000195,
	Motion_OffhandPunchFastMed = 0x10000196,
	Motion_OffhandPunchFastLow = 0x10000197,
	Motion_OffhandPunchSlowHigh = 0x10000198,
	Motion_OffhandPunchSlowMed = 0x10000199,
	Motion_OffhandPunchSlowLow = 0x1000019a

	/*
Motion_SnowAngelState = 0x43000115,
Motion_WarmHands = 0x13000116,
Motion_CurtseyState = 0x43000117,
Motion_AFKState = 0x43000118,
Motion_MeditateState = 0x43000119,
Command_TradePanel = 0x900011a,
Motion_LogOut = 0x1000011b,
Motion_DoubleSlashLow = 0x1000011c,
Motion_DoubleSlashMed = 0x1000011d,
Motion_DoubleSlashHigh = 0x1000011e,
Motion_TripleSlashLow = 0x1000011f,
Motion_TripleSlashMed = 0x10000120,
Motion_TripleSlashHigh = 0x10000121,
Motion_DoubleThrustLow = 0x10000122,
Motion_DoubleThrustMed = 0x10000123,
Motion_DoubleThrustHigh = 0x10000124,
Motion_TripleThrustLow = 0x10000125,
Motion_TripleThrustMed = 0x10000126,
Motion_TripleThrustHigh = 0x10000127,
Motion_MagicPowerUp01Purple = 0x10000128,
Motion_MagicPowerUp02Purple = 0x10000129,
Motion_MagicPowerUp03Purple = 0x1000012a,
Motion_MagicPowerUp04Purple = 0x1000012b,
Motion_MagicPowerUp05Purple = 0x1000012c,
Motion_MagicPowerUp06Purple = 0x1000012d,
Motion_MagicPowerUp07Purple = 0x1000012e,
Motion_MagicPowerUp08Purple = 0x1000012f,
Motion_MagicPowerUp09Purple = 0x10000130,
Motion_MagicPowerUp10Purple = 0x10000131,
Motion_Helper = 0x13000132,
Motion_Pickup5 = 0x40000133,
Motion_Pickup10 = 0x40000134,
Motion_Pickup15 = 0x40000135,
Motion_Pickup20 = 0x40000136,
Motion_HouseRecall = 0x10000137,
Motion_AtlatlCombat = 0x80000138,
Motion_ThrownShieldCombat = 0x80000139,
Motion_SitState = 0x4300013a,
Motion_SitCrossleggedState = 0x4300013b,
Motion_SitBackState = 0x4300013c,
Motion_PointLeftState = 0x4300013d,
Motion_PointRightState = 0x4300013e,
Motion_TalktotheHandState = 0x4300013f,
Motion_PointDownState = 0x43000140,
Motion_DrudgeDanceState = 0x43000141,
Motion_PossumState = 0x43000142,
Motion_ReadState = 0x43000143,
Motion_ThinkerState = 0x43000144,
Motion_HaveASeatState = 0x43000145,
Motion_AtEaseState = 0x43000146,
Motion_NudgeLeft = 0x13000147,
Motion_NudgeRight = 0x13000148,
Motion_PointLeft = 0x13000149,
Motion_PointRight = 0x1300014a,
Motion_PointDown = 0x1300014b,
Motion_Knock = 0x1300014c,
Motion_ScanHorizon = 0x1300014d,
Motion_DrudgeDance = 0x1300014e,
Motion_HaveASeat = 0x1300014f,
Motion_LifestoneRecall = 0x10000150,
Command_CharacterOptionsPanel = 0x9000151,
Command_SoundAndGraphicsPanel = 0x9000152,
Command_HelpfulSpellsPanel = 0x9000153,
Command_HarmfulSpellsPanel = 0x9000154,
Command_CharacterInformationPanel = 0x9000155,
Command_LinkStatusPanel = 0x9000156,
Command_VitaePanel = 0x9000157,
Command_ShareFellowshipXP = 0x9000158,
Command_ShareFellowshipLoot = 0x9000159,
Command_AcceptCorpseLooting = 0x900015a,
Command_IgnoreTradeRequests = 0x900015b,
Command_DisableWeather = 0x900015c,
Command_DisableHouseEffect = 0x900015d,
Command_SideBySideVitals = 0x900015e,
Command_ShowRadarCoordinates = 0x900015f,
Command_ShowSpellDurations = 0x9000160,
Command_MuteOnLosingFocus = 0x9000161,
Motion_Fishing = 0x10000162,
Motion_MarketplaceRecall = 0x10000163,
Motion_EnterPKLite = 0x10000164,
Command_AllegianceChat = 0x9000165,
Command_AutomaticallyAcceptFellowshipRequests = 0x9000166,
Command_Reply = 0x9000167,
Command_MonarchReply = 0x9000168,
Command_PatronReply = 0x9000169,
Command_ToggleCraftingChanceOfSuccessDialog = 0x900016a,
Command_UseClosestUnopenedCorpse = 0x900016b,
Command_UseNextUnopenedCorpse = 0x900016c,
Command_IssueSlashCommand = 0x900016d,
Motion_AllegianceHometownRecall = 0x1000016e,
Motion_PKArenaRecall = 0x1000016f,
Motion_OffhandSlashHigh = 0x10000170,
Motion_OffhandSlashMed = 0x10000171,
Motion_OffhandSlashLow = 0x10000172,
Motion_OffhandThrustHigh = 0x10000173,
Motion_OffhandThrustMed = 0x10000174,
Motion_OffhandThrustLow = 0x10000175,
Motion_OffhandDoubleSlashLow = 0x10000176,
Motion_OffhandDoubleSlashMed = 0x10000177,
Motion_OffhandDoubleSlashHigh = 0x10000178,
Motion_OffhandTripleSlashLow = 0x10000179,
Motion_OffhandTripleSlashMed = 0x1000017a,
Motion_OffhandTripleSlashHigh = 0x1000017b,
Motion_OffhandDoubleThrustLow = 0x1000017c,
Motion_OffhandDoubleThrustMed = 0x1000017d,
Motion_OffhandDoubleThrustHigh = 0x1000017e,
Motion_OffhandTripleThrustLow = 0x1000017f,
Motion_OffhandTripleThrustMed = 0x10000180,
Motion_OffhandTripleThrustHigh = 0x10000181,
Motion_OffhandKick = 0x10000182,
Motion_AttackHigh4 = 0x10000183,
Motion_AttackMed4 = 0x10000184,
Motion_AttackLow4 = 0x10000185,
Motion_AttackHigh5 = 0x10000186,
Motion_AttackMed5 = 0x10000187,
Motion_AttackLow5 = 0x10000188,
Motion_AttackHigh6 = 0x10000189,
Motion_AttackMed6 = 0x1000018a,
Motion_AttackLow6 = 0x1000018b,
Motion_PunchFastHigh = 0x1000018c,
Motion_PunchFastMed = 0x1000018d,
Motion_PunchFastLow = 0x1000018e,
Motion_PunchSlowHigh = 0x1000018f,
Motion_PunchSlowMed = 0x10000190,
Motion_PunchSlowLow = 0x10000191,
Motion_OffhandPunchFastHigh = 0x10000192,
Motion_OffhandPunchFastMed = 0x10000193,
Motion_OffhandPunchFastLow = 0x10000194,
Motion_OffhandPunchSlowHigh = 0x10000195,
Motion_OffhandPunchSlowMed = 0x10000196,
Motion_OffhandPunchSlowLow = 0x10000197
*/
};

enum EmoteType
{
	Invalid_EmoteType = 0,
	Act_EmoteType = 1,
	AwardXP_EmoteType = 2,
	Give_EmoteType = 3,
	MoveHome_EmoteType = 4, //partial implementation
	Motion_EmoteType = 5,
	Move_EmoteType = 6, //not implemented
	PhysScript_EmoteType = 7,
	Say_EmoteType = 8,
	Sound_EmoteType = 9,
	Tell_EmoteType = 10,
	Turn_EmoteType = 11,
	TurnToTarget_EmoteType = 12,
	TextDirect_EmoteType = 13,
	CastSpell_EmoteType = 14,
	Activate_EmoteType = 15,
	WorldBroadcast_EmoteType = 16,
	LocalBroadcast_EmoteType = 17,
	DirectBroadcast_EmoteType = 18,
	CastSpellInstant_EmoteType = 19,
	UpdateQuest_EmoteType = 20,
	InqQuest_EmoteType = 21,
	StampQuest_EmoteType = 22,
	StartEvent_EmoteType = 23,
	StopEvent_EmoteType = 24,
	BLog_EmoteType = 25,
	AdminSpam_EmoteType = 26,
	TeachSpell_EmoteType = 27,
	AwardSkillXP_EmoteType = 28,
	AwardSkillPoints_EmoteType = 29,
	InqQuestSolves_EmoteType = 30,
	EraseQuest_EmoteType = 31,
	DecrementQuest_EmoteType = 32,
	IncrementQuest_EmoteType = 33,
	AddCharacterTitle_EmoteType = 34, //not implemented
	InqBoolStat_EmoteType = 35,
	InqIntStat_EmoteType = 36,
	InqFloatStat_EmoteType = 37,
	InqStringStat_EmoteType = 38,
	InqAttributeStat_EmoteType = 39,
	InqRawAttributeStat_EmoteType = 40,
	InqSecondaryAttributeStat_EmoteType = 41,
	InqRawSecondaryAttributeStat_EmoteType = 42,
	InqSkillStat_EmoteType = 43,
	InqRawSkillStat_EmoteType = 44,
	InqSkillTrained_EmoteType = 45,
	InqSkillSpecialized_EmoteType = 46,
	AwardTrainingCredits_EmoteType = 47,
	InflictVitaePenalty_EmoteType = 48,
	AwardLevelProportionalXP_EmoteType = 49,
	AwardLevelProportionalSkillXP_EmoteType = 50,
	InqEvent_EmoteType = 51,
	ForceMotion_EmoteType = 52,
	SetIntStat_EmoteType = 53,
	IncrementIntStat_EmoteType = 54,
	DecrementIntStat_EmoteType = 55,
	CreateTreasure_EmoteType = 56,
	ResetHomePosition_EmoteType = 57,
	InqFellowQuest_EmoteType = 58,
	InqFellowNum_EmoteType = 59,
	UpdateFellowQuest_EmoteType = 60, //not implemented
	StampFellowQuest_EmoteType = 61,
	AwardNoShareXP_EmoteType = 62,
	SetSanctuaryPosition_EmoteType = 63,
	TellFellow_EmoteType = 64,
	FellowBroadcast_EmoteType = 65,
	LockFellow_EmoteType = 66, //not implemented
	Goto_EmoteType = 67,
	//none below this are implemented nor are they used by our data set.
	PopUp_EmoteType = 68,
	SetBoolStat_EmoteType = 69,
	SetQuestCompletions_EmoteType = 70, //Functionality Implemented
	InqNumCharacterTitles_EmoteType = 71,
	Generate_EmoteType = 72, //Functionality Implemented
	PetCastSpellOnOwner_EmoteType = 73,
	TakeItems_EmoteType = 74,
	InqYesNo_EmoteType = 75,
	InqOwnsItems_EmoteType = 76,
	DeleteSelf_EmoteType = 77,
	KillSelf_EmoteType = 78,
	UpdateMyQuest_EmoteType = 79,
	InqMyQuest_EmoteType = 80,
	StampMyQuest_EmoteType = 81,
	InqMyQuestSolves_EmoteType = 82,
	EraseMyQuest_EmoteType = 83,
	DecrementMyQuest_EmoteType = 84,
	IncrementMyQuest_EmoteType = 85,
	SetMyQuestCompletions_EmoteType = 86,
	MoveToPos_EmoteType = 87,
	LocalSignal_EmoteType = 88,
	InqPackSpace_EmoteType = 89,
	RemoveVitaePenalty_EmoteType = 90,
	SetEyeTexture_EmoteType = 91,
	SetEyePalette_EmoteType = 92,
	SetNoseTexture_EmoteType = 93,
	SetNosePalette_EmoteType = 94,
	SetMouthTexture_EmoteType = 95,
	SetMouthPalette_EmoteType = 96,
	SetHeadObject_EmoteType = 97,
	SetHeadPalette_EmoteType = 98,
	TeleportTarget_EmoteType = 99,
	TeleportSelf_EmoteType = 100,
	StartBarber_EmoteType = 101,
	InqQuestBitsOn_EmoteType = 102,
	InqQuestBitsOff_EmoteType = 103,
	InqMyQuestBitsOn_EmoteType = 104,
	InqMyQuestBitsOff_EmoteType = 105,
	SetQuestBitsOn_EmoteType = 106,
	SetQuestBitsOff_EmoteType = 107,
	SetMyQuestBitsOn_EmoteType = 108,
	SetMyQuestBitsOff_EmoteType = 109,
	UntrainSkill_EmoteType = 110,
	SetAltRacialSkills_EmoteType = 111,
	SpendLuminance_EmoteType = 112,
	AwardLuminance_EmoteType = 113,
	InqInt64Stat_EmoteType = 114,
	SetInt64Stat_EmoteType = 115,
	OpenMe_EmoteType = 116,
	CloseMe_EmoteType = 117,
	SetFloatStat_EmoteType = 118,
	AddContract_EmoteType = 119,
	RemoveContract_EmoteType = 120,
	InqContractsFull_EmoteType = 121
};

enum RegenerationType
{
	Undef_RegenerationType = 0,
	Destruction_RegenerationType = 1,
	PickUp_RegenerationType = 2,
	Death_RegenerationType = 4
};

enum RegenLocationType
{
	Undef_RegenLocationType = 0x0,
	OnTop_RegenLocationType = 0x1,
	Scatter_RegenLocationType = 0x2,
	Specific_RegenLocationType = 0x4,
	Contain_RegenLocationType = 0x8,
	Wield_RegenLocationType = 0x10,
	Shop_RegenLocationType = 0x20,
	Checkpoint_RegenLocationType = 0x38,
	Treasure_RegenLocationType = 0x40,
	OnTopTreasure_RegenLocationType = 0x41,
	ScatterTreasure_RegenLocationType = 0x42,
	SpecificTreasure_RegenLocationType = 0x44,
	ContainTreasure_RegenLocationType = 0x48,
	WieldTreasure_RegenLocationType = 0x50,
	ShopTreasure_RegenLocationType = 0x60
};

enum ATTACK_HEIGHT
{
	UNDEF_ATTACK_HEIGHT = 0x0,
	HIGH_ATTACK_HEIGHT = 0x1,
	MEDIUM_ATTACK_HEIGHT = 0x2,
	LOW_ATTACK_HEIGHT = 0x3,
	NUM_ATTACK_HEIGHTS = 0x4,
	FORCE_ATTACK_HEIGHT_32_BIT = 0x7FFFFFFF,
};

enum AttackType
{
	Undef_AttackType = 0x0,
	Punch_AttackType = 0x1,
	Thrust_AttackType = 0x2,
	Slash_AttackType = 0x4,
	Kick_AttackType = 0x8,
	OffhandPunch_AttackType = 0x10,
	DoubleSlash_AttackType = 0x20,
	TripleSlash_AttackType = 0x40,
	DoubleThrust_AttackType = 0x80,
	TripleThrust_AttackType = 0x100,
	OffhandThrust_AttackType = 0x200,
	OffhandSlash_AttackType = 0x400,
	OffhandDoubleSlash_AttackType = 0x800,
	OffhandTripleSlash_AttackType = 0x1000,
	OffhandDoubleThrust_AttackType = 0x2000,
	OffhandTripleThrust_AttackType = 0x4000,
	Unarmed_AttackType = 0x19,
	MultiStrike_AttackType = 0x79E0,
	FORCE_AttackType_32_BIT = 0x7FFFFFFF,
};

enum WealthRating
{
	Undef_WealthRating = 0,
	Shoddy_WealthRating = 1,
	Poor_WealthRating = 2,
	Medium_WealthRating = 3,
	Good_WealthRating = 4,
	Rich_WealthRating = 5,
	Incomparable_WealthRating = 6,
	NumWealthRatings_WealthRating = 6
};

enum PlayerOptions
{
	// this isnt a bitshift into characteroptions
	Invalid_PlayerOption = 0xFFFFFFFF,
	AutoRepeatAttack_PlayerOption = 0x0, // 1
	IgnoreAllegianceRequests_PlayerOption = 0x1, // 2
	IgnoreFellowshipRequests_PlayerOption = 0x2, // 4
	IgnoreTradeRequests_PlayerOption = 0x3, // 8
	DisableMostWeatherEffects_PlayerOption = 0x4, // 0x10
	PersistentAtDay_PlayerOption = 0x5, // 0x20
	AllowGive_PlayerOption = 0x6, // 0x40
	ViewCombatTarget_PlayerOption = 0x7, // 0x80
	ShowTooltips_PlayerOption = 0x8, // 0x100
	UseDeception_PlayerOption = 0x9, // 0x200
	ToggleRun_PlayerOption = 0xA, // 0x400
	StayInChatMode_PlayerOption = 0xB, // 0x800
	AdvancedCombatUI_PlayerOption = 0xC, // 0x1000
	AutoTarget_PlayerOption = 0xD, // 0x2000
	VividTargetingIndicator_PlayerOption = 0xE, // 0x4000
	FellowshipShareXP_PlayerOption = 0xF, // 0x8000
	AcceptLootPermits_PlayerOption = 0x10, // 0x10000
	FellowshipShareLoot_PlayerOption = 0x11, // 0x20000
	FellowshipAutoAcceptRequests_PlayerOption = 0x12, // 0x40000
	SideBySideVitals_PlayerOption = 0x13, // 0x80000
	CoordinatesOnRadar_PlayerOption = 0x14, // 0x100000
	SpellDuration_PlayerOption = 0x15, // 0x200000
	DisableHouseRestrictionEffects_PlayerOption = 0x16, // 0x400000
	DragItemOnPlayerOpensSecureTrade_PlayerOption = 0x17, // 0x800000
	DisplayAllegianceLogonNotifications_PlayerOption = 0x18, // 0x1000000
	UseChargeAttack_PlayerOption = 0x19, // 0x2000000
	UseCraftSuccessDialog_PlayerOption = 0x1A, // 0x4000000 // 31
	HearAllegianceChat_PlayerOption = 0x1B, // 0x8000000 // 30
	DisplayDateOfBirth_PlayerOption = 0x1C,
	DisplayAge_PlayerOption = 0x1D,
	DisplayChessRank_PlayerOption = 0x1E,
	DisplayFishingSkill_PlayerOption = 0x1F,
	DisplayNumberDeaths_PlayerOption = 0x20,
	DisplayTimeStamps_PlayerOption = 0x21,
	SalvageMultiple_PlayerOption = 0x22,
	HearGeneralChat_PlayerOption = 0x23,
	HearTradeChat_PlayerOption = 0x24,
	HearLFGChat_PlayerOption = 0x25,
	HearRoleplayChat_PlayerOption = 0x26,
	AppearOffline_PlayerOption = 0x27,
	DisplayNumberCharacterTitles_PlayerOption = 0x28,
	MainPackPreferred_PlayerOption = 0x29,
	LeadMissileTargets_PlayerOption = 0x2A,
	UseFastMissiles_PlayerOption = 0x2B,
	FilterLanguage_PlayerOption = 0x2C,
	ConfirmVolatileRareUse_PlayerOption = 0x2D,
	HearSocietyChat_PlayerOption = 0x2E,
	ShowHelm_PlayerOption = 0x2F,
	DisableDistanceFog_PlayerOption = 0x30,
	UseMouseTurning_PlayerOption = 0x31,
	ShowCloak_PlayerOption = 0x32,
	LockUI_PlayerOption = 0x33,
	TotalNumberOfPlayerOptions_PlayerOption = 0x34
};

enum CharacterOption
{
	Undef_CharacterOption = 0x0,
	AutoRepeatAttack_CharacterOption = 0x2,
	IgnoreAllegianceRequests_CharacterOption = 0x4,
	IgnoreFellowshipRequests_CharacterOption = 0x8,
	AllowGive_CharacterOption = 0x40,
	ViewCombatTarget_CharacterOption = 0x80,
	ShowTooltips_CharacterOption = 0x100,
	UseDeception_CharacterOption = 0x200,
	ToggleRun_CharacterOption = 0x400,
	StayInChatMode_CharacterOption = 0x800,
	AdvancedCombatUI_CharacterOption = 0x1000,
	AutoTarget_CharacterOption = 0x2000,
	VividTargetingIndicator_CharacterOption = 0x8000,
	DisableMostWeatherEffects_CharacterOption = 0x10000,
	IgnoreTradeRequests_CharacterOption = 0x20000,
	FellowshipShareXP_CharacterOption = 0x40000,
	AcceptLootPermits_CharacterOption = 0x80000,
	FellowshipShareLoot_CharacterOption = 0x100000,
	SideBySideVitals_CharacterOption = 0x200000,
	CoordinatesOnRadar_CharacterOption = 0x400000,
	SpellDuration_CharacterOption = 0x800000,
	DisableHouseRestrictionEffects_CharacterOption = 0x2000000,
	DragItemOnPlayerOpensSecureTrade_CharacterOption = 0x4000000,
	DisplayAllegianceLogonNotifications_CharacterOption = 0x8000000,
	UseChargeAttack_CharacterOption = 0x10000000,
	AutoAcceptFellowRequest_CharacterOption = 0x20000000,
	HearAllegianceChat_CharacterOption = 0x40000000,
	UseCraftSuccessDialog_CharacterOption = 0x80000000,
	Default_CharacterOption = 0x50C4A54A,
	FORCE_CharacterOption_32_BIT = 0x7FFFFFFF,
};

enum CharacterOptions2
{
	Undef_CharacterOptions2 = 0x0,
	PersistentAtDay_CharacterOptions2 = 0x1,
	DisplayDateOfBirth_CharacterOptions2 = 0x2,
	DisplayChessRank_CharacterOptions2 = 0x4,
	DisplayFishingSkill_CharacterOptions2 = 0x8,
	DisplayNumberDeaths_CharacterOptions2 = 0x10,
	DisplayAge_CharacterOptions2 = 0x20,
	TimeStamp_CharacterOptions2 = 0x40,
	SalvageMultiple_CharacterOptions2 = 0x80,
	HearGeneralChat_CharacterOptions2 = 0x100,
	HearTradeChat_CharacterOptions2 = 0x200,
	HearLFGChat_CharacterOptions2 = 0x400,
	HearRoleplayChat_CharacterOptions2 = 0x800,
	AppearOffline_CharacterOptions2 = 0x1000,
	DisplayNumberCharacterTitles_CharacterOptions2 = 0x2000,
	MainPackPreferred_CharacterOptions2 = 0x4000,
	LeadMissileTargets_CharacterOptions2 = 0x8000,
	UseFastMissiles_CharacterOptions2 = 0x10000,
	FilterLanguage_CharacterOptions2 = 0x20000,
	ConfirmVolatileRareUse_CharacterOptions2 = 0x40000,
	HearSocietyChat_CharacterOptions2 = 0x80000,
	ShowHelm_CharacterOptions2 = 0x100000,
	DisableDistanceFog_CharacterOptions2 = 0x200000,
	UseMouseTurning_CharacterOptions2 = 0x400000,
	ShowCloak_CharacterOptions2 = 0x800000,
	LockUI_CharacterOptions2 = 0x1000000,
	Default_CharacterOptions2 = 0x948700,
	FORCE_CharacterOptions2_32_BIT = 0x7FFFFFFF,
};

enum ChannelID
{
	Undef_ChannelID = 0,
	Abuse_ChannelID = 0x1,
	Admin_ChannelID = 0x2,
	Audit_ChannelID = 0x4,
	Advocate1_ChannelID = 0x8,
	Advocate2_ChannelID = 0x10,
	Advocate3_ChannelID = 0x20,
	QA1_ChannelID = 0x40,
	QA2_ChannelID = 0x80,
	Debug_ChannelID = 0x100,
	Sentinel_ChannelID = 0x200,
	Help_ChannelID = 0x400,
	AllBroadcast_ChannelID = 0x401,
	ValidChans_ChannelID = 0x73F,
	Fellow_ChannelID = 0x800,
	Vassals_ChannelID = 0x1000,
	Patron_ChannelID = 0x2000,
	Monarch_ChannelID = 0x4000,
	AlArqas_ChannelID = 0x8000,
	Holtburg_ChannelID = 0x10000,
	Lytelthorpe_ChannelID = 0x20000,
	Nanto_ChannelID = 0x40000,
	Rithwic_ChannelID = 0x80000,
	Samsur_ChannelID = 1048576,
	Shoushi_ChannelID = 2097152,
	Yanshi_ChannelID = 4194304,
	Yaraq_ChannelID = 8388608,
	Covassals_ChannelID = 16777216,
	TownChans_ChannelID = 16744448,
	AllegianceBroadcast_ChannelID = 33554432,
	FellowBroadcast_channelID = 67108864,
	GhostChans_ChannelID = 117471232,
	AllChans_ChannelID = 117473087
};

enum AllegianceVersion
{
	Undef_AllegianceVersion = 0x0,
	SpokespersonAdded_AllegianceVersion = 0x1,
	PoolsAdded_AllegianceVersion = 0x2,
	MotdAdded_AllegianceVersion = 0x3,
	ChatRoomIDAdded_AllegianceVersion = 0x4,
	BannedCharactersAdded_AllegianceVersion = 0x5,
	MultipleAllegianceOfficersAdded_AllegianceVersion = 0x6,
	Bindstones_AllegianceVersion = 0x7,
	AllegianceName_AllegianceVersion = 0x8,
	OfficersTitlesAdded_AllegianceVersion = 0x9,
	LockedState_AllegianceVersion = 0xA,
	ApprovedVassal_AllegianceVersion = 0xB,
	Newest_AllegianceVersion = 0xB,
	FORCE_AllegianceVersion_32_BIT = 0x7FFFFFFF,
};

enum AllegianceIndex
{
	Undef_AllegianceIndex = 0x0,
	LoggedIn_AllegianceIndex = 0x1,
	Update_AllegianceIndex = 0x2,
	HasAllegianceAge_AllegianceIndex = 0x4,
	HasPackedLevel_AllegianceIndex = 0x8,
	MayPassupExperience_AllegianceIndex = 0x10,
	Forceuint32_t_AllegianceIndex = 0xFFFFFFFF,
};

enum eAllegianceOfficerLevel
{
	Undef_AllegianceOfficerLevel = 0,
	Speaker_AllegianceOfficerLevel = 1,
	Seneschal_AllegianceOfficerLevel = 2,
	Castellan_AllegianceOfficerLevel = 3,
	NumberOfOfficerTitles_AllegianceOfficerLevel = 3,
	Forceuint32_t_eAllegianceOfficerLevel = 0xFFFFFFFF,
};

enum ArmorEnchantment_BFIndex
{
	BF_ARMOR_LEVEL = 0x1,
	BF_ARMOR_MOD_VS_SLASH = 0x2,
	BF_ARMOR_MOD_VS_PIERCE = 0x4,
	BF_ARMOR_MOD_VS_BLUDGEON = 0x8,
	BF_ARMOR_MOD_VS_COLD = 0x10,
	BF_ARMOR_MOD_VS_FIRE = 0x20,
	BF_ARMOR_MOD_VS_ACID = 0x40,
	BF_ARMOR_MOD_VS_ELECTRIC = 0x80,
	BF_ARMOR_MOD_VS_NETHER = 0x100,
	BF_ARMOR_LEVEL_HI = 0x10000,
	BF_ARMOR_MOD_VS_SLASH_HI = 0x20000,
	BF_ARMOR_MOD_VS_PIERCE_HI = 0x40000,
	BF_ARMOR_MOD_VS_BLUDGEON_HI = 0x80000,
	BF_ARMOR_MOD_VS_COLD_HI = 0x100000,
	BF_ARMOR_MOD_VS_FIRE_HI = 0x200000,
	BF_ARMOR_MOD_VS_ACID_HI = 0x400000,
	BF_ARMOR_MOD_VS_ELECTRIC_HI = 0x800000,
	BF_ARMOR_MOD_VS_NETHER_HI = 0x1000000
};

enum WeaponEnchantment_BFIndex
{
	BF_WEAPON_OFFENSE = 0x1,
	BF_WEAPON_DEFENSE = 0x2,
	BF_WEAPON_TIME = 0x4,
	BF_DAMAGE = 0x8,
	BF_DAMAGE_VARIANCE = 0x10,
	BF_DAMAGE_MOD = 0x20,
	BF_WEAPON_OFFENSE_HI = 0x10000,
	BF_WEAPON_DEFENSE_HI = 0x20000,
	BF_WEAPON_TIME_HI = 0x40000,
	BF_DAMAGE_HI = 0x80000,
	BF_DAMAGE_VARIANCE_HI = 0x100000,
	BF_DAMAGE_MOD_HI = 0x200000
};

enum ResistanceEnchantment_BFIndex
{
	BF_RESIST_SLASH = 0x1,
	BF_RESIST_PIERCE = 0x2,
	BF_RESIST_BLUDGEON = 0x4,
	BF_RESIST_FIRE = 0x8,
	BF_RESIST_COLD = 0x10,
	BF_RESIST_ACID = 0x20,
	BF_RESIST_ELECTRIC = 0x40,
	BF_RESIST_HEALTH_BOOST = 0x80,
	BF_RESIST_STAMINA_DRAIN = 0x100,
	BF_RESIST_STAMINA_BOOST = 0x200,
	BF_RESIST_MANA_DRAIN = 0x400,
	BF_RESIST_MANA_BOOST = 0x800,
	BF_MANA_CON_MOD = 0x1000,
	BF_ELE_DAMAGE_MOD = 0x2000,
	BF_RESIST_NETHER = 0x4000,
	BF_RESIST_SLASH_HI = 0x10000,
	BF_RESIST_PIERCE_HI = 0x20000,
	BF_RESIST_BLUDGEON_HI = 0x40000,
	BF_RESIST_FIRE_HI = 0x80000,
	BF_RESIST_COLD_HI = 0x100000,
	BF_RESIST_ACID_HI = 0x200000,
	BF_RESIST_ELECTRIC_HI = 0x400000,
	BF_RESIST_HEALTH_BOOST_HI = 0x800000,
	BF_RESIST_STAMINA_DRAIN_HI = 0x1000000,
	BF_RESIST_STAMINA_BOOST_HI = 0x2000000,
	BF_RESIST_MANA_DRAIN_HI = 0x4000000,
	BF_RESIST_MANA_BOOST_HI = 0x8000000,
	BF_MANA_CON_MOD_HI = 0x10000000,
	BF_ELE_DAMAGE_MOD_HI = 0x20000000,
	BF_RESIST_NETHER_HI = 0x40000000
};

enum EnchantmentTypeEnum
{
	Undef_EnchantmentType = 0x0,
	Attribute_EnchantmentType = 0x1,
	SecondAtt_EnchantmentType = 0x2,
	Int_EnchantmentType = 0x4,
	Float_EnchantmentType = 0x8,
	Skill_EnchantmentType = 0x10,
	BodyDamageValue_EnchantmentType = 0x20,
	BodyDamageVariance_EnchantmentType = 0x40,
	BodyArmorValue_EnchantmentType = 0x80,
	SingleStat_EnchantmentType = 0x1000,
	MultipleStat_EnchantmentType = 0x2000,
	Multiplicative_EnchantmentType = 0x4000,
	Additive_EnchantmentType = 0x8000,
	AttackSkills_EnchantmentType = 0x10000,
	DefenseSkills_EnchantmentType = 0x20000,
	Multiplicative_Degrade_EnchantmentType = 0x100000,
	Additive_Degrade_EnchantmentType = 0x200000,
	Vitae_EnchantmentType = 0x800000,
	Cooldown_EnchantmentType = 0x1000000,
	Beneficial_EnchantmentType = 0x2000000,
	StatTypes_EnchantmentType = 0xFF,
	FORCE_EnchantmentTypeEnum_32_BIT = 0x7FFFFFFF,
};

enum EnchantmentVersion
{
	Undef_EnchantmentVersion = 0x0,
	SpellSetID_EnchantmentVersion = 0x1,
	Newest_EnchantmentVersion = 0x1,
	FORCE_EnchantmentVersion_32_BIT = 0x7FFFFFFF,
};

enum Placement
{
	Default = 0,
	RightHandCombat = 1,
	RightHandNonCombat = 2,
	LeftHand = 3,
	Belt = 4,
	Quiver = 5,
	Shield = 6,
	LeftWeapon = 7,
	LeftUnarmed = 8,
	SpecialCrowssbowBolt = 51,
	MissileFlight = 52,
	Resting = 101,
	Other = 102,
	Hook = 103,
	Random1 = 121,
	Random2 = 122,
	Random3 = 123,
	Random4 = 124,
	Random5 = 125,
	Random6 = 126,
	Random7 = 127,
	Random8 = 128,
	Random9 = 129,
	Random10 = 130,

	XXXUnknownA = 0x0000000A,
	XXXUnknownF = 0x0000000F,
	XXXUnknown14 = 0x00000014,
	XXXUnknown1E = 0x0000001E,
	XXXUnknown20 = 0x00000020,
	XXXUnknown3C = 0x0000003C,
	XXXUnknown69 = 0x00000069,
	XXXUnknown6A = 0x0000006A,
	XXXUnknown63 = 0x00000063,
	XXXUnknown68 = 0x00000068,
	XXXUnknown78 = 0x00000078,
	XXXUnknown84 = 0x00000084,
	XXXUnknownF0 = 0x000000F0,
	XXXUnknown3F2 = 0x000003F2,
};

enum CombatStyle
{
	Undef_CombatStyle = 0x0,
	Unarmed_CombatStyle = 0x1,
	OneHanded_CombatStyle = 0x2,
	OneHandedAndShield_CombatStyle = 0x4,
	TwoHanded_CombatStyle = 0x8,
	Bow_CombatStyle = 0x10,
	Crossbow_CombatStyle = 0x20,
	Sling_CombatStyle = 0x40,
	ThrownWeapon_CombatStyle = 0x80,
	DualWield_CombatStyle = 0x100,
	Magic_CombatStyle = 0x200,
	Atlatl_CombatStyle = 0x400,
	ThrownShield_CombatStyle = 0x800,
	Reserved1_CombatStyle = 0x1000,
	Reserved2_CombatStyle = 0x2000,
	Reserved3_CombatStyle = 0x4000,
	Reserved4_CombatStyle = 0x8000,
	StubbornMagic_CombatStyle = 0x10000,
	StubbornProjectile_CombatStyle = 0x20000,
	StubbornMelee_CombatStyle = 0x40000,
	StubbornMissile_CombatStyle = 0x80000,
	Melee_CombatStyles = 0x10F,
	Missile_CombatStyles = 0xCF0,
	Magic_CombatStyles = 0x200,
	All_CombatStyle = 0xFFFF,
	FORCE_CombatStyle_32_BIT = 0x7FFFFFFF,
};

enum RadarBlipEnum
{
	Undef_RadarBlipEnum = 0,
	Blue_RadarBlipEnum = 1,
	Gold_RadarBlipEnum = 2,
	White_RadarBlipEnum = 3,
	Purple_RadarBlipEnum = 4,
	Red_RadarBlipEnum = 5,
	Pink_RadarBlipEnum = 6,
	Green_RadarBlipEnum = 7,
	Yellow_RadarBlipEnum = 8,
	Cyan_RadarBlipEnum = 9,
	BrightGreen_RadarBlipEnum = 10,

	LifeStone_RadarBlipEnum = 1,
	Creature_RadarBlipEnum = 2,
	Default_RadarBlipEnum = 3,
	Portal_RadarBlipEnum = 4,
	PlayerKiller_RadarBlipEnum = 5,
	PKLite_RadarBlipEnum = 6,
	Advocate_RadarBlipEnum = 6,
	Vendor_RadarBlipEnum = 8,
	NPC_RadarBlipEnum = 8,
	Sentinel_RadarBlipEnum = 9,
	Admin_RadarBlipEnum = 9,
	Fellowship_RadarBlipEnum = 10,
	FellowshipLeader_RadarBlipEnum = 10,

	Count_RadarBlipEnum = 10
};

enum DestinationType
{
	Undef_DestinationType = 0x0,
	Contain_DestinationType = 0x1,
	Wield_DestinationType = 0x2,
	Shop_DestinationType = 0x4,
	Treasure_DestinationType = 0x8,
	HouseBuy_DestinationType = 0x10,
	HouseRent_DestinationType = 0x20,
	Checkpoint_DestinationType = 0x7,
	ContainTreasure_DestinationType = 0x9,
	WieldTreasure_DestinationType = 0xA,
	ShopTreasure_DestinationType = 0xC,
	FORCE_DestinationType_32_BIT = 0x7FFFFFFF,
};

enum PropertyNameEnum
{
	Invalid_PropertyName = 0x00000000,
	UICore_Menu_default_selected_item_PropertyName = 0x00000001,
	UICore_Menu_listbox_PropertyName = 0x00000002,
	UICore_Menu_open_center_PropertyName = 0x00000003,
	UICore_Menu_open_down_PropertyName = 0x00000004,
	UICore_Menu_open_up_PropertyName = 0x00000005,
	UICore_Menu_popup_PropertyName = 0x00000006,
	UICore_Menu_popup_layout_PropertyName = 0x00000007,
	UICore_Menu_selection_display_PropertyName = 0x00000008,
	UICore_Menu_text_item_PropertyName = 0x00000009,
	UICore_Menu_text_item_layout_PropertyName = 0x0000000A,
	UICore_Button_boolean_button_PropertyName = 0x0000000B,
	UICore_Button_broadcast_when_ghosted_PropertyName = 0x0000000C,
	UICore_Button_ghosted_PropertyName = 0x0000000D,
	UICore_Button_highlighted_PropertyName = 0x0000000E,
	UICore_Button_hot_button_PropertyName = 0x0000000F,
	UICore_Button_hot_click_first_interval_PropertyName = 0x00000010,
	UICore_Button_hot_click_interval_PropertyName = 0x00000011,
	UICore_Button_click_action_PropertyName = 0x00000012,
	UICore_Button_rollover_PropertyName = 0x00000013,
	UICore_Text_horizontal_justification_PropertyName = 0x00000014,
	UICore_Text_vertical_justification_PropertyName = 0x00000015,
	UICore_Text_editable_PropertyName = 0x00000016,
	UICore_Text_entry_PropertyName = 0x00000017,
	UICore_Text_font_PropertyName = 0x00000018,
	UICore_Text_font_color_PropertyName = 0x00000019,
	UICore_Text_fonts_PropertyName = 0x0000001A,
	UICore_Text_font_colors_PropertyName = 0x0000001B,
	UICore_Text_tag_fonts_PropertyName = 0x0000001C,
	UICore_Text_tag_font_colors_PropertyName = 0x0000001D,
	UICore_Text_max_chars_PropertyName = 0x0000001E,
	UICore_Text_no_IME_PropertyName = 0x0000001F,
	UICore_Text_one_line_PropertyName = 0x00000020,
	UICore_Text_outline_PropertyName = 0x00000021,
	UICore_Text_font_outline_color_PropertyName = 0x00000022,
	UICore_Text_left_margin_PropertyName = 0x00000023,
	UICore_Text_right_margin_PropertyName = 0x00000024,
	UICore_Text_top_margin_PropertyName = 0x00000025,
	UICore_Text_bottom_margin_PropertyName = 0x00000026,
	UICore_Text_selectable_PropertyName = 0x00000027,
	UICore_Text_trim_from_top_PropertyName = 0x00000028,
	UICore_Text_fit_to_text_PropertyName = 0x00000029,
	UICore_Resizebar_border_bottom_PropertyName = 0x0000002A,
	UICore_Resizebar_border_left_PropertyName = 0x0000002B,
	UICore_Resizebar_border_right_PropertyName = 0x0000002C,
	UICore_Resizebar_border_top_PropertyName = 0x0000002D,
	UICore_Panel_pages_PropertyName = 0x0000002E,
	UICore_Panel_page_data_PropertyName = 0x0000002F,
	UICore_Panel_tab_element_PropertyName = 0x00000030,
	UICore_Panel_page_element_PropertyName = 0x00000031,
	UICore_Panel_page_open_PropertyName = 0x00000032,
	UICore_Element_activatable_PropertyName = 0x00000033,
	UICore_Element_activate_on_show_PropertyName = 0x00000034,
	UICore_Element_focus_on_show_PropertyName = 0x00000035,
	UICore_Element_container_PropertyName = 0x00000036,
	UICore_Element_context_menu_PropertyName = 0x00000037,
	UICore_Element_disallow_drag_in_PropertyName = 0x00000038,
	UICore_Element_disallow_drag_out_PropertyName = 0x00000039,
	UICore_Element_dragable_PropertyName = 0x0000003A,
	UICore_Element_hide_PropertyName = 0x0000003B,
	UICore_Element_max_height_PropertyName = 0x0000003C,
	UICore_Element_max_width_PropertyName = 0x0000003D,
	UICore_Element_min_height_PropertyName = 0x0000003E,
	UICore_Element_min_width_PropertyName = 0x0000003F,
	UICore_Element_blocks_clicks_PropertyName = 0x00000040,
	UICore_Element_notify_on_resize_PropertyName = 0x00000041,
	UICore_Element_notify_on_move_PropertyName = 0x00000042,
	UICore_Element_notify_on_create_PropertyName = 0x00000043,
	UICore_Element_resize_line_PropertyName = 0x00000044,
	UICore_Element_save_loc_PropertyName = 0x00000045,
	UICore_Element_save_size_PropertyName = 0x00000046,
	UICore_Element_tooltip_ID_PropertyName = 0x00000047,
	UICore_Element_tooltip_layout_PropertyName = 0x00000048,
	UICore_Element_tooltip_entry_PropertyName = 0x00000049,
	UICore_Element_tooltip_text_ID_PropertyName = 0x0000004A,
	UICore_Element_tooltips_on_PropertyName = 0x0000004B,
	UICore_Element_viewport_PropertyName = 0x0000004C,
	UICore_Element_alphablendmod_PropertyName = 0x0000004D,
	UICore_Element_InputMapID_PropertyName = 0x0000004E,
	UICore_Element_HoverDelay_PropertyName = 0x00000050,
	UICore_Element_ShouldEraseBackground_PropertyName = 0x00000051,
	UICore_Element_ClampGameViewEdgeToObject_PropertyName = 0x00000052,
	UICore_Element_DrawAfterChildren_PropertyName = 0x00000053,
	UICore_Element_ImageTileOffset_X_PropertyName = 0x00000054,
	UICore_Element_ImageTileOffset_Y_PropertyName = 0x00000055,
	UICore_Element_ImageTileOffset_PropertyName = 0x00000056,
	UICore_Element_VisibilityToggle_InputAction_PropertyName = 0x00000057,
	UICore_Element_VisibilityToggle_ToggleType_PropertyName = 0x00000058,
	UICore_ListBox_click_select_PropertyName = 0x00000059,
	UICore_ListBox_drag_rollover_PropertyName = 0x0000005A,
	UICore_ListBox_drag_select_PropertyName = 0x0000005B,
	UICore_ListBox_horizontal_PropertyName = 0x0000005C,
	UICore_ListBox_item_normal_state_PropertyName = 0x0000005D,
	UICore_ListBox_item_selected_state_PropertyName = 0x0000005E,
	UICore_ListBox_max_columns_PropertyName = 0x0000005F,
	UICore_ListBox_max_rows_PropertyName = 0x00000060,
	UICore_ListBox_selected_item_state_change_PropertyName = 0x00000061,
	UICore_ListBox_entry_template_element_PropertyName = 0x00000062,
	UICore_ListBox_entry_template_layout_PropertyName = 0x00000063,
	UICore_ListBox_entry_templates_PropertyName = 0x00000064,
	UICore_ListBox_entry_template_PropertyName = 0x00000065,
	UICore_Meter_goal_position_PropertyName = 0x00000066,
	UICore_Meter_frame_meter_PropertyName = 0x00000067,
	UICore_Meter_move_fill_PropertyName = 0x00000068,
	UICore_Meter_position_PropertyName = 0x00000069,
	UICore_Meter_smooth_movement_PropertyName = 0x0000006A,
	UICore_Meter_smooth_movement_duration_PropertyName = 0x0000006B,
	UICore_Meter_frame_PropertyName = 0x0000006C,
	UICore_Meter_frame_array_PropertyName = 0x0000006D,
	UICore_Meter_frame_array_draw_mode_PropertyName = 0x0000006E,
	UICore_Meter_child_direction_PropertyName = 0x0000006F,
	UICore_Field_drag_rollover_PropertyName = 0x00000070,
	UICore_Scrollable_horizontal_scrollbar_PropertyName = 0x00000071,
	UICore_Scrollable_vertical_scrollbar_PropertyName = 0x00000072,
	UICore_Scrollable_keep_in_bounds_PropertyName = 0x00000073,
	UICore_Scrollable_preserve_percentage_on_resize_PropertyName = 0x00000074,
	UICore_Scrollbar_disallow_updating_PropertyName = 0x00000075,
	UICore_Scrollbar_disabled_PropertyName = 0x00000076,
	UICore_Scrollbar_increment_button_PropertyName = 0x00000077,
	UICore_Scrollbar_decrement_button_PropertyName = 0x00000078,
	UICore_Scrollbar_hide_when_disabled_PropertyName = 0x00000079,
	UICore_Scrollbar_ghost_when_disabled_PropertyName = 0x0000007A,
	UICore_Scrollbar_horizontal_PropertyName = 0x0000007B,
	UICore_Scrollbar_move_to_touched_position_PropertyName = 0x0000007C,
	UICore_Scrollbar_num_stop_locations_PropertyName = 0x0000007D,
	UICore_Scrollbar_stops_dont_include_endpoints_PropertyName = 0x0000007E,
	UICore_Scrollbar_stop_locations_PropertyName = 0x0000007F,
	UICore_Scrollbar_page_first_interval_PropertyName = 0x00000080,
	UICore_Scrollbar_page_interval_PropertyName = 0x00000081,
	UICore_Scrollbar_proportional_PropertyName = 0x00000082,
	UICore_Scrollbar_smooth_movement_PropertyName = 0x00000083,
	UICore_Scrollbar_smooth_movement_duration_PropertyName = 0x00000084,
	UICore_Scrollbar_goal_position_PropertyName = 0x00000085,
	UICore_Scrollbar_position_PropertyName = 0x00000086,
	UICore_Scrollbar_current_stop_PropertyName = 0x00000087,
	UICore_Scrollbar_widget_size_PropertyName = 0x00000088,
	UICore_Scrollbar_min_widget_size_PropertyName = 0x00000089,
	UICore_ContextMenu_open_delay_PropertyName = 0x0000008A,
	UICore_ContextMenu_lvl_PropertyName = 0x0000008B,
	UICore_ContextMenu_index_PropertyName = 0x0000008C,
	UICore_Dialog_topmost_PropertyName = 0x0000008D,
	UICore_Dialog_type_PropertyName = 0x0000008E,
	UICore_Dialog_confirmation_yesbuttontext_PropertyName = 0x00000090,
	UICore_Dialog_confirmation_nobuttontext_PropertyName = 0x00000091,
	UICore_Dialog_confirmation_response_PropertyName = 0x00000092,
	UICore_Dialog_message_okbuttontext_PropertyName = 0x00000095,
	UICore_Dialog_textinput_donebuttontext_PropertyName = 0x00000097,
	UICore_Dialog_textinput_response_PropertyName = 0x00000098,
	UICore_Dialog_confirmationtextinput_donebuttontext_PropertyName = 0x0000009A,
	UICore_Dialog_confirmationtextinput_cancelbuttontext_PropertyName = 0x0000009B,
	UICore_Dialog_confirmationtextinput_response_PropertyName = 0x0000009C,
	UICore_Dialog_wait_key_PropertyName = 0x0000009E,
	UICore_Dialog_menu_item_array_PropertyName = 0x000000A0,
	UICore_Dialog_menu_item_text_PropertyName = 0x000000A1,
	UICore_Dialog_menu_okbuttontext_PropertyName = 0x000000A2,
	UICore_Dialog_menu_choice_PropertyName = 0x000000A4,
	UICore_Dialog_confirmationmenu_item_array_PropertyName = 0x000000A6,
	UICore_Dialog_confirmationmenu_item_text_PropertyName = 0x000000A7,
	UICore_Dialog_confirmationmenu_okbuttontext_PropertyName = 0x000000A8,
	UICore_Dialog_confirmationmenu_cancelbuttontext_PropertyName = 0x000000A9,
	UICore_Dialog_confirmationmenu_choice_PropertyName = 0x000000AB,
	UICore_Dialog_modal_PropertyName = 0x000000AC,
	UICore_Browser_url_PropertyName = 0x000000AD,
	UICore_ColorPicker_show_selection_PropertyName = 0x000000AE,
	UICore_ColorPicker_current_selection_PropertyName = 0x000000AF,
	UICore_GroupBox_default_button_PropertyName = 0x000000B0,
	UICore_GroupBox_selected_button_PropertyName = 0x000000B1,
	UICore_GroupBox_allow_button_click_on_selected_button_PropertyName = 0x000000C1,
	UICore_Dialog_queue_PropertyName = 0x000000C3,
	UICore_Dialog_text_PropertyName = 0x000000C5,
	UICore_Dialog_duration_PropertyName = 0x000000C6,
	UICore_Text_truncate_text_to_fit_PropertyName = 0x000000C7,
	UICore_Text_lose_focus_on_escape_PropertyName = 0x000000CB,
	UICore_Text_lose_focus_on_acceptinput_PropertyName = 0x000000CC,
	UICore_Element_ObjectMode_PropertyName = 0x000000CD,
	UICore_Text_auto_tooltip_truncated_text_PropertyName = 0x000000D0,
	UICore_Text_select_all_on_gainfocus_mousedown_PropertyName = 0x000000D1,
	GameplayOptionList_PropertyName = 0x000000D2,
	GameplayOption_PropertyName = 0x000000D3,
	Option_Name_PropertyName = 0x000000D4,
	Option_Description_PropertyName = 0x000000D5,
	Option_PropertyName_PropertyName = 0x000000D6,
	Option_MaxFloatValue_PropertyName = 0x000000D7,
	Option_MinFloatValue_PropertyName = 0x000000D8,
	Option_MaxIntValue_PropertyName = 0x000000D9,
	Option_MinIntValue_PropertyName = 0x000000DA,
	Option_NumStops_PropertyName = 0x000000DB,
	VisibilityLayer_PropertyName = 0x000000DC,
	WB_RenderMaterial_Grid_PropertyName = 0x000000DD,
	Tools_VisibilityArray_PropertyName = 0x000000DE,
	Tools_VisibilityStruct_PropertyName = 0x000000DF,
	Tools_Visibility_Properties_PropertyName = 0x000000E0,
	Tools_VisibilityInheritance_PropertyName = 0x000000E1,
	Tools_VisibilityState_PropertyName = 0x000000E2,
	SaveOnExit_PropertyName = 0x000000E3,
	WorldBuilderSounds_PropertyName = 0x000000E4,
	DefaultTextEditor_PropertyName = 0x000000E5,
	DefaultTexture_PropertyName = 0x000000E6,
	LandblockAutosaveTimer_PropertyName = 0x000000E7,
	LandblockAutosaveRCSCheckEnabled_PropertyName = 0x000000E8,
	ShowEmptyEditors_PropertyName = 0x000000E9,
	AlwaysStartInEntityWorkspace_PropertyName = 0x000000EA,
	RandomHeadingRotation_PropertyName = 0x000000EB,
	FlushRCP_PropertyName = 0x000000EC,
	CopyScreenshotToClipboard_PropertyName = 0x000000ED,
	ShareFilePropertyLastDirectory_PropertyName = 0x000000EE,
	ShowEncounters_PropertyName = 0x000000EF,
	ShowEntityLinks_PropertyName = 0x000000F0,
	ShowEntityHeading_PropertyName = 0x000000F1,
	ShowMayaStyleUI_PropertyName = 0x000000F2,
	ShowEntityOrigin_PropertyName = 0x000000F3,
	EntityPasteOffset_PropertyName = 0x000000F4,
	EntityMovementOffset_PropertyName = 0x000000F5,
	EntityNudgeOffset_PropertyName = 0x000000F6,
	MoveRelativeToXYZ_PropertyName = 0x000000F7,
	UndoStackSize_PropertyName = 0x000000F8,
	EntityCullingDistance_PropertyName = 0x000000F9,
	OverrideNudgeCommand_PropertyName = 0x000000FA,
	UsePhysicsPlacement_PropertyName = 0x000000FB,
	UseSnapPoints_PropertyName = 0x000000FC,
	BackgroundColor_PropertyName = 0x000000FD,
	AmbientLightColor_PropertyName = 0x000000FE,
	UseAmbientLight_PropertyName = 0x000000FF,
	ForceSunlight_PropertyName = 0x00000100,
	DefaultRegion_PropertyName = 0x00000101,
	LandblockDrawingRadius_PropertyName = 0x00000102,
	LandblockOutlineColor_PropertyName = 0x00000103,
	ShowLandblockOutline_PropertyName = 0x00000104,
	CellOutlineColor_PropertyName = 0x00000105,
	TeleportOffset_PropertyName = 0x00000106,
	UseLandblockCache_PropertyName = 0x00000107,
	ShowLandscapeInDungeon_PropertyName = 0x00000108,
	ShowEnvCells_PropertyName = 0x00000109,
	ContentOrGeometry_PropertyName = 0x0000010A,
	GridEnabled_PropertyName = 0x0000010B,
	GridSnap_PropertyName = 0x0000010C,
	GridAngleSnap_PropertyName = 0x0000010D,
	GridSnapAngle_PropertyName = 0x0000010E,
	GridRange_PropertyName = 0x0000010F,
	GridBlocks_PropertyName = 0x00000110,
	GridCells_PropertyName = 0x00000111,
	GridXYAxisColor_PropertyName = 0x00000112,
	GridMajorAxisColor_PropertyName = 0x00000113,
	GridMinorAxisColor_PropertyName = 0x00000114,
	CameraFlySpeed_PropertyName = 0x00000115,
	CameraMoveSpeed_PropertyName = 0x00000116,
	CameraRotateSpeed_PropertyName = 0x00000117,
	InvertMouseX_PropertyName = 0x00000118,
	InvertMouseY_PropertyName = 0x00000119,
	InvertStrafe_PropertyName = 0x0000011A,
	CursorPlacementMode_PropertyName = 0x0000011B,
	CursorColor_PropertyName = 0x0000011C,
	SelectionColor_PropertyName = 0x0000011D,
	SelectionBlinks_PropertyName = 0x0000011E,
	ShowPortalSelection_PropertyName = 0x0000011F,
	PortalSelectionColor_PropertyName = 0x00000120,
	PortalSelectionBlinks_PropertyName = 0x00000121,
	LockBlocksInPerforce_PropertyName = 0x00000122,
	ToggleTerrainRaycast_PropertyName = 0x00000123,
	ToggleCeilingRaycast_PropertyName = 0x00000124,
	VisualizeCeiling_PropertyName = 0x00000125,
	Physics_EtherealityType_PropertyName = 0x00000126,
	Physics_EtherealToType_PropertyName = 0x00000127,
	Physics_PlacementEtherealToType_PropertyName = 0x00000128,
	WB_Camera_Entity_PropertyName = 0x00000129,
	WB_LoadError_Entity_PropertyName = 0x0000012A,
	WB_LightSource_Entity_PropertyName = 0x0000012B,
	ShowSnapPoints_PropertyName = 0x0000012C,
	SpawnSecondEditor_PropertyName = 0x0000012D,
	Physics_AdjustableScale_PropertyName = 0x0000012E,
	LinkName_PropertyName = 0x0000012F,
	LinkColor_PropertyName = 0x00000130,
	ShowAppliedPropertyType_PropertyName = 0x00000131,
	ShowEnvCellsInWorld_PropertyName = 0x00000132,
	RefreshLandscape_PropertyName = 0x00000133,
	UI_Allegiance_VassalID_PropertyName = 0x10000001,
	UI_Credits_TextArea_PropertyName = 0x10000002,
	UI_Credits_StringTable_PropertyName = 0x10000003,
	UI_Credits_Duration_PropertyName = 0x10000004,
	UI_Credits_PictureArray_PropertyName = 0x10000005,
	UI_Credits_SoundArray_PropertyName = 0x10000006,
	UI_Credits_Sound_PropertyName = 0x10000007,
	UI_Credits_Picture_PropertyName = 0x10000008,
	UI_CharacterManagement_CharacterInstanceID_PropertyName = 0x10000009,
	UI_Chargen_SkillID_PropertyName = 0x1000000A,
	UI_Chat_TalkFocus_PropertyName = 0x1000000B,
	UI_Effects_EffectsUIType_PropertyName = 0x1000000C,
	UI_Fellowship_FellowID_PropertyName = 0x1000000D,
	UI_ItemList_ItemSlotID_PropertyName = 0x1000000E,
	UI_ItemList_ItemID_PropertyName = 0x1000000F,
	UI_ItemList_SpellID_PropertyName = 0x10000010,
	UI_ItemList_IsContainer_PropertyName = 0x10000011,
	UI_ItemList_IsShortcut_PropertyName = 0x10000012,
	UI_ItemList_IsVendor_PropertyName = 0x10000013,
	UI_ItemList_IsSalvage_PropertyName = 0x10000014,
	UI_ItemList_FixedListSize_PropertyName = 0x10000015,
	UI_ItemList_AllowDragging_PropertyName = 0x10000016,
	UI_ItemList_AtLeastOneEmptySlot_PropertyName = 0x10000017,
	UI_Keyboard_ActionMappingListBox_PropertyName = 0x10000018,
	UI_Keyboard_OKButton_PropertyName = 0x10000019,
	UI_Keyboard_CancelButton_PropertyName = 0x1000001A,
	UI_Keyboard_ResetToDefaultsButton_PropertyName = 0x1000001B,
	UI_Keyboard_RevertToSavedButton_PropertyName = 0x1000001C,
	UI_Keyboard_CurrentKeymapLabel_PropertyName = 0x1000001D,
	UI_Keyboard_LoadKeymapButton_PropertyName = 0x1000001E,
	UI_Keyboard_SaveKeymapAsButton_PropertyName = 0x1000001F,
	UI_Keyboard_KeymapFilename_PropertyName = 0x10000020,
	UI_MiniGame_PieceIconArray_PropertyName = 0x10000021,
	UI_MiniGame_PieceIcon_PropertyName = 0x10000022,
	UI_Options_Revert_Title_PropertyName = 0x10000023,
	UI_Options_Default_Title_PropertyName = 0x10000024,
	UI_Options_Menu_Value_PropertyName = 0x10000025,
	UI_Options_Menu_FontIndex_PropertyName = 0x10000026,
	UI_Options_Menu_ColorIndex_PropertyName = 0x10000027,
	UI_SpewBox_MaxMessagesToDisplay_PropertyName = 0x10000028,
	UICore_Element_PanelID_PropertyName = 0x10000029,
	UI_ActionKeyMap_ResetToDefaultsButton_PropertyName = 0x1000002A,
	UI_ActionKeyMap_BindButtons_PropertyName = 0x1000002B,
	UI_ActionKeyMap_BindButton_PropertyName = 0x1000002C,
	UI_Radar_Radius_PropertyName = 0x1000002D,
	UI_Radar_CenterPoint_PropertyName = 0x1000002E,
	UI_Radar_X_PropertyName = 0x1000002F,
	UI_Radar_Y_PropertyName = 0x10000030,
	UI_Radar_NorthToken_PropertyName = 0x10000031,
	UI_Radar_SouthToken_PropertyName = 0x10000032,
	UI_Radar_EastToken_PropertyName = 0x10000033,
	UI_Radar_WestToken_PropertyName = 0x10000034,
	UI_Radar_CoordinatesContainer_PropertyName = 0x10000035,
	UI_Radar_CombinedCoordsField_PropertyName = 0x10000036,
	UI_Radar_XCoordField_PropertyName = 0x10000037,
	UI_Radar_YCoordField_PropertyName = 0x10000038,
	UI_Vendor_ShopFilters_PropertyName = 0x10000039,
	UI_InfoRegion_Index_PropertyName = 0x1000003A,
	UI_InfoRegion_StatType_PropertyName = 0x1000003B,
	UI_InfoRegion_StatID_PropertyName = 0x1000003C,
	UI_Usage_ToolID_PropertyName = 0x1000003D,
	UI_Usage_TargetID_PropertyName = 0x1000003E,
	UI_Spellbook_DeletedSpellID_PropertyName = 0x1000003F,
	UI_StatManagement_SkillToTrainID_PropertyName = 0x10000040,
	UI_StatManagement_CostToTrain_PropertyName = 0x10000041,
	UI_ItemList_ShortcutOverlayArray_PropertyName = 0x10000042,
	UI_ItemList_ShortcutOverlayArray_Ghosted_PropertyName = 0x10000043,
	UI_ItemList_ShortcutOverlay_PropertyName = 0x10000044,
	UI_Intro_StateArray_PropertyName = 0x10000047,
	UI_Intro_State_PropertyName = 0x10000048,
	UI_Panel_RestoreLastOpenPanelWhenClosed_PropertyName = 0x10000049,
	UI_SpellComponent_ComponentDataID_PropertyName = 0x1000004C,
	UI_SpellComponentHeader_ComponentCategory_PropertyName = 0x1000004D,
	UI_Map_MapX0_PropertyName = 0x1000004E,
	UI_Map_MapX1_PropertyName = 0x1000004F,
	UI_Map_MapY0_PropertyName = 0x10000050,
	UI_Map_MapY1_PropertyName = 0x10000051,
	UI_ItemList_SingleSelection_PropertyName = 0x10000052,
	UI_ItemList_DragScroll_Horizontal_PropertyName = 0x10000053,
	UI_ItemList_DragScroll_Vertical_PropertyName = 0x10000054,
	UI_ItemList_DragScroll_JumpDistance_PropertyName = 0x10000057,
	UI_ItemList_DragScroll_MarginWidth_PropertyName = 0x10000059,
	UI_ItemList_DragScroll_MarginHeight_PropertyName = 0x1000005A,
	UI_ItemList_DragScroll_Delay_PropertyName = 0x1000005B,
	UI_ItemList_DragScroll_ItemScrolling_PropertyName = 0x1000005C,
	UI_ItemList_DragScroll_SpellScrolling_PropertyName = 0x1000005D,
	UI_ItemList_ShortcutOverlayArray_Empty_PropertyName = 0x1000005E,
	UI_Admin_QualityType_PropertyName = 0x10000064,
	UI_Admin_QualityEnum_PropertyName = 0x10000065,
	UI_Admin_NewEntry_PropertyName = 0x10000067,
	UI_Admin_ListBoxEntryType_PropertyName = 0x10000068,
	UI_Chat_WindowID_PropertyName = 0x1000007E,
	Option_TextType_PropertyName = 0x1000007F,
	Option_DefaultOpacity_PropertyName = 0x10000080,
	Option_ActiveOpacity_PropertyName = 0x10000081,
	UIOption_CheckboxBitfield_FullyCheckedImage_PropertyName = 0x10000082,
	UIOption_CheckboxBitfield_PartlyCheckedImage_PropertyName = 0x10000083,
	UIOption_CheckboxBitfield_ChildIndex_PropertyName = 0x10000084,
	UI_Social_FriendID_PropertyName = 0x10000085,
	Option_Placement_X_PropertyName = 0x10000086,
	Option_Placement_Y_PropertyName = 0x10000087,
	Option_Placement_Width_PropertyName = 0x10000088,
	Option_Placement_Height_PropertyName = 0x10000089,
	Option_Placement_Visibility_PropertyName = 0x1000008A,
	Option_Placement_PropertyName = 0x1000008B,
	Option_PlacementArray_PropertyName = 0x1000008C,
	Option_Placement_Title_PropertyName = 0x1000008D,
	UI_Social_CharacterTitleID_PropertyName = 0x1000008E,
	UI_Social_SquelchID_PropertyName = 0x1000008F
};

/*
linked lever or door activated generators use the ACTIVATION_RESPONSE quality with an empty target
ACTIVATION_TARGET is 0
ACTIVATION_RESPONSE Generate
that says in response to th eactivation, activate my generator
and if its linkable, of course it will have a generator profile slot with the placeholder
im not sure what all the activation_responses are but Use = 0x2, Animate = 0x4, Talk = 0x10, ??? = 0x800, CastSpell = 0x1000, Generate 0x10000
0x800 is on some weird puzzle levers that dont actually activate anything until you have them all set correctly
*/

enum HouseType
{
	Undef_HouseType = 0,
	Cottage_HouseType = 1,
	Villa_HouseType = 2,
	Mansion_HouseType = 3,
	Apartment_HouseType = 4
};

enum ActivationResponseEnum
{
	Undef_ActivationResponse = 0,
	Use_ActivationResponse = 2,
	Animate_ActivationResponse = 4,
	Talk_ActivationResponse = 0x10,
	Unk800_ActivationResponse = 0x800,
	CastSpell_ActivationResponse = 0x1000,
	Generate_ActivationResponse = 0x10000
};

enum HouseBitmask
{
	Undef_HouseBitmask = 0x0,
	Active_HouseBitmask = 0x1,
	RequiresMonarch_HouseBitmask = 0x2,
	Force32Bit_HouseBitmask = 0x7FFFFFFF,
};

enum HouseOp
{
	Undef_HouseOp = 0x0,
	Buy_House = 0x1,
	Rent_House = 0x2,
	Force32Bit_House = 0x7FFFFFFF,
};

enum HousePanelTextColor
{
	Normal_HousePanelTextColor = 0x0,
	RentPaid_HousePanelTextColor = 0x1,
	RentNotPaid_HousePanelTextColor = 0x2,
};

enum eAllegianceHouseAction
{
	Undef_AllegianceHouseAction = 0x0,
	CheckStatus_AllegianceHouseAction = 0x1,
	GuestOpen_AllegianceHouseAction = 0x2,
	GuestClose_AllegianceHouseAction = 0x3,
	StorageOpen_AllegianceHouseAction = 0x4,
	StorageClose_AllegianceHouseAction = 0x5,
	NumberOfActions_AllegianceHouseAction = 0x5,
	FORCE_AllegianceHouseAction_32_BIT = 0x7FFFFFFF,
};

enum RDBBitmask
{
	Undef_RDBBitmask = 0x0,
	OpenHouse_RDBBitmask = 0x1,
	Force32Bit_RDBBitmask = 0x7FFFFFFF,
};

enum ImbuedEffectType
{
	Undef_ImbuedEffectType = 0x0,
	CriticalStrike_ImbuedEffectType = 0x1,
	CripplingBlow_ImbuedEffectType = 0x2,
	ArmorRending_ImbuedEffectType = 0x4,
	SlashRending_ImbuedEffectType = 0x8,
	PierceRending_ImbuedEffectType = 0x10,
	BludgeonRending_ImbuedEffectType = 0x20,
	AcidRending_ImbuedEffectType = 0x40,
	ColdRending_ImbuedEffectType = 0x80,
	ElectricRending_ImbuedEffectType = 0x100,
	FireRending_ImbuedEffectType = 0x200,
	MeleeDefense_ImbuedEffectType = 0x400,
	MissileDefense_ImbuedEffectType = 0x800,
	MagicDefense_ImbuedEffectType = 0x1000,
	Spellbook_ImbuedEffectType = 0x2000,
	NetherRending_ImbuedEffectType = 0x4000,
	IgnoreSomeMagicProjectileDamage_ImbuedEffectType = 0x20000000,
	AlwaysCritical_ImbuedEffectType = 0x40000000,
	IgnoreAllArmor_ImbuedEffectType = 0x80000000,
	FORCE_ImbuedEffectType_32_BIT = 0x7FFFFFFF,
};

enum SchoolOfMagic
{
	Undef_Magic,
	War_Magic,
	Life_Magic,
	ItemEnchantment_Magic,
	CreatureEnchantment_Magic,
	Void_Magic 
};

enum GameEventState
{
	Undef_GameEventState = 0x0,
	Enabled_GameEventState = 0x1,
	Disabled_GameEventState = 0x2,
	Off_GameEventState = 0x3,
	On_GameEventState = 0x4,
	FORCE_GameEventState_32_BIT = 0x7FFFFFFF,
};

enum LightClass
{
	SUNLIGHT = 0,
	STATIC,
	DYNAMIC,
	NO_LIGHT = -1
};

enum LightingType
{
	DYNAMIC_LIGHTING = 0,
	STATIC_LIGHTING,
	FULL_LIGHTING
};

enum LightingMode
{
	LM_RESTORE_LIGHTING,
	LM_LOW_LIGHTING,
	LM_HIGH_LIGHTING
};

enum LightState
{
	STATIC_LS = 1
};

enum RMFieldType
{
	RMFIELD_LAYER_DIFFUSE = 0x0,
	RMFIELD_LAYER_SPECULAR = 0x1,
	RMFIELD_LAYER_SPECULARPOWER = 0x2,
	RMFIELD_LAYER_DYE = 0x3,
	RMFIELD_LAYER_CULLMODE = 0x4,
	RMFIELD_LAYER_DEPTHTEST = 0x5,
	RMFIELD_LAYER_DEPTHWRITE = 0x6,
	RMFIELD_LAYER_ALPHATEST = 0x7,
	RMFIELD_LAYER_ALPHATESTREF = 0x8,
	RMFIELD_LAYER_STAGE_TEXTURE = 0x9,
	RMFIELD_LAYER_STAGE_ADDRESSMODEU = 0xA,
	RMFIELD_LAYER_STAGE_ADDRESSMODEV = 0xB,
	RMFIELD_LAYER_MOD_ORIGIN_XTRANSLATE = 0xC,
	RMFIELD_LAYER_MOD_ORIGIN_YTRANSLATE = 0xD,
	RMFIELD_LAYER_MOD_ORIGIN_ZTRANSLATE = 0xE,
	RMFIELD_LAYER_MOD_ORIGIN_XSCALE = 0xF,
	RMFIELD_LAYER_MOD_ORIGIN_YSCALE = 0x10,
	RMFIELD_LAYER_MOD_ORIGIN_ZSCALE = 0x11,
	RMFIELD_LAYER_MOD_ORIGIN_ORIGINPHASE = 0x12,
	RMFIELD_LAYER_MOD_ORIGIN_NORMALPHASE = 0x13,
	RMFIELD_LAYER_MOD_NORMAL_XTRANSLATE = 0x14,
	RMFIELD_LAYER_MOD_NORMAL_YTRANSLATE = 0x15,
	RMFIELD_LAYER_MOD_NORMAL_ZTRANSLATE = 0x16,
	RMFIELD_LAYER_MOD_NORMAL_XSCALE = 0x17,
	RMFIELD_LAYER_MOD_NORMAL_YSCALE = 0x18,
	RMFIELD_LAYER_MOD_NORMAL_ZSCALE = 0x19,
	RMFIELD_LAYER_MOD_NORMAL_ORIGINPHASE = 0x1A,
	RMFIELD_LAYER_MOD_NORMAL_NORMALPHASE = 0x1B,
	RMFIELD_LAYER_MOD_DIFFUSE_R = 0x1C,
	RMFIELD_LAYER_MOD_DIFFUSE_G = 0x1D,
	RMFIELD_LAYER_MOD_DIFFUSE_B = 0x1E,
	RMFIELD_LAYER_MOD_DIFFUSE_A = 0x1F,
	RMFIELD_LAYER_MOD_UVTRANSLATE_UTRANSLATE = 0x20,
	RMFIELD_LAYER_MOD_UVTRANSLATE_VTRANSLATE = 0x21,
	RMFIELD_LAYER_MOD_UVROTATE_ROTATE = 0x22,
	RMFIELD_LAYER_MOD_UVSCALE_USCALE = 0x23,
	RMFIELD_LAYER_MOD_UVSCALE_VSCALE = 0x24,
};

enum RMDataType
{
	RMDATA_WAVEFORM = 0x3E8,
	RMDATA_COLOR = 0x7D0,
	RMDATA_TEXTURE = 0xBB8,
	RMDATA_BOOL = 0xFA0,
	RMDATA_TEXTURE_PTR = 0x2710,
	RMDATA_INVALID = 0x7FFFFFFF,
};

enum PixelFormatID
{
	PFID_UNKNOWN = 0x0,
	PFID_R8G8B8 = 0x14,
	PFID_A8R8G8B8 = 0x15,
	PFID_X8R8G8B8 = 0x16,
	PFID_R5G6B5 = 0x17,
	PFID_X1R5G5B5 = 0x18,
	PFID_A1R5G5B5 = 0x19,
	PFID_A4R4G4B4 = 0x1A,
	PFID_R3G3B2 = 0x1B,
	PFID_A8 = 0x1C,
	PFID_A8R3G3B2 = 0x1D,
	PFID_X4R4G4B4 = 0x1E,
	PFID_A2B10G10R10 = 0x1F,
	PFID_A8B8G8R8 = 0x20,
	PFID_X8B8G8R8 = 0x21,
	PFID_A2R10G10B10 = 0x23,
	PFID_A8P8 = 0x28,
	PFID_P8 = 0x29,
	PFID_L8 = 0x32,
	PFID_A8L8 = 0x33,
	PFID_A4L4 = 0x34,
	PFID_V8U8 = 0x3C,
	PFID_L6V5U5 = 0x3D,
	PFID_X8L8V8U8 = 0x3E,
	PFID_Q8W8V8U8 = 0x3F,
	PFID_V16U16 = 0x40,
	PFID_A2W10V10U10 = 0x43,
	PFID_UYVY = 0x59565955,
	PFID_R8G8_B8G8 = 0x47424752,
	PFID_YUY2 = 0x32595559,
	PFID_G8R8_G8B8 = 0x42475247,
	PFID_DXT1 = 0x31545844,
	PFID_DXT2 = 0x32545844,
	PFID_DXT3 = 0x33545844,
	PFID_DXT4 = 0x34545844,
	PFID_DXT5 = 0x35545844,
	PFID_D16_LOCKABLE = 0x46,
	PFID_D32 = 0x47,
	PFID_D15S1 = 0x49,
	PFID_D24S8 = 0x4B,
	PFID_D24X8 = 0x4D,
	PFID_D24X4S4 = 0x4F,
	PFID_D16 = 0x50,
	PFID_VERTEXDATA = 0x64,
	PFID_INDEX16 = 0x65,
	PFID_INDEX32 = 0x66,
	PFID_CUSTOM_R8G8B8A8 = 0xF0,
	PFID_CUSTOM_A8B8G8R8 = 0xF1,
	PFID_CUSTOM_B8G8R8 = 0xF2,
	PFID_CUSTOM_LSCAPE_R8G8B8 = 0xF3,
	PFID_CUSTOM_LSCAPE_ALPHA = 0xF4,
	PFID_CUSTOM_RAW_JPEG = 0x1F4,
	PFID_CUSTOM_FIRST = 0xF0,
	PFID_CUSTOM_LAST = 0x1F4,
	PFID_INVALID = 0x7FFFFFFF,
};

enum TextureType
{
	TEXTURETYPE_UNDEFINED = 0x1,
	TEXTURETYPE_2D = 0x2,
	TEXTURETYPE_3D = 0x3,
	TEXTURETYPE_CUBE = 0x4,
	TEXTURETYPE_MOVIE2D = 0x5,
};

enum RenderPassType
{
	RenderPass_Default = 0x0,
	RenderPass_DirectionalLightDiffuseAndSpecular = 0x1,
	RenderPass_PointLightDiffuseAndSpecular = 0x2,
	RenderPass_PointLightDiffuseAndSpecularProjector = 0x3,
	RenderPass_AmbientLight_DirectionalLightDiffuseAndSpecular = 0x4,
	RenderPass_AmbientLight_PointLightDiffuseAndSpecular = 0x5,
	RenderPass_DistanceFog = 0x6,
	RenderPass_FixedFunctionGlow = 0x7,
	RenderPass_ShaderGlow = 0x8,
	RenderPass_LandscapeShadowMap = 0x9,
	RenderPass_AlphaBlend = 0xA,
	RenderPass_AL_0DL_0PL = 0xB,
	RenderPass_AL_0DL_1PL = 0xC,
	RenderPass_AL_0DL_2PL = 0xD,
	RenderPass_AL_0DL_3PL = 0xE,
	RenderPass_AL_0DL_4PL = 0xF,
	RenderPass_AL_0DL_5PL = 0x10,
	RenderPass_AL_0DL_6PL = 0x11,
	RenderPass_AL_0DL_7PL = 0x12,
	RenderPass_AL_0DL_8PL = 0x13,
	RenderPass_AL_1DL_0PL = 0x14,
	RenderPass_AL_1DL_1PL = 0x15,
	RenderPass_AL_1DL_2PL = 0x16,
	RenderPass_AL_1DL_3PL = 0x17,
	RenderPass_AL_1DL_4PL = 0x18,
	RenderPass_AL_1DL_5PL = 0x19,
	RenderPass_AL_1DL_6PL = 0x1A,
	RenderPass_AL_1DL_7PL = 0x1B,
	RenderPass_AL_0DL_0PL_Fog = 0x1C,
	RenderPass_AL_0DL_1PL_Fog = 0x1D,
	RenderPass_AL_0DL_2PL_Fog = 0x1E,
	RenderPass_AL_0DL_3PL_Fog = 0x1F,
	RenderPass_AL_0DL_4PL_Fog = 0x20,
	RenderPass_AL_0DL_5PL_Fog = 0x21,
	RenderPass_AL_0DL_6PL_Fog = 0x22,
	RenderPass_AL_0DL_7PL_Fog = 0x23,
	RenderPass_AL_0DL_8PL_Fog = 0x24,
	RenderPass_AL_1DL_0PL_Fog = 0x25,
	RenderPass_AL_1DL_1PL_Fog = 0x26,
	RenderPass_AL_1DL_2PL_Fog = 0x27,
	RenderPass_AL_1DL_3PL_Fog = 0x28,
	RenderPass_AL_1DL_4PL_Fog = 0x29,
	RenderPass_AL_1DL_5PL_Fog = 0x2A,
	RenderPass_AL_1DL_6PL_Fog = 0x2B,
	RenderPass_AL_1DL_7PL_Fog = 0x2C,
	RenderPass_MaxPasses = 0x2D,
	RenderPass_Invalid = 0x2E,
};

enum ShaderVersionType
{
	ShaderVersion_1_1 = 0x0,
	ShaderVersion_2_0 = 0x1,
	ShaderVersion_MaxVersions = 0x2,
	ShaderVersion_Invalid = 0x3,
};

enum TexAddress
{
	TEXADDRESS_WRAP = 0x1,
	TEXADDRESS_MIRROR = 0x2,
	TEXADDRESS_CLAMP = 0x3,
	TEXADDRESS_BORDER = 0x4,
	TEXADDRESS_MIRRORONCE = 0x5,
	TEXADDRESS_INVALID = 0x7FFFFFFF,
};

enum TexFilterMode
{
	TEXFILTER_NONE = 0x0,
	TEXFILTER_POINT = 0x1,
	TEXFILTER_LINEAR = 0x2,
	TEXFILTER_ANISOTROPIC = 0x3,
	TEXFILTER_PYRAMIDALQUAD = 0x6,
	TEXFILTER_GAUSSIANQUAD = 0x7,
	TEXFILTER_INVALID = 0x7FFFFFFF,
};

enum TextureOp
{
	TEXOP_DISABLE = 0x1,
	TEXOP_SELECTARG1 = 0x2,
	TEXOP_SELECTARG2 = 0x3,
	TEXOP_MODULATE = 0x4,
	TEXOP_MODULATE2X = 0x5,
	TEXOP_MODULATE4X = 0x6,
	TEXOP_ADD = 0x7,
	TEXOP_ADDSIGNED = 0x8,
	TEXOP_ADDSIGNED2X = 0x9,
	TEXOP_SUBTRACT = 0xA,
	TEXOP_ADDSMOOTH = 0xB,
	TEXOP_BLENDDIFFUSEALPHA = 0xC,
	TEXOP_BLENDTEXTUREALPHA = 0xD,
	TEXOP_BLENDFACTORALPHA = 0xE,
	TEXOP_BLENDTEXTUREALPHAPM = 0xF,
	TEXOP_BLENDCURRENTALPHA = 0x10,
	TEXOP_PREMODULATE = 0x11,
	TEXOP_MODULATEALPHA_ADDCOLOR = 0x12,
	TEXOP_MODULATECOLOR_ADDALPHA = 0x13,
	TEXOP_MODULATEINVALPHA_ADDCOLOR = 0x14,
	TEXOP_MODULATEINVCOLOR_ADDALPHA = 0x15,
	TEXOP_BUMPENVMAP = 0x16,
	TEXOP_BUMPENVMAPLUMINANCE = 0x17,
	TEXOP_DOTPRODUCT3 = 0x18,
	TEXOP_MULTIPLYADD = 0x19,
	TEXOP_LERP = 0x1A,
	TEXOP_INVALID = 0x7FFFFFFF,
};

enum BlendMode
{
	BLEND_ZERO = 0x1,
	BLEND_ONE = 0x2,
	BLEND_SRCCOLOR = 0x3,
	BLEND_INVSRCCOLOR = 0x4,
	BLEND_SRCALPHA = 0x5,
	BLEND_INVSRCALPHA = 0x6,
	BLEND_DSTALPHA = 0x7,
	BLEND_INVDSTALPHA = 0x8,
	BLEND_DSTCOLOR = 0x9,
	BLEND_INVDSTCOLOR = 0xA,
	BLEND_SRCALPHASAT = 0xB,
	BLEND_BOTHSRCALPHA = 0xC,
	BLEND_BOTHINVSRCALPHA = 0xD,
	BLEND_BLENDFACTOR = 0xE,
	BLEND_INVBLENDFACTOR = 0xF,
	BLEND_INVALID = 0x7FFFFFFF,
};

enum CullModeType
{
	CULLMODE_NONE = 0x1,
	CULLMODE_CW = 0x2,
	CULLMODE_CCW = 0x3,
	CULLMODE_INVALID = 0x7FFFFFFF,
};

enum BlendOpType
{
	BLENDOP_ADD = 0x1,
	BLENDOP_SUBTRACT = 0x2,
	BLENDOP_REVSUBTRACT = 0x3,
	BLENDOP_MIN = 0x4,
	BLENDOP_MAX = 0x5,
	BLENDOP_INVALID = 0x7FFFFFFF,
};

enum DepthTestMode
{
	DEPTHTEST_NEVER = 0x1,
	DEPTHTEST_LESS = 0x2,
	DEPTHTEST_EQUAL = 0x3,
	DEPTHTEST_LESSEQUAL = 0x4,
	DEPTHTEST_GREATER = 0x5,
	DEPTHTEST_NOTEQUAL = 0x6,
	DEPTHTEST_GREATEREQUAL = 0x7,
	DEPTHTEST_ALWAYS = 0x8,
	DEPTHTEST_INVALID = 0x7FFFFFFF,
};

enum WaveformType
{
	WAVEFORM_INVALID = 0x0,
	WAVEFORM_NONE = 0x1,
	WAVEFORM_SPEED = 0x2,
	WAVEFORM_NOISE = 0x3,
	WAVEFORM_SINE = 0x4,
	WAVEFORM_SQUARE = 0x5,
	WAVEFORM_BOUNCE = 0x6,
	WAVEFORM_PERLIN = 0x7,
	WAVEFORM_FRACTAL = 0x8,
	WAVEFORM_FRAMELOOP = 0x9,
	NUM_WAVEFORMS = 0xA,
};

enum PrimType
{
	PRIMTYPE_POINTLIST = 0x1,
	PRIMTYPE_LINELIST = 0x2,
	PRIMTYPE_LINESTRIP = 0x3,
	PRIMTYPE_TRIANGLELIST = 0x4,
	PRIMTYPE_TRIANGLESTRIP = 0x5,
	PRIMTYPE_TRIANGLEFAN = 0x6,
	PRIMTYPE_INVALID = 0x7FFFFFFF,
};

enum ColorSource
{
	FromMaterial = 0x0,
	FromVertex = 0x1,
};

enum FillModeType
{
	FILLMODE_POINT = 0x1,
	FILLMODE_WIREFRAME = 0x2,
	FILLMODE_SOLID = 0x3,
	FILLMODE_INVALID = 0x7FFFFFFF,
};

enum AlphaTestFunc
{
	ALPHATESTFUNC_NEVER = 0x1,
	ALPHATESTFUNC_LESS = 0x2,
	ALPHATESTFUNC_EQUAL = 0x3,
	ALPHATESTFUNC_LESSEQUAL = 0x4,
	ALPHATESTFUNC_GREATER = 0x5,
	ALPHATESTFUNC_NOTEQUAL = 0x6,
	ALPHATESTFUNC_GREATEREQUAL = 0x7,
	ALPHATESTFUNC_ALWAYS = 0x8,
	ALPHATESTFUNC_INVALID = 0x7FFFFFFF,
};

enum ClientEventsEnum
{
	CHANGE_PLAYER_OPTION = 0x0005, // Change player option
	MELEE_ATTACK = 0x0008, // Melee Attack
	MISSILE_ATTACK = 0x000A, // Missile Attack
	SET_AFK_MODE = 0x000F, // Set AFK Mode
	SET_AFK_MESSAGE = 0x0010, //Set AFK Message
	TEXT_CLIENT = 0x0015, // Client Text
	REMOVE_FRIEND = 0x0017,
	ADD_FRIEND = 0x0018,
	STORE_ITEM = 0x0019, // Store Item
	EQUIP_ITEM = 0x001A, // Equip Item
	DROP_ITEM = 0x001B, // Drop Item
	ALLEGIANCE_SWEAR = 0x001D, // Swear Allegiance request
	ALLEGIANCE_BREAK = 0x001E, // Break Allegiance request
	ALLEGIANCE_SEND_UPDATES = 0x001F, // Set 'Send Allegiance Updates' request
	CLEAR_FRIENDS = 0x0025,
	RECALL_PKL_ARENA = 0x0026,
	RECALL_PK_ARENA = 0x0027,
	SET_DISPLAY_TITLE = 0x002C,
	CONFIRMATION_RESPONSE = 0x0275, // confirmation response (currently only used for crafting)
	UST_SALVAGE_REQUEST = 0x027D, // ust salvage request
	ALLEGIANCE_QUERY_NAME = 0x0030,
	ALLEGIANCE_CLEAR_NAME = 0x0031,
	SEND_TELL_BY_GUID = 0x0032, // Send tell by GUID
	ALLEGIANCE_SET_NAME = 0x0033,
	USE_ITEM_EX = 0x0035, // Use Item Ex
	USE_OBJECT = 0x0036, // Use Object
	ALLEGIANCE_SET_OFFICER = 0x003B,
	ALLEGIANCE_SET_OFFICER_TITLE = 0x003C,
	ALLEGIANCE_LIST_OFFICER_TITLES = 0x003D,
	ALLEGIANCE_CLEAR_OFFICER_TITLES = 0x003E,
	ALLEGIANCE_LOCK_ACTION = 0x003F,
	ALLEGIANCE_APPROVED_VASSAL = 0x0040,
	ALLEGIANCE_CHAT_GAG = 0x0041,
	ALLEGIANCE_HOUSE_ACTION = 0x0042,
	SPEND_XP_VITALS = 0x0044, // spend XP on vitals (attribute2nd)
	SPEND_XP_ATTRIBUTES = 0x0045, // spend XP on attributes
	SPEND_XP_SKILLS = 0x0046, // spend XP on skills
	SPEND_SKILL_CREDITS = 0x0047, // spend credits to train a skill
	CAST_UNTARGETED_SPELL = 0x0048, // cast untargeted spell
	CAST_TARGETED_SPELL = 0x004A, // cast targeted spell
	CHANGE_COMBAT_STANCE = 0x0053, // Evt_Combat__ChangeCombatMode_ID "Change Combat Mode"
	STACKABLE_MERGE = 0x0054, // Evt_Inventory__StackableMerge
	STACKABLE_SPLIT_TO_CONTAINER = 0x0055, // Evt_Inventory__StackableSplitToContainer
	STACKABLE_SPLIT_TO_3D = 0x0056, // Evt_Inventory__StackableSplitTo3D
	STACKABLE_SPLIT_TO_WIELD = 0x019B, // Evt_Inventory__StackableSplitToWield
	SQUELCH_CHARACTER_MODIFY = 0x0058,
	SQUELCH_ACCOUNT_MODIFY = 0x0059,
	SQUELCH_GLOBAL_MODIFY = 0x005B,
	SEND_TELL_BY_NAME = 0x005D, // Send Tell by Name
	BUY_FROM_VENDOR = 0x005F, // Buy from Vendor
	SELL_TO_VENDOR = 0x0060, // Sell to Vendor
	RECALL_LIFESTONE = 0x0063, // Lifestone Recall
	LOGIN_COMPLETE = 0x00A1, // "Login Complete"
	FELLOW_CREATE = 0x00A2, // "Create Fellowship"
	FELLOW_QUIT = 0x00A3, // "Quit Fellowship"
	FELLOW_DISMISS = 0x00A4, // "Fellowship Dismiss"
	FELLOW_RECRUIT = 0x00A5, // "Fellowship Recruit"
	FELLOW_UPDATE = 0x00A6, // "Fellowship Update"
	BOOK_ADD_PAGE = 0x00AA,
	BOOK_MODIFY_PAGE = 0x00AB,
	BOOK_DATA = 0x00AC,
	BOOK_DELETE_PAGE = 0x00AD,
	BOOK_PAGE_DATA = 0x00AE,
	GIVE_OBJECT = 0x00CD, // Give someone an item
	PUT_OBJECT_IN_CONTAINER = 0x00CD, // Put object in container
	INSCRIBE = 0x00BF, // "Inscribe"
	APPRAISE = 0x00C8, // Identify
	ADMIN_TELEPORT = 0x00D6, // Advocate teleport (triggered by having an admin flag set, // clicking the mini-map)
	ABUSE_LOG_REQUEST = 0x0140,
	CHANNEL_ADD = 0x0145,
	CHANNEL_REMOVE = 0x0146,
	CHANNEL_TEXT = 0x0147, // Channel Text
	CHANNEL_LIST = 0x0148,
	CHANNEL_INDEX = 0x0149,
	TEXT_CHANNEL = 0x0147, // Channel Text
	NO_LONGER_VIEWING_CONTAINER = 0x0195, // No longer viewing contents
	ADD_ITEM_SHORTCUT = 0x019C, // Add item to shortcut bar
	REMOVE_ITEM_SHORTCUT = 0x019D, // Remove item to shortcut bar
	CHARACTER_OPTIONS = 0x01A1, // 
	SPELLBOOK_REMOVE = 0x01A8, // Delete spell from spellbook
	TOGGLE_SHOW_HELM = 0x01A1, // Toggle show helm?
	CANCEL_ATTACK = 0x01B7, // Cancel attack
	QUERY_HEALTH = 0x01BF, // Request health update
	QUERY_AGE = 0x01C2,
	QUERY_BIRTH = 0x01C4,
	HEALTH_UPDATE_REQUEST = 0x01BF, // Request health update
	TEXT_INDIRECT = 0x01DF, // Indirect Text (@me)
	TEXT_EMOTE = 0x01E1, // Emote Text (*laugh* sends 'laughs')
	ADD_TO_SPELLBAR = 0x01E3, // Add item to spell bar
	REMOVE_FROM_SPELLBAR = 0x01E4, // Remove item from spell bar
	PING = 0x01E9, // Ping
	TRADE_OPEN = 0x1F6, // Open Trade Negotiations
	TRADE_CLOSE = 0x1F7, // Close Trade Negotiations
	TRADE_ADD = 0x1F8, // AddToTrade
	TRADE_ACCEPT = 0x1FA, // Accept trade
	TRADE_DECLINE = 0x1FB, // Decline trade
	TRADE_RESET = 0x204, // Reset trade
	CLEAR_PLAYER_CONSENT_LIST = 0x0216, // Clears the player's corpse looting consent list, /consent clear
	DISPLAY_PLAYER_CONSENT_LIST = 0x0217, // Display the player's corpse looting consent list, /consent who 
	REMOVE_FROM_PLAYER_CONSENT_LIST = 0x0218, // Remove your corpse looting permission for the given player, /consent remove 
	ADD_PLAYER_PERMISSION = 0x0219, // Grants a player corpse looting permission, /permit add
	REMOVE_PLAYER_PERMISSION = 0x021A, // Revokes a player's corpse looting permission, /permit remove
	HOUSE_BUY = 0x021C, // House_BuyHouse 
	HOUSE_ABANDON = 0x021F, // House_AbandonHouse 
	HOUSE_OF_PLAYER_QUERY = 0x21E, // Query your house info, during signin 
	HOUSE_RENT = 0x0221, // House_RentHouse 
	DESIRED_COMPS_SET = 0x0224,
	HOUSE_ADD_GUEST = 0x0245, // House_AddPermanentGuest 
	HOUSE_REMOVE_GUEST = 0x0246, // House_RemovePermanentGuest
	HOUSE_SET_OPEN_ACCESS = 0x0247, // House_SetOpenHouseStatus
	HOUSE_CHANGE_STORAGE_PERMISSIONS = 0x0249, // House_ChangeStoragePermission
	HOUSE_BOOT_GUEST = 0x024A, // Boots a specific player from your house / house boot
	HOUSE_CLEAR_STORAGE_PERMISSIONS = 0x024C, // House_RemoveAllStoragePermission 
	HOUSE_GUEST_LIST = 0x024D, // House_RequestFullGuestList
	ALLEGIANCE_SET_MOTD = 0x0254, // Sets the allegiance message of the day, /allegiance motd set
	ALLEGIANCE_QUERY_MOTD = 0x0255, // Query the motd, /allegiance motd
	ALLEGIANCE_CLEAR_MOTD = 0x0256, // Clear the motd, /allegiance motd clear
	HOUSE_QUERY_SLUMLORD = 0x0258, // Gets SlumLord info, sent after getting a failed house transaction
	ALLEGIANCE_MOTD = 0x0255, // Request allegiance MOTD
	HOUSE_SET_OPEN_STORAGE_ACCESS = 0x025C, // House_AddAllStoragePermission
	HOUSE_REMOVE_ALL_GUESTS = 0x025E, // House_RemoveAllPermanentGuests
	HOUSE_BOOT_ALL = 0x025F, // Boot everyone from your house, /house boot -all
	RECALL_HOUSE = 0x0262, // House Recall
	ITEM_MANA_REQUEST = 0x0263, // Request Item Mana
	HOUSE_SET_HOOKS_VISIBILITY = 0x0266, // House_SetHooksVisibility 
	HOUSE_CHANGE_ALLEGIANCE_GUEST_PERMISSIONS = 0x0267, // House_ModifyAllegianceGuestPermission 
	HOUSE_CHANGE_ALLEGIANCE_STORAGE_PERMISSIONS = 0x0268, // House_ModifyAllegianceStoragePermission
	CHESS_JOIN = 0x0269, // Joins a chess game
	CHESS_QUIT = 0x026A, // Quits a chess game
	CHESS_MOVE = 0x026B, // Makes a chess move
	CHESS_PASS = 0x026D, // Pass your move
	CHESS_STALEMATE = 0x026E, // Offer or confirm stalemate
	HOUSE_LIST_AVAILABLE = 0x0270, // Lists available house /house available
	ALLEGIANCE_BOOT_PLAYER = 0x0277, // Boots a player from the allegiance, optionally all characters on their account
	RECALL_HOUSE_MANSION = 0x0278, // House_TeleToMansion
	DIE_COMMAND = 0x0279, // "/die" command
	ALLEGIANCE_INFO_REQUEST = 0x027B, // allegiance info request
	SPELLBOOK_FILTERS = 0x0286, // filter player spellbook by type/level
	RECALL_MARKET = 0x028D, // Marketplace Recall
	PKLITE = 0x028F, // Enter PKLite mode
	FELLOW_ASSIGN_NEW_LEADER = 0x0290, // "Fellowship Assign New Leader"
	FELLOW_CHANGE_OPENNESS = 0x0291, // "Fellowship Change Openness"
	ALLEGIANCE_CHAT_BOOT = 0x02A0, // Boots a player from the allegiance chat
	ALLEGIANCE_ADD_PLAYER_BAN = 0x02A1, // Bans a player from the allegiance
	ALLEGIANCE_REMOVE_PLAYER_BAN = 0x02A2, // Removes a player ban from the allegiance
	ALLEGIANCE_LIST_BANS = 0x02A3, // Display allegiance bans
	ALLEGIANCE_REMOVE_OFFICER = 0x02A5, // Removes an allegiance officer
	ALLEGIANCE_LIST_OFFICERS = 0x02A6, // List allegiance officers
	ALLEGIANCE_CLEAR_OFFICERS = 0x02A7, // Clear allegiance officers
	RECALL_ALLEGIANCE_HOMETOWN = 0x02AB, // Allegiance_RecallAllegianceHometown (bindstone)
	FINISH_BARBER = 0x0311, // Completes the barber interaction
	CONTRACT_ABANDON = 0x0316, // Abandons a contract
	MOVEMENT_JUMP = 0xF61B, // Jump Movement
	MOVEMENT_MOVE_TO_STATE = 0xF61C, // Move to state data
	MOVEMENT_DO_MOVEMENT_COMMAND = 0xF61E, // Performs a movement based on input
	MOVEMENT_TURN_TO = 0xF649, // Turn to event data
	MOVEMENT_STOP = 0xF661, // Stops a movement
	MOVEMENT_AUTONOMY_LEVEL = 0xF752, // Sets an autonomy level
	MOVEMENT_AUTONOMOUS_POSITION = 0xF753, // Sends an autonomous position
	MOVEMENT_JUMP_NON_AUTONOMOUS = 0xF7C9 // Performs a non autonomous jump

};

enum ItemSet
{
	Invalid_ItemSet,
	Test_ItemSet,
	Test2_ItemSet,
	UNKNOWN_3_ItemSet,
	CarraidasBenediction_ItemSet,
	NobleRelic_ItemSet,
	AncientRelic_ItemSet,
	AlduressaRelic_ItemSet,
	Shoujen_ItemSet, // Ninja Nanjou
	EmpyreanRings_ItemSet,
	ArmMindHeart_ItemSet,
	ArmorPerfectLight_ItemSet,
	ArmorPerfectLight2_ItemSet,
	Soldiers_ItemSet,
	Adepts_ItemSet,
	Archers_ItemSet,
	Defenders_ItemSet,
	Tinkers_ItemSet,
	Crafters_ItemSet,
	Hearty_ItemSet,
	Dexterous_ItemSet,
	Wise_ItemSet,
	Swift_ItemSet,
	Hardened_ItemSet,
	Reinforced_ItemSet,
	Interlocking_ItemSet,
	Flameproof_ItemSet,
	Acidproof_ItemSet,
	Coldproof_ItemSet,
	Lightningproof_ItemSet,
	SocietyArmor_ItemSet,
	ColosseumClothing_ItemSet,
	GraveyardClothing_ItemSet,
	OlthoiClothing_ItemSet,
	NoobieArmor_ItemSet,
	AetheriaDefense_ItemSet,
	AetheriaDestruction_ItemSet,
	AetheriaFury_ItemSet,
	AetheriaGrowth_ItemSet,
	AetheriaVigor_ItemSet,
	RareDamageResistance_ItemSet,
	RareDamageBoost_ItemSet,
	OlthoiArmorDRed_ItemSet,
	OlthoiArmorCRat_ItemSet,
	OlthoiArmorCRed_ItemSet,
	OlthoiArmorDRat_ItemSet,
	AlduressaRelicUpgrade_ItemSet,
	AncientRelicUpgrade_ItemSet,
	NobleRelicUpgrade_ItemSet,
	CloakAlchemy_ItemSet,
	CloakArcaneLore_ItemSet,
	CloakArmorTinkering_ItemSet,
	CloakAssessPerson_ItemSet,
	CloakAxe_ItemSet,
	CloakBow_ItemSet,
	CloakCooking_ItemSet,
	CloakCreatureEnchantment_ItemSet,
	CloakCrossbow_ItemSet,
	CloakDagger_ItemSet,
	CloakDeception_ItemSet,
	CloakFletching_ItemSet,
	CloakHealing_ItemSet,
	CloakItemEnchantment_ItemSet,
	CloakItemTinkering_ItemSet,
	CloakLeadership_ItemSet,
	CloakLifeMagic_ItemSet,
	CloakLoyalty_ItemSet,
	CloakMace_ItemSet,
	CloakMagicDefense_ItemSet,
	CloakMagicItemTinkering_ItemSet,
	CloakManaConversion_ItemSet,
	CloakMeleeDefense_ItemSet,
	CloakMissileDefense_ItemSet,
	CloakSalvaging_ItemSet,
	CloakSpear_ItemSet,
	CloakStaff_ItemSet,
	CloakSword_ItemSet,
	CloakThrownWeapon_ItemSet,
	CloakTwoHandedCombat_ItemSet,
	CloakUnarmedCombat_ItemSet,
	CloakVoidMagic_ItemSet,
	CloakWarMagic_ItemSet,
	CloakWeaponTinkering_ItemSet,
	CloakAssessCreature_ItemSet,
	CloakDirtyFighting_ItemSet,
	CloakDualWield_ItemSet,
	CloakRecklessness_ItemSet,
	CloakShield_ItemSet,
	CloakSneakAttack_ItemSet,
	Reinforced_Shoujen_ItemSet, // Ninja Shozoku
	CloakSummoning_ItemSet,
	ShroudedSoul_ItemSet,
	DarkenedMind_ItemSet,
	CloudedSpirit_ItemSet,
	MinorStingingShroudedSoul_ItemSet,
	MinorSparkingShroudedSoul_ItemSet,
	MinorSmolderingShroudedSoul_ItemSet,
	MinorShiveringShroudedSoul_ItemSet,
	MinorStingingDarkenedMind_ItemSet,
	MinorSparkingDarkenedMind_ItemSet,
	MinorSmolderingDarkenedMind_ItemSet,
	MinorShiveringDarkenedMind_ItemSet,
	MinorStingingCloudedSpirit_ItemSet,
	MinorSparkingCloudedSpirit_ItemSet,
	MinorSmolderingCloudedSpirit_ItemSet,
	MinorShiveringCloudedSpirit_ItemSet,
	MajorStingingShroudedSoul_ItemSet,
	MajorSparkingShroudedSoul_ItemSet,
	MajorSmolderingShroudedSoul_ItemSet,
	MajorShiveringShroudedSoul_ItemSet,
	MajorStingingDarkenedMind_ItemSet,
	MajorSparkingDarkenedMind_ItemSet,
	MajorSmolderingDarkenedMind_ItemSet,
	MajorShiveringDarkenedMind_ItemSet,
	MajorStingingCloudedSpirit_ItemSet,
	MajorSparkingCloudedSpirit_ItemSet,
	MajorSmolderingCloudedSpirit_ItemSet,
	MajorShiveringCloudedSpirit_ItemSet,
	BlackfireStingingShroudedSoul_ItemSet,
	BlackfireSparkingShroudedSoul_ItemSet,
	BlackfireSmolderingShroudedSoul_ItemSet,
	BlackfireShiveringShroudedSoul_ItemSet,
	BlackfireStingingDarkenedMind_ItemSet,
	BlackfireSparkingDarkenedMind_ItemSet,
	BlackfireSmolderingDarkenedMind_ItemSet,
	BlackfireShiveringDarkenedMind_ItemSet,
	BlackfireStingingCloudedSpirit_ItemSet,
	BlackfireSparkingCloudedSpirit_ItemSet,
	BlackfireSmolderingCloudedSpirit_ItemSet,
	BlackfireShiveringCloudedSpirit_ItemSet,
	ShimmeringShadowsSet_ItemSet,
	BrownSocietyLocket_ItemSet,
	YellowSocietyLocket_ItemSet,
	RedSocietyBand_ItemSet,
	GreenSocietyBand_ItemSet,
	PurpleSocietyBand_ItemSet,
	BlueSocietyBand_ItemSet,
	GauntletGarb_ItemSet,
	ParagonMissile_ItemSet,
	ParagonMagic_ItemSet,
	ParagonMelee_ItemSet
};

enum AllegianceLockAction
{
	LockedOff = 1,
	LockedOn,
	ToggleLocked,
	CheckLocked,
	DisplayBypass,
	ClearBypass
};

enum ConfirmationTypes
{
	SwearAllegianceConfirm = 1,
	AlterSkillConfirm,
	AlterAttributeConfirm,
	FellowshipConfirm,
	CraftConfirm,
	AugmentationConfirm,
	YesNoConfirm
};

enum TargetingTacticType
{
	Target_None		    = 0x00,
	Target_Random		= 0x01,
	Target_Focused		= 0x02,
	Target_LastDamager	= 0x04,
	Target_TopDamager	= 0x08,
	Target_Weakest		= 0x10,
	Target_Strongest	= 0x20,
	Target_Nearest		= 0x40
};

enum SquelchTypes {
	AllChannels_Squelch = 1,
	Speech_Squelch = 2,
	SpeechDirect_Squelch = 3, // @tell
	Combat_Squelch = 6,
	Magic_Squelch = 7,
	Emote_Squelch = 12,
	AppraisalChannel_Squelch = 16,
	MagicCastingChannel_Squelch = 17,
	AllegienceChannel_Squelch = 18,
	FellowshipChannel_Squelch = 19,
	CombatEnemy_Squelch = 21,
	CombatSelf_Squelch = 22,
	Recall_Squelch = 23,
	Craft_Squelch = 24,
	Salvaging_Squelch = 25
};

enum SquelchMasks {
	Speech_Mask = 0x00000004,
	SpeechDirect_Mask = 0x00000008, // @tell
	Combat_Mask = 0x00000040,
	Magic_Mask = 0x00000080,
	Emote_Mask = 0x00001000,
	AppraisalChannel_Mask = 0x00010000,
	MagicCastingChannel_Mask = 0x00020000,
	AllegienceChannel_Mask = 0x00040000,
	FellowshipChannel_Mask = 0x00080000,
	CombatEnemy_Mask = 0x00200000,
	CombatSelf_Mask = 0x00400000,
	Recall_Mask = 0x00800000,
	Craft_Mask = 0x01000000,
	Salvaging_Mask = 0x02000000,
	AllChannels_Mask = 0xFFFFFFFF
};

enum SpellCategory
{
	Undef_SpellCategory,
	Strength_Raising_SpellCategory,
	Strength_Lowering_SpellCategory,
	Endurance_Raising_SpellCategory,
	Endurance_Lowering_SpellCategory,
	Quickness_Raising_SpellCategory,
	Quickness_Lowering_SpellCategory,
	Coordination_Raising_SpellCategory,
	Coordination_Lowering_SpellCategory,
	Focus_Raising_SpellCategory,
	Focus_Lowering_SpellCategory,
	Self_Raising_SpellCategory,
	Self_Lowering_SpellCategory,
	Focus_Concentration_SpellCategory,
	Focus_Disruption_SpellCategory,
	Focus_Brilliance_SpellCategory,
	Focus_Dullness_SpellCategory,
	Axe_Raising_SpellCategory,
	Axe_Lowering_SpellCategory,
	Bow_Raising_SpellCategory,
	Bow_Lowering_SpellCategory,
	Crossbow_Raising_SpellCategory,
	Crossbow_Lowering_SpellCategory,
	Dagger_Raising_SpellCategory,
	Dagger_Lowering_SpellCategory,
	Mace_Raising_SpellCategory,
	Mace_Lowering_SpellCategory,
	Spear_Raising_SpellCategory,
	Spear_Lowering_SpellCategory,
	Staff_Raising_SpellCategory,
	Staff_Lowering_SpellCategory,
	Sword_Raising_SpellCategory,
	Sword_Lowering_SpellCategory,
	Thrown_Weapons_Raising_SpellCategory,
	Thrown_Weapons_Lowering_SpellCategory,
	Unarmed_Combat_Raising_SpellCategory,
	Unarmed_Combat_Lowering_SpellCategory,
	Melee_Defense_Raising_SpellCategory,
	Melee_Defense_Lowering_SpellCategory,
	Missile_Defense_Raising_SpellCategory,
	Missile_Defense_Lowering_SpellCategory,
	Magic_Defense_Raising_SpellCategory,
	Magic_Defense_Lowering_SpellCategory,
	Creature_Enchantment_Raising_SpellCategory,
	Creature_Enchantment_Lowering_SpellCategory,
	Item_Enchantment_Raising_SpellCategory,
	Item_Enchantment_Lowering_SpellCategory,
	Life_Magic_Raising_SpellCategory,
	Life_Magic_Lowering_SpellCategory,
	War_Magic_Raising_SpellCategory,
	War_Magic_Lowering_SpellCategory,
	Mana_Conversion_Raising_SpellCategory,
	Mana_Conversion_Lowering_SpellCategory,
	Arcane_Lore_Raising_SpellCategory,
	Arcane_Lore_Lowering_SpellCategory,
	Appraise_Armor_Raising_SpellCategory,
	Appraise_Armor_Lowering_SpellCategory,
	Appraise_Item_Raising_SpellCategory,
	Appraise_Item_Lowering_SpellCategory,
	Appraise_Magic_Item_Raising_SpellCategory,
	Appraise_Magic_Item_Lowering_SpellCategory,
	Appraise_Weapon_Raising_SpellCategory,
	Appraise_Weapon_Lowering_SpellCategory,
	Assess_Monster_Raising_SpellCategory,
	Assess_Monster_Lowering_SpellCategory,
	Deception_Raising_SpellCategory,
	Deception_Lowering_SpellCategory,
	Healing_Raising_SpellCategory,
	Healing_Lowering_SpellCategory,
	Jump_Raising_SpellCategory,
	Jump_Lowering_SpellCategory,
	Leadership_Raising_SpellCategory,
	Leadership_Lowering_SpellCategory,
	Lockpick_Raising_SpellCategory,
	Lockpick_Lowering_SpellCategory,
	Loyalty_Raising_SpellCategory,
	Loyalty_Lowering_SpellCategory,
	Run_Raising_SpellCategory,
	Run_Lowering_SpellCategory,
	Health_Raising_SpellCategory,
	Health_Lowering_SpellCategory,
	Stamina_Raising_SpellCategory,
	Stamina_Lowering_SpellCategory,
	Mana_Raising_SpellCategory,
	Mana_Lowering_SpellCategory,
	Mana_Remedy_SpellCategory,
	Mana_Malediction_SpellCategory,
	Health_Transfer_to_caster_SpellCategory,
	Health_Transfer_from_caster_SpellCategory,
	Stamina_Transfer_to_caster_SpellCategory,
	Stamina_Transfer_from_caster_SpellCategory,
	Mana_Transfer_to_caster_SpellCategory,
	Mana_Transfer_from_caster_SpellCategory,
	Health_Accelerating_SpellCategory,
	Health_Decelerating_SpellCategory,
	Stamina_Accelerating_SpellCategory,
	Stamina_Decelerating_SpellCategory,
	Mana_Accelerating_SpellCategory,
	Mana_Decelerating_SpellCategory,
	Vitae_Raising_SpellCategory,
	Vitae_Lowering_SpellCategory,
	Acid_Protection_SpellCategory,
	Acid_Vulnerability_SpellCategory,
	Bludgeon_Protection_SpellCategory,
	Bludgeon_Vulnerability_SpellCategory,
	Cold_Protection_SpellCategory,
	Cold_Vulnerability_SpellCategory,
	Electric_Protection_SpellCategory,
	Electric_Vulnerability_SpellCategory,
	Fire_Protection_SpellCategory,
	Fire_Vulnerability_SpellCategory,
	Pierce_Protection_SpellCategory,
	Pierce_Vulnerability_SpellCategory,
	Slash_Protection_SpellCategory,
	Slash_Vulnerability_SpellCategory,
	Armor_Raising_SpellCategory,
	Armor_Lowering_SpellCategory,
	Acid_Missile_SpellCategory,
	Bludgeoning_Missile_SpellCategory,
	Cold_Missile_SpellCategory,
	Electric_Missile_SpellCategory,
	Fire_Missile_SpellCategory,
	Piercing_Missile_SpellCategory,
	Slashing_Missile_SpellCategory,
	Acid_Seeker_SpellCategory,
	Bludgeoning_Seeker_SpellCategory,
	Cold_Seeker_SpellCategory,
	Electric_Seeker_SpellCategory,
	Fire_Seeker_SpellCategory,
	Piercing_Seeker_SpellCategory,
	Slashing_Seeker_SpellCategory,
	Acid_Burst_SpellCategory,
	Bludgeoning_Burst_SpellCategory,
	Cold_Burst_SpellCategory,
	Electric_Burst_SpellCategory,
	Fire_Burst_SpellCategory,
	Piercing_Burst_SpellCategory,
	Slashing_Burst_SpellCategory,
	Acid_Blast_SpellCategory,
	Bludgeoning_Blast_SpellCategory,
	Cold_Blast_SpellCategory,
	Electric_Blast_SpellCategory,
	Fire_Blast_SpellCategory,
	Piercing_Blast_SpellCategory,
	Slashing_Blast_SpellCategory,
	Acid_Scatter_SpellCategory,
	Bludgeoning_Scatter_SpellCategory,
	Cold_Scatter_SpellCategory,
	Electric_Scatter_SpellCategory,
	Fire_Scatter_SpellCategory,
	Piercing_Scatter_SpellCategory,
	Slashing_Scatter_SpellCategory,
	Attack_Mod_Raising_SpellCategory,
	Attack_Mod_Lowering_SpellCategory,
	Damage_Raising_SpellCategory,
	Damage_Lowering_SpellCategory,
	Defense_Mod_Raising_SpellCategory,
	Defense_Mod_Lowering_SpellCategory,
	Weapon_Time_Raising_SpellCategory,
	Weapon_Time_Lowering_SpellCategory,
	Armor_Value_Raising_SpellCategory,
	Armor_Value_Lowering_SpellCategory,
	Acid_Resistance_Raising_SpellCategory,
	Acid_Resistance_Lowering_SpellCategory,
	Bludgeon_Resistance_Raising_SpellCategory,
	Bludgeon_Resistance_Lowering_SpellCategory,
	Cold_Resistance_Raising_SpellCategory,
	Cold_Resistance_Lowering_SpellCategory,
	Electric_Resistance_Raising_SpellCategory,
	Electric_Resistance_Lowering_SpellCategory,
	Fire_Resistance_Raising_SpellCategory,
	Fire_Resistance_Lowering_SpellCategory,
	Pierce_Resistance_Raising_SpellCategory,
	Pierce_Resistance_Lowering_SpellCategory,
	Slash_Resistance_Raising_SpellCategory,
	Slash_Resistance_Lowering_SpellCategory,
	Bludgeoning_Resistance_Raising_SpellCategory,
	Bludgeoning_Resistance_Lowering_SpellCategory,
	Slashing_Resistance_Raising_SpellCategory,
	Slashing_Resistance_Lowering_SpellCategory,
	Piercing_Resistance_Raising_SpellCategory,
	Piercing_Resistance_Lowering_SpellCategory,
	Electrical_Resistance_Raising_SpellCategory,
	Electrical_Resistance_Lowering_SpellCategory,
	Frost_Resistance_Raising_SpellCategory,
	Frost_Resistance_Lowering_SpellCategory,
	Flame_Resistance_Raising_SpellCategory,
	Flame_Resistance_Lowering_SpellCategory,
	Acidic_Resistance_Raising_SpellCategory,
	Acidic_Resistance_Lowering_SpellCategory,
	Armor_Level_Raising_SpellCategory,
	Armor_Level_Lowering_SpellCategory,
	Lockpick_Resistance_Raising_SpellCategory,
	Lockpick_Resistance_Lowering_SpellCategory,
	Appraisal_Resistance_Raising_SpellCategory,
	Appraisal_Resistance_Lowering_SpellCategory,
	Vision_Raising_SpellCategory,
	Vision_Lowering_SpellCategory,
	Transparency_Raising_SpellCategory,
	Transparency_Lowering_SpellCategory,
	Portal_Tie_SpellCategory,
	Portal_Recall_SpellCategory,
	Portal_Creation_SpellCategory,
	Portal_Item_Creation_SpellCategory,
	Vitae_SpellCategory,
	Assess_Person_Raising_SpellCategory,
	Assess_Person_Lowering_SpellCategory,
	Acid_Volley_SpellCategory,
	Bludgeoning_Volley_SpellCategory,
	Frost_Volley_SpellCategory,
	Lightning_Volley_SpellCategory,
	Flame_Volley_SpellCategory,
	Force_Volley_SpellCategory,
	Blade_Volley_SpellCategory,
	Portal_Sending_SpellCategory,
	Lifestone_Sending_SpellCategory,
	Cooking_Raising_SpellCategory,
	Cooking_Lowering_SpellCategory,
	Fletching_Raising_SpellCategory,
	Fletching_Lowering_SpellCategory,
	Alchemy_Lowering_SpellCategory,
	Alchemy_Raising_SpellCategory,
	Acid_Ring_SpellCategory,
	Bludgeoning_Ring_SpellCategory,
	Cold_Ring_SpellCategory,
	Electric_Ring_SpellCategory,
	Fire_Ring_SpellCategory,
	Piercing_Ring_SpellCategory,
	Slashing_Ring_SpellCategory,
	Acid_Wall_SpellCategory,
	Bludgeoning_Wall_SpellCategory,
	Cold_Wall_SpellCategory,
	Electric_Wall_SpellCategory,
	Fire_Wall_SpellCategory,
	Piercing_Wall_SpellCategory,
	Slashing_Wall_SpellCategory,
	Acid_Strike_SpellCategory,
	Bludgeoning_Strike_SpellCategory,
	Cold_Strike_SpellCategory,
	Electric_Strike_SpellCategory,
	Fire_Strike_SpellCategory,
	Piercing_Strike_SpellCategory,
	Slashing_Strike_SpellCategory,
	Acid_Streak_SpellCategory,
	Bludgeoning_Streak_SpellCategory,
	Cold_Streak_SpellCategory,
	Electric_Streak_SpellCategory,
	Fire_Streak_SpellCategory,
	Piercing_Streak_SpellCategory,
	Slashing_Streak_SpellCategory,
	Dispel_SpellCategory,
	Creature_Mystic_Raising_SpellCategory,
	Creature_Mystic_Lowering_SpellCategory,
	Item_Mystic_Raising_SpellCategory,
	Item_Mystic_Lowering_SpellCategory,
	War_Mystic_Raising_SpellCategory,
	War_Mystic_Lowering_SpellCategory,
	Health_Restoring_SpellCategory,
	Health_Depleting_SpellCategory,
	Mana_Restoring_SpellCategory,
	Mana_Depleting_SpellCategory,
	Strength_Increase_SpellCategory,
	Strength_Decrease_SpellCategory,
	Endurance_Increase_SpellCategory,
	Endurance_Decrease_SpellCategory,
	Quickness_Increase_SpellCategory,
	Quickness_Decrease_SpellCategory,
	Coordination_Increase_SpellCategory,
	Coordination_Decrease_SpellCategory,
	Focus_Increase_SpellCategory,
	Focus_Decrease_SpellCategory,
	Self_Increase_SpellCategory,
	Self_Decrease_SpellCategory,
	GreatVitality_Raising_SpellCategory,
	PoorVitality_Lowering_SpellCategory,
	GreatVigor_Raising_SpellCategory,
	PoorVigor_Lowering_SpellCategory,
	GreaterIntellect_Raising_SpellCategory,
	LessorIntellect_Lowering_SpellCategory,
	LifeGiver_Raising_SpellCategory,
	LifeTaker_Lowering_SpellCategory,
	StaminaGiver_Raising_SpellCategory,
	StaminaTaker_Lowering_SpellCategory,
	ManaGiver_Raising_SpellCategory,
	ManaTaker_Lowering_SpellCategory,
	Acid_Ward_Protection_SpellCategory,
	Acid_Ward_Vulnerability_SpellCategory,
	Fire_Ward_Protection_SpellCategory,
	Fire_Ward_Vulnerability_SpellCategory,
	Cold_Ward_Protection_SpellCategory,
	Cold_Ward_Vulnerability_SpellCategory,
	Electric_Ward_Protection_SpellCategory,
	Electric_Ward_Vulnerability_SpellCategory,
	Leadership_Obedience_Raising_SpellCategory,
	Leadership_Obedience_Lowering_SpellCategory,
	Melee_Defense_Shelter_Raising_SpellCategory,
	Melee_Defense_Shelter_Lowering_SpellCategory,
	Missile_Defense_Shelter_Raising_SpellCategory,
	Missile_Defense_Shelter_Lowering_SpellCategory,
	Magic_Defense_Shelter_Raising_SpellCategory,
	Magic_Defense_Shelter_Lowering_SpellCategory,
	HuntersAcumen_Raising_SpellCategory,
	HuntersAcumen_Lowering_SpellCategory,
	StillWater_Raising_SpellCategory,
	StillWater_Lowering_SpellCategory,
	StrengthofEarth_Raising_SpellCategory,
	StrengthofEarth_Lowering_SpellCategory,
	Torrent_Raising_SpellCategory,
	Torrent_Lowering_SpellCategory,
	Growth_Raising_SpellCategory,
	Growth_Lowering_SpellCategory,
	CascadeAxe_Raising_SpellCategory,
	CascadeAxe_Lowering_SpellCategory,
	CascadeDagger_Raising_SpellCategory,
	CascadeDagger_Lowering_SpellCategory,
	CascadeMace_Raising_SpellCategory,
	CascadeMace_Lowering_SpellCategory,
	CascadeSpear_Raising_SpellCategory,
	CascadeSpear_Lowering_SpellCategory,
	CascadeStaff_Raising_SpellCategory,
	CascadeStaff_Lowering_SpellCategory,
	StoneCliffs_Raising_SpellCategory,
	StoneCliffs_Lowering_SpellCategory,
	MaxDamage_Raising_SpellCategory,
	MaxDamage_Lowering_SpellCategory,
	Bow_Damage_Raising_SpellCategory,
	Bow_Damage_Lowering_SpellCategory,
	Bow_Range_Raising_SpellCategory,
	Bow_Range_Lowering_SpellCategory,
	Extra_Defense_Mod_Raising_SpellCategory,
	Extra_Defense_Mod_Lowering_SpellCategory,
	Extra_Bow_Skill_Raising_SpellCategory,
	Extra_Bow_Skill_Lowering_SpellCategory,
	Extra_Alchemy_Skill_Raising_SpellCategory,
	Extra_Alchemy_Skill_Lowering_SpellCategory,
	Extra_Arcane_Lore_Skill_Raising_SpellCategory,
	Extra_Arcane_Lore_Skill_Lowering_SpellCategory,
	Extra_Appraise_Armor_Skill_Raising_SpellCategory,
	Extra_Appraise_Armor_Skill_Lowering_SpellCategory,
	Extra_Cooking_Skill_Raising_SpellCategory,
	Extra_Cooking_Skill_Lowering_SpellCategory,
	Extra_Crossbow_Skill_Raising_SpellCategory,
	Extra_Crossbow_Skill_Lowering_SpellCategory,
	Extra_Deception_Skill_Raising_SpellCategory,
	Extra_Deception_Skill_Lowering_SpellCategory,
	Extra_Loyalty_Skill_Raising_SpellCategory,
	Extra_Loyalty_Skill_Lowering_SpellCategory,
	Extra_Fletching_Skill_Raising_SpellCategory,
	Extra_Fletching_Skill_Lowering_SpellCategory,
	Extra_Healing_Skill_Raising_SpellCategory,
	Extra_Healing_Skill_Lowering_SpellCategory,
	Extra_Melee_Defense_Skill_Raising_SpellCategory,
	Extra_Melee_Defense_Skill_Lowering_SpellCategory,
	Extra_Appraise_Item_Skill_Raising_SpellCategory,
	Extra_Appraise_Item_Skill_Lowering_SpellCategory,
	Extra_Jumping_Skill_Raising_SpellCategory,
	Extra_Jumping_Skill_Lowering_SpellCategory,
	Extra_Life_Magic_Skill_Raising_SpellCategory,
	Extra_Life_Magic_Skill_Lowering_SpellCategory,
	Extra_Lockpick_Skill_Raising_SpellCategory,
	Extra_Lockpick_Skill_Lowering_SpellCategory,
	Extra_Appraise_Magic_Item_Skill_Raising_SpellCategory,
	Extra_Appraise_Magic_Item_Skill_Lowering_SpellCategory,
	Extra_Mana_Conversion_Skill_Raising_SpellCategory,
	Extra_Mana_Conversion_Skill_Lowering_SpellCategory,
	Extra_Assess_Creature_Skill_Raising_SpellCategory,
	Extra_Assess_Creature_Skill_Lowering_SpellCategory,
	Extra_Assess_Person_Skill_Raising_SpellCategory,
	Extra_Assess_Person_Skill_Lowering_SpellCategory,
	Extra_Run_Skill_Raising_SpellCategory,
	Extra_Run_Skill_Lowering_SpellCategory,
	Extra_Sword_Skill_Raising_SpellCategory,
	Extra_Sword_Skill_Lowering_SpellCategory,
	Extra_Thrown_Weapons_Skill_Raising_SpellCategory,
	Extra_Thrown_Weapons_Skill_Lowering_SpellCategory,
	Extra_Unarmed_Combat_Skill_Raising_SpellCategory,
	Extra_Unarmed_Combat_Skill_Lowering_SpellCategory,
	Extra_Appraise_Weapon_Skill_Raising_SpellCategory,
	Extra_Appraise_Weapon_Skill_Lowering_SpellCategory,
	Armor_Increase_SpellCategory,
	Armor_Decrease_SpellCategory,
	Extra_Acid_Resistance_Raising_SpellCategory,
	Extra_Acid_Resistance_Lowering_SpellCategory,
	Extra_Bludgeon_Resistance_Raising_SpellCategory,
	Extra_Bludgeon_Resistance_Lowering_SpellCategory,
	Extra_Fire_Resistance_Raising_SpellCategory,
	Extra_Fire_Resistance_Lowering_SpellCategory,
	Extra_Cold_Resistance_Raising_SpellCategory,
	Extra_Cold_Resistance_Lowering_SpellCategory,
	Extra_Attack_Mod_Raising_SpellCategory,
	Extra_Attack_Mod_Lowering_SpellCategory,
	Extra_Armor_Value_Raising_SpellCategory,
	Extra_Armor_Value_Lowering_SpellCategory,
	Extra_Pierce_Resistance_Raising_SpellCategory,
	Extra_Pierce_Resistance_Lowering_SpellCategory,
	Extra_Slash_Resistance_Raising_SpellCategory,
	Extra_Slash_Resistance_Lowering_SpellCategory,
	Extra_Electric_Resistance_Raising_SpellCategory,
	Extra_Electric_Resistance_Lowering_SpellCategory,
	Extra_Weapon_Time_Raising_SpellCategory,
	Extra_Weapon_Time_Lowering_SpellCategory,
	Bludgeon_Ward_Protection_SpellCategory,
	Bludgeon_Ward_Vulnerability_SpellCategory,
	Slash_Ward_Protection_SpellCategory,
	Slash_Ward_Vulnerability_SpellCategory,
	Pierce_Ward_Protection_SpellCategory,
	Pierce_Ward_Vulnerability_SpellCategory,
	Stamina_Restoring_SpellCategory,
	Stamina_Depleting_SpellCategory,
	Fireworks_SpellCategory,
	Health_Divide_SpellCategory,
	Stamina_Divide_SpellCategory,
	Mana_Divide_SpellCategory,
	Coordination_Increase2_SpellCategory,
	Strength_Increase2_SpellCategory,
	Focus_Increase2_SpellCategory,
	Endurance_Increase2_SpellCategory,
	Self_Increase2_SpellCategory,
	Melee_Defense_Multiply_SpellCategory,
	Missile_Defense_Multiply_SpellCategory,
	Magic_Defense_Multiply_SpellCategory,
	Attributes_Decrease_SpellCategory,
	LifeGiver_Raising2_SpellCategory,
	Item_Enchantment_Raising2_SpellCategory,
	Skills_Decrease_SpellCategory,
	Extra_Mana_Conversion_Bonus_SpellCategory,
	War_Mystic_Raising2_SpellCategory,
	War_Mystic_Lowering2_SpellCategory,
	Magic_Defense_Shelter_Raising2_SpellCategory,
	Extra_Life_Magic_Skill_Raising2_SpellCategory,
	Creature_Mystic_Raising2_SpellCategory,
	Item_Mystic_Raising2_SpellCategory,
	Mana_Raising2_SpellCategory,
	Self_Raising2_SpellCategory,
	CreatureEnchantment_Raising2_SpellCategory,
	Salvaging_Raising_SpellCategory,
	Extra_Salvaging_Raising_SpellCategory,
	Extra_Salvaging_Raising2_SpellCategory,
	CascadeAxe_Raising2_SpellCategory,
	Extra_Bow_Skill_Raising2_SpellCategory,
	Extra_Thrown_Weapons_Skill_Raising2_SpellCategory,
	Extra_Crossbow_Skill_Raising2_SpellCategory,
	CascadeDagger_Raising2_SpellCategory,
	CascadeMace_Raising2_SpellCategory,
	Extra_Unarmed_Combat_Skill_Raising2_SpellCategory,
	CascadeSpear_Raising2_SpellCategory,
	CascadeStaff_Raising2_SpellCategory,
	Extra_Sword_Skill_Raising2_SpellCategory,
	Acid_Protection_Rare_SpellCategory,
	Acid_Resistance_Raising_Rare_SpellCategory,
	Alchemy_Raising_Rare_SpellCategory,
	Appraisal_Resistance_Lowering_Rare_SpellCategory,
	Appraise_Armor_Raising_Rare_SpellCategory,
	Appraise_Item_Raising_Rare_SpellCategory,
	Appraise_Magic_Item_Raising_Rare_SpellCategory,
	Appraise_Weapon_Raising_Rare_SpellCategory,
	Arcane_Lore_Raising_Rare_SpellCategory,
	Armor_Raising_Rare_SpellCategory,
	Armor_Value_Raising_Rare_SpellCategory,
	Assess_Monster_Raising_Rare_SpellCategory,
	Assess_Person_Raising_Rare_SpellCategory,
	Attack_Mod_Raising_Rare_SpellCategory,
	Axe_Raising_Rare_SpellCategory,
	Bludgeon_Protection_Rare_SpellCategory,
	Bludgeon_Resistance_Raising_Rare_SpellCategory,
	Bow_Raising_Rare_SpellCategory,
	Cold_Protection_Rare_SpellCategory,
	Cold_Resistance_Raising_Rare_SpellCategory,
	Cooking_Raising_Rare_SpellCategory,
	Coordination_Raising_Rare_SpellCategory,
	Creature_Enchantment_Raising_Rare_SpellCategory,
	Crossbow_Raising_Rare_SpellCategory,
	Dagger_Raising_Rare_SpellCategory,
	Damage_Raising_Rare_SpellCategory,
	Deception_Raising_Rare_SpellCategory,
	Defense_Mod_Raising_Rare_SpellCategory,
	Electric_Protection_Rare_SpellCategory,
	Electric_Resistance_Raising_Rare_SpellCategory,
	Endurance_Raising_Rare_SpellCategory,
	Fire_Protection_Rare_SpellCategory,
	Fire_Resistance_Raising_Rare_SpellCategory,
	Fletching_Raising_Rare_SpellCategory,
	Focus_Raising_Rare_SpellCategory,
	Healing_Raising_Rare_SpellCategory,
	Health_Accelerating_Rare_SpellCategory,
	Item_Enchantment_Raising_Rare_SpellCategory,
	Jump_Raising_Rare_SpellCategory,
	Leadership_Raising_Rare_SpellCategory,
	Life_Magic_Raising_Rare_SpellCategory,
	Lockpick_Raising_Rare_SpellCategory,
	Loyalty_Raising_Rare_SpellCategory,
	Mace_Raising_Rare_SpellCategory,
	Magic_Defense_Raising_Rare_SpellCategory,
	Mana_Accelerating_Rare_SpellCategory,
	Mana_Conversion_Raising_Rare_SpellCategory,
	Melee_Defense_Raising_Rare_SpellCategory,
	Missile_Defense_Raising_Rare_SpellCategory,
	Pierce_Protection_Rare_SpellCategory,
	Pierce_Resistance_Raising_Rare_SpellCategory,
	Quickness_Raising_Rare_SpellCategory,
	Run_Raising_Rare_SpellCategory,
	Self_Raising_Rare_SpellCategory,
	Slash_Protection_Rare_SpellCategory,
	Slash_Resistance_Raising_Rare_SpellCategory,
	Spear_Raising_Rare_SpellCategory,
	Staff_Raising_Rare_SpellCategory,
	Stamina_Accelerating_Rare_SpellCategory,
	Strength_Raising_Rare_SpellCategory,
	Sword_Raising_Rare_SpellCategory,
	Thrown_Weapons_Raising_Rare_SpellCategory,
	Unarmed_Combat_Raising_Rare_SpellCategory,
	War_Magic_Raising_Rare_SpellCategory,
	Weapon_Time_Raising_Rare_SpellCategory,
	Armor_Increase_Inky_Armor_SpellCategory,
	Magic_Defense_Shelter_Raising_Fiun_SpellCategory,
	Extra_Run_Skill_Raising_Fiun_SpellCategory,
	Extra_Mana_Conversion_Skill_Raising_Fiun_SpellCategory,
	Attributes_Increase_Cantrip1_SpellCategory,
	Extra_Melee_Defense_Skill_Raising2_SpellCategory,
	ACTDPurchaseRewardSpell_SpellCategory,
	ACTDPurchaseRewardSpellHealth_SpellCategory,
	SaltAsh_Attack_Mod_Raising_SpellCategory,
	Quickness_Increase2_SpellCategory,
	Extra_Alchemy_Skill_Raising2_SpellCategory,
	Extra_Cooking_Skill_Raising2_SpellCategory,
	Extra_Fletching_Skill_Raising2_SpellCategory,
	Extra_Lockpick_Skill_Raising2_SpellCategory,
	MucorManaWell_SpellCategory,
	Stamina_Restoring2_SpellCategory,
	Allegiance_Raising_SpellCategory,
	Health_DoT_SpellCategory,
	Health_DoT_Secondary_SpellCategory,
	Health_DoT_Tertiary_SpellCategory,
	Health_HoT_SpellCategory,
	Health_HoT_Secondary_SpellCategory,
	Health_HoT_Tertiary_SpellCategory,
	Health_Divide_Secondary_SpellCategory,
	Health_Divide_Tertiary_SpellCategory,
	SetSword_Raising_SpellCategory,
	SetAxe_Raising_SpellCategory,
	SetDagger_Raising_SpellCategory,
	SetMace_Raising_SpellCategory,
	SetSpear_Raising_SpellCategory,
	SetStaff_Raising_SpellCategory,
	SetUnarmed_Raising_SpellCategory,
	SetBow_Raising_SpellCategory,
	SetCrossbow_Raising_SpellCategory,
	SetThrown_Raising_SpellCategory,
	SetItemEnchantment_Raising_SpellCategory,
	SetCreatureEnchantment_Raising_SpellCategory,
	SetWarMagic_Raising_SpellCategory,
	SetLifeMagic_Raising_SpellCategory,
	SetMeleeDefense_Raising_SpellCategory,
	SetMissileDefense_Raising_SpellCategory,
	SetMagicDefense_Raising_SpellCategory,
	SetStamina_Accelerating_SpellCategory,
	SetCooking_Raising_SpellCategory,
	SetFletching_Raising_SpellCategory,
	SetLockpick_Raising_SpellCategory,
	SetAlchemy_Raising_SpellCategory,
	SetSalvaging_Raising_SpellCategory,
	SetArmorExpertise_Raising_SpellCategory,
	SetWeaponExpertise_Raising_SpellCategory,
	SetItemTinkering_Raising_SpellCategory,
	SetMagicItemExpertise_Raising_SpellCategory,
	SetLoyalty_Raising_SpellCategory,
	SetStrength_Raising_SpellCategory,
	SetEndurance_Raising_SpellCategory,
	SetCoordination_Raising_SpellCategory,
	SetQuickness_Raising_SpellCategory,
	SetFocus_Raising_SpellCategory,
	SetWillpower_Raising_SpellCategory,
	SetHealth_Raising_SpellCategory,
	SetStamina_Raising_SpellCategory,
	SetMana_Raising_SpellCategory,
	SetSprint_Raising_SpellCategory,
	SetJumping_Raising_SpellCategory,
	SetSlashResistance_Raising_SpellCategory,
	SetBludgeonResistance_Raising_SpellCategory,
	SetPierceResistance_Raising_SpellCategory,
	SetFlameResistance_Raising_SpellCategory,
	SetAcidResistance_Raising_SpellCategory,
	SetFrostResistance_Raising_SpellCategory,
	SetLightningResistance_Raising_SpellCategory,
	Crafting_LockPick_Raising_SpellCategory,
	Crafting_Fletching_Raising_SpellCategory,
	Crafting_Cooking_Raising_SpellCategory,
	Crafting_Alchemy_Raising_SpellCategory,
	Crafting_ArmorTinkering_Raising_SpellCategory,
	Crafting_WeaponTinkering_Raising_SpellCategory,
	Crafting_MagicTinkering_Raising_SpellCategory,
	Crafting_ItemTinkering_Raising_SpellCategory,
	SkillPercent_Alchemy_Raising_SpellCategory,
	TwoHanded_Raising_SpellCategory,
	TwoHanded_Lowering_SpellCategory,
	Extra_TwoHanded_Skill_Raising_SpellCategory,
	Extra_TwoHanded_Skill_Lowering_SpellCategory,
	Extra_TwoHanded_Skill_Raising2_SpellCategory,
	TwoHanded_Raising_Rare_SpellCategory,
	SetTwoHanded_Raising_SpellCategory,
	GearCraft_Raising_SpellCategory,
	GearCraft_Lowering_SpellCategory,
	Extra_GearCraft_Skill_Raising_SpellCategory,
	Extra_GearCraft_Skill_Lowering_SpellCategory,
	Extra_GearCraft_Skill_Raising2_SpellCategory,
	GearCraft_Raising_Rare_SpellCategory,
	SetGearCraft_Raising_SpellCategory,
	LoyaltyMana_Raising_SpellCategory,
	LoyaltyStamina_Raising_SpellCategory,
	LeadershipHealth_Raising_SpellCategory,
	TrinketDamage_Raising_SpellCategory,
	TrinketDamage_Lowering_SpellCategory,
	TrinketHealth_Raising_SpellCategory,
	TrinketStamina_Raising_SpellCategory,
	TrinketMana_Raising_SpellCategory,
	TrinketXP_Raising_SpellCategory,
	DeceptionArcaneLore_Raising_SpellCategory,
	HealOverTime_Raising_SpellCategory,
	DamageOverTime_Raising_SpellCategory,
	HealingResistRating_Raising_SpellCategory,
	AetheriaDamageRating_Raising_SpellCategory,
	AetheriaDamageReduction_Raising_SpellCategory,
		SKIPPED_SpellCategory,
	AetheriaHealth_Raising_SpellCategory,
	AetheriaStamina_Raising_SpellCategory,
	AetheriaMana_Raising_SpellCategory,
	AetheriaCriticalDamage_Raising_SpellCategory,
	AetheriaHealingAmplification_Raising_SpellCategory,
	AetheriaProcDamageRating_Raising_SpellCategory,
	AetheriaProcDamageReduction_Raising_SpellCategory,
	AetheriaProcHealthOverTime_Raising_SpellCategory,
	AetheriaProcDamageOverTime_Raising_SpellCategory,
	AetheriaProcHealingReduction_Raising_SpellCategory,
	RareDamageRating_Raising_SpellCategory,
	RareDamageReductionRating_Raising_SpellCategory,
	AetheriaEndurance_Raising_SpellCategory,
	NetherDamageOverTime_Raising_SpellCategory,
	NetherDamageOverTime_Raising2_SpellCategory,
	NetherDamageOverTime_Raising3_SpellCategory,
	Nether_Streak_SpellCategory,
	Nether_Missile_SpellCategory,
	Nether_Ring_SpellCategory,
	NetherDamageRating_Lowering_SpellCategory,
	NetherDamageHealingReduction_Raising_SpellCategory,
	Void_Magic_Lowering_SpellCategory,
	Void_Magic_Raising_SpellCategory,
	Void_Mystic_Raising_SpellCategory,
	SetVoidMagic_Raising_SpellCategory,
	Void_Magic_Raising_Rare_SpellCategory,
	Void_Mystic_Raising2_SpellCategory,
	LuminanceDamageRating_Raising_SpellCategory,
	LuminanceDamageReduction_Raising_SpellCategory,
	LuminanceHealth_Raising_SpellCategory,
	AetheriaCriticalReduction_Raising_SpellCategory,
	Extra_Missile_Defense_Skill_Raising_SpellCategory,
	Extra_Missile_Defense_Skill_Lowering_SpellCategory,
	Extra_Missile_Defense_Skill_Raising2_SpellCategory,
	AetheriaHealthResistance_Raising_SpellCategory,
	AetheriaDotResistance_Raising_SpellCategory,
	Cloak_Skill_Raising_SpellCategory,
	Cloak_All_Skill_Raising_SpellCategory,
	Cloak_Magic_Defense_Lowering_SpellCategory,
	Cloak_Melee_Defense_Lowering_SpellCategory,
	Cloak_Missile_Defense_Lowering_SpellCategory,
	DirtyFighting_Lowering_SpellCategory,
	DirtyFighting_Raising_SpellCategory,
	Extra_DirtyFighting_Raising_SpellCategory,
	DualWield_Lowering_SpellCategory,
	DualWield_Raising_SpellCategory,
	Extra_DualWield_Raising_SpellCategory,
	Recklessness_Lowering_SpellCategory,
	Recklessness_Raising_SpellCategory,
	Extra_Recklessness_Raising_SpellCategory,
	Shield_Lowering_SpellCategory,
	Shield_Raising_SpellCategory,
	Extra_Shield_Raising_SpellCategory,
	SneakAttack_Lowering_SpellCategory,
	SneakAttack_Raising_SpellCategory,
	Extra_SneakAttack_Raising_SpellCategory,
	Rare_DirtyFighting_Raising_SpellCategory,
	Rare_DualWield_Raising_SpellCategory,
	Rare_Recklessness_Raising_SpellCategory,
	Rare_Shield_Raising_SpellCategory,
	Rare_SneakAttack_Raising_SpellCategory,
	DF_Attack_Skill_Debuff_SpellCategory,
	DF_Bleed_Damage_SpellCategory,
	DF_Defense_Skill_Debuff_SpellCategory,
	DF_Healing_Debuff_SpellCategory,
	SetDirtyFighting_Raising_SpellCategory,
	SetDualWield_Raising_SpellCategory,
	SetRecklessness_Raising_SpellCategory,
	SetShield_Raising_SpellCategory,
	SetSneakAttack_Raising_SpellCategory,
	LifeGiver_Mhoire_SpellCategory,
	RareDamageRating_Raising2_SpellCategory,
	Spell_Damage_Raising_SpellCategory,
	Summoning_Raising_SpellCategory,
	Summoning_Lowering_SpellCategory,
	Extra_Summoning_Skill_Raising_SpellCategory,
	SetSummoning_Raising_SpellCategory
};

enum PortalEnum
{
	UNDEF = 0,
	PLAYER_PASSABLE = 1,
	PK_BANNED = 2,
	PKLITE_BANNED = 4,
	PLAYER_NPK_ONLY = 7,
	NPK_BANNED = 8,
	PLAYER_PK_PKL_ONLY = 9,
	NOT_SUMMONABLE = 16,
	PLAYER_NOTSUMMONABLE = 17,
	PLAYER_PK_PKL_ONLY_NOTSUMMONABLE = 25,
	NOT_RECALLABLE_NOR_LINKABLE = 32,
	PLAYER_NOTRECALLABLE_NOTLINKABLE = 33,
	PLAYER_PK_PKL_ONLY_NOTRECALLABLE_NOTLINKABLE = 41,
	PLAYER_NOTRECALLABLE_NOTLINKABLE_NOTSUMMONABLE = 49,
	PLAYER_PK_PKL_ONLY_NOTSUMMONABLE_NOTRECALLABLE_NOTLINKABLE = 57,
	ONLY_OLTHOI_PCS = 64,
	NO_OLTHOI_PCS = 128,
	NO_VITAE = 256,
	NO_NEW_ACCOUNTS = 512
};

enum GeneratorTimeType
{
	Undef_GeneratorTimeType,
	RealTime_GeneratorTimeType,
	Defined_GeneratorTimeType,
	Event_GeneratorTimeType,
	Night_GeneratorTimeType,
	Day_GeneratorTimeType
};

enum GeneratorDefinedTimes
{
	Undef_GeneratorDefinedTimes,
	Dusk_GeneratorDefinedTimes,
	Dawn_GeneratorDefinedTimes
};

enum GeneratorDestruct
{
	Undef_GeneratorDestruct,
	Nothing_GeneratorDestruct,
	Destroy_GeneratorDestruct,
	Kill_GeneratorDestruct
};

enum GeneratorType
{
	Undef_GeneratorType,
	Relative_GeneratorType,
	Absolute_GeneratorType
};


#include "GameStatEnums.h"
