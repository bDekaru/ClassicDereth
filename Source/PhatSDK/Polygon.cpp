
#include "StdAfx.h"
#include "Vertex.h"
#include "Polygon.h"
#include "Transition.h"

CVertexArray *CPolygon::pack_verts;

CPolygon::CPolygon()
{
	vertices = NULL;
	vertex_ids = NULL;

	poly_id = -1;

	num_pts = 0;
	stippling = 0;

	sides_type = 0;
	pos_uv_indices = NULL;
	neg_uv_indices = NULL;
	pos_surface = -1;
	neg_surface = -1;

	screen = NULL;
}

CPolygon::~CPolygon()
{
	Destroy();
}

void CPolygon::Destroy()
{
	if (pos_uv_indices)
	{
		delete[] pos_uv_indices;
		pos_uv_indices = NULL;
	}

	if (sides_type == 2)
	{
		if (neg_uv_indices)
		{
			delete[] neg_uv_indices;
			neg_uv_indices = NULL;
		}
	}

	if (vertices)
	{
		delete[] vertices;
		vertices = NULL;
	}

	if (vertex_ids)
	{
		delete[] vertex_ids;
		vertex_ids = NULL;
	}

	if (screen)
	{
		delete[] screen;
		screen = NULL;
	}

	num_pts = 0;
	stippling = 0;
	pos_surface = -1;
	neg_surface = -1;
	poly_id = -1;

}

void CPolygon::SetPackVerts(CVertexArray *Verts)
{
	pack_verts = Verts;
}

BOOL CPolygon::UnPack(BYTE** ppData, ULONG iSize)
{
	UNPACK(short, poly_id);

	UNPACK(BYTE, num_pts);
	UNPACK(BYTE, stippling);

	UNPACK(DWORD, sides_type);
	UNPACK(short, pos_surface);
	UNPACK(short, neg_surface);

	vertices = new CVertex*[num_pts];
	vertex_ids = new short[num_pts];

#if PHATSDK_RENDER_AVAILABLE
	screen = new CVertex*[num_pts];
#endif

	for (DWORD i = 0; i < num_pts; i++)
	{
		short Index;
		UNPACK(short, Index);

		vertex_ids[i] = Index;
		vertices[i] = (CVertex *)((BYTE *)pack_verts->vertices + Index * CVertexArray::vertex_size);
	}

	if (!(stippling & 4))
	{
#if PHATSDK_RENDER_AVAILABLE
		pos_uv_indices = new char[num_pts];

		for (DWORD i = 0; i < num_pts; i++)
			UNPACK(char, pos_uv_indices[i]);
#else
		*ppData = *ppData + (num_pts * sizeof(char));
#endif
	}

	if ((sides_type == 2) && !(stippling & 8))
	{
#if PHATSDK_RENDER_AVAILABLE
		neg_uv_indices = new char[num_pts];

		for (DWORD i = 0; i < num_pts; i++)
			UNPACK(char, neg_uv_indices[i]);
#else
		*ppData = *ppData + (num_pts * sizeof(char));
#endif
	}

	if (sides_type == CullNone)
	{
		neg_surface = pos_surface;
		neg_uv_indices = pos_uv_indices;
	}

#ifdef PRE_TOD
	PACK_ALIGN();
#else
	// CFTOD: PACK_ALIGN();
#endif

	make_plane();

	return TRUE;
}

void CPolygon::make_plane()
{
	// Not meant for human eyes.
	int i;
	CVertex *pPin;
	CVertex **ppSpread;

	Vector Norm(0, 0, 0);

	for (i = num_pts - 2, pPin = vertices[0], ppSpread = &vertices[1]; i > 0; i--, ppSpread++)
	{
		Vector V1 = ppSpread[0]->origin - pPin->origin;
		Vector V2 = ppSpread[1]->origin - pPin->origin;

		Norm = Norm + cross_product(V1, V2);
	}

	Norm.normalize();

	float distsum = 0;

	for (i = num_pts, ppSpread = vertices; i > 0; i--, ppSpread++)
		distsum += Norm.dot_product(ppSpread[0]->origin);

	plane.m_dist = -(distsum / num_pts);
	plane.m_normal = Norm;
}

