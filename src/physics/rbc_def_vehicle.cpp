// ============================================================================
// rbc_def_vehicle.cpp â€” vehicle wheel constraint (10 non-inline funcs).
// Source: source/rbc_def_vehicle.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rbc_def_vehicle.o):
//   set_wheel_state_accelerating @0x884DA0
//   set_wheel_state_braking      @0x884DD0
//   set_no_collision             @0x884DF0
//   set_collision                @0x884E00
//   lerp_float                   @0x884E50
//   set                          @0x884EC0
//   get_wheel_collide_segment    @0x884FE0
//   do_collision                 @0x8850F0
//   epilog_vel_constraint        @0x885200
//   setup_constraint             @0x8854D0
// ============================================================================

#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs (unported physics objects provide definitions later)
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void PHYS_ASSERT_UNIT(const math::Dir3& v);
extern void PHYS_ASSERT_ORTHOGONAL(const math::Dir3& v1, const math::Dir3& v2);
extern const math::Dir3& Float4_SignMask_207;
extern const math::Dir3& Float4_Zero_207;
float lr;    // 0xE53F30 = 1000.0f (overlapping const array)
float lr_0;  // 0xE53F34 = 1.0f
float lr_1;  // 0xE53F38 = 100.0f

// ============================================================================
// pulse_sum_constraint_solver allocation helpers - IDA inline COMDATs
// ============================================================================
pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_wheel_side(
    pulse_sum_wheel* psw) {
    char* addr = (char*)(((intptr_t)m_solver_memory_allocater.m_buffer_cur + 15) & ~15);
    pulse_sum_normal* result;
    if (addr + 160 > m_solver_memory_allocater.m_buffer_end) {
        result = NULL;
    } else {
        m_solver_memory_allocater.m_buffer_cur = addr + 160;
        result = (pulse_sum_normal*)addr;
        if (addr != NULL) {
            psw->m_side = result;
            return result;
        }
    }
    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                  SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
        __debugbreak();
    if (result != NULL) {
        psw->m_side = result;
        return result;
    }
    psw->m_side = NULL;
    return NULL;
}

pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_wheel_fwd(
    pulse_sum_wheel* psw) {
    if (psw->m_side == NULL &&
        _tlAssert("c:/cod/code/tl/physics/include/constraint_solver\\pulse_sum_constraint_solver.h",
                  201, "psw->m_side", defaultFileName))
        __debugbreak();
    char* addr = (char*)(((intptr_t)m_solver_memory_allocater.m_buffer_cur + 15) & ~15);
    pulse_sum_normal* result;
    if (addr + 160 > m_solver_memory_allocater.m_buffer_end) {
        result = NULL;
    } else {
        m_solver_memory_allocater.m_buffer_cur = addr + 160;
        result = (pulse_sum_normal*)addr;
        if (addr != NULL) {
            psw->m_fwd = result;
            return result;
        }
    }
    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                  SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
        __debugbreak();
    if (result != NULL) {
        psw->m_fwd = result;
        return result;
    }
    psw->m_fwd = NULL;
    return NULL;
}

pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_normal() {
    char* addr = (char*)(((intptr_t)m_solver_memory_allocater.m_buffer_cur + 15) & ~15);
    pulse_sum_normal* result;
    if (addr + 160 > m_solver_memory_allocater.m_buffer_end) {
        result = NULL;
    } else {
        m_solver_memory_allocater.m_buffer_cur = addr + 160;
        result = (pulse_sum_normal*)addr;
        if (addr == NULL)
            result = NULL;
    }
    if (result == NULL) {
        if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                      SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
            __debugbreak();
    }
    pulse_sum_normal* last = m_list_pulse_sum_normal.m_last;
    if (last != NULL)
        last->m_link.m_next_link = result;
    else
        m_list_pulse_sum_normal.m_first = result;
    m_list_pulse_sum_normal.m_last = result;
    result->m_link.m_next_link = NULL;
    return result;
}

