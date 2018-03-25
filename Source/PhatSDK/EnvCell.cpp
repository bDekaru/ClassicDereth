
#include "StdAfx.h"
#include "Surface.h"
#include "PhysicsObj.h"
#include "Environment.h"
#include "EnvCell.h"
#include "Transition.h"
#include "LandCell.h"

#if PHATSDK_IS_SERVER
#include "ServerCellManager.h"
#endif

CEnvCell::CEnvCell()
#if PHATSDK_RENDER_AVAILABLE
	: portal_view(128)
#endif
{
	num_surfaces = 0;
	surfaces = NULL;
	structure = NULL;
	env = NULL;
	num_portals = 0;
	portals = NULL;
	num_static_objects = 0;
	static_object_ids = NULL;
	static_object_frames = NULL;
	static_objects = NULL;
	incell_timestamp = 0;
	light_array = NULL;
}

CEnvCell::~CEnvCell()
{
	Destroy();
}

DBObj* CEnvCell::Allocator()
{
	return((DBObj *)new CEnvCell());
}

void CEnvCell::Destroyer(DBObj* pEnvCell)
{
	delete ((CEnvCell *)pEnvCell);
}

CEnvCell *CEnvCell::Get(DWORD ID)
{
	return (CEnvCell *)ObjCaches::EnvCells->Get(ID);
}

void CEnvCell::Release(CEnvCell *pEnvCell)
{
	if (pEnvCell)
		ObjCaches::EnvCells->Release(pEnvCell->GetID());
}

void CEnvCell::Destroy()
{
	if (surfaces)
	{
		for (DWORD i = 0; i < num_surfaces; i++)
			CSurface::Release(surfaces[i]);

		delete[] surfaces;
		surfaces = NULL;
	}

	// Missing code here

	if (env)
	{
		CEnvironment::Release(env);
		env = NULL;
	}

	// Missing code here

	if (portals)
	{
		delete[] portals;
		portals = NULL;
	}
	num_portals = 0;

	if (stab_list)
	{
		delete[] stab_list;
		stab_list = NULL;
	}
	num_stabs = 0;

	if (static_object_ids)
	{
		delete[] static_object_ids;
		static_object_ids = NULL;
	}

	if (static_object_frames)
	{
		delete[] static_object_frames;
		static_object_frames = NULL;
	}

	if (static_objects)
	{
		for (DWORD i = 0; i < num_static_objects; i++)
		{
			if (static_objects[i])
			{
				static_objects[i]->leave_world();
				delete static_objects[i];
			}
		}

		delete[] static_objects;
		static_objects = NULL;
	}
	num_static_objects = 0;

	structure = NULL;
	seen_outside = FALSE;

	release_objects();

#if PHATSDK_USE_EXTENDED_CELL_DATA
	num_dynamic_objects = 0;
	SafeDeleteArray(dynamic_object_wcids);
	SafeDeleteArray(dynamic_object_pos);
	SafeDeleteArray(dynamic_object_iids);
#endif
}

