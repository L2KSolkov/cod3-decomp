// ============================================================================
// rb_ragdoll_model.cpp - ragdoll model (31 non-inline funcs).
// Source: source/rb_ragdoll_model.cpp (phys_xboxr:rb_ragdoll_model.o)
// Verified against IDA (phys_xboxr:rb_ragdoll_model.o).
// ============================================================================

#include "rb_ragdoll_model.h"
#include "physics_system.h"
#include "pulse_sum.h"

#include <math.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlFatal(const char* Format, ...);
extern const math::Dir3& Float4_SignMask_204;

namespace nuge {
void apply_ballistic_target(rigid_body* const* list_rigid_body, int rbodies_count,
                            const math::Dir3* target, float* dist_sq);
void get_ballistic_info(rigid_body* const* list_rigid_body, int rbodies_count,
                        math::Dir3* center_of_mass, math::Dir3* total_momentum,
                        float* total_mass);
}

// ============================================================================
// reset_stability - ea: 0x87C850
// ============================================================================
void rb_ragdoll_model::reset_stability() {
    m_stable = false;
    m_stable_time = 0.0f;
    m_perfect_stable = false;
    m_stable_body_count = 0;
    m_avg_te = 1000.0f;
    m_contact_ratio = 0.0f;
    m_stability_ratio = 0.0f;
}

// ============================================================================
// reset_ballistic_target / set_ballistic_target - ea: 0x87C890 / 0x87C8A0
// ============================================================================
void rb_ragdoll_model::reset_ballistic_target() {
    m_ballistic_target_active = 0;
}

void rb_ragdoll_model::set_ballistic_target(const math::Dir3* target) {
    m_ballistic_target.v = target->v;
    m_ballistic_target_active = 2;
}

// ============================================================================
// get_rigid_body_id - ea: 0x87C8E0
// ============================================================================
int rb_ragdoll_model::get_rigid_body_id(const rigid_body* const rb) {
    int m_alloc_count = m_list_rigid_body.m_alloc_count;
    int v4 = 0;
    if (m_alloc_count <= 0)
        return -1;
    while (1) {
        if ((v4 < 0 || v4 >= m_alloc_count) &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                      "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        if (rb == m_list_rigid_body[v4])
            break;
        m_alloc_count = m_list_rigid_body.m_alloc_count;
        if (++v4 >= m_alloc_count)
            return -1;
    }
    return v4;
}

// ============================================================================
// add_rigid_body - ea: 0x87C940
// ============================================================================
rigid_body* rb_ragdoll_model::add_rigid_body(int rb_id) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_id] != NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 65,
                  "m_list_rigid_body[rb_id] == NULL", ""))
        __debugbreak();
    rigid_body* rigid_body = phys_sys::create_rigid_body(false);
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    m_list_rigid_body[rb_id] = rigid_body;
    return rigid_body;
}

// ============================================================================
// add_joint - ea: 0x87C9E0
// ============================================================================
rigid_body_constraint_ragdoll* rb_ragdoll_model::add_joint(int rb_parent_id, int rb_id) {
    if ((rb_id < 0 || rb_id >= m_joints.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    int rb_ida = rb_id;
    if (m_joints[rb_id] != NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 101,
                  "m_joints[rb_id] == NULL", ""))
        __debugbreak();
    if ((rb_parent_id < 0 || rb_parent_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_parent_id] == NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 102,
                  "m_list_rigid_body[rb_parent_id] != NULL", ""))
        __debugbreak();
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_id] == NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 103,
                  "m_list_rigid_body[rb_id] != NULL", ""))
        __debugbreak();
    m_joints[rb_ida] = phys_sys::create_rbc_ragdoll(
        m_list_rigid_body[rb_parent_id], m_list_rigid_body[rb_id], false);
    return m_joints[rb_ida];
}

// ============================================================================
// add_actuator - ea: 0x87CBE0
// ============================================================================
rigid_body_constraint_angular_actuator* rb_ragdoll_model::add_actuator(
    int rb_parent_id, int rb_id) {
    if ((rb_id < 0 || rb_id >= m_actuators.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    int rb_ida = rb_id;
    if (m_actuators[rb_id] != NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 110,
                  "m_actuators[rb_id] == NULL", ""))
        __debugbreak();
    if (rb_parent_id != -1 &&
        (rb_parent_id < 0 || m_list_rigid_body[rb_parent_id] == NULL) &&
        _tlAssert("source/rb_ragdoll_model.cpp", 111,
                  "rb_parent_id == -1 || (rb_parent_id >= 0 && m_list_rigid_body[rb_parent_id] != NULL)", ""))
        __debugbreak();
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_ida] == NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 112,
                  "m_list_rigid_body[rb_id] != NULL", ""))
        __debugbreak();
    rigid_body* rb_parent;
    if (rb_parent_id == -1) {
        rb_parent = phys_sys::get_environment_rigid_body();
    } else {
        rb_parent = m_list_rigid_body[rb_parent_id];
    }
    m_actuators[rb_ida] = phys_sys::create_rbc_angular_actuator(
        rb_parent, m_list_rigid_body[rb_ida], false);
    return m_actuators[rb_ida];
}

