// ============================================================================
// rbc_def_ragdoll.cpp â€” ragdoll constraint (14 non-inline funcs).
// Source: source/rbc_def_ragdoll.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rbc_def_ragdoll.o):
//   ragdoll::set / set_damp_k / set_snider_style / set_theta_min_max /
//            set_hinge / set_swivel / add_joint_limit / pull_together /
//            do_collision / setup_hinge / setup_constraint
//   ragdoll_joint_limit_info::set / set_b1_ud_loc / set_theta_limit
// ============================================================================

#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void PHYS_ASSERT_UNIT(const math::Dir3& v);
extern const math::Dir3* construct_orth_ud(const math::Dir3* result, const math::Dir3* ud);

#define MAX_JOINT_LIMITS 2

// ============================================================================
// rigid_body_constraint_ragdoll::set â€” ea: 0x888E80
// ============================================================================
void rigid_body_constraint_ragdoll::set(const math::Dir3& b1_r_loc,
                                        const math::Dir3& b2_r_loc) {
    this->m_b1_r_loc.v = b1_r_loc.v;
    this->m_b2_r_loc.v = b2_r_loc.v;
}

// ============================================================================
// rigid_body_constraint_ragdoll::set_damp_k â€” ea: 0x888ED0
// ============================================================================
void rigid_body_constraint_ragdoll::set_damp_k(float damp_k) {
    unsigned int m_flags = this->m_flags;
    this->m_damp_k = damp_k;
    if (damp_k <= 0.0f)
        this->m_flags = m_flags & 0xFFFFFFBF;
    else
        this->m_flags = m_flags | 0x40;
}

// ============================================================================
// ragdoll_joint_limit_info::set â€” ea: 0x888F00
// ============================================================================
void ragdoll_joint_limit_info::set(const math::Dir3& b1_ud_loc, float theta_limit) {
    float v3 = theta_limit;
    if (theta_limit <= 0.0f) {
        v3 = theta_limit;
        if (_tlAssert("source/rbc_def_ragdoll.cpp", 7, "theta_limit > 0.0f", ""))
            __debugbreak();
    }
    __m128 v5 = _mm_mul_ps(b1_ud_loc.v, b1_ud_loc.v);
    float v6 = v3 - 0.04363323f;
    float len = sqrt(v5.m128_f32[0]
                     + (_mm_shuffle_ps(v5, v5, 85).m128_f32[0]
                        + _mm_shuffle_ps(v5, v5, 170).m128_f32[0]));
    this->m_b1_ud_loc.v = _mm_div_ps(b1_ud_loc.v, _mm_set1_ps(len));
    float v8 = 0.0f;
    this->m_b1_ud_limit_co_ = cos(theta_limit);
    this->m_b1_ud_limit_si_ = sin(theta_limit);
    if (v6 <= 0.0f)
        v8 = v6;
    this->m_b1_ud_active_limit_co_ = cos(v8);
    PHYS_ASSERT_UNIT(this->m_b1_ud_loc);
}

// ============================================================================
// ragdoll_joint_limit_info::set_b1_ud_loc â€” ea: 0x888FD0
// ============================================================================
void ragdoll_joint_limit_info::set_b1_ud_loc(const math::Dir3& b1_ud_loc) {
    __m128 v2 = _mm_mul_ps(b1_ud_loc.v, b1_ud_loc.v);
    float len = sqrt(v2.m128_f32[0]
                     + (_mm_shuffle_ps(v2, v2, 85).m128_f32[0]
                        + _mm_shuffle_ps(v2, v2, 170).m128_f32[0]));
    this->m_b1_ud_loc.v = _mm_div_ps(b1_ud_loc.v, _mm_set1_ps(len));
    PHYS_ASSERT_UNIT(this->m_b1_ud_loc);
}

