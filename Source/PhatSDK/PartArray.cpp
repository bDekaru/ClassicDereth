
#include <StdAfx.h>
#include "Frame.h"
#include "AnimHooks.h"
#include "Setup.h"
#include "Palette.h"
#include "PhysicsPart.h"
#include "PartArray.h"
#include "GfxObj.h"
#include "Animation.h"
#include "PhysicsObj.h"
#include "ObjCell.h"
#include "Movement.h"
#include "MotionTable.h"
#include "AnimHooks.h"
#include "MovementManager.h"

CSequence::CSequence()
{
	first_cyclic = NULL;
	velocity = Vector(0, 0, 0);
	omega = Vector(0, 0, 0);
	hook_obj = NULL; // 0x28
	frame_number = 0.0;
	curr_anim = NULL;
	placement_frame = NULL;
	placement_frame_id = 0;
	bIsTrivial = 0;
}

CSequence::~CSequence()
{
	if (!anim_list.Empty())
		anim_list.DestroyContents();
}

void CSequence::set_object(CPhysicsObj *pPhysicsObj)
{
	hook_obj = pPhysicsObj;
}

void CSequence::set_placement_frame(AnimFrame *PlacementFrame, uint32_t ID)
{
	placement_frame = PlacementFrame;
	placement_frame_id = ID;
}

AnimFrame *CSequence::get_curr_animframe()
{
	if (!curr_anim)
		return placement_frame;

	return curr_anim->get_part_frame(get_curr_frame_number());
}

int32_t CSequence::get_curr_frame_number()
{
	return((int32_t)((double)floor(frame_number)));
}

void CSequence::multiply_cyclic_animation_framerate(float rate)
{
	AnimSequenceNode *pnode = first_cyclic;

	while (pnode)
	{
		pnode->multiply_framerate(rate);
		pnode = pnode->GetNext();
	}
}

BOOL CSequence::has_anims()
{
	return (anim_list.head_ ? TRUE : FALSE);
}

void CSequence::remove_cyclic_anims()
{
	AnimSequenceNode *pnode = first_cyclic;

	while (pnode)
	{
		if (curr_anim == pnode)
		{
			curr_anim = pnode->GetPrev();

			if (curr_anim)
				frame_number = curr_anim->get_ending_frame();
			else
				frame_number = 0.0;
		}

		AnimSequenceNode *pnext = pnode->GetNext();
		anim_list.RemoveAndDelete(pnode);

		pnode = pnext;
	}

	first_cyclic = (AnimSequenceNode *)anim_list.tail_;
}

void CSequence::remove_all_link_animations()
{
	while (first_cyclic && first_cyclic->GetPrev())
	{
		if (curr_anim == first_cyclic->GetPrev())
		{
			curr_anim = first_cyclic;

			if (curr_anim)
				frame_number = curr_anim->get_starting_frame();
		}

		anim_list.RemoveAndDelete(first_cyclic->GetPrev());
	}
}

void CSequence::remove_link_animations(uint32_t amount)
{
	for (uint32_t i = 0; i < amount; i++)
	{
		if (!first_cyclic->GetPrev())
			return;

		if (curr_anim == first_cyclic->GetPrev())
		{
			curr_anim = first_cyclic;

			if (curr_anim)
				frame_number = curr_anim->get_starting_frame();
		}

		anim_list.RemoveAndDelete(first_cyclic->GetPrev());
	}
}

void CSequence::set_velocity(const Vector& Velocity)
{
	velocity = Velocity;
}

void CSequence::set_omega(const Vector& Omega)
{
	omega = Omega;
}

void CSequence::clear()
{
	clear_animations();
	clear_physics();

	placement_frame = NULL;
	placement_frame_id = 0;
}

void CSequence::clear_animations()
{
	if (!anim_list.Empty())
		anim_list.DestroyContents();

	first_cyclic = NULL;
	frame_number = 0;
	curr_anim = NULL;
}

