// ============================================================================
// CDL GJK — Gilbert-Johnson-Keerthi collision detection
// Source: source/cdl_gjk.cpp
// ea: 0x81E600-0x81F6B0 (compute_det, gjk, collide, collide_partial, collide_full)
// ============================================================================
// NOTE: GJK requires global data tables (p_0, q_0, y_0, det_, dp, etc.)
// at specific addresses in the .bss section. Full reconstruction deferred.
// ============================================================================

#include "core/math_types.h"
#include "cdl_types.h"
#include <cstring>
#include <stdint.h>

// ============================================================================
// GJK Global State (mirrors .bss layout at 0x10E0BC0-0x10E0DE0)
// These are pre-allocated global arrays used by compute_det() and gjk().
// ============================================================================
static math::Position3 p_0[4];     // support points on convex A (ea: 0x10E0D20)
static math::Position3 q_0[4];     // support points on convex B (ea: 0x10E0CE0)
static math::Position3 y_0[4];     // Minkowski difference points (ea: 0x10E0D60)
static float det_[16][8];          // determinant cache (ea: 0x10E0BC0)
static float dp_[5][4];            // dot product cache (ea: 0x10E0DA0)
static unsigned int cur_mask;      // current simplex mask (ea: 0x10E0CC4)
static unsigned int new_mask;      // new simplex mask (ea: 0x10E0CC8)
static unsigned int w_mask;        // walker mask (ea: 0x10E0CC0)
static unsigned int w_ind;         // walker index (ea: 0x10E0CCC)

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

// ============================================================================
// compute_det — recompute determinant tables from current y_0 points
// ea: 0x81E600
// Stub: full SSE-optimized implementation is ~200 instructions.
// ============================================================================
void compute_det() {
    // Recomputes dp_[][] and det_[][] from current y_0[] simplex points.
    // Full implementation pending global-data table reconstruction.
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
    unsigned int __formal)
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
    return gjk(convexA, aToWorld, convexB, bToWorld, cinfo, 0.0f, false, 20, 0, 0);
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
    int result = gjk(convexA, aToWorld, convexB, bToWorld, cinfo, 0.0f, true, 50, 0, 0);
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
    int result = gjk(convexA, aToWorld, convexB, bToWorld, cinfo, 0.0f, true, maxIter, flags, 0);
    return result == 0;
}
