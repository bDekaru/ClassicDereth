
#pragma once

class CMaterial
{
public:
	CMaterial();
	CMaterial(CMaterial*);
	~CMaterial();

	void CheckAlphaValues();
	void SetDiffuseSimple(float Amount);
	void SetLuminositySimple(float Amount);
	void SetTranslucencySimple(float Amount);

	BOOL has_alpha;

#if PHATSDK_RENDER_AVAILABLE
	D3DMATERIAL9 d3d_material;
#endif
};
