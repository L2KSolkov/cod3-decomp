// ============================================================================
// ngl_dx_core.cpp - D3D device core: init/present params, frame lock, render
// frame (25 non-inline funcs, verified against IDA ngl_xboxr:ngl_dx_core.o).
// Source: src/dx/ngl_dx_core.cpp (ngl_xboxr)
// Data: nglGammaRamp/nglFenceEndOfRendering/nglFrameLock/nglPrevFrameLock/
// nglXbDisplayModeFlag/nglDisplayModes/nglPresentParams/nglDxCaps/nglD3D/
// nglDev/nglFrameLockImmediate/nglFlipCycle/nglLastFlipCycle live here.
// ============================================================================

#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_gpu_debug.h"
#include "ngl/nglTexture.h"
#include "ngl/nglDebug.h"
#include "ngl/ngl_dx_fsaa.h"
#include "ngl/nglPalette.h"
#include "ngl/nglScene.h"
#include "core/tlFixedString.h"
#include "filesystem/apk.h"
#include "threading/jobqueue.h"
#include "xbox_shim.h"

#include <intrin.h>
#include <stdio.h>
#include <string.h>

// Shader static data definitions (ngl_xboxr)
unsigned int nglGpuQuadPCVertexShader::Shader = 0;     // ?Shader@nglGpuQuadPCVertexShader@@3KA
unsigned int nglGpuQuadPCUVVertexShader::Shader = 0;   // ?Shader@nglGpuQuadPCUVVertexShader@@3KA
unsigned int nglGpuQuadPUVVertexShader::Shader = 0;    // ?Shader@nglGpuQuadPUVVertexShader@@3KA
unsigned int nglGpuQuadPUV4VertexShader::Shader = 0;   // ?Shader@nglGpuQuadPUV4VertexShader@@3KA
unsigned int nglGpuQuadPUVMatColVertexShader::Shader = 0;  // ?Shader@nglGpuQuadPUVMatColVertexShader@@3KA
unsigned int nglGpuDebugVertexShader::Shader = 0;      // ?Shader@nglGpuDebugVertexShader@@3KA
unsigned int* nglGpuColPixelShader::Shader = nullptr;  // ?Shader@nglGpuColPixelShader@@3PAKA
unsigned int* nglGpuTexPixelShader::Shader = nullptr;  // ?Shader@nglGpuTexPixelShader@@3PAKA
unsigned int* nglGpuTexColPixelShader::Shader = nullptr;  // ?Shader@nglGpuTexColPixelShader@@3PAKA
unsigned int* nglGpuFilterPixelShader::Shader = nullptr;  // ?Shader@nglGpuFilterPixelShader@@3PAKA
unsigned int* nglGpuZFogPixelShader::Shader = nullptr;    // ?Shader@nglGpuZFogPixelShader@@3PAKA
unsigned int* nglGpuDebugPixelShader::Shader = nullptr;   // ?Shader@nglGpuDebugPixelShader@@3PAKA
unsigned int* nglDOFPixelShader::Shader = nullptr;        // ?Shader@nglDOFPixelShader@@3PAKA
unsigned int* nglGlowShaderPixelFX::Shader = nullptr;     // ?Shader@nglGlowShaderPixelFX@@3PAKA
unsigned int* nglGlowShaderPixelPreFX::Shader = nullptr;  // ?Shader@nglGlowShaderPixelPreFX@@3PAKA
unsigned int* nglGlowShaderPixelPostFX::Shader = nullptr; // ?Shader@nglGlowShaderPixelPostFX@@3PAKA
// IDA declares one-element VS/PS storage arrays and one-element shader-table
// arrays for each internal shader. The original microcode payloads are not
// present in the Win32 port, so the table entries remain null.
static unsigned int nglGpuQuadPCVertexShaderVS[1] = {};
static unsigned int nglGpuQuadPCUVVertexShaderVS[1] = {};
static unsigned int nglGpuQuadPUVVertexShaderVS[1] = {};
static unsigned int nglGpuQuadPUV4VertexShaderVS[1] = {};
static unsigned int nglGpuQuadPUVMatColVertexShaderVS[1] = {};
static unsigned int nglGpuDebugVertexShaderVS[1] = {};
static unsigned int* nglGpuColPixelShaderPS[1] = {};
static unsigned int* nglGpuTexPixelShaderPS[1] = {};
static unsigned int* nglGpuTexColPixelShaderPS[1] = {};
static unsigned int* nglGpuFilterPixelShaderPS[1] = {};
static unsigned int* nglGpuZFogPixelShaderPS[1] = {};
static unsigned int* nglGpuDebugPixelShaderPS[1] = {};
static unsigned int* nglDOFPixelShaderPS[1] = {};
static unsigned int* nglGlowShaderPixelFXPS[1] = {};
static unsigned int* nglGlowShaderPixelPreFXPS[1] = {};
static unsigned int* nglGlowShaderPixelPostFXPS[1] = {};
static const unsigned int* nglGpuQuadPCVertexShaderTable[1] = {};
static const unsigned int* nglGpuQuadPCUVVertexShaderTable[1] = {};
static const unsigned int* nglGpuPUVVertexShaderTable[1] = {};
static const unsigned int* nglGpuPUV4VertexShaderTable[1] = {};
static const unsigned int* nglGpuPUVMatColVertexShaderTable[1] = {};
static const unsigned int* nglGpuDebugVertexShaderTable[1] = {};
static const unsigned int* nglGpuColPixelShaderTable[1] = {};
static const unsigned int* nglGpuTexPixelShaderTable[1] = {};
static const unsigned int* nglGpuTexColPixelShaderTable[1] = {};
static const unsigned int* nglGpuFilterPixelShaderTable[1] = {};
static const unsigned int* nglGpuZFogPixelShaderTable[1] = {};
static const unsigned int* nglGpuDebugPixelShaderTable[1] = {};
static const unsigned int* nglDOFPixelShaderTable[1] = {};
static const unsigned int* nglGlowShaderPixelFXTable[1] = {};
static const unsigned int* nglGlowShaderPixelPreFXTable[1] = {};
static const unsigned int* nglGlowShaderPixelPostFXTable[1] = {};
unsigned int* nglGpuQuadPCVertexShader::VS = nglGpuQuadPCVertexShaderVS;
unsigned int* nglGpuQuadPCUVVertexShader::VS = nglGpuQuadPCUVVertexShaderVS;
unsigned int* nglGpuQuadPUVVertexShader::VS = nglGpuQuadPUVVertexShaderVS;
unsigned int* nglGpuQuadPUV4VertexShader::VS = nglGpuQuadPUV4VertexShaderVS;
unsigned int* nglGpuQuadPUVMatColVertexShader::VS = nglGpuQuadPUVMatColVertexShaderVS;
unsigned int* nglGpuDebugVertexShader::VS = nglGpuDebugVertexShaderVS;
unsigned int** nglGpuColPixelShader::PS = nglGpuColPixelShaderPS;
unsigned int** nglGpuTexPixelShader::PS = nglGpuTexPixelShaderPS;
unsigned int** nglGpuTexColPixelShader::PS = nglGpuTexColPixelShaderPS;
unsigned int** nglGpuFilterPixelShader::PS = nglGpuFilterPixelShaderPS;
unsigned int** nglGpuZFogPixelShader::PS = nglGpuZFogPixelShaderPS;
unsigned int** nglGpuDebugPixelShader::PS = nglGpuDebugPixelShaderPS;
unsigned int** nglDOFPixelShader::PS = nglDOFPixelShaderPS;
unsigned int** nglGlowShaderPixelFX::PS = nglGlowShaderPixelFXPS;
unsigned int** nglGlowShaderPixelPreFX::PS = nglGlowShaderPixelPreFXPS;
unsigned int** nglGlowShaderPixelPostFX::PS = nglGlowShaderPixelPostFXPS;
const unsigned int** nglGpuQuadPCVertexShader::VShaderTable = nglGpuQuadPCVertexShaderTable;
const unsigned int** nglGpuQuadPCUVVertexShader::VShaderTable = nglGpuQuadPCUVVertexShaderTable;
const unsigned int** nglGpuQuadPUVVertexShader::VShaderTable = nglGpuPUVVertexShaderTable;
const unsigned int** nglGpuQuadPUV4VertexShader::VShaderTable = nglGpuPUV4VertexShaderTable;
const unsigned int** nglGpuQuadPUVMatColVertexShader::VShaderTable = nglGpuPUVMatColVertexShaderTable;
const unsigned int** nglGpuDebugVertexShader::VShaderTable = nglGpuDebugVertexShaderTable;
const unsigned int** nglGpuColPixelShader::PShaderTable = nglGpuColPixelShaderTable;
const unsigned int** nglGpuTexPixelShader::PShaderTable = nglGpuTexPixelShaderTable;
const unsigned int** nglGpuTexColPixelShader::PShaderTable = nglGpuTexColPixelShaderTable;
const unsigned int** nglGpuFilterPixelShader::PShaderTable = nglGpuFilterPixelShaderTable;
const unsigned int** nglGpuZFogPixelShader::PShaderTable = nglGpuZFogPixelShaderTable;
const unsigned int** nglGpuDebugPixelShader::PShaderTable = nglGpuDebugPixelShaderTable;
const unsigned int** nglDOFPixelShader::PShaderTable = nglDOFPixelShaderTable;
const unsigned int** nglGlowShaderPixelFX::PShaderTable = nglGlowShaderPixelFXTable;
const unsigned int** nglGlowShaderPixelPreFX::PShaderTable = nglGlowShaderPixelPreFXTable;
const unsigned int** nglGlowShaderPixelPostFX::PShaderTable = nglGlowShaderPixelPostFXTable;
unsigned int nglShaderParamSet::NumParams = 0;   // ?NumParams@nglShaderParamSet@@2IA
unsigned int nglSceneParamSet::NumParams = 0;    // ?NumParams@nglSceneParamSet@@2IA
unsigned int gpuHashIndexBuffer = 0;             // ?gpuHashIndexBuffer@@3IA
void* nglEmptyParamSet = nullptr;                // ?nglEmptyParamSet@@3PAXA (ngl_params.o @ 0x1241D70)
const _D3DVERTEXSHADERINPUT nglGpuPCVertexElements[3] = {
    {0, 0, 50, 0, 0},
    {0, 12, 64, 0, 0},
    {0, 0, 2, 0, 0},
};    // ?nglGpuPCVertexElements (ngl_gpu.o)
const _D3DVERTEXSHADERINPUT nglGpuPCUVVertexElements[4] = {
    {0, 0, 50, 0, 0},
    {0, 12, 64, 0, 0},
    {0, 16, 34, 0, 0},
    {0, 0, 2, 0, 0},
};  // ?nglGpuPCUVVertexElements (ngl_gpu.o)
const _D3DVERTEXSHADERINPUT nglGpuPUVVertexElements[3] = {
    {0, 0, 50, 0, 0},
    {0, 12, 34, 0, 0},
    {0, 0, 2, 0, 0},
};   // ?nglGpuPUVVertexElements (ngl_gpu.o)
const _D3DVERTEXSHADERINPUT nglGpuPUV4VertexElements[6] = {
    {0, 0, 50, 0, 0},
    {0, 12, 34, 0, 0},
    {1, 20, 34, 0, 0},
    {2, 28, 34, 0, 0},
    {3, 36, 34, 0, 0},
    {0, 0, 2, 0, 0},
};  // ?nglGpuPUV4VertexElements (ngl_gpu.o)
gpuVertexFormat nglGpuPCVertexFmt;    // ?nglGpuPCVertexFmt (ngl_gpu.o)
gpuVertexFormat nglGpuPUVVertexFmt;   // ?nglGpuPUVVertexFmt (ngl_gpu.o)
gpuVertexFormat nglGpuPUV4VertexFmt;  // ?nglGpuPUV4VertexFmt (ngl_gpu.o)

