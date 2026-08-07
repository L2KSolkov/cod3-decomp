// ============================================================================
// pulse_sum.h — pulse-sum constraint solver types.
// Source: c:\cod\code\tl\physics\include\pulse_sum.h
// Verified against IDA local types.
// ============================================================================
#ifndef COD3_PHYSICS_PULSE_SUM_H
#define COD3_PHYSICS_PULSE_SUM_H

#include "physics/phys_types.h"
#include <intrin.h>

// ============================================================================
// pulse_sum_angular — angular constraint row (144 bytes, verified against IDA)
// ============================================================================
struct pulse_sum_angular {
    phys_link_list_base<pulse_sum_angular> m_link;  // +0x00
    uint8_t        _pad4[12];                       // +0x04
    math::Dir3     m_ud;                            // +0x10
    math::Dir3     m_b1_r;                          // +0x20
    math::Dir3     m_b2_r;                          // +0x30
    math::Dir3     m_b1_ap;                         // +0x40
    math::Dir3     m_b2_ap;                         // +0x50
    float          m_pulse_sum_min;                 // +0x60
    float          m_pulse_sum_max;                 // +0x64
    float          m_pulse_sum;                     // +0x68
    float          m_right_side;                    // +0x6C
    float          m_big_dirt;                      // +0x70
    float          m_cfm;                           // +0x74
    float          m_denom;                         // +0x78
    unsigned int   m_flags;                         // +0x7C
    pulse_sum_node* m_b1;                           // +0x80
    pulse_sum_node* m_b2;                           // +0x84
    pulse_sum_cache* m_pulse_sum_cache;             // +0x88

    float get_pos() const { return m_pulse_sum; }
};
static_assert(sizeof(pulse_sum_angular) == 0x90, "pulse_sum_angular size mismatch");

// ============================================================================
// pulse_sum_wheel â€” wheel constraint pulse-sum bundle (used by
// rigid_body_constraint_wheel). Layout verified against IDA local type.
// ============================================================================
struct pulse_sum_wheel {
    phys_link_list_base<pulse_sum_wheel> m_link;  // +0x00
    uint8_t          _pad4[12];                   // +0x04 (align to 16)
    pulse_sum_normal m_suspension;                // +0x10 (160 bytes)
    pulse_sum_normal* m_side;                     // +0xB0
    pulse_sum_normal* m_fwd;                      // +0xB4

    void set_side_fwd_ratios(float side_ratio, float fwd_ratio);
};
static_assert(sizeof(pulse_sum_wheel) == 0xC0, "pulse_sum_wheel size mismatch");
static_assert(offsetof(pulse_sum_wheel, m_suspension) == 0x10, "pulse_sum_wheel::m_suspension offset mismatch");

// ============================================================================
// pulse_sum_contact â€” contact constraint row (implemented in
// phys_constraint_solver_multithreaded.o)
// ============================================================================
struct pulse_sum_contact;

// ============================================================================
// pulse_sum_constraint_solver — the solver (112 bytes; methods in
// phys_constraint_solver_multithreaded.o, unresolved here).
// ============================================================================
class pulse_sum_constraint_solver {
public:
    struct solver_info {
        uint8_t data[28];  // opaque
    };

    int                m_psys_psc_visit_counter;      // +0x00
    int                m_psys_next_psc_visit_counter; // +0x04
    int                m_psys_max_vel_iters;          // +0x08
    int                m_psys_max_vel_pos_iters;      // +0x0C
    rigid_body*        m_first_partition_head;        // +0x10
    solver_info        m_si;                          // +0x14
    phys_memory_heap   m_solver_memory_allocater;     // +0x30
    uint8_t            _pad40[0x30];                  // +0x40 (pulse sum lists)

    pulse_sum_angular* create_pulse_sum_angular(rigid_body* b1, const math::Dir3* b1_r,
                                                rigid_body* b2, const math::Dir3* b2_r,
                                                const math::Dir3* ud,
                                                pulse_sum_cache* ps_cache);
    pulse_sum_normal*  create_pulse_sum_normal();
    pulse_sum_wheel*   create_pulse_sum_wheel();
    pulse_sum_normal*  create_pulse_sum_wheel_side(pulse_sum_wheel* psw);
    pulse_sum_normal*  create_pulse_sum_wheel_fwd(pulse_sum_wheel* psw);
    pulse_sum_contact* create_pulse_sum_contact(rigid_body* b1, rigid_body* b2,
                                                contact_point_info* cpi, float delta_t);
};
static_assert(sizeof(pulse_sum_constraint_solver) == 0x70, "pulse_sum_constraint_solver size mismatch");

// ============================================================================
// phys_inplace_avl_tree â€” AVL tree used by contact constraints (4 bytes)
// ============================================================================
template <typename Key, typename T>
struct phys_inplace_avl_tree {
    T* m_tree_root;  // +0x00

    void remove(const Key* key);
};

struct physics_system {
    int      m_flags;                                     // +0x00
    uint8_t  _pad4[0x20 - 0x04];                          // +0x04
    float    m_outside_sub_delta_t;                       // +0x20
    uint8_t  _pad24[0xF20 - 0x24];                        // +0x24
    phys_inplace_avl_tree<rigid_body_pair_key, rigid_body_constraint_contact>
        m_search_tree_rbc_contact;                        // +0xF20
    uint8_t _padF44[0x1120 - 0xF44];                      // +0xF44
    phys_memory_heap m_contact_point_buffer_1;            // +0x1120
    phys_memory_heap m_contact_point_buffer_2;            // +0x11A0
};

extern physics_system* g_physics_system;  // ?g_physics_system@@3PAVphysics_system@@A
extern void verify_is_in_physics_system(rigid_body_constraint_contact* rbc,
                                        rigid_body* b1_, rigid_body* b2_);
extern void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m);
extern void SetIdentity(math::Mat43& m);

namespace rbint {
void calc_col_mat(rigid_body* rb, const outer_time* outside_delta_t);
}

// ============================================================================
// rbint — rigid-body intrinsic math (methods in phys_util.o, unresolved)
// ============================================================================
namespace rbint {
const math::Dir3* multiply(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* collide_multiply(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* add_pos(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* collide_add_pos(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
}

#endif // COD3_PHYSICS_PULSE_SUM_H
