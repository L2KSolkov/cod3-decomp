// ============================================================================
// phys_util.cpp Ã¢â‚¬â€ rigid-body aggregate utilities (8 non-inline funcs).
// Source: source/phys_util.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:phys_util.o):
//   nuge::get_ballistic_info        @0x8836A0
//   nuge::apply_ballistic_target    @0x883830
//   nuge::calc_velocities (5-arg)   @0x883D40
//   nuge::calc_velocities (6-arg)   @0x884360
//   nuge::calc_sphere_inertia       @0x884460
//   nuge::calc_box_inertia          @0x884500
//   nuge::calc_bound_sphere         @0x8846A0
//   nuge::calc_bound_box            @0x8847B0
// ============================================================================

#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern const math::Dir3& Float4_Zero_206;

// phys_memory_heap::fast_align_start - ea: 0x878550
char* phys_memory_heap::fast_align_start(int alignment, const char* error_msg) {
    char* aligned = (char*)(((uintptr_t)&m_buffer_cur[alignment - 1])
                            & ~(uintptr_t)(alignment - 1));
    m_buffer_cur = aligned;
    if (aligned < m_buffer_end)
        return m_buffer_cur;
    if (!_tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 96,
                   "m_buffer_cur < m_buffer_end", error_msg))
        __debugbreak();
    return m_buffer_cur;
}

// phys_v2_cross - ea: 0x887160
double phys_v2_cross(const math::Dir3* v1, const math::Dir3* v2) {
    return v2->v.m128_f32[1] * v1->v.m128_f32[0]
         - v1->v.m128_f32[1] * v2->v.m128_f32[0];
}

// phys_v2_le - ea: 0x8871D0
bool phys_v2_le(const math::Dir3& v1, const math::Dir3& v2) {
    return v2.v.m128_f32[0] > v1.v.m128_f32[0]
        || (v1.v.m128_f32[0] == v2.v.m128_f32[0]
            && v2.v.m128_f32[1] > v1.v.m128_f32[1]);
}

// phys_v2_rotr - ea: 0x887270
const math::Dir3* phys_v2_rotr(math::Dir3* result, const math::Dir3* v) {
    result->v.m128_f32[0] = -v->v.m128_f32[1];
    result->v.m128_f32[1] = v->v.m128_f32[0];
    result->v.m128_f32[3] = 0.0f;
    return result;
}

// phys_v3_to_v2_inv_multiply - ea: 0x888BB0
const math::Dir3* phys_v3_to_v2_inv_multiply(math::Dir3* result,
                                             const math::Mat43* m,
                                             const math::Dir3* v) {
    const __m128 vx = _mm_mul_ps(v->v, m->x.v);
    const __m128 vy = _mm_mul_ps(v->v, m->y.v);
    result->v.m128_f32[0] = vx.m128_f32[0]
        + _mm_shuffle_ps(vx, vx, 85).m128_f32[0]
        + _mm_shuffle_ps(vx, vx, 170).m128_f32[0];
    result->v.m128_f32[1] = vy.m128_f32[0]
        + _mm_shuffle_ps(vy, vy, 85).m128_f32[0]
        + _mm_shuffle_ps(vy, vy, 170).m128_f32[0];
    return result;
}

// displace_contact_p - ea: 0x888C50
void displace_contact_p(contact_manifold_mesh_point** mp, const math::Dir3* d,
                        const math::Mat43* contact_mat) {
    (*mp)->m_contact_p.v = _mm_add_ps((*mp)->m_contact_p.v, d->v);
    (*mp)->m_p.v = _mm_add_ps(
        (*mp)->m_p.v,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(d->v, d->v, 0), contact_mat->x.v),
                _mm_mul_ps(_mm_shuffle_ps(d->v, d->v, 85), contact_mat->y.v)),
            _mm_mul_ps(_mm_shuffle_ps(d->v, d->v, 170), contact_mat->z.v)));
}

// MSVC low/high dword helpers (used to mirror release decompile register reuse)
#define LODWORD(x) (*((unsigned int*)&(x)))
#define HIDWORD(x) (*((unsigned int*)&(x) + 1))

