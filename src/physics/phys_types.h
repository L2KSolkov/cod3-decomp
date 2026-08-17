// ============================================================================
// COD3 Physics Types — rigid_body, constraint hierarchy, GJK cache, manifolds
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include <stddef.h>
#include <stdint.h>

extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlFatal(const char* Format, ...);
extern const char* const defaultFileName;

// ============================================================================
// outer_time — time wrapper used in rigid_body (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct outer_time {
    float m_time;  // +0x00

    outer_time() {}  // ea: 0x87E8C0
};
static_assert(sizeof(outer_time) == 4, "outer_time size mismatch");

// time_sub - ea: 0x88B0A0 (inline COMDAT)
inline float time_sub(const outer_time* t1, const outer_time* t2) {
    return t1->m_time - t2->m_time;
}

// ============================================================================
// phys_mem_info - engine memory budget (48 bytes, verified against IDA).
// ============================================================================
struct phys_mem_info {
    int m_num_rigid_body;                  // +0x00
    int m_num_user_rigid_body;             // +0x04
    int m_contact_point_buffer_size;       // +0x08
    int m_num_rbc_point;                   // +0x0C
    int m_num_rbc_hinge;                   // +0x10
    int m_num_rbc_dist;                    // +0x14
    int m_num_rbc_ragdoll;                 // +0x18
    int m_num_rbc_wheel;                   // +0x1C
    int m_num_rbc_angular_actuator;        // +0x20
    int m_num_rbc_custom_orientation;      // +0x24
    int m_num_rbc_custom_path;             // +0x28
    int m_num_rbc_contact;                 // +0x2C

    phys_mem_info();
};
static_assert(sizeof(phys_mem_info) == 0x30, "phys_mem_info size mismatch");

// ============================================================================
// phys_link_list_base<T> — intrusive singly-linked list head (4 bytes)
// ============================================================================
template <typename T>
struct phys_link_list_base {
    T* m_next_link;  // +0x00
};

// ============================================================================
// phys_simple_link_list<T> — simple list head (4 bytes)
// ============================================================================
template <typename T>
struct phys_simple_link_list {
    T* m_first;  // +0x00
};

// ============================================================================
// pulse_sum_cache — cached pulse-sum result per constraint (8 bytes)
// Size: 0x08 (8 bytes) — verified against IDA
// ============================================================================
struct pulse_sum_cache {
    int   m_visit_key;  // +0x00
    float m_pulse_sum;  // +0x04
};
static_assert(sizeof(pulse_sum_cache) == 8, "pulse_sum_cache size mismatch");

class pulse_sum_constraint_solver;  // defined in pulse_sum.h

// ============================================================================
// pulse_sum_node — solver node attached to a rigid_body (64 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct pulse_sum_node {
    phys_link_list_base<pulse_sum_node> m_link;   // +0x00
    uint8_t      _pad4[12];                       // +0x04 (align to 16)
    math::Dir3   t_vel;                           // +0x10
    math::Dir3   a_vel;                           // +0x20
    float        m_inv_mass;                      // +0x30
    struct rigid_body* m_rb;                      // +0x34
    uint8_t      _pad38[8];                       // +0x38

    pulse_sum_node() {}  // ea: 0x892480
};
static_assert(sizeof(pulse_sum_node) == 0x40, "pulse_sum_node size mismatch");
static_assert(offsetof(pulse_sum_node, t_vel) == 0x10, "pulse_sum_node::t_vel offset mismatch");
static_assert(offsetof(pulse_sum_node, m_inv_mass) == 0x30, "pulse_sum_node::m_inv_mass offset mismatch");

// ============================================================================
// pulse_sum_normal — constraint row solver data (160 bytes)
// Size: 0xA0 (160 bytes) — verified against IDA
// ============================================================================
struct pulse_sum_normal {
    phys_link_list_base<pulse_sum_normal> m_link;  // +0x00
    uint8_t      _pad4[12];                        // +0x04 (align to 16)
    math::Dir3   m_ud;                             // +0x10
    math::Dir3   m_b1_r;                           // +0x20
    math::Dir3   m_b2_r;                           // +0x30
    math::Dir3   m_b1_ap;                          // +0x40
    math::Dir3   m_b2_ap;                          // +0x50
    float        m_pulse_sum_min;                  // +0x60
    float        m_pulse_sum_max;                  // +0x64
    float        m_pulse_sum;                      // +0x68
    float        m_right_side;                     // +0x6C
    float        m_big_dirt;                       // +0x70
    float        m_cfm;                            // +0x74
    float        m_denom;                          // +0x78
    float        m_pulse_limit_ratio;              // +0x7C
    unsigned int m_flags;                          // +0x80
    struct pulse_sum_normal* m_pulse_parent;       // +0x84
    struct pulse_sum_node*   m_b1;                 // +0x88
    struct pulse_sum_node*   m_b2;                 // +0x8C
    pulse_sum_cache*         m_pulse_sum_cache;    // +0x90
    uint8_t      _pad94[12];                       // +0x94

    float get_pos() const { return m_pulse_sum; }
    void  set(rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2, const math::Dir3* b2_r,
              const math::Dir3* ud, pulse_sum_cache* ps_cache, const math::Dir3* b1_r_displace);
    void  setup_vel_uni_standard(float delta_t, float max_penalty_restitution_vel);
    void  setup_vel_uni_standard_pos_adjust(float delta_t, float pos, float max_penalty_restitution_vel);
    const float& get_unclamped_pulse_sum() const { return m_pulse_sum; }
    const math::Dir3* get_relative_velocity(const math::Dir3* result);
    const math::Dir3* get_relative_velocity_change_dir(math::Dir3* result);
    void  set_pulse_sum_limits_parent_ratio(float limit_ratio, pulse_sum_normal* parent);

    // solver methods (phys_constraint_solver_multithreaded.o)
    float get_vel();
    float get_last_vel();
    float get_pos();
    float get_objective();
    float clamp_pulse_sum(float ps);
    void  apply(const float* s_);
    void  calc_abs(const math::Dir3* b1_r_displace);
    float get_unclamped_pulse_sum();
    void  set_object_vel(const math::Dir3* object_vel);
    void  set_object_col_pt(const math::Dir3* object_col_pt);
    void  SOLVER_apply_relaxation(float* error_sq, bool add_error);
    void  SOLVER_solver_intermediate(int iter, float delta_t);
    void  SOLVER_solver_prolog(int iter, float delta_t);
    void  project();
};
static_assert(sizeof(pulse_sum_normal) == 0xA0, "pulse_sum_normal size mismatch");
static_assert(offsetof(pulse_sum_normal, m_ud) == 0x10, "pulse_sum_normal::m_ud offset mismatch");

