#include <StdAfx.h>
#include "PhysicsObj.h"
#include "AnimHooks.h"

AnimFrame::AnimFrame()
{
    num_parts = 0;
    frame = NULL;

    num_frame_hooks = 0;
    hooks = NULL;
}

AnimFrame::~AnimFrame()
{
    Destroy();
}

void AnimFrame::Destroy()
{
    // Destroy Frames
    if (frame)
    {
        delete [] frame;
        frame = NULL;
    }

    // Destroy Anim Hooks
    while (hooks)
    {
        CAnimHook *pNext = hooks->next_hook;
        delete hooks;
        hooks = pNext;
    }

    hooks = NULL;
}

BOOL AnimFrame::UnPack(uint32_t ObjCount, BYTE **ppData, ULONG iSize)
{
    Destroy();

    num_parts = ObjCount;
    frame = new AFrame[ ObjCount ];

    for (uint32_t i = 0; i < num_parts; i++)
        UNPACK_OBJ_READER(frame[i]);
    
    UNPACK(uint32_t, num_frame_hooks);

    for (uint32_t i = 0; i < num_frame_hooks; i++)
        CAnimHook::UnPackHook(ppData, iSize)->add_to_list(&hooks);

    return TRUE;
}

CAnimHook::CAnimHook()
{
	next_hook = NULL;
	direction_ = AnimHookDir::UNKNOWN_ANIMHOOK;
}

CAnimHook::~CAnimHook()
{
}

CAnimHook* CAnimHook::UnPackHook(BYTE** ppData, ULONG iSize)
{
    int hook_type;
	int hook_dir;

    UNPACK(int, hook_type);
    UNPACK(int, hook_dir);

    CAnimHook *pHook = NULL;

    // DEBUGOUT("Unpacking hook %u\r\n", hook_type);

    switch (hook_type)
    {
    case 0:
        pHook = new NOOPHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 1:
        pHook = new SoundHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 2:
        pHook = new SoundTableHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 3:
        pHook = new AttackHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 4:
        pHook = new AnimDoneHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 5:
        pHook = new ReplaceObjectHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 6:
        pHook = new EtherealHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 7:
        pHook = new TransparentPartHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 8:
        pHook = new LuminousHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 9:
        pHook = new LuminousPartHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 10:
        pHook = new DiffuseHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 11:
        pHook = new DiffusePartHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 12:
        pHook = new ScaleHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 13:
        pHook = new CreateParticleHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 14:
        pHook = new DestroyParticleHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 15:
        pHook = new StopParticleHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 16:
        pHook = new NoDrawHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 17:
        pHook = new DefaultScriptHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 18:
        pHook = new DefaultScriptPartHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 19:
        pHook = new CallPESHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 20:
        pHook = new TransparentHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 21:
        pHook = new SoundTweakedHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 22:
        pHook = new SetOmegaHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 23:
        pHook = new TextureVelocityHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 24:
        pHook = new TextureVelocityPartHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 25:
        pHook = new SetLightHook();
        pHook->UnPack(ppData, iSize);
        break;
    case 26:
        pHook = new CreateBlockingParticleHook();
        pHook->UnPack(ppData, iSize);
        break;
    default:
        goto UnknownHook;
    }

    pHook->direction_ = (AnimHookDir) hook_dir;

UnknownHook:
    PACK_ALIGN();

    return pHook;
}

void CAnimHook::GetDataRecursion(LPVOID lpVoid)
{
}

void CAnimHook::add_to_list(CAnimHook** pList)
{
    CAnimHook *pNode = *pList;

    if (!pNode)
    {
        // We are the first element.
        *pList = this;
        return;
    }

    // Iterate through all the nodes.
    while(pNode->next_hook)
        pNode = pNode->next_hook;

    // Append on last element.
    pNode->next_hook = this;
}

//=============================================================================
// NOOPHook
//=============================================================================

NOOPHook::NOOPHook()
{
}

void NOOPHook::Execute(CPhysicsObj *pOwner)
{
}

int NOOPHook::GetType()
{
    return 0;
}

ULONG NOOPHook::pack_size()
{
    return 0;
}

ULONG NOOPHook::Pack(BYTE** ppData, ULONG iSize)
{
    return 0;
}

