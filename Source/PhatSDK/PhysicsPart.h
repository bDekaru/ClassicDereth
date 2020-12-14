
#pragma once

#include "Frame.h"
#include "Palette.h"

// class CPhysicsObj;
class CGfxObj;
class CSurface;
class CMaterial;
class BBox;
class GfxObjDegradeInfo;

enum PartDrawState
{
	DEFAULT_DS = 0,
	NODRAW_DS
};

class CPhysicsPart
{
public:
	CPhysicsPart();
	~CPhysicsPart();

	// Static Functions.
	static CPhysicsPart *makePhysicsPart(uint32_t ID);
	static BOOL LoadGfxObjArray(uint32_t ID, GfxObjDegradeInfo*& DegradeInfo, CGfxObj**& Objects);
	static void ReleaseGfxObjArray(GfxObjDegradeInfo*& DegradeInfo, CGfxObj**& Objects);

	// Static Variables.
	static uint32_t player_iid;

	// Member Functions
	BOOL Always2D();
	BOOL CopySurfaces();
	void DetermineBasePal();
	void Draw(BOOL bIsBuilding);
	BBox *GetBoundingBox() const;
	float GetMaxDegradeDistance();
	uint32_t GetObjectIID() const; // Probably inlined.
	void RestoreSurfaces();
	void SetGfxObjArray(GfxObjDegradeInfo* DegradeInfo, CGfxObj** Objects);
	void SetNoDraw(BOOL NoDraw);
	BOOL SetPart(uint32_t ID);
	void SetTranslucency(float Amount);
	BOOL UsePalette(Palette *pPalette);
	void UpdateViewerDistance(void);

	void calc_draw_frame();
	TransitionState find_obj_collisions(CTransition *transition);

#if PHATSDK_RENDER_AVAILABLE
	float m_ViewerDist; // 0x00
	Vector viewer_heading; // 0x04
	GfxObjDegradeInfo* degrades; // 0x10
	uint32_t deg_level; // 0x14
	uint32_t deg_mode; // 0x18
	uint32_t draw_state; // 0x1C
#endif

	CGfxObj** gfxobj; // 0x20
	Vector gfxobj_scale; // 0x24

	Position pos; // 0x30

#if PHATSDK_RENDER_AVAILABLE
	Position draw_pos; // 0x78

	CMaterial* material; // 0xC0
#endif

#if PHATSDK_LOAD_SURFACES
	CSurface** surfaces; // 0xC4
#endif

	uint32_t original_palette_id; // 0xC8

	// missing some rendering related stuff here

#if PHATSDK_RENDER_AVAILABLE
	Palette* shiftPal; // 0xD8
#endif

	// missing some rendering related stuff here

	CPhysicsObj* physobj; // 0xE0
	int32_t physobj_index; // 0xE4
};