namespace nuge {

// ============================================================================
// get_ballistic_info Ã¢â‚¬â€ ea: 0x8836A0
// ============================================================================
void get_ballistic_info(rigid_body* const* list_rigid_body, int rbodies_count,
                        math::Dir3* center_of_mass, math::Dir3* total_momentum,
                        float* total_mass) {
    if (((unsigned int)center_of_mass & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)total_momentum & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    center_of_mass->v = Float4_Zero_206.v;
    total_momentum->v = Float4_Zero_206.v;
    *total_mass = 0.0f;
    for (int i = 0; i < rbodies_count; ++i) {
        rigid_body* v7 = list_rigid_body[i];
        if (v7 != NULL) {
            float v8 = 1.0f / v7->m_inv_mass;
            if ((~(v7->m_flags >> 6) & 1) == 0) {
                v8 = 1.0f / v7->m_inv_mass;
                if (_tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                              "debug_flag_is_not_in_collision()", "")) {
                    __debugbreak();
                }
            }
            __m128 v9 = _mm_set1_ps(v8);
            center_of_mass->v = _mm_add_ps(center_of_mass->v, _mm_mul_ps(v7->m_mat.w.v, v9));
            total_momentum->v = _mm_add_ps(total_momentum->v, _mm_mul_ps(v7->m_t_vel.v, v9));
            *total_mass = *total_mass + v8;
        }
    }
    if (*total_mass <= 0.0000099999997f) {
        if (_tlAssert("source/phys_util.cpp", 23, "*total_mass > 0.00001f", ""))
            __debugbreak();
    }
    center_of_mass->v = _mm_div_ps(center_of_mass->v, _mm_set1_ps(*total_mass));
}

// ============================================================================
// apply_ballistic_target Ã¢â‚¬â€ ea: 0x883830
// ============================================================================
void apply_ballistic_target(rigid_body* const* list_rigid_body, int rbodies_count,
                            const math::Dir3* target, float* dist_sq) {
    unsigned char v40[96];
    float v41;
    float v42_lo;
    float v42_hi;
    float v43;
    float v44;
    float v45;
    float v46;
    float v8;

    nuge::get_ballistic_info(list_rigid_body, rbodies_count,
                             (math::Dir3*)&v40[64],
                             (math::Dir3*)&v40[48],
                             &v44);
    __m128 v5 = _mm_div_ps(*(__m128*)&v40[48], _mm_set1_ps(v44));
    __m128 v6 = _mm_sub_ps(target->v, *(__m128*)&v40[64]);
    __m128 v7 = _mm_mul_ps(v6, v6);
    v45 = v7.m128_f32[0]
          + (_mm_shuffle_ps(v7, v7, 85).m128_f32[0]
             + _mm_shuffle_ps(v7, v7, 170).m128_f32[0]);
    *dist_sq = v45;
    v42_lo = _mm_shuffle_ps(v6, v6, 85).m128_f32[0];
    v42_hi = v6.m128_f32[0];
    v8 = _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
    v46 = v8;
    v43 = v6.m128_f32[0];
    *(__m128*)v40 = v5;
    double v9 = sqrt(v8 * v8 + v6.m128_f32[0] * v6.m128_f32[0]);
    v45 = (float)v9;
    if (v9 >= 0.001) {
        float m_gravity_multiplier = (*list_rigid_body)->m_gravity_multiplier;
        *&v40[80] = v6.m128_f32[0];
        __m128 v11 = _mm_set1_ps(v45);
        float v12 = ((m_gravity_multiplier * -9.8000002f) * v45) * 0.5f;
        float v13 = v42_lo / v45;
        memset(&v40[84], 0, 12);
        __m128 v14 = _mm_div_ps(*(__m128*)&v40[80], v11);
        v42_hi = _mm_shuffle_ps(v5, v5, 85).m128_f32[0];
        __m128 v15 = _mm_mul_ps(v5, v14);
        *(__m128*)&v40[16] = v14;
        v42_lo = v15.m128_f32[0]
                 + (_mm_shuffle_ps(v15, v15, 85).m128_f32[0]
                    + _mm_shuffle_ps(v15, v15, 170).m128_f32[0]);
        *&v40[80] = v42_lo;
        *&v40[32] = v42_lo;
        v15.m128_f32[0] = 1.0f;
        float v16 = v12 * -2.0f;
        int v17 = 0;
        v46 = 1.0f;
        int v18 = 0;
        *&v40[92] = 0;
        *&v40[64] = 1065353216;
        *&v40[72] = 0;
        *&v40[76] = 0;
        *&v40[48] = 0;
        v42_lo = v12 * -2.0f;
        *&v40[56] = 0;
        *&v40[60] = 0;
        while (1) {
            *&v40[80] = v15.m128_f32[0];
            *&v40[84] = (v15.m128_f32[0] * v13) - (v12 / v15.m128_f32[0]);
            __m128 v19 = _mm_sub_ps(*(__m128*)&v40[80], *(__m128*)&v40[32]);
            *&v40[68] = (v12 / (v15.m128_f32[0] * v15.m128_f32[0])) + v13;
            *&v40[52] = v16 / ((v15.m128_f32[0] * v15.m128_f32[0]) * v15.m128_f32[0]);
            __m128 v20 = _mm_mul_ps(*(__m128*)&v40[64], v19);
            v41 = v20.m128_f32[0]
                  + (_mm_shuffle_ps(v20, v20, 85).m128_f32[0]
                     + _mm_shuffle_ps(v20, v20, 170).m128_f32[0]);
            __m128 v21 = _mm_mul_ps(*(__m128*)&v40[48], v19);
            v43 = v21.m128_f32[0]
                  + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                     + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]);
            __m128 v22 = _mm_mul_ps(*(__m128*)&v40[64], *(__m128*)&v40[64]);
            v42_hi = v22.m128_f32[0]
                     + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                        + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]);
            float v23 = v46 - v41 / (v42_hi + v43);
            v45 = v23;
            if (fabs(v23 - v46) < 0.001)
                v18 = 1;
            v15.m128_f32[0] = v45;
            ++v17;
            v46 = v45;
            if (v17 > 10 || v18 != 0)
                break;
            v16 = v42_lo;
        }
        double v24 = fabs(v45);
        v45 = (float)v24;
        if (v24 < 1.0 && v45 > 0.001) {
            if (v15.m128_f32[0] < 0.0f)
                v15.m128_f32[0] = -1.0f;
            else
                v15.m128_f32[0] = 1.0f;
        }
        int v25 = rbodies_count;
        *&v40[52] = (v15.m128_f32[0] * v13) - (v12 / v15.m128_f32[0]);
        *&v40[48] = 0;
        *&v40[56] = 0;
        *&v40[60] = 0;
        int v26 = 0;
        __m128 v27 = _mm_add_ps(_mm_mul_ps(*(__m128*)&v40[16], _mm_set1_ps(v15.m128_f32[0])),
                                *(__m128*)&v40[48]);
        *(__m128*)&v40[48] = v27;
        v46 = 0.0f;
        for (int i = 0; i < v25; ++i) {
            rigid_body* v33 = list_rigid_body[i];
            if (v33 != NULL)
                v46 = (1.0f / (v33->m_inv_mass * v33->m_inv_mass)) + v46;
        }
        if (v46 <= 0.0000099999997f) {
            bool v34 = _tlAssert("source/phys_util.cpp", 82, "total_square_mass > 0.00001f", "");
            v25 = rbodies_count;
            v27 = *(__m128*)&v40[48];
            if (v34)
                __debugbreak();
        }
        __m128 v35 = _mm_set1_ps(v44 / v46);
        __m128 v37 = _mm_mul_ps(_mm_sub_ps(v27, *(__m128*)v40), v35);
        *(__m128*)&v40[16] = v37;
        for (int i = 0; i < v25; ++i) {
            rigid_body* v38 = list_rigid_body[i];
            if (v38 != NULL) {
                float inv = v38->m_inv_mass * v38->m_inv_mass;
                math::Dir3 force;
                force.v = _mm_div_ps(v37, _mm_set1_ps(inv));
                v38->add_force(force);
            }
        }
    }
}

