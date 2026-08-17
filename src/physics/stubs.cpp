// AUTO-GENERATED STUBS — Physics engine (phys_xboxr)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_physics(void) {
    COD3_UNIMPLEMENTED("physics");
}

// pulse_sum solver stubs (phys_constraint_solver_multithreaded.o; port later)
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
const math::Dir3* pulse_sum_contact::psc_cpi::get_relative_velocity(
    psc_cpi* self, math::Dir3* result)
{
    (void)self; (void)result;
    return nullptr;
}
pulse_sum_angular* pulse_sum_constraint_solver::create_pulse_sum_angular(
    rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2,
    const math::Dir3* b2_r, const math::Dir3* ud, pulse_sum_cache* ps_cache)
{
    (void)b1; (void)b1_r; (void)b2; (void)b2_r; (void)ud; (void)ps_cache;
    return nullptr;
}
pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_normal()
{
    return nullptr;
}
pulse_sum_wheel* pulse_sum_constraint_solver::create_pulse_sum_wheel()
{
    return nullptr;
}
pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_wheel_side(
    pulse_sum_wheel* psw)
{
    (void)psw;
    return nullptr;
}
pulse_sum_normal* pulse_sum_constraint_solver::create_pulse_sum_wheel_fwd(
    pulse_sum_wheel* psw)
{
    (void)psw;
    return nullptr;
}
pulse_sum_contact* pulse_sum_constraint_solver::create_pulse_sum_contact(
    rigid_body* b1, rigid_body* b2, contact_point_info* cpi, float delta_t)
{
    (void)b1; (void)b2; (void)cpi; (void)delta_t;
    return nullptr;
}
void pulse_sum_constraint_solver::create_point(
    rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2,
    const math::Dir3* b2_r, pulse_sum_cache* ps_cache, float delta_t)
{
    (void)b1; (void)b1_r; (void)b2; (void)b2_r; (void)ps_cache; (void)delta_t;
}
void pulse_sum_constraint_solver::create_hinge(
    rigid_body* b1, const math::Dir3* b1_axis, rigid_body* b2,
    const math::Dir3* b2_axis, const math::Dir3* a1, const math::Dir3* a2,
    pulse_sum_cache* ps_cache, float delta_t)
{
    (void)b1; (void)b1_axis; (void)b2; (void)b2_axis; (void)a1; (void)a2;
    (void)ps_cache; (void)delta_t;
}

// construct_orth_ud (rbc_def_ragdoll.o inline; stub)
const math::Dir3 construct_orth_ud(const math::Dir3& v, const math::Dir3& ud)
{
    (void)v; (void)ud;
    math::Dir3 r = {};
    return r;
}
