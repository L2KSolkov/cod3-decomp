// ============================================================================
// CDL Common — geometry distance & intersection utilities
// Source: source/cdl_common.cpp (line ref: 9)
// ea: 0x81D4E0-0x81E130 (intersect, dist2, is_inside, calc_closest)
// ============================================================================

#include "core/math_types.h"
#include "cdl_types.h"
#include <math.h>
#include <stdint.h>

// CDL profiler globals (cdl_xboxr:cdl_common.o) - debug timing data
struct cdl_proftimer {
    uint64_t stamp;
    uint64_t value;
    void start();
    void stop();
};
struct cdl_profcounter {
    uint64_t value;
};
// ?cdl_proftimer_update_rb@@3Ucdl_proftimer@@A (game.o data @ 0x01334968)
cdl_proftimer cdl_proftimer_update_rb;
cdl_proftimer cdl_proftimer_vmcalls;              // ?cdl_proftimer_vmcalls@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_cl_msgs;              // ?cdl_proftimer_cl_msgs@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_temp2;                // ?cdl_proftimer_temp2@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_fx_all;               // ?cdl_proftimer_fx_all@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_fx_update;            // ?cdl_proftimer_fx_update@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_fx_render;            // ?cdl_proftimer_fx_render@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_cvar;                 // ?cdl_proftimer_cvar@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_dobj_anim;            // ?cdl_proftimer_dobj_anim@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_ent_actors;           // ?cdl_proftimer_ent_actors@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_proxy_queries;        // ?cdl_proftimer_proxy_queries@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_sight_trace_point;    // ?cdl_proftimer_sight_trace_point@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_sight_trace_sphere;   // ?cdl_proftimer_sight_trace_sphere@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_temp0;                // ?cdl_proftimer_temp0@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_trace_point_list;     // ?cdl_proftimer_trace_point_list@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_trace_sphere_list;    // ?cdl_proftimer_trace_sphere_list@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_segment_patch;        // ?cdl_proftimer_segment_patch@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_segment_brush;        // ?cdl_proftimer_segment_brush@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_traverse;             // ?cdl_proftimer_traverse@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_vsphere_poly;         // ?cdl_proftimer_vsphere_poly@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_collide_segment_list; // ?cdl_proftimer_collide_segment_list@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_vsphere_traverse;     // ?cdl_proftimer_vsphere_traverse@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_vsphere_brush;        // ?cdl_proftimer_vsphere_brush@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_vsphere_patch;        // ?cdl_proftimer_vsphere_patch@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_wheel_collision;      // ?cdl_proftimer_wheel_collision@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_temp1;                // ?cdl_proftimer_temp1@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_temp3;                // ?cdl_proftimer_temp3@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_drones;               // ?cdl_proftimer_drones@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_task_sys;             // ?cdl_proftimer_task_sys@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_smoke_mgr;            // ?cdl_proftimer_smoke_mgr@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_notifies;             // ?cdl_proftimer_notifies@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_subtitles;            // ?cdl_proftimer_subtitles@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_aethread;             // ?cdl_proftimer_aethread@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_scn_anim;             // ?cdl_proftimer_scn_anim@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_veh_ctrl;             // ?cdl_proftimer_veh_ctrl@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_ent_advance;          // ?cdl_proftimer_ent_advance@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_draw;                 // ?cdl_proftimer_draw@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_audio;                // ?cdl_proftimer_audio@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_streaming;            // ?cdl_proftimer_streaming@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_pak_mgr;              // ?cdl_proftimer_pak_mgr@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_music_mgr;            // ?cdl_proftimer_music_mgr@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_effect_sys;           // ?cdl_proftimer_effect_sys@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_rumble_mgr;           // ?cdl_proftimer_rumble_mgr@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_scn_effect;           // ?cdl_proftimer_scn_effect@@3Ucdl_proftimer@@A
cdl_proftimer cdl_proftimer_entities;             // ?cdl_proftimer_entities@@3Ucdl_proftimer@@A
cdl_profcounter cdl_profcounter_temp0;            // ?cdl_profcounter_temp0@@3Ucdl_profcounter@@A
cdl_profcounter cdl_profcounter_temp1;            // ?cdl_profcounter_temp1@@3Ucdl_profcounter@@A

// External assert
extern bool _tlAssert(const char* file, int line, const char* cond, const char* msg);