#if PHATSDK_RENDER_AVAILABLE
BOOL CPolygon::polygon_hits_ray(const Ray& ray, float *time)
{
	// return peafixed_polygon_hits_ray(ray, time);

	if (!sides_type && (plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	if (!plane.compute_time_of_intersection(ray, time))
		return FALSE;

	return point_in_polygon(ray.m_origin + (ray.m_direction * (*time)));
}

BOOL CPolygon::peafixed_polygon_hits_ray(const Ray& ray, float *depth)
{
	if (!sides_type && (plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	float u, v;

	for (int i = 1; i < num_pts - 1;)
	{
		CVertex* CurrVertex1 = vertices[0];
		CVertex* CurrVertex2 = vertices[i];
		CVertex* CurrVertex3 = vertices[++i];

		if (D3DXIntersectTri(
			(D3DXVECTOR3 *)&CurrVertex1->origin,
			(D3DXVECTOR3 *)&CurrVertex2->origin,
			(D3DXVECTOR3 *)&CurrVertex3->origin,
			(D3DXVECTOR3 *)&ray.m_origin,
			(D3DXVECTOR3 *)&ray.m_direction,
			&u, &v, depth))
			return TRUE;
	}

	return FALSE;
}
#endif

BOOL CPolygon::point_in_polygon(const Vector& point)
{
	CVertex* LastVertex = vertices[num_pts - 1];

	for (int i = 0; i < num_pts; i++)
	{
		CVertex* CurrVertex = vertices[i];

		Vector crossp =
			cross_product(plane.m_normal, CurrVertex->origin - LastVertex->origin);

		float dotp = crossp.dot_product(point - LastVertex->origin);

		if (dotp < 0.0)
			return FALSE;

		LastVertex = CurrVertex;
	}

	return TRUE;
}

BOOL CPolygon::point_in_poly2D(const Vector &point, Sidedness side)
{
	int prevVertexIndex = 0;
	for (int i = num_pts - 1; i >= 0; i--)
	{
		CVertex *prevVertex = vertices[prevVertexIndex];
		CVertex *currVertex = vertices[i];

		float yOffset = prevVertex->origin.y - currVertex->origin.y;
		float xOffset = currVertex->origin.x - prevVertex->origin.x;
		
		float someVal = -(yOffset * currVertex->origin.x) - (xOffset * currVertex->origin.y) + (xOffset * point.y) + (yOffset * point.x);
		if (side)
		{
			if (someVal < 0.0)
				return FALSE;
		}
		else if (someVal > 0.0)
		{
			return FALSE;
		}

		prevVertexIndex = i;
	}

	return TRUE;
}

int CPolygon::check_walkable(class CSphere *sphere, class Vector *up)
{
	float dp = up->dot_product(plane.m_normal);
	if (dp < F_EPSILON)
		return FALSE;

	int result = TRUE;

	Vector center = sphere->center - (*up * (plane.dot_product(sphere->center) / dp));
	float radMag = sphere->radius*sphere->radius;
	
	DWORD prev_vertex = num_pts - 1;
	for (DWORD i = 0; i < num_pts; i++)
	{
		CVertex *pPrevVertex = vertices[prev_vertex];
		prev_vertex = i;
		CVertex *pCurrVertex = vertices[i];

		Vector edge = pCurrVertex->origin - pPrevVertex->origin;
		Vector disp = center - pPrevVertex->origin;
		Vector cross = cross_product(plane.m_normal, edge);
		float someNewDp = disp.dot_product(cross);
		if (someNewDp < 0.0)
		{
			if ((cross.dot_product(cross) * radMag) < (someNewDp*someNewDp))
				return FALSE;

			double dispEdgeDotProduct = disp.dot_product(edge);
			if (dispEdgeDotProduct >= 0.0 && dispEdgeDotProduct <= edge.dot_product(edge))
				return TRUE;

			result = FALSE;
		}

		if (disp.dot_product(disp) <= radMag)
			return TRUE;
	}

	return result;
}

BOOL CPolygon::walkable_hits_sphere(SPHEREPATH *path, CSphere *object, Vector *up)
{
	if (up->dot_product(plane.m_normal) > path->walkable_allowance)
	{
		Vector contact_pt;

		BOOL hit = polygon_hits_sphere_slow_but_sure(object, &contact_pt);
		if (hit != polygon_hits_sphere(object, &contact_pt))
		{
			polygon_hits_sphere_slow_but_sure(object, &contact_pt);
			polygon_hits_sphere(object, &contact_pt);
		}

		return hit;
	}

	return FALSE;
}

int CPolygon::polygon_hits_sphere(CSphere *object, Vector *contact_pt)
{
	float v4 = plane.dot_product(object->center);
	float v5 = object->radius - F_EPSILON;

	if (v5 < fabs(v4))
		return FALSE;

	float someVal2 = v5 * v5 - v4 * v4;
	BOOL result = TRUE;

	*contact_pt = object->center - (plane.m_normal * v4);

	DWORD prev_vertex = num_pts - 1;
	for (DWORD i = 0; i < num_pts; i++)
	{
		CVertex *pPrevVertex = vertices[prev_vertex];
		prev_vertex = i;
		CVertex *pCurrVertex = vertices[i];

		Vector edge = pCurrVertex->origin - pPrevVertex->origin;
		Vector disp = *contact_pt - pPrevVertex->origin;
		Vector cross = cross_product(plane.m_normal, edge);
		float someNewDp = disp.dot_product(cross);

		if (someNewDp < 0.0)
		{
			if ((cross.dot_product(cross) * someVal2) < (someNewDp*someNewDp))
				return FALSE;

			double dispEdgeDotProduct = disp.dot_product(edge);
			if (dispEdgeDotProduct >= 0.0 && dispEdgeDotProduct <= edge.dot_product(edge))
				return TRUE;

			result = FALSE;
		}

		if (disp.dot_product(disp) <= someVal2)
			return TRUE;
	}

	return result;
}

int CPolygon::hits_sphere(CSphere *object)
{
	Vector contact_pt;
	return polygon_hits_sphere_slow_but_sure(object, &contact_pt);
}

BOOL CPolygon::polygon_hits_sphere_slow_but_sure(CSphere *object, Vector *contact_pt)
{
	double dp = plane.dot_product(object->center);
	float radMag = (object->radius - F_EPSILON);
	if (radMag >= fabs(dp))
	{
		float someVal = (radMag*radMag) - (dp*dp);
		*contact_pt = object->center - (plane.m_normal * dp);

		if (num_pts > 0)
		{
			DWORD prev_vertex = num_pts - 1;
			for (DWORD i = 0; i < num_pts; i++)
			{
				CVertex *pPrevVertex = vertices[prev_vertex];
				prev_vertex = i;
				CVertex *pCurrVertex = vertices[i];
				Vector voffset = pCurrVertex->origin - pPrevVertex->origin;

				Vector cross = cross_product(plane.m_normal, voffset);

				double someDp = (*contact_pt - pPrevVertex->origin).dot_product(cross);

				if (someDp < 0.0)
				{
					prev_vertex = num_pts - 1;
					for (i = 0; i < num_pts; i++)
					{
						pPrevVertex = vertices[prev_vertex];
						prev_vertex = i;
						pCurrVertex = vertices[i];

						Vector edge = pCurrVertex->origin - pPrevVertex->origin;
						Vector disp = *contact_pt - pPrevVertex->origin;
						cross = cross_product(plane.m_normal, edge);
						someDp = disp.dot_product(cross);

						if (someDp < 0.0)
						{
							if ((cross.dot_product(cross)*someVal) < (someDp*someDp))
								return FALSE;

							double someOtherDp = disp.dot_product(edge);
							if (someOtherDp >= 0.0 && someOtherDp <= edge.dot_product(edge))
								return TRUE;
						}

						if (disp.dot_product(disp) <= someVal)
							return TRUE;
					}

					return FALSE;
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}

int CPolygon::adjust_sphere_to_plane(SPHEREPATH *path, CSphere *valid_pos, Vector *movement)
{
	float dp1 = plane.dot_product(valid_pos->center); // v5
	float dp2 = movement->dot_product(plane.m_normal); // v9
	float v8;

	if (dp2 <= F_EPSILON)
	{
		if (dp2 >= -0.00019999999)
			return FALSE;

		v8 = dp1 - valid_pos->radius;
	}
	else
	{
		v8 = (-valid_pos->radius) - dp1;
	}

	float amt = v8 / dp2;
	float someVal = (1.0 - amt) * path->walk_interp;
	if (someVal >= path->walk_interp || someVal < -0.5)
		return FALSE;

	valid_pos->center -= (*movement * amt);
	path->walk_interp = someVal;
	return TRUE;
}

int CPolygon::pos_hits_sphere(CSphere *object, Vector *movement, Vector *contact_pt, CPolygon **struck_poly)
{
	int hit = polygon_hits_sphere_slow_but_sure(object, contact_pt);

	if (hit)
		*struck_poly = this;

	if (movement->dot_product(plane.m_normal) >= 0.0)
		return 0;

	return hit;
}

BOOL CPolygon::check_small_walkable(CSphere *sphere, Vector *up)
{
	// unsure of the accuracy of this code
	float dp = up->dot_product(plane.m_normal);
	if (dp < F_EPSILON)
		return FALSE;

	int result = TRUE;

	Vector center = sphere->center - (*up * (plane.dot_product(sphere->center) / dp));
	float radMag = sphere->radius*sphere->radius*0.25;

	DWORD prev_vertex = num_pts - 1;
	for (DWORD i = 0; i < num_pts; i++)
	{
		CVertex *pPrevVertex = vertices[prev_vertex];
		prev_vertex = i;
		CVertex *pCurrVertex = vertices[i];

		Vector edge = pCurrVertex->origin - pPrevVertex->origin;
		Vector disp = center - pPrevVertex->origin;
		Vector cross = cross_product(plane.m_normal, edge);
		float someNewDp = disp.dot_product(cross);
		if (someNewDp < 0.0)
		{
			if ((cross.dot_product(cross) * radMag) < (someNewDp*someNewDp))
				return FALSE;

			double dispEdgeDotProduct = disp.dot_product(edge);
			if (dispEdgeDotProduct >= 0.0 && dispEdgeDotProduct <= edge.dot_product(edge))
				return TRUE;

			result = FALSE;
		}

		if (disp.dot_product(disp) <= radMag)
			return TRUE;
	}

	return result;
}

double CPolygon::adjust_sphere_to_poly(CSphere *check_pos, Vector *curr_pos, Vector *movement)
{
	UNFINISHED(); // need for collision
	return 0;
}

void CPolygon::adjust_to_placement_poly(CSphere *struck_sphere, CSphere *other_sphere, float radius, int center_solid, int solid_check)
{
	double dp = plane.dot_product(struck_sphere->center);
	double v7;

	if (solid_check)
	{
		double rad = radius;
		if (center_solid)
			rad *= -1.0;
		if (dp <= 0.0)
			rad *= -1.0;
		v7 = rad - dp;
	}
	else
		v7 = radius - dp;

	Vector v = plane.m_normal * v7;
	struck_sphere->center += v;
	other_sphere->center += v;
}

//
// For ray-tracing vertices -- something not in the client.
//
#if PHATSDK_RENDER_AVAILABLE
CVertex *PickVertexFromPolygon(CPolygon *pPoly, const Ray& ray)
{
	if (!pPoly->sides_type && (pPoly->plane.m_normal.dot_product(ray.m_direction) > 0))
		return FALSE;

	float curr_depth = FLT_MAX;

	CVertex*
		result = NULL;

	for (int i = 1; i < pPoly->num_pts - 1;)
	{
		CVertex* CurrVertex1 = pPoly->vertices[0];
		CVertex* CurrVertex2 = pPoly->vertices[i];
		CVertex* CurrVertex3 = pPoly->vertices[++i];

		float u, v, depth;
		if (D3DXIntersectTri(
			(D3DXVECTOR3 *)&CurrVertex1->origin,
			(D3DXVECTOR3 *)&CurrVertex2->origin,
			(D3DXVECTOR3 *)&CurrVertex3->origin,
			(D3DXVECTOR3 *)&ray.m_origin,
			(D3DXVECTOR3 *)&ray.m_direction,
			&u, &v, &depth) && (depth < curr_depth))
		{
			Vector u_scalar = (CurrVertex2->origin - CurrVertex1->origin);
			Vector v_scalar = (CurrVertex3->origin - CurrVertex1->origin);
			Vector face_point = CurrVertex1->origin + (u_scalar * u) + (v_scalar * v);

			float vdist1 = (face_point - CurrVertex1->origin).magnitude();
			float vdist2 = (face_point - CurrVertex2->origin).magnitude();
			float vdist3 = (face_point - CurrVertex3->origin).magnitude();

			if (vdist1 <= vdist2)
				result = ((vdist1 <= vdist3) ? CurrVertex1 : CurrVertex3);
			else
				result = ((vdist2 <= vdist3) ? CurrVertex2 : CurrVertex3);
		}
	}

	return result;
}
#endif

int CPolygon::find_crossed_edge(CSphere *sphere, Vector *up, Vector *normal)
{
	float dp1 = up->dot_product(plane.m_normal); // v4

	if (fabs(dp1) < F_EPSILON)
		return FALSE;

	float dp2ratio = (plane.dot_product(sphere->center) / dp1);
	Vector center = sphere->center - (*up * dp2ratio);

	DWORD prev_vertex = num_pts - 1;
	for (DWORD i = 0; i < num_pts; i++)
	{
		CVertex *pPrevVertex = vertices[prev_vertex];
		prev_vertex = i;
		CVertex *pCurrVertex = vertices[i];

		Vector edge = pCurrVertex->origin - pPrevVertex->origin;
		Vector disp = center - pPrevVertex->origin;
		Vector cross = cross_product(plane.m_normal, edge);

		if (disp.dot_product(cross) < 0.0)
		{
			*normal = cross;
			*normal = *normal * (1.0 / normal->magnitude());
			return TRUE;
		}
	}

	return FALSE;
}