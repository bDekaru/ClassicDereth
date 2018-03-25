
#include "StdAfx.h"
#include "MotionTable.h"
#include "PhysicsObj.h"
#include "PartArray.h"
#include "Movement.h"
#include "MovementManager.h"

MotionTableManager *MotionTableManager::Create(DWORD arg0)
{
	MotionTableManager *pMTManager = new MotionTableManager(arg0);

	return pMTManager;
}

MotionTableManager::MotionTableManager(DWORD ID)
{
	physics_obj = NULL;
	table = NULL;

	animation_counter = 0;

	if (ID)
		SetMotionTableID(ID);
}

MotionTableManager::~MotionTableManager()
{
	Destroy();
}

void MotionTableManager::Destroy()
{
	while (pending_animations.head_)
	{
		AnimNode *pAnim = (AnimNode *)pending_animations.head_;
		pending_animations.RemoveAndDelete(pAnim);
	}

	if (table)
	{
		CMotionTable::Release(table);
		table = NULL;
	}
}

void MotionTableManager::UseTime()
{
	CheckForCompletedMotions();
}

void MotionTableManager::SetPhysicsObject(CPhysicsObj *pPhysicsObj)
{
	physics_obj = pPhysicsObj;
}

DWORD MotionTableManager::GetMotionTableID()
{
	return (table ? table->GetID() : 0);
}

BOOL MotionTableManager::SetMotionTableID(DWORD ID)
{
	CMotionTable::Release(table);

	table = CMotionTable::Get(ID);

	return (table ? TRUE : FALSE);
}

void MotionTableManager::HandleEnterWorld(CSequence *pSequence)
{
	pSequence->remove_all_link_animations();

	while (pending_animations.head_)
		AnimationDone(FALSE);
}

void MotionTableManager::HandleExitWorld()
{
	while (pending_animations.head_)
		AnimationDone(FALSE);
}

void MotionTableManager::AnimationDone(BOOL success)
{
	AnimNode *pnode = (AnimNode *)pending_animations.head_;

	if (pnode)
	{
		animation_counter++;

		do
		{
			if (pnode->num_anims > animation_counter)
				break;

			if (pnode->motion & 0x10000000)
				state.remove_action_head();

			DWORD motion_id = pnode->motion;
			physics_obj->MotionDone(motion_id, success);
			animation_counter -= pnode->num_anims;

			if (pending_animations.head_)
			{
				pending_animations.RemoveAndDelete(pending_animations.head_);
			}

#if PHATSDK_IS_SERVER
			if (physics_obj->weenie_obj)
				physics_obj->weenie_obj->OnMotionDone(motion_id, success);
#endif

			pnode = (AnimNode *)pending_animations.head_;

		} while (pnode);

		if (animation_counter && !pnode)
			animation_counter = 0;
	}
}

void MotionTableManager::initialize_state(CSequence *seq)
{
	DWORD num_anims = 0;

	if (table)
	{
		table->SetDefaultState(&state, seq, &num_anims);
	}
	
	add_to_queue(0x41000003, num_anims, seq);
}

void MotionTableManager::remove_redundant_links(CSequence *seq)
{
	AnimNode *entry = (AnimNode *)pending_animations.tail_;

	while (entry)
	{
		if (entry->num_anims)
		{
			if (!(entry->motion & 0x40000000) || (entry->motion & 0x20000000))
			{
				if (!(entry->motion & 0x80000000))
					return;

				AnimNode *node2 = (AnimNode *)entry->dllist_prev;

				while (node2)
				{
					if (node2->motion == entry->motion)
					{
						truncate_animation_list(node2, seq);
						return;
					}

					if (node2->num_anims && (node2->motion & 0x70000000))
						return;

					node2 = (AnimNode *)node2->dllist_prev;
				}
			}
			else
			{
				AnimNode *node2 = (AnimNode *)entry->dllist_prev;

				while (node2)
				{
					if ((node2->motion == entry->motion) && node2->num_anims)
					{
						truncate_animation_list(node2, seq);
						return;
					}

					if (node2->num_anims && (node2->motion & 0xB0000000))
						return;

					node2 = (AnimNode *)node2->dllist_prev;
				}
			}
		}

		entry = (AnimNode *)entry->dllist_prev;
	}
}

