// ============================================================================
// tr_fx5.cpp - render.o particle proximity update
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "aeps/apsGroup.h"
#include "aeps/apsEffect.h"

#include <math.h>

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

// ParticleEffect::RaycastData (IDA layout; struct U-tag)
class ParticleEffect {
public:
    struct RaycastData {
        apsBounds mBounds;              // +0x00
        proximity_data_t* mProximityData;  // +0x20
    };
};

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