extern int gjk(
    const cdlConvex& a, const math::Mat43& a2b,
    const cdlConvex& b, cdl_cinfo2& cinfo,
    float sep_tresh2, bool full_gjk,
    unsigned int abase, unsigned int anquads,
    unsigned int bbase, unsigned int bnquads);

static const __m128 Float4_SignMask = { -0.0f, -0.0f, -0.0f, -0.0f };

// SSE reassembly macros (match compiler's dot/hadd patterns)
#define DOT3(v) ((v).m128_f32[0] + ((v).m128_f32[1] + (v).m128_f32[2]))

// ============================================================================
// math::operator/ — compose A with the inverse affine transform B
// ea: 0x81D260
// ============================================================================
math::Mat43 math::operator/(const math::Mat43& a, const math::Mat43& b) {
    const __m128 b_y = b.y.v;
    const __m128 b_z = b.z.v;
    const __m128 v5 = _mm_shuffle_ps(b.x.v, b_y, 68);
    const __m128 v6 = _mm_shuffle_ps(_mm_shuffle_ps(b.x.v, b_y, 238), b_z, 168);
    const __m128 v7 = _mm_shuffle_ps(v5, b_z, 136);
    const __m128 v8 = _mm_shuffle_ps(v5, b_z, 221);
    const __m128 a_w = a.w.v;
    const __m128 inverse_translation = _mm_add_ps(
        _mm_mul_ps(_mm_shuffle_ps(a_w, a_w, 170), v6),
        _mm_xor_ps(
            Float4_SignMask,
            _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(b.w.v, b.w.v, 0), v7),
                    _mm_mul_ps(_mm_shuffle_ps(b.w.v, b.w.v, 85), v8)),
                _mm_mul_ps(_mm_shuffle_ps(b.w.v, b.w.v, 170), v6))));

    math::Mat43 result;
    result.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 0), v7),
            _mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 85), v8)),
        _mm_mul_ps(_mm_shuffle_ps(a.x.v, a.x.v, 170), v6));
    result.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 0), v7),
            _mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 85), v8)),
        _mm_mul_ps(_mm_shuffle_ps(a.y.v, a.y.v, 170), v6));
    result.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 0), v7),
            _mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 85), v8)),
        _mm_mul_ps(_mm_shuffle_ps(a.z.v, a.z.v, 170), v6));
    result.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a_w, a_w, 0), v7),
            _mm_mul_ps(_mm_shuffle_ps(a_w, a_w, 85), v8)),
        inverse_translation);
    return result;
}

// intersect — convex pair overlap test
// ea: 0x81E130
bool intersect(
    const cdlConvex& a, const math::Mat43& a2w,
    const cdlConvex& b, const math::Mat43& b2w,
    float tresh)
{
    const float radius = a.m_sphere.v.m128_f32[3]
        + b.m_sphere.v.m128_f32[3];

    const __m128 b_center_world = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(b.m_sphere.v, b.m_sphere.v, 0), b2w.x.v),
            _mm_mul_ps(_mm_shuffle_ps(b.m_sphere.v, b.m_sphere.v, 0x55), b2w.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(b.m_sphere.v, b.m_sphere.v, 0xAA), b2w.z.v),
            b2w.w.v));
    const __m128 a_center_world = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0), a2w.x.v),
            _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0x55), a2w.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0xAA), a2w.z.v),
            a2w.w.v));
    const __m128 center_delta = _mm_sub_ps(a_center_world, b_center_world);
    if (DOT3(_mm_mul_ps(center_delta, center_delta)) > radius * radius)
        return false;

    const __m128 b_x = b2w.x.v;
    const __m128 b_y = b2w.y.v;
    const __m128 b_z = b2w.z.v;
    const __m128 inverse_row_z = _mm_shuffle_ps(
        _mm_shuffle_ps(b_x, b_y, 0xEE), b_z, 0xA8);
    const __m128 inverse_row_y = _mm_shuffle_ps(
        _mm_shuffle_ps(b_x, b_y, 0x44), b_z, 0xDD);
    const __m128 inverse_row_x = _mm_shuffle_ps(
        _mm_shuffle_ps(b_x, b_y, 0x44), b_z, 0x88);
    const __m128 inverse_translation = _mm_xor_ps(
        Float4_SignMask,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(b2w.w.v, b2w.w.v, 0), inverse_row_x),
                _mm_mul_ps(_mm_shuffle_ps(b2w.w.v, b2w.w.v, 0x55), inverse_row_y)),
            _mm_mul_ps(_mm_shuffle_ps(b2w.w.v, b2w.w.v, 0xAA), inverse_row_z)));

    const auto transform_axis = [&](const __m128 axis) {
        return _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(axis, axis, 0), inverse_row_x),
                _mm_mul_ps(_mm_shuffle_ps(axis, axis, 0x55), inverse_row_y)),
            _mm_mul_ps(_mm_shuffle_ps(axis, axis, 0xAA), inverse_row_z));
    };

    math::Mat43 a2b;
    a2b.x.v = transform_axis(a2w.x.v);
    a2b.y.v = transform_axis(a2w.y.v);
    a2b.z.v = transform_axis(a2w.z.v);
    a2b.w.v = _mm_add_ps(transform_axis(a2w.w.v), inverse_translation);

    cdl_cinfo2 cinfo;
    cinfo.ni.v = _mm_sub_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0), a2b.x.v),
                _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0x55), a2b.y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(a.m_sphere.v, a.m_sphere.v, 0xAA), a2b.z.v),
                a2b.w.v)),
        b.m_sphere.v);
    return gjk(a, a2b, b, cinfo, tresh * tresh, true,
               0, 0, 0, 0) != 0;
}

