
#pragma once

// Enums courtesy of acclient PDB and aclogviewer

enum GenericQualitiesPackHeader {
	Packed_None = 0,
	Packed_IntStats = (1 << 0),
	Packed_BoolStats = (1 << 1),
	Packed_FloatStats = (1 << 2),
	Packed_StringStats = (1 << 3)
};

enum StatType {
	Undef_StatType,
	Int_StatType,
	Float_StatType,
	Position_StatType,
	Skill_StatType,
	String_StatType,
	DataID_StatType,
	InstanceID_StatType,
	Attribute_StatType,
	Attribute_2nd_StatType,
	BodyDamageValue_StatType,
	BodyDamageVariance_StatType,
	BodyArmorValue_StatType,
	Bool_StatType,
	Int64_StatType,
	Num_StatTypes,
	DID_StatType = DataID_StatType,
	IID_StatType = InstanceID_StatType
};

enum STypeAttribute {
	UNDEF_ATTRIBUTE,
	STRENGTH_ATTRIBUTE,
	ENDURANCE_ATTRIBUTE,
	QUICKNESS_ATTRIBUTE,
	COORDINATION_ATTRIBUTE,
	FOCUS_ATTRIBUTE,
	SELF_ATTRIBUTE
};

enum STypeAttribute2nd {
	UNDEF_ATTRIBUTE_2ND,
	MAX_HEALTH_ATTRIBUTE_2ND,
	HEALTH_ATTRIBUTE_2ND,
	MAX_STAMINA_ATTRIBUTE_2ND,
	STAMINA_ATTRIBUTE_2ND,
	MAX_MANA_ATTRIBUTE_2ND,
	MANA_ATTRIBUTE_2ND
};

enum STypeBool {
	UNDEF_BOOL,
	STUCK_BOOL,
	OPEN_BOOL,
	LOCKED_BOOL,
	ROT_PROOF_BOOL,
	ALLEGIANCE_UPDATE_REQUEST_BOOL,
	AI_USES_MANA_BOOL,
	AI_USE_HUMAN_MAGIC_ANIMATIONS_BOOL,
	ALLOW_GIVE_BOOL,
	CURRENTLY_ATTACKING_BOOL,
	ATTACKER_AI_BOOL,
	IGNORE_COLLISIONS_BOOL,
	REPORT_COLLISIONS_BOOL,
	ETHEREAL_BOOL,
	GRAVITY_STATUS_BOOL,
	LIGHTS_STATUS_BOOL,
	SCRIPTED_COLLISION_BOOL,
	INELASTIC_BOOL,
	VISIBILITY_BOOL,
	ATTACKABLE_BOOL,
	SAFE_SPELL_COMPONENTS_BOOL,
	ADVOCATE_STATE_BOOL,
	INSCRIBABLE_BOOL,
	DESTROY_ON_SELL_BOOL,
	UI_HIDDEN_BOOL,
	IGNORE_HOUSE_BARRIERS_BOOL,
	HIDDEN_ADMIN_BOOL,
	PK_WOUNDER_BOOL,
	PK_KILLER_BOOL,
	NO_CORPSE_BOOL,
	UNDER_LIFESTONE_PROTECTION_BOOL,
	ITEM_MANA_UPDATE_PENDING_BOOL,
	GENERATOR_STATUS_BOOL,
	RESET_MESSAGE_PENDING_BOOL,
	DEFAULT_OPEN_BOOL,
	DEFAULT_LOCKED_BOOL,
	DEFAULT_ON_BOOL,
	OPEN_FOR_BUSINESS_BOOL,
	IS_FROZEN_BOOL,
	DEAL_MAGICAL_ITEMS_BOOL,
	LOGOFF_IM_DEAD_BOOL,
	REPORT_COLLISIONS_AS_ENVIRONMENT_BOOL,
	ALLOW_EDGE_SLIDE_BOOL,
	ADVOCATE_QUEST_BOOL,
	IS_ADMIN_BOOL,
	IS_ARCH_BOOL,
	IS_SENTINEL_BOOL,
	IS_ADVOCATE_BOOL,
	CURRENTLY_POWERING_UP_BOOL,
	GENERATOR_ENTERED_WORLD_BOOL,
	NEVER_FAIL_CASTING_BOOL,
	VENDOR_SERVICE_BOOL,
	AI_IMMOBILE_BOOL,
	DAMAGED_BY_COLLISIONS_BOOL,
	IS_DYNAMIC_BOOL,
	IS_HOT_BOOL,
	IS_AFFECTING_BOOL,
	AFFECTS_AIS_BOOL,
	SPELL_QUEUE_ACTIVE_BOOL,
	GENERATOR_DISABLED_BOOL,
	IS_ACCEPTING_TELLS_BOOL,
	LOGGING_CHANNEL_BOOL,
	OPENS_ANY_LOCK_BOOL,
	UNLIMITED_USE_BOOL,
	GENERATED_TREASURE_ITEM_BOOL,
	IGNORE_MAGIC_RESIST_BOOL,
	IGNORE_MAGIC_ARMOR_BOOL,
	AI_ALLOW_TRADE_BOOL,
	SPELL_COMPONENTS_REQUIRED_BOOL,
	IS_SELLABLE_BOOL,
	IGNORE_SHIELDS_BY_SKILL_BOOL,
	NODRAW_BOOL,
	ACTIVATION_UNTARGETED_BOOL,
	HOUSE_HAS_GOTTEN_PRIORITY_BOOT_POS_BOOL,
	GENERATOR_AUTOMATIC_DESTRUCTION_BOOL,
	HOUSE_HOOKS_VISIBLE_BOOL,
	HOUSE_REQUIRES_MONARCH_BOOL,
	HOUSE_HOOKS_ENABLED_BOOL,
	HOUSE_NOTIFIED_HUD_OF_HOOK_COUNT_BOOL,
	AI_ACCEPT_EVERYTHING_BOOL,
	IGNORE_PORTAL_RESTRICTIONS_BOOL,
	REQUIRES_BACKPACK_SLOT_BOOL,
	DONT_TURN_OR_MOVE_WHEN_GIVING_BOOL,
	NPC_LOOKS_LIKE_OBJECT_BOOL,
	IGNORE_CLO_ICONS_BOOL,
	APPRAISAL_HAS_ALLOWED_WIELDER_BOOL,
	CHEST_REGEN_ON_CLOSE_BOOL,
	LOGOFF_IN_MINIGAME_BOOL,
	PORTAL_SHOW_DESTINATION_BOOL,
	PORTAL_IGNORES_PK_ATTACK_TIMER_BOOL,
	NPC_INTERACTS_SILENTLY_BOOL,
	RETAINED_BOOL,
	IGNORE_AUTHOR_BOOL,
	LIMBO_BOOL,
	APPRAISAL_HAS_ALLOWED_ACTIVATOR_BOOL,
	EXISTED_BEFORE_ALLEGIANCE_XP_CHANGES_BOOL,
	IS_DEAF_BOOL,
	IS_PSR_BOOL,
	INVINCIBLE_BOOL,
	IVORYABLE_BOOL,
	DYABLE_BOOL,
	CAN_GENERATE_RARE_BOOL,
	CORPSE_GENERATED_RARE_BOOL,
	NON_PROJECTILE_MAGIC_IMMUNE_BOOL,
	ACTD_RECEIVED_ITEMS_BOOL,
	EXECUTING_EMOTE, // NOTE: Missing 1 retasked in an attempt to deal with vendor and NPC issue
	FIRST_ENTER_WORLD_DONE_BOOL,
	RECALLS_DISABLED_BOOL,
	RARE_USES_TIMER_BOOL,
	ACTD_PREORDER_RECEIVED_ITEMS_BOOL,
	AFK_BOOL,
	IS_GAGGED_BOOL,
	PROC_SPELL_SELF_TARGETED_BOOL,
	IS_ALLEGIANCE_GAGGED_BOOL,
	EQUIPMENT_SET_TRIGGER_PIECE_BOOL,
	UNINSCRIBE_BOOL,
	WIELD_ON_USE_BOOL,
	CHEST_CLEARED_WHEN_CLOSED_BOOL,
	NEVER_ATTACK_BOOL,
	SUPPRESS_GENERATE_EFFECT_BOOL,
	TREASURE_CORPSE_BOOL,
	EQUIPMENT_SET_ADD_LEVEL_BOOL,
	BARBER_ACTIVE_BOOL,
	TOP_LAYER_PRIORITY_BOOL,
	NO_HELD_ITEM_SHOWN_BOOL,
	LOGIN_AT_LIFESTONE_BOOL,
	OLTHOI_PK_BOOL,
	ACCOUNT_15_DAYS_BOOL,
	HAD_NO_VITAE_BOOL,
	NO_OLTHOI_TALK_BOOL,
	AUTOWIELD_LEFT_BOOL,
	MERGE_LOCKED,
	NUM_BOOL_STAT_VALUES
};

