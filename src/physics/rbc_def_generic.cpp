// ============================================================================
// rbc_def_generic.cpp â€” generic constraint types (16 non-inline funcs).
// Source: source/rbc_def_generic.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rbc_def_generic.o):
//   point::set / epilog_vel_constraint / setup_constraint
//   distance::set / outer_prolog_update / inner_update / outer_epilog_update /
//               setup_constraint
//   hinge::set / do_collision / setup_constraint
//   angular_actuator::set / outer_prolog_update / inner_update /
//               outer_epilog_update / setup_constraint
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
extern void PHYS_ASSERT_ORTHOGONAL(const math::Dir3& v1, const math::Dir3& v2);
extern void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m);
extern const math::Dir3* construct_orth_ud(const math::Dir3* result, const math::Dir3* ud);

// ============================================================================
// rigid_body_constraint_point::set â€” ea: 0x890320
// ============================================================================
void rigid_body_constraint_point::set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc) {
    this->m_b1_r_loc.v = b1_r_loc.v;
    this->m_b2_r_loc.v = b2_r_loc.v;
}

// ============================================================================
// rigid_body_constraint_point::epilog_vel_constraint â€” ea: 0x890370
// ============================================================================
void rigid_body_constraint_point::epilog_vel_constraint(float) {
    this->m_stress = (this->m_ps_cache_list[2].m_pulse_sum * this->m_ps_cache_list[2].m_pulse_sum)
                     + (this->m_ps_cache_list[1].m_pulse_sum * this->m_ps_cache_list[1].m_pulse_sum)
                     + (this->m_ps_cache_list[0].m_pulse_sum * this->m_ps_cache_list[0].m_pulse_sum);
}

// ============================================================================
// rigid_body_constraint_point::setup_constraint â€” ea: 0x890E60
// ============================================================================
void rigid_body_constraint_point::setup_constraint(pulse_sum_constraint_solver* psys,
                                                   float delta_t) {
    math::Dir3 v10;
    math::Dir3 v9;
    const math::Dir3* v7 = rbint::multiply(&v10, this->b2, &this->m_b2_r_loc);
    const math::Dir3* v5 = rbint::multiply(&v9, this->b1, &this->m_b1_r_loc);
    psys->create_point(this->b1, v5, this->b2, v7, this->m_ps_cache_list, delta_t);
}

// ============================================================================
// rigid_body_constraint_distance::set â€” ea: 0x8903B0
// ============================================================================
void rigid_body_constraint_distance::set(const math::Dir3& b1_r_loc,
                                         const math::Dir3& b2_r_loc,
                                         float min_distance, float max_distance) {
    this->m_b1_r_loc.v = b1_r_loc.v;
    this->m_b2_r_loc.v = b2_r_loc.v;
    this->m_min_distance = min_distance;
    this->m_max_distance = max_distance;
    this->m_next_max_distance = max_distance;
    this->m_max_distance_vel = 0.0f;
    this->m_damp_coef = 0.0f;
    this->m_flags = 1;
}

// ============================================================================
// rigid_body_constraint_distance::outer_prolog_update â€” ea: 0x890420
// ============================================================================
void rigid_body_constraint_distance::outer_prolog_update(const outer_time* outside_delta_t) {
    this->m_max_distance_vel = (this->m_next_max_distance - this->m_max_distance)
                               / (rbcint::get_time_scale(this)->m_time * outside_delta_t->m_time);
}

// ============================================================================
// rigid_body_constraint_distance::inner_update â€” ea: 0x890460
// ============================================================================
void rigid_body_constraint_distance::inner_update(float delta_t) {
    this->m_max_distance = (this->m_max_distance_vel * delta_t) + this->m_max_distance;
}

// ============================================================================
// rigid_body_constraint_distance::outer_epilog_update â€” ea: 0x890480
// ============================================================================
void rigid_body_constraint_distance::outer_epilog_update(const outer_time*) {
    this->m_max_distance = this->m_next_max_distance;
}

