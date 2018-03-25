
#pragma once

#include "MathLib.h"

class CVertexArray;
class CVertex;

enum eCullModes
{
	CullCW = 0,
	CullNone = 1,
	CullUnknown = 2
};

class CPolygon
{
public:
	CPolygon();
	~CPolygon();

	static void SetPackVerts(CVertexArray *Verts);

	void Destroy();
	BOOL UnPack(BYTE** ppData, ULONG iSize);

	void make_plane();

#if PHATSDK_RENDER_AVAILABLE
	BOOL polygon_hits_ray(const Ray& ray, float *time);
#endif

	BOOL point_in_polygon(const Vector& point);
	BOOL point_in_poly2D(const Vector &point, Sidedness side);

#if PHATSDK_RENDER_AVAILABLE
	// !! PeaFIXed
	BOOL peafixed_polygon_hits_ray(const Ray& ray, float *depth);
#endif

	int check_walkable(class CSphere *sphere, class Vector *up);
	int walkable_hits_sphere(struct SPHEREPATH *path, CSphere *object, Vector *up);
	int hits_sphere(CSphere *object);
	int polygon_hits_sphere_slow_but_sure(CSphere *object, Vector *contact_pt);
	int polygon_hits_sphere(CSphere *object, Vector *contact_pt);
	int adjust_sphere_to_plane(SPHEREPATH *path, CSphere *valid_pos, Vector *movement);
	int pos_hits_sphere(CSphere *object, Vector *movement, Vector *contact_pt, CPolygon **struck_poly);
	double adjust_sphere_to_poly(CSphere *check_pos, Vector *curr_pos, Vector *movement);
	BOOL check_small_walkable(CSphere *sphere, Vector *up);
	void adjust_to_placement_poly(CSphere *struck_sphere, CSphere *other_sphere, float radius, int center_solid, int solid_check);
	int find_crossed_edge(CSphere *sphere, Vector *up, Vector *normal);

	static CVertexArray *pack_verts;

	CVertex **vertices; // 0x00
	short *vertex_ids; // 0x04 vertex indices
	CVertex **screen; // 0x08 Vec2DScreen
	short poly_id; // 0x0C poly index
	BYTE num_pts; // 0x0E
	char stippling; // 0x0F
	int sides_type; // 0x10 - 0=CULLCW, 1=CULLNONE (aka: m_iUnknown)
	char* pos_uv_indices; // 0x14
	char* neg_uv_indices; // 0x18
	short pos_surface; // 0x1C positive surface index
	short neg_surface; // 0x1E negative surface index

	Plane plane; // 0x20

};