enum STypeDID {
	UNDEF_DID,
	SETUP_DID,
	MOTION_TABLE_DID,
	SOUND_TABLE_DID,
	COMBAT_TABLE_DID,
	QUALITY_FILTER_DID,
	PALETTE_BASE_DID,
	CLOTHINGBASE_DID,
	ICON_DID,
	EYES_TEXTURE_DID,
	NOSE_TEXTURE_DID,
	MOUTH_TEXTURE_DID,
	DEFAULT_EYES_TEXTURE_DID,
	DEFAULT_NOSE_TEXTURE_DID,
	DEFAULT_MOUTH_TEXTURE_DID,
	HAIR_PALETTE_DID,
	EYES_PALETTE_DID,
	SKIN_PALETTE_DID,
	HEAD_OBJECT_DID,
	ACTIVATION_ANIMATION_DID,
	INIT_MOTION_DID,
	ACTIVATION_SOUND_DID,
	PHYSICS_EFFECT_TABLE_DID,
	USE_SOUND_DID,
	USE_TARGET_ANIMATION_DID,
	USE_TARGET_SUCCESS_ANIMATION_DID,
	USE_TARGET_FAILURE_ANIMATION_DID,
	USE_USER_ANIMATION_DID,
	SPELL_DID,
	SPELL_COMPONENT_DID,
	PHYSICS_SCRIPT_DID,
	LINKED_PORTAL_ONE_DID,
	WIELDED_TREASURE_TYPE_DID,
	INVENTORY_TREASURE_TYPE_DID,
	SHOP_TREASURE_TYPE_DID,
	DEATH_TREASURE_TYPE_DID,
	MUTATE_FILTER_DID,
	ITEM_SKILL_LIMIT_DID,
	USE_CREATE_ITEM_DID,
	DEATH_SPELL_DID,
	VENDORS_CLASSID_DID,
	ITEM_SPECIALIZED_ONLY_DID,
	HOUSEID_DID,
	ACCOUNT_HOUSEID_DID,
	RESTRICTION_EFFECT_DID,
	CREATION_MUTATION_FILTER_DID,
	TSYS_MUTATION_FILTER_DID,
	LAST_PORTAL_DID,
	LINKED_PORTAL_TWO_DID,
	ORIGINAL_PORTAL_DID,
	ICON_OVERLAY_DID,
	ICON_OVERLAY_SECONDARY_DID,
	ICON_UNDERLAY_DID,
	AUGMENTATION_MUTATION_FILTER_DID,
	AUGMENTATION_EFFECT_DID,
	PROC_SPELL_DID,
	AUGMENTATION_CREATE_ITEM_DID,
	ALTERNATE_CURRENCY_DID,
	BLUE_SURGE_SPELL_DID,
	YELLOW_SURGE_SPELL_DID,
	RED_SURGE_SPELL_DID,
	OLTHOI_DEATH_TREASURE_TYPE_DID,

	NUM_DID_STAT_VALUES
};

