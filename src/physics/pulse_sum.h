// ============================================================================
// pulse_sum.h — pulse-sum constraint solver types.
// Source: c:\cod\code\tl\physics\include\pulse_sum.h
// Verified against IDA local types.
// ============================================================================
#ifndef COD3_PHYSICS_PULSE_SUM_H
#define COD3_PHYSICS_PULSE_SUM_H

#include "physics/phys_types.h"
#include <intrin.h>
#include <math.h>
#include <new>

extern const math::Dir3& Float4_Zero_212;
extern const math::Dir3& Float4_Two_212;
extern const math::Dir3& Float4_XAxis_214;
extern const math::Dir3& Float4_YAxis_214;
extern const math::Dir3& Float4_ZAxis_214;

// ============================================================================
// Matrix helpers (inline COMDATs, render.o / rbc_def_ragdoll.o).
//   SetIdentity - ea: 0x6EB220
//   make_rotate(Mat43&, Dir3 const&, float, float) - ea: 0x6EB2F0
//   make_rotate(Mat43*, Dir3 const&, float) - ea: 0x6EB560
//   make_rotate(Mat43*, Dir3 const&, Dir3 const&) - ea: 0x88A4B0
// ============================================================================
inline void SetIdentity(math::Mat43& m) {
    m.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    m.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 0.0f);
}

inline void make_rotate(math::Mat43& m, const math::Dir3& u, float ca, float sa) {
    const float ux = u.v.m128_f32[0];
    const float uy = u.v.m128_f32[1];
    const float uz = u.v.m128_f32[2];
    const float omc = 1.0f - ca;
    const float ux_sa = ux * sa;
    const float uy_ux_omc = (uy * ux) * omc;
    const float uz_ux_omc = (uz * ux) * omc;
    const float uz_uy_omc = (uz * uy) * omc;
    m.x.v = _mm_setr_ps((ux * ux) * omc + ca, uz * sa + uy_ux_omc,
                        uz_ux_omc - uy * sa, 0.0f);
    m.y.v = _mm_setr_ps(uy_ux_omc - uz * sa, (uy * uy) * omc + ca,
                        ux_sa + uz_uy_omc, 0.0f);
    m.z.v = _mm_setr_ps(uy * sa + uz_ux_omc, uz_uy_omc - ux_sa,
                        (uz * uz) * omc + ca, 0.0f);
}

inline void make_rotate(math::Mat43* mat, const math::Dir3& v, float theta_factor) {
    const __m128 vv = v.v;
    const __m128 vv2 = _mm_mul_ps(vv, vv);
    const float len = sqrt(vv2.m128_f32[0]
                           + (_mm_shuffle_ps(vv2, vv2, 85).m128_f32[0]
                              + _mm_shuffle_ps(vv2, vv2, 170).m128_f32[0]));
    if (len >= 1e-5f) {
        const float angle = len * theta_factor;
        math::Dir3 axis;
        axis.v = _mm_mul_ps(vv, _mm_set_ps1(1.0f / len));
        const float sa = sin(angle);
        const float ca = cos(angle);
        make_rotate(*mat, axis, ca, sa);
    } else {
        SetIdentity(*mat);
    }
}

inline void make_rotate(math::Mat43* mat, const math::Dir3& v1,
                        const math::Dir3& v2) {
    const __m128 cross = _mm_sub_ps(
        _mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 9), _mm_shuffle_ps(v2.v, v2.v, 18)),
        _mm_mul_ps(_mm_shuffle_ps(v1.v, v1.v, 18), _mm_shuffle_ps(v2.v, v2.v, 9)));
    const __m128 cross2 = _mm_mul_ps(cross, cross);
    const float len_sq = cross2.m128_f32[0]
                         + (_mm_shuffle_ps(cross2, cross2, 85).m128_f32[0]
                            + _mm_shuffle_ps(cross2, cross2, 170).m128_f32[0]);
    if (len_sq >= 1e-5f) {
        const __m128 dotv = _mm_mul_ps(v1.v, v2.v);
        const float len = sqrt(len_sq);
        const float inv_len = 1.0f / len;
        math::Dir3 axis;
        axis.v = _mm_mul_ps(cross, _mm_set_ps1(inv_len));
        const float dot = dotv.m128_f32[0]
                          + (_mm_shuffle_ps(dotv, dotv, 85).m128_f32[0]
                             + _mm_shuffle_ps(dotv, dotv, 170).m128_f32[0]);
        const float len2 = sqrt(dot * dot + len * len);
        const float inv_len2 = 1.0f / len2;
        make_rotate(*mat, axis, inv_len2 * dot, inv_len2 * len);
    } else {
        SetIdentity(*mat);
    }
}

extern void orthonormalize(math::Mat43* mat);
extern const char* SOLVER_MEMORY_ALLOCATER_ERROR_MSG;

namespace nuge {
void calc_velocities(const math::Mat43* mat0, const math::Mat43* mat1, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel);
}

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
    void  setup_vel_uni_standard(float delta_t, float max_penalty_restitution_vel);

    void set(rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2,
             const math::Dir3* b2_r, const math::Dir3* ud, pulse_sum_cache* ps_cache);
    float get_vel();
    float get_pos();
    float get_objective();
    float clamp_pulse_sum(float ps);
    void  apply(const float* s_);
    void  calc_abs();
    void  project();
    void  SOLVER_apply_relaxation(float* error_sq);
    void  SOLVER_solver_prolog(int iter, float delta_t);
    void  SOLVER_solver_intermediate(int iter, float delta_t);
    void  set_object_vel(const math::Dir3* object_vel);
    void  set_object_col_pt(const math::Dir3* object_col_pt);
};
static_assert(sizeof(pulse_sum_angular) == 0x90, "pulse_sum_angular size mismatch");

// pulse_sum_point - point constraint row (272 bytes, IDA ordinal 5466).
struct pulse_sum_point {
    phys_link_list_base<pulse_sum_point> m_link;  // +0x00
    uint8_t      _pad4[0x10 - 0x04];              // +0x04
    math::Dir3   m_b1_r;                          // +0x10
    math::Dir3   m_b2_r;                          // +0x20
    math::Dir3   m_b1_apx;                        // +0x30
    math::Dir3   m_b2_apx;                        // +0x40
    math::Dir3   m_b1_apy;                        // +0x50
    math::Dir3   m_b2_apy;                        // +0x60
    math::Dir3   m_b1_apz;                        // +0x70
    math::Dir3   m_b2_apz;                        // +0x80
    math::Dir3   m_pulse_sum;                     // +0x90
    math::Dir3   m_right_side;                    // +0xA0
    math::Dir3   m_big_dirt;                      // +0xB0
    math::Dir3   m_cr23;                          // +0xC0
    math::Dir3   m_cr31;                          // +0xD0
    math::Dir3   m_cr12;                          // +0xE0
    math::Dir3   m_denom;                         // +0xF0
    pulse_sum_node* m_b1;                         // +0x100
    pulse_sum_node* m_b2;                         // +0x104
    pulse_sum_cache* m_pulse_sum_cache;           // +0x108

    void set(rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2,
             const math::Dir3* b2_r, pulse_sum_cache* ps_cache);
    const math::Dir3* get_vel(const math::Dir3* result);
    const math::Dir3* get_pos(const math::Dir3* result);
    const math::Dir3* get_objective(const math::Dir3* result);
    const math::Dir3* phys_diag_multiply_and_square(const math::Dir3* result,
                                                    const math::Dir3* v1,
                                                    const math::Dir3* v2);
    void  apply(const math::Dir3* s_);
    void  calc_abs();
    void  project();
    void  SOLVER_apply_relaxation(float* error_sq);
    void  SOLVER_solver_prolog(int iter, float delta_t);
    void  SOLVER_solver_intermediate(int iter, float delta_t);
    void  set_object_vel(const math::Dir3* object_vel);
    void  set_object_col_pt(const math::Dir3* object_col_pt);
};
static_assert(sizeof(pulse_sum_point) == 0x110, "pulse_sum_point size mismatch");

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

    bool clamp_pulse_sum_pulse_chain(float* ps1_, float* ps2_);
    bool pulse_chain_within_limits();
    void addp_pulse_chain();
    void SOLVER_apply_relaxation(float* error_sq);
    void SOLVER_solver_prolog(int iter, float delta_t);
    void SOLVER_solver_intermediate(int iter, float delta_t);
};
static_assert(sizeof(pulse_sum_wheel) == 0xC0, "pulse_sum_wheel size mismatch");
static_assert(offsetof(pulse_sum_wheel, m_suspension) == 0x10, "pulse_sum_wheel::m_suspension offset mismatch");

// ============================================================================
// pulse_sum_contact â€” contact constraint row (64 bytes, IDA ordinal 5475).
// ============================================================================
struct pulse_sum_contact {
    struct vec2 {  // 8 bytes
        float x;  // +0x00
        float y;  // +0x04