// ============================================================================
// calc_velocities (5-arg) Ã¢â‚¬â€ ea: 0x883D40
// ============================================================================
void calc_velocities(const math::Mat43* mat0, const math::Mat43* mat1, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel) {
    if (((unsigned int)t_vel & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)a_vel & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }

    // Relative rotation mat0^-1 * mat1, decomposed to axis-angle * delta_t.
    __m128 v8 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(mat0->x.v.m128_f32[0]), mat1->x.v),
            _mm_mul_ps(_mm_set1_ps(mat0->x.v.m128_f32[1]), mat1->y.v)),
        _mm_mul_ps(_mm_set1_ps(mat0->x.v.m128_f32[2]), mat1->z.v));
    __m128 v9 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(mat0->y.v.m128_f32[0]), mat1->x.v),
            _mm_mul_ps(_mm_set1_ps(mat0->y.v.m128_f32[1]), mat1->y.v)),
        _mm_mul_ps(_mm_set1_ps(mat0->y.v.m128_f32[2]), mat1->z.v));
    __m128 v12 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(mat0->z.v.m128_f32[0]), mat1->x.v),
            _mm_mul_ps(_mm_set1_ps(mat0->z.v.m128_f32[1]), mat1->y.v)),
        _mm_mul_ps(_mm_set1_ps(mat0->z.v.m128_f32[2]), mat1->z.v));

    float v15 = v9.m128_f32[1];
    float v19 = (v12.m128_f32[2] + v15) + v8.m128_f32[0];
    float v16w = v8.m128_f32[2];
    float v16x = v8.m128_f32[0];
    float v17w = v9.m128_f32[0];
    float v17x = v9.m128_f32[1];
    float v18w = v12.m128_f32[1];
    float v18x = v12.m128_f32[2];

    __m128 v20;
    float v21;
    if (v19 >= -0.33333299f) {
        float q0 = 0.5f / sqrt(v19 + 1.0f);
        v20.m128_f32[0] = v17x - v18w;
        v20.m128_f32[1] = v16w - v17w;
        v20.m128_f32[2] = v18x - v16x;
        v20.m128_f32[3] = v19 + 1.0f;
        v21 = q0;
    } else if (v15 <= v8.m128_f32[0]) {
        if (v8.m128_f32[0] > v12.m128_f32[2]) {
            float q1 = 0.5f / sqrt(((v8.m128_f32[0] - v15) - v12.m128_f32[2]) + 1.0f);
            v20.m128_f32[0] = ((v8.m128_f32[0] - v15) - v12.m128_f32[2]) + 1.0f;
            v20.m128_f32[1] = v16x + v18x;
            v20.m128_f32[2] = v17w + v16w;
            v20.m128_f32[3] = v17x - v18w;
            v21 = q1;
        } else {
            float q2 = 0.5f / sqrt(((v12.m128_f32[2] - v8.m128_f32[0]) - v15) + 1.0f);
            v20.m128_f32[0] = v17w + v16w;
            v20.m128_f32[1] = v18w + v17x;
            v20.m128_f32[2] = ((v12.m128_f32[2] - v8.m128_f32[0]) - v15) + 1.0f;
            v20.m128_f32[3] = v16x - v18x;
            v21 = q2;
        }
    } else if (v15 <= v12.m128_f32[2]) {
        float q3 = 0.5f / sqrt(((v12.m128_f32[2] - v8.m128_f32[0]) - v15) + 1.0f);
        v20.m128_f32[0] = v17w + v16w;
        v20.m128_f32[1] = v18w + v17x;
        v20.m128_f32[2] = ((v12.m128_f32[2] - v8.m128_f32[0]) - v15) + 1.0f;
        v20.m128_f32[3] = v16x - v18x;
        v21 = q3;
    } else {
        float q4 = 0.5f / sqrt(((v15 - v8.m128_f32[0]) - v12.m128_f32[2]) + 1.0f);
        v20.m128_f32[0] = v16x + v18x;
        v20.m128_f32[1] = ((v15 - v8.m128_f32[0]) - v12.m128_f32[2]) + 1.0f;
        v20.m128_f32[2] = v18w + v17x;
        v20.m128_f32[3] = v16w - v17w;
        v21 = q4;
    }

    __m128 v22 = _mm_mul_ps(v20, _mm_set1_ps(v21));
    float v37 = _mm_shuffle_ps(v22, v22, 255).m128_f32[0];
    float v34 = (float)fabs(v37);
    float v24;
    if (v34 >= 0.5f) {
        float v35 = sqrt(fabs((1.0f - v34) * 0.5f));
        float v35sq = v35 * v35;
        float v35sq2 = v35sq * v35sq;
        v24 = (v35sq2 * v35sq * -0.1079625f)
              - (v35sq2 * 0.15000001f)
              - (v35sq * 0.33333331f)
              - (v35 * 2.0f)
              + 1.570796f;
    } else {
        float v34sq = v34 * v34;
        float v34sq2 = v34sq * v34sq;
        v24 = (v34sq2 * v34sq * 0.053981241f)
              + (v34sq2 * 0.075000003f)
              + (v34sq * 0.1666667f)
              + v34;
    }
    if (v37 < 0.0f)
        v24 = 0.0f - v24;

    __m128 v25 = _mm_mul_ps(v22, v22);
    float v38 = sqrt(v25.m128_f32[0]
                     + (_mm_shuffle_ps(v25, v25, 85).m128_f32[0]
                        + _mm_shuffle_ps(v25, v25, 170).m128_f32[0]));
    if (v38 <= 0.000099999997f) {
        a_vel->v = Float4_Zero_206.v;
    } else {
        float v26 = ((1.5707964f - v24) * 2.0f) * (-1.0f / v38) * (1.0f / delta_t);
        a_vel->v = _mm_mul_ps(v22, _mm_set1_ps(v26));
    }
    t_vel->v = _mm_mul_ps(_mm_sub_ps(mat1->w.v, mat0->w.v),
                          _mm_set1_ps(1.0f / delta_t));
}