// ============================================================================
// phys_memory_heap — linear frame allocator used by collision (16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct phys_memory_heap {
    char* m_buffer_start;  // +0x00
    char* m_buffer_end;    // +0x04
    char* m_buffer_cur;    // +0x08
    char* m_user_start;    // +0x0C

    // allocate - ea: 0x718AF0 (inline COMDAT, physics.o)
    void* allocate(int size, int alignment, bool no_error,
                   const char* error_msg);

    void* fast_align_start(int alignment, const char* error_msg);

    // allocate_no_error - ea: 0x65FC20 (inline COMDAT, game.o)
    char* allocate_no_error(int size, int alignment) {
        if (size <= 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 59,
                      "size > 0", ""))
            __debugbreak();
        char* result = (char*)((~(alignment - 1)) & (intptr_t)&m_buffer_cur[alignment - 1]);
        if (&result[size] > m_buffer_end)
            return NULL;
        m_buffer_cur = &result[size];
        return result;
    }

    // allocate_buffer - ea: 0x88F500 (inline COMDAT)
    void allocate_buffer(int size, int alignment, phys_memory_heap* allocater) {
        char* no_error = allocater->allocate_no_error(size, alignment);
        if (no_error == NULL &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 89,
                      "addr", "phys_memory_heap overflow."))
            __debugbreak();
        set_buffer(no_error, size, alignment);
    }

    // set_buffer - ea: 0x601DE0 (inline COMDAT, game.o)
    void set_buffer(char* start, int size, unsigned int alignment) {
        if (this->m_buffer_start != NULL &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 32,
                      "m_buffer_start == NULL", ""))
            __debugbreak();
        if (this->m_buffer_end != NULL &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 33,
                      "m_buffer_end == NULL", ""))
            __debugbreak();
        if (this->m_buffer_cur != NULL &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 34,
                      "m_buffer_cur == NULL", ""))
            __debugbreak();
        if (size <= 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 35,
                      "size > 0", ""))
            __debugbreak();
        if (((size_t)start) % alignment != 0 &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 36,
                      "((size_t)start) % alignment == 0", ""))
            __debugbreak();
        this->m_buffer_start = start;
        this->m_buffer_cur = start;
        this->m_user_start = start;
        this->m_buffer_end = &start[size];
    }
};
static_assert(sizeof(phys_memory_heap) == 0x10, "phys_memory_heap size mismatch");

// ============================================================================
// rb_inplace_partition_node — spatial partition node (64 bytes, IDA ordinal 4806).
// ============================================================================
struct rigid_body_constraint_point;
struct rigid_body_constraint_hinge;
struct rigid_body_constraint_distance;
struct rigid_body_constraint_ragdoll;
struct rigid_body_constraint_wheel;
struct rigid_body_constraint_angular_actuator;
struct rigid_body_constraint_custom_orientation;
struct rigid_body_constraint_custom_path;
struct rigid_body_constraint_contact;

struct rb_inplace_partition_node {
    rigid_body_constraint_point*   m_rbc_point_first;          // +0x00
    rigid_body_constraint_hinge*   m_rbc_hinge_first;          // +0x04
    rigid_body_constraint_distance* m_rbc_dist_first;          // +0x08
    rigid_body_constraint_ragdoll* m_rbc_ragdoll_first;        // +0x0C
    rigid_body_constraint_wheel*   m_rbc_wheel_first;          // +0x10
    rigid_body_constraint_angular_actuator* m_rbc_angular_actuator_first;  // +0x14
    rigid_body_constraint_custom_orientation* m_rbc_custom_orientation_first; // +0x18
    rigid_body_constraint_custom_path* m_rbc_custom_path_first; // +0x1C
    rigid_body_constraint_contact* m_rbc_contact_first;        // +0x20
    rigid_body*  m_partition_head;                             // +0x24
    rigid_body*  m_partition_tail;                             // +0x28
    rigid_body*  m_next_node;                                  // +0x2C
    int          m_partition_size;                             // +0x30
    int          m_sub_steps;                                  // +0x34
    float        m_group_delta_t;                              // +0x38
    rigid_body*  m_next_partition_head;                        // +0x3C
};
static_assert(sizeof(rb_inplace_partition_node) == 0x40, "rb_inplace_partition_node size mismatch");

// ============================================================================
// rigid_body — rigid body physics object (432 bytes)
// Size: 0x1B0 (432 bytes) — verified against IDA
// ============================================================================
class rigid_body {
public:
    math::Mat43  m_mat;                       // +0x000 — world transform
    math::Mat43  m_col_mat;                   // +0x040 — collision transform
    math::Mat33  m_world_inv_inertia;         // +0x080 — world-space inverse inertia
    math::Dir3   m_inv_inertia;               // +0x0B0 — local inverse inertia
    math::Dir3   m_gravity_dir;               // +0x0C0 — gravity direction
    math::Dir3   m_t_vel;                     // +0x0D0 — translational velocity
    math::Dir3   m_a_vel;                     // +0x0E0 — angular velocity
    math::Dir3   m_last_t_vel;                // +0x0F0 — previous frame t-vel
    math::Dir3   m_last_a_vel;                // +0x100 — previous frame a-vel
    math::Dir3   m_force_sum;                 // +0x110 — accumulated force
    math::Dir3   m_torque_sum;                // +0x120 — accumulated torque
    float        m_inv_mass;                  // +0x130
    float        m_gravity_multiplier;        // +0x134
    float        m_max_avel;                  // +0x138
    outer_time   m_time_scale;                // +0x13C
    float        m_max_delta_t;               // +0x140
    unsigned int m_flags;                     // +0x144
    float        m_fric_coef;                 // +0x148
    unsigned int m_tick;                      // +0x14C
    pulse_sum_node* m_node;                   // +0x150
    int32_t      m_constraint_count;          // +0x154
    int32_t      m_contact_count;             // +0x158
    int32_t      m_stable_min_contact_count;  // +0x15C
    float        m_stable_energy_time;        // +0x160
    float        m_stable_te;                 // +0x164
    rb_inplace_partition_node m_partition_node;  // +0x168 (64 bytes)