// ============================================================================
// rigid_body_constraint_distance::setup_constraint â€” ea: 0x891120
// ============================================================================
void rigid_body_constraint_distance::setup_constraint(pulse_sum_constraint_solver* psys,
                                                      float delta_t) {
    if ((this->m_flags & 1) != 0) {
        if (this->m_min_distance < 0.0f &&
            _tlAssert("source/rbc_def_generic.cpp", 155, "m_min_distance >= 0.0f", "")) {
            __debugbreak();
        }
        if (this->m_max_distance < this->m_min_distance &&
            _tlAssert("source/rbc_def_generic.cpp", 156, "m_min_distance <= m_max_distance", "")) {
            __debugbreak();
        }

        math::Dir3 b2_r;
        math::Dir3 v29[4];
        rbint::multiply(&b2_r, this->b1, &this->m_b1_r_loc);
        rbint::multiply(&v29[3], this->b2, &this->m_b2_r_loc);
        rbint::add_pos(&v29[2], this->b1, &b2_r);
        rbint::add_pos(v29, this->b2, &v29[3]);
        __m128 v5 = _mm_sub_ps(v29[0].v, v29[2].v);
        __m128 v6 = _mm_mul_ps(v5, v5);
        float dist_sq = v6.m128_f32[0]
                        + (_mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                           + _mm_shuffle_ps(v6, v6, 170).m128_f32[0]);
        if (dist_sq >= 0.0000010000001f) {
            float inv_len = 1.0f / sqrt(dist_sq);
            v29[1].v = _mm_mul_ps(v5, _mm_set1_ps(inv_len));
            pulse_sum_normal* psn = psys->create_pulse_sum_normal();
            math::Dir3 b1_r_displace;
            b1_r_displace.v = Float4_Zero_213.v;
            math::Dir3 ud;
            ud.v = _mm_xor_ps(Float4_SignMask_213.v, v29[1].v);
            psn->set(this->b1, &b2_r, this->b2, &v29[3], &ud, this->m_ps_cache_list,
                     &b1_r_displace);
            psn->m_pulse_sum_min = -10000000.0f;
            psn->m_pulse_sum_max = 0.0f;
            psn->setup_vel_uni_standard_pos_adjust(delta_t, 0.0f - this->m_max_distance,
                                                   1154777088.0f);
            psn->m_right_side = this->m_max_distance_vel + psn->m_right_side;
            if (this->m_min_distance > 0.0099999998f) {
                pulse_sum_normal* psn2 = psys->create_pulse_sum_normal();
                psn2->set(this->b1, &b2_r, this->b2, &v29[3], &v29[1],
                          &this->m_ps_cache_list[1], &b1_r_displace);
                psn2->m_pulse_sum_min = -887581056.0f;
                psn2->m_pulse_sum_max = 0.0f;
                psn2->setup_vel_uni_standard_pos_adjust(delta_t, this->m_min_distance,
                                                        1154777088.0f);
            }
            if (this->m_damp_coef > 0.0000099999997f
                && dist_sq >= ((this->m_max_distance - 0.1f) * (this->m_max_distance - 0.1f))
                && ((this->m_max_distance + 0.1f) * (this->m_max_distance + 0.1f)) >= dist_sq) {
                math::Dir3 rel_vel;
                const math::Dir3* rv = psn->get_relative_velocity(&rel_vel);
                __m128 v11 = _mm_mul_ps(rv->v, v29[1].v);
                float dot = v11.m128_f32[0]
                            + (_mm_shuffle_ps(v11, v11, 85).m128_f32[0]
                               + _mm_shuffle_ps(v11, v11, 170).m128_f32[0]);
                __m128 v12 = _mm_sub_ps(rv->v, _mm_mul_ps(v29[1].v, _mm_set1_ps(dot)));
                __m128 v13 = _mm_mul_ps(v12, v12);
                float mag_sq = v13.m128_f32[0]
                               + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                                  + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]);
                double v14 = sqrt(mag_sq);
                math::Dir3 dir = rel_vel;
                dir.v = v12;
                float mag = (float)v14;
                if (mag < 0.000099999997f) {
                    const math::Dir3* cd = psn->get_relative_velocity_change_dir(&dir);
                    __m128 v15 = _mm_mul_ps(cd->v, v29[1].v);
                    float dot2 = v15.m128_f32[0]
                                 + (_mm_shuffle_ps(v15, v15, 85).m128_f32[0]
                                    + _mm_shuffle_ps(v15, v15, 170).m128_f32[0]);
                    __m128 v12b = _mm_sub_ps(cd->v, _mm_mul_ps(v29[1].v, _mm_set1_ps(dot2)));
                    __m128 v16 = _mm_mul_ps(v12b, v12b);
                    float mag2 = v16.m128_f32[0]
                                 + (_mm_shuffle_ps(v16, v16, 85).m128_f32[0]
                                    + _mm_shuffle_ps(v16, v16, 170).m128_f32[0]);
                    mag = sqrt(mag2);
                    dir.v = v12b;
                }
                if (mag > 0.000099999997f) {
                    dir.v = _mm_mul_ps(dir.v, _mm_set1_ps(1.0f / mag));
                    pulse_sum_normal* psn3 = psys->create_pulse_sum_normal();
                    math::Dir3 b1_r;
                    math::Dir3 b2_r2;
                    math::Dir3 zero;
                    zero.v = Float4_Zero_213.v;
                    if ((this->m_flags & 2) != 0) {
                        const math::Dir3* sp = rbint::sub_pos(&v29[2], this->b1, v29);
                        b1_r.v = sp->v;
                        b2_r2.v = v29[3].v;
                    } else {
                        b1_r.v = b2_r.v;
                        const math::Dir3* sp = rbint::sub_pos(v29, this->b2, &v29[2]);
                        b2_r2.v = sp->v;
                    }
                    psn3->set(this->b1, &b1_r, this->b2, &b2_r2, &dir,
                              &this->m_ps_cache_list[2], &zero);
                    float v22 = 1.0f / (this->m_damp_coef * delta_t);
                    psn3->m_right_side = 0.0f;
                    psn3->m_big_dirt = 0.0f;
                    float v23 = psn3->m_denom + v22;
                    psn3->m_cfm = v22;
                    psn3->m_pulse_sum_min = -10000000.0f;
                    psn3->m_denom = v23;
                    psn3->m_pulse_sum_max = 10000000.0f;
                }
            }
        }
    }
}

