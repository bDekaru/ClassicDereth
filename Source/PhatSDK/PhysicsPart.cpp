
#include <StdAfx.h>
#include "Material.h"
#include "GfxObj.h"
#include "Surface.h"
#include "Palette.h"
#include "PhysicsPart.h"
#include "PhysicsObj.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

uint32_t CPhysicsPart::player_iid = -1;

CPhysicsPart::CPhysicsPart()
{
    gfxobj = NULL;
    gfxobj_scale = Vector(1.0f, 1.0f, 1.0f);

    physobj = NULL;
    physobj_index = -1;
    original_palette_id = 0;

#if PHATSDK_RENDER_AVAILABLE
	deg_level = 0;
	degrades = NULL;
	deg_mode = 1;
	viewer_heading = Vector(0, 0, 1);
	draw_state = 0;
	material = NULL;
	surfaces = NULL;
	shiftPal = NULL;
    m_ViewerDist = FLT_MAX;
#endif
}

CPhysicsPart::~CPhysicsPart()
{
#if PHATSDK_RENDER_AVAILABLE
    if (shiftPal)
    {
        Palette::releasePalette(shiftPal);
        shiftPal = NULL;
    }

    if (gfxobj && gfxobj[0])
    {
        if (material != gfxobj[0]->material)
        {
            if (material)
                delete material;
        }
        material = NULL;
    }
#endif

    RestoreSurfaces();

#if PHATSDK_RENDER_AVAILABLE
    ReleaseGfxObjArray(degrades, gfxobj);
#else
	GfxObjDegradeInfo *dummy = NULL;
	ReleaseGfxObjArray(dummy, gfxobj);
#endif
}

CPhysicsPart *CPhysicsPart::makePhysicsPart(uint32_t ID)
{
    CPhysicsPart *pPart = new CPhysicsPart();

    if (!pPart->SetPart(ID))
    {
        delete pPart;
        pPart = NULL;
    }

    return pPart;
}

BOOL CPhysicsPart::SetPart(uint32_t ID)
{
    if (!ID)
        return FALSE;

    GfxObjDegradeInfo* deginfo = NULL;
    CGfxObj** objarray = NULL;

    if (!LoadGfxObjArray(ID, deginfo, objarray))
        return FALSE;

    SetGfxObjArray(deginfo, objarray);
    return TRUE;
}

void CPhysicsPart::RestoreSurfaces()
{
#if PHATSDK_RENDER_AVAILABLE
    if (gfxobj)
    {
        if (gfxobj[0])
        {
            if (gfxobj[0]->m_rgSurfaces != surfaces)
            {
                for (uint32_t i = 0; i < gfxobj[0]->num_surfaces; i++)
                    CSurface::releaseCustomSurface(surfaces[i]);

                delete [] surfaces;
                surfaces = gfxobj[0]->m_rgSurfaces;
            }
        }
    }
#endif
}

void CPhysicsPart::DetermineBasePal()
{
#if PHATSDK_LOAD_SURFACES
    for (uint32_t i = 0; i < gfxobj[0]->num_surfaces; i++)
    {
        if (surfaces[i] && surfaces[i]->GetOriginalPaletteID())
        {
            original_palette_id = surfaces[i]->GetOriginalPaletteID();
            return;
        }
    }
#endif

    original_palette_id = 0;
}

#if PHATSDK_RENDER_AVAILABLE
BOOL CPhysicsPart::UsePalette(Palette *pPalette)
{
    if (!pPalette)
        return FALSE;

    if (surfaces == gfxobj[0]->m_rgSurfaces && !CopySurfaces())
        return FALSE;

    for (uint32_t i = 0; i < gfxobj[0]->num_surfaces; i++)
        surfaces[i]->UsePalette(pPalette);

    if (shiftPal)
    {
        Palette::releasePalette(shiftPal);
        shiftPal = NULL;
    }

    shiftPal = Palette::copyRef(pPalette);
    return TRUE;
}

