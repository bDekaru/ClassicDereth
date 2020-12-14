
#include <StdAfx.h>
#include "PhysicsObj.h"
#include "LandDefs.h"
#include "LandCell.h"
#include "LandBlock.h"
#include "Scene.h"

CLandBlock::CLandBlock() : static_objects(0x80)
{
	max_zval = 0.0;
	min_zval = 0.0;
	dyn_objs_init_done = 0;
	lbi_exists = FALSE;

	in_view = OUTSIDE;
	lbi = NULL;
	num_static_objects = 0;
	dir = LandDefs::Direction::UNKNOWN;

	num_buildings = 0;
	buildings = NULL;
	stab_num = 0;
	stablist = NULL;

	draw_array = NULL;
	draw_array_size = 0;

	closest.x = -1;
	closest.y = -1;
}

CLandBlock::~CLandBlock()
{
	Destroy();
}

DBObj* CLandBlock::Allocator()
{
	return((DBObj *)new CLandBlock());
}

void CLandBlock::Destroyer(DBObj* pLandBlock)
{
	delete ((CLandBlock *)pLandBlock);
}

CLandBlock *CLandBlock::Get(uint32_t ID)
{
	// LOG(Temp, Normal, "CLandBlock::Get(0x%08X)\n", ID);
	return (CLandBlock *)ObjCaches::LandBlocks->Get(ID);
}

void CLandBlock::Release(CLandBlock *pLandBlock)
{
	if (pLandBlock)
	{
		// LOG(Temp, Normal, "CLandBlock::Release(0x%08X) links: %d\n", pLandBlock->id, pLandBlock->m_lLinks);
		ObjCaches::LandBlocks->Release(pLandBlock->GetID());
	}
}

void CLandBlock::add_static_object(CPhysicsObj *pObject)
{
	if (num_static_objects >= static_objects.alloc_size)
		static_objects.grow(static_objects.alloc_size + 40);

	static_objects.array_data[num_static_objects++] = pObject;
}

void CLandBlock::destroy_static_objects(void)
{
	// the cells do this work
	static_objects.shrink(0);
	num_static_objects = 0;
}

void CLandBlock::destroy_buildings(void)
{

	if (buildings)
	{
		for (uint32_t i = 0; i < num_buildings; i++)
		{
			if (buildings[i])
			{
				buildings[i]->remove();
				delete buildings[i];
			}
		}

		delete[] buildings;
		buildings = NULL;
	}
	num_buildings = 0;

	if (stablist)
	{
		delete[] stablist;
		stablist = NULL;
		stab_num = 0;
	}
}

void CLandBlock::Destroy(void)
{
	destroy_static_objects();
	destroy_buildings();

	if (lbi)
	{
		CLandBlockInfo::Release(lbi);
		lbi = NULL;
	}

	closest.x = -1;
	closest.y = -1;

	lbi_exists = FALSE;

	if (draw_array)
	{
		delete[] draw_array;
		draw_array = NULL;
	}
}

void CLandBlock::init(void)
{
	CLandBlockStruct::init();
}

void CLandBlock::get_land_limits(void)
{
	int NumHeightVerts = LandDefs::side_vertex_count * LandDefs::side_vertex_count;

	BYTE High, Low;
	High = Low = height[0];

	for (int i = 1; i < NumHeightVerts; i++)
	{
		BYTE heightVal = height[i];

		if (High < heightVal)
			High = heightVal;
		if (Low > heightVal)
			Low = heightVal;
	}

	max_zval = LandDefs::Land_Height_Table[High] + LandDefs::max_object_height;
	min_zval = LandDefs::Land_Height_Table[High] - 1.0f;
}

void CLandBlock::init_lcell_ptrs(void)
{
	int NumLandCells = side_cell_count * side_cell_count;

	for (int i = 0; i < NumLandCells; i++)
		lcell[i].myLandBlock_ = this;
}

void CLandBlock::notify_change_size(void)
{
	// Not done.
}