BOOL CEnvCell::UnPack(BYTE **ppData, ULONG iSize)
{
#ifdef PRE_TOD
#else
	UNPACK(DWORD, id);
#endif

	DWORD Flags;

	UNPACK(DWORD, Flags);
	seen_outside = FBitSet(Flags, 0);

	UNPACK(DWORD, id);

	DWORD LandBlock = GetID() & 0xFFFF0000;

	UNPACK(BYTE, num_surfaces);
	UNPACK(BYTE, num_portals);
	UNPACK(WORD, num_stabs);

	surfaces = new CSurface*[num_surfaces];
	for (DWORD i = 0; i < num_surfaces; i++)
	{
		DWORD SurfaceID;
		UNPACK(WORD, SurfaceID);
		SurfaceID |= 0x08000000;

		surfaces[i] = CSurface::Get(SurfaceID);
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	DWORD EnvironmentID, StructIndex;

	UNPACK(WORD, EnvironmentID);
	EnvironmentID |= 0x0D000000;

	UNPACK(WORD, StructIndex);

	env = CEnvironment::Get(EnvironmentID);

	if (env)
		structure = env->get_cellstruct(StructIndex);

	pos.objcell_id = GetID();
	UNPACK_OBJ(pos.frame);

	if (num_portals > 0)
	{
		portals = new CCellPortal[num_portals];

		for (DWORD i = 0; i < num_portals; i++)
		{
			// This is the index of the portal polygon.
			WORD PolyIndex;

			portals[i].UnPack(LandBlock, &PolyIndex, ppData, iSize);
			portals[i].portal = structure->get_portal(PolyIndex);
		}
	}
#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	if (num_stabs > 0)
	{
		stab_list = new DWORD[num_stabs];

		for (DWORD i = 0; i < num_stabs; i++)
		{
			DWORD CellID;
			UNPACK(WORD, CellID);

			stab_list[i] = LandBlock | CellID;
		}
	}
#ifdef PRE_TOD
	PACK_ALIGN();
#endif

	if (Flags & 2)
	{
		UNPACK(DWORD, num_static_objects);

		if (num_static_objects > 0)
		{
			static_object_ids = new DWORD[num_static_objects];
			static_object_frames = new Frame[num_static_objects];

			for (DWORD i = 0; i < num_static_objects; i++)
			{
				UNPACK(DWORD, static_object_ids[i]);
				UNPACK_OBJ(static_object_frames[i]);
			}
		}
	}
#ifdef PRE_TOD
	PACK_ALIGN();
#endif

#if PHATSDK_USE_EXTENDED_CELL_DATA
	if (Flags & 4)
	{
		UNPACK(DWORD, num_dynamic_objects);

		if (num_dynamic_objects > 0)
		{
			dynamic_object_wcids = new DWORD[num_dynamic_objects];
			dynamic_object_pos = new Position[num_dynamic_objects];
			dynamic_object_iids = new DWORD[num_dynamic_objects];

			for (DWORD i = 0; i < num_dynamic_objects; i++)
			{
				UNPACK(DWORD, dynamic_object_wcids[i]);
				UNPACK_OBJ_READER(dynamic_object_pos[i]);
				UNPACK(DWORD, dynamic_object_iids[i]);
			}
		}
	}
#endif

	if (Flags & 8)
		UNPACK(DWORD, restriction_obj);
	else
		restriction_obj = 0;

	// PolyLightCache..
	// m_PolyLights = new Vector[ structure->vertex_array.num_vertices ];
	// ... or...
	// v5->light_array = (RGBColor *)operator new[](12 * v5->structure->vertex_array.num_vertices);

	calc_clip_planes();

#if PHATSDK_RENDER_AVAILABLE
	// if (DBCache::IsRunTime())
	{
		use_built_mesh = 1;
		if (constructed_mesh || !D3DPolyRender::ConstructMesh(
			num_surfaces,
			surfaces,
			&structure->vertex_array,
			structure->num_polygons,
			structure->polygons,
			3.0,
			1,
			&constructed_mesh))
		{
			use_built_mesh = 0;
		}
	}
#endif

	return TRUE;
}

void CEnvCell::calc_clip_planes()
{
	// DEBUGOUT("Unfinished calc clipplanes!\r\n");
}

void CEnvCell::init_static_objects()
{
	if (static_objects)
	{
		for (DWORD i = 0; i < num_static_objects; i++)
		{
			if (static_objects[i])
			{
				if (!static_objects[i]->is_completely_visible())
					static_objects[i]->calc_cross_cells_static();
			}
		}
	}
	else
	{
		if (num_static_objects > 0)
		{
			static_objects = new CPhysicsObj*[num_static_objects];

			for (DWORD i = 0; i < num_static_objects; i++)
			{
				if (static_object_ids[i])
					static_objects[i] = CPhysicsObj::makeObject(static_object_ids[i], 0, FALSE);
				else
					static_objects[i] = NULL;

				if (static_objects[i])
					static_objects[i]->add_obj_to_cell(this, &static_object_frames[i]);
			}
		}
	}
}

CCellPortal::CCellPortal()
{
	other_cell_id = 0;
	other_cell_ptr = NULL;
	portal = NULL;
	portal_side = 0;
	other_portal_id = -1;
	exact_match = -1;
}

CCellPortal::~CCellPortal()
{
}

BOOL CCellPortal::UnPack(DWORD LandBlock, WORD *PolyIndex, BYTE **ppData, ULONG iSize)
{
	BYTE Flags;
	UNPACK(WORD, Flags);

	exact_match = FBitSet(Flags, 0);
	portal_side = FBitSet(~Flags, 1);

	UNPACK(WORD, *PolyIndex);

	WORD CellID;
	UNPACK(WORD, CellID);

	if (Flags & 4)
		other_cell_id = -1;
	else
		other_cell_id = LandBlock | CellID;

	UNPACK(short, other_portal_id);

	return TRUE;
}

CEnvCell *CCellPortal::GetOtherCell(BOOL do_not_load)
{
	return CEnvCell::GetVisible(other_cell_id);
}

BOOL CEnvCell::point_in_cell(const Vector& point)
{
	if (!portals)
		return FALSE;

	Vector local_point = pos.frame.globaltolocal(point);

	return structure->point_in_cell(local_point);
}

TransitionState CEnvCell::find_collisions(CTransition *transit)
{
	TransitionState ts = find_env_collisions(transit);

	if (ts == OK_TS)
		ts = find_obj_collisions(transit);

	return ts;
}

TransitionState CEnvCell::find_env_collisions(CTransition *transition)
{
	TransitionState ts = check_entry_restrictions(transition);

	if (ts == OK_TS)
	{
		transition->sphere_path.obstruction_ethereal = 0;

		if (structure->physics_bsp)
		{
			transition->sphere_path.cache_localspace_sphere(&pos, 1.0);

			if (transition->sphere_path.insert_type == SPHEREPATH::INITIAL_PLACEMENT_INSERT)
				ts = structure->physics_bsp->placement_insert(transition);
			else
				ts = structure->physics_bsp->find_collisions(transition, 1.0);

			if (ts != 1 && !(transition->object_info.state & 1))
				transition->collision_info.collided_with_environment = 1;
		}
	}

	return ts;
}

CEnvCell *CEnvCell::GetVisible(DWORD cell_id)
{
	// Should get any requested cell on a server, or visible cells on a client?
	return g_pPhatSDK->EnvCell_GetVisible(cell_id);
}

CEnvCell *CEnvCell::find_visible_child_cell(Vector *origin, const int bSearchCells)
{
	if (point_in_cell(*origin))
		return this;

	if (bSearchCells)
	{
		for (DWORD i = 0; i < num_stabs; i++)
		{
			CEnvCell *pEnvCell = CEnvCell::GetVisible(stab_list[i]);

			if (pEnvCell)
			{
				if (pEnvCell->point_in_cell(*origin))
					return pEnvCell;
			}
		}
	}
	else
	{
		for (DWORD i = 0; i < num_portals; i++)
		{
			CEnvCell *pEnvCell = portals[i].GetOtherCell(TRUE);

			if (pEnvCell)
			{
				if (pEnvCell->point_in_cell(*origin))
					return pEnvCell;
			}
		}
	}

	return NULL;
}

void CEnvCell::find_transit_cells(Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	int check_outside = 0;

	for (DWORD i = 0; i < num_portals; i++)
	{
		CCellPortal *cellPortal = &portals[i];
		if (cellPortal->other_cell_id == -1)
		{
			for (DWORD j = 0; j < num_sphere; j++)
			{
				float ia = sphere[j].radius + F_EPSILON;
				Vector v = pos.frame.globaltolocal(sphere[j].center);

				float v17 = cellPortal->portal->plane.dot_product(v);
				if (v17 > -ia && v17 < ia)
				{
					check_outside = 1;
					break;
				}
			}
		}
		else
		{
			CEnvCell *otherCell = cellPortal->GetOtherCell(cell_array->do_not_load_cells);
			if (otherCell)
			{
				for (DWORD j = 0; j < num_sphere; j++)
				{
					Vector v = otherCell->pos.frame.globaltolocal(sphere[j].center);

					CSphere sph;
					sph.center = v;
					sph.radius = sphere[j].radius;

					if (otherCell->structure->sphere_intersects_cell(&sph))
					{
						cell_array->add_cell(otherCell->GetID(), otherCell);
						break;
					}

					// assert(!otherCell->structure->point_in_cell(sph.center));
				}
			}
			else
			{
				for (DWORD j = 0; j < num_sphere; j++)
				{
					Vector v = pos.frame.globaltolocal(sphere[j].center);

					CSphere sph;
					sph.center = v;
					sph.radius = sphere[j].radius + F_EPSILON;
					int ps = portals[i].portal_side;

					float pdp = portals[i].portal->plane.dot_product(sph.center);

					if ((pdp > -sph.radius && ps == 1) || (pdp < sph.radius && ps == 0))
					{
						cell_array->add_cell(portals[i].other_cell_id, NULL);
						break;
					}
				}
			}
		}
	}

	if (check_outside)
		CLandCell::add_all_outside_cells(p, num_sphere, sphere, cell_array);
}

void CEnvCell::find_transit_cells(const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	int check_outside = 0;
	float radius, neg_radius;

	for (DWORD i = 0; i < num_portals; i++)
	{
		for (DWORD j = 0; j < num_parts; j++)
		{
			CPhysicsPart *part = parts[j];
			if (!part)
				continue;

			CSphere *gfxSphere = part->gfxobj[0]->physics_sphere;

			if (!gfxSphere)
				gfxSphere = part->gfxobj[0]->drawing_sphere;		

			if (!gfxSphere)
				continue;

			Vector center = pos.localtolocal(part->pos, gfxSphere->center);

			double radVal = gfxSphere->radius + F_EPSILON;
			radius = radVal;
			neg_radius = -radVal;

			float dp = portals[i].portal->plane.dot_product(center);

			if (portals[i].portal_side)
			{
				if (portals[i].portal_side == 1 && dp < neg_radius)
					continue;
			}
			else
			{
				if (dp > radius)
					continue;
			}

			BBox *bbox = parts[j]->GetBoundingBox();
			
			BBox box;
			box.LocalToLocal(bbox, &parts[j]->pos, &pos);
			if (portals[i].portal->plane.intersect_box(&box) == portals[i].portal_side)
				continue;

			if (portals[i].other_cell_id == -1)
			{
				check_outside = 1;
				break;
			}

			CEnvCell *otherCell = portals[i].GetOtherCell(cell_array->do_not_load_cells);
			if (!otherCell)
			{
				cell_array->add_cell(portals[i].other_cell_id, NULL);
				break;
			}

			BBox cell_box;
			cell_box.LocalToLocal(bbox, &parts[j]->pos, &otherCell->pos);
			if (otherCell->structure->box_intersects_cell(&cell_box))
			{
				cell_array->add_cell(otherCell->id, otherCell);
				break;
			}
		}
	}

	if (check_outside)
		CLandCell::add_all_outside_cells(num_parts, parts, cell_array);
}

bool CEnvCell::Custom_GetDungeonDrop(int dropIndex, Frame *pDropFrame, int *pNumDrops)
{
	int numDrops = 0;
	bool bFoundDrop = false;

	for (DWORD i = 0; i < num_static_objects; i++)
	{
		DWORD setupID = static_object_ids[i];
		if ((setupID >= 0x02000C39 && setupID <= 0x02000C48) || (setupID == 0x02000F4A))
		{
			if (dropIndex == numDrops && pDropFrame)
			{
				*pDropFrame = static_object_frames[i];
				bFoundDrop = true;
			}

			numDrops++;
			break;
		}
	}

	if (pNumDrops)
		*pNumDrops = numDrops;

	return bFoundDrop || (dropIndex < 0);
}

CPhysicsObj *CEnvCell::recursively_get_object(DWORD obj_iid, PackableHashTable<unsigned long, int> *visited_cells)
{
	CPhysicsObj *pObject = get_object(obj_iid);

	if (!pObject)
	{
		for (DWORD i = 0; i < num_portals; i++)
		{
			DWORD cellid = portals[i].other_cell_id;

			if (visited_cells->lookup(cellid))
				continue;

			if (cellid != (DWORD)-1)
			{
				int ptrue = 1;
				visited_cells->add(cellid, &ptrue);

				CEnvCell *pOtherCell = portals[i].GetOtherCell(FALSE);
				if (pOtherCell)
				{
					int ptrue = 1;
					visited_cells->add(cellid, &ptrue);

					pObject = pOtherCell->recursively_get_object(obj_iid, visited_cells);
					if (pObject)
						return pObject;
				}
			}
		}
	}

	return pObject;
}

void CEnvCell::check_building_transit(int portal_id, Position *p, const unsigned int num_sphere, CSphere *sphere, CELLARRAY *cell_array, SPHEREPATH *path)
{
	if (portal_id >= 0)
	{
		for (DWORD i = 0; i < num_sphere; i++)
		{
			CSphere v15;

			v15.center = pos.frame.globaltolocal(sphere[i].center);
			v15.radius = sphere[i].radius;

			if (structure->sphere_intersects_cell(&v15))
			{
				if (path)
					path->hits_interior_cell = 1;

				cell_array->add_cell(GetID(), this);
			}
		}
	}
}

void CEnvCell::check_building_transit(int portal_id, const unsigned int num_parts, CPhysicsPart **parts, CELLARRAY *cell_array)
{
	if (portal_id >= 0)
	{
		for (DWORD i = 0; i < num_parts; i++)
		{
			if (parts[i])
			{
				CSphere *bounding_sphere;

				if (parts[i]->gfxobj[0]->physics_sphere)
					bounding_sphere = parts[i]->gfxobj[0]->physics_sphere;
				else
					bounding_sphere = parts[i]->gfxobj[0]->drawing_sphere;

				if (bounding_sphere)
				{
					Vector center = pos.localtolocal(parts[i]->pos, bounding_sphere->center);
					float radius = bounding_sphere->radius + F_EPSILON;
					float neg_radius = -radius;

					int portal_side = portals[portal_id].portal_side;

					float dp = portals[portal_id].portal->plane.dot_product(center);
					if (portal_side == 1)
					{
						if (dp > radius)
							continue;
					}
					else if (portal_side == 0)
					{
						if (dp < neg_radius)
							continue;
					}

					BBox box;
					box.LocalToLocal(parts[i]->GetBoundingBox(), &parts[i]->pos, &pos);
					int v19 = portals[portal_id].portal->plane.intersect_box(&box);
					if (v19 == 3 || v19 == portal_side)
					{
						BBox cell_box;
						cell_box.LocalToLocal(parts[i]->GetBoundingBox(), &parts[i]->pos, &pos);
						if (structure->box_intersects_cell(&cell_box))
						{
							cell_array->add_cell(GetID(), this);
							find_transit_cells(num_parts, parts, cell_array);
							return;
						}
					}
				}
			}
		}
	}
}

