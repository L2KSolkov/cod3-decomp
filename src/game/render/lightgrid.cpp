// ============================================================================
// lightgrid.cpp - render.o LightGridMgr (LightGrid.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "core/ae_array.h"

#include <math.h>

namespace LightGrid {
struct GridPoint {
    unsigned int gridpoint;  // +0x00
};

// LightGrid::Light (IDA type; size 0x1C)
struct Light {
    math::Vector4::Packed mPosition;   // +0x00 (16)
    math::Position3::Packed mColor;    // +0x10 (12)
};
static_assert(sizeof(Light) == 0x1C, "LightGrid::Light size mismatch");

// LightGrid::LightIndex (IDA type; size 4)
struct LightIndex {
    unsigned short mIndex;          // +0x00
    unsigned short mAttenuationInt; // +0x02
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
    Light* mLights;        // +0x00
    int mNumLights;        // +0x04
    GridPoint* mGridPoints;  // +0x08
    int mNumGridPoints;    // +0x0C
    LightIndex* mLightIndices;  // +0x10
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
    void GetLightListForGrid(
        const LightGrid::TOC& toc, const LightGrid::GridPoint** grid,
        float* weights,
        ae_sized_array<LightGrid::LightIndex, 12>& lights);  // ?GetLightListForGrid@LightGridMgr@@AAEXABUTOC@LightGrid@@PAPBUGridPoint@3@PAMAAV?$ae_sized_array@ULightIndex@LightGrid@@$0M@@@@Z
public:
    void SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                         const math::Position3& pos,
                         LightGridData* pLG);  // ?SampleLightGrid@LightGridMgr@@QAEXABUTOC@LightGrid@@HABVPosition3@math@@PAVLightGridData@@@Z
    void SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                         const math::Position3& pos,
                         math::Mat44* dir, math::Mat44* color);  // ?SampleLightGrid@LightGridMgr@@QAEXABUTOC@LightGrid@@HABVPosition3@math@@PAVMat44@5@2@Z
};

extern int g_LightGridDecruftifier;  // ?g_LightGridDecruftifier@@3HA (g.o)
float g_pointmultiplier = 2.0f;      // ?g_pointmultiplier@@3MA (render.o data)
bool g_pointmultiplieractive = false; // ?g_pointmultiplieractive@@3_NA (render.o data)

void nglListAddDirLight(unsigned int LightCat, const math::Dir3& Dir,
                        const math::Vector4& Color);  // ?nglListAddDirLight@@YAXIABVDir3@math@@ABVVector4@2@@Z
void nglSetAmbientLight(float r, float g, float b);  // ?nglSetAmbientLight@@YAXMMM@Z
extern int g_lightGridBlueErrors;  // ?g_lightGridBlueErrors@@3HA (g.o)

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

// ============================================================================
// GetLightListForGrid - ea: 0x006C9530
// ============================================================================
void LightGridMgr::GetLightListForGrid(
    const LightGrid::TOC& toc, const LightGrid::GridPoint** grid,
    float* weights,
    ae_sized_array<LightGrid::LightIndex, 12>& lights)
{
    int v12 = 0;
    do
    {
        int count = ((*grid)->gridpoint >> 15) & 3;
        int i = 0;
        if (count != 0)
        {
            do
            {
                LightGrid::LightIndex* v6 =
                    &toc.mLightIndices[i + ((*grid)->gridpoint >> 17)];
                if (v6->mIndex >= (unsigned int)toc.mNumLights)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\LightGridMgr.cpp";
                    AeAssert::gCurrentLine = 335;
                    AeAssert::gCurrentExpr = "lightIndex.mIndex < toc.mNumLights";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Light Index Overflow"))
                        __debugbreak();
                }
                unsigned int v7 = 0;
                if (lights.m_size > 0)
                {
                    while (1)
                    {
                        if (v7 >= 12)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                            AeAssert::gCurrentLine = 154;
                            AeAssert::gCurrentExpr =
                                "idx >= 0 && idx < _CAPACITY";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("out of bounds"))
                                __debugbreak();
                        }
                        if (lights.m_elements[v7].mIndex == v6->mIndex)
                            break;
                        if (++v7 >= (unsigned int)lights.m_size)
                            goto add_new;
                    }
                    lights.m_elements[v7].mAttenuationInt =
                        (unsigned short)(v6->mAttenuationInt * weights[v12]
                                         + lights.m_elements[v7].mAttenuationInt);
                    goto next_i;
                }
add_new:
                {
                    int m_size = lights.m_size;
                    if (v7 == (unsigned int)m_size)
                    {
                        if (m_size >= 12)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                            AeAssert::gCurrentLine = 192;
                            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("no room left in array"))
                                __debugbreak();
                        }
                        int v9 = lights.m_size + 1;
                        lights.m_size = v9;
                        lights.m_elements[v9 - 1].mIndex = v6->mIndex;
                        lights.m_elements[lights.m_size - 1].mAttenuationInt =
                            (unsigned short)(v6->mAttenuationInt * weights[v12]);
                    }
                }
