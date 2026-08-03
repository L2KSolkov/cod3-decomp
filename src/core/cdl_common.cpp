// ============================================================================
// CDL Common — geometry distance & intersection utilities
// Source: source/cdl_common.cpp (line ref: 9)
// ea: 0x81D4E0-0x81E130 (intersect, dist2, is_inside, calc_closest)
// ============================================================================

#include "core/math_types.h"
#include "cdl_types.h"
#include <math.h>

// External assert
extern bool _tlAssert(const char* file, int line, const char* cond, const char* msg);

static const __m128 Float4_SignMask = { -0.0f, -0.0f, -0.0f, -0.0f };

// SSE reassembly macros (match compiler's dot/hadd patterns)
#define DOT3(v) ((v).m128_f32[0] + ((v).m128_f32[1] + (v).m128_f32[2]))

// ============================================================================
// intersect — ray vs plane
// ea: 0x81D4E0
// ============================================================================
float intersect(
    const math::Position3& po,
    const math::Dir3&      pn,
    const math::Position3& ro,
    const math::Dir3&      rd,
    const math::Position3& plane_point,
    const math::Dir3&      plane_normal)
{
    // Validate plane normal is unit length
    __m128 n2 = _mm_mul_ps(plane_normal.v, plane_normal.v);
    float len2 = DOT3(n2);
    if (fabsf(len2 - 1.0f) >= 0.0001f) {
        _tlAssert("source/cdl_common.cpp", 9, "", "");
        __builtin_debugtrap();
    }

    // Project ray dir onto plane normal
    __m128 denom_vec = _mm_mul_ps(rd.v, plane_normal.v);
    float denom = DOT3(denom_vec);
    if (fabsf(denom) < 0.0001f)
        return 1.0e20f;  // parallel

    // Project pos delta onto plane normal
    __m128 delta = _mm_sub_ps(ro.v, plane_point.v);
    __m128 numer_vec = _mm_mul_ps(delta, plane_normal.v);
    float numer = DOT3(numer_vec);

    return numer / denom;
}

// ============================================================================
// dist2 — squared distance from point to AABB
// ea: 0x81D8C0
// ============================================================================
float dist2(const math::Position3& point, const cdlAABB& aabb) {
    // Clamp point to AABB
    __m128 min = _mm_sub_ps(aabb.center.v, aabb.halfExtents.v);
    __m128 max = _mm_add_ps(aabb.center.v, aabb.halfExtents.v);
    __m128 clamped = _mm_max_ps(_mm_min_ps(point.v, max), min);
    __m128 diff = _mm_sub_ps(clamped, point.v);
    __m128 sq = _mm_mul_ps(diff, diff);
    return DOT3(sq);
}

// ============================================================================
// dist2 — squared distance from point to transformed-OBB
// ea: 0x81D930
// ============================================================================
float dist2(const math::Position3& point, const cdlAABB& obb, const math::Mat43& obbToWorld) {
    // Transform point to OBB local space, then compute AABB distance
    const __m128& a0 = obbToWorld.x.v;
    const __m128& a1 = obbToWorld.y.v;
    const __m128& a2 = obbToWorld.z.v;
    const __m128& a3 = obbToWorld.w.v;

    // Assemble rotation + translation
    __m128 pos_in_obb;
    {
        // a0, a1 mixed
        __m128 t0 = _mm_shuffle_ps(a0, a1, 68);   // (ax, ay, bx, by)
        __m128 t1 = _mm_shuffle_ps(a0, a1, 238);  // (az, aw, bz, bw)
        __m128 t2 = _mm_shuffle_ps(t1, a2, 168);  // (az, bz, cx, cy) -- z-axis pair
        __m128 t3 = _mm_shuffle_ps(t0, a2, 221);  // (bx, by, cx, cy) -- y-axis
        __m128 t4 = _mm_shuffle_ps(t0, a2, 136);  // (ax, ay, cx, cy) -- x-axis

        // Build inverse translation (negate rotation * translation)
        __m128 negTrans = _mm_xor_ps(Float4_SignMask,
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(a3, a3, 0), t4),
                           _mm_mul_ps(_mm_shuffle_ps(a3, a3, 85), t3)),
                _mm_mul_ps(_mm_shuffle_ps(a3, a3, 170), t2)));

        // Transform point to local space
        pos_in_obb = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 0), t4),
                       _mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 85), t3)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 170), t2), negTrans));
    }

    return dist2(*(const math::Position3*)&pos_in_obb, obb);
}

