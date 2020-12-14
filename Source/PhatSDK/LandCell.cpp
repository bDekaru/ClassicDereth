
#include <StdAfx.h>
#include "LandDefs.h"
#include "LandCell.h"
#include "Polygon.h"

#if PHATSDK_IS_SERVER
#include "ServerCellManager.h"
#endif

CLandCell::CLandCell()
{
	id = 0;
	m_lLinks = 0;

	polygons = new CPolygon*[LandDefs::polys_per_landcell];
	in_view = FALSE;
}

CLandCell::~CLandCell()
{
	delete[] polygons;
}

BOOL CLandCell::IsInView(void)
{
	return in_view;
}

BOOL CLandCell::find_terrain_poly(const Vector &origin, CPolygon **walkable)
{
	for (uint32_t i = 0; i < 2; i++)
	{
		if (polygons[i]->point_in_poly2D(origin, Sidedness::POSITIVE))
		{
			*walkable = polygons[i];
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CLandCell::point_in_cell(const Vector &point)
{
	CPolygon *poly;
	return find_terrain_poly(point, (CPolygon **)&poly) != 0;
}

CLandCell *CLandCell::Get(uint32_t cell_id)
{
#if PHATSDK_IS_SERVER
	return (CLandCell *) g_pCellManager->GetObjCell(cell_id);
#else
	// return CObjCell::landscape->get_landcell(cell_id);
	UNFINISHED();
	return NULL;
#endif
}

void CLandCell::add_outside_cell(CELLARRAY *cell_array, int x, int y)
{
	if (x >= 0 && y >= 0 && x < 2040 && y < 2040)
	{
		uint32_t cell_id = (((y >> 3) | 32 * (x & 0xFFFFFFF8)) << 16) | ((y & 7) + 8 * (x & 7) + 1);

		CLandCell *pLandCell = CLandCell::Get(cell_id);
		cell_array->add_cell(cell_id, pLandCell);
	}
}

void CLandCell::check_add_cell_boundary(CELLARRAY *cell_array, Vec2D *pt, int x, int y, float incell_max, float incell_min)
{
	if (pt->x > (double)incell_max)
	{
		int v6 = x + 1;
		add_outside_cell(cell_array, x + 1, y);
		if (pt->y > (double)incell_max)
			add_outside_cell(cell_array, v6, y + 1);
		if (pt->y < (double)incell_min)
			add_outside_cell(cell_array, v6, y - 1);
	}
	if (pt->x < (double)incell_min)
	{
		int v7 = x - 1;
		add_outside_cell(cell_array, x - 1, y);
		if (pt->y >(double)incell_max)
			add_outside_cell(cell_array, v7, y + 1);
		if (pt->y < (double)incell_min)
			add_outside_cell(cell_array, v7, y - 1);
	}
	if (pt->y >(double)incell_max)
		add_outside_cell(cell_array, x, y + 1);
	if (pt->y < (double)incell_min)
		add_outside_cell(cell_array, x, y - 1);
}

void CLandCell::add_all_outside_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array)
{
	if (!cell_array->added_outside)
	{
		if (num_sphere)
		{
			for (uint32_t i = 0; i < num_sphere; i++)
			{
				Vector point = sphere[i].center;
				uint32_t pt_cell = p->objcell_id;
				if (!LandDefs::adjust_to_outside(&pt_cell, &point))
					break;

				Vec2D pt;
				pt.x = point.x - ((floor(point.x / 24.0)) * 24.0);
				pt.y = point.y - ((floor(point.y / 24.0)) * 24.0);
				float min_rad = sphere[i].radius;
				float max_rad = 24.0 - min_rad;
				
				int32_t x, y;
				if (LandDefs::gid_to_lcoord(pt_cell, x, y))
				{
					add_outside_cell(cell_array, x, y);
					check_add_cell_boundary(cell_array, &pt, x, y, max_rad, min_rad);
				}
			}
		}
		else
		{
			Vector point = p->frame.m_origin;
			uint32_t pt_cell = p->objcell_id;

			if (LandDefs::adjust_to_outside(&pt_cell, &point))
			{
				int32_t x, y;
				if (LandDefs::gid_to_lcoord(pt_cell, x, y))
				{
					add_outside_cell(cell_array, x, y);
				}
			}
		}
	}
}

void CLandCell::add_all_outside_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	if (!cell_array->added_outside)
	{
		cell_array->added_outside = 1;

		if (num_parts)
		{
			int min_x = 0;
			int min_y = 0;
			int max_x = 0;
			int max_y = 0;
			
			for (uint32_t i = 0; i < num_parts; i++)
			{
				if (parts[i])
				{
					Vector loc = parts[i]->pos.frame.m_origin;
					uint32_t pt_id = parts[i]->pos.objcell_id;
					if (!LandDefs::adjust_to_outside(&pt_id, &loc))
						pt_id = 0;

					CLandCell *cell = CLandCell::Get(pt_id);
					if (cell)
					{
						int32_t x, y;
						if (LandDefs::gid_to_lcoord(pt_id, x, y))
						{
							int lx = ((unsigned int)(uint16_t)pt_id - 1) >> 3;
							int ly = ((unsigned char)pt_id - 1) & 7;

							for (uint32_t j = 0; j < num_parts; j++)
							{
								if (parts[j])
								{
									int v19, v20, v21, v22;

									if (parts[j]->Always2D())
									{
										CSphere *bounding_sphere;

										if (parts[j]->gfxobj[0]->physics_sphere)
											bounding_sphere = parts[j]->gfxobj[0]->physics_sphere;
										else
											bounding_sphere = parts[j]->gfxobj[0]->drawing_sphere;

										float radius = bounding_sphere->radius;
										v19 = floor((loc.x - radius) / 24.0);
										v20 = floor((loc.y - radius) / 24.0);
										v21 = floor((loc.x + radius) / 24.0);
										v22 = floor((loc.y + radius) / 24.0);
									}
									else
									{
										BBox box;
										box.LocalToGlobal(parts[i]->GetBoundingBox(), &parts[j]->pos, &cell->pos);
										v19 = floor(box.m_Min.x / 24.0);
										v20 = floor(box.m_Min.y / 24.0);
										v21 = floor(box.m_Max.x / 24.0);
										v22 = floor(box.m_Max.y / 24.0);
									}
																		
									int v25 = v19 - lx;
									if (v25 < min_x)
										min_x = v25;
									int v26 = v20 - ly;
									if (v26 < min_y)
										min_y = v26;

									int max_xc = v21;
									if (max_xc - lx > max_x)
										max_x = max_xc - lx;	
									int v27 = v22 - ly;
									if (v27 > max_y)
										max_y = v27;
								}
							}

							CLandCell::add_cell_block(min_x + x, min_y + y, x + max_x, y + max_y, cell_array);
						}
					}
				}
			}
		}
	}
}

void CLandCell::add_cell_block(int min_x, int min_y, int max_x, int max_y, CELLARRAY *cell_array)
{
	for (int i = min_x; i <= max_x; ++i)
	{
		for (int j = min_y; j <= max_y; ++j)
		{
			if (i >= 0 && j >= 0 && i < 2040 && j < 2040)
			{
				uint32_t cell_id = (((j >> 3) | 32 * (i & 0xFFFFFFF8)) << 16) | ((j & 7) + 8 * (i & 7) + 1);
				CLandCell *pLandCell = CLandCell::Get(cell_id);
				cell_array->add_cell(cell_id, pLandCell);
			}
		}
	}
}

TransitionState CLandCell::find_env_collisions(CTransition *transition)
{
	TransitionState ts;

	ts = check_entry_restrictions(transition);

	if (ts == OK_TS)
	{
		Vector blockOffset = LandDefs::get_block_offset(transition->sphere_path.check_pos.objcell_id, GetID());
		
		Vector local_point = transition->sphere_path.global_low_point - blockOffset;

		CPolygon *walkable = NULL;
		if (find_terrain_poly(local_point, &walkable))
		{
			if (get_block_water_type() == LandDefs::WaterType::ENTIRELY_WATER
				&& !(transition->object_info.state & OBJECTINFO::IS_VIEWER_OI)
				&& !(transition->object_info.object->m_PhysicsState & MISSILE_PS))
			{
				return COLLIDED_TS;
			}

			float water_depth = get_water_depth(&local_point);

			CSphere* check_pos = &transition->sphere_path.global_sphere[0];
			check_pos->center -= LandDefs::get_block_offset(transition->sphere_path.check_pos.objcell_id, GetID());

			ts = transition->object_info.validate_walkable(
				check_pos,
				&walkable->plane,
				water_type != 0,
				water_depth,
				&transition->sphere_path,
				&transition->collision_info,
				GetID());
		}
	}

	return ts;
}

TransitionState CLandCell::find_collisions(CTransition *transition)
{
	TransitionState ts;
	
	ts = find_env_collisions(transition);
	if (ts == OK_TS)
	{
		ts = CSortCell::find_collisions(transition);
		if (ts == OK_TS)
		{
			ts = find_obj_collisions(transition);
		}
	}

	return ts;
}

void CLandCell::find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	CLandCell::add_all_outside_cells(num_parts, parts, cell_array);
	CSortCell::find_transit_cells(num_parts, parts, cell_array);
}

void CLandCell::find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	CLandCell::add_all_outside_cells(p, num_sphere, sphere, cell_array);
	CSortCell::find_transit_cells(p, num_sphere, sphere, cell_array, path);
}
