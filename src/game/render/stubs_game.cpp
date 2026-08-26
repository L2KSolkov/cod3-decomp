// Game-renderer compatibility translation unit. Implementations live in the
// owning render sources.
#include "core/tlFixedString.h"

// render.o entry points referenced by cg/cl/g logic.
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