next_i:
                ++i;
            } while (i < count);
        }
        ++v12;
        ++grid;
    } while (v12 * 4 + 4 < 16);
}

// ============================================================================
// SampleLightGrid (LightGridData) - ea: 0x006C9790
// ============================================================================
void LightGridMgr::SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                                   const math::Position3& pos,
                                   LightGridData* pLG)
{
    if (toc.mNumCells >= 10000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\LightGridMgr.cpp";
        AeAssert::gCurrentLine = 367;
        AeAssert::gCurrentExpr = "toc.mNumCells<10000";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("*** LightGrid using old Data Format ***"))
            __debugbreak();
    }

    LightGridData local;
    LightGridData* v8 = pLG;
    if (pLG == nullptr)
    {
        pLG = &local;
        v8 = &local;
    }

    const LightGrid::GridPoint* grid[4];
    float weights[4];
    if (GetGridInfo(toc, cellidx, pos, grid, weights))
    {
        math::Position3 ambient[4];
        GetAmbientColors(grid, ambient);
        __m128 w0 = _mm_loadu_ps(weights);
        __m128 v9 = _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(ambient[0].v, _mm_shuffle_ps(w0, w0, 0)),
                    _mm_mul_ps(ambient[1].v, _mm_shuffle_ps(w0, w0, 85))),
                _mm_mul_ps(ambient[2].v, _mm_shuffle_ps(w0, w0, 170))),
            _mm_mul_ps(ambient[3].v, _mm_shuffle_ps(w0, w0, 255)));
        v8->m_ambientColor.x = v9.m128_f32[0];
        v8->m_ambientColor.y = _mm_shuffle_ps(v9, v9, 85).m128_f32[0];
        v8->m_ambientColor.z = _mm_shuffle_ps(v9, v9, 170).m128_f32[0];
        v8->m_numDirectional = 0;

        ae_sized_array<LightGrid::LightIndex, 12> lights;
        lights.m_size = 0;
        GetLightListForGrid(toc, grid, weights, lights);

        for (int li = 0; li < 3 && li < lights.m_size; ++li)
        {
            int bestIdx = -1;
            unsigned int bestAtten = 0;
            for (unsigned int k = 0; k < (unsigned int)lights.m_size; ++k)
            {
                if (lights.m_elements[k].mAttenuationInt > bestAtten)
                {
                    bestAtten = lights.m_elements[k].mAttenuationInt;
                    bestIdx = (int)k;
                }
            }
            if (bestIdx < 0)
                break;
            lights.m_elements[bestIdx].mAttenuationInt = 0;

            if (v8->m_numDirectional >= 3)
            {
                AeAssert::gCurrentAuthor = AeAssert::ARO;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\LightGridMgr.cpp";
                AeAssert::gCurrentLine = 421;
                AeAssert::gCurrentExpr = "pLG->m_numDirectional < 3";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("index out of range"))
                    __debugbreak();
            }

            LightGrid::Light* light = &toc.mLights[lights.m_elements[bestIdx].mIndex];
            float multiplier = 1.0f;
            if (light->mPosition.w != 0.0f && g_pointmultiplieractive)
                multiplier = g_pointmultiplier;
            float scale = (float)bestAtten / (65535.0f / multiplier);

            math::Vector4::Packed* p_x =
                &v8->m_directionalColor[v8->m_numDirectional];
            math::Dir3::Packed* v18 = &v8->m_directionalDir[v8->m_numDirectional];
            p_x->x = light->mColor.x * scale;
            p_x->y = light->mColor.y * scale;
            p_x->z = light->mColor.z * scale;
            p_x->w = scale;

            math::Vector4 colorVec;
            colorVec.v = _mm_setr_ps(p_x->x, p_x->y, p_x->z, scale);
            if (light->mPosition.w == 0.0f)
            {
                math::Dir3 dir;
                dir.v = _mm_setr_ps(light->mPosition.x, light->mPosition.y,
                                    light->mPosition.z, 0.0f);
                nglListAddDirLight(0xFFFFFFFFu, dir, colorVec);
                v18->x = dir.v.m128_f32[0];
                v18->y = dir.v.m128_f32[1];
                v18->z = dir.v.m128_f32[2];
            }
            else
            {
                math::Position3 lightPos;
                lightPos.v = _mm_setr_ps(light->mPosition.x, light->mPosition.y,
                                         light->mPosition.z, 0.0f);
                __m128 v21 = _mm_sub_ps(pos.v, lightPos.v);
                float distSq = v21.m128_f32[0] + (v21.m128_f32[1] + v21.m128_f32[2]);
                float dist = sqrtf(distSq);
                math::Dir3 dir;
                dir.v = _mm_div_ps(v21, _mm_set1_ps(dist));
                nglListAddDirLight(0xFFFFFFFFu, dir, colorVec);
                v18->x = dir.v.m128_f32[0];
                v18->y = dir.v.m128_f32[1];
                v18->z = dir.v.m128_f32[2];
            }
            ++v8->m_numDirectional;
        }
        nglSetAmbientLight(v8->m_ambientColor.x, v8->m_ambientColor.y,
                           v8->m_ambientColor.z);
    }
    else
    {
        if (g_lightGridBlueErrors != 0)
            nglSetAmbientLight(0.0f, 0.0f, 1.0f);
        else
            nglSetAmbientLight(0.80000001f, 0.80000001f, 0.80000001f);
        if (pLG != nullptr)
        {
            pLG->m_ambientColor.x = 0.0f;
            pLG->m_ambientColor.y = 0.0f;
            pLG->m_ambientColor.z = 0.0f;
            pLG->m_numDirectional = 0;
        }
    }
}

