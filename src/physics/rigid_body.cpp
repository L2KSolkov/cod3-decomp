// ============================================================================
// rigid_body.cpp â€” rigid body setup + force accumulation (7 non-inline funcs).
// Source: source/rigid_body.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rigid_body.o):
//   rigid_body::set_mass            @0x886180
//   user_rigid_body::set            @0x886200
//   rigid_body::set_inertia         @0x8863B0
//   rigid_body::set                 @0x886550
//   rigid_body::update_col_mat      @0x8869B0
//   rigid_body::add_force (3-arg)   @0x886A00
//   environment_rigid_body::set     @0x886C00
// ============================================================================

#include "pulse_sum.h"

#include <intrin.h>

// ============================================================================
// SSE constants (rdata COMDATs; referenced across the physics lib)
// ============================================================================
static const math::Dir3 c_Float4_Zero = { _mm_setzero_ps() };
static const math::Dir3 c_Float4_Two = { _mm_set1_ps(2.0f) };
static const math::Dir3 c_Float4_SignMask = { _mm_set1_ps(-0.0f) };
static const math::Position3 c_Float4_ZeroPos = { _mm_setzero_ps() };
static const math::Position3 c_Float4_OnePos = { _mm_set1_ps(1.0f) };
const math::Dir3& Float4_Zero_206 = c_Float4_Zero;
const math::Dir3& Float4_Zero_207 = c_Float4_Zero;
const math::Dir3& Float4_Zero_208 = c_Float4_Zero;
const math::Dir3& Float4_Zero_210 = c_Float4_Zero;
const math::Dir3& Float4_Zero_212 = c_Float4_Zero;
const math::Dir3& Float4_Zero_213 = c_Float4_Zero;
const math::Dir3& Float4_Two_208 = c_Float4_Two;
const math::Dir3& Float4_Two_212 = c_Float4_Two;
const math::Dir3& Float4_SignMask_207 = c_Float4_SignMask;
const math::Dir3& Float4_SignMask_210 = c_Float4_SignMask;
const math::Dir3& Float4_SignMask_213 = c_Float4_SignMask;
const math::Dir3& Float4_SignMask_214 = c_Float4_SignMask;
const math::Position3& Float4_Zero_2 = c_Float4_ZeroPos;
const math::Position3& Float4_One_2 = c_Float4_OnePos;

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern const math::Dir3& Float4_Zero_208;
extern const math::Dir3& Float4_Two_208;
extern void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m);

// ea: 0x886F80 (rigid_body.o COMDAT)
void rbint::calc_col_mat(rigid_body* const rb, const outer_time* outside_delta_t)
{
    float m_time = rb->m_time_scale.m_time * outside_delta_t->m_time;
    __m128 dt = _mm_shuffle_ps(_mm_set_ss(m_time), _mm_set_ss(m_time), 0);
    __m128 v8;
    if ((signed int)rb->m_flags >= 0)
    {
        v8 = _mm_mul_ps(rb->m_t_vel.v, dt);
    }
    else
    {
        __m128 inv = _mm_mul_ps(rb->m_force_sum.v,
                                _mm_shuffle_ps(_mm_set_ss(rb->m_inv_mass * m_time),
                                               _mm_set_ss(rb->m_inv_mass * m_time), 0));
        v8 = _mm_mul_ps(_mm_add_ps(rb->m_t_vel.v, inv), dt);
    }
    rb->m_col_mat.w.v = _mm_add_ps(rb->m_mat.w.v, v8);
    math::Mat43 rot;
    make_rotate(&rot, rb->m_a_vel, m_time);
    __m128 rx = rot.x.v;
    __m128 ry = rot.y.v;
    __m128 rz = rot.z.v;
    __m128 v14 = rb->m_mat.x.v;
    __m128 v15 = rb->m_mat.y.v;
    __m128 v16 = rb->m_mat.z.v;
    rb->m_col_mat.x.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v14, v14, 0), rx),
                   _mm_mul_ps(_mm_shuffle_ps(v14, v14, 85), ry)),
        _mm_mul_ps(_mm_shuffle_ps(v14, v14, 170), rz));
    rb->m_col_mat.y.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v15, v15, 0), rx),
                   _mm_mul_ps(_mm_shuffle_ps(v15, v15, 85), ry)),
        _mm_mul_ps(_mm_shuffle_ps(v15, v15, 170), rz));
    rb->m_col_mat.z.v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v16, v16, 0), rx),
                   _mm_mul_ps(_mm_shuffle_ps(v16, v16, 85), ry)),
        _mm_mul_ps(_mm_shuffle_ps(v16, v16, 170), rz));
}