// ============================================================================
// calc_velocities (6-arg) Ã¢â‚¬â€ ea: 0x884360
// ============================================================================
void calc_velocities(const math::Mat43& mat0, const math::Mat43& mat1,
                     const math::Dir3& center_offset_loc, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel) {
    if (((unsigned int)t_vel & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)a_vel & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    calc_velocities(&mat0, &mat1, delta_t, t_vel, a_vel);
    __m128 v6 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(center_offset_loc.v.m128_f32[0]), mat1.x.v),
            _mm_mul_ps(_mm_set1_ps(center_offset_loc.v.m128_f32[1]), mat1.y.v)),
        _mm_mul_ps(_mm_set1_ps(center_offset_loc.v.m128_f32[2]), mat1.z.v));
    t_vel->v = _mm_add_ps(
        t_vel->v,
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(a_vel->v, a_vel->v, 9),
                       _mm_shuffle_ps(v6, v6, 18)),
            _mm_mul_ps(_mm_shuffle_ps(a_vel->v, a_vel->v, 18),
                       _mm_shuffle_ps(v6, v6, 9))));
}

// ============================================================================
// calc_sphere_inertia Ã¢â‚¬â€ ea: 0x884460
// ============================================================================
void calc_sphere_inertia(float radius, math::Dir3* unit_inertia, float* volume) {
    if (((unsigned int)unit_inertia & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    float v4 = ((radius * radius) * radius) * 4.1887898f;
    *volume = v4;
    math::Dir3 v5;
    v5.v.m128_f32[0] = ((v4 * radius) * radius) * 0.40000001f;
    v5.v.m128_f32[1] = v5.v.m128_f32[0];
    v5.v.m128_f32[2] = v5.v.m128_f32[0];
    v5.v.m128_f32[3] = 0.0f;
    unit_inertia->v = v5.v;
}

// ============================================================================
// calc_box_inertia Ã¢â‚¬â€ ea: 0x884500
// ============================================================================
void calc_box_inertia(const math::Dir3* dim, math::Dir3* unit_inertia, float* volume) {
    if (((unsigned int)unit_inertia & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    float dz = _mm_shuffle_ps(dim->v, dim->v, 170).m128_f32[0];
    float dy = _mm_shuffle_ps(dim->v, dim->v, 85).m128_f32[0];
    *volume = (dz * dy) * dim->v.m128_f32[0];

    __m128 v7;
    v7.m128_f32[0] = dim->v.m128_f32[0] * dim->v.m128_f32[0];
    v7.m128_f32[1] = dy * dy;
    v7.m128_f32[2] = dz * dz;
    v7.m128_f32[3] = 0.0f;
    float v5 = ((dz * dy) * dim->v.m128_f32[0]) * 0.083333336f;
    __m128 v6 = _mm_mul_ps(v7, _mm_set1_ps(v5));
    float v10 = _mm_shuffle_ps(v6, v6, 85).m128_f32[0];
    float v8 = _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
    v7.m128_f32[0] = v8 + v10;
    v7.m128_f32[1] = v8 + v6.m128_f32[0];
    v7.m128_f32[2] = v10 + v6.m128_f32[0];
    v7.m128_f32[3] = 0.0f;
    unit_inertia->v = v7;
}

// ============================================================================
// calc_bound_sphere Ã¢â‚¬â€ ea: 0x8846A0
// ============================================================================
void calc_bound_sphere(const math::Dir3* vert_list, int vert_count, float* radius,
                       math::Dir3* com) {
    if (((unsigned int)vert_list & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)com & 0xF) != 0) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                      "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", ""))
            __debugbreak();
    }
    __m128 v8 = Float4_Zero_206.v;
    for (int i = 0; i < vert_count; ++i)
        v8 = _mm_add_ps(v8, vert_list[i].v);
    __m128 v11 = _mm_div_ps(v8, _mm_set1_ps((float)vert_count));
    *radius = 0.0f;
    for (int i = 0; i < vert_count; ++i) {
        __m128 v13 = _mm_sub_ps(vert_list[i].v, v11);
        __m128 v14 = _mm_mul_ps(v13, v13);
        float v15 = v14.m128_f32[0]
                    + (_mm_shuffle_ps(v14, v14, 85).m128_f32[0]
                       + _mm_shuffle_ps(v14, v14, 170).m128_f32[0]);
        if (v15 > *radius)
            *radius = v15;
    }
    *radius = sqrt(*radius);
    com->v = v11;
}

