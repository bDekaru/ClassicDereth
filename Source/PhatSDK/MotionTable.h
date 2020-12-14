
#pragma once

#include "ObjCache.h"
#include "MathLib.h"
#include "DLListBase.h"
#include "Frame.h"
#include "Animation.h"
#include "HashData.h"

class CMotionTable;
class CSequence;
class MotionData;
struct MovementStruct;

class MotionList
{
public:
	MotionList()
	{
		motion = 0;
		speed_mod = 1.0f;
		next = NULL;
	}

	MotionList(MotionList *other)
	{
		motion = other->motion;
		speed_mod = other->speed_mod;
		next = NULL;
	}

	uint32_t motion;
	float speed_mod;
	MotionList *next;
};

class MotionState
{
public:
	MotionState(MotionState *pstate);
	MotionState();

	void Destroy();

	void remove_action_head();
	void remove_modifier(MotionList *ptarget, MotionList *plast);

	void clear_modifiers();
	void clear_actions();
	void copy(MotionState *pstate);
	void add_modifier_no_check(uint32_t value1, float value2);
	void add_action(uint32_t action, float speed_mod);
	BOOL add_modifier(uint32_t modifier, float speed_mod);

	uint32_t style;
	uint32_t substate;
	float substate_mod;
	MotionList *modifier_head; // 0x0C
	MotionList *action_head; // 0x10
	MotionList *action_tail; // 0x14
};

class MotionTableManager
{
public:
	class AnimNode : public DLListData // size: 0x10
	{
	public:
		uint32_t motion; // 0x08
		uint32_t num_anims; // 0x0C
	};

	static MotionTableManager *Create(uint32_t ID);

	MotionTableManager(uint32_t ID);
	~MotionTableManager();

	void Destroy();

	uint32_t GetMotionTableID();
	BOOL SetMotionTableID(uint32_t ID);

	void CheckForCompletedMotions();
	void SetPhysicsObject(CPhysicsObj *pPhysicsObj);
	void HandleEnterWorld(CSequence *pSequence);
	void HandleExitWorld();

	void initialize_state(CSequence *psequence);
	void AnimationDone(BOOL success);

	// unfinished:
	void remove_redundant_links(CSequence *initstate);
	void truncate_animation_list(AnimNode *pnode, CSequence *psequence);
	uint32_t PerformMovement(const MovementStruct &cmd, CSequence *psequence);

	void add_to_queue(uint32_t motionid, uint32_t counter, CSequence *psequence);

	void UseTime();

	CPhysicsObj *physics_obj; // 0x00
	CMotionTable *table; // 0x04
	MotionState state; // 0x08
	uint32_t animation_counter;
	DLListBase pending_animations; // 0x24, 0x28
};

class MotionData : public LongHashData
{
public:
	MotionData();
	virtual ~MotionData();

	void Destroy();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	BYTE num_anims;
	AnimData *anims;
	Vector velocity;
	Vector omega;
	BYTE bitfield;
};

class CMotionTable : public DBObj
{
public:
	CMotionTable();
	virtual ~CMotionTable();

	static DBObj *Allocator();
	static void Destroyer(DBObj *pObject);
	static CMotionTable *Get(uint32_t ID);
	static void Release(CMotionTable *pMotionTable);

	void Destroy();

	BOOL UnPack(BYTE **ppData, ULONG uiSize);

	BOOL SetDefaultState(MotionState *pstate, CSequence *psequence, uint32_t *unknown);

	void HandleEnterWorld();
	void HandleExitWorld();
	
	BOOL StopSequenceMotion(uint32_t motion, float speed, MotionState *curr_state, CSequence *sequence, uint32_t *num_anims);
	BOOL StopObjectCompletely(MotionState *pstate, CSequence *psequence, uint32_t *pcounter);

	BOOL GetObjectSequence(uint32_t value, MotionState *pstate, CSequence *psequence, float value2, uint32_t *pcounter, uint32_t unknown);
	MotionData *get_link(uint32_t arg_0, uint32_t arg_4, float arg_8, uint32_t arg_C, float arg_10);
	void re_modify(CSequence *psequence, MotionState *pstate);
	BOOL DoObjectMotion(uint32_t motionid, MotionState *pstate, CSequence *psequence, float funknown, uint32_t *pcounter);
	BOOL StopObjectMotion(uint32_t motionid, float funknown, MotionState *pstate, CSequence *psequence, uint32_t *pcounter);
	BOOL is_allowed(uint32_t motionid, MotionData *pmotiondata, MotionState *pstate);

	LongNIValHash<uint32_t> style_defaults;
	LongHash<MotionData> cycles;
	LongHash<MotionData> modifiers;
	LongNIValHash<LongHash<MotionData> *> links;
	uint32_t default_style;
};
