// ============================================================================
// anim_heap.cpp - AnimHeap (core.o)
// ============================================================================

#include "game/core/core_systems.h"
#include "core/mem_heap.h"

extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_align(unsigned int alignment, unsigned int size);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void mem_heap_free_ctx(void* heap, void* ptr);
extern int gPakHeaps_m_size;
void* gPakHeaps_elements[32];  // ?gPakHeaps_elements (core.o)

// ea: 0x004BD660
void* AnimHeap::Allocate(int size)
{
    unsigned int used_byte = *(unsigned int*)&mHeap[0x488];
    if (size + used_byte <= 0x100000)
        return mem_heap_malloc(reinterpret_cast<mem_heap*>(mHeap), 16, size);
    return nullptr;
}

// ea: 0x004BD690
void AnimHeap::Free(void* ptr, int size)
{
    (void)size;
    mem_heap_free(reinterpret_cast<mem_heap*>(mHeap), ptr);
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
