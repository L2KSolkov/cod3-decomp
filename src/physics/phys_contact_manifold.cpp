// ============================================================================
// phys_contact_manifold.cpp â€” contact manifold convex-poly generation (10 funcs).
// Source: source/phys_contact_manifold.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:phys_contact_manifold.o):
//   compute_convex_poly_area / _perimeter / qsort /
//   setup_list_sorted_mesh_point / generate_convex_poly_internal /
//   comp_feature_normal / generate_convex_poly
//   phys_contact_manifold_process::find_bottom / intersect_poly_segment /
//   intersect_poly_poly
// ============================================================================

#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
const char* const g_contact_manifold_error_msg =
    "contact_manifold memory overflow: INCREASE phys_contact_manifold_process::ALLOCATER_MEMORY_SIZE";

// phys_memory_heap::fast_allocate - ea: 0x7195C0
char* phys_memory_heap::fast_allocate(int size, const char* error_msg)
{
    char* buffer_cur = m_buffer_cur;
    m_buffer_cur = &buffer_cur[size];
    if (m_buffer_cur <= m_buffer_end)
        return buffer_cur;
    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 104,
                  "m_buffer_cur <= m_buffer_end", error_msg))
        __debugbreak();
    return buffer_cur;
}

// phys_contact_manifold::is_feature_point - ea: 0x719430
bool phys_contact_manifold::is_feature_point(const math::Dir3& p) const
{
    const __m128 delta = _mm_sub_ps(p.v, m_feature_hitp.v);
    const __m128 dotv = _mm_mul_ps(delta, m_feature_hitn.v);
    const float dot = dotv.m128_f32[0]
                    + _mm_shuffle_ps(dotv, dotv, 85).m128_f32[0]
                    + _mm_shuffle_ps(dotv, dotv, 170).m128_f32[0];
    if (m_feature_distance_eps < dot)
    {
        const __m128 sqv = _mm_mul_ps(delta, delta);
        const float sq = sqv.m128_f32[0]
                       + _mm_shuffle_ps(sqv, sqv, 85).m128_f32[0]
                       + _mm_shuffle_ps(sqv, sqv, 170).m128_f32[0];
        if (m_sin_feautre_angular_eps_sq * sq < dot * dot)
            return false;
    }
    return true;
}

// phys_contact_manifold::add_mesh_point - ea: 0x719500
void phys_contact_manifold::add_mesh_point(const math::Dir3& p)
{
    contact_manifold_mesh_point* mp =
        (contact_manifold_mesh_point*)m_allocater->fast_allocate(
            32, g_contact_manifold_error_msg);
    if (((uintptr_t)mp & 0xF) != 0
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\collision\\phys_contact_manifold.h",
               187,
               "(unsigned int)(mp) % PHYS_ALIGNOF(contact_manifold_mesh_point) == 0",
               defaultFileName))
        __debugbreak();
    if (mp != &m_list_mesh_point[m_list_mesh_point_count]
        && _tlAssert(
               "c:\\cod\\code\\tl\\physics\\include\\collision\\phys_contact_manifold.h",
               188, "mp == m_list_mesh_point + m_list_mesh_point_count",
               defaultFileName))
        __debugbreak();
    ++m_list_mesh_point_count;
    mp->m_p.v = p.v;
}

// phys_contact_manifold::rht - ea: 0x8889F0
bool phys_contact_manifold::rht(const math::Dir3* e1, const math::Dir3* e2,
                                float min_length2, float min_sin_sq) {
    const double cross = phys_v2_cross(e1, e2);
    if (cross <= 0.0)
        return false;
    const __m128 e1_mul = _mm_mul_ps(e1->v, e1->v);
    const float e1_sq = e1_mul.m128_f32[0]
        + _mm_shuffle_ps(e1_mul, e1_mul, 85).m128_f32[0]
        + _mm_shuffle_ps(e1_mul, e1_mul, 170).m128_f32[0];
    if (min_length2 >= e1_sq)
        return false;
    const __m128 e2_mul = _mm_mul_ps(e2->v, e2->v);
    const float e2_sq = e2_mul.m128_f32[0]
        + _mm_shuffle_ps(e2_mul, e2_mul, 85).m128_f32[0]
        + _mm_shuffle_ps(e2_mul, e2_mul, 170).m128_f32[0];
    return min_length2 < e2_sq
        && min_sin_sq < (float)((cross * cross) / (e2_sq * e1_sq));
}

// phys_contact_manifold_process::isect_info::init - ea: 0x888AC0
void phys_contact_manifold_process::isect_info::init(phys_contact_manifold* cman) {
    m_cman = cman;
    m_i = cman->m_list_contact_point;
    m_next_i = m_i + 1;
    m_last_i = &m_i[cman->m_list_contact_point_count - 1];
    m_edge.v = _mm_sub_ps((*m_next_i)->m_contact_p.v, (*m_i)->m_contact_p.v);
}

// phys_contact_manifold_process::isect_info::update - ea: 0x888B30
void phys_contact_manifold_process::isect_info::update() {
    m_i = m_next_i;
    if (m_next_i == m_last_i)
        m_next_i = m_cman->m_list_contact_point;
    else
        ++m_next_i;
    m_edge.v = _mm_sub_ps((*m_next_i)->m_contact_p.v, (*m_i)->m_contact_p.v);
}