enum STypeFloat {
	UNDEF_FLOAT,
	HEARTBEAT_INTERVAL_FLOAT,
	HEARTBEAT_TIMESTAMP_FLOAT,
	HEALTH_RATE_FLOAT,
	STAMINA_RATE_FLOAT,
	MANA_RATE_FLOAT,
	HEALTH_UPON_RESURRECTION_FLOAT,
	STAMINA_UPON_RESURRECTION_FLOAT,
	MANA_UPON_RESURRECTION_FLOAT,
	START_TIME_FLOAT,
	STOP_TIME_FLOAT,
	RESET_INTERVAL_FLOAT,
	SHADE_FLOAT,
	ARMOR_MOD_VS_SLASH_FLOAT,
	ARMOR_MOD_VS_PIERCE_FLOAT,
	ARMOR_MOD_VS_BLUDGEON_FLOAT,
	ARMOR_MOD_VS_COLD_FLOAT,
	ARMOR_MOD_VS_FIRE_FLOAT,
	ARMOR_MOD_VS_ACID_FLOAT,
	ARMOR_MOD_VS_ELECTRIC_FLOAT,
	COMBAT_SPEED_FLOAT,
	WEAPON_LENGTH_FLOAT,
	DAMAGE_VARIANCE_FLOAT,
	CURRENT_POWER_MOD_FLOAT,
	ACCURACY_MOD_FLOAT,
	STRENGTH_MOD_FLOAT,
	MAXIMUM_VELOCITY_FLOAT,
	ROTATION_SPEED_FLOAT,
	MOTION_TIMESTAMP_FLOAT,
	WEAPON_DEFENSE_FLOAT,
	WIMPY_LEVEL_FLOAT,
	VISUAL_AWARENESS_RANGE_FLOAT,
	AURAL_AWARENESS_RANGE_FLOAT,
	PERCEPTION_LEVEL_FLOAT,
	POWERUP_TIME_FLOAT,
	MAX_CHARGE_DISTANCE_FLOAT,
	CHARGE_SPEED_FLOAT,
	BUY_PRICE_FLOAT,
	SELL_PRICE_FLOAT,
	DEFAULT_SCALE_FLOAT,
	LOCKPICK_MOD_FLOAT,
	REGENERATION_INTERVAL_FLOAT,
	REGENERATION_TIMESTAMP_FLOAT,
	GENERATOR_RADIUS_FLOAT,
	TIME_TO_ROT_FLOAT,
	DEATH_TIMESTAMP_FLOAT,
	PK_TIMESTAMP_FLOAT,
	VICTIM_TIMESTAMP_FLOAT,
	LOGIN_TIMESTAMP_FLOAT,
	CREATION_TIMESTAMP_FLOAT,
	MINIMUM_TIME_SINCE_PK_FLOAT,
	DEPRECATED_HOUSEKEEPING_PRIORITY_FLOAT,
	ABUSE_LOGGING_TIMESTAMP_FLOAT,
	LAST_PORTAL_TELEPORT_TIMESTAMP_FLOAT,
	USE_RADIUS_FLOAT,
	HOME_RADIUS_FLOAT,
	RELEASED_TIMESTAMP_FLOAT,
	MIN_HOME_RADIUS_FLOAT,
	FACING_FLOAT,
	RESET_TIMESTAMP_FLOAT,
	LOGOFF_TIMESTAMP_FLOAT,
	ECON_RECOVERY_INTERVAL_FLOAT,
	WEAPON_OFFENSE_FLOAT,
	DAMAGE_MOD_FLOAT,
	RESIST_SLASH_FLOAT,
	RESIST_PIERCE_FLOAT,
	RESIST_BLUDGEON_FLOAT,
	RESIST_FIRE_FLOAT,
	RESIST_COLD_FLOAT,
	RESIST_ACID_FLOAT,
	RESIST_ELECTRIC_FLOAT,
	RESIST_HEALTH_BOOST_FLOAT,
	RESIST_STAMINA_DRAIN_FLOAT,
	RESIST_STAMINA_BOOST_FLOAT,
	RESIST_MANA_DRAIN_FLOAT,
	RESIST_MANA_BOOST_FLOAT,
	TRANSLUCENCY_FLOAT,
	PHYSICS_SCRIPT_INTENSITY_FLOAT,
	FRICTION_FLOAT,
	ELASTICITY_FLOAT,
	AI_USE_MAGIC_DELAY_FLOAT,
	ITEM_MIN_SPELLCRAFT_MOD_FLOAT,
	ITEM_MAX_SPELLCRAFT_MOD_FLOAT,
	ITEM_RANK_PROBABILITY_FLOAT,
	SHADE2_FLOAT,
	SHADE3_FLOAT,
	SHADE4_FLOAT,
	ITEM_EFFICIENCY_FLOAT,
	ITEM_MANA_UPDATE_TIMESTAMP_FLOAT,
	SPELL_GESTURE_SPEED_MOD_FLOAT,
	SPELL_STANCE_SPEED_MOD_FLOAT,
	ALLEGIANCE_APPRAISAL_TIMESTAMP_FLOAT,
	POWER_LEVEL_FLOAT,
	ACCURACY_LEVEL_FLOAT,
	ATTACK_ANGLE_FLOAT,
	ATTACK_TIMESTAMP_FLOAT,
	CHECKPOINT_TIMESTAMP_FLOAT,
	SOLD_TIMESTAMP_FLOAT,
	USE_TIMESTAMP_FLOAT,
	USE_LOCK_TIMESTAMP_FLOAT,
	HEALKIT_MOD_FLOAT,
	FROZEN_TIMESTAMP_FLOAT,
	HEALTH_RATE_MOD_FLOAT,
	ALLEGIANCE_SWEAR_TIMESTAMP_FLOAT,
	OBVIOUS_RADAR_RANGE_FLOAT,
	HOTSPOT_CYCLE_TIME_FLOAT,
	HOTSPOT_CYCLE_TIME_VARIANCE_FLOAT,
	SPAM_TIMESTAMP_FLOAT,
	SPAM_RATE_FLOAT,
	BOND_WIELDED_TREASURE_FLOAT,
	BULK_MOD_FLOAT,
	SIZE_MOD_FLOAT,
	GAG_TIMESTAMP_FLOAT,
	GENERATOR_UPDATE_TIMESTAMP_FLOAT,
	DEATH_SPAM_TIMESTAMP_FLOAT,
	DEATH_SPAM_RATE_FLOAT,
	WILD_ATTACK_PROBABILITY_FLOAT,
	FOCUSED_PROBABILITY_FLOAT,
	CRASH_AND_TURN_PROBABILITY_FLOAT,
	CRASH_AND_TURN_RADIUS_FLOAT,
	CRASH_AND_TURN_BIAS_FLOAT,
	GENERATOR_INITIAL_DELAY_FLOAT,
	AI_ACQUIRE_HEALTH_FLOAT,
	AI_ACQUIRE_STAMINA_FLOAT,
	AI_ACQUIRE_MANA_FLOAT,
	RESIST_HEALTH_DRAIN_FLOAT,
	LIFESTONE_PROTECTION_TIMESTAMP_FLOAT,
	AI_COUNTERACT_ENCHANTMENT_FLOAT,
	AI_DISPEL_ENCHANTMENT_FLOAT,
	TRADE_TIMESTAMP_FLOAT,
	AI_TARGETED_DETECTION_RADIUS_FLOAT,
	EMOTE_PRIORITY_FLOAT,
	LAST_TELEPORT_START_TIMESTAMP_FLOAT,
	EVENT_SPAM_TIMESTAMP_FLOAT,
	EVENT_SPAM_RATE_FLOAT,
	INVENTORY_OFFSET_FLOAT,
	CRITICAL_MULTIPLIER_FLOAT,
	MANA_STONE_DESTROY_CHANCE_FLOAT,
	SLAYER_DAMAGE_BONUS_FLOAT,
	ALLEGIANCE_INFO_SPAM_TIMESTAMP_FLOAT,
	ALLEGIANCE_INFO_SPAM_RATE_FLOAT,
	NEXT_SPELLCAST_TIMESTAMP_FLOAT,
	APPRAISAL_REQUESTED_TIMESTAMP_FLOAT,
	APPRAISAL_HEARTBEAT_DUE_TIMESTAMP_FLOAT,
	MANA_CONVERSION_MOD_FLOAT,
	LAST_PK_ATTACK_TIMESTAMP_FLOAT,
	FELLOWSHIP_UPDATE_TIMESTAMP_FLOAT,
	CRITICAL_FREQUENCY_FLOAT,
	LIMBO_START_TIMESTAMP_FLOAT,
	WEAPON_MISSILE_DEFENSE_FLOAT,
	WEAPON_MAGIC_DEFENSE_FLOAT,
	IGNORE_SHIELD_FLOAT,
	ELEMENTAL_DAMAGE_MOD_FLOAT,
	START_MISSILE_ATTACK_TIMESTAMP_FLOAT,
	LAST_RARE_USED_TIMESTAMP_FLOAT,
	IGNORE_ARMOR_FLOAT,
	PROC_SPELL_RATE_FLOAT,
	RESISTANCE_MODIFIER_FLOAT,
	ALLEGIANCE_GAG_TIMESTAMP_FLOAT,
	ABSORB_MAGIC_DAMAGE_FLOAT,
	CACHED_MAX_ABSORB_MAGIC_DAMAGE_FLOAT,
	GAG_DURATION_FLOAT,
	ALLEGIANCE_GAG_DURATION_FLOAT,
	GLOBAL_XP_MOD_FLOAT,
	HEALING_MODIFIER_FLOAT,
	ARMOR_MOD_VS_NETHER_FLOAT,
	RESIST_NETHER_FLOAT,
	COOLDOWN_DURATION_FLOAT,
	WEAPON_AURA_OFFENSE_FLOAT,
	WEAPON_AURA_DEFENSE_FLOAT,
	WEAPON_AURA_ELEMENTAL_FLOAT,
	WEAPON_AURA_MANA_CONV_FLOAT,

