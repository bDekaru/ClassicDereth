
#pragma once

#include <map>
#include <unordered_map>
#include <list>
#include <queue>
#include <set>
#include <fstream>
#include <assert.h>
#include <time.h>

#include "PhatSDKSettings.h"

#ifndef LOG
#define LOG(category, level, format, ...) 
#endif

#ifndef LOG_PRIVATE
#define LOG_PRIVATE(category, level, format, ...) 
#endif

#if PHATSDK_DISABLE_ANNOYING_WARNINGS
#pragma warning(disable: 4244)
#pragma warning(disable: 4018)
#endif

// =============================================
// Legacy support stuff
// =============================================
#define UNFINISHED() { DebugBreak(); } // things that aren't finished
#define UNFINISHED_UNSAFE() { }
#define UNFINISHED_LEGACY(comment) DEBUGOUT("Unfinished code.\r\n"); DebugBreak();
#define UNFINISHED_WARNING_LEGACY(comment) DEBUGOUT("Unfinished code.\r\n");
typedef DWORD UNKNOWN_ARG;

#ifndef DEBUGOUT
#define DEBUGOUT(format, ...)
#endif

#if _DEBUG
#define DEBUG_BREAK() DebugBreak()
#else
#define DEBUG_BREAK()
#endif

#define SafeDelete(x) { if (x) { delete x; x = 0; } }
#define SafeDeleteArray(x) { if (x) { delete [] x; x = 0; } }

template<class typeName>
FORCEINLINE void CloneMemberPointerData(typeName * &selfMember, const typeName * otherMember)
{
	if (otherMember)
	{
		if (!selfMember)
			selfMember = new typeName;

		*selfMember = *otherMember;
	}
	else
	{
		SafeDelete(selfMember);
	}
}

// ======

#ifndef F_EPSILON
#define F_EPSILON (0.0002f)
#endif

#define INVALID_TIME ((double)-1.0)

class CWeenieObject;
class CPhysicsObj;

#include "Support/BinaryReader.h"
#include "Support/BinaryWriter.h"
#include "Support/DATDisk.h"
#include "Support/ISAAC.h"
#include "Support/Json.h"
#include "Support/Packable.h"
#include "Support/PackableJson.h"
#include "GameEnums.h"
#include "GameStatEnums.h"
#include "SpellEnums.h"
#include "Support/PhatDataBin.h"

// This is a huge file to be putting in the precompiled header 
// #include "WClassID.h"

#include "ACCharGenData.h"
#include "ACCharGenResult.h"
#include "AllegianceProfile.h"
#include "Animation.h"
#include "AnimHooks.h"
#include "Appraisal.h"
#include "BSPData.h"
#include "BuildingObj.h"
#include "ChildList.h"
#include "ClothingTable.h"
#include "CombatSystem.h"
#include "DArray.h"
#include "DLListBase.h"
#include "EnvCell.h"
#include "Environment.h"
#include "ExperienceTable.h"
#include "Frame.h"
#include "GameSky.h"
#include "GameTime.h"
#include "GfxObj.h"
#include "HashData.h"
#include "Housing.h"
#include "LandBlock.h"
#include "LandBlockInfo.h"
#include "LandBlockStruct.h"
#include "LandCell.h"
#include "LandDefs.h"
#include "LandScape.h"
#include "LegacyPackObj.h"
#include "LegacyStubs.h"
#include "MagicSystem.h"
#include "Material.h"
#include "MathLib.h"
#include "MotionTable.h"
#include "Movement.h"
#include "MovementManager.h"
#include "MutationFilter.h"
#include "ObjCache.h"
#include "ObjCell.h"
#include "ObjDesc.h"
#include "ObjectInventory.h"
#include "ObjectMaint.h"
#include "Palette.h"
#include "PalSet.h"
#include "PartArray.h"
#include "PartCell.h"
#include "Particles.h"
#include "Physics.h"
#include "PhysicsObj.h"
#include "PhysicsPart.h"
#include "PlayerModule.h"
#include "Polygon.h"
#include "PositionManager.h"
#include "PString.h"
#include "Qualities.h"
#include "QualityFilter.h"
#include "QuestDefDB.h"
#include "QuestTable.h"
#include "RegionDesc.h"
//#include "RenderEngine.h"
#include "SArray.h"
#include "Scripts.h"
#include "Setup.h"
#include "SkillTable.h"
#include "SkyDesc.h"
#include "SmartArray.h"
#include "SortCell.h"
#include "SoundDesc.h"
#include "SpellComponentTable.h"
#include "SpellTable.h"
#include "Surface.h"
#include "TargetManager.h"
#include "Texture.h"
#include "Transition.h"
#include "VendorProfile.h"
#include "Vertex.h"
#include "VitaeSystem.h"

