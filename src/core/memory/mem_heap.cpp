// ============================================================================
// Memory Subsystem — mem_heap, dlmalloc wrappers, sentinel checks
// Source: mem_lib.o + ae_heap.o + mem_lib_platform.o + dlmalloc.o (75 funcs)
// ea: 0x7BADD0-0x7BD370
// dlmalloc.o → use system malloc/free (dlmalloc embedded for Xbox only)
// ============================================================================

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdint>

#ifdef _WIN32
  #include <malloc.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#else
  #define OutputDebugStringA(s) fprintf(stderr, "%s", s)
  #define DebugBreak() __builtin_debugtrap()
  typedef void* HANDLE;
  static HANDLE CreateFileA(const char*,int,int,void*,int,int,void*) { return nullptr; }
  static int CloseHandle(HANDLE) { return 0; }
#endif

// ============================================================================
// Types
// ============================================================================

struct malloc_chunk {
    unsigned int prev_foot;  // size of previous chunk
    unsigned int head;       // size + flags
    malloc_chunk* fd;       // forward link
    malloc_chunk* bk;       // backward link
};

struct malloc_state {
    unsigned int   max_fast;
    malloc_chunk*  fastbins[10];
    malloc_chunk*  top;
    malloc_chunk*  last_remainder;
    malloc_chunk*  bins[256];
    unsigned int   binmap[4];
    unsigned int   trim_threshold;
    unsigned int   top_pad;
    unsigned int   mmap_threshold;
    int            n_mmaps;
    int            n_mmaps_max;
    int            max_n_mmaps;
    unsigned int   pagesize;          // +0x45C
    unsigned int   mmapped_mem;
    unsigned int   sbrked_mem;
    unsigned int   max_sbrked_mem;
    unsigned int   max_mmapped_mem;
    unsigned int   max_total_mem;
};

enum mem_heap_type {
    MEM_HEAP_DEFAULT = 0,
    MEM_HEAP_DEBUG   = 1,
    MEM_HEAP_COMBINE = 2,
};

struct mem_heap {
    malloc_state av;              // +0x000 (1140 bytes)
    void*        start;           // +0x474
    void*        end;             // +0x478
    void*        cur_left;        // +0x47C
    void*        cur_right;       // +0x480
    unsigned     size;            // +0x484
    unsigned     used_byte;       // +0x488
    unsigned     high_used_byte;  // +0x48C
    mem_heap*    reserve;         // +0x490
    int          total_allocs;    // +0x494
    int          total_frees;     // +0x498
};

// ============================================================================
// Globals
// ============================================================================

static mem_heap  s_heap_default;
static mem_heap  s_heap_debug;
static mem_heap  s_heap_combine;
static mem_heap* s_current_heap = &s_heap_default;
static bool      s_no_mem_break = false;
static const char* s_mem_context = "root";
static int       s_checkpoint_alloc_count = 0;

// ============================================================================
// dlmalloc stubs — use system malloc/free
// ============================================================================

#define dlmalloc(s)    malloc(s)
#define dlfree(p)      free(p)
#define dlrealloc(p,s) realloc(p,s)
#define dlcalloc(n,s)  calloc(n,s)
#define dlmemalign(a,s) ((void*)0) // stub

// ============================================================================
// mem_heap_create — initialize a heap object
// ea: 0x7BAEF0, 0x7BAF60
// ============================================================================

void mem_heap_init(mem_heap* heap, void* start, void* cur_left, void* cur_right) {
    memset(heap, 0, sizeof(mem_heap));
    heap->start = start;
    heap->cur_left = cur_left;
    heap->cur_right = cur_right;
    heap->end = cur_right;
}

int mem_heap_create(void* start, void* end, mem_heap* out) {
    if (!out) return -1;
    mem_heap_init(out, start, start, end);
    out->size = (unsigned)((uintptr_t)end - (uintptr_t)start);
    return 0;
}

void mem_heap_create(mem_heap* heap, void* start, void* end,
                     mem_heap* reserve) {
    memset(heap, 0, 0x49C);
    heap->start = start;
    heap->end = end;
    heap->cur_left = start;
    heap->cur_right = end;
    heap->size = (unsigned)((uintptr_t)end - (uintptr_t)start);
    heap->used_byte = 0;
    heap->high_used_byte = 0;
    heap->reserve = reserve;
    heap->total_allocs = 0;
}