// ============================================================================
// ragdoll_joint_limit_info::set_theta_limit â€” ea: 0x889040
// ============================================================================
void ragdoll_joint_limit_info::set_theta_limit(float theta_limit) {
    float v2 = theta_limit;
    if (theta_limit <= 0.0f) {
        v2 = theta_limit;
        if (_tlAssert("source/rbc_def_ragdoll.cpp", 23, "theta_limit > 0.0f", ""))
            __debugbreak();
    }
    float v4 = v2 - 0.04363323f;
    this->m_b1_ud_limit_co_ = cos(theta_limit);
    float theta_limita = 0.0f;
    this->m_b1_ud_limit_si_ = sin(theta_limit);
    if (v4 <= 0.0f)
        theta_limita = v4;
    this->m_b1_ud_active_limit_co_ = cos(theta_limita);
}

// ============================================================================
// rigid_body_constraint_ragdoll::set_snider_style â€” ea: 0x8890B0
// ============================================================================
void rigid_body_constraint_ragdoll::set_snider_style(const math::Dir3& b1_axis_loc,
                                                     const math::Dir3& b1_ref_loc) {
    __m128 v = b1_axis_loc.v;
    __m128 v5 = _mm_mul_ps(v, v);
    float len1 = sqrt(v5.m128_f32[0]
                      + (_mm_shuffle_ps(v5, v5, 85).m128_f32[0]
                         + _mm_shuffle_ps(v5, v5, 170).m128_f32[0]));
    this->m_b1_axis_loc.v = _mm_div_ps(v, _mm_set1_ps(len1));
    __m128 v12 = b1_ref_loc.v;
    __m128 v13 = _mm_mul_ps(v12, v12);
    float len2 = sqrt(v13.m128_f32[0]
                      + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                         + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]));
    this->m_b1_ref_loc.v = _mm_div_ps(v12, _mm_set1_ps(len2));
    if ((this->m_flags & 4) != 0) {
        math::Dir3 v20;
        this->m_b1_a1_loc.v = construct_orth_ud(&v20, &this->m_b1_axis_loc)->v;
        __m128 v17 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 9),
                       _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 18),
                       _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 9)));
        this->m_b1_a2_loc.v = v17;
    }
    PHYS_ASSERT_UNIT(this->m_b1_axis_loc);
    PHYS_ASSERT_UNIT(this->m_b1_ref_loc);
}

// ============================================================================
// rigid_body_constraint_ragdoll::set_theta_min_max â€” ea: 0x889230
// ============================================================================
void rigid_body_constraint_ragdoll::set_theta_min_max(const math::Dir3& b2_ref_loc,
                                                      float theta_min, float theta_max) {
    math::Mat43 rot;
    make_rotate(&rot, this->m_b2_axis_loc, theta_min);
    __m128 v7 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v8 = _mm_mul_ps(v7, v7);
    float len = sqrt(v8.m128_f32[0]
                     + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                        + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]));
    this->m_b2_ref_min_loc.v = _mm_div_ps(v7, _mm_set1_ps(len));

    make_rotate(&rot, this->m_b2_axis_loc, theta_max);
    __m128 v12 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v13 = _mm_mul_ps(v12, v12);
    float len2 = sqrt(v13.m128_f32[0]
                      + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                         + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]));
    this->m_b2_ref_max_loc.v = _mm_div_ps(v12, _mm_set1_ps(len2));
}