    void add_force(const math::Dir3& force);
    void add_force(const math::Dir3& force, const math::Dir3& point, float torque_mult);
    void add_torque(const math::Dir3& torque)  // inline COMDAT 0x87C6F0 (rb_ragdoll_model.o)
    {
        if ((torque.v.m128_f32[0] != torque.v.m128_f32[0]
             || torque.v.m128_f32[1] != torque.v.m128_f32[1]
             || torque.v.m128_f32[2] != torque.v.m128_f32[2])
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 154,
                         "(torque.GetX() == torque.GetX() && torque.GetY() == torque.GetY() && torque.GetZ() == torque.GetZ())",
                         "invalid vector"))
            __debugbreak();
        if ((~(m_flags >> 6) & 1) == 0
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 155,
                         "debug_flag_is_not_in_collision()", defaultFileName))
            __debugbreak();
        m_torque_sum.v = _mm_add_ps(m_torque_sum.v, torque.v);
    }
    void set_mass(float mass);
    void set_inertia(const math::Dir3& inertia);
    void set(float mass, const math::Dir3& inertia, const math::Mat43& mat,
             const math::Dir3& t_vel, const math::Dir3& a_vel, float fric_coef,
             int stable_min_contact_count);
    void update_col_mat();

    // get_mat / dangerous_get_mat - ea: 0x6E4D00 / 0x6E4D50 (inline COMDATs)
    const math::Mat43& get_mat() const { return m_mat; }  // ?get_mat@rigid_body@@QBEABVMat43@math@@XZ
    math::Mat43& dangerous_get_mat() { return m_mat; }    // ?dangerous_get_mat@rigid_body@@QAEAAVMat43@math@@XZ

    // translate_col_mat - ea: 0x718B90 (physics.o inline COMDAT)
    void translate_col_mat(const math::Dir3& t)  // ?translate_col_mat@rigid_body@@QAEXABVDir3@math@@@Z
    {
        if ((t.v.m128_f32[0] != t.v.m128_f32[0]
             || t.v.m128_f32[1] != t.v.m128_f32[1]
             || t.v.m128_f32[2] != t.v.m128_f32[2])
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 119,
                   "(t.GetX() == t.GetX() && t.GetY() == t.GetY() && t.GetZ() == t.GetZ())",
                   "invalid vector"))
            __debugbreak();
        if ((~(m_flags >> 6) & 1) == 0
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\rigid_body.h", 120,
                   "debug_flag_is_in_collision()", defaultFileName))
            __debugbreak();
        m_col_mat.w.v = _mm_add_ps(m_col_mat.w.v, t.v);
    }

    // set_gravity_dir / set_max_avel - ea: 0x6E4EA0 / 0x6E4EC0 (inline COMDATs)
    void set_gravity_dir(const math::Dir3& d)  // ?set_gravity_dir@rigid_body@@QAEXABVDir3@math@@@Z
    {
        m_gravity_dir.v = d.v;
    }
    void set_max_avel(float max_avel)  // ?set_max_avel@rigid_body@@QAEXM@Z
    {
        m_max_avel = max_avel;
    }

    rigid_body() {}  // ea: 0x880C60
    rigid_body& operator=(const rigid_body& other);  // ea: 0x892160

    // get_time_scale / get_max_delta_t - ea: 0x88B0B0 / 0x88B0C0
    const outer_time* get_time_scale() const { return &m_time_scale; }
    float get_max_delta_t() const { return m_max_delta_t; }
};
static_assert(sizeof(rigid_body) == 0x1B0, "rigid_body size mismatch");
static_assert(offsetof(rigid_body, m_mat) == 0x000, "rigid_body::m_mat offset mismatch");
static_assert(offsetof(rigid_body, m_inv_mass) == 0x130, "rigid_body::m_inv_mass offset mismatch");
static_assert(offsetof(rigid_body, m_flags) == 0x144, "rigid_body::m_flags offset mismatch");
static_assert(offsetof(rigid_body, m_node) == 0x150, "rigid_body::m_node offset mismatch");
static_assert(offsetof(rigid_body, m_partition_node) == 0x168, "rigid_body::m_partition_node offset mismatch");

// ============================================================================
// rigid_body_pair_key — body-pair lookup key (8 bytes)
// ============================================================================
struct rigid_body_pair_key {
    rigid_body* m_b1;  // +0x00
    rigid_body* m_b2;  // +0x04

    rigid_body_pair_key() {}
    rigid_body_pair_key(rigid_body* b1, rigid_body* b2) : m_b1(b1), m_b2(b2) {}
};
static_assert(sizeof(rigid_body_pair_key) == 8, "rigid_body_pair_key size mismatch");

// ============================================================================
// rigid_body_constraint — base constraint (12 bytes)
// Size: 0x0C (12 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint {
    rigid_body* b1;                        // +0x00
    rigid_body* b2;                        // +0x04
    rigid_body_constraint* m_next;         // +0x08

    // outer_prolog_update / outer_epilog_update - ea: 0x88B100 / 0x88B110
    void outer_prolog_update(const outer_time*) {}
    void outer_epilog_update(const outer_time*) {}

    // solver callbacks (overridden by derived constraint types).
    void do_collision(float delta_t) {}
    void inner_update(float delta_t) {}
    void epilog_vel_constraint(float delta_t) {}
};
static_assert(sizeof(rigid_body_constraint) == 0x0C, "rigid_body_constraint size mismatch");

// ============================================================================
// rigid_body_constraint_point — point (ball) constraint (80 bytes)
// Size: 0x50 (80 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_point : rigid_body_constraint {
    math::Dir3      m_b1_r_loc;       // +0x10
    math::Dir3      m_b2_r_loc;       // +0x20
    pulse_sum_cache m_ps_cache_list[3]; // +0x30
    float           m_stress;         // +0x48

    rigid_body_constraint_point() {  // ea: 0x87E910
        m_ps_cache_list[0].m_visit_key = -1;
        m_ps_cache_list[1].m_visit_key = -1;
        m_ps_cache_list[2].m_visit_key = -1;
        m_stress = 0.0f;
    }
    void set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc);
    void epilog_vel_constraint(float delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_point) == 0x50, "rigid_body_constraint_point size mismatch");
static_assert(offsetof(rigid_body_constraint_point, m_b1_r_loc) == 0x10, "point::m_b1_r_loc offset mismatch");

// ============================================================================
// rigid_body_constraint_distance — distance constraint (96 bytes)
// Size: 0x60 (96 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_distance : rigid_body_constraint {
    math::Dir3      m_b1_r_loc;       // +0x10
    math::Dir3      m_b2_r_loc;       // +0x20
    float           m_min_distance;   // +0x30
    float           m_max_distance;   // +0x34
    float           m_next_max_distance;  // +0x38
    float           m_max_distance_vel;   // +0x3C
    float           m_damp_coef;      // +0x40
    unsigned int    m_flags;          // +0x44
    pulse_sum_cache m_ps_cache_list[3]; // +0x48

    rigid_body_constraint_distance() {  // ea: 0x881270
        m_ps_cache_list[0].m_visit_key = -1;
        m_ps_cache_list[1].m_visit_key = -1;
        m_ps_cache_list[2].m_visit_key = -1;
    }
    void set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc,
             float min_distance, float max_distance);
    void outer_prolog_update(const outer_time* outside_delta_t);
    void inner_update(float delta_t);
    void outer_epilog_update(const outer_time* outside_delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_distance) == 0x60, "rigid_body_constraint_distance size mismatch");

