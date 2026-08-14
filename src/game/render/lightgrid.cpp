// ============================================================================
// lightgrid.cpp - render.o LightGridMgr (LightGrid.cpp)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>

namespace LightGrid {
struct GridPoint {
    unsigned int gridpoint;  // +0x00
};

struct Cell {
    math::Position3::Packed mBase;  // +0x00 (12 bytes)
    float mXGridDelta;              // +0x0C
    float mYGridDelta;              // +0x10
    uint16_t mNumXRows;             // +0x14
    uint16_t mNumYRows;             // +0x16
    unsigned int mFirstGridPoint;   // +0x18
    unsigned int mCellIndex;        // +0x1C
};

struct TOC {
    void* mLights;         // +0x00
    int mNumLights;        // +0x04
    GridPoint* mGridPoints;  // +0x08
    int mNumGridPoints;    // +0x0C
    void* mLightIndices;   // +0x10
    int mNumLightIndices;  // +0x14
    Cell* mCells;          // +0x18
    int mNumCells;         // +0x1C
    void* mDLightInfos;    // +0x20
    int mNumDLightInfos;   // +0x24
};
}  // namespace LightGrid

class LightGridMgr {
private:
    bool GetGridInfo(const LightGrid::TOC& toc, int cellidx,
                     const math::Position3& pos,
                     const LightGrid::GridPoint** grid,
                     float* weights);  // ?GetGridInfo@LightGridMgr@@AAE_NABUTOC@LightGrid@@HABVPosition3@math@@PAPBUGridPoint@3@PAM@Z
    void GetAmbientColors(const LightGrid::GridPoint** grid,
                          math::Position3* ambientColors);  // ?GetAmbientColors@LightGridMgr@@AAEXPAPBUGridPoint@LightGrid@@PAVPosition3@math@@@Z
};

extern int g_LightGridDecruftifier;  // ?g_LightGridDecruftifier@@3HA (g.o)

// ea: 0x006BC8B0
bool LightGridMgr::GetGridInfo(const LightGrid::TOC& toc, int cellidx,
                               const math::Position3& pos,
                               const LightGrid::GridPoint** grid,
                               float* weights)
{
    int mNumCells = toc.mNumCells;
    if (mNumCells <= 0)
        return false;
    LightGrid::Cell* mCells = toc.mCells;
    int v7 = 0;
    while (mCells[v7].mCellIndex != (unsigned int)cellidx)
    {
        if (++v7 >= mNumCells)
            return false;
    }
    LightGrid::Cell* v11 = &mCells[v7];
    if (v11 == nullptr)
        return false;
    float base[3] = {
        v11->mBase.x,
        v11->mBase.y,
        v11->mBase.z,
    };
    float v13 = (pos.v.m128_f32[0] - base[0]) / v11->mXGridDelta;
    float v14 = (pos.v.m128_f32[1] - base[1]) / v11->mYGridDelta;
    int v15 = (int)v13;
    int v16 = (int)v14;
    if (v13 < 0.0f)
    {
        v15 = 0;
        v13 = 0.0f;
    }
    if (v15 > v11->mNumXRows - 1)
    {
        v15 = v11->mNumXRows - 1;
        if (v13 >= v11->mNumXRows)
            v13 = (v11->mNumXRows - 1) + 0.999f;
    }
    if (v16 < 0)
    {
        v16 = 0;
        v14 = 0.0f;
    }
    if (v16 > v11->mNumYRows - 1)
    {
        v16 = v11->mNumYRows - 1;
        if (v14 >= v11->mNumYRows)
            v14 = (v11->mNumYRows - 1) + 0.999f;
    }
    float v17 = v13 - v15;
    int v18 = v15 + v11->mFirstGridPoint + v16 * v11->mNumXRows;
    const LightGrid::GridPoint** g = grid;
    g[0] = &toc.mGridPoints[v18];
    g[1] = &toc.mGridPoints[v18 + 1];
    g[2] = &toc.mGridPoints[v18 + v11->mNumXRows];
    float v19 = v14 - v16;
    g[3] = &toc.mGridPoints[v18 + 1 + v11->mNumXRows];
    weights[0] = (1.0f - v17) * (1.0f - v19);
    weights[1] = (1.0f - v19) * v17;
    weights[2] = (1.0f - v17) * v19;
    weights[3] = v19 * v17;
    if (g_LightGridDecruftifier == 0)
        return true;
    float v20 = -1.0f;
    int v21 = 0;
    int v22 = -1;
    int v25 = 0, v26 = 0, v27 = 0, v23 = 0;
    if ((grid[0]->gridpoint & 0x7FFF) == 0x7C1F)
    {
        v21 = 1;
        v25 = 1;
    }
    else
    {
        if (weights[0] > -1.0f)
        {
            v20 = weights[0];
            v22 = 0;
        }
    }
    if ((grid[1]->gridpoint & 0x7FFF) == 0x7C1F)
    {
        v26 = 1;
        ++v21;
    }
    else if (weights[1] > v20)
    {
        v20 = weights[1];
        v22 = 1;
    }
    if ((grid[2]->gridpoint & 0x7FFF) == 0x7C1F)
    {
        v27 = 1;
        ++v21;
    }
    else if (weights[2] > v20)
    {
        v20 = weights[2];
        v22 = 2;
    }
    if ((grid[3]->gridpoint & 0x7FFF) == 0x7C1F)
    {
        v23 = 1;
        ++v21;
    }
    else if (weights[3] > v20)
    {
        v22 = 3;
    }
    if (v21 == 4 || v22 == -1)
        return false;
    if (v21 != 0)
    {
        LightGrid::GridPoint** g = (LightGrid::GridPoint**)grid;
        if (v25 != 0)
            g[0] = g[v22];
        if (v26 != 0)
            g[1] = g[v22];
        if (v27 != 0)
            g[2] = g[v22];
        if (v23 != 0)
            g[3] = g[v22];
    }
    return true;
}

