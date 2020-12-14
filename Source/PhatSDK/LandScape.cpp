
#include <StdAfx.h>
#include "LandScape.h"
#include "LandBlock.h"
#include "LandDefs.h"
#include "GameSky.h"
#include "GfxObj.h"
#include "LandCell.h"

#if PHATSDK_RENDER_AVAILABLE
#include "Render.h"
#endif

LPVOID block_interval = NULL;

LScape::LScape()
{
    land_blocks = NULL;
    block_draw_list = NULL;
    loaded_cell_id = 0;
    viewer_cell_id = 0;
    viewer_b_xoff = 0;
    viewer_b_yoff = 0;
    m_DetailMesh = NULL;
    m_2C = 1;
    m_30 = 0;
    m_34 = 0;
    m_38 = 0;
    m_3C = 0;

    if (/*RenderOptions::bSingleSurfaceLScape*/ FALSE)
    {
        mid_radius = 1;
        mid_width = 3;
    }
    else
    {
        mid_radius = 5;
        mid_width = 11;
    }

    // ambient_level = 0.4;
    // sunlight = 1.2;
    // uint32_t_5F15E0 = 0;
    // uint32_t_5F15E4 = 0.5;

    CLandBlock::init();

    // Finish me first.
    sky = NULL; // new GameSky;
}

LScape::~LScape()
{
    release_all();

    if (block_interval)
    {
        delete [] block_interval;
        block_interval = NULL;
    }

    if (sky)
    {
        delete sky;
        sky = NULL;
    }

    if (m_DetailMesh)
    {
        delete m_DetailMesh;
        m_DetailMesh = NULL;
    }

    CleanupDetailSurfaces();
}

void LScape::CleanupDetailSurfaces(void)
{
    // Finish me
}

void LScape::ChangeRegion(void)
{
    if (!GenerateDetailMesh())
    {
        if (m_DetailMesh)
            delete m_DetailMesh;

        m_DetailMesh = NULL;
    }
}

BOOL LScape::GenerateDetailMesh(void)
{
    // Finish me.
    return FALSE;
}

#if PHATSDK_RENDER_AVAILABLE
void LScape::draw(void)
{
    if (sky)
   {
        UNFINISHED_LEGACY("sky->Draw(0);");
   }

    UNFINISHED_LEGACY("Missing landscape draw code");

    for (int i = (mid_width * mid_width) - 1; i > 0; i--)
    {

    }

    if (sky && m_2C)
   {
        UNFINISHED_LEGACY("sky->Draw(1);");
   }
}

void LScape::pea_draw(void)
{
    if (land_blocks)
    {
        for (int i = (mid_width * mid_width) - 1; i > 0; i--)
        {
            if (land_blocks[i])
            {
				FakeRenderDevice::DrawBlock(land_blocks[i]);
            }
        }
    }
}
#endif

CLandBlock* LScape::get_all(uint32_t LandBlock)
{
    CLandBlock *pLandBlock;

    if (pLandBlock = CLandBlock::Get(LandBlock))
    {
        int32_t LCoordX, LCoordY;

        LandDefs::blockid_to_lcoord(
            LandBlock, LCoordX, LCoordY);

        pLandBlock->block_coord.x = LCoordX;
        pLandBlock->block_coord.y = LCoordY;
    }
    else
        DEBUGOUT("Missing LandBlock %08X\r\n", LandBlock);

    return pLandBlock;
}

BOOL LScape::get_block_shift(uint32_t Block1, uint32_t Block2, uint32_t* XOut, uint32_t* YOut)
{
    if (!Block1)
        return FALSE;

    int32_t X2, Y2, X1, Y1;
    LandDefs::blockid_to_lcoord(Block2, X2, Y2);
    LandDefs::blockid_to_lcoord(Block1, X1, Y1);

    *XOut = (X2 - X1) / LandDefs::lblock_side;
    *YOut = (Y2 - Y1) / LandDefs::lblock_side;

    return TRUE;
}

void LScape::update_loadpoint(uint32_t LandBlock)
{
    if (!LandBlock)
    {
        release_all();
        return;
    }

    uint32_t ShiftX, ShiftY;
    BOOL Unknown;
    Unknown = !get_block_shift(loaded_cell_id, LandBlock, &ShiftX, &ShiftY);

    BOOL MissingBlock = FALSE;

    if (land_blocks)
    {
        for (int x = 0; x < mid_width; x++) {
            for (int y = 0; y < mid_width; y++) {
                if (!land_blocks[x*mid_width + y])
                {
                    MissingBlock = FALSE;
                    break;
                }
            }
        }
    }

    if (ShiftX || ShiftY || !land_blocks || Unknown || MissingBlock)
    {
        update_block(LandBlock, ShiftX, ShiftY, Unknown);

        if ((loaded_cell_id >> LandDefs::block_part_shift) != (LandBlock >> LandDefs::block_part_shift))
            Unknown = TRUE;

        loaded_cell_id = LandBlock;
        calc_draw_order(viewer_cell_id, Unknown);
    }
    else
        loaded_cell_id = LandBlock;
}

