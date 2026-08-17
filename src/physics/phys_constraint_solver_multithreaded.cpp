// ============================================================================
// phys_constraint_solver_multithreaded.cpp - pulse-sum constraint solver
// (18 non-inline funcs). Source: source/phys_constraint_solver_multithreaded.cpp
// (phys_xboxr:phys_constraint_solver_multithreaded.o).
// Verified against IDA (phys_xboxr:phys_constraint_solver_multithreaded.o).
// ============================================================================

#include "physics_system.h"
#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <new>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void PHYS_ASSERT_UNIT(const math::Dir3* v);
extern const math::Dir3& Float4_SignMask_214;
extern math::Dir3 construct_orth_ud(const math::Dir3& ud);

const __m128 Float4_XAxis_214 = {1.0f, 0.0f, 0.0f, 0.0f};
const __m128 Float4_YAxis_214 = {0.0f, 1.0f, 0.0f, 0.0f};
const __m128 Float4_ZAxis_214 = {0.0f, 0.0f, 1.0f, 0.0f};

// ============================================================================
// phys_constraint_solver_multithreaded::init / shutdown - ea: 0x8932B0/0x8932C0
// ============================================================================
namespace phys_constraint_solver_multithreaded {
void init() {
}

void shutdown() {
}
}

// ============================================================================
// pulse_sum_normal::get_vel - ea: 0x8932D0
// ============================================================================
float pulse_sum_normal::get_vel() {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_ap.v;
    }
    rigid_body* m_rb = m_b1->m_rb;
    __m128 v6 = _mm_mul_ps(
        _mm_sub_ps(
            _mm_add_ps(
                m_rb->m_t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v, m_rb->m_a_vel.v, 9),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v, m_rb->m_a_vel.v, 18),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
            b2_vel.v),
        m_ud.v);
    return v6.m128_f32[0] + _mm_shuffle_ps(v6, v6, 85).m128_f32[0] +
           _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
}

// ============================================================================
// pulse_sum_normal::get_last_vel - ea: 0x8933C0
// ============================================================================
float pulse_sum_normal::get_last_vel() {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_last_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_ap.v;
    }
    rigid_body* m_rb = m_b1->m_rb;
    __m128 v6 = _mm_mul_ps(
        _mm_sub_ps(
            _mm_add_ps(
                m_rb->m_last_t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_last_a_vel.v, m_rb->m_last_a_vel.v, 9),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_last_a_vel.v, m_rb->m_last_a_vel.v, 18),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
            b2_vel.v),
        m_ud.v);
    return v6.m128_f32[0] + _mm_shuffle_ps(v6, v6, 85).m128_f32[0] +
           _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
}

// ============================================================================
// pulse_sum_normal::get_pos - ea: 0x8934B0
// ============================================================================
float pulse_sum_normal::get_pos() {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_pos;
    if (m_b2 != NULL) {
        rigid_body* m_rb = m_b2->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", ""))
            __debugbreak();
        b2_pos.v = _mm_add_ps(m_rb->m_mat.w.v, m_b2_r.v);
    } else {
        b2_pos.v = m_b2_r.v;
    }
    rigid_body* v5 = m_b1->m_rb;
    if ((~(v5->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    __m128 v6 = _mm_mul_ps(
        _mm_sub_ps(_mm_add_ps(v5->m_mat.w.v, m_b1_r.v), b2_pos.v), m_ud.v);
    return v6.m128_f32[0] + _mm_shuffle_ps(v6, v6, 85).m128_f32[0] +
           _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
}

// ============================================================================
// pulse_sum_normal::set - ea: 0x8935A0
// ============================================================================
void pulse_sum_normal::set(rigid_body* const b1, const math::Dir3* b1_r,
                           rigid_body* const b2, const math::Dir3* b2_r,
                           const math::Dir3* ud, pulse_sum_cache* const ps_cache,
                           const math::Dir3* b1_r_displace) {
    if (ps_cache == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 153,
                  "ps_cache", ""))
        __debugbreak();
    if (b1 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 154,
                  "b1", ""))
        __debugbreak();
    if (b2 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 155,
                  "b2", ""))
        __debugbreak();
    if (b1 == b2 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 156,
                  "b1 != b2", ""))
        __debugbreak();
    PHYS_ASSERT_UNIT(ud);

    rigid_body* v10 = b1;
    if (b1->m_node != NULL)
        goto check_flags;
    if (b2->m_node == NULL) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 158,
                      "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)", ""))
            __debugbreak();
        v10 = b1;
    }
check_flags:
    if (v10->m_node != NULL)
        v10->m_flags;  // (flags check below)
    unsigned int v12 = (v10->m_node != NULL) ? ((v10->m_flags & 0x30) == 0)
                                             : (v10->m_flags & 0x30);
    if (v12 == 0 ||
        (b2->m_node == NULL ? (b2->m_flags & 0x30) : ((b2->m_flags & 0x30) == 0)) == 0) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 159,
                      "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)", ""))
            __debugbreak();
        v10 = b1;
    }
    pulse_sum_node* m_node = v10->m_node;
    if (m_node != NULL) {
        m_b1 = m_node;
        m_b1_r.v = b1_r->v;
        pulse_sum_node* v16 = b2->m_node;
        if (v16 != NULL) {
            m_b2 = v16;
            m_b2_r.v = b2_r->v;
        } else {
            m_b2 = NULL;
            math::Dir3 v25;
            v25 = rbint::gtv(b2, *b2_r);
            set_object_vel(&v25);
            math::Dir3 v24;
            v24 = rbint::add_pos(b2, *b2_r);
            set_object_col_pt(&v24);
        }
        m_ud.v = ud->v;
    } else {
        m_b1 = b2->m_node;
        m_b1_r.v = b2_r->v;
        m_b2 = NULL;
        math::Dir3 v24;
        v24 = rbint::gtv(v10, *b1_r);
        set_object_vel(&v24);
        math::Dir3 v25;
        v25 = rbint::add_pos(b1, *b1_r);
        set_object_col_pt(&v25);
        m_ud.v = _mm_xor_ps(Float4_SignMask_214.v, ud->v);
    }
    m_pulse_sum_cache = ps_cache;
    m_flags = 0;
    calc_abs(b1_r_displace);
}

// ============================================================================
// pulse_sum_normal::get_relative_velocity_change_dir - ea: 0x893890
// ============================================================================
const math::Dir3* pulse_sum_normal::get_relative_velocity_change_dir(math::Dir3* result) {
    __m128 m_inv_mass_low = _mm_set_ss(m_b1->m_inv_mass);
    pulse_sum_node* m_b2 = this->m_b2;
    __m128 v4 = _mm_add_ps(
        _mm_mul_ps(m_ud.v, _mm_shuffle_ps(m_inv_mass_low, m_inv_mass_low, 0)),
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_b1_ap.v, m_b1_ap.v, 9),
                       _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(m_b1_ap.v, m_b1_ap.v, 18),
                       _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9))));
    math::Dir3 v6;
    v6.v = v4;
    if (m_b2 != NULL)
        v6.v = _mm_add_ps(
            v4,
            _mm_add_ps(
                _mm_mul_ps(m_ud.v, _mm_shuffle_ps(_mm_set_ss(m_b2->m_inv_mass), _mm_set_ss(m_b2->m_inv_mass), 0)),
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2_ap.v, m_b2_ap.v, 9),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2_ap.v, m_b2_ap.v, 18),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    result->v = v6.v;
    return result;
}

// ============================================================================
// pulse_sum_normal::get_relative_velocity - ea: 0x893970
// ============================================================================
const math::Dir3* pulse_sum_normal::get_relative_velocity(const math::Dir3* result) {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_ap.v;
    }
    rigid_body* m_rb = m_b1->m_rb;
    __m128 m_a_vel = m_rb->m_a_vel.v;
    math::Dir3 v8;
    v8.v = m_b1_r.v;
    ((math::Dir3*)result)->v = _mm_sub_ps(
        _mm_add_ps(
            m_rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 9), _mm_shuffle_ps(v8.v, v8.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 18), _mm_shuffle_ps(v8.v, v8.v, 9)))),
        b2_vel.v);
    return (const math::Dir3*)result;
}

// ============================================================================
// pulse_sum_normal::get_unclamped_pulse_sum - ea: 0x893A40
// ============================================================================
float pulse_sum_normal::get_unclamped_pulse_sum() {
    float ps = (m_right_side - get_objective() - m_cfm * m_pulse_sum) / m_denom + m_pulse_sum;
    return clamp_pulse_sum(ps);
}

// ============================================================================
// pulse_sum_point::get_vel - ea: 0x893A70
// ============================================================================
const math::Dir3* pulse_sum_point::get_vel(const math::Dir3* result) {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_apx.v;
    }
    rigid_body* m_rb = m_b1->m_rb;
    __m128 m_a_vel = m_rb->m_a_vel.v;
    math::Dir3 v8;
    v8.v = m_b1_r.v;
    ((math::Dir3*)result)->v = _mm_sub_ps(
        _mm_add_ps(
            m_rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 9), _mm_shuffle_ps(v8.v, v8.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 18), _mm_shuffle_ps(v8.v, v8.v, 9)))),
        b2_vel.v);
    return (const math::Dir3*)result;
}

// ============================================================================
// pulse_sum_point::get_pos - ea: 0x893B40
// ============================================================================
const math::Dir3* pulse_sum_point::get_pos(const math::Dir3* result) {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_pos;
    if (m_b2 != NULL) {
        rigid_body* m_rb = m_b2->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", ""))
            __debugbreak();
        b2_pos.v = _mm_add_ps(m_rb->m_mat.w.v, m_b2_r.v);
    } else {
        b2_pos.v = m_b2_r.v;
    }
    rigid_body* v5 = m_b1->m_rb;
    if ((~(v5->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    ((math::Dir3*)result)->v = _mm_sub_ps(
        _mm_add_ps(v5->m_mat.w.v, m_b1_r.v), b2_pos.v);
    return (const math::Dir3*)result;
}

// ============================================================================
// pulse_sum_point::set - ea: 0x893C10
// ============================================================================
void pulse_sum_point::set(rigid_body* const b1, const math::Dir3* b1_r,
                          rigid_body* const b2, const math::Dir3* b2_r,
                          pulse_sum_cache* const ps_cache) {
    if (ps_cache == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 155,
                  "ps_cache", ""))
        __debugbreak();
    if (b1 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 156,
                  "b1", ""))
        __debugbreak();
    if (b2 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 157,
                  "b2", ""))
        __debugbreak();
    if (b1 == b2 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 158,
                  "b1 != b2", ""))
        __debugbreak();
    if (b1->m_node == NULL && b2->m_node == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 159,
                  "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)", ""))
        __debugbreak();
    unsigned int v8 = (b1->m_node != NULL) ? ((b1->m_flags & 0x30) == 0)
                                           : (b1->m_flags & 0x30);
    if (v8 == 0 ||
        (b2->m_node == NULL ? (b2->m_flags & 0x30) : ((b2->m_flags & 0x30) == 0)) == 0) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 160,
                      "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)", ""))
            __debugbreak();
    }
    if (b1->m_node == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_point_inline.h", 162,
                  "rbint::get_pulse_sum_node(b1) != NULL", ""))
        __debugbreak();
    m_b1 = b1->m_node;
    m_b1_r.v = b1_r->v;
    pulse_sum_node* m_node = b2->m_node;
    if (m_node != NULL) {
        m_b2 = m_node;
        m_b2_r.v = b2_r->v;
    } else {
        m_b2 = NULL;
        math::Dir3 v14;
        v14 = rbint::gtv(b2, *b2_r);
        set_object_vel(&v14);
        math::Dir3 v13;
        v13 = rbint::add_pos(b2, *b2_r);
        set_object_col_pt(&v13);
    }
    m_pulse_sum_cache = ps_cache;
    calc_abs();
}

// ============================================================================
// pulse_sum_angular::get_vel - ea: 0x893E50
// ============================================================================
float pulse_sum_angular::get_vel() {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_ap.v;
    }
    rigid_body* m_rb = m_b1->m_rb;
    __m128 v6 = _mm_mul_ps(
        _mm_sub_ps(
            _mm_add_ps(
                m_rb->m_t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v, m_rb->m_a_vel.v, 9),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_rb->m_a_vel.v, m_rb->m_a_vel.v, 18),
                               _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
            b2_vel.v),
        m_ud.v);
    return v6.m128_f32[0] + _mm_shuffle_ps(v6, v6, 85).m128_f32[0] +
           _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
}

