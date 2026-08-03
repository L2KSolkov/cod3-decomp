// ============================================================================
// bdAlignedOffsetMemory — aligned allocator with configurable offset
// Reconstructed from COD3 release decompilation.
// ea: 0x8A03D0 (bdAlignedOffsetMalloc), 0x8A04A0 (bdAlignedOffsetFree), 0x8A04B0 (bdAlignedOffsetRealloc)
// ============================================================================

#include <cstdlib>
#include <cstring>

typedef unsigned int bdUWord;

// COD3: alignment must be power of 2 (enforced at ea:0x8A03D0)
// Layout: [blockPtr][padding...][offset][userData] where blockPtr is stored before userData

void* bdAlignedOffsetMalloc(bdUWord size, bdUWord align, bdUWord offset) {
    // Check alignment is power of 2
    if ((align - 1) & align)
        return nullptr;

    bdUWord total = size + align + offset + sizeof(void*);
    void* raw = malloc(total);
    if (!raw)
        return nullptr;

    // Align (raw + sizeof(void*) + offset) up to 'align' boundary
    bdUWord aligned = (~(align - 1)) & ((bdUWord)(uintptr_t)raw + sizeof(void*) + align + offset - 1);
    bdUWord userPtr = aligned - offset;

    // Store original block pointer behind user data (at userPtr - sizeof(void*))
    *((void**)(userPtr - sizeof(void*))) = raw;

    return (void*)userPtr;
}

// ea: 0x8A04A0 — free: retrieves stored block pointer at (ptr - sizeof(void*)), calls free()
void bdAlignedOffsetFree(void* ptr) {
    if (!ptr) return;
    void* raw = *((void**)((uintptr_t)ptr - sizeof(void*)));
    free(raw);
}

// ea: 0x8A04B0 — realloc: allocate new, copy min(oldSize, newSize), free old
void* bdAlignedOffsetRealloc(void* ptr, bdUWord oldSize, bdUWord newSize, bdUWord align, bdUWord offset) {
    void* newPtr = bdAlignedOffsetMalloc(newSize, align, offset);
    if (newPtr && ptr) {
        bdUWord copy = (newSize < oldSize) ? newSize : oldSize;
        memcpy(newPtr, ptr, copy);
        bdAlignedOffsetFree(ptr);
    }
    return newPtr;
}