void MotionTableManager::truncate_animation_list(AnimNode *node, CSequence *seq)
{
	if (!node)
		return;

	AnimNode *entry = (AnimNode *)pending_animations.tail_;

	DWORD sum = 0;

	while (entry != node)
	{
		if (!entry)
			return;

		sum += entry->num_anims;
		entry->num_anims = 0;

		entry = (AnimNode *)entry->dllist_prev;
	}

	seq->remove_link_animations(sum);
}

void MotionTableManager::add_to_queue(DWORD motionid, DWORD counter, CSequence *psequence)
{
	AnimNode *node = new AnimNode;
	node->motion = motionid;
	node->num_anims = counter;

	pending_animations.InsertAfter(node, pending_animations.tail_);
	remove_redundant_links(psequence);
}

DWORD MotionTableManager::PerformMovement(const MovementStruct &ms, CSequence *seq)
{
	if (!table)
		return 7;

	switch (ms.type)
	{
	case 2:
		{
			DWORD counter;
			if (!table->DoObjectMotion(ms.motion, &state, seq, ms.params->speed, &counter))
				return 0x43;

			add_to_queue(ms.motion, counter, seq);
			return 0;
		}
	case 4:
		{
			DWORD counter;
			if (!table->StopObjectMotion(ms.motion, ms.params->speed, &state, seq, &counter))
				return 0x43;

			add_to_queue(0x41000003, counter, seq);
			return 0;
		}
	case 5:
		{
			DWORD counter;
			table->StopObjectCompletely(&state, seq, &counter);
			add_to_queue(0x41000003, counter, seq);
			return 0;
		}

	default:
		return (DWORD)seq;
	}
}

void MotionTableManager::CheckForCompletedMotions()
{
	while (pending_animations.head_)
	{
		AnimNode *pAnim = ((AnimNode *)pending_animations.head_);

		if (pAnim->num_anims)
			return;

		if (pAnim->motion & 0x10000000)
			state.remove_action_head();

		DWORD motion_id = pAnim->motion;
		physics_obj->MotionDone(motion_id, TRUE);

		if (pending_animations.head_) // this can go bad; probably due to somewhere in MotionDone()
			pending_animations.RemoveAndDelete(pAnim);

#if PHATSDK_IS_SERVER
		if (physics_obj->weenie_obj)
			physics_obj->weenie_obj->OnMotionDone(motion_id, TRUE);
#endif
	}
}

MotionState::MotionState()
{
	style = 0;
	substate = 0;
	substate_mod = 1.0f;
	modifier_head = NULL;
	action_head = NULL;
	action_tail = NULL;
}

MotionState::MotionState(MotionState *pstate)
{
	copy(pstate);
}

void MotionState::Destroy()
{
	while (modifier_head)
	{
		MotionList *toDelete = modifier_head;
		modifier_head = modifier_head->next;

		delete toDelete;
	}

	while (action_head)
	{
		MotionList *toDelete = action_head;
		action_head = action_head->next;

		delete toDelete;
	}
	action_tail = NULL;
}

void MotionState::copy(MotionState *pstate)
{
	style = pstate->style;
	substate = pstate->substate;
	substate_mod = pstate->substate_mod;

	MotionList *old_modifier = pstate->modifier_head;

	if (old_modifier)
	{
		modifier_head = new MotionList(old_modifier);
		MotionList *new_modifier_prev = modifier_head;

		old_modifier = old_modifier->next;

		while (old_modifier)
		{
			MotionList *modifier_copy = new MotionList(old_modifier);

			// Set the previous list entry's next to be this new one
			new_modifier_prev->next = modifier_copy;

			// Now we're the previous list entry
			new_modifier_prev = modifier_copy;

			old_modifier = old_modifier->next;
		}
	}
	else
	{
		modifier_head = NULL;
	}

	if (pstate->action_head)
	{
		action_tail = action_head = new MotionList(pstate->action_head);

		MotionList *paction = pstate->action_head;

		while (paction)
		{
			MotionList *pactioncopy = new MotionList(paction);

			action_tail->next = pactioncopy;
			action_tail = pactioncopy;

			paction = paction->next;
		}
	}
	else
	{
		action_head = NULL;
		action_tail = NULL;
	}
}

