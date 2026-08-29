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
bool Warning(const char* fmt, ...);
}

// ae_sized_array_base<T,CAPACITY> stores the element backing region.  The
// DbLinkedHandle specialization in game_types.h uses the pointer/raw-storage
// layout emitted by the original engine for non-trivial handle elements.
template <typename T, int CAPACITY>
class ae_sized_array_base {
public:
    T m_elements[CAPACITY];  // +0x00

    // ea: 0x005EAC70
    // ea: 0x005EEBF0
    // ea: 0x004B16A0
    ae_sized_array_base() {}
};

template <typename T, int CAPACITY>
class ae_sized_array : public ae_sized_array_base<T, CAPACITY> {
public:
    int     m_size;                // +sizeof(T)*CAPACITY

    // ea: 0x005EA480
    // ea: 0x005EEB20
    // ea: 0x004AC550
    // ea: 0x004AD0F0
    // ea: 0x004ADFD0
    // ea: 0x004B1450
    // ea: 0x004B1470
    // ea: 0x004B14D0
    // ea: 0x004B2BF0
    ae_sized_array() : m_size(0) {}

    // ea: 0x005EA1A0
    // ea: 0x005EA680
    // ea: 0x005EA8A0
    // ea: 0x005EABB0
    // ea: 0x004AC520
    // ea: 0x004AD180
    // ea: 0x004ADDB0
    // ea: 0x004AE060
    // ea: 0x004AC5E0
    int size() const { return m_size; }
    // ea: 0x005EA490
    bool empty() const { return m_size == 0; }
    // ea: 0x005EEAD0
    void clear() { m_size = 0; }

    // ea: 0x005EA120
    // ea: 0x004AC4A0
    // ea: 0x004AC560
    // ea: 0x004AD100
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

    // ea: 0x005EA820
    // ea: 0x004ACFF0
    // ea: 0x004AD070
    // ea: 0x004ADFE0
    T& operator[](unsigned int idx) {
        if (idx >= static_cast<unsigned int>(CAPACITY)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        return m_elements[idx];
    }

    // ea: 0x004ADD30
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

    // ea: 0x005EA1B0
    // ea: 0x004AD190
    // ea: 0x004ADDC0
    // ea: 0x004AE070
    // ea: 0x005EA690
    // ea: 0x005EA4B0
    // ea: 0x004AC5F0
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
    // ea: 0x005EA970
    // ea: 0x004AE6D0
    int capacity() const { return CAPACITY; }
    // ea: 0x005EE950
    // ea: 0x004B14F0
    int free_space() const { return CAPACITY - m_size; }

    // ea: 0x005EA540
    // ea: 0x005EA980
    // ea: 0x004AC690
    // ea: 0x004AE100
    T& pop_back() {
        if (m_size != 0)
            --m_size;
        return m_elements[m_size];
    }
    T& expand() {
        if (m_size >= CAPACITY) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 192;
            AeAssert::gCurrentExpr = "m_size < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("no room left in array"))
                __debugbreak();
        }
        int size = m_size;
        m_size = size + 1;
        return m_elements[size];
    }
    // ea: 0x005EABC0
    void set_size(int size) { m_size = size; }
    void erase(int idx) {
        int size = m_size;
        if (size > 1 && idx < size)
            m_elements[idx] = m_elements[size - 1];
        if (m_size != 0)
            --m_size;
    }
    // ea: 0x005EEAE0
    // ea: 0x004B1660
    // ea: 0x005EED00
    T& back() { return m_elements[(m_size - 1 <= 0) ? 0 : m_size - 1]; }  // ?back@?$ae_sized_array@H$0CN@@@QAEAAHXZ (g.o 0x4B1660)
    // ea: 0x005EE7D0
    const T& back() const { return m_elements[(m_size - 1 <= 0) ? 0 : m_size - 1]; }

    // Binary mangle: const_iterator@?$ae_sized_array@...@@ (Entity* const*)
    class const_iterator {
    public:
        friend class ae_sized_array;
        const T* m_ptr;  // +0x00
        const_iterator() : m_ptr(nullptr) {}
    private:
        // ea: 0x005EAC80
        // ea: 0x004AE5D0
        const_iterator(const T* ptr) : m_ptr(ptr) {}  // ??0const_iterator@...@@AAE@PBQAVEntity@@@Z (g.o 0x4AE5D0)
    public:
        // ea: 0x005EA730
        // ea: 0x004ACFB0
        const T& operator*() const { return *m_ptr; }  // ??Dconst_iterator@...@@QBEAB...@@XZ
        const T* operator->() const { return m_ptr; }
        // ea: 0x005EA740
        // ea: 0x004ACFC0
        const_iterator& operator++() { ++m_ptr; return *this; }  // ??Econst_iterator@...@@QAEAAV01@XZ
        // ea: 0x005EA750
        // ea: 0x004ACFD0
        bool operator!=(const_iterator rhs) const { return m_ptr != rhs.m_ptr; }  // ??9const_iterator@...@@QBE_NV01@@Z
    };

    class iterator {
    public:
        friend class ae_sized_array;
        T* m_ptr;  // +0x00
        iterator() : m_ptr(nullptr) {}
    private:
    // ea: 0x004AE610
        // ea: 0x004AE650
        iterator(T* ptr) : m_ptr(ptr) {}  // ??0iterator@...@@AAE@PAPAVEntity@@@Z (g.o 0x4AE610)
    public:
        // ea: 0x004AD230
        // ea: 0x004AD280
        T& operator*() const { return *m_ptr; }  // ??Diterator@...@@QBEAAPAV...@@XZ / QBEAAV...@@XZ
        T* operator->() const { return m_ptr; }
        // ea: 0x004AD240
        // ea: 0x004AD290
        iterator& operator++() { ++m_ptr; return *this; }  // ??Eiterator@...@@QAEAAV01@XZ
        // ea: 0x004AD250
        // ea: 0x004AD2A0
        bool operator!=(iterator rhs) const { return m_ptr != rhs.m_ptr; }  // ??9iterator@...@@QBE_NV01@@Z
    };

    // ea: 0x004B0EE0
    // ea: 0x004B1490
    iterator begin() { return iterator(m_elements); }  // ?begin@...@@QAE?AViterator@1@XZ
    // ea: 0x004B0EF0
    // ea: 0x004B14A0
    iterator end() { return iterator(&m_elements[m_size]); }  // ?end@...@@QAE?AViterator@1@XZ
    // ea: 0x004B0F10
    // ea: 0x005EEB40
    const_iterator begin() const { return const_iterator(m_elements); }  // ?begin@...@@QBE?AVconst_iterator@1@XZ
    // ea: 0x004B0F20
    // ea: 0x005EEB60
    const_iterator end() const { return const_iterator(&m_elements[m_size]); }  // ?end@...@@QBE?AVconst_iterator@1@XZ
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
