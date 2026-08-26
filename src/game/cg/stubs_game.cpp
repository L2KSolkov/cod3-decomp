// AUTO-GENERATED STUBS — Client game (cg.o)
// Remaining unported non-inline functions (5 entries below, including the
// diagnostic entry point)
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_cg(void) {
    COD3_UNIMPLEMENTED("game_cg");
}

// cg.o function stubs (ported later)
#include "game/game_types.h"
#include "game/trace_types.h"

void CG_ClipMoveToEntities(const math::Position3* start,
                           const math::Position3* mins,
                           const math::Position3* maxs,
                           const math::Position3* end,
                           const collision_context_t* context, int a6,
                           trace_t* a7)
{
    (void)start; (void)mins; (void)maxs; (void)end;
    (void)context; (void)a6; (void)a7;
}
void CG_OffsetFirstPersonView() {}
void CG_OffsetThirdPersonView() {}
void CG_UpdateShellShockSound(const void* a) { (void)a; }