        vec2() : x(0.0f), y(0.0f) {}           // ea: 0x891FB0
        vec2(float x_, float y_) : x(x_), y(y_) {}  // ea: 0x891FC0
        vec2 operator-(const vec2& other) const { return vec2(x - other.x, y - other.y); }
        void operator-=(const vec2& other) { x -= other.x; y -= other.y; }
    };
    static_assert(sizeof(vec2) == 8, "vec2 size mismatch");

    struct psc_cpi;  // defined below

    phys_link_list_base<pulse_sum_contact> m_link;  // +0x00
    uint8_t    _pad4[0x10 - 0x04];                 // +0x04
    math::Dir3 m_ud_n;                             // +0x10
    float      m_fric_coef;                        // +0x20
    pulse_sum_node* m_b1;                          // +0x24
    pulse_sum_node* m_b2;                          // +0x28
    psc_cpi*   m_list_cpi;                         // +0x2C
    int        m_list_cpi_count;                   // +0x30

    void set(rigid_body* b1, rigid_body* b2, contact_point_info* cpi, float delta_t);
};
static_assert(sizeof(pulse_sum_contact) == 0x40, "pulse_sum_contact size mismatch");

// ============================================================================
// pulse_sum_contact::psc_cpi - per-contact-point pulse-sum row (160 bytes).
// ============================================================================
struct pulse_sum_contact::psc_cpi {
    math::Dir3 m_b1_r;                            // +0x00
    math::Dir3 m_b2_r;                            // +0x10
    math::Dir3 m_ud_f1;                           // +0x20
    math::Dir3 m_b1_ap_n;                         // +0x30
    math::Dir3 m_b2_ap_n;                         // +0x40
    math::Dir3 m_b1_ap_f1;                        // +0x50
    math::Dir3 m_b2_ap_f1;                        // +0x60
    vec2       m_pulse_sum;                       // +0x70
    vec2       m_right_side;                      // +0x78
    float      m_big_dirt;                        // +0x80
    float      m_denom_xx;                        // +0x84
    float      m_denom_yy;                        // +0x88
    float      m_denom_xy;                        // +0x8C
    pulse_sum_cache* m_pulse_sum_cache;           // +0x90

    psc_cpi() {  // ea: 0x893250
        m_pulse_sum.x = 0.0f;
        m_pulse_sum.y = 0.0f;
        m_right_side.x = 0.0f;
        m_right_side.y = 0.0f;
        m_big_dirt = 0.0f;
        m_denom_xx = 0.0f;
        m_denom_yy = 0.0f;
        m_denom_xy = 0.0f;
        m_pulse_sum_cache = NULL;
    }

    void set_object_vel(psc_cpi* self, const math::Dir3* object_vel);
    void set_object_col_pt(psc_cpi* self, const math::Dir3* object_col_pt);
    const math::Dir3* get_relative_velocity_change_dir(psc_cpi* self);
    const math::Dir3* get_relative_velocity(psc_cpi* self, math::Dir3* result);
    const math::Dir3* get_last_relative_velocity(psc_cpi* self, math::Dir3* result);
    float get_impact_vel(psc_cpi* self, const math::Dir3* normal);
    float get_impact_dist(psc_cpi* self);
    void  setup_vel_uni_restitution(psc_cpi* self, const math::Dir3* relative_velocity,
                                    float restitution_k, float max_restitution_v,
                                    float delta_t, float max_penalty_restitution_vel);
    void  calc_fric_dir(psc_cpi* self, const math::Dir3* relative_velocity);
    void  calc_abs_and_fric_dir(psc_cpi* self, const math::Dir3* relative_velocity);
    void  apply(psc_cpi* self, const vec2* s_);
    void  clamp_n(psc_cpi* self);
    void  clamp_f(psc_cpi* self);
    void  project(psc_cpi* self);
    void  SOLVER_solver_intermediate(psc_cpi* self, int iter, float delta_t);
    void  SOLVER_solver_prolog(psc_cpi* self, int iter, float delta_t);
    void  SOLVER_apply_relaxation(psc_cpi* self, float* error_sq);
    void  set_pulse_sum_cache(psc_cpi* self, pulse_sum_cache* cache);
    const vec2* get_vel(psc_cpi* self, vec2* result);
    const vec2* get_objective(psc_cpi* self, vec2* result);
};
static_assert(sizeof(pulse_sum_contact::psc_cpi) == 0xA0, "psc_cpi size mismatch");

// ============================================================================
// phys_link_list<T> - doubly-linked list head (8 bytes, IDA ordinal 5861).
// ============================================================================
template <typename T>
struct phys_link_list {
    T* m_first;  // +0x00
    T* m_last;   // +0x04

    phys_link_list() : m_first(NULL), m_last(NULL) {}  // ea: 0x88E240

    struct iterator {
        T* m_ptr;  // +0x00

        iterator(T* ptr) : m_ptr(ptr) {}  // ea: 0x894C90
        T& operator*() const { return *m_ptr; }       // ea: 0x894B20
        bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }  // ea: 0x894B00
        iterator& operator++() { m_ptr = m_ptr->m_next_link; return *this; }  // ea: 0x8979F0
    };

    iterator begin() { return iterator(m_first); }  // ea: 0x897930
    iterator end() { return iterator(NULL); }       // ea: 0x897940

    void add(T* node) {  // ea: 0x897900
        node->m_next_link = NULL;
        if (m_last != NULL)
            m_last->m_next_link = node;
        else
            m_first = node;
        m_last = node;
    }

    void remove_all() {  // ea: 0x894AA0
        m_first = NULL;
        m_last = NULL;
    }
};
static_assert(sizeof(phys_link_list<int>) == 8, "phys_link_list size mismatch");

// ============================================================================
// pulse_sum_constraint_solver — the solver (112 bytes; methods in
// phys_constraint_solver_multithreaded.o, unresolved here).
// ============================================================================
class pulse_sum_constraint_solver {
public:
    struct solver_info {
        int   m_psc_visit_counter;      // +0x00
        int   m_next_psc_visit_counter; // +0x04
        int   m_max_vel_iters;          // +0x08
        int   m_max_vel_pos_iters;      // +0x0C
        float m_max_vel_error_sq;       // +0x10
        float m_max_vel_pos_error_sq;   // +0x14
        float m_delta_t;                // +0x18
    };

    int                m_psys_psc_visit_counter;      // +0x00
    int                m_psys_next_psc_visit_counter; // +0x04
    int                m_psys_max_vel_iters;          // +0x08
    int                m_psys_max_vel_pos_iters;      // +0x0C
    rigid_body*        m_first_partition_head;        // +0x10
    solver_info        m_si;                          // +0x14
    phys_memory_heap   m_solver_memory_allocater;     // +0x30
    phys_link_list<pulse_sum_node>    m_list_pulse_sum_node;    // +0x40
    phys_link_list<pulse_sum_normal>  m_list_pulse_sum_normal;  // +0x48
    phys_link_list<pulse_sum_point>   m_list_pulse_sum_point;   // +0x50
    phys_link_list<pulse_sum_angular> m_list_pulse_sum_angular; // +0x58
    phys_link_list<pulse_sum_wheel>   m_list_pulse_sum_wheel;   // +0x60
    phys_link_list<pulse_sum_contact> m_list_pulse_sum_contact; // +0x68

    pulse_sum_constraint_solver() {  // ea: 0x88EE00
        m_psys_psc_visit_counter = 0;
        m_psys_next_psc_visit_counter = 0;
        m_psys_max_vel_iters = 0;
        m_psys_max_vel_pos_iters = 0;
        m_first_partition_head = NULL;
        m_solver_memory_allocater.m_buffer_start = NULL;
        m_solver_memory_allocater.m_buffer_end = NULL;
        m_solver_memory_allocater.m_buffer_cur = NULL;
        m_solver_memory_allocater.m_user_start = NULL;
        m_list_pulse_sum_node.m_first = NULL;
        m_list_pulse_sum_node.m_last = NULL;
        m_list_pulse_sum_normal.m_first = NULL;
        m_list_pulse_sum_normal.m_last = NULL;
        m_list_pulse_sum_point.m_first = NULL;
        m_list_pulse_sum_point.m_last = NULL;
        m_list_pulse_sum_angular.m_first = NULL;
        m_list_pulse_sum_angular.m_last = NULL;
        m_list_pulse_sum_wheel.m_first = NULL;
        m_list_pulse_sum_wheel.m_last = NULL;
        m_list_pulse_sum_contact.m_first = NULL;
        m_list_pulse_sum_contact.m_last = NULL;
    }

    void list_partition_head_reset() { m_first_partition_head = NULL; }  // ea: 0x88B0D0
    void list_partition_head_add(rigid_body* rb) {  // ea: 0x88B0E0
        rb->m_partition_node.m_next_partition_head = m_first_partition_head;
        m_first_partition_head = rb;
    }

    // pulse_sum_constraint_solver inline methods (this unit).
    struct temp_user_rigid_body;
    struct user_rigid_body_restore_info;