// ============================================================================
// Cross-object externs
// ============================================================================
extern int nglFrame;                                   // ngl_internal.o
extern nglDisplayModeType nglDisplayMode;              // ngl_internal.o
extern unsigned int nglDisplayMode_Set;                // ngl_internal.o
extern unsigned int nglVBlankCount;                    // ngl_internal.o
extern unsigned int nglFrameVBlankCount;               // ngl_internal.o
extern nglFont* nglSysFont;                            // ngl_font.o
extern nglScene* nglBuildScene;                        // ngl_scene.o
extern unsigned char* nglListWork;                     // ngl_scene.o
extern unsigned char* nglListWorkPos;                  // ngl_scene.o
extern int nglListWorkSize;                            // ngl_scene.o
extern nglPerfInfoStruct nglAvgPerfInfo;               // ngl_debug.o
extern nglDebugStruct nglDebug;                        // ngl_debug.o
extern void nglProfileShaders();                       // ngl_debug.o
extern void nglAveragePerfInfo(unsigned int Frames);   // ngl_debug.o
extern void nglInitShaderProfiling();                  // ngl_debug.o
extern void nglRenderPerfBar();                        // ngl_debug.o
extern void nglFlushLinesBatch();                      // ngl_debug.o
struct nglLightContext;
extern nglLightContext* nglCreateLightContext();       // ngl_lighting.o
extern struct nglLightContext* nglDefaultLightContext; // ngl_lighting.o
extern nglScene* nglListBeginScene(nglSceneParamType ParamSource);  // ngl_scene.o
extern void nglSetFBCopy(bool Upload, bool Download);  // ngl_scene.o
extern void nglSetZCopy(bool Upload, bool Download);   // ngl_scene.o
extern void nglRenderScene();                          // ngl_scene.o
extern void (*nglEndOfFrameCallback)(void* Data);      // ngl_scene.o
extern void* nglEndOfFrameData;                        // ngl_scene.o
extern void (*nglEndOfRenderCallback)(void* Data);     // ngl_scene.o
extern void* nglEndOfRenderData;                       // ngl_scene.o
extern void (*nglEndOfVBlankCallback)(void* Data);     // ngl_scene.o
extern void* nglEndOfVBlankData;                       // ngl_scene.o
extern unsigned int nglPushBufferSize;                 // ngl_dx_buf.o
extern unsigned int nglPushBufferKickoffSize;          // ngl_dx_buf.o
extern void ngliInitWorkBuffers();                     // ngl_dx_buf.o
extern void nglCreateFilterTextures(unsigned int FilterTexWidth,
                                    unsigned int FilterTexHeight);  // ngl_dx_filters.o