void CSequence::apply_physics(Frame *pframe, double quantum, double sign)
{
	if (sign >= 0.0)
		quantum = fabs(quantum);
	else
		quantum = -fabs(quantum);

	pframe->m_origin += velocity * quantum;
	pframe->rotate(omega * quantum);
}

void CSequence::apricot()
{
	AnimSequenceNode *pNode = static_cast<AnimSequenceNode *>(anim_list.head_);

	while (pNode != curr_anim)
	{
		if (pNode == first_cyclic)
			break;

		anim_list.Remove(static_cast<DLListData *>(pNode));

		if (pNode)
			delete pNode;

		pNode = static_cast<AnimSequenceNode *>(anim_list.head_);
	}
}

void CSequence::update(double time_elapsed, Frame* pframe)
{
	if (!anim_list.Empty())
	{
		update_internal(time_elapsed, &curr_anim, &frame_number, pframe);
		apricot();
	}
	else
	{
		if (pframe)
			apply_physics(pframe, time_elapsed, time_elapsed);
	}
}

void CSequence::update_internal(double time_elapsed, AnimSequenceNode** ppanim, double *pframenum, Frame *pframe)
{
iterate_anim:

	double framerate = (*ppanim)->get_framerate();
	double frametime = framerate * time_elapsed;

	int32_t lastframe = (int32_t)floor(*pframenum);

	double newframenum = (*pframenum) + frametime;
	*pframenum = newframenum;

	double var_10 = 0;
	int32_t var_28 = 0;

	if (frametime > 0.0)
	{
		if (double((*ppanim)->get_high_frame()) < floor(newframenum))
		{
			double whatever = (*pframenum) - (*ppanim)->get_high_frame() - 1.0;

			if (whatever < 0)
				whatever = 0.0;

			if (F_EPSILON < fabs(framerate))
				var_10 = whatever / framerate;
			else
				var_10 = 0;

			var_28 = 1;
			*pframenum = (*ppanim)->get_high_frame();
		}

		while (floor(*pframenum) > lastframe)
		{
			if (pframe)
			{
				if ((*ppanim)->anim->pos_frames)
					pframe->combine(pframe, (*ppanim)->get_pos_frame(lastframe));

				if (F_EPSILON < fabs(framerate))
					apply_physics(pframe, 1.0 / framerate, time_elapsed);
			}

			execute_hooks((*ppanim)->get_part_frame(lastframe), 1);

			lastframe++;
		}
	}
	else if (frametime < 0.0)
	{
		if (double((*ppanim)->get_low_frame()) > floor(newframenum))
		{
			double whatever = (*pframenum) - (*ppanim)->get_low_frame();

			if (whatever > 0)
				whatever = 0.0;

			if (F_EPSILON < fabs(framerate))
				var_10 = whatever / framerate;
			else
				var_10 = 0;

			var_28 = 1;
			*pframenum = (*ppanim)->get_low_frame();
		}

		while (floor(*pframenum) < lastframe)
		{
			if (pframe)
			{
				if ((*ppanim)->anim->pos_frames)
					pframe->subtract1(pframe, (*ppanim)->get_pos_frame(lastframe));

				if (F_EPSILON < fabs(framerate))
					apply_physics(pframe, 1.0 / framerate, time_elapsed);
			}

			execute_hooks((*ppanim)->get_part_frame(lastframe), -1);

			lastframe--;
		}
	}
	else
	{
		if (pframe)
		{
			if (F_EPSILON < fabs(time_elapsed))
				apply_physics(pframe, time_elapsed, time_elapsed);
		}
	}

	if (var_28)
	{
		if (hook_obj)
		{
			AnimSequenceNode *pNode = static_cast<AnimSequenceNode *>(anim_list.head_);

			if (pNode != first_cyclic)
			{
				static AnimDoneHook anim_done_hook;
				hook_obj->add_anim_hook(&anim_done_hook);
			}
		}

		advance_to_next_animation(time_elapsed, ppanim, pframenum, pframe);
		time_elapsed = var_10;

		goto iterate_anim;
	}
}

