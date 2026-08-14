// ============================================================================
// tr_fx5.cpp - render.o particle proximity update
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsGroup.h"
#include "aeps/apsEffect.h"

#include <math.h>

// AeAssert / _tlAssert
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // ?_tlAssert@@YA_NPBDH00@Z

extern int calc_cell_index(const math::Position3& cur_pos,
                           float (&cached_pos)[4],
                           int& cached_index);  // ?calc_cell_index@@YAHABVPosition3@math@@AAY03MAAH@Z (tr_bsp2.cpp)
extern int g_camera_cell;  // render.o @ 0x11E993C (defined in pakmanager.cpp)
struct proximity_data_t;

// portals (render.o @ 0x11EA840; defined in pakmanager.cpp)
struct PortalCellView {
    uint8_t _pad[320];
    math::Vector4** m_planes;   // +320 (inner m_slot_array)
    int m_alloc_count;          // +324
};
struct PortalsView {
    uint8_t m_buffer[64 * 336];
    PortalCellView* m_slot_array;  // +21504
    int m_alloc_count;             // +21508
};
extern PortalsView portals;  // ?portals@@3V?$phys_static_array@V?$phys_static_array@VVector4@math@@$0BE@@@$0EA@@@A

// ParticleEffect view (mEffect +0x1C, cached_pos +0x00, cached_cell_index +0x10)
class ParticleEffect {
public:
    struct RaycastData {
        apsBounds mBounds;              // +0x00
        proximity_data_t* mProximityData;  // +0x20
    };

    float cached_pos[4];       // +0x00
    int cached_cell_index;     // +0x10
    int culled;                // +0x14
    uint8_t _pad[0x1C - 0x18];
    apsEffect* mEffect;        // +0x1C
};

// ============================================================================
// calc_cull_status - ea: 0x006C8300
// ============================================================================
int calc_cull_status(ParticleEffect* effect)
{
    apsBounds bounds;
    bounds.Init();
    effect->mEffect->GetBounds(bounds);

    math::Position3 center;
    center.v = _mm_mul_ps(
        _mm_add_ps(bounds.mMax.v, bounds.mMin.v),
        _mm_set1_ps(0.5f));
    __m128 v2 = _mm_sub_ps(bounds.mMax.v, bounds.mMin.v);
    __m128 v3 = _mm_mul_ps(v2, v2);
    float radius = sqrtf(v3.m128_f32[0]
                       + (_mm_shuffle_ps(v3, v3, 85).m128_f32[0]
                          + _mm_shuffle_ps(v3, v3, 170).m128_f32[0])) * 0.5f;

    int cell = calc_cell_index(center, effect->cached_pos,
                               effect->cached_cell_index);
    if (cell == g_camera_cell)
        return 0;
    if (cell >= 0)
    {
        int totalCells = portals.m_alloc_count;
        if (totalCells == 0)
            return 1;
        int cellIdx = 0;
        for (; cellIdx < totalCells; ++cellIdx)
        {
            PortalCellView* inner = &portals.m_slot_array[cellIdx];
            int planeCount = inner->m_alloc_count;
            int j = 0;
            for (; j < planeCount; ++j)
            {
                if (j < 0 || j >= planeCount)
                {
                    if (_tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            114, "i >= 0 && i < m_alloc_count", "unknown"))
                        __debugbreak();
                }
                __m128 plane = inner->m_planes[j]->v;
                float planeW = _mm_shuffle_ps(plane, plane, 255).m128_f32[0];
                __m128 v12 = _mm_mul_ps(plane, center.v);
                float dot = v12.m128_f32[0]
                          + (_mm_shuffle_ps(v12, v12, 85).m128_f32[0]
                             + _mm_shuffle_ps(v12, v12, 170).m128_f32[0]);
                if (-radius > (dot - planeW))
                    break;
            }
            if (j == planeCount)
                break;  // visible through this portal
        }
        if (cellIdx >= totalCells)
            return 1;
    }
    return 0;
}

// proximity_data_t (game_types.h)
struct proximity_data_t;
void query_proximity_data(const math::Position3& lo,
                          const math::Position3& hi,
                          proximity_data_t& out);  // ?query_proximity_data@@YAXABVPosition3@math@@0AAUproximity_data_t@@@Z

// apsBounds::ClampHalfSize (apsMath.o)
static void ClampHalfSize(apsBounds* b, float maxHalfX, float maxHalfY,
                          float maxHalfZ)
{
    __m128 half = _mm_setr_ps(maxHalfX, maxHalfY, maxHalfZ, 0.0f);
    b->mMin.v = _mm_max_ps(b->mMin.v, _mm_mul_ps(half, _mm_set1_ps(-1.0f)));
    b->mMax.v = _mm_min_ps(b->mMax.v, half);
}

// ============================================================================
// ProximityUpdate - ea: 0x006C8180
// ============================================================================
void ProximityUpdate(ParticleEffect::RaycastData& raycastData,
                     const apsEffect::CollisionData& colData)
{
    apsBounds mBounds = colData.mBounds;
    ClampHalfSize(&mBounds, 400.0f, 400.0f, 400.0f);

    __m128 cmp = _mm_cmplt_ps(
        _mm_max_ps(
            _mm_sub_ps(raycastData.mBounds.mMin.v, mBounds.mMin.v),
            _mm_sub_ps(mBounds.mMax.v, raycastData.mBounds.mMax.v)),
        _mm_setzero_ps());
    if ((_mm_movemask_ps(cmp) & 7) != 7)
    {
        raycastData.mBounds = mBounds;
        float radius = mBounds.Radius() * 0.5f;
        __m128 grow = _mm_set1_ps(radius);
        raycastData.mBounds.mMin.v = _mm_sub_ps(raycastData.mBounds.mMin.v, grow);
        raycastData.mBounds.mMax.v = _mm_add_ps(raycastData.mBounds.mMax.v, grow);

        math::Position3 lo;
        lo.v = raycastData.mBounds.mMin.v;
        math::Position3 hi;
        hi.v = raycastData.mBounds.mMax.v;
        query_proximity_data(lo, hi, *raycastData.mProximityData);
    }
}