extern void nglPostProcessFiltersTex();                // ngl_dx_filters.o
extern void nglDxInitShaders(bool RegisterShaders);    // ngl_dx_shader.o
extern void nglDxUnbindTexStages();                    // ngl_dx_texture.o
extern void nglInitFrameBufferTexture();               // ngl_dx_tex_create.o
extern void nglUpdateInternalTextures();               // ngl_dx_tex_create.o
extern void nglScreenShot(const char* FileName);       // ngl_texture.o
extern void nglSceneDumpStart();                       // ngl_dx_scenedump.o
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlPrintf(const char* fmt, ...);
extern void tlFatal(const char* fmt, ...);

// ngl_font.o (declared here to avoid pulling nglRenderNode.h into TUs that
// also include aeps/apsRenderNode.h - the two headers define nglRenderNode).
struct nglFont;
extern nglFont* nglSysFont;
extern void nglGetStringDimensions(nglFont* Font, const char* Text, unsigned int* Width,
                                   unsigned int* Height, float ScaleX, float ScaleY);
extern void nglListAddString(nglFont* Font, const char* Text, float x, float y, float z,
                             unsigned int Color, float ScaleX, float ScaleY);

// ============================================================================
// Data (ngl_dx_core.o)
// ============================================================================
unsigned char nglGammaRamp[0x300];
int nglFenceEndOfRendering = -1;
nglFrameLockType nglFrameLock = NGLFL_ONE;
nglFrameLockType nglPrevFrameLock = (nglFrameLockType)-1;
unsigned int nglXbDisplayModeFlag[7] = { 0, 0, 0, 8, 2, 4, 0 };
nglDisplayModeType nglDisplayModes[6] = {
    { 0, 0, false, false, false, 0 },       // [0] unused
    { 640, 480, false, false, false, 0 },   // [1] 480i
    { 640, 576, false, true, false, 0 },    // [2] PAL
    { 640, 480, true, false, false, 0 },    // [3] 480p
    { 1280, 720, true, false, false, 0 },   // [4] 720p
    { 1920, 1080, true, false, false, 0 },  // [5] 1080p
};
_D3DPRESENT_PARAMETERS_ nglPresentParams;
_D3DCAPS8 nglDxCaps;
Direct3D* nglD3D = NULL;
gpuD3DDevice* nglDev = NULL;
unsigned int nglFrameLockImmediate = 0;
unsigned int nglFlipCycle = 0;
unsigned int nglLastFlipCycle = 0;