// ============================================================================
// rigid_body::set_mass â€” ea: 0x886180
// ============================================================================
void rigid_body::set_mass(float mass) {
    if (mass <= 0.000001f &&
        _tlAssert("source/rigid_body.cpp", 17, "mass > 0.000001f", "")) {
        __debugbreak();
    }
    this->m_inv_mass = 1.0f / mass;
}

// ============================================================================
// user_rigid_body::set â€” ea: 0x886200
// ============================================================================
void user_rigid_body::set(const math::Mat43* dictator) {
    if (dictator == NULL &&
        _tlAssert("source/rigid_body.cpp", 85, "dictator", "")) {
        __debugbreak();
    }
    this->m_mat = *dictator;
    this->m_inv_mass = 0.0f;
    this->m_inv_inertia.v = Float4_Zero_208.v;
    this->m_t_vel.v = Float4_Zero_208.v;
    this->m_a_vel.v = Float4_Zero_208.v;
    this->m_last_t_vel.v = Float4_Zero_208.v;
    this->m_last_a_vel.v = Float4_Zero_208.v;
    this->m_dictator = dictator;
    this->m_time_scale.m_time = 1.0f;
    this->m_node = NULL;
    this->m_max_delta_t = 0.033898305f;
    this->m_flags = 32;
}

// ============================================================================
// rigid_body::set_inertia â€” ea: 0x8863B0
// ============================================================================
void rigid_body::set_inertia(const math::Dir3& inertia) {
    if (inertia.v.m128_f32[0] <= 0.000001f &&
        _tlAssert("source/rigid_body.cpp", 8, "inertia.GetX() > 0.000001f", "")) {
        __debugbreak();
    }
    if (_mm_shuffle_ps(inertia.v, inertia.v, 85).m128_f32[0] <= 0.000001f &&
        _tlAssert("source/rigid_body.cpp", 9, "inertia.GetY() > 0.000001f", "")) {
        __debugbreak();
    }
    if (_mm_shuffle_ps(inertia.v, inertia.v, 170).m128_f32[0] <= 0.000001f &&
        _tlAssert("source/rigid_body.cpp", 10, "inertia.GetZ() > 0.000001f", "")) {
        __debugbreak();
    }
    __m128 v3 = _mm_rcp_ps(inertia.v);
    this->m_inv_inertia.v = _mm_mul_ps(
        _mm_sub_ps(Float4_Two_208.v, _mm_mul_ps(v3, inertia.v)), v3);
}

