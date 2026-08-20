// ============================================================================
// physics_system.h - phys_sys API (phys_xboxr:physics_system.o, 87 funcs).
// ============================================================================

#ifndef COD3_PHYSICS_PHYSICS_SYSTEM_H
#define COD3_PHYSICS_PHYSICS_SYSTEM_H

#include "physics/phys_types.h"

struct physics_system;

extern physics_system* g_physics_system;  // ?g_physics_system@@3PAVphysics_system@@A

enum phys_proftimer_e {
    phys_proftimer_phys_col_detect = 0,
    phys_proftimer_phys_misc = 1,
    phys_proftimer_phys_solver = 2,
};

struct phys_proftimer_callbacks {
    void (*proftimer_start)(phys_proftimer_e);  // +0x00
    void (*proftimer_stop)(phys_proftimer_e);   // +0x04

    phys_proftimer_callbacks();  // ea: 0x88B050
};
extern phys_proftimer_callbacks g_phys_proftimer_callbacks;

void PHYS_START_PROF_TIMER(phys_proftimer_e p);
void PHYS_STOP_PROF_TIMER(phys_proftimer_e p);

void IPN_init(rigid_body* rb_partition_head);
double IPN_get_max_delta_t(rigid_body* rb_partition_head);
void IPN_reset_rbc_lists(rigid_body* rb_partition_head);
void IPN_partition_process(const rigid_body_constraint* rbc);
rigid_body* IPN_get_partition(const rigid_body_constraint* rbc);

extern int g_physics_system_size;
extern int g_physics_system_alignment;

namespace phys_sys {

environment_rigid_body* get_environment_rigid_body();
void set_max_delta_t(float max_delta_t);
float get_max_delta_t();
void set_v_tol(int max_v_iters, int max_v_siters);
void get_v_tol(int* max_v_iters, int* max_v_siters);
void set_vp_tol(int max_vp_iters, int max_vp_siters);
void get_vp_tol(int* max_vp_iters, int* max_vp_siters);
void set_collision_callback(void (*collision_callback)());
void phys_frame_advance(float delta_t);
void phys_init(phys_mem_info* pmi);
void phys_shutdown();
void solver_memory_buffer_set(void* buffer, int buffer_size);
void solver_memory_buffer_nullify();
void set_phys_proftimer_callbacks(const phys_proftimer_callbacks* ppc);
rigid_body_constraint_contact* get_rbc_contact(rigid_body* const b1, rigid_body* const b2);

int max_slots_rigid_body();
int max_slots_user_rigid_body();
int max_slots_rbc_point();
int max_slots_rbc_hinge();
int max_slots_rbc_dist();
int max_slots_rbc_ragdoll();
int max_slots_rbc_wheel();
int max_slots_rbc_angular_actuator();
int max_slots_rbc_custom_orientation();
int max_slots_rbc_custom_path();
int max_slots_rbc_contact();
int available_slots_rigid_body();
int available_slots_user_rigid_body();
int available_slots_rbc_point();
int available_slots_rbc_hinge();
int available_slots_rbc_dist();
int available_slots_rbc_ragdoll();
int available_slots_rbc_wheel();
int available_slots_rbc_angular_actuator();
int available_slots_rbc_custom_orientation();
int available_slots_rbc_custom_path();
int available_slots_rbc_contact();
int used_slots_rigid_body();
int used_slots_user_rigid_body();
int used_slots_rbc_point();
int used_slots_rbc_hinge();
int used_slots_rbc_dist();
int used_slots_rbc_ragdoll();
int used_slots_rbc_wheel();
int used_slots_rbc_angular_actuator();
int used_slots_rbc_custom_orientation();
int used_slots_rbc_custom_path();
int used_slots_rbc_contact();

rigid_body* create_rigid_body(bool no_error);
user_rigid_body* create_user_rigid_body(bool no_error);
rigid_body_constraint_point* create_rbc_point(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error);
rigid_body_constraint_custom_orientation* create_rbc_custom_orientation(
    rigid_body* const b1, rigid_body* const b2, bool no_error);
rigid_body_constraint_hinge* create_rbc_hinge(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error);
rigid_body_constraint_distance* create_rbc_dist(rigid_body* const b1, rigid_body* const b2,
                                                bool no_error);
rigid_body_constraint_ragdoll* create_rbc_ragdoll(rigid_body* const b1, rigid_body* const b2,
                                                  bool no_error);
rigid_body_constraint_wheel* create_rbc_wheel(rigid_body* const b1, rigid_body* const b2,
                                              bool no_error);
rigid_body_constraint_angular_actuator* create_rbc_angular_actuator(
    rigid_body* const b1, rigid_body* const b2, bool no_error);
rigid_body_constraint_custom_path* create_rbc_custom_path(rigid_body* const b1,
                                                          rigid_body* const b2,
                                                          bool no_error);
rigid_body_constraint_contact* create_rbc_contact(rigid_body* const b1,
                                                  rigid_body* const b2,
                                                  bool no_error);
user_rigid_body* get_user_rigid_body(const math::Mat43* const dictactor);

void destroy(rigid_body_constraint_point* const rbc);
void destroy(rigid_body_constraint_hinge* const rbc);
void destroy(rigid_body_constraint_distance* const rbc);
void destroy(rigid_body_constraint_ragdoll* const rbc);
void destroy(rigid_body_constraint_wheel* const rbc);
void destroy(rigid_body_constraint_angular_actuator* const rbc);
void destroy(rigid_body_constraint_custom_orientation* const rbc);
void destroy(rigid_body_constraint_custom_path* const rbc);
void destroy(rigid_body_constraint_contact* const rbc);
void destroy(rigid_body* const rb);
void destroy(user_rigid_body* const rb);

void destroy_all_rigid_body();
void destroy_all_user_rigid_body();
void destroy_all_rbc_point();
void destroy_all_rbc_hinge();
void destroy_all_rbc_dist();
void destroy_all_rbc_ragdoll();
void destroy_all_rbc_wheel();
void destroy_all_rbc_angular_actuator();
void destroy_all_rbc_custom_orientation();
void destroy_all_rbc_custom_path();
void destroy_all_rbc_contact();
void destroy_all_constraint(rigid_body* const rb);
void destroy_all_constraint_with_user_rigid_body(rigid_body* const rb);
int destroy_all_unused_user_rigid_body();

} // namespace phys_sys

#endif // COD3_PHYSICS_PHYSICS_SYSTEM_H