// ============================================================================
// remove_actuator - ea: 0x87CDF0
// ============================================================================
void rb_ragdoll_model::remove_actuator(int rb_id) {
    if ((rb_id < 0 || rb_id >= m_actuators.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_actuators[rb_id] == NULL &&
        _tlAssert("source/rb_ragdoll_model.cpp", 120, "m_actuators[rb_id]", ""))
        __debugbreak();
    phys_sys::destroy(m_actuators[rb_id]);
    m_actuators[rb_id] = NULL;
}

// ============================================================================
// update_ballistic_target - ea: 0x87CEF0
// ============================================================================
void rb_ragdoll_model::update_ballistic_target() {
    int m_ballistic_target_active = this->m_ballistic_target_active;
    if (m_ballistic_target_active != 0) {
        int m_alloc_count = m_list_rigid_body.m_alloc_count;
        this->m_ballistic_target_active = m_ballistic_target_active - 1;
        if (m_alloc_count <= 0 &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                      "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        float dist_sq;
        nuge::apply_ballistic_target((rigid_body* const*)m_list_rigid_body.m_slot_array, m_alloc_count,
                                     &m_ballistic_target, &dist_sq);
        if (dist_sq < 2.25f)
            this->m_ballistic_target_active = 0;
    }
}

// ============================================================================
// get_ballistic_info - ea: 0x87CF70
// ============================================================================
void rb_ragdoll_model::get_ballistic_info(math::Dir3* center_of_mass,
                                          math::Dir3* total_momentum,
                                          float* total_mass) {
    if (((unsigned int)center_of_mass & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", ""))
        __debugbreak();
    if (((unsigned int)total_momentum & 0xF) != 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_math.h", 365,
                  "uint(v) % PHYS_ALIGNOF(phys_vec3) == 0", ""))
        __debugbreak();
    int m_alloc_count = m_list_rigid_body.m_alloc_count;
    if (m_alloc_count <= 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    nuge::get_ballistic_info((rigid_body* const*)m_list_rigid_body.m_slot_array, m_alloc_count,
                             center_of_mass, total_momentum, total_mass);
}

// ============================================================================
// get/set_gravity_multiplier - ea: 0x87D060 / 0x87D0B0
// ============================================================================
float rb_ragdoll_model::get_gravity_multiplier(int rb_id) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    return m_list_rigid_body[rb_id]->m_gravity_multiplier;
}

void rb_ragdoll_model::set_gravity_multiplier(int rb_id, float g) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    m_list_rigid_body[rb_id]->m_gravity_multiplier = g;
}

// ============================================================================
// apply_pulse - ea: 0x87D100 / 0x87D1C0
// ============================================================================
void rb_ragdoll_model::apply_pulse(int rb_id, const math::Dir3* pulse) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_id] != NULL) {
        reset_stability();
        m_list_rigid_body[rb_id]->add_force(*pulse);
    }
}

void rb_ragdoll_model::apply_pulse(int rb_id, const math::Dir3* hitp,
                                   const math::Dir3* pulse, float torque_mult) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_id] != NULL) {
        reset_stability();
        m_list_rigid_body[rb_id]->add_force(*pulse, *hitp, torque_mult);
    }
}

// ============================================================================
// apply_pulse_damp_tvel / apply_pulse_damp_avel - ea: 0x87D290 / 0x87D4D0
// ============================================================================
void rb_ragdoll_model::apply_pulse_damp_tvel(int rb_id, const math::Dir3* damp) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    rigid_body* rb = m_list_rigid_body[rb_id];
    if (rb != NULL) {
        reset_stability();
        float pulse[3];
        pulse[0] = -(1.0f - damp->v.m128_f32[0]) * rb->m_t_vel.v.m128_f32[0];
        pulse[1] = -(1.0f - damp->v.m128_f32[1]) * rb->m_t_vel.v.m128_f32[1];
        pulse[2] = -(1.0f - damp->v.m128_f32[2]) * rb->m_t_vel.v.m128_f32[2];
        rb->add_force(*(math::Dir3*)pulse);
    }
}