// ============================================================================
// pulse_sum_angular::get_pos - ea: 0x893ED0
// ============================================================================
float pulse_sum_angular::get_pos() {
    pulse_sum_node* m_b2 = this->m_b2;
    math::Dir3 b2_pos;
    if (m_b2 != NULL) {
        rigid_body* m_rb = m_b2->m_rb;
        if ((~(m_rb->m_flags >> 6) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", ""))
            __debugbreak();
        b2_pos.v = _mm_add_ps(m_rb->m_mat.w.v, m_b2_r.v);
    } else {
        b2_pos.v = m_b2_r.v;
    }
    rigid_body* v5 = m_b1->m_rb;
    if ((~(v5->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    __m128 v6 = _mm_mul_ps(
        _mm_sub_ps(_mm_add_ps(v5->m_mat.w.v, m_b1_r.v), b2_pos.v), m_ud.v);
    return v6.m128_f32[0] + _mm_shuffle_ps(v6, v6, 85).m128_f32[0] +
           _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
}

// ============================================================================
// pulse_sum_angular::set - ea: 0x893F50
// ============================================================================
void pulse_sum_angular::set(rigid_body* const b1, const math::Dir3* b1_r,
                            rigid_body* const b2, const math::Dir3* b2_r,
                            const math::Dir3* ud, pulse_sum_cache* const ps_cache) {
    if (ps_cache == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 122,
                  "ps_cache", ""))
        __debugbreak();
    if (b1 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 123,
                  "b1", ""))
        __debugbreak();
    if (b2 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 124,
                  "b2", ""))
        __debugbreak();
    if (b1 == b2 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 125,
                  "b1 != b2", ""))
        __debugbreak();
    PHYS_ASSERT_UNIT(ud);
    rigid_body* v8 = b1;
    if (b1->m_node != NULL)
        goto check_flags;
    if (b2->m_node == NULL) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 127,
                      "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)", ""))
            __debugbreak();
        v8 = b1;
    }
check_flags:
    unsigned int v10 = (v8->m_node != NULL) ? ((v8->m_flags & 0x30) == 0)
                                            : (v8->m_flags & 0x30);
    if (v10 == 0 ||
        (b2->m_node == NULL ? (b2->m_flags & 0x30) : ((b2->m_flags & 0x30) == 0)) == 0) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h", 128,
                      "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)", ""))
            __debugbreak();
        v8 = b1;
    }
    pulse_sum_node* m_node = v8->m_node;
    if (m_node != NULL) {
        m_b1 = m_node;
        m_b1_r.v = b1_r->v;
        pulse_sum_node* v14 = b2->m_node;
        if (v14 != NULL) {
            m_b2 = v14;
            m_b2_r.v = b2_r->v;
        } else {
            m_b2 = NULL;
            set_object_vel(&b2->m_a_vel);
            set_object_col_pt(b2_r);
        }
        m_ud.v = ud->v;
    } else {
        m_b1 = b2->m_node;
        m_b1_r.v = b2_r->v;
        m_b2 = NULL;
        set_object_vel(&v8->m_a_vel);
        set_object_col_pt(b1_r);
        m_ud.v = _mm_xor_ps(Float4_SignMask_214.v, ud->v);
    }
    m_pulse_sum_cache = ps_cache;
    m_flags = 0;
    calc_abs();
}

// ============================================================================
// pulse_sum_constraint_solver::solve_iterative - ea: 0x8945B0
// ============================================================================
void pulse_sum_constraint_solver::solve_iterative(int max_iters, float max_error_sq) {
    pulse_sum_constraint_solver* v3 = this;
    int v4 = 0;
    float error_sq = 100.0f;
    while (v4 <= max_iters && error_sq > max_error_sq || v4 < 1) {
        pulse_sum_normal* m_first = v3->m_list_pulse_sum_normal.m_first;
        int iters = v4 + 1;
        for (error_sq = 0.0f; m_first != NULL; m_first = m_first->m_link.m_next_link) {
            float objective = m_first->get_objective();
            float v7 = m_first->m_cfm * m_first->m_pulse_sum;
            float m_pulse_sum = m_first->m_pulse_sum;
            float ps = m_first->m_pulse_sum - (objective + v7 - m_first->m_right_side) / m_first->m_denom;
            float v52 = m_first->clamp_pulse_sum(ps);
            float v8 = m_first->m_pulse_sum_min - 0.0001f;
            m_first->m_pulse_sum = v52;
            if ((v52 < v8 || v52 > (m_first->m_pulse_sum_max + 0.0001f)) &&
                _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 119,
                          "m_pulse_sum >= (m_pulse_sum_min - .0001f) && m_pulse_sum <= (m_pulse_sum_max + .0001f)", ""))
                __debugbreak();
            float s_ = m_first->m_pulse_sum - m_pulse_sum;
            m_first->apply(&s_);
            float d = (m_first->m_pulse_sum - m_pulse_sum) * m_first->m_denom;
            if (d * d > error_sq)
                error_sq = d * d;
        }
        for (pulse_sum_point* i = v3->m_list_pulse_sum_point.m_first; i != NULL; i = i->m_link.m_next_link)
            i->SOLVER_apply_relaxation(&error_sq);
        for (pulse_sum_angular* j = v3->m_list_pulse_sum_angular.m_first; j != NULL; j = j->m_link.m_next_link)
            j->SOLVER_apply_relaxation(&error_sq);
        for (pulse_sum_wheel* k = v3->m_list_pulse_sum_wheel.m_first; k != NULL; k = k->m_link.m_next_link) {
            float v12 = k->m_suspension.get_objective();
            float v13 = k->m_suspension.m_cfm * k->m_suspension.m_pulse_sum;
            float v50 = k->m_suspension.m_pulse_sum;
            float psa = k->m_suspension.m_pulse_sum - (v12 + v13 - k->m_suspension.m_right_side) / k->m_suspension.m_denom;
            float v52 = k->m_suspension.clamp_pulse_sum(psa);
            float v14 = k->m_suspension.m_pulse_sum_min - 0.0001f;
            k->m_suspension.m_pulse_sum = v52;
            if ((v52 < v14 || v52 > (k->m_suspension.m_pulse_sum_max + 0.0001f)) &&
                _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 119,
                          "m_pulse_sum >= (m_pulse_sum_min - .0001f) && m_pulse_sum <= (m_pulse_sum_max + .0001f)", ""))
                __debugbreak();
            float v47 = k->m_suspension.m_pulse_sum - v50;
            k->m_suspension.apply(&v47);
            float d = (k->m_suspension.m_pulse_sum - v50) * k->m_suspension.m_denom;
            if (d * d > error_sq)
                error_sq = d * d;
            pulse_sum_normal* m_side = k->m_side;
            if (m_side != NULL) {
                bool v16 = (k->m_fwd == NULL);
                float v44 = m_side->m_pulse_sum;
                m_side->SOLVER_apply_relaxation(&error_sq, v16);
                pulse_sum_normal* m_fwd = k->m_fwd;
                if (m_fwd != NULL) {
                    pulse_sum_normal* v18 = k->m_fwd;
                    float v43 = m_fwd->m_pulse_sum;
                    float v19 = v18->get_objective();
                    float v20 = v18->m_cfm * v18->m_pulse_sum;
                    float v46 = v18->m_pulse_sum;
                    float psb = v18->m_pulse_sum - (v19 + v20 - v18->m_right_side) / v18->m_denom;
                    float v52b = v18->clamp_pulse_sum(psb);
                    float v21 = v18->m_pulse_sum_min - 0.0001f;
                    v18->m_pulse_sum = v52b;
                    if ((v52b < v21 || v52b > (v18->m_pulse_sum_max + 0.0001f)) &&
                        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 119,
                                  "m_pulse_sum >= (m_pulse_sum_min - .0001f) && m_pulse_sum <= (m_pulse_sum_max + .0001f)", ""))
                        __debugbreak();
                    float v45 = v18->m_pulse_sum - v46;
                    v18->apply(&v45);
                    k->addp_pulse_chain();
                    float ds = (k->m_side->m_pulse_sum - v44) * k->m_side->m_denom;
                    if (ds * ds > error_sq)
                        error_sq = ds * ds;
                    float df = (k->m_fwd->m_pulse_sum - v43) * k->m_fwd->m_denom;
                    if (df * df > error_sq)
                        error_sq = df * df;
                }
            }
        }
        pulse_sum_contact* m_next_link = v3->m_list_pulse_sum_contact.m_first;
        if (m_next_link != NULL) {
            do {
                pulse_sum_contact::psc_cpi* m_list_cpi = m_next_link->m_list_cpi;
                for (pulse_sum_contact::psc_cpi* m = &m_list_cpi[m_next_link->m_list_cpi_count];
                     m_list_cpi != m; ++m_list_cpi) {
                    float y = m_list_cpi->m_pulse_sum.y;
                    float x = m_list_cpi->m_pulse_sum.x;
                    float v40 = y;
                    pulse_sum_contact::vec2 obj_buf;
                    const pulse_sum_contact::vec2* v26 =
                        m_list_cpi->get_objective(m_list_cpi, &obj_buf);
                    float v27 = v26->y - m_list_cpi->m_right_side.y;
                    float v28 = m_list_cpi->m_pulse_sum.x - ((v26->x - m_list_cpi->m_right_side.x) / m_list_cpi->m_denom_xx);
                    m_list_cpi->m_pulse_sum.x = v28;
                    if (v28 > 0.0f)
                        m_list_cpi->m_pulse_sum.x = 0.0f;
                    float v29 = x;
                    float v30 = (((m_list_cpi->m_pulse_sum.x - x) * m_list_cpi->m_denom_xy) + v27) / m_list_cpi->m_denom_yy;
                    float v31 = m_list_cpi->m_pulse_sum.x;
                    float v32 = m_list_cpi->m_pulse_sum.y - v30;
                    m_list_cpi->m_pulse_sum.y = v32;
                    float v33 = 0.0f - (v31 * m_next_link->m_fric_coef);
                    if (v32 <= v33) {
                        if ((0.0f - v33) > v32)
                            m_list_cpi->m_pulse_sum.y = 0.0f - v33;
                    } else {
                        m_list_cpi->m_pulse_sum.y = v33;
                    }
                    pulse_sum_contact::vec2 v41;
                    v41.x = m_list_cpi->m_pulse_sum.x - v29;
                    v41.y = m_list_cpi->m_pulse_sum.y - v40;
                    m_list_cpi->apply(m_list_cpi, &v41);
                    float v34 = (m_list_cpi->m_denom_yy * v41.y) * (m_list_cpi->m_denom_yy * v41.y);
                    float dx = (m_list_cpi->m_denom_xx * v41.x) * (m_list_cpi->m_denom_xx * v41.x);
                    if (dx > error_sq)
                        error_sq = dx;
                    if (v34 > error_sq)
                        error_sq = v34;
                }
                m_next_link = m_next_link->m_link.m_next_link;
            } while (m_next_link != NULL);
            v3 = this;
        }
        v4 = iters;
    }
}

// ============================================================================
// list_constraint_solver::process - ea: 0x894A50
// ============================================================================
void phys_constraint_solver_multithreaded_list_constraint_solver::process(
    const physics_system* psys, int psys_next_psc_visit_counter) {
    pulse_sum_constraint_solver* m_constraint_solver = this->m_constraint_solver;
    if (m_constraint_solver->m_first_partition_head != NULL) {
        int m_max_vel_pos_iters = psys->m_max_vel_pos_iters;
        int m_max_vel_iters = psys->m_max_vel_iters;
        m_constraint_solver->m_psys_psc_visit_counter = psys->m_psc_visit_counter;
        m_constraint_solver->m_psys_next_psc_visit_counter = psys_next_psc_visit_counter;
        m_constraint_solver->m_psys_max_vel_iters = m_max_vel_iters;
        m_constraint_solver->m_psys_max_vel_pos_iters = m_max_vel_pos_iters;
        for (rigid_body* i = m_constraint_solver->m_first_partition_head;
             i != NULL; i = i->m_partition_node.m_next_partition_head)
            m_constraint_solver->execute_constraint_solver(i);
    }
}

// ============================================================================
// pulse_sum_contact::set - ea: 0x8941F0
// ============================================================================
void pulse_sum_contact::set(rigid_body* const b1, rigid_body* const b2,
                            contact_point_info* cpi, float delta_t) {
    if (b1 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 275,
                  "b1", ""))
        __debugbreak();
    if (b2 == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 276,
                  "b2", ""))
        __debugbreak();
    if (b1 == b2 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 277,
                  "b1 != b2", ""))
        __debugbreak();
    if (b1->m_node == NULL && b2->m_node == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 278,
                  "rbint::get_pulse_sum_node(b1) || rbint::get_pulse_sum_node(b2)", ""))
        __debugbreak();
    unsigned int v7 = (b1->m_node != NULL) ? ((b1->m_flags & 0x30) == 0)
                                           : (b1->m_flags & 0x30);
    if (v7 == 0 ||
        (b2->m_node == NULL ? (b2->m_flags & 0x30) : ((b2->m_flags & 0x30) == 0)) == 0) {
        if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 279,
                      "rbint::verify_pulse_sum_node(b1) && rbint::verify_pulse_sum_node(b2)", ""))
            __debugbreak();
    }
    if (b1->m_node == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 281,
                  "rbint::get_pulse_sum_node(b1) != NULL",
                  "b1 in contact constraint cannot be environment or user rigid body."))
        __debugbreak();
    m_b1 = b1->m_node;
    m_b2 = b2->m_node;
    m_ud_n.v = cpi->m_normal.v;
    int m_list_cpi_count = this->m_list_cpi_count;
    m_fric_coef = cpi->m_fric_coef;
    if (m_list_cpi_count != cpi->m_point_pair_count &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h", 287,
                  "m_list_cpi_count == cpi->m_point_pair_count", ""))
        __debugbreak();
    int m_point_pair_count = cpi->m_point_pair_count;
    math::Dir3* m_list_b1_r_loc = cpi->m_list_b1_r_loc;
    math::Dir3* m_list_b2_r_loc = cpi->m_list_b2_r_loc;
    math::Dir3* v33 = m_list_b1_r_loc;
    int v31 = 0;
    if (m_point_pair_count > 0) {
        int pp_i = 0;
        int b2_idx = 0;
        do {
            psc_cpi* v12 = m_list_cpi + b2_idx;
            math::Dir3 v26;
            v26 = rbint::multiply(b1, *v33);
            v12->m_b1_r.v = v26.v;
            math::Dir3 v30;
            v30 = rbint::multiply(b2, *m_list_b2_r_loc);
            if (m_b2 != NULL) {
                v12->m_b2_r = v30;
            } else {
                rigid_body* v16 = b2;
                __m128 v = v30.v;
                __m128 v29 = _mm_add_ps(
                    b2->m_t_vel.v,
                    _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(b2->m_a_vel.v, b2->m_a_vel.v, 9),
                                   _mm_shuffle_ps(v30.v, v30.v, 18)),
                        _mm_mul_ps(_mm_shuffle_ps(b2->m_a_vel.v, b2->m_a_vel.v, 18),
                                   _mm_shuffle_ps(v, v, 9))));
                if ((~(b2->m_flags >> 6) & 1) == 0) {
                    bool v18 = _tlAssert("c:/cod/code/tl/physics/include\\rigid_body_internal.h", 270,
                                         "b->debug_flag_is_not_in_collision()", "");
                    v = v30.v;
                    v16 = b2;
                    if (!v18)
                        __debugbreak();
                }
                math::Dir3 relative_velocity_4;
                relative_velocity_4.v = _mm_add_ps(v16->m_mat.w.v, v);
                if (m_b2 != NULL &&
                    _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new.h", 52,
                              "psc->m_b2 == NULL", ""))
                    __debugbreak();
                v12->m_b2_ap_n.v = relative_velocity_4.v;
            }
            math::Dir3 v27 = v12->get_relative_velocity(this);
            v12->calc_abs_and_fric_dir(this, &v27);
            v12->m_pulse_sum_cache = (pulse_sum_cache*)&cpi->m_list_pulse_sum_cache_info[pp_i / 0x10u];
            v12->setup_vel_uni_restitution(this, &v27, cpi->m_bounce_coef,
                                           cpi->m_max_restitution_vel, delta_t, 170.0f);
            b2_idx += 10;
            pp_i += 16;
            ++v33;
            v31++;
            ++m_list_b2_r_loc;
        } while (v31 < cpi->m_point_pair_count);
    }
}

