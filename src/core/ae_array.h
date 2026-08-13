// ============================================================================
// AE (Angel Engine) Array — fixed-capacity sized array
// Source: c:/cod/code/ae/core/ae_array.h
// ============================================================================
// ea: 0x7BD8F9-0x7BD9B4 (usage in PoolAllocator constructor)
// ============================================================================

#pragma once

#include <stdint.h>

template <typename T, int CAPACITY>
class ae_sized_array {
public:
    T       m_elements[CAPACITY];  // +0x00
    int     m_size;                // +sizeof(T)*CAPACITY

    ae_sized_array() : m_size(0) {}

    int size() const { return m_size; }

    T& operator[](int idx) {
        return m_elements[idx];
    }

    const T& operator[](int idx) const {
        return m_elements[idx];
    }

    void push_back(const T& val) {
        // assert: m_size < CAPACITY
        m_elements[m_size] = val;
        ++m_size;
    }
};

// ============================================================================
// reserved_slist — intrusive singly-linked free list node
// Used by PoolAllocator::BlockPool for free block tracking.
// ea: 0x7BD670-0x7BD6F6 (Pop), 0x7BD700-0x7BD7EF (Push)
// ============================================================================
template <typename T>
struct reserved_slist {
    struct slist_node {
        slist_node* m_next;  // +0x00
    };

    slist_node* m_head;  // +0x00

    reserved_slist() : m_head(nullptr) {}

    bool empty() const { return m_head == nullptr; }
};