void MotionState::add_action(DWORD action, float speed_mod)
{
	MotionList *new_action = new MotionList();

	new_action->motion = action;
	new_action->speed_mod = speed_mod;
	new_action->next = NULL;

	if (!action_head)
		action_head = new_action;

	MotionList *pCurrentTail = action_tail;
	if (pCurrentTail)
		pCurrentTail->next = new_action;

	action_tail = new_action;
}

BOOL MotionState::add_modifier(DWORD modifier, float speed_mod)
{
	MotionList *iterator = modifier_head;

	while (iterator)
	{
		if (iterator->motion == modifier)
			return FALSE;

		iterator = iterator->next;
	}

	if (substate == modifier)
		return FALSE;

	add_modifier_no_check(modifier, speed_mod);
	return TRUE;
}

void MotionState::add_modifier_no_check(DWORD value1, float value2)
{
	MotionList *modifier = new MotionList;

	modifier->motion = value1;
	modifier->speed_mod = value2;
	modifier->next = modifier_head;

	modifier_head = modifier;
}

void MotionState::remove_modifier(MotionList *ptarget, MotionList *plast)
{
	if (plast)
	{
		plast->next = ptarget->next;
		delete ptarget;
	}
	else
	{
		modifier_head = ptarget->next;
		delete ptarget;
	}
}

void MotionState::clear_modifiers()
{
	MotionList *modifier = modifier_head;

	while (modifier)
	{
		MotionList *pnext = modifier->next;

		delete modifier;
		modifier = pnext;
	}

	modifier_head = NULL;
}

void MotionState::clear_actions()
{
	MotionList *paction = action_head;

	while (paction)
	{
		MotionList *pnext = paction->next;

		delete paction;
		paction = pnext;
	}

	action_head = NULL;
	action_tail = NULL;
}

void MotionState::remove_action_head()
{
	MotionList *paction = action_head;

	if (paction)
	{
		action_head = paction->next;

		if (!action_head)
			action_tail = NULL;

		delete paction;
	}
}

CMotionTable::CMotionTable() : cycles(64), modifiers(16)
{
}

CMotionTable::~CMotionTable()
{
	Destroy();
}

DBObj* CMotionTable::Allocator()
{
	return((DBObj *)new CMotionTable());
}

void CMotionTable::Destroyer(DBObj* pMotionTable)
{
	delete ((CMotionTable *)pMotionTable);
}

CMotionTable *CMotionTable::Get(DWORD ID)
{
	return (CMotionTable *)ObjCaches::MotionTables->Get(ID);
}

void CMotionTable::Release(CMotionTable *pMotionTable)
{
	if (pMotionTable)
		ObjCaches::MotionTables->Release(pMotionTable->GetID());
}

void CMotionTable::Destroy()
{
	cycles.destroy_contents();
	modifiers.destroy_contents();

	// kinda assuming this is what the code did.
	style_defaults.destroy_contents();

	// not the same, but functionally should work
	LongNIValHashIter<LongHash<MotionData> *> iter(&links);

	while (!iter.EndReached() && iter.GetCurrent())
	{
		auto current = iter.GetCurrent();

		/*
		HashBaseIter<MotionData> iter2(current->m_Data);
		while (!iter2.EndReached() && iter2.GetCurrent())
		{
			auto current2 = iter2.GetCurrent();
			delete current2;

			iter2.Next();
		}
		*/

		current->m_Data->destroy_contents();

		delete current->m_Data;
		current->m_Data = NULL;

		iter.Next();
	}

	links.destroy_contents();
}