// ============================================================================
// rigid_body_constraint_ragdoll::set_hinge â€” ea: 0x8893A0
// ============================================================================
void rigid_body_constraint_ragdoll::set_hinge(const math::Dir3& b1_axis_loc,
                                              const math::Dir3& b2_axis_loc,
                                              const math::Dir3& b1_ref_loc,
                                              const math::Dir3& b2_ref_loc,
                                              float theta_min, float theta_max) {
    __m128 v = b1_axis_loc.v;
    __m128 v9 = _mm_mul_ps(v, v);
    float len1 = sqrt(v9.m128_f32[0]
                      + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                         + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]));
    this->m_b1_axis_loc.v = _mm_div_ps(v, _mm_set1_ps(len1));
    __m128 v16 = b2_axis_loc.v;
    __m128 v17 = _mm_mul_ps(v16, v16);
    float len2 = sqrt(v17.m128_f32[0]
                      + (_mm_shuffle_ps(v17, v17, 85).m128_f32[0]
                         + _mm_shuffle_ps(v17, v17, 170).m128_f32[0]));
    this->m_b2_axis_loc.v = _mm_div_ps(v16, _mm_set1_ps(len2));
    __m128 v21 = b1_ref_loc.v;
    __m128 v22 = _mm_mul_ps(v21, v21);
    float len3 = sqrt(v22.m128_f32[0]
                      + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                         + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]));
    this->m_b1_ref_loc.v = _mm_div_ps(v21, _mm_set1_ps(len3));
    this->m_flags |= 4u;
    math::Dir3 v38;
    this->m_b1_a1_loc.v = construct_orth_ud(&v38, &this->m_b1_axis_loc)->v;
    __m128 v39 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 9),
                   _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 18),
                   _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 9)));
    this->m_b1_a2_loc.v = v39;

    math::Mat43 rot;
    make_rotate(&rot, this->m_b2_axis_loc, theta_min);
    __m128 v29 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v30 = _mm_mul_ps(v29, v29);
    float len4 = sqrt(v30.m128_f32[0]
                      + (_mm_shuffle_ps(v30, v30, 85).m128_f32[0]
                         + _mm_shuffle_ps(v30, v30, 170).m128_f32[0]));
    this->m_b2_ref_min_loc.v = _mm_div_ps(v29, _mm_set1_ps(len4));

    make_rotate(&rot, this->m_b2_axis_loc, theta_max);
    __m128 v34 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v35 = _mm_mul_ps(v34, v34);
    float len5 = sqrt(v35.m128_f32[0]
                      + (_mm_shuffle_ps(v35, v35, 85).m128_f32[0]
                         + _mm_shuffle_ps(v35, v35, 170).m128_f32[0]));
    this->m_b2_ref_max_loc.v = _mm_div_ps(v34, _mm_set1_ps(len5));
}

// ============================================================================
// rigid_body_constraint_ragdoll::set_swivel â€” ea: 0x8896A0
// ============================================================================
void rigid_body_constraint_ragdoll::set_swivel(const math::Dir3& b1_axis_loc,
                                               const math::Dir3& b2_axis_loc,
                                               const math::Dir3& b1_ref_loc,
                                               const math::Dir3& b2_ref_loc,
                                               float theta_min, float theta_max) {
    __m128 v = b1_axis_loc.v;
    __m128 v9 = _mm_mul_ps(v, v);
    float len1 = sqrt(v9.m128_f32[0]
                      + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                         + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]));
    this->m_b1_axis_loc.v = _mm_div_ps(v, _mm_set1_ps(len1));
    __m128 v16 = b2_axis_loc.v;
    __m128 v17 = _mm_mul_ps(v16, v16);
    float len2 = sqrt(v17.m128_f32[0]
                      + (_mm_shuffle_ps(v17, v17, 85).m128_f32[0]
                         + _mm_shuffle_ps(v17, v17, 170).m128_f32[0]));
    this->m_b2_axis_loc.v = _mm_div_ps(v16, _mm_set1_ps(len2));
    __m128 v21 = b1_ref_loc.v;
    __m128 v22 = _mm_mul_ps(v21, v21);
    float len3 = sqrt(v22.m128_f32[0]
                      + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                         + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]));
    this->m_b1_ref_loc.v = _mm_div_ps(v21, _mm_set1_ps(len3));
    this->m_flags |= 8u;

    math::Mat43 rot;
    make_rotate(&rot, this->m_b2_axis_loc, theta_min);
    __m128 v26 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v27 = _mm_mul_ps(v26, v26);
    float len4 = sqrt(v27.m128_f32[0]
                      + (_mm_shuffle_ps(v27, v27, 85).m128_f32[0]
                         + _mm_shuffle_ps(v27, v27, 170).m128_f32[0]));
    this->m_b2_ref_min_loc.v = _mm_div_ps(v26, _mm_set1_ps(len4));

    make_rotate(&rot, this->m_b2_axis_loc, theta_max);
    __m128 v31 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v32 = _mm_mul_ps(v31, v31);
    float len5 = sqrt(v32.m128_f32[0]
                      + (_mm_shuffle_ps(v32, v32, 85).m128_f32[0]
                         + _mm_shuffle_ps(v32, v32, 170).m128_f32[0]));
    this->m_b2_ref_max_loc.v = _mm_div_ps(v31, _mm_set1_ps(len5));
}

