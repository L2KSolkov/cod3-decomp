// ============================================================================
// physics_system_internal.cpp - physics engine internals (14 non-inline funcs).
// Source: source/physics_system_internal.cpp (phys_xboxr:physics_system_internal.o)
// Verified against IDA (phys_xboxr:physics_system_internal.o).
// ============================================================================

#include "physics_system.h"
#include "pulse_sum.h"

#include <math.h>
#include <string.h>
#include <new>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlMemFree(void* Ptr);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void nuge_calc_velocities(const math::Mat43* mat0, const math::Mat43* mat1,
                                 float delta_t, math::Dir3* t_vel, math::Dir3* a_vel);
extern void tlWarning(const char* Format, ...);
extern const math::Dir3& Float4_Zero_212;

bool tlScratchpadLocked = false;  // ?tlScratchpadLocked@@3_NA (tl_system.o data)
phys_proftimer_callbacks g_phys_proftimer_callbacks;  // ?g_phys_proftimer_callbacks (phys_xboxr @ 0x14DBBF4)
const char* SOLVER_MEMORY_ALLOCATER_ERROR_MSG =
    "Solver memory allocater error";  // ?SOLVER_MEMORY_ALLOCATER_ERROR_MSG@@3PBDB (physics_system_internal.o)

// phys_constraint_solver_multithreaded.o (list_constraint_solver::process)
extern void list_constraint_solver_process(
    phys_constraint_solver_multithreaded_list_constraint_solver* lcs,
    physics_system* psys, int psys_next_psc_visit_counter);

namespace nuge {
void calc_velocities(const math::Mat43* mat0, const math::Mat43* mat1,
                     float delta_t, math::Dir3* t_vel, math::Dir3* a_vel);
}

// calc_col_mat (user_rigid_body) - ea: 0x88E9F0
void rbint::calc_col_mat(user_rigid_body* rb, const outer_time* outside_delta_t) {
    if ((rb->m_flags & 0x20) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 128,
                  "rb->is_user_rigid_body()", ""))
        __debugbreak();
    if (rb->m_dictator == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 129,
                  "rb->m_dictator", ""))
        __debugbreak();
    float m_time = rb->m_time_scale.m_time * outside_delta_t->m_time;
    math::Dir3 v4;
    v4.v = rb->m_t_vel.v;
    math::Mat43 v20;
    v20.w.v = _mm_add_ps(rb->m_mat.w.v, _mm_mul_ps(v4.v, _mm_shuffle_ps(_mm_set_ss(m_time), _mm_set_ss(m_time), 0)));
    rb->m_col_mat.w = v20.w;
    make_rotate(&v20, rb->m_a_vel, m_time);
    __m128 v8 = v20.y.v;
    __m128 v9 = v20.x.v;
    math::Dir3 v10;
    v10.v = rb->m_mat.y.v;
    v20.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 0), v20.x.v),
            _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 85), v8)),
        _mm_mul_ps(_mm_shuffle_ps(rb->m_mat.x.v, rb->m_mat.x.v, 170), v20.z.v));
    __m128 v11 = _mm_shuffle_ps(v10.v, v10.v, 0);
    __m128 v12 = _mm_mul_ps(_mm_shuffle_ps(v10.v, v10.v, 170), v20.z.v);
    __m128 v13 = _mm_shuffle_ps(v10.v, v10.v, 85);
    math::Dir3 v14;
    v14.v = rb->m_mat.z.v;
    __m128 v15 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(v11, v9), _mm_mul_ps(v13, v8)), v12);
    __m128 v16 = _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 170), v20.z.v);
    rb->m_col_mat.x = v20.y;
    v20.z.v = v15;
    v20.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 0), v9),
            _mm_mul_ps(_mm_shuffle_ps(v14.v, v14.v, 85), v8)),
        v16);
    rb->m_col_mat.y = v20.z;
    rb->m_col_mat.z.v = v20.w.v;
}

// ============================================================================
// Data (physics_system_internal.o)
// ============================================================================
int g_physics_system_size = 0;          // ?g_physics_system_size@@3HA
int g_physics_system_alignment = 0;     // ?g_physics_system_alignment@@3HA

// TODO: shared with physics_system.o (defined here; owns the symbol).
physics_system* g_physics_system = NULL;

