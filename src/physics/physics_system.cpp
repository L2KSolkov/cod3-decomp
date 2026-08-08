// ============================================================================
// physics_system.cpp - physics engine pool/API layer (87 non-inline funcs).
// Source: source/physics_system.cpp (phys_xboxr:physics_system.o)
// Verified against IDA (phys_xboxr:physics_system.o).
// ============================================================================

#include "physics_system.h"
#include "pulse_sum.h"

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlFatal(const char* Format, ...);

// physics_system_internal.o (create_inst/destroy_inst/frame_advance stubs until
// physics_system_internal.o is ported).
extern void physics_system_create_inst(phys_mem_info* pmi);
extern void physics_system_destroy_inst();
extern void physics_system_frame_advance(physics_system* psys, float delta_t);

// ============================================================================
// Data
// ============================================================================
phys_proftimer_callbacks g_phys_proftimer_callbacks = { { NULL, NULL } };

namespace phys_sys {

// ============================================================================
// phys_mem_info ctor - ea: 0x87EB10
// ============================================================================
// ============================================================================
// Simple accessors
// ============================================================================
environment_rigid_body* get_environment_rigid_body() {
    return &g_physics_system->m_environment_rigid_body;
}

void set_max_delta_t(float max_delta_t) {
    g_physics_system->m_max_delta_t = max_delta_t;
}

float get_max_delta_t() {
    return g_physics_system->m_max_delta_t;
}

void set_v_tol(int max_v_iters, int max_v_siters) {
    g_physics_system->m_max_vel_iters = max_v_iters;
    g_physics_system->m_max_vel_siters = max_v_siters;
}

void get_v_tol(int* max_v_iters, int* max_v_siters) {
    *max_v_iters = g_physics_system->m_max_vel_iters;
    *max_v_siters = g_physics_system->m_max_vel_siters;
}

void set_vp_tol(int max_vp_iters, int max_vp_siters) {
    g_physics_system->m_max_vel_pos_iters = max_vp_iters;
    g_physics_system->m_max_vel_pos_siters = max_vp_siters;
}

void get_vp_tol(int* max_vp_iters, int* max_vp_siters) {
    *max_vp_iters = g_physics_system->m_max_vel_pos_iters;
    *max_vp_siters = g_physics_system->m_max_vel_pos_siters;
}

void set_collision_callback(void (*collision_callback)()) {
    g_physics_system->m_collision_callback = collision_callback;
}

void phys_frame_advance(float delta_t) {
    physics_system_frame_advance(g_physics_system, delta_t);
}

void phys_init(phys_mem_info* pmi) {
    physics_system_create_inst(pmi);
}

void phys_shutdown() {
    physics_system_destroy_inst();
}

void solver_memory_buffer_set(void* buffer, int buffer_size) {
    g_physics_system->solver_memory_buffer_set(buffer, buffer_size);
}

void solver_memory_buffer_nullify() {
    g_physics_system->solver_memory_buffer_nullify();
}

void set_phys_proftimer_callbacks(const phys_proftimer_callbacks* ppc) {
    g_phys_proftimer_callbacks = *ppc;
}

rigid_body_constraint_contact* get_rbc_contact(rigid_body* const b1, rigid_body* const b2) {
    if (g_physics_system->is_member(b1) == 0 &&
        _tlAssert("source/physics_system.cpp", 124, "PSYS()->is_member(b1)", ""))
        __debugbreak();
    if (g_physics_system->is_member(b2) == 0 &&
        _tlAssert("source/physics_system.cpp", 125, "PSYS()->is_member(b2)", ""))
        __debugbreak();
    rigid_body_pair_key key(b1, b2);
    return g_physics_system->m_search_tree_rbc_contact.find(key);
}

// ============================================================================
// Slot-count accessors
// ============================================================================
#define SLOT_ACCESSORS(POOL, TYPE)                                         \
    int max_slots_##TYPE() {                                               \
        return g_physics_system->POOL.m_slot_array_size;                   \
    }                                                                      \
    int available_slots_##TYPE() {                                         \
        return g_physics_system->POOL.m_slot_array_size -                  \
               g_physics_system->POOL.m_alloc_count;                       \
    }                                                                      \
    int used_slots_##TYPE() {                                              \
        return g_physics_system->POOL.m_alloc_count;                       \
    }

SLOT_ACCESSORS(m_list_rigid_body, rigid_body)
SLOT_ACCESSORS(m_list_user_rigid_body, user_rigid_body)
SLOT_ACCESSORS(m_list_rbc_point, rbc_point)
SLOT_ACCESSORS(m_list_rbc_hinge, rbc_hinge)
SLOT_ACCESSORS(m_list_rbc_dist, rbc_dist)
SLOT_ACCESSORS(m_list_rbc_ragdoll, rbc_ragdoll)
SLOT_ACCESSORS(m_list_rbc_wheel, rbc_wheel)
SLOT_ACCESSORS(m_list_rbc_angular_actuator, rbc_angular_actuator)
SLOT_ACCESSORS(m_list_rbc_custom_orientation, rbc_custom_orientation)
SLOT_ACCESSORS(m_list_rbc_custom_path, rbc_custom_path)
SLOT_ACCESSORS(m_list_rbc_contact, rbc_contact)

#undef SLOT_ACCESSORS

// ============================================================================
// create<T> free template - ea: 0x881AC0 (point/custom_orientation),
//    0x883430 (contact, adds to search tree)
// ============================================================================
template <typename Pool, typename T>
T* create(Pool* list_name, T*, rigid_body* const b1, rigid_body* const b2,
          bool no_error, const char* error_msg) {
    if (g_physics_system->is_member(b1) == 0 &&
        _tlAssert("source/physics_system.cpp", 70, "PSYS()->is_member(b1)", ""))
        __debugbreak();
    if (g_physics_system->is_member(b2) == 0 &&
        _tlAssert("source/physics_system.cpp", 71, "PSYS()->is_member(b2)", ""))
        __debugbreak();
    T* result = list_name->add(no_error, error_msg);
    if (result != NULL) {
        result->b1 = b1;
        result->b2 = b2;
    }
    return result;
}

