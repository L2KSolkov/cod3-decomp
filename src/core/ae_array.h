// ============================================================================
// AE (Angel Engine) Array — fixed-capacity sized array
// Source: c:/cod/code/ae/core/ae_array.h
// ============================================================================
// ea: 0x7BD8F9-0x7BD9B4 (usage in PoolAllocator constructor)
// ============================================================================

#pragma once

#include <stdint.h>

namespace AeAssert {
enum ECoderId : int;
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

template <typename T, int CAPACITY>
class ae_sized_array {
public:
    T       m_elements[CAPACITY];  // +0x00
    int     m_size;                // +sizeof(T)*CAPACITY

    ae_sized_array() : m_size(0) {}

    int size() const { return m_size; }

    T& operator[](int idx) {
        if (idx < 0 || idx >= CAPACITY) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        return m_elements[idx];
    }

    const T& operator[](int idx) const {
        if (idx < 0 || idx >= CAPACITY) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 148;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        return m_elements[idx];
    }

    void push_back(const T& val) {
        if (m_size >= CAPACITY) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 174;
            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no room left in array"))
                __debugbreak();
        }
        if (m_size < CAPACITY) {
            m_elements[m_size] = val;
            ++m_size;
        }
    }
    int capacity() const { return CAPACITY; }
    int free_space() const { return CAPACITY - m_size; }

    T& pop_back() {
        if (m_size != 0)
            --m_size;
        return m_elements[m_size];
    }
    T& back() { return m_elements[(m_size - 1 <= 0) ? 0 : m_size - 1]; }  // ?back@?$ae_sized_array@H$0CN@@@QAEAAHXZ (g.o 0x4B1660)

    // Binary mangle: const_iterator@?$ae_sized_array@...@@ (Entity* const*)
    class const_iterator {
    public:
        friend class ae_sized_array;
        const T* m_ptr;  // +0x00
        const_iterator() : m_ptr(nullptr) {}
    private:
        const_iterator(const T* ptr) : m_ptr(ptr) {}  // ??0const_iterator@...@@AAE@PBQAVEntity@@@Z (g.o 0x4AE5D0)
    public:
        const T& operator*() const { return *m_ptr; }  // ??Dconst_iterator@...@@QBEAB...@@XZ
        const_iterator& operator++() { ++m_ptr; return *this; }  // ??Econst_iterator@...@@QAEAAV01@XZ
        bool operator!=(const_iterator rhs) const { return m_ptr != rhs.m_ptr; }  // ??9const_iterator@...@@QBE_NV01@@Z
    };

    class iterator {
    public:
        friend class ae_sized_array;
        T* m_ptr;  // +0x00
        iterator() : m_ptr(nullptr) {}
    private:
        iterator(T* ptr) : m_ptr(ptr) {}  // ??0iterator@...@@AAE@PAPAVEntity@@@Z (g.o 0x4AE610)
    public:
        T& operator*() const { return *m_ptr; }  // ??Diterator@...@@QBEAAPAV...@@XZ / QBEAAV...@@XZ
        iterator& operator++() { ++m_ptr; return *this; }  // ??Eiterator@...@@QAEAAV01@XZ
        bool operator!=(iterator rhs) const { return m_ptr != rhs.m_ptr; }  // ??9iterator@...@@QBE_NV01@@Z
    };

    iterator begin() { return iterator(m_elements); }  // ?begin@...@@QAE?AViterator@1@XZ
    iterator end() { return iterator(&m_elements[m_size]); }  // ?end@...@@QAE?AViterator@1@XZ
    const_iterator begin() const { return const_iterator(m_elements); }  // ?begin@...@@QBE?AVconst_iterator@1@XZ
    const_iterator end() const { return const_iterator(&m_elements[m_size]); }  // ?end@...@@QBE?AVconst_iterator@1@XZ
};

// ae_sized_array_base<T,CAPACITY> - derived from by ae_sized_array when it
// needs a separate m_elementdata storage (m_elements points into it).
template <typename T, int CAPACITY>
class ae_sized_array_base {
public:
    T m_elementdata[CAPACITY];  // +0x00
    T* m_elements;              // +0x4000
    int m_size;                 // +0x4004

    ae_sized_array_base() : m_elements(m_elementdata), m_size(0) {}
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
