
#pragma once

#include "SortCell.h"

class CPolygon;

class CLandCell : public CSortCell
{
public:
	CLandCell();
	~CLandCell();

	static CLandCell *Get(uint32_t cell_id);

	static void add_outside_cell(CELLARRAY *cell_array, int x, int y);
	static void add_cell_block(int min_x, int min_y, int max_x, int max_y, CELLARRAY *cell_array);
	static void check_add_cell_boundary(CELLARRAY *cell_array, Vec2D *pt, int x, int y, float incell_max, float incell_min);
	static void add_all_outside_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array);
	static void add_all_outside_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array);

	virtual TransitionState find_collisions(CTransition *transition) override;
	virtual TransitionState find_env_collisions(CTransition *transition) override;
	virtual void find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array) override;
	virtual void find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path) override;

	BOOL IsInView(void);
	BOOL find_terrain_poly(const Vector &origin, CPolygon **walkable);
	BOOL point_in_cell(const Vector &pt) override;

	CPolygon **polygons; // 0xF0
	BOOL in_view; // 0xF4
};