// contact specialization - ea: 0x883430
rigid_body_constraint_contact* create(
    phys_heap_memory_pool<rigid_body_constraint_contact>* list,
    rigid_body_constraint_contact* a2, rigid_body* const b1, rigid_body* const b2,
    bool no_error, const char* error_msg) {
    (void)a2;
    if (g_physics_system->is_member(b1) == 0 &&
        _tlAssert("source/physics_system.cpp", 82, "PSYS()->is_member(b1)", ""))
        __debugbreak();
    if (g_physics_system->is_member(b2) == 0 &&
        _tlAssert("source/physics_system.cpp", 83, "PSYS()->is_member(b2)", ""))
        __debugbreak();
    rigid_body_pair_key key(b1, b2);
    physics_system* v6 = g_physics_system;
    rigid_body_constraint_contact* result =
        g_physics_system->m_search_tree_rbc_contact.find(key);
    if (result == NULL) {
        int m_alloc_count = v6->m_list_rbc_contact.m_alloc_count;
        if (m_alloc_count < v6->m_list_rbc_contact.m_slot_array_size) {
            rigid_body_constraint_contact* v9 =
                v6->m_list_rbc_contact.m_alloc_list[m_alloc_count];
            v6->m_list_rbc_contact.m_alloc_count = m_alloc_count + 1;
            if (v9 != NULL) {
                v9->m_list_contact_point_info_buffer_1.m_first = NULL;
                v9->m_list_contact_point_info_buffer_2.m_first = NULL;
                v9->m_solver_priority = 0;
                v9->b1 = b1;
                v9->b2 = b2;
                g_physics_system->m_search_tree_rbc_contact.add(key, v9);
            }
            return v9;
        } else {
            if (!no_error)
                tlFatal(error_msg);
            return NULL;
        }
    }
    return result;
}

// ============================================================================
// create_* entry points
// ============================================================================
rigid_body* create_rigid_body(bool no_error) {
    int m_alloc_count = g_physics_system->m_list_rigid_body.m_alloc_count;
    if (m_alloc_count < g_physics_system->m_list_rigid_body.m_slot_array_size) {
        rigid_body* result = g_physics_system->m_list_rigid_body.m_alloc_list[m_alloc_count];
        g_physics_system->m_list_rigid_body.m_alloc_count = m_alloc_count + 1;
        return result;
    }
    if (!no_error)
        tlFatal("POOL OUT OF MEMORY, rigid_body, INCREASE phys_mem_info::m_num_rigid_body.");
    return NULL;
}

user_rigid_body* create_user_rigid_body(bool no_error) {
    int m_alloc_count = g_physics_system->m_list_user_rigid_body.m_alloc_count;
    if (m_alloc_count < g_physics_system->m_list_user_rigid_body.m_slot_array_size) {
        user_rigid_body* result =
            g_physics_system->m_list_user_rigid_body.m_alloc_list[m_alloc_count];
        g_physics_system->m_list_user_rigid_body.m_alloc_count = m_alloc_count + 1;
        return result;
    }
    if (!no_error)
        tlFatal("POOL OUT OF MEMORY, user_rigid_body, INCREASE phys_mem_info::m_num_user_rigid_body.");
    return NULL;
}

