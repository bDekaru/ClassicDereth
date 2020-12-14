#include <StdAfx.h>
#include "Polygon.h"
#include "BSPData.h"
#include "Transition.h"
#include "LandDefs.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

CPolygon *BSPNODE::pack_poly;
uint32_t BSPNODE::pack_tree_type;

CSphere::CSphere(const Vector& center_, float radius_)
{
	center = center_;
	radius = radius_;
}

CSphere::CSphere()
{
	center = Vector(0, 0, 0);
	radius = 0.0f;
}

ULONG CSphere::pack_size()
{
	return(center.pack_size() + sizeof(float));
}

BOOL CSphere::UnPack(BYTE** ppData, ULONG iSize)
{
	if (iSize < pack_size())
		return FALSE;

	UNPACK_OBJ(center);
	UNPACK(float, radius);

	return TRUE;
}

BOOL CSphere::intersects(CSphere* pSphere)
{
	// The offset of the spheres.
	Vector offset = center - pSphere->center;

	// The sum of the radius of both spheres.
	float reach = radius + pSphere->radius;

	float dist = offset.dot_product(offset) - (reach * reach);

	// Do they intersect?
	if (F_EPSILON > dist)
		return TRUE;

	return FALSE;
}

BOOL CSphere::sphere_intersects_ray(const Ray& ray)
{
	// The offset of the spheres.
	Vector offset = ray.m_origin - center;

	float off_dir_dp = offset.dot_product(ray.m_direction);
	float off_off_dp = offset.dot_product(offset);
	float dir_dir_dp = ray.m_direction.dot_product(ray.m_direction);

	float compare = dir_dir_dp * (off_off_dp - (radius * radius));

	if (compare < (off_dir_dp * off_dir_dp))
		return TRUE;

	return FALSE;
}

double CSphere::find_time_of_collision(const Vector &movement, const Vector &disp, float radsum)
{
	double result;

	double v4 = (movement.x * movement.x) + (movement.y * movement.y) + (movement.z * movement.z);
	double v5 = -((disp.z * movement.z) + (disp.y * movement.y) + (movement.x * disp.x));
	double v6 = ((disp.x * disp.x) + (disp.y * disp.y) + (disp.z * disp.z)) - (radsum * radsum);

	double v7 = (v5 * v5) - (v6 * v4);

	if (v6 < F_EPSILON || v4 < 0.00019999999 || v7 < 0.0)
	{
		result = -1.0;
	}
	else
	{
		double v8 = sqrt(v7);

		if ((v5 - v8) < 0.0)
			result = (v8 - ((disp.z * movement.z) + (disp.y * movement.y) + (movement.x * disp.x))) / v4;
		else
			result = (v5 - v8) / v4;
	}
	return result;
}

BOOL CSphere::collides_with_sphere(Vector *disp, const float radsum)
{
	return (radsum >= disp->magnitude());
}

TransitionState CSphere::collide_with_point(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum, int sphere_num)
{
	TransitionState result;

	if (object.state & OBJECTINFO::PERFECT_CLIP_OI)
	{
		float radsuma = radsum + F_EPSILON;
		Vector old_disp = path.global_curr_center[sphere_num] - center;

		Vector collision_normal = LandDefs::get_block_offset(path.curr_pos.objcell_id, path.check_pos.objcell_id);

		// v9 = path.global_curr_center[sphere_num]
		Vector offset = collision_normal + (check_pos.center - path.global_curr_center[sphere_num]);
		collision_normal = offset;

		float coltime = (float)find_time_of_collision(collision_normal, old_disp, radsuma);
		if (coltime < F_EPSILON || coltime > 1.0)
		{
			result = TransitionState::COLLIDED_TS;
		}
		else
		{
			offset = (offset * coltime) - offset;
			collision_normal = ((offset + check_pos.center) - center) * (1.0f / radsum);

			collisions.set_collision_normal(collision_normal);
			path.add_offset_to_check_pos(&offset, check_pos.radius);
			result = TransitionState::ADJUSTED_TS;
		}
	}
	else
	{
		Vector offset = path.global_curr_center[sphere_num] - center;

		if (!offset.normalize_check_small())
			collisions.set_collision_normal(offset);

		result = TransitionState::COLLIDED_TS;
	}
	return result;
}


TransitionState CCylSphere::collide_with_point(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum, int sphere_num)
{
	Vector collision_normal;
	BOOL definate = normal_of_collision(path, check_pos, disp, radsum, sphere_num, &collision_normal);

	if (collision_normal.normalize_check_small())
		return COLLIDED_TS;

	if (!(object->state & OBJECTINFO::PERFECT_CLIP_OI))
	{
		collisions->set_collision_normal(collision_normal);
		return COLLIDED_TS;
	}

	Vector movement = LandDefs::get_block_offset(path->curr_pos.objcell_id, path->check_pos.objcell_id);
	movement += (check_pos->center - path->global_curr_center[0]); // doesn't use sphere # ?		
	Vector old_disp = path->global_curr_center[0] - low_pt; // doesn't use sphere # ?
	float radsuma = radsum + F_EPSILON;

	if (!definate)
	{
		if (fabs(movement.z) < 0.00019999999)
			return COLLIDED_TS;

		Vector offset;
		double v15;
		if (movement.z > 0.0)
		{
			offset.z = -1.0f;
			v15 = -((old_disp.z + check_pos->radius) / movement.z);
		}
		else
		{
			offset.z = 1.0f;
			v15 = (check_pos->radius + height - old_disp.z) / movement.z;
		}

		double time = v15;
		collision_normal = Vector(0, 0, offset.z);

		Vector v67;
		Vector result;

		v67.x = movement.x * v15;
		offset.x = v67.x;
		result.x = v67.x;
		v67.y = movement.y * v15;
		result.y = v67.y;
		offset.y = v67.y;
		result.z = v15 * movement.z;
		offset.z = result.z;

		float definatea = radsuma * radsuma;
		if ((v67.y + old_disp.y) * (v67.y + old_disp.y) + (v67.x + old_disp.x) * (v67.x + old_disp.x) >= definatea)
		{
			double v16 = movement.y*movement.y + movement.x*movement.x;
			if (fabs(v16) < F_EPSILON)
				return COLLIDED_TS;

			double v17 = -(movement.y * old_disp.y + movement.x * old_disp.x);
			double v18 = v17 * v17 - (old_disp.y * old_disp.y + old_disp.x * old_disp.x - definatea) * v16;

			if (v18 >= 0.0 && v16 > F_EPSILON)
			{
				double v19 = sqrt(v18);
				double v20;
				if (v17 - v19 < 0.0)
				{
					time = (v19 - (movement.y * old_disp.y + movement.x * old_disp.x)) / v16;
					v20 = time;
				}
				else
				{
					v20 = (v17 - v19) / v16;
					time = v20;
				}
				double _f = v20;
				offset = movement * _f;
			}

			Vector v25 = path->global_curr_center[0] + offset;
			Vector v26 = v25 - low_pt;

			collision_normal = v26;
			collision_normal.z = 0.0f;
			collision_normal /= radsuma;
		}

		if (time >= 0.0)
		{
			if (time > 1.0)
				return COLLIDED_TS;

			offset = (path->global_curr_center[0] + offset) - check_pos->center;
			collisions->set_collision_normal(collision_normal);
			path->add_offset_to_check_pos(&offset, check_pos->radius);
			return ADJUSTED_TS;
		}

		return COLLIDED_TS;
	}

	if (collision_normal.z != 0.0)
	{
		if (fabs(movement.z) < F_EPSILON)
			return COLLIDED_TS;

		double time;
		if (movement.z > 0.0)
			time = -((old_disp.z + check_pos->radius) / movement.z);
		else
			time = (check_pos->radius + height - old_disp.z) / movement.z;

		Vector offset = movement * time;
		if (time >= 0.0)
		{
			if (time > 1.0)
				return COLLIDED_TS;

			Vector v50 = path->global_curr_center[0] + offset;
			offset = v50 - check_pos->center;
			collisions->set_collision_normal(collision_normal);
			path->add_offset_to_check_pos(&offset, check_pos->radius);
			return ADJUSTED_TS;
		}

		return COLLIDED_TS;
	}
	
	double v36 = movement.y * movement.y + movement.x * movement.x;
	double v37 = -(movement.y * old_disp.y + movement.x * old_disp.x);
	double v38 = v37 * v37 - (old_disp.y * old_disp.y + old_disp.x * old_disp.x - radsuma * radsuma) * v36;
	if (v38 >= 0.0 && v36 >= F_EPSILON)
	{
		double v39 = sqrt(v38);
		double v40;
		double timea;

		if (v37 - v39 < 0.0)
		{
			timea = (v39 - (movement.y * old_disp.y + movement.x * old_disp.x)) / v36;
			v40 = timea;
		}
		else
		{
			v40 = (v37 - v39) / v36;
			timea = v40;
		}

		Vector offset = movement * v40;
		if (timea >= 0.0 && timea <= 1.0)
		{
			Vector collision_normal = (path->global_curr_center[0] + offset) - low_pt;
			collision_normal.z = 0.0f;
			collision_normal /= radsum;

			offset = (path->global_curr_center[0] + offset) - check_pos->center;

			collisions->set_collision_normal(collision_normal);
			path->add_offset_to_check_pos(&offset, check_pos->radius);
			return ADJUSTED_TS;
		}
	}
	return COLLIDED_TS;
}