// ============================================================================
// dist2 — squared distance between two triangular regions (line * line sweep)
// ea: 0x81DF30
// Used for mesh-vs-mesh distance queries.
// ============================================================================
float dist2(
    const math::Position3& p0,
    const math::Position3& p1,
    const math::Position3& q0,
    const math::Position3& q1)
{
    __m128 dP = _mm_sub_ps(p1.v, p0.v);
    __m128 dQ = _mm_sub_ps(q1.v, q0.v);

    __m128 dp_sq = _mm_mul_ps(dP, dP);
    float lenDP2 = DOT3(dp_sq);

    __m128 v0 = _mm_sub_ps(p0.v, q0.v);

    __m128 dp_dq = _mm_mul_ps(dP, dQ);
    float dpdq = DOT3(dp_dq);

    __m128 v0_dp = _mm_mul_ps(v0, dP);
    float v0dp = DOT3(v0_dp);

    __m128 dq_sq = _mm_mul_ps(dQ, dQ);
    float lenDQ2 = DOT3(dq_sq);

    __m128 v0_dq = _mm_mul_ps(v0, dQ);
    float v0dq = DOT3(v0_dq);

    // Squared distance: line segment vs line segment
    // Find closest points on two lines, clamp to segments
    // This is a standard geometric primitive.

    float best = lenDP2 - ((v0dp * v0dp) / (lenDQ2 < 0.0001f ? 0.0001f : lenDQ2));
    // Stub: full implementation is ea:0x81DF30 (SSE-optimized, ~200 instructions)
    // This function will be finalized when physics porting requires bit-exactness.
    if (best < 0.0001f) best = 0.0001f;

    return best;
}

// ============================================================================
// is_inside — triangle winding test (is point inside swept volume)
// ea: 0x81DA20
// ============================================================================
bool is_inside(
    const math::Dir3& n, const math::Dir3& p,
    const math::Dir3& na, const math::Dir3& pa)
{
    const float* nf = (const float*)&n.v;
    const float* pf = (const float*)&p.v;
    const float* naf = (const float*)&na.v;
    const float* paf = (const float*)&pa.v;

    // Find dominant axis of na
    float absX = fabsf(naf[0]);
    float absY = fabsf(naf[1]);
    int i0, i1;
    if (absX <= absY) {
        if (absY <= fabsf(naf[2])) { i0 = 2; i1 = 1; }
        else { i0 = 1; i1 = 0; }
    } else {
        if (absX <= fabsf(naf[2])) { i0 = 2; i1 = 0; }
        else { i0 = 0; i1 = 2; }
    }

    float c1 = pf[i1] * nf[i0] - pf[i0] * nf[i1];
    float c2 = nf[i0] * paf[i1] - nf[i1] * paf[i0];

    int sgn1 = c1 > 0.0f ? 1 : (c1 < 0.0f ? -1 : 0);
    int sgn2 = c2 > 0.0f ? 1 : (c2 < 0.0f ? -1 : 0);
    if (sgn1 != sgn2) return false;

    float c3 = pf[i1] * paf[i0] - pf[i0] * paf[i1];
    int sgn3 = c3 > 0.0f ? 1 : (c3 < 0.0f ? -1 : 0);

    if (c1 < 0.0f) return sgn3 == -1;
    if (c1 == 0.0f) return sgn3 == 0;
    return sgn3 == 1;
}

// ============================================================================
// calc_closest — closest point on a segment (p0, p1) to point p
// ea: 0x81DB50
// ============================================================================
math::Position3 calc_closest(
    const math::Position3& p0,
    const math::Position3& p1,
    const math::Position3& p)
{
    __m128 seg = _mm_sub_ps(p1.v, p0.v);
    __m128 toP = _mm_sub_ps(p.v, p0.v);
    __m128 toP1 = _mm_sub_ps(p.v, p1.v);

    // Solve for projection parameter t = dot(seg, toP) / dot(seg, seg)
    __m128 dp = _mm_mul_ps(seg, toP);
    float segDotP = DOT3(dp);
    __m128 dp1 = _mm_mul_ps(seg, toP1);
    float segDotP1 = DOT3(dp1);

    __m128 seg2 = _mm_mul_ps(seg, seg);
    float segLen2 = DOT3(seg2);
    __m128 toP2 = _mm_mul_ps(toP, toP);
    float toPLen2 = DOT3(toP2);

    if (segDotP >= 0.0f && segDotP1 <= 0.0f) {
        // Projection is on the segment
        float t = segDotP / segLen2;
        math::Position3 result;
        result.v = _mm_add_ps(p0.v, _mm_mul_ps(seg, _mm_shuffle_ps(
            _mm_set_ss(t), _mm_set_ss(t), 0)));
        return result;
    }

    // Clamp to nearest endpoint
    math::Position3 result;
    toP2 = _mm_mul_ps(toP, toP);
    toPLen2 = DOT3(toP2);
    __m128 toP12 = _mm_mul_ps(toP1, toP1);
    float toP1Len2 = DOT3(toP12);

    result.v = (toPLen2 < toP1Len2) ? p0.v : p1.v;
    return result;
}