pulse_sum_wheel* pulse_sum_constraint_solver::create_pulse_sum_wheel() {
    char* addr = (char*)(((intptr_t)m_solver_memory_allocater.m_buffer_cur + 15) & ~15);
    pulse_sum_wheel* result;
    if (addr + 192 > m_solver_memory_allocater.m_buffer_end) {
        result = NULL;
    } else {
        m_solver_memory_allocater.m_buffer_cur = addr + 192;
        result = (pulse_sum_wheel*)addr;
        if (addr == NULL)
            result = NULL;
    }
    if (result == NULL) {
        if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                      SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
            __debugbreak();
    }
    pulse_sum_wheel* last = m_list_pulse_sum_wheel.m_last;
    if (last != NULL)
        last->m_link.m_next_link = result;
    else
        m_list_pulse_sum_wheel.m_first = result;
    m_list_pulse_sum_wheel.m_last = result;
    result->m_link.m_next_link = NULL;
    return result;
}

// ============================================================================
// lerp_float â€” ea: 0x884E50
// ============================================================================
double lerp_float(float tgt, float cur, float rate, float delta_t) {
    float step;
    if ((tgt - cur) <= 0.0f)
        step = 0.0f - (rate * delta_t);
    else
        step = rate * delta_t;
    double v4 = fabs((double)(tgt - cur));
    if (v4 <= 0.050000001 || fabs(step) > v4)
        return tgt;
    else
        return step + cur;
}

// ============================================================================
// game.o 0x0065C4C0
double rigid_body_constraint_wheel::get_displaced_center_dist() const
{
    return m_wheel_displaced_center_dist;
}

// rigid_body_constraint_wheel::set_wheel_state_accelerating â€” ea: 0x884DA0
// ============================================================================
void rigid_body_constraint_wheel::set_wheel_state_accelerating(float desired_speed_k,
                                                               float acceleration_factor_k) {
    this->m_desired_speed_k = desired_speed_k;
    this->m_acceleration_factor_k = acceleration_factor_k;
    this->m_wheel_state = 0;
}

// ============================================================================
// rigid_body_constraint_wheel::set_wheel_state_braking â€” ea: 0x884DD0
// ============================================================================
void rigid_body_constraint_wheel::set_wheel_state_braking(float braking_factor_k) {
    this->m_braking_factor_k = braking_factor_k;
    this->m_wheel_state = 1;
}

// ============================================================================
// rigid_body_constraint_wheel::set_no_collision â€” ea: 0x884DF0
// ============================================================================
void rigid_body_constraint_wheel::set_no_collision() {
    this->m_wheel_flags &= ~1u;
    this->b2 = NULL;
}

// ============================================================================
// rigid_body_constraint_wheel::set_collision â€” ea: 0x884E00
// ============================================================================
void rigid_body_constraint_wheel::set_collision(rigid_body* rb, const math::Dir3* hitp_loc,
                                                const math::Dir3* hitn_loc) {
    this->m_wheel_flags |= 1u;
    this->b2 = rb;
    this->m_b2_hitp_loc.v = hitp_loc->v;
    this->m_b2_hitn_loc.v = hitn_loc->v;
}

