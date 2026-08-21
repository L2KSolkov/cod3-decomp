// ============================================================================
// bdAlignedOffsetMemory — aligned allocator with configurable offset
// Reconstructed from COD3 release decompilation.
// ea: 0x8A03D0 (bdAlignedOffsetMalloc), 0x8A04A0 (bdAlignedOffsetFree), 0x8A04B0 (bdAlignedOffsetRealloc)
// ============================================================================

#include "bd/bd_types.h"

#include <cstdlib>
#include <cstring>

typedef unsigned int bdUWord;

extern const char defaultFileName[];

// ea: 0x8A03C0
void* bdMalloc(unsigned int nSize) {
    return malloc(nSize);
}

void* bdAlignedOffsetMalloc(bdUWord size, bdUWord align, bdUWord offset) {
    const bdUWord mask = align - 1;
    if ((mask & align) != 0) {
        do {
            bdMessageProxy proxy(
                ".\\bdMemory\\bdAlignedOffsetMemory.cpp",
                "void *__cdecl bdAlignedOffsetMalloc(__w64 const unsigned int,__w64 const unsigned int,__w64 const unsigned int)",
                0x15u, "dw/err");
            proxy.log(defaultFileName,
                      "bdAlignedOffsetMalloc, alignment must a power of 2.");
        } while (g_assertFalse);
        return nullptr;
    }

    const bdUWord extra = align + offset + 4;
    void* raw = malloc(extra + size);
    if (raw != nullptr) {
        const uintptr_t aligned = (~(uintptr_t)mask)
            & (reinterpret_cast<uintptr_t>(raw) + extra);
        const uintptr_t user = aligned - offset;
        *reinterpret_cast<void**>(user - 4) = raw;
        if ((mask & static_cast<bdUWord>(aligned)) != 0) {
            do {
                bdMessageProxy proxy(
                    ".\\bdMemory\\bdAlignedOffsetMemory.cpp",
                    "void *__cdecl bdAlignedOffsetMalloc(__w64 const unsigned int,__w64 const unsigned int,__w64 const unsigned int)",
                    0x2Bu, "dw/err");
                proxy.log(defaultFileName,
                          "bdAlignedOffsetMalloc, incorrect alignment.");
            } while (g_assertFalse);
        }
        return reinterpret_cast<void*>(user);
    }
    return raw;
}

void bdAlignedOffsetFree(void* ptr) {
    free(*reinterpret_cast<void**>(static_cast<unsigned char*>(ptr) - 4));
}

void* bdAlignedOffsetRealloc(void* ptr, bdUWord oldSize, bdUWord newSize, bdUWord align, bdUWord offset) {
    void* newPtr = bdAlignedOffsetMalloc(newSize, align, offset);
    bdUWord copy = oldSize;
    if (newSize < oldSize)
        copy = newSize;
    memcpy(newPtr, ptr, copy);
    free(*reinterpret_cast<void**>(static_cast<unsigned char*>(ptr) - 4));
    return newPtr;
}