// ============================================================================
// pulse_sum_normal inline row methods (COMDATs in this unit).
// ============================================================================
float pulse_sum_normal::get_objective() {
    pulse_sum_node* m_b1 = this->m_b1;
    __m128 a_vel = m_b1->a_vel.v;
    math::Dir3 v3;
    v3.v = m_b1_r.v;
    __m128 v4 = _mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 18), _mm_shuffle_ps(v3.v, v3.v, 9));
    __m128 v5 = _mm_shuffle_ps(v3.v, v3.v, 18);
    __m128 t_vel = m_b1->t_vel.v;
    pulse_sum_node* m_b2 = this->m_b2;
    __m128 v8 = _mm_add_ps(t_vel, _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 9), v5), v4));
    if (m_b2 != NULL)
        v8 = _mm_sub_ps(
            v8,
            _mm_add_ps(
                m_b2->t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 9),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 18),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    __m128 v9 = _mm_mul_ps(v8, m_ud.v);
    return v9.m128_f32[0] + _mm_shuffle_ps(v9, v9, 85).m128_f32[0] +
           _mm_shuffle_ps(v9, v9, 170).m128_f32[0];
}

void pulse_sum_normal::apply(const float* s_) {
    pulse_sum_node* m_b1 = this->m_b1;
    float inv = m_b1->m_inv_mass * *s_;
    m_b1->t_vel.v = _mm_add_ps(m_b1->t_vel.v, _mm_mul_ps(m_ud.v, _mm_shuffle_ps(_mm_set_ss(inv), _mm_set_ss(inv), 0)));
    m_b1->a_vel.v = _mm_add_ps(m_b1->a_vel.v, _mm_mul_ps(m_b1_ap.v, _mm_shuffle_ps(_mm_set_ss(*s_), _mm_set_ss(*s_), 0)));
    pulse_sum_node* m_b2 = this->m_b2;
    if (m_b2 != NULL) {
        float inv2 = m_b2->m_inv_mass * *s_;
        m_b2->t_vel.v = _mm_sub_ps(m_b2->t_vel.v, _mm_mul_ps(m_ud.v, _mm_shuffle_ps(_mm_set_ss(inv2), _mm_set_ss(inv2), 0)));
        m_b2->a_vel.v = _mm_sub_ps(m_b2->a_vel.v, _mm_mul_ps(m_b2_ap.v, _mm_shuffle_ps(_mm_set_ss(*s_), _mm_set_ss(*s_), 0)));
    }
}

float pulse_sum_normal::clamp_pulse_sum(float ps) {
    if ((m_flags & 1) != 0) {
        if (m_pulse_parent == NULL &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 76,
                      "m_pulse_parent", ""))
            __debugbreak();
        if (m_pulse_limit_ratio < 0.0f &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 77,
                      "m_pulse_limit_ratio >= 0.0f", ""))
            __debugbreak();
        float v3 = fabs(m_pulse_parent->m_pulse_sum) * m_pulse_limit_ratio;
        m_pulse_sum_max = v3;
        m_pulse_sum_min = -v3;
    }
    if (m_pulse_sum_max < m_pulse_sum_min &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h", 81,
                  "m_pulse_sum_min <= m_pulse_sum_max", ""))
        __debugbreak();
    unsigned int m_flags = this->m_flags;
    if (m_pulse_sum_min <= ps) {
        if (ps <= m_pulse_sum_max) {
            m_flags &= 0xFFFFFFFD;
            this->m_flags = m_flags;
            return ps;
        }
        this->m_flags = m_flags | 2;
        return m_pulse_sum_max;
    }
    this->m_flags = m_flags | 2;
    return m_pulse_sum_min;
}

void pulse_sum_normal::calc_abs(const math::Dir3* b1_r_displace) {
    rigid_body* m_rb = this->m_b1->m_rb;
    __m128 v5 = _mm_add_ps(this->m_b1_r.v, b1_r_displace->v);
    math::Dir3 v13;
    v13.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(v5, v5, 9), _mm_shuffle_ps(this->m_ud.v, this->m_ud.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(v5, v5, 18), _mm_shuffle_ps(this->m_ud.v, this->m_ud.v, 9)));
    math::Dir3 v12;
    this->m_b1_ap.v = rbint::inv_L(&v12, m_rb, &v13)->v;
    pulse_sum_node* m_b1 = this->m_b1;
    pulse_sum_node* m_b2 = this->m_b2;
    __m128 v8 = _mm_mul_ps(
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_ap.v, this->m_b1_ap.v, 9),
                       _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b1_ap.v, this->m_b1_ap.v, 18),
                       _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9))),
        this->m_ud.v);
    float v14 = v8.m128_f32[0] + (_mm_shuffle_ps(v8, v8, 85).m128_f32[0] +
                                  _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
    this->m_denom = m_b1->m_inv_mass + v14;
    if (m_b2 != NULL) {
        const rigid_body* v11 = m_b2->m_rb;
        v13.v = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9),
                       _mm_shuffle_ps(this->m_ud.v, this->m_ud.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18),
                       _mm_shuffle_ps(this->m_ud.v, this->m_ud.v, 9)));
        this->m_b2_ap.v = rbint::inv_L(&v12, v11, &v13)->v;
        __m128 v9 = _mm_mul_ps(this->m_b2_ap.v, v13.v);
        pulse_sum_node* v10 = this->m_b2;
        v14 = v9.m128_f32[0] + (_mm_shuffle_ps(v9, v9, 85).m128_f32[0] +
                                _mm_shuffle_ps(v9, v9, 170).m128_f32[0]);
        this->m_denom = (v10->m_inv_mass + v14) + this->m_denom;
    }
    if (this->m_denom <= 0.0000099999997f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal_inline.h",
                  11, "m_denom > 0.00001f", defaultFileName))
        __debugbreak();
}

void pulse_sum_normal::set_object_vel(const math::Dir3* object_vel) {
    if (m_b2 != NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h", 79,
                  "m_b2 == NULL", ""))
        __debugbreak();
    m_b2_ap.v = object_vel->v;
}

void pulse_sum_normal::set_object_col_pt(const math::Dir3* object_col_pt) {
    if (m_b2 != NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h", 82,
                  "m_b2 == NULL", ""))
        __debugbreak();
    m_b2_r.v = object_col_pt->v;
}

void pulse_sum_normal::SOLVER_apply_relaxation(float* error_sq, bool add_error) {
    // ea: 0x8958A0 - scalar relaxation with cfm/denom, error tracking.
    float m_last_pulse_sum = m_pulse_sum;
    float s_ = m_last_pulse_sum - (get_objective() + m_last_pulse_sum * m_cfm - m_right_side) / m_denom;
    s_ = clamp_pulse_sum(s_);
    apply(&s_);
    float d = (m_pulse_sum - m_last_pulse_sum) * m_denom;
    if (add_error && d * d > *error_sq)
        *error_sq = d * d;
}

void pulse_sum_normal::SOLVER_solver_intermediate(int iter, float delta_t) {
    (void)iter;
    (void)delta_t;
}

void pulse_sum_normal::SOLVER_solver_prolog(int iter, float delta_t) {
    (void)iter;
    (void)delta_t;
}

void pulse_sum_normal::project() {
}

// ============================================================================
// pulse_sum_point row methods
// ============================================================================
const math::Dir3* pulse_sum_point::get_objective(const math::Dir3* result) {
    // ea: 0x8960C0 - relative velocity (objective = vel difference).
    pulse_sum_node* m_b1 = this->m_b1;
    __m128 a_vel = m_b1->a_vel.v;
    math::Dir3 v3;
    v3.v = m_b1_r.v;
    __m128 v4 = _mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 18), _mm_shuffle_ps(v3.v, v3.v, 9));
    __m128 v5 = _mm_shuffle_ps(v3.v, v3.v, 18);
    __m128 t_vel = m_b1->t_vel.v;
    pulse_sum_node* m_b2 = this->m_b2;
    __m128 v8 = _mm_add_ps(t_vel, _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 9), v5), v4));
    if (m_b2 != NULL)
        v8 = _mm_sub_ps(
            v8,
            _mm_add_ps(
                m_b2->t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 9),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 18),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    ((math::Dir3*)result)->v = v8;
    return result;
}

const math::Dir3* pulse_sum_point::phys_diag_multiply_and_square(
    const math::Dir3* result, const math::Dir3* v1, const math::Dir3* v2) {
    // ea: 0x891E20 - (v1 * denom) element-wise, squared into result.
    math::Dir3 r;
    r.v = _mm_mul_ps(v1->v, v2->v);
    ((math::Dir3*)result)->v = _mm_mul_ps(r.v, r.v);
    return result;
}

void pulse_sum_point::apply(const math::Dir3* s_) {
    // ea: 0x8928E0 - apply impulse along x/y/z rows.
    pulse_sum_node* m_b1 = this->m_b1;
    m_b1->t_vel.v = _mm_add_ps(m_b1->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b1->m_inv_mass * s_->v.m128_f32[0]), m_b1_apx.v));
    m_b1->t_vel.v = _mm_add_ps(m_b1->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b1->m_inv_mass * s_->v.m128_f32[1]), m_b1_apy.v));
    m_b1->t_vel.v = _mm_add_ps(m_b1->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b1->m_inv_mass * s_->v.m128_f32[2]), m_b1_apz.v));
    pulse_sum_node* m_b2 = this->m_b2;
    if (m_b2 != NULL) {
        m_b2->t_vel.v = _mm_sub_ps(m_b2->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b2->m_inv_mass * s_->v.m128_f32[0]), m_b2_apx.v));
        m_b2->t_vel.v = _mm_sub_ps(m_b2->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b2->m_inv_mass * s_->v.m128_f32[1]), m_b2_apy.v));
        m_b2->t_vel.v = _mm_sub_ps(m_b2->t_vel.v, _mm_mul_ps(_mm_set_ss(m_b2->m_inv_mass * s_->v.m128_f32[2]), m_b2_apz.v));
    }
}