	NUM_FLOAT_STAT_VALUES
};

enum STypeInt {
	UNDEF_INT,
	ITEM_TYPE_INT,
	CREATURE_TYPE_INT,
	PALETTE_TEMPLATE_INT,
	CLOTHING_PRIORITY_INT,
	ENCUMB_VAL_INT,
	ITEMS_CAPACITY_INT,
	CONTAINERS_CAPACITY_INT,
	MASS_INT,
	LOCATIONS_INT,
	CURRENT_WIELDED_LOCATION_INT,
	MAX_STACK_SIZE_INT,
	STACK_SIZE_INT,
	STACK_UNIT_ENCUMB_INT,
	STACK_UNIT_MASS_INT,
	STACK_UNIT_VALUE_INT,
	ITEM_USEABLE_INT,
	RARE_ID_INT,
	UI_EFFECTS_INT,
	VALUE_INT,
	COIN_VALUE_INT,
	TOTAL_EXPERIENCE_INT,
	AVAILABLE_CHARACTER_INT,
	TOTAL_SKILL_CREDITS_INT,
	AVAILABLE_SKILL_CREDITS_INT,
	LEVEL_INT,
	ACCOUNT_REQUIREMENTS_INT,
	ARMOR_TYPE_INT,
	ARMOR_LEVEL_INT,
	ALLEGIANCE_CP_POOL_INT,
	ALLEGIANCE_RANK_INT,
	CHANNELS_ALLOWED_INT,
	CHANNELS_ACTIVE_INT,
	BONDED_INT,
	MONARCHS_RANK_INT,
	ALLEGIANCE_FOLLOWERS_INT,
	RESIST_MAGIC_INT,
	RESIST_ITEM_APPRAISAL_INT,
	RESIST_LOCKPICK_INT,
	DEPRECATED_RESIST_REPAIR_INT,
	COMBAT_MODE_INT,
	CURRENT_ATTACK_HEIGHT_INT,
	COMBAT_COLLISIONS_INT,
	NUM_DEATHS_INT,
	DAMAGE_INT,
	DAMAGE_TYPE_INT,
	DEFAULT_COMBAT_STYLE_INT,
	ATTACK_TYPE_INT,
	WEAPON_SKILL_INT,
	WEAPON_TIME_INT,
	AMMO_TYPE_INT,
	COMBAT_USE_INT,
	PARENT_LOCATION_INT,
	PLACEMENT_POSITION_INT,
	WEAPON_ENCUMBRANCE_INT,
	WEAPON_MASS_INT,
	SHIELD_VALUE_INT,
	SHIELD_ENCUMBRANCE_INT,
	MISSILE_INVENTORY_LOCATION_INT,
	FULL_DAMAGE_TYPE_INT,
	WEAPON_RANGE_INT,
	ATTACKERS_SKILL_INT,
	DEFENDERS_SKILL_INT,
	ATTACKERS_SKILL_VALUE_INT,
	ATTACKERS_CLASS_INT,
	PLACEMENT_INT,
	CHECKPOINT_STATUS_INT,
	TOLERANCE_INT,
	TARGETING_TACTIC_INT,
	COMBAT_TACTIC_INT,
	HOMESICK_TARGETING_TACTIC_INT,
	NUM_FOLLOW_FAILURES_INT,
	FRIEND_TYPE_INT,
	FOE_TYPE_INT,
	MERCHANDISE_ITEM_TYPES_INT,
	MERCHANDISE_MIN_VALUE_INT,
	MERCHANDISE_MAX_VALUE_INT,
	NUM_ITEMS_SOLD_INT,
	NUM_ITEMS_BOUGHT_INT,
	MONEY_INCOME_INT,
	MONEY_OUTFLOW_INT,
	MAX_GENERATED_OBJECTS_INT,
	INIT_GENERATED_OBJECTS_INT,
	ACTIVATION_RESPONSE_INT,
	ORIGINAL_VALUE_INT,
	NUM_MOVE_FAILURES_INT,
	MIN_LEVEL_INT,
	MAX_LEVEL_INT,
	LOCKPICK_MOD_INT,
	BOOSTER_ENUM_INT,
	BOOST_VALUE_INT,
	MAX_STRUCTURE_INT,
	STRUCTURE_INT,
	PHYSICS_STATE_INT,
	TARGET_TYPE_INT,
	RADARBLIP_COLOR_INT,
	ENCUMB_CAPACITY_INT,
	LOGIN_TIMESTAMP_INT,
	CREATION_TIMESTAMP_INT,
	PK_LEVEL_MODIFIER_INT,
	GENERATOR_TYPE_INT,
	AI_ALLOWED_COMBAT_STYLE_INT,
	LOGOFF_TIMESTAMP_INT,
	GENERATOR_DESTRUCTION_TYPE_INT,
	ACTIVATION_CREATE_CLASS_INT,
	ITEM_WORKMANSHIP_INT,
	ITEM_SPELLCRAFT_INT,
	ITEM_CUR_MANA_INT,
	ITEM_MAX_MANA_INT,
	ITEM_DIFFICULTY_INT,
	ITEM_ALLEGIANCE_RANK_LIMIT_INT,
	PORTAL_BITMASK_INT,
	ADVOCATE_LEVEL_INT,
	GENDER_INT,
	ATTUNED_INT,
	ITEM_SKILL_LEVEL_LIMIT_INT,
	GATE_LOGIC_INT,
	ITEM_MANA_COST_INT,
	LOGOFF_INT,
	ACTIVE_INT,
	ATTACK_HEIGHT_INT,
	NUM_ATTACK_FAILURES_INT,
	AI_CP_THRESHOLD_INT,
	AI_ADVANCEMENT_STRATEGY_INT,
	VERSION_INT,
	AGE_INT,
	VENDOR_HAPPY_MEAN_INT,
	VENDOR_HAPPY_VARIANCE_INT,
	CLOAK_STATUS_INT,
	VITAE_CP_POOL_INT,
	NUM_SERVICES_SOLD_INT,
	MATERIAL_TYPE_INT,
	NUM_ALLEGIANCE_BREAKS_INT,
	SHOWABLE_ON_RADAR_INT,
	PLAYER_KILLER_STATUS_INT,
	VENDOR_HAPPY_MAX_ITEMS_INT,
	SCORE_PAGE_NUM_INT,
	SCORE_CONFIG_NUM_INT,
	SCORE_NUM_SCORES_INT,
	DEATH_LEVEL_INT,
	AI_OPTIONS_INT,
	OPEN_TO_EVERYONE_INT,
	GENERATOR_TIME_TYPE_INT,
	GENERATOR_START_TIME_INT,
	GENERATOR_END_TIME_INT,
	GENERATOR_END_DESTRUCTION_TYPE_INT,
	XP_OVERRIDE_INT,
	NUM_CRASH_AND_TURNS_INT,
	COMPONENT_WARNING_THRESHOLD_INT,
	HOUSE_STATUS_INT,
	HOOK_PLACEMENT_INT,
	HOOK_TYPE_INT,
	HOOK_ITEM_TYPE_INT,
	AI_PP_THRESHOLD_INT,
	GENERATOR_VERSION_INT,
	HOUSE_TYPE_INT,
	PICKUP_EMOTE_OFFSET_INT,
	WEENIE_ITERATION_INT,
	WIELD_REQUIREMENTS_INT,
	WIELD_SKILLTYPE_INT,
	WIELD_DIFFICULTY_INT,
	HOUSE_MAX_HOOKS_USABLE_INT,
	HOUSE_CURRENT_HOOKS_USABLE_INT,
	ALLEGIANCE_MIN_LEVEL_INT,
	ALLEGIANCE_MAX_LEVEL_INT,
	HOUSE_RELINK_HOOK_COUNT_INT,
	SLAYER_CREATURE_TYPE_INT,
	CONFIRMATION_IN_PROGRESS_INT,
	CONFIRMATION_TYPE_IN_PROGRESS_INT,
	TSYS_MUTATION_DATA_INT,
	NUM_ITEMS_IN_MATERIAL_INT,
	NUM_TIMES_TINKERED_INT,
	APPRAISAL_LONG_DESC_DECORATION_INT,
	APPRAISAL_LOCKPICK_SUCCESS_PERCENT_INT,
	APPRAISAL_PAGES_INT,
	APPRAISAL_MAX_PAGES_INT,
	APPRAISAL_ITEM_SKILL_INT,
	GEM_COUNT_INT,
	GEM_TYPE_INT,
	IMBUED_EFFECT_INT,
	ATTACKERS_RAW_SKILL_VALUE_INT,
	CHESS_RANK_INT,
	CHESS_TOTALGAMES_INT,
	CHESS_GAMESWON_INT,
	CHESS_GAMESLOST_INT,
	TYPE_OF_ALTERATION_INT,
	SKILL_TO_BE_ALTERED_INT,
	SKILL_ALTERATION_COUNT_INT,
	HERITAGE_GROUP_INT,
	TRANSFER_FROM_ATTRIBUTE_INT,
	TRANSFER_TO_ATTRIBUTE_INT,
	ATTRIBUTE_TRANSFER_COUNT_INT,
	FAKE_FISHING_SKILL_INT,
	NUM_KEYS_INT,
	DEATH_TIMESTAMP_INT,
	PK_TIMESTAMP_INT,
	VICTIM_TIMESTAMP_INT,
	HOOK_GROUP_INT,
	ALLEGIANCE_SWEAR_TIMESTAMP_INT,
	HOUSE_PURCHASE_TIMESTAMP_INT,
	REDIRECTABLE_EQUIPPED_ARMOR_COUNT_INT,
	MELEEDEFENSE_IMBUEDEFFECTTYPE_CACHE_INT,
	MISSILEDEFENSE_IMBUEDEFFECTTYPE_CACHE_INT,
	MAGICDEFENSE_IMBUEDEFFECTTYPE_CACHE_INT,
	ELEMENTAL_DAMAGE_BONUS_INT,
	IMBUE_ATTEMPTS_INT,
	IMBUE_SUCCESSES_INT,
	CREATURE_KILLS_INT,
	PLAYER_KILLS_PK_INT,
	PLAYER_KILLS_PKL_INT,
	RARES_TIER_ONE_INT,
	RARES_TIER_TWO_INT,
	RARES_TIER_THREE_INT,
	RARES_TIER_FOUR_INT,
	RARES_TIER_FIVE_INT,
	AUGMENTATION_STAT_INT,								// ???
	AUGMENTATION_FAMILY_STAT_INT,						// ???
	AUGMENTATION_INNATE_FAMILY_INT,						// Count of other INNATE max 10
	AUGMENTATION_INNATE_STRENGTH_INT,					// Reinforcement of the Lugians
	AUGMENTATION_INNATE_ENDURANCE_INT,					// Bleeargh's Fortitude
	AUGMENTATION_INNATE_COORDINATION_INT,				// Oswald's Enhancement	
	AUGMENTATION_INNATE_QUICKNESS_INT,					// Siraluun's Blessing
	AUGMENTATION_INNATE_FOCUS_INT,						// Enduring Calm
	AUGMENTATION_INNATE_SELF_INT,						// Steadfast Will
	AUGMENTATION_SPECIALIZE_SALVAGING_INT,				// Ciandra's Essence
	AUGMENTATION_SPECIALIZE_ITEM_TINKERING_INT,			// Yoshi's Essence
	AUGMENTATION_SPECIALIZE_ARMOR_TINKERING_INT,		// Jibril's Essence
	AUGMENTATION_SPECIALIZE_MAGIC_ITEM_TINKERING_INT,	// Celdiseth's Essence
	AUGMENTATION_SPECIALIZE_WEAPON_TINKERING_INT,		// Koga's Essence
	AUGMENTATION_EXTRA_PACK_SLOT_INT,					// Shadow of the Seventh Mule
	AUGMENTATION_INCREASED_CARRYING_CAPACITY_INT,		// Might of the Seventh Mule - 5x
	AUGMENTATION_LESS_DEATH_ITEM_LOSS_INT,				// Clutch of the Miser - 5x
	AUGMENTATION_SPELLS_REMAIN_PAST_DEATH_INT,			// Enduring Enchantment
	AUGMENTATION_CRITICAL_DEFENSE_INT,					// Critical Protection
	AUGMENTATION_BONUS_XP_INT,							// Quick Learner
	AUGMENTATION_BONUS_SALVAGE_INT,						// Ciandra's Fortune - 4x
	AUGMENTATION_BONUS_IMBUE_CHANCE_INT,				// Charmed Smith
	AUGMENTATION_FASTER_REGEN_INT,						// Innate Renewal - 2x
	AUGMENTATION_INCREASED_SPELL_DURATION_INT,			// Archmage's Endurance - 5x
	AUGMENTATION_RESISTANCE_FAMILY_INT,					// Count of other RESISTANCT max 2
	AUGMENTATION_RESISTANCE_SLASH_INT,					// Enhancement of the Blade Turner
	AUGMENTATION_RESISTANCE_PIERCE_INT,					// Enhancement of the Arrow Turner
	AUGMENTATION_RESISTANCE_BLUNT_INT,					// Enhancement of the Mace Turner
	AUGMENTATION_RESISTANCE_ACID_INT,					// Caustic Enhancement
	AUGMENTATION_RESISTANCE_FIRE_INT,					// Fiery Enhancement
	AUGMENTATION_RESISTANCE_FROST_INT,					// Icy Enhancement
	AUGMENTATION_RESISTANCE_LIGHTNING_INT,				// Storm's Enhancement	
	RARES_TIER_ONE_LOGIN_INT,
	RARES_TIER_TWO_LOGIN_INT,
	RARES_TIER_THREE_LOGIN_INT,
	RARES_TIER_FOUR_LOGIN_INT,
	RARES_TIER_FIVE_LOGIN_INT,
	RARES_LOGIN_TIMESTAMP_INT,
	RARES_TIER_SIX_INT,
	RARES_TIER_SEVEN_INT,
	RARES_TIER_SIX_LOGIN_INT,
	RARES_TIER_SEVEN_LOGIN_INT,
	ITEM_ATTRIBUTE_LIMIT_INT,
	ITEM_ATTRIBUTE_LEVEL_LIMIT_INT,
	ITEM_ATTRIBUTE_2ND_LIMIT_INT,
	ITEM_ATTRIBUTE_2ND_LEVEL_LIMIT_INT,
	CHARACTER_TITLE_ID_INT,
	NUM_CHARACTER_TITLES_INT,
	RESISTANCE_MODIFIER_TYPE_INT,
	FREE_TINKERS_BITFIELD_INT,
	EQUIPMENT_SET_ID_INT,
	PET_CLASS_INT,
	LIFESPAN_INT,
	REMAINING_LIFESPAN_INT,
	USE_CREATE_QUANTITY_INT,
	WIELD_REQUIREMENTS_2_INT,
	WIELD_SKILLTYPE_2_INT,
	WIELD_DIFFICULTY_2_INT,
	WIELD_REQUIREMENTS_3_INT,
	WIELD_SKILLTYPE_3_INT,
	WIELD_DIFFICULTY_3_INT,
	WIELD_REQUIREMENTS_4_INT,
	WIELD_SKILLTYPE_4_INT,
	WIELD_DIFFICULTY_4_INT,
	UNIQUE_INT,
	SHARED_COOLDOWN_INT,
	FACTION1_BITS_INT,
	FACTION2_BITS_INT,
	FACTION3_BITS_INT,
	HATRED1_BITS_INT,
	HATRED2_BITS_INT,
	HATRED3_BITS_INT,
	SOCIETY_RANK_CELHAN_INT,
	SOCIETY_RANK_ELDWEB_INT,
	SOCIETY_RANK_RADBLO_INT,
	HEAR_LOCAL_SIGNALS_INT,
	HEAR_LOCAL_SIGNALS_RADIUS_INT,
	CLEAVING_INT,
	AUGMENTATION_SPECIALIZE_GEARCRAFT_INT,
	AUGMENTATION_INFUSED_CREATURE_MAGIC_INT,		// Infused Creature Magic
	AUGMENTATION_INFUSED_ITEM_MAGIC_INT,			// Infused Item Magic
	AUGMENTATION_INFUSED_LIFE_MAGIC_INT,			// Infused Life Magic
	AUGMENTATION_INFUSED_WAR_MAGIC_INT,				// Infused War Magic
	AUGMENTATION_CRITICAL_EXPERTISE_INT,			// Eye of the Remorseless
	AUGMENTATION_CRITICAL_POWER_INT,				// Hand of the Remorseless
	AUGMENTATION_SKILLED_MELEE_INT,					// Master of the Steel Circle
	AUGMENTATION_SKILLED_MISSILE_INT,				// Master of the Focused Eye
	AUGMENTATION_SKILLED_MAGIC_INT,					// Master of the Five Fold Path
	IMBUED_EFFECT_2_INT,
	IMBUED_EFFECT_3_INT,
	IMBUED_EFFECT_4_INT,
	IMBUED_EFFECT_5_INT,
	DAMAGE_RATING_INT,
	DAMAGE_RESIST_RATING_INT,
	AUGMENTATION_DAMAGE_BONUS_INT,					// Frenzy of the Slayer
	AUGMENTATION_DAMAGE_REDUCTION_INT,				// Iron Skin of the Invincible
	IMBUE_STACKING_BITS_INT,
	HEAL_OVER_TIME_INT,
	CRIT_RATING_INT,
	CRIT_DAMAGE_RATING_INT,
	CRIT_RESIST_RATING_INT,
	CRIT_DAMAGE_RESIST_RATING_INT,
	HEALING_RESIST_RATING_INT,
	DAMAGE_OVER_TIME_INT,
	ITEM_MAX_LEVEL_INT,
	ITEM_XP_STYLE_INT,
	EQUIPMENT_SET_EXTRA_INT,
	AETHERIA_BITFIELD_INT,
	HEALING_BOOST_RATING_INT,
	HERITAGE_SPECIFIC_ARMOR_INT,
	ALTERNATE_RACIAL_SKILLS_INT,
	AUGMENTATION_JACK_OF_ALL_TRADES_INT,			// Jack of All Trades
	AUGMENTATION_RESISTANCE_NETHER_INT,
	AUGMENTATION_INFUSED_VOID_MAGIC_INT,			// Infused Void Magic
	WEAKNESS_RATING_INT,
	NETHER_OVER_TIME_INT,
	NETHER_RESIST_RATING_INT,
	LUMINANCE_AWARD_INT,
	LUM_AUG_DAMAGE_RATING_INT,						// Destruction & Valor & Crystal of Surging Strength & Rare Armor Damage Boost V
	LUM_AUG_DAMAGE_REDUCTION_RATING_INT,			// Invulnerability & Protection & Crystal of Towering Defense & Rare Damage Reduction V
	LUM_AUG_CRIT_DAMAGE_RATING_INT,					// Retribution & Glory
	LUM_AUG_CRIT_REDUCTION_RATING_INT,				// Hardening & Temperance
	LUM_AUG_SURGE_EFFECT_RATING_INT,				// ???
	LUM_AUG_SURGE_CHANCE_RATING_INT,				// Aetheric Vision
	LUM_AUG_ITEM_MANA_USAGE_INT,					// Mana Flow			
	LUM_AUG_ITEM_MANA_GAIN_INT,						// Mana Infusion
	LUM_AUG_VITALITY_INT,							// Crystal of Vitality
	LUM_AUG_HEALING_RATING_INT,						// Purity
	LUM_AUG_SKILLED_CRAFT_INT,						// Craftsman
	LUM_AUG_SKILLED_SPEC_INT,						// Specialization
	LUM_AUG_NO_DESTROY_CRAFT_INT,
	RESTRICT_INTERACTION_INT,
	OLTHOI_LOOT_TIMESTAMP_INT,
	OLTHOI_LOOT_STEP_INT,
	USE_CREATES_CONTRACT_ID_INT,
	DOT_RESIST_RATING_INT,
	LIFE_RESIST_RATING_INT,
	CLOAK_WEAVE_PROC_INT,
	WEAPON_TYPE_INT,
	MELEE_MASTERY_INT,
	RANGED_MASTERY_INT,
	SNEAK_ATTACK_RATING_INT,
	RECKLESSNESS_RATING_INT,
	DECEPTION_RATING_INT,
	COMBAT_PET_RANGE_INT,
	WEAPON_AURA_DAMAGE_INT,
	WEAPON_AURA_SPEED_INT,
	SUMMONING_MASTERY_INT,
	HEARTBEAT_LIFESPAN_INT,
	USE_LEVEL_REQUIREMENT_INT,
	LUM_AUG_ALL_SKILLS_INT,
	USE_REQUIRES_SKILL_INT,
	USE_REQUIRES_SKILL_LEVEL_INT,
	USE_REQUIRES_SKILL_SPEC_INT,
	USE_REQUIRES_LEVEL_INT,
	GEAR_DAMAGE_INT,
	GEAR_DAMAGE_RESIST_INT,
	GEAR_CRIT_INT,
	GEAR_CRIT_RESIST_INT,
	GEAR_CRIT_DAMAGE_INT,
	GEAR_CRIT_DAMAGE_RESIST_INT,
	GEAR_HEALING_BOOST_INT,
	GEAR_NETHER_RESIST_INT,
	GEAR_LIFE_RESIST_INT,
	GEAR_MAX_HEALTH_INT,
	UNKNOWN_380_INT,
	PK_DAMAGE_RATING_INT, 
	PK_DAMAGE_RESIST_RATING_INT,
	GEAR_PK_DAMAGE_RATING_INT,
	GEAR_PK_DAMAGE_RESIST_RATING_INT,
	UNKNOWN_385_SEEN_INT,
	OVERPOWER_INT,
	OVERPOWER_RESIST_INT,
	GEAR_OVERPOWER_INT,
	GEAR_OVERPOWER_RESIST_INT,
	ENLIGHTENMENT_INT,