    pulse_sum_node* create_pulse_sum_node();          // ea: 0x897A90
    void solve_constraints();                         // ea: 0x897B10
    void execute_constraint_solver(rigid_body* head); // ea: 0x898250
    void add_urb(temp_user_rigid_body** list_turb,
                 user_rigid_body_restore_info** list_urbri,
                 rigid_body_constraint* rbc);         // ea: 0x8980E0
    void list_urbri_restore(user_rigid_body_restore_info* list_urbri);  // ea: 0x893260
    void set_solver_params(int psys_psc_visit_counter, int psys_next_psc_visit_counter,
                           int psys_max_vel_iters, int psys_max_vel_pos_iters);  // ea: 0x893290
    static bool psc_is_persistant(pulse_sum_cache* ps_cache, int visit_counter);  // ea: 0x892140
    static void set_pulse_sum(pulse_sum_cache* ps_cache, int visit_counter, float pulse_sum);  // ea: 0x8926A0
    static float get_pulse_sum(pulse_sum_cache* ps_cache, int visit_counter);  // ea: 0x8926C0
    void solve_iterative(int max_iters, float max_error_sq);  // ea: 0x8945B0

    struct temp_user_rigid_body : public user_rigid_body {
        user_rigid_body* m_original_urb;   // +0x1C0
        temp_user_rigid_body* m_next_link; // +0x1C4

        temp_user_rigid_body();  // ea: 0x8980D0
        void set(temp_user_rigid_body* next_link, user_rigid_body* original_urb);  // ea: 0x897A50
    };

    struct user_rigid_body_restore_info {
        user_rigid_body_restore_info* m_next_link;   // +0x00
        user_rigid_body** m_rbc_urb;                 // +0x04
        user_rigid_body* m_original_urb;             // +0x08

        void set(user_rigid_body_restore_info* next_link, user_rigid_body** rbc_urb,
                 temp_user_rigid_body* original_urb);  // ea: 0x891110
        void restore();  // ea: 0x891130
    };

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
    void create_point(rigid_body* b1, const math::Dir3* b1_r, rigid_body* b2,
                      const math::Dir3* b2_r, pulse_sum_cache* ps_cache, float delta_t);
    void create_hinge(rigid_body* b1, const math::Dir3* b1_axis, rigid_body* b2,
                      const math::Dir3* b2_axis, const math::Dir3* a1, const math::Dir3* a2,
                      pulse_sum_cache* ps_cache, float delta_t);
};
static_assert(sizeof(pulse_sum_constraint_solver) == 0x70, "pulse_sum_constraint_solver size mismatch");

// ============================================================================
// phys_inplace_avl_tree â€” AVL tree used by contact constraints (4 bytes)
// ============================================================================
template <typename Key, typename T>
struct phys_inplace_avl_tree {
    T* m_tree_root;  // +0x00

    struct stack_item {
        T** m_node;
        int  m_child;
    };

    static bool key_lt(const Key& a, const Key& b) {
        if (a.m_b1 != b.m_b1)
            return a.m_b1 < b.m_b1;
        return a.m_b2 < b.m_b2;
    }

    static int avl_max(int a, int b) { return a <= b ? b : a; }  // ea: 0x881040

    // find - ea: 0x7175C0
    T* find(const Key& key) {
        T* result = m_tree_root;
        if (m_tree_root != NULL) {
            do {
                if (key.m_b1 == result->m_avl_key.m_b1 &&
                    key.m_b2 == result->m_avl_key.m_b2)
                    break;
                if (key_lt(key, result->m_avl_key))
                    result = result->m_avl_tree_node.m_left;
                else
                    result = result->m_avl_tree_node.m_right;
            } while (result != NULL);
        }
        return result;
    }