// ============================================================================
// ragdoll_joint_limit_info — ragdoll cone joint limit (32 bytes)
// Size: 0x20 (32 bytes) — verified against IDA
// ============================================================================
struct ragdoll_joint_limit_info {
    math::Dir3 m_b1_ud_loc;             // +0x00
    float      m_b1_ud_limit_co_;       // +0x10
    float      m_b1_ud_limit_si_;       // +0x14
    float      m_b1_ud_active_limit_co_;// +0x18
    uint8_t    _pad1C[4];               // +0x1C

    ragdoll_joint_limit_info() {}  // ea: 0x87E940
    void set(const math::Dir3& b1_ud_loc, float theta_limit);
    void set_b1_ud_loc(const math::Dir3& b1_ud_loc);
    void set_theta_limit(float theta_limit);
};
static_assert(sizeof(ragdoll_joint_limit_info) == 0x20, "ragdoll_joint_limit_info size mismatch");

// ============================================================================
// rigid_body_constraint_ragdoll — ragdoll constraint (336 bytes)
// Size: 0x150 (336 bytes) — verified against IDA
// ============================================================================
// class tag (V) required: binary manglings use PAV/PBV (e.g.
// ?create_rbc_ragdoll@phys_sys@@SAPAVrigid_body_constraint_ragdoll@@...)
class rigid_body_constraint_ragdoll : public rigid_body_constraint {
public:
    math::Dir3      m_b1_r_loc;       // +0x10
    math::Dir3      m_b2_r_loc;       // +0x20
    unsigned int    m_flags;          // +0x30
    pulse_sum_cache m_ps_cache_list[10]; // +0x34
    uint8_t         _pad84[12];       // +0x84 (align to 16)
    math::Dir3      m_b1_axis_loc;    // +0x90
    math::Dir3      m_b2_axis_loc;    // +0xA0
    math::Dir3      m_b1_a1_loc;      // +0xB0
    math::Dir3      m_b1_a2_loc;      // +0xC0
    math::Dir3      m_b1_ref_loc;     // +0xD0
    math::Dir3      m_b2_ref_min_loc; // +0xE0
    math::Dir3      m_b2_ref_max_loc; // +0xF0
    ragdoll_joint_limit_info m_joint_limits[2];  // +0x100
    int             m_joint_limits_count;  // +0x140
    float           m_damp_k;         // +0x144
    uint8_t         _pad148[8];       // +0x148

    rigid_body_constraint_ragdoll() {  // ea: 0x881310
        for (int i = 0; i < 10; ++i)
            m_ps_cache_list[i].m_visit_key = -1;
        m_flags = 0;
        m_joint_limits_count = 0;
    }
    void set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc);
    void set_damp_k(float damp_k);
    void set_snider_style(const math::Dir3& b1_axis_loc, const math::Dir3& b1_ref_loc);
    void set_theta_min_max(const math::Dir3& b2_ref_loc, float theta_min, float theta_max);
    void set_hinge(const math::Dir3& b1_axis_loc, const math::Dir3& b2_axis_loc,
                   const math::Dir3& b1_ref_loc, const math::Dir3& b2_ref_loc,
                   float theta_min, float theta_max);
    void set_swivel(const math::Dir3& b1_axis_loc, const math::Dir3& b2_axis_loc,
                    const math::Dir3& b1_ref_loc, const math::Dir3& b2_ref_loc,
                    float theta_min, float theta_max);
    void add_joint_limit(const math::Dir3& b1_ud_loc, float theta_limit);
    const float& pull_together();
    void do_collision(float delta_t);
    void setup_hinge(pulse_sum_constraint_solver* psys, const math::Dir3& b1_ref,
                     const math::Dir3& b2_axis, float delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
    void set_joint_limit_active(unsigned int f, bool b);
    // get_joint_limit_active - ea: 0x6E5060 (inline COMDAT)
    const unsigned int get_joint_limit_active(unsigned int f) const
    {
        return (m_flags >> (6 + f)) & 1;
    }
};
static_assert(sizeof(rigid_body_constraint_ragdoll) == 0x150, "rigid_body_constraint_ragdoll size mismatch");
static_assert(offsetof(rigid_body_constraint_ragdoll, m_b1_axis_loc) == 0x90, "ragdoll::m_b1_axis_loc offset mismatch");

// ============================================================================
// rigid_body_constraint_hinge — hinge constraint (240 bytes)
// Size: 0xF0 (240 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_hinge : rigid_body_constraint {
    math::Dir3      m_b1_r_loc;       // +0x10
    math::Dir3      m_b2_r_loc;       // +0x20
    math::Dir3      m_b1_axis_loc;    // +0x30
    math::Dir3      m_b2_axis_loc;    // +0x40
    math::Dir3      m_b1_a1_loc;      // +0x50
    math::Dir3      m_b1_a2_loc;      // +0x60
    math::Dir3      m_b1_ref_loc;     // +0x70
    math::Dir3      m_b2_ref_min_loc; // +0x80
    math::Dir3      m_b2_ref_max_loc; // +0x90
    float           m_damp_k;         // +0xA0
    unsigned int    m_flags;          // +0xA4
    pulse_sum_cache m_ps_cache[8];    // +0xA8

    rigid_body_constraint_hinge() {  // ea: 0x881210
        for (int i = 0; i < 8; ++i)
            m_ps_cache[i].m_visit_key = -1;
    }
    void set(const math::Dir3& b1_r_loc, const math::Dir3& b2_r_loc,
             const math::Dir3& b1_axis_loc, const math::Dir3& b2_axis_loc,
             const math::Dir3& b1_ref_loc, const math::Dir3& b2_ref_loc,
             float theta_min, float theta_max, float damp_k);
    void do_collision(float delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_hinge) == 0xF0, "rigid_body_constraint_hinge size mismatch");
static_assert(offsetof(rigid_body_constraint_hinge, m_b1_axis_loc) == 0x30, "hinge::m_b1_axis_loc offset mismatch");