#if PHATSDK_USE_PHYSICS_AND_WEENIE_DESC
#include "PhysicsDesc.h"
#include "PublicWeenieDesc.h"
#endif

#include "OldPublicWeenieDesc.h"

#if PHATSDK_USE_SMART_BOX
#include "SmartBox.h"
#endif

#if PHATSDK_USE_WEENIE_STUB
#include "WeenieObjectStub.h"
#else
#include "WeenieObject.h"
#endif

#include "ClothingCache.h"
#include "Support/WeenieDefaults.h"
#include "Support/WeenieSave.h"
#include "Support/LandBlockExtendedData.h"
#include "Support/RegionDescExtendedData.h"
#include "Support/SpellTableExtendedData.h"
#include "Support/TreasureTable.h"
#include "Support/CraftTable.h"
#include "Support/GameEventDefDB.h"
#include "Support/SkillChecks.h"

class CPhatSDKRandom
{
public:
	CPhatSDKRandom()
	{
		m_iSeed = (signed)((unsigned int)(time(0) * 54321) << 2) - 143510491;
	}

	int m_iSeed;
	
	inline uint32_t GenRandom15()
	{
		m_iSeed = (m_iSeed * 214013) + 2531011;

		uint32_t dwRandom15 = (unsigned)m_iSeed;
		dwRandom15 >>= 16;
		dwRandom15 &= 0x7FFF;

		return dwRandom15;
	}

	uint32_t GenRandom32()
	{
		uint32_t dwRandom32 = 0;
		dwRandom32 |= GenRandom15();
		dwRandom32 <<= 15;
		dwRandom32 |= GenRandom15();
		dwRandom32 <<= 15;
		dwRandom32 |= GenRandom15();

		return dwRandom32;
	}

	uint32_t GenUInt(uint32_t min, uint32_t max)
	{
		if (max <= min)
			return min;

		unsigned int range = max - min;
		return (min + (GenRandom32() % (range + 1)));
	}

	int32_t GenInt(int32_t min, int32_t max)
	{
		if (max <= min)
			return min;

		unsigned int range = (unsigned)(max - min);
		return (min + (int)(GenRandom32() % (range + 1)));
	}

	double GenFloat(double min, double max)
	{
		if (max <= min)
			return min;

		double range = max - min;
		double value = min + (range * ((double)GenRandom32() / 0xFFFFFFFF));

		return value;
	}

	float GenFloat(float min, float max)
	{
		if (max <= min)
			return min;

		double range = max - min;
		double value = min + (range * ((double)GenRandom32() / 0xFFFFFFFF));

		return (float)value;
	}
};

class CPhatSDKImpl
{
public:
	CPhatSDKRandom Random;

	void UpdateInternalTime();

	virtual double GetCurrTime() = 0;
	virtual int GetCurrTimeStamp() { return time(NULL); }
	virtual double GetRandomFloat(double min, double max) { return Random.GenFloat(min, max); }
	virtual int GetRandomInt(int min, int max) { return Random.GenInt(min, max); }
	virtual unsigned int GetRandomUInt(unsigned int min, unsigned int max) { return Random.GenUInt(min, max); }
	virtual BYTE *GetPortalDataEntry(DWORD id, DWORD *length) = 0;
	virtual BYTE *GetCellDataEntry(DWORD id, DWORD *length) = 0;

	virtual class CEnvCell *EnvCell_GetVisible(DWORD cell_id) = 0;

	virtual class CQuestDefDB *GetQuestDefDB() { return NULL; }
};

extern CPhatSDKImpl *g_pPhatSDK;

namespace Random
{
	inline float RollDice(float min, float max) {
		return (float) g_pPhatSDK->GetRandomFloat(min, max);
	}
};

class Timer
{
public:
	static double cur_time;
};
