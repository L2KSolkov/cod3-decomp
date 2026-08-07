// ============================================================================
// COD3 Physics Types — rigid_body, constraint hierarchy, GJK cache, manifolds
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// outer_time — time wrapper used in rigid_body (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct outer_time {
    float m_time;  // +0x00
};
static_assert(sizeof(outer_time) == 4, "outer_time size mismatch");

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
    const float& get_unclamped_pulse_sum() const { return m_pulse_sum; }
    const math::Dir3* get_relative_velocity(const math::Dir3* result);
    const math::Dir3* get_relative_velocity_change_dir(const math::Dir3* result);
    void  set_pulse_sum_limits_parent_ratio(float limit_ratio, pulse_sum_normal* parent);
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
};
static_assert(sizeof(phys_memory_heap) == 0x10, "phys_memory_heap size mismatch");

// ============================================================================
// rb_inplace_partition_node — spatial partition node (64 bytes)
// Exact layout TBD during physics porting.
// ============================================================================
struct rb_inplace_partition_node {
    uint8_t data[64];
};
static_assert(sizeof(rb_inplace_partition_node) == 0x40, "rb_inplace_partition_node size mismatch");

// ============================================================================
// rigid_body — rigid body physics object (432 bytes)
// Size: 0x1B0 (432 bytes) — verified against IDA
// ============================================================================
struct rigid_body {
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
    void set_mass(float mass);
    void set_inertia(const math::Dir3& inertia);
    void set(float mass, const math::Dir3& inertia, const math::Mat43& mat,
             const math::Dir3& t_vel, const math::Dir3& a_vel, float fric_coef,
             int stable_min_contact_count);
    void update_col_mat();
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
};
static_assert(sizeof(ragdoll_joint_limit_info) == 0x20, "ragdoll_joint_limit_info size mismatch");

// ============================================================================
// rigid_body_constraint_ragdoll — ragdoll constraint (336 bytes)
// Size: 0x150 (336 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_ragdoll : rigid_body_constraint {
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
};
static_assert(sizeof(rigid_body_constraint_angular_actuator) == 0xD0, "rigid_body_constraint_angular_actuator size mismatch");
static_assert(offsetof(rigid_body_constraint_angular_actuator, m_target_mat) == 0x10, "angular_actuator::m_target_mat offset mismatch");

// ============================================================================
// rigid_body_constraint_wheel — vehicle wheel constraint (224 bytes)
// Size: 0xE0 (224 bytes) — verified against IDA
// ============================================================================
struct rigid_body_constraint_wheel : rigid_body_constraint {
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

    void set_wheel_state_accelerating(float desired_speed_k, float acceleration_factor_k);
    void set_wheel_state_braking(float braking_factor_k);
    void set_no_collision();
    void set_collision(rigid_body* rb, const math::Dir3* hitp_loc, const math::Dir3* hitn_loc);
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

    void set(const math::Mat43* dictator);
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
struct rigid_body_constraint_custom_path : rigid_body_constraint {
    math::Mat43     m_path_mat;        // +0x10
    math::Dir3      b1_r_loc;          // +0x50
    user_rigid_body* m_urb;            // +0x60
    pulse_sum_cache m_list_psc[3];     // +0x64
    uint8_t         _pad7C[4];         // +0x7C

    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
};
static_assert(sizeof(rigid_body_constraint_custom_path) == 0x80, "rigid_body_constraint_custom_path size mismatch");

// ============================================================================
// contact_point_info — solver contact point (80 bytes)
// Size: 0x50 (80 bytes) — verified against IDA
// ============================================================================
struct contact_point_info {
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

    void verify_constraint(rigid_body* b1_, rigid_body* b2_);
    void setup_constraint(pulse_sum_constraint_solver* psys, float delta_t);
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
};
static_assert(sizeof(phys_contact_manifold) == 0x40, "phys_contact_manifold size mismatch");
static_assert(offsetof(phys_contact_manifold, m_feature_hitp) == 0x00, "manifold::m_feature_hitp offset mismatch");

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
};
static_assert(sizeof(phys_gjk_info) == 0x330, "phys_gjk_info size mismatch");
static_assert(offsetof(phys_gjk_info, cg2_to_cg1_xform) == 0x000, "gjk_info::cg2_to_cg1_xform offset mismatch");
static_assert(offsetof(phys_gjk_info, m_set_list) == 0x1EC, "gjk_info::m_set_list offset mismatch");

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