rigid_body_constraint_point* create_rbc_point(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error) {
    return create(&g_physics_system->m_list_rbc_point, (rigid_body_constraint_point*)NULL,
                  b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_point, INCREASE phys_mem_info::m_num_rbc_point.");
}

rigid_body_constraint_custom_orientation* create_rbc_custom_orientation(
    rigid_body* const b1, rigid_body* const b2, bool no_error) {
    return create(&g_physics_system->m_list_rbc_custom_orientation,
                  (rigid_body_constraint_custom_orientation*)NULL, b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_custom_orientation, INCREASE phys_mem_info::m_num_rbc_custom_orientation.");
}

rigid_body_constraint_hinge* create_rbc_hinge(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error) {
    return create(&g_physics_system->m_list_rbc_hinge, (rigid_body_constraint_hinge*)NULL,
                  b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_hinge, INCREASE phys_mem_info::m_num_rbc_hinge.");
}

rigid_body_constraint_distance* create_rbc_dist(rigid_body* const b1, rigid_body* const b2,
                                                bool no_error) {
    return create(&g_physics_system->m_list_rbc_dist, (rigid_body_constraint_distance*)NULL,
                  b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_dist, INCREASE phys_mem_info::m_num_rbc_dist.");
}

rigid_body_constraint_ragdoll* create_rbc_ragdoll(rigid_body* const b1, rigid_body* const b2,
                                                  bool no_error) {
    return create(&g_physics_system->m_list_rbc_ragdoll,
                  (rigid_body_constraint_ragdoll*)NULL, b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_ragdoll, INCREASE phys_mem_info::m_num_rbc_ragdoll.");
}

rigid_body_constraint_wheel* create_rbc_wheel(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error) {
    return create(&g_physics_system->m_list_rbc_wheel, (rigid_body_constraint_wheel*)NULL,
                  b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_wheel, INCREASE phys_mem_info::m_num_rbc_wheel.");
}

rigid_body_constraint_angular_actuator* create_rbc_angular_actuator(
    rigid_body* const b1, rigid_body* const b2, bool no_error) {
    return create(&g_physics_system->m_list_rbc_angular_actuator,
                  (rigid_body_constraint_angular_actuator*)NULL, b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_angular_actuator, INCREASE phys_mem_info::m_num_rbc_angular_actuator.");
}

rigid_body_constraint_custom_path* create_rbc_custom_path(
    rigid_body* const b1, rigid_body* const b2, bool no_error) {
    return create(&g_physics_system->m_list_rbc_custom_path,
                  (rigid_body_constraint_custom_path*)NULL, b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_custom_path, INCREASE phys_mem_info::m_num_rbc_custom_path.");
}

rigid_body_constraint_contact* create_rbc_contact(rigid_body* const b1,
                                                  rigid_body* const b2,
                                                  bool no_error) {
    return create(&g_physics_system->m_list_rbc_contact,
                  (rigid_body_constraint_contact*)NULL, b1, b2, no_error,
                  "OUT_OF_MEMORY, rbc_contact, INCREASE phys_mem_info::m_num_rbc_contact.");
}

// ============================================================================
// get_user_rigid_body - ea: 0x87F0C0
// ============================================================================
user_rigid_body* get_user_rigid_body(const math::Mat43* const dictactor) {
    user_rigid_body** m_alloc_list = g_physics_system->m_list_user_rigid_body.m_alloc_list;
    user_rigid_body** v2 = &m_alloc_list[g_physics_system->m_list_user_rigid_body.m_alloc_count];
    if (v2 == m_alloc_list)
        return NULL;
    user_rigid_body* v3;
    while (1) {
        v3 = *m_alloc_list;
        if ((v3->m_flags & 0x20) == 0 &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 114,
                      "rb->is_user_rigid_body()", ""))
            __debugbreak();
        if (v3->m_dictator == dictactor)
            break;
        if (v2 == ++m_alloc_list)
            return NULL;
    }
    return v3;
}

// ============================================================================
// destroy_* entry points
// ============================================================================
void destroy(rigid_body_constraint_point* const rbc) {
    g_physics_system->m_list_rbc_point.remove(rbc);
}
void destroy(rigid_body_constraint_hinge* const rbc) {
    g_physics_system->m_list_rbc_hinge.remove(rbc);
}
void destroy(rigid_body_constraint_distance* const rbc) {
    g_physics_system->m_list_rbc_dist.remove(rbc);
}
void destroy(rigid_body_constraint_ragdoll* const rbc) {
    g_physics_system->m_list_rbc_ragdoll.remove(rbc);
}
void destroy(rigid_body_constraint_wheel* const rbc) {
    g_physics_system->m_list_rbc_wheel.remove(rbc);
}
void destroy(rigid_body_constraint_angular_actuator* const rbc) {
    g_physics_system->m_list_rbc_angular_actuator.remove(rbc);
}
void destroy(rigid_body_constraint_custom_orientation* const rbc) {
    g_physics_system->m_list_rbc_custom_orientation.remove(rbc);
}
void destroy(rigid_body_constraint_custom_path* const rbc) {
    g_physics_system->m_list_rbc_custom_path.remove(rbc);
}
void destroy(rigid_body_constraint_contact* const rbc) {
    g_physics_system->m_list_rbc_contact.remove(rbc);
}

void destroy(rigid_body* const rb) {
    destroy_all_constraint(rb);
    g_physics_system->m_list_rigid_body.remove(rb);
}

void destroy(user_rigid_body* const rb) {
    destroy_all_constraint(rb);
    g_physics_system->m_list_user_rigid_body.remove(rb);
}

// ============================================================================
// destroy_all_*
// ============================================================================
void destroy_all_rigid_body() {
    g_physics_system->m_list_rbc_point.reset_buffer();
    g_physics_system->m_list_rbc_hinge.reset_buffer();
    g_physics_system->m_list_rbc_dist.reset_buffer();
    g_physics_system->m_list_rbc_ragdoll.reset_buffer();
    g_physics_system->m_list_rbc_wheel.reset_buffer();
    g_physics_system->m_list_rbc_angular_actuator.reset_buffer();
    g_physics_system->m_list_rbc_custom_orientation.reset_buffer();
    g_physics_system->m_list_rbc_custom_path.reset_buffer();
    g_physics_system->m_list_rbc_contact.reset_buffer();
    g_physics_system->m_list_rigid_body.reset_buffer();
}

void destroy_all_user_rigid_body() {
    phys_heap_memory_pool<rigid_body_constraint_point>* p_rbc_point =
        &g_physics_system->m_list_rbc_point;
    rigid_body_constraint_point** m_alloc_list = p_rbc_point->m_alloc_list;
    if (&m_alloc_list[p_rbc_point->m_alloc_count] != m_alloc_list) {
        do {
            rigid_body* b1 = (*m_alloc_list)->b1;
            if (b1 != NULL && (b1->m_flags & 0x20) != 0 ||
                (b1 = (*m_alloc_list)->b2) != NULL && (b1->m_flags & 0x20) != 0) {
                p_rbc_point->remove(*m_alloc_list);
                p_rbc_point = &g_physics_system->m_list_rbc_point;
            } else {
                ++m_alloc_list;
            }
        } while (&g_physics_system->m_list_rbc_point
                         .m_alloc_list[g_physics_system->m_list_rbc_point.m_alloc_count] !=
                 m_alloc_list);
    }
    phys_heap_memory_pool<rigid_body_constraint_hinge>* p_rbc_hinge =
        &g_physics_system->m_list_rbc_hinge;
    rigid_body_constraint_hinge** v5 = p_rbc_hinge->m_alloc_list;
    if (&v5[p_rbc_hinge->m_alloc_count] != v5) {
        do {
            rigid_body* v7 = (*v5)->b1;
            if (v7 != NULL && (v7->m_flags & 0x20) != 0 ||
                (v7 = (*v5)->b2) != NULL && (v7->m_flags & 0x20) != 0) {
                p_rbc_hinge->remove(*v5);
                p_rbc_hinge = &g_physics_system->m_list_rbc_hinge;
            } else {
                ++v5;
            }
        } while (&g_physics_system->m_list_rbc_hinge
                         .m_alloc_list[g_physics_system->m_list_rbc_hinge.m_alloc_count] !=
                 v5);
    }
    phys_heap_memory_pool<rigid_body_constraint_distance>* p_rbc_dist =
        &g_physics_system->m_list_rbc_dist;
    rigid_body_constraint_distance** v9 = p_rbc_dist->m_alloc_list;
    if (&v9[p_rbc_dist->m_alloc_count] != v9) {
        do {
            rigid_body* v11 = (*v9)->b1;
            if (v11 != NULL && (v11->m_flags & 0x20) != 0 ||
                (v11 = (*v9)->b2) != NULL && (v11->m_flags & 0x20) != 0) {
                p_rbc_dist->remove(*v9);
                p_rbc_dist = &g_physics_system->m_list_rbc_dist;
            } else {
                ++v9;
            }
        } while (&g_physics_system->m_list_rbc_dist
                         .m_alloc_list[g_physics_system->m_list_rbc_dist.m_alloc_count] !=
                 v9);
    }
    phys_heap_memory_pool<rigid_body_constraint_ragdoll>* p_rbc_ragdoll =
        &g_physics_system->m_list_rbc_ragdoll;
    rigid_body_constraint_ragdoll** v13 = p_rbc_ragdoll->m_alloc_list;
    if (&v13[p_rbc_ragdoll->m_alloc_count] != v13) {
        do {
            rigid_body* v15 = (*v13)->b1;
            if (v15 != NULL && (v15->m_flags & 0x20) != 0 ||
                (v15 = (*v13)->b2) != NULL && (v15->m_flags & 0x20) != 0) {
                p_rbc_ragdoll->remove(*v13);
                p_rbc_ragdoll = &g_physics_system->m_list_rbc_ragdoll;
            } else {
                ++v13;
            }
        } while (&g_physics_system->m_list_rbc_ragdoll
                         .m_alloc_list[g_physics_system->m_list_rbc_ragdoll.m_alloc_count] !=
                 v13);
    }
    phys_heap_memory_pool<rigid_body_constraint_wheel>* p_rbc_wheel =
        &g_physics_system->m_list_rbc_wheel;
    rigid_body_constraint_wheel** v17 = p_rbc_wheel->m_alloc_list;
    if (&v17[p_rbc_wheel->m_alloc_count] != v17) {
        do {
            rigid_body* v19 = (*v17)->b1;
            if (v19 != NULL && (v19->m_flags & 0x20) != 0 ||
                (v19 = (*v17)->b2) != NULL && (v19->m_flags & 0x20) != 0) {
                p_rbc_wheel->remove(*v17);
                p_rbc_wheel = &g_physics_system->m_list_rbc_wheel;
            } else {
                ++v17;
            }
        } while (&g_physics_system->m_list_rbc_wheel
                         .m_alloc_list[g_physics_system->m_list_rbc_wheel.m_alloc_count] !=
                 v17);
    }
    phys_heap_memory_pool<rigid_body_constraint_angular_actuator>* p_rbc_aa =
        &g_physics_system->m_list_rbc_angular_actuator;
    rigid_body_constraint_angular_actuator** v21 = p_rbc_aa->m_alloc_list;
    if (&v21[p_rbc_aa->m_alloc_count] != v21) {
        do {
            rigid_body* v23 = (*v21)->b1;
            if (v23 != NULL && (v23->m_flags & 0x20) != 0 ||
                (v23 = (*v21)->b2) != NULL && (v23->m_flags & 0x20) != 0) {
                p_rbc_aa->remove(*v21);
                p_rbc_aa = &g_physics_system->m_list_rbc_angular_actuator;
            } else {
                ++v21;
            }
        } while (&g_physics_system->m_list_rbc_angular_actuator
                         .m_alloc_list[g_physics_system->m_list_rbc_angular_actuator.m_alloc_count] !=
                 v21);
    }
    phys_heap_memory_pool<rigid_body_constraint_custom_orientation>* p_rbc_co =
        &g_physics_system->m_list_rbc_custom_orientation;
    rigid_body_constraint_custom_orientation** v25 = p_rbc_co->m_alloc_list;
    if (&v25[p_rbc_co->m_alloc_count] != v25) {
        do {
            rigid_body* v27 = (*v25)->b1;
            if (v27 != NULL && (v27->m_flags & 0x20) != 0 ||
                (v27 = (*v25)->b2) != NULL && (v27->m_flags & 0x20) != 0) {
                p_rbc_co->remove(*v25);
                p_rbc_co = &g_physics_system->m_list_rbc_custom_orientation;
            } else {
                ++v25;
            }
        } while (&g_physics_system->m_list_rbc_custom_orientation
                         .m_alloc_list[g_physics_system->m_list_rbc_custom_orientation.m_alloc_count] !=
                 v25);
    }
    phys_heap_memory_pool<rigid_body_constraint_custom_path>* p_rbc_cp =
        &g_physics_system->m_list_rbc_custom_path;
    rigid_body_constraint_custom_path** v29 = p_rbc_cp->m_alloc_list;
    if (&v29[p_rbc_cp->m_alloc_count] != v29) {
        do {
            rigid_body* v31 = (*v29)->b1;
            if (v31 != NULL && (v31->m_flags & 0x20) != 0 ||
                (v31 = (*v29)->b2) != NULL && (v31->m_flags & 0x20) != 0) {
                p_rbc_cp->remove(*v29);
                p_rbc_cp = &g_physics_system->m_list_rbc_custom_path;
            } else {
                ++v29;
            }
        } while (&g_physics_system->m_list_rbc_custom_path
                         .m_alloc_list[g_physics_system->m_list_rbc_custom_path.m_alloc_count] !=
                 v29);
    }
    phys_heap_memory_pool<rigid_body_constraint_contact>* p_rbc_contact =
        &g_physics_system->m_list_rbc_contact;
    rigid_body_constraint_contact** v33 = p_rbc_contact->m_alloc_list;
    if (&v33[p_rbc_contact->m_alloc_count] != v33) {
        do {
            rigid_body* v35 = (*v33)->b1;
            if (v35 != NULL && (v35->m_flags & 0x20) != 0 ||
                (v35 = (*v33)->b2) != NULL && (v35->m_flags & 0x20) != 0) {
                p_rbc_contact->remove(*v33);
                p_rbc_contact = &g_physics_system->m_list_rbc_contact;
            } else {
                ++v33;
            }
        } while (&g_physics_system->m_list_rbc_contact
                         .m_alloc_list[g_physics_system->m_list_rbc_contact.m_alloc_count] !=
                 v33);
    }
    g_physics_system->m_list_user_rigid_body.reset_buffer();
}

void destroy_all_rbc_point() {
    g_physics_system->m_list_rbc_point.reset_buffer();
}
void destroy_all_rbc_hinge() {
    g_physics_system->m_list_rbc_hinge.reset_buffer();
}
void destroy_all_rbc_dist() {
    g_physics_system->m_list_rbc_dist.reset_buffer();
}
void destroy_all_rbc_ragdoll() {
    g_physics_system->m_list_rbc_ragdoll.reset_buffer();
}
void destroy_all_rbc_wheel() {
    g_physics_system->m_list_rbc_wheel.reset_buffer();
}
void destroy_all_rbc_angular_actuator() {
    g_physics_system->m_list_rbc_angular_actuator.reset_buffer();
}
void destroy_all_rbc_custom_orientation() {
    g_physics_system->m_list_rbc_custom_orientation.reset_buffer();
}
void destroy_all_rbc_custom_path() {
    g_physics_system->m_list_rbc_custom_path.reset_buffer();
}
void destroy_all_rbc_contact() {
    g_physics_system->m_list_rbc_contact.reset_buffer();
}

void destroy_all_constraint(rigid_body* const rb) {
    phys_heap_memory_pool<rigid_body_constraint_point>* p_rbc_point =
        &g_physics_system->m_list_rbc_point;
    rigid_body_constraint_point** m_alloc_list = p_rbc_point->m_alloc_list;
    if (&m_alloc_list[p_rbc_point->m_alloc_count] != m_alloc_list) {
        do {
            rigid_body* b1 = (*m_alloc_list)->b1;
            if (b1 != NULL && b1 == rb ||
                (b1 = (*m_alloc_list)->b2) != NULL && b1 == rb) {
                p_rbc_point->remove(*m_alloc_list);
                p_rbc_point = &g_physics_system->m_list_rbc_point;
            } else {
                ++m_alloc_list;
            }
        } while (&g_physics_system->m_list_rbc_point
                         .m_alloc_list[g_physics_system->m_list_rbc_point.m_alloc_count] !=
                 m_alloc_list);
    }
    phys_heap_memory_pool<rigid_body_constraint_hinge>* p_rbc_hinge =
        &g_physics_system->m_list_rbc_hinge;
    rigid_body_constraint_hinge** v6 = p_rbc_hinge->m_alloc_list;
    if (&v6[p_rbc_hinge->m_alloc_count] != v6) {
        do {
            rigid_body* v8 = (*v6)->b1;
            if (v8 != NULL && v8 == rb ||
                (v8 = (*v6)->b2) != NULL && v8 == rb) {
                p_rbc_hinge->remove(*v6);
                p_rbc_hinge = &g_physics_system->m_list_rbc_hinge;
            } else {
                ++v6;
            }
        } while (&g_physics_system->m_list_rbc_hinge
                         .m_alloc_list[g_physics_system->m_list_rbc_hinge.m_alloc_count] !=
                 v6);
    }
    phys_heap_memory_pool<rigid_body_constraint_distance>* p_rbc_dist =
        &g_physics_system->m_list_rbc_dist;
    rigid_body_constraint_distance** v10 = p_rbc_dist->m_alloc_list;
    if (&v10[p_rbc_dist->m_alloc_count] != v10) {
        do {
            rigid_body* v12 = (*v10)->b1;
            if (v12 != NULL && v12 == rb ||
                (v12 = (*v10)->b2) != NULL && v12 == rb) {
                p_rbc_dist->remove(*v10);
                p_rbc_dist = &g_physics_system->m_list_rbc_dist;
            } else {
                ++v10;
            }
        } while (&g_physics_system->m_list_rbc_dist
                         .m_alloc_list[g_physics_system->m_list_rbc_dist.m_alloc_count] !=
                 v10);
    }
    phys_heap_memory_pool<rigid_body_constraint_ragdoll>* p_rbc_ragdoll =
        &g_physics_system->m_list_rbc_ragdoll;
    rigid_body_constraint_ragdoll** v14 = p_rbc_ragdoll->m_alloc_list;
    if (&v14[p_rbc_ragdoll->m_alloc_count] != v14) {
        do {
            rigid_body* v16 = (*v14)->b1;
            if (v16 != NULL && v16 == rb ||
                (v16 = (*v14)->b2) != NULL && v16 == rb) {
                p_rbc_ragdoll->remove(*v14);
                p_rbc_ragdoll = &g_physics_system->m_list_rbc_ragdoll;
            } else {
                ++v14;
            }
        } while (&g_physics_system->m_list_rbc_ragdoll
                         .m_alloc_list[g_physics_system->m_list_rbc_ragdoll.m_alloc_count] !=
                 v14);
    }
    phys_heap_memory_pool<rigid_body_constraint_wheel>* p_rbc_wheel =
        &g_physics_system->m_list_rbc_wheel;
    rigid_body_constraint_wheel** v18 = p_rbc_wheel->m_alloc_list;
    if (&v18[p_rbc_wheel->m_alloc_count] != v18) {
        do {
            rigid_body* v20 = (*v18)->b1;
            if (v20 != NULL && v20 == rb ||
                (v20 = (*v18)->b2) != NULL && v20 == rb) {
                p_rbc_wheel->remove(*v18);
                p_rbc_wheel = &g_physics_system->m_list_rbc_wheel;
            } else {
                ++v18;
            }
        } while (&g_physics_system->m_list_rbc_wheel
                         .m_alloc_list[g_physics_system->m_list_rbc_wheel.m_alloc_count] !=
                 v18);
    }
    phys_heap_memory_pool<rigid_body_constraint_angular_actuator>* p_rbc_aa =
        &g_physics_system->m_list_rbc_angular_actuator;
    rigid_body_constraint_angular_actuator** v22 = p_rbc_aa->m_alloc_list;
    if (&v22[p_rbc_aa->m_alloc_count] != v22) {
        do {
            rigid_body* v24 = (*v22)->b1;
            if (v24 != NULL && v24 == rb ||
                (v24 = (*v22)->b2) != NULL && v24 == rb) {
                p_rbc_aa->remove(*v22);
                p_rbc_aa = &g_physics_system->m_list_rbc_angular_actuator;
            } else {
                ++v22;
            }
        } while (&g_physics_system->m_list_rbc_angular_actuator
                         .m_alloc_list[g_physics_system->m_list_rbc_angular_actuator.m_alloc_count] !=
                 v22);
    }
    phys_heap_memory_pool<rigid_body_constraint_custom_orientation>* p_rbc_co =
        &g_physics_system->m_list_rbc_custom_orientation;
    rigid_body_constraint_custom_orientation** v26 = p_rbc_co->m_alloc_list;
    if (&v26[p_rbc_co->m_alloc_count] != v26) {
        do {
            rigid_body* v28 = (*v26)->b1;
            if (v28 != NULL && v28 == rb ||
                (v28 = (*v26)->b2) != NULL && v28 == rb) {
                p_rbc_co->remove(*v26);
                p_rbc_co = &g_physics_system->m_list_rbc_custom_orientation;
            } else {
                ++v26;
            }
        } while (&g_physics_system->m_list_rbc_custom_orientation
                         .m_alloc_list[g_physics_system->m_list_rbc_custom_orientation.m_alloc_count] !=
                 v26);
    }
    phys_heap_memory_pool<rigid_body_constraint_custom_path>* p_rbc_cp =
        &g_physics_system->m_list_rbc_custom_path;
    rigid_body_constraint_custom_path** v30 = p_rbc_cp->m_alloc_list;
    if (&v30[p_rbc_cp->m_alloc_count] != v30) {
        do {
            rigid_body* v32 = (*v30)->b1;
            if (v32 != NULL && v32 == rb ||
                (v32 = (*v30)->b2) != NULL && v32 == rb) {
                p_rbc_cp->remove(*v30);
                p_rbc_cp = &g_physics_system->m_list_rbc_custom_path;
            } else {
                ++v30;
            }
        } while (&g_physics_system->m_list_rbc_custom_path
                         .m_alloc_list[g_physics_system->m_list_rbc_custom_path.m_alloc_count] !=
                 v30);
    }
    phys_heap_memory_pool<rigid_body_constraint_contact>* p_rbc_contact =
        &g_physics_system->m_list_rbc_contact;
    rigid_body_constraint_contact** v34 = p_rbc_contact->m_alloc_list;
    if (&v34[p_rbc_contact->m_alloc_count] != v34) {
        do {
            rigid_body* v36 = (*v34)->b1;
            if (v36 != NULL && v36 == rb ||
                (v36 = (*v34)->b2) != NULL && v36 == rb) {
                p_rbc_contact->remove(*v34);
                p_rbc_contact = &g_physics_system->m_list_rbc_contact;
            } else {
                ++v34;
            }
        } while (&g_physics_system->m_list_rbc_contact
                         .m_alloc_list[g_physics_system->m_list_rbc_contact.m_alloc_count] !=
                 v34);
    }
}

void destroy_all_constraint_with_user_rigid_body(rigid_body* const rb) {
    phys_heap_memory_pool<rigid_body_constraint_point>* p_rbc_point =
        &g_physics_system->m_list_rbc_point;
    rigid_body_constraint_point** m_alloc_list = p_rbc_point->m_alloc_list;
    if (&m_alloc_list[p_rbc_point->m_alloc_count] != m_alloc_list) {
        do {
            rigid_body* b1 = (*m_alloc_list)->b1;
            if (b1 != NULL) {
                rigid_body* b2 = (*m_alloc_list)->b2;
                if (b2 != NULL &&
                    (b1 == rb && (b2->m_flags & 0x20) != 0 ||
                     b2 == rb && (b1->m_flags & 0x20) != 0)) {
                    p_rbc_point->remove(*m_alloc_list);
                    p_rbc_point = &g_physics_system->m_list_rbc_point;
                } else {
                    ++m_alloc_list;
                }
            } else {
                ++m_alloc_list;
            }
        } while (&g_physics_system->m_list_rbc_point
                         .m_alloc_list[g_physics_system->m_list_rbc_point.m_alloc_count] !=
                 m_alloc_list);
    }
    phys_heap_memory_pool<rigid_body_constraint_hinge>* p_rbc_hinge =
        &g_physics_system->m_list_rbc_hinge;
    rigid_body_constraint_hinge** m_ptr = p_rbc_hinge->m_alloc_list;
    if (&m_ptr[p_rbc_hinge->m_alloc_count] != m_ptr) {
        while (1) {
            rigid_body_constraint_hinge* v8 = *m_ptr;
            rigid_body* v9 = v8->b1;
            if (v9 == NULL)
                goto advance_hinge;
            rigid_body* v10 = v8->b2;
            if (v10 == NULL)
                goto advance_hinge;
            if ((v9 != rb || (v10->m_flags & 0x20) == 0) &&
                (v10 != rb || (v9->m_flags & 0x20) == 0))
                break;
            p_rbc_hinge->remove(v8);
            p_rbc_hinge = &g_physics_system->m_list_rbc_hinge;
            if (&g_physics_system->m_list_rbc_hinge
                    .m_alloc_list[g_physics_system->m_list_rbc_hinge.m_alloc_count] ==
                m_ptr)
                goto hinge_done;
            continue;
        advance_hinge:
            ++m_ptr;
            p_rbc_hinge = &g_physics_system->m_list_rbc_hinge;
            if (&g_physics_system->m_list_rbc_hinge
                    .m_alloc_list[g_physics_system->m_list_rbc_hinge.m_alloc_count] ==
                m_ptr)
                break;
        }
    }
hinge_done:
    phys_heap_memory_pool<rigid_body_constraint_distance>* p_rbc_dist =
        &g_physics_system->m_list_rbc_dist;
    rigid_body_constraint_distance** v11 = p_rbc_dist->m_alloc_list;
    if (&v11[p_rbc_dist->m_alloc_count] != v11) {
        while (1) {
            rigid_body* v13 = (*v11)->b1;
            if (v13 == NULL)
                goto advance_dist;
            rigid_body* v14 = (*v11)->b2;
            if (v14 == NULL)
                goto advance_dist;
            if ((v13 != rb || (v14->m_flags & 0x20) == 0) &&
                (v14 != rb || (v13->m_flags & 0x20) == 0))
                break;
            p_rbc_dist->remove(*v11);
            p_rbc_dist = &g_physics_system->m_list_rbc_dist;
            if (&g_physics_system->m_list_rbc_dist
                    .m_alloc_list[g_physics_system->m_list_rbc_dist.m_alloc_count] ==
                v11)
                goto dist_done;
            continue;
        advance_dist:
            ++v11;
            p_rbc_dist = &g_physics_system->m_list_rbc_dist;
            if (&g_physics_system->m_list_rbc_dist
                    .m_alloc_list[g_physics_system->m_list_rbc_dist.m_alloc_count] ==
                v11)
                break;
        }
    }
dist_done:
    phys_heap_memory_pool<rigid_body_constraint_ragdoll>* p_rbc_ragdoll =
        &g_physics_system->m_list_rbc_ragdoll;
    rigid_body_constraint_ragdoll** v15 = p_rbc_ragdoll->m_alloc_list;
    if (&v15[p_rbc_ragdoll->m_alloc_count] != v15) {
        while (1) {
            rigid_body* v17 = (*v15)->b1;
            if (v17 == NULL)
                goto advance_ragdoll;
            rigid_body* v18 = (*v15)->b2;
            if (v18 == NULL)
                goto advance_ragdoll;
            if ((v17 != rb || (v18->m_flags & 0x20) == 0) &&
                (v18 != rb || (v17->m_flags & 0x20) == 0))
                break;
            p_rbc_ragdoll->remove(*v15);
            p_rbc_ragdoll = &g_physics_system->m_list_rbc_ragdoll;
            if (&g_physics_system->m_list_rbc_ragdoll
                    .m_alloc_list[g_physics_system->m_list_rbc_ragdoll.m_alloc_count] ==
                v15)
                goto ragdoll_done;
            continue;
        advance_ragdoll:
            ++v15;
            p_rbc_ragdoll = &g_physics_system->m_list_rbc_ragdoll;
            if (&g_physics_system->m_list_rbc_ragdoll
                    .m_alloc_list[g_physics_system->m_list_rbc_ragdoll.m_alloc_count] ==
                v15)
                break;
        }
    }
ragdoll_done:
    phys_heap_memory_pool<rigid_body_constraint_wheel>* p_rbc_wheel =
        &g_physics_system->m_list_rbc_wheel;
    rigid_body_constraint_wheel** v19 = p_rbc_wheel->m_alloc_list;
    if (&v19[p_rbc_wheel->m_alloc_count] != v19) {
        while (1) {
            rigid_body* v21 = (*v19)->b1;
            if (v21 == NULL)
                goto advance_wheel;
            rigid_body* v22 = (*v19)->b2;
            if (v22 == NULL)
                goto advance_wheel;
            if ((v21 != rb || (v22->m_flags & 0x20) == 0) &&
                (v22 != rb || (v21->m_flags & 0x20) == 0))
                break;
            p_rbc_wheel->remove(*v19);
            p_rbc_wheel = &g_physics_system->m_list_rbc_wheel;
            if (&g_physics_system->m_list_rbc_wheel
                    .m_alloc_list[g_physics_system->m_list_rbc_wheel.m_alloc_count] ==
                v19)
                goto wheel_done;
            continue;
        advance_wheel:
            ++v19;
            p_rbc_wheel = &g_physics_system->m_list_rbc_wheel;
            if (&g_physics_system->m_list_rbc_wheel
                    .m_alloc_list[g_physics_system->m_list_rbc_wheel.m_alloc_count] ==
                v19)
                break;
        }
    }
wheel_done:
    phys_heap_memory_pool<rigid_body_constraint_angular_actuator>* p_rbc_aa =
        &g_physics_system->m_list_rbc_angular_actuator;
    rigid_body_constraint_angular_actuator** v23 = p_rbc_aa->m_alloc_list;
    if (&v23[p_rbc_aa->m_alloc_count] != v23) {
        while (1) {
            rigid_body* v25 = (*v23)->b1;
            if (v25 == NULL)
                goto advance_aa;
            rigid_body* v26 = (*v23)->b2;
            if (v26 == NULL)
                goto advance_aa;
            if ((v25 != rb || (v26->m_flags & 0x20) == 0) &&
                (v26 != rb || (v25->m_flags & 0x20) == 0))
                break;
            p_rbc_aa->remove(*v23);
            p_rbc_aa = &g_physics_system->m_list_rbc_angular_actuator;
            if (&g_physics_system->m_list_rbc_angular_actuator
                    .m_alloc_list[g_physics_system->m_list_rbc_angular_actuator.m_alloc_count] ==
                v23)
                goto aa_done;
            continue;
        advance_aa:
            ++v23;
            p_rbc_aa = &g_physics_system->m_list_rbc_angular_actuator;
            if (&g_physics_system->m_list_rbc_angular_actuator
                    .m_alloc_list[g_physics_system->m_list_rbc_angular_actuator.m_alloc_count] ==
                v23)
                break;
        }
    }
aa_done:
    phys_heap_memory_pool<rigid_body_constraint_custom_orientation>* p_rbc_co =
        &g_physics_system->m_list_rbc_custom_orientation;
    rigid_body_constraint_custom_orientation** v27 = p_rbc_co->m_alloc_list;
    if (&v27[p_rbc_co->m_alloc_count] != v27) {
        while (1) {
            rigid_body* v29 = (*v27)->b1;
            if (v29 == NULL)
                goto advance_co;
            rigid_body* v30 = (*v27)->b2;
            if (v30 == NULL)
                goto advance_co;
            if ((v29 != rb || (v30->m_flags & 0x20) == 0) &&
                (v30 != rb || (v29->m_flags & 0x20) == 0))
                break;
            p_rbc_co->remove(*v27);
            p_rbc_co = &g_physics_system->m_list_rbc_custom_orientation;
            if (&g_physics_system->m_list_rbc_custom_orientation
                    .m_alloc_list[g_physics_system->m_list_rbc_custom_orientation.m_alloc_count] ==
                v27)
                goto co_done;
            continue;
        advance_co:
            ++v27;
            p_rbc_co = &g_physics_system->m_list_rbc_custom_orientation;
            if (&g_physics_system->m_list_rbc_custom_orientation
                    .m_alloc_list[g_physics_system->m_list_rbc_custom_orientation.m_alloc_count] ==
                v27)
                break;
        }
    }
co_done:
    phys_heap_memory_pool<rigid_body_constraint_custom_path>* p_rbc_cp =
        &g_physics_system->m_list_rbc_custom_path;
    rigid_body_constraint_custom_path** v31 = p_rbc_cp->m_alloc_list;
    if (&v31[p_rbc_cp->m_alloc_count] != v31) {
        while (1) {
            rigid_body* v33 = (*v31)->b1;
            if (v33 == NULL)
                goto advance_cp;
            rigid_body* v34 = (*v31)->b2;
            if (v34 == NULL)
                goto advance_cp;
            if ((v33 != rb || (v34->m_flags & 0x20) == 0) &&
                (v34 != rb || (v33->m_flags & 0x20) == 0))
                break;
            p_rbc_cp->remove(*v31);
            p_rbc_cp = &g_physics_system->m_list_rbc_custom_path;
            if (&g_physics_system->m_list_rbc_custom_path
                    .m_alloc_list[g_physics_system->m_list_rbc_custom_path.m_alloc_count] ==
                v31)
                goto cp_done;
            continue;
        advance_cp:
            ++v31;
            p_rbc_cp = &g_physics_system->m_list_rbc_custom_path;
            if (&g_physics_system->m_list_rbc_custom_path
                    .m_alloc_list[g_physics_system->m_list_rbc_custom_path.m_alloc_count] ==
                v31)
                break;
        }
    }
cp_done:
    phys_heap_memory_pool<rigid_body_constraint_contact>* p_rbc_contact =
        &g_physics_system->m_list_rbc_contact;
    rigid_body_constraint_contact** v35 = p_rbc_contact->m_alloc_list;
    if (&v35[p_rbc_contact->m_alloc_count] != v35) {
        while (1) {
            rigid_body* v37 = (*v35)->b1;
            if (v37 == NULL)
                goto advance_contact;
            rigid_body* v38 = (*v35)->b2;
            if (v38 == NULL)
                goto advance_contact;
            if ((v37 != rb || (v38->m_flags & 0x20) == 0) &&
                (v38 != rb || (v37->m_flags & 0x20) == 0))
                break;
            p_rbc_contact->remove(*v35);
            p_rbc_contact = &g_physics_system->m_list_rbc_contact;
            if (&g_physics_system->m_list_rbc_contact
                    .m_alloc_list[g_physics_system->m_list_rbc_contact.m_alloc_count] ==
                v35)
                return;
            continue;
        advance_contact:
            ++v35;
            p_rbc_contact = &g_physics_system->m_list_rbc_contact;
            if (&g_physics_system->m_list_rbc_contact
                    .m_alloc_list[g_physics_system->m_list_rbc_contact.m_alloc_count] ==
                v35)
                return;
        }
    }
}

int destroy_all_unused_user_rigid_body() {
    physics_system* v0 = g_physics_system;
    user_rigid_body** m_alloc_list = g_physics_system->m_list_user_rigid_body.m_alloc_list;
    phys_heap_memory_pool<user_rigid_body>* p_list = &g_physics_system->m_list_user_rigid_body;
    int result = (int)&m_alloc_list[g_physics_system->m_list_user_rigid_body.m_alloc_count];
    for (; result != (int)m_alloc_list;
         result = (int)&v0->m_list_user_rigid_body
                       .m_alloc_list[v0->m_list_user_rigid_body.m_alloc_count]) {
        if ((*m_alloc_list)->m_constraint_count != 0) {
            ++m_alloc_list;
        } else {
            p_list->remove(*m_alloc_list);
            v0 = g_physics_system;
        }
        p_list = &v0->m_list_user_rigid_body;
    }
    return result;
}

// ============================================================================
// update_constraint_infos - ea: 0x87F130
// ============================================================================
static rigid_body_constraint_contact** update_constraint_infos() {
    environment_rigid_body* p_env = &g_physics_system->m_environment_rigid_body;
    g_physics_system->m_environment_rigid_body.m_constraint_count = 0;
    p_env->m_contact_count = 0;

    user_rigid_body** m_alloc_list = g_physics_system->m_list_user_rigid_body.m_alloc_list;
    for (user_rigid_body** i = &m_alloc_list[g_physics_system->m_list_user_rigid_body.m_alloc_count];
         m_alloc_list != i; *((int*)((char*)*m_alloc_list + 344)) = 0) {
        *((int*)((char*)*m_alloc_list + 340)) = 0;
        ++m_alloc_list;
    }
    rigid_body** v4 = g_physics_system->m_list_rigid_body.m_alloc_list;
    for (rigid_body** j = &v4[g_physics_system->m_list_rigid_body.m_alloc_count];
         v4 != j; *((int*)((char*)*v4 + 344)) = 0) {
        *((int*)((char*)*v4 + 340)) = 0;
        ++v4;
    }

    physics_system* v7 = g_physics_system;
    rigid_body_constraint_point** v8 = g_physics_system->m_list_rbc_point.m_alloc_list;
    rigid_body_constraint_point** v9 = &v8[g_physics_system->m_list_rbc_point.m_alloc_count];
    if (v9 != v8) {
        do {
            rigid_body_constraint_point* v10 = *v8;
            rigid_body* b1 = v10->b1;
            if (b1 != NULL)
                ++b1->m_constraint_count;
            rigid_body* b2 = v10->b2;
            if (b2 != NULL)
                ++b2->m_constraint_count;
            ++v8;
        } while (v8 != v9);
        v7 = g_physics_system;
    }
    rigid_body_constraint_hinge** v13 = v7->m_list_rbc_hinge.m_alloc_list;
    rigid_body_constraint_hinge** v14 = &v13[v7->m_list_rbc_hinge.m_alloc_count];
    if (v14 != v13) {
        do {
            rigid_body_constraint_hinge* v15 = *v13;
            rigid_body* v16 = v15->b1;
            if (v16 != NULL)
                ++v16->m_constraint_count;
            rigid_body* v17 = v15->b2;
            if (v17 != NULL)
                ++v17->m_constraint_count;
            ++v13;
        } while (v13 != v14);
        v7 = g_physics_system;
    }
    rigid_body_constraint_distance** v18 = v7->m_list_rbc_dist.m_alloc_list;
    rigid_body_constraint_distance** v19 = &v18[v7->m_list_rbc_dist.m_alloc_count];
    if (v19 != v18) {
        do {
            rigid_body_constraint_distance* v20 = *v18;
            rigid_body* v21 = v20->b1;
            if (v21 != NULL)
                ++v21->m_constraint_count;
            rigid_body* v22 = v20->b2;
            if (v22 != NULL)
                ++v22->m_constraint_count;
            ++v18;
        } while (v18 != v19);
        v7 = g_physics_system;
    }
    rigid_body_constraint_ragdoll** v23 = v7->m_list_rbc_ragdoll.m_alloc_list;
    rigid_body_constraint_ragdoll** v24 = &v23[v7->m_list_rbc_ragdoll.m_alloc_count];
    if (v24 != v23) {
        do {
            rigid_body_constraint_ragdoll* v25 = *v23;
            rigid_body* v26 = v25->b1;
            if (v26 != NULL)
                ++v26->m_constraint_count;
            rigid_body* v27 = v25->b2;
            if (v27 != NULL)
                ++v27->m_constraint_count;
            ++v23;
        } while (v23 != v24);
        v7 = g_physics_system;
    }
    rigid_body_constraint_wheel** v28 = v7->m_list_rbc_wheel.m_alloc_list;
    for (rigid_body_constraint_wheel** k = &v28[v7->m_list_rbc_wheel.m_alloc_count];
         v28 != k; ++v28) {
        rigid_body_constraint_wheel* v30 = *v28;
        rigid_body* v31 = v30->b1;
        if (v31 != NULL)
            ++v31->m_constraint_count;
        rigid_body* v32 = v30->b2;
        if (v32 != NULL)
            ++v32->m_constraint_count;
        if ((v30->m_wheel_flags & 1) != 0) {
            if (v30->b1 != NULL)
                ++v30->b1->m_contact_count;
            rigid_body* v33 = v30->b2;
            if (v33 != NULL)
                ++v33->m_contact_count;
        }
    }
    physics_system* v34 = g_physics_system;
    rigid_body_constraint_angular_actuator** v35 =
        g_physics_system->m_list_rbc_angular_actuator.m_alloc_list;
    rigid_body_constraint_angular_actuator** v36 =
        &v35[g_physics_system->m_list_rbc_angular_actuator.m_alloc_count];
    if (v36 != v35) {
        do {
            rigid_body_constraint_angular_actuator* v37 = *v35;
            rigid_body* v38 = v37->b1;
            if (v38 != NULL)
                ++v38->m_constraint_count;
            rigid_body* v39 = v37->b2;
            if (v39 != NULL)
                ++v39->m_constraint_count;
            ++v35;
        } while (v35 != v36);
        v34 = g_physics_system;
    }
    rigid_body_constraint_custom_orientation** v40 =
        v34->m_list_rbc_custom_orientation.m_alloc_list;
    rigid_body_constraint_custom_orientation** v41 =
        &v40[v34->m_list_rbc_custom_orientation.m_alloc_count];
    if (v41 != v40) {
        do {
            rigid_body_constraint_custom_orientation* v42 = *v40;
            rigid_body* v43 = v42->b1;
            if (v43 != NULL)
                ++v43->m_constraint_count;
            rigid_body* v44 = v42->b2;
            if (v44 != NULL)
                ++v44->m_constraint_count;
            ++v40;
        } while (v40 != v41);
        v34 = g_physics_system;
    }
    rigid_body_constraint_custom_path** v45 = v34->m_list_rbc_custom_path.m_alloc_list;
    rigid_body_constraint_custom_path** v46 = &v45[v34->m_list_rbc_custom_path.m_alloc_count];
    if (v46 != v45) {
        do {
            rigid_body_constraint_custom_path* v47 = *v45;
            rigid_body* v48 = v47->b1;
            if (v48 != NULL)
                ++v48->m_constraint_count;
            rigid_body* v49 = v47->b2;
            if (v49 != NULL)
                ++v49->m_constraint_count;
            ++v45;
        } while (v45 != v46);
        v34 = g_physics_system;
    }
    rigid_body_constraint_contact** v50 = v34->m_list_rbc_contact.m_alloc_list;
    rigid_body_constraint_contact** result =
        g_physics_system->m_list_rbc_contact.m_alloc_list;
    for (rigid_body_constraint_contact** m =
             &result[g_physics_system->m_list_rbc_contact.m_alloc_count];
         v50 != m; ++v50) {
        rigid_body_constraint_contact* v53 = *v50;
        contact_point_info* m_first = v53->m_list_contact_point_info_buffer_1.m_first;
        int n = 0;
        for (; m_first != NULL; m_first = m_first->m_next_link)
            n += m_first->m_point_pair_count;
        contact_point_info* v57 = v53->m_list_contact_point_info_buffer_2.m_first;
        int ii = 0;
        for (; v57 != NULL; v57 = v57->m_next_link)
            ii += v57->m_point_pair_count;
        rigid_body* v59 = v53->b1;
        if (ii <= n)
            ii = n;
        if (v59 != NULL)
            v59->m_contact_count += ii;
        rigid_body* rb2 = v53->b2;
        if (rb2 != NULL)
            rb2->m_contact_count += ii;
    }
    return result;
}

} // namespace phys_sys

// phys_mem_info ctor - ea: 0x87EB10 (non-inline, physics_system.o)
phys_mem_info::phys_mem_info() {
    m_num_rigid_body = 0;
    m_num_user_rigid_body = 0;
    m_contact_point_buffer_size = 0;
    m_num_rbc_point = 0;
    m_num_rbc_hinge = 0;
    m_num_rbc_dist = 0;
    m_num_rbc_ragdoll = 0;
    m_num_rbc_wheel = 0;
    m_num_rbc_angular_actuator = 0;
    m_num_rbc_custom_orientation = 0;
    m_num_rbc_custom_path = 0;
    m_num_rbc_contact = 0;
}
