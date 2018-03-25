
#pragma once

#include "MathLib.h"
#include "Polygon.h"
#include "DArray.h"
#include "GameEnums.h"
#include "Vertex.h"

// #define COLLISION_ROUTINES

class CPolygon;
class CTransition;
struct OBJECTINFO;
struct SPHEREPATH;
struct COLLISIONINFO;

class CSphere
{
public:
	CSphere(const Vector& origin, float radius);
	CSphere();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);
	BOOL intersects(CSphere* pSphere);
	BOOL sphere_intersects_ray(const Ray& ray);

	TransitionState step_sphere_down(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum);
	TransitionState step_sphere_up(CTransition *transition, CSphere &check_pos, Vector &disp, float radsum);

	TransitionState land_on_sphere(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum);

	TransitionState intersects_sphere(CTransition *transition, int is_creature);
	TransitionState intersects_sphere(Position *p, float scale, CTransition *transition, int is_creature);
	BOOL collides_with_sphere(Vector *disp, const float radsum);

	TransitionState collide_with_point(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum, int sphere_num);

	TransitionState slide_sphere(SPHEREPATH *path, COLLISIONINFO *collisions, Vector *collision_normal, Vector *curr_pos);
	TransitionState slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float *radsum, const int sphere_number);
	TransitionState slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, Vector *disp, float radsum, int sphere_num);

	double find_time_of_collision(const Vector &movement, const Vector &disp, float radsum);

	Vector center;
	float radius = 0.0f;
};

class CCylSphere
{
public:
	CCylSphere();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	TransitionState intersects_sphere(CTransition *transition);
	TransitionState intersects_sphere(Position *p, float scale, CTransition *transition);
	BOOL collides_with_sphere(CSphere *check_pos, Vector *disp, const float radsum);

	TransitionState land_on_cylinder(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum);
	int normal_of_collision(SPHEREPATH *path, CSphere *check_pos, Vector *disp, float radsum, int sphere_num, Vector *normal);
	TransitionState step_sphere_down(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum);
	TransitionState step_sphere_up(CTransition *transition, CSphere &check_pos, Vector &disp, float radsum);
	TransitionState slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum, int sphere_num);
	TransitionState collide_with_point(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum, int sphere_num);

	Vector low_pt; // 0x0
	float height; // 0xC
	float radius; // 0x10
};

class CSolid
{
public:
	CSolid(const Vector& Vec);
	CSolid();

	ULONG pack_size();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	// Alternatively, could utilize CVector class.
	Vector m_Vec;
};

class CPortalPoly
{
public:
	CPortalPoly();
	~CPortalPoly();

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	void Destroy();

	long portal_index; //0x00
	CPolygon *portal; //0x04
};

class BSPNODE
{
public:
	BSPNODE();
	virtual ~BSPNODE(); //0x00

	void Destroy();

	static BOOL UnPackChild(BSPNODE** pOut, BYTE** ppData, ULONG iSize);
	BOOL UnPack(BYTE** ppData, ULONG iSize);

#if PHATSDK_RENDER_AVAILABLE
	// Draw routines
	void draw_no_check();
	void draw_check(DWORD unknown);
#endif

	/*
	virtual int sphere_intersects_poly(CSphere* pSphere, Vector*, CPolygon**, Vector*); // 0x04 
	virtual int sphere_intersects_solid(CSphere* pSphere, BOOL); // 0x08
	virtual int point_intersects_solid(Vector* pPoint); // 0x0C
	virtual int sphere_intersects_solid_poly(CSphere* pSphere, float, BOOL*, CPolygon**, BOOL); // 0x10
	virtual void find_walkable(void* HolyFucker, CSphere*, CPolygon**, Vector*, Vector*, BOOL*); // 0x14
	virtual int hits_walkable(void* HolyFucker, CSphere*, Vector*); // 0x18
	*/

	virtual BOOL point_intersects_solid(Vector *point);
	virtual int hits_walkable(SPHEREPATH *path, CSphere *valid_pos, Vector *up);
	virtual int sphere_intersects_poly(CSphere *check_pos, Vector *movement, CPolygon **polygon, Vector *contact_pt);
	virtual int sphere_intersects_solid(CSphere *check_pos, int center_check);
	virtual void find_walkable(SPHEREPATH *path, CSphere *valid_pos, CPolygon **polygon, Vector *movement, Vector *up, BOOL *changed);
	virtual int sphere_intersects_solid_poly(CSphere *check_pos, float radius, BOOL *center_solid, CPolygon **hit_poly, BOOL check_center);

