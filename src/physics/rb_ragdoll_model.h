// ============================================================================
// rb_ragdoll_model.h - ragdoll model (phys_xboxr:rb_ragdoll_model.o, 31 funcs).
// ============================================================================

#ifndef COD3_PHYSICS_RB_RAGDOLL_MODEL_H
#define COD3_PHYSICS_RB_RAGDOLL_MODEL_H

#include "physics/phys_types.h"

// ============================================================================
// phys_static_array<T, CAP> - fixed inline array. class tag (V) matches the
// binary manglings (e.g. ?m_list_rigid_body@rb_ragdoll_model@@2V?$
// phys_static_array@PAVrigid_body@@$09@@A).
// m_buffer[40] + m_slot_array (points at buffer) + m_alloc_count.
// ============================================================================
template <typename T, int CAP>
class phys_static_array {
public:
    char m_buffer[CAP * sizeof(T)];  // +0x00
    T* const  m_slot_array;          // +CAP*sizeof(T)
    int       m_alloc_count;         // +CAP*sizeof(T)+4

    phys_static_array() : m_slot_array((T*)m_buffer), m_alloc_count(0) {}

    T& operator[](int i) { return ((T*)m_buffer)[i]; }  // ea: 0x87E1A0
    const T& operator[](int i) const { return ((T*)m_buffer)[i]; }
    int get_count() const { return m_alloc_count; }  // ea: 0x87E180

    struct iterator {
        T** m_ptr;  // +0x00

        iterator(T** ptr) : m_ptr(ptr) {}  // ea: 0x87E550
        T*& operator*() const { return *m_ptr; }  // ea: 0x87E3A0
        bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }  // ea: 0x87E380
        iterator& operator++() { ++m_ptr; return *this; }  // ea: 0x87E370
    };

    iterator begin() { return iterator((T**)m_buffer); }  // ea: 0x87E560
    iterator end() { return iterator(&((T**)m_buffer)[m_alloc_count]); }  // ea: 0x87E570

    T add(bool no_error, const char* error_msg) {  // ea: 0x87E3B0
        if (m_alloc_count < CAP) {
            T slot = ((T*)m_buffer)[m_alloc_count];
            m_alloc_count = m_alloc_count + 1;
            return slot;
        }
        if (!no_error)
            tlFatal(error_msg);
        return (T)NULL;
    }

    void push_back(const T& val) {  // ea: 0x87E590
        ((T*)m_buffer)[m_alloc_count] = val;
        m_alloc_count = m_alloc_count + 1;
    }

    void remove_all() { m_alloc_count = 0; }  // ea: 0x87E6B0
    void reset_buffer() { m_alloc_count = 0; }  // ea: 0x87E470

    T* find_by_val(const T& val) {  // ea: 0x87E1F0
        for (int i = 0; i < m_alloc_count; ++i) {
            if (((T*)m_buffer)[i] == val)
                return &((T*)m_buffer)[i];
        }
        return NULL;
    }
};
static_assert(sizeof(phys_static_array<void*, 10>) == 48, "phys_static_array size mismatch");

// ============================================================================
// rb_ragdoll_model (304 bytes, IDA ordinal 4812).
// ============================================================================
class rb_ragdoll_model {
public:
    phys_static_array<rigid_body*, 10> m_list_rigid_body;         // +0x00
    phys_static_array<rigid_body_constraint_ragdoll*, 10> m_joints;   // +0x30
    phys_static_array<rigid_body_constraint_angular_actuator*, 10> m_actuators;  // +0x60
    phys_static_array<user_rigid_body*, 10> m_list_user_rigid_body;   // +0x90
    phys_static_array<rigid_body_constraint*, 10> m_list_rbc_with_urb;  // +0xC0
    bool       m_stable;             // +0xF0
    float      m_stable_time;        // +0xF4
    bool       m_perfect_stable;     // +0xF8
    int        m_stable_body_count;  // +0xFC
    float      m_avg_te;             // +0x100
    float      m_contact_ratio;      // +0x104
    float      m_stability_ratio;    // +0x108
    math::Dir3 m_ballistic_target;   // +0x110
    int        m_ballistic_target_active;  // +0x120

    void reset_stability();
    void reset_ballistic_target();
    void set_ballistic_target(const math::Dir3* target);
    int  get_rigid_body_id(const rigid_body* const rb);
    rigid_body* add_rigid_body(int rb_id);
    // get_rigid_body - ea: 0x718CB0 (physics.o inline COMDAT)
    rigid_body* get_rigid_body(int rb_id)
    {
        if ((rb_id < 0 || rb_id >= m_list_rigid_body.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        return m_list_rigid_body.m_slot_array[rb_id];
    }
    rigid_body_constraint_ragdoll* add_joint(int rb_parent_id, int rb_id);
    rigid_body_constraint_angular_actuator* add_actuator(int rb_parent_id, int rb_id);
    void remove_actuator(int rb_id);
    // get_joint - ea: 0x718D20 (physics.o inline COMDAT)
    rigid_body_constraint_ragdoll* get_joint(int rb_id)
    {
        if ((rb_id < 0 || rb_id >= m_joints.m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                   108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        return m_joints.m_slot_array[rb_id];
    }
    void update_ballistic_target();
    void get_ballistic_info(math::Dir3* center_of_mass, math::Dir3* total_momentum,
                            float* total_mass);
    float get_gravity_multiplier(int rb_id);
    void set_gravity_multiplier(int rb_id, float g);
    void apply_pulse(int rb_id, const math::Dir3* pulse);
    void apply_pulse(int rb_id, const math::Dir3* hitp, const math::Dir3* pulse,
                     float torque_mult);
    void apply_pulse_damp_tvel(int rb_id, const math::Dir3* damp);
    void apply_pulse_damp_avel(int rb_id, const math::Dir3* damp);
    void apply_pulse_damp_tvel(const math::Dir3* damp);
    void apply_pulse_damp_avel(const math::Dir3* damp);
    void set_max_rb_index(int max_rb_index);
    void pull_joints_together();
    void remove_rigid_body(int rb_id);
    void add_rbc_with_urb_point(const math::Mat43* const dictator, int rb_id,
                                const math::Dir3* anchor_pt_loc, const math::Dir3* rb_pt_loc);
    void add_rbc_with_urb_point(const math::Mat43* const dictator, int rb_id,
                                const math::Dir3* pt_abs);
    void remove_all_user_rigid_body();
    void update_stability(float delta_t);
    void set_gravity_multiplier(float g);
    void apply_pulse_velocity_field(const math::Dir3* vel_);
    void disable_forces();
    void enable_forces();
    void set_time_scale(float time_scale);
    void set_max_delta_t(float max_delta_t);
    void add_user_rigid_body(user_rigid_body* rb);
    void add_rbc_with_urb(rigid_body_constraint* rbc);
};
static_assert(sizeof(rb_ragdoll_model) == 0x130, "rb_ragdoll_model size mismatch");

#endif // COD3_PHYSICS_RB_RAGDOLL_MODEL_H
