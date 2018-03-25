
#pragma once

double GetSkillChance(int skill, int difficulty);
double GetMagicSkillChance(int skill, int difficulty);
bool GenericSkillCheck(int offense, int defense);
bool AppraisalSkillCheck(int offense, int defense);

int GetManaCost(int skill, int difficulty, int manaCost, int manaConversion);

bool TryMagicResist(int offense, int defense);
bool TryMeleeEvade(int offense, int defense);
bool TryMissileEvade(int offense, int defense);