void CSequence::execute_hooks(AnimFrame *pAnimFrame, int dir)
{
	if (hook_obj)
	{
		CAnimHook *pHook = pAnimFrame->hooks;

		while (pHook)
		{
			if (pHook->direction_ == CAnimHook::BOTH_ANIMHOOK || pHook->direction_ == dir)
				hook_obj->add_anim_hook(pHook);

			pHook = pHook->next_hook;
		}
	}
}

void CSequence::advance_to_next_animation(double TimeElapsed, AnimSequenceNode** ppAnim, double *pFrameNum, Frame *pFrame)
{
	if (TimeElapsed >= 0.0)
	{
		if ((*ppAnim)->get_framerate() < 0.0f)
		{
			if (pFrame)
			{
				if ((*ppAnim)->anim->pos_frames)
					pFrame->subtract1(pFrame, (*ppAnim)->get_pos_frame((int32_t)floor(*pFrameNum)));

				if (F_EPSILON < fabs((*ppAnim)->get_framerate()))
					apply_physics(pFrame, 1.0 / (*ppAnim)->get_framerate(), TimeElapsed);
			}
		}

		if ((*ppAnim)->GetNext())
			*ppAnim = (*ppAnim)->GetNext();
		else
			*ppAnim = first_cyclic;

		*pFrameNum = (*ppAnim)->get_starting_frame();

		if ((*ppAnim)->get_framerate() > 0.0f)
		{
			if (pFrame)
			{
				if ((*ppAnim)->anim->pos_frames)
					pFrame->combine(pFrame, (*ppAnim)->get_pos_frame((int32_t)floor(*pFrameNum)));

				if (F_EPSILON < fabs((*ppAnim)->get_framerate()))
					apply_physics(pFrame, 1.0 / (*ppAnim)->get_framerate(), TimeElapsed);
			}
		}
	}
	else
	{
		if ((*ppAnim)->get_framerate() >= 0.0f)
		{
			if (pFrame)
			{
				if ((*ppAnim)->anim->pos_frames)
					pFrame->subtract1(pFrame, (*ppAnim)->get_pos_frame((int32_t)floor(*pFrameNum)));

				if (F_EPSILON < fabs((*ppAnim)->get_framerate()))
					apply_physics(pFrame, 1.0 / (*ppAnim)->get_framerate(), TimeElapsed);
			}
		}

		if ((*ppAnim)->GetPrev())
			*ppAnim = (*ppAnim)->GetPrev();
		else
			*ppAnim = static_cast<AnimSequenceNode *>(anim_list.tail_);

		*pFrameNum = (*ppAnim)->get_ending_frame();

		if ((*ppAnim)->get_framerate() < 0.0f)
		{
			if (pFrame)
			{
				if ((*ppAnim)->anim->pos_frames)
					pFrame->combine(pFrame, (*ppAnim)->get_pos_frame((int32_t)floor(*pFrameNum)));

				if (F_EPSILON < fabs((*ppAnim)->get_framerate()))
					apply_physics(pFrame, 1.0 / (*ppAnim)->get_framerate(), TimeElapsed);
			}
		}
	}
}

void CSequence::combine_physics(const Vector& Velocity, const Vector& Omega)
{
	velocity += Velocity;
	omega += Omega;
}

void CSequence::subtract_physics(const Vector& Velocity, const Vector& Omega)
{
	velocity -= Velocity;
	omega -= Omega;
}

void CSequence::clear_physics()
{
	velocity = Vector(0, 0, 0);
	omega = Vector(0, 0, 0);
}

void CSequence::append_animation(AnimData *pAnimData)
{
	AnimSequenceNode *pNode = new AnimSequenceNode(pAnimData);

	if (!pNode->has_anim())
	{
		// Useless null check performed by the client.
		if (pNode)
			delete pNode;

		return;
	}

	DLListData *pListNode = static_cast<DLListData *>(pNode);

	anim_list.InsertAfter(pListNode, anim_list.tail_);
	first_cyclic = (AnimSequenceNode *)anim_list.tail_;

	if (!curr_anim)
	{
		curr_anim = static_cast<AnimSequenceNode *>(anim_list.head_);
		frame_number = curr_anim->get_starting_frame();
	}
}