void LScape::calc_draw_order(uint32_t, BOOL)
{
    // Not done.
}

void LScape::update_block(uint32_t LandBlock, int32_t ShiftX, int32_t ShiftY, BOOL Unknown)
{
    int32_t GidX, GidY;
    LandDefs::gid_to_lcoord(
        LandBlock, GidX, GidY);

    GidX -= mid_radius * LandDefs::lblock_side;
    GidY -= mid_radius * LandDefs::lblock_side;

    if (land_blocks && !Unknown &&
        (ShiftX <  mid_width)    && (ShiftY <  mid_width) &&
        (ShiftX > -mid_width) && (ShiftY > -mid_width))
    {
        if (ShiftX != 0 || ShiftY != 0)
        {
            if (ShiftX < 0)
            {
                if (ShiftY < 0)
                {
                    for (int x = mid_width - 1, gid_x = (x * LandDefs::lblock_side) + GidX; x >= 0; x--, gid_x -= LandDefs::lblock_side) {
                        for (int y = mid_width - 1, gid_y = (y * LandDefs::lblock_side) + GidY; y >= 0; y--, gid_y -= LandDefs::lblock_side) {

                            if (((mid_width + ShiftX) >= x) || ((mid_width + ShiftY) >= y))
                            {
                                if (land_blocks[x*mid_width + y])
                                    land_blocks[x*mid_width + y]->release_all();
                            }

                            if (((ShiftX + x) >= 0) && ((ShiftY + y) >= mid_width))
                                land_blocks[mid_width*x + y] = land_blocks[(mid_width*(ShiftX + x)) + (ShiftY + y)];
                            else
                            {
                                if (LandDefs::in_bounds(gid_x, gid_y))
                                    land_blocks[x*mid_width + y] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                                else
                                    land_blocks[x*mid_width + y] = NULL;
                            }
                        }
                    }
                }
                else
                {
                    for (int x = mid_width - 1, gid_x = (x * LandDefs::lblock_side) + GidX; x >= 0; x--, gid_x -= LandDefs::lblock_side) {
                        for (int y = 0, gid_y = GidY; y < mid_width; y++, gid_y += LandDefs::lblock_side) {

                            if ((x >= (mid_width + ShiftX)) || (ShiftY > y))
                            {
                                if (land_blocks[x*mid_width + y])
                                    land_blocks[x*mid_width + y]->release_all();
                            }

                            if (((ShiftX + x) >= 0) && ((ShiftY + y) < mid_width))
                                land_blocks[mid_width*x + y] = land_blocks[(mid_width*(ShiftX + x)) + (ShiftY + y)];
                            else
                            {
                                if (LandDefs::in_bounds(gid_x, gid_y))
                                    land_blocks[x*mid_width + y] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                                else
                                    land_blocks[x*mid_width + y] = NULL;
                            }
                        }
                    }
                }
            }
            else
            if (ShiftY < 0)
            {
                for (int x = 0, gid_x = GidX; x < mid_width; x++, gid_x += LandDefs::lblock_side) {
                    for (int y = mid_width - 1, gid_y = (y * LandDefs::lblock_side) + GidY; y >= 0; y--, gid_y -= LandDefs::lblock_side) {

                        if ((ShiftX > x) || (y >= (mid_width + ShiftY)))
                        {
                            if (land_blocks[x*mid_width + y])
                                land_blocks[x*mid_width + y]->release_all();
                        }

                        if (((ShiftX + x) < mid_width) && ((ShiftY + y) >= 0))
                            land_blocks[mid_width*x + y] = land_blocks[(mid_width*(ShiftX + x)) + (ShiftY + y)];
                        else
                        {
                            if (LandDefs::in_bounds(gid_x, gid_y))
                                land_blocks[x*mid_width + y] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                            else
                                land_blocks[x*mid_width + y] = NULL;
                        }
                    }
                }
            }
            else
            {
                for (int x = 0, gid_x = GidX; x < mid_width; x++, gid_x += LandDefs::lblock_side) {
                    for (int y = 0, gid_y = GidY; y < mid_width; y++, gid_y += LandDefs::lblock_side) {

                        if (x < ShiftX || y < ShiftY) {
                            if (land_blocks[x*mid_width + y])
                                land_blocks[x*mid_width + y]->release_all();
                        }

                        int32_t curr_x = ShiftX + x;
                        int32_t curr_y = ShiftY + y;

                        if ((curr_x < mid_width) && (curr_y < mid_width))
                            land_blocks[x*mid_width + y] = land_blocks[curr_x*mid_width + curr_y];
                        else
                        {
                            if (LandDefs::in_bounds(gid_x, gid_y))
                                land_blocks[x*mid_width + y] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                            else
                                land_blocks[x*mid_width + y] = NULL;
                        }
                    }
                }
            }
        }
        else
        {
            for (int x = 0, gid_x = GidX; x < mid_width; x++, gid_x += LandDefs::lblock_side) {
                for (int y = 0, gid_y = GidY; y < mid_width; y++, gid_y += LandDefs::lblock_side) {
                    uint32_t index = x*mid_width + y;

                    if (!land_blocks[index]) {
                        if (LandDefs::in_bounds(gid_x, gid_y))
                            land_blocks[index] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                        else
                            land_blocks[index] = NULL;
                    }
                }
            }
        }
    }
    else
    {
        release_all();

        if (!land_blocks)
            land_blocks = new CLandBlock*[ mid_width * mid_width ];

        for (int x = 0, gid_x = GidX; x < mid_width; x++, gid_x += LandDefs::lblock_side) {
            for (int y = 0, gid_y = GidY; y < mid_width; y++, gid_y += LandDefs::lblock_side) {
                uint32_t index = (x*mid_width) + y;

                if (LandDefs::in_bounds(gid_x, gid_y))
                    land_blocks[index] = get_all(LandDefs::get_block_gid(gid_x, gid_y));
                else
                    land_blocks[index] = NULL;
            }
        }
    }

    for (int x = 0; x < mid_width; x++) {
        for (int y = 0; y < mid_width; y++) {

            CLandBlock *pLB = land_blocks[x*mid_width + y];

            if (pLB)
            {
                int32_t Magic1, Magic2;
                get_block_orient(x, y, Magic1, Magic2);

                if ((pLB->side_cell_count == LandDefs::lblock_side) &&
                     (pLB->side_cell_count != (LandDefs::lblock_side / Magic1)))
                    pLB->notify_change_size();
                
                if (pLB->generate(pLB->GetID(), Magic1, Magic2))
                {
                    pLB->closest.x = -1;
                    pLB->closest.y = -1;

                    pLB->init_lcell_ptrs();
                    pLB->calc_lighting();
                    pLB->get_land_limits();
                }
            }
        }
    }
}

