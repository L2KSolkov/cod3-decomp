// AUTO-GENERATED STUBS — Game renderer integration (render.o)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>
#include "core/tlFixedString.h"

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_game_render(void) {
    COD3_UNIMPLEMENTED("game_render");
}

// render.o entry points referenced by cg/cl/g logic; unresolved entries remain
// explicit placeholders until their release bodies are ported.
#include "game/game_types.h"
struct trace_t;
struct DObjSkelMat;
struct nglTexture;

struct PakInfoNode {
    unsigned char _pad[0xB4];
    TPakId pakId;  // IDA type: +0xB4
};

class PakManager {
public:
    static PakManager* sInst;
    const PakInfoNode* GetPakInfo(const char* long_name) const;
    TPakId GetGlobalPakId() const;
};

extern char* Com_SkipPath(char* pathname);
extern void Com_StripExtension(const char* in, char* out);
extern TPakId CurPakId();
extern nglTexture* cdGetTexture(TPakId pakId, const tlFixedString& name);
extern nglTexture* nglWhiteTex;

// ea: 0x006C2B00
nglTexture* GetTextureData(const char* name, int image_type,
                           const char* fromPak)
{
    (void)image_type;
    char textureName[64];
    Com_StripExtension(name, textureName);

    const PakInfoNode* pakInfo = PakManager::sInst->GetPakInfo(fromPak);
    TPakId pakId = (pakInfo == nullptr || pakInfo->pakId == PAK_ID_INVALID)
        ? PakManager::sInst->GetGlobalPakId()
        : pakInfo->pakId;

    tlFixedString fixedName(Com_SkipPath(textureName));
    nglTexture* result = nullptr;
    if (pakInfo != nullptr)
        result = cdGetTexture(pakId, fixedName);
    if (result == nullptr)
    {
        fixedName = tlFixedString(Com_SkipPath(textureName));
        result = cdGetTexture(CurPakId(), fixedName);
        if (result == nullptr)
            return nglWhiteTex;
    }
    return result;
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