CPartArray::CPartArray()
{
	pa_state = 0;
	owner = NULL;
	motion_table_manager = NULL;
	setup = NULL;
	num_parts = 0;
	parts = NULL;
	scale = Vector(1.0f, 1.0f, 1.0f);
	pals = NULL;
	lights = NULL;
	last_animframe = 0;
}

CPartArray::~CPartArray()
{
	Destroy();
}

void CPartArray::Destroy()
{
	if (motion_table_manager)
	{
		delete motion_table_manager;
		motion_table_manager = NULL;
	}

	DestroyPals();
	DestroyLights();
	DestroyParts();
	DestroySetup();

	sequence.set_object(NULL);

	pa_state = 0;
	owner = NULL;
}

CPartArray *CPartArray::CreateMesh(CPhysicsObj *pPhysicsObj, uint32_t ID)
{
	CPartArray *pPartArray = new CPartArray();

	pPartArray->owner = pPhysicsObj;
	pPartArray->sequence.set_object(pPhysicsObj);

	if (!pPartArray->SetMeshID(ID))
	{
		delete pPartArray;

		return NULL;
	}

	pPartArray->SetPlacementFrame(0x65);
	return pPartArray;
}

CPartArray *CPartArray::CreateSetup(CPhysicsObj *_owner, uint32_t setup_did, BOOL bCreateParts)
{
	CPartArray *pPartArray = new CPartArray();

	pPartArray->owner = _owner;
	pPartArray->sequence.set_object(_owner);

	if (!pPartArray->SetSetupID(setup_did, bCreateParts))
	{
		delete pPartArray;
		return NULL;
	}

	pPartArray->SetPlacementFrame(0x65);
	return pPartArray;
}

CPartArray *CPartArray::CreateParticle(CPhysicsObj *_owner, uint32_t _num_parts, CSphere *sorting_sphere)
{
	CPartArray *pPartArray = new CPartArray();

	pPartArray->owner = _owner;
	pPartArray->sequence.set_object(_owner);

	pPartArray->setup = CSetup::makeParticleSetup(_num_parts, sorting_sphere);

	if (!pPartArray->setup || !pPartArray->InitParts())
	{
		delete pPartArray;
		return NULL;
	}

	return pPartArray;
}

void CPartArray::AddLightsToCell(CObjCell *pCell)
{
	if (!pCell || !lights)
		return;

	for (uint32_t i = 0; i < lights->num_lights; i++)
		pCell->add_light(&lights->m_Lights[i]);
}

void CPartArray::RemoveLightsFromCell(CObjCell *pCell)
{
	if (!pCell || !lights)
		return;

	for (uint32_t i = 0; i < lights->num_lights; i++)
		pCell->remove_light(&lights->m_Lights[i]);
}

void CPartArray::SetCellID(uint32_t ID)
{
	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i])
			parts[i]->pos.objcell_id = ID;
	}
}

BOOL CPartArray::SetMeshID(uint32_t ID)
{
	if (!ID)
		return FALSE;

	CSetup *pSetup = CSetup::makeSimpleSetup(ID);

	if (!pSetup)
		return FALSE;

	DestroyParts();
	DestroySetup();

	setup = pSetup;

	if (!InitParts())
		return FALSE;

	return TRUE;
}

BOOL CPartArray::SetSetupID(uint32_t ID, BOOL bCreateParts)
{
	if (setup && setup->GetID() == ID)
		return TRUE;

	CSetup *pSetup = CSetup::Get(ID);

	if (!pSetup)
		return FALSE;

	DestroyPals();
	DestroyLights();
	DestroyParts();
	DestroySetup();

	setup = pSetup;

	if (bCreateParts)
	{
		if (!InitParts())
			return FALSE;
	}

	InitLights();
	InitDefaults();

	return TRUE;
}

void CPartArray::DestroyParts()
{
	if (parts)
	{
		for (uint32_t i = 0; i < num_parts; i++)
		{
			if (parts[i])
				delete parts[i];
		}

		delete[] parts;
		parts = NULL;
	}
	num_parts = 0;
}

