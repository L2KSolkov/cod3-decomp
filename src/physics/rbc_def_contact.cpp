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
// pulse_sum_constraint_solver::create_pulse_sum_contact - ea: 0x88AC40
// ============================================================================
pulse_sum_contact* pulse_sum_constraint_solver::create_pulse_sum_contact(
    rigid_body* b1, rigid_body* b2, contact_point_info* cpi, float delta_t) {
    pulse_sum_contact* result = (pulse_sum_contact*)m_solver_memory_allocater.allocate(
        160 * cpi->m_point_pair_count + 64, 16, (cpi->m_flags & 2) != 0,
        SOLVER_MEMORY_ALLOCATER_ERROR_MSG);
    if (result != NULL) {
        pulse_sum_contact* last = m_list_pulse_sum_contact.m_last;
        if (last != NULL)
            last->m_link.m_next_link = result;
        else
            m_list_pulse_sum_contact.m_first = result;
        m_list_pulse_sum_contact.m_last = result;
        result->m_link.m_next_link = NULL;
        result->m_list_cpi = (pulse_sum_contact::psc_cpi*)&result[1];
        result->m_list_cpi_count = cpi->m_point_pair_count;
        result->set(b1, b2, cpi, delta_t);
    }
    return result;
}

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

// rigid_body_constraint_contact::add_cpi_simple - ea: 0x878650
void rigid_body_constraint_contact::add_cpi_simple(
    contact_point_info* cpi, rigid_body* const b1_, rigid_body* const b2_) {
    if (cpi == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                  275, "cpi", defaultFileName))
        __debugbreak();
    if (cpi->m_list_b1_r_loc == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                  276, "cpi->m_list_b1_r_loc", defaultFileName))
        __debugbreak();
    if (cpi->m_list_b2_r_loc == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                  277, "cpi->m_list_b2_r_loc", defaultFileName))
        __debugbreak();
    if (cpi->m_list_pulse_sum_cache_info == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                  278, "cpi->m_list_pulse_sum_cache_info", defaultFileName))
        __debugbreak();
    if (cpi->m_point_pair_count <= 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rbc_defs\\rbc_def_contact.h",
                  279, "cpi->m_point_pair_count > 0", defaultFileName))
        __debugbreak();
    verify_constraint(b1_, b2_);
    cpi->m_next_link = m_list_contact_point_info_buffer_1.m_first;
    m_list_contact_point_info_buffer_1.m_first = cpi;
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
