
#pragma once

#include "ObjCache.h"
#include "DLListBase.h"

class CAnimation;
class AFrame;
class AnimFrame;

class AnimData : public LegacyPackObj
{
public:
	AnimData();
	~AnimData();

	ULONG pack_size();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	uint32_t anim_id; // 0x04
	int32_t low_frame; // 0x08
	int32_t high_frame; // 0x0C
	float framerate; // 0x10
};

class AnimSequenceNode : public LegacyPackObj, public DLListData
{
public:
	AnimSequenceNode();
	AnimSequenceNode(AnimData *pAnimData);
	virtual ~AnimSequenceNode();

	ULONG pack_size();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	float get_framerate();
	void multiply_framerate(float fRate);

	BOOL has_anim();
	int32_t get_low_frame();
	int32_t get_high_frame();
	float get_starting_frame();
	float get_ending_frame();
	void set_animation_id(uint32_t AnimID);

	AnimFrame *get_part_frame(int32_t index);
	AFrame *get_pos_frame(int32_t index);

	AnimSequenceNode *GetNext();
	AnimSequenceNode *GetPrev();

	//private:

	 // DLListNode inherited..
	 // AnimSequenceNode *m_pNext; // 0x04
	 // AnimSequenceNode *m_pPrev; // 0x08 
	CAnimation *anim; // 0x0C
	float framerate; // 0x10
	int32_t low_frame; // 0x14
	int32_t high_frame; // 0x18
};

class CAnimation : public DBObj
{
public:
	CAnimation();
	virtual ~CAnimation();

	static DBObj *Allocator();
	static void Destroyer(DBObj *pAnimation);
	static CAnimation *Get(uint32_t ID);
	static void Release(CAnimation *pAnimation);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	// CAnimation members..
	AFrame *pos_frames; // 0x1C / 0x28
	AnimFrame *part_frames; // 0x20 / 0x2C
	BOOL has_hooks; // 0x24 / 0x30
	uint32_t num_parts; // 0x28 / 0x34
	uint32_t num_frames; // 0x2C / 0x38
};




