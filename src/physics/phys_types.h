// ============================================================================
// COD3 Physics Types — rigid_body, constraint definitions, GJK types
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "core/math_types.h"
#include <stdint.h>

// ============================================================================
// outer_time — time wrapper used in rigid_body (4 bytes)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct outer_time {
    float m_time;  // +0x00
};
static_assert(sizeof(outer_time) == 4, "outer_time size mismatch");

// Forward declarations for physics types
struct pulse_sum_node;
struct rb_inplace_partition_node;

// ============================================================================
// rigid_body — rigid body physics object (432 bytes)
// Size: 0x1B0 (432 bytes) — verified against IDA
// ============================================================================
struct rigid_body {
    math::Mat43  m_mat;                    // +0x000 — world transform
    math::Mat43  m_col_mat;               // +0x040 — collision transform
    math::Mat33  m_world_inv_inertia;     // +0x080 — world-space inverse inertia
    math::Dir3   m_inv_inertia;           // +0x0B0 — local inverse inertia
    math::Dir3   m_gravity_dir;           // +0x0C0 — gravity direction
    math::Dir3   m_t_vel;                 // +0x0D0 — translational velocity
    math::Dir3   m_a_vel;                 // +0x0E0 — angular velocity
    math::Dir3   m_last_t_vel;            // +0x0F0 — previous frame t-vel
    math::Dir3   m_last_a_vel;            // +0x100 — previous frame a-vel
    math::Dir3   m_force_sum;             // +0x110 — accumulated force
    math::Dir3   m_torque_sum;            // +0x120 — accumulated torque
    float        m_inv_mass;              // +0x130
    float        m_gravity_multiplier;    // +0x134
    float        m_max_avel;              // +0x138
    outer_time   m_time_scale;            // +0x13C
    float        m_max_delta_t;           // +0x140
    unsigned int m_flags;                 // +0x144
    float        m_fric_coef;             // +0x148
    unsigned int m_tick;                  // +0x14C
    pulse_sum_node* m_node;              // +0x150
    int32_t      m_constraint_count;      // +0x154
    int32_t      m_contact_count;         // +0x158
    int32_t      m_stable_min_contact_count; // +0x15C
    float        m_stable_energy_time;    // +0x160
    float        m_stable_te;             // +0x164
    // rb_inplace_partition_node — 64 bytes at +0x168
    uint8_t      m_partition_node[64];    // +0x168
    // padding to end
    uint8_t      _pad_1A8[8];             // +0x1A8
};
static_assert(sizeof(rigid_body) == 0x1B0, "rigid_body size mismatch");
static_assert(offsetof(rigid_body, m_mat) == 0x000, "rigid_body::m_mat offset mismatch");
static_assert(offsetof(rigid_body, m_inv_mass) == 0x130, "rigid_body::m_inv_mass offset mismatch");
static_assert(offsetof(rigid_body, m_flags) == 0x144, "rigid_body::m_flags offset mismatch");

// ============================================================================
// rb_inplace_partition_node — spatial partition node (64 bytes)
// ============================================================================
struct rb_inplace_partition_node {
    uint8_t data[64];  // placeholder — exact layout TBD during physics porting
};
static_assert(sizeof(rb_inplace_partition_node) == 0x40, "rb_inplace_partition_node size mismatch");

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
