
#pragma once

class GameSky;
class CLandBlock;
class CGfxObj;
class Position;

class LScape
{
public:
	LScape();
	~LScape();

	class CLandBlock *get_landblock(DWORD cell_id);
	class CLandCell *get_landcell(DWORD x_lcoord_offset);

	void calc_draw_order(DWORD, BOOL);
	void draw(void);
	void pea_draw(void); // Replacement for "draw"
	class CLandBlock *get_all(DWORD LandBlock);
	void get_block_orient(long x, long y, long& magic1, long& magic2);
	BOOL get_block_shift(DWORD Block1, DWORD Block2, DWORD* XOut, DWORD* YOut);
	void release_all(void);
	void set_sky_position(const Position& Pos);
	void update_block(DWORD LandBlock, long ShiftX, long ShiftY, BOOL Unknown);
	void update_loadpoint(DWORD LandBlock);

	void ChangeRegion(void);
	void CleanupDetailSurfaces(void);
	BOOL GenerateDetailMesh(void);
	BOOL SetMidRadius(long MidRadius);

	long mid_radius; // 0x00
	long mid_width; // 0x04
	CLandBlock **land_blocks; // 0x08
	LPVOID block_draw_list; // 0x0C
	DWORD loaded_cell_id; // 0x10
	DWORD viewer_cell_id;
	long viewer_b_xoff;
	long viewer_b_yoff;
	GameSky *sky; // 0x20

	/* should be:
	CSurface *landscape_detail_surface;
	CSurface *environment_detail_surface;
	CSurface *building_detail_surface;
	CSurface *object_detail_surface;
	*/

	// this isn't right
	CGfxObj *m_DetailMesh; // 0x24
	DWORD m_28;
	DWORD m_2C;
	DWORD m_30;
	DWORD m_34;
	DWORD m_38;
	DWORD m_3C;
};