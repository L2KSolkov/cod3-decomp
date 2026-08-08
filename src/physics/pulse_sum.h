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
    void  setup_vel_uni_standard(float delta_t, float max_penalty_restitution_vel);
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

    bool is_member(const T* data) const;
    int  get_max_slots() const { return m_slot_array_size; }
    int  get_available_slots() const { return m_slot_array_size - m_alloc_count; }
    int  get_used_slots() const { return m_alloc_count; }
    T*   add(bool no_error, const char* error_msg);
    void remove(T* data);
    void remove_all() { reset_buffer(); }
    void reset_buffer();
    void call_destructors() {}

    struct iterator {
        T** m_ptr;  // +0x00

        iterator(T** ptr) : m_ptr(ptr) {}
        T& operator*() const { return **m_ptr; }
        T* operator->() const { return *m_ptr; }
        iterator& operator++() { ++m_ptr; return *this; }
        bool operator!=(const iterator& other) const { return m_ptr != other.m_ptr; }
        iterator next_after_remove() const { return iterator(m_ptr); }
    };

    iterator begin() { return iterator(m_alloc_list); }
    iterator end() { return iterator(&m_alloc_list[m_alloc_count]); }
};
static_assert(sizeof(phys_heap_memory_pool<int>) == 0x14, "phys_heap_memory_pool size mismatch");

// ============================================================================
// physics_system - the physics engine singleton (864 bytes).
// Layout verified against IDA local types (ordinal 7028).
// ============================================================================
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

extern physics_system* g_physics_system;  // ?g_physics_system@@3PAVphysics_system@@A
extern void verify_is_in_physics_system(rigid_body_constraint_contact* rbc,
                                        rigid_body* b1_, rigid_body* b2_);
extern void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m);
extern void SetIdentity(math::Mat43& m);

namespace rbint {
void calc_col_mat(rigid_body* rb, const outer_time* outside_delta_t);

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

extern void make_rotate(math::Mat43* mat, const math::Dir3* v, float theta_factor);
extern void make_rotate(math::Mat43* mat, const math::Dir3* v1, const math::Dir3* v2);

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