void pulse_sum_point::calc_abs() {
    pulse_sum_node* m_b1 = this->m_b1;
    __m128 v2;
    __m128* p_v;
    __m128 v4;
    __m128 v5;
    __m128 v6;
    __m128 v7;
    __m128 v8;
    __m128 v9;
    pulse_sum_node* m_b2;
    float m_inv_mass;
    rigid_body* m_rb;
    __m128 v13;
    __m128 v14;
    __m128 v15;
    __m128 x;
    __m128 v;
    __m128 v18;
    __m128 v19;
    __m128 v20;
    __m128 v21;
    __m128 v22;
    __m128 v23;
    __m128 v24;
    math::Dir3* p_m_cr23;
    __m128 v26;
    __m128 v27;
    math::Dir3* p_m_cr31;
    math::Dir3* p_m_cr12;
    math::Dir3* p_m_denom;
    __m128 v30;
    __m128 v31;
    __m128 v32;
    __m128 v33;
    __m128 v34;
    __m128 v35;
    __m128 v36;
    __m128 v37;
    __m128 v38;
    __m128 v40;
    math::Dir3 zz_4;
    float v42;
    __m128 v43;
    __m128 v44;
    math::Dir3 zx_4;

    v43.m128_f32[0] = m_b1->m_inv_mass;
    v43.m128_f32[1] = v43.m128_f32[0];
    v43.m128_f32[2] = v43.m128_f32[0];
    v2 = _mm_shuffle_ps(Float4_XAxis_214, Float4_XAxis_214, 9);
    p_v = &m_b1->m_rb->m_world_inv_inertia.x.v;
    v4 = _mm_shuffle_ps(Float4_XAxis_214, Float4_XAxis_214, 18);
    v5 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9), v4),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18), v2));
    v6 = v43;
    this->m_b1_apx.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v5, v5, 0), *p_v),
            _mm_mul_ps(_mm_shuffle_ps(v5, v5, 85), p_v[1])),
        _mm_mul_ps(_mm_shuffle_ps(v5, v5, 170), p_v[2]));
    v44 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apx.v, this->m_b1_apx.v, 9),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apx.v, this->m_b1_apx.v, 18),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9)));
    v44.m128_f32[0] += v6.m128_f32[0];
    v36 = _mm_shuffle_ps(Float4_YAxis_214, Float4_YAxis_214, 9);
    v34 = _mm_shuffle_ps(Float4_YAxis_214, Float4_YAxis_214, 18);
    v7 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9), v34),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18), v36));
    this->m_b1_apy.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 0), *p_v),
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 85), p_v[1])),
        _mm_mul_ps(_mm_shuffle_ps(v7, v7, 170), p_v[2]));
    zz_4.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apy.v, this->m_b1_apy.v, 9),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apy.v, this->m_b1_apy.v, 18),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9)));
    zz_4.v.m128_f32[1] += _mm_shuffle_ps(v6, v6, 85).m128_f32[0];
    v35 = _mm_shuffle_ps(Float4_ZAxis_214, Float4_ZAxis_214, 9);
    v8 = _mm_shuffle_ps(Float4_ZAxis_214, Float4_ZAxis_214, 18);
    v9 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9), v8),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18), v35));
    v37 = v8;
    this->m_b1_apz.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v9, v9, 0), *p_v),
            _mm_mul_ps(_mm_shuffle_ps(v9, v9, 85), p_v[1])),
        _mm_mul_ps(_mm_shuffle_ps(v9, v9, 170), p_v[2]));
    v40 = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apz.v, this->m_b1_apz.v, 9),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(this->m_b1_apz.v, this->m_b1_apz.v, 18),
                   _mm_shuffle_ps(this->m_b1_r.v, this->m_b1_r.v, 9)));
    m_b2 = this->m_b2;
    v40.m128_f32[2] += _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
    if (m_b2 != NULL) {
        m_inv_mass = m_b2->m_inv_mass;
        m_rb = m_b2->m_rb;
        zx_4.v.m128_f32[0] = m_inv_mass;
        zx_4.v.m128_f32[1] = m_inv_mass;
        zx_4.v.m128_f32[2] = m_inv_mass;
        v13 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9), v4),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18), v2));
        v14 = _mm_mul_ps(_mm_shuffle_ps(v13, v13, 170), m_rb->m_world_inv_inertia.z.v);
        v15 = _mm_mul_ps(_mm_shuffle_ps(v13, v13, 85), m_rb->m_world_inv_inertia.y.v);
        x = m_rb->m_world_inv_inertia.x.v;
        m_rb = (rigid_body*)((char*)m_rb + 128);
        v = zx_4.v;
        this->m_b2_apx.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v13, v13, 0), x), v15), v14);
        v44 = _mm_add_ps(
            v44,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apx.v, this->m_b2_apx.v, 9),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apx.v, this->m_b2_apx.v, 18),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9))));
        v44.m128_f32[0] += zx_4.v.m128_f32[0];
        v18 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9), v34),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18), v36));
        this->m_b2_apy.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v18, v18, 0), m_rb->m_mat.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(v18, v18, 85), m_rb->m_mat.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(v18, v18, 170), m_rb->m_mat.z.v));
        zz_4.v = _mm_add_ps(
            zz_4.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apy.v, this->m_b2_apy.v, 9),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apy.v, this->m_b2_apy.v, 18),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9))));
        zz_4.v.m128_f32[1] += _mm_shuffle_ps(v, v, 85).m128_f32[0];
        v19 = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9), v37),
            _mm_mul_ps(_mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18), v35));
        this->m_b2_apz.v = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v19, v19, 0), m_rb->m_mat.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(v19, v19, 85), m_rb->m_mat.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(v19, v19, 170), m_rb->m_mat.z.v));
        v40 = _mm_add_ps(
            v40,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apz.v, this->m_b2_apz.v, 9),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(this->m_b2_apz.v, this->m_b2_apz.v, 18),
                           _mm_shuffle_ps(this->m_b2_r.v, this->m_b2_r.v, 9))));
        v40.m128_f32[2] += _mm_shuffle_ps(v, v, 170).m128_f32[0];
    }
    v20 = v40;
    v21 = v44;
    v22 = _mm_shuffle_ps(zz_4.v, zz_4.v, 18);
    v38 = _mm_shuffle_ps(zz_4.v, zz_4.v, 9);
    v23 = _mm_shuffle_ps(v20, v20, 9);
    v24 = _mm_shuffle_ps(v20, v20, 18);
    p_m_cr23 = &this->m_cr23;
    p_m_cr23->v = _mm_sub_ps(_mm_mul_ps(v38, v24), _mm_mul_ps(v22, v23));
    v26 = _mm_shuffle_ps(v21, v21, 9);
    v27 = _mm_shuffle_ps(v21, v21, 18);
    p_m_cr31 = &this->m_cr31;
    p_m_cr31->v = _mm_sub_ps(_mm_mul_ps(v23, v27), _mm_mul_ps(v24, v26));
    p_m_cr12 = &this->m_cr12;
    p_m_cr12->v = _mm_sub_ps(_mm_mul_ps(v26, v22), _mm_mul_ps(v27, v38));
    p_m_denom = &this->m_denom;
    v30 = p_m_cr12->v;
    p_m_denom->v.m128_f32[0] = v44.m128_f32[0];
    p_m_denom->v.m128_f32[1] = _mm_shuffle_ps(zz_4.v, zz_4.v, 85).m128_f32[0];
    v31 = _mm_mul_ps(v30, v40);
    p_m_denom->v.m128_f32[2] = _mm_shuffle_ps(v40, v40, 170).m128_f32[0];
    p_m_denom->v.m128_f32[3] = 0.0f;
    v42 = v31.m128_f32[0] + (_mm_shuffle_ps(v31, v31, 85).m128_f32[0] +
                             _mm_shuffle_ps(v31, v31, 170).m128_f32[0]);
    v32 = _mm_set_ss(1.0f / v42);
    v33 = _mm_shuffle_ps(v32, v32, 0);
    p_m_cr23->v = _mm_mul_ps(p_m_cr23->v, v33);
    p_m_cr31->v = _mm_mul_ps(p_m_cr31->v, v33);
    p_m_cr12->v = _mm_mul_ps(p_m_cr12->v, v33);
}

void pulse_sum_point::project() {
}

void pulse_sum_point::SOLVER_apply_relaxation(float* error_sq) {
    // ea: 0x8962F0
    math::Dir3 objective;
    get_objective(&objective);
    math::Dir3 s_ = m_pulse_sum;
    s_.v = _mm_sub_ps(
        m_pulse_sum.v,
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(m_cr12.v, _mm_shuffle_ps(_mm_set_ss(objective.v.m128_f32[0] - m_right_side.v.m128_f32[0]), _mm_set_ss(0.0f), 0)),
                _mm_mul_ps(m_cr23.v, _mm_shuffle_ps(_mm_set_ss(objective.v.m128_f32[1] - m_right_side.v.m128_f32[1]), _mm_set_ss(0.0f), 0))),
            _mm_mul_ps(m_cr31.v, _mm_shuffle_ps(_mm_set_ss(objective.v.m128_f32[2] - m_right_side.v.m128_f32[2]), _mm_set_ss(0.0f), 0))));
    math::Dir3 delta = m_pulse_sum;
    m_pulse_sum = s_;
    delta.v = _mm_sub_ps(m_pulse_sum.v, delta.v);
    apply(&delta);
    math::Dir3 err;
    phys_diag_multiply_and_square(&err, &delta, &m_denom);
    float e = err.v.m128_f32[0];
    if (err.v.m128_f32[1] > e) e = err.v.m128_f32[1];
    if (err.v.m128_f32[2] > e) e = err.v.m128_f32[2];
    if (e > *error_sq)
        *error_sq = e;
}

void pulse_sum_point::SOLVER_solver_prolog(int iter, float delta_t) {
    // ea: 0x896190 - right_side -= vel; pulse_sum from cache*delta_t; apply.
    math::Dir3 vel;
    get_vel(&vel);
    m_right_side.v = _mm_sub_ps(m_right_side.v, vel.v);
    for (int i = 0; i < 3; ++i) {
        if (m_pulse_sum_cache[i].m_visit_key != iter)
            m_pulse_sum_cache[i].m_pulse_sum = 0.0f;
        m_pulse_sum.v.m128_f32[i] = m_pulse_sum_cache[i].m_pulse_sum * delta_t;
    }
    apply(&m_pulse_sum);
}

void pulse_sum_point::SOLVER_solver_intermediate(int iter, float delta_t) {
    // ea: 0x892A40 - write back pulse_sum/delta_t into cache.
    for (int i = 0; i < 3; ++i) {
        m_pulse_sum_cache[i].m_pulse_sum = m_pulse_sum.v.m128_f32[i] / delta_t;
        m_pulse_sum_cache[i].m_visit_key = iter;
    }
    m_right_side.v = _mm_add_ps(m_big_dirt.v, m_right_side.v);
}

void pulse_sum_point::set_object_vel(const math::Dir3* object_vel) {
    m_b2_apx.v = object_vel->v;
}

void pulse_sum_point::set_object_col_pt(const math::Dir3* object_col_pt) {
    m_b2_r.v = object_col_pt->v;
}

// ============================================================================
// pulse_sum_angular row methods
// ============================================================================
float pulse_sum_angular::get_objective() {
    // ea: 0x896580
    pulse_sum_node* m_b1 = this->m_b1;
    __m128 a_vel = m_b1->a_vel.v;
    math::Dir3 v3;
    v3.v = m_b1_r.v;
    __m128 v4 = _mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 18), _mm_shuffle_ps(v3.v, v3.v, 9));
    __m128 v5 = _mm_shuffle_ps(v3.v, v3.v, 18);
    __m128 t_vel = m_b1->t_vel.v;
    pulse_sum_node* m_b2 = this->m_b2;
    __m128 v8 = _mm_add_ps(t_vel, _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 9), v5), v4));
    if (m_b2 != NULL)
        v8 = _mm_sub_ps(
            v8,
            _mm_add_ps(
                m_b2->t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 9),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 18),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    __m128 v9 = _mm_mul_ps(v8, m_ud.v);
    return v9.m128_f32[0] + _mm_shuffle_ps(v9, v9, 85).m128_f32[0] +
           _mm_shuffle_ps(v9, v9, 170).m128_f32[0];
}

void pulse_sum_angular::apply(const float* s_) {
    pulse_sum_node* m_b1 = this->m_b1;
    m_b1->a_vel.v = _mm_add_ps(m_b1->a_vel.v, _mm_mul_ps(m_b1_ap.v, _mm_shuffle_ps(_mm_set_ss(*s_), _mm_set_ss(*s_), 0)));
    pulse_sum_node* m_b2 = this->m_b2;
    if (m_b2 != NULL)
        m_b2->a_vel.v = _mm_sub_ps(m_b2->a_vel.v, _mm_mul_ps(m_b2_ap.v, _mm_shuffle_ps(_mm_set_ss(*s_), _mm_set_ss(*s_), 0)));
}

void pulse_sum_angular::calc_abs() {
    math::Dir3 result;
    const math::Dir3* b1_ap = rbint::inv_L(&result, m_b1->m_rb, &m_ud);
    m_b1_ap.v = b1_ap->v;
    __m128 v = _mm_mul_ps(m_b1_ap.v, m_ud.v);
    m_denom = v.m128_f32[0] + _mm_shuffle_ps(v, v, 85).m128_f32[0] +
              _mm_shuffle_ps(v, v, 170).m128_f32[0];
    if (m_b2 != NULL) {
        const math::Dir3* b2_ap = rbint::inv_L(&result, m_b2->m_rb, &m_ud);
        m_b2_ap.v = b2_ap->v;
        v = _mm_mul_ps(m_b2_ap.v, m_ud.v);
        m_denom += v.m128_f32[0] + _mm_shuffle_ps(v, v, 85).m128_f32[0] +
                   _mm_shuffle_ps(v, v, 170).m128_f32[0];
    }
    if (m_denom <= 0.0000099999997f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_angular_inline.h",
                  10, "m_denom > 0.00001f", defaultFileName))
        __debugbreak();
}

void pulse_sum_angular::project() {
}

void pulse_sum_angular::SOLVER_apply_relaxation(float* error_sq) {
    float m_last_pulse_sum = m_pulse_sum;
    float s_ = m_last_pulse_sum - (get_objective() + m_last_pulse_sum * m_cfm - m_right_side) / m_denom;
    if (m_pulse_sum_min <= s_) {
        if (s_ > m_pulse_sum_max)
            s_ = m_pulse_sum_max;
    } else {
        s_ = m_pulse_sum_min;
    }
    m_pulse_sum = s_;
    float delta = m_pulse_sum - m_last_pulse_sum;
    apply(&delta);
    float v7 = delta * m_denom;
    if (v7 * v7 > *error_sq)
        *error_sq = v7 * v7;
}