// ============================================================================
// rigid_body_constraint_wheel::set â€” ea: 0x884EC0
// ============================================================================
void rigid_body_constraint_wheel::set(const math::Dir3* wheel_center_loc,
                                      const math::Dir3* suspension_dir_loc,
                                      const math::Dir3* wheel_axis_loc, float wheel_radius,
                                      float fwd_fric_k, float side_fric_k,
                                      float suspension_stiffness_k, float suspension_damp_k,
                                      float hard_limit_dist, float roll_stability_factor) {
    this->m_b1_wheel_center_loc.v = wheel_center_loc->v;
    this->m_b1_suspension_dir_loc.v = suspension_dir_loc->v;
    this->m_b1_wheel_axis_loc.v = wheel_axis_loc->v;
    this->m_wheel_radius = wheel_radius;
    this->m_fwd_fric_k = fwd_fric_k;
    this->m_side_fric_k = side_fric_k;
    this->m_suspension_stiffness_k = suspension_stiffness_k;
    this->m_suspension_damp_k = suspension_damp_k;
    this->m_hard_limit_dist = hard_limit_dist;
    this->m_roll_stability_factor = roll_stability_factor;
    this->m_braking_factor_k = 0.0f;
    this->m_wheel_state = 1;
    this->m_wheel_flags = 0;
    this->m_turning_radius_ratio_max_speed = 1.0f;
    this->m_turning_radius_ratio_accel = 1.0f;
    this->m_wheel_vel = 0.0f;
    this->m_wheel_fwd = 0.0f;
    this->m_wheel_displaced_center_dist = 0.0f;
    PHYS_ASSERT_UNIT(this->m_b1_suspension_dir_loc);
    PHYS_ASSERT_UNIT(this->m_b1_wheel_axis_loc);
    PHYS_ASSERT_ORTHOGONAL(this->m_b1_suspension_dir_loc, this->m_b1_wheel_axis_loc);
}

// ============================================================================
// rigid_body_constraint_wheel::get_wheel_collide_segment â€” ea: 0x884FE0
// ============================================================================
void rigid_body_constraint_wheel::get_wheel_collide_segment(const math::Mat43* b1_mat,
                                                            math::Dir3* p0, math::Dir3* p1) {
    if (((unsigned int)p0 & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    if (((unsigned int)p1 & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", "")) {
        __debugbreak();
    }
    math::Dir3 v5;
    math::Dir3 v6;
    v5.v = b1_mat->z.v;
    v6.v = b1_mat->y.v;
    __m128 v7 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_wheel_center_loc.v, this->m_b1_wheel_center_loc.v, 0),
                       b1_mat->x.v),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_wheel_center_loc.v, this->m_b1_wheel_center_loc.v, 85),
                       v6.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_wheel_center_loc.v, this->m_b1_wheel_center_loc.v, 170),
                       v5.v),
            b1_mat->w.v));
    __m128 v8 = _mm_mul_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(this->m_b1_suspension_dir_loc.v, this->m_b1_suspension_dir_loc.v, 0),
                           b1_mat->x.v),
                _mm_mul_ps(_mm_shuffle_ps(this->m_b1_suspension_dir_loc.v, this->m_b1_suspension_dir_loc.v, 85),
                           v6.v)),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_suspension_dir_loc.v, this->m_b1_suspension_dir_loc.v, 170),
                       v5.v)),
        _mm_set1_ps(this->m_wheel_radius));
    p0->v = _mm_sub_ps(v7, v8);
    p1->v = _mm_add_ps(v7, v8);
}

// ============================================================================
// rigid_body_constraint_wheel::do_collision â€” ea: 0x8850F0
// ============================================================================
void rigid_body_constraint_wheel::do_collision(float) {
    if ((this->m_wheel_flags & 1) != 0) {
        math::Dir3 v13[3];
        math::Dir3 v12;
        math::Dir3 v11;
        v13[2] = rbint::collide_multiply(this->b1, this->m_b1_suspension_dir_loc);
        v13[1].v = _mm_mul_ps(v13[2].v, _mm_set1_ps(this->m_wheel_radius));
        v13[0] = rbint::collide_multiply(this->b1, this->m_b1_wheel_center_loc);
        const math::Dir3* v4 = &v13[0];
        v13[0].v = _mm_add_ps(v4->v, v13[1].v);
        v13[1] = rbint::collide_multiply(this->b2, this->m_b2_hitp_loc);
        v12 = rbint::collide_add_pos(this->b2, v13[1]);
        v11 = rbint::collide_add_pos(this->b1, v13[0]);
        const math::Dir3* v5 = &v12;
        const math::Dir3* v6 = &v11;
        __m128 v7 = _mm_mul_ps(_mm_sub_ps(v6->v, v5->v), v13[2].v);
        float v14 = v7.m128_f32[0]
                    + (_mm_shuffle_ps(v7, v7, 85).m128_f32[0]
                       + _mm_shuffle_ps(v7, v7, 170).m128_f32[0]);
        unsigned int m_wheel_flags = this->m_wheel_flags;
        if (v14 < (this->m_hard_limit_dist - 3.4000001f))
            this->m_wheel_flags = m_wheel_flags & 0xFFFFFFFD;
        else
            this->m_wheel_flags = m_wheel_flags | 2;
    }
}