void subtract_motion(CSequence *psequence, MotionData *pmotiondata, float unknown)
{
	if (!pmotiondata)
		return;

	Vector vec18 = pmotiondata->omega * unknown;
	Vector vec0C = pmotiondata->velocity * unknown;
	psequence->subtract_physics(vec0C, vec18);
}

void combine_motion(CSequence *psequence, MotionData *pmotiondata, float unknown)
{
	if (!pmotiondata)
		return;

	Vector vec18 = pmotiondata->omega * unknown;
	Vector vec0C = pmotiondata->velocity * unknown;
	psequence->combine_physics(vec0C, vec18);
}

BOOL CMotionTable::StopSequenceMotion(DWORD motion, float speed, MotionState *curr_state, CSequence *sequence, DWORD *num_anims)
{
	*num_anims = 0;

	if ((motion & 0x40000000) && (curr_state->substate == motion))
	{
		DWORD value;
		style_defaults.lookup(curr_state->style, &value);
		GetObjectSequence(value, curr_state, sequence, 1.0f, num_anims, 1);
		return TRUE;
	}

	if (!(motion & 0x20000000))
		return FALSE;

	MotionList *pmod = curr_state->modifier_head;
	MotionList *plastmod = NULL;

	while (pmod)
	{
		if (pmod->motion == motion)
		{
			DWORD datakey = (pmod->motion << 16) | (motion & 0xFFFFFF);

			MotionData *pmotiondata;
			pmotiondata = modifiers.lookup(datakey);

			if (!pmotiondata)
				pmotiondata = modifiers.lookup(motion & 0xFFFFFF);

			if (!pmotiondata)
				return FALSE;

			subtract_motion(sequence, pmotiondata, pmod->speed_mod);
			curr_state->remove_modifier(pmod, plastmod);
			return TRUE;
		}

		plastmod = pmod;
		pmod = pmod->next;
	}

	return FALSE;
}

BOOL CMotionTable::StopObjectCompletely(MotionState *pstate, CSequence *psequence, DWORD *pcounter)
{
	BOOL unknown = 0;
	float something;

	while (pstate->modifier_head)
	{
		something = pstate->modifier_head->speed_mod;

		if (StopSequenceMotion(pstate->modifier_head->motion, pstate->modifier_head->speed_mod, pstate, psequence, pcounter))
			unknown = 1;
	}

	something = pstate->substate_mod;
	if (!StopSequenceMotion(pstate->substate, something, pstate, psequence, pcounter))
		return unknown;

	return TRUE;
}

AnimData sub_445200(float speed, AnimData *pdata)
{
	AnimData result;

	result.anim_id = pdata->anim_id;
	result.low_frame = pdata->low_frame;
	result.high_frame = pdata->high_frame;
	result.framerate = pdata->framerate * speed;

	return result;
}

void add_motion(CSequence *psequence, MotionData *pmotiondata, float speed)
{
	if (!pmotiondata)
		return;

	psequence->set_velocity(pmotiondata->velocity * speed);
	psequence->set_omega(pmotiondata->omega * speed);

	for (DWORD i = 0; i < pmotiondata->num_anims; i++)
		psequence->append_animation(&sub_445200(speed, pmotiondata->anims + i));
}

BOOL CMotionTable::SetDefaultState(MotionState *pstate, CSequence *psequence, DWORD *num_anims)
{
	DWORD default_substate;
	style_defaults.lookup(default_style, &default_substate);

	if (!default_substate)
	{
		return FALSE;
	}

	pstate->clear_modifiers();
	pstate->clear_actions();

	DWORD datakey = (default_style << 16) | (default_substate & 0xFFFFFF);

	MotionData *pMotionData = cycles.lookup(datakey);

	if (!pMotionData)
	{
		return FALSE;
	}

	*num_anims = pMotionData->num_anims - 1;

	pstate->style = default_style;
	pstate->substate = default_substate;
	pstate->substate_mod = 1.0f;

	psequence->clear_physics();
	psequence->clear_animations();

	add_motion(psequence, pMotionData, pstate->substate_mod);
	return TRUE;
}

