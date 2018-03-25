
#include "StdAfx.h"
#include "Material.h"

CMaterial::CMaterial()
{
	has_alpha = FALSE;

#if PHATSDK_RENDER_AVAILABLE
	memset(&d3d_material, 0, sizeof(d3d_material));
	d3d_material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	d3d_material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
#endif
}

CMaterial::CMaterial(CMaterial *pMaterial)
{
	has_alpha = pMaterial->has_alpha;
}

CMaterial::~CMaterial()
{
}

void CMaterial::CheckAlphaValues()
{
#if PHATSDK_RENDER_AVAILABLE
	if (d3d_material.Ambient.a < 1.0f || d3d_material.Diffuse.a < 1.0f || d3d_material.Specular.a < 1.0f || d3d_material.Emissive.a < 1.0f)
		has_alpha = TRUE;
	else
		has_alpha = FALSE;
#endif
}

void CMaterial::SetDiffuseSimple(float Amount)
{
#if PHATSDK_RENDER_AVAILABLE
	d3d_material.Diffuse.r = Amount;
	d3d_material.Diffuse.g = Amount;
	d3d_material.Diffuse.b = Amount;
#endif
}

void CMaterial::SetLuminositySimple(float Amount)
{
#if PHATSDK_RENDER_AVAILABLE
	d3d_material.Emissive.r = Amount;
	d3d_material.Emissive.g = Amount;
	d3d_material.Emissive.b = Amount;
#endif
}

void CMaterial::SetTranslucencySimple(float Amount)
{
#if PHATSDK_RENDER_AVAILABLE
	Amount = 1 - Amount;
	d3d_material.Ambient.a = Amount;
	d3d_material.Diffuse.a = Amount;
	d3d_material.Specular.a = Amount;
	d3d_material.Emissive.a = Amount;

	CheckAlphaValues();
#endif
}