// ============================================================================
// rigid_body_constraint_hinge::set â€” ea: 0x890510
// ============================================================================
void rigid_body_constraint_hinge::set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc,
                                      const math::Dir3& b1_axis_loc,
                                      const math::Dir3& b2_axis_loc,
                                      const math::Dir3& b1_ref_loc,
                                      const math::Dir3& b2_ref_loc,
                                      float theta_min, float theta_max, float damp_k) {
    this->m_flags = 0;
    this->m_b1_r_loc.v = b1_r_loc.v;
    this->m_b2_r_loc.v = b2_r_loc.v;

    // Normalize axes + refs
    __m128 v = b1_axis_loc.v;
    __m128 v13 = _mm_mul_ps(v, v);
    float len1 = sqrt(v13.m128_f32[0]
                      + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                         + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]));
    this->m_b1_axis_loc.v = _mm_div_ps(v, _mm_set1_ps(len1));

    __m128 v18 = b2_axis_loc.v;
    __m128 v19 = _mm_mul_ps(v18, v18);
    float len2 = sqrt(v19.m128_f32[0]
                      + (_mm_shuffle_ps(v19, v19, 85).m128_f32[0]
                         + _mm_shuffle_ps(v19, v19, 170).m128_f32[0]));
    this->m_b2_axis_loc.v = _mm_div_ps(v18, _mm_set1_ps(len2));

    __m128 v23 = b1_ref_loc.v;
    __m128 v24 = _mm_mul_ps(v23, v23);
    float len3 = sqrt(v24.m128_f32[0]
                      + (_mm_shuffle_ps(v24, v24, 85).m128_f32[0]
                         + _mm_shuffle_ps(v24, v24, 170).m128_f32[0]));
    this->m_b1_ref_loc.v = _mm_div_ps(v23, _mm_set1_ps(len3));

    math::Dir3 a1;
    const math::Dir3* v28 = construct_orth_ud(&a1, &this->m_b1_axis_loc);
    this->m_b1_a1_loc.v = v28->v;
    __m128 cross = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 9),
                   _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_axis_loc.v, this->m_b1_axis_loc.v, 18),
                   _mm_shuffle_ps(this->m_b1_a1_loc.v, this->m_b1_a1_loc.v, 9)));
    this->m_b1_a2_loc.v = cross;

    math::Mat43 rot;
    make_rotate(&rot, this->m_b2_axis_loc, theta_min);
    __m128 v33 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v34 = _mm_mul_ps(v33, v33);
    float len4 = sqrt(v34.m128_f32[0]
                      + (_mm_shuffle_ps(v34, v34, 85).m128_f32[0]
                         + _mm_shuffle_ps(v34, v34, 170).m128_f32[0]));
    this->m_b2_ref_min_loc.v = _mm_div_ps(v33, _mm_set1_ps(len4));

    make_rotate(&rot, this->m_b2_axis_loc, theta_max);
    __m128 v39 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[0]), rot.x.v),
            _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[1]), rot.y.v)),
        _mm_mul_ps(_mm_set1_ps(b2_ref_loc.v.m128_f32[2]), rot.z.v));
    __m128 v40 = _mm_mul_ps(v39, v39);
    float len5 = sqrt(v40.m128_f32[0]
                      + (_mm_shuffle_ps(v40, v40, 85).m128_f32[0]
                         + _mm_shuffle_ps(v40, v40, 170).m128_f32[0]));
    this->m_b2_ref_max_loc.v = _mm_div_ps(v39, _mm_set1_ps(len5));

    this->m_damp_k = damp_k;
    PHYS_ASSERT_UNIT(this->m_b1_axis_loc);
    PHYS_ASSERT_UNIT(this->m_b2_axis_loc);
    PHYS_ASSERT_UNIT(this->m_b1_ref_loc);
    PHYS_ASSERT_UNIT(this->m_b1_a1_loc);
    PHYS_ASSERT_UNIT(this->m_b1_a2_loc);
    PHYS_ASSERT_UNIT(this->m_b2_ref_min_loc);
    PHYS_ASSERT_UNIT(this->m_b2_ref_max_loc);
    PHYS_ASSERT_ORTHOGONAL(this->m_b1_axis_loc, this->m_b1_ref_loc);
    PHYS_ASSERT_ORTHOGONAL(this->m_b1_a1_loc, this->m_b1_a2_loc);
}

