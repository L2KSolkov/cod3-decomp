// ============================================================================
// mem_heap — heap allocator stubs (until mem_mp_xboxr ported)
// ============================================================================

#pragma once

#include <stdlib.h>

inline void* mem_heap_malloc(unsigned int size) {
    return malloc(size);
}

inline void mem_heap_free(void* ptr) {
    free(ptr);
}