	NUM_INT_STAT_VALUES
};

enum STypeInt64 {
	UNDEF_INT64,
	TOTAL_EXPERIENCE_INT64,
	AVAILABLE_EXPERIENCE_INT64,
	AUGMENTATION_COST_INT64,
	ITEM_TOTAL_XP_INT64,
	ITEM_BASE_XP_INT64,
	AVAILABLE_LUMINANCE_INT64,
	MAXIMUM_LUMINANCE_INT64,
	INTERACTION_REQS_INT64,

	NUM_INT64_STAT_VALUES
};

enum STypeIID {
	UNDEF_IID,
	OWNER_IID,
	CONTAINER_IID,
	WIELDER_IID,
	FREEZER_IID,
	VIEWER_IID,
	GENERATOR_IID,
	SCRIBE_IID,
	CURRENT_COMBAT_TARGET_IID,
	CURRENT_ENEMY_IID,
	PROJECTILE_LAUNCHER_IID,
	CURRENT_ATTACKER_IID,
	CURRENT_DAMAGER_IID,
	CURRENT_FOLLOW_TARGET_IID,
	CURRENT_APPRAISAL_TARGET_IID,
	CURRENT_FELLOWSHIP_APPRAISAL_TARGET_IID,
	ACTIVATION_TARGET_IID,
	CREATOR_IID,
	VICTIM_IID,
	KILLER_IID,
	VENDOR_IID,
	CUSTOMER_IID,
	BONDED_IID,
	WOUNDER_IID,
	ALLEGIANCE_IID,
	PATRON_IID,
	MONARCH_IID,
	COMBAT_TARGET_IID,
	HEALTH_QUERY_TARGET_IID,
	LAST_UNLOCKER_IID,
	CRASH_AND_TURN_TARGET_IID,
	ALLOWED_ACTIVATOR_IID,
	HOUSE_OWNER_IID,
	HOUSE_IID,
	SLUMLORD_IID,
	MANA_QUERY_TARGET_IID,
	CURRENT_GAME_IID,
	REQUESTED_APPRAISAL_TARGET_IID,
	ALLOWED_WIELDER_IID,
	ASSIGNED_TARGET_IID,
	LIMBO_SOURCE_IID,
	SNOOPER_IID,
	TELEPORTED_CHARACTER_IID,
	PET_IID,
	PET_OWNER_IID,
	PET_DEVICE_IID,