// ============================================================================
// verify_time_scale - ea: 0x88B5B0
// ============================================================================
void verify_time_scale(rigid_body_constraint* rbc, const outer_time* time_scale) {
    rigid_body* b1 = rbc->b1;
    if (b1 != NULL && (b1->m_flags & 0x10) == 0 &&
        fabs(b1->m_time_scale.m_time - time_scale->m_time) >= 0.001 &&
        _tlAssert("source/physics_system_internal.cpp", 199,
                  "fabsf(time_sub(rbc->get_b1()->get_time_scale(),time_scale)) < 0.001f", ""))
        __debugbreak();
    rigid_body* b2 = rbc->b2;
    if (b2 != NULL && (b2->m_flags & 0x10) == 0 &&
        fabs(b2->m_time_scale.m_time - time_scale->m_time) >= 0.001 &&
        _tlAssert("source/physics_system_internal.cpp", 203,
                  "fabsf(time_sub(rbc->get_b2()->get_time_scale(),time_scale)) < 0.001f", ""))
        __debugbreak();
}

// ============================================================================
// IPN_merge - ea: 0x88B650
// ============================================================================
void IPN_merge(rigid_body* dest, rigid_body* source) {
    if (dest->m_partition_node.m_partition_head != dest &&
        _tlAssert("source/physics_system_internal.cpp", 247,
                  "GIPN(dest)->m_partition_head == dest", ""))
        __debugbreak();
    if (source->m_partition_node.m_partition_head != source &&
        _tlAssert("source/physics_system_internal.cpp", 248,
                  "GIPN(source)->m_partition_head == source", ""))
        __debugbreak();
    if (dest == source &&
        _tlAssert("source/physics_system_internal.cpp", 249,
                  "dest != source", ""))
        __debugbreak();
    dest->m_partition_node.m_partition_tail->m_partition_node.m_next_node = source;
    int m_partition_size = dest->m_partition_node.m_partition_size;
    dest->m_partition_node.m_partition_tail = source->m_partition_node.m_partition_tail;
    dest->m_partition_node.m_partition_size =
        source->m_partition_node.m_partition_size + m_partition_size;
    rigid_body* m_next_node = source;
    do {
        m_next_node->m_partition_node.m_partition_head =
            dest->m_partition_node.m_partition_head;
        m_next_node = m_next_node->m_partition_node.m_next_node;
    } while (m_next_node != NULL);
    source->m_partition_node.m_partition_tail = NULL;
    source->m_partition_node.m_partition_size = 0;
}

// ============================================================================
// IPN_verify_time_scale - ea: 0x88B730
// ============================================================================
void IPN_verify_time_scale(rigid_body* rb_partition_head) {
    if (rb_partition_head->m_partition_node.m_partition_head != rb_partition_head &&
        _tlAssert("source/physics_system_internal.cpp", 209,
                  "GIPN(rb_partition_head)->m_partition_head == rb_partition_head", ""))
        __debugbreak();
    for (rigid_body_constraint_point* i =
             rb_partition_head->m_partition_node.m_rbc_point_first;
         i != NULL; i = (rigid_body_constraint_point*)i->m_next)
        verify_time_scale(i, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_hinge* j =
             rb_partition_head->m_partition_node.m_rbc_hinge_first;
         j != NULL; j = (rigid_body_constraint_hinge*)j->m_next)
        verify_time_scale(j, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_distance* k =
             rb_partition_head->m_partition_node.m_rbc_dist_first;
         k != NULL; k = (rigid_body_constraint_distance*)k->m_next)
        verify_time_scale(k, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_ragdoll* m =
             rb_partition_head->m_partition_node.m_rbc_ragdoll_first;
         m != NULL; m = (rigid_body_constraint_ragdoll*)m->m_next)
        verify_time_scale(m, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_wheel* n =
             rb_partition_head->m_partition_node.m_rbc_wheel_first;
         n != NULL; n = (rigid_body_constraint_wheel*)n->m_next)
        verify_time_scale(n, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_angular_actuator* ii =
             rb_partition_head->m_partition_node.m_rbc_angular_actuator_first;
         ii != NULL; ii = (rigid_body_constraint_angular_actuator*)ii->m_next)
        verify_time_scale(ii, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_custom_orientation* jj =
             rb_partition_head->m_partition_node.m_rbc_custom_orientation_first;
         jj != NULL; jj = (rigid_body_constraint_custom_orientation*)jj->m_next)
        verify_time_scale(jj, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_custom_path* kk =
             rb_partition_head->m_partition_node.m_rbc_custom_path_first;
         kk != NULL; kk = (rigid_body_constraint_custom_path*)kk->m_next)
        verify_time_scale(kk, &rb_partition_head->m_time_scale);
    for (rigid_body_constraint_contact* mm =
             rb_partition_head->m_partition_node.m_rbc_contact_first;
         mm != NULL; mm = (rigid_body_constraint_contact*)mm->m_next)
        verify_time_scale(mm, &rb_partition_head->m_time_scale);
}

