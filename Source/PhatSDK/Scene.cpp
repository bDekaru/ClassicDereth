
#include <StdAfx.h>
#include "Scene.h"

ObjectDesc::ObjectDesc()
{
}

ObjectDesc::~ObjectDesc()
{
}

DEFINE_PACK(ObjectDesc)
{
	UNFINISHED();
}

DEFINE_UNPACK(ObjectDesc)
{
	obj_id = pReader->Read<uint32_t>();
	base_loc.UnPack(pReader);
	freq = pReader->Read<float>();
	displace_x = pReader->Read<float>();
	displace_y = pReader->Read<float>();
	min_scale = pReader->Read<float>();
	max_scale = pReader->Read<float>();
	max_rot = pReader->Read<float>();
	min_slope = pReader->Read<float>();
	max_slope = pReader->Read<float>();
	align = pReader->Read<int>();
	orient = pReader->Read<int>();
	weenie_obj = pReader->Read<int>();

	return true;
}

double ObjectDesc::ScaleObj(unsigned int x, unsigned int y, unsigned int k)
{
	if (min_scale == max_scale)
		return max_scale;

	return pow(max_scale / min_scale,
		(double)(1813693831 * y - (k + 32593) * (1360117743 * y * x + 1888038839) - 1109124029 * x) * 2.3283064e-10) * min_scale;
}

void ObjectDesc::ObjAlign(Plane *plane, Vector *loc, Frame *obj_frame)
{
	*obj_frame = base_loc;
	obj_frame->m_origin = *loc;	
	obj_frame->set_heading((plane->m_normal * -1.0f).get_heading());
}

BOOL ObjectDesc::CheckSlope(float z_val)
{
	return z_val >= (double)min_slope && z_val <= (double)max_slope;
}

void ObjectDesc::GetObjFrame(unsigned int x, unsigned int y, unsigned int k, Vector *loc, Frame *obj_frame)
{
	*obj_frame = base_loc;
	obj_frame->m_origin = *loc;
	if (max_rot > 0.0)
	{
		float degrees = (double)(1813693831 * y - (k + 63127) * (1360117743 * y * x + 1888038839) - 1109124029 * x) * 2.3283064e-10 * max_rot;
		obj_frame->set_heading(degrees);
	}
}

void ObjectDesc::Place(unsigned int ix, unsigned int iy, unsigned int iq, Vector *obj_loc)
{
	*obj_loc = base_loc.m_origin;

	double baseX, baseY;

	if (displace_x <= 0.0)
		baseX = obj_loc->x;
	else
		baseX = (double)(1813693831 * iy - (iq + 45773) * (1360117743 * iy * ix + 1888038839) - 1109124029 * ix) * 2.3283064e-10 * displace_x + obj_loc->x;

	if (this->displace_y <= 0.0)
		baseY = obj_loc->y;
	else
		baseY = (double)(1813693831 * iy - (iq + 72719) * (1360117743 * iy * ix + 1888038839) - 1109124029 * ix) * 2.3283064e-10 * displace_y + obj_loc->y;

	double v7 = (double)(1813693831 * iy - ix * (1870387557 * iy + 1109124029) - 402451965) * 2.3283064e-10;

	if (v7 >= 0.25)
	{
		if (v7 >= 0.5)
		{
			if (v7 >= 0.75)
			{
				obj_loc->x = baseY;
				obj_loc->y = -baseX;
			}
			else
			{
				obj_loc->x = -baseX;
				obj_loc->y = -baseY;
			}
		}
		else
		{
			obj_loc->x = -baseY;
			obj_loc->y = baseX;
		}
	}
	else
	{
		obj_loc->y = baseY;
		obj_loc->x = baseX;
	}
}


Scene::Scene()
{
}

Scene::~Scene()
{
	Destroy();
}

DBObj *Scene::Allocator()
{
	return((DBObj *)new Scene());
}

void Scene::Destroyer(DBObj *pScene)
{
	delete ((Scene *)pScene);
}

Scene *Scene::Get(uint32_t ID)
{
	return (Scene *)ObjCaches::Scenes->Get(ID);
}

void Scene::Release(Scene *pScene)
{
	if (pScene)
		ObjCaches::Scenes->Release(pScene->GetID());
}

void Scene::Destroy()
{
	if (objects)
	{
		delete [] objects;
		objects = NULL;
	}
	num_objects = 0;
}

DEFINE_PACK(Scene)
{
	UNFINISHED();
}

DEFINE_UNPACK(Scene)
{
	Destroy();
	
	version = pReader->ReadUInt32();
	num_objects = pReader->ReadUInt32();
	objects = new ObjectDesc[num_objects];

	for (uint32_t i = 0; i < num_objects; i++)
	{
		objects[i].UnPack(pReader);
	}

	return true;
}

DEFINE_LEGACY_PACK_MIGRATOR(Scene);

