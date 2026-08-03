// ============================================================================
// bdAlignedOffsetMemory — aligned allocator with offset (Demonware 2.0)
// Verified against COD3 ea: 0x8A03D0 (malloc), 0x8A04A0 (free), 0x8A04B0 (realloc)
// ============================================================================

#include "bdUtilities/bdBitOperations.h"
#include <cstdlib>
#include <cstring>
#include <cstdint>

#define BD_CALL
#define BD_NULL nullptr

typedef unsigned int bdUWord;

void* BD_CALL bdAlignedOffsetMalloc(bdUWord size, bdUWord align, bdUWord offset) {
    if (!BD_IS_POWER_OF_2(align)) return BD_NULL;

    bdUWord padding = align + sizeof(void*) + offset;
    bdUWord blockPtr = (bdUWord)(uintptr_t)malloc(padding + size);
    if (!blockPtr) return BD_NULL;

    bdUWord alignedPtr = BD_PREVIOUS_MULTIPLE_OF_M(blockPtr + padding, align);
    bdUWord dataPtr = alignedPtr - offset;

    *((bdUWord*)(dataPtr - sizeof(void*))) = blockPtr;
    return (void*)dataPtr;
}

void BD_CALL bdAlignedOffsetFree(void* p) {
    if (!p) return;
    bdUWord dataPtr = (bdUWord)(uintptr_t)p;
    bdUWord* headerPtr = (bdUWord*)(dataPtr - sizeof(void*));
    free((void*)(uintptr_t)*headerPtr);
}

void* BD_CALL bdAlignedOffsetRealloc(void* p, bdUWord size, bdUWord align, bdUWord offset, bdUWord oldSize) {
    void* dataPtr = bdAlignedOffsetMalloc(size, align, offset);
    if (dataPtr && p) {
        bdUWord copy = size < oldSize ? size : oldSize;
        memcpy(dataPtr, p, copy);
        bdAlignedOffsetFree(p);
    }
    return dataPtr;
}