// ============================================================================
// rigid_body_constraint_ragdoll::add_joint_limit â€” ea: 0x889930
// ============================================================================
void rigid_body_constraint_ragdoll::add_joint_limit(const math::Dir3& b1_ud_loc,
                                                    float theta_limit) {
    if (this->m_joint_limits_count >= MAX_JOINT_LIMITS &&
        _tlAssert("source/rbc_def_ragdoll.cpp", 102,
                  "m_joint_limits_count < MAX_JOINT_LIMITS", "")) {
        __debugbreak();
    }
    this->m_joint_limits[this->m_joint_limits_count].set(b1_ud_loc, theta_limit);
    ++this->m_joint_limits_count;
}

// ============================================================================
// rigid_body_constraint_ragdoll::pull_together â€” ea: 0x889990
// ============================================================================
const float& rigid_body_constraint_ragdoll::pull_together() {
    math::Dir3 v1;
    v1.v = this->m_b2_r_loc.v;
    rigid_body* b2 = this->b2;
    __m128 v3 = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 170), b2->m_mat.z.v),
                           b2->m_mat.w.v);
    __m128 v4 = _mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 85), b2->m_mat.y.v);
    __m128 v5 = _mm_shuffle_ps(v1.v, v1.v, 0);
    math::Dir3 v6;
    v6.v = this->m_b1_r_loc.v;
    __m128 v7 = _mm_sub_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(v6.v, v6.v, 0), this->b1->m_mat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(v6.v, v6.v, 85), this->b1->m_mat.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v6.v, v6.v, 170), this->b1->m_mat.z.v),
                       this->b1->m_mat.w.v)),
        _mm_add_ps(_mm_add_ps(_mm_mul_ps(v5, b2->m_mat.x.v), v4), v3));
    __m128 v8 = _mm_add_ps(b2->m_mat.w.v, v7);
    __m128 v9 = _mm_mul_ps(v7, v7);
    b2->m_mat.w.v = v8;
    static float result;
    result = v9.m128_f32[0]
             + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]);
    return result;
}