// ============================================================================
// physics_system::get_alignment / get_buffer_size - ea: 0x88D550 / 0x88D560
// ============================================================================
int physics_system::get_alignment() {
    return 16;
}

unsigned int physics_system::get_buffer_size(const phys_mem_info* pmi) {
    return (((((((((((((((((((440 * pmi->m_num_rigid_body +
                               ((456 * pmi->m_num_user_rigid_body + 879) & 0xFFFFFFF0) + 3)
                              & 0xFFFFFFFC)
                             + 2 * (pmi->m_contact_point_buffer_size +
                                    26 * pmi->m_num_rbc_contact)
                             + 15)
                            & 0xFFFFFFF0)
                           + 88 * pmi->m_num_rbc_point + 15)
                          & 0xFFFFFFF0)
                         + 248 * pmi->m_num_rbc_hinge + 15)
                        & 0xFFFFFFF0)
                       + 104 * pmi->m_num_rbc_dist + 3)
                      & 0xFFFFFFFC)
                     + (pmi->m_num_rbc_custom_orientation << 6) + 15)
                    & 0xFFFFFFF0)
                   + 136 * pmi->m_num_rbc_custom_path + 15)
                  & 0xFFFFFFF0)
                 + 344 * pmi->m_num_rbc_ragdoll + 15)
                & 0xFFFFFFF0)
               + 232 * pmi->m_num_rbc_wheel + 15)
              & 0xFFFFFFF0)
             + 216 * pmi->m_num_rbc_angular_actuator + 15)
            & 0xFFFFFFF0;
}

