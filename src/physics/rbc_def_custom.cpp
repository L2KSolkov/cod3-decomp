// ============================================================================
// rbc_def_custom.cpp — custom constraint setup (2 non-inline funcs).
// Source: source/rbc_def_custom.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rbc_def_custom.o):
//   custom_orientation::setup_constraint @0x898910
//   custom_path::setup_constraint        @0x898CF0
// ============================================================================
#include "pulse_sum.h"

#include <math.h>
#include <string.h>

// tl_system.o (tl_xboxr, ported)
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// rigid_body_constraint_custom_orientation::setup_constraint
// ea: 0x898910
// ============================================================================
void rigid_body_constraint_custom_orientation::setup_constraint(pulse_sum_constraint_solver* psys,
                                                                float delta_t) {
    rigid_body* b1 = this->b1;
    if ((~(b1->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", "")) {
        __debugbreak();
    }
    rigid_body* b2 = this->b2;
    if ((~(b2->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", "")) {
        __debugbreak();
    }

    float torque_resistance = this->m_torque_resistance;
    if (torque_resistance > 0.0f) {
        pulse_sum_angular* ps = psys->create_pulse_sum_angular(
            b1, &b1->m_mat.z, b2, &b2->m_mat.z, &b1->m_mat.y, this->m_ps_cache_list);
        ps->m_pulse_sum_min = -10000000.0f;
        ps->m_pulse_sum_max = 10000000.0f;
        float v7 = 1.0f / ((torque_resistance * 10.0f) * delta_t);
        ps->m_right_side = 0.0f;
        ps->m_big_dirt = 0.0f;
        float m_denom = ps->m_denom;
        ps->m_cfm = v7;
        ps->m_denom = m_denom + v7;

        pulse_sum_angular* v9 = psys->create_pulse_sum_angular(
            b1, &b1->m_mat.z, b2, &b2->m_mat.z, &b1->m_mat.x, &this->m_ps_cache_list[1]);
        v9->m_pulse_sum_min = -10000000.0f;
        v9->m_right_side = 0.0f;
        v9->m_big_dirt = 0.0f;
        float v10 = v9->m_denom;
        v9->m_pulse_sum_max = 10000000.0f;
        v9->m_cfm = v7;
        v9->m_denom = v10 + v7;
    }

    if (this->m_active && this->m_upright_strength > 0.0f) {
        const math::Mat43* mat = &b1->m_mat;
        float v24 = fabs(_mm_shuffle_ps(mat->y.v, mat->y.v, 170).m128_f32[0]);
        float v25 = fabs(_mm_shuffle_ps(mat->x.v, mat->x.v, 170).m128_f32[0]);
        float v14;
        if (_mm_shuffle_ps(mat->z.v, mat->z.v, 170).m128_f32[0] >= 0.0f) {
            v14 = v25;
        } else {
            v14 = 1.0f;
            v24 = 1.0f;
        }

        float roll = ((delta_t * this->m_upright_strength) * 30.0f) * v14;
        pulse_sum_angular* v15 = psys->create_pulse_sum_angular(
            b1, &b1->m_mat.z, b2, &b2->m_mat.z, &b1->m_mat.y, &this->m_ps_cache_list[2]);
        double v17;
        if (this->m_no_orientation_correction) {
            float v16 = 0.0f;
            if (_mm_shuffle_ps(b1->m_mat.x.v, b1->m_mat.x.v, 170).m128_f32[0] >= 0.0f) {
                v15->m_pulse_sum_min = 0.0f;
                v16 = roll;
            } else {
                v15->m_pulse_sum_min = 0.0f - roll;
            }
            v15->m_pulse_sum_max = v16;
            v17 = v15->get_pos() * (1.0f / delta_t) * 0.0;
        } else {
            v15->m_pulse_sum_min = 0.0f - roll;
            v15->m_pulse_sum_max = roll;
            v17 = v15->get_pos() * (1.0f / delta_t) * -1.0;
        }
        v15->m_right_side = (float)v17;
        v15->m_big_dirt = 0.0f;
        v15->m_cfm = 0.0f;

        float v18 = ((delta_t * this->m_upright_strength) * 100.0f) * v24;
        pulse_sum_angular* v19 = psys->create_pulse_sum_angular(
            b1, &b1->m_mat.z, b2, &b2->m_mat.z, &b1->m_mat.x, &this->m_ps_cache_list[3]);
        double v21;
        if (this->m_no_orientation_correction) {
            float v20 = v18 * 0.5f;
            if (_mm_shuffle_ps(b1->m_mat.y.v, b1->m_mat.y.v, 170).m128_f32[0] <= 0.0f) {
                v19->m_pulse_sum_min = 0.0f;
                v19->m_pulse_sum_max = v20;
            } else {
                v19->m_pulse_sum_min = 0.0f - v20;
                v19->m_pulse_sum_max = 0.0f;
            }
            v21 = v19->get_pos() * (1.0f / delta_t) * 0.0;
        } else {
            v19->m_pulse_sum_min = 0.0f - v18;
            v19->m_pulse_sum_max = v18;
            v21 = v19->get_pos() * (1.0f / delta_t) * -1.0;
        }
        v19->m_right_side = (float)v21;
        v19->m_big_dirt = 0.0f;
        v19->m_cfm = 0.0f;
    }
}

// ============================================================================
// rigid_body_constraint_custom_path::setup_constraint
// ea: 0x898CF0
// ============================================================================
void rigid_body_constraint_custom_path::setup_constraint(pulse_sum_constraint_solver* psys,
                                                         float delta_t) {
    rigid_body* b1 = this->b1;
    math::Dir3 v33;
    math::Dir3 b1_r;
    v33 = rbint::multiply(b1, this->b1_r_loc);

    math::Dir3 b2_r_4[2];
    memset(b2_r_4, 0, sizeof(b2_r_4));
    b1_r = rbint::multiply(this->b2, *(const math::Dir3*)b2_r_4);

    if (this->m_urb == NULL &&
        _tlAssert("source/rbc_def_custom.cpp", 137, "m_urb", "")) {
        __debugbreak();
    }

    pulse_sum_normal* ps = psys->create_pulse_sum_normal();
    rigid_body* v7 = this->b1;
    rigid_body* b2 = this->b2;
    memset(b2_r_4, 0, sizeof(b2_r_4));
    b2_r_4[1].v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);  // Float4_XAxis
    ps->set(v7, &v33, b2, &b1_r, &b2_r_4[1], this->m_list_psc, (const math::Dir3*)b2_r_4);

    float dt = delta_t;
    if (dt <= 0.0041666669f)
        dt = 0.0041666669f;
    double pos = ps->get_pos();
    double v9 = pos / dt;
    ps->m_right_side = 0.0f;
    ps->m_cfm = 0.0f;
    ps->m_pulse_sum_min = -10000000.0f;
    ps->m_pulse_sum_max = 10000000.0f;
    ps->m_big_dirt = (float)(v9 * -0.5);

    pulse_sum_normal* v10 = psys->create_pulse_sum_normal();
    rigid_body* v11 = this->b1;
    rigid_body* v31 = this->b2;
    memset(b2_r_4, 0, sizeof(b2_r_4));
    b2_r_4[1].v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);  // Float4_YAxis
    v10->set(v11, &v33, v31, &b1_r, &b2_r_4[1], &this->m_list_psc[1], (const math::Dir3*)b2_r_4);

    dt = delta_t;
    if (dt <= 0.0041666669f)
        dt = 0.0041666669f;
    double v12 = v10->get_pos();
    double v13 = v12 / dt;
    v10->m_right_side = 0.0f;
    v10->m_cfm = 0.0f;
    v10->m_pulse_sum_min = -10000000.0f;
    v10->m_pulse_sum_max = 10000000.0f;
    v10->m_big_dirt = (float)(v13 * -0.5);

    rigid_body* v14 = this->b1;
    if ((~(v14->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", "")) {
        __debugbreak();
    }
    rigid_body* v16 = this->b2;
    if ((~(v16->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", "")) {
        __debugbreak();
    }

    b2_r_4[1] = v14->m_mat.x;
    b2_r_4[0] = v16->m_mat.y;

    __m128 v21 = _mm_mul_ps(b2_r_4[1].v, b2_r_4[0].v);
    float v36 = v21.m128_f32[0] + (_mm_shuffle_ps(v21, v21, 85).m128_f32[0]
                                   + _mm_shuffle_ps(v21, v21, 170).m128_f32[0]);
    __m128 v22 = _mm_sub_ps(b2_r_4[1].v, _mm_mul_ps(b2_r_4[0].v, _mm_set1_ps(v36)));
    __m128 v23 = _mm_mul_ps(v22, v22);
    v36 = sqrt(v23.m128_f32[0] + (_mm_shuffle_ps(v23, v23, 85).m128_f32[0]
                                  + _mm_shuffle_ps(v23, v23, 170).m128_f32[0]));
    if (v36 > 0.000099999997f) {
        rigid_body* v24 = this->b2;
        rigid_body* v25 = this->b1;
        math::Dir3 v32;
        v32.v = _mm_div_ps(v22, _mm_set1_ps(v36));
        b2_r_4[0].v = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(b2_r_4[0].v, b2_r_4[0].v, 9),
                       _mm_shuffle_ps(v32.v, v32.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(b2_r_4[0].v, b2_r_4[0].v, 18),
                       _mm_shuffle_ps(v32.v, v32.v, 9)));
        pulse_sum_angular* v26 = psys->create_pulse_sum_angular(
            v25, &b2_r_4[1], v24, &v32, &b2_r_4[0], &this->m_list_psc[2]);
        dt = delta_t;
        if (dt <= 0.0041666669f)
            dt = 0.0041666669f;
        double v28 = v26->get_pos();
        double v29 = v28 / dt;
        v26->m_right_side = 0.0f;
        v26->m_cfm = 0.0f;
        v26->m_pulse_sum_min = -10000000.0f;
        v26->m_pulse_sum_max = 10000000.0f;
        v26->m_big_dirt = (float)(v29 * -0.5);
    }
}