// ea: 0x006BCBB0
void LightGridMgr::GetAmbientColors(const LightGrid::GridPoint** grid,
                                    math::Position3* ambientColors)
{
    unsigned int gridpoint = grid[0]->gridpoint;
    ambientColors[0].v.m128_f32[0] = (gridpoint & 0x1F) * 0.032258064f;
    ambientColors[0].v.m128_f32[1] =
        ((gridpoint >> 5) & 0x1F) * 0.032258064f;
    ambientColors[0].v.m128_f32[2] =
        ((gridpoint >> 10) & 0x1F) * 0.032258064f;
    ambientColors[0].v.m128_f32[3] = 0.0f;
    unsigned int v4 = grid[1]->gridpoint;
    ambientColors[1].v.m128_f32[0] = (v4 & 0x1F) * 0.032258064f;
    ambientColors[1].v.m128_f32[1] = ((v4 >> 5) & 0x1F) * 0.032258064f;
    ambientColors[1].v.m128_f32[2] = ((v4 >> 10) & 0x1F) * 0.032258064f;
    ambientColors[1].v.m128_f32[3] = 0.0f;
    unsigned int v5 = grid[2]->gridpoint;
    ambientColors[2].v.m128_f32[0] = (v5 & 0x1F) * 0.032258064f;
    ambientColors[2].v.m128_f32[1] = ((v5 >> 5) & 0x1F) * 0.032258064f;
    ambientColors[2].v.m128_f32[2] = ((v5 >> 10) & 0x1F) * 0.032258064f;
    ambientColors[2].v.m128_f32[3] = 0.0f;
    unsigned int v6 = grid[3]->gridpoint;
    ambientColors[3].v.m128_f32[0] = (v6 & 0x1F) * 0.032258064f;
    ambientColors[3].v.m128_f32[1] = ((v6 >> 5) & 0x1F) * 0.032258064f;
    ambientColors[3].v.m128_f32[2] = ((v6 >> 10) & 0x1F) * 0.032258064f;
    ambientColors[3].v.m128_f32[3] = 0.0f;
}
