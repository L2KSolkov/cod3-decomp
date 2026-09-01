// ============================================================================
// mem_heap — heap allocator (implemented in src/core/memory/mem_heap.cpp)
// ============================================================================

#pragma once

#include <stddef.h>

// The release heap embeds its allocator state before the public bookkeeping.
// Keep this ABI layout shared so callers cannot accidentally treat allocator
// bytes as start/end pointers.
struct malloc_chunk {
    unsigned int prev_foot;
    unsigned int head;
    malloc_chunk* fd;
    malloc_chunk* bk;
};

struct malloc_state {
    unsigned int max_fast;
    malloc_chunk* fastbins[10];
    malloc_chunk* top;
    malloc_chunk* last_remainder;
    malloc_chunk* bins[256];
    unsigned int binmap[4];
    unsigned int trim_threshold;
    unsigned int top_pad;
    unsigned int mmap_threshold;
    int n_mmaps;
    int n_mmaps_max;
    int max_n_mmaps;
    unsigned int pagesize;
    unsigned int mmapped_mem;
    unsigned int sbrked_mem;
    unsigned int max_sbrked_mem;
    unsigned int max_mmapped_mem;
    unsigned int max_total_mem;
};

struct mem_heap {
    malloc_state av;
    void* start;
    void* end;
    void* cur_left;
    void* cur_right;
    unsigned int size;
    unsigned int used_byte;
    unsigned int high_used_byte;
    mem_heap* reserve;
    int total_allocs;
    int total_frees;
};

static_assert(sizeof(malloc_chunk) == 0x10, "malloc_chunk layout mismatch");
static_assert(sizeof(malloc_state) == 0x474, "malloc_state layout mismatch");
static_assert(offsetof(mem_heap, start) == 0x474, "mem_heap::start offset mismatch");
static_assert(offsetof(mem_heap, end) == 0x478, "mem_heap::end offset mismatch");
static_assert(offsetof(mem_heap, cur_left) == 0x47C, "mem_heap::cur_left offset mismatch");
static_assert(offsetof(mem_heap, cur_right) == 0x480, "mem_heap::cur_right offset mismatch");
static_assert(offsetof(mem_heap, size) == 0x484, "mem_heap::size offset mismatch");
static_assert(offsetof(mem_heap, used_byte) == 0x488, "mem_heap::used_byte offset mismatch");
static_assert(offsetof(mem_heap, high_used_byte) == 0x48C, "mem_heap::high_used_byte offset mismatch");
static_assert(offsetof(mem_heap, reserve) == 0x490, "mem_heap::reserve offset mismatch");
static_assert(offsetof(mem_heap, total_allocs) == 0x494, "mem_heap::total_allocs offset mismatch");
static_assert(offsetof(mem_heap, total_frees) == 0x498, "mem_heap::total_frees offset mismatch");
static_assert(sizeof(mem_heap) == 0x49C, "mem_heap layout mismatch");

void* mem_heap_malloc(unsigned int size);
void* mem_heap_malloc(int alignment, unsigned int size);
void* mem_heap_malloc(mem_heap* heap, int alignment, unsigned int size);
void mem_heap_free(void* ptr);
void mem_heap_free(mem_heap* heap, void* ptr);
