// ============================================================================
// COD3 Memory Types — mem_heap, ae_heap_wrapper, cFreeList
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include <stddef.h>

// ============================================================================
// mem_heap — heap structure (opaque; mem_lib.o owns layout)
// ============================================================================
struct mem_heap;

// ============================================================================
// ae_heap_base — heap interface base (4 bytes, vtable only)
// Size: 0x04 (4 bytes) — verified against IDA
// ============================================================================
struct ae_heap_base {
    struct ae_heap_base_vtbl* __vftable;  // +0x00
};
static_assert(sizeof(ae_heap_base) == 4, "ae_heap_base size mismatch");

// ============================================================================
// ae_heap_wrapper — heap wrapper (8 bytes)
// Size: 0x08 (8 bytes) — verified against IDA
// ============================================================================
struct ae_heap_wrapper : ae_heap_base {
    mem_heap* mHeap;  // +0x04
};
static_assert(sizeof(ae_heap_wrapper) == 8, "ae_heap_wrapper size mismatch");

// ============================================================================
// cFreeList<T> — free-list based object pool (12 bytes)
// Size: 0x0C (12 bytes) — verified against IDA
// ============================================================================
template <typename T>
struct cFreeList {
    T*           mpFree;  // +0x00
    int          mUsed;   // +0x04
    int          mFree;   // +0x08
};
static_assert(sizeof(cFreeList<char>) == 0x0C, "cFreeList size mismatch");