// ============================================================================
// mem_heap_destroy
// ea: 0x7BAFC0, 0x7BB000
// ============================================================================

int mem_heap_destroy(mem_heap* heap) {
    if (!heap) return -1;
    memset(heap, 0, sizeof(mem_heap));
    return 0;
}

int mem_heap_destroy(int heap_type) {
    mem_heap* h = nullptr;
    switch (heap_type) {
        case MEM_HEAP_DEBUG:   h = &s_heap_debug; break;
        case MEM_HEAP_COMBINE: h = &s_heap_combine; break;
        default:               h = &s_heap_default; break;
    }
    return mem_heap_destroy(h);
}

// ============================================================================
// mem_heap_get / mem_heap_set_current
// ea: 0x7BB050-0x7BB0C0
// ============================================================================

mem_heap* mem_heap_get(mem_heap_type type) {
    switch (type) {
        case MEM_HEAP_DEBUG:   return &s_heap_debug;
        case MEM_HEAP_COMBINE: return &s_heap_combine;
        default:               return &s_heap_default;
    }
}

mem_heap* mem_heap_set_current(mem_heap* heap) {
    mem_heap* old = s_current_heap;
    s_current_heap = heap;
    return old;
}

mem_heap* mem_heap_set_current(mem_heap_type type) {
    return mem_heap_set_current(mem_heap_get(type));
}

mem_heap* mem_heap_get_current() {
    return s_current_heap;
}

// ============================================================================
// mem_heap_malloc — allocate from heap
// ea: 0x7BB910-0x7BB940
// ============================================================================

void* mem_heap_malloc(mem_heap* heap, unsigned size, int flags) {
#ifdef _WIN32
    void* ptr = _aligned_malloc(size, 16);
#else
    void* ptr = malloc(size);
#endif
    if (ptr) {
        if (!heap) heap = s_current_heap;
        heap->total_allocs++;
        heap->used_byte += size;
        if (heap->used_byte > heap->high_used_byte)
            heap->high_used_byte = heap->used_byte;
    }
    return ptr;
}

void* mem_heap_malloc(mem_heap* heap, int alignment, unsigned size) {
#ifdef _WIN32
    void* ptr = _aligned_malloc(size, alignment < 16 ? 16 : alignment);
#else
    (void)alignment;
    void* ptr = malloc(size);
#endif
    if (ptr) {
        if (!heap) heap = s_current_heap;
        heap->total_allocs++;
        heap->used_byte += size;
        if (heap->used_byte > heap->high_used_byte)
            heap->high_used_byte = heap->used_byte;
    }
    return ptr;
}

void* mem_heap_malloc_flags(unsigned size, int flags) {
    return mem_heap_malloc(s_current_heap, size, flags);
}

void* mem_heap_malloc(unsigned size) {
    return mem_heap_malloc(s_current_heap, 16, size);
}

void* mem_heap_malloc(int alignment, unsigned size) {
    return mem_heap_malloc(s_current_heap, alignment, size);
}

// ============================================================================
// mem_heap_free — free back to heap
// ea: 0x7BBDA0
// ============================================================================

void mem_heap_free(void* ptr) {
    if (!ptr) return;
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
    s_current_heap->total_frees++;
}

void mem_heap_free(mem_heap* heap, void* ptr) {
    if (!ptr) return;
#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif
    if (heap) heap->total_frees++;
}

// ============================================================================
// mem_heap_realloc
// ea: 0x7BBA50
// ============================================================================

void* mem_heap_realloc(void* ptr, unsigned newSize) {
#ifdef _WIN32
    void* newPtr = _aligned_realloc(ptr, newSize, 16);
#else
    void* newPtr = realloc(ptr, newSize);
#endif
    if (newPtr) {
        s_current_heap->used_byte += newSize; // approximate
    }
    return newPtr;
}

void* mem_heap_realloc(mem_heap* heap, void* ptr, unsigned newSize) {
#ifdef _WIN32
    void* newPtr = _aligned_realloc(ptr, newSize, 16);
#else
    void* newPtr = realloc(ptr, newSize);
#endif
    return newPtr;
}

// ============================================================================
// mem_heap_calloc
// ea: 0x7BB9B0
// ============================================================================