	NUM_IID_STAT_VALUES
};

enum STypePosition {
	UNDEF_POSITION,
	LOCATION_POSITION,
	DESTINATION_POSITION,
	INSTANTIATION_POSITION,
	SANCTUARY_POSITION,
	HOME_POSITION,
	ACTIVATION_MOVE_POSITION,
	TARGET_POSITION,
	LINKED_PORTAL_ONE_POSITION,
	LAST_PORTAL_POSITION,
	PORTAL_STORM_POSITION,
	CRASH_AND_TURN_POSITION,
	PORTAL_SUMMON_LOC_POSITION,
	HOUSE_BOOT_POSITION,
	LAST_OUTSIDE_DEATH_POSITION,
	LINKED_LIFESTONE_POSITION,
	LINKED_PORTAL_TWO_POSITION,
	SAVE_1_POSITION,
	SAVE_2_POSITION,
	SAVE_3_POSITION,
	SAVE_4_POSITION,
	SAVE_5_POSITION,
	SAVE_6_POSITION,
	SAVE_7_POSITION,
	SAVE_8_POSITION,
	SAVE_9_POSITION,
	RELATIVE_DESTINATION_POSITION,
	TELEPORTED_CHARACTER_POSITION,

	NUM_POSITION_STAT_VALUES
};