void CLandBlock::release_all(void)
{
	release_objs();
	// CEnvCell::release_visible(stab_num, stablist);

	// Not exactly how the client does it, but similar I think?
	Release(this);
}

void CLandBlock::release_objs()
{
	if (side_vertex_count == 9)
	{
		for (uint32_t i = 0; i < side_cell_count; i++)
		{
			for (uint32_t j = 0; j < side_cell_count; ++j)
				lcell[(i * side_cell_count) + j].release_objects();
		}

		dyn_objs_init_done = 0;
	}
}

BOOL CLandBlock::UnPack(BYTE **ppData, ULONG iSize)
{
	Destroy();

	UNPACK(uint32_t, id);
	UNPACK(uint32_t, lbi_exists);

	CLandBlockStruct::UnPack(ppData, iSize);

	if (lbi_exists)
		lbi = CLandBlockInfo::Get((GetID() & LandDefs::blockid_mask) | LandDefs::lbi_cell_id);

	return TRUE;
}

CLandCell *CLandBlock::get_landcell(uint32_t cell_id)
{
	int32_t x, y;
	LandDefs::gid_to_lcoord(cell_id, x, y);

	uint32_t index = (y & 7) + ((x & 7) * side_cell_count);
	return (lcell[index].id == cell_id) ? &lcell[index] : NULL;
}

void CLandBlock::adjust_scene_obj_height()
{
	uint32_t num_lbi_objects = lbi ? lbi->num_objects : 0;

	while (num_lbi_objects < num_static_objects)
	{
		CPhysicsObj *static_obj = static_objects.data[num_lbi_objects];
		if (static_obj)
		{
			Frame obj_frame = static_obj->m_Position.frame;
			Vector obj_vector = obj_frame.m_origin;
			CPolygon *walkable = NULL;

			if (((CLandCell *)static_obj->cell)->find_terrain_poly(obj_vector, &walkable))
			{
				if (fabs(walkable->plane.m_normal.z) > F_EPSILON)
				{
					obj_vector.z = -(walkable->plane.dot_product(obj_vector) / walkable->plane.m_normal.z);
				}

				if (fabs(obj_vector.z - obj_frame.m_origin.z) > F_EPSILON)
				{
					obj_frame.m_origin = obj_vector;					
					static_obj->set_initial_frame(&obj_frame);
				}
			}
		}

		num_lbi_objects++;
	}
}

const BOOL use_scene_files = TRUE;

void CLandBlock::init_static_objs(LongNIValHash<uint32_t> *hash)
{
	if (side_cell_count == 8)
	{
		if (num_static_objects)
		{
			adjust_scene_obj_height();

			for (uint32_t i = 0; i < num_static_objects; i++)
			{
				CPhysicsObj *static_obj = static_objects.data[i];
				if (static_obj && !static_obj->is_completely_visible())
				{
					static_obj->calc_cross_cells_static();
				}
			}
		}
		else
		{
			if (lbi)
			{
				for (uint32_t i = 0; i < lbi->num_objects; i++)
				{
					CPhysicsObj *static_obj = CPhysicsObj::makeObject(lbi->object_ids[i], 0, 0);
					if (static_obj)
					{
						Position p;
						p.objcell_id = GetID();
						p.frame = lbi->object_frames[i];

						Vector loc = p.frame.m_origin;
						uint32_t cell_id = p.objcell_id;
						cell_id = LandDefs::adjust_to_outside(&cell_id, &loc) ? cell_id : 0;
						CLandCell *pLandCell = get_landcell(cell_id);

						if (pLandCell)
						{
							static_obj->add_obj_to_cell(pLandCell, &lbi->object_frames[i]);
							add_static_object(static_obj);
						}
						else
						{
							delete static_obj;
						}
					}
				}

				for (uint32_t i = 0; i < (side_cell_count * side_cell_count); i++)
				{					
					uint32_t restriction = lbi->GetRestrictionIID(lcell[i].GetID());
					if (restriction)
					{
						lcell[i].restriction_obj = restriction;
					}
				}
			}

			if (use_scene_files)
				get_land_scenes();
		}

		if (num_static_objects < static_objects.alloc_size)
			static_objects.shrink(num_static_objects);
	}
}