BOOL CMotionTable::UnPack(BYTE** ppData, ULONG iSize)
{
	// not done.
	BYTE *pOldDataPtr = *ppData;

	Destroy();

	if (iSize < 12)
		goto error_return;

	UNPACK(DWORD, id);
	UNPACK(DWORD, default_style);

	DWORD Unk;
	UNPACK(DWORD, Unk);

	// iSize -= 8;

	for (DWORD i = 0; i < Unk; i++)
	{
		if (iSize < 8)
			goto error_return;

		DWORD Val1, Val2;
		UNPACK(DWORD, Val1);
		UNPACK(DWORD, Val2);
		// iSize -=8;

		style_defaults.add(Val2, Val1);
	}

	if (iSize < 4)
		goto error_return;

	DWORD Unk2;
	UNPACK(DWORD, Unk2);

	for (DWORD i = 0; i < Unk2; i++)
	{
		MotionData *pMotionData = new MotionData;

		BYTE *pOldPos = *ppData;

		if (!UNPACK_POBJ(pMotionData))
			goto error_return;

		iSize -= ((*ppData) - pOldPos);
		cycles.add(pMotionData);
	}

	DWORD var_18;
	UNPACK(DWORD, var_18);

	for (DWORD i = 0; i < var_18; i++)
	{
		MotionData *pMotionData = new MotionData;

		BYTE *pOldPos = *ppData;

		if (!UNPACK_POBJ(pMotionData))
			goto error_return;

		iSize -= ((*ppData) - pOldPos);
		modifiers.add(pMotionData);
	}

	UNPACK(DWORD, var_18);

	for (DWORD i = 0; i < var_18; i++)
	{
		if (iSize < 8)
			goto error_return;

		DWORD var_10, var_8;
		UNPACK(DWORD, var_10);
		UNPACK(DWORD, var_8);
		// iSize -= 8;

		LongHash<MotionData> *pMotionDataTable = new LongHash<MotionData>(256);

		for (DWORD j = 0; j < var_8; j++)
		{
			MotionData *pMotionData = new MotionData;

			BYTE *pOldPos = *ppData;

			if (!UNPACK_POBJ(pMotionData))
				goto error_return;

			iSize -= ((*ppData) - pOldPos);
			pMotionDataTable->add(pMotionData);
		}

		links.add(pMotionDataTable, var_10);
	}

	// some sort of alignment check here? we skipped it.

	PACK_ALIGN();

	return TRUE;

error_return:
	*ppData = pOldDataPtr;
	return FALSE;
}

MotionData *CMotionTable::get_link(
	DWORD somekey, DWORD dwordvalue1, float floatvalue1, DWORD dwordvalue2, float floatvalue2)
{
	LongHash<MotionData> *arg_4;
	MotionData *var_4 = 0;

	if (floatvalue2 < 0.0 || floatvalue1 < 0.0)
	{
		if (links.lookup((somekey << 16) | (dwordvalue2 & 0xFFFFFF), &arg_4))
		{
			var_4 = arg_4->lookup(dwordvalue1);

			if (var_4)
				return var_4;
		}
	}
	else
	{
		if (links.lookup((somekey << 16) | (dwordvalue1 & 0xFFFFFF), &arg_4))
		{
			var_4 = arg_4->lookup(dwordvalue2);

			if (var_4)
				return var_4;
		}
	}

	if (floatvalue2 < 0.0 || floatvalue1 < 0.0)
	{
		DWORD value;
		if (!style_defaults.lookup(somekey, &value) || !links.lookup((somekey << 16) | (dwordvalue1 & 0xFFFFFF), &arg_4))
			return 0;

		return arg_4->lookup(value);
	}
	else
	{
		if (!links.lookup((somekey << 16), &arg_4))
			return 0;

		return arg_4->lookup(dwordvalue2);
	}
}