// ============================================================================
// rigid_body_constraint_angular_actuator — angular actuator (208 bytes)
// Size: 0xD0 (208 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_angular_actuator : rigid_body_constraint {
    math::Mat43     m_target_mat;         // +0x10
    math::Mat43     m_next_target_mat;    // +0x50
    math::Dir3      m_a_vel;              // +0x90
    float           m_power;              // +0xA0
    float           m_power_scale;        // +0xA4
    bool            m_enabled;            // +0xA8
    uint8_t         _padA9[3];            // +0xA9
    pulse_sum_cache m_ps_cache_list[3];   // +0xAC
    uint8_t         _padC4[12];           // +0xC4

    rigid_body_constraint_angular_actuator() {  // ea: 0x8812E0
        m_ps_cache_list[0].m_visit_key = -1;
        m_ps_cache_list[1].m_visit_key = -1;
        m_ps_cache_list[2].m_visit_key = -1;
    }
    void set(float power, const math::Mat43& target_mat);
    void outer_prolog_update(const outer_time* outside_delta_t);
    void inner_update(float delta_t);
    void outer_epilog_update(const outer_time* outside_delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_angular_actuator) == 0xD0, "rigid_body_constraint_angular_actuator size mismatch");
static_assert(offsetof(rigid_body_constraint_angular_actuator, m_target_mat) == 0x10, "angular_actuator::m_target_mat offset mismatch");

// ============================================================================
// rigid_body_constraint_wheel — vehicle wheel constraint (224 bytes)
// Size: 0xE0 (224 bytes) — verified against IDA
// ============================================================================
// class tag (V) required for PAV manglings in vehicle_collision.cpp symbols
class rigid_body_constraint_wheel : public rigid_body_constraint {
public:
    math::Dir3      m_b2_hitp_loc;             // +0x10
    math::Dir3      m_b2_hitn_loc;             // +0x20
    math::Dir3      m_b1_wheel_center_loc;     // +0x30
    math::Dir3      m_b1_suspension_dir_loc;   // +0x40
    math::Dir3      m_b1_wheel_axis_loc;       // +0x50
    float           m_wheel_radius;            // +0x60
    float           m_fwd_fric_k;              // +0x64
    float           m_side_fric_k;             // +0x68
    float           m_suspension_stiffness_k;  // +0x6C
    float           m_suspension_damp_k;       // +0x70
    float           m_hard_limit_dist;         // +0x74
    float           m_roll_stability_factor;   // +0x78
    float           m_turning_radius_ratio_max_speed; // +0x7C
    float           m_turning_radius_ratio_accel;     // +0x80
    float           m_desired_speed_k;         // +0x84
    float           m_acceleration_factor_k;   // +0x88
    float           m_braking_factor_k;        // +0x8C
    float           m_wheel_vel;               // +0x90
    float           m_wheel_fwd;               // +0x94
    float           m_wheel_pos;               // +0x98
    float           m_wheel_displaced_center_dist;  // +0x9C
    float           m_wheel_normal_force;      // +0xA0
    unsigned int    m_wheel_state;             // +0xA4
    unsigned int    m_wheel_flags;             // +0xA8
    pulse_sum_cache m_ps_cache_list[4];        // +0xAC
    struct pulse_sum_normal* m_ps_suspension;  // +0xCC
    struct pulse_sum_normal* m_ps_side_fric;   // +0xD0
    struct pulse_sum_normal* m_ps_fwd_fric;    // +0xD4

    rigid_body_constraint_wheel() {  // ea: 0x8812B0
        m_ps_cache_list[0].m_visit_key = -1;
        m_ps_cache_list[1].m_visit_key = -1;
        m_ps_cache_list[2].m_visit_key = -1;
        m_ps_cache_list[3].m_visit_key = -1;
    }
    void set_wheel_state_accelerating(float desired_speed_k, float acceleration_factor_k);
    void set_wheel_state_braking(float braking_factor_k);
    void set_no_collision();
    void set_collision(rigid_body* rb, const math::Dir3* hitp_loc, const math::Dir3* hitn_loc);
    void get_wheel_collide_segment(const math::Mat43& b1_mat, math::Dir3* const p0,
                                   math::Dir3* const p1) const;  // 0x884FE0
    void set(const math::Dir3* wheel_center_loc, const math::Dir3* suspension_dir_loc,
             const math::Dir3* wheel_axis_loc, float wheel_radius, float fwd_fric_k,
             float side_fric_k, float suspension_stiffness_k, float suspension_damp_k,
             float hard_limit_dist, float roll_stability_factor);
    void get_wheel_collide_segment(const math::Mat43* b1_mat, math::Dir3* p0, math::Dir3* p1);
    void do_collision(float delta_t);
    void epilog_vel_constraint(float delta_t);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_wheel) == 0xE0, "rigid_body_constraint_wheel size mismatch");
static_assert(offsetof(rigid_body_constraint_wheel, m_b2_hitp_loc) == 0x10, "wheel::m_b2_hitp_loc offset mismatch");

// ============================================================================
// rigid_body_constraint_custom — custom constraint base (16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_custom : rigid_body_constraint {
    struct custom_constraint;                 // fwd

    struct custom_constraint {
        struct custom_constraint_vtbl* __vftable;  // +0x00
    };

    custom_constraint* m_cc;                  // +0x0C
};
static_assert(sizeof(rigid_body_constraint_custom) == 0x10, "rigid_body_constraint_custom size mismatch");
static_assert(sizeof(rigid_body_constraint_custom::custom_constraint) == 4, "custom_constraint size mismatch");

// ============================================================================
// rigid_body_constraint_custom_orientation — orientation constraint (56 bytes)
// Size: 0x38 (56 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_custom_orientation : rigid_body_constraint {
    pulse_sum_cache m_ps_cache_list[4];    // +0x0C
    bool            m_active;              // +0x2C
    bool            m_no_orientation_correction;  // +0x2D
    uint8_t         _pad2E[2];             // +0x2E
    float           m_torque_resistance;   // +0x30
    float           m_upright_strength;    // +0x34

    rigid_body_constraint_custom_orientation() {  // ea: 0x87E950
        m_ps_cache_list[0].m_visit_key = -1;
        m_ps_cache_list[1].m_visit_key = -1;
        m_ps_cache_list[2].m_visit_key = -1;
        m_ps_cache_list[3].m_visit_key = -1;
        m_active = false;
        m_no_orientation_correction = false;
        m_torque_resistance = 100.0f;
        m_upright_strength = 100.0f;
    }
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_custom_orientation) == 0x38, "rigid_body_constraint_custom_orientation size mismatch");
static_assert(offsetof(rigid_body_constraint_custom_orientation, m_active) == 0x2C, "custom_orientation::m_active offset mismatch");