// ============================================================================
// physics_system::physics_system - ea: 0x88DA40
// ============================================================================
physics_system::physics_system() {
    m_search_tree_rbc_contact.m_tree_root = NULL;
    m_list_user_rigid_body.m_slot_array = NULL;
    m_list_user_rigid_body.m_alloc_list = NULL;
    m_list_user_rigid_body.m_index_array = NULL;
    m_list_user_rigid_body.m_slot_array_size = 0;
    m_list_user_rigid_body.m_alloc_count = 0;
    m_list_rigid_body.m_slot_array = NULL;
    m_list_rigid_body.m_alloc_list = NULL;
    m_list_rigid_body.m_index_array = NULL;
    m_list_rigid_body.m_slot_array_size = 0;
    m_list_rigid_body.m_alloc_count = 0;
    m_list_rbc_contact.m_slot_array = NULL;
    m_list_rbc_contact.m_alloc_list = NULL;
    m_list_rbc_contact.m_index_array = NULL;
    m_list_rbc_contact.m_slot_array_size = 0;
    m_list_rbc_contact.m_alloc_count = 0;
    m_contact_point_buffer_1.m_buffer_start = NULL;
    m_contact_point_buffer_1.m_buffer_end = NULL;
    m_contact_point_buffer_1.m_buffer_cur = NULL;
    m_contact_point_buffer_1.m_user_start = NULL;
    m_contact_point_buffer_2.m_buffer_start = NULL;
    m_contact_point_buffer_2.m_buffer_end = NULL;
    m_contact_point_buffer_2.m_buffer_cur = NULL;
    m_contact_point_buffer_2.m_user_start = NULL;
    m_list_rbc_point.m_slot_array = NULL;
    m_list_rbc_point.m_alloc_list = NULL;
    m_list_rbc_point.m_index_array = NULL;
    m_list_rbc_point.m_slot_array_size = 0;
    m_list_rbc_point.m_alloc_count = 0;
    m_list_rbc_hinge.m_slot_array = NULL;
    m_list_rbc_hinge.m_alloc_list = NULL;
    m_list_rbc_hinge.m_index_array = NULL;
    m_list_rbc_hinge.m_slot_array_size = 0;
    m_list_rbc_hinge.m_alloc_count = 0;
    m_list_rbc_dist.m_slot_array = NULL;
    m_list_rbc_dist.m_alloc_list = NULL;
    m_list_rbc_dist.m_index_array = NULL;
    m_list_rbc_dist.m_slot_array_size = 0;
    m_list_rbc_dist.m_alloc_count = 0;
    m_list_rbc_custom_orientation.m_slot_array = NULL;
    m_list_rbc_custom_orientation.m_alloc_list = NULL;
    m_list_rbc_custom_orientation.m_index_array = NULL;
    m_list_rbc_custom_orientation.m_slot_array_size = 0;
    m_list_rbc_custom_orientation.m_alloc_count = 0;
    m_list_rbc_custom_path.m_slot_array = NULL;
    m_list_rbc_custom_path.m_alloc_list = NULL;
    m_list_rbc_custom_path.m_index_array = NULL;
    m_list_rbc_custom_path.m_slot_array_size = 0;
    m_list_rbc_custom_path.m_alloc_count = 0;
    m_list_rbc_ragdoll.m_slot_array = NULL;
    m_list_rbc_ragdoll.m_alloc_list = NULL;
    m_list_rbc_ragdoll.m_index_array = NULL;
    m_list_rbc_ragdoll.m_slot_array_size = 0;
    m_list_rbc_ragdoll.m_alloc_count = 0;
    m_list_rbc_wheel.m_slot_array = NULL;
    m_list_rbc_wheel.m_alloc_list = NULL;
    m_list_rbc_wheel.m_index_array = NULL;
    m_list_rbc_wheel.m_slot_array_size = 0;
    m_list_rbc_wheel.m_alloc_count = 0;
    m_list_rbc_angular_actuator.m_slot_array = NULL;
    m_list_rbc_angular_actuator.m_alloc_list = NULL;
    m_list_rbc_angular_actuator.m_index_array = NULL;
    m_list_rbc_angular_actuator.m_slot_array_size = 0;
    m_list_rbc_angular_actuator.m_alloc_count = 0;
    m_constraint_solver.m_solver_memory_allocater.m_buffer_start = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_buffer_end = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_buffer_cur = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_user_start = NULL;
    m_constraint_solver.m_list_pulse_sum_node.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_node.m_last = NULL;
    m_constraint_solver.m_list_pulse_sum_normal.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_normal.m_last = NULL;
    m_constraint_solver.m_list_pulse_sum_point.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_point.m_last = NULL;
    m_constraint_solver.m_list_pulse_sum_angular.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_angular.m_last = NULL;
    m_constraint_solver.m_list_pulse_sum_wheel.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_wheel.m_last = NULL;
    m_constraint_solver.m_list_pulse_sum_contact.m_first = NULL;
    m_constraint_solver.m_list_pulse_sum_contact.m_last = NULL;
    m_outside_sub_delta_t = 0.0f;
    m_flags = 0;
    m_psc_visit_counter = 0;
    m_collision_callback = NULL;
    m_max_delta_t = 0.051282052f;
    m_max_vel_iters = 4;
    m_max_vel_siters = 4;
    m_max_vel_pos_iters = 4;
    m_max_vel_pos_siters = 8;
    m_environment_rigid_body.set();
    m_solver_memory_high = 0;
}

// ============================================================================
// physics_system::allocate_buffer - ea: 0x88DC70
// ============================================================================
physics_system* physics_system::allocate_buffer(const phys_mem_info* pmi,
                                                phys_memory_heap* allocater) {
    char* v3 = (char*)(((intptr_t)allocater->m_buffer_cur + 15) & 0xFFFFFFF0);
    physics_system* v4;
    if ((v3 + 864) > allocater->m_buffer_end) {
        v4 = NULL;
    } else {
        allocater->m_buffer_cur = v3 + 864;
        v4 = (physics_system*)v3;
        if (v3 != NULL)
            goto label6;
    }
    if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 89,
                  "addr", "phys_memory_heap overflow."))
        __debugbreak();
    if (v4 == NULL)
        return NULL;