void CMotionTable::re_modify(CSequence *psequence, MotionState *pstate)
{
	if (!pstate->modifier_head)
		return;

	MotionState state(pstate);

	while (state.modifier_head)
	{
		float arg_4 = pstate->modifier_head->speed_mod;
		DWORD _edi = pstate->modifier_head->motion;

		pstate->remove_modifier(pstate->modifier_head, 0);
		state.remove_modifier(state.modifier_head, 0);

		DWORD unused;
		GetObjectSequence(_edi, pstate, psequence, arg_4, &unused, 0);
	}
}

BOOL CMotionTable::DoObjectMotion(DWORD motionid, MotionState *pstate, CSequence *psequence, float funknown, DWORD *pcounter)
{
	return GetObjectSequence(motionid, pstate, psequence, funknown, pcounter, 0);
}

BOOL CMotionTable::StopObjectMotion(DWORD motionid, float funknown, MotionState *pstate, CSequence *psequence, DWORD *pcounter)
{
	return StopSequenceMotion(motionid, funknown, pstate, psequence, pcounter);
}

BOOL CMotionTable::is_allowed(DWORD motionid, MotionData *pmotiondata, MotionState *pstate)
{
	if (!pmotiondata)
		return FALSE;

	if (pmotiondata->bitfield & 2)
	{
		if (motionid != pstate->substate)
		{
			DWORD value;
			style_defaults.lookup(pstate->style, &value);

			return ((value == pstate->substate) ? TRUE : FALSE);
		}
	}

	return TRUE;
}

BOOL same_sign(float value1, float value2)
{
	if (value1 < 0.0f)
	{
		if (value2 < 0.0f)
			return TRUE;

		return FALSE;
	}
	else
	{
		if (value2 >= 0.0f)
			return TRUE;

		return FALSE;
	}
}

void change_cycle_speed(CSequence *psequence, MotionData *pmotiondata, float unknown1, float unknown2)
{
	if (F_EPSILON < abs(unknown1))
	{
		psequence->multiply_cyclic_animation_framerate(unknown2 / unknown1);
	}
	else
		if (F_EPSILON > abs(unknown2))
		{
			psequence->multiply_cyclic_animation_framerate(0);
		}
}

