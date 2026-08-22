// ============================================================================
// anim_heap.cpp - AnimHeap (core.o)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"
#include "core/mem_heap.h"
#include "core/memory_types.h"

extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_align(unsigned int alignment, unsigned int size);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void mem_heap_free_ctx(void* heap, void* ptr);
extern int gPakHeaps_m_size;
void* gPakHeaps_elements[32];  // ?gPakHeaps_elements (core.o)

class AnimCacheHeap : public ae_heap_base {
public:
    AnimCacheHeap();
    void* Malloc(unsigned int size, unsigned int alignment);
    void Free(void* ptr);
    bool CheckFree(void* ptr);
};
static_assert(sizeof(AnimCacheHeap) == 4, "AnimCacheHeap size mismatch");

extern "C" void* nalAnimationCache_MemAlloc(unsigned int size,
                                               unsigned int formal);
extern "C" void nalAnimationCache_MemFree(void* ptr, unsigned int size);

class AnimCacheHeapVtableAdapter {
public:
    virtual ~AnimCacheHeapVtableAdapter() {}
    virtual void* Malloc(unsigned int size, unsigned int alignment)
    {
        return reinterpret_cast<AnimCacheHeap*>(this)->Malloc(size, alignment);
    }
    virtual void Free(void* ptr)
    {
        reinterpret_cast<AnimCacheHeap*>(this)->Free(ptr);
    }
    virtual bool CheckFree(void* ptr)
    {
        return reinterpret_cast<AnimCacheHeap*>(this)->CheckFree(ptr);
    }
    virtual mem_heap* GetHeapPointer()
    {
        return nullptr;
    }
};
static AnimCacheHeapVtableAdapter s_animCacheHeapVtableAdapter;

// ea: 0x004DF390
AnimCacheHeap::AnimCacheHeap()
{
    __vftable = *reinterpret_cast<ae_heap_base_vtbl**>(
        &s_animCacheHeapVtableAdapter);
}

// ea: 0x004DF3A0
void* AnimCacheHeap::Malloc(unsigned int size, unsigned int alignment)
{
    return nalAnimationCache_MemAlloc(size, alignment);
}

// ea: 0x004DF3C0
void AnimCacheHeap::Free(void* ptr)
{
    unsigned int size = (reinterpret_cast<unsigned int*>(ptr)[-1]
                         & 0xFFFFFFFCu);
    nalAnimationCache_MemFree(ptr, size);
}

// ea: 0x004DF3F0
bool AnimCacheHeap::CheckFree(void* ptr)
{
    const uintptr_t block = reinterpret_cast<uintptr_t>(
        AnimHeapStatics::sInst->mBlock);
    const uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    if (address < block || address >= block + 0x100000u)
        return false;
    Free(ptr);
    return true;
}

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
    AnimCacheHeap* v1 = nullptr;
    void* memory = mem_heap_malloc(4u);
    if (memory != nullptr)
        v1 = new (memory) AnimCacheHeap();
    gPakHeaps_elements[gPakHeaps_m_size] = v1;
    ++gPakHeaps_m_size;
}
