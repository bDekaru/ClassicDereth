
#pragma once

#include "DArray.h"
#include "MathLib.h"
#include "BSPData.h"

class CObjCell;

enum EnvCollisionProfile_Bitfield
{
	Undef_ECPB = 0x0,
	MyContact_ECPB = 0x1,
	FORCE_EnvCollisionProfile_Bitfield_32_BIT = 0x7FFFFFFF,
};

class EnvCollisionProfile
{
public:
	Vector velocity;
	unsigned int _bitfield = 0;

	void SetMeInContact(const int hasContact)
	{
		if (hasContact)
			_bitfield |= MyContact_ECPB;
		else
			_bitfield &= ~((uint32_t)MyContact_ECPB);
	}
};

struct OBJECTINFO
{
	enum ObjectInfoEnum
	{
		DEFAULT_OI = 0x0,
		CONTACT_OI = 0x1, // 0x1
		ON_WALKABLE_OI = 0x2, // 0x2
		IS_VIEWER_OI = 0x4, // 0x4
		PATH_CLIPPED_OI = 0x8, // 0x8
		FREE_ROTATE_OI = 0x10, // 0x10
		PERFECT_CLIP_OI = 0x40, // 0x40
		IS_IMPENETRABLE = 0x80, // 0x80
		IS_PLAYER = 0x100, // 0x100
		EDGE_SLIDE = 0x200, // 0x200
		IGNORE_CREATURES = 0x400, // 0x400
		IS_PK = 0x800, // 0x800
		IS_PKLITE = 0x1000, // 0x1000
		FORCE_ObjectInfoEnum_32_BIT = 0x7FFFFFFF,
	};

	OBJECTINFO();

	void init(CPhysicsObj *object, int object_state);
	int missile_ignore(CPhysicsObj *collideobject);
	float get_walkable_z();
	void kill_velocity();
	BOOL is_valid_walkable(Vector *normal);
	TransitionState validate_walkable(CSphere *check_pos, Plane *contact_plane, const int is_water, const float water_depth, SPHEREPATH *path, COLLISIONINFO *collisions, unsigned int land_cell_id);

	CPhysicsObj *object; // 0
	int state; // 4
	float scale; // 8
	float step_up_height; // 0xC
	float step_down_height; // 0x10
	int ethereal; // 0x14
	int step_down; // 0x18
	unsigned int targetID; // 0x1C
};

struct SPHEREPATH
{
	enum InsertType
	{
		TRANSITION_INSERT = 0,
		PLACEMENT_INSERT = 1,
		INITIAL_PLACEMENT_INSERT = 2
	};

	SPHEREPATH();
	~SPHEREPATH();

	void init();
	void init_sphere(unsigned int num_sphere, CSphere *sphere, const float scale);
	void init_path(CObjCell *begin_cell, Position *begin_pos, Position *end_pos);

	BOOL is_walkable_allowable(float zval);

	void cache_global_curr_center();
	void cache_global_sphere(Vector *offset);
	void cache_localspace_sphere(Position *p, const float scale);

	Vector get_curr_pos_check_pos_block_offset();

	void add_offset_to_check_pos(Vector *offset);
	void add_offset_to_check_pos(Vector *offset, const float radius);

	void set_collide(const Vector &collision_normal);

	TransitionState step_up_slide(OBJECTINFO *object, COLLISIONINFO *collisions);
	void restore_check_pos();
	int check_walkables();
	void set_check_pos(Position *p, CObjCell *cell);
	void save_check_pos();
	void set_walkable(CSphere *sphere, CPolygon *poly, Vector *zaxis, Position *local_pos, float scale);
	void set_neg_poly_hit(int step_up, Vector *collision_normal);

	void adjust_check_pos(uint32_t cell_id);
	Position get_walkable_pos();
	void set_walkable_check_pos(CSphere *sphere);

	TransitionState precipice_slide(COLLISIONINFO *collisions);