// ============================================================================
// rigid_body_constraint_wheel::epilog_vel_constraint â€” ea: 0x885200
// ============================================================================
void rigid_body_constraint_wheel::epilog_vel_constraint(float delta_t) {
    bool no_collision = (this->m_wheel_flags & 1) == 0;
    float m_wheel_displaced_center_dist = this->m_wheel_displaced_center_dist;
    float prev_wheel_displaced_center_dist = m_wheel_displaced_center_dist;
    if (no_collision) {
        this->m_wheel_displaced_center_dist = 0.0f;
    } else {
        math::Dir3 v24[4];
        math::Dir3 v23;
        math::Dir3 v22;
        v24[2] = rbint::multiply(this->b1, this->m_b1_suspension_dir_loc);
        v24[1].v = _mm_mul_ps(v24[2].v, _mm_set1_ps(this->m_wheel_radius));
        v24[0] = rbint::multiply(this->b1, this->m_b1_wheel_center_loc);
        const math::Dir3* v6 = &v24[0];
        v24[0].v = _mm_add_ps(v6->v, v24[1].v);
        v24[1] = rbint::multiply(this->b2, this->m_b2_hitp_loc);
        v23 = rbint::add_pos(this->b2, v24[1]);
        v22 = rbint::add_pos(this->b1, v24[0]);
        const math::Dir3* v7 = &v23;
        const math::Dir3* v8 = &v22;
        __m128 v9 = _mm_mul_ps(_mm_sub_ps(v8->v, v7->v), v24[2].v);
        float v25 = v9.m128_f32[0]
                    + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                       + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]);
        float m_hard_limit_dist = v25;
        if (v25 > this->m_hard_limit_dist)
            m_hard_limit_dist = this->m_hard_limit_dist;
        this->m_wheel_displaced_center_dist = m_hard_limit_dist;
        m_wheel_displaced_center_dist = prev_wheel_displaced_center_dist;
    }
    if (m_wheel_displaced_center_dist > this->m_wheel_displaced_center_dist) {
        float v12 = m_wheel_displaced_center_dist - (delta_t * 51.0f);
        if (this->m_wheel_displaced_center_dist > v12)
            v12 = this->m_wheel_displaced_center_dist;
        this->m_wheel_displaced_center_dist = v12;
    }
    if (this->m_ps_suspension != NULL)
        this->m_wheel_normal_force = this->m_ps_cache_list[1].m_pulse_sum;
    else
        this->m_wheel_normal_force = 0.0f;

    pulse_sum_normal* m_ps_side_fric = this->m_ps_side_fric;
    if (m_ps_side_fric != NULL) {
        pulse_sum_normal* m_ps_fwd_fric = this->m_ps_fwd_fric;
        if (m_ps_fwd_fric != NULL) {
            if ((m_ps_fwd_fric->m_flags & 4) != 0)
                this->m_wheel_flags |= 4u;
            this->m_wheel_vel = m_ps_fwd_fric->get_unclamped_pulse_sum()
                                * (this->m_wheel_fwd / this->m_wheel_radius)
                                + this->m_wheel_vel;
        } else if ((m_ps_side_fric->m_flags & 2) != 0) {
            this->m_wheel_flags |= 4u;
        }
    }
    if (fabs(this->m_wheel_vel) > 0.34999999f)
        this->m_wheel_pos = (this->m_wheel_vel * delta_t) + this->m_wheel_pos;

    if ((this->m_wheel_flags & 1) == 0) {
        float v15;
        float v16;
        if (this->m_wheel_state != 0) {
            v15 = lr_0;
            v16 = 0.0f;
            if (this->m_braking_factor_k >= 50.0f)
                v15 = lr;
        } else {
            v15 = lr_1;
            if (this->m_desired_speed_k <= 0.0f)
                v16 = -50.0f;
            else
                v16 = 50.0f;
        }
        float m_wheel_vel = this->m_wheel_vel;
        float v18 = v15 * delta_t;
        if ((v16 - m_wheel_vel) <= 0.0f)
            v18 = 0.0f - v18;
        float v25 = v18;
        double v19 = fabs((double)(v16 - m_wheel_vel));
        if (v19 <= 0.050000001 || fabs(v25) > v19)
            this->m_wheel_vel = v16;
        else
            this->m_wheel_vel = v18 + m_wheel_vel;
    }
}