void* mem_heap_calloc(unsigned count, unsigned size) {
    return calloc(count, size);
}

void* mem_heap_calloc(mem_heap* heap, unsigned count, unsigned size) {
    return calloc(count, size);
}

// ============================================================================
// mem_sentinel — heap sentinel/guard checks
// ea: 0x7BAE40-0x7BAED0
// ============================================================================

int mem_sentinel_alloc_size(int size, int align) {
    return size + 16; // sentinel + align
}

void* mem_sentinel_init(void* ptr, int size, int align) {
    if (!ptr) return nullptr;
    // Write sentinel pattern before and after the payload
    uint32_t* sentinel_start = (uint32_t*)ptr;
    *sentinel_start = 0xDEADBEEF;
    void* payload = (uint8_t*)ptr + 8;
    uint32_t* sentinel_end = (uint32_t*)((uint8_t*)payload + size);
    *sentinel_end = 0xDEADBEEF;
    return payload;
}

void* mem_sentinel_mem_heap_free(void* ptr) {
    if (!ptr) return nullptr;
    uint32_t* sentinel = (uint32_t*)((uint8_t*)ptr - 8);
    if (*sentinel != 0xDEADBEEF) {
        DebugBreak(); // sentinel corruption
    }
    return (void*)sentinel;
}

// ============================================================================
// mem_validate / mem_checkpoint / mem_context
// ea: 0x7BB210-0x7BB330
// ============================================================================

void mem_validate_heap(mem_heap* heap) {
    // Walk chunks validating consistency
}

void mem_push_context(const char* context) {
    s_mem_context = context;
}

void mem_pop_context() {
    s_mem_context = "root";
}

int mem_get_used_bytes(mem_heap_type type) {
    mem_heap* h = mem_heap_get(type);
    return h ? h->used_byte : -1;
}

int mem_get_high_used_bytes(mem_heap_type type) {
    mem_heap* h = mem_heap_get(type);
    return h ? h->high_used_byte : -1;
}

int mem_get_free_bytes(mem_heap_type type) {
    mem_heap* h = mem_heap_get(type);
    return h ? h->size - h->used_byte : -1;
}

void mem_enable_no_mem_break(bool enable) {
    s_no_mem_break = enable;
}

void mem_break() {
    if (!s_no_mem_break) DebugBreak();
}

int mem_set_checkpoint() {
    int count = s_current_heap->total_allocs;
    s_checkpoint_alloc_count = count;
    return count;
}

void mem_report_mem_usage() {
    char buf[256];
    snprintf(buf, sizeof(buf), "[mem] used=%d high=%d size=%d allocs=%d frees=%d\n",
             s_current_heap->used_byte, s_current_heap->high_used_byte,
             s_current_heap->size, s_current_heap->total_allocs, s_current_heap->total_frees);
    OutputDebugStringA(buf);
}

int mem_init() {
    memset(&s_heap_default, 0, sizeof(s_heap_default));
    s_current_heap = &s_heap_default;
    return 0;
}

// ============================================================================
// Platform-specific stubs
// ea: 0x7BC0D0-0x7BC1F0
// ============================================================================

void* mem_get_arena_hi() { return nullptr; }
void* mem_get_arena_lo() { return nullptr; }
void* mem_get_debug_arena_hi() { return nullptr; }
void* mem_get_debug_arena_lo() { return nullptr; }
void* mem_get_combine_arena_hi() { return nullptr; }
void* mem_get_combine_arena_lo() { return nullptr; }
void  mem_get_stack(void* out, int level) {}

unsigned mem_host_fopen(const char* path, const char* mode) {
    // Xbox: MountUtilityDrive + CreateFile
    return 0;
}
int mem_host_fclose(unsigned handle) { return 0; }
int mem_host_fwrite(const void* data, unsigned size, unsigned handle) { return 0; }

int mem_host_fprintf(unsigned handle, const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
    return 0;
}

// ============================================================================
// ae_heap — Angel Engine heap wrapper
// ea: 0x7BBE20-0x7BBF40
// ============================================================================

class ae_heap_base {
public:
    ae_heap_base();
    virtual ~ae_heap_base();
    mem_heap* GetHeapPointer();
protected:
    void* MemAlloc(unsigned size, unsigned align, mem_heap* heap);
    void  MemFree(void* ptr, mem_heap* heap);
    bool  MemCheckFree(void* ptr, mem_heap* heap);
};

