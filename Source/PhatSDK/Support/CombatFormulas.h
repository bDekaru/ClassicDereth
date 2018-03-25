#pragma once
#include "SpellcastingManager.h"

double GetImbueMultiplier(double currentSkill, double minEffectivenessSkill, double maxEffectivenessSkill, double maxMultiplier, bool allowNegative = false);
void CalculateDamage(DamageEventData *dmgEvent, SpellCastData *spellData = NULL);
void CalculateAttributeDamageBonus(DamageEventData *dmgEvent);
void CalculateSkillDamageBonus(DamageEventData *dmgEvent, SpellCastData *spellData);
void CalculateCriticalHitData(DamageEventData *dmgEvent, SpellCastData *spellData);
void CalculateSlayerData(DamageEventData *dmgEvent);
void CalculateRendingAndMiscData(DamageEventData *dmgEvent);