	unsigned int num_sphere; // 0
	CSphere *local_sphere; // 4
	Vector local_low_point; // 8
	CSphere *global_sphere; // 0x14
	Vector global_low_point; // 0x18
	CSphere *localspace_sphere; // 0x24
	Vector localspace_low_point; // 0x28
	Vector *localspace_curr_center; // 0x34
	Vector *global_curr_center; // 0x38
	Position localspace_pos; // 0x3C
	Vector localspace_z; // 0x84
	CObjCell *begin_cell; // 0x90
	Position *begin_pos; // 0x94
	Position *end_pos; // 0x98
	CObjCell *curr_cell; // 0x9C
	Position curr_pos; // 0xA0
	Vector global_offset; // 0xE8
	int step_up; // 0xF4
	Vector step_up_normal; // 0xF8
	int collide; // 0x104
	CObjCell *check_cell; // 0x108
	Position check_pos; // 0x10C
	SPHEREPATH::InsertType insert_type; // 0x154
	int step_down; // 0x158
	SPHEREPATH::InsertType backup; // 0x15C
	CObjCell *backup_cell; // 0x160
	Position backup_check_pos; // 0x164
	int obstruction_ethereal; // 0x1AC
	int hits_interior_cell; // 0x1B0
	int bldg_check; // 0x1B4
	float walkable_allowance; // 0x1B8
	float walk_interp; // 0x1BC
	float step_down_amt; // 0x1C0
	CSphere walkable_check_pos; // 0x1C4
	CPolygon *walkable; // 0x1D4
	int check_walkable; // 0x1D8
	Vector walkable_up; // 0x1DC
	Position walkable_pos; // 0x1E8
	float walkable_scale; // 0x230
	int cell_array_valid; // 0x234
	int neg_step_up; // 0x238
	Vector neg_collision_normal; // 0x23C
	int neg_poly_hit; // 0x248
	int placement_allows_sliding; // 0x24C
};

struct COLLISIONINFO
{
	COLLISIONINFO();

	void init();
	void add_object(CPhysicsObj *object, TransitionState ts);
	void set_contact_plane(Plane *plane, int is_water);
	void set_collision_normal(const Vector &normal);
	void set_sliding_normal(Vector *normal);

	int last_known_contact_plane_valid = 0; // 0x0 (0x270)
	Plane last_known_contact_plane;  // 0x4 
	int last_known_contact_plane_is_water = 0; // 0x14
	int contact_plane_valid = 0; // 0x18
	Plane contact_plane; // 0x1C
	unsigned int contact_plane_cell_id = 0; // 0x2C
	unsigned int last_known_contact_plane_cell_id = 0; // 0x30
	int contact_plane_is_water = 0; // 0x34
	int sliding_normal_valid = 0; // 0x38
	Vector sliding_normal; // 0x3C
	int collision_normal_valid = 0; // 0x48
	Vector collision_normal; // 0x4C
	Vector adjust_offset; // 0x58
	unsigned int num_collide_object = 0; // 0x64
	DArray<CPhysicsObj const *> collide_object; // 0x68 (0x270+0x68=0x2D8)
	CPhysicsObj *last_collided_object = NULL; // 0x78
	int collided_with_environment = 0; // 0x7C
	int frames_stationary_fall = 0; // 0x80
};

struct CELLINFO
{
	unsigned int cell_id = 0;
	CObjCell *cell = NULL;
};

struct CELLARRAY
{
	CELLARRAY();

	void add_cell(const unsigned int cell_id, CObjCell *cell);
	void remove_cell(int index);

	int added_outside = 0; // 0
	int do_not_load_cells = 0; // 4
	unsigned int num_cells = 0; // 8
	DArray<CELLINFO> cells; // 0xC
};

class CTransition
{
public:
	static CTransition *makeTransition();
	static void cleanupTransition(CTransition *);
	static int transition_level;

	CTransition();

	void init();
	void init_object(CPhysicsObj *object, int object_state);
	void init_sphere(unsigned int num_sphere, CSphere *sphere, const float scale);
	void init_path(CObjCell *begin_cell, Position *begin_pos, Position *end_pos);

	int check_collisions(CPhysicsObj *object);
	void init_contact_plane(unsigned int cell_id, Plane *plane, int is_water);
	void init_last_known_contact_plane(unsigned int cell_id, Plane *plane, int is_water);
	void init_sliding_normal(Vector *normal);

	BOOL step_up(Vector *collision_normal);
	BOOL step_down(float step_down_ht, float z_val);
	int edge_slide(TransitionState *ts, float step_down_ht, float z_val);

	int check_walkable(float z_chk);
	TransitionState transitional_insert(int num_insertion_attempts);
	int find_valid_position();
	TransitionState validate_transition(TransitionState ts, int *redo);
	int find_transitional_position();
	int find_placement_position();
	TransitionState validate_placement_transition(TransitionState ts, int *redo);
	int find_placement_pos();
	void calc_num_steps(Vector *offset, Vector *offset_per_step, unsigned int *num_steps);

	TransitionState validate_placement(TransitionState ts, int adjust);
	TransitionState placement_insert();
	TransitionState insert_into_cell(CObjCell *cell, int num_insertion_attempts);
	TransitionState check_other_cells(CObjCell *curr_cell);
	Vector adjust_offset(Vector *offset);
	TransitionState cliff_slide(Plane *contact_plane);

	void build_cell_array(CObjCell **new_cell_p);

	OBJECTINFO object_info; // 0x0
	SPHEREPATH sphere_path; // 0x20
	COLLISIONINFO collision_info; // 0x270
	CELLARRAY cell_array; // 0x2F4
	class CObjCell *new_cell_ptr; // 0x310
};