    // add - ea: 0x881E80
    void add(const Key& key, T* data) {
        stack_item the_stack[32];
        stack_item* cur = the_stack;
        the_stack[0].m_node = &m_tree_root;
        bool done = (m_tree_root == NULL);
        for (; !done; ++cur) {
            T* node = *cur->m_node;
            if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256 &&
                _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 586,
                          "cur_item + 1 - the_stack < 32", ""))
                __debugbreak();
            if (key_lt(key, node->m_avl_key)) {
                cur->m_child = -1;
                cur[1].m_node = &node->m_avl_tree_node.m_left;
            } else {
                if (!key_lt(node->m_avl_key, key) &&
                    _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 595,
                              "key > root->get_avl_key()", ""))
                    __debugbreak();
                cur->m_child = 1;
                cur[1].m_node = &node->m_avl_tree_node.m_right;
            }
            done = (*cur[1].m_node == NULL);
        }
        *cur->m_node = data;
        data->m_avl_tree_node.m_left = NULL;
        data->m_avl_tree_node.m_right = NULL;
        data->m_avl_tree_node.m_balance = 0;
        data->m_avl_key = key;
        if (cur > the_stack) {
            T** m_node;
            do {
                m_node = cur[-1].m_node;
                int m_child = cur[-1].m_child;
                --cur;
                (*m_node)->m_avl_tree_node.m_balance += m_child;
                T* root = *m_node;
                int m_balance = root->m_avl_tree_node.m_balance;
                if (m_balance == -2) {
                    int v16 = root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance;
                    if (v16 != -1 && v16 != 1 &&
                        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 613,
                                  "GAVLN(GAVLN(root)->m_left)->m_balance == -1 || GAVLN(GAVLN(root)->m_left)->m_balance == 1",
                                  ""))
                        __debugbreak();
                    T* m_left = root->m_avl_tree_node.m_left;
                    if (m_left->m_avl_tree_node.m_balance == 1) {
                        T* m_right = m_left->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_left->m_avl_tree_node.m_right =
                            m_right->m_avl_tree_node.m_left;
                        int v20 = m_right->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_left = root->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance +=
                            v20 <= 0 ? 1 : v20 + 1;
                        int v21 = -root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_balance +=
                            (v21 < 0 || root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance == 0)
                                ? 1 : v21 + 1;
                        root->m_avl_tree_node.m_left = m_right;
                    }
                    T* v22 = root->m_avl_tree_node.m_left;
                    root->m_avl_tree_node.m_left = v22->m_avl_tree_node.m_right;
                    v22->m_avl_tree_node.m_right = root;
                    root->m_avl_tree_node.m_balance +=
                        ((-v22->m_avl_tree_node.m_balance < 0 ||
                          v22->m_avl_tree_node.m_balance == 0)
                             ? -v22->m_avl_tree_node.m_balance + 1
                             : 1);
                    v22->m_avl_tree_node.m_balance +=
                        root->m_avl_tree_node.m_balance <= 0
                            ? 1
                            : root->m_avl_tree_node.m_balance + 1;
                    *m_node = v22;
                    if (v22->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 617,
                                  "GAVLN(root)->m_balance == 0", ""))
                        __debugbreak();
                } else if (m_balance == 2) {
                    int v24 = root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance;
                    if (v24 != -1 && v24 != 1 &&
                        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 621,
                                  "GAVLN(GAVLN(root)->m_right)->m_balance == -1 || GAVLN(GAVLN(root)->m_right)->m_balance == 1",
                                  ""))
                        __debugbreak();
                    T* v26 = root->m_avl_tree_node.m_right;
                    if (v26->m_avl_tree_node.m_balance == -1) {
                        T* v27 = v26->m_avl_tree_node.m_left;
                        v26->m_avl_tree_node.m_left = v27->m_avl_tree_node.m_right;
                        int v28 = v27->m_avl_tree_node.m_balance;
                        v27->m_avl_tree_node.m_right = root->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance +=
                            v28 >= 0 ? 1 : -v28 + 1;
                        v27->m_avl_tree_node.m_balance +=
                            root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance <= 0
                                ? 1
                                : root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance + 1;
                        root->m_avl_tree_node.m_right = v27;
                    }
                    T* v29 = root->m_avl_tree_node.m_right;
                    root->m_avl_tree_node.m_right = v29->m_avl_tree_node.m_left;
                    v29->m_avl_tree_node.m_left = root;
                    root->m_avl_tree_node.m_balance +=
                        v29->m_avl_tree_node.m_balance <= 0 ? 1 : v29->m_avl_tree_node.m_balance + 1;
                    v29->m_avl_tree_node.m_balance +=
                        (-root->m_avl_tree_node.m_balance < 0 ||
                         root->m_avl_tree_node.m_balance == 0)
                            ? 1
                            : -root->m_avl_tree_node.m_balance + 1;
                    *m_node = v29;
                    if (v29->m_avl_tree_node.m_balance == 0)
                        continue;
                    if (_tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_avl_tree.h", 625,
                                  "GAVLN(root)->m_balance == 0", ""))
                        __debugbreak();
                } else {
                    continue;
                }
            } while ((*m_node)->m_avl_tree_node.m_balance != 0 && cur > the_stack);
        }
    }

    // rotate_right - ea: 0x881730
    void rotate_right(T** root) {
        T* m_left = (*root)->m_avl_tree_node.m_left;
        (*root)->m_avl_tree_node.m_left = m_left->m_avl_tree_node.m_right;
        int m_balance = m_left->m_avl_tree_node.m_balance;
        m_left->m_avl_tree_node.m_right = *root;
        (*root)->m_avl_tree_node.m_balance +=
            m_balance >= 0 ? 1 : -m_balance + 1;
        m_left->m_avl_tree_node.m_balance +=
            (*root)->m_avl_tree_node.m_balance <= 0
                ? 1
                : (*root)->m_avl_tree_node.m_balance + 1;
        *root = m_left;
    }

    // rotate_left - ea: 0x881790
    void rotate_left(T** root) {
        T* m_right = (*root)->m_avl_tree_node.m_right;
        (*root)->m_avl_tree_node.m_right = m_right->m_avl_tree_node.m_left;
        int m_balance = m_right->m_avl_tree_node.m_balance;
        m_right->m_avl_tree_node.m_left = *root;
        (*root)->m_avl_tree_node.m_balance +=
            m_balance <= 0 ? 1 : m_balance + 1;
        int neg = -(*root)->m_avl_tree_node.m_balance;
        m_right->m_avl_tree_node.m_balance +=
            -1 - (neg & ((neg < 0 || (*root)->m_avl_tree_node.m_balance == 0) ? -1 : 0));
        *root = m_right;
    }

    // remove - ea: 0x88ACC0
    void remove(const Key* key) {
        stack_item the_stack[32];
        stack_item* cur = the_stack;
        the_stack[0].m_node = &m_tree_root;
        while (1) {
            T* node = *cur->m_node;
            if (node == NULL &&
                _tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 644,
                          "root", ""))
                __debugbreak();
            if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256 &&
                _tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 645,
                          "cur_item + 1 - the_stack < 32", ""))
                __debugbreak();
            if (!key_lt(*key, node->m_avl_key))
                break;
            cur->m_child = -1;
            cur[1].m_node = &node->m_avl_tree_node.m_left;
            ++cur;
        }
        T* node = *cur->m_node;
        if (key_lt(node->m_avl_key, *key)) {
            cur->m_child = 1;
            cur[1].m_node = &node->m_avl_tree_node.m_right;
            ++cur;
        }
        T* found = *cur->m_node;
        if ((key->m_b1 != found->m_avl_key.m_b1 || key->m_b2 != found->m_avl_key.m_b2) &&
            _tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 659,
                      "key == root->get_avl_key()", ""))
            __debugbreak();
        T** v9 = cur->m_node;
        T* v10 = *cur->m_node;
        stack_item* right_item = NULL;
        if (v10->m_avl_tree_node.m_right != NULL) {
            cur->m_child = 1;
            ++cur;
            cur->m_node = &v10->m_avl_tree_node.m_right;
            right_item = cur;
            while ((*cur->m_node)->m_avl_tree_node.m_left != NULL) {
                if (((cur - the_stack + 8) & 0xFFFFFFF8) >= 256 &&
                    _tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 674,
                              "cur_item + 1 - the_stack < 32", ""))
                    __debugbreak();
                cur->m_child = -1;
                cur[1].m_node = &(*cur->m_node)->m_avl_tree_node.m_left;
                ++cur;
            }
            T* m_left = *cur->m_node;
            *cur->m_node = m_left->m_avl_tree_node.m_right;
            m_left->m_avl_tree_node.m_left = (*v9)->m_avl_tree_node.m_left;
            m_left->m_avl_tree_node.m_right = (*v9)->m_avl_tree_node.m_right;
            m_left->m_avl_tree_node.m_balance = (*v9)->m_avl_tree_node.m_balance;
            right_item->m_node = &m_left->m_avl_tree_node.m_right;
            *v9 = m_left;
        } else {
            *v9 = v10->m_avl_tree_node.m_left;
        }
        if (cur > the_stack) {
            T** v15;
            do {
                v15 = cur[-1].m_node;
                int m_child = cur[-1].m_child;
                --cur;
                (*v15)->m_avl_tree_node.m_balance -= m_child;
                T* root = *v15;
                int m_balance = root->m_avl_tree_node.m_balance;
                if (m_balance == -2) {
                    T* v19 = root->m_avl_tree_node.m_left;
                    if (v19->m_avl_tree_node.m_balance == 1) {
                        T* m_right = v19->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_left->m_avl_tree_node.m_right =
                            m_right->m_avl_tree_node.m_left;
                        int v21 = m_right->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_left = root->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance +=
                            v21 <= 0 ? 1 : v21 + 1;
                        int v22 = -root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance;
                        m_right->m_avl_tree_node.m_balance +=
                            (v22 < 0 || root->m_avl_tree_node.m_left->m_avl_tree_node.m_balance == 0)
                                ? 1 : v22 + 1;
                        root->m_avl_tree_node.m_left = m_right;
                    }
                    T* v23 = root->m_avl_tree_node.m_left;
                    root->m_avl_tree_node.m_left = v23->m_avl_tree_node.m_right;
                    v23->m_avl_tree_node.m_right = root;
                    root->m_avl_tree_node.m_balance +=
                        ((-v23->m_avl_tree_node.m_balance < 0 ||
                          v23->m_avl_tree_node.m_balance == 0)
                             ? -v23->m_avl_tree_node.m_balance + 1
                             : 1);
                    int v24 = v23->m_avl_tree_node.m_balance;
                    int v25 = root->m_avl_tree_node.m_balance <= 0
                                  ? 1
                                  : root->m_avl_tree_node.m_balance + 1;
                    v23->m_avl_tree_node.m_balance = v25 + v24;
                    *v15 = v23;
                    int v26 = v23->m_avl_tree_node.m_balance;
                    if (v25 + v24 == 0 || v26 == 1)
                        continue;
                    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 701,
                                  "GAVLN(root)->m_balance == 0 || GAVLN(root)->m_balance == +1", ""))
                        __debugbreak();
                } else if (m_balance == 2) {
                    T* v28 = root->m_avl_tree_node.m_right;
                    if (v28->m_avl_tree_node.m_balance == -1) {
                        T* v29 = v28->m_avl_tree_node.m_left;
                        root->m_avl_tree_node.m_right->m_avl_tree_node.m_left =
                            v29->m_avl_tree_node.m_right;
                        int v30 = v29->m_avl_tree_node.m_balance;
                        v29->m_avl_tree_node.m_right = root->m_avl_tree_node.m_right;
                        root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance +=
                            v30 >= 0 ? 1 : -v30 + 1;
                        v29->m_avl_tree_node.m_balance +=
                            root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance <= 0
                                ? 1
                                : root->m_avl_tree_node.m_right->m_avl_tree_node.m_balance + 1;
                        root->m_avl_tree_node.m_right = v29;
                    }
                    T* v31 = root->m_avl_tree_node.m_right;
                    root->m_avl_tree_node.m_right = v31->m_avl_tree_node.m_left;
                    int v32 = v31->m_avl_tree_node.m_balance;
                    v31->m_avl_tree_node.m_left = root;
                    root->m_avl_tree_node.m_balance += v32 <= 0 ? 1 : v32 + 1;
                    int v33 = (-root->m_avl_tree_node.m_balance < 0 ||
                               root->m_avl_tree_node.m_balance == 0)
                                  ? 1
                                  : -root->m_avl_tree_node.m_balance + 1;
                    bool v34 = (v33 + v31->m_avl_tree_node.m_balance == 0);
                    v31->m_avl_tree_node.m_balance += v33;
                    *v15 = v31;
                    int v35 = v31->m_avl_tree_node.m_balance;
                    if (v34 || v35 == -1)
                        continue;
                    if (_tlAssert("c:/cod/code/tl/physics/include\\phys_avl_tree.h", 708,
                                  "GAVLN(root)->m_balance == 0 || GAVLN(root)->m_balance == -1", ""))
                        __debugbreak();
                } else {
                    continue;
                }
            } while ((*v15)->m_avl_tree_node.m_balance == 0 && cur > the_stack);
        }
    }
};

// ============================================================================
// phys_heap_memory_pool<T> - fixed-slot heap pool (20 bytes).
// Layout verified against IDA local types.
// ============================================================================
template <typename T>
struct phys_heap_memory_pool {
    T*   m_slot_array;      // +0x00
    T**  m_alloc_list;      // +0x04
    int* m_index_array;     // +0x08
    int  m_slot_array_size; // +0x0C
    int  m_alloc_count;     // +0x10

    struct iterator {
        T** m_ptr;  // +0x00

        iterator(T** ptr) : m_ptr(ptr) {}
        iterator() : m_ptr(NULL) {}
        T& operator*() const { return **m_ptr; }
        T* operator->() const { return *m_ptr; }
        iterator& operator++() { ++m_ptr; return *this; }
        iterator& operator--() { --m_ptr; return *this; }
        bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }
        iterator next_after_remove() const { return iterator(m_ptr); }
        iterator next() const { return iterator(m_ptr + 1); }  // ea: 0x88EF20
        iterator prev() const { return iterator(m_ptr - 1); }  // ea: 0x88EF40
    };

    bool is_member(const T* data) const;
    int  get_max_slots() const { return m_slot_array_size; }
    int  get_available_slots() const { return m_slot_array_size - m_alloc_count; }
    int  get_used_slots() const { return m_alloc_count; }
    int  get_count() const { return m_alloc_count; }
    T*   add(bool no_error, const char* error_msg);
    void remove(T* data);
    void remove_all() { reset_buffer(); }
    void reset_buffer();
    void call_destructors() {}
    void destroy();                     // destroy + free arrays
    void calc_index_array();            // ea: 0x88E320
    void swap_adjacent_fast(iterator* i, iterator* i_next);  // ea: 0x88E370

    static int get_alignment() { return 16; }  // ea: 0x88EE50
    static int get_buffer_size(int size);      // ea: 0x88E2B0
    void allocate_buffer(int size, phys_memory_heap* allocater);  // ea: 0x88F740
    ~phys_heap_memory_pool() { destroy(); }

    iterator begin() { return iterator(m_alloc_list); }
    iterator end() { return iterator(&m_alloc_list[m_alloc_count]); }
    iterator before_begin() { return iterator(m_alloc_list - 1); }  // ea: 0x88EE80
};
static_assert(sizeof(phys_heap_memory_pool<int>) == 0x14, "phys_heap_memory_pool size mismatch");

// ============================================================================
// physics_system - the physics engine singleton (864 bytes).
// Layout verified against IDA local types (ordinal 7028).
// ============================================================================
// list_constraint_solver - partition list fed to the multithreaded solver.
struct physics_system;

struct phys_constraint_solver_multithreaded_list_constraint_solver {
    pulse_sum_constraint_solver* m_constraint_solver;  // +0x00

    void reset(pulse_sum_constraint_solver* constraint_solver) {  // ea: 0x88B3F0
        m_constraint_solver = constraint_solver;
        constraint_solver->m_first_partition_head = NULL;
    }

    void add(rigid_body* cg) {  // ea: 0x88B410
        cg->m_partition_node.m_next_partition_head =
            m_constraint_solver->m_first_partition_head;
        m_constraint_solver->m_first_partition_head = cg;
    }

    void process(const physics_system* psys, int psys_next_psc_visit_counter);  // ea: 0x894A50
};

struct physics_system {
    int      m_flags;                                     // +0x00
    float    m_outside_sub_delta_t;                       // +0x04
    int      m_psc_visit_counter;                         // +0x08
    void     (*m_collision_callback)();                   // +0x0C
    float    m_max_delta_t;                               // +0x10
    int      m_max_vel_iters;                             // +0x14
    int      m_max_vel_siters;                            // +0x18
    int      m_max_vel_pos_iters;                         // +0x1C
    int      m_max_vel_pos_siters;                        // +0x20
    uint8_t  _pad24[0x30 - 0x24];                         // +0x24
    environment_rigid_body m_environment_rigid_body;      // +0x30
    int      m_solver_memory_high;                        // +0x1E0
    phys_inplace_avl_tree<rigid_body_pair_key, rigid_body_constraint_contact>
        m_search_tree_rbc_contact;                        // +0x1E4
    phys_heap_memory_pool<user_rigid_body> m_list_user_rigid_body;    // +0x1E8
    phys_heap_memory_pool<rigid_body>      m_list_rigid_body;         // +0x1FC
    phys_heap_memory_pool<rigid_body_constraint_contact>
        m_list_rbc_contact;                               // +0x210
    phys_memory_heap m_contact_point_buffer_1;            // +0x224
    phys_memory_heap m_contact_point_buffer_2;            // +0x234
    phys_heap_memory_pool<rigid_body_constraint_point> m_list_rbc_point;  // +0x244
    phys_heap_memory_pool<rigid_body_constraint_hinge> m_list_rbc_hinge;  // +0x258
    phys_heap_memory_pool<rigid_body_constraint_distance> m_list_rbc_dist; // +0x26C
    phys_heap_memory_pool<rigid_body_constraint_custom_orientation>
        m_list_rbc_custom_orientation;                    // +0x280
    phys_heap_memory_pool<rigid_body_constraint_custom_path>
        m_list_rbc_custom_path;                           // +0x294
    phys_heap_memory_pool<rigid_body_constraint_ragdoll> m_list_rbc_ragdoll; // +0x2A8
    phys_heap_memory_pool<rigid_body_constraint_wheel> m_list_rbc_wheel;     // +0x2BC
    phys_heap_memory_pool<rigid_body_constraint_angular_actuator>
        m_list_rbc_angular_actuator;                      // +0x2D0
    pulse_sum_constraint_solver m_constraint_solver;      // +0x2E4

    bool is_member(const rigid_body* rb) const;
    void solver_memory_buffer_set(void* buffer, int buffer_size);
    void solver_memory_buffer_nullify();

    physics_system();  // ea: 0x88DA40
    ~physics_system();  // ea: 0x890130

    static int get_alignment();               // ea: 0x88D550
    static unsigned int get_buffer_size(const phys_mem_info* pmi);  // ea: 0x88D560
    static physics_system* allocate_buffer(const phys_mem_info* pmi,
                                           phys_memory_heap* allocater);  // ea: 0x88DC70
    static void create_inst(phys_mem_info* pmi);    // ea: 0x88DE20
    static void destroy_inst();                     // ea: 0x88E140
    static void free_buffer(physics_system* psys);  // ea: 0x88E130

    void frame_advance(float delta_t);              // ea: 0x88DEC0
    void time_step(float outside_delta_t, bool last_step);  // ea: 0x88D620
    void solver_priority_sort();                    // ea: 0x88F550
    void set_flag(unsigned int f, int b);           // ea: 0x88B430
    void set_outside_sub_delta_t(float outside_sub_delta_t);  // ea: 0x88B460
    void generate_partitions_and_stuff(
        phys_constraint_solver_multithreaded_list_constraint_solver* list_cs,
        int* next_psc_visit_counter, float delta_t);  // ea: 0x88C830
};
static_assert(sizeof(physics_system) == 0x360, "physics_system size mismatch");

// physics_system::is_member - ea: 0x881380 (inline COMDAT)
inline bool physics_system::is_member(const rigid_body* rb) const {
    if (rb == NULL)
        return true;
    unsigned int m_flags = rb->m_flags;
    if ((m_flags & 0x10) != 0)
        return rb == &this->m_environment_rigid_body;
    if ((m_flags & 0x20) != 0)
        return m_list_user_rigid_body.is_member((const user_rigid_body*)rb);
    return m_list_rigid_body.is_member(rb);
}

// physics_system::solver_memory_buffer_set - ea: 0x87EA30
inline void physics_system::solver_memory_buffer_set(void* buffer, int buffer_size) {
    m_constraint_solver.m_solver_memory_allocater.set_buffer((char*)buffer, buffer_size, 1);
}

// physics_system::solver_memory_buffer_nullify - ea: 0x87EA50
inline void physics_system::solver_memory_buffer_nullify() {
    m_constraint_solver.m_solver_memory_allocater.m_buffer_start = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_buffer_end = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_buffer_cur = NULL;
    m_constraint_solver.m_solver_memory_allocater.m_user_start = NULL;
}

// physics_system::set_flag - ea: 0x88B430
inline void physics_system::set_flag(unsigned int f, int b) {
    if (b != 0)
        m_flags |= f;
    else
        m_flags &= ~f;
}

// physics_system::set_outside_sub_delta_t - ea: 0x88B460
inline void physics_system::set_outside_sub_delta_t(float outside_sub_delta_t) {
    m_outside_sub_delta_t = outside_sub_delta_t;
}

// ============================================================================
// phys_heap_memory_pool<T> method bodies (template COMDATs, physics_system.o).
// ============================================================================
template <typename T>
bool phys_heap_memory_pool<T>::is_member(const T* data) const {
    intptr_t diff = (const char*)data - (const char*)m_slot_array;
    if (diff % sizeof(T) == 0) {
        int idx = (int)(diff / sizeof(T));
        if (idx >= 0 && idx < m_slot_array_size) {
            int alloc = m_index_array[idx];
            if (alloc >= 0 && alloc < m_alloc_count)
                return true;
        }
    }
    return false;
}

template <typename T>
T* phys_heap_memory_pool<T>::add(bool no_error, const char* error_msg) {
    int m_alloc_count = this->m_alloc_count;
    if (m_alloc_count < m_slot_array_size) {
        T* result = m_alloc_list[m_alloc_count];
        m_alloc_count = m_alloc_count + 1;
        this->m_alloc_count = m_alloc_count;
        return result;
    }
    if (!no_error)
        tlFatal(error_msg);
    return NULL;
}