// ============================================================================
// rigid_body_constraint_ragdoll::do_collision â€” ea: 0x889A60
// ============================================================================
void rigid_body_constraint_ragdoll::do_collision(float) {
    unsigned int m_flags = this->m_flags;
    if ((m_flags & 0x80u) == 0) {
        math::Dir3 v19[6];
        math::Dir3 v20[4];
        v19[4] = rbint::collide_multiply(this->b2, this->m_b2_ref_min_loc);
        v20[0] = rbint::collide_multiply(this->b2, this->m_b2_ref_max_loc);
        v20[2] = rbint::collide_multiply(this->b2, this->m_b2_axis_loc);
        v20[1] = rbint::collide_multiply(this->b1, this->m_b1_ref_loc);
        __m128 v11;
        if ((this->m_flags & 8) != 0) {
            int v6 = 0;
            if (this->m_joint_limits_count > 0) {
                int count = this->m_joint_limits_count;
                int idx = 0;
                do {
                    v19[5] = rbint::collide_multiply(
                        this->b1, this->m_joint_limits[idx].m_b1_ud_loc);
                    const math::Dir3* v7 = &v19[5];
                    __m128 v8 = _mm_mul_ps(v20[2].v, v7->v);
                    float dot = v8.m128_f32[0]
                                + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                                   + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
                    this->set_joint_limit_active(v6, this->m_joint_limits[idx].m_b1_ud_limit_si_ >= dot);
                    ++v6;
                    ++idx;
                } while (v6 < count);
            }
            v19[5] = rbint::collide_multiply(this->b1, this->m_b1_axis_loc);
            const math::Dir3* v10 = &v19[5];
            math::Mat43 rot;
            make_rotate(&rot, *v10, v20[2]);
            v11 = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v20[1].v, v20[1].v, 0), rot.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(v20[1].v, v20[1].v, 85), rot.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(v20[1].v, v20[1].v, 170), rot.z.v));
        } else {
            v11 = v20[1].v;
        }
        __m128 v12 = _mm_shuffle_ps(v11, v11, 18);
        __m128 v13 = _mm_shuffle_ps(v11, v11, 9);
        __m128 v14 = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(v13, _mm_shuffle_ps(v19[4].v, v19[4].v, 18)),
                _mm_mul_ps(v12, _mm_shuffle_ps(v19[4].v, v19[4].v, 9))),
            v20[2].v);
        float cross_min = v14.m128_f32[0]
                          + (_mm_shuffle_ps(v14, v14, 85).m128_f32[0]
                             + _mm_shuffle_ps(v14, v14, 170).m128_f32[0]);
        unsigned int v15 = this->m_flags;
        unsigned int v16;
        if (cross_min < -0.043618999f)
            v16 = v15 & 0xFFFFFFEF;
        else
            v16 = v15 | 0x10;
        __m128 v17 = _mm_mul_ps(
            _mm_sub_ps(_mm_mul_ps(v13, _mm_shuffle_ps(v20[0].v, v20[0].v, 18)),
                       _mm_mul_ps(v12, _mm_shuffle_ps(v20[0].v, v20[0].v, 9))),
            v20[2].v);
        float cross_max = v17.m128_f32[0]
                          + (_mm_shuffle_ps(v17, v17, 85).m128_f32[0]
                             + _mm_shuffle_ps(v17, v17, 170).m128_f32[0]);
        bool v18 = cross_max > 0.043618999f;
        this->m_flags = v16;
        if (v18)
            this->m_flags = v16 & 0xFFFFFFDF;
        else
            this->m_flags = v16 | 0x20;
    } else {
        this->m_flags = m_flags | 0x30;
        for (int i = 0; i < this->m_joint_limits_count; ++i) {
            if (i >= MAX_JOINT_LIMITS &&
                _tlAssert("c:/cod/code/tl/physics/include/rbc_defs\\rbc_def_ragdoll.h", 47,
                          "f >= 0 && f < MAX_JOINT_LIMITS", "")) {
                __debugbreak();
            }
            this->m_flags |= 1 << i;
        }
    }
}

// ============================================================================
// rigid_body_constraint_ragdoll::setup_hinge â€” ea: 0x889D10
// ============================================================================
void rigid_body_constraint_ragdoll::setup_hinge(pulse_sum_constraint_solver* psys,
                                                const math::Dir3& b1_ref,
                                                const math::Dir3& b2_axis, float delta_t) {
    math::Dir3 v11;
    if ((this->m_flags & 0x10) != 0) {
        v11 = rbint::multiply(this->b2, this->m_b2_ref_min_loc);
        math::Dir3 ud;
        ud.v = _mm_xor_ps(Float4_SignMask_210.v, b2_axis.v);
        pulse_sum_angular* v7 = psys->create_pulse_sum_angular(
            this->b1, &b1_ref, this->b2, &v11, &ud, &this->m_ps_cache_list[6]);
        v7->m_pulse_sum_min = -10000000.0f;
        v7->m_pulse_sum_max = 0.0f;
        v7->setup_vel_uni_standard(delta_t, 5.0f);
    }
    if ((this->m_flags & 0x20) != 0) {
        v11 = rbint::multiply(this->b2, this->m_b2_ref_max_loc);
        pulse_sum_angular* pa = psys->create_pulse_sum_angular(
            this->b1, &b1_ref, this->b2, &v11, &b2_axis, &this->m_ps_cache_list[7]);
        pa->m_pulse_sum_min = -10000000.0f;
        pa->m_pulse_sum_max = 0.0f;
        pa->setup_vel_uni_standard(delta_t, 5.0f);
    }
}