// ============================================================================
// rigid_body_constraint_hinge::do_collision â€” ea: 0x8908B0
// ============================================================================
void rigid_body_constraint_hinge::do_collision(float) {
    unsigned int m_flags = this->m_flags;
    if ((m_flags & 4) != 0) {
        this->m_flags = m_flags & 0xFFFFFFFC;
        return;
    }
    math::Dir3 v17[5];
    rbint::collide_multiply(&v17[3], this->b1, &this->m_b1_ref_loc);
    rbint::collide_multiply(&v17[2], this->b2, &this->m_b2_ref_min_loc);
    rbint::collide_multiply(&v17[1], this->b2, &this->m_b2_ref_max_loc);
    unsigned int v5 = this->m_flags;
    unsigned int v9;
    if ((v5 & 8) != 0) {
        __m128 v6 = _mm_mul_ps(v17[3].v, v17[2].v);
        float dot_min = v6.m128_f32[0]
                        + (_mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                           + _mm_shuffle_ps(v6, v6, 170).m128_f32[0]);
        __m128 v7 = _mm_mul_ps(v17[3].v, v17[1].v);
        float dot_max = v7.m128_f32[0]
                        + (_mm_shuffle_ps(v7, v7, 85).m128_f32[0]
                           + _mm_shuffle_ps(v7, v7, 170).m128_f32[0]);
        if (dot_min >= dot_max) {
            unsigned int v8 = v5 | 1;
            this->m_flags = v8;
            this->m_flags = v8 & 0xFFFFFFFD;
            return;
        }
        v9 = v5 & 0xFFFFFFFE;
    } else {
        rbint::collide_multiply(v17, this->b2, &this->m_b2_axis_loc);
        __m128 v10 = v17[3].v;
        __m128 v11 = _mm_shuffle_ps(v10, v10, 18);
        __m128 v12 = _mm_shuffle_ps(v10, v10, 9);
        __m128 v13 = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(v12, _mm_shuffle_ps(v17[2].v, v17[2].v, 18)),
                _mm_mul_ps(v11, _mm_shuffle_ps(v17[2].v, v17[2].v, 9))),
            v17[0].v);
        float cross_min = v13.m128_f32[0]
                          + (_mm_shuffle_ps(v13, v13, 85).m128_f32[0]
                             + _mm_shuffle_ps(v13, v13, 170).m128_f32[0]);
        unsigned int v14 = this->m_flags;
        if (cross_min < -0.043618999f)
            v9 = v14 & 0xFFFFFFFE;
        else
            v9 = v14 | 1;
        __m128 v15 = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(v12, _mm_shuffle_ps(v17[1].v, v17[1].v, 18)),
                _mm_mul_ps(v11, _mm_shuffle_ps(v17[1].v, v17[1].v, 9))),
            v17[0].v);
        float cross_max = v15.m128_f32[0]
                          + (_mm_shuffle_ps(v15, v15, 85).m128_f32[0]
                             + _mm_shuffle_ps(v15, v15, 170).m128_f32[0]);
        bool v16 = cross_max > 0.043618999f;
        this->m_flags = v9;
        if (!v16) {
            this->m_flags = v9 | 2;
            return;
        }
        this->m_flags = v9 & 0xFFFFFFFD;
    }
}