// ============================================================================
// user_rigid_body — external body reference (448 bytes)
// Size: 0x1C0 (448 bytes) — verified against IDA
// ============================================================================
struct user_rigid_body : rigid_body {
    const math::Mat43* m_dictator;  // +0x1B0

    void set(const math::Mat43* const dictator);  // ?set@user_rigid_body@@QAEXQBVMat43@math@@@Z
    user_rigid_body() {}  // ea: 0x881720
};
static_assert(sizeof(user_rigid_body) == 0x1C0, "user_rigid_body size mismatch");
static_assert(offsetof(user_rigid_body, m_dictator) == 0x1B0, "user_rigid_body::m_dictator offset mismatch");

// ============================================================================
// environment_rigid_body — static world body (432 bytes)
// Size: 0x1B0 (432 bytes) — verified against IDA
// ============================================================================
struct environment_rigid_body : rigid_body {
    void set();
};
static_assert(sizeof(environment_rigid_body) == 0x1B0, "environment_rigid_body size mismatch");

// ============================================================================
// rigid_body_constraint_custom_path — custom path constraint (128 bytes)
// Size: 0x80 (128 bytes) — verified against IDA
// ============================================================================
class rigid_body_constraint_custom_path : public rigid_body_constraint {
public:
    math::Mat43     m_path_mat;        // +0x10
    math::Dir3      b1_r_loc;          // +0x50
    user_rigid_body* m_urb;            // +0x60
    pulse_sum_cache m_list_psc[3];     // +0x64
    uint8_t         _pad7C[4];         // +0x7C

    rigid_body_constraint_custom_path() {  // ea: 0x881290
        m_list_psc[0].m_visit_key = -1;
        m_list_psc[1].m_visit_key = -1;
        m_list_psc[2].m_visit_key = -1;
    }
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_custom_path) == 0x80, "rigid_body_constraint_custom_path size mismatch");

// ============================================================================
// contact_point_info — solver contact point (80 bytes)
// Size: 0x50 (80 bytes) — verified against IDA
// ============================================================================
class contact_point_info {
public:
    struct pulse_sum_cache_info {
        pulse_sum_cache m_ps_cache_list[2];   // +0x00
    };
    static_assert(sizeof(pulse_sum_cache_info) == 0x10, "pulse_sum_cache_info size mismatch");

    math::Dir3  m_normal;                 // +0x00
    math::Dir3  m_contact_rigid_body_vel; // +0x10
    float       m_fric_coef;              // +0x20
    float       m_bounce_coef;            // +0x24
    float       m_max_restitution_vel;    // +0x28
    int         m_flags;                  // +0x2C
    int         m_point_pair_count;       // +0x30
    math::Dir3* m_list_b1_r_loc;          // +0x34
    math::Dir3* m_list_b2_r_loc;          // +0x38
    pulse_sum_cache_info* m_list_pulse_sum_cache_info;  // +0x3C
    contact_point_info*   m_next_link;    // +0x40
    uint8_t     _pad44[12];               // +0x44

    static phys_memory_heap* get_cpi_allocater();

    // set - ea: 0x6F1180 (physics.o inline COMDAT)
    void set(float fric_coef, float bounce_coef, float max_restitution_vel,
             bool no_overflow_error);
    // create_cpi - ea: 0x718F20 (physics.o inline COMDAT)
    static contact_point_info* create_cpi(int point_pair_count, bool no_error,
                                          phys_memory_heap* allocater);
    // get_closest_psc - ea: 0x718D90 (physics.o inline COMDAT)
    void get_closest_psc(const math::Dir3& normal, const math::Dir3& b1_r_loc,
                         const math::Dir3& b2_r_loc, float* closest_error,
                         const pulse_sum_cache_info** closest_psc) const;
    // set_closest_cached_psc - ea: 0x71BA70 (physics.o inline COMDAT)
    static void set_closest_cached_psc(const contact_point_info* cached_cpi,
                                       const math::Dir3& normal,
                                       const math::Dir3& b1_r_loc,
                                       const math::Dir3& b2_r_loc,
                                       pulse_sum_cache_info* psc);
    void set_closest_cached_psc(const contact_point_info* cached_cpi);
};
static_assert(sizeof(contact_point_info) == 0x50, "contact_point_info size mismatch");
static_assert(offsetof(contact_point_info, m_normal) == 0x00, "contact_point_info::m_normal offset mismatch");

// ============================================================================
// rigid_body_constraint_contact — collision contact constraint (44 bytes)
// Size: 0x2C (44 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_contact : rigid_body_constraint {
    struct avl_tree_node {
        rigid_body_constraint_contact* m_left;   // +0x00
        rigid_body_constraint_contact* m_right;  // +0x04
        int                            m_balance; // +0x08
    };
    static_assert(sizeof(avl_tree_node) == 0x0C, "avl_tree_node size mismatch");

    phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_1;  // +0x0C
    phys_simple_link_list<contact_point_info> m_list_contact_point_info_buffer_2;  // +0x10
    unsigned int        m_solver_priority;   // +0x14
    avl_tree_node       m_avl_tree_node;     // +0x18
    rigid_body_pair_key m_avl_key;           // +0x24

    // get_solver_priority - ea: 0x88B120
    unsigned int get_solver_priority() const { return m_solver_priority; }
    // get_cached_point_count / get_point_count - ea: 0x881DE0 / 0x881E00
    int get_cached_point_count() const {
        int n = 0;
        for (const contact_point_info* c = m_list_contact_point_info_buffer_1.m_first;
             c != NULL; c = c->m_next_link)
            n += c->m_point_pair_count;
        return n;
    }
    int get_point_count() const {
        int n = 0;
        for (const contact_point_info* c = m_list_contact_point_info_buffer_2.m_first;
             c != NULL; c = c->m_next_link)
            n += c->m_point_pair_count;
        return n;
    }
    // epilog_cache - ea: 0x88E8E0
    void epilog_cache() {}

    void verify_constraint(rigid_body* b1_, rigid_body* b2_);
    void add_cpi_simple(contact_point_info* cpi, rigid_body* const b1_,
                        rigid_body* const b2_);
    const contact_point_info* get_cached_cpi() const;
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
    void add_point_list(rigid_body* b1_, rigid_body* b2_,
                        const math::Dir3* list_b1_r_loc,
                        const math::Dir3* list_b2_r_loc, int num_points,
                        const math::Dir3& normal_, float fric_coef,
                        float bounce_coef, float max_restitution_vel,
                        bool no_overflow_error);  // 0x8816A0
    ~rigid_body_constraint_contact();
};
static_assert(sizeof(rigid_body_constraint_contact) == 0x2C, "rigid_body_constraint_contact size mismatch");
static_assert(offsetof(rigid_body_constraint_contact, m_avl_tree_node) == 0x18, "contact::m_avl_tree_node offset mismatch");