BOOL NOOPHook::UnPack(BYTE** ppData, ULONG iSize)
{
    return TRUE;
}

//=============================================================================
// NoDrawHook
//=============================================================================

NoDrawHook::NoDrawHook()
{
}

void NoDrawHook::Execute(CPhysicsObj *pOwner)
{
    pOwner->set_nodraw(_no_draw, 0);
}

int NoDrawHook::GetType()
{
    return 16;
}

ULONG NoDrawHook::pack_size()
{
    return sizeof(BOOL);
}

ULONG NoDrawHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(BOOL, _no_draw);
    }

    return PackSize;
}

BOOL NoDrawHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(BOOL, _no_draw);
    return TRUE;
}

//=============================================================================
// DefaultScriptHook
//=============================================================================

DefaultScriptHook::DefaultScriptHook()
{
}

void DefaultScriptHook::Execute(CPhysicsObj *pOwner) {
    // UNFINISHED_LEGACY("DefaultScriptHook::Execute");
}

int DefaultScriptHook::GetType()
{
    return 17;
}

ULONG DefaultScriptHook::pack_size()
{
    return 0;
}

ULONG DefaultScriptHook::Pack(BYTE** ppData, ULONG iSize)
{
    return 0;
}

BOOL DefaultScriptHook::UnPack(BYTE** ppData, ULONG iSize)
{
    return TRUE;
}

//=============================================================================
// DefaultScriptPartHook
//=============================================================================

DefaultScriptPartHook::DefaultScriptPartHook()
{
}

void DefaultScriptPartHook::Execute(CPhysicsObj *pOwner) {
    // UNFINISHED_LEGACY("DefaultScriptPartHook::Execute");
}

int DefaultScriptPartHook::GetType()
{
    return 18;
}

ULONG DefaultScriptPartHook::pack_size()
{
    return sizeof(uint32_t);
}

ULONG DefaultScriptPartHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, _part_index);
    }

    return PackSize;
}

BOOL DefaultScriptPartHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, _part_index);
    return TRUE;
}

//=============================================================================
// SoundHook
//=============================================================================

SoundHook::SoundHook()
{
    gid_ = 0;
}

void SoundHook::Execute(CPhysicsObj *pOwner) {
    // DEBUGOUT("Unfinished SoundHook::Execute\r\n"); // __asm int 3;
}

int SoundHook::GetType()
{
    return 1;
}

void SoundHook::GetDataRecursion(LPVOID)
{
    // UNFINISHED_LEGACY("SoundHook::GetDataRecursion");
}

ULONG SoundHook::pack_size()
{
    return sizeof(uint32_t);
}

ULONG SoundHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    PACK(uint32_t, gid_);

    return PackSize;
}

BOOL SoundHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, gid_);

    // SoundManager::CreateSound(m_dwSoundID);
	// TODO
    // UNFINISHED_LEGACY("SoundManager::CreateSound(m_dwSoundID)");

    return TRUE;
}

//=============================================================================
// SoundTableHook
//=============================================================================

SoundTableHook::SoundTableHook()
{
    sound_type_ = 0;
}

void SoundTableHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("SoundTableHook::Execute");
}

int SoundTableHook::GetType()
{
    return 2;
}

ULONG SoundTableHook::pack_size()
{
    return sizeof(uint32_t);
}

ULONG SoundTableHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    PACK(uint32_t, sound_type_);

    return PackSize;
}

BOOL SoundTableHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, sound_type_);

    return TRUE;
}

//=============================================================================
// AnimDoneHook
//=============================================================================

AnimDoneHook::AnimDoneHook()
{
}

void AnimDoneHook::Execute(CPhysicsObj *pOwner)
{
    pOwner->Hook_AnimDone();
}

int AnimDoneHook::GetType()
{
    return 4;
}

ULONG AnimDoneHook::pack_size()
{
    return 0;
}

ULONG AnimDoneHook::Pack(BYTE** ppData, ULONG iSize)
{
    return 0;
}

BOOL AnimDoneHook::UnPack(BYTE** ppData, ULONG iSize)
{
    return TRUE;
}

//=============================================================================
//ReplaceObjectHook
//=============================================================================

ReplaceObjectHook::ReplaceObjectHook()
{
}

ReplaceObjectHook::~ReplaceObjectHook()
{
}