void rb_ragdoll_model::apply_pulse_damp_avel(int rb_id, const math::Dir3* damp) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    rigid_body* rb = m_list_rigid_body[rb_id];
    if (rb != NULL) {
        reset_stability();
        float torque[3];
        torque[0] = -(1.0f - damp->v.m128_f32[0]) * rb->m_a_vel.v.m128_f32[0];
        torque[1] = -(1.0f - damp->v.m128_f32[1]) * rb->m_a_vel.v.m128_f32[1];
        torque[2] = -(1.0f - damp->v.m128_f32[2]) * rb->m_a_vel.v.m128_f32[2];
        rb->add_torque(*(math::Dir3*)torque);
    }
}

void rb_ragdoll_model::apply_pulse_damp_tvel(const math::Dir3* damp) {
    for (int i = 0; i < m_list_rigid_body.m_alloc_count; ++i)
        apply_pulse_damp_tvel(i, damp);
}

void rb_ragdoll_model::apply_pulse_damp_avel(const math::Dir3* damp) {
    for (int i = 0; i < m_list_rigid_body.m_alloc_count; ++i)
        apply_pulse_damp_avel(i, damp);
}

// ============================================================================
// set_max_rb_index - ea: 0x87D770
// ============================================================================
void rb_ragdoll_model::set_max_rb_index(int max_rb_index) {
    if (m_list_rigid_body.m_alloc_count != 0 &&
        _tlAssert("source/rb_ragdoll_model.cpp", 7,
                  "m_list_rigid_body.get_count() == 0", ""))
        __debugbreak();
    if (m_joints.m_alloc_count != 0 &&
        _tlAssert("source/rb_ragdoll_model.cpp", 8,
                  "m_joints.get_count() == 0", ""))
        __debugbreak();
    if (m_actuators.m_alloc_count != 0 &&
        _tlAssert("source/rb_ragdoll_model.cpp", 9,
                  "m_actuators.get_count() == 0", ""))
        __debugbreak();
    for (int i = max_rb_index; i != 0; --i) {
        if (m_list_rigid_body.m_alloc_count < 10) {
            m_list_rigid_body[m_list_rigid_body.m_alloc_count] = NULL;
            m_list_rigid_body.m_alloc_count = m_list_rigid_body.m_alloc_count + 1;
        } else {
            tlFatal("phys array add overflow.");
        }
        if (m_joints.m_alloc_count < 10) {
            m_joints[m_joints.m_alloc_count] = NULL;
            m_joints.m_alloc_count = m_joints.m_alloc_count + 1;
        } else {
            tlFatal("phys array add overflow.");
        }
        if (m_actuators.m_alloc_count < 10) {
            m_actuators[m_actuators.m_alloc_count] = NULL;
            m_actuators.m_alloc_count = m_actuators.m_alloc_count + 1;
        } else {
            tlFatal("phys array add overflow.");
        }
    }
}

// ============================================================================
// pull_joints_together - ea: 0x87D890
// ============================================================================
void rb_ragdoll_model::pull_joints_together() {
    for (int i = 0; i < 50; ) {
        rigid_body_constraint_ragdoll** m_slot_array = (rigid_body_constraint_ragdoll**)m_joints.m_slot_array;
        int counter = i + 1;
        rigid_body_constraint_ragdoll** v4 = &m_slot_array[m_joints.m_alloc_count];
        float max_error_sq = 0.0f;
        if (v4 == m_slot_array)
            break;
        do {
            if (*m_slot_array != NULL) {
                float error_sq = (*m_slot_array)->pull_together();
                if (error_sq > max_error_sq)
                    max_error_sq = error_sq;
            }
            ++m_slot_array;
        } while (m_slot_array != v4);
        i = counter;
        if (max_error_sq <= 0.00001f)
            break;
    }
}

// ============================================================================
// remove_rigid_body - ea: 0x87D910
// ============================================================================
void rb_ragdoll_model::remove_rigid_body(int rb_id) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    rigid_body* v4 = m_list_rigid_body[rb_id];
    if (v4 != NULL) {
        for (rigid_body_constraint_ragdoll** i = (rigid_body_constraint_ragdoll**)m_joints.m_slot_array;
             i != &((rigid_body_constraint_ragdoll**)m_joints.m_slot_array)[m_joints.m_alloc_count]; ++i) {
            if (*i != NULL && ((*i)->b1 == v4 || (*i)->b2 == v4))
                *i = NULL;
        }
        for (rigid_body_constraint_angular_actuator** j = (rigid_body_constraint_angular_actuator**)m_actuators.m_slot_array;
             j != &((rigid_body_constraint_angular_actuator**)m_actuators.m_slot_array)[m_actuators.m_alloc_count]; ++j) {
            if (*j != NULL && ((*j)->b1 == v4 || (*j)->b2 == v4))
                *j = NULL;
        }
        for (rigid_body_constraint** k = (rigid_body_constraint**)m_list_rbc_with_urb.m_slot_array;
             k != &((rigid_body_constraint**)m_list_rbc_with_urb.m_slot_array)[m_list_rbc_with_urb.m_alloc_count]; ++k) {
            if (*k != NULL && ((*k)->b1 == v4 || (*k)->b2 == v4))
                *k = NULL;
        }
        phys_sys::destroy(m_list_rigid_body[rb_id]);
        m_list_rigid_body[rb_id] = NULL;
    }
}