// ============================================================================
// SampleLightGrid (Mat44 dir/color) - ea: 0x006C9E20
// ============================================================================
void LightGridMgr::SampleLightGrid(const LightGrid::TOC& toc, int cellidx,
                                   const math::Position3& pos,
                                   math::Mat44* dir, math::Mat44* color)
{
    if (toc.mNumCells >= 10000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\LightGridMgr.cpp";
        AeAssert::gCurrentLine = 479;
        AeAssert::gCurrentExpr = "toc.mNumCells<10000";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("*** LightGrid using old Data Format ***"))
            __debugbreak();
    }

    const LightGrid::GridPoint* grid[4];
    float weights[4];
    if (!GetGridInfo(toc, cellidx, pos, grid, weights))
        return;

    math::Position3 ambient[4];
    GetAmbientColors(grid, ambient);
    __m128 w0 = _mm_loadu_ps(weights);
    __m128 v9 = _mm_add_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(ambient[0].v, _mm_shuffle_ps(w0, w0, 0)),
                _mm_mul_ps(ambient[1].v, _mm_shuffle_ps(w0, w0, 85))),
            _mm_mul_ps(ambient[2].v, _mm_shuffle_ps(w0, w0, 170))),
        _mm_mul_ps(ambient[3].v, _mm_shuffle_ps(w0, w0, 255)));
    color->w.v = _mm_shuffle_ps(v9,
                                _mm_shuffle_ps(_mm_set1_ps(1.0f), v9, 160), 52);

    ae_sized_array<LightGrid::LightIndex, 12> lights;
    lights.m_size = 0;
    GetLightListForGrid(toc, grid, weights, lights);

    int v11 = 0;
    for (int li = 0; li < 3 && li < lights.m_size; ++li)
    {
        int bestIdx = -1;
        unsigned int bestAtten = 0;
        for (unsigned int k = 0; k < (unsigned int)lights.m_size; ++k)
        {
            if (lights.m_elements[k].mAttenuationInt > bestAtten)
            {
                bestAtten = lights.m_elements[k].mAttenuationInt;
                bestIdx = (int)k;
            }
        }
        if (bestIdx < 0)
            break;
        lights.m_elements[bestIdx].mAttenuationInt = 0;

        LightGrid::Light* light = &toc.mLights[lights.m_elements[bestIdx].mIndex];
        if (light->mPosition.w == 0.0f)
        {
            dir->x.v = _mm_setr_ps(light->mPosition.x, light->mPosition.y,
                                   light->mPosition.z, 0.0f);
        }
        else
        {
            __m128 lightPos = _mm_setr_ps(light->mPosition.x,
                                          light->mPosition.y,
                                          light->mPosition.z, 0.0f);
            __m128 v20 = _mm_sub_ps(pos.v, lightPos);
            float distSq = v20.m128_f32[0] + (v20.m128_f32[1] + v20.m128_f32[2]);
            __m128 v23 = _mm_div_ps(v20, _mm_set1_ps(sqrtf(distSq)));
            dir->x.v = _mm_shuffle_ps(v23,
                                      _mm_shuffle_ps(_mm_setzero_ps(), v23, 160), 52);
        }

        float scale = (float)bestAtten * 0.000015259022f;
        __m128 colv = _mm_mul_ps(
            _mm_setr_ps(light->mColor.x, light->mColor.y, light->mColor.z, 0.0f),
            _mm_set1_ps(scale));
        color->x.v = _mm_shuffle_ps(colv,
                                    _mm_shuffle_ps(_mm_set1_ps(1.0f), colv, 160), 52);
        ++v11;
        dir = (math::Mat44*)((char*)dir + 16);
        color = (math::Mat44*)((char*)color + 16);
    }
    if (v11 < 3)
    {
        for (int i = v11; i < 3; ++i)
        {
            dir->x.v = _mm_setzero_ps();
            color->x.v = _mm_setzero_ps();
            dir = (math::Mat44*)((char*)dir + 16);
            color = (math::Mat44*)((char*)color + 16);
        }
    }
}