TransitionState CSphere::step_sphere_down(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum)
{
	if (!collides_with_sphere(&disp, radsum))
	{
		if (path.num_sphere <= 1)
			return TransitionState::OK_TS;

		Vector disp2 = path.global_sphere[1].center - center;

		if (!collides_with_sphere(&disp2, radsum))
			return TransitionState::OK_TS;
	}

	float step_down_interp = path.step_down_amt * path.walk_interp;

	float radsuma = radsum + F_EPSILON;
	float v13 = (sqrt((radsuma*radsuma) - ((disp.x*disp.x) + (disp.y*disp.y))) - disp.z) / step_down_interp;

	float timecheck = (1.0f - v13) * path.walk_interp;

	if (fabs(step_down_interp) >= F_EPSILON && (timecheck < path.walk_interp) && (timecheck >= -0.1))
	{
		float step_down_interp2 = step_down_interp * v13;

		float invRadSum = (1 / radsuma);
		disp.x = disp.x * invRadSum;
		disp.y = disp.y * invRadSum;
		disp.z = (disp.z + step_down_interp2) * invRadSum;

		if (disp.z <= path.walkable_allowance)
			return TransitionState::OK_TS;

		Vector disp2 = center + (disp * radius);

		Plane rest_plane(disp, disp2);
		collisions.set_contact_plane(&rest_plane, 1);

		collisions.contact_plane_cell_id = path.check_pos.objcell_id;
		path.walk_interp = timecheck;

		disp2.x = 0;
		disp2.y = 0;
		disp2.z = step_down_interp2;
		path.add_offset_to_check_pos(&disp2, check_pos.radius);

		return TransitionState::ADJUSTED_TS;
	}

	return TransitionState::COLLIDED_TS;
}

TransitionState CSphere::slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float *radsum, const int sphere_number)
{
	Vector collision_normal = path->global_curr_center[sphere_number] - center;

	if (collision_normal.normalize_check_small())
		return TransitionState::COLLIDED_TS;

	return check_pos->slide_sphere(path, collisions, &collision_normal, &path->global_curr_center[sphere_number]);
}

TransitionState CSphere::slide_sphere(SPHEREPATH *path, COLLISIONINFO *collisions, Vector *collision_normal, Vector *curr_pos)
{
	// !! collision_normal gets modified

	if (collision_normal->x == 0.0 && collision_normal->y == 0.0 && collision_normal->z == 0.0)
	{
		Vector offset = *curr_pos - center;
		Vector direction = offset * 0.5;

		path->add_offset_to_check_pos(&direction, radius);
		return TransitionState::ADJUSTED_TS;
	}

	collisions->set_collision_normal(*collision_normal);

	Vector offset = LandDefs::get_block_offset(path->curr_pos.objcell_id, path->check_pos.objcell_id);

	Vector v23_24_25 = offset + (center - *curr_pos);

	Plane *plane = &collisions->contact_plane;
	if (!collisions->contact_plane_valid)
		plane = &collisions->last_known_contact_plane;

	// 	return Vector(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);

	Vector direction = cross_product(*collision_normal, plane->m_normal);
	float patha = direction.sum_of_square();

	if (patha >= F_EPSILON)
	{
		float dp = direction.dot_product(v23_24_25);

		float invPatha = 1.0f / patha;
		offset.x = direction.x * dp * invPatha;
		offset.y = direction.y * dp * invPatha;
		offset.z = direction.z * dp * invPatha;

		if (offset.sum_of_square() < F_EPSILON)
			return TransitionState::COLLIDED_TS;

		offset = offset - v23_24_25;
		path->add_offset_to_check_pos(&offset, radius);
		return TransitionState::SLID_TS;
	}

	if (collision_normal->dot_product(plane->m_normal) >= 0.0)
	{
		float dp = collision_normal->dot_product(v23_24_25);

		direction.x = -collision_normal->x * dp;
		direction.y = -collision_normal->y * dp;
		direction.z = -collision_normal->z * dp;

		path->add_offset_to_check_pos(&direction, radius);
		return TransitionState::SLID_TS;
	}

	*collision_normal = v23_24_25 * -1.0f;

	if (!collision_normal->normalize_check_small())
		collisions->set_collision_normal(*collision_normal);

	return TransitionState::OK_TS;
}

TransitionState CSphere::slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, Vector *disp, float radsum, int sphere_num)
{
	Vector collision_normal = path->global_curr_center[sphere_num] - center;

	if (collision_normal.normalize_check_small())
		return TransitionState::COLLIDED_TS;

	collisions->set_collision_normal(collision_normal);

	Plane *somePlane;
	if (collisions->contact_plane_valid)
		somePlane = &collisions->contact_plane;
	else
		somePlane = &collisions->last_known_contact_plane;

	Vector skid_dir = somePlane->m_normal;

	Vector direction;

	// cross product?
	direction.x = skid_dir.z * collision_normal.y - skid_dir.y * collision_normal.z;
	direction.y = collision_normal.z * skid_dir.x - skid_dir.z * collision_normal.x;
	direction.z = skid_dir.y * collision_normal.x - collision_normal.y * skid_dir.x;

	Vector blockOffset = LandDefs::get_block_offset(path->curr_pos.objcell_id, path->check_pos.objcell_id);

	Vector someVec = blockOffset + (path->global_sphere[sphere_num].center - path->global_curr_center[sphere_num]);

	float someValue = (direction.x*direction.x) + (direction.y*direction.y) + (direction.z*direction.z);

	if (someValue >= F_EPSILON)
	{
		float dp = someVec.dot_product(direction);
		skid_dir = direction * dp;

		float invSomeValue = 1.0f / someValue;

		collision_normal.x = skid_dir.x * invSomeValue;
		skid_dir.x = collision_normal.x;
		direction.x = collision_normal.x;

		skid_dir.y = skid_dir.y * invSomeValue;
		direction.y = skid_dir.y;

		skid_dir.z = skid_dir.z * invSomeValue;
		direction.z = skid_dir.z;

		direction.x = skid_dir.x = collision_normal.x = skid_dir.x * invSomeValue;
		direction.y = skid_dir.y = skid_dir.y * invSomeValue;
		direction.z = skid_dir.z = skid_dir.z * invSomeValue;

		if ((collision_normal.x * collision_normal.x) < F_EPSILON)
			return TransitionState::COLLIDED_TS;

		direction.x = collision_normal.x - someVec.x;
		direction.y = direction.y - someVec.y;
		direction.z = direction.z - someVec.z;

		path->add_offset_to_check_pos(&direction, path->global_sphere[sphere_num].radius);
		return TransitionState::SLID_TS;
	}

	if (skid_dir.dot_product(*disp) < 0.0)
		return TransitionState::COLLIDED_TS;

	float dp = someVec.dot_product(collision_normal);

	direction = collision_normal * -dp;
	path->add_offset_to_check_pos(&direction, path->global_sphere[sphere_num].radius);
	return TransitionState::SLID_TS;
}

TransitionState CSphere::step_sphere_up(CTransition *transition, CSphere &check_pos, Vector &disp, float radsum)
{
	float radsuma = radsum + F_EPSILON;

	if (transition->object_info.step_up_height < (radsuma - disp.z))
	{
		return slide_sphere(
			&transition->object_info,
			&transition->sphere_path,
			&transition->collision_info,
			&disp,
			radsuma,
			0);
	}
	else
	{
		Vector collision_normal = *transition->sphere_path.global_curr_center - center;

		if (transition->step_up(&collision_normal))
			return TransitionState::OK_TS;
		else
			return transition->sphere_path.step_up_slide(
				&transition->object_info,
				&transition->collision_info);
	}
}

const float z_for_landing_0 = 0.0871557f;

TransitionState CSphere::land_on_sphere(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum)
{
	Vector collision_normal = *path.global_curr_center - center;

	if (collision_normal.normalize_check_small())
	{
		return TransitionState::COLLIDED_TS;
	}
	else
	{
		path.set_collide(collision_normal);
		path.walkable_allowance = z_for_landing_0;
		return TransitionState::ADJUSTED_TS;
	}
}