void ReplaceObjectHook::Execute(CPhysicsObj *pOwner)
{
}

int ReplaceObjectHook::GetType()
{
    return 5;
}

void ReplaceObjectHook::GetDataRecursion(LPVOID) {
    // UNFINISHED_LEGACY("ReplaceObjectHook::GetDataRecursion");
}

ULONG ReplaceObjectHook::pack_size()
{
    return ap_change.pack_size();
}

ULONG ReplaceObjectHook::Pack(BYTE** ppData, ULONG iSize)
{
	// UNFINISHED();
    // return PACK_OBJ(ap_change);
	return 0;
}

BOOL ReplaceObjectHook::UnPack(BYTE** ppData, ULONG iSize)
{
    return UNPACK_OBJ(ap_change);
}


//=============================================================================
//TransparentHook
//=============================================================================

TransparentHook::TransparentHook()
{
}

void TransparentHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("TransparentHook::Execute");
}

int TransparentHook::GetType()
{
    return 20;
}

ULONG TransparentHook::pack_size()
{
    return(sizeof(float) * 3);
}

ULONG TransparentHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL TransparentHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// TransparentPartHook
//=============================================================================

TransparentPartHook::TransparentPartHook()
{
}

void TransparentPartHook::Execute(CPhysicsObj *pOwner)
{
	// UNFINISHED_WARNING_LEGACY("TransparentPartHook::Execute");
}

int TransparentPartHook::GetType()
{
    return 7;
}

ULONG TransparentPartHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*3);
}

ULONG TransparentPartHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, part);
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL TransparentPartHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, part);
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// LuminousPartHook
//=============================================================================

LuminousPartHook::LuminousPartHook()
{
}

void LuminousPartHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("LuminousPartHook::Execute");
}

int LuminousPartHook::GetType()
{
    return 9;
}

ULONG LuminousPartHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*3);
}

ULONG LuminousPartHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, part);
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL LuminousPartHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, part);
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// LuminousHook
//=============================================================================

LuminousHook::LuminousHook()
{
}

void LuminousHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("LuminousHook::Execute");
}

int LuminousHook::GetType()
{
    return 8;
}

ULONG LuminousHook::pack_size()
{
    return(sizeof(float)*3);
}

ULONG LuminousHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL LuminousHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// DiffusePartHook
//=============================================================================

DiffusePartHook::DiffusePartHook()
{
}

void DiffusePartHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("DiffusePartHook::Execute");
}

int DiffusePartHook::GetType()
{
    return 11;
}

ULONG DiffusePartHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*3);
}

ULONG DiffusePartHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, part);
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL DiffusePartHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, part);
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// DiffuseHook
//=============================================================================

DiffuseHook::DiffuseHook()
{
}

void DiffuseHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("DiffuseHook::Execute");
}

int DiffuseHook::GetType()
{
    return 10;
}

ULONG DiffuseHook::pack_size()
{
    return(sizeof(float)*3);
}

ULONG DiffuseHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(float, start);
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL DiffuseHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(float, start);
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// ScaleHook
//=============================================================================

ScaleHook::ScaleHook()
{
}

void ScaleHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("ScaleHook::Execute");
}

int ScaleHook::GetType()
{
    return 12;
}

ULONG ScaleHook::pack_size()
{
    return(sizeof(float)*2);
}

ULONG ScaleHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(float, end);
        PACK(float, time);
    }

    return PackSize;
}

BOOL ScaleHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(float, end);
    UNPACK(float, time);

    return TRUE;
}

//=============================================================================
// CreateParticleHook
//=============================================================================

CreateParticleHook::CreateParticleHook()
{
}

void CreateParticleHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_RENDER_AVAILABLE
    pOwner->create_particle_emitter(emitter_info_id, part_index, &offset, emitter_id);
#endif
}

int CreateParticleHook::GetType()
{
    return 13;
}

void CreateParticleHook::GetDataRecursion(LPVOID)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("CreateParticleHook::GetDataRecursion");
#endif
}

ULONG CreateParticleHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(int32_t) + offset.pack_size() + sizeof(uint32_t));
}

ULONG CreateParticleHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, emitter_info_id);
        PACK(uint32_t, part_index);
        PACK_OBJ_WRITER(offset);
        PACK(uint32_t, emitter_id);
    }

    return PackSize;
}