// ============================================================================
// add_rbc_with_urb_point - ea: 0x87DA80 / 0x87DF30
// ============================================================================
void rb_ragdoll_model::add_rbc_with_urb_point(const math::Mat43* const dictator,
                                              int rb_id,
                                              const math::Dir3* anchor_pt_loc,
                                              const math::Dir3* rb_pt_loc) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    if (m_list_rigid_body[rb_id] != NULL) {
        user_rigid_body* user_rigid_body = phys_sys::get_user_rigid_body(dictator);
        if (user_rigid_body == NULL) {
            user_rigid_body = phys_sys::create_user_rigid_body(false);
            user_rigid_body->set(dictator);
        }
        rigid_body_constraint_point* rbc_point = phys_sys::create_rbc_point(
            m_list_rigid_body[rb_id], user_rigid_body, false);
        rbc_point->set(*rb_pt_loc, *anchor_pt_loc);
        add_user_rigid_body(user_rigid_body);
        add_rbc_with_urb(rbc_point);
    }
}

void rb_ragdoll_model::add_rbc_with_urb_point(const math::Mat43* const dictator,
                                              int rb_id, const math::Dir3* pt_abs) {
    if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc", 108,
                  "i >= 0 && i < m_alloc_count", ""))
        __debugbreak();
    rigid_body* rb = m_list_rigid_body[rb_id];
    if (rb != NULL) {
        // Convert absolute point into dictator-local and rb-local anchors.
        math::Mat43 invD;
        math::Dir3 anchor_pt_loc;
        math::Dir3 rb_pt_loc;
        // InvDictator transpose (dictator is orthonormal): anchor = inv(D)*(pt-w).
        math::Dir3 d;
        d.v = _mm_sub_ps(pt_abs->v, dictator->w.v);
        anchor_pt_loc.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 0), dictator->x.v),
                _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 85), dictator->y.v)),
            _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 170), dictator->z.v));
        d.v = _mm_sub_ps(pt_abs->v, rb->m_mat.w.v);
        rb_pt_loc.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 0), rb->m_mat.x.v),
                _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 85), rb->m_mat.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(d.v, d.v, 170), rb->m_mat.z.v));
        add_rbc_with_urb_point(dictator, rb_id, &anchor_pt_loc, &rb_pt_loc);
    }
}