void pulse_sum_angular::SOLVER_solver_prolog(int iter, float delta_t) {
    m_right_side = m_right_side - get_vel();
    if (m_pulse_sum_cache->m_visit_key != iter)
        m_pulse_sum_cache->m_pulse_sum = 0.0f;
    m_pulse_sum = m_pulse_sum_cache->m_pulse_sum * delta_t;
    if (m_pulse_sum_min <= m_pulse_sum) {
        if (m_pulse_sum > m_pulse_sum_max)
            m_pulse_sum = m_pulse_sum_max;
    } else {
        m_pulse_sum = m_pulse_sum_min;
    }
    apply(&m_pulse_sum);
}

void pulse_sum_angular::SOLVER_solver_intermediate(int iter, float delta_t) {
    m_pulse_sum_cache->m_pulse_sum = m_pulse_sum / delta_t;
    m_pulse_sum_cache->m_visit_key = iter;
    m_right_side = m_big_dirt + m_right_side;
}

void pulse_sum_angular::set_object_vel(const math::Dir3* object_vel) {
    m_b2_ap.v = object_vel->v;
}

void pulse_sum_angular::set_object_col_pt(const math::Dir3* object_col_pt) {
    m_b2_r.v = object_col_pt->v;
}

// ============================================================================
// pulse_sum_wheel row methods
// ============================================================================
bool pulse_sum_wheel::clamp_pulse_sum_pulse_chain(float* ps1_, float* ps2_) {
    // ea: 0x892D10 - clamp side/fwd to suspension friction cone.
    if (m_side == NULL || m_fwd == NULL)
        return false;
    float limit = fabs(m_suspension.m_pulse_sum) * m_side->m_pulse_limit_ratio;
    float ps1 = m_side->m_pulse_sum;
    if (ps1 > limit) ps1 = limit;
    if (ps1 < -limit) ps1 = -limit;
    float limit2 = fabs(m_suspension.m_pulse_sum) * m_fwd->m_pulse_limit_ratio;
    float ps2 = m_fwd->m_pulse_sum;
    if (ps2 > limit2) ps2 = limit2;
    if (ps2 < -limit2) ps2 = -limit2;
    *ps1_ = ps1;
    *ps2_ = ps2;
    return true;
}

bool pulse_sum_wheel::pulse_chain_within_limits() {
    if (m_side == NULL || m_fwd == NULL)
        return true;
    float v6 = 0.0f - (m_side->m_pulse_limit_ratio * m_suspension.m_pulse_sum);
    float v7 = 0.0f - (m_fwd->m_pulse_limit_ratio * m_suspension.m_pulse_sum);
    return (v7 * v7) * (v6 * v6) >=
           ((m_side->m_pulse_sum * m_side->m_pulse_sum) * (v7 * v7) +
            (m_fwd->m_pulse_sum * m_fwd->m_pulse_sum) * (v6 * v6)) * 0.99999f;
}

void pulse_sum_wheel::addp_pulse_chain() {
    float ps1_, ps2_;
    if (clamp_pulse_sum_pulse_chain(&ps1_, &ps2_)) {
        float s_ = ps1_ - m_side->m_pulse_sum;
        m_side->apply(&s_);
        m_side->m_pulse_sum = ps1_;
        s_ = ps2_ - m_fwd->m_pulse_sum;
        m_fwd->apply(&s_);
        m_fwd->m_pulse_sum = ps2_;
    }
    if (!pulse_chain_within_limits() &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_wheel_inline.h", 85,
                  "pulse_chain_within_limits()", ""))
        __debugbreak();
}

void pulse_sum_wheel::SOLVER_apply_relaxation(float* error_sq) {
    float v50 = m_suspension.m_pulse_sum;
    float psa = v50 - (m_suspension.get_objective() + m_suspension.m_cfm * v50 - m_suspension.m_right_side) / m_suspension.m_denom;
    m_suspension.m_pulse_sum = m_suspension.clamp_pulse_sum(psa);
    float v47 = m_suspension.m_pulse_sum - v50;
    m_suspension.apply(&v47);
    float d = (m_suspension.m_pulse_sum - v50) * m_suspension.m_denom;
    if (d * d > *error_sq)
        *error_sq = d * d;
    if (m_side != NULL) {
        m_side->SOLVER_apply_relaxation(error_sq, m_fwd == NULL);
        if (m_fwd != NULL) {
            float v46 = m_fwd->m_pulse_sum;
            float psb = v46 - (m_fwd->get_objective() + m_fwd->m_cfm * v46 - m_fwd->m_right_side) / m_fwd->m_denom;
            m_fwd->m_pulse_sum = m_fwd->clamp_pulse_sum(psb);
            float v45 = m_fwd->m_pulse_sum - v46;
            m_fwd->apply(&v45);
            addp_pulse_chain();
            float ds = (m_side->m_pulse_sum - 0.0f) * m_side->m_denom;
            if (ds * ds > *error_sq)
                *error_sq = ds * ds;
            float df = (m_fwd->m_pulse_sum - 0.0f) * m_fwd->m_denom;
            if (df * df > *error_sq)
                *error_sq = df * df;
        }
    }
}

void pulse_sum_wheel::SOLVER_solver_prolog(int iter, float delta_t) {
    m_suspension.m_right_side = m_suspension.m_right_side - m_suspension.get_vel();
    if (m_suspension.m_pulse_sum_cache->m_visit_key != iter)
        m_suspension.m_pulse_sum_cache->m_pulse_sum = 0.0f;
    m_suspension.m_pulse_sum = m_suspension.m_pulse_sum_cache->m_pulse_sum * delta_t;
    m_suspension.m_pulse_sum = m_suspension.clamp_pulse_sum(m_suspension.m_pulse_sum);
    m_suspension.apply(&m_suspension.m_pulse_sum);
    if (m_side != NULL) {
        if (m_side->m_pulse_sum_cache->m_visit_key != iter)
            m_side->m_pulse_sum_cache->m_pulse_sum = 0.0f;
        m_side->m_pulse_sum = m_side->m_pulse_sum_cache->m_pulse_sum * delta_t;
        m_side->m_pulse_sum = m_side->clamp_pulse_sum(m_side->m_pulse_sum);
        m_side->apply(&m_side->m_pulse_sum);
    }
    if (m_fwd != NULL) {
        if (m_fwd->m_pulse_sum_cache->m_visit_key != iter)
            m_fwd->m_pulse_sum_cache->m_pulse_sum = 0.0f;
        m_fwd->m_pulse_sum = m_fwd->m_pulse_sum_cache->m_pulse_sum * delta_t;
        m_fwd->m_pulse_sum = m_fwd->clamp_pulse_sum(m_fwd->m_pulse_sum);
        m_fwd->apply(&m_fwd->m_pulse_sum);
    }
}

void pulse_sum_wheel::SOLVER_solver_intermediate(int iter, float delta_t) {
    float inv_dt = 1.0f / delta_t;
    m_suspension.m_pulse_sum_cache->m_pulse_sum = m_suspension.m_pulse_sum * inv_dt;
    m_suspension.m_pulse_sum_cache->m_visit_key = iter;
    m_suspension.m_right_side = m_suspension.m_big_dirt + m_suspension.m_right_side;
    if (m_side != NULL) {
        m_side->m_pulse_sum_cache->m_pulse_sum = m_side->m_pulse_sum * inv_dt;
        m_side->m_pulse_sum_cache->m_visit_key = iter;
        m_side->m_right_side = m_side->m_big_dirt + m_side->m_right_side;
    }
    if (m_fwd != NULL) {
        m_fwd->m_pulse_sum_cache->m_pulse_sum = m_fwd->m_pulse_sum * inv_dt;
        m_fwd->m_pulse_sum_cache->m_visit_key = iter;
        m_fwd->m_right_side = m_fwd->m_big_dirt + m_fwd->m_right_side;
    }
}

// ============================================================================
// pulse_sum_contact::psc_cpi row methods
// ============================================================================
void pulse_sum_contact::psc_cpi::SOLVER_apply_relaxation(psc_cpi* self, float* error_sq) {
    // ea: 0x897200
    vec2 m_last_pulse_sum = m_pulse_sum;
    vec2 objective;
    get_objective(self, &objective);
    float v6 = m_pulse_sum.x - ((objective.x - m_right_side.x) / m_denom_xx);
    m_pulse_sum.x = v6;
    if (v6 > 0.0f)
        m_pulse_sum.x = 0.0f;
    float v5 = objective.y - m_right_side.y;
    float v7 = m_pulse_sum.y - ((((m_pulse_sum.x - m_last_pulse_sum.x) * m_denom_xy) + v5) / m_denom_yy);
    m_pulse_sum.y = v7;
    float v8 = 0.0f - (((pulse_sum_contact*)self)->m_fric_coef * m_pulse_sum.x);
    if (v7 <= v8) {
        if ((0.0f - v8) > v7)
            m_pulse_sum.y = 0.0f - v8;
    } else {
        m_pulse_sum.y = v8;
    }
    vec2 delta_pulse_sum;
    delta_pulse_sum.x = m_pulse_sum.x - m_last_pulse_sum.x;
    delta_pulse_sum.y = m_pulse_sum.y - m_last_pulse_sum.y;
    apply(self, &delta_pulse_sum);
    float v9 = (m_denom_xx * delta_pulse_sum.x) * (m_denom_xx * delta_pulse_sum.x);
    float v10 = (delta_pulse_sum.y * m_denom_yy) * (delta_pulse_sum.y * m_denom_yy);
    if (v9 > *error_sq)
        *error_sq = v9;
    if (v10 > *error_sq)
        *error_sq = v10;
}

void pulse_sum_contact::psc_cpi::SOLVER_solver_prolog(psc_cpi* self, int iter, float delta_t) {
    // ea: 0x897160
    vec2 vel;
    get_vel(self, &vel);
    m_right_side.x -= vel.x;
    m_right_side.y -= vel.y;
    if (m_pulse_sum_cache->m_visit_key != iter)
        m_pulse_sum_cache->m_pulse_sum = 0.0f;
    float m_pulse_sum_x = m_pulse_sum_cache->m_pulse_sum;
    pulse_sum_cache* v9 = m_pulse_sum_cache + 1;
    m_pulse_sum.x = m_pulse_sum_x * delta_t;
    if (v9->m_visit_key != iter)
        v9->m_pulse_sum = 0.0f;
    m_pulse_sum.y = v9->m_pulse_sum * delta_t;
    if (m_pulse_sum.x > 0.0f)
        m_pulse_sum.x = 0.0f;
    float v11 = 0.0f - (((pulse_sum_contact*)self)->m_fric_coef * m_pulse_sum.x);
    if (m_pulse_sum.y <= v11) {
        if ((0.0f - v11) > m_pulse_sum.y)
            m_pulse_sum.y = 0.0f - v11;
    } else {
        m_pulse_sum.y = v11;
    }
    apply(self, &m_pulse_sum);
}

void pulse_sum_contact::psc_cpi::SOLVER_solver_intermediate(psc_cpi* self, int iter, float delta_t) {
    float inv_dt = 1.0f / delta_t;
    m_pulse_sum_cache->m_pulse_sum = m_pulse_sum.x * inv_dt;
    m_pulse_sum_cache->m_visit_key = iter;
    m_pulse_sum_cache[1].m_pulse_sum = m_pulse_sum.y * inv_dt;
    m_pulse_sum_cache[1].m_visit_key = iter;
}

void pulse_sum_contact::psc_cpi::set_pulse_sum_cache(psc_cpi* self, pulse_sum_cache* cache) {
    m_pulse_sum_cache = cache;
}

const pulse_sum_contact::vec2* pulse_sum_contact::psc_cpi::get_vel(psc_cpi* self, vec2* result) {
    // ea: 0x896F00
    pulse_sum_contact* psc = (pulse_sum_contact*)self;
    pulse_sum_node* m_b2 = psc->m_b2;
    math::Dir3 b2_vel;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_vel.v = _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9), _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18), _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_vel.v = m_b2_ap_n.v;
    }
    rigid_body* m_rb = psc->m_b1->m_rb;
    __m128 m_a_vel = m_rb->m_a_vel.v;
    __m128 v10 = _mm_sub_ps(
        _mm_add_ps(
            m_rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 9), _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(m_a_vel, m_a_vel, 18), _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
        b2_vel.v);
    __m128 v11 = _mm_mul_ps(v10, m_ud_f1.v);
    __m128 v12 = _mm_mul_ps(v10, psc->m_ud_n.v);
    result->x = v12.m128_f32[0] + _mm_shuffle_ps(v12, v12, 85).m128_f32[0] + _mm_shuffle_ps(v12, v12, 170).m128_f32[0];
    result->y = v11.m128_f32[0] + _mm_shuffle_ps(v11, v11, 85).m128_f32[0] + _mm_shuffle_ps(v11, v11, 170).m128_f32[0];
    return result;
}

const pulse_sum_contact::vec2* pulse_sum_contact::psc_cpi::get_objective(psc_cpi* self, vec2* result) {
    // ea: 0x897020
    pulse_sum_contact* psc = (pulse_sum_contact*)self;
    pulse_sum_node* m_b1 = psc->m_b1;
    __m128 a_vel = m_b1->a_vel.v;
    __m128 t_vel = m_b1->t_vel.v;
    pulse_sum_node* m_b2 = psc->m_b2;
    __m128 v7 = _mm_add_ps(
        t_vel,
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 9), _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(a_vel, a_vel, 18), _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9))));
    if (m_b2 != NULL)
        v7 = _mm_sub_ps(
            v7,
            _mm_add_ps(
                m_b2->t_vel.v,
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 9), _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2->a_vel.v, m_b2->a_vel.v, 18), _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    __m128 v8 = _mm_mul_ps(v7, m_ud_f1.v);
    __m128 v10 = _mm_mul_ps(v7, psc->m_ud_n.v);
    result->x = v10.m128_f32[0] + _mm_shuffle_ps(v10, v10, 85).m128_f32[0] + _mm_shuffle_ps(v10, v10, 170).m128_f32[0];
    result->y = v8.m128_f32[0] + _mm_shuffle_ps(v8, v8, 85).m128_f32[0] + _mm_shuffle_ps(v8, v8, 170).m128_f32[0];
    return result;
}

void pulse_sum_contact::psc_cpi::apply(psc_cpi* self, const vec2* s_) {
    // ea: 0x892FB0 - apply 2D impulse (n + f1) to both bodies.
    pulse_sum_contact* psc = (pulse_sum_contact*)self;
    pulse_sum_node* m_b1 = psc->m_b1;
    __m128 n_imp = _mm_mul_ps(psc->m_ud_n.v, _mm_set1_ps(s_->x));
    __m128 f_imp = _mm_mul_ps(m_ud_f1.v, _mm_set1_ps(s_->y));
    m_b1->t_vel.v = _mm_add_ps(m_b1->t_vel.v, _mm_mul_ps(_mm_add_ps(n_imp, f_imp), _mm_set1_ps(m_b1->m_inv_mass)));
    m_b1->a_vel.v = _mm_add_ps(m_b1->a_vel.v, _mm_mul_ps(m_b1_ap_n.v, _mm_set1_ps(s_->x)));
    m_b1->a_vel.v = _mm_add_ps(m_b1->a_vel.v, _mm_mul_ps(m_b1_ap_f1.v, _mm_set1_ps(s_->y)));
    pulse_sum_node* m_b2 = psc->m_b2;
    if (m_b2 != NULL) {
        m_b2->t_vel.v = _mm_sub_ps(m_b2->t_vel.v, _mm_mul_ps(_mm_add_ps(n_imp, f_imp), _mm_set1_ps(m_b2->m_inv_mass)));
        m_b2->a_vel.v = _mm_sub_ps(m_b2->a_vel.v, _mm_mul_ps(m_b2_ap_n.v, _mm_set1_ps(s_->x)));
        m_b2->a_vel.v = _mm_sub_ps(m_b2->a_vel.v, _mm_mul_ps(m_b2_ap_f1.v, _mm_set1_ps(s_->y)));
    }
}

math::Dir3 pulse_sum_contact::psc_cpi::get_relative_velocity_change_dir(
    pulse_sum_contact* psc) {
    math::Dir3 v3;
    v3.v = psc->m_ud_n.v;
    pulse_sum_node* m_b2 = psc->m_b2;
    __m128 v5 = _mm_add_ps(
        _mm_mul_ps(v3.v, _mm_set1_ps(psc->m_b1->m_inv_mass)),
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_b1_ap_n.v, m_b1_ap_n.v, 9),
                       _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(m_b1_ap_n.v, m_b1_ap_n.v, 18),
                       _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9))));
    math::Dir3 result;
    result.v = v5;
    if (m_b2 != NULL) {
        result.v = _mm_add_ps(
            v5,
            _mm_add_ps(
                _mm_mul_ps(v3.v, _mm_set1_ps(m_b2->m_inv_mass)),
                _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(m_b2_ap_n.v, m_b2_ap_n.v, 9),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                    _mm_mul_ps(_mm_shuffle_ps(m_b2_ap_n.v, m_b2_ap_n.v, 18),
                               _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9)))));
    }
    return result;
}

math::Dir3 pulse_sum_contact::psc_cpi::get_last_relative_velocity(
    pulse_sum_contact* psc) {
    pulse_sum_node* m_b2 = psc->m_b2;
    math::Dir3 b2_ap_n;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_ap_n.v = _mm_add_ps(
            rb->m_last_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_ap_n.v = m_b2_ap_n.v;
    }
    rigid_body* rb = psc->m_b1->m_rb;
    math::Dir3 result;
    result.v = _mm_sub_ps(
        _mm_add_ps(
            rb->m_last_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 9),
                           _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_last_a_vel.v, rb->m_last_a_vel.v, 18),
                           _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
        b2_ap_n.v);
    return result;
}

math::Dir3 pulse_sum_contact::psc_cpi::get_relative_velocity(
    pulse_sum_contact* psc) {
    pulse_sum_node* m_b2 = psc->m_b2;
    math::Dir3 b2_ap_n;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        b2_ap_n.v = _mm_add_ps(
            rb->m_mat.w.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9))));
    } else {
        b2_ap_n.v = m_b2_ap_n.v;
    }
    rigid_body* rb = psc->m_b1->m_rb;
    math::Dir3 result;
    result.v = _mm_sub_ps(
        _mm_add_ps(
            rb->m_t_vel.v,
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 9),
                           _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(rb->m_a_vel.v, rb->m_a_vel.v, 18),
                           _mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9)))),
        b2_ap_n.v);
    return result;
}

float pulse_sum_contact::psc_cpi::get_impact_vel(
    pulse_sum_contact* psc, const math::Dir3* relative_velocity) {
    math::Dir3 last = get_last_relative_velocity(psc);
    __m128 n = psc->m_ud_n.v;
    __m128 v6 = _mm_mul_ps(last.v, n);
    float last_vel = v6.m128_f32[0]
                   + _mm_shuffle_ps(v6, v6, 85).m128_f32[0]
                   + _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
    __m128 v7 = _mm_mul_ps(relative_velocity->v, n);
    float vel = v7.m128_f32[0]
              + _mm_shuffle_ps(v7, v7, 85).m128_f32[0]
              + _mm_shuffle_ps(v7, v7, 170).m128_f32[0];
    return vel <= last_vel ? last_vel : vel;
}

float pulse_sum_contact::psc_cpi::get_impact_dist(pulse_sum_contact* psc) {
    pulse_sum_node* m_b2 = psc->m_b2;
    math::Dir3 b2_r;
    const math::Dir3* p_m_b2_r;
    if (m_b2 != NULL) {
        rigid_body* rb = m_b2->m_rb;
        if ((~(rb->m_flags >> 6) & 1) == 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                      "debug_flag_is_not_in_collision()", defaultFileName))
            __debugbreak();
        b2_r.v = _mm_add_ps(rb->m_mat.w.v, m_b2_r.v);
        p_m_b2_r = &b2_r;
    } else {
        p_m_b2_r = &m_b2_r;
    }
    rigid_body* rb = psc->m_b1->m_rb;
    if ((~(rb->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body.h", 79,
                  "debug_flag_is_not_in_collision()", defaultFileName))
        __debugbreak();
    __m128 v = _mm_mul_ps(
        _mm_sub_ps(_mm_add_ps(rb->m_mat.w.v, m_b1_r.v), p_m_b2_r->v),
        psc->m_ud_n.v);
    return v.m128_f32[0]
         + _mm_shuffle_ps(v, v, 85).m128_f32[0]
         + _mm_shuffle_ps(v, v, 170).m128_f32[0];
}

void pulse_sum_contact::psc_cpi::calc_fric_dir(
    pulse_sum_contact* psc, const math::Dir3* relative_velocity) {
    m_ud_f1.v = relative_velocity->v;
    math::Dir3 ud_n;
    ud_n.v = psc->m_ud_n.v;
    __m128 v6 = _mm_mul_ps(m_ud_f1.v, ud_n.v);
    float mag = v6.m128_f32[0]
              + _mm_shuffle_ps(v6, v6, 85).m128_f32[0]
              + _mm_shuffle_ps(v6, v6, 170).m128_f32[0];
    m_ud_f1.v = _mm_sub_ps(m_ud_f1.v, _mm_mul_ps(ud_n.v, _mm_set1_ps(mag)));
    __m128 v8 = _mm_mul_ps(m_ud_f1.v, m_ud_f1.v);
    float len = sqrt(v8.m128_f32[0]
                     + _mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                     + _mm_shuffle_ps(v8, v8, 170).m128_f32[0]);
    if (len < 0.000099999997f) {
        m_ud_f1 = get_relative_velocity_change_dir(psc);
        __m128 v11 = _mm_mul_ps(m_ud_f1.v, ud_n.v);
        float dot = v11.m128_f32[0]
                  + _mm_shuffle_ps(v11, v11, 85).m128_f32[0]
                  + _mm_shuffle_ps(v11, v11, 170).m128_f32[0];
        m_ud_f1.v = _mm_sub_ps(m_ud_f1.v, _mm_mul_ps(ud_n.v, _mm_set1_ps(dot)));
        __m128 v12 = _mm_mul_ps(m_ud_f1.v, m_ud_f1.v);
        len = sqrt(v12.m128_f32[0]
                   + _mm_shuffle_ps(v12, v12, 85).m128_f32[0]
                   + _mm_shuffle_ps(v12, v12, 170).m128_f32[0]);
        if (len < 0.000099999997f)
            m_ud_f1 = construct_orth_ud(ud_n);
    }
    m_ud_f1.v = _mm_mul_ps(m_ud_f1.v, _mm_set1_ps(1.0f / (len < 0.000099999997f ? 1.0f : len)));
}

void pulse_sum_contact::psc_cpi::calc_abs_and_fric_dir(
    pulse_sum_contact* psc, const math::Dir3* relative_velocity) {
    rigid_body* rb = psc->m_b1->m_rb;
    math::Dir3 b1_t_n;
    b1_t_n.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9),
                   _mm_shuffle_ps(psc->m_ud_n.v, psc->m_ud_n.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18),
                   _mm_shuffle_ps(psc->m_ud_n.v, psc->m_ud_n.v, 9)));
    rbint::inv_L(&m_b1_ap_n, rb, &b1_t_n);
    __m128 v5 = _mm_mul_ps(m_b1_ap_n.v, b1_t_n.v);
    m_denom_xx = psc->m_b1->m_inv_mass
               + v5.m128_f32[0]
               + _mm_shuffle_ps(v5, v5, 85).m128_f32[0]
               + _mm_shuffle_ps(v5, v5, 170).m128_f32[0];
    math::Dir3 b2_t_n;
    if (psc->m_b2 != NULL) {
        b2_t_n.v = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9),
                       _mm_shuffle_ps(psc->m_ud_n.v, psc->m_ud_n.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18),
                       _mm_shuffle_ps(psc->m_ud_n.v, psc->m_ud_n.v, 9)));
        rbint::inv_L(&m_b2_ap_n, psc->m_b2->m_rb, &b2_t_n);
        __m128 v8 = _mm_mul_ps(m_b2_ap_n.v, b2_t_n.v);
        m_denom_xx += psc->m_b2->m_inv_mass
                    + v8.m128_f32[0]
                    + _mm_shuffle_ps(v8, v8, 85).m128_f32[0]
                    + _mm_shuffle_ps(v8, v8, 170).m128_f32[0];
    }
    calc_fric_dir(psc, relative_velocity);
    math::Dir3 b1_t_f1;
    b1_t_f1.v = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 9),
                   _mm_shuffle_ps(m_ud_f1.v, m_ud_f1.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(m_b1_r.v, m_b1_r.v, 18),
                   _mm_shuffle_ps(m_ud_f1.v, m_ud_f1.v, 9)));
    rbint::inv_L(&m_b1_ap_f1, rb, &b1_t_f1);
    __m128 v11 = _mm_mul_ps(m_b1_ap_f1.v, b1_t_f1.v);
    __m128 v12 = _mm_mul_ps(m_b1_ap_f1.v, b1_t_n.v);
    m_denom_yy = psc->m_b1->m_inv_mass
               + v11.m128_f32[0]
               + _mm_shuffle_ps(v11, v11, 85).m128_f32[0]
               + _mm_shuffle_ps(v11, v11, 170).m128_f32[0];
    m_denom_xy = v12.m128_f32[0]
               + _mm_shuffle_ps(v12, v12, 85).m128_f32[0]
               + _mm_shuffle_ps(v12, v12, 170).m128_f32[0];
    if (psc->m_b2 != NULL) {
        math::Dir3 b2_t_f1;
        b2_t_f1.v = _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 9),
                       _mm_shuffle_ps(m_ud_f1.v, m_ud_f1.v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(m_b2_r.v, m_b2_r.v, 18),
                       _mm_shuffle_ps(m_ud_f1.v, m_ud_f1.v, 9)));
        rbint::inv_L(&m_b2_ap_f1, psc->m_b2->m_rb, &b2_t_f1);
        __m128 v15 = _mm_mul_ps(m_b2_ap_f1.v, b2_t_f1.v);
        __m128 v16 = _mm_mul_ps(m_b2_ap_f1.v, b2_t_n.v);
        m_denom_yy += psc->m_b2->m_inv_mass
                    + v15.m128_f32[0]
                    + _mm_shuffle_ps(v15, v15, 85).m128_f32[0]
                    + _mm_shuffle_ps(v15, v15, 170).m128_f32[0];
        m_denom_xy += v16.m128_f32[0]
                    + _mm_shuffle_ps(v16, v16, 85).m128_f32[0]
                    + _mm_shuffle_ps(v16, v16, 170).m128_f32[0];
    }
    if (m_denom_xx <= 0.0f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h",
                  58, "m_denom_xx > 0.0f", defaultFileName))
        __debugbreak();
    if (m_denom_yy <= 0.0f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_contact_new_inline.h",
                  59, "m_denom_yy > 0.0f", defaultFileName))
        __debugbreak();
}

void pulse_sum_contact::psc_cpi::setup_vel_uni_restitution(
    pulse_sum_contact* psc, const math::Dir3* relative_velocity, float restitution_k,
    float max_restitution_v, float delta_t, float max_penalty_restitution_vel) {
    float rv = get_impact_dist(psc);
    float dt = delta_t;
    if (delta_t <= 0.0041666669f)
        dt = 0.0041666669f;
    m_big_dirt = -(rv / dt);
    if (-(rv / dt) < 0.0f)
        m_big_dirt = (-(rv / dt)) * 0.30000001f;
    if (-max_penalty_restitution_vel > m_big_dirt)
        m_big_dirt = -max_penalty_restitution_vel;
    float big_dirt = m_big_dirt;
    if (big_dirt < 0.0f) {
        m_right_side.x = 0.0f;
        m_right_side.y = 0.0f;
    } else {
        m_right_side.x = big_dirt;
        m_right_side.y = 0.0f;
        m_big_dirt = 0.0f;
    }
    if (restitution_k > 0.0000099999997f && max_restitution_v > 0.0000099999997f && rv >= 0.0f) {
        float min_impact_vel = psc->m_b1->m_rb->m_gravity_multiplier;
        float rva = get_impact_vel(psc, relative_velocity);
        if (rva > min_impact_vel) {
            float v11 = rva * restitution_k;
            if (v11 > max_restitution_v)
                v11 = max_restitution_v;
            if (m_big_dirt > -0.0000099999997f) {
                m_right_side.x -= v11;
            } else if (m_big_dirt > -v11) {
                m_big_dirt = 0.0f;
                m_right_side.x = -v11;
            }
        }
    }
}

void pulse_sum_contact::psc_cpi::set_object_vel(psc_cpi* self, const math::Dir3* object_vel) {
    m_b2_ap_n.v = object_vel->v;
}

void pulse_sum_contact::psc_cpi::set_object_col_pt(psc_cpi* self, const math::Dir3* object_col_pt) {
    m_b2_r.v = object_col_pt->v;
}

// ============================================================================
// pulse_sum_constraint_solver methods
// ============================================================================
pulse_sum_node* pulse_sum_constraint_solver::create_pulse_sum_node() {
    // ea: 0x897A90
    char* v3 = (char*)(((intptr_t)m_solver_memory_allocater.m_buffer_cur + 15) & 0xFFFFFFF0);
    pulse_sum_node* v4;
    if ((v3 + 64) > m_solver_memory_allocater.m_buffer_end) {
        v4 = NULL;
    } else {
        m_solver_memory_allocater.m_buffer_cur = v3 + 64;
        v4 = (pulse_sum_node*)v3;
        if (v3 != NULL)
            goto alloc_ok;
    }
    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                  SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
        __debugbreak();
    if (v4 == NULL)
        return NULL;
alloc_ok:
    pulse_sum_node* result = ::new ((void*)v4) pulse_sum_node();
    pulse_sum_node* m_last = m_list_pulse_sum_node.m_last;
    if (m_last != NULL)
        m_last->m_link.m_next_link = result;
    else
        m_list_pulse_sum_node.m_first = result;
    m_list_pulse_sum_node.m_last = result;
    result->m_link.m_next_link = NULL;
    return result;
}

void pulse_sum_constraint_solver::set_solver_params(int psys_psc_visit_counter,
                                                    int psys_next_psc_visit_counter,
                                                    int psys_max_vel_iters,
                                                    int psys_max_vel_pos_iters) {
    m_psys_psc_visit_counter = psys_psc_visit_counter;
    m_psys_next_psc_visit_counter = psys_next_psc_visit_counter;
    m_psys_max_vel_iters = psys_max_vel_iters;
    m_psys_max_vel_pos_iters = psys_max_vel_pos_iters;
}

void pulse_sum_constraint_solver::list_urbri_restore(user_rigid_body_restore_info* list_urbri) {
    for (user_rigid_body_restore_info* i = list_urbri; i != NULL; i = i->m_next_link)
        *i->m_rbc_urb = i->m_original_urb;
}

void pulse_sum_constraint_solver::add_urb(temp_user_rigid_body** list_turb,
                                          user_rigid_body_restore_info** list_urbri,
                                          rigid_body_constraint* rbc) {
    // ea: 0x8980E0
    pulse_sum_constraint_solver* v4 = this;
    user_rigid_body** urb = NULL;
    if (rbc->b1 != NULL && (rbc->b1->m_flags & 0x20) != 0) {
        urb = (user_rigid_body**)&rbc->b1;
    } else if (rbc->b2 != NULL && (rbc->b2->m_flags & 0x20) != 0) {
        urb = (user_rigid_body**)&rbc->b2;
    }
    if (urb == NULL)
        return;
    temp_user_rigid_body* v8 = *list_turb;
    temp_user_rigid_body* v9 = NULL;
    if (*list_turb != NULL) {
        while (v9 == NULL) {
            if (v8->m_original_urb == *urb)
                v9 = v8;
            v8 = v8->m_next_link;
            if (v8 == NULL) {
                if (v9 != NULL)
                    break;
                goto alloc_urbri;
            }
        }
        goto alloc_urbri;
    }
alloc_urbri:
    if (v9 == NULL) {
        // Allocate a fresh temp_user_rigid_body (464 bytes, 0x1D0).
        char* v11 = (char*)(((intptr_t)v4->m_solver_memory_allocater.m_buffer_cur + 15) & 0xFFFFFFF0);
        if ((v11 + 464) > v4->m_solver_memory_allocater.m_buffer_end) {
            v9 = NULL;
        } else {
            v4->m_solver_memory_allocater.m_buffer_cur = v11 + 464;
            v9 = (temp_user_rigid_body*)v11;
            if (v11 != NULL)
                goto turb_ok;
        }
        if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                      SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
            __debugbreak();
        if (v9 == NULL)
            return;
turb_ok:
        user_rigid_body* v12 = *urb;
        temp_user_rigid_body* v13 = *list_turb;
        *(rigid_body*)v9 = *(rigid_body*)v12;
        v9->m_dictator = v12->m_dictator;
        v9->m_next_link = v13;
        v9->m_original_urb = v12;
        v9->m_flags |= 0x40;
        *list_turb = v9;
    }
    // Allocate restore info (12 bytes).
    char* v16 = (char*)(((intptr_t)v4->m_solver_memory_allocater.m_buffer_cur + 3) & 0xFFFFFFFC);
    user_rigid_body_restore_info* v17;
    if ((v16 + 12) > v4->m_solver_memory_allocater.m_buffer_end) {
        v17 = NULL;
    } else {
        v4->m_solver_memory_allocater.m_buffer_cur = v16 + 12;
        v17 = (user_rigid_body_restore_info*)v16;
        if (v16 != NULL)
            goto urbri_ok;
    }
    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89, "addr",
                  SOLVER_MEMORY_ALLOCATER_ERROR_MSG))
        __debugbreak();
    if (v17 == NULL)
        return;
urbri_ok:
    v17->m_next_link = *list_urbri;
    v17->m_rbc_urb = urb;
    v17->m_original_urb = *urb;
    *urb = v9;
    *list_urbri = v17;
}

void pulse_sum_constraint_solver::solve_constraints() {
    // ea: 0x897B10
    if (m_si.m_psc_visit_counter >= m_si.m_next_psc_visit_counter &&
        _tlAssert("c:/cod/code/tl/physics/include/constraint_solver\\pulse_sum_constraint_solver_inline.h", 115,
                  "m_si.m_psc_visit_counter < m_si.m_next_psc_visit_counter", ""))
        __debugbreak();
    for (pulse_sum_node* i = m_list_pulse_sum_node.m_first; i != NULL; i = i->m_link.m_next_link) {
        rbint::euler_integrate_velocity(i->m_rb, m_si.m_delta_t);
        i->t_vel.v = _mm_setzero_ps();
        i->a_vel.v = _mm_setzero_ps();
    }
    for (pulse_sum_normal* j = m_list_pulse_sum_normal.m_first; j != NULL; j = j->m_link.m_next_link) {
        float vel = j->get_vel();
        j->m_right_side = j->m_right_side - vel;
        if (j->m_pulse_sum_cache->m_visit_key != m_si.m_psc_visit_counter)
            j->m_pulse_sum_cache->m_pulse_sum = 0.0f;
        j->m_pulse_sum = j->m_pulse_sum_cache->m_pulse_sum * m_si.m_delta_t;
        j->m_pulse_sum = j->clamp_pulse_sum(j->m_pulse_sum);
        j->apply(&j->m_pulse_sum);
    }
    for (pulse_sum_point* k = m_list_pulse_sum_point.m_first; k != NULL; k = k->m_link.m_next_link)
        k->SOLVER_solver_prolog(m_si.m_psc_visit_counter, m_si.m_delta_t);
    for (pulse_sum_angular* m = m_list_pulse_sum_angular.m_first; m != NULL; m = m->m_link.m_next_link) {
        m->m_right_side = m->m_right_side - m->get_vel();
        if (m->m_pulse_sum_cache->m_visit_key != m_si.m_psc_visit_counter)
            m->m_pulse_sum_cache->m_pulse_sum = 0.0f;
        m->m_pulse_sum = m->m_pulse_sum_cache->m_pulse_sum * m_si.m_delta_t;
        m->apply(&m->m_pulse_sum);
    }
    for (pulse_sum_wheel* n = m_list_pulse_sum_wheel.m_first; n != NULL; n = n->m_link.m_next_link)
        n->SOLVER_solver_prolog(m_si.m_psc_visit_counter, m_si.m_delta_t);
    for (pulse_sum_contact* c = m_list_pulse_sum_contact.m_first;
         c != NULL; c = c->m_link.m_next_link) {
        for (pulse_sum_contact::psc_cpi* cp = c->m_list_cpi;
             cp != &c->m_list_cpi[c->m_list_cpi_count]; ++cp)
            cp->SOLVER_solver_prolog(cp, m_si.m_psc_visit_counter, m_si.m_delta_t);
    }
    solve_iterative(m_si.m_max_vel_iters, m_si.m_max_vel_error_sq);
    for (pulse_sum_normal* kk = m_list_pulse_sum_normal.m_first; kk != NULL; kk = kk->m_link.m_next_link) {
        kk->m_pulse_sum_cache->m_pulse_sum = kk->m_pulse_sum / m_si.m_delta_t;
        kk->m_pulse_sum_cache->m_visit_key = m_si.m_next_psc_visit_counter;
        kk->m_right_side = kk->m_big_dirt + kk->m_right_side;
    }
    for (pulse_sum_point* mm = m_list_pulse_sum_point.m_first; mm != NULL; mm = mm->m_link.m_next_link)
        mm->SOLVER_solver_intermediate(m_si.m_next_psc_visit_counter, m_si.m_delta_t);
    for (pulse_sum_angular* nn = m_list_pulse_sum_angular.m_first; nn != NULL; nn = nn->m_link.m_next_link) {
        nn->m_pulse_sum_cache->m_pulse_sum = nn->m_pulse_sum / m_si.m_delta_t;
        nn->m_pulse_sum_cache->m_visit_key = m_si.m_next_psc_visit_counter;
        nn->m_right_side = nn->m_big_dirt + nn->m_right_side;
    }
    for (pulse_sum_wheel* i1 = m_list_pulse_sum_wheel.m_first; i1 != NULL; i1 = i1->m_link.m_next_link)
        i1->SOLVER_solver_intermediate(m_si.m_next_psc_visit_counter, m_si.m_delta_t);
    for (pulse_sum_contact* i2 = m_list_pulse_sum_contact.m_first; i2 != NULL; i2 = i2->m_link.m_next_link) {
        for (pulse_sum_contact::psc_cpi* cp = i2->m_list_cpi;
             cp != &i2->m_list_cpi[i2->m_list_cpi_count]; ++cp)
            cp->SOLVER_solver_intermediate(cp, m_si.m_next_psc_visit_counter, m_si.m_delta_t);
    }
    solve_iterative(m_si.m_max_vel_pos_iters, m_si.m_max_vel_pos_error_sq);
    bool any_stable = false;
    for (pulse_sum_node* m_first = m_list_pulse_sum_node.m_first;
         m_first != NULL; m_first = m_first->m_link.m_next_link) {
        rigid_body* rb = m_first->m_rb;
        rb->m_mat.w.v = _mm_add_ps(rb->m_mat.w.v, m_first->t_vel.v);
        rb->m_a_vel.v = _mm_add_ps(rb->m_a_vel.v, m_first->a_vel.v);
        rbint::euler_integrate_pos(rb, m_si.m_delta_t);
        rbint::update_stability(rb, m_si.m_delta_t);
        if (!any_stable && (rb->m_flags & 4) != 0 && (rb->m_flags & 0x100) == 0)
            any_stable = true;
    }
    for (pulse_sum_node* i3 = m_list_pulse_sum_node.m_first; i3 != NULL; i3 = i3->m_link.m_next_link) {
        if (any_stable)
            i3->m_rb->m_flags |= 8u;
        else
            i3->m_rb->m_flags &= ~8u;
    }
}