TransitionState CSphere::intersects_sphere(CTransition *_transition, int is_creature)
{
	Vector disp = _transition->sphere_path.global_sphere[0].center - center;
	float radsum = (radius + _transition->sphere_path.global_sphere[0].radius) - F_EPSILON;

	int result = 1;
	if (_transition->sphere_path.obstruction_ethereal || _transition->sphere_path.insert_type == SPHEREPATH::PLACEMENT_INSERT)
	{
		if (radsum >= disp.magnitude())
			return TransitionState::COLLIDED_TS;

		if (_transition->sphere_path.num_sphere >= 2)
		{
			Vector disp = _transition->sphere_path.global_sphere[1].center - center;

			if (collides_with_sphere(&disp, radsum))
				return TransitionState::COLLIDED_TS;
		}

		return TransitionState::OK_TS;
	}
	else
	{
		if (_transition->sphere_path.step_down)
		{
			if (is_creature)
				return TransitionState::OK_TS;

			return step_sphere_down(
				_transition->object_info,
				_transition->sphere_path,
				_transition->collision_info,
				_transition->sphere_path.global_sphere[0],
				disp,
				radsum);
		}
		else
		{
			if (_transition->sphere_path.check_walkable)
			{
				if (collides_with_sphere(&disp, radsum))
					return TransitionState::COLLIDED_TS;

				if (_transition->sphere_path.num_sphere >= 2)
				{
					disp = _transition->sphere_path.global_sphere[1].center - center;

					if (collides_with_sphere(&disp, radsum))
						return TransitionState::COLLIDED_TS;
				}

				return TransitionState::OK_TS;
			}
			else
			{
				if (_transition->sphere_path.collide)
				{
					if (!is_creature)
						return TransitionState::OK_TS;

					if (!collides_with_sphere(&disp, radsum))
					{
						if (_transition->sphere_path.num_sphere >= 2)
						{
							Vector disp2 = _transition->sphere_path.global_sphere[1].center - center;

							if (!collides_with_sphere(&disp2, radsum))
								return TransitionState::OK_TS;
						}
					}

					Vector blockOffset = _transition->sphere_path.get_curr_pos_check_pos_block_offset();
					Vector someOffset = *_transition->sphere_path.global_curr_center - _transition->sphere_path.global_sphere->center;
					Vector movement = someOffset - blockOffset;
					radsum += F_EPSILON;

					float someValue1 = (movement.x*movement.x) + (movement.y*movement.y) + (movement.z*movement.z);
					float someValue2 = -((movement.x*disp.x) + (movement.y*disp.y) + (movement.z*disp.z));

					if (fabs(someValue1) < F_EPSILON)
						return TransitionState::COLLIDED_TS;

					float someValue3 = someValue2 + sqrt((someValue2*someValue2) - ((((disp.x*disp.x) + (disp.y*disp.y) + (disp.z*disp.z)) - (radsum*radsum))*someValue1));

					if (someValue3 > 1.0)
					{
						someValue3 = (someValue2 + someValue2) - someValue3;
					}

					float someFraction = someValue3 / someValue1;
					float timechecka = (1 - someFraction) * _transition->sphere_path.walk_interp;
					if (timechecka >= _transition->sphere_path.walk_interp || timechecka < -0.1)
						return TransitionState::COLLIDED_TS;

					movement *= someFraction;
					disp += movement;
					disp /= radsum;

					if (!_transition->sphere_path.is_walkable_allowable(disp.z))
						return TransitionState::OK_TS;

					Vector someVector = disp * _transition->sphere_path.global_sphere->radius;
					Vector disp2 = _transition->sphere_path.global_sphere->center - someVector;

					Plane contact_plane(disp, disp2);

					_transition->collision_info.set_contact_plane(&contact_plane, TRUE);
					_transition->collision_info.contact_plane_cell_id = _transition->sphere_path.check_pos.objcell_id;
					_transition->sphere_path.walk_interp = timechecka;

					_transition->sphere_path.add_offset_to_check_pos(&movement, _transition->sphere_path.global_sphere->radius);

					return TransitionState::ADJUSTED_TS;
				}
				else
				{
					if (_transition->object_info.state & (OBJECTINFO::CONTACT_OI | OBJECTINFO::ON_WALKABLE_OI))
					{
						if (collides_with_sphere(&disp, radsum))
						{
							return step_sphere_up(
								_transition,
								_transition->sphere_path.global_sphere[0],
								disp, radsum);
						}

						if (_transition->sphere_path.num_sphere >= 2)
						{
							Vector disp2 = _transition->sphere_path.global_sphere[1].center - center;

							if (collides_with_sphere(&disp2, radsum))
							{
								return slide_sphere(
									&_transition->object_info,
									&_transition->sphere_path,
									&_transition->collision_info,
									&_transition->sphere_path.global_sphere[1],
									&disp2, &radsum, 1);
							}
						}

						return TransitionState::OK_TS;
					}
					else if (_transition->object_info.state & (OBJECTINFO::PATH_CLIPPED_OI))
					{
						if (collides_with_sphere(&disp, radsum))
						{
							return collide_with_point(
								_transition->object_info,
								_transition->sphere_path,
								_transition->collision_info,
								_transition->sphere_path.global_sphere[0],
								disp, radsum, 0);
						}

						return TransitionState::OK_TS;
					}
					else
					{
						if (collides_with_sphere(&disp, radsum))
						{
							return land_on_sphere(
								_transition->object_info,
								_transition->sphere_path,
								_transition->collision_info,
								_transition->sphere_path.global_sphere[0],
								disp, radsum);
						}
						else
						{
							if (_transition->sphere_path.num_sphere >= 2)
							{
								Vector disp2 = _transition->sphere_path.global_sphere[1].center - center;

								if (collides_with_sphere(&disp2, radsum))
								{
									return collide_with_point(
										_transition->object_info,
										_transition->sphere_path,
										_transition->collision_info,
										_transition->sphere_path.global_sphere[1],
										disp2, radsum, 1);
								}
							}

							return TransitionState::OK_TS;
						}
					}
				}
			}
		}
	}
}

TransitionState CSphere::intersects_sphere(Position *pos, float scale, CTransition *transition, int is_creature)
{
	CSphere global_sphere;

	Vector v = center * scale;
	global_sphere.center = transition->sphere_path.check_pos.localtoglobal(*pos, v);
	global_sphere.radius = scale * radius;

	return global_sphere.intersects_sphere(transition, is_creature);
}

CCylSphere::CCylSphere()
{
	low_pt = Vector(0, 0, 0);
	height = 0;
	radius = 0;
}

ULONG CCylSphere::pack_size()
{
	return (low_pt.pack_size() + sizeof(float) * 2);
}

BOOL CCylSphere::UnPack(BYTE** ppData, ULONG iSize)
{
	UNPACK_OBJ(low_pt);
	UNPACK(float, radius);
	UNPACK(float, height);
	return TRUE;
}

TransitionState CCylSphere::step_sphere_down(OBJECTINFO &object, SPHEREPATH &path, COLLISIONINFO &collisions, CSphere &check_pos, Vector &disp, float radsum)
{
	if (!collides_with_sphere(&check_pos, &disp, radsum))
	{
		if (path.num_sphere <= 1)
			return TransitionState::OK_TS;

		Vector disp2 = path.global_sphere[1].center - low_pt;

		if (!collides_with_sphere(&path.global_sphere[1], &disp2, radsum))
			return TransitionState::OK_TS;
	}

	float step_down_interp = path.step_down_amt * path.walk_interp;
	if (fabs(step_down_interp) < F_EPSILON)
		return COLLIDED_TS;

	double v12 = height + check_pos.radius - disp.z;
	double v13 = (1.0 - v12 / step_down_interp) * path.walk_interp;

	if (v13 >= path.walk_interp || v13 < -0.1)
		return COLLIDED_TS;

	Vector contact_pt;
	contact_pt.x = check_pos.center.x;
	contact_pt.y = check_pos.center.y;
	contact_pt.z = check_pos.center.z + (v12 - check_pos.radius);

	Vector normal;
	normal = Vector(0, 0, 1.0f);

	Plane contact_plane(normal, contact_pt);

	collisions.set_contact_plane(&contact_plane, 1);

	collisions.contact_plane_cell_id = path.check_pos.objcell_id;
	path.walk_interp = v13;
	normal = Vector(0, 0, v12);
	path.add_offset_to_check_pos(&normal, check_pos.radius);

	return ADJUSTED_TS;
}

TransitionState CCylSphere::slide_sphere(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum, int sphere_num)
{
	Vector collision_normal;
	normal_of_collision(path, check_pos, disp, radsum, sphere_num, &collision_normal);

	if (collision_normal.normalize_check_small())
		return COLLIDED_TS;

	return check_pos->slide_sphere(
			path,
			collisions,
			&collision_normal,
			&path->global_curr_center[sphere_num]);
}

TransitionState CCylSphere::step_sphere_up(CTransition *transition, CSphere &check_pos, Vector &disp, float radsum)
{
	if (transition->object_info.step_up_height < (check_pos.radius + height - disp.z))
	{
		return slide_sphere(
			&transition->object_info,
			&transition->sphere_path,
			&transition->collision_info,
			&check_pos,
			&disp,
			radsum,
			0);
	}
	else
	{
		Vector collision_normal;
		normal_of_collision(&transition->sphere_path, &check_pos, &disp, radsum, 0, &collision_normal);

		if (collision_normal.normalize_check_small())
			return COLLIDED_TS;

		Vector v6 = transition->sphere_path.localspace_pos.localtoglobalvec(collision_normal);
		if (transition->step_up(&v6))
			return OK_TS;

		return transition->sphere_path.step_up_slide(&transition->object_info, &transition->collision_info);
	}
}

