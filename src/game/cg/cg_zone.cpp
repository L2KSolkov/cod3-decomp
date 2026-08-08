// ============================================================================
// cg_zone.cpp - client zone allocators (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"

// ea: 0x0068C900
void* cg_Z_MallocInternal(int size)
{
    return _Z_MallocInternal((unsigned int)size);
}

// ea: 0x0068C910
void cg_Z_FreeInternal(void* ptr)
{
    _Z_FreeInternal(ptr);
}
