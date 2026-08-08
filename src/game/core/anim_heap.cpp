// ============================================================================
// anim_heap.cpp - AnimHeap (core.o)
// ============================================================================

#include "game/core/core_systems.h"

extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_align(unsigned int alignment, unsigned int size);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void mem_heap_free_ctx(void* heap, void* ptr);
extern int gPakHeaps_m_size;
extern void* gPakHeaps_elements[32];

// AnimHeap.mHeap is a mem_heap (0x49C bytes); expose minimal ops.
extern void* mem_heap_alloc(void* heap, unsigned int alignment,
                            unsigned int size);
extern void mem_heap_release(void* heap, void* ptr);

// ea: 0x004BD660
void* AnimHeap::Allocate(unsigned int size)
{
    unsigned int used_byte = *(unsigned int*)&mHeap[0x488];
    if (size + used_byte <= 0x100000)
        return mem_heap_alloc(&mHeap, 16, size);
    return nullptr;
}

// ea: 0x004BD690
void AnimHeap::Free(void* ptr, int size)
{
    (void)size;
    mem_heap_release(&mHeap, ptr);
}

// ea: 0x004C13F0
void AnimHeap::LinkAnimHeap()
{
    void* v1 = mem_heap_malloc(4u);
    if (v1 == nullptr)
        v1 = nullptr;
    gPakHeaps_elements[gPakHeaps_m_size] = v1;
    ++gPakHeaps_m_size;
}