TransitionState CCylSphere::intersects_sphere(CTransition *_transition)
{
	Vector disp = _transition->sphere_path.global_sphere[0].center - low_pt;
	float radsum = (radius + _transition->sphere_path.global_sphere[0].radius) - F_EPSILON;

	int result = 1;
	if (_transition->sphere_path.obstruction_ethereal || _transition->sphere_path.insert_type == SPHEREPATH::PLACEMENT_INSERT)
	{
		if (collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
			return TransitionState::COLLIDED_TS;

		if (_transition->sphere_path.num_sphere >= 2)
		{
			Vector disp = _transition->sphere_path.global_sphere[1].center - low_pt;

			if (collides_with_sphere(&_transition->sphere_path.global_sphere[1], &disp, radsum))
				return TransitionState::COLLIDED_TS;
		}

		return TransitionState::OK_TS;
	}
	else
	{
		if (_transition->sphere_path.step_down)
		{
			return step_sphere_down(
				_transition->object_info,
				_transition->sphere_path,
				_transition->collision_info,
				_transition->sphere_path.global_sphere[0],
				disp,
				radsum);
		}
		else
		{
			if (_transition->sphere_path.check_walkable)
			{
				if (collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
					return TransitionState::COLLIDED_TS;

				if (_transition->sphere_path.num_sphere >= 2)
				{
					Vector disp2 = _transition->sphere_path.global_sphere[1].center - low_pt;

					if (collides_with_sphere(&_transition->sphere_path.global_sphere[1], &disp2, radsum))
						return TransitionState::COLLIDED_TS;
				}

				return TransitionState::OK_TS;
			}
			else
			{
				if (_transition->sphere_path.collide)
				{
					if (!collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
					{
						if (_transition->sphere_path.num_sphere >= 2)
						{
							Vector disp2 = _transition->sphere_path.global_sphere[1].center - low_pt;

							if (!collides_with_sphere(&_transition->sphere_path.global_sphere[1], &disp2, radsum))
								return TransitionState::OK_TS;
						}
					}

					Vector blockOffset = _transition->sphere_path.get_curr_pos_check_pos_block_offset();
					Vector someOffset = *_transition->sphere_path.global_curr_center - _transition->sphere_path.global_sphere->center;
					Vector movement = someOffset - blockOffset;
					radsum += F_EPSILON;

					float someValue1 = (movement.x*movement.x) + (movement.y*movement.y) + (movement.z*movement.z);
					float someValue2 = -((movement.x*disp.x) + (movement.y*disp.y) + (movement.z*disp.z));

					if (fabs(someValue1) < F_EPSILON)
						return TransitionState::COLLIDED_TS;

					float someValue3 = someValue2 + sqrt((someValue2*someValue2) - ((((disp.x*disp.x) + (disp.y*disp.y) + (disp.z*disp.z)) - (radsum*radsum))*someValue1));

					if (someValue3 > 1.0)
					{
						someValue3 = (someValue2 + someValue2) - someValue3;
					}

					float someFraction = someValue3 / someValue1;
					float timechecka = (1 - someFraction) * _transition->sphere_path.walk_interp;
					if (timechecka >= _transition->sphere_path.walk_interp || timechecka < -0.1)
						return TransitionState::COLLIDED_TS;

					movement *= someFraction;
					disp += movement;
					disp /= radsum;

					if (!_transition->sphere_path.is_walkable_allowable(disp.z))
						return TransitionState::OK_TS;

					Vector someVector = disp * _transition->sphere_path.global_sphere->radius;
					Vector disp2 = _transition->sphere_path.global_sphere->center - someVector;

					Plane contact_plane(disp, disp2);

					_transition->collision_info.set_contact_plane(&contact_plane, TRUE);
					_transition->collision_info.contact_plane_cell_id = _transition->sphere_path.check_pos.objcell_id;
					_transition->sphere_path.walk_interp = timechecka;

					_transition->sphere_path.add_offset_to_check_pos(&movement, _transition->sphere_path.global_sphere->radius);

					return TransitionState::ADJUSTED_TS;
				}
				else
				{
					if (_transition->object_info.state & (OBJECTINFO::CONTACT_OI | OBJECTINFO::ON_WALKABLE_OI))
					{
						if (collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
						{
							return step_sphere_up(
								_transition,
								_transition->sphere_path.global_sphere[0],
								disp, radsum);
						}

						if (_transition->sphere_path.num_sphere >= 2)
						{
							Vector disp2 = _transition->sphere_path.global_sphere[1].center - low_pt;

							if (collides_with_sphere(&_transition->sphere_path.global_sphere[1], &disp2, radsum))
							{
								return slide_sphere(
									&_transition->object_info,
									&_transition->sphere_path,
									&_transition->collision_info,
									&_transition->sphere_path.global_sphere[1],
									&disp2, radsum, 1);
							}
						}

						return TransitionState::OK_TS;
					}
					else if (_transition->object_info.state & (OBJECTINFO::PATH_CLIPPED_OI))
					{
						if (collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
						{
							return collide_with_point(
								&_transition->object_info,
								&_transition->sphere_path,
								&_transition->collision_info,
								&_transition->sphere_path.global_sphere[0],
								&disp, radsum, 0);
						}

						return TransitionState::OK_TS;
					}
					else
					{
						if (collides_with_sphere(&_transition->sphere_path.global_sphere[0], &disp, radsum))
						{
							return land_on_cylinder(
								&_transition->object_info,
								&_transition->sphere_path,
								&_transition->collision_info,
								&_transition->sphere_path.global_sphere[0],
								&disp, radsum);
						}
						else
						{
							if (_transition->sphere_path.num_sphere >= 2)
							{
								Vector disp2 = _transition->sphere_path.global_sphere[1].center - low_pt;

								if (collides_with_sphere(&_transition->sphere_path.global_sphere[1], &disp2, radsum))
								{
									return collide_with_point(
										&_transition->object_info,
										&_transition->sphere_path,
										&_transition->collision_info,
										&_transition->sphere_path.global_sphere[1],
										&disp2, radsum, 1);
								}
							}

							return TransitionState::OK_TS;
						}
					}
				}
			}
		}
	}
}

int CCylSphere::normal_of_collision(SPHEREPATH *path, CSphere *check_pos, Vector *disp, float radsum, int sphere_num, Vector *normal)
{
	Vector old_disp = path->global_curr_center[sphere_num] - low_pt;

	if ((radsum * radsum) < (old_disp.x*old_disp.x + old_disp.y*old_disp.y))
	{
		*normal = old_disp;
		normal->z = 0.0f;

		if ((check_pos->radius - F_EPSILON + (height * 0.5)) >= fabs(height * 0.5 - old_disp.z) || fabs(old_disp.z - disp->z) <= F_EPSILON)
		{
			return TRUE;
		}
		return FALSE;
	}

	if (disp->z - old_disp.z <= 0.0)
	{
		*normal = Vector(0, 0, 1);
		return TRUE;
	}

	*normal = Vector(0, 0, -1);
	return TRUE;
}

TransitionState CCylSphere::land_on_cylinder(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *disp, float radsum)
{
	TransitionState result;

	Vector collision_normal;
	normal_of_collision(path, check_pos, disp, radsum, 0, &collision_normal);

	if (collision_normal.normalize_check_small())
	{
		result = COLLIDED_TS;
	}
	else
	{
		path->set_collide(collision_normal);

		const float z_for_landing_2 = 0.0871557f;
		path->walkable_allowance = z_for_landing_2;
		result = ADJUSTED_TS;
	}
	return result;
}

TransitionState CCylSphere::intersects_sphere(Position *p, float scale, CTransition *transition)
{
	CCylSphere global_cylsphere;

	transition->sphere_path.cache_localspace_sphere(p, 1.0);
	global_cylsphere.radius = scale * radius;
	global_cylsphere.height = scale * height;

	Vector v = low_pt * scale;
	global_cylsphere.low_pt = transition->sphere_path.check_pos.localtoglobal(*p, v);

	return global_cylsphere.intersects_sphere(transition);
}

BOOL CCylSphere::collides_with_sphere(CSphere *check_pos, Vector *disp, const float radsum)
{
	return (((radsum * radsum) >= (disp->x * disp->x + disp->y * disp->y)) && (((check_pos->radius - F_EPSILON) + (height * 0.5)) >= fabs(height * 0.5 - disp->z)));
}

CSolid::CSolid(const Vector& Vec)
{
	m_Vec = Vec;
}

CSolid::CSolid()
{
	m_Vec = Vector(0, 0, 0);
}

ULONG CSolid::pack_size()
{
	return m_Vec.pack_size();
}

BOOL CSolid::UnPack(BYTE** ppData, ULONG iSize)
{
	return UNPACK_OBJ(m_Vec);
}

CPortalPoly::CPortalPoly()
{
	Destroy();
}

CPortalPoly::~CPortalPoly()
{
	Destroy();
}

void CPortalPoly::Destroy()
{
	portal = NULL;
	portal_index = -1;
}

BOOL CPortalPoly::UnPack(BYTE** ppData, ULONG iSize)
{
	short Index;
	short What;
	UNPACK(short, Index);
	UNPACK(short, What);

	portal = &BSPNODE::pack_poly[Index];
	portal_index = What;

	return TRUE;
}

BSPNODE::BSPNODE()
{
	type = '####';
	num_polys = 0;
	in_polys = NULL;
	pos_node = NULL;
	neg_node = NULL;
}

BSPNODE::~BSPNODE()
{
	if (pos_node)
	{
		delete pos_node;
		pos_node = NULL;
	}

	if (in_polys)
	{
		delete[] in_polys;
		in_polys = NULL;
	}

	num_polys = 0;

	if (neg_node)
	{
		delete neg_node;
		neg_node = NULL;
	}

	type = '####';
}

BOOL BSPNODE::UnPack(BYTE** ppData, ULONG iSize)
{
	BYTE *pOld = *ppData;

	UNPACK_OBJ(splitting_plane);

	switch (type)
	{
	case 'BPnn':
	case 'BPIn':
		if (!BSPNODE::UnPackChild(&pos_node, ppData, iSize))
			return FALSE;

		break;
	case 'BpIN':
	case 'BpnN':
		if (!BSPNODE::UnPackChild(&neg_node, ppData, iSize))
			return FALSE;

		break;
	case 'BPIN':
	case 'BPnN':
		if (!BSPNODE::UnPackChild(&pos_node, ppData, iSize))
			return FALSE;
		if (!BSPNODE::UnPackChild(&neg_node, ppData, iSize))
			return FALSE;

		break;
	}

	if (pack_tree_type) /* 5e4818 in august? */
	{
		if (pack_tree_type == 1)
			UNPACK_OBJ(sphere);

		return TRUE;
	}

	UNPACK_OBJ(sphere);
	UNPACK(uint32_t, num_polys);

	if (num_polys)
	{
		in_polys = new CPolygon*[num_polys];

		for (uint32_t i = 0; i < num_polys; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			in_polys[i] = pack_poly + Index;
		}
	}

#ifdef PRE_TOD
	// CFTOD:
	PACK_ALIGN();
#endif

	return TRUE;
}

BOOL BSPNODE::UnPackChild(BSPNODE** pOut, BYTE** ppData, ULONG iSize)
{
	uint32_t NodeType;
	UNPACK(uint32_t, NodeType);

	if (NodeType == 'PORT')
	{
		BSPPORTAL* pPortal = new BSPPORTAL();

		*pOut = pPortal;
		pPortal->type = 'PORT';

		return pPortal->UnPackPortal(ppData, iSize);
	}
	else if (NodeType == 'LEAF')
	{
		BSPLEAF* pLeaf = new BSPLEAF();

		*pOut = pLeaf;
		pLeaf->type = 'LEAF';

		return pLeaf->UnPackLeaf(ppData, iSize);
	}
	else
	{
		BSPNODE* pNode = new BSPNODE();

		*pOut = pNode;
		pNode->type = NodeType;

		return pNode->UnPack(ppData, iSize);
	}
}

#if 0
void BSPNODE::draw_no_check()
{
#define BSP_GETFRAMEFUNC Render::GetFrameCurrent

	// Step through the binary tree, beginning with me.
	BSPNODE *pNode = this;

	// Side represents the visual plane this polygon lies on.
	int side;

next_node:
	switch (pNode->type)
	{
	case 'BPIn':
		side = pNode->splitting_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);
		case 2:
			pNode = pNode->pos_node;
			goto next_node;
		case 1:
			pNode->pos_node->draw_no_check();

			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);
		}
		break;
	case 'BpIN':
		side = pNode->splitting_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 1:
			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);
		case 2:
			pNode = pNode->neg_node;
			goto next_node;
		case 0:
			pNode->neg_node->draw_no_check();

			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);
		default:
			// return
			;
		}
		break;
	case 'BPnn':
		pNode = pNode->pos_node;
		goto next_node;
	case 'BpnN':
		pNode = pNode->neg_node;
		goto next_node;
	case 'BPOL':
		Render::polyDraw(pNode->in_polys[0]);
		break; // return
	case 'BPIN':
	case 'PORT':
		side = pNode->splitting_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			pNode->neg_node->draw_no_check();

			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);

			pNode = pNode->pos_node;
			goto next_node;
		case 1:
			pNode->pos_node->draw_no_check();

			for (UINT i = 0; i < pNode->num_polys; i++)
				Render::polyDraw(pNode->in_polys[i]);

			pNode = pNode->neg_node;
			goto next_node;
		case 2:
			pNode->pos_node->draw_no_check();

			pNode = pNode->neg_node;
			goto next_node;
		default:
			// return
			;
		}
		break;
	case 'BPnN':
		side = pNode->splitting_plane.which_side(BSP_GETFRAMEFUNC()->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			pNode->neg_node->draw_no_check();

			pNode = pNode->pos_node;
			goto next_node;
		case 1:
			pNode->pos_node->draw_no_check();

			pNode = pNode->neg_node;
			goto next_node;
		case 2:
			pNode->pos_node->draw_no_check();

			pNode = pNode->neg_node;
			goto next_node;
		default:
			// return
			;
		}
		break;
	case 'BpIn':
		for (UINT i = 0; i < pNode->num_polys; i++)
			Render::polyDraw(pNode->in_polys[i]);
		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}