// ============================================================================
// rigid_body::set â€” ea: 0x886550
// ============================================================================
void rigid_body::set(float mass, const math::Dir3& inertia, const math::Mat43& mat,
                     const math::Dir3& t_vel, const math::Dir3& a_vel, float fric_coef,
                     int stable_min_contact_count) {
    __m128 v = inertia.v;
    if (_mm_shuffle_ps(v, v, 85).m128_f32[0] == v.m128_f32[0]
        && _mm_shuffle_ps(v, v, 170).m128_f32[0] == v.m128_f32[0]) {
        // all components self-equal => no NaN
    } else if (_tlAssert("source/rigid_body.cpp", 26,
                         "(inertia.GetX() == inertia.GetX() && inertia.GetY() == inertia.GetY() && inertia.GetZ() == inertia.GetZ())",
                         "invalid vector")) {
        __debugbreak();
    }
    __m128 v11 = t_vel.v;
    if (_mm_shuffle_ps(v11, v11, 85).m128_f32[0] == v11.m128_f32[0]
        && _mm_shuffle_ps(v11, v11, 170).m128_f32[0] == v11.m128_f32[0]) {
    } else if (_tlAssert("source/rigid_body.cpp", 27,
                         "(t_vel.GetX() == t_vel.GetX() && t_vel.GetY() == t_vel.GetY() && t_vel.GetZ() == t_vel.GetZ())",
                         "invalid vector")) {
        __debugbreak();
    }
    __m128 v12 = a_vel.v;
    if (_mm_shuffle_ps(v12, v12, 85).m128_f32[0] == v12.m128_f32[0]
        && _mm_shuffle_ps(v12, v12, 170).m128_f32[0] == v12.m128_f32[0]) {
    } else if (_tlAssert("source/rigid_body.cpp", 28,
                         "(a_vel.GetX() == a_vel.GetX() && a_vel.GetY() == a_vel.GetY() && a_vel.GetZ() == a_vel.GetZ())",
                         "invalid vector")) {
        __debugbreak();
    }

    this->set_mass(mass);
    this->set_inertia(inertia);
    this->m_mat = mat;
    this->m_t_vel.v = t_vel.v;
    this->m_a_vel.v = a_vel.v;
    this->m_fric_coef = fric_coef;
    this->m_stable_min_contact_count = stable_min_contact_count;
    this->m_force_sum.v = Float4_Zero_208.v;
    this->m_torque_sum.v = Float4_Zero_208.v;
    this->m_last_t_vel.v = this->m_t_vel.v;
    this->m_last_a_vel.v = this->m_a_vel.v;
    this->m_gravity_multiplier = 1.0f;
    this->m_time_scale.m_time = 1.0f;
    this->m_max_delta_t = 0.033898305f;
    this->m_flags = 0;
    this->m_tick = 0;
    this->m_gravity_dir.v.m128_f32[0] = 0.0f;
    this->m_gravity_dir.v.m128_f32[1] = -1.0f;
    this->m_gravity_dir.v.m128_f32[2] = 0.0f;
    this->m_gravity_dir.v.m128_f32[3] = 0.0f;
    this->m_max_avel = 1000.0f;
    this->m_stable_energy_time = 0.0f;
    this->m_stable_te = 1000.0f;
    if ((g_physics_system->m_flags & 1) != 0) {
        outer_time v15;
        v15.m_time = g_physics_system->m_outside_sub_delta_t;
        rbint::calc_col_mat(this, &v15);
        this->m_flags |= 0x40u;
    }
    PHYS_ASSERT_ORTHONORMAL(&this->m_mat);
}

// ============================================================================
// rigid_body::update_col_mat â€” ea: 0x8869B0
// ============================================================================
void rigid_body::update_col_mat() {
    if ((this->m_flags & 0x50) == 0 &&
        _tlAssert("source/rigid_body.cpp", 69, "debug_flag_is_in_collision()", "")) {
        __debugbreak();
    }
    outer_time outside_delta_t;
    outside_delta_t.m_time = g_physics_system->m_outside_sub_delta_t;
    rbint::calc_col_mat(this, &outside_delta_t);
}

// ============================================================================
// rigid_body::add_force (3-arg) â€” ea: 0x886A00
// ============================================================================
void rigid_body::add_force(const math::Dir3& force, const math::Dir3& point,
                           float torque_mult) {
    if ((~(this->m_flags >> 6) & 1) == 0 &&
        _tlAssert("source/rigid_body.cpp", 78, "debug_flag_is_not_in_collision()", "")) {
        __debugbreak();
    }
    this->m_force_sum.v = _mm_add_ps(this->m_force_sum.v, force.v);
    __m128 v5 = _mm_sub_ps(point.v, this->m_mat.w.v);
    this->m_torque_sum.v = _mm_add_ps(
        this->m_torque_sum.v,
        _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v5, v5, 9), _mm_shuffle_ps(force.v, force.v, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v5, v5, 18), _mm_shuffle_ps(force.v, force.v, 9))),
            _mm_set1_ps(torque_mult)));
}

// ============================================================================
// environment_rigid_body::set â€” ea: 0x886C00
// ============================================================================
void environment_rigid_body::set() {
    this->m_inv_mass = 0.0f;
    this->m_inv_inertia.v = Float4_Zero_208.v;
    this->m_flags = 0;
    SetIdentity(this->m_mat);
    SetIdentity(this->m_col_mat);
    this->m_t_vel.v = Float4_Zero_208.v;
    this->m_a_vel.v = Float4_Zero_208.v;
    this->m_last_t_vel.v = Float4_Zero_208.v;
    this->m_last_a_vel.v = Float4_Zero_208.v;
    unsigned int m_flags = this->m_flags;
    this->m_time_scale.m_time = 1.0f;
    this->m_node = NULL;
    this->m_max_delta_t = 0.033898305f;
    this->m_flags = m_flags | 0x10;
}
