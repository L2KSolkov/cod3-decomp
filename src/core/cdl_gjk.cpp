// ============================================================================
// CDL GJK — Gilbert-Johnson-Keerthi collision detection
// Source: source/cdl_gjk.cpp
// ea: 0x81E600-0x81F6B0 (compute_det, gjk, collide, collide_partial, collide_full)
// ============================================================================
// NOTE: GJK requires global data tables (p_0, q, y_0, det_, dp, etc.)
// matching the .bss layout recovered from the IDA dump.
// ============================================================================

#include "core/math_types.h"
#include "cdl_types.h"
#include <cmath>
#include <cstring>
#include <stdint.h>

// ============================================================================
// GJK Global State (mirrors .bss layout at 0x10E0BC0-0x10E0DE0)
// These are pre-allocated global arrays used by compute_det() and gjk().
// ============================================================================
static math::Position3 p_0[4];     // support points on convex A (ea: 0x10E0D20)
static math::Position3 q[4];       // support points on convex B (ea: 0x10E0CE0)
static math::Position3 y_0[4];     // Minkowski difference points (ea: 0x10E0D60)
static float det_[16][4];          // determinant cache (ea: 0x10E0BC0)
alignas(16) static float dp_storage[20]; // dp (ea: 0x10E0DA0) plus its 0x10E0DF0 tail
static float (*const dp)[4] = reinterpret_cast<float (*)[4]>(dp_storage);
static unsigned int cur_mask;      // current simplex mask (ea: 0x10E0CC4)
static unsigned int new_mask;      // new simplex mask (ea: 0x10E0CC8)
static unsigned int w_mask;        // walker mask (ea: 0x10E0CC0)
static unsigned int w_ind;         // walker index (ea: 0x10E0CCC)

// IDA names these as separate weak dwords, but the generated data list places
// them inside the determinant and dot-product tables. The original code uses
// scalar SSE loads/stores, so these are float aliases of those exact slots.
static float& dword_10E0C30 = det_[7][0];
static float& dword_10E0C34 = det_[7][1];
static float& dword_10E0C38 = det_[7][2];
static float& dword_10E0C70 = det_[11][0];
static float& dword_10E0C74 = det_[11][1];
static float& dword_10E0C7C = det_[11][3];
static float& dword_10E0C90 = det_[13][0];
static float& dword_10E0C98 = det_[13][2];
static float& dword_10E0C9C = det_[13][3];
static float& dword_10E0CA4 = det_[14][1];
static float& dword_10E0CA8 = det_[14][2];
static float& dword_10E0CAC = det_[14][3];
static float& dword_10E0CB0 = det_[15][0];
static float& dword_10E0CB4 = det_[15][1];
static float& dword_10E0CB8 = det_[15][2];
static float& dword_10E0CBC = det_[15][3];
static float& dword_10E0DA4 = dp_storage[1];
static float& dword_10E0DA8 = dp_storage[2];
static float& dword_10E0DAC = dp_storage[3];
static float& dword_10E0DB0 = dp_storage[4];
static float& dword_10E0DB4 = dp_storage[5];
static float& dword_10E0DB8 = dp_storage[6];
static float& dword_10E0DBC = dp_storage[7];
static float& dword_10E0DC0 = dp_storage[8];
static float& dword_10E0DC4 = dp_storage[9];
static float& dword_10E0DCC = dp_storage[11];
static float& dword_10E0DD0 = dp_storage[12];
static float& dword_10E0DD4 = dp_storage[13];
static float& dword_10E0DD8 = dp_storage[14];

static float dot3(__m128 value) {
    return value.m128_f32[0]
        + (_mm_shuffle_ps(value, value, 85).m128_f32[0]
           + _mm_shuffle_ps(value, value, 170).m128_f32[0]);
}
static float* const det_flat = &det_[0][0];

// Profile counters
struct cdl_proftimer { uint64_t stamp; uint64_t value; };
struct cdl_profcounter { uint64_t value; };
cdl_proftimer cdl_proftimer_closest;
cdl_proftimer cdl_proftimer_support;
cdl_proftimer cdl_proftimer_collide;
cdl_proftimer cdl_proftimer_gjk;
cdl_proftimer cdl_proftimer_push_out_sphere;
cdl_proftimer cdl_proftimer_test1;
cdl_proftimer cdl_proftimer_test2;
cdl_proftimer cdl_proftimer_local_failure;
cdl_proftimer cdl_proftimer_partial_failure;
cdl_proftimer cdl_proftimer_full_failure;
cdl_proftimer cdl_proftimer_make_hull;
cdl_profcounter cdl_profcounter_collide_calls;
cdl_profcounter cdl_profcounter_gjk_separated;
cdl_profcounter cdl_profcounter_gjk_invalid;
cdl_profcounter cdl_profcounter_gjk;