void CPartArray::DestroySetup()
{
	if (setup)
	{
		if (setup->GetID())
			CSetup::Release(setup);
		else
			delete setup;

		setup = NULL;
	}
}

void CPartArray::DestroyPals()
{
	if (pals)
	{
#if PHATSDK_RENDER_AVAILABLE
		for (uint32_t i = 0; i < num_parts; i++)
		{
			if (pals[i])
			{
				Palette::releasePalette(pals[i]);
				pals[i] = NULL;
			}
		}
#endif

		delete[] pals;
		pals = NULL;
	}
}

void CPartArray::DestroyLights()
{
	if (!owner || !lights)
		return;

	RemoveLightsFromCell(owner->cell);

	if (lights)
		delete lights;

	lights = NULL;
}

BOOL CPartArray::InitLights()
{
	if (owner && (setup->num_lights > 0))
	{
		lights = new LIGHTLIST(setup->num_lights);

		for (uint32_t i = 0; i < lights->num_lights; i++)
		{
			// Init the lights
			lights->m_Lights[i].lightinfo = &setup->lights[i];

			if (owner->m_PhysicsState & STATIC_PS)
				lights->m_Lights[i].state |= 1;
		}

		AddLightsToCell(owner->cell);
	}

	return TRUE;
}

void CPartArray::InitDefaults()
{
	if (setup->default_anim_id)
	{
		sequence.clear_animations();

		AnimData DefaultAnim;
		DefaultAnim.anim_id = setup->default_anim_id;
		DefaultAnim.low_frame = 0;
		DefaultAnim.high_frame = 0xFFFFFFFF;
		DefaultAnim.framerate = 30.0f;

		sequence.append_animation(&DefaultAnim);
	}

	if (owner)
		owner->InitDefaults(setup);
}

BOOL CPartArray::InitParts()
{
	num_parts = setup->num_parts;

	if (!num_parts)
		return FALSE;

	parts = new CPhysicsPart*[num_parts];

	if (!parts)
		return FALSE;

	for (uint32_t i = 0; i < num_parts; i++)
		parts[i] = NULL;

	if (setup->parts)
	{
		uint32_t i;

		for (i = 0; i < num_parts; i++)
		{
			parts[i] = CPhysicsPart::makePhysicsPart(setup->parts[i]);

			if (!parts[i])
				break;
		}

		if (i != num_parts)
			return FALSE;

		for (i = 0; i < num_parts; i++)
		{
			parts[i]->physobj = owner;
			parts[i]->physobj_index = i;
		}

		if (setup->default_scale)
		{
			for (i = 0; i < num_parts; i++)
				parts[i]->gfxobj_scale = setup->default_scale[i];
		}
	}

	return TRUE;
}

int CPartArray::GetPlacementFrameID() // custom
{
	return sequence.placement_frame_id;
}

int CPartArray::GetActivePlacementFrameID() // custom
{
	return sequence.curr_anim ? 0 : sequence.placement_frame_id;
}

BOOL CPartArray::SetPlacementFrame(uint32_t ID)
{
	PlacementType *pt;

	pt = setup->placement_frames.lookup(ID);

	if (!pt)
	{
		pt = setup->placement_frames.lookup(0);

		if (!pt)
		{
			sequence.set_placement_frame(0, 0);
			return FALSE;
		}
		else
			sequence.set_placement_frame(&pt->m_AnimFrame, 0);
	}
	else
		sequence.set_placement_frame(&pt->m_AnimFrame, ID);

	return TRUE;
}

void CPartArray::SetFrame(Frame *pFrame)
{
	UpdateParts(pFrame);

#if PHATSDK_RENDER_AVAILABLE
	if (lights)
		lights->set_frame(pFrame);
#endif
}

void CPartArray::SetTranslucencyInternal(float Amount)
{
	if (setup)
	{
		for (uint32_t i = 0; i < num_parts; i++)
		{
			CPhysicsPart *pPart = parts[i];

			if (pPart)
				pPart->SetTranslucency(Amount);
		}
	}
}