// ============================================================================
// nglDxCheckErrorD3D - ea: 0x83FF80
// ============================================================================
long nglDxCheckErrorD3D(long Status, const char* FileName, unsigned int Line) {
    char Message[512];
    if (Status != 0) {
        D3DXGetErrorStringA(Status, Message, 0x200u);
        tlFatal("Direct3D failure on line %d of file %s: %s\n", Line, FileName, Message);
        SetLastError(0);
        return Status;
    } else {
        SetLastError(0);
        return 0;
    }
}

// ============================================================================
// nglFlip - ea: 0x83FFE0
// ============================================================================
void nglFlip() {
    D3DDevice_Swap(0);
    if (nglFSAA != NGLFSAA_NONE)
        nglDxState.FSAAFixup();
    ++nglFrame;
}

// ============================================================================
// nglSetDisplayMode - ea: 0x840010
// ============================================================================
void nglSetDisplayMode(unsigned int* Modes, unsigned int ModeCount) {
    if (ModeCount == 0
        && _tlAssert("src/dx/ngl_dx_core.cpp", 357, "ModeCount > 0",
                     "No supported display modes specified"))
        __debugbreak();
    nglDisplayMode = nglDisplayModes[1];
    nglDisplayMode_Set = 1;
    XGetVideoStandard();
    unsigned int VideoFlags = XGetVideoFlags();
    unsigned int i = 0;
    if (ModeCount != 0) {
        for (;;) {
            if (Modes[i] >= 6
                && _tlAssert("src/dx/ngl_dx_core.cpp", 377,
                             "NGLFB_GET_FRONT(Modes[i]) < NGLFB_MAX",
                             "Invalid display mode enum."))
                __debugbreak();
            if ((VideoFlags & nglXbDisplayModeFlag[Modes[i]]) != 0)
                break;
            if (++i >= ModeCount) {
                nglDisplayMode.Widescreen = VideoFlags & 1;
                return;
            }
        }
        unsigned int Front = Modes[i];
        nglDisplayMode.Width = nglDisplayModes[Front].Width;
        nglDisplayMode.Height = nglDisplayModes[Front].Height;
        nglDisplayMode.Progressive = nglDisplayModes[Front].Progressive;
        nglDisplayMode.PAL = nglDisplayModes[Front].PAL;
        nglDisplayMode.Widescreen = nglDisplayModes[Front].Widescreen;
        nglDisplayMode.ImpersonateMode = nglDisplayModes[Front].ImpersonateMode;
        nglDisplayMode_Set = Modes[i];
    }
    nglDisplayMode.Widescreen = VideoFlags & 1;
}