void CLandBlock::init_buildings()
{
	if (side_cell_count == 8)
	{
		if (lbi)
		{
			if (!buildings)
			{
				num_buildings = 0;
				stab_num = 0;
				uint32_t max_stab = 0;

				if (lbi->num_buildings > 0)
					buildings = new CBuildingObj *[lbi->num_buildings];

				for (uint32_t i = 0; i < lbi->num_buildings; i++)
				{
					BuildInfo *buildInfo = lbi->buildings[i];
					
					CBuildingObj *building = 
						CBuildingObj::makeBuilding(
							buildInfo->building_id,
							buildInfo->num_portals,
							buildInfo->portals,
							buildInfo->num_leaves);

					if (building)
					{
						Position p;
						p.objcell_id = GetID();
						p.frame = buildInfo->building_frame;
						
						Vector loc = p.frame.m_origin;
						uint32_t cell_id = p.objcell_id;
						BOOL adjusted = LandDefs::adjust_to_outside(&cell_id, &loc);
						CLandCell *pLandCell = get_landcell(adjusted ? cell_id : 0);
						if (pLandCell)
						{
							building->set_initial_frame(&buildInfo->building_frame);
							building->add_to_cell(pLandCell);
							buildings[num_buildings++] = building;
							building->add_to_stablist(stablist, max_stab, stab_num);
						}
						else
							delete building;						
					}
				}
			}
		}
	}
}

