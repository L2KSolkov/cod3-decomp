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
class ae_heap_base {
public:
    ae_heap_base();
    virtual ~ae_heap_base();
    virtual mem_heap* GetHeapPointer();

protected:
    // ?MemAlloc@ae_heap_base@@IAEPAXIIPAUmem_heap@@@Z (ae_heap.o 0x7BBEE0)
    void* MemAlloc(unsigned int size, unsigned int align, mem_heap* heap);
    // ?MemFree@ae_heap_base@@IAEXPAXPAUmem_heap@@@Z (ae_heap.o 0x7BBF20)
    void MemFree(void* ptr, mem_heap* heap);
    // ?MemCheckFree@ae_heap_base@@IAE_NPAXPAUmem_heap@@@Z (ae_heap.o 0x7BBF40)
    bool MemCheckFree(void* ptr, mem_heap* heap);
};
static_assert(sizeof(ae_heap_base) == 4, "ae_heap_base size mismatch");

// ============================================================================
// ae_heap_wrapper — heap wrapper (8 bytes)
// Size: 0x08 (8 bytes) — verified against IDA
// ============================================================================
class ae_heap_wrapper : public ae_heap_base {
public:
    mem_heap* mHeap;  // +0x04

    // ?ae_heap_wrapper@ae_heap_wrapper@@QAE@PAUmem_heap@@@Z (streamer.o 0x684D70)
    explicit ae_heap_wrapper(mem_heap* heap);
    virtual ~ae_heap_wrapper();
    // ?Malloc@ae_heap_wrapper@@UAEPAXII@Z (streamer.o 0x684D90)
    virtual void* Malloc(unsigned int size, unsigned int align);
    // ?Free@ae_heap_wrapper@@UAEXPAX@Z (streamer.o 0x684DB0)
    virtual void Free(void* ptr);
    // ?CheckFree@ae_heap_wrapper@@UAE_NPAX@Z (streamer.o 0x684DD0)
    virtual bool CheckFree(void* ptr);
    // ?GetHeapPointer@ae_heap_wrapper@@UAEPAUmem_heap@@XZ (streamer.o 0x684DF0)
    virtual mem_heap* GetHeapPointer();
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

    // ?Alloc@?$cFreeList@VEntity@@@@QAEPAVEntity@@XZ etc. (core.o)
    T* Alloc()
    {
        T* p = mpFree;
        if (p != nullptr)
        {
            mpFree = *(T**)p;
            ++mUsed;
            --mFree;
        }
        return p;
    }
};
static_assert(sizeof(cFreeList<char>) == 0x0C, "cFreeList size mismatch");