BOOL CPartArray::CacheHasPhysicsBSP()
{
	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i]->gfxobj[0]->physics_bsp)
		{
			pa_state |= 0x10000;
			return TRUE;
		}
	}

	pa_state &= ~(0x10000UL);
	return FALSE;
}

BOOL CPartArray::AllowsFreeHeading()
{
	return(setup->allow_free_heading);
}

float CPartArray::GetHeight() const
{
	return(setup->height * scale.z);
}

float CPartArray::GetRadius() const
{
	return(setup->radius * scale.z);
}

void CPartArray::UpdateParts(Frame *pFrame)
{
#if 1 // !PHATSDK_IS_SERVER // major performance hog on server
	AnimFrame *CurrFrame = sequence.get_curr_animframe();

	if (!CurrFrame)
		return;

	uint32_t PartCount = min(CurrFrame->num_parts, num_parts);

	for (uint32_t i = 0; i < PartCount; i++)
		parts[i]->pos.frame.combine(pFrame, &CurrFrame->frame[i], &scale);
#endif 
}

void CPartArray::Update(float fTimeElapsed, Frame *pFrame)
{
	sequence.update(fTimeElapsed, pFrame);
}

#if PHATSDK_RENDER_AVAILABLE
void CPartArray::UpdateViewerDistance(void)
{
	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i])
			parts[i]->UpdateViewerDistance();
	}
}
#endif

void CPartArray::SetNoDrawInternal(BOOL NoDraw)
{
	if (!setup)
		return;

	for (uint32_t i = 0; i < num_parts; i++)
	{
		CPhysicsPart *pPart = parts[i];

		if (pPart)
			pPart->SetNoDraw(NoDraw);
	}
}

#if PHATSDK_RENDER_AVAILABLE
void CPartArray::Draw(Position *Pos)
{
	for (uint32_t i = 0; i < num_parts; i++)
	{
		CPhysicsPart *pPart = parts[i];

		if (pPart)
			pPart->Draw(FALSE);
	}
}
#endif

uint32_t CPartArray::GetDataID(void)
{
	if (setup->GetID())
	{
		// Return Setup ID.
		return setup->GetID();
	}
	else
		if (num_parts == 1)
		{
			// Return Mesh ID.
			return parts[0]->gfxobj[0]->GetID();
		}
		else
			return 0;
}

uint32_t CPartArray::GetNumSphere()
{
	return setup->num_sphere;
}

void CPartArray::AnimationDone(BOOL success)
{
	if (motion_table_manager)
		motion_table_manager->AnimationDone(success);
}

void CPartArray::HandleMovement()
{
	if (motion_table_manager)
		motion_table_manager->UseTime();
}

void CPartArray::CheckForCompletedMotions()
{
	if (motion_table_manager)
		motion_table_manager->CheckForCompletedMotions();
}

void CPartArray::HandleEnterWorld()
{
	if (motion_table_manager)
		motion_table_manager->HandleEnterWorld(&sequence);
}

void CPartArray::HandleExitWorld()
{
	if (motion_table_manager)
		motion_table_manager->HandleExitWorld();
}

uint32_t CPartArray::DoInterpretedMotion(uint32_t mid, MovementParameters *params)
{
	if (!motion_table_manager)
		return 7;

	MovementStruct cmd;
	cmd.type = MovementTypes::InterpretedCommand;
	cmd.motion = mid;
	cmd.params = params;

	return motion_table_manager->PerformMovement(cmd, &sequence);
}

uint32_t CPartArray::StopInterpretedMotion(uint32_t mid, MovementParameters *params)
{
	if (!motion_table_manager)
		return 7;

	MovementStruct cmd;
	cmd.type = MovementTypes::StopInterpretedCommand;
	cmd.motion = mid;
	cmd.params = params;

	return motion_table_manager->PerformMovement(cmd, &sequence);
}

uint32_t CPartArray::StopCompletely_Internal()
{
	if (!motion_table_manager)
		return 7;

	MovementStruct cmd;
	cmd.type = MovementTypes::StopCompletely;

	return motion_table_manager->PerformMovement(cmd, &sequence);
}