BOOL CreateParticleHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, emitter_info_id);
    UNPACK(uint32_t, part_index);
    UNPACK_OBJ_READER(offset);
    UNPACK(uint32_t, emitter_id);

    return TRUE;
}

//=============================================================================
// CreateBlockingParticleHook
//=============================================================================

CreateBlockingParticleHook::CreateBlockingParticleHook()
{
}

void CreateBlockingParticleHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("CreateBlockingParticleHook::Execute");
#endif
}

int CreateBlockingParticleHook::GetType()
{
    return 26;
}

//=============================================================================
// DestroyParticleHook
//=============================================================================

DestroyParticleHook::DestroyParticleHook()
{
}

void DestroyParticleHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("DestroyParticleHook::Execute");
#endif
}

int DestroyParticleHook::GetType()
{
    return 14;
}

ULONG DestroyParticleHook::pack_size()
{
    return(sizeof(uint32_t));
}

ULONG DestroyParticleHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, emitter_id);
    }

    return PackSize;
}

BOOL DestroyParticleHook::UnPack(BYTE** ppData, ULONG iSize)
{
    if (emitter_id == 0)
	    return true;
    
    UNPACK(uint32_t, emitter_id);

    return TRUE;
}

//=============================================================================
// StopParticleHook
//=============================================================================

StopParticleHook::StopParticleHook()
{
}

void StopParticleHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("StopParticleHook::Execute");
#endif
}

int StopParticleHook::GetType()
{
    return 15;
}

ULONG StopParticleHook::pack_size()
{
    return(sizeof(uint32_t));
}

ULONG StopParticleHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, emitter_id);
    }

    return PackSize;
}

BOOL StopParticleHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, emitter_id);

    return TRUE;
}

//=============================================================================
// CallPESHook
//=============================================================================

CallPESHook::CallPESHook()
{
}

void CallPESHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("CallPESHook::Execute");
#endif
}

int CallPESHook::GetType()
{
    return 19;
}

void CallPESHook::GetDataRecursion(LPVOID)
{
#if PHATSDK_RENDER_AVAILABLE
    UNFINISHED_LEGACY("CallPESHook::GetDataRecursion");
#endif
}

ULONG CallPESHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float));
}

ULONG CallPESHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, pes);
        PACK(float, pause);
    }

    return PackSize;
}

BOOL CallPESHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, pes);
    UNPACK(float, pause);

    return TRUE;
}

//=============================================================================
// SoundTweakedHook
//=============================================================================

SoundTweakedHook::SoundTweakedHook()
{
    gid_ = 0;
    prio = 0.9f;
    prob = 0.0f;
    vol = 0.0f;
}

SoundTweakedHook::~SoundTweakedHook()
{
    // DEBUGOUT("Missing DestroySound call here!\r\n");
    // SoundManager::DestroySound
    // __asm int 3;
}

void SoundTweakedHook::Execute(CPhysicsObj *pOwner)
{
    // MISSING CODE HERE
    // __asm int 3;
}

int SoundTweakedHook::GetType()
{
    return 21;
}

void SoundTweakedHook::GetDataRecursion(LPVOID)
{
    // UNFINISHED_LEGACY("SoundTweakedHook::GetDataRecursion");
}

ULONG SoundTweakedHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*3);
}

ULONG SoundTweakedHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    PACK(uint32_t, gid_);
    PACK(float, prob);
    PACK(float, prio);
    PACK(float, vol);

    return PackSize;
}

BOOL SoundTweakedHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, gid_);
    UNPACK(float, prob);
    UNPACK(float, prio);
    UNPACK(float, vol);

    // DEBUGOUT("Missing CreateSound call here!\r\n");
    // SoundManager::CreateSound
    // __asm int 3;

    return TRUE;
}

//=============================================================================
// SetOmegaHook
//=============================================================================

SetOmegaHook::SetOmegaHook()
{
}

void SetOmegaHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("SetOmegaHook::Execute");
}

int SetOmegaHook::GetType()
{
    return 22;
}

ULONG SetOmegaHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*2);
}

ULONG SetOmegaHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(Vector, axis);
    }

    return PackSize;
}

BOOL SetOmegaHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK_OBJ(axis);
    return TRUE;
}

