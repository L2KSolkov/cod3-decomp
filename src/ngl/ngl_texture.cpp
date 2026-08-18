// ============================================================================
// ngl_texture.cpp - NGL texture resource directory + apk callbacks (8 funcs).
// Source: src/ngl_texture.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_texture.o). Data globals here match the
// map attribution: nglIFLSpeed, nglDefaultTex, nglTextureAnimFrame,
// nglWhiteTex, nglTextureDirectory all live in ngl_texture.o.
// ============================================================================

#include "core/tlSkipList.h"
#include "core/tlFixedString.h"
#include "ngl/nglTexture.h"
#include "filesystem/apk.h"

#include <stdio.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void tlWarning(const char* fmt, ...);
extern void* nglGetResource(const tlFixedString* FileName, unsigned int FourCC);
extern int nglFrame;
extern void ngliProcessTexture(apk::apkFile* File, apk::apkFileEntry* Entry);

// ============================================================================
// Globals (data, map-attributed to ngl_texture.o)
// ============================================================================
float nglIFLSpeed = 0.0f;
int nglTextureAnimFrame = 0;
nglTexture* nglDefaultTex = NULL;
nglTexture* nglWhiteTex = NULL;
tlSkipList<nglTexture, tlFixedString> nglTextureDirectory;

static int ScreenCount = 0;
static char Buf[0x40];

// ============================================================================
// nglSetIFLSpeed - ea: 0x8421D0
// ============================================================================
void nglSetIFLSpeed(float FPS) {
    nglIFLSpeed = FPS;
}

// ============================================================================
// nglScreenShot - ea: 0x8421F0
// ============================================================================
void nglScreenShot(const char* FileName) {
    nglTexture* FrontBufferTex = nglGetFrontBufferTex();
    if (FileName != NULL) {
        nglSaveTexture(FrontBufferTex, FileName);
    } else {
        sprintf(Buf, "screenshot%4.4d", ScreenCount++);
        nglSaveTexture(FrontBufferTex, Buf);
    }
}

// ============================================================================
// nglGetTexture - ea: 0x842240
// ============================================================================
nglTexture* nglGetTexture(const tlFixedString& FileName) {
    return (nglTexture*)nglGetResource(&FileName, 0x584554);  // 'TEX '
}

// ============================================================================
// nglCanReleaseTexture - ea: 0x842260
// ============================================================================
bool nglCanReleaseTexture(nglTexture* Tex) {
    return Tex->LastFrameRef + 1 < nglFrame;
}

// ============================================================================
// nglProcessTexture - ea: 0x842280
// ============================================================================
void nglProcessTexture(nglTexture* Tex) {
    (void)Tex;
}

// ============================================================================
// nglAPKTextureLoadCallback - ea: 0x842290
// ============================================================================
void nglAPKTextureLoadCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglTexture* Data = (nglTexture*)Entry->GetData(File, SectionIndex, true);
    if (Data != NULL) {
        ngliProcessTexture(File, Entry);
        nglTextureDirectory.Add(Data);
        Data->Flags |= 8u;
    }
}

// ============================================================================
// nglAPKTextureDeleteCallback - ea: 0x8422F0
// ============================================================================
void nglAPKTextureDeleteCallback(apk::apkFile* File, apk::apkFileEntry* Entry, void* UserData) {
    (void)UserData;
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    const nglTexture* Data = (const nglTexture*)Entry->GetData(File, SectionIndex, true);
    if (Data != NULL) {
        nglTextureDirectory.Del(Data);
        if (Data->LastFrameRef + 1 >= nglFrame) {
            tlWarning("NGL: Texture %s destroyed while still referenced by the async renderer.\n",
                      Data->FileName->str);
            ngliWaitForResource();
        }
        ngliUnloadTexture(File, Entry);
    }
}

// ============================================================================
// nglTextureInit - ea: 0x842370
// ============================================================================
void nglTextureInit() {
    nglTextureDirectory.Level = 0;
    nglTextureDirectory.Head =
        (tlSkipList<nglTexture, tlFixedString>::Instance*)tlMemAlloc(0x44u, 8u, 0x1000000);
    for (int i = 0; i <= 15; ++i)
        nglTextureDirectory.Head->Forward[i] = NULL;
    apk::apkRegisterFileType(0x584554, 3u, nglAPKTextureLoadCallback,
                             nglAPKTextureDeleteCallback, NULL);
}
