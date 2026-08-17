// ============================================================================
// phys_collision.cpp — collision feature helpers (1 non-inline func).
// Source: phys_xboxr:phys_collision.o
// Verified against IDA:
//   calc_feature_normal @0x878A90
//   (process @0x878C10 — large; ported separately if/when infra available)
// ============================================================================
#include "physics/phys_types.h"
#include "physics/pulse_sum.h"
#include "physics/physics_system.h"
#include "core/math_types.h"

#include <intrin.h>

extern void PHYS_ASSERT_UNIT(const math::Dir3& v);

namespace {

float dot3(__m128 a, __m128 b) {
    const __m128 p = _mm_mul_ps(a, b);
    return p.m128_f32[0]
         + _mm_shuffle_ps(p, p, 85).m128_f32[0]
         + _mm_shuffle_ps(p, p, 170).m128_f32[0];
}

__m128 transform_point(__m128 p, const math::Mat43& m) {
    return _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(p, p, 0), m.x.v),
            _mm_mul_ps(_mm_shuffle_ps(p, p, 85), m.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(p, p, 170), m.z.v),
            m.w.v));
}

__m128 transform_dir(__m128 p, const math::Mat43& m) {
    return _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(p, p, 0), m.x.v),
            _mm_mul_ps(_mm_shuffle_ps(p, p, 85), m.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(p, p, 170), m.z.v));
}

}

// ============================================================================
// calc_feature_normal — find the mesh triangle whose normal is most aligned
// with the hit normal (bit-faithful to the release build's SSE shuffles).
// ea: 0x878A90
// ============================================================================
const math::Dir3 calc_feature_normal(phys_contact_manifold* cman, const math::Dir3& hitn) {
    math::Dir3 result;
    // best normal halves (x,z lanes packed as the decompiler grouped them)
    __int64 normal_12 = *(__int64*)&hitn.v.m128_f32[2];
    __int64 bestXZ = *(__int64*)&hitn.v.m128_f32[0];

    float bestScore = 0.99240386f;

    contact_manifold_mesh_point* list = cman->m_list_mesh_point;
    contact_manifold_mesh_point* end = &list[cman->m_list_mesh_point_count];

    for (contact_manifold_mesh_point* a = list; a < end; ++a) {
        math::Dir3 base = a->m_p;
        for (contact_manifold_mesh_point* b = a + 1; b < end; ++b) {
            __m128 v1 = b->m_p.v;
            v1 = _mm_sub_ps(v1, base.v);
            for (contact_manifold_mesh_point* c = b + 1; c < end; ++c) {
                __m128 v2 = c->m_p.v;
                v2 = _mm_sub_ps(v2, base.v);

                // cross = v1 x v2, computed in permuted lane order [x, z, y]
                __m128 v13 = _mm_shuffle_ps(v1, v1, 9);   // [y1, x1, z1, x1]
                __m128 v12 = _mm_shuffle_ps(v1, v1, 18);  // [z1, y1, x1, x1]
                __m128 cross = _mm_sub_ps(
                    _mm_mul_ps(v13, _mm_shuffle_ps(v2, v2, 18)),
                    _mm_mul_ps(v12, _mm_shuffle_ps(v2, v2, 9)));

                __m128 sq = _mm_mul_ps(cross, cross);
                float lenSq = sq.m128_f32[0]
                            + (_mm_shuffle_ps(sq, sq, 85).m128_f32[0]
                               + _mm_shuffle_ps(sq, sq, 170).m128_f32[0]);
                if (lenSq > 0.0000099999997f) {
                    __m128 dp = _mm_mul_ps(cross, hitn.v);
                    float dot = dp.m128_f32[0]
                              + (_mm_shuffle_ps(dp, dp, 85).m128_f32[0]
                                 + _mm_shuffle_ps(dp, dp, 170).m128_f32[0]);
                    float score = (dot / lenSq) * dot;
                    if (score >= bestScore) {
                        bestXZ = *(__int64*)&cross.m128_f32[0];
                        normal_12 = *(__int64*)&cross.m128_f32[2];
                        bestScore = score;
                    }
                }
            }
        }
    }

    result.v.m128_f32[0] = *(float*)&bestXZ;
    result.v.m128_f32[1] = *(float*)((char*)&bestXZ + 4);
    result.v.m128_f32[2] = *(float*)&normal_12;
    result.v.m128_f32[3] = *(float*)((char*)&normal_12 + 4);
    return result;
}