// ============================================================================
// rigid_body_constraint_wheel::setup_constraint â€” ea: 0x8854D0
// ============================================================================
void rigid_body_constraint_wheel::setup_constraint(pulse_sum_constraint_solver* psys,
                                                   float delta_t) {
    unsigned int v5 = this->m_wheel_flags & 0xFFFFFFFB;
    this->m_ps_suspension = NULL;
    this->m_ps_side_fric = NULL;
    this->m_ps_fwd_fric = NULL;
    this->m_wheel_fwd = 0.0f;
    this->m_wheel_flags = v5;
    if ((v5 & 1) != 0) {
        math::Dir3 v58[4];
        math::Dir3 v56[4];
        math::Dir3 v57[2];
        math::Dir3 v55;
        v58[2] = rbint::multiply(this->b1, this->m_b1_suspension_dir_loc);
        v56[2].v = _mm_mul_ps(v58[2].v, _mm_set1_ps(this->m_wheel_radius));
        v58[1] = rbint::multiply(this->b1, this->m_b1_wheel_center_loc);
        const math::Dir3* v6 = &v58[1];
        v58[0].v = _mm_add_ps(v6->v, v56[2].v);
        v58[3] = rbint::multiply(this->b2, this->m_b2_hitp_loc);
        v57[1] = rbint::add_pos(this->b2, v58[3]);
        v58[1] = rbint::add_pos(this->b1, v58[0]);
        const math::Dir3* v7 = &v57[1];
        __m128 v = v58[1].v;
        unsigned int m_wheel_flags = this->m_wheel_flags;
        v58[1].v = _mm_sub_ps(v, v7->v);
        if ((m_wheel_flags & 2) != 0) {
            v57[1] = rbint::multiply(this->b2, this->m_b2_hitn_loc);
            v55.v = _mm_xor_ps(Float4_SignMask_207.v, v57[1].v);
            pulse_sum_normal* pulse_sum_normal = psys->create_pulse_sum_normal();
            rigid_body* v11 = this->b2;
            rigid_body* v12 = this->b1;
            v56[2].v = Float4_Zero_207.v;
            v57[1].v = _mm_sub_ps(v58[0].v,
                                  _mm_mul_ps(v58[2].v,
                                             _mm_set1_ps(this->m_hard_limit_dist)));
            pulse_sum_normal->set(v12, &v57[1], v11, &v58[3], &v55, this->m_ps_cache_list,
                                  &v56[2]);
            pulse_sum_normal->m_pulse_sum_min = -10000000.0f;
            pulse_sum_normal->m_pulse_sum_max = 0.0f;
            pulse_sum_normal->setup_vel_uni_standard(delta_t, 170.0f);
        }
        v57[1] = rbint::multiply(this->b2, this->m_b2_hitn_loc);
        __m128 v13 = _mm_xor_ps(Float4_SignMask_207.v, v57[1].v);
        __m128 v14 = _mm_mul_ps(v58[1].v, v13);
        float v59 = v14.m128_f32[0]
                    + (_mm_shuffle_ps(v14, v14, 85).m128_f32[0]
                       + _mm_shuffle_ps(v14, v14, 170).m128_f32[0]);
        v57[0].v = v13;
        __m128 v15 = _mm_mul_ps(v13, _mm_set1_ps(v59));
        v58[0].v = _mm_sub_ps(v58[0].v, v15);
        v58[3].v = _mm_sub_ps(v58[3].v, v15);
        __m128 v16 = _mm_setzero_ps();
        v16.m128_f32[0] = 0.0f - this->m_roll_stability_factor;
        v56[1].v = _mm_mul_ps(v58[2].v, _mm_shuffle_ps(v16, v16, 0));
        pulse_sum_wheel* pw = psys->create_pulse_sum_wheel();
        pw->m_side = NULL;
        pw->m_fwd = NULL;
        pulse_sum_wheel* ps_wheel = pw;
        this->m_ps_suspension = &pw->m_suspension;
        this->m_ps_suspension->set(this->b1, v58, this->b2, &v58[3], &v57[0],
                                   &this->m_ps_cache_list[1], &v56[1]);
        this->m_ps_suspension->m_pulse_sum_min = -10000000.0f;
        this->m_ps_suspension->m_pulse_sum_max = 0.0f;
        float m_suspension_damp_k = this->m_suspension_damp_k;
        pulse_sum_normal* v20 = this->m_ps_suspension;
        float v59b = this->m_suspension_stiffness_k * delta_t;
        float v72 = 1.0f / ((m_suspension_damp_k * delta_t) + (v59b * delta_t));
        double pos = v20->get_pos();
        double v22 = v59b * v72;
        float denom = v20->m_denom;
        v20->m_big_dirt = 0.0f;
        v20->m_cfm = v72;
        v20->m_denom = denom + v72;
        v20->m_right_side = (float)(pos * (v22 * delta_t) * (-1.0 / delta_t));

        v58[1] = rbint::multiply(this->b1, this->m_b1_wheel_axis_loc);
        __m128 v23 = _mm_mul_ps(v58[1].v, v57[0].v);
        float v59c = v23.m128_f32[0]
                     + (_mm_shuffle_ps(v23, v23, 85).m128_f32[0]
                        + _mm_shuffle_ps(v23, v23, 170).m128_f32[0]);
        __m128 v24 = _mm_sub_ps(v58[1].v, _mm_mul_ps(v57[0].v, _mm_set1_ps(v59c)));
        __m128 v25 = _mm_mul_ps(v24, v24);
        float v59d = v25.m128_f32[0]
                     + (_mm_shuffle_ps(v25, v25, 85).m128_f32[0]
                        + _mm_shuffle_ps(v25, v25, 170).m128_f32[0]);
        float v72b = sqrt(v59d);
        if (v72b > 0.001f) {
            __m128 v26 = _mm_set1_ps(1.0f / v72b);
            v56[0].v = _mm_mul_ps(v24, v26);
            pulse_sum_normal* pulse_sum_wheel_side =
                psys->create_pulse_sum_wheel_side(ps_wheel);
            this->m_ps_side_fric = pulse_sum_wheel_side;
            this->m_ps_side_fric->set(this->b1, v58, this->b2, &v58[3], &v56[0],
                                      &this->m_ps_cache_list[2], &v56[1]);
            this->m_ps_side_fric->m_right_side = 0.0f;
            this->m_ps_side_fric->m_big_dirt = 0.0f;
            this->m_ps_side_fric->m_cfm = 0.0f;
            v58[2].v = _mm_xor_ps(
                Float4_SignMask_207.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(v56[0].v, v56[0].v, 9),
                               _mm_shuffle_ps(v57[0].v, v57[0].v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(v56[0].v, v56[0].v, 18),
                               _mm_shuffle_ps(v57[0].v, v57[0].v, 9))));
            __m128 v30 = this->m_ps_suspension->get_relative_velocity(&v58[1])->v;
            unsigned int m_wheel_state = this->m_wheel_state;
            __m128 v32 = _mm_mul_ps(v30, v58[2].v);
            float v59e = v32.m128_f32[0]
                         + (_mm_shuffle_ps(v32, v32, 85).m128_f32[0]
                            + _mm_shuffle_ps(v32, v32, 170).m128_f32[0]);
            this->m_wheel_vel = v59e / this->m_wheel_radius;
            if ((m_wheel_state == 0 && this->m_acceleration_factor_k < 0.000099999997f)
                || (m_wheel_state == 1 && this->m_braking_factor_k < 0.000099999997f)) {
                this->m_ps_side_fric->set_pulse_sum_limits_parent_ratio(
                    this->m_side_fric_k, this->m_ps_suspension);
                return;
            }
            pulse_sum_normal* v33 = this->m_ps_side_fric;
            v33->m_pulse_sum_min = -10000000.0f;
            v33->m_pulse_sum_max = 10000000.0f;
            pulse_sum_normal* pulse_sum_wheel_fwd = psys->create_pulse_sum_wheel_fwd(ps_wheel);
            this->m_ps_fwd_fric = pulse_sum_wheel_fwd;
            this->m_ps_fwd_fric->set(this->b1, v58, this->b2, &v58[3], &v58[2],
                                     &this->m_ps_cache_list[3], &v56[1]);
            if (this->m_wheel_state != 0) {
                pulse_sum_normal* m_ps_fwd_fric = this->m_ps_fwd_fric;
                m_ps_fwd_fric->m_right_side = 0.0f;
                m_ps_fwd_fric->m_big_dirt = 0.0f;
                m_ps_fwd_fric->m_cfm = 0.0f;
                float v43 = this->m_braking_factor_k * delta_t;
                this->m_ps_fwd_fric->m_pulse_sum_min = 0.0f - v43;
            } else {
                float v36 = this->m_desired_speed_k * this->m_turning_radius_ratio_max_speed;
                float v37 = ((this->m_acceleration_factor_k * this->m_turning_radius_ratio_accel)
                             * delta_t) / (this->m_wheel_radius * this->m_wheel_radius);
                if (v37 <= 0.000001f) {
                    if (_tlAssert("source/rbc_def_vehicle.cpp", 176, "k_ > .000001f", ""))
                        __debugbreak();
                }
                pulse_sum_normal* v39 = this->m_ps_fwd_fric;
                float m_denom = v39->m_denom;
                float v41 = 1.0f / v37;
                v39->m_right_side = this->m_wheel_radius * v36;
                v39->m_big_dirt = 0.0f;
                v39->m_cfm = v41;
                v39->m_denom = m_denom + v41;
                pulse_sum_normal* v42 = this->m_ps_fwd_fric;
                if (v36 <= 0.000099999997f) {
                    v42->m_pulse_sum_min = -10000000.0f;
                    if (v36 < -0.000099999997f) {
                        v42->m_pulse_sum_max = 0.0f;
                        goto LABEL_21;
                    }
                } else {
                    v42->m_pulse_sum_min = 0.0f;
                }
                v42->m_pulse_sum_max = 10000000.0f;
            }
        LABEL_21:
            ps_wheel->set_side_fwd_ratios(this->m_side_fric_k, this->m_fwd_fric_k);
            const math::Dir3* v45 =
                this->m_ps_fwd_fric->get_relative_velocity_change_dir(&v58[1]);
            __m128 v46 = _mm_mul_ps(v45->v, v58[2].v);
            float v59f = v46.m128_f32[0]
                         + (_mm_shuffle_ps(v46, v46, 85).m128_f32[0]
                            + _mm_shuffle_ps(v46, v46, 170).m128_f32[0]);
            this->m_wheel_fwd = v59f;
        }
    }
}