// ============================================================================
// nglXbSetGammaRamp - ea: 0x840130
// ============================================================================
void nglXbSetGammaRamp(unsigned char* Ramp) {
    if (Ramp != NULL)
        D3DDevice_SetGammaRamp(0, (const _D3DGAMMARAMP*)Ramp);
}

// ============================================================================
// nglDxInitOcclusionQuery - ea: 0x840150
// ============================================================================
void nglDxInitOcclusionQuery() {
}

// ============================================================================
// nglXbInitPushBufferSize - ea: 0x840160
// ============================================================================
void nglXbInitPushBufferSize() {
    if (nglPushBufferSize < 0xFFFF
        && _tlAssert("src/dx/ngl_dx_core.cpp", 476, "nglPushBufferSize >= 65535",
                     "PushBufferSize must be at least 64 KB !"))
        __debugbreak();
    if (nglPushBufferSize % nglPushBufferKickoffSize != 0
        && _tlAssert("src/dx/ngl_dx_core.cpp", 477,
                     "nglPushBufferSize % nglPushBufferKickoffSize == 0",
                     "PushBufferSize must be a multiple of PushBufferKickoffSize !"))
        __debugbreak();
    if (nglPushBufferSize / nglPushBufferKickoffSize < 4
        && _tlAssert("src/dx/ngl_dx_core.cpp", 478,
                     "nglPushBufferSize / nglPushBufferKickoffSize >= 4",
                     "PushBufferSize / PushBufferKickoffSize must be at least 4 !"))
        __debugbreak();
    Direct3D_SetPushBufferSize(nglPushBufferSize, nglPushBufferKickoffSize);
}

// ============================================================================
// nglDxInitDisplayMode - ea: 0x840210
// ============================================================================
void nglDxInitDisplayMode() {
    if (nglDisplayMode_Set == 0) {
        unsigned int DefaultModes[2];
        DefaultModes[0] = 3;
        DefaultModes[1] = 1;
        nglSetDisplayMode(DefaultModes, 2);
    }
}

// ============================================================================
// nglDxInitPresentParams - ea: 0x840240
// ============================================================================
void nglDxInitPresentParams() {
    memset(&nglPresentParams, 0, sizeof(nglPresentParams));
    nglPresentParams.BackBufferHeight = nglDisplayMode.Height;
    nglPresentParams.BackBufferWidth = nglDisplayMode.Width;
    nglPresentParams.BackBufferFormat = D3DFMT_LIN_A8R8G8B8;
    nglPresentParams.BackBufferCount = 1;
    nglPresentParams.MultiSampleType = 17;
    nglPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    nglPresentParams.hDeviceWindow = NULL;
    nglPresentParams.Windowed = 0;
    nglPresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
    nglPresentParams.EnableAutoDepthStencil = 1;
    nglPresentParams.Flags = (nglDisplayMode.Progressive ? 64 : 32)
                             | (nglDisplayMode.Widescreen ? 16 : 256);
    nglPresentParams.FullScreen_PresentationInterval = 1;
    nglPresentParams.FullScreen_RefreshRateInHz = 0;
}

