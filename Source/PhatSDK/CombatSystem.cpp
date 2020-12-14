
#include <StdAfx.h>
#include "PhatSDK.h"

CombatManeuver::CombatManeuver()
{
}

CombatManeuver::~CombatManeuver()
{
}

DEFINE_PACK(CombatManeuver)
{
	UNFINISHED();
}

DEFINE_UNPACK(CombatManeuver)
{
	style = pReader->Read<uint32_t>();
	attack_height = (ATTACK_HEIGHT) pReader->Read<uint32_t>();
	attack_type = (AttackType)pReader->Read<uint32_t>();
	min_skill_level = pReader->Read<uint32_t>();
	motion = pReader->Read<uint32_t>();
	return true;
}

CombatManeuverTable::CombatManeuverTable()
{
}

CombatManeuverTable::~CombatManeuverTable()
{
	Clear();
}

void CombatManeuverTable::Clear()
{
	_num_combat_maneuvers = 0;
	SafeDeleteArray(_cmt);
}

CombatManeuver *CombatManeuverTable::TryGetCombatManuever(uint32_t style, AttackType at, ATTACK_HEIGHT height) // custom
{
	for (uint32_t i = 0; i < _num_combat_maneuvers; i++)
	{
		CombatManeuver *cm = &_cmt[i];

		if ((cm->style == style) && (cm->attack_type & at) && (cm->attack_height == height))
			return cm;
	}

	return NULL;
}

uint32_t CombatManeuverTable::CombatStyleToMotion(CombatStyle style)
{
	UNFINISHED();
	return 0;
}

COMBAT_MODE CombatManeuverTable::CombatStyleToCombatMode(CombatStyle style)
{
	UNFINISHED();
	return COMBAT_MODE::UNDEF_COMBAT_MODE;
}

DEFINE_DBOBJ(CombatManeuverTable, CombatManeuverTables);
DEFINE_LEGACY_PACK_MIGRATOR(CombatManeuverTable);

DEFINE_PACK(CombatManeuverTable)
{
	UNFINISHED();
}

DEFINE_UNPACK(CombatManeuverTable)
{
	Clear();

	pReader->Read<uint32_t>(); // file ID

	_num_combat_maneuvers = pReader->Read<uint32_t>();
	_cmt = new CombatManeuver[_num_combat_maneuvers];
	for (uint32_t i = 0; i < _num_combat_maneuvers; i++)
		_cmt[i].UnPack(pReader);

	return true;
}

BOOL CombatSystem::InqCombatHitAdjectives(DAMAGE_TYPE damage_type, const double php, std::string &single_adj, std::string &plural_adj)
{
	if (php < 0.0)
		return FALSE;

	switch (damage_type)
	{
	default:
		single_adj = "hit";
		plural_adj = "hits";
		return TRUE;

	case NETHER_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "eradicate";
			plural_adj = "eradicates";
		}
		else if (php > 0.25)
		{
			single_adj = "wither";
			plural_adj = "withers";
		}
		else if (php > 0.1)
		{
			single_adj = "twist";
			plural_adj = "twists";
		}
		else
		{
			single_adj = "scar";
			plural_adj = "scars";
		}
		return TRUE;

	case HEALTH_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "deplete";
			plural_adj = "depletes";
		}
		else if (php > 0.25)
		{
			single_adj = "siphon";
			plural_adj = "siphons";
		}
		else if (php > 0.1)
		{
			single_adj = "exhaust";
			plural_adj = "exhausts";
		}
		else
		{
			single_adj = "drain";
			plural_adj = "drains";
		}
		return TRUE;

	case ELECTRIC_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "blast";
			plural_adj = "blasts";
		}
		else if (php > 0.25)
		{
			single_adj = "jolt";
			plural_adj = "jolts";
		}
		else if (php > 0.1)
		{
			single_adj = "shock";
			plural_adj = "shocks";
		}
		else
		{
			single_adj = "spark";
			plural_adj = "sparks";
		}
		return TRUE;

	case ACID_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "dissolve";
			plural_adj = "dissolves";
		}
		else if (php > 0.25)
		{
			single_adj = "corrode";
			plural_adj = "corrodes";
		}
		else if (php > 0.1)
		{
			single_adj = "sear";
			plural_adj = "sears";
		}
		else
		{
			single_adj = "blister";
			plural_adj = "blisters";
		}
		return TRUE;

	case COLD_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "freeze";
			plural_adj = "freezes";
		}
		else if (php > 0.25)
		{
			single_adj = "frost";
			plural_adj = "frosts";
		}
		else if (php > 0.1)
		{
			single_adj = "chill";
			plural_adj = "chills";
		}
		else
		{
			single_adj = "numb";
			plural_adj = "numbs";
		}
		return TRUE;

	case BLUDGEON_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "crush";
			plural_adj = "crushes";
		}
		else if (php > 0.25)
		{
			single_adj = "smash";
			plural_adj = "smashes";
		}
		else if (php > 0.1)
		{
			single_adj = "bash";
			plural_adj = "bashes";
		}
		else
		{
			single_adj = "graze";
			plural_adj = "grazes";
		}
		return TRUE;

	case SLASH_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "mangle";
			plural_adj = "mangles";
		}
		else if (php > 0.25)
		{
			single_adj = "slash";
			plural_adj = "slashes";
		}
		else if (php > 0.1)
		{
			single_adj = "cut";
			plural_adj = "cuts";
		}
		else
		{
			single_adj = "scratch";
			plural_adj = "scratches";
		}
		return TRUE;

	case PIERCE_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "gore";
			plural_adj = "gores";
		}
		else if (php > 0.25)
		{
			single_adj = "impale";
			plural_adj = "impales";
		}
		else if (php > 0.1)
		{
			single_adj = "stab";
			plural_adj = "stabs";
		}
		else
		{
			single_adj = "nick";
			plural_adj = "nicks";
		}
		return TRUE;

	case FIRE_DAMAGE_TYPE:
		if (php > 0.5)
		{
			single_adj = "incinerate";
			plural_adj = "incinerates";
		}
		else if (php > 0.25)
		{
			single_adj = "burn";
			plural_adj = "burns";
		}
		else if (php > 0.1)
		{
			single_adj = "scorch";
			plural_adj = "scorches";
		}
		else
		{
			single_adj = "singe";
			plural_adj = "singes";
		}
		return TRUE;
	}
}