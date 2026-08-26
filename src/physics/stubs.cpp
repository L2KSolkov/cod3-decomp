// Physics compatibility translation unit. Implementations live in the owning
// physics sources.

#include <stdio.h>
#include <math.h>
#include <intrin.h>

// pulse_sum solver implementations (phys_constraint_solver_multithreaded.o).
#include "physics/pulse_sum.h"

void pulse_sum_angular::setup_vel_uni_standard(
    float delta_t, float max_penalty_restitution_vel)
{
    if (delta_t <= 0.0041666669f)
        delta_t = 0.0041666669f;
    float delta_ta = -(get_pos() / delta_t);
    m_big_dirt = delta_ta;
    if (delta_ta < 0.0f)
        m_big_dirt = delta_ta * 0.30000001f;
    if ((0.0f - max_penalty_restitution_vel) > m_big_dirt)
        m_big_dirt = 0.0f - max_penalty_restitution_vel;
    float big_dirt = m_big_dirt;
    m_cfm = 0.0f;
    if (big_dirt < 0.0f) {
        m_right_side = 0.0f;
    } else {
        m_right_side = big_dirt;
        m_big_dirt = 0.0f;
    }
}
void pulse_sum_normal::setup_vel_uni_standard(
    float delta_t, float max_penalty_restitution_vel)
{
    if (delta_t <= 0.0041666669f)
        delta_t = 0.0041666669f;
    float delta_ta = -(get_pos() / delta_t);
    m_big_dirt = delta_ta;
    if (delta_ta < 0.0f)
        m_big_dirt = delta_ta * 0.30000001f;
    if ((0.0f - max_penalty_restitution_vel) > m_big_dirt)
        m_big_dirt = 0.0f - max_penalty_restitution_vel;
    float big_dirt = m_big_dirt;
    m_cfm = 0.0f;
    if (big_dirt < 0.0f) {
        m_right_side = 0.0f;
    } else {
        m_right_side = big_dirt;
        m_big_dirt = 0.0f;
    }
}
void pulse_sum_normal::setup_vel_uni_standard_pos_adjust(
    float delta_t, float pos, float max_penalty_restitution_vel)
{
    if (delta_t <= 0.0041666669f)
        delta_t = 0.0041666669f;
    float delta_ta = -((get_pos() + pos) / delta_t);
    m_big_dirt = delta_ta;
    if (delta_ta < 0.0f)
        m_big_dirt = delta_ta * 0.30000001f;
    if ((0.0f - max_penalty_restitution_vel) > m_big_dirt)
        m_big_dirt = 0.0f - max_penalty_restitution_vel;
    float big_dirt = m_big_dirt;
    m_cfm = 0.0f;
    if (big_dirt < 0.0f) {
        m_right_side = 0.0f;
    } else {
        m_right_side = big_dirt;
        m_big_dirt = 0.0f;
    }
}
void pulse_sum_normal::set_pulse_sum_limits_parent_ratio(
    float limit_ratio, pulse_sum_normal* parent)
{
    if (limit_ratio < 0.0f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
                  227, "limit_ratio >= 0.0f", defaultFileName))
        __debugbreak();
    if (parent == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_normal.h",
                  228, "parent", defaultFileName))
        __debugbreak();
    unsigned int flags = m_flags;
    m_pulse_limit_ratio = limit_ratio;
    m_pulse_parent = parent;
    m_flags = flags | 1;
    m_pulse_sum_min = 0.0f;
    m_pulse_sum_max = 0.0f;
}
void pulse_sum_wheel::set_side_fwd_ratios(float side_ratio, float fwd_ratio)
{
    if ((m_side == NULL || m_fwd == NULL) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\constraint_solver\\pulse_sum_wheel.h",
                  40, "m_side && m_fwd", defaultFileName))
        __debugbreak();
    m_side->m_pulse_limit_ratio = side_ratio;
    m_fwd->m_pulse_limit_ratio = fwd_ratio;
}
const __m128 Float4_XAxis_210 = {1.0f, 0.0f, 0.0f, 0.0f};
const __m128 Float4_YAxis_210 = {0.0f, 1.0f, 0.0f, 0.0f};
const __m128 Float4_ZAxis_210 = {0.0f, 0.0f, 1.0f, 0.0f};

// construct_orth_ud - ea: 0x88A2D0
math::Dir3 construct_orth_ud(const math::Dir3& ud)
{
    __m128 v2 = _mm_mul_ps(ud.v, ud.v);
    float ud_len = sqrt(v2.m128_f32[0]
                        + _mm_shuffle_ps(v2, v2, 85).m128_f32[0]
                        + _mm_shuffle_ps(v2, v2, 170).m128_f32[0]);
    if (fabs(ud_len - 1.0f) >= 0.000099999997f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 259,
                  "fabsf(nud - 1.0f) < .0001f", defaultFileName))
        __debugbreak();

    __m128 v = ud.v;
    __m128 v4 = _mm_shuffle_ps(v, v, 18);
    __m128 v5 = _mm_shuffle_ps(v, v, 9);
    __m128 v6 = _mm_sub_ps(
        _mm_mul_ps(v5, _mm_shuffle_ps(Float4_XAxis_210, Float4_XAxis_210, 18)),
        _mm_mul_ps(v4, _mm_shuffle_ps(Float4_XAxis_210, Float4_XAxis_210, 9)));
    __m128 v7 = _mm_mul_ps(v6, v6);
    float len = sqrt(v7.m128_f32[0]
                     + _mm_shuffle_ps(v7, v7, 85).m128_f32[0]
                     + _mm_shuffle_ps(v7, v7, 170).m128_f32[0]);
    if (len < 0.000099999997f) {
        v6 = _mm_sub_ps(
            _mm_mul_ps(v5, _mm_shuffle_ps(Float4_YAxis_210, Float4_YAxis_210, 18)),
            _mm_mul_ps(v4, _mm_shuffle_ps(Float4_YAxis_210, Float4_YAxis_210, 9)));
        __m128 v9 = _mm_mul_ps(v6, v6);
        len = sqrt(v9.m128_f32[0]
                   + _mm_shuffle_ps(v9, v9, 85).m128_f32[0]
                   + _mm_shuffle_ps(v9, v9, 170).m128_f32[0]);
        if (len < 0.000099999997f) {
            v6 = _mm_sub_ps(
                _mm_mul_ps(v5, _mm_shuffle_ps(Float4_ZAxis_210, Float4_ZAxis_210, 18)),
                _mm_mul_ps(v4, _mm_shuffle_ps(Float4_ZAxis_210, Float4_ZAxis_210, 9)));
            __m128 v11 = _mm_mul_ps(v6, v6);
            len = sqrt(v11.m128_f32[0]
                       + _mm_shuffle_ps(v11, v11, 85).m128_f32[0]
                       + _mm_shuffle_ps(v11, v11, 170).m128_f32[0]);
            if (len < 0.000099999997f &&
                _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 274,
                          "0", "not possible"))
                __debugbreak();
        }
    }
    math::Dir3 result;
    result.v = _mm_div_ps(v6, _mm_set1_ps(len));
    return result;
}