label6:
    physics_system* v5 = ::new ((void*)v4) physics_system();
    v5->m_list_user_rigid_body.allocate_buffer(pmi->m_num_user_rigid_body, allocater);
    v5->m_list_rigid_body.allocate_buffer(pmi->m_num_rigid_body, allocater);
    v5->m_list_rbc_contact.allocate_buffer(pmi->m_num_rbc_contact, allocater);
    int allocatera = pmi->m_contact_point_buffer_size;
    char* pmia = allocater->allocate_no_error(allocatera, 1);
    if (pmia == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 89,
                  "addr", "phys_memory_heap overflow."))
        __debugbreak();
    v5->m_contact_point_buffer_1.set_buffer(pmia, allocatera, 1);
    int allocaterb = pmi->m_contact_point_buffer_size;
    char* pmib = allocater->allocate_no_error(allocaterb, 1);
    if (pmib == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 89,
                  "addr", "phys_memory_heap overflow."))
        __debugbreak();
    v5->m_contact_point_buffer_2.set_buffer(pmib, allocaterb, 1);
    v5->m_list_rbc_point.allocate_buffer(pmi->m_num_rbc_point, allocater);
    v5->m_list_rbc_hinge.allocate_buffer(pmi->m_num_rbc_hinge, allocater);
    v5->m_list_rbc_dist.allocate_buffer(pmi->m_num_rbc_dist, allocater);
    v5->m_list_rbc_custom_orientation.allocate_buffer(
        pmi->m_num_rbc_custom_orientation, allocater);
    v5->m_list_rbc_custom_path.allocate_buffer(pmi->m_num_rbc_custom_path, allocater);
    v5->m_list_rbc_ragdoll.allocate_buffer(pmi->m_num_rbc_ragdoll, allocater);
    v5->m_list_rbc_wheel.allocate_buffer(pmi->m_num_rbc_wheel, allocater);
    v5->m_list_rbc_angular_actuator.allocate_buffer(
        pmi->m_num_rbc_angular_actuator, allocater);
    return v5;
}

// ============================================================================
// physics_system::create_inst / destroy_inst / free_buffer
// ============================================================================
void physics_system::create_inst(phys_mem_info* pmi) {
    g_physics_system_size = physics_system::get_buffer_size(pmi);
    g_physics_system_alignment = 16;
    void* v1 = tlMemAlloc(g_physics_system_size, 0x10u, 0);
    phys_memory_heap memory_heap;
    memset(&memory_heap, 0, sizeof(memory_heap));
    memory_heap.set_buffer((char*)v1, g_physics_system_size, g_physics_system_alignment);
    g_physics_system = physics_system::allocate_buffer(pmi, &memory_heap);
    if (v1 != g_physics_system &&
        _tlAssert("source/physics_system_internal.cpp", 685,
                  "addr == g_physics_system", ""))
        __debugbreak();
}

void physics_system::destroy_inst() {
    g_physics_system->~physics_system();
    tlMemFree(g_physics_system);
    g_physics_system = NULL;
    g_physics_system_size = 0;
    g_physics_system_alignment = 0;
}

void physics_system::free_buffer(physics_system* psys) {
    psys->~physics_system();
}