// phys_contact_manifold_process::comp_contact_mat - ea: 0x8787F0
void phys_contact_manifold_process::comp_contact_mat(const math::Dir3* contact_normal)
{
    const __m128 normal = contact_normal->v;
    __m128 nrow;
    if (fabsf(normal.m128_f32[0]) >= 0.80000001f)
        nrow = _mm_mul_ps(normal, _mm_set1_ps(normal.m128_f32[1]));
    else
        nrow = _mm_mul_ps(normal, _mm_set1_ps(normal.m128_f32[0]));

    const __m128 yrow = _mm_sub_ps(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), nrow);
    __m128 ysq = _mm_mul_ps(yrow, yrow);
    const float ylen = sqrtf(ysq.m128_f32[0]
        + _mm_shuffle_ps(ysq, ysq, 85).m128_f32[0]
        + _mm_shuffle_ps(ysq, ysq, 170).m128_f32[0]);
    if (ylen <= 0.1f
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\collision\\phys_contact_manifold.h",
                     360, "nyrow > 0.1f", defaultFileName))
        __debugbreak();

    this->contact_mat.y.v = _mm_div_ps(yrow, _mm_set1_ps(ylen));
    const __m128 y = this->contact_mat.y.v;
    this->contact_mat.x.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(y, y, 9), _mm_shuffle_ps(normal, normal, 18)),
        _mm_mul_ps(_mm_shuffle_ps(y, y, 18), _mm_shuffle_ps(normal, normal, 9)));
    this->contact_mat.z.v = normal;
}

// contact_point_info::set_closest_cached_psc - ea: 0x879690
void contact_point_info::set_closest_cached_psc(const contact_point_info* cached_cpi)
{
    pulse_sum_cache_info* cache = m_list_pulse_sum_cache_info;
    pulse_sum_cache_info* end = &cache[2 * m_point_pair_count];
    math::Dir3* b1_r_loc = m_list_b1_r_loc;
    for (math::Dir3* b2_r_loc = m_list_b2_r_loc; cache != end;
         ++b2_r_loc, cache += 2, ++b1_r_loc) {
        contact_point_info::set_closest_cached_psc(
            cached_cpi, m_normal, *b1_r_loc, *b2_r_loc, cache);
    }
}