// ============================================================================
// remove_all_user_rigid_body - ea: 0x87DB50
// ============================================================================
void rb_ragdoll_model::remove_all_user_rigid_body() {
    for (user_rigid_body** i = (user_rigid_body**)m_list_user_rigid_body.m_slot_array;
         i != &((user_rigid_body**)m_list_user_rigid_body.m_slot_array)[m_list_user_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            phys_sys::destroy(*i);
    }
    m_list_user_rigid_body.m_alloc_count = 0;
    m_list_rbc_with_urb.m_alloc_count = 0;
}

// ============================================================================
// update_stability - ea: 0x87DBA0
// ============================================================================
void rb_ragdoll_model::update_stability(float delta_t) {
    int v5 = 0;
    rigid_body** m_slot_array = (rigid_body**)m_list_rigid_body.m_slot_array;
    rigid_body** v4 = &m_slot_array[m_list_rigid_body.m_alloc_count];
    if (v4 != m_slot_array) {
        do {
            if (*m_slot_array != NULL)
                ++v5;
            ++m_slot_array;
        } while (m_slot_array != v4);
    }
    if (v5 <= 0 &&
        _tlAssert("source/rb_ragdoll_model.cpp", 167, "rbodies_count > 0", ""))
        __debugbreak();

    rigid_body** v6 = (rigid_body**)m_list_rigid_body.m_slot_array;
    rigid_body** v8 = &v6[m_list_rigid_body.m_alloc_count];
    m_avg_te = 0.0f;
    m_contact_ratio = 0.0f;
    for (rigid_body** v7 = v6; v7 != v8; ++v7) {
        rigid_body* v9 = *v7;
        if (v9 != NULL) {
            m_avg_te = v9->m_stable_te + m_avg_te;
            float v10 = 1.0f;
            if (v9->m_contact_count <= 0)
                v10 = 0.0f;
            m_contact_ratio = m_contact_ratio + v10;
        }
    }
    float v11 = (float)v5;
    float v12 = 1.0f / v5;
    m_avg_te = v12 * m_avg_te;
    m_contact_ratio = m_contact_ratio * v12;
    m_stable_body_count = 0;
    for (rigid_body** v6b = v6; v6b != v8; ++v6b) {
        if (*v6b != NULL && ((*v6b)->m_flags & 4) != 0)
            ++m_stable_body_count;
    }
    m_stability_ratio = m_stable_body_count / v11;
    m_perfect_stable = (m_stable_body_count == v5);
    if ((v11 * 0.8f) <= m_stable_body_count) {
        if (!m_stable) {
            float v14 = m_stable_time + delta_t;
            m_stable_time = v14;
            if (v14 >= 0.25f || m_perfect_stable)
                m_stable = true;
        }
    } else {
        m_stable_time = 0.0f;
        m_stable = false;
    }
}

// ============================================================================
// set_gravity_multiplier (all) - ea: 0x87DD50
// ============================================================================
void rb_ragdoll_model::set_gravity_multiplier(float g) {
    for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
         i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            (*i)->m_gravity_multiplier = g;
    }
}

// ============================================================================
// apply_pulse_velocity_field - ea: 0x87DD80
// ============================================================================
void rb_ragdoll_model::apply_pulse_velocity_field(const math::Dir3* vel_) {
    __m128 v4 = vel_->v;
    __m128 v5 = _mm_mul_ps(v4, v4);
    float v13 = v5.m128_f32[0] + _mm_shuffle_ps(v5, v5, 85).m128_f32[0] +
                _mm_shuffle_ps(v5, v5, 170).m128_f32[0];
    if (v13 > 0.0001f) {
        for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
             i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
            rigid_body* v8 = *i;
            if (v8 != NULL) {
                __m128 v9 = _mm_mul_ps(_mm_sub_ps(v4, v8->m_t_vel.v), v4);
                float dot = v9.m128_f32[0] + _mm_shuffle_ps(v9, v9, 85).m128_f32[0] +
                            _mm_shuffle_ps(v9, v9, 170).m128_f32[0];
                if (dot >= 0.0f) {
                    math::Dir3 v11;
                    v11.v = _mm_mul_ps(v4, _mm_set1_ps(dot / v13));
                    v8->add_force(v11);
                }
            }
        }
    }
}

// ============================================================================
// disable_forces / enable_forces / set_time_scale / set_max_delta_t
// ============================================================================
void rb_ragdoll_model::disable_forces() {
    for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
         i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            (*i)->m_flags |= 1u;
    }
}

void rb_ragdoll_model::enable_forces() {
    for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
         i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            (*i)->m_flags &= ~1u;
    }
}

void rb_ragdoll_model::set_time_scale(float time_scale) {
    for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
         i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            (*i)->m_time_scale.m_time = time_scale;
    }
}

void rb_ragdoll_model::set_max_delta_t(float max_delta_t) {
    for (rigid_body** i = (rigid_body**)m_list_rigid_body.m_slot_array;
         i != &((rigid_body**)m_list_rigid_body.m_slot_array)[m_list_rigid_body.m_alloc_count]; ++i) {
        if (*i != NULL)
            (*i)->m_max_delta_t = max_delta_t;
    }
}

// ============================================================================
// add_user_rigid_body / add_rbc_with_urb - ea: 0x87E7B0 / 0x87E830 (inline)
// ============================================================================
void rb_ragdoll_model::add_user_rigid_body(user_rigid_body* rb) {
    if (m_list_user_rigid_body.m_alloc_count < 10) {
        m_list_user_rigid_body[m_list_user_rigid_body.m_alloc_count] = rb;
        m_list_user_rigid_body.m_alloc_count = m_list_user_rigid_body.m_alloc_count + 1;
    }
}

void rb_ragdoll_model::add_rbc_with_urb(rigid_body_constraint* rbc) {
    if (m_list_rbc_with_urb.m_alloc_count < 10) {
        m_list_rbc_with_urb[m_list_rbc_with_urb.m_alloc_count] = rbc;
        m_list_rbc_with_urb.m_alloc_count = m_list_rbc_with_urb.m_alloc_count + 1;
    }
}