BOOL LScape::SetMidRadius(int32_t MidRadius)
{
    if (MidRadius < 1)
        return FALSE;

    // if (RenderOptions::bSingleSurfaceLScape && MidRadius != 1)
    //      return FALSE;

    if (land_blocks)
        return FALSE;

    mid_radius = MidRadius;
    mid_width = MidRadius*2 + 1;
    return TRUE;
}

void LScape::get_block_orient(int32_t x, int32_t y, int32_t& magic1, int32_t& magic2)
{
    int32_t ydist = y - mid_radius;
    int32_t ydista = abs(ydist);
    int32_t xdist = x - mid_radius;
    int32_t xdista = abs(xdist);

    if (xdista <= ydista)
        xdista = ydista;

    if (xdista <= 1)
        magic1 = 1;
    else
    if (xdista <= 2)
        magic1 = 2;
    else
    if (xdista <= 4)
        magic1 = 4;
    else
    {
        magic1 = 8;
        magic2 = 0;
        return;
    }

    if (xdist != magic1)
    {
        if (xdist != -magic1)
        {
            if (ydist != magic1)
            {
                if (ydist != -magic1)
                    magic2 = 0;
                else
                    magic2 = 2;
            }
            else
                magic2 = 1;
        }
        else
        if (ydist != magic1)
        {
            if (ydist != -magic1)
                magic2 = 4;
            else
                magic2 = 6;
        }
        else
            magic2 = 5;
    }
    else
    if (ydist != magic1)
    {
        if (ydist != -magic1)
            magic2 = 3;
        else
            magic2 = 8;
    }
    else
        magic2 = 7;
}

void LScape::release_all(void)
{
    if (land_blocks)
    {
        for (int x = 0; x < mid_width; x++) {
            for (int y = 0; y < mid_width; y++) {

                CLandBlock *pLandBlock = land_blocks[(x * mid_width) + y];
                
                if (pLandBlock)
                    pLandBlock->release_all();

                land_blocks[(x * mid_width) + y] = NULL;
            }
        }

        if (land_blocks)
        {
            delete [] land_blocks;
            land_blocks = NULL;
        }

        if (block_draw_list)
        {
            delete [] block_draw_list;
            block_draw_list = NULL;
        }
    }

    if (sky)
	{
		UNFINISHED_LEGACY("sky->SetInactive();");
	}

    loaded_cell_id = 0;
    viewer_cell_id = 0;
}

void LScape::set_sky_position(const Position& Pos)
{
    if (loaded_cell_id && sky)
    {
        UNFINISHED_LEGACY("sky->UpdatePosition()");
        // sky->UpdatePosition(Pos);
    }
}

CLandBlock *LScape::get_landblock(uint32_t cell_id)
{
	UNFINISHED();
	return NULL;
}

CLandCell *LScape::get_landcell(uint32_t x_lcoord_offset)
{
	CLandBlock *pLandBlock = get_landblock(x_lcoord_offset);

	if (pLandBlock)
	{
		int32_t x, y;
		LandDefs::gid_to_lcoord(x_lcoord_offset, x, y);
		return &pLandBlock->lcell[y % 8 + x_lcoord_offset % 8 * pLandBlock->side_cell_count];
	}

	return NULL;
}