#endif

#if PHATSDK_RENDER_AVAILABLE
void BSPNODE::draw_no_check()
{
	// Side represents the visual plane that this polygon lies on.
	int side;

	switch (type)
	{
	case 'BPIn':
		side = splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);
		case 2:
			pos_node->draw_no_check();
			break;
		case 1:
			pos_node->draw_no_check();

			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);
		}
		break;
	case 'BpIN':
		side = splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 1:
			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);
		case 2:
			neg_node->draw_no_check();
			break;
		case 0:
			neg_node->draw_no_check();

			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);
		}
		break;
	case 'BPnn':
		pos_node->draw_no_check();
		break;
	case 'BpnN':
		neg_node->draw_no_check();
		break;
	case 'BPOL':
		Render::polyDraw(in_polys[0]);
		break;
	case 'BPIN':
	case 'PORT':
		side = splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			neg_node->draw_no_check();

			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);

			pos_node->draw_no_check();
			break;
		case 1:
			pos_node->draw_no_check();

			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);

			neg_node->draw_no_check();
			break;
		case 2:
			pos_node->draw_no_check();
			neg_node->draw_no_check();
			break;
		}
		break;
	case 'BPnN':
		side = splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON);

		switch (side)
		{
		case 0:
			neg_node->draw_no_check();
			pos_node->draw_no_check();
			break;
		case 1:
			pos_node->draw_no_check();
			neg_node->draw_no_check();
			break;
		case 2:
			pos_node->draw_no_check();
			neg_node->draw_no_check();
			break;
		}
		break;
	case 'BpIn':
		for (UINT i = 0; i < num_polys; i++)
			Render::polyDraw(in_polys[i]);

		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}

void BSPNODE::draw_check(uint32_t unknown)
{
	switch (type)
	{
	case 'BPIn':

		switch (Render::checkplanes(sphere, &unknown))
		{
		case 1:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);

				pos_node->draw_check(unknown);
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_check(unknown);

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);
				break;
			case Sidedness::IN_PLANE:
				pos_node->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);

				pos_node->draw_no_check();
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_no_check();

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);
				break;
			}
			break;
		}

		break;

	case 'BpIN':

		switch (Render::checkplanes(sphere, &unknown))
		{
		case 1:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_check(unknown);

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);
				break;
			case Sidedness::NEGATIVE:
				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);

				neg_node->draw_check(unknown);
				break;
			case Sidedness::IN_PLANE:
				neg_node->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_no_check();

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);
				break;
			case Sidedness::NEGATIVE:
				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);

				neg_node->draw_no_check();
				break;
			case Sidedness::IN_PLANE:
				neg_node->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BPnn':
		switch (Render::checkplanes(sphere, &unknown))
		{
		case Sidedness::NEGATIVE:
			pos_node->draw_check(unknown);
			break;
		case Sidedness::IN_PLANE:
			pos_node->draw_no_check();
			break;
		}

		break;
	case 'BpnN':
		switch (Render::checkplanes(sphere, &unknown))
		{
		case Sidedness::NEGATIVE:
			neg_node->draw_check(unknown);
			break;
		case Sidedness::IN_PLANE:
			neg_node->draw_no_check();
			break;
		}

		break;
	case 'BPOL':
		switch (Render::checkplanes(sphere, &unknown))
		{
		case Sidedness::NEGATIVE:
			Render::polyDrawClip(in_polys[0], unknown);
			break;
		case Sidedness::IN_PLANE:
			Render::polyDraw(in_polys[0]);
			break;
		}

		break;
	case 'BPIN':
	case 'PORT':

		switch (Render::checkplanes(sphere, &unknown))
		{
		case 1:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_check(unknown);

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);

				pos_node->draw_check(unknown);
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_check(unknown);

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDrawClip(in_polys[i], unknown);

				neg_node->draw_check(unknown);
				break;
			case Sidedness::IN_PLANE:
				pos_node->draw_check(unknown);
				neg_node->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_no_check();

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);

				pos_node->draw_no_check();
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_no_check();

				for (UINT i = 0; i < num_polys; i++)
					Render::polyDraw(in_polys[i]);

				neg_node->draw_no_check();
				break;
			case Sidedness::IN_PLANE:
				pos_node->draw_no_check();
				neg_node->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BPnN':

		switch (Render::checkplanes(sphere, &unknown))
		{
		case 1:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_check(unknown);
				pos_node->draw_check(unknown);
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_check(unknown);
				neg_node->draw_check(unknown);
				break;
			case Sidedness::IN_PLANE:
				pos_node->draw_check(unknown);
				neg_node->draw_check(unknown);
				break;
			}
			break;
		case 2:

			switch (splitting_plane.which_side(Render::FrameCurrent->m_LocalView.m_origin, F_EPSILON))
			{
			case Sidedness::POSITIVE:
				neg_node->draw_no_check();
				pos_node->draw_no_check();
				break;
			case Sidedness::NEGATIVE:
				pos_node->draw_no_check();
				neg_node->draw_no_check();
				break;
			case Sidedness::IN_PLANE:
				pos_node->draw_no_check();
				neg_node->draw_no_check();
				break;
			}
			break;
		}

		break;
	case 'BpIn':

		switch (Render::checkplanes(sphere, &unknown))
		{
		case 1:
			for (UINT i = 0; i < num_polys; i++)
				Render::polyDrawClip(in_polys[i], unknown);
			break;
		case 2:
			for (UINT i = 0; i < num_polys; i++)
				Render::polyDraw(in_polys[i]);
			break;
		}

		break;
	default:
		// OutputDebug("BSP ID: %X\n", pNode->m_dwID);
		// __asm int 3;
		break;
	}
}
#endif

BOOL BSPNODE::point_intersects_solid(Vector *point)
{
	if (splitting_plane.dot_product(*point) >= 0)
		return pos_node->point_intersects_solid(point);
	else
		return neg_node->point_intersects_solid(point);
}

int BSPNODE::sphere_intersects_solid_poly(CSphere *check_pos, float radius, BOOL *center_solid, CPolygon **hit_poly, BOOL check_center)
{
	if (!sphere.intersects(check_pos))
		return 0;

	float fDist = splitting_plane.dot_product(check_pos->center);
	float fReach = radius - F_EPSILON;

	if (fDist >= (fReach))
		return pos_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, check_center);
	if (fDist <= -(fReach))
		return neg_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, check_center);

	if (fDist < 0.0)
	{
		neg_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, check_center);

		if (*hit_poly)
			return *center_solid;

		return pos_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, NULL);
	}
	else
	{
		pos_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, check_center);

		if (*hit_poly)
			return *center_solid;

		return neg_node->sphere_intersects_solid_poly(check_pos, radius, center_solid, hit_poly, NULL);
	}
}

