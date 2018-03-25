
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

	DWORD motion;
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
	void add_modifier_no_check(DWORD value1, float value2);
	void add_action(DWORD action, float speed_mod);
	BOOL add_modifier(DWORD modifier, float speed_mod);

	DWORD style;
	DWORD substate;
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
		DWORD motion; // 0x08
		DWORD num_anims; // 0x0C
	};

	static MotionTableManager *Create(DWORD ID);

	MotionTableManager(DWORD ID);
	~MotionTableManager();

	void Destroy();

	DWORD GetMotionTableID();
	BOOL SetMotionTableID(DWORD ID);

	void CheckForCompletedMotions();
	void SetPhysicsObject(CPhysicsObj *pPhysicsObj);
	void HandleEnterWorld(CSequence *pSequence);
	void HandleExitWorld();

	void initialize_state(CSequence *psequence);
	void AnimationDone(BOOL success);

	// unfinished:
	void remove_redundant_links(CSequence *initstate);
	void truncate_animation_list(AnimNode *pnode, CSequence *psequence);
	DWORD PerformMovement(const MovementStruct &cmd, CSequence *psequence);

	void add_to_queue(DWORD motionid, DWORD counter, CSequence *psequence);

	void UseTime();

	CPhysicsObj *physics_obj; // 0x00
	CMotionTable *table; // 0x04
	MotionState state; // 0x08
	DWORD animation_counter;
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
	static CMotionTable *Get(DWORD ID);
	static void Release(CMotionTable *pMotionTable);

	void Destroy();

	BOOL UnPack(BYTE **ppData, ULONG uiSize);

	BOOL SetDefaultState(MotionState *pstate, CSequence *psequence, DWORD *unknown);

	void HandleEnterWorld();
	void HandleExitWorld();
	
	BOOL StopSequenceMotion(DWORD motion, float speed, MotionState *curr_state, CSequence *sequence, DWORD *num_anims);
	BOOL StopObjectCompletely(MotionState *pstate, CSequence *psequence, DWORD *pcounter);

	BOOL GetObjectSequence(DWORD value, MotionState *pstate, CSequence *psequence, float value2, DWORD *pcounter, DWORD unknown);
	MotionData *get_link(DWORD arg_0, DWORD arg_4, float arg_8, DWORD arg_C, float arg_10);
	void re_modify(CSequence *psequence, MotionState *pstate);
	BOOL DoObjectMotion(DWORD motionid, MotionState *pstate, CSequence *psequence, float funknown, DWORD *pcounter);
	BOOL StopObjectMotion(DWORD motionid, float funknown, MotionState *pstate, CSequence *psequence, DWORD *pcounter);
	BOOL is_allowed(DWORD motionid, MotionData *pmotiondata, MotionState *pstate);

	LongNIValHash<DWORD> style_defaults;
	LongHash<MotionData> cycles;
	LongHash<MotionData> modifiers;
	LongNIValHash<LongHash<MotionData> *> links;
	DWORD default_style;
};
