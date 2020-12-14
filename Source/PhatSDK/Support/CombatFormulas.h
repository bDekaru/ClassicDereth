#pragma once
#include "SpellcastingManager.h"
#include "WeenieObject.h"

double GetImbueMultiplier(double currentSkill, double minEffectivenessSkill, double maxEffectivenessSkill, double minMultiplier, double maxMultiplier, bool allowNegative = false);
void CalculateDamage(DamageEventData *dmgEvent, SpellCastData *spellData = NULL);
void CalculateAttributeDamageBonus(DamageEventData *dmgEvent);
void CalculateSkillDamageBonus(DamageEventData *dmgEvent, SpellCastData *spellData);
void CalculateCriticalHitData(DamageEventData *dmgEvent, SpellCastData *spellData);
void CalculateSlayerData(DamageEventData *dmgEvent);
void CalculateRendingAndMiscData(DamageEventData *dmgEvent);
void CalculateRatingData(DamageEventData *dmgEvent);
void CalculateAttackConditions(DamageEventData *dmgEvent, float attackPower, double angle);

