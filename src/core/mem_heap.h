// ============================================================================
// mem_heap — heap allocator (implemented in src/core/memory/mem_heap.cpp)
// ============================================================================

#pragma once

void* mem_heap_malloc(unsigned int size);
void* mem_heap_malloc(int alignment, unsigned int size);
void mem_heap_free(void* ptr);
