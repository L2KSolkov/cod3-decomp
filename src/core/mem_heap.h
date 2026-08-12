// ============================================================================
// mem_heap — heap allocator (implemented in src/core/memory/mem_heap.cpp)
// ============================================================================

#pragma once

struct mem_heap;

void* mem_heap_malloc(unsigned int size);
void* mem_heap_malloc(int alignment, unsigned int size);
void* mem_heap_malloc(mem_heap* heap, int alignment, unsigned int size);
void mem_heap_free(void* ptr);
void mem_heap_free(mem_heap* heap, void* ptr);