template <typename T>
void phys_heap_memory_pool<T>::remove(T* data) {
    if (data != NULL) {
        if (!is_member(data))
            tlFatal("phys_memory_pool: trying to delete an invalid pointer");
        if (!is_member(data) &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 61,
                      "is_member(data)",
                      "phys_memory_pool: trying to delete an invalid pointer"))
            __debugbreak();
        if (m_alloc_count <= 0 &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 62,
                      "m_alloc_count > 0",
                      "phys_memory_pool: trying to delete an invalid pointer"))
            __debugbreak();
        int slot_index = (int)((const char*)data - (const char*)m_slot_array) / (int)sizeof(T);
        int v3 = m_index_array[slot_index];
        if ((v3 < 0 || v3 >= m_alloc_count) &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 67,
                      "alloc_index >= 0 && alloc_index < m_alloc_count",
                      "phys_memory_pool: trying to delete an invalid pointer"))
            __debugbreak();
        if (v3 >= 0) {
            int m_alloc_count = this->m_alloc_count;
            if (v3 < m_alloc_count) {
                if (m_alloc_count > 1) {
                    T** m_alloc_list = this->m_alloc_list;
                    int v6 = m_alloc_count - 1;
                    this->m_alloc_count = v6;
                    T* v7 = m_alloc_list[v6];
                    int v8 = (int)((const char*)v7 - (const char*)m_slot_array) / (int)sizeof(T);
                    T* v9 = m_alloc_list[v3];
                    m_alloc_list[v3] = v7;
                    m_alloc_list[m_alloc_count] = v9;
                    m_index_array[slot_index] = m_alloc_count;
                    m_index_array[v8] = v3;
                } else {
                    reset_buffer();
                }
            }
        }
    }
}

template <typename T>
void phys_heap_memory_pool<T>::reset_buffer() {
    int v1 = 0;
    if (m_slot_array_size > 0) {
        int v2 = 0;
        do {
            m_index_array[v1] = v1;
            m_alloc_list[v1] = &m_slot_array[v2];
            ++v1;
            ++v2;
        } while (v1 < m_slot_array_size);
    }
    m_alloc_count = 0;
}

template <typename T>
int phys_heap_memory_pool<T>::get_buffer_size(int size) {
    return (int)(sizeof(T) * size + 4 * size + 4 * size);  // slots + alloc list + index
}

template <typename T>
void phys_heap_memory_pool<T>::allocate_buffer(int size, phys_memory_heap* allocater) {
    if (m_index_array != NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 260,
                  "!m_index_array", ""))
        __debugbreak();
    if (m_slot_array != NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 261,
                  "!m_slot_array", ""))
        __debugbreak();
    if (m_alloc_list != NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 262,
                  "!m_alloc_list", ""))
        __debugbreak();
    if (size > 0) {
        m_slot_array_size = size;
        T* slot = (T*)allocater->allocate_no_error((int)sizeof(T) * size, 16);
        if (slot == NULL &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 89,
                      "addr", "phys_memory_heap overflow."))
            __debugbreak();
        T** alloc = (T**)&slot[size];
        m_slot_array = slot;
        m_alloc_list = alloc;
        m_index_array = (int*)&alloc[size];
        reset_buffer();
    }
}

template <typename T>
void phys_heap_memory_pool<T>::calc_index_array() {
    for (int i = 0; i < m_alloc_count; ++i)
        m_index_array[m_alloc_list[i] - m_slot_array] = i;
}

template <typename T>
void phys_heap_memory_pool<T>::swap_adjacent_fast(iterator* i, iterator* i_next) {
    if ((i == NULL || i_next == NULL) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 224,
                  "i && i_next", ""))
        __debugbreak();
    T** m_alloc_list = this->m_alloc_list;
    if ((i->m_ptr < m_alloc_list || i->m_ptr >= &m_alloc_list[m_alloc_count]) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 225,
                  "i->m_ptr >= m_alloc_list && i->m_ptr < m_alloc_list + m_alloc_count", ""))
        __debugbreak();
    T** v5 = this->m_alloc_list;
    if ((i_next->m_ptr < v5 || i_next->m_ptr >= &v5[m_alloc_count]) &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc", 226,
                  "i_next->m_ptr >= m_alloc_list && i_next->m_ptr < m_alloc_list + m_alloc_count", ""))
        __debugbreak();
    T* v6 = *i->m_ptr;
    *i->m_ptr = *i_next->m_ptr;
    *i_next->m_ptr = v6;
}

template <typename T>
void phys_heap_memory_pool<T>::destroy() {
    // ea: 0x88F060 (per-instantiation COMDAT)
    for (int i = 0; i < m_alloc_count; ++i)
        m_alloc_list[i]->~T();
    if (m_slot_array != NULL) {
        if (m_alloc_list != (T**)&m_slot_array[m_slot_array_size] &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 236,
                      "m_alloc_list == (T**)(m_slot_array + m_slot_array_size)", ""))
            __debugbreak();
        if (m_index_array != (int*)&m_alloc_list[m_slot_array_size] &&
            _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_mem.h", 237,
                      "m_index_array == (int*)(m_alloc_list + m_slot_array_size)", ""))
            __debugbreak();
    }
    m_index_array = NULL;
    m_slot_array = NULL;
    m_slot_array_size = 0;
    m_alloc_list = NULL;
    m_alloc_count = 0;
}

// ============================================================================
// phys_static_memory_pool<T,N> - fixed inline-slot pool
// (phys_memory_pool_base.inc). Layout verified for <vehicle_rb_parameter,10>:
// slots inline at +0x00, then m_alloc_list[N], m_index_array[N],
// m_slot_array, m_alloc_count (total N*sizeof(T) + N*8 + 8).
// COMDATs in physics.o: add @0x717420, operator[] @0x717480,
// get_count @0x7174F0, reset_buffer @0x717CF0, call_destructors @0x717CE0,
// ctor @0x71AFD0, dtor @0x71B000.
// ============================================================================
template <typename T, int N>
struct phys_static_memory_pool {
    T   m_slots[N];        // +0x00
    T*  m_alloc_list[N];   // +N*sizeof(T)
    int m_index_array[N];  // +N*sizeof(T)+N*4
    T*  m_slot_array;      // +N*sizeof(T)+N*8
    int m_alloc_count;     // +N*sizeof(T)+N*8+4

    phys_static_memory_pool()
    {
        m_slot_array = (T*)this;
        m_alloc_count = 0;
        reset_buffer();
    }
    ~phys_static_memory_pool() {}

    void reset_buffer()
    {
        for (int i = 0; i < N; ++i)
        {
            m_index_array[i] = i;
            m_alloc_list[i] = m_slot_array + i;
        }
        m_alloc_count = 0;
    }

    void call_destructors() {}

    T* add(bool no_error, const char* error_msg)
    {
        int count = m_alloc_count;
        if (count < N)
        {
            T* result = m_alloc_list[count];
            m_alloc_count = count + 1;
            if (result != NULL)
                new (result) T;
            return result;
        }
        if (!no_error)
            tlFatal(error_msg);
        return NULL;
    }

    T& operator[](int i)
    {
        if ((i < 0 || i >= m_alloc_count)
            && _tlAssert(
                   "c:\\cod\\code\\tl\\physics\\include\\phys_memory_pool_base.inc",
                   178, "i >= 0 && i < m_alloc_count", ""))
            __debugbreak();
        return *m_alloc_list[i];
    }

    int get_count() const { return m_alloc_count; }
};

extern physics_system* g_physics_system;  // ?g_physics_system@@3PAVphysics_system@@A
extern void verify_is_in_physics_system(rigid_body_constraint_contact* rbc,
                                        rigid_body* b1_, rigid_body* b2_);
extern void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m);

namespace rbint {
void calc_col_mat(rigid_body* rb, const outer_time* outside_delta_t);
math::Dir3* mul_L(math::Dir3* result, const rigid_body* rb, const math::Dir3* t);

// get_pulse_sum_node - ea: 0x8924A0
inline pulse_sum_node* get_pulse_sum_node(const rigid_body* rb) {
    return rb->m_node;
}

// verify_pulse_sum_node - ea: 0x8925F0
inline unsigned int verify_pulse_sum_node(rigid_body* const rb) {
    if (rb->m_node != NULL)
        return (rb->m_flags & 0x30) == 0;
    return rb->m_flags & 0x30;
}

// add_vel - ea: 0x8924B0
inline void add_vel(rigid_body* rb, const math::Dir3* t, const math::Dir3* a) {
    rb->m_t_vel.v = _mm_add_ps(rb->m_t_vel.v, t->v);
    rb->m_a_vel.v = _mm_add_ps(rb->m_a_vel.v, a->v);
}

// get_last_t_vel / get_last_a_vel - ea: 0x892620 / 0x892630
inline const math::Dir3* get_last_t_vel(rigid_body* rb) { return &rb->m_last_t_vel; }
inline const math::Dir3* get_last_a_vel(rigid_body* rb) { return &rb->m_last_a_vel; }

// gtv - ea: 0x8955A0
inline const math::Dir3* gtv(const math::Dir3* result, rigid_body* const b,
                             const math::Dir3* r) {
    ((math::Dir3*)result)->v = _mm_add_ps(
        b->m_t_vel.v,
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(b->m_a_vel.v, b->m_a_vel.v, 9), _mm_shuffle_ps(r->v, r->v, 18)),
            _mm_mul_ps(_mm_shuffle_ps(b->m_a_vel.v, b->m_a_vel.v, 18), _mm_shuffle_ps(r->v, r->v, 9))));
    return result;
}

