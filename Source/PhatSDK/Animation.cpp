
#include <StdAfx.h>
#include "AnimHooks.h"
#include "Animation.h"

AnimData::AnimData()
{
    anim_id = 0;
    low_frame = 0;
    high_frame = -1;
    framerate = 30.0f;
}

AnimData::~AnimData()
{
}

ULONG AnimData::pack_size()
{
    return(0x10);
}

BOOL AnimData::UnPack(BYTE **ppData, ULONG iSize)
{
    if (iSize < pack_size())
        return FALSE;

    UNPACK(uint32_t, anim_id);
    UNPACK(int32_t, low_frame);
    UNPACK(int32_t, high_frame);
    UNPACK(float, framerate);

    return TRUE;
}

AnimSequenceNode::AnimSequenceNode()
{
    anim = NULL;
    framerate = 30.0f;
    low_frame = -1;
    high_frame = -1;
}

AnimSequenceNode::AnimSequenceNode(AnimData *pAnimData)
{
    anim    = NULL;
    framerate    = pAnimData->framerate;
    low_frame        = pAnimData->low_frame;
    high_frame        = pAnimData->high_frame;

    set_animation_id(pAnimData->anim_id);
}

AnimSequenceNode::~AnimSequenceNode()
{
    if (anim)
        CAnimation::Release(anim);
}

ULONG AnimSequenceNode::pack_size()
{
    return(0x10);
}

BOOL AnimSequenceNode::UnPack(BYTE **ppData, ULONG iSize)
{
    if (iSize < pack_size())
        return FALSE;

    uint32_t AnimID;
    UNPACK(uint32_t, AnimID);

    UNPACK(int32_t, low_frame);
    UNPACK(int32_t, high_frame);
    UNPACK(float, framerate);

    set_animation_id(AnimID);

    return TRUE;
}

float AnimSequenceNode::get_framerate()
{
    return framerate;
}

BOOL AnimSequenceNode::has_anim()
{
    return(anim ? TRUE : FALSE);
}

void AnimSequenceNode::multiply_framerate(float fRate)
{
    if (fRate < 0)
        std::swap(low_frame, high_frame);
    
    framerate *= fRate;
}

float AnimSequenceNode::get_starting_frame()
{
    if (framerate >= 0)
        return (float)(low_frame);
    else
        return (float)(high_frame + 1) - F_EPSILON;
}

float AnimSequenceNode::get_ending_frame()
{
    if (framerate >= 0)
        return (float)(high_frame + 1) - F_EPSILON;
    else
        return (float)(low_frame);
}

int32_t AnimSequenceNode::get_low_frame()
{
    return low_frame;
}

int32_t AnimSequenceNode::get_high_frame()
{
    return high_frame;
}

AnimSequenceNode *AnimSequenceNode::GetNext()
{
    return(static_cast<AnimSequenceNode *>(dllist_next));
}

AnimSequenceNode *AnimSequenceNode::GetPrev()
{
    return(static_cast<AnimSequenceNode *>(dllist_prev));
}

AnimFrame *AnimSequenceNode::get_part_frame(int32_t index)
{
    if (!anim)
        return NULL;

    if (index < 0 || index >= anim->num_frames)
        return NULL;

    return(&anim->part_frames[ index ]);
}

AFrame *AnimSequenceNode::get_pos_frame(int32_t index)
{
    if (!anim)
        return NULL;

    if (index < 0 || index >= anim->num_frames)
        return NULL;

    return(&anim->pos_frames[ index ]);
}

void AnimSequenceNode::set_animation_id(uint32_t AnimID)
{
    if (anim)
        CAnimation::Release(anim);

    anim = CAnimation::Get(AnimID);

    if (anim)
    {
        if (high_frame < 0)
            high_frame = anim->num_frames - 1;
        
        if (((uint32_t)low_frame) >= anim->num_frames)
            low_frame = anim->num_frames - 1;

        if (((uint32_t)high_frame) >= anim->num_frames)
            high_frame = anim->num_frames - 1;

        if (low_frame > high_frame)
            high_frame = low_frame;
    }
}

CAnimation::CAnimation()
{
    pos_frames = NULL;
    part_frames = NULL;
    has_hooks = FALSE;
    num_parts = 0;
    num_frames = 0;
}

CAnimation::~CAnimation()
{
    Destroy();
}

DBObj *CAnimation::Allocator()
{
    return((DBObj *)new CAnimation());
}

void CAnimation::Destroyer(DBObj *pAnimation)
{
    delete((CAnimation *)pAnimation);
}

CAnimation *CAnimation::Get(uint32_t ID)
{
    return (CAnimation *)ObjCaches::Animations->Get(ID);
}

void CAnimation::Release(CAnimation *pAnimation)
{
    if (pAnimation)
        ObjCaches::Animations->Release(pAnimation->GetID());
}

void CAnimation::Destroy()
{
    if (pos_frames)
    {
        delete [] pos_frames;
        pos_frames = NULL;
    }

    if (part_frames)
    {
        delete [] part_frames;
        part_frames = NULL;
    }

    num_frames = 0;
    num_parts = 0;
}

BOOL CAnimation::UnPack(BYTE **ppData, ULONG iSize)
{
    Destroy();

    UNPACK(uint32_t, id);

    uint32_t PackFlags;

    UNPACK(uint32_t, PackFlags);

    UNPACK(uint32_t, num_parts);
    UNPACK(uint32_t, num_frames);

    if (PackFlags & 1)
    {
        pos_frames = new AFrame[ num_frames ];

        for (uint32_t i = 0; i < num_frames; i++)
            UNPACK_OBJ_READER(pos_frames[i]);
    }

    has_hooks = (PackFlags & 2) ? TRUE : FALSE;

    part_frames = new AnimFrame[ num_frames ];

    for (uint32_t i = 0; i < num_frames; i++)
        part_frames[i].UnPack(num_parts, ppData, iSize);

    return TRUE;
}


