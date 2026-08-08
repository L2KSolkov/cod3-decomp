// ============================================================================
// hunk_mem.cpp - memory allocation wrappers (core.o)
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <string.h>

extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void Sys_OutOfMemError();
extern void Com_Memset(void* dest, int val, unsigned int count);

// ea: 0x004BB010
void _Z_FreeInternal(void* ptr)
{
    mem_heap_free(ptr);
}

// ea: 0x004C02F0
void* _Z_MallocInternal(int size)
{
    void* v1 = mem_heap_malloc(size);
    if (v1 == nullptr && size > 0)
        Sys_OutOfMemError();
    Com_Memset(v1, 0, size);
    return v1;
}

// ea: 0x004C0330
char* CopyStringInternal(const char* in)
{
    int v1 = (int)strlen(in) + 1;
    void* v2 = mem_heap_malloc(v1);
    if (v2 == nullptr && v1 > 0)
        Sys_OutOfMemError();
    Com_Memset(v2, 0, v1);
    strcpy((char*)v2, in);
    return (char*)v2;
}