int BSPNODE::sphere_intersects_poly(CSphere *check_pos, Vector *movement, CPolygon **polygon, Vector *contact_pt)
{
	// !! modifies contact_pt

	if (!sphere.intersects(check_pos))
		return false;

	float fDist = splitting_plane.dot_product(check_pos->center);
	float fReach = check_pos->radius - F_EPSILON;

	if (fDist >= fReach)
		return pos_node->sphere_intersects_poly(check_pos, movement, polygon, contact_pt);
	if (fDist <= -fReach)
		return neg_node->sphere_intersects_poly(check_pos, movement, polygon, contact_pt);

	if (pos_node->sphere_intersects_poly(check_pos, movement, polygon, contact_pt))
		return true;
	if (neg_node->sphere_intersects_poly(check_pos, movement, polygon, contact_pt))
		return true;

	return false;
}

int BSPNODE::hits_walkable(SPHEREPATH *path, CSphere *valid_pos, Vector *up)
{
	if (!sphere.intersects(valid_pos))
		return false;

	float fDist = splitting_plane.dot_product(valid_pos->center);
	float fReach = valid_pos->radius - F_EPSILON;

	if (fDist >= fReach)
		return pos_node->hits_walkable(path, valid_pos, up);
	if (fDist <= -fReach)
		return neg_node->hits_walkable(path, valid_pos, up);

	if (pos_node->hits_walkable(path, valid_pos, up))
		return true;
	if (neg_node->hits_walkable(path, valid_pos, up))
		return true;

	return false;
}

int BSPNODE::sphere_intersects_solid(CSphere *check_pos, int center_check)
{
	// Is this sphere even within our bounds?
	if (!sphere.intersects(check_pos))
		return 0;

	float fDist = splitting_plane.dot_product(check_pos->center);
	float fReach = check_pos->radius - F_EPSILON;

	// Is this sphere inside our plane?
	if ((fDist) >= (fReach))
		return pos_node->sphere_intersects_solid(check_pos, center_check);

	if ((fDist) <= (-fReach))
		return neg_node->sphere_intersects_solid(check_pos, center_check);

	if (fDist < 0.0)
	{
		if (pos_node->sphere_intersects_solid(check_pos, FALSE))
			return 1;

		return neg_node->sphere_intersects_solid(check_pos, center_check);
	}
	else
	{
		if (pos_node->sphere_intersects_solid(check_pos, center_check))
			return 1;

		return neg_node->sphere_intersects_solid(check_pos, FALSE);
	}
}

void BSPNODE::find_walkable(SPHEREPATH *path, CSphere *valid_pos, CPolygon **polygon, Vector *movement, Vector *up, BOOL *changed)
{
	if (!sphere.intersects(valid_pos))
		return;

	float fDist = splitting_plane.dot_product(valid_pos->center);
	float fReach = valid_pos->radius - F_EPSILON;

	if (fDist >= fReach) {
		pos_node->find_walkable(path, valid_pos, polygon, movement, up, changed);
		return;
	}
	if (fDist <= -fReach) {
		neg_node->find_walkable(path, valid_pos, polygon, movement, up, changed);
		return;
	}

	pos_node->find_walkable(path, valid_pos, polygon, movement, up, changed);
	neg_node->find_walkable(path, valid_pos, polygon, movement, up, changed);
	return;
}

BoundingType BSPNODE::sphere_intersects_cell_bsp(CSphere *curr_sphere)
{
	/*
	float rad_ = curr_sphere->radius + 0.01;

	BSPNODE *curr_node = this;
	double dp = curr_node->splitting_plane.dot_product(curr_sphere->center);

	while (dp > -rad_)
	{
		curr_node = curr_node->pos_node;
		if (dp < rad_)
		{
			if (curr_node)
				return (curr_node->sphere_intersects_cell_bsp(curr_sphere) != 0) ? BoundingType::PARTIALLY_INSIDE : BoundingType::OUTSIDE;
			
			return BoundingType::PARTIALLY_INSIDE;
		}

		if (!curr_node)
			return BoundingType::ENTIRELY_INSIDE;

		dp = curr_node->splitting_plane.dot_product(curr_sphere->center);
	}

	return BoundingType::OUTSIDE;
	*/

	double planeSphereDotProduct = splitting_plane.dot_product(curr_sphere->center);
	float checkRadius = curr_sphere->radius + 0.01;
	assert(checkRadius >= F_EPSILON);

	if (planeSphereDotProduct <= -checkRadius)
		return BoundingType::OUTSIDE;

	if (planeSphereDotProduct >= checkRadius)
	{
		if (pos_node)
			return pos_node->sphere_intersects_cell_bsp(curr_sphere);

		return BoundingType::ENTIRELY_INSIDE;
	}

	if (pos_node)
		return (BoundingType)((int)pos_node->sphere_intersects_cell_bsp(curr_sphere) != 0);

	return BoundingType::PARTIALLY_INSIDE;
}

int BSPNODE::box_intersects_cell_bsp(BBox *box)
{
	BSPNODE *node = this;

	do
	{
		Vector v;

		v = box->m_Min;
		if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
		{
			v = box->m_Max;
			if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
			{
				v.x = box->m_Min.x;
				v.y = box->m_Min.y;
				v.z = box->m_Max.z;
				if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
				{
					v.x = box->m_Min.x;
					v.y = box->m_Max.y;
					v.z = box->m_Min.z;
					if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
					{
						v.x = box->m_Max.x;
						v.y = box->m_Min.y;
						v.z = box->m_Min.z;
						if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
						{
							v.x = box->m_Max.x;
							v.y = box->m_Min.y;
							v.z = box->m_Max.z;
							if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
							{
								v.x = box->m_Min.x;
								v.y = box->m_Max.y;
								v.z = box->m_Max.z;
								if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
								{
									v.x = box->m_Max.x;
									v.y = box->m_Max.y;
									v.z = box->m_Min.z;
									if (node->splitting_plane.which_side(v, F_EPSILON) == 1)
									{
										return 0;
									}
								}
							}
						}
					}
				}
			}
		}

		node = node->pos_node;
	} while (node);

	return 1;
}

BSPLEAF::BSPLEAF()
{
	leaf_index = (uint32_t)-1;
	solid = 0;
}

BSPLEAF::~BSPLEAF()
{
}