//=============================================================================
// TextureVelocityHook
//=============================================================================

TextureVelocityHook::TextureVelocityHook()
{
    u_speed = 0.0f;
    v_speed = 0.0f;
}

void TextureVelocityHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("TextureVelocityHook::Execute");
}

int TextureVelocityHook::GetType()
{
    return 23;
}

ULONG TextureVelocityHook::pack_size()
{
    return(sizeof(float)*2);
}

ULONG TextureVelocityHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(float, u_speed);
        PACK(float, v_speed);
    }

    return PackSize;
}

BOOL TextureVelocityHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(float, u_speed);
    UNPACK(float, v_speed);

    return TRUE;
}

//=============================================================================
// TextureVelocityPartHook
//=============================================================================

TextureVelocityPartHook::TextureVelocityPartHook()
{
    part_index = -1;
    u_speed = 0.0f;
    v_speed = 0.0f;
}

void TextureVelocityPartHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("TextureVelocityPartHook::Execute");
}

int TextureVelocityPartHook::GetType()
{
    return 24;
}

ULONG TextureVelocityPartHook::pack_size()
{
    return(sizeof(uint32_t) + sizeof(float)*2);
}

ULONG TextureVelocityPartHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, part_index);
        PACK(float, u_speed);
        PACK(float, v_speed);
    }

    return PackSize;
}

BOOL TextureVelocityPartHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, part_index);
    UNPACK(float, u_speed);
    UNPACK(float, v_speed);

    return TRUE;
}

//=============================================================================
// SetLightHook
//=============================================================================

SetLightHook::SetLightHook()
{
}

void SetLightHook::Execute(CPhysicsObj *pOwner)
{
    // UNFINISHED_LEGACY("SetLightHook::Execute");
}

int SetLightHook::GetType()
{
    return 25;
}

ULONG SetLightHook::pack_size()
{
    return(sizeof(uint32_t));
}

ULONG SetLightHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, _lights_on);
    }

    return PackSize;
}

BOOL SetLightHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, _lights_on);

    return TRUE;
}

//=============================================================================
// AttackCone -- Needs to be moved elsewhere
//=============================================================================

AttackCone::AttackCone()
{
}

ULONG AttackCone::pack_size()
{
    return(sizeof(uint32_t) + left.pack_size() + right.pack_size() + sizeof(float)*2);
}

ULONG AttackCone::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, part_index);
        PACK_OBJ_WRITER(left);
        PACK_OBJ_WRITER(right);
        PACK(float, radius);
        PACK(float, height);
    }

    return PackSize;
}

BOOL AttackCone::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, part_index);
    UNPACK_OBJ_READER(left);
    UNPACK_OBJ_READER(right);
    UNPACK(float, radius);
    UNPACK(float, height);
        
    return TRUE;
}

//=============================================================================
// AttackHook
//=============================================================================

AttackHook::AttackHook()
{
}

void AttackHook::Execute(CPhysicsObj *pOwner)
{
#if PHATSDK_IS_SERVER
    // UNFINISHED_LEGACY("AttackHook::Execute");
	if (pOwner->GetWeenie())
		pOwner->GetWeenie()->HandleAttackHook(m_cone);
#endif
}

int AttackHook::GetType()
{
    return 3;
}

ULONG AttackHook::pack_size()
{
    return(m_cone.pack_size());
}

ULONG AttackHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK_OBJ(m_cone);
    }

    return PackSize;
}

BOOL AttackHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK_OBJ(m_cone);

    return TRUE;
}

//=============================================================================
// EtherealHook
//=============================================================================

EtherealHook::EtherealHook()
{
}

void EtherealHook::Execute(CPhysicsObj *pOwner)
{
	pOwner->set_ethereal(ethereal, 0);
}

int EtherealHook::GetType()
{
    return 6;
}

ULONG EtherealHook::pack_size()
{
    return(sizeof(uint32_t));
}

ULONG EtherealHook::Pack(BYTE** ppData, ULONG iSize)
{
    ULONG PackSize = pack_size();

    if (iSize >= PackSize)
    {
        PACK(uint32_t, ethereal);
    }

    return PackSize;
}

BOOL EtherealHook::UnPack(BYTE** ppData, ULONG iSize)
{
    UNPACK(uint32_t, ethereal);

    return TRUE;
}