// ============================================================================
// intersect — ray vs plane
// ea: 0x81D4E0
// ============================================================================
float intersect(
    const math::Position3& po,
    const math::Dir3&      pn,
    const math::Position3& ro,
    const math::Dir3&      rd)
{
    // Validate ray dir is unit length
    __m128 n2 = _mm_mul_ps(rd.v, rd.v);
    float len2 = DOT3(n2);
    if (fabsf(len2 - 1.0f) >= 0.0001f) {
        _tlAssert("source/cdl_common.cpp", 9, "", "");
        __debugbreak();
    }

    // Project ray dir onto plane normal
    __m128 denom_vec = _mm_mul_ps(rd.v, pn.v);
    float denom = DOT3(denom_vec);
    if (fabsf(denom) < 0.0001f)
        return 1.0e20f;  // parallel

    // Project pos delta onto plane normal
    __m128 delta = _mm_sub_ps(po.v, ro.v);
    __m128 numer_vec = _mm_mul_ps(delta, pn.v);
    float numer = DOT3(numer_vec);

    return numer / denom;
}

// ============================================================================
// dist2 — squared distance from point to AABB
// ea: 0x81D8C0
// ============================================================================
float dist2(const math::Position3& point, const cdlAABB& aabb) {
    // Clamp point to AABB
    __m128 min = _mm_sub_ps(aabb.m_sphere.v, aabb.m_dims.v);
    __m128 max = _mm_add_ps(aabb.m_sphere.v, aabb.m_dims.v);
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
// intersect — point against transformed AABB
// ea: 0x81D5E0
// ============================================================================
bool intersect(const cdlAABB& a, const math::Mat43& a2w, const math::Position3& point) {
    const __m128 v3 = a2w.y.v;
    const __m128 v4 = a2w.z.v;
    const __m128 v5 = _mm_shuffle_ps(a2w.x.v, v3, 68);
    const __m128 v6 = _mm_shuffle_ps(_mm_shuffle_ps(a2w.x.v, v3, 238), v4, 168);
    const __m128 v8 = _mm_shuffle_ps(v5, v4, 221);
    const __m128 v9 = _mm_shuffle_ps(v5, v4, 136);
    const __m128 v10 = _mm_xor_ps(
        Float4_SignMask,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 0), v9),
                _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 85), v8)),
            _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 170), v6)));
    const __m128 v14 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 0), v9),
            _mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 85), v8)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(point.v, point.v, 170), v6), v10));
    return ((_mm_movemask_ps(_mm_cmplt_ps(v14, _mm_add_ps(a.m_sphere.v, a.m_dims.v))) & 7) == 7)
        && ((_mm_movemask_ps(_mm_cmplt_ps(_mm_sub_ps(a.m_sphere.v, a.m_dims.v), v14)) & 7) == 7);
}

