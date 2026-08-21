// ============================================================================
// ngl_xb_tex_load.cpp — texture loading helper (1 non-inline func).
// Source: source/xbox/ngl_xb_tex_load.cpp
// Verified against IDA (ngl_xboxr:ngl_xb_tex_load.o):
//   ngliProcessTexture @0x844D00 (?ngliProcessTexture@@YAXPAVapkFile@apk@@PAVapkFileEntry@2@@Z)
//   Register@D3DResource (inline COMDAT, ngl_xb_tex_load.o -> d3d8.h)
// ============================================================================
#include "nglTexture.h"
#include "filesystem/apk.h"
#include "core/tlFixedString.h"

// D3DResource::Register - ea: 0x844CF0
void __stdcall D3DResource::Register(void* pBase) {
    D3DResource_Register(this, pBase);
}

// ============================================================================
// ngliProcessTexture — fix up a texture loaded from an APK entry:
//   - resolve the "image" section into the texture
//   - register the "physical" section as the texture's backing memory
// ea: 0x844D00
// ============================================================================
void ngliProcessTexture(apk::apkFile* File, apk::apkFileEntry* Entry) {
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);

    nglTexture* Data = (nglTexture*)Entry->GetData(File, SectionIndex, true);
    bool physical = (Data->Flags & 4) == 0;
    Data->File = File;
    Data->LastFrameRef = -1;

    if (physical) {
        tlFixedString physicalName("physical");
        int v5 = File->GetSectionIndex(physicalName);
        void* v6 = Entry->GetData(File, v5, true);
        if (v6 != NULL)
            D3DResource_Register((D3DResource*)Data->Texture, v6);
    }
}