// ============================================================================
// physics_system::frame_advance - ea: 0x88DEC0
// ============================================================================
void physics_system::frame_advance(float delta_t) {
    int sub_delta_t = (int)(delta_t / m_max_delta_t);
    if (sub_delta_t < 1)
        sub_delta_t = 1;
    float v5 = (float)sub_delta_t;
    if ((delta_t / (float)sub_delta_t) > m_max_delta_t)
        v5 = (float)++sub_delta_t;

    outer_time sub_delta;
    sub_delta.m_time = delta_t / v5;
    outer_time full_delta;
    full_delta.m_time = delta_t;

    rigid_body** rb_list = m_list_rigid_body.m_alloc_list;
    rigid_body** rb_end = &rb_list[m_list_rigid_body.m_alloc_count];
    if (rb_end != rb_list) {
        do
            rbint::prolog_frame_advance(*rb_list++, &full_delta);
        while (rb_list != rb_end);
    }

    user_rigid_body** urb_list = m_list_user_rigid_body.m_alloc_list;
    user_rigid_body** urb_end = &urb_list[m_list_user_rigid_body.m_alloc_count];
    if (urb_end != urb_list) {
        do
            rbint::prolog_frame_advance(*urb_list++, &full_delta);
        while (urb_list != urb_end);
    }

    rigid_body_constraint_distance** dist_list = m_list_rbc_dist.m_alloc_list;
    rigid_body_constraint_distance** dist_end =
        &dist_list[m_list_rbc_dist.m_alloc_count];
    for (; dist_list != dist_end; ++dist_list)
        (*dist_list)->outer_prolog_update(&full_delta);

    rigid_body_constraint_angular_actuator** aa_list =
        m_list_rbc_angular_actuator.m_alloc_list;
    rigid_body_constraint_angular_actuator** aa_end =
        &aa_list[m_list_rbc_angular_actuator.m_alloc_count];
    for (; aa_list != aa_end; ++aa_list)
        (*aa_list)->outer_prolog_update(&full_delta);

    int v14 = 0;
    int v15 = 0;
    if (sub_delta_t > 0) {
        do {
            v15 = v14 + 1;
            time_step(sub_delta.m_time, v15 == sub_delta_t);
            v14 = v15;
        } while (v15 < sub_delta_t);
    }

    rigid_body** v16 = m_list_rigid_body.m_alloc_list;
    rigid_body** v16_end = &v16[m_list_rigid_body.m_alloc_count];
    for (; v16 != v16_end; ++v16) {
        (*v16)->m_force_sum.v = Float4_Zero_212.v;
        (*v16)->m_torque_sum.v = Float4_Zero_212.v;
    }

    rigid_body_constraint_distance** dist2 = m_list_rbc_dist.m_alloc_list;
    rigid_body_constraint_distance** dist2_end =
        &dist2[m_list_rbc_dist.m_alloc_count];
    for (; dist2 != dist2_end; ++dist2)
        (*dist2)->outer_epilog_update(&full_delta);

    rigid_body_constraint_angular_actuator** aa2 =
        m_list_rbc_angular_actuator.m_alloc_list;
    rigid_body_constraint_angular_actuator** aa2_end =
        &aa2[m_list_rbc_angular_actuator.m_alloc_count];
    for (; aa2 != aa2_end; ++aa2)
        (*aa2)->outer_epilog_update(&full_delta);
}

