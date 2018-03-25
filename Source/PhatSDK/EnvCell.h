
#pragma once

#include "ObjCache.h"
#include "ObjCell.h"
#include "DArray.h"
#include "MathLib.h"
#include "Packable.h"
#include "Frame.h"

#if PHATSDK_RENDER_AVAILABLE
struct portal_info
{
	int seen;
	int inflag;
};

struct view_poly
{
	int vertex_count;
	int vertex_index;
	float xmin;
	float xmax;
	float ymin;
	float ymax;
};

struct view_vertex
{
	Vec2D pt;
	Plane plane;
};

struct view_type
{
	unsigned int vertex_count_total;
	DArray<view_poly> poly;
	DArray<view_vertex> vertex;
};

struct portal_view_type
{
	DArray<portal_info> portal;
	view_type view;
	float max_indist;
	unsigned int view_count;
	int cell_view_done;
	int view_timestamp;
	int update_count;
};
#endif

class CSurface;
class CEnvironment;
class CCellStruct;
class CCellPortal;
class CPolygon;

class CEnvCell : public CObjCell
{
public:
	CEnvCell();
	~CEnvCell();

	static DBObj *Allocator();
	static void Destroyer(DBObj*);
	static CEnvCell *Get(DWORD ID);
	static void Release(CEnvCell *);

	void Destroy();
	BOOL UnPack(BYTE **ppData, ULONG iSize);

	void calc_clip_planes();
	void init_static_objects();
	BOOL point_in_cell(const Vector& point);

	virtual TransitionState find_collisions(class CTransition *) override;
	virtual TransitionState find_env_collisions(CTransition *transition) override;

	static CEnvCell *GetVisible(DWORD cell_id);
	CEnvCell *find_visible_child_cell(Vector *origin, const int bSearchCells);

	virtual void find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path) override;
	virtual void find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array);

	void check_building_transit(int portal_id, Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path);
	void check_building_transit(int portal_id, const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array);

	CPhysicsObj *recursively_get_object(DWORD obj_iid, PackableHashTable<unsigned long, int> *visited_cells);

	// custom, this doesn't really exist
	bool Custom_GetDungeonDrop(int dropIndex, Frame *pDropFrame, int *pNumDrops);

	DWORD num_surfaces; // 0xF8
	CSurface **surfaces; // 0xFC
	CCellStruct *structure; // 0x100
	CEnvironment *env; // 0x104
	DWORD num_portals; // 0x108
	CCellPortal *portals; // 0x10C
	DWORD num_static_objects; // 0x110
	DWORD *static_object_ids; // 0x114
	Frame *static_object_frames; // 0x118
	CPhysicsObj **static_objects; // 0x11C
	LPVOID light_array; // 0x120
	int incell_timestamp = 0;

#if PHATSDK_RENDER_AVAILABLE
	class MeshBuffer *constructed_mesh = NULL;
	int use_built_mesh = 0;
	unsigned int m_current_render_frame_num = 0;
	unsigned int num_view = 0;
	DArray<portal_view_type *> portal_view;
#endif

#if PHATSDK_IS_SERVER
	bool needs_post_load = true;
#endif

#if PHATSDK_USE_EXTENDED_CELL_DATA
	DWORD num_dynamic_objects = 0;
	DWORD *dynamic_object_wcids = NULL;
	Position *dynamic_object_pos = NULL;
	DWORD *dynamic_object_iids = NULL;
#endif
};

class CCellPortal
{
public:
	CCellPortal();
	~CCellPortal();

	BOOL UnPack(DWORD LandBlock, WORD *PolyIndex, BYTE **ppData, ULONG iSize);
	CEnvCell *GetOtherCell(BOOL do_not_load);

	DWORD other_cell_id; // 0x0
	CEnvCell *other_cell_ptr; // 0x4
	CPolygon *portal; // 0x8
	int portal_side; // 0xC
	int other_portal_id; // 0x10
	int exact_match; // 0x14
};