// ============================================================================
// rigid_body_constraint_hinge::setup_constraint â€” ea: 0x890ED0
// ============================================================================
void rigid_body_constraint_hinge::setup_constraint(pulse_sum_constraint_solver* psys,
                                                   float delta_t) {
    math::Dir3 v19;
    math::Dir3 b2_ref_max_4;
    math::Dir3 v21;
    math::Dir3 b1_axis_4[2];
    const math::Dir3* v16 = rbint::multiply(&v19, this->b2, &this->m_b2_r_loc);
    const math::Dir3* v5 = rbint::multiply(&b2_ref_max_4, this->b1, &this->m_b1_r_loc);
    psys->create_point(this->b1, v5, this->b2, v16, this->m_ps_cache, delta_t);
    rbint::multiply(&v21, this->b1, &this->m_b1_axis_loc);
    rbint::multiply(&b1_axis_4[1], this->b2, &this->m_b2_axis_loc);
    const math::Dir3* v17 = rbint::multiply(b1_axis_4, this->b1, &this->m_b1_a2_loc);
    const math::Dir3* v6 = rbint::multiply(&v19, this->b1, &this->m_b1_a1_loc);
    psys->create_hinge(this->b1, &v21, this->b2, &b1_axis_4[1], v6, v17,
                       &this->m_ps_cache[4], delta_t);
    if (this->m_damp_k > 0.0000099999997f) {
        b1_axis_4[0].v = Float4_Zero_213.v;
        b2_ref_max_4.v = Float4_Zero_213.v;
        pulse_sum_angular* v9 = psys->create_pulse_sum_angular(
            this->b1, &b2_ref_max_4, this->b2, b1_axis_4, &b1_axis_4[1],
            &this->m_ps_cache[3]);
        v9->m_right_side = 0.0f;
        v9->m_big_dirt = 0.0f;
        v9->m_cfm = 0.0f;
        float v10 = this->m_damp_k * delta_t;
        v9->m_pulse_sum_min = 0.0f - v10;
        v9->m_pulse_sum_max = v10;
    }
    if ((this->m_flags & 3) != 0) {
        rbint::multiply(b1_axis_4, this->b1, &this->m_b1_ref_loc);
        if ((this->m_flags & 1) != 0) {
            rbint::multiply(&v19, this->b2, &this->m_b2_ref_min_loc);
            b2_ref_max_4.v = _mm_xor_ps(Float4_SignMask_213.v, b1_axis_4[1].v);
            pulse_sum_angular* v13 = psys->create_pulse_sum_angular(
                this->b1, b1_axis_4, this->b2, &v19, &b2_ref_max_4,
                &this->m_ps_cache[6]);
            v13->m_pulse_sum_min = -10000000.0f;
            v13->m_pulse_sum_max = 0.0f;
            v13->setup_vel_uni_standard(delta_t, 5.0f);
        }
        if ((this->m_flags & 2) != 0) {
            rbint::multiply(&v19, this->b2, &this->m_b2_ref_max_loc);
            pulse_sum_angular* pa = psys->create_pulse_sum_angular(
                this->b1, b1_axis_4, this->b2, &v19, &b1_axis_4[1],
                &this->m_ps_cache[7]);
            pa->m_pulse_sum_min = -10000000.0f;
            pa->m_pulse_sum_max = 0.0f;
            pa->setup_vel_uni_standard(delta_t, 5.0f);
        }
    }
}