void pulse_sum_constraint_solver::execute_constraint_solver(rigid_body* head) {
    // ea: 0x898250
    if (m_first_partition_head == NULL &&
        _tlAssert("c:/cod/code/tl/physics/include/constraint_solver\\pulse_sum_constraint_solver_inline.h", 197,
                  "m_first_partition_head", ""))
        __debugbreak();
    if (head == NULL &&
        _tlAssert("c:/cod/code/tl/physics/include/constraint_solver\\pulse_sum_constraint_solver_inline.h", 198,
                  "head", ""))
        __debugbreak();
    if (m_psys_psc_visit_counter >= m_psys_next_psc_visit_counter &&
        _tlAssert("c:/cod/code/tl/physics/include/constraint_solver\\pulse_sum_constraint_solver_inline.h", 199,
                  "m_psys_psc_visit_counter < m_psys_next_psc_visit_counter", ""))
        __debugbreak();

    rigid_body* v3 = head;
    float delta_t = v3->m_partition_node.m_group_delta_t;
    temp_user_rigid_body* list_turb = NULL;
    user_rigid_body_restore_info* list_urbri = NULL;
    m_solver_memory_allocater.m_buffer_cur = m_solver_memory_allocater.m_buffer_start;

    for (rigid_body* n = v3; n != NULL; n = n->m_partition_node.m_next_node)
        n->m_flags |= 0x40u;
    for (rigid_body_constraint_point* i = v3->m_partition_node.m_rbc_point_first;
         i != NULL; i = (rigid_body_constraint_point*)i->m_next)
        add_urb(&list_turb, &list_urbri, i);
    for (rigid_body_constraint_hinge* j = v3->m_partition_node.m_rbc_hinge_first;
         j != NULL; j = (rigid_body_constraint_hinge*)j->m_next) {
        add_urb(&list_turb, &list_urbri, j);
        j->do_collision(delta_t);
    }
    for (rigid_body_constraint_distance* k = v3->m_partition_node.m_rbc_dist_first;
         k != NULL; k = (rigid_body_constraint_distance*)k->m_next)
        add_urb(&list_turb, &list_urbri, k);
    for (rigid_body_constraint_ragdoll* m = v3->m_partition_node.m_rbc_ragdoll_first;
         m != NULL; m = (rigid_body_constraint_ragdoll*)m->m_next) {
        add_urb(&list_turb, &list_urbri, m);
        m->do_collision(delta_t);
    }
    for (rigid_body_constraint_wheel* n = v3->m_partition_node.m_rbc_wheel_first;
         n != NULL; n = (rigid_body_constraint_wheel*)n->m_next) {
        add_urb(&list_turb, &list_urbri, n);
        n->do_collision(delta_t);
    }
    for (rigid_body_constraint_angular_actuator* ii =
             v3->m_partition_node.m_rbc_angular_actuator_first;
         ii != NULL; ii = (rigid_body_constraint_angular_actuator*)ii->m_next)
        add_urb(&list_turb, &list_urbri, ii);
    for (rigid_body_constraint_custom_orientation* jj =
             v3->m_partition_node.m_rbc_custom_orientation_first;
         jj != NULL; jj = (rigid_body_constraint_custom_orientation*)jj->m_next)
        add_urb(&list_turb, &list_urbri, jj);
    for (rigid_body_constraint_custom_path* kk =
             v3->m_partition_node.m_rbc_custom_path_first;
         kk != NULL; kk = (rigid_body_constraint_custom_path*)kk->m_next)
        add_urb(&list_turb, &list_urbri, kk);
    for (rigid_body_constraint_contact* mm = v3->m_partition_node.m_rbc_contact_first;
         mm != NULL; mm = (rigid_body_constraint_contact*)mm->m_next)
        add_urb(&list_turb, &list_urbri, mm);

    for (rigid_body* v15 = v3; v15 != NULL; v15 = v15->m_partition_node.m_next_node)
        v15->m_flags &= ~0x40u;
    for (temp_user_rigid_body* nn = list_turb; nn != NULL; nn = nn->m_next_link)
        nn->m_flags &= ~0x40u;

    m_solver_memory_allocater.m_user_start = m_solver_memory_allocater.m_buffer_cur;
    int v17 = m_psys_max_vel_iters / v3->m_partition_node.m_sub_steps;
    if (v17 <= 1)
        v17 = 1;
    m_si.m_max_vel_iters = v17;
    int v18 = m_psys_max_vel_pos_iters / v3->m_partition_node.m_sub_steps;
    if (v18 <= 1)
        v18 = 1;
    m_si.m_max_vel_error_sq = 11.56f;
    m_si.m_max_vel_pos_error_sq = 289.0f;
    m_si.m_max_vel_pos_iters = v18;
    m_si.m_delta_t = delta_t / v3->m_partition_node.m_sub_steps;

    for (int step = 0; step < v3->m_partition_node.m_sub_steps; ++step) {
        m_solver_memory_allocater.m_buffer_cur = m_solver_memory_allocater.m_user_start;
        m_list_pulse_sum_node.remove_all();
        m_list_pulse_sum_normal.remove_all();
        m_list_pulse_sum_point.remove_all();
        m_list_pulse_sum_angular.remove_all();
        m_list_pulse_sum_wheel.remove_all();
        m_list_pulse_sum_contact.remove_all();
        for (rigid_body* v19 = v3; v19 != NULL; v19 = v19->m_partition_node.m_next_node)
            rbint::setup_constraint(v19, create_pulse_sum_node());
        for (rigid_body_constraint_point* i1 = v3->m_partition_node.m_rbc_point_first;
             i1 != NULL; i1 = (rigid_body_constraint_point*)i1->m_next)
            i1->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_hinge* i2 = v3->m_partition_node.m_rbc_hinge_first;
             i2 != NULL; i2 = (rigid_body_constraint_hinge*)i2->m_next)
            i2->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_distance* i3 = v3->m_partition_node.m_rbc_dist_first;
             i3 != NULL; i3 = (rigid_body_constraint_distance*)i3->m_next)
            i3->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_ragdoll* i4 = v3->m_partition_node.m_rbc_ragdoll_first;
             i4 != NULL; i4 = (rigid_body_constraint_ragdoll*)i4->m_next)
            i4->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_wheel* i5 = v3->m_partition_node.m_rbc_wheel_first;
             i5 != NULL; i5 = (rigid_body_constraint_wheel*)i5->m_next)
            i5->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_angular_actuator* i6 =
                 v3->m_partition_node.m_rbc_angular_actuator_first;
             i6 != NULL; i6 = (rigid_body_constraint_angular_actuator*)i6->m_next)
            i6->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_custom_orientation* i7 =
                 v3->m_partition_node.m_rbc_custom_orientation_first;
             i7 != NULL; i7 = (rigid_body_constraint_custom_orientation*)i7->m_next)
            i7->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_custom_path* i8 =
                 v3->m_partition_node.m_rbc_custom_path_first;
             i8 != NULL; i8 = (rigid_body_constraint_custom_path*)i8->m_next)
            i8->setup_constraint(this, m_si.m_delta_t);
        for (rigid_body_constraint_contact* i9 = v3->m_partition_node.m_rbc_contact_first;
             i9 != NULL; i9 = (rigid_body_constraint_contact*)i9->m_next)
            i9->setup_constraint(this, m_si.m_delta_t);
        int v30 = m_psys_psc_visit_counter + step;
        m_si.m_psc_visit_counter = v30;
        m_si.m_next_psc_visit_counter =
            (step + 1 == v3->m_partition_node.m_sub_steps) ? m_psys_next_psc_visit_counter : v30 + 1;
        solve_constraints();
        for (temp_user_rigid_body* i10 = list_turb; i10 != NULL; i10 = i10->m_next_link)
            rbint::substep((user_rigid_body*)i10, m_si.m_delta_t);
        for (rigid_body_constraint_distance* i13 = v3->m_partition_node.m_rbc_dist_first;
             i13 != NULL; i13 = (rigid_body_constraint_distance*)i13->m_next)
            i13->inner_update(m_si.m_delta_t);
        for (rigid_body_constraint_angular_actuator* i16 =
                 v3->m_partition_node.m_rbc_angular_actuator_first;
             i16 != NULL; i16 = (rigid_body_constraint_angular_actuator*)i16->m_next)
            i16->inner_update(m_si.m_delta_t);
    }
    for (user_rigid_body_restore_info* i20 = list_urbri; i20 != NULL; i20 = i20->m_next_link)
        *i20->m_rbc_urb = i20->m_original_urb;
    for (rigid_body_constraint_point* i21 = v3->m_partition_node.m_rbc_point_first;
         i21 != NULL; i21 = (rigid_body_constraint_point*)i21->m_next)
        i21->epilog_vel_constraint(delta_t);
    for (rigid_body_constraint_wheel* i25 = v3->m_partition_node.m_rbc_wheel_first;
         i25 != NULL; i25 = (rigid_body_constraint_wheel*)i25->m_next)
        i25->epilog_vel_constraint(delta_t);
}

// ============================================================================
// temp_user_rigid_body / user_rigid_body_restore_info
// ============================================================================
pulse_sum_constraint_solver::temp_user_rigid_body::temp_user_rigid_body() {
}

void pulse_sum_constraint_solver::temp_user_rigid_body::set(temp_user_rigid_body* next_link,
                                                            user_rigid_body* original_urb) {
    *(rigid_body*)this = *(rigid_body*)original_urb;
    m_dictator = original_urb->m_dictator;
    unsigned int m_flags = this->m_flags;
    m_original_urb = original_urb;
    m_next_link = next_link;
    this->m_flags = m_flags | 0x40;
}

void pulse_sum_constraint_solver::user_rigid_body_restore_info::set(
    user_rigid_body_restore_info* next_link, user_rigid_body** rbc_urb,
    temp_user_rigid_body* original_urb) {
    m_next_link = next_link;
    m_rbc_urb = rbc_urb;
    m_original_urb = original_urb;
}

void pulse_sum_constraint_solver::user_rigid_body_restore_info::restore() {
    *m_rbc_urb = m_original_urb;
}

// ============================================================================
// rbint helpers (COMDATs in this unit)
// ============================================================================
void rbint::euler_integrate_pos(rigid_body* const rb, float delta_t) {
    math::Dir3 v23;
    mul_L(&v23, rb, &rb->m_a_vel);
    rb->m_mat.w.v = _mm_add_ps(rb->m_mat.w.v, _mm_mul_ps(rb->m_t_vel.v, _mm_set1_ps(delta_t)));
    math::Mat43 rot;
    make_rotate(&rot, rb->m_a_vel, delta_t);
    math::Mat43 out;
    out.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 0), rot.x.v),
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 85), rot.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 170), rot.z.v));
    out.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.y.v, rb->m_mat.y.v, 0), rot.x.v),
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.y.v, rb->m_mat.y.v, 85), rot.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.y.v, rb->m_mat.y.v, 170), rot.z.v));
    out.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.z.v, rb->m_mat.z.v, 0), rot.x.v),
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.z.v, rb->m_mat.z.v, 85), rot.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.z.v, rb->m_mat.z.v, 170), rot.z.v));
    out.w.v = rb->m_mat.w.v;
    rb->m_mat = out;
    math::Dir3 v22;
    mul_inv_L(&v22, rb, &v23);
    rb->m_a_vel.v = v22.v;
    if (++rb->m_tick > 5) {
        rb->m_tick = 0;
        orthonormalize(&rb->m_mat);
    }
}

void rbint::setup_constraint(rigid_body* rb, pulse_sum_node* psn) {
    // ea: 0x897870 - attach node to body, cache velocity state.
    if (psn != NULL) {
        rb->m_node = psn;
        psn->m_rb = rb;
        psn->m_inv_mass = rb->m_inv_mass;
    }
}

void rbint::substep(user_rigid_body* rb, float delta_t) {
    // ea: 0x895210 - integrate pos with a-vel applied.
    euler_integrate_pos(rb, delta_t);
}

// ============================================================================
// pulse_sum_constraint_solver::psc_is_persistant - ea: 0x892140
// ============================================================================
bool pulse_sum_constraint_solver::psc_is_persistant(pulse_sum_cache* ps_cache, int visit_counter) {
    return ps_cache->m_visit_key == -1 || ps_cache->m_visit_key == visit_counter;
}

// ============================================================================
// pulse_sum_constraint_solver::set_pulse_sum / get_pulse_sum - ea: 0x8926A0/0x8926C0
// ============================================================================
void pulse_sum_constraint_solver::set_pulse_sum(pulse_sum_cache* ps_cache, int visit_counter,
                                                float pulse_sum) {
    ps_cache->m_pulse_sum = pulse_sum;
    ps_cache->m_visit_key = visit_counter;
}

float pulse_sum_constraint_solver::get_pulse_sum(pulse_sum_cache* ps_cache, int visit_counter) {
    if (ps_cache->m_visit_key != visit_counter)
        return 0.0f;
    return ps_cache->m_pulse_sum;
}

// ============================================================================
// rigid_body::operator= - ea: 0x892160 (inline COMDAT, 0x1B0-byte copy)
// ============================================================================
rigid_body& rigid_body::operator=(const rigid_body& other) {
    memcpy(this, &other, sizeof(rigid_body));
    return *this;
}