void CLandBlock::get_land_scenes()
{
#if 0
	static Frame obj_frame;
	static Position obj_pos;
	
	if (CRegionDesc::current_region)
	{
		for (uint32_t i = 0; i < side_vertex_count; i++)
		{
			for (uint32_t j = 0; j < side_vertex_count; j++)
			{
				uint32_t v4 = (terrain[(i * side_vertex_count) + j] >> 2) & 0x1F;
				uint32_t v6 = (terrain[(i * side_vertex_count) + j] >> 2) & 0x1F;
				uint32_t scene_id = (terrain[(i * side_vertex_count) + j] >> 11);

				if (scene_id < CRegionDesc::current_region->NumSceneType(v6))
				{
					uint32_t scene_count = CRegionDesc::current_region->SceneCount(v4, scene_id);

					if (scene_count)
					{
						uint32_t v9 = i + block_coord.x;
						uint32_t v10 = j + block_coord.y;

						double v11 = (double)(v10 * (712977289 * v9 + 1813693831) - 1109124029 * v9 + 2139937281) * 2.3283064e-10;
						uint32_t v12 = (uint32_t) floor(v11 * scene_count);
						
						if (scene_count == 1 || (v12 >= scene_count))
						{
							v12 = 0;
						}

						uint32_t sid = CRegionDesc::current_region->GetScene(v4, scene_id, v12);

						if (sid != 0)
						{
							Scene *pScene = Scene::Get(sid);

							if (pScene)
							{
								uint32_t v44 = 1813693831 * v10;
								int v16 = -1109124029 * v9;
								uint32_t v38 = 1360117743 * v9 * v10 + 1888038839;
								uint32_t v17 = 23399 * v38;
								int v43 = -1109124029 * v9;

								for (uint32_t k = 0; k < pScene->num_objects; k++)
								{
									ObjectDesc *obj = &pScene->objects[k];
									uint32_t someval = v16 + v44 - v17;

									if ((double)someval * 2.3283064e-10 < obj->freq && !obj->weenie_obj)
									{
										uint32_t obj_id = obj->obj_id;

										Vector obj_vector;
										obj->Place(v9, v10, k, &obj_vector);
										obj_vector.x = (double)i * 24.0 + obj_vector.x;
										obj_vector.y = (double)j * 24.0 + obj_vector.y;

										const float flt_844AEC = 24.0 * 8.0;

										if (obj_vector.x >= 0.0
											&& obj_vector.y >= 0.0
											&& obj_vector.x < (double)flt_844AEC
											&& obj_vector.y < (double)flt_844AEC
											&& !on_road(&obj_vector))
										{
											Position p;
											p.objcell_id = GetID();
											p.frame.m_origin = obj_vector;
											CLandCell *pCell = get_landcell(p.get_outside_cell_id());


											if (!CSortCell::has_building((CSortCell *)&cell->vfptr)
												&& CLandCell::find_terrain_poly(cell, &obj_vector, &walkable)
												&& ObjectDesc::CheckSlope((ObjectDesc *)v18, walkable->plane.N.z))
											{
												v20 = &walkable->plane;
												Plane::set_height(&walkable->plane, &obj_vector);
												if (*(_uint32_t *)(v18 + 100))
													ObjectDesc::ObjAlign((ObjectDesc *)v18, v20, &obj_vector, &obj_frame);
												else
													ObjectDesc::GetObjFrame((ObjectDesc *)v18, v9, v10, kq, &obj_vector, &obj_frame);
												v21 = CPhysicsObj::makeObject(obj_id, 0, 0);
												v22 = v21;
												if (v21)
												{
													CPhysicsObj::set_initial_frame(v21, &obj_frame);
													if (CPhysicsObj::obj_within_block(v22))
													{
														CPhysicsObj::add_obj_to_cell(v22, (CObjCell *)&cell->vfptr, &obj_frame);
														v23 = ObjectDesc::ScaleObj((ObjectDesc *)v18, v9, v10, kq);
														CPhysicsObj::SetScaleStatic(v22, v23);
														CLandBlock::add_static_object(this, v22);
													}
													else
													{
														v22->vfptr->__vecDelDtor((HashBaseData<unsigned long> *)v22, 1u);
													}
												}
												v15 = scene;
											}
											p.vfptr = (PackObjVtbl *)&PackObj::`vftable';
										}
									}
									v17 = v38 + scene_id;
									v24 = kq++ + 1 < v15->num_objects;
									scene_id += v38;
									v35 += 112;
									if (!v24)
										break;
									v16 = v43;
								}

								Scene::Release(pScene);




								kq = 0;
								if (v14->num_objects)
								{
									v35 = 0;
									v44 = 1813693831 * v10;
									v16 = -1109124029 * v9;
									v38 = 1360117743 * v9 * v10 + 1888038839;
									v17 = 23399 * v38;
									v43 = -1109124029 * v9;
									scene_id = 23399 * v38;
									while (1)
									{
										v18 = (int)&v15->objects[v35 / 0x70];
										obj_id.id = v16 + v44 - v17;
										if ((double)obj_id.id * 2.3283064e-10 < *(float *)(v18 + 68) && !*(_uint32_t *)(v18 + 108))
										{
											obj_id.id = *(_uint32_t *)v18;
											ObjectDesc::Place((ObjectDesc *)v18, v9, v10, kq, &obj_vector);
											obj_vector.x = (double)iq * 24.0 + obj_vector.x;
											obj_vector.y = (double)jq * 24.0 + obj_vector.y;
											if (obj_vector.x >= 0.0
												&& obj_vector.y >= 0.0
												&& obj_vector.x < (double)flt_844AEC
												&& obj_vector.y < (double)flt_844AEC
												&& !CLandBlock::on_road(this, &obj_vector))
											{
												p.vfptr = (PackObjVtbl *)&Position::`vftable';
													p.objcell_id = 0;
												LOuint32_t(p.frame.qw) = 1065353216;
												LOuint32_t(p.frame.qx) = 0;
												LOuint32_t(p.frame.qy) = 0;
												LOuint32_t(p.frame.qz) = 0;
												LOuint32_t(p.frame.m_fOrigin.x) = 0;
												LOuint32_t(p.frame.m_fOrigin.y) = 0;
												LOuint32_t(p.frame.m_fOrigin.z) = 0;
												Frame::cache(&p.frame);
												p.objcell_id = m_DID.id;
												p.frame.m_fOrigin.y = obj_vector.y;
												p.frame.m_fOrigin.x = obj_vector.x;
												p.frame.m_fOrigin.z = obj_vector.z;
												v19 = Position::get_outside_cell_id(&p);
												cell = CLandBlock::get_landcell(this, v19);
												if (!CSortCell::has_building((CSortCell *)&cell->vfptr)
													&& CLandCell::find_terrain_poly(cell, &obj_vector, &walkable)
													&& ObjectDesc::CheckSlope((ObjectDesc *)v18, walkable->plane.N.z))
												{
													v20 = &walkable->plane;
													Plane::set_height(&walkable->plane, &obj_vector);
													if (*(_uint32_t *)(v18 + 100))
														ObjectDesc::ObjAlign((ObjectDesc *)v18, v20, &obj_vector, &obj_frame);
													else
														ObjectDesc::GetObjFrame((ObjectDesc *)v18, v9, v10, kq, &obj_vector, &obj_frame);
													v21 = CPhysicsObj::makeObject(obj_id, 0, 0);
													v22 = v21;
													if (v21)
													{
														CPhysicsObj::set_initial_frame(v21, &obj_frame);
														if (CPhysicsObj::obj_within_block(v22))
														{
															CPhysicsObj::add_obj_to_cell(v22, (CObjCell *)&cell->vfptr, &obj_frame);
															v23 = ObjectDesc::ScaleObj((ObjectDesc *)v18, v9, v10, kq);
															CPhysicsObj::SetScaleStatic(v22, v23);
															CLandBlock::add_static_object(this, v22);
														}
														else
														{
															v22->vfptr->__vecDelDtor((HashBaseData<unsigned long> *)v22, 1u);
														}
													}
													v15 = scene;
												}
												p.vfptr = (PackObjVtbl *)&PackObj::`vftable';
											}
										}
										v17 = v38 + scene_id;
										v24 = kq++ + 1 < v15->num_objects;
										scene_id += v38;
										v35 += 112;
										if (!v24)
											break;
										v16 = v43;
									}
								}
								v15->vfptr->Release((Interface *)v15);
							}
						}
				}
			}
		}

						v4 = ((unsigned int)*(unsigned __int16 *)((char *)terrain + v36) >> 2) & 0x1F;
						v6 = ((unsigned int)*(unsigned __int16 *)((char *)terrain + v36) >> 2) & 0x1F;
						scene_id = (unsigned int)*(unsigned __int16 *)((char *)terrain + v36) >> 11;
						v5 = scene_id;
						if (v5 < CRegionDesc::current_region->NumSceneType(v6))
						{
							v7 = CRegionDesc::current_region->SceneCount(v4, v5);
							v8 = v7;
							if (v7)
							{
								v9 = iq + block_coord.x;
								v10 = jq + block_coord.y;
								if (v7 == 1
									|| (v38 = v10 * (712977289 * v9 + 1813693831) - 1109124029 * v9 + 2139937281,
										v11 = (double)v38 * 2.3283064e-10,
										v38 = v7,
										v12 = (uint64_t)_floor(v11 * (double)v7),
										(unsigned int)v12 >= v8))
								{
									LOuint32_t(v12) = 0;
								}
								CRegionDesc::current_region->GetScene(&sid, v4, scene_id, v12);
								if (sid.id != stru_844AF4.id)
								{
									QualifiedDataID::QualifiedDataID(&v47, sid, 0x1Bu);
									v14 = (Scene *)DBObj::Get(v13);
									v15 = v14;
									scene = v14;
									if (v14)
									{
										kq = 0;
										if (v14->num_objects)
										{
											v35 = 0;
											v44 = 1813693831 * v10;
											v16 = -1109124029 * v9;
											v38 = 1360117743 * v9 * v10 + 1888038839;
											v17 = 23399 * v38;
											v43 = -1109124029 * v9;
											scene_id = 23399 * v38;
											while (1)
											{
												v18 = (int)&v15->objects[v35 / 0x70];
												obj_id.id = v16 + v44 - v17;
												if ((double)obj_id.id * 2.3283064e-10 < *(float *)(v18 + 68) && !*(_uint32_t *)(v18 + 108))
												{
													obj_id.id = *(_uint32_t *)v18;
													ObjectDesc::Place((ObjectDesc *)v18, v9, v10, kq, &obj_vector);
													obj_vector.x = (double)iq * 24.0 + obj_vector.x;
													obj_vector.y = (double)jq * 24.0 + obj_vector.y;
													if (obj_vector.x >= 0.0
														&& obj_vector.y >= 0.0
														&& obj_vector.x < (double)flt_844AEC
														&& obj_vector.y < (double)flt_844AEC
														&& !CLandBlock::on_road(this, &obj_vector))
													{
														p.vfptr = (PackObjVtbl *)&Position::`vftable';
															p.objcell_id = 0;
														LOuint32_t(p.frame.qw) = 1065353216;
														LOuint32_t(p.frame.qx) = 0;
														LOuint32_t(p.frame.qy) = 0;
														LOuint32_t(p.frame.qz) = 0;
														LOuint32_t(p.frame.m_fOrigin.x) = 0;
														LOuint32_t(p.frame.m_fOrigin.y) = 0;
														LOuint32_t(p.frame.m_fOrigin.z) = 0;
														Frame::cache(&p.frame);
														p.objcell_id = m_DID.id;
														p.frame.m_fOrigin.y = obj_vector.y;
														p.frame.m_fOrigin.x = obj_vector.x;
														p.frame.m_fOrigin.z = obj_vector.z;
														v19 = Position::get_outside_cell_id(&p);
														cell = CLandBlock::get_landcell(this, v19);
														if (!CSortCell::has_building((CSortCell *)&cell->vfptr)
															&& CLandCell::find_terrain_poly(cell, &obj_vector, &walkable)
															&& ObjectDesc::CheckSlope((ObjectDesc *)v18, walkable->plane.N.z))
														{
															v20 = &walkable->plane;
															Plane::set_height(&walkable->plane, &obj_vector);
															if (*(_uint32_t *)(v18 + 100))
																ObjectDesc::ObjAlign((ObjectDesc *)v18, v20, &obj_vector, &obj_frame);
															else
																ObjectDesc::GetObjFrame((ObjectDesc *)v18, v9, v10, kq, &obj_vector, &obj_frame);
															v21 = CPhysicsObj::makeObject(obj_id, 0, 0);
															v22 = v21;
															if (v21)
															{
																CPhysicsObj::set_initial_frame(v21, &obj_frame);
																if (CPhysicsObj::obj_within_block(v22))
																{
																	CPhysicsObj::add_obj_to_cell(v22, (CObjCell *)&cell->vfptr, &obj_frame);
																	v23 = ObjectDesc::ScaleObj((ObjectDesc *)v18, v9, v10, kq);
																	CPhysicsObj::SetScaleStatic(v22, v23);
																	CLandBlock::add_static_object(this, v22);
																}
																else
																{
																	v22->vfptr->__vecDelDtor((HashBaseData<unsigned long> *)v22, 1u);
																}
															}
															v15 = scene;
														}
														p.vfptr = (PackObjVtbl *)&PackObj::`vftable';
													}
												}
												v17 = v38 + scene_id;
												v24 = kq++ + 1 < v15->num_objects;
												scene_id += v38;
												v35 += 112;
												if (!v24)
													break;
												v16 = v43;
											}
										}
										v15->vfptr->Release((Interface *)v15);
									}
								}
								v1 = this;
								v2 = region;
							}
						}
						v25 = side_vertex_count;
						v27 = __OFSUB__(jq + 1, v25);
						v26 = jq++ + 1 - v25 < 0;
						v36 += 2;
					} while (v26 ^ v27);
				}
				v3 = side_vertex_count;
				v27 = __OFSUB__(iq + 1, v3);
				v26 = iq++ + 1 - v3 < 0;
				v37 += 18;
			} while (v26 ^ v27);
		}
	}
#endif
}