void CPartArray::InitializeMotionTables()
{
	if (motion_table_manager)
		motion_table_manager->initialize_state(&sequence);
}

uint32_t CPartArray::GetSetupID()
{
	if (setup)
		return setup->GetID();

	return 0;
}

uint32_t CPartArray::GetMotionTableID()
{
	if (motion_table_manager)
		return motion_table_manager->GetMotionTableID();

	return 0;
}

BOOL CPartArray::SetMotionTableID(uint32_t ID)
{
	if (motion_table_manager)
	{
		if (ID == motion_table_manager->GetMotionTableID())
			return TRUE;

		if (motion_table_manager)
		{
			delete motion_table_manager;
			motion_table_manager = NULL;
		}
	}

	if (ID)
	{
		motion_table_manager = MotionTableManager::Create(ID);

		if (!motion_table_manager)
			return FALSE;

		motion_table_manager->SetPhysicsObject(owner);
	}

	return TRUE;
}

LIGHTLIST::LIGHTLIST(uint32_t LightCount)
{
	num_lights = LightCount;
	m_Lights = new LIGHTOBJ[LightCount];
}

LIGHTLIST::~LIGHTLIST()
{
	delete[] m_Lights;
}

void LIGHTLIST::set_frame(Frame *pFrame)
{
	for (uint32_t i = 0; i < num_lights; i++)
		m_Lights[i].global_offset = *pFrame;
}

LIGHTOBJ::LIGHTOBJ() : lightinfo(NULL)
{
	state = 0;
}

LIGHTOBJ::~LIGHTOBJ()
{
	// Finish me?
}

CSphere *CPartArray::GetSphere()
{
	return setup->sphere;
}

uint32_t CPartArray::GetNumCylsphere()
{
	return setup->num_cylsphere;
}

CCylSphere *CPartArray::GetCylsphere()
{
	return setup->cylsphere;
}

float CPartArray::GetStepDownHeight()
{
	if (setup)
		return setup->step_down_height * scale.z;

	return 0.01f;
}

float CPartArray::GetStepUpHeight()
{
	if (setup)
		return setup->step_up_height * scale.z;

	return 0.01f;
}

TransitionState CPartArray::FindObjCollisions(class CTransition *transition)
{
	TransitionState ts = OK_TS;

	for (uint32_t i = 0; i < num_parts; i++)
	{
		CPhysicsPart *part = parts[i];
		if (part)
		{
			ts = part->find_obj_collisions(transition);
			if (ts != OK_TS)
				break;
		}
	}

	return ts;
}

void CPartArray::RemoveParts(CObjCell *obj_cell)
{
	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i])
			obj_cell->remove_part(parts[i]);
	}
}

CSphere *CPartArray::GetSortingSphere()
{
	if (setup)
	{
		return &setup->sorting_sphere;
	}
	else
	{
		static CSphere default_sorting_sphere;
		return &default_sorting_sphere;
	}
}

void CPartArray::AddPartsShadow(CObjCell *obj_cell, unsigned int num_shadow_parts)
{
	ClipPlaneList **clip_planes_ = NULL;

	if (num_shadow_parts > 1)
		clip_planes_ = obj_cell->clip_planes;

	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i])
			obj_cell->add_part(parts[i], clip_planes_, &obj_cell->pos.frame, num_shadow_parts);
	}
}

void CPartArray::calc_cross_cells_static(CObjCell *cell, CELLARRAY *cell_array)
{
	cell->find_transit_cells(num_parts, parts, cell_array);
}

BOOL CPartArray::HasAnims()
{
	return sequence.has_anims();
}

BOOL CPartArray::SetScaleInternal(Vector &new_scale)
{
	scale = new_scale;

	for (uint32_t i = 0; i < num_parts; i++)
	{
		if (parts[i])
		{
			if (setup->default_scale)
				parts[i]->gfxobj_scale = setup->default_scale[i] * scale;
			else
				parts[i]->gfxobj_scale = scale;
		}
	}

	return TRUE;
}