// ============================================================================
// nglRenderStartCallback / nglRenderFinishCallback - ea: 0x8402F0 / 0x840320
// ============================================================================
void nglRenderStartCallback(unsigned long Param) {
    nglPerfInfo.RenderStart = __rdtsc();
}

void nglRenderFinishCallback(unsigned long Param) {
    nglPerfInfo.RenderFinish = __rdtsc() - nglPerfInfo.RenderStart;
}

// ============================================================================
// nglVBlankCallback - ea: 0x840360
// ============================================================================
void nglVBlankCallback(_D3DVBLANKDATA* VBlankData) {
    ++nglVBlankCount;
}

// ============================================================================
// ngliLoadPhysicalSection / ngliUnloadPhysicalSection - ea: 0x840370 / 0x840380
// ============================================================================
void ngliLoadPhysicalSection(apk::apkFile* File, apk::apkFileSection* Section,
                             void* UserData) {
    __wbinvd();
}

void ngliUnloadPhysicalSection(apk::apkFile* File, apk::apkFileSection* Section,
                               void* UserData) {
}

// ============================================================================
// ngliPreInit - ea: 0x840390
// ============================================================================
void ngliPreInit() {
    nglD3D = Direct3DCreate8(0);
    nglXbInitPushBufferSize();
    if (nglDisplayMode_Set == 0) {
        unsigned int Modes[2];
        Modes[0] = 3;
        Modes[1] = 1;
        nglSetDisplayMode(Modes, 2);
    }
    nglDxInitPresentParams();
    tlPrintf("Creating the D3D device...\n");
    Direct3D_CreateDevice(0, D3DDEVTYPE_HAL, NULL, 0x40, &nglPresentParams,
                          (void**)&nglDev);
    nglGpuAcquireDevice();
    ngliInitWorkBuffers();
    D3DDevice_GetDeviceCaps(&nglDxCaps);
    nglDxState.Init();
    nglDxInitShaders(true);
    nglGpuInitShaders();
    D3DDevice_SetGammaRamp(0, (const _D3DGAMMARAMP*)nglGammaRamp);
    nglDxInitPalette();
    D3DDevice_SetVerticalBlankCallback(nglVBlankCallback);
    for (unsigned int i = 0; i < 4; ++i) {
        nglDxTexCache.Prev[i].Tex = NULL;
        nglDxTexCache.Prev[i].TexHash = -1;
        nglDxTexCache.Prev[i].WrapU = -1;
        nglDxTexCache.Prev[i].WrapV = -1;
        nglDxTexCache.Prev[i].WrapW = -1;
        nglDxTexCache.Prev[i].FilterFlags = -1;
    }
    nglInitFrameBufferTexture();
    nglDxSetFSAA(NGLFSAA_NONE);
    tlFixedString Name("physical");
    apk::apkRegisterSectionType(Name, ngliLoadPhysicalSection,
                                ngliUnloadPhysicalSection, NULL);
}

// ============================================================================
// ngliPostInit - ea: 0x8404C0
// ============================================================================
void ngliPostInit() {
    nglCreateFilterTextures(0x100, 0x100);
    nglGpuReleaseDevice();
}

// ============================================================================
// ngliExit - ea: 0x8404E0
// ============================================================================
void ngliExit() {
}

// ============================================================================
// nglDxSetFrameLockParams - ea: 0x8404F0
// ============================================================================
void nglDxSetFrameLockParams(nglFrameLockType flock) {
    unsigned int Interval = 1;
    switch (flock) {
    case NGLFL_NONE:
        nglFrameLockImmediate = 0;
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = 0x80000000;
        break;
    case NGLFL_ONE:
        nglFrameLockImmediate = 0;
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = 1;
        break;
    case NGLFL_TWO:
        nglFrameLockImmediate = 0;
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = 2;
        break;
    case NGLFL_ONE_OR_IMMEDIATE:
        nglFrameLockImmediate = 1;
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = 0x80000001;
        break;
    case NGLFL_TWO_OR_IMMEDIATE:
        nglFrameLockImmediate = 1;
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = 0x80000002;
        break;
    default:
        nglFrameLock = flock;
        nglPresentParams.FullScreen_PresentationInterval = Interval;
        break;
    }
}

// ============================================================================
// nglSetFrameLock - ea: 0x8405A0
// ============================================================================
nglFrameLockType nglSetFrameLock(nglFrameLockType flock) {
    nglFrameLockType Prev = nglPrevFrameLock;
    if (flock != nglPrevFrameLock) {
        nglGpuAcquireDevice();
        nglDxSetFrameLockParams(flock);
        unsigned int Interval = nglPresentParams.FullScreen_PresentationInterval;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PRESENTATIONINTERVAL, Interval) == 0)
            dword_BC2E0C = Interval;
        nglPrevFrameLock = flock;
        nglGpuReleaseDevice();
    }
    return Prev;
}

// ============================================================================
// nglWaitForRendering - ea: 0x840600
// ============================================================================
void nglWaitForRendering() {
    nglFrame += 2;
}

// ============================================================================
// ngliListInit - ea: 0x840610
// ============================================================================
void ngliListInit() {
    nglFrameVBlankCount = nglVBlankCount;
    nglPerfInfo.ListSubmitCycles = __rdtsc();
    nglListWorkPos = nglListWork;
    nglDefaultLightContext = nglCreateLightContext();
    if (nglSyncDebug.DumpFrameLog != 0)
        nglDebug.DumpFrameLog = 0;
    if (nglSyncDebug.DumpSceneFile != 0)
        nglDebug.DumpSceneFile = 0;
    if (nglSyncDebug.DumpTextures != 0)
        nglDebug.DumpTextures = 0;
    if (nglSyncDebug.ProfileShaders != 0) {
        nglDebug.ProfileShaders = 0;
        nglProfileShaders();
    }
    nglSyncDebug = nglDebug;
    nglBuildScene = NULL;
    nglListBeginScene(NGLSCENE_DEFAULTS);
    nglSetFBCopy(false, true);
    nglSetZCopy(false, false);
    nglSceneDumpStart();
    nglInitShaderProfiling();
}

// ============================================================================
// nglRenderPerfInfo - ea: 0x840700
// ============================================================================
void nglRenderPerfInfo() {
    char Work[2048];
    unsigned int w;
    unsigned int h;
    if (nglSyncDebug.ShowPerfInfo == 1) {
        sprintf(Work,
                "\x01[802020FF]\x02[1.1]NGL 3.0.0\x02[1]\x01[FFFFFFFF]\n"
                "%7.2f FPS\n%5.2fms CPU\n%5.2fms GPU\n%5.2fms SCENE BUILD\n"
                "%5.2fms SCENE SUBMIT\n% 7d POLYS\n% 7d NODES\nLIST % 8d/% 8d\n",
                nglAvgPerfInfo.FPS, nglAvgPerfInfo.CPUMS, nglAvgPerfInfo.RenderMS,
                nglAvgPerfInfo.ListSubmitMS, nglAvgPerfInfo.ListSendMS,
                nglAvgPerfInfo.TotalPolys, nglAvgPerfInfo.NodeCount,
                nglAvgPerfInfo.ListWorkUsage, nglListWorkSize);
    } else {
        sprintf(Work, "%.2f FPS\n%.2fms\n", nglAvgPerfInfo.FPS, nglAvgPerfInfo.RenderMS);
    }
    nglGetStringDimensions(nglSysFont, Work, &w, &h, 1.0f, 1.0f);
    nglQuad q;
    nglInitQuad(&q);
    float y2 = (float)(h + 20);
    float x2 = (float)nglGetScreenWidth() - 30.0f;
    float x1 = (float)(-50 - (int)w + nglGetScreenWidth());
    nglSetQuadRect(&q, x1, 20.0f, x2, y2);
    nglSetQuadColor(&q, 0xC0000000);
    nglSetQuadZ(&q, -9999.0f);
    nglListAddQuad(&q);
    x1 = (float)(-40 - (int)w + nglGetScreenWidth());
    nglListAddString(nglSysFont, Work, x1, 30.0f, -9999.0f, 0xFFFFFFFF, 1.0f, 1.0f);
}

// ============================================================================
// nglRenderDebug - ea: 0x8408B0
// ============================================================================
void nglRenderDebug() {
    if (nglSyncDebug.ShowPerfInfo != 0)
        nglRenderPerfInfo();
    if (nglSyncDebug.ShowPerfBar != 0)
        nglRenderPerfBar();
}