// phys_contact_manifold::reset_list_mesh_point - ea: 0x8786A0
void phys_contact_manifold::reset_list_mesh_point() {
    phys_memory_heap* allocater = m_allocater;
    allocater->m_buffer_cur = (char*)(((uintptr_t)allocater->m_buffer_cur + 15) & ~uintptr_t(15));
    if (allocater->m_buffer_cur >= allocater->m_buffer_end
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 96,
                     "m_buffer_cur < m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    m_list_mesh_point = (contact_manifold_mesh_point*)allocater->m_buffer_cur;
    m_list_mesh_point_count = 0;
}

// phys_contact_manifold::alloc_sorted_list_mesh_point - ea: 0x8786F0
void phys_contact_manifold::alloc_sorted_list_mesh_point() {
    phys_memory_heap* allocater = m_allocater;
    allocater->m_buffer_cur = (char*)(((uintptr_t)allocater->m_buffer_cur + 3) & ~uintptr_t(3));
    if (allocater->m_buffer_cur >= allocater->m_buffer_end
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 96,
                     "m_buffer_cur < m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    m_list_sorted_mesh_point = (contact_manifold_mesh_point**)allocater->m_buffer_cur;
    allocater->m_buffer_cur += 4 * m_list_mesh_point_count;
    if (allocater->m_buffer_cur > allocater->m_buffer_end
        && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 104,
                     "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
}

// phys_contact_manifold::xform_mesh_points - ea: 0x878770
void phys_contact_manifold::xform_mesh_points(const math::Mat43* xform) {
    for (contact_manifold_mesh_point* i = m_list_mesh_point;
         i != &m_list_mesh_point[m_list_mesh_point_count]; ++i) {
        const __m128 p = i->m_p.v;
        i->m_p.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(p, p, 0), xform->x.v),
                _mm_mul_ps(_mm_shuffle_ps(p, p, 85), xform->y.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(p, p, 170), xform->z.v),
                xform->w.v));
    }
}

// ============================================================================
// phys_contact_manifold::compute_convex_poly_area â€” ea: 0x8873A0
// ============================================================================
const float& phys_contact_manifold::compute_convex_poly_area() {
    contact_manifold_mesh_point** m_list_contact_point = this->m_list_contact_point;
    contact_manifold_mesh_point** v2 = m_list_contact_point + 1;
    contact_manifold_mesh_point** v3 = &m_list_contact_point[this->m_list_contact_point_count - 1];
    __m128* v4 = (__m128*)*m_list_contact_point;
    float i = 0.0f;
    for (; v2 != v3; ++v2) {
        __m128 v6 = (*(__m128**)v2)[1];
        __m128 v7 = _mm_sub_ps((*(__m128**)v2)[1], v6);
        __m128 v8 = _mm_sub_ps(v6, v4[1]);
        i = (_mm_shuffle_ps(v7, v7, 85).m128_f32[0] * v8.m128_f32[0]
             - _mm_shuffle_ps(v8, v8, 85).m128_f32[0] * v7.m128_f32[0]) + i;
    }
    float v9 = i * 0.5f;
    static float result;
    result = v9;
    if (v9 < 0.0f)
        return result = 0.0f - v9;
    return result;
}

// ============================================================================
// phys_contact_manifold::compute_convex_poly_perimeter â€” ea: 0x887460
// ============================================================================
const float& phys_contact_manifold::compute_convex_poly_perimeter() {
    contact_manifold_mesh_point** m_list_contact_point = this->m_list_contact_point;
    contact_manifold_mesh_point** v2 = &m_list_contact_point[this->m_list_contact_point_count];
    contact_manifold_mesh_point** v3 = v2 - 1;
    static float result;
    result = 0.0f;
    if (m_list_contact_point == v2)
        return result;
    do {
        __m128 v5 = _mm_sub_ps(((__m128*)*m_list_contact_point)[1], ((__m128*)*v3)[1]);
        __m128 v6 = _mm_mul_ps(v5, v5);
        v3 = m_list_contact_point++;
        result = result + sqrt(v6.m128_f32[0]
                               + (_mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                                  + _mm_shuffle_ps(v6, v6, 170).m128_f32[0]));
    } while (m_list_contact_point != v2);
    return result;
}

// ============================================================================
// phys_contact_manifold::qsort â€” ea: 0x8874F0
// ============================================================================
void phys_contact_manifold::qsort(contact_manifold_mesh_point** i0_mp,
                                  contact_manifold_mesh_point** i1_mp) {
    while (1) {
        contact_manifold_mesh_point** v3 = i0_mp;
        contact_manifold_mesh_point* pivot = *i0_mp;
        contact_manifold_mesh_point** v4 = i0_mp + 1;
        contact_manifold_mesh_point** v5 = i0_mp;
        if (i0_mp + 1 <= i1_mp) {
            do {
                const math::Dir3* v6 = (const math::Dir3*)*v4;
                if (phys_v2_le(*(const math::Dir3*)((char*)v6 + 16), pivot->m_contact_p)) {
                    contact_manifold_mesh_point* v7 = v5[1];
                    ++v5;
                    *v4 = v7;
                    *v5 = (contact_manifold_mesh_point*)v6;
                }
                ++v4;
            } while (v4 <= i1_mp);
            v3 = i0_mp;
        }
        *v3 = *v5;
        *v5 = pivot;
        if (((unsigned int)(v5 - v3) & 0xFFFFFFFC) > 4)
            phys_contact_manifold::qsort(v3, v5 - 1);
        if (((unsigned int)(i1_mp - v5) & 0xFFFFFFFC) <= 4)
            break;
        i0_mp = v5 + 1;
    }
}

