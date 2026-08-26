// Broc compatibility translation unit. Non-inline functions from Broc.o,
// mp_util_wad.o, and mp_anim_wad.o live in their owning sources.

#include <stdio.h>

#ifndef OutputDebugStringA
#define OutputDebugStringA(msg) fprintf(stderr, "%s\n", msg)
#endif

// Broc::string::is_empty (mp_util_wad.o; ea: 0x004A9D00)
#include "engine/broc_types.h"
bool Broc::string::is_empty() const
{
    const uintptr_t block = reinterpret_cast<uintptr_t>(mBlock);
    return block == 0 || block == static_cast<uintptr_t>(-12) ||
           *reinterpret_cast<const unsigned char*>(block + 0x0C) == 0;
}