// inv_L (3-arg) - ea: 0x894FE0
inline const math::Dir3* inv_L(const math::Dir3* result, const rigid_body* rb,
                               const math::Dir3* t) {
    if ((~(rb->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body_internal.h", 19,
                  "rb->debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    ((math::Dir3*)result)->v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 0), rb->m_world_inv_inertia.x.v),
            _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 85), rb->m_world_inv_inertia.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 170), rb->m_world_inv_inertia.z.v));
    return result;
}

// inv_L (4-arg) - ea: 0x895080
inline const math::Dir3* inv_L(const math::Dir3* result, const rigid_body* rb,
                               const math::Dir3* t, float delta_t) {
    if ((~(rb->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body_internal.h", 30,
                  "rb->debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    ((math::Dir3*)result)->v = _mm_mul_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 0), rb->m_world_inv_inertia.x.v),
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 85), rb->m_world_inv_inertia.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 170), rb->m_world_inv_inertia.z.v)),
        _mm_shuffle_ps(_mm_set_ss(delta_t), _mm_set_ss(delta_t), 0));
    return result;
}

// mul_inv_L - ea: 0x895130
inline const math::Dir3* mul_inv_L(const math::Dir3* result, const rigid_body* rb,
                                   const math::Dir3* t) {
    if ((~(rb->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:/cod/code/tl/physics/include\\rigid_body_internal.h", 41,
                  "rb->debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    math::Dir3 v3;
    v3.v = rb->m_mat.y.v;
    math::Dir3 v4;
    v4.v = rb->m_mat.z.v;
    __m128 v6 = _mm_shuffle_ps(rb->m_mat.x.v, v3.v, 68);
    __m128 v7 = _mm_mul_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 0), _mm_shuffle_ps(v6, v4.v, 136)),
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 85), _mm_shuffle_ps(v6, v4.v, 221))),
            _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 170),
                       _mm_shuffle_ps(_mm_shuffle_ps(rb->m_mat.x.v, v3.v, 238), v4.v, 168))),
        rb->m_inv_inertia.v);
    ((math::Dir3*)result)->v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 0), rb->m_mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v7, v7, 85), v3.v)),
        _mm_mul_ps(_mm_shuffle_ps(v7, v7, 170), v4.v));
    return result;
}

// euler_integrate_velocity - ea: 0x895350
inline void euler_integrate_velocity(rigid_body* const rb, float delta_t) {
    float inv_mass_dt = rb->m_inv_mass * delta_t;
    math::Dir3 v4;
    v4.v = rb->m_force_sum.v;
    rb->m_last_t_vel.v = rb->m_t_vel.v;
    rb->m_last_a_vel.v = rb->m_a_vel.v;
    rb->m_t_vel.v = _mm_add_ps(
        rb->m_t_vel.v,
        _mm_mul_ps(v4.v, _mm_shuffle_ps(_mm_set_ss(inv_mass_dt), _mm_set_ss(inv_mass_dt), 0)));
    math::Dir3 v8;
    rb->m_a_vel.v = _mm_add_ps(rb->m_a_vel.v, inv_L(&v8, rb, &rb->m_torque_sum, delta_t)->v);
}

// euler_integrate_pos - ea: 0x895410
inline void euler_integrate_pos(rigid_body* const rb, float delta_t);

// update_stability - ea: 0x892500
inline void update_stability(rigid_body* const rb, float delta_t) {
    __m128 v3 = _mm_mul_ps(rb->m_t_vel.v, rb->m_t_vel.v);
    float v8 = v3.m128_f32[0] + _mm_shuffle_ps(v3, v3, 85).m128_f32[0] +
               _mm_shuffle_ps(v3, v3, 170).m128_f32[0];
    __m128 v4 = _mm_mul_ps(rb->m_a_vel.v, rb->m_a_vel.v);
    v4.m128_f32[0] = v4.m128_f32[0] + _mm_shuffle_ps(v4, v4, 85).m128_f32[0] +
                     _mm_shuffle_ps(v4, v4, 170).m128_f32[0] + v8;
    rb->m_stable_te = v4.m128_f32[0];
    if (v4.m128_f32[0] <= 141.61f) {
        unsigned int m_flags = rb->m_flags;
        if ((m_flags & 4) == 0) {
            float v7 = rb->m_stable_energy_time + delta_t;
            rb->m_stable_energy_time = v7;
            if (v7 >= 0.5f && rb->m_contact_count >= rb->m_stable_min_contact_count)
                rb->m_flags = m_flags | 4;
        }
    } else {
        rb->m_flags &= 0xFFFFFFFB;
        rb->m_stable_energy_time = 0.0f;
    }
}

// setup_constraint - ea: 0x897870
inline void setup_constraint(rigid_body* rb, pulse_sum_node* psn);

// substep - ea: 0x895210
inline void substep(user_rigid_body* rb, float delta_t);

// prolog_frame_advance (rigid_body) - ea: 0x88EBC0
inline void prolog_frame_advance(rigid_body* rb, const outer_time* outside_delta_t) {
    float m_time = rb->m_time_scale.m_time * outside_delta_t->m_time;
    float avel_sq = rb->m_a_vel.v.m128_f32[0] * rb->m_a_vel.v.m128_f32[0] +
                    rb->m_a_vel.v.m128_f32[1] * rb->m_a_vel.v.m128_f32[1] +
                    rb->m_a_vel.v.m128_f32[2] * rb->m_a_vel.v.m128_f32[2];
    float max_avel_sq = rb->m_max_avel * rb->m_max_avel;
    if (avel_sq > max_avel_sq) {
        float scale = rb->m_max_avel / sqrtf(avel_sq) - 1.0f;
        math::Dir3 v10;
        v10.v = _mm_mul_ps(rb->m_a_vel.v, _mm_shuffle_ps(_mm_set_ss(scale), _mm_set_ss(scale), 0));
        math::Dir3 v9;
        rb->m_torque_sum.v = _mm_add_ps(rb->m_torque_sum.v, mul_L(&v9, rb, &v10)->v);
    }
    __m128 m_time_low = _mm_shuffle_ps(_mm_set_ss(m_time), _mm_set_ss(m_time), 0);
    rb->m_force_sum.v = _mm_div_ps(rb->m_force_sum.v, m_time_low);
    rb->m_torque_sum.v = _mm_div_ps(rb->m_torque_sum.v, m_time_low);
    if (rb->m_inv_mass <= 0.0001f &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 167,
                  "rb->get_inv_mass() > .0001f", ""))
        __debugbreak();
    float grav = (rb->m_gravity_multiplier / rb->m_inv_mass) * 9.8000002f;
    rb->m_force_sum.v = _mm_add_ps(
        rb->m_force_sum.v,
        _mm_mul_ps(rb->m_gravity_dir.v, _mm_shuffle_ps(_mm_set_ss(grav), _mm_set_ss(grav), 0)));
}

// prolog_frame_advance (user_rigid_body) - ea: 0x88B130
inline void prolog_frame_advance(user_rigid_body* rb, const outer_time* outside_delta_t) {
    if ((rb->m_flags & 0x20) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 120,
                  "rb->is_user_rigid_body()", ""))
        __debugbreak();
    if (rb->m_dictator == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 121,
                  "rb->m_dictator", ""))
        __debugbreak();
    nuge::calc_velocities(&rb->m_mat, rb->m_dictator,
                          rb->m_time_scale.m_time * outside_delta_t->m_time,
                          &rb->m_t_vel, &rb->m_a_vel);
}

// take_next_step - ea: 0x88B1C0
inline void take_next_step(user_rigid_body* rb, const outer_time*) {
    if ((rb->m_flags & 0x20) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 147,
                  "rb->is_user_rigid_body()", ""))
        __debugbreak();
    if (rb->m_dictator == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 148,
                  "rb->m_dictator", ""))
        __debugbreak();
    rb->m_mat = rb->m_col_mat;
}

// take_last_step - ea: 0x88B2A0
inline void take_last_step(user_rigid_body* rb, const outer_time*) {
    if ((rb->m_flags & 0x20) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 154,
                  "rb->is_user_rigid_body()", ""))
        __debugbreak();
    if (rb->m_dictator == NULL &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 155,
                  "rb->m_dictator", ""))
        __debugbreak();
    rb->m_mat = *rb->m_dictator;
}