// ============================================================================
// phys_contact_manifold::setup_list_sorted_mesh_point â€” ea: 0x887590
// ============================================================================
void phys_contact_manifold::setup_list_sorted_mesh_point() {
    contact_manifold_mesh_point** m_list_sorted_mesh_point = this->m_list_sorted_mesh_point;
    int count = this->m_list_mesh_point_count;
    contact_manifold_mesh_point* m_list_mesh_point = this->m_list_mesh_point;
    for (contact_manifold_mesh_point** i = &m_list_sorted_mesh_point[count];
         m_list_sorted_mesh_point != i; ++m_list_mesh_point) {
        *m_list_sorted_mesh_point++ = m_list_mesh_point;
    }
}

// ============================================================================
// phys_contact_manifold::generate_convex_poly_internal â€” ea: 0x8875B0
// ============================================================================
void phys_contact_manifold::generate_convex_poly_internal() {
    contact_manifold_mesh_point** m_list_sorted_mesh_point = this->m_list_sorted_mesh_point;
    contact_manifold_mesh_point** v4 = &m_list_sorted_mesh_point[this->m_list_mesh_point_count];
    contact_manifold_mesh_point* m_list_mesh_point = this->m_list_mesh_point;
    for (; m_list_sorted_mesh_point != v4; ++m_list_mesh_point)
        *m_list_sorted_mesh_point++ = m_list_mesh_point;
    this->qsort(this->m_list_sorted_mesh_point,
                &this->m_list_sorted_mesh_point[this->m_list_mesh_point_count - 1]);

    contact_manifold_mesh_point** v6 = this->m_list_sorted_mesh_point;
    contact_manifold_mesh_point** m_list_contact_point = this->m_list_contact_point;
    contact_manifold_mesh_point** v9 = v6 + this->m_list_mesh_point_count;
    contact_manifold_mesh_point** v2 = v6;
    contact_manifold_mesh_point** last_i_smp = m_list_contact_point + 1;
    if (v6 < v9) {
        do {
            contact_manifold_mesh_point* v26 = *v6;
            if (m_list_contact_point > last_i_smp) {
                __m128 v25 = (*(__m128**)v6)[1];
                contact_manifold_mesh_point** v13;
                do {
                    __m128 m_contact_p = (*(m_list_contact_point - 1))->m_contact_p.v;
                    __m128* v12 = (__m128*)*(m_list_contact_point - 2);
                    v13 = m_list_contact_point - 1;
                    math::Dir3 v24;
                    v24.v = _mm_sub_ps(v25, m_contact_p);
                    math::Dir3 v23;
                    v23.v = _mm_sub_ps(m_contact_p, v12[1]);
                    if (phys_contact_manifold::rht(&v23, &v24, 0.1156f, 0.00030458649f))
                        break;
                    --m_list_contact_point;
                } while (v13 > last_i_smp);
                v6 = v2;
            }
            phys_memory_heap* m_allocater = this->m_allocater;
            if ((m_list_contact_point < (contact_manifold_mesh_point**)m_allocater->m_buffer_start
                 || (m_list_contact_point + 1) > (contact_manifold_mesh_point**)m_allocater->m_buffer_end)
                && _tlAssert("source/phys_contact_manifold.cpp", 231,
                             "m_allocater->fast_is_within_buffer_limits(cp_mp,sizeof(contact_manifold_mesh_point*))",
                             g_contact_manifold_error_msg)) {
                __debugbreak();
            }
            *m_list_contact_point = v26;
            v6 = v6 + 1;
            ++m_list_contact_point;
            v2 = v6;
        } while (v6 < v9);
    }

    // Reverse pass
    contact_manifold_mesh_point** v16 = v6 - 2;
    last_i_smp = m_list_contact_point;
    contact_manifold_mesh_point** v22 = v16;
    if (v16 >= v9 - 2) {
        do {
            contact_manifold_mesh_point* v26 = *v16;
            if (m_list_contact_point > last_i_smp) {
                __m128 v23 = (*(__m128**)v16)[1];
                contact_manifold_mesh_point** v20;
                do {
                    __m128 v18 = (*(m_list_contact_point - 1))->m_contact_p.v;
                    __m128* v19 = (__m128*)*(m_list_contact_point - 2);
                    v20 = m_list_contact_point - 1;
                    math::Dir3 v24;
                    v24.v = _mm_sub_ps(v23, v18);
                    math::Dir3 v25;
                    v25.v = _mm_sub_ps(v18, v19[1]);
                    if (phys_contact_manifold::rht(&v25, &v24, 0.1156f, 0.00030458649f))
                        break;
                    --m_list_contact_point;
                } while (v20 > last_i_smp);
                v16 = v22;
            }
            phys_memory_heap* v21 = this->m_allocater;
            if ((m_list_contact_point < (contact_manifold_mesh_point**)v21->m_buffer_start
                 || (m_list_contact_point + 1) > (contact_manifold_mesh_point**)v21->m_buffer_end)
                && _tlAssert("source/phys_contact_manifold.cpp", 252,
                             "m_allocater->fast_is_within_buffer_limits(cp_mp,sizeof(contact_manifold_mesh_point*))",
                             g_contact_manifold_error_msg)) {
                __debugbreak();
            }
            v16 = v16 - 1;
            *m_list_contact_point++ = v26;
            v22 = v16;
        } while (v16 >= v9 - 2);
    }
    this->m_list_contact_point_count = (int)(m_list_contact_point - this->m_list_contact_point - 1);
}