// Tolerance settings
static float abs_error2 = 0.0001f;
static float rel_error2 = 0.01f;
static const float SEP_TRESHOLD2_0 = 1.0f;
static const float PEN_TRESHOLD2_0 = 0.010000001f;

// ============================================================================
// compute_det — recompute determinant tables from current y_0 points
// ea: 0x81E600
// ============================================================================
int compute_det() {
    unsigned int v0 = w_ind;
    unsigned int v1 = 0;
    unsigned int mask = 1;
    math::Position3* v2 = y_0;
    float* v3 = dp_storage + w_ind;

    do {
        if ((mask & cur_mask) != 0) {
            const __m128 product = _mm_mul_ps(v2->v, y_0[v0].v);
            const float value = dot3(product);
            dp[v0][v1] = value;
            *v3 = value;
        }
        ++v2;
        ++v1;
        v3 += 4;
        mask *= 2;
    } while (v1 < 4);

    const unsigned int v5 = w_mask;
    const float v7 = dot3(_mm_mul_ps(y_0[v0].v, y_0[v0].v));
    dp_storage[5 * v0] = v7;
    float* v8 = dp_storage;
    det_[v5][v0] = 1.0f;
    unsigned int v9 = 0;
    unsigned int v10 = 1;
    unsigned int bit = 0;
    unsigned int v22 = 1;
    float* v23 = dp_storage;
    float* v24 = dp_storage;
    float* sj = dp_storage;
    float* v31 = dp_storage + 4 * v0;

    do {
        if ((v10 & cur_mask) != 0) {
            const unsigned int v11 = v10 | w_mask;
            const unsigned int v29 = v9 + 4 * v0;
            const unsigned int s2 = v9 + 4 * (v10 | w_mask);
            det_flat[s2] = v7 - dp_storage[v29];
            const unsigned int v17 = v0 + 4 * v11;
            det_flat[v17] = *v8 - *v31;
            unsigned int v12 = 0;
            unsigned int v13 = 1;
            unsigned int v32 = 1;
            if (v8 > dp_storage) {
                float* v33 = v24;
                float* v26 = dp_storage + 4 * v0;
                float* v14 = dp_storage + 4 * v0;
                float* v28 = dp_storage;
                float* v20 = dp_storage + 4 * v0;
                float* v27 = v23;
                do {
                    if ((v13 & cur_mask) != 0) {
                        const unsigned int v15 = v11 | v13;
                        det_[v15][v12] = ((dp_storage[v29] - *v14) * det_flat[v17])
                            + ((*sj - *v33) * det_flat[s2]);
                        det_[v15][bit] = ((*v14 - dp_storage[v29]) * det_[(v32 | w_mask)][v0])
                            + ((*v28 - *v27) * det_[(v32 | w_mask)][v12]);
                        v14 = v20;
                        v0 = w_ind;
                        det_[v15][w_ind] = ((*v28 - *v26) * det_[(v22 | v32)][v12])
                            + ((*v33 - *v31) * det_[(v22 | v32)][bit]);
                        v13 = v32;
                    }
                    v28 += 5;
                    v27 += 4;
                    v26 += 4;
                    ++v33;
                    ++v12;
                    ++v14;
                    v13 *= 2;
                    v20 = v14;
                    v32 = v13;
                } while (v12 < bit);
                v9 = bit;
                v8 = sj;
            }
        }
        v24 += 4;
        v31 += 4;
        ++v23;
        v8 += 5;
        ++v9;
        v10 = 2 * v22;
        bit = v9;
        sj = v8;
        v22 *= 2;
    } while (v8 < dp_storage + 20);

    if (new_mask == 15) {
        dword_10E0CB0 = ((dword_10E0DD4 - dword_10E0DD0) * dword_10E0CAC)
            + ((dword_10E0DC4 - dword_10E0DC0) * dword_10E0CA8)
            + ((dword_10E0DB4 - dword_10E0DB0) * dword_10E0CA4);
        dword_10E0CB4 = ((dp_storage[0] - dword_10E0DA4) * dword_10E0C90)
            + ((dword_10E0DD0 - dword_10E0DD4) * dword_10E0C9C)
            + ((dword_10E0DC0 - dword_10E0DC4) * dword_10E0C98);
        dword_10E0CB8 = ((dword_10E0DD0 - dword_10E0DD8) * dword_10E0C7C)
            + ((dp_storage[0] - dword_10E0DA8) * dword_10E0C70)
            + ((dword_10E0DB0 - dword_10E0DB8) * dword_10E0C74);
        dword_10E0CBC = ((dword_10E0DC0 - dword_10E0DCC) * dword_10E0C38)
            + ((dword_10E0DB0 - dword_10E0DBC) * dword_10E0C34)
            + ((dp_storage[0] - dword_10E0DAC) * dword_10E0C30);
    }
    return 16;
}