// epilog_frame_advance - ea: 0x88B380
inline void epilog_frame_advance(rigid_body* const rb) {
    rb->m_force_sum.v = Float4_Zero_212.v;
    rb->m_torque_sum.v = Float4_Zero_212.v;
}

// mul_L - ea: 0x88E8F0
inline math::Dir3* mul_L(math::Dir3* result, const rigid_body* rb,
                         const math::Dir3* t) {
    if ((~(rb->m_flags >> 6) & 1) == 0 &&
        _tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 76,
                  "rb->debug_flag_is_not_in_collision()", ""))
        __debugbreak();
    math::Dir3 v3;
    v3.v = rb->m_inv_inertia.v;
    math::Dir3 v4;
    v4.v = rb->m_mat.y.v;
    __m128 v5 = _mm_rcp_ps(v3.v);
    math::Dir3 v6;
    v6.v = rb->m_mat.z.v;
    __m128 v7 = _mm_mul_ps(_mm_sub_ps(Float4_Two_212.v, _mm_mul_ps(v5, v3.v)), v5);
    __m128 v9 = _mm_shuffle_ps(rb->m_mat.x.v, v4.v, 68);
    __m128 v10 = _mm_mul_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 0), _mm_shuffle_ps(v9, v6.v, 136)),
                _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 85), _mm_shuffle_ps(v9, v6.v, 221))),
            _mm_mul_ps(_mm_shuffle_ps(t->v, t->v, 170),
                       _mm_shuffle_ps(_mm_shuffle_ps(rb->m_mat.x.v, v4.v, 238), v6.v, 168))),
        v7);
    result->v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v10, v10, 0), rb->m_mat.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v10, v10, 85), v4.v)),
        _mm_mul_ps(_mm_shuffle_ps(v10, v10, 170), v6.v));
    return result;
}

// calc_col_mat (user_rigid_body) - ea: 0x88E9F0
inline void calc_col_mat(user_rigid_body* rb, const outer_time* outside_delta_t);

// get_dictator - ea: 0x87E9A0
inline const math::Mat43* get_dictator(const user_rigid_body* rb) {
    if ((rb->m_flags & 0x20) != 0)
        return rb->m_dictator;
    bool v1 = !_tlAssert("c:\\cod\\code\\tl\\physics\\include\\rigid_body_internal.h", 114,
                         "rb->is_user_rigid_body()", "");
    const math::Mat43* result = rb->m_dictator;
    if (!v1)
        __debugbreak();
    return result;
}

// constraint_info_reset - ea: 0x87E9E0
inline void constraint_info_reset(rigid_body* rb) {
    rb->m_constraint_count = 0;
    rb->m_contact_count = 0;
}

// increment_constraint_count - ea: 0x87EA00
inline void increment_constraint_count(rigid_body* rb) {
    ++rb->m_constraint_count;
}

// increment_contact_count - ea: 0x87EA10
inline void increment_contact_count(rigid_body* rb, int c) {
    rb->m_contact_count += c;
}
}

// ============================================================================
// rbint — rigid-body intrinsic math (methods in phys_util.o, unresolved)
// ============================================================================
namespace rbint {
const math::Dir3* multiply(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* collide_multiply(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* add_pos(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* collide_add_pos(const math::Dir3* result, rigid_body* b, const math::Dir3* r);
const math::Dir3* sub_pos(const math::Dir3* result, rigid_body* b, const math::Dir3* p);
}

// ============================================================================
// rbcint â€” constraint base helpers (get_time_scale in rbc_def_generic.o)
// ============================================================================
namespace rbcint {
const outer_time* get_time_scale(rigid_body_constraint* rbc);

// process_constraint_info - ea: 0x87EA70
inline void process_constraint_info(rigid_body_constraint* rbc) {
    if (rbc->b1 != NULL)
        ++rbc->b1->m_constraint_count;
    rigid_body* b2 = rbc->b2;
    if (b2 != NULL)
        ++b2->m_constraint_count;
}

// process_constraint_info (wheel) - ea: 0x87EAA0
inline void process_constraint_info(rigid_body_constraint_wheel* rbc_wheel) {
    if (rbc_wheel->b1 != NULL)
        ++rbc_wheel->b1->m_constraint_count;
    rigid_body* b2 = rbc_wheel->b2;
    if (b2 != NULL)
        ++b2->m_constraint_count;
    if ((rbc_wheel->m_wheel_flags & 1) != 0) {
        if (rbc_wheel->b1 != NULL)
            ++rbc_wheel->b1->m_contact_count;
        rigid_body* v2 = rbc_wheel->b2;
        if (v2 != NULL)
            ++v2->m_contact_count;
    }
}

// set - ea: 0x87EAF0
inline void set(rigid_body_constraint* rbc, rigid_body* const b1, rigid_body* const b2) {
    rbc->b1 = b1;
    rbc->b2 = b2;
}

// set_next - ea: 0x88B480
inline void set_next(rigid_body_constraint* rbc, rigid_body_constraint* next) {
    rbc->m_next = next;
}
}

// ============================================================================
// nuge â€” rigid-body aggregate utilities (phys_util.o)
// ============================================================================
namespace nuge {
void calc_velocities(const math::Mat43* mat0, const math::Mat43* mat1, float delta_t,
                     math::Dir3* t_vel, math::Dir3* a_vel);
}

extern const math::Dir3& Float4_Zero_213;
extern const math::Dir3& Float4_SignMask_213;
extern const math::Dir3& Float4_Zero_210;
extern const math::Dir3& Float4_SignMask_210;
extern const math::Dir3& Float4_Zero_212;
extern const math::Dir3& Float4_Two_212;

// ============================================================================
// phys_list_condition functors (inline COMDATs, physics_system.o).
// ============================================================================
struct phys_list_condition_functor_has_rigid_body {
    rigid_body* const m_rb;  // +0x00

    phys_list_condition_functor_has_rigid_body(rigid_body* const rb) : m_rb(rb) {}

    bool test(const rigid_body_constraint* c) {  // ea: 0x880580
        if (c->b1 == NULL || c->b1 != m_rb) {
            rigid_body* b2 = c->b2;
            if (b2 == NULL || b2 != m_rb)
                return false;
        }
        return true;
    }
};

struct phys_list_condition_functor_has_user_rigid_body {
    phys_list_condition_functor_has_user_rigid_body() {}

    bool test(const rigid_body_constraint* c) {  // ea: 0x8805B0
        if (c->b1 == NULL || (c->b1->m_flags & 0x20) == 0) {
            rigid_body* b2 = c->b2;
            if (b2 == NULL || (b2->m_flags & 0x20) == 0)
                return false;
        }
        return true;
    }
};

struct phys_list_condition_functor_has_rigid_body_and_user_rigid_body {
    rigid_body* const m_rb;  // +0x00

    phys_list_condition_functor_has_rigid_body_and_user_rigid_body(rigid_body* const rb)
        : m_rb(rb) {}

    bool test(const rigid_body_constraint* c) {  // ea: 0x880600
        rigid_body* b1 = c->b1;
        if (b1 != NULL) {
            rigid_body* b2 = c->b2;
            if (b2 != NULL) {
                if (b1 == m_rb && (b2->m_flags & 0x20) != 0)
                    return true;
                if (b2 == m_rb && (b1->m_flags & 0x20) != 0)
                    return true;
            }
        }
        return false;
    }
};

struct phys_list_condition_functor_has_no_constraints {
    phys_list_condition_functor_has_no_constraints() {}

    bool test(const rigid_body* rb) {  // ea: 0x880640
        return rb->m_constraint_count == 0;
    }
};

// ============================================================================
// Contact-manifold helpers (phys_contact_manifold.o / phys_util.o)
// ============================================================================
extern bool phys_v2_le(const math::Dir3& v1, const math::Dir3& v2);
extern const float& phys_v2_cross(const math::Dir3& v1, const math::Dir3& v2);
extern const math::Dir3* phys_v2_rotr(const math::Dir3& result, const math::Dir3& v);
extern const math::Dir3* phys_v3_to_v2_inv_multiply(const math::Dir3* result,
                                                    const math::Mat43* m, const math::Dir3* v);
extern void displace_contact_p(contact_manifold_mesh_point** mp, const math::Dir3& d,
                               const math::Mat43* contact_mat);
extern const char* g_contact_manifold_error_msg;

// ============================================================================
// GJK constants and helpers
// ============================================================================
extern float PEN_THRESH;     // 0xE53EE0
extern float CONV_THRESH;    // 0xE53EE4
extern float SEP_CONV_THRESH; // 0xE53EE8
extern const math::Dir3& Float4_SignMask_203;
extern void phys_full_inv_multiply_mat(math::Mat43& dest_m, const math::Mat43& left_m,
                                       const math::Mat43& right_m);

#endif // COD3_PHYSICS_PULSE_SUM_H
