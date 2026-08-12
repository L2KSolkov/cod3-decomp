// AUTO-GENERATED STUBS — Game renderer integration (render.o)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_render(void) {
    COD3_UNIMPLEMENTED("game_render");
}

// render.o entry points referenced by cg/cl/g logic; ported later.
#include "game/game_types.h"
struct trace_t;
struct DObjSkelMat;
struct nglTexture;

// ea: 0x6B2000 (render.o) - stub
nglTexture* GetTextureData(const char* name, int image_type,
                           const char* fromPak)
{
    (void)name; (void)image_type; (void)fromPak;
    return nullptr;
}

// ea: 0x6CB470 (render.o) - stub
int XModelTraceLine(IVPointer<XModel> model, trace_t* results,
                    DObjSkelMat* boneMtxList, const float* localStart,
                    const float* localEnd, int contentmask)
{
    (void)model; (void)results; (void)boneMtxList;
    (void)localStart; (void)localEnd; (void)contentmask;
    return 0;
}