// ============================================================================
// physics_system::time_step - ea: 0x88D620
// ============================================================================
void physics_system::time_step(float outside_delta_t, bool last_step) {
    m_outside_sub_delta_t = outside_delta_t;
    if (g_phys_proftimer_callbacks.proftimer_start != NULL)
        g_phys_proftimer_callbacks.proftimer_start(phys_proftimer_phys_col_detect);

    rigid_body** m_alloc_list = m_list_rigid_body.m_alloc_list;
    for (rigid_body** i = &m_alloc_list[m_list_rigid_body.m_alloc_count];
         m_alloc_list != i; (*m_alloc_list++)->m_flags = (*(m_alloc_list - 1))->m_flags | 0x40)
        ;
    user_rigid_body** v8 = m_list_user_rigid_body.m_alloc_list;
    for (user_rigid_body** j = &v8[m_list_user_rigid_body.m_alloc_count];
         v8 != j; (*v8++)->m_flags = (*(v8 - 1))->m_flags | 0x40)
        ;

    rigid_body** v12 = m_list_rigid_body.m_alloc_list;
    m_flags |= 1u;
    rigid_body** v13 = &v12[m_list_rigid_body.m_alloc_count];
    if (v13 != v12) {
        outer_time t;
        t.m_time = outside_delta_t;
        do
            rbint::calc_col_mat(*v12++, &t);
        while (v12 != v13);
    }
    user_rigid_body** v14 = m_list_user_rigid_body.m_alloc_list;
    user_rigid_body** v15 = &v14[m_list_user_rigid_body.m_alloc_count];
    if (v15 != v14) {
        outer_time t;
        t.m_time = outside_delta_t;
        do
            rbint::calc_col_mat(*v14++, &t);
        while (v14 != v15);
    }

    if (m_collision_callback != NULL)
        m_collision_callback();

    rigid_body** v17 = m_list_rigid_body.m_alloc_list;
    m_flags &= ~1u;
    for (rigid_body** k = &v17[m_list_rigid_body.m_alloc_count]; v17 != k; ++v17)
        (*v17)->m_flags &= ~0x40u;
    user_rigid_body** v19 = m_list_user_rigid_body.m_alloc_list;
    for (user_rigid_body** m = &v19[m_list_user_rigid_body.m_alloc_count];
         v19 != m; ++v19)
        (*v19)->m_flags &= ~0x40u;

    if (g_phys_proftimer_callbacks.proftimer_stop != NULL)
        g_phys_proftimer_callbacks.proftimer_stop(phys_proftimer_phys_col_detect);
    if (g_phys_proftimer_callbacks.proftimer_start != NULL)
        g_phys_proftimer_callbacks.proftimer_start(phys_proftimer_phys_misc);

    solver_priority_sort();
    phys_constraint_solver_multithreaded_list_constraint_solver list_cs;
    list_cs.m_constraint_solver = &m_constraint_solver;
    m_constraint_solver.m_first_partition_head = NULL;
    int next_psc_visit_counter = 0;
    generate_partitions_and_stuff(&list_cs, &next_psc_visit_counter, outside_delta_t);

    if (g_phys_proftimer_callbacks.proftimer_stop != NULL)
        g_phys_proftimer_callbacks.proftimer_stop(phys_proftimer_phys_misc);
    if (g_phys_proftimer_callbacks.proftimer_start != NULL)
        g_phys_proftimer_callbacks.proftimer_start(phys_proftimer_phys_solver);

    if (tlScratchpadLocked &&
        _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 294,
                  "!tlScratchpadLocked", "Scratchpad is already locked!"))
        __debugbreak();
    tlScratchpadLocked = true;
    int v21 = m_psc_visit_counter + next_psc_visit_counter;
    list_constraint_solver_process(&list_cs, this, v21);
    m_psc_visit_counter = v21;
    if (!tlScratchpadLocked &&
        _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 300,
                  "tlScratchpadLocked", "Scratchpad is already unlocked!"))
        __debugbreak();
    tlScratchpadLocked = false;

    if (g_phys_proftimer_callbacks.proftimer_stop != NULL)
        g_phys_proftimer_callbacks.proftimer_stop(phys_proftimer_phys_solver);

    user_rigid_body** v22 = m_list_user_rigid_body.m_alloc_list;
    if (last_step) {
        for (user_rigid_body** n = &v22[m_list_user_rigid_body.m_alloc_count];
             v22 != n; ++v22)
            rbint::take_last_step(*v22, (const outer_time*)&last_step);
    } else {
        user_rigid_body** ii =
            &m_list_user_rigid_body.m_alloc_list[m_list_user_rigid_body.m_alloc_count];
        for (; v22 != ii; ++v22)
            rbint::take_next_step(*v22, (const outer_time*)&last_step);
    }

    if (g_phys_proftimer_callbacks.proftimer_start != NULL)
        g_phys_proftimer_callbacks.proftimer_start(phys_proftimer_phys_misc);

    // Swap the two contact-point buffers (double buffering).
    char* temp_buffer = m_contact_point_buffer_1.m_buffer_start;
    char* m_buffer_end = m_contact_point_buffer_1.m_buffer_end;
    char* m_user_start = m_contact_point_buffer_1.m_user_start;
    char* m_buffer_cur = m_contact_point_buffer_1.m_buffer_cur;
    m_contact_point_buffer_1.m_buffer_start = m_contact_point_buffer_2.m_buffer_start;
    m_contact_point_buffer_1.m_buffer_end = m_contact_point_buffer_2.m_buffer_end;
    m_contact_point_buffer_1.m_buffer_cur = m_contact_point_buffer_2.m_buffer_cur;
    m_contact_point_buffer_1.m_user_start = m_contact_point_buffer_2.m_user_start;
    m_contact_point_buffer_2.m_buffer_start = temp_buffer;
    m_contact_point_buffer_2.m_buffer_end = m_buffer_end;
    m_contact_point_buffer_2.m_buffer_cur = m_buffer_cur;
    m_contact_point_buffer_2.m_user_start = m_user_start;
    m_contact_point_buffer_1.m_buffer_cur = m_contact_point_buffer_1.m_buffer_start;

    rigid_body_constraint_contact** v26 = m_list_rbc_contact.m_alloc_list;
    phys_heap_memory_pool<rigid_body_constraint_contact>* p_m_list_rbc_contact =
        &m_list_rbc_contact;
    if (&v26[m_list_rbc_contact.m_alloc_count] != v26) {
        do {
            rigid_body_constraint_contact* v28 = *v26;
            contact_point_info* m_first = v28->m_list_contact_point_info_buffer_1.m_first;
            contact_point_info* v30 = m_first;
            int v31 = 0;
            if (m_first == NULL)
                goto label_remove;
            do {
                v31 += v30->m_point_pair_count;
                v30 = v30->m_next_link;
            } while (v30 != NULL);
            if (v31 != 0) {
                ++v26;
                int v32 = 0;
                v28->m_list_contact_point_info_buffer_2.m_first = m_first;
                v28->m_list_contact_point_info_buffer_1.m_first = NULL;
                contact_point_info* v33 = m_first;
                if (m_first == NULL)
                    goto label_assert_cached;
                do {
                    v32 += v33->m_point_pair_count;
                    v33 = v33->m_next_link;
                } while (v33 != NULL);
                if (v32 <= 0) {
label_assert_cached:
                    if (_tlAssert("source/physics_system_internal.cpp", 546,
                                  "rbc.get_cached_point_count() > 0", ""))
                        __debugbreak();
                }
                contact_point_info* v35 = v28->m_list_contact_point_info_buffer_1.m_first;
                int v36 = 0;
                if (v35 != NULL) {
                    do {
                        v36 += v35->m_point_pair_count;
                        v35 = v35->m_next_link;
                    } while (v35 != NULL);
                    if (v36 != 0 &&
                        _tlAssert("source/physics_system_internal.cpp", 547,
                                  "rbc.get_point_count() == 0", ""))
                        __debugbreak();
                }
            } else {
label_remove:
                p_m_list_rbc_contact->remove(*v26);
            }
        } while (&p_m_list_rbc_contact
                     ->m_alloc_list[p_m_list_rbc_contact->m_alloc_count] !=
                 v26);
    }

    if (g_phys_proftimer_callbacks.proftimer_stop != NULL)
        g_phys_proftimer_callbacks.proftimer_stop(phys_proftimer_phys_misc);
}