// ============================================================================
// calc_bound_box Ã¢â‚¬â€ ea: 0x8847B0
// ============================================================================
void calc_bound_box(const math::Dir3* vert_list, int vert_count, math::Dir3* dim,
                    math::Dir3* com) {
    if (((unsigned int)vert_list & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)dim & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)com & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    __m128 v4 = _mm_set1_ps(1.0e10f);   // +FLT_MAX-ish (0x4B189680)
    __m128 v5 = _mm_set1_ps(-1.0e10f);  // -FLT_MAX-ish (0xCB189680)
    for (int i = 0; i < vert_count; ++i) {
        __m128 v = vert_list[i].v;
        if (v4.m128_f32[0] > v.m128_f32[0])
            v4.m128_f32[0] = v.m128_f32[0];
        if (v.m128_f32[0] > v5.m128_f32[0])
            v5.m128_f32[0] = v.m128_f32[0];
        float v8 = _mm_shuffle_ps(v, v, 85).m128_f32[0];
        if (v4.m128_f32[1] > v8)
            v4.m128_f32[1] = v8;
        if (v8 > v5.m128_f32[1])
            v5.m128_f32[1] = v8;
        float v9 = _mm_shuffle_ps(v, v, 170).m128_f32[0];
        if (v4.m128_f32[2] > v9)
            v4.m128_f32[2] = v9;
        if (v9 > v5.m128_f32[2])
            v5.m128_f32[2] = v9;
    }
    com->v = _mm_mul_ps(_mm_add_ps(v4, v5), _mm_set1_ps(0.5f));
    dim->v = _mm_sub_ps(v5, v4);
}

}  // namespace nuge
