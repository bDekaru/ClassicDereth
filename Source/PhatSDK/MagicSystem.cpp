
#include <StdAfx.h>
#include "PhatSDK.h"
#include "MagicSystem.h"

SpellComponentTable *MagicSystem::GetSpellComponentTable()
{
	return CachedSpellComponentTable;
}

CSpellTable *MagicSystem::GetSpellTable()
{
	return CachedSpellTable;
}

uint32_t MagicSystem::GetLowestTaperID()
{
	return 63;
}

uint32_t MagicSystem::DeterminePowerLevelOfComponent(uint32_t scid)
{
	uint32_t result = 0;

	switch (scid)
	{
	case 1u:
		result = 1;
		break;
	case 2u:
		result = 2;
		break;
	case 3u:
		result = 3;
		break;
	case 4u:
		result = 4;
		break;
	case 5u:
		result = 5;
		break;
	case 6u:
		result = 6;
		break;
	case 0x6Eu:
		result = 6;
		break;
	case 0x70u:
		result = 7;
		break;
	case 0xC0u:
		result = 7;
		break;
	case 0xC1u:
		result = 8;
		break;
	}

	return result;
}