// ============================================================================
// physics_system::~physics_system - ea: 0x890130
// ============================================================================
physics_system::~physics_system() {
    m_list_user_rigid_body.~phys_heap_memory_pool();
    m_list_rigid_body.~phys_heap_memory_pool();
    m_list_rbc_contact.~phys_heap_memory_pool();
    m_list_rbc_point.~phys_heap_memory_pool();
    m_list_rbc_hinge.~phys_heap_memory_pool();
    m_list_rbc_dist.~phys_heap_memory_pool();
    m_list_rbc_custom_orientation.~phys_heap_memory_pool();
    m_list_rbc_custom_path.~phys_heap_memory_pool();
    m_list_rbc_ragdoll.~phys_heap_memory_pool();
    m_list_rbc_wheel.~phys_heap_memory_pool();
    m_list_rbc_angular_actuator.~phys_heap_memory_pool();
}

// ============================================================================
// physics_system::solver_priority_sort - ea: 0x88F550
// (TODO: full sort reconstruction deferred; contacts are sorted by priority
// via bubble sort in the original.)
// ============================================================================
void physics_system::solver_priority_sort() {
    // Insertion-style sort of the contact list by m_solver_priority (descending).
    phys_heap_memory_pool<rigid_body_constraint_contact>* pool = &m_list_rbc_contact;
    for (int i = 1; i < pool->m_alloc_count; ++i) {
        rigid_body_constraint_contact* key = pool->m_alloc_list[i];
        int j = i - 1;
        while (j >= 0 && pool->m_alloc_list[j]->m_solver_priority <
                             key->m_solver_priority) {
            pool->m_alloc_list[j + 1] = pool->m_alloc_list[j];
            --j;
        }
        pool->m_alloc_list[j + 1] = key;
    }
    pool->calc_index_array();
}

// ============================================================================
// TODO stubs (deferred from this unit; heavy reconstruction targets)
// ============================================================================
void physics_system::generate_partitions_and_stuff(
    phys_constraint_solver_multithreaded_list_constraint_solver* list_cs,
    int* next_psc_visit_counter, float delta_t) {
    // ea: 0x88C830 - TODO: full partition builder (37KB function).
    (void)list_cs;
    (void)next_psc_visit_counter;
    (void)delta_t;
}

// IPN_verify_rigid_bodies - ea: 0x88B890 (debug verifier, 45KB function).
void IPN_verify_rigid_bodies(rigid_body* rb_partition_head) {
    (void)rb_partition_head;
}