// ea: 0x004B4E10
ae_heap_base::ae_heap_base() {}
ae_heap_base::~ae_heap_base() {}

// ea: 0x004B4E20
mem_heap* ae_heap_base::GetHeapPointer() { return nullptr; }

class ae_heap : public ae_heap_base {
    mem_heap mHeap;
public:
    ae_heap(unsigned size) { memset(&mHeap, 0, sizeof(mHeap)); }
    ~ae_heap();
};

ae_heap::~ae_heap() {
    mem_heap_free(mHeap.start);
}

void* ae_heap_base::MemAlloc(unsigned size, unsigned align, mem_heap* heap) {
    if (size + heap->used_byte <= heap->size)
        return mem_heap_malloc(heap, static_cast<int>(align), size);
    return nullptr;
}

void ae_heap_base::MemFree(void* ptr, mem_heap* heap) {
    if (!heap) heap = &s_heap_default;
    mem_heap_free(heap, ptr);
}

bool ae_heap_base::MemCheckFree(void* ptr, mem_heap* heap) {
    if (ptr < heap->start || ptr >= heap->end)
        return false;
    mem_heap_free(heap, ptr);
    return true;
}

// ============================================================================
// Missing internal wrappers (from mem_lib.o, ea: 0x7BADD0-0x7BDA0)
// ============================================================================

// InitQuickPool — initialize the quick-fit pool (Xbox-only small-block cache)
// ea: 0x7BADD0
void InitQuickPool() {
    // Xbox: configures dlmalloc's fastbins. No-op with system malloc.
}

// mem_alt_sbrk — alternative sbrk for Xbox memory layout
// ea: 0x7BB1D0
void* mem_alt_sbrk(long increment) {
    // Xbox: uses MmAllocateContiguousMemory. No-op with system malloc.
    return nullptr;
}

// mem_get_current_av — get the current dlmalloc state
// ea: 0x7BB200
malloc_state* mem_get_current_av() {
    return &s_current_heap->av;
}

// Validate chunk list integrity (debug only)
// ea: 0x7BB210
void validate_chunk_list(malloc_chunk* start, bool fullCheck) {
    // No-op: system malloc handles integrity internally
}

// mem_heap_malloc_private — internal alloc bypassing sentinel checks
// ea: 0x7BB5A0
void* mem_heap_malloc_private(mem_heap* heap, unsigned size) {
    return mem_heap_malloc(heap, size, 0);
}

// mem_heap_realloc_private — internal realloc
// ea: 0x7BB510
void* mem_heap_realloc_private(void* ptr, unsigned newSize) {
    return mem_heap_realloc(ptr, newSize);
}

// mem_heap_free_private — internal free
// ea: 0x7BB8B0
void mem_heap_free_private(mem_heap* heap, void* ptr) {
    mem_heap_free(heap, ptr);
}

void mem_heap_free_private(void* ptr) {
    mem_heap_free(ptr);
}

// mem_heap_malloc_ctx — allocate with context tracking
// ea: 0x7BB990
void* mem_heap_malloc_ctx(unsigned size, int flags, const char* file, const char* func, int line) {
    (void)file;
    (void)func;
    (void)line;
    return mem_heap_malloc(flags, size);
}

// IDA canonical parameter order used by cl.o call sites.
void* mem_heap_malloc_ctx(int alignment, unsigned size, const char* file,
                          const char* func, int line) {
    return mem_heap_malloc_ctx(size, alignment, file, func, line);
}

// mem_heap_free_check_reserve — check if freeing from reserve heap
// ea: 0x7BBB00
bool mem_heap_free_check_reserve(mem_heap* heap, void* ptr) {
    return false; // No reserve heap with system malloc
}

// debug_malloc / debug_free — debug alloc/free with sentinel guards
// ea: 0x7BBAC0, 0x7BBDB0
void* debug_malloc(unsigned size) {
    void* raw = malloc(size + 16);
    return mem_sentinel_init(raw, size, 0);
}

void debug_free(void* ptr) {
    void* raw = mem_sentinel_mem_heap_free(ptr);
    free(raw);
}