BOOL CMotionTable::GetObjectSequence(DWORD motionid, MotionState *curr_state, CSequence *sequence, float speed_mod, DWORD *num_anims, DWORD stop_modifiers)
{
	*num_anims = 0;

	if (!curr_state->style || !curr_state->substate)
		return FALSE;

	MotionData *var_10 = 0;
	MotionData *var_4 = 0;

	DWORD mtype2 = curr_state->substate;

	DWORD new_substate;
	style_defaults.lookup(curr_state->style, &new_substate);

	if (motionid == new_substate && !stop_modifiers && (mtype2 & 0x20000000))
		return TRUE;

	if (motionid & 0x80000000)
	{
		//__asm int 3

		if (curr_state->style == motionid)
			return TRUE;

		style_defaults.lookup(curr_state->style, &new_substate);

		if (mtype2 != new_substate)
			var_4 = get_link(curr_state->style, mtype2, curr_state->substate_mod, new_substate, speed_mod);

		DWORD arg_14;
		if (style_defaults.lookup(curr_state->style, &arg_14))
		{
			MotionData *arg_4 = cycles.lookup((motionid << 16) | (arg_14 & 0xFFFFFF));

			if (arg_4)
			{
				if (arg_4->bitfield & 1)
					curr_state->clear_modifiers();

				MotionData *arg_0 = get_link(curr_state->style, new_substate, curr_state->substate_mod, motionid, speed_mod);

				if (!arg_0 && (curr_state->style != motionid))
				{
					arg_0 = get_link(curr_state->style, new_substate, 1.0f, default_style, 1.0f);

					DWORD var_8;
					style_defaults.lookup(default_style, &var_8);
					var_10 = get_link(default_style, var_8, 1.0f, motionid, 1.0f);
				}

				sequence->clear_physics();
				sequence->remove_cyclic_anims();

				add_motion(sequence, var_4, speed_mod);
				add_motion(sequence, arg_0, speed_mod);
				add_motion(sequence, var_10, speed_mod);
				add_motion(sequence, arg_4, speed_mod);

				curr_state->substate = arg_14;
				curr_state->style = motionid;
				curr_state->substate_mod = speed_mod;

				re_modify(sequence, curr_state);

				DWORD someval1 = (var_4 ? var_4->num_anims : 0);
				DWORD someval2 = (arg_0 ? arg_0->num_anims : 0);
				DWORD someval3 = (var_10 ? var_10->num_anims : 0);

				*num_anims = (((DWORD)arg_4->num_anims) + someval3 + someval2 + someval1) - 1;
				return TRUE;
			}
		}
	}

	if (motionid & 0x40000000)
	{
		DWORD arg_0 = motionid & 0xFFFFFF;

		MotionData *pmotiondata = cycles.lookup((curr_state->style << 16) | arg_0);

		if (pmotiondata || (pmotiondata = cycles.lookup((default_style << 16) | arg_0)))
		{
			if (is_allowed(motionid, pmotiondata, curr_state))
			{
				if (motionid == mtype2 && same_sign(speed_mod, curr_state->substate_mod) && sequence->has_anims())
				{
					// DEBUGOUT("Movement alteration1 at %f old: %f new: %f\n", PhysicsTimer::curr_time, curr_state->substate_mod, speed_mod);
					change_cycle_speed(sequence, pmotiondata, curr_state->substate_mod, speed_mod);
					subtract_motion(sequence, pmotiondata, curr_state->substate_mod);
					combine_motion(sequence, pmotiondata, speed_mod);
					curr_state->substate_mod = speed_mod;
					return TRUE;
				}

				if (pmotiondata->bitfield & 1)
					curr_state->clear_modifiers();

				MotionData *pmotiondata2 = get_link(curr_state->style, curr_state->substate, curr_state->substate_mod, motionid, speed_mod);

				if (!pmotiondata2 || !same_sign(speed_mod, curr_state->substate_mod))
				{
					DWORD arg_14;
					style_defaults.lookup(curr_state->style, &arg_14);
					pmotiondata2 = get_link(curr_state->style, curr_state->substate, curr_state->substate_mod, arg_14, 1.0f);
					var_10 = get_link(curr_state->style, arg_14, 1.0f, motionid, speed_mod);
				}

				// DEBUGOUT("Movement alteration2 at %f old: %f new: %f\n", PhysicsTimer::curr_time, curr_state->substate_mod, speed_mod);

				sequence->clear_physics();
				sequence->remove_cyclic_anims();

				if (var_10)
				{
					add_motion(sequence, pmotiondata2, curr_state->substate_mod);
					add_motion(sequence, var_10, speed_mod);
				}
				else
				{
					float arg_14;

					if (curr_state->substate_mod < 0.0 && speed_mod > 0.0)
						arg_14 = -speed_mod;
					else
						arg_14 = speed_mod;

					add_motion(sequence, pmotiondata2, arg_14);
				}

				add_motion(sequence, pmotiondata, speed_mod);

				if ((curr_state->substate != motionid) && (curr_state->substate & 0x20000000))
				{
					DWORD arg_14;
					style_defaults.lookup(curr_state->style, &arg_14);

					if (arg_14 != motionid)
						curr_state->add_modifier_no_check(curr_state->substate, curr_state->substate_mod);
				}

				curr_state->substate_mod = speed_mod;
				curr_state->substate = motionid;
				re_modify(sequence, curr_state);

				DWORD someval1 = (pmotiondata2 ? pmotiondata2->num_anims : 0);
				DWORD someval2 = (var_10 ? var_10->num_anims : 0);

				*num_anims = (((DWORD)pmotiondata->num_anims) + someval2 + someval1) - 1;
				return TRUE;
			}
		}
	}

	if (motionid & 0x10000000) // CM_Action
	{
		DWORD theKey = (mtype2 & 0xFFFFFF) | (curr_state->style << 16);

		MotionData *pmotiondata = cycles.lookup(theKey);

		if (pmotiondata)
		{
			MotionData *motionc = get_link(curr_state->style, mtype2, curr_state->substate_mod, motionid, speed_mod);
			if (motionc)
			{
				curr_state->add_action(motionid, speed_mod);
				sequence->clear_physics();
				sequence->remove_cyclic_anims();
				add_motion(sequence, motionc, speed_mod);
				add_motion(sequence, pmotiondata, curr_state->substate_mod);
				re_modify(sequence, curr_state);
				*num_anims = motionc->num_anims;
				return TRUE;
			}
			else
			{
				style_defaults.lookup(curr_state->style, &new_substate);

				MotionData *motiond = get_link(curr_state->style, mtype2, curr_state->substate_mod, new_substate, 1.0f);
				if (motiond)
				{
					MotionData *link2 = get_link(curr_state->style, new_substate, 1.0f, motionid, speed_mod);
					if (link2)
					{
						MotionData *linkc = cycles.lookup(theKey);
						if (linkc)
						{
							MotionData *pSomeLink = get_link(curr_state->style, new_substate, 1.0f, mtype2, curr_state->substate_mod);
							curr_state->add_action(motionid, speed_mod);
							sequence->clear_physics();
							sequence->remove_cyclic_anims();
							add_motion(sequence, motiond, 1.0f);
							add_motion(sequence, link2, speed_mod);
							add_motion(sequence, pSomeLink, 1.0f);
							add_motion(sequence, linkc, curr_state->substate_mod);
							re_modify(sequence, curr_state);

							DWORD anim_count = motiond->num_anims + link2->num_anims;
							if (pSomeLink)
								anim_count += pSomeLink->num_anims;
							*num_anims = anim_count;
							return TRUE;
						}						
					}
				}
			}
		}
	}

	if (motionid & 0x20000000) // CM_Modifier
	{
		DWORD theKey = curr_state->style << 16;
		MotionData *v24 = cycles.lookup((curr_state->style << 16) | mtype2 & 0xFFFFFF);
		if (v24)
		{
			if (!(v24->bitfield & 1))
			{
				MotionData *someOtherMotion = modifiers.lookup(theKey | (motionid));
				if (someOtherMotion || (someOtherMotion = modifiers.lookup(motionid & 0xFFFFFF)))
				{
					if (!curr_state->add_modifier(motionid, speed_mod))
					{
						StopSequenceMotion(motionid, 1.0f, curr_state, sequence, num_anims);
						if (!curr_state->add_modifier(motionid, speed_mod))
							return FALSE;
					}

					combine_motion(sequence, someOtherMotion, speed_mod);
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}

MotionData::MotionData()
{
	num_anims = 0;
	anims = NULL;
	velocity = Vector(0, 0, 0);
	omega = Vector(0, 0, 0);
	bitfield = 0;
}

MotionData::~MotionData()
{
	Destroy();
}

void MotionData::Destroy()
{
	if (anims)
	{
		delete[] anims;
		anims = NULL;
	}

	num_anims = 0;
}

BOOL MotionData::UnPack(BYTE** ppData, ULONG iSize)
{
	Destroy(); // custom

	UNPACK(DWORD, id);

	UNPACK(BYTE, num_anims);
	UNPACK(BYTE, bitfield);

	BYTE flags;
	UNPACK(BYTE, flags);

	PACK_ALIGN();

	if (num_anims)
	{
		anims = new AnimData[num_anims];
	}

	for (DWORD i = 0; i < num_anims; i++) {
		UNPACK_OBJ(anims[i]);
	}

	if (flags & 1) {
		UNPACK_OBJ(velocity);
	}
	else
		velocity = Vector(0, 0, 0);

	if (flags & 2) {
		UNPACK_OBJ(omega);
	}
	else
		omega = Vector(0, 0, 0);

	return TRUE;
}