// ============================================================================
// gjk — GJK collision detection solver
// ea: 0x81EA70
// Returns: 0 = colliding, 1 = separated (within error), 2 = failed/separated
// ============================================================================
int gjk(
    const cdlConvex& convexA, const math::Mat43& aToWorld,
    const cdlConvex& convexB, const math::Mat43& bToWorld,
    cdl_cinfo2& cinfo,
    float tresh,
    bool full,
    unsigned int maxIter,
    unsigned int flags,
    unsigned int __formal,
    unsigned int __formal2)
{
    // Full implementation at ea:0x81EA70 (~700 instructions of SSE-optimized GJK)
    // Requires support-mapping virtual calls on cdlConvex and determinant tables.
    // Stub: report collision failure (not colliding).
    cinfo.pa.v = _mm_setzero_ps();
    cinfo.pb.v = _mm_setzero_ps();
    cinfo.ni.v = _mm_setzero_ps();
    return 2; // failed
}

// ============================================================================
// collide_partial — check collision, return early if separated
// ea: 0x81F2F0
// ============================================================================
int collide_partial(
    const cdlConvex& convexA, const math::Mat43& aToWorld,
    const cdlConvex& convexB, const math::Mat43& bToWorld,
    cdl_cinfo2& cinfo)
{
    const int result = gjk(convexA, aToWorld, convexB, bToWorld, cinfo,
                           SEP_TRESHOLD2_0, false, 0, 0, 0, 0);
    if (result == 1) {
        const __m128 squared = _mm_mul_ps(cinfo.ni.v, cinfo.ni.v);
        const float length2 = dot3(squared);
        if (length2 <= PEN_TRESHOLD2_0)
            return 2;

        const float length = std::sqrt(length2);
        cinfo.ni.v = _mm_div_ps(cinfo.ni.v, _mm_set1_ps(length));
        const __m128 half_normal = _mm_mul_ps(cinfo.ni.v, _mm_set1_ps(0.5f));
        cinfo.pa.v = _mm_sub_ps(cinfo.pa.v, half_normal);
        cinfo.pb.v = _mm_add_ps(cinfo.pb.v, half_normal);
    }
    return result;
}

// ============================================================================
// collide_full — check collision, compute full penetration info
// ea: 0x81F3D0
// ============================================================================
bool collide_full(
    const cdlConvex& convexA, const math::Mat43& aToWorld,
    const cdlConvex& convexB, const math::Mat43& bToWorld,
    cdl_cinfo2& cinfo)
{
    int result = gjk(convexA, aToWorld, convexB, bToWorld, cinfo, 0.0f, true, 0, 0, 0, 0);
    return result == 0;  // 0 = colliding
}

// ============================================================================
// collide — full collision check with configurable parameters
// ea: 0x81F6B0
// ============================================================================
bool collide(
    const cdlConvex& convexA, const math::Mat43& aToWorld,
    const cdlConvex& convexB, const math::Mat43& bToWorld,
    cdl_cinfo2& cinfo,
    unsigned int maxIter,
    unsigned int flags,
    unsigned int absTresh,
    unsigned int relTresh)
{
    int result = gjk(convexA, aToWorld, convexB, bToWorld, cinfo, 0.0f, true, maxIter, flags, absTresh, relTresh);
    return result == 0;
}
