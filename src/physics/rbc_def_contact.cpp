// ============================================================================
// rbc_def_contact.cpp â€” contact constraint (4 non-inline funcs).
// Source: source/rbc_def_contact.cpp (phys_xboxr)
// Verified against IDA (phys_xboxr:rbc_def_contact.o):
//   contact_point_info::get_cpi_allocater @0x88AA20
//   verify_constraint                     @0x88AA30
//   setup_constraint                      @0x88AA90
//   ~rigid_body_constraint_contact        @0x88AAD0
// ============================================================================

#include "pulse_sum.h"

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// contact_point_info::get_cpi_allocater â€” ea: 0x88AA20
// ============================================================================
phys_memory_heap* contact_point_info::get_cpi_allocater() {
    return &g_physics_system->m_contact_point_buffer_1;
}

// ============================================================================
// rigid_body_constraint_contact::verify_constraint â€” ea: 0x88AA30
// ============================================================================
void rigid_body_constraint_contact::verify_constraint(rigid_body* b1_, rigid_body* b2_) {
    if ((this->b1 != b1_ || this->b2 != b2_)
        && (this->b1 != b2_ || this->b2 != b1_)
        && _tlAssert("source/rbc_def_contact.cpp", 14,
                     "(b1 == b1_ && b2 == b2_) || (b1 == b2_ && b2 == b1_)", "")) {
        __debugbreak();
    }
    verify_is_in_physics_system(this, b1_, b2_);
}

// ============================================================================
// rigid_body_constraint_contact::setup_constraint â€” ea: 0x88AA90
// ============================================================================
void rigid_body_constraint_contact::setup_constraint(pulse_sum_constraint_solver* psys,
                                                     float delta_t) {
    contact_point_info* m_first = this->m_list_contact_point_info_buffer_1.m_first;
    for (this->m_list_contact_point_info_buffer_2.m_first = NULL;
         m_first != NULL;
         m_first = m_first->m_next_link) {
        psys->create_pulse_sum_contact(this->b1, this->b2, m_first, delta_t);
    }
}

// ============================================================================
// rigid_body_constraint_contact::~rigid_body_constraint_contact â€” ea: 0x88AAD0
// ============================================================================
rigid_body_constraint_contact::~rigid_body_constraint_contact() {
    g_physics_system->m_search_tree_rbc_contact.remove(&this->m_avl_key);
}