enum STypeSkill {
	UNDEF_SKILL,
	AXE_SKILL,
	BOW_SKILL,
	CROSSBOW_SKILL,
	DAGGER_SKILL,
	MACE_SKILL,
	MELEE_DEFENSE_SKILL,
	MISSILE_DEFENSE_SKILL,
	SLING_SKILL,
	SPEAR_SKILL,
	STAFF_SKILL,
	SWORD_SKILL,
	THROWN_WEAPON_SKILL,
	UNARMED_COMBAT_SKILL,
	ARCANE_LORE_SKILL,
	MAGIC_DEFENSE_SKILL,
	MANA_CONVERSION_SKILL,
	SPELLCRAFT_SKILL,
	ITEM_APPRAISAL_SKILL,
	PERSONAL_APPRAISAL_SKILL,
	DECEPTION_SKILL,
	HEALING_SKILL,
	JUMP_SKILL,
	LOCKPICK_SKILL,
	RUN_SKILL,
	AWARENESS_SKILL,
	ARMS_AND_ARMOR_REPAIR_SKILL,
	CREATURE_APPRAISAL_SKILL,
	WEAPON_APPRAISAL_SKILL,
	ARMOR_APPRAISAL_SKILL,
	MAGIC_ITEM_APPRAISAL_SKILL,
	CREATURE_ENCHANTMENT_SKILL,
	ITEM_ENCHANTMENT_SKILL,
	LIFE_MAGIC_SKILL,
	WAR_MAGIC_SKILL,
	LEADERSHIP_SKILL,
	LOYALTY_SKILL,
	FLETCHING_SKILL,
	ALCHEMY_SKILL,
	COOKING_SKILL,
	SALVAGING_SKILL,
	TWO_HANDED_COMBAT_SKILL,
	GEARCRAFT_SKILL,
	VOID_MAGIC_SKILL,
	HEAVY_WEAPONS_SKILL,
	LIGHT_WEAPONS_SKILL,
	FINESSE_WEAPONS_SKILL,
	MISSILE_WEAPONS_SKILL,
	SHIELD_SKILL,
	DUAL_WIELD_SKILL,
	RECKLESSNESS_SKILL,
	SNEAK_ATTACK_SKILL,
	DIRTY_FIGHTING_SKILL,
	CHALLENGE_SKILL,
	SUMMONING_SKILL,
	NUM_SKILL
};

