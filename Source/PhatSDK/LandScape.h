
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

	class CLandBlock *get_landblock(uint32_t cell_id);
	class CLandCell *get_landcell(uint32_t x_lcoord_offset);

	void calc_draw_order(uint32_t, BOOL);
	void draw(void);
	void pea_draw(void); // Replacement for "draw"
	class CLandBlock *get_all(uint32_t LandBlock);
	void get_block_orient(int32_t x, int32_t y, int32_t& magic1, int32_t& magic2);
	BOOL get_block_shift(uint32_t Block1, uint32_t Block2, uint32_t* XOut, uint32_t* YOut);
	void release_all(void);
	void set_sky_position(const Position& Pos);
	void update_block(uint32_t LandBlock, int32_t ShiftX, int32_t ShiftY, BOOL Unknown);
	void update_loadpoint(uint32_t LandBlock);

	void ChangeRegion(void);
	void CleanupDetailSurfaces(void);
	BOOL GenerateDetailMesh(void);
	BOOL SetMidRadius(int32_t MidRadius);

	int32_t mid_radius; // 0x00
	int32_t mid_width; // 0x04
	CLandBlock **land_blocks; // 0x08
	LPVOID block_draw_list; // 0x0C
	uint32_t loaded_cell_id; // 0x10
	uint32_t viewer_cell_id;
	int32_t viewer_b_xoff;
	int32_t viewer_b_yoff;
	GameSky *sky; // 0x20

	/* should be:
	CSurface *landscape_detail_surface;
	CSurface *environment_detail_surface;
	CSurface *building_detail_surface;
	CSurface *object_detail_surface;
	*/

	// this isn't right
	CGfxObj *m_DetailMesh; // 0x24
	uint32_t m_28;
	uint32_t m_2C;
	uint32_t m_30;
	uint32_t m_34;
	uint32_t m_38;
	uint32_t m_3C;
};