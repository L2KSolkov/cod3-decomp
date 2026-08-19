// ============================================================================
// ngl_internal.cpp - NGL core globals + init/exit (18 funcs).
// Source: src/ngl_internal.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_internal.o).
// ============================================================================

#include "core/tlSkipList.h"
#include "core/tlFixedString.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglDebug.h"
#include "ngl/nglTexture.h"
#include "filesystem/apk.h"

#include <float.h>
#include <intrin.h>
#include <stdio.h>
#include <stdint.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern bool _tlAssert(const char* file, int line, const char* expr, const char* msg);
extern void tlWarning(const char* fmt, ...);
extern void tlPrintf(const char* fmt, ...);
extern void tlMemFree(void* Ptr);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlInitListInit(void);

// ngl_dx_core.o / ngl_dx_draw.o externs
extern void nglDebugInit(void);
extern void nglMeshInit(void);
extern void nglTextureInit(void);
extern void ngliPreInit(void);
extern void ngliPostInit(void);
extern void ngliInitWhiteTexture(void);
extern void ngliListInit(void);
extern void ngliExitList(void);
extern void nglDxResetDevice(void);

// ============================================================================
// Globals (data)
// ============================================================================
bool nglInitialized = false;
unsigned int nglVBlankCount = 0;
unsigned int nglFrameVBlankCount = 0;
int nglFrame = 0;
void* (*nglResourceCallbackFunction)(const tlFixedString&, unsigned int) = NULL;

nglDisplayModeType nglDisplayMode = { 0 };
unsigned int nglDisplayMode_Set = 0;

char nglVersionString[4] = { 0, 0, 0, 0 };

// ngl_texture.o / ngl_font.o / ngl_morph.o (defined in their cpp files)
extern nglTexture* nglDefaultTex;
extern nglTexture* nglWhiteTex;
extern tlSkipList<nglTexture, tlFixedString> nglTextureDirectory;
extern tlSkipList<nglFont, tlFixedString> nglFontDirectory;
extern tlSkipList<nglMorphSet, tlFixedString> nglMorphDirectory;
extern tlSkipList<nglMesh, tlFixedString> nglMeshDirectory;        // ngl_mesh.o
extern tlSkipList<nglMaterial, tlFixedString> nglMaterialDirectory; // ngl_mesh.o

// GetKey accessors for the skip list template.
const tlFixedString* GetKey(const nglTexture* t) { return t->FileName; }

const tlFixedString* GetKey(const nglTexture* t);
const tlFixedString* GetKey(const nglMesh* m);
const tlFixedString* GetKey(const nglMaterial* m);
const tlFixedString* GetKey(const nglFont* f);
const tlFixedString* GetKey(const nglMorphSet* m);
// ============================================================================
// nglShaderNextID - ea: 0x840F20
// ============================================================================
int nglShaderNextID() {
    extern int nglShader_NextID;
    return nglShader_NextID++;
}

// ============================================================================
// nglGetDisplayMode - ea: 0x840F30
// ============================================================================
unsigned int nglGetDisplayMode() {
    return nglDisplayMode_Set;
}

// ============================================================================
// nglIsDisplayPAL - ea: 0x840F40
// ============================================================================
bool nglIsDisplayPAL() {
    return nglDisplayMode.PAL;
}

// ============================================================================
// nglIsDisplayWidescreen - ea: 0x840F50
// ============================================================================
bool nglIsDisplayWidescreen() {
    return nglDisplayMode.Widescreen;
}

// ============================================================================
// nglGetVersion - ea: 0x840F60
// ============================================================================
unsigned int nglGetVersion() {
    return (unsigned int)(uintptr_t)nglVersionString;
}

// ============================================================================
// nglGetVBlankCount - ea: 0x840F70
// ============================================================================
unsigned int nglGetVBlankCount() {
    return nglVBlankCount;
}

// ============================================================================
// nglGetVBlankMS - ea: 0x840F80
// ============================================================================
float nglGetVBlankMS() {
    return nglDisplayMode.PAL ? 20.0f : 16.666666f;
}

// ============================================================================
// nglGetFPS - ea: 0x840FA0
// ============================================================================
float nglGetFPS() {
    return nglPerfInfo.FPS;
}

// ============================================================================
// nglGetFrameCount - ea: 0x840FB0
// ============================================================================
int nglGetFrameCount() {
    return nglFrame;
}

// ============================================================================
// nglGetScreenWidth / nglGetScreenHeight - ea: 0x840FC0 / 0x840FD0
// ============================================================================
int nglGetScreenWidth() {
    return 640;
}

int nglGetScreenHeight() {
    return 480;
}

// ============================================================================
// nglSetResourceCallback - ea: 0x840FE0
// ============================================================================
void nglSetResourceCallback(void* (*Callback)(const tlFixedString&, unsigned int)) {
    nglResourceCallbackFunction = Callback;
}

// ============================================================================
// nglCanReleaseFile - ea: 0x840FF0
// ============================================================================
bool nglCanReleaseFile(apk::apkFile* File) {
    return File->LastFrameRef + 1 < nglFrame;
}

// ============================================================================
// nglIsInitialized - ea: 0x841010
// ============================================================================
bool nglIsInitialized() {
    return nglInitialized;
}

// ============================================================================
// nglGetResource - ea: 0x841020
// ============================================================================
void* nglGetResource(const tlFixedString& FileName, unsigned int FourCC) {
    if (nglResourceCallbackFunction != NULL)
        return nglResourceCallbackFunction(FileName, FourCC);

    if (FourCC > 0x48524F4D) {  // 'MORH'
        if (FourCC == 0x4D485345)  // 'MESH'
            return nglMeshDirectory.Find(FileName);
        if (FourCC == 0x544E4F46)  // 'FONT'
            return nglFontDirectory.Find(FileName);
        return NULL;
    }
    if (FourCC == 0x48524F4D)  // 'MORH'
        return nglMorphDirectory.Find(FileName);
    if (FourCC == 0x54414D)  // 'MAT '
        return nglMaterialDirectory.Find(FileName);
    if (FourCC != 0x584554)  // 'TEX '
        return NULL;

    nglTexture* tex = nglTextureDirectory.Find(FileName);
    if (tex == NULL) {
        tlWarning("NGL: Unable to locate texture resource %s - assigning default texture.\n",
                 FileName.str);
        return nglDefaultTex;
    }
    return tex;
}

// ============================================================================
// nglInitDefaultResources - ea: 0x8410D0
// ============================================================================
void nglInitDefaultResources() {
    ngliInitWhiteTexture();
    if (nglDefaultTex == NULL)
        nglDefaultTex = nglWhiteTex;
    apk::apkSetResourceCallback(nglGetResource);
}

// ============================================================================
// nglInit - ea: 0x841100
// ============================================================================
void nglInit() {
    if (nglInitialized) {
        _tlAssert("src/ngl_internal.cpp", 215, "nglInitialized == false",
                  "NGL is already initialized.");
        __debugbreak();
    }
    _controlfp(0x300u, 0x300u);
    _mm_setcsr(_mm_getcsr() | 0x6000);
    tlPrintf("\n");
    tlPrintf("-----------------------------------------------------------------\n");
    tlPrintf("  Nyarlathotep's Graphics Laboratory 3.0.0\n");
    tlPrintf("  (c) 2003 Treyarch LLC\n");
    tlPrintf("\n");
    nglDebugInit();
    nglMeshInit();
    nglTextureInit();
    ngliPreInit();
    tlInitListInit();
    ngliPostInit();
    ngliInitWhiteTexture();
    if (nglDefaultTex == NULL)
        nglDefaultTex = nglWhiteTex;
    apk::apkSetResourceCallback(nglGetResource);
    ngliListInit();
    nglInitialized = true;
}

// ============================================================================
// nglExit - ea: 0x8411F0
// ============================================================================
void nglExit() {
    if (!nglInitialized) {
        _tlAssert("src/ngl_internal.cpp", 254, "nglInitialized == true",
                  "NGL is not initialized.");
        __debugbreak();
    }
    ngliExitList();
    nglFontDirectory.Destroy();
    nglTextureDirectory.Destroy();
    nglMorphDirectory.Destroy();
    nglMeshDirectory.Destroy();
    nglMaterialDirectory.Destroy();
    nglInitialized = false;
}
