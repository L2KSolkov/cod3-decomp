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
    (void)delta_t; (void)max_penalty_restitution_vel;
}
void pulse_sum_normal::setup_vel_uni_standard(
    float delta_t, float max_penalty_restitution_vel)
{
    (void)delta_t; (void)max_penalty_restitution_vel;
}
void pulse_sum_normal::setup_vel_uni_standard_pos_adjust(
    float delta_t, float pos, float max_penalty_restitution_vel)
{
    (void)delta_t; (void)pos; (void)max_penalty_restitution_vel;
}
void pulse_sum_normal::set_pulse_sum_limits_parent_ratio(
    float limit_ratio, pulse_sum_normal* parent)
{
    (void)limit_ratio; (void)parent;
}
void pulse_sum_wheel::set_side_fwd_ratios(float side_ratio, float fwd_ratio)
{
    (void)side_ratio; (void)fwd_ratio;
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
