
#pragma once
#include "ObjCache.h"
#include "LandBlockStruct.h"
#include "LandBlockInfo.h"
#include "Frame.h"
#include "DArray.h"

struct SqCoord
{
	int x;
	int y;
};

class CLandBlock : public DBObj, public CLandBlockStruct
{
public:
	CLandBlock();
	~CLandBlock();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static CLandBlock *Get(uint32_t ID);
	static void Release(CLandBlock *);

	static void init(void);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	void add_static_object(CPhysicsObj *pObject);
	void destroy_static_objects();
	void destroy_buildings();
	void get_land_limits();
	void init_lcell_ptrs();
	void notify_change_size();
	void release_all();
	void release_objs();
	class CLandCell *get_landcell(uint32_t cell_id);
	void init_static_objs(LongNIValHash<uint32_t> *hash);
	void adjust_scene_obj_height();
	void get_land_scenes();
	void init_buildings();

	SqCoord block_coord;
	Frame block_frame; // 0x6C / 0x78
	float max_zval; // 0xAC / 0xB8
	float min_zval; // 0xB0 / 0xBC
	uint32_t dyn_objs_init_done; // 0xB4 / 0xC0
	BOOL lbi_exists; // 0xB8 / 0xC4
	LandDefs::Direction dir; // 0xBC / 0xC8
	SqCoord closest; // 0xC0 / 0xCC
	BoundingType in_view; // 0xC8 / 0xD4
	class CLandBlockInfo *lbi; // 0xCC / 0xD8
	uint32_t num_static_objects; // 0xD0 / 0xDC
	DArray<CPhysicsObj *> static_objects; // 0xD4 / 0xE0
	uint32_t num_buildings; // 0xE4 / 0xF0
	class CBuildingObj **buildings; // 0xE8 / 0xF4
	uint32_t stab_num; // 0xEC / 0xF8
	uint32_t *stablist; // 0xF0 / 0xFC
	class CLandCell **draw_array; // 0xFC / 0x108
	uint32_t draw_array_size; // 0x100 / 0x10C

#if PHATSDK_IS_SERVER
	bool needs_post_load = true;
#endif
};