	BOOL point_inside_cell_bsp(const Vector& point);
	BoundingType sphere_intersects_cell_bsp(CSphere *sphere);
	int box_intersects_cell_bsp(BBox *box);
	void DetachPortalsAndPurgeNodes(SmartArray<BSPNODE *> *io_PortalsToKeep);
	void LinkPortalNodeChain(SmartArray<BSPNODE *> *_Portals);

	CSphere sphere; //0x04 (size: 0x10)
	Plane splitting_plane; //0x14 (size: 0x10)
	union {
		DWORD type; // 0x24
		char type_string[4]; // 0x24
	};
	DWORD num_polys; //0x28
	CPolygon **in_polys; //0x2C
	BSPNODE *pos_node;
	BSPNODE *neg_node;

	static DWORD pack_tree_type;
	static CPolygon *pack_poly;
};

class BSPLEAF : public BSPNODE
{
public:
	BSPLEAF();
	virtual ~BSPLEAF();

	BOOL UnPackLeaf(BYTE** ppData, ULONG iSize);

	virtual BOOL point_intersects_solid(Vector *point);
	virtual int hits_walkable(SPHEREPATH *path, CSphere *valid_pos, Vector *up);
	virtual int sphere_intersects_poly(CSphere *check_pos, Vector *movement, CPolygon **polygon, Vector *contact_pt);
	virtual int sphere_intersects_solid(CSphere *check_pos, BOOL center_check);
	virtual void find_walkable(SPHEREPATH *path, CSphere *valid_pos, CPolygon **polygon, Vector *movement, Vector *up, BOOL *changed);
	virtual int sphere_intersects_solid_poly(CSphere *check_pos, float radius, BOOL *center_solid, CPolygon **hit_poly, BOOL check_center);

	DWORD leaf_index; //0x38
	DWORD solid; //0x3C
};

class BSPPORTAL : public BSPNODE
{
public:
	BSPPORTAL();
	virtual ~BSPPORTAL();

	void Destroy();
	BOOL UnPackPortal(BYTE** ppData, ULONG iSize);

	DWORD num_portals; //0x38
	CPortalPoly **in_portals; //0x3C
};

class BSPTREE // size: 0x04
{
public:
	BSPTREE();
	virtual ~BSPTREE();

	void Destroy();

	BOOL UnPack(BYTE** ppData, ULONG iSize);
	CSphere *GetSphere();

#if PHATSDK_RENDER_AVAILABLE
	// Draw routines
	void draw_no_check();
#endif

	// Cell routines
	BOOL point_inside_cell_bsp(const Vector& point);
	// int box_intersects_cell_bsp(BBox *box);

	TransitionState find_collisions(CTransition *transition, float scale);
	TransitionState placement_insert(CTransition *transition);
	TransitionState collide_with_pt(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *curr_pos, CPolygon *hit_poly, Vector *contact_pt, float scale);
	int adjust_to_plane(CSphere *check_pos, Vector curr_pos, CPolygon *hit_poly, Vector *contact_pt);
	TransitionState check_walkable(SPHEREPATH *path, CSphere *check_pos, float scale);
	TransitionState slide_sphere(SPHEREPATH *path, COLLISIONINFO *collisions, Vector *collision_normal);
	TransitionState step_sphere_down(SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, float scale);
	TransitionState step_sphere_up(CTransition *transition, Vector *collision_normal);
	BoundingType sphere_intersects_cell_bsp(CSphere *sphere);
	int box_intersects_cell_bsp(class BBox *box);
	void RemoveNonPortalNodes();

private:
	BSPNODE *root_node;
};

struct ClipPlane
{
	ClipPlane()
	{
		plane = NULL;
	}

	ClipPlane(Plane *_plane, Sidedness _side)
	{
		plane = _plane;
		side = _side;
	}

	Plane *plane;
	Sidedness side;
};

struct ClipPlaneList
{
	ClipPlaneList() : cplane_list(128) { }

	unsigned int cplane_num = 0;
	DArray<ClipPlane> cplane_list;
	int leaf_contains_obj = 0;
};