// ============================================================================
// nglListSendBatch - ea: 0x8408D0
// ============================================================================
void nglListSendBatch(jqBatch* Batch) {
    float Const[4];
    nglGpuAcquireDevice();
    if (nglSyncDebug.DumpFrameLog != 0)
        tlPrintf("LOG: ============================= Frame log start ===========================\n");
    Const[0] = 0.0f;
    Const[1] = 0.0f;
    Const[2] = nglFSAAParams.v.m128_f32[0];
    Const[3] = nglFSAAParams.v.m128_f32[1];
    D3DDevice_SetVertexShaderConstant1Fast(0, Const);
    nglFlushLinesBatch();
    for (unsigned int i = 0; i < 4; ++i) {
        nglDxTexCache.Prev[i].Tex = NULL;
        nglDxTexCache.Prev[i].TexHash = -1;
        nglDxTexCache.Prev[i].WrapU = -1;
        nglDxTexCache.Prev[i].WrapV = -1;
        nglDxTexCache.Prev[i].WrapW = -1;
        nglDxTexCache.Prev[i].FilterFlags = -1;
    }
    nglPerfInfo.ListSubmitCycles = __rdtsc() - nglPerfInfo.ListSubmitCycles;
    nglPerfInfo.ListSubmitMS = (float)(nglPerfInfo.ListSubmitCycles * 0.0000013636364);
    nglPerfInfo.ListSendCycles = __rdtsc();
    D3DDevice_InsertCallback(
        D3DCALLBACK_WRITE,
        reinterpret_cast<void (*)(unsigned int)>(nglRenderStartCallback), 0);
    D3DDevice_BeginScene();
    nglRenderScene();
    nglPostProcessFiltersTex();
    D3DDevice_EndScene();
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
    gpuHashIndexBuffer = 0;
    D3DDevice_SetIndices(NULL, 0);
    nglDxUnbindTexStages();
    D3DDevice_SetVertexShader(0);
    D3DDevice_SetPixelShaderProgram(NULL);
    gpuHashPixelShader = 0;
    gpuHashVertexShader = 0;
    nglPerfInfo.ListSendCycles = __rdtsc() - nglPerfInfo.ListSendCycles;
    nglPerfInfo.ListWorkUsage = (unsigned int)(nglListWorkPos - nglListWork);
    nglPerfInfo.ListSendMS = (float)(nglPerfInfo.ListSendCycles * 0.0000013636364);
    if (nglEndOfFrameCallback != NULL)
        nglEndOfFrameCallback(nglEndOfFrameData);
    D3DDevice_InsertCallback(
        D3DCALLBACK_WRITE,
        reinterpret_cast<void (*)(unsigned int)>(nglRenderFinishCallback), 0);
    if (nglFenceEndOfRendering != -1)
        D3DDevice_BlockOnFence(nglFenceEndOfRendering);
    nglFenceEndOfRendering = D3DDevice_InsertFence();
    if (nglEndOfRenderCallback != NULL)
        nglEndOfRenderCallback(nglEndOfRenderData);
    nglPerfInfo.CPUMS = (float)((__rdtsc() - nglPerfInfo.CPUStart) * 0.0000013636364);
    if (nglEndOfVBlankCallback != NULL)
        nglEndOfVBlankCallback(nglEndOfVBlankData);
    D3DDevice_Swap(0);
    if (nglFSAA != NGLFSAA_NONE)
        nglDxState.FSAAFixup();
    ++nglFrame;
    nglPerfInfo.FPS = 1000.0f / nglPerfInfo.CPUMS;
    nglPerfInfo.TotalMS = nglPerfInfo.TotalMS + nglPerfInfo.CPUMS;
    nglPerfInfo.CPUStart = __rdtsc();
    nglPerfInfo.FrameMS = nglPerfInfo.CPUMS;
    nglPerfInfo.RenderMS = nglPerfInfo.CPUMS;
    nglPerfInfo.TotalSeconds = nglPerfInfo.TotalMS * 0.001f;
    if (nglDebug.ScreenShot != 0) {
        nglScreenShot(NULL);
        nglDebug.ScreenShot = 0;
    }
    nglSyncPerfInfo = nglPerfInfo;
    nglPerfInfo.NodeCount = 0;
    nglPerfInfo.TotalVerts = 0;
    nglPerfInfo.TotalPolys = 0;
    nglAveragePerfInfo(60);
    nglBuildScene = NULL;
    nglGpuReleaseDevice();
}

// ============================================================================
// ngliWaitForResource - ea: 0x840CE0
// ============================================================================
void ngliWaitForResource() {
    nglGpuAcquireDevice();
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
    gpuHashIndexBuffer = 0;
    D3DDevice_SetIndices(NULL, 0);
    nglDxUnbindTexStages();
    D3DDevice_BlockUntilIdle();
    nglGpuReleaseDevice();
}

// ============================================================================
// nglDxResetDevice - ea: 0x840D30
// ============================================================================
void nglDxResetDevice() {
    nglDxSetFrameLockParams(nglFrameLock);
    D3DDevice_PersistDisplay();
    D3DDevice_Reset(&nglPresentParams);
    nglUpdateInternalTextures();
}