BOOL BSPLEAF::UnPackLeaf(BYTE** ppData, ULONG iSize)
{
	UNPACK(uint32_t, leaf_index);

	if (pack_tree_type != 1)
		return TRUE;

	UNPACK(uint32_t, solid);
	UNPACK_OBJ(sphere);
	UNPACK(uint32_t, num_polys);

	if (num_polys)
	{
		in_polys = new CPolygon*[num_polys];

		for (uint32_t i = 0; i < num_polys; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			in_polys[i] = &pack_poly[Index];
		}
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#else
	// CFTOD PACK_ALIGN();
	// CFTOD Another word here?
#endif

	return TRUE;
}

BOOL BSPLEAF::point_intersects_solid(Vector *point)
{
	return num_polys != 0;
}

int BSPLEAF::sphere_intersects_solid_poly(CSphere *check_pos, float radius, BOOL *center_solid, CPolygon **hit_poly, BOOL check_center)
{
	if (!num_polys)
		return NULL;

	if (check_center && solid)
		*center_solid = TRUE;

	if (!sphere.intersects(check_pos))
		return *center_solid;

	for (UINT i = 0; i < num_polys; i++)
	{
		if (in_polys[i]->hits_sphere(check_pos))
		{
			*hit_poly = in_polys[i];
			return true;
		}
	}

	return *center_solid;
}

int BSPLEAF::hits_walkable(SPHEREPATH *path, CSphere *valid_pos, Vector *up)
{
	if (!num_polys)
		return false;
	if (!sphere.intersects(valid_pos))
		return false;

	for (UINT i = 0; i < num_polys; i++)
	{
		if (in_polys[i]->walkable_hits_sphere(path, valid_pos, up))
		{
			if (in_polys[i]->check_small_walkable(valid_pos, up))
				return true;
		}
	}

	return false;
}

int BSPLEAF::sphere_intersects_poly(CSphere *check_pos, Vector *movement, CPolygon **polygon, Vector *contact_pt)
{
	// !! modifies contact_pt

	if (!num_polys)
		return false;
	if (!sphere.intersects(check_pos))
		return false;

	for (UINT i = 0; i < num_polys; i++)
	{
		if (in_polys[i]->pos_hits_sphere(check_pos, movement, contact_pt, polygon))
			return true;
	}

	return false;
}

int BSPLEAF::sphere_intersects_solid(CSphere *check_pos, BOOL center_check)
{
	if (!num_polys)
		return false;

	if (solid && center_check)
		return true;

	if (!sphere.intersects(check_pos))
		return false;

	for (UINT i = 0; i < num_polys; i++)
	{
		if (in_polys[i]->hits_sphere(check_pos))
			return true;
	}

	return false;
}

void BSPLEAF::find_walkable(SPHEREPATH *path, CSphere *valid_pos, CPolygon **polygon, Vector *movement, Vector *up, BOOL *changed)
{
	if (!num_polys)
		return;
	if (!sphere.intersects(valid_pos))
		return;

	for (UINT i = 0; i < num_polys; i++)
	{
		if (in_polys[i]->walkable_hits_sphere(path, valid_pos, up))
		{
			if (in_polys[i]->adjust_sphere_to_plane(path, valid_pos, movement))
			{
				*changed = TRUE;
				*polygon = in_polys[i];
			}
		}
	}

	return;
}

// End of BSPLEAF collision routines

BSPPORTAL::BSPPORTAL()
{
	num_portals = 0;
	in_portals = NULL;
}

BSPPORTAL::~BSPPORTAL()
{
	if (in_portals)
	{
		for (uint32_t i = 0; i < num_portals; i++)
		{
			if (in_portals[i])
				delete in_portals[i];
		}

		delete[] in_portals;
		in_portals = NULL;
	}

	num_portals = 0;
}

BOOL BSPPORTAL::UnPackPortal(BYTE** ppData, ULONG iSize)
{
	BYTE *pOld = *ppData;

	if (!UNPACK_OBJ(splitting_plane))
		return FALSE;

	if (!BSPNODE::UnPackChild(&pos_node, ppData, iSize) ||
		!BSPNODE::UnPackChild(&neg_node, ppData, iSize))
	{
		*ppData = pOld;
		return FALSE;
	}

	if (pack_tree_type)
		return TRUE;

	UNPACK_OBJ(sphere);

	UNPACK(uint32_t, num_polys);
	UNPACK(uint32_t, num_portals);

	if (num_polys)
	{
		in_polys = new CPolygon*[num_polys];

		for (uint32_t i = 0; i < num_polys; i++)
		{
			WORD Index;
			UNPACK(WORD, Index);

			in_polys[i] = &pack_poly[Index];
		}
	}

	if (num_portals)
	{
		in_portals = new CPortalPoly *[num_portals];

		for (uint32_t i = 0; i < num_portals; i++)
		{
			in_portals[i] = new CPortalPoly();
			in_portals[i]->UnPack(ppData, iSize);
		}
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#else
	// CFTOD PACK_ALIGN();
	// CFTOD Another word here?
#endif

	return TRUE;
}

BSPTREE::BSPTREE()
{
	root_node = NULL;
}

BSPTREE::~BSPTREE()
{
	Destroy();
}

void BSPTREE::Destroy()
{
	if (root_node)
	{
		delete root_node;
		root_node = NULL;
	}
}

BOOL BSPTREE::UnPack(BYTE** ppData, ULONG iSize)
{
	Destroy();

	return BSPNODE::UnPackChild(&root_node, ppData, iSize);
}

CSphere *BSPTREE::GetSphere()
{
	return &root_node->sphere;
}

#if PHATSDK_RENDER_AVAILABLE
void BSPTREE::draw_no_check()
{
	root_node->draw_no_check();

	Render::polyListFinishInternal();
	Render::polyListClear();
}
#endif

BOOL BSPTREE::point_inside_cell_bsp(const Vector& point)
{
	return root_node->point_inside_cell_bsp(point);
}

BOOL BSPNODE::point_inside_cell_bsp(const Vector& point)
{
	int side = splitting_plane.which_side(point, F_EPSILON);

	switch (side)
	{
	case 0: // FRONT
	case 2: // CLOSE

		if (!pos_node)
			return TRUE;

		return pos_node->point_inside_cell_bsp(point);
	case 1: // BEHIND
	default:
		return FALSE;
	}
}

TransitionState BSPTREE::find_collisions(CTransition *transition, float scale)
{
	Vector *localCurrCenter = transition->sphere_path.localspace_curr_center;
	SPHEREPATH *transitSpherePath = &transition->sphere_path;
	CSphere *localSphere = transition->sphere_path.localspace_sphere;
	COLLISIONINFO *collisions = &transition->collision_info;
	CPolygon *hit_poly = NULL;

	Vector movement = transition->sphere_path.localspace_sphere->center - *transition->sphere_path.localspace_curr_center;

	if ((transition->sphere_path.insert_type == SPHEREPATH::PLACEMENT_INSERT) || transition->sphere_path.obstruction_ethereal)
	{
		BOOL check_center = 1;
		if (transition->sphere_path.bldg_check)
		{
			check_center = transition->sphere_path.hits_interior_cell == 0;
		}

		if (root_node->sphere_intersects_solid(transition->sphere_path.localspace_sphere, check_center)
			|| transition->sphere_path.num_sphere > 1 && root_node->sphere_intersects_solid(&transition->sphere_path.localspace_sphere[1], check_center))
		{
			return COLLIDED_TS;
		}
	}
	else
	{
		if (transition->sphere_path.check_walkable)
			return check_walkable(&transition->sphere_path, transition->sphere_path.localspace_sphere, scale);

		if (transition->sphere_path.step_down)
			return step_sphere_down(&transition->sphere_path, &transition->collision_info, transition->sphere_path.localspace_sphere, scale);

		if (transition->sphere_path.collide)
		{
			CSphere valid_pos = *localSphere;

			CPolygon *pWalkablePoly = NULL;
			int changed = 0;

			root_node->find_walkable(transitSpherePath, &valid_pos, (CPolygon **)&pWalkablePoly, &movement, &transitSpherePath->localspace_z, &changed);
			if (changed)
			{
				Vector collision_normal = valid_pos.center - localSphere->center;
				collision_normal = transitSpherePath->localspace_pos.localtoglobalvec(collision_normal) * scale;
				transitSpherePath->add_offset_to_check_pos(&collision_normal);

				Plane trans = Plane::localtoglobal(transitSpherePath->check_pos, transitSpherePath->localspace_pos, pWalkablePoly->plane);
				trans.m_dist *= scale;

				collisions->set_contact_plane(&trans, 0);
				collisions->contact_plane_cell_id = transitSpherePath->check_pos.objcell_id;
				transitSpherePath->set_walkable(&valid_pos, pWalkablePoly, &transitSpherePath->localspace_z, &transitSpherePath->localspace_pos, scale);
				return TransitionState::ADJUSTED_TS;
			}
		}
		else
		{
			Vector contact_pt;

			if (transition->object_info.state & OBJECTINFO::CONTACT_OI)
			{
				if (root_node->sphere_intersects_poly(
					localSphere,
					&movement,
					&hit_poly,
					&contact_pt))
				{
					return step_sphere_up(transition, &hit_poly->plane.m_normal);
				}

				CPolygon *hitResult = NULL;
				if (transitSpherePath->num_sphere > 1)
				{
					if (root_node->sphere_intersects_poly(
						&transitSpherePath->localspace_sphere[1],
						&movement,
						&hitResult,
						&contact_pt))
					{
						return slide_sphere(transitSpherePath, collisions, &hitResult->plane.m_normal);
					}
					if (hitResult)
					{
						Vector v = transitSpherePath->localspace_pos.localtoglobalvec(hitResult->plane.m_normal);
						transitSpherePath->set_neg_poly_hit(0, &v);
						return OK_TS;
					}
					if (hit_poly)
					{
						Vector v = transitSpherePath->localspace_pos.localtoglobalvec(hit_poly->plane.m_normal);
						transitSpherePath->set_neg_poly_hit(1, &v);
						return OK_TS;
					}
				}
			}
			else
			{
				if (!(transition->object_info.state & OBJECTINFO::PATH_CLIPPED_OI))
				{
					if (root_node->sphere_intersects_poly(
						localSphere,
						&movement,
						&hit_poly,
						&contact_pt)
						|| hit_poly)
					{
						Vector collision_normal = transitSpherePath->localspace_pos.localtoglobalvec(hit_poly->plane.m_normal);
						transitSpherePath->set_collide(collision_normal);
						transitSpherePath->walkable_allowance = z_for_landing_0;
						return TransitionState::ADJUSTED_TS;
					}
					if (transitSpherePath->num_sphere > 1
						&& (root_node->sphere_intersects_poly(
							&transitSpherePath->localspace_sphere[1],
							&movement,
							&hit_poly,
							&contact_pt)
							|| hit_poly))
					{
						Vector v = transitSpherePath->localspace_pos.localtoglobalvec(hit_poly->plane.m_normal);
						collisions->set_collision_normal(v);
						return TransitionState::COLLIDED_TS;
					}
				}
				else if (root_node->sphere_intersects_poly(
					localSphere,
					&movement,
					&hit_poly,
					&contact_pt)
					|| hit_poly)
				{
					return collide_with_pt(
						&transition->object_info,
						transitSpherePath,
						collisions,
						localSphere,
						localCurrCenter,
						hit_poly,
						&contact_pt,
						scale);
				}
			}
		}
	}

	return OK_TS;
}

TransitionState BSPTREE::placement_insert(CTransition *transition)
{
	CSphere valid_pos = transition->sphere_path.localspace_sphere[0];
	CSphere valid_pos2;

	float localSphereRadius = transition->sphere_path.localspace_sphere[0].radius;  // transitiona

	if (transition->sphere_path.num_sphere > 1)
	{
		valid_pos2 = transition->sphere_path.localspace_sphere[1];
	}

	CPolygon *hit_poly = NULL;
	BOOL someBool = TRUE;
	if (transition->sphere_path.bldg_check)
		someBool = (transition->sphere_path.hits_interior_cell == 0);

	for (uint32_t i = 0; i < 20; i++)
	{
		/*
		BOOL center_solid = FALSE;
		if (root_node->sphere_intersects_solid_poly(&valid_pos, localSphereRadius, &center_solid, &hit_poly, someBool))
		{
			if (hit_poly != NULL)
			{
				hit_poly->adjust_to_placement_poly(&valid_pos, &valid_pos2, localSphereRadius, center_solid, someBool);
				continue;
			}
		}
		else
		{
			if (transition->sphere_path.num_sphere >= 2)
			{
				if (root_node->sphere_intersects_solid_poly(&valid_pos2, localSphereRadius, &center_solid, &hit_poly, someBool))
				{
					if (hit_poly != NULL)
					{
						hit_poly->adjust_to_placement_poly(&valid_pos, &valid_pos2, localSphereRadius, center_solid, someBool);
						continue;
					}
				}
				else
				{
					if (i != 0)
					{
						Vector offset = valid_pos.center - transition->sphere_path.localspace_sphere->center;
						Vector result = transition->sphere_path.localspace_pos.localtoglobalvec(offset);
						offset = result;
						transition->sphere_path.add_offset_to_check_pos(&offset);
						return ADJUSTED_TS;
					}

					return OK_TS;
				}
			}
			else
			{
				if (i != 0)
				{
					Vector offset = valid_pos.center - transition->sphere_path.localspace_sphere->center;
					Vector result = transition->sphere_path.localspace_pos.localtoglobalvec(offset);
					offset = result;
					transition->sphere_path.add_offset_to_check_pos(&offset);
					return ADJUSTED_TS;
				}

				return OK_TS;
			}
		}*/

		BOOL center_solid = FALSE;
		if (root_node->sphere_intersects_solid_poly(&valid_pos, localSphereRadius, &center_solid, &hit_poly, someBool))
		{
			if (hit_poly != NULL)
			{
				hit_poly->adjust_to_placement_poly(&valid_pos, &valid_pos2, localSphereRadius, center_solid, someBool);
				continue;
			}
		}
		else
		{
			if (transition->sphere_path.num_sphere >= 2)
			{
				if (root_node->sphere_intersects_solid_poly(&valid_pos2, localSphereRadius, &center_solid, &hit_poly, someBool))
				{
					if (hit_poly != NULL)
					{
						hit_poly->adjust_to_placement_poly(&valid_pos2, &valid_pos, localSphereRadius, center_solid, someBool);
						continue;
					}
				}
				else
				{
					if (i != 0)
					{
						Vector offset = valid_pos.center - transition->sphere_path.localspace_sphere->center;
						Vector result = transition->sphere_path.localspace_pos.localtoglobalvec(offset);
						offset = result;
						transition->sphere_path.add_offset_to_check_pos(&offset);
						return ADJUSTED_TS;
					}

					return OK_TS;
				}
			}
			else
			{
				if (i != 0)
				{
					Vector offset = valid_pos.center - transition->sphere_path.localspace_sphere->center;
					Vector result = transition->sphere_path.localspace_pos.localtoglobalvec(offset);
					offset = result;
					transition->sphere_path.add_offset_to_check_pos(&offset);
					return ADJUSTED_TS;
				}

				return OK_TS;
			}
		}

		localSphereRadius *= 2;
	}

	return COLLIDED_TS;
}

int BSPTREE::adjust_to_plane(CSphere *check_pos, Vector curr_pos, CPolygon *hit_poly, Vector *contact_pt)
{
	// !! check_pos gets modified

	double ltime = 0.0;
	double utime = 1.0;
	int v6 = 0;
	Vector movement = check_pos->center - curr_pos;

	uint32_t i;
	for (i = 0; i < 15; i++)
	{
		double time_touch = hit_poly->adjust_sphere_to_poly(check_pos, &curr_pos, &movement);
		if (time_touch == 1.0)
		{
			check_pos->center = curr_pos + (movement * time_touch); // !! check_pos gets modified

			if (!root_node->sphere_intersects_poly(
				check_pos,
				&movement,
				&hit_poly,
				contact_pt))
			{
				ltime = time_touch;
				break;
			}

			utime = time_touch;
		}
	}

	if (i < 15)
	{
		for (i = 0; i < 15; i++)
		{
			double avgTime = (ltime + utime) * 0.5;

			check_pos->center = curr_pos + (movement * avgTime); // !! check_pos gets modified

			if (root_node->sphere_intersects_poly(
				check_pos,
				&movement,
				&hit_poly,
				contact_pt))
			{
				utime = (ltime + utime) * 0.5;
			}
			else
			{
				ltime = (ltime + utime) * 0.5;
			}

			if ((utime - ltime) < 0.02)
				break;
		}

		check_pos->center = curr_pos + (movement * ltime);
		return 1;
	}

	return 0;
}

TransitionState BSPTREE::collide_with_pt(OBJECTINFO *object, SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, Vector *curr_pos, CPolygon *hit_poly, Vector *contact_pt, float scale)
{
	if (!(object->state & 0x40))
	{
		Vector offset = path->localspace_pos.localtoglobalvec(hit_poly->plane.m_normal);
		collisions->set_collision_normal(offset);
		return COLLIDED_TS;
	}

	CSphere valid_pos = *check_pos;

	// v is v12
	Vector v = *curr_pos;
	if (!adjust_to_plane(&valid_pos, v, hit_poly, contact_pt))
		return COLLIDED_TS;

	Vector result = path->localspace_pos.localtoglobalvec(hit_poly->plane.m_normal);
	collisions->set_collision_normal(result);

	Vector offset = valid_pos.center - check_pos->center;
	result = path->localspace_pos.localtoglobalvec(offset);

	offset = result * scale;
	path->add_offset_to_check_pos(&offset);

	return ADJUSTED_TS;
}

TransitionState BSPTREE::check_walkable(SPHEREPATH *path, CSphere *check_pos, float scale)
{
	CSphere valid_pos = *check_pos;
	return (TransitionState)((root_node->hits_walkable(path, &valid_pos, &path->localspace_z) != 0) + 1);
}

TransitionState BSPTREE::slide_sphere(SPHEREPATH *path, COLLISIONINFO *collisions, Vector *collision_normal)
{
	Vector v = path->localspace_pos.localtoglobalvec(*collision_normal);

	return path->global_sphere->slide_sphere(path, collisions, &v, path->global_curr_center);
}

/*
int BSPTREE::box_intersects_cell_bsp(BBox *box)
{
	return root_node->box_intersects_cell_bsp(box);
}
*/

TransitionState BSPTREE::step_sphere_down(SPHEREPATH *path, COLLISIONINFO *collisions, CSphere *check_pos, float scale)
{
	float step_down_amount_ = -(path->step_down_amt * path->walk_interp); // v8

	Vector trans = (path->localspace_z * step_down_amount_) * (1.0 / scale);
	CSphere valid_pos = *check_pos;
	BOOL didChange = FALSE;

	CPolygon *polyHit = NULL;
	root_node->find_walkable(path, &valid_pos, &polyHit, &trans, &path->localspace_z, &didChange);

	if (didChange)
	{
		Vector changeVec = valid_pos.center - check_pos->center;
		Vector adjustedVec = path->localspace_pos.localtoglobalvec(changeVec);
		Vector offset = adjustedVec;
		path->check_pos.frame.m_origin += offset;
		path->cache_global_sphere(&offset);

		Plane result = Plane::localtoglobal(path->check_pos, path->localspace_pos, polyHit->plane);

		collisions->contact_plane_valid = 1;
		collisions->contact_plane.m_normal = result.m_normal;
		collisions->contact_plane.m_dist = result.m_dist * scale;

		collisions->contact_plane_is_water = 0;
		collisions->contact_plane_cell_id = path->check_pos.objcell_id;
		path->set_walkable(&valid_pos, polyHit, &path->localspace_z, &path->localspace_pos, scale);

		return ADJUSTED_TS;
	}

	return OK_TS;
}

TransitionState BSPTREE::step_sphere_up(CTransition *transition, Vector *collision_normal)
{
	Vector v = transition->sphere_path.localspace_pos.localtoglobalvec(*collision_normal);

	if (transition->step_up(&v))
		return OK_TS;

	return transition->sphere_path.step_up_slide(&transition->object_info, &transition->collision_info);
}

BoundingType BSPTREE::sphere_intersects_cell_bsp(CSphere *sphere)
{
	return root_node->sphere_intersects_cell_bsp(sphere);
}

int BSPTREE::box_intersects_cell_bsp(BBox *box)
{
	return root_node->box_intersects_cell_bsp(box);
}

void BSPNODE::DetachPortalsAndPurgeNodes(std::vector<BSPNODE*> &keep)//SmartArray<BSPNODE *> *io_PortalsToKeep)
{
	if (pos_node)
	{
		pos_node->DetachPortalsAndPurgeNodes(keep);

		if (pos_node->type == 'PORT')
		{
			keep.push_back(pos_node); // AddToEnd
		}
		else
		{
			delete pos_node;
		}

		pos_node = NULL;
	}

	if (neg_node)
	{
		neg_node->DetachPortalsAndPurgeNodes(keep);

		if (neg_node->type == 'PORT')
		{
			keep.push_back(neg_node); // AddToEnd
		}
		else
		{
			delete neg_node;
		}

		neg_node = NULL;
	}
}

void BSPNODE::LinkPortalNodeChain(std::vector<BSPNODE*> &portals)//SmartArray<BSPNODE *> *_Portals)
{
	BSPNODE **tmp = &pos_node;
	for (int i = (int)portals.size() - 1; i >= 0; i--)
	{
		*tmp = portals[i];
		tmp = &(*tmp)->pos_node;

		//if (i > 0)
		//{
		//	pos_node->pos_node = portals[i - 1];
		//}
	}
}

void BSPTREE::RemoveNonPortalNodes()
{
	//SmartArray<BSPNODE *> PortalsToKeep;
	std::vector<BSPNODE*> portals;
	root_node->DetachPortalsAndPurgeNodes(portals);
	root_node->LinkPortalNodeChain(portals);
	//root_node->DetachPortalsAndPurgeNodes(&PortalsToKeep);
	//root_node->LinkPortalNodeChain(&PortalsToKeep);
}