// ============================================================================
// rigid_body_constraint_ragdoll::setup_constraint â€” ea: 0x889E10
// ============================================================================
void rigid_body_constraint_ragdoll::setup_constraint(pulse_sum_constraint_solver* psys,
                                                     float delta_t) {
    math::Dir3 v40[4];
    math::Dir3 b1_axis_4;
    v40[0] = rbint::multiply(this->b2, this->m_b2_r_loc);
    const math::Dir3* v34 = &v40[0];
    b1_axis_4 = rbint::multiply(this->b1, this->m_b1_r_loc);
    const math::Dir3* v5 = &b1_axis_4;
    psys->create_point(this->b1, v5, this->b2, v34, this->m_ps_cache_list, delta_t);
    math::Dir3 v37;
    v37 = rbint::multiply(this->b1, this->m_b1_axis_loc);
    v40[2] = rbint::multiply(this->b2, this->m_b2_axis_loc);

    if ((this->m_flags & 0x40) != 0) {
        rigid_body* b1 = this->b1;
        rigid_body* v8 = this->b2;
        __m128 v9 = _mm_sub_ps(v8->m_a_vel.v, this->b1->m_a_vel.v);
        __m128 v10 = _mm_mul_ps(v9, v9);
        float mag_sq = v10.m128_f32[0]
                       + (_mm_shuffle_ps(v10, v10, 85).m128_f32[0]
                          + _mm_shuffle_ps(v10, v10, 170).m128_f32[0]);
        math::Dir3 dir;
        dir.v = v9;
        float mag = sqrt(mag_sq);
        if (mag <= 0.000099999997f) {
            dir.v = v40[2].v;
        } else {
            dir.v = _mm_div_ps(v9, _mm_set1_ps(mag));
        }
        math::Dir3 zero;
        zero.v = Float4_Zero_210.v;
        pulse_sum_angular* pa = psys->create_pulse_sum_angular(
            b1, &dir, v8, &zero, &v40[2], &this->m_ps_cache_list[3]);
        if ((this->m_flags & 0x200) != 0) {
            float v12 = 1.0f / (this->m_damp_k * delta_t);
            pa->m_right_side = 0.0f;
            pa->m_big_dirt = 0.0f;
            float v13 = pa->m_denom + v12;
            pa->m_cfm = v12;
            pa->m_pulse_sum_min = -10000000.0f;
            pa->m_denom = v13;
            pa->m_pulse_sum_max = 10000000.0f;
        } else {
            pa->m_right_side = 0.0f;
            pa->m_big_dirt = 0.0f;
            pa->m_cfm = 0.0f;
            float v14 = this->m_damp_k * delta_t;
            pa->m_pulse_sum_min = 0.0f - v14;
            pa->m_pulse_sum_max = v14;
        }
    }
    if ((this->m_flags & 4) != 0) {
        v40[0] = rbint::multiply(this->b1, this->m_b1_a2_loc);
        const math::Dir3* v35 = &v40[0];
        b1_axis_4 = rbint::multiply(this->b1, this->m_b1_a1_loc);
        const math::Dir3* v15 = &b1_axis_4;
        psys->create_hinge(this->b1, &v37, this->b2, &v40[2], v15, v35,
                           &this->m_ps_cache_list[4], delta_t);
        if ((this->m_flags & 0x30) != 0) {
            v40[0] = rbint::multiply(this->b1, this->m_b1_ref_loc);
            const math::Dir3* v16 = &v40[0];
            this->setup_hinge(psys, *v16, v40[2], delta_t);
        }
    }
    unsigned int m_flags = this->m_flags;
    if ((m_flags & 8) != 0) {
        if ((m_flags & 0x100) == 0) {
            int count = this->m_joint_limits_count;
            int idx = 0;
            while (idx < count) {
                if (idx >= MAX_JOINT_LIMITS &&
                    _tlAssert("c:/cod/code/tl/physics/include/rbc_defs\\rbc_def_ragdoll.h", 48,
                              "f >= 0 && f < MAX_JOINT_LIMITS", "")) {
                    __debugbreak();
                }
                if (((1 << idx) & this->m_flags) != 0) {
                    v40[0] = rbint::multiply(this->b1, this->m_joint_limits[idx].m_b1_ud_loc);
                    __m128 v21 = _mm_mul_ps(v40[2].v, v40[0].v);
                    float dot = v21.m128_f32[0]
                                + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                                   + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]);
                    __m128 v22 = _mm_sub_ps(v40[2].v, _mm_mul_ps(v40[0].v, _mm_set1_ps(dot)));
                    __m128 v23 = _mm_mul_ps(v22, v22);
                    float mag_sq = v23.m128_f32[0]
                                   + (_mm_shuffle_ps(v23, v23, 85).m128_f32[0]
                                      + _mm_shuffle_ps(v23, v23, 170).m128_f32[0]);
                    float mag = sqrt(mag_sq);
                    if (mag >= 0.000099999997f) {
                        float si = this->m_joint_limits[idx].m_b1_ud_limit_si_;
                        float co = this->m_joint_limits[idx].m_b1_ud_limit_co_;
                        math::Dir3 v26;
                        v26.v = _mm_add_ps(
                            _mm_mul_ps(v22, _mm_set1_ps(si / mag)),
                            _mm_mul_ps(v40[0].v, _mm_set1_ps(co)));
                        math::Dir3 ud;
                        ud.v = _mm_mul_ps(
                            _mm_sub_ps(
                                _mm_mul_ps(_mm_shuffle_ps(v26.v, v26.v, 9),
                                           _mm_shuffle_ps(v40[0].v, v40[0].v, 18)),
                                _mm_mul_ps(_mm_shuffle_ps(v26.v, v26.v, 18),
                                           _mm_shuffle_ps(v40[0].v, v40[0].v, 9))),
                            _mm_set1_ps(1.0f / si));
                        PHYS_ASSERT_UNIT(ud);
                        pulse_sum_angular* v30 = psys->create_pulse_sum_angular(
                            this->b1, &v26, this->b2, &v40[2], &ud,
                            &this->m_ps_cache_list[8 + idx]);
                        v30->m_pulse_sum_min = -10000000.0f;
                        v30->m_pulse_sum_max = 0.0f;
                        v30->setup_vel_uni_standard(delta_t, 5.0f);
                    }
                }
                ++idx;
            }
        }
        if ((this->m_flags & 0x30) != 0) {
            math::Mat43 rot;
            make_rotate(&rot, v37, v40[2]);
            b1_axis_4 = rbint::multiply(this->b1, this->m_b1_ref_loc);
            const math::Dir3* v32 = &b1_axis_4;
            math::Dir3 b1_ref;
            b1_ref.v = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_set1_ps(v32->v.m128_f32[0]), rot.x.v),
                    _mm_mul_ps(_mm_set1_ps(v32->v.m128_f32[1]), rot.y.v)),
                _mm_mul_ps(_mm_set1_ps(v32->v.m128_f32[2]), rot.z.v));
            this->setup_hinge(psys, b1_ref, v40[2], delta_t);
        }
    }
}