BOOL CPhysicsPart::CopySurfaces()
{
    if (surfaces != gfxobj[0]->m_rgSurfaces)
        return TRUE;

    surfaces = new CSurface*[ gfxobj[0]->num_surfaces ];

    if (!surfaces)
        return FALSE;

    for (uint32_t i = 0; i < gfxobj[0]->num_surfaces; i++)
    {
        surfaces[i] = CSurface::makeCustomSurface(gfxobj[0]->m_rgSurfaces[i]);

        if (!surfaces[i])
            return FALSE;
    }

    return TRUE;
}
#endif

void CPhysicsPart::SetGfxObjArray(GfxObjDegradeInfo *DegradeInfo, CGfxObj **Objects)
{
    RestoreSurfaces();

#if PHATSDK_RENDER_AVAILABLE
	ReleaseGfxObjArray(degrades, gfxobj);
	degrades = DegradeInfo;
#else
	GfxObjDegradeInfo *dummy = NULL;
    ReleaseGfxObjArray(dummy, gfxobj);
#endif

#if PHATSDK_LOAD_SURFACES
	surfaces = Objects[0]->m_rgSurfaces;
#endif

    gfxobj = Objects;

    DetermineBasePal();

#if PHATSDK_RENDER_AVAILABLE
    if (shiftPal)
        UsePalette(shiftPal);
#endif
}

BOOL CPhysicsPart::LoadGfxObjArray(uint32_t ID, GfxObjDegradeInfo*& DegradeInfo, CGfxObj**& Objects)
{
#if PHATSDK_RENDER_AVAILABLE

#if PRE_TOD
    DegradeInfo = GfxObjDegradeInfo::Get(0x11000000 | (ID & 0xFFFFFF));
#else
	CGfxObj *pGfxObjBase = CGfxObj::Get(ID);
	if (!pGfxObjBase)
	{
		return FALSE;
	}

	DegradeInfo = GfxObjDegradeInfo::Get(pGfxObjBase->m_didDegrade);
	CGfxObj::Release(pGfxObjBase);
#endif

    if (DegradeInfo)
    {
        Objects = new CGfxObj*[DegradeInfo->num_degrades];

        for (uint32_t i = 0; i < DegradeInfo->num_degrades; i++)
            Objects[i] = CGfxObj::Get(DegradeInfo->degrades[i].gfxobj_id);
    }
    else
    {
        Objects = new CGfxObj*[1];
        Objects[0] = CGfxObj::Get(ID);
    }
#else
	DegradeInfo = NULL;
	Objects = new CGfxObj*[1];
	Objects[0] = CGfxObj::Get(ID);
#endif

    if (!Objects[0])
    {
        ReleaseGfxObjArray(DegradeInfo, Objects);
        return FALSE;
    }

    return TRUE;
}

void CPhysicsPart::ReleaseGfxObjArray(GfxObjDegradeInfo*& DegradeInfo, CGfxObj**& Objects)
{
    uint32_t ObjCount;

    if (DegradeInfo)
    {
        ObjCount = DegradeInfo->num_degrades;
        GfxObjDegradeInfo::Release(DegradeInfo);
        DegradeInfo = NULL;
    }
    else
	{
        ObjCount = 1;
	}
    
    if (Objects)
    {
        for (uint32_t i = 0; i < ObjCount; i++)
        {
            if (Objects[i])
            {
                CGfxObj::Release(Objects[i]);
                Objects[i] = NULL;
            }
        }

        delete [] Objects;
        Objects = NULL;
    }
}

float CPhysicsPart::GetMaxDegradeDistance()
{
#if PHATSDK_RENDER_AVAILABLE
    if (degrades)
        return degrades->get_max_degrade_distance();
#endif

    return 100.0f;
}

BBox *CPhysicsPart::GetBoundingBox() const
{
    return &(gfxobj[0]->gfx_bound_box);
}

void CPhysicsPart::SetNoDraw(BOOL NoDraw)
{
#if PHATSDK_RENDER_AVAILABLE
    if (NoDraw)
        draw_state |= NODRAW_DS;
    else
        draw_state &= ~NODRAW_DS;
#endif
}