// ============================================================================
// phys_contact_manifold::comp_feature_normal â€” ea: 0x887A10
// ============================================================================
const math::Dir3* phys_contact_manifold::comp_feature_normal(math::Dir3* result,
                                                             const math::Mat43* contact_mat) {
    bool v4 = this->m_list_contact_point_count == 2;
    if (this->m_list_contact_point_count < 2) {
        if (_tlAssert("source/phys_contact_manifold.cpp", 81,
                      "m_list_contact_point_count >= 2", ""))
            __debugbreak();
        v4 = this->m_list_contact_point_count == 2;
    }
    if (v4) {
        __m128 v5 = _mm_sub_ps(this->m_list_contact_point[1]->m_p.v,
                               this->m_list_contact_point[0]->m_p.v);
        __m128 v6 = _mm_mul_ps(v5, v5);
        float v35 = v6.m128_f32[0]
                    + (_mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                       + _mm_shuffle_ps(v6, v6, 170).m128_f32[0]);
        if (v35 <= 0.0f &&
            _tlAssert("source/phys_contact_manifold.cpp", 87, "nedge1_sq > 0.0f", ""))
            __debugbreak();
        math::Dir3 v7;
        v7.v = contact_mat->z.v;
        __m128 v8 = _mm_mul_ps(v7.v, v5);
        float v34 = v8.m128_f32[0]
                    + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                       + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
        math::Dir3 edge1;
        edge1.v = _mm_sub_ps(v7.v, _mm_mul_ps(v5, _mm_set1_ps(v34 / v35)));
        __m128 v10 = _mm_mul_ps(edge1.v, edge1.v);
        if ((v10.m128_f32[0]
             + (_mm_shuffle_ps(v10, v10, 85).m128_f32[0]
                + _mm_shuffle_ps(v10, v10, 170).m128_f32[0])) <= 0.0f
            && _tlAssert("source/phys_contact_manifold.cpp", 92, "nretv_sq > 0.0f", ""))
            __debugbreak();
        result->v = edge1.v;
        return result;
    } else {
        math::Dir3 v;
        v.v = _mm_setzero_ps();
        math::Dir3 v13;
        v13.v = _mm_setzero_ps();
        math::Dir3 v14;
        v14.v = _mm_setzero_ps();
        math::Dir3 v15;
        v15.v = _mm_setzero_ps();
        math::Dir3 retv;
        retv.v = _mm_setzero_ps();
        contact_manifold_mesh_point* m_list_mesh_point = this->m_list_mesh_point;
        contact_manifold_mesh_point* v17 = &m_list_mesh_point[this->m_list_mesh_point_count];
        if (m_list_mesh_point != v17) {
            do {
                __m128 v26;
                v26.m128_f32[0] = m_list_mesh_point->m_contact_p.v.m128_f32[0];
                v26.m128_f32[1] = _mm_shuffle_ps(m_list_mesh_point->m_contact_p.v,
                                                 m_list_mesh_point->m_contact_p.v, 85).m128_f32[0];
                v26.m128_f32[2] = 1.0f;
                v26.m128_f32[3] = 0.0f;
                float v33 = _mm_shuffle_ps(v26, v26, 85).m128_f32[0];
                __m128 v18 = _mm_mul_ps(m_list_mesh_point->m_p.v, contact_mat->z.v);
                float v32 = v18.m128_f32[0]
                            + (_mm_shuffle_ps(v18, v18, 85).m128_f32[0]
                               + _mm_shuffle_ps(v18, v18, 170).m128_f32[0]);
                retv.v = _mm_add_ps(v.v, _mm_mul_ps(v26, _mm_set1_ps(v26.m128_f32[0])));
                v13.v = _mm_add_ps(v13.v, _mm_mul_ps(v26, _mm_set1_ps(v33)));
                v.v = retv.v;
                v14.v = _mm_add_ps(v14.v, v26);
                ++m_list_mesh_point;
                v15.v = _mm_add_ps(v15.v, _mm_mul_ps(v26, _mm_set1_ps(v32)));
            } while (m_list_mesh_point != v17);
        }
        __m128 v19 = _mm_shuffle_ps(v14.v, v14.v, 9);
        __m128 v20 = _mm_shuffle_ps(v14.v, v14.v, 18);
        __m128 v25 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v13.v, v13.v, 9), v20),
            _mm_mul_ps(_mm_shuffle_ps(v13.v, v13.v, 18), v19));
        __m128 v21 = _mm_mul_ps(v.v, v25);
        float v36 = v21.m128_f32[0]
                    + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                       + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]);
        if (fabs(v36) <= 0.0f) {
            if (_tlAssert("source/phys_contact_manifold.cpp", 114, "fabsf(det) > 0.0f", ""))
                __debugbreak();
        }
        __m128 v22 = _mm_mul_ps(v15.v, v25);
        __m128 v23 = _mm_mul_ps(
            v.v,
            _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v15.v, v15.v, 9), v20),
                       _mm_mul_ps(_mm_shuffle_ps(v15.v, v15.v, 18), v19)));
        math::Dir3 v27;
        v27.v.m128_f32[0] = v22.m128_f32[0]
                            + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                               + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]);
        v27.v.m128_f32[1] = v23.m128_f32[0]
                            + (_mm_shuffle_ps(v23, v23, 85).m128_f32[0]
                               + _mm_shuffle_ps(v23, v23, 170).m128_f32[0]);
        v27.v.m128_f32[2] = 0.0f - v36;
        v27.v.m128_f32[3] = 0.0f;
        math::Dir3 v28;
        v28.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(v27.v.m128_f32[0]), contact_mat->x.v),
                _mm_mul_ps(_mm_set1_ps(v27.v.m128_f32[1]), contact_mat->y.v)),
            _mm_mul_ps(_mm_set1_ps(v27.v.m128_f32[2]), contact_mat->z.v));
        __m128 v24 = _mm_mul_ps(v28.v, v28.v);
        if ((v24.m128_f32[0]
             + (_mm_shuffle_ps(v24, v24, 85).m128_f32[0]
                + _mm_shuffle_ps(v24, v24, 170).m128_f32[0])) <= 0.0f
            && _tlAssert("source/phys_contact_manifold.cpp", 120, "nretv_sq > 0.0f", ""))
            __debugbreak();
        result->v = v28.v;
        return result;
    }
}