// phys_contact_manifold_process::copy_poly - ea: 0x8789B0
void phys_contact_manifold_process::copy_poly(phys_contact_manifold* cman)
{
    m_allocater.m_buffer_cur = (char*)(((uintptr_t)m_allocater.m_buffer_cur + 15) & ~uintptr_t(15));
    if (m_allocater.m_buffer_cur >= m_allocater.m_buffer_end
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 96,
                     "m_buffer_cur < m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    m_list_isect_point = (math::Dir3*)m_allocater.m_buffer_cur;
    m_contact_point_count = cman->m_list_contact_point_count;
    m_allocater.m_buffer_cur += 16 * m_contact_point_count;
    if (m_allocater.m_buffer_cur > m_allocater.m_buffer_end
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 104,
                     "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    for (int i = 0; i < cman->m_list_contact_point_count; ++i)
        m_list_isect_point[i].v = cman->m_list_contact_point[i]->m_contact_p.v;
}

// ?process@phys_contact_manifold_process@@QAEXPAVphys_collide_data@@@Z
void phys_contact_manifold_process::process(phys_collide_data* d)
{
    cman1.m_list_mesh_point = nullptr;
    cman1.m_list_sorted_mesh_point = nullptr;
    cman1.m_list_contact_point = nullptr;
    cman2.m_list_mesh_point = nullptr;
    cman2.m_list_sorted_mesh_point = nullptr;
    cman2.m_list_contact_point = nullptr;
    m_list_isect_point = nullptr;
    m_allocater.m_buffer_cur = m_allocater.m_buffer_start;
    m_cpi = nullptr;

    phys_full_inv_multiply_mat(cg1_to_rb2_xform, *d->rb2_to_world_xform,
                               *d->cg1_to_world_xform);
    phys_gjk_collision_info* cinfo = d->cg1_cinfo_loc;
    const __m128 n = cinfo->m_n.v;
    const __m128 n85 = _mm_mul_ps(n, _mm_set1_ps(0.85f));
    math::Dir3 p1_displaced;
    p1_displaced.v = _mm_sub_ps(cinfo->m_p1.v, n85);
    math::Dir3 p2_displaced;
    p2_displaced.v = _mm_add_ps(cinfo->m_p2.v, n85);
    const float nfn = dot3(_mm_sub_ps(cinfo->m_p1.v, cinfo->m_p2.v), n);
    float feature_eps = -nfn;
    if (feature_eps < 0.0f)
        feature_eps = 0.0f;
    else if (feature_eps > 10.200001f)
        feature_eps = 10.200001f;
    feature_eps += 2.5500002f;

    cman1.reset_list_mesh_point();
    cman1.m_feature_hitp = p1_displaced;
    cman1.m_feature_hitn = cinfo->m_n;
    cman1.m_feature_distance_eps = feature_eps;
    cman1.m_sin_feautre_angular_eps_sq = 0.000027415317f;
    d->gjk_cg1->get_feature(&cman1);
    cman1.alloc_sorted_list_mesh_point();

    cman2.reset_list_mesh_point();
    const math::Mat43* cg2_to_cg1 = d->cg2_to_cg1_xform;
    const __m128 xform_y = cg2_to_cg1->y.v;
    const __m128 xform_z = cg2_to_cg1->z.v;
    const __m128 neg_n = _mm_xor_ps(_mm_set1_ps(-0.0f), n);
    const __m128 v31 = _mm_shuffle_ps(cg2_to_cg1->x.v, xform_y, 68);
    const __m128 v32 = _mm_shuffle_ps(v31, xform_z, 221);
    const __m128 v33 = _mm_shuffle_ps(v31, xform_z, 136);
    const __m128 v34 = _mm_shuffle_ps(cg2_to_cg1->x.v, xform_y, 238);
    const __m128 v36 = _mm_mul_ps(_mm_shuffle_ps(neg_n, neg_n, 170),
                                  _mm_shuffle_ps(v34, xform_z, 168));
    const __m128 v37 = _mm_mul_ps(_mm_shuffle_ps(neg_n, neg_n, 85), v32);
    const __m128 v38 = _mm_mul_ps(_mm_shuffle_ps(neg_n, neg_n, 0), v33);
    const __m128 cman2_hitn = _mm_add_ps(_mm_add_ps(v38, v37), v36);
    const __m128 v42 = _mm_shuffle_ps(cg2_to_cg1->x.v, xform_y, 68);
    const __m128 v43 = _mm_shuffle_ps(
        _mm_shuffle_ps(cg2_to_cg1->x.v, xform_y, 238), cg2_to_cg1->z.v, 168);
    const __m128 v45 = _mm_shuffle_ps(v42, cg2_to_cg1->z.v, 221);
    const __m128 v46 = _mm_shuffle_ps(v42, cg2_to_cg1->z.v, 136);
    const __m128 v48 = _mm_xor_ps(
        _mm_set1_ps(-0.0f),
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(cg2_to_cg1->w.v, cg2_to_cg1->w.v, 0), v46),
                _mm_mul_ps(_mm_shuffle_ps(cg2_to_cg1->w.v, cg2_to_cg1->w.v, 85), v45)),
            _mm_mul_ps(_mm_shuffle_ps(cg2_to_cg1->w.v, cg2_to_cg1->w.v, 170), v43)));
    const __m128 p2 = p2_displaced.v;
    const __m128 cman2_hitp = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(p2, p2, 0), v46),
                   _mm_mul_ps(_mm_shuffle_ps(p2, p2, 85), v45)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(p2, p2, 170), v43), v48));
    cman2.m_feature_hitp.v = cman2_hitp;
    cman2.m_feature_hitn.v = cman2_hitn;
    cman2.m_feature_distance_eps = feature_eps;
    cman2.m_sin_feautre_angular_eps_sq = 0.000027415317f;
    d->gjk_cg2->get_feature(&cman2);
    cman2.xform_mesh_points(d->cg2_to_cg1_xform);
    cman2.alloc_sorted_list_mesh_point();

    const int mesh_count_1 = cman1.m_list_mesh_point_count;
    const int mesh_count_2 = cman2.m_list_mesh_point_count;
    if (mesh_count_1 < 2 || mesh_count_2 < 2
        || (mesh_count_1 == 2 && mesh_count_2 == 2)) {
        m_cpi = contact_point_info::create_cpi(1, d->no_overflow_error, m_cpi_allocater);
        if (m_cpi == nullptr)
            return;
        m_cpi->m_list_b1_r_loc[0].v = transform_point(cman1.m_feature_hitp.v,
                                                       *d->cg1_to_rb1_xform);
        m_cpi->m_list_b2_r_loc[0].v = transform_point(cman2.m_feature_hitp.v,
                                                       cg1_to_rb2_xform);
        m_contact_point_count = 1;
    } else {
        comp_contact_mat(&cinfo->m_n);
        cman1.generate_convex_poly(&contact_mat);
        cman2.generate_convex_poly(&contact_mat);
        const int poly_count_1 = cman1.m_list_contact_point_count;
        const int poly_count_2 = cman2.m_list_contact_point_count;
        if (poly_count_1 < 2 || poly_count_2 < 2
            || (poly_count_1 == 2 && poly_count_2 == 2)) {
            m_cpi = contact_point_info::create_cpi(1, d->no_overflow_error, m_cpi_allocater);
            if (m_cpi == nullptr)
                return;
            m_cpi->m_list_b1_r_loc[0].v = transform_point(cman1.m_feature_hitp.v,
                                                           *d->cg1_to_rb1_xform);
            m_cpi->m_list_b2_r_loc[0].v = transform_point(cman2.m_feature_hitp.v,
                                                           cg1_to_rb2_xform);
            m_contact_point_count = 1;
        } else {
            intersect_poly_poly();
            if (m_contact_point_count == 0) {
                phys_contact_manifold* smaller = &cman1;
                if (cman2.compute_convex_poly_perimeter() < cman1.compute_convex_poly_perimeter())
                    smaller = &cman2;
                copy_poly(smaller);
            }
            if (m_contact_point_count <= 0
                && _tlAssert("source/phys_collision.cpp", 104,
                             "m_contact_point_count > 0", defaultFileName))
                __debugbreak();
            if (m_list_isect_point == nullptr
                && _tlAssert("source/phys_collision.cpp", 105,
                             "m_list_isect_point", defaultFileName))
                __debugbreak();

            math::Dir3 fn1;
            math::Dir3 fn2;
            float fn1_sq;
            float fn2_sq;
            if (nfn >= 0.0f) {
                cman1.comp_feature_normal(&fn1, &contact_mat);
                fn1_sq = dot3(fn1.v, fn1.v);
                const float fn1_dot_n = dot3(fn1.v, n);
                if (fn1_sq * 0.99240386f > fn1_dot_n * fn1_dot_n) {
                    fn1 = calc_feature_normal(&cman1, cinfo->m_n);
                    fn1_sq = dot3(fn1.v, fn1.v);
                }
                cman2.comp_feature_normal(&fn2, &contact_mat);
                fn2_sq = dot3(fn2.v, fn2.v);
                const float fn2_dot_n = dot3(fn2.v, n);
                if (fn2_sq * 0.99240386f > fn2_dot_n * fn2_dot_n) {
                    fn2 = calc_feature_normal(&cman2, cinfo->m_n);
                    fn2_sq = dot3(fn2.v, fn2.v);
                }
            } else {
                fn1 = cinfo->m_n;
                fn2 = cinfo->m_n;
                fn1_sq = 1.0f;
                fn2_sq = 1.0f;
            }

            m_cpi = contact_point_info::create_cpi(
                m_contact_point_count, d->no_overflow_error, m_cpi_allocater);
            if (m_cpi == nullptr)
                return;
            const float inv_fn1_sq = 1.0f / fn1_sq;
            const float inv_fn2_sq = 1.0f / fn2_sq;
            for (int i = 0; i < m_contact_point_count; ++i) {
                const __m128 p = m_list_isect_point[i].v;
                const __m128 contact_p = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(p, p, 0), contact_mat.x.v),
                        _mm_mul_ps(_mm_shuffle_ps(p, p, 85), contact_mat.y.v)),
                    _mm_mul_ps(_mm_shuffle_ps(p, p, 170), contact_mat.z.v));
                const float d1 = dot3(_mm_sub_ps(contact_p, cman1.m_feature_hitp.v), fn1.v);
                const __m128 p1 = _mm_sub_ps(contact_p,
                    _mm_mul_ps(fn1.v, _mm_set1_ps(d1 * inv_fn1_sq)));
                const float d2 = dot3(_mm_sub_ps(contact_p, cman2.m_feature_hitp.v), fn2.v);
                const __m128 p2 = _mm_sub_ps(contact_p,
                    _mm_mul_ps(fn2.v, _mm_set1_ps(d2 * inv_fn2_sq)));
                m_cpi->m_list_b1_r_loc[i].v = transform_point(p1, *d->cg1_to_rb1_xform);
                m_cpi->m_list_b2_r_loc[i].v = transform_point(p2, cg1_to_rb2_xform);
            }
        }
    }

    if (m_cpi == nullptr
        && _tlAssert("source/phys_collision.cpp", 153, "m_cpi", defaultFileName))
        __debugbreak();
    m_cpi->set(d->fric_coef, d->bounce_coef, 3400.0f, d->no_overflow_error);
    math::Dir3 world_normal;
    world_normal.v = _mm_xor_ps(_mm_set1_ps(-0.0f),
                                transform_dir(cinfo->m_n.v, *d->cg1_to_world_xform));
    m_cpi->m_normal = world_normal;
    PHYS_ASSERT_UNIT(m_cpi->m_normal);

    rigid_body_constraint_contact* rbc_contact =
        phys_sys::create_rbc_contact(d->rb1, d->rb2, d->no_overflow_error);
    if (rbc_contact != nullptr) {
        rbc_contact->m_solver_priority = d->solver_priority;
        if (rbc_contact->b1 != d->rb1) {
            math::Dir3* list_b1 = m_cpi->m_list_b1_r_loc;
            m_cpi->m_list_b1_r_loc = m_cpi->m_list_b2_r_loc;
            m_cpi->m_list_b2_r_loc = list_b1;
            m_cpi->m_normal.v = _mm_xor_ps(_mm_set1_ps(-0.0f), m_cpi->m_normal.v);
        }
        rbc_contact->add_cpi_simple(m_cpi, d->rb1, d->rb2);
        m_cpi->set_closest_cached_psc(rbc_contact->get_cached_cpi());
    }
}