enum STypeString {
	UNDEF_STRING,
	NAME_STRING,
	TITLE_STRING,
	SEX_STRING,
	HERITAGE_GROUP_STRING,
	TEMPLATE_STRING,
	ATTACKERS_NAME_STRING,
	INSCRIPTION_STRING,
	SCRIBE_NAME_STRING,
	VENDORS_NAME_STRING,
	FELLOWSHIP_STRING,
	MONARCHS_NAME_STRING,
	LOCK_CODE_STRING,
	KEY_CODE_STRING,
	USE_STRING,
	SHORT_DESC_STRING,
	LONG_DESC_STRING,
	ACTIVATION_TALK_STRING,
	USE_MESSAGE_STRING,
	ITEM_HERITAGE_GROUP_RESTRICTION_STRING,
	PLURAL_NAME_STRING,
	MONARCHS_TITLE_STRING,
	ACTIVATION_FAILURE_STRING,
	SCRIBE_ACCOUNT_STRING,
	TOWN_NAME_STRING,
	CRAFTSMAN_NAME_STRING,
	USE_PK_SERVER_ERROR_STRING,
	SCORE_CACHED_TEXT_STRING,
	SCORE_DEFAULT_ENTRY_FORMAT_STRING,
	SCORE_FIRST_ENTRY_FORMAT_STRING,
	SCORE_LAST_ENTRY_FORMAT_STRING,
	SCORE_ONLY_ENTRY_FORMAT_STRING,
	SCORE_NO_ENTRY_STRING,
	QUEST_STRING,
	GENERATOR_EVENT_STRING,
	PATRONS_TITLE_STRING,
	HOUSE_OWNER_NAME_STRING,
	QUEST_RESTRICTION_STRING,
	APPRAISAL_PORTAL_DESTINATION_STRING,
	TINKER_NAME_STRING,
	IMBUER_NAME_STRING,
	HOUSE_OWNER_ACCOUNT_STRING,
	DISPLAY_NAME_STRING,
	DATE_OF_BIRTH_STRING,
	THIRD_PARTY_API_STRING,
	KILL_QUEST_STRING,
	AFK_STRING,
	ALLEGIANCE_NAME_STRING,
	AUGMENTATION_ADD_QUEST_STRING,
	KILL_QUEST_2_STRING,
	KILL_QUEST_3_STRING,
	USE_SENDS_SIGNAL_STRING,
	GEAR_PLATING_NAME_STRING,

	NUM_STRING_STAT_VALUES
};