// ============================================================================
// contact_manifold_mesh_point — mesh contact point (32 bytes)
// Size: 0x20 (32 bytes) — verified against IDA
// ============================================================================
struct contact_manifold_mesh_point {
    math::Dir3 m_p;          // +0x00
    math::Dir3 m_contact_p;  // +0x10
};
static_assert(sizeof(contact_manifold_mesh_point) == 0x20, "contact_manifold_mesh_point size mismatch");

class phys_collide_data;
struct phys_gjk_geom;

// ============================================================================
// phys_contact_manifold — collision manifold (64 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
struct phys_contact_manifold {
    math::Dir3  m_feature_hitp;             // +0x00
    math::Dir3  m_feature_hitn;             // +0x10
    float       m_feature_distance_eps;     // +0x20
    float       m_sin_feautre_angular_eps_sq;  // +0x24
    phys_memory_heap* m_allocater;          // +0x28
    contact_manifold_mesh_point* m_list_mesh_point;      // +0x2C
    int         m_list_mesh_point_count;    // +0x30
    contact_manifold_mesh_point** m_list_sorted_mesh_point;  // +0x34
    contact_manifold_mesh_point** m_list_contact_point;      // +0x38
    int         m_list_contact_point_count; // +0x3C

    const float& compute_convex_poly_area();
    const float& compute_convex_poly_perimeter();
    void reset_list_mesh_point();
    void alloc_sorted_list_mesh_point();
    void xform_mesh_points(const math::Mat43* xform);
    void qsort(contact_manifold_mesh_point** i0_mp, contact_manifold_mesh_point** i1_mp);
    void setup_list_sorted_mesh_point();
    void generate_convex_poly_internal();
    const math::Dir3* comp_feature_normal(math::Dir3* result, const math::Mat43* contact_mat);
    void generate_convex_poly(const math::Mat43* contact_mat);

    static bool rht(const math::Dir3* e1, const math::Dir3* e2,
                    float min_length2, float min_sin_sq);
};
static_assert(sizeof(phys_contact_manifold) == 0x40, "phys_contact_manifold size mismatch");
static_assert(offsetof(phys_contact_manifold, m_feature_hitp) == 0x00, "manifold::m_feature_hitp offset mismatch");

// ============================================================================
// phys_contact_manifold_process â€” convex polygon intersection processor
// Size: 0x1140 (4416 bytes) â€” verified against IDA
// ============================================================================
struct phys_contact_manifold_process {
    phys_contact_manifold_process();  // ea: 0x88E1C0
    struct bridge {
        math::Dir3 m_intersection_p;                    // +0x00
        contact_manifold_mesh_point** m_left_i;         // +0x10
        contact_manifold_mesh_point** m_right_i;        // +0x14
        uint8_t    _pad18[8];                           // +0x18
    };
    static_assert(sizeof(bridge) == 0x20, "bridge size mismatch");

    struct isect_info {
        phys_contact_manifold* m_cman;                  // +0x00
        contact_manifold_mesh_point** m_i;              // +0x04
        contact_manifold_mesh_point** m_next_i;         // +0x08
        contact_manifold_mesh_point** m_last_i;         // +0x0C
        math::Dir3 m_edge;                              // +0x10

        void init(phys_contact_manifold* cman);
        void update();
    };
    static_assert(sizeof(isect_info) == 0x20, "isect_info size mismatch");

    math::Mat43   contact_mat;                          // +0x000
    math::Mat43   cg1_to_rb2_xform;                     // +0x200
    phys_memory_heap* m_cpi_allocater;                  // +0x400
    uint8_t       m_list_cpi[8];                        // +0x84 (opaque)
    contact_point_info* m_cpi;                          // +0x8C
    math::Dir3*   m_list_isect_point;                   // +0x90
    uint8_t       _pad094[0xA0 - 0x94];                 // +0x94
    phys_contact_manifold cman1;                        // +0xA0
    phys_contact_manifold cman2;                        // +0xE0
    int           m_contact_point_count;                // +0x120
    phys_memory_heap m_allocater;                       // +0x124
    char          m_allocater_memory[4096];             // +0x134

    bool find_bottom(bridge* b, isect_info* left_cman, isect_info* right_cman);
    void intersect_poly_segment(phys_contact_manifold* cman, const math::Dir3* p0,
                                const math::Dir3* p1);
    void intersect_poly_poly();
    void copy_poly(phys_contact_manifold* cman);
    void comp_contact_mat(const math::Dir3* contact_normal);
    void process(phys_collide_data* d);  // ?process@phys_contact_manifold_process@@QAEXPAVphys_collide_data@@@Z (phys_collision.o)
};
static_assert(offsetof(phys_contact_manifold_process, contact_mat) == 0x000, "process::contact_mat offset mismatch");
static_assert(offsetof(phys_contact_manifold_process, cman1) == 0xA0, "process::cman1 offset mismatch");
static_assert(offsetof(phys_contact_manifold_process, m_allocater) == 0x124, "process::m_allocater offset mismatch");
static_assert(sizeof(phys_contact_manifold_process) == 0x1140, "phys_contact_manifold_process size mismatch");

// ============================================================================
// phys_gjk_collision_info — GJK collision result (48 bytes)
// Size: 0x30 (48 bytes) — verified against IDA
// ============================================================================
struct phys_gjk_collision_info {
    math::Dir3 m_p1;  // +0x00 — contact point on body 1
    math::Dir3 m_p2;  // +0x10 — contact point on body 2
    math::Dir3 m_n;   // +0x20 — contact normal
};
static_assert(sizeof(phys_gjk_collision_info) == 0x30, "phys_gjk_collision_info size mismatch");

// ============================================================================
// phys_gjk_geom_id_pair_key — geometry pair cache key (8 bytes)
// ============================================================================
struct phys_gjk_geom_id_pair_key {
    unsigned int m_id1;  // +0x00
    unsigned int m_id2;  // +0x04
};
static_assert(sizeof(phys_gjk_geom_id_pair_key) == 8, "phys_gjk_geom_id_pair_key size mismatch");