// ============================================================================
// intersect — sphere against transformed AABB
// ea: 0x81D6F0
// ============================================================================
bool intersect(const cdlAABB& a, const math::Mat43& a2w, const cdlSphere& sphere) {
    const __m128 sphere_data = sphere.m_sphere.v;
    const __m128 v3 = a2w.y.v;
    const __m128 v4 = a2w.z.v;
    const __m128 v5 = _mm_shuffle_ps(a2w.x.v, v3, 68);
    const __m128 v6 = _mm_shuffle_ps(_mm_shuffle_ps(a2w.x.v, v3, 238), v4, 168);
    const __m128 v8 = _mm_shuffle_ps(v5, v4, 221);
    const __m128 v9 = _mm_shuffle_ps(v5, v4, 136);
    const float radius = _mm_shuffle_ps(sphere_data, sphere_data, 255).m128_f32[0];
    const __m128 inverse_translation = _mm_xor_ps(
        Float4_SignMask,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 0), v9),
                _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 85), v8)),
            _mm_mul_ps(_mm_shuffle_ps(a2w.w.v, a2w.w.v, 170), v6)));
    const __m128 local_center = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(sphere_data, sphere_data, 0), v9),
            _mm_mul_ps(_mm_shuffle_ps(sphere_data, sphere_data, 85), v8)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sphere_data, sphere_data, 170), v6), inverse_translation));
    const __m128 min_corner = _mm_sub_ps(a.m_sphere.v, a.m_dims.v);
    const __m128 max_corner = _mm_add_ps(a.m_sphere.v, a.m_dims.v);
    const __m128 radius2 = _mm_setr_ps(radius * radius, radius * radius, radius * radius, 0.0f);
    if ((_mm_movemask_ps(_mm_cmplt_ps(min_corner, _mm_add_ps(local_center, radius2))) & 7) != 7)
        return false;
    if ((_mm_movemask_ps(_mm_cmplt_ps(_mm_sub_ps(local_center, radius2), max_corner)) & 7) != 7)
        return false;
    const __m128 delta = _mm_sub_ps(_mm_min_ps(max_corner, _mm_max_ps(min_corner, local_center)), local_center);
    const __m128 squared = _mm_mul_ps(delta, delta);
    return DOT3(squared) <= ((radius * radius) * (radius * radius));
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
    __m128 v4 = _mm_sub_ps(p1.v, p0.v);
    __m128 v5 = _mm_mul_ps(v4, v4);
    __m128 v6 = _mm_sub_ps(q1.v, p0.v);
    __m128 v7 = _mm_mul_ps(v4, v6);
    float v24 = DOT3(v5);
    __m128 v8 = _mm_mul_ps(v6, v6);
    float v27 = DOT3(v7);
    float best = DOT3(v8) - ((v27 * v27) / v24);

    __m128 v10 = _mm_sub_ps(q0.v, p1.v);
    __m128 v11 = _mm_mul_ps(v10, v10);
    __m128 v12 = _mm_sub_ps(q1.v, p1.v);
    float v25 = DOT3(v11);
    __m128 v13 = _mm_mul_ps(v12, v12);
    __m128 v14 = _mm_mul_ps(v10, v12);
    float v20 = DOT3(v13);
    float v22 = DOT3(v14);
    if (best > (v20 - ((v22 * v22) / v25)))
        best = v20 - ((v22 * v22) / v25);

    __m128 v15 = _mm_sub_ps(q0.v, p0.v);
    __m128 v16 = _mm_mul_ps(v15, v15);
    float v23 = DOT3(v16);
    __m128 v17 = _mm_mul_ps(v6, v6);
    float v26 = DOT3(v17);
    __m128 v18 = _mm_mul_ps(v15, v6);
    float v21 = DOT3(v18);
    if (best > (v26 - ((v21 * v21) / v23)))
        best = v26 - ((v21 * v21) / v23);

    if (best < 0.000099999997f)
        return 0.000099999997f;
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
// calc_closest - closest point on a triangle (v0, v1, v2) to point p
// ea: 0x81DB50 (verified: 4 ref args, by-value Position3 return)
// ============================================================================
math::Position3 calc_closest(const math::Position3& v0,
                             const math::Position3& v1,
                             const math::Position3& v2,
                             const math::Position3& p)
{
    float v37 = (p.v.m128_f32[0] - v0.v.m128_f32[0])
            * (v1.v.m128_f32[0] - v0.v.m128_f32[0])
        + (p.v.m128_f32[1] - v0.v.m128_f32[1])
            * (v1.v.m128_f32[1] - v0.v.m128_f32[1])
        + (p.v.m128_f32[2] - v0.v.m128_f32[2])
            * (v1.v.m128_f32[2] - v0.v.m128_f32[2]);
    float v33 = (p.v.m128_f32[0] - v0.v.m128_f32[0])
            * (v0.v.m128_f32[0] - v2.v.m128_f32[0])
        + (p.v.m128_f32[1] - v0.v.m128_f32[1])
            * (v0.v.m128_f32[1] - v2.v.m128_f32[1])
        + (p.v.m128_f32[2] - v0.v.m128_f32[2])
            * (v0.v.m128_f32[2] - v2.v.m128_f32[2]);
    if (!(v33 < 0.0f || v37 > 0.0f))
    {
        math::Position3 result;
        result.v = v0.v;
        return result;
    }
    float v32 = (v0.v.m128_f32[0] - v2.v.m128_f32[0])
            * (v0.v.m128_f32[0] - v2.v.m128_f32[0])
        + (v0.v.m128_f32[1] - v2.v.m128_f32[1])
            * (v0.v.m128_f32[1] - v2.v.m128_f32[1])
        + (v0.v.m128_f32[2] - v2.v.m128_f32[2])
            * (v0.v.m128_f32[2] - v2.v.m128_f32[2]);
    float v31 = v32 + v33;
    float v36 = (v1.v.m128_f32[0] - v0.v.m128_f32[0])
            * (v1.v.m128_f32[0] - v0.v.m128_f32[0])
        + (v1.v.m128_f32[1] - v0.v.m128_f32[1])
            * (v1.v.m128_f32[1] - v0.v.m128_f32[1])
        + (v1.v.m128_f32[2] - v0.v.m128_f32[2])
            * (v1.v.m128_f32[2] - v0.v.m128_f32[2]);
    float x02 = (p.v.m128_f32[0] - v1.v.m128_f32[0])
            * (v2.v.m128_f32[0] - v1.v.m128_f32[0])
        + (p.v.m128_f32[1] - v1.v.m128_f32[1])
            * (v2.v.m128_f32[1] - v1.v.m128_f32[1])
        + (p.v.m128_f32[2] - v1.v.m128_f32[2])
            * (v2.v.m128_f32[2] - v1.v.m128_f32[2]);
    if (!(v37 < v36 || x02 > 0.0f))
    {
        math::Position3 result;
        result.v = v1.v;
        return result;
    }
    float v34 = (v2.v.m128_f32[0] - v1.v.m128_f32[0])
            * (v2.v.m128_f32[0] - v1.v.m128_f32[0])
        + (v2.v.m128_f32[1] - v1.v.m128_f32[1])
            * (v2.v.m128_f32[1] - v1.v.m128_f32[1])
        + (v2.v.m128_f32[2] - v1.v.m128_f32[2])
            * (v2.v.m128_f32[2] - v1.v.m128_f32[2]);
    if (!(x02 < v34 || v31 > 0.0f))
    {
        math::Position3 result;
        result.v = v2.v;
        return result;
    }
    math::Position3 result;
    if (v37 < 0.0f || v36 < v37)
    {
        if (x02 < 0.0f || v34 < x02)
        {
            if (v31 < 0.0f || v32 < v31)
            {
                result.v = p.v;
                return result;
            }
            float t = v31 / v32;
            result.v = _mm_add_ps(
                v2.v, _mm_mul_ps(_mm_sub_ps(v0.v, v2.v),
                                 _mm_shuffle_ps(_mm_set_ss(t), _mm_set_ss(t), 0)));
            return result;
        }
        float t = x02 / v34;
        result.v = _mm_add_ps(
            v1.v,
            _mm_mul_ps(_mm_sub_ps(v2.v, v1.v),
                       _mm_shuffle_ps(_mm_set_ss(t), _mm_set_ss(t), 0)));
        return result;
    }
    float t = v37 / v36;
    result.v = _mm_add_ps(
        v0.v,
        _mm_mul_ps(_mm_sub_ps(v1.v, v0.v),
                   _mm_shuffle_ps(_mm_set_ss(t), _mm_set_ss(t), 0)));
    return result;
}