// ============================================================================
// rigid_body_constraint_angular_actuator::set â€” ea: 0x890AB0
// ============================================================================
void rigid_body_constraint_angular_actuator::set(float power, const math::Mat43& target_mat) {
    this->m_power = power;
    this->m_power_scale = 1.0f;
    this->m_target_mat = target_mat;
    this->m_next_target_mat = target_mat;
    this->m_a_vel.v = _mm_setzero_ps();
    this->m_enabled = true;
    PHYS_ASSERT_ORTHONORMAL(&this->m_target_mat);
}

// ============================================================================
// rigid_body_constraint_angular_actuator::outer_prolog_update â€” ea: 0x890C40
// ============================================================================
void rigid_body_constraint_angular_actuator::outer_prolog_update(const outer_time* outside_delta_t) {
    this->m_target_mat.w.v = _mm_setzero_ps();
    this->m_next_target_mat.w.v = _mm_setzero_ps();
    math::Dir3 t_vel;
    const outer_time* time_scale = rbcint::get_time_scale(this);
    nuge::calc_velocities(&this->m_target_mat, &this->m_next_target_mat,
                          time_scale->m_time * outside_delta_t->m_time,
                          &t_vel, &this->m_a_vel);
}

// ============================================================================
// rigid_body_constraint_angular_actuator::inner_update â€” ea: 0x890D20
// ============================================================================
void rigid_body_constraint_angular_actuator::inner_update(float delta_t) {
    math::Mat43 v19;
    make_rotate(&v19, this->m_a_vel, delta_t);
    math::Dir3 v4;
    v4.v = this->m_target_mat.x.v;
    math::Dir3 v9;
    v9.v = this->m_target_mat.y.v;
    math::Dir3 v14;
    v14.v = this->m_target_mat.z.v;
    __m128 v5 = v19.z.v;
    __m128 v11 = v19.y.v;
    __m128 v13 = v19.x.v;
    v19.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 0), v19.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 85), v11)),
        _mm_mul_ps(_mm_shuffle_ps(v4.v, v4.v, 170), v5));
    v19.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v9.v, v9.v, 0), v19.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v9.v, v9.v, 85), v11)),
        _mm_mul_ps(_mm_shuffle_ps(v9.v, v9.v, 170), v19.z.v));
    v19.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 0), v13),
            _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 85), v11)),
        _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 170), v5));
    this->m_target_mat.x.v = v19.y.v;
    this->m_target_mat.y.v = v19.z.v;
    this->m_target_mat.z.v = v19.w.v;
}

// ============================================================================
// rigid_body_constraint_angular_actuator::outer_epilog_update â€” ea: 0x890490
// ============================================================================
void rigid_body_constraint_angular_actuator::outer_epilog_update(const outer_time*) {
    this->m_target_mat = this->m_next_target_mat;
}