// ============================================================================
// phys_gjk_cache_info — cached GJK support data (128 bytes)
// Size: 0x80 (128 bytes) — verified against IDA
// ============================================================================
struct phys_gjk_cache_info {
    math::Dir3  m_support_dir;      // +0x00
    math::Dir3  m_support_a[3];     // +0x10
    math::Dir3  m_support_b[3];     // +0x40
    int         m_support_count;    // +0x70
    phys_gjk_geom_id_pair_key m_key; // +0x74
    unsigned int m_flags;           // +0x7C

    // update_swapped - ea: 0x6F1950 (physics.o inline COMDAT)
    void update_swapped(bool swapped);
};
static_assert(sizeof(phys_gjk_cache_info) == 0x80, "phys_gjk_cache_info size mismatch");
static_assert(offsetof(phys_gjk_cache_info, m_support_dir) == 0x00, "gjk_cache_info::m_support_dir offset mismatch");

// ============================================================================
// phys_gjk_info — GJK simplex state (816 bytes)
// Size: 0x330 (816 bytes) — verified against IDA
// ============================================================================
struct phys_gjk_info {
    struct phys_gjk_set_info {
        float m_lamda[4];     // +0x00
        int   m_candidate;    // +0x10
    };
    static_assert(sizeof(phys_gjk_set_info) == 0x14, "phys_gjk_set_info size mismatch");

    math::Mat43  cg2_to_cg1_xform;    // +0x000
    phys_gjk_collision_info cg1_cinfo_loc;  // +0x040
    math::Dir3   m_gjk_sep_vec;        // +0x070
    math::Dir3   m_w_verts[4];         // +0x080
    math::Dir3   m_a_verts[4];         // +0x0C0
    math::Dir3   m_b_verts[4];         // +0x100
    math::Dir3   m_b_loc_verts[4];     // +0x140
    math::Dir3   m_support_dir;        // +0x180
    int          m_flags;              // +0x190
    int          m_w_set;              // +0x194
    int          m_last_w_set;         // +0x198
    int          m_gjk_iter;           // +0x19C
    float        m_gjk_sep_thresh;     // +0x1A0
    float        m_upper_dist_sq;      // +0x1A4
    float        m_lower_dist_sq;      // +0x1A8
    float        m_dot_ij[4][4];       // +0x1AC
    phys_gjk_set_info m_set_list[16];  // +0x1EC
    uint8_t      _pad32C[4];           // +0x32C

    enum gjk_retval_e {
        GJK_INVALID = 0,
        GJK_SEPARATED = 1,
        GJK_VALID = 2,
        GJK_PENETRATING = 3,
    };

    int  gjk_subalgorithm(int w_set, int new_index);
    int  seed_simplex(int cached_vert_count);
    gjk_retval_e gjk(phys_collide_data* d, const math::Dir3* initial_support_dir,
                     bool in_separation_loop);
    gjk_retval_e collide(phys_collide_data* d);
    bool phys_collide_do_gjk_collide(phys_collide_data* d, float sep_thresh);

    // helpers (phys_gjk.cpp, inline/non-inline)
    int  init_gjk(phys_collide_data* d, const math::Dir3* initial_support_dir,
                  bool in_separation_loop);
    void comp_lambda_2(int index_0, int index_1);
    void comp_lambda_3(int index_0, int index_1, int index_2);
    void comp_lambda_4();
    bool comp_v(int w_set, math::Dir3* v);
    void comp_closest_points(int w_set, math::Dir3* a, math::Dir3* b);
    const math::Dir3& get_initial_support_dir(const math::Dir3* result,
                                              phys_collide_data* d);
    void gjk_cache_update_separated(phys_collide_data* d);
    void gjk_cache_update_colliding(phys_collide_data* d);
};
static_assert(sizeof(phys_gjk_info) == 0x330, "phys_gjk_info size mismatch");
static_assert(offsetof(phys_gjk_info, cg2_to_cg1_xform) == 0x000, "gjk_info::cg2_to_cg1_xform offset mismatch");
static_assert(offsetof(phys_gjk_info, m_set_list) == 0x1EC, "gjk_info::m_set_list offset mismatch");

// ============================================================================
// phys_gjk_geom â€” GJK geometry interface (4 bytes)
// ============================================================================
struct phys_gjk_geom {
    struct phys_gjk_geom_vtbl* __vftable;  // +0x00

    const math::Dir3* support(const math::Dir3* result, const math::Mat43* xform,
                              const math::Dir3* v) const;
    float get_geom_radius() const;
};
static_assert(sizeof(phys_gjk_geom) == 0x4, "phys_gjk_geom size mismatch");

// ============================================================================
// phys_collide_data â€” GJK collision request (84 bytes)
// ============================================================================
class phys_collide_data {
public:
    const phys_gjk_geom* gjk_cg1;          // +0x00
    const phys_gjk_geom* gjk_cg2;          // +0x04
    const math::Mat43*   cg1_to_world_xform; // +0x08
    const math::Mat43*   cg2_to_world_xform; // +0x0C
    const math::Mat43*   cg1_to_rb1_xform; // +0x10
    const math::Mat43*   rb2_to_world_xform; // +0x14
    rigid_body*          rb1;              // +0x18
    rigid_body*          rb2;              // +0x1C
    unsigned int         id1;              // +0x20
    unsigned int         id2;              // +0x24
    phys_gjk_cache_info* gjk_ci;           // +0x28
    float                fric_coef;        // +0x2C
    float                bounce_coef;      // +0x30
    const math::Mat43*   cg2_to_cg1_xform; // +0x34
    phys_gjk_collision_info* cg1_cinfo_loc; // +0x38
    void*                pcd_callback;     // +0x3C
    phys_gjk_info*       gjk_info;         // +0x40
    phys_contact_manifold_process* cman_process; // +0x44
    bool                 no_overflow_error; // +0x48
    int                  solver_priority;  // +0x4C
    phys_collide_data*   m_next;           // +0x50
};
static_assert(sizeof(phys_collide_data) == 0x54, "phys_collide_data size mismatch");

// ============================================================================
// gjk_sep_dir â€” GJK separation-direction computation
// ============================================================================
namespace gjk_sep_dir {
const math::Dir3& comp_sep_dir(const math::Dir3* result, phys_collide_data* m_pcd,
                               phys_gjk_info* m_gjk_info);
}

// ============================================================================
// phys_vec3 — physics 3D vector (aligned, similar to Dir3)
// Used within GJK/collision subsystem
// ============================================================================
struct phys_vec3 {
    float x, y, z;
    float pad;  // 16-byte alignment
};
static_assert(sizeof(phys_vec3) == 0x10, "phys_vec3 size mismatch");

// ============================================================================
// Phys memory alignment constant
// ============================================================================
#define PHYS_ALIGNOF(type) 16