// ============================================================================
// phys_contact_manifold::generate_convex_poly â€” ea: 0x887EA0
// ============================================================================
void phys_contact_manifold::generate_convex_poly(const math::Mat43* contact_mat) {
    if (this->m_list_mesh_point_count <= 0 &&
        _tlAssert("source/phys_contact_manifold.cpp", 262,
                  "m_list_mesh_point_count > 0", ""))
        __debugbreak();
    phys_memory_heap* m_allocater = this->m_allocater;
    char* m_buffer_end = m_allocater->m_buffer_end;
    unsigned int v7 = ((unsigned int)m_allocater->m_buffer_cur + 3) & 0xFFFFFFFC;
    m_allocater->m_buffer_cur = (char*)v7;
    if ((char*)v7 >= m_buffer_end &&
        _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 96,
                  "m_buffer_cur < m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    int count = this->m_list_mesh_point_count;
    contact_manifold_mesh_point** m_buffer_cur = (contact_manifold_mesh_point**)m_allocater->m_buffer_cur;
    this->m_list_contact_point = m_buffer_cur;
    if (count == 1) {
        phys_memory_heap* v10 = this->m_allocater;
        if ((m_buffer_cur < (contact_manifold_mesh_point**)v10->m_buffer_start
             || (m_buffer_cur + 1) > (contact_manifold_mesh_point**)v10->m_buffer_end)
            && _tlAssert("source/phys_contact_manifold.cpp", 266,
                         "m_allocater->fast_is_within_buffer_limits(m_list_contact_point,sizeof(contact_manifold_mesh_point*))",
                         g_contact_manifold_error_msg))
            __debugbreak();
        this->m_list_contact_point_count = 1;
        *this->m_list_contact_point = this->m_list_mesh_point;
    } else {
        contact_manifold_mesh_point* v13 = this->m_list_mesh_point;
        contact_manifold_mesh_point* v24 = &v13[count];
        if (v13 != v24) {
            do {
                math::Dir3 v23;
                phys_v3_to_v2_inv_multiply(&v23, contact_mat, &v13->m_p);
                v13->m_contact_p.v = v23.v;
                ++v13;
            } while (v13 != v24);
        }
        this->generate_convex_poly_internal();
        if (this->m_list_contact_point_count == 2) {
            __m128 v16 = _mm_sub_ps(((__m128*)this->m_list_contact_point[0])[1],
                                    *(__m128*)(*(this->m_list_contact_point + 1) + 16));
            __m128 v17 = _mm_mul_ps(v16, v16);
            float d = v17.m128_f32[0]
                      + (_mm_shuffle_ps(v17, v17, 85).m128_f32[0]
                         + _mm_shuffle_ps(v17, v17, 170).m128_f32[0]);
            if (d < 0.1156f)
                this->m_list_contact_point_count = 1;
        }
    }
    if (this->m_list_contact_point_count <= 0 &&
        _tlAssert("source/phys_contact_manifold.cpp", 282,
                  "m_list_contact_point_count > 0", ""))
        __debugbreak();
    phys_memory_heap* v18 = this->m_allocater;
    contact_manifold_mesh_point** v19 = (contact_manifold_mesh_point**)v18->m_buffer_cur;
    char* v21 = (char*)(v19 + this->m_list_contact_point_count);
    bool v22 = v21 <= v18->m_buffer_end;
    v18->m_buffer_cur = v21;
    if (!v22 &&
        _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 104,
                  "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    if (this->m_list_contact_point != v19 &&
        _tlAssert("source/phys_contact_manifold.cpp", 284,
                  "m_list_contact_point == temp", ""))
        __debugbreak();
}

// ============================================================================
// phys_contact_manifold_process::find_bottom â€” ea: 0x8877C0
// ============================================================================
bool phys_contact_manifold_process::find_bottom(bridge* b, isect_info* left_cman,
                                                isect_info* right_cman) {
    contact_manifold_mesh_point** m_left_i = b->m_left_i;
    contact_manifold_mesh_point** m_right_i = b->m_right_i;
    contact_manifold_mesh_point** next_left_v = m_left_i;
    contact_manifold_mesh_point** v38 = m_right_i;
    contact_manifold_mesh_point** v32;
    contact_manifold_mesh_point** v31;
    __m128* p_v = NULL;
    __m128 v29 = _mm_setzero_ps();
    __m128* v20 = NULL;
    __m128 v29b = _mm_setzero_ps();
    do {
        v31 = b->m_right_i;
        bool v7 = m_left_i == left_cman->m_last_i;
        v32 = m_left_i;
        contact_manifold_mesh_point** m_list_contact_point =
            v7 ? left_cman->m_cman->m_list_contact_point : m_left_i + 1;
        contact_manifold_mesh_point* v9 = *m_list_contact_point;
        __m128 m_contact_p = v9->m_contact_p.v;
        p_v = &v9->m_contact_p.v;
        __m128 v12 = _mm_sub_ps(m_contact_p, (*(__m128**)m_left_i)[1]);
        v29 = v12;
        float v13 = _mm_shuffle_ps(v12, v12, 85).m128_f32[0];
        while (1) {
            contact_manifold_mesh_point** v14 = b->m_right_i;
            contact_manifold_mesh_point** v15 =
                v14 == right_cman->m_cman->m_list_contact_point ? right_cman->m_last_i : v14 - 1;
            __m128 v16 = _mm_sub_ps((*v15)->m_contact_p.v, *p_v);
            float v42 = _mm_shuffle_ps(v16, v16, 85).m128_f32[0];
            float v39 = v16.m128_f32[0];
            if ((v42 * v12.m128_f32[0]) - (v13 * v16.m128_f32[0]) >= 0.0f)
                break;
            v7 = v15 == v38;
            b->m_right_i = v15;
            if (v7)
                return false;
        }
        contact_manifold_mesh_point** v18 = b->m_right_i;
        contact_manifold_mesh_point** m_last_i =
            v18 == right_cman->m_cman->m_list_contact_point ? right_cman->m_last_i : v18 - 1;
        v20 = &(*m_last_i)->m_contact_p.v;
        __m128 v21 = _mm_sub_ps(*v20, (*(__m128**)v18)[1]);
        v29b = v21;
        float v22 = _mm_shuffle_ps(v21, v21, 85).m128_f32[0];
        while (1) {
            contact_manifold_mesh_point** v23 = b->m_left_i;
            contact_manifold_mesh_point** v24 =
                v23 == left_cman->m_last_i ? left_cman->m_cman->m_list_contact_point : v23 + 1;
            __m128 v25 = _mm_sub_ps((*v24)->m_contact_p.v, *v20);
            float v37 = _mm_shuffle_ps(v25, v25, 85).m128_f32[0];
            if ((v37 * v21.m128_f32[0]) - (v22 * v25.m128_f32[0]) <= 0.0f)
                break;
            v7 = v24 == next_left_v;
            b->m_left_i = v24;
            if (v7)
                return false;
        }
        m_left_i = b->m_left_i;
    } while (v32 != m_left_i || v31 != b->m_right_i);

    math::Dir3 v29d;
    v29d.v = v29;
    math::Dir3 v29e;
    v29e.v = v29b;
    float v42 = (float)phys_v2_cross(&v29d, &v29e);
    if (v42 <= 0.000099999997f) {
        b->m_intersection_p = (*b->m_right_i)->m_contact_p;
    } else {
        math::Dir3 v28;
        v28.v = _mm_sub_ps(*v20, *p_v);
        float v26 = (float)phys_v2_cross(&v28, &v29e);
        float t = v26 / v42;
        b->m_intersection_p.v = _mm_add_ps(*p_v, _mm_mul_ps(v29, _mm_set1_ps(t)));
    }
    return true;
}

// ============================================================================
// phys_contact_manifold_process::intersect_poly_segment â€” ea: 0x8880A0
// ============================================================================
void phys_contact_manifold_process::intersect_poly_segment(phys_contact_manifold* cman,
                                                           const math::Dir3* p0,
                                                           const math::Dir3* p1) {
    contact_manifold_mesh_point** m_list_contact_point = cman->m_list_contact_point;
    contact_manifold_mesh_point** v6 = &m_list_contact_point[cman->m_list_contact_point_count - 1];
    __m128* v7 = (__m128*)*v6;
    __m128 v8 = _mm_sub_ps(p1->v, p0->v);
    float t_enter = 1.0f;
    float t_exit = 1.0f;
    if (m_list_contact_point > v6) {
        goto alloc;
    }
    while (1) {
        __m128* v9 = (__m128*)*m_list_contact_point;
        __m128 v10 = _mm_sub_ps(v7[1], (*(__m128**)m_list_contact_point)[1]);
        math::Dir3 v23;
        v23.v.m128_f32[0] = 0.0f - _mm_shuffle_ps(v10, v10, 85).m128_f32[0];
        v23.v.m128_f32[1] = v10.m128_f32[0];
        v23.v.m128_f32[2] = 0.0f;
        v23.v.m128_f32[3] = 0.0f;
        __m128 v11 = _mm_mul_ps(_mm_sub_ps(v7[1], p0->v), v23.v);
        float v24 = v11.m128_f32[0]
                    + (_mm_shuffle_ps(v11, v11, 85).m128_f32[0]
                       + _mm_shuffle_ps(v11, v11, 170).m128_f32[0]);
        __m128 v12 = _mm_mul_ps(v8, v23.v);
        float v13 = v12.m128_f32[0]
                    + (_mm_shuffle_ps(v12, v12, 85).m128_f32[0]
                       + _mm_shuffle_ps(v12, v12, 170).m128_f32[0]);
        bool v14;
        if (v13 <= -0.000099999997f || v13 >= 0.000099999997f) {
            float v15 = v24 / v13;
            if (v13 >= 0.0f) {
                if (t_enter > v15)
                    t_enter = v15;
            } else if (v15 > t_exit) {
                t_exit = v15;
            }
            v14 = t_exit <= t_enter;
        } else {
            v14 = v24 >= 0.0f;
        }
        if (!v14)
            break;
        ++m_list_contact_point;
        v7 = v9;
        if (m_list_contact_point > v6) {
            if (t_enter < t_exit &&
                _tlAssert("source/phys_contact_manifold.cpp", 319, "t_enter <= t_exit", ""))
                __debugbreak();
            goto alloc;
        }
    }
    return;

alloc:
    char* m_buffer_end = this->m_allocater.m_buffer_end;
    char* v18 = (char*)(((unsigned int)this->m_allocater.m_buffer_cur + 15) & 0xFFFFFFF0);
    this->m_allocater.m_buffer_cur = v18;
    if (v18 >= m_buffer_end &&
        _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 96,
                  "m_buffer_cur < m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    this->m_list_isect_point = (math::Dir3*)this->m_allocater.m_buffer_cur;
    char* v19 = this->m_allocater.m_buffer_end;
    char* v21 = this->m_allocater.m_buffer_cur + 32;
    this->m_allocater.m_buffer_cur = v21;
    if (v21 > v19 &&
        _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 104,
                  "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
        __debugbreak();
    this->m_contact_point_count = 2;
    this->m_list_isect_point[0].v = _mm_add_ps(p0->v, _mm_mul_ps(v8, _mm_set1_ps(t_exit)));
    this->m_list_isect_point[1].v = _mm_add_ps(p0->v, _mm_mul_ps(v8, _mm_set1_ps(t_enter)));
}

// ============================================================================
// phys_contact_manifold_process::intersect_poly_poly â€” ea: 0x888340
// ============================================================================
void phys_contact_manifold_process::intersect_poly_poly() {
    phys_contact_manifold_process* v2 = this;
    int m_list_contact_point_count = this->cman1.m_list_contact_point_count;
    if ((m_list_contact_point_count < 2 || this->cman2.m_list_contact_point_count < 2) &&
        _tlAssert("source/phys_contact_manifold.cpp", 394,
                  "cman1.get_poly_vert_count() >= 2 && cman2.get_poly_vert_count() >= 2", ""))
        __debugbreak();
    if (v2->cman1.m_list_contact_point_count <= 2
        && v2->cman2.m_list_contact_point_count <= 2
        && _tlAssert("source/phys_contact_manifold.cpp", 395,
                     "cman1.get_poly_vert_count() > 2 || cman2.get_poly_vert_count() > 2", ""))
        __debugbreak();
    v2->m_list_isect_point = NULL;
    v2->m_contact_point_count = 0;
    if (v2->cman1.m_list_contact_point_count == 2) {
        if (v2->cman1.m_list_contact_point_count <= 0 &&
            _tlAssert("c:/cod/code/tl/physics/include/collision\\phys_contact_manifold.h", 227,
                      "i >= 0 && i < m_list_contact_point_count", ""))
            __debugbreak();
        this->intersect_poly_segment(&v2->cman2,
                                     &v2->cman1.m_list_contact_point[0]->m_contact_p,
                                     &(v2->cman1.m_list_contact_point[1]->m_contact_p));
    } else if (v2->cman2.m_list_contact_point_count == 2) {
        if (v2->cman2.m_list_contact_point_count <= 0 &&
            _tlAssert("c:/cod/code/tl/physics/include/collision\\phys_contact_manifold.h", 227,
                      "i >= 0 && i < m_list_contact_point_count", ""))
            __debugbreak();
        this->intersect_poly_segment(&v2->cman1,
                                     &v2->cman2.m_list_contact_point[0]->m_contact_p,
                                     &(v2->cman2.m_list_contact_point[1]->m_contact_p));
    } else {
        isect_info gb_cman1;
        isect_info gb_cman2;
        gb_cman1.init(&v2->cman1);
        gb_cman2.init(&v2->cman2);
        isect_info* v4 = &gb_cman1;
        isect_info* nvdisplace = &gb_cman2;
        isect_info* v52 = v4;
        bridge* v50 = (bridge*)this->m_allocater.fast_align_start(16, g_contact_manifold_error_msg);
        bridge* term_left_i = v50;
        bridge* v5 = v50;
        int m_buffer_cur = gb_cman1.m_cman->m_list_contact_point_count
                           + gb_cman2.m_cman->m_list_contact_point_count;
        int left_gb_count = 0;
        isect_info* left_gb = NULL;
        if (m_buffer_cur >= 0) {
            do {
                v5->m_left_i = v4->m_i;
                v5->m_right_i = nvdisplace->m_i;
                if (phys_v2_cross(&v4->m_edge, &nvdisplace->m_edge) < 0.0) {
                    v5->m_intersection_p = nvdisplace->m_edge;
                    nvdisplace->update();
                } else {
                    v5->m_intersection_p = v4->m_edge;
                    v4->update();
                }
                __m128 v17 = _mm_sub_ps(
                    (*(__m128**)nvdisplace->m_i)[1],
                    (*(__m128**)v4->m_i)[1]);
                float cross = _mm_shuffle_ps(v17, v17, 85).m128_f32[0] * v5->m_intersection_p.v.m128_f32[0]
                              - _mm_shuffle_ps(v5->m_intersection_p.v, v5->m_intersection_p.v, 85).m128_f32[0] * v17.m128_f32[0];
                if (cross <= 0.0f) {
                    if (cross >= 0.0f) {
                        math::Dir3 v20;
                        phys_v2_rotr(&v20, &v5->m_intersection_p);
                        __m128 v21 = _mm_mul_ps(v20.v, v20.v);
                        float len = sqrt(v21.m128_f32[0]
                                         + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                                            + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]));
                        if (len <= 0.000099999997f) {
                            if (_tlAssert("source/phys_contact_manifold.cpp", 458,
                                         "nvdisplace > 0.0001f", ""))
                                __debugbreak();
                        }
                        v20.v = _mm_mul_ps(v20.v, _mm_set1_ps(0.034000002f / len));
                        displace_contact_p(nvdisplace->m_cman->m_list_contact_point, &v20,
                                           &this->contact_mat);
                    } else {
                        if (left_gb != NULL) {
                            if ((term_left_i < (bridge*)this->m_allocater.m_buffer_start
                                 || (term_left_i + 1) > (bridge*)this->m_allocater.m_buffer_end)
                                && _tlAssert("source/phys_contact_manifold.cpp", 444,
                                             "m_allocater.fast_is_within_buffer_limits(b_cur,sizeof(bridge))",
                                             g_contact_manifold_error_msg))
                                __debugbreak();
                            if (!this->find_bottom(term_left_i, left_gb, nvdisplace))
                                return;
                            term_left_i += 1;
                        }
                        isect_info* tmp = nvdisplace;
                        nvdisplace = v4;
                        v4 = tmp;
                        v52 = tmp;
                    }
                }
                left_gb = (isect_info*)((char*)left_gb + 1);
                v5 = term_left_i;
                ++left_gb_count;
            } while (left_gb_count <= m_buffer_cur);
        }
        int v24 = (int)(term_left_i - v50);
        if (v24 % 2 == 0) {
            char* v26 = this->m_allocater.m_buffer_cur + 32 * v24;
            char* m_buffer_cur_old = this->m_allocater.m_buffer_cur;
            bool v27 = v26 <= this->m_allocater.m_buffer_end;
            this->m_allocater.m_buffer_cur = v26;
            if (!v27 &&
                _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 104,
                          "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
                __debugbreak();
            if (m_buffer_cur_old != (char*)v50 &&
                _tlAssert("source/phys_contact_manifold.cpp", 469,
                          "temp_ptr == list_bridge", ""))
                __debugbreak();
            if (left_gb_count > 0) {
                math::Dir3* v28 = (math::Dir3*)this->m_allocater.fast_align_start(16, g_contact_manifold_error_msg);
                this->m_list_isect_point = v28;
                math::Dir3* v29 = v28;
                int max_ctr = 0;
                if (left_gb_count > 0) {
                    bridge* term_i = v50;
                    int v33 = 0;
                    do {
                        if ((v29 < (math::Dir3*)this->m_allocater.m_buffer_start
                             || v29 + 1 > (math::Dir3*)this->m_allocater.m_buffer_end)
                            && _tlAssert("source/phys_contact_manifold.cpp", 482,
                                         "m_allocater.fast_is_within_buffer_limits(ip_i,sizeof(phys_vec2))",
                                         g_contact_manifold_error_msg))
                            __debugbreak();
                        *v29 = term_i->m_intersection_p;
                        ++v29;
                        v33 = max_ctr + 1;
                        contact_manifold_mesh_point* v34 = term_i->m_left_i[0];
                        contact_manifold_mesh_point** max_ptr =
                            v50[((max_ctr + 1) % left_gb_count)].m_right_i;
                        contact_manifold_mesh_point* m_cp = v52->m_cman->m_list_contact_point[0];
                        contact_manifold_mesh_point* v35;
                        if (v34 == m_cp)
                            v35 = v52->m_cman->m_list_contact_point[0];
                        else
                            v35 = v34;
                        if (v35 != (contact_manifold_mesh_point*)max_ptr) {
                            do {
                                if ((v29 < (math::Dir3*)this->m_allocater.m_buffer_start
                                     || v29 + 1 > (math::Dir3*)this->m_allocater.m_buffer_end)
                                    && _tlAssert("source/phys_contact_manifold.cpp", 488,
                                                 "m_allocater.fast_is_within_buffer_limits(ip_i,sizeof(phys_vec2))",
                                                 g_contact_manifold_error_msg))
                                    __debugbreak();
                                *v29 = v35->m_contact_p;
                                ++v29;
                                if (v35 == m_cp)
                                    v35 = v52->m_cman->m_list_contact_point[0];
                                else
                                    ++v35;
                            } while (v35 != (contact_manifold_mesh_point*)max_ptr);
                        }
                        isect_info* tmp = nvdisplace;
                        nvdisplace = v52;
                        v52 = tmp;
                        max_ctr = v33;
                        term_i += 1;
                    } while (v33 < left_gb_count);
                }
                int v38 = (int)(v29 - this->m_list_isect_point);
                this->m_contact_point_count = v38;
                char* v39 = this->m_allocater.m_buffer_cur;
                char* m_buffer_end = this->m_allocater.m_buffer_end;
                char* v42 = &v39[16 * v38];
                this->m_allocater.m_buffer_cur = v42;
                if (v42 > m_buffer_end &&
                    _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 104,
                              "m_buffer_cur <= m_buffer_end", g_contact_manifold_error_msg))
                    __debugbreak();
                if (v39 != (char*)this->m_list_isect_point &&
                    _tlAssert("source/phys_contact_manifold.cpp", 498,
                              "temp_ptr == m_list_isect_point", ""))
                    __debugbreak();
            } else {
                this->copy_poly(nvdisplace->m_cman);
            }
        }
    }
}