void CPhysicsPart::SetTranslucency(float Amount)
{
#if PHATSDK_RENDER_AVAILABLE
    if (!physobj || !(physobj->m_PhysicsState & CLOAKED_PS))
    {
        if (FALSE /*RenderOptions::bUseMaterials */)
        {
            DebugBreak();
            // if (CopyMaterial())
            //    m_Material->SetTranslucencySimple(Amount);
        }
        else
        {
            if (Amount == 1.0f)
                draw_state |= NODRAW_DS;
            else
            {
                draw_state &= ~NODRAW_DS;

                if (gfxobj[0]->m_rgSurfaces != surfaces || CopySurfaces())
                {
                    for (uint32_t i = 0; i < gfxobj[0]->num_surfaces; i++)
                        surfaces[i]->SetTranslucency(Amount);
                }
            }
        }
    }
#endif
}

// Probably inlined.
uint32_t CPhysicsPart::GetObjectIID(void) const
{
    return (physobj ? physobj->GetID() : 0);
}

#if PHATSDK_RENDER_AVAILABLE
void CPhysicsPart::UpdateViewerDistance(void)
{
    Vector offset = Render::ViewerPos.get_offset(pos, gfxobj[0]->sort_center * gfxobj_scale);
    
    // Cache off distance for later usage.
    m_ViewerDist = offset.magnitude();

    if (m_ViewerDist > F_EPSILON)
        viewer_heading = offset.normalize();
    else
        viewer_heading = Vector(0, 0, 1);

    if (degrades && (GetObjectIID() != player_iid))
    {
        degrades->get_degrade(m_ViewerDist / gfxobj_scale.z, &deg_level, &deg_mode);
    }
    else
    {
        deg_level = 0;
        deg_mode = 1;
    }

    if (gfxobj[deg_level])
        calc_draw_frame();
}

void CPhysicsPart::calc_draw_frame(void)
{
    draw_pos = pos;

    switch(deg_mode)
    {
    case 1:
        // Do nothing..
        break;
    case 2:
        draw_pos.frame.set_vector_heading(viewer_heading);
        break;
    case 3:
        draw_pos.frame.rotate_around_axis_to_vector(0, viewer_heading);
        break;
    case 4:
        draw_pos.frame.rotate_around_axis_to_vector(1, viewer_heading);
        break;
    case 5:
        draw_pos.frame.rotate_around_axis_to_vector(2, viewer_heading);
        break;
    }
}

void CPhysicsPart::Draw(BOOL bIsBuilding)
{
    if (draw_state & NODRAW_DS)
         return;

    uint32_t activeindex = (degrades && (deg_level < degrades->num_degrades)) ? deg_level : 0;

    if (!gfxobj[activeindex])
        return;

    UpdateViewerDistance();

    Render::SetMaterial(material);
    Render::SetSurfaceArray(surfaces);
    Render::SetObjectScale(&gfxobj_scale);

    /*
    Omitted:
    if (!m_PhysicsObj || !m_dw08)
    {
        Render::check_curr_object = FALSE;
        
        if (creature_mode)
            Render::check_curr_object = TRUE;
    }
    else
        Render::check_curr_object = TRUE;
    */

    // ACRenderDevice::DrawMesh
    int iResult = Render::DrawMesh(gfxobj[activeindex], &draw_pos, bIsBuilding);

    if (iResult == 3)
    {
        if (m_ViewerDist < Render::m_CurrDepth)
            Render::m_CurrDepth = m_ViewerDist;
    }


    /*
    Omitted:
    switch (iResult)
    {
        // These assume 0x08 is GetID()
    case 3:
        selected_object_id = (m_PhysicsObj ? m_PhysicsObj->GetID() : 0);
        selected_object_index = m_PartIndex;

    case 2:            
        
        if (viewcone_check_object_id == (m_PhysicsObj ? m_PhysicsObj->GetID() : 0))
            selected_object_in_view = TRUE;
    }
    */
}
#endif

TransitionState CPhysicsPart::find_obj_collisions(CTransition *transition)
{
	TransitionState ts = OK_TS;

	if (gfxobj[0] && gfxobj[0]->physics_bsp)
	{
		transition->sphere_path.cache_localspace_sphere(&pos, gfxobj_scale.z);
		ts = gfxobj[0]->find_obj_collisions(transition, gfxobj_scale.z);
	}

	return ts;
}

BOOL CPhysicsPart::Always2D()
{
#if PHATSDK_RENDER_AVAILABLE
	if (degrades && degrades->num_degrades)
		return degrades->degrades->degrade_mode != 1;
#endif

	return 0;
}
