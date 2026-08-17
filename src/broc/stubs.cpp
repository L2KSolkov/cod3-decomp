// AUTO-GENERATED STUBS — Broc engine core (MPBrocCore_xboxd + mp_level.xboxd)
// All non-inline functions from Broc.o, mp_util_wad.o, mp_anim_wad.o are now in Broc.cpp or stubbed there.

#include <stdio.h>

#ifndef OutputDebugStringA
#define OutputDebugStringA(msg) fprintf(stderr, "%s\n", msg)
#endif

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_broc(void) {
    COD3_UNIMPLEMENTED("broc");
}

// Broc::string::is_empty (mp_util_wad.o; ea: 0x004A9D00)
#include "engine/broc_types.h"
bool Broc::string::is_empty() const
{
    const uintptr_t block = reinterpret_cast<uintptr_t>(mBlock);
    return block == 0 || block == static_cast<uintptr_t>(-12) ||
           *reinterpret_cast<const unsigned char*>(block + 0x0C) == 0;
}