// ============================================================================
// rigid_body_constraint_angular_actuator::setup_constraint â€” ea: 0x8915F0
// ============================================================================
void rigid_body_constraint_angular_actuator::setup_constraint(pulse_sum_constraint_solver* psys,
                                                              float delta_t) {
    this->m_ps_cache_list[0].m_pulse_sum = this->m_ps_cache_list[0].m_pulse_sum * 0.89999998f;
    this->m_ps_cache_list[1].m_pulse_sum = this->m_ps_cache_list[1].m_pulse_sum * 0.89999998f;
    this->m_ps_cache_list[2].m_pulse_sum = this->m_ps_cache_list[2].m_pulse_sum * 0.89999998f;
    if (this->m_enabled) {
        float v5 = (this->m_power_scale * this->m_power) * delta_t;
        rigid_body* b2 = this->b2;
        if ((~((b2->m_flags >> 6)) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", "")) {
            __debugbreak();
        }
        rigid_body* b1 = this->b1;
        if ((~(this->b1->m_flags >> 6) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", "")) {
            __debugbreak();
        }
        __m128 x = b1->m_mat.x.v;
        __m128 y = b1->m_mat.y.v;
        __m128 z = b1->m_mat.z.v;
        math::Dir3 v34[3];
        v34[0].v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.x.v.m128_f32[0]), x),
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.x.v.m128_f32[1]), y)),
            _mm_mul_ps(_mm_set1_ps(this->m_target_mat.x.v.m128_f32[2]), z));
        v34[1].v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.y.v.m128_f32[0]), x),
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.y.v.m128_f32[1]), y)),
            _mm_mul_ps(_mm_set1_ps(this->m_target_mat.y.v.m128_f32[2]), z));
        v34[2].v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.z.v.m128_f32[0]), x),
                _mm_mul_ps(_mm_set1_ps(this->m_target_mat.z.v.m128_f32[1]), y)),
            _mm_mul_ps(_mm_set1_ps(this->m_target_mat.z.v.m128_f32[2]), z));
        __m128 a_vel_w = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_set1_ps(this->m_a_vel.v.m128_f32[0]), x),
                _mm_mul_ps(_mm_set1_ps(this->m_a_vel.v.m128_f32[1]), y)),
            _mm_mul_ps(_mm_set1_ps(this->m_a_vel.v.m128_f32[2]), z));

        float v37 = -1.0f / delta_t;
        float v38 = v5;
        float v35 = 0.0f - v38;

        pulse_sum_angular* ps0 = psys->create_pulse_sum_angular(
            this->b1, &v34[0], this->b2, &b2->m_mat.x, &v34[1], this->m_ps_cache_list);
        ps0->m_pulse_sum_min = v35;
        ps0->m_pulse_sum_max = v38;
        double pos0 = ps0->get_pos() * v37;
        ps0->m_big_dirt = 0.0f;
        ps0->m_right_side = (float)pos0;
        ps0->m_cfm = 0.0f;
        __m128 v22 = _mm_mul_ps(a_vel_w, v34[1].v);
        float dot0 = v22.m128_f32[0]
                     + (_mm_shuffle_ps(v22, v22, 85).m128_f32[0]
                        + _mm_shuffle_ps(v22, v22, 170).m128_f32[0]);
        ps0->m_right_side = (float)pos0 - dot0;

        pulse_sum_angular* ps1 = psys->create_pulse_sum_angular(
            this->b1, &v34[1], this->b2, &b2->m_mat.y, &v34[2],
            &this->m_ps_cache_list[1]);
        ps1->m_pulse_sum_min = v35;
        ps1->m_pulse_sum_max = v38;
        double pos1 = ps1->get_pos() * v37;
        ps1->m_right_side = (float)pos1;
        ps1->m_big_dirt = 0.0f;
        ps1->m_cfm = 0.0f;
        __m128 v27 = _mm_mul_ps(a_vel_w, v34[2].v);
        float dot1 = v27.m128_f32[0]
                     + (_mm_shuffle_ps(v27, v27, 85).m128_f32[0]
                        + _mm_shuffle_ps(v27, v27, 170).m128_f32[0]);
        ps1->m_right_side = (float)pos1 - dot1;

        pulse_sum_angular* ps2 = psys->create_pulse_sum_angular(
            this->b1, &v34[2], this->b2, &b2->m_mat.z, &v34[0],
            &this->m_ps_cache_list[2]);
        ps2->m_pulse_sum_min = v35;
        ps2->m_pulse_sum_max = v38;
        double pos2 = ps2->get_pos() * v37;
        ps2->m_big_dirt = 0.0f;
        ps2->m_right_side = (float)pos2;
        ps2->m_cfm = 0.0f;
        __m128 v31 = _mm_mul_ps(a_vel_w, v34[0].v);
        float dot2 = v31.m128_f32[0]
                     + (_mm_shuffle_ps(v31, v31, 85).m128_f32[0]
                        + _mm_shuffle_ps(v31, v31, 170).m128_f32[0]);
        ps2->m_right_side = (float)pos2 - dot2;
    }
}
