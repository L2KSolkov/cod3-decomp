// ============================================================================
// ngl_scene.cpp - NGL scene graph + matrix management (67 funcs).
// Source: src/ngl_scene.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_scene.o).
// ============================================================================

#include "ngl/ngl_scene.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglTexture.h"
#include "ngl/nglDebug.h"
#include "core/tlFixedString.h"

#include <intrin.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglDebugStruct nglSyncDebug;                    // ngl_debug.o
class nglRenderNode;
extern void nglListAddNode(nglRenderNode* Node);
extern nglTexture* nglGetFrontBufferTex();             // ngl_dx_texture.o
extern nglTexture* nglGetBackBufferTex();              // ngl_dx_texture.o
extern void nglInitQuad(nglQuad* Quad);                // ngl_quad.o
extern void nglSetQuadTex(nglQuad* Quad, nglTexture* Tex);  // ngl_quad.o
extern void nglSetQuadRect(nglQuad* Quad, float x1, float y1, float x2, float y2);  // ngl_quad.o
extern void nglListAddQuad(nglQuad* Quad);             // ngl_quad.o
extern void nglScreenShot(const char* FileName);       // ngl_texture.o
extern bool nglIsDisplayWidescreen();                   // ngl_internal.o
extern int nglGetScreenWidth();                         // ngl_internal.o
extern int nglGetScreenHeight();                        // ngl_internal.o
extern float nglGetVBlankMS();                          // ngl_internal.o
extern unsigned int nglFlipCycle;                       // ngl_dx_core.o
extern nglFrameLockType nglFrameLock;                   // ngl_dx_core.o
extern nglLightContext* nglDefaultLightContext;         // ngl_lighting.o
extern void nglSetLightContext(nglLightContext* Context, nglScene* Scene);  // ngl_lighting.o
extern void tlFatal(const char* fmt, ...);
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlMemFree(void* Ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern int nglFrame;  // ?nglFrame@@3HA (ngl_internal.o)

// ea: 0x660140 (game.o inline COMDAT)
void* nglListAlloc(unsigned int Bytes, unsigned int Alignment)
{
    char* result = (char*)((~(Alignment - 1))
                           & (uintptr_t)&nglListWorkPos[Alignment - 1]);
    if (result + Bytes <= (char*)nglListWork + nglListWorkSize)
    {
        nglListWorkPos = (unsigned char*)result + Bytes;
    }
    else
    {
        if (nglLastListAllocWarnFrame != nglFrame)
        {
            tlFatal(
                "Render list allocation overflow. Reserved = %d Requested = %d Free = %d.\n",
                nglListWorkSize, Bytes,
                (int)(((char*)nglListWork + nglListWorkSize) - result));
            nglLastListAllocWarnFrame = nglFrame;
        }
        return nullptr;
    }
    return result;
}

// ngli* helpers (ngl_dx_scene.o, ported later).
extern void ngliSetRenderTarget(nglTexture* Tex);
extern void ngliSetZTarget(nglTexture* Tex);
extern void ngliSetClearStencil(unsigned int Stencil);
extern void ngliEnableFog(bool Enable);
extern void ngliEnableDepthOfField(bool Enable);
extern void ngliSetupBeginSceneDefaults(nglScene* Scene);
extern void ngliSetupBeginScene(nglScene* Scene);
extern void ngliListInit();
extern void ngliListSend();
extern void ngliSetDefaultSceneParams();
extern math::Mat44* ngliGetDeviceMatrix(math::Mat44* result,
                                        nglTexture* RenderTarget);
extern void ngliRenderSceneNode(void* Data);
extern void nglSceneDumpCamera(const math::Mat43* WorldToView);


// ngl_sort helpers (ngl_scene.o inline COMDATs).
struct nglOpaqueCompare { int dummy; };
struct nglTransCompare { int dummy; };
extern void nglSortList_Impl(nglRenderNode** List, int Count);

// ============================================================================
// Data (ngl_scene.o)
// ============================================================================
nglScene* nglBuildScene = NULL;
nglScene* nglRootBuildScene = NULL;
unsigned char* nglListWork = NULL;
unsigned char* nglListWorkPos = NULL;
int nglListWorkSize = 0;
int nglLastListAllocWarnFrame = 0;
void (*nglEndOfRenderCallback)(void* Data) = NULL;
void* nglEndOfRenderData = NULL;
void (*nglEndOfFrameCallback)(void* Data) = NULL;
void* nglEndOfFrameData = NULL;
void (*nglEndOfVBlankCallback)(void* Data) = NULL;
void* nglEndOfVBlankData = NULL;

namespace nglHiresScreenShot {
bool ScreenshotInProgress = false;
unsigned int CurTilesCount = 0;
unsigned int TotalTilesCount = 0;
unsigned int NColumns = 0;
unsigned int NRows = 0;
float* xx1 = NULL;
float* yy1 = NULL;
float* xx2 = NULL;
float* yy2 = NULL;
int ShotCount = 0;
}

// ============================================================================
// nglRenderCallbackNode - 32 bytes
// ============================================================================
struct nglRenderCallbackNode : nglRenderNode {
    int           Type;      // +0x0C
    void (*Fn)(void*);       // +0x10
    void*         Data;      // +0x14
    nglSortInfo   SortInfo;  // +0x18
};
static_assert(sizeof(nglRenderCallbackNode) == 0x20, "nglRenderCallbackNode size mismatch");

extern void nglListAddNode(nglRenderNode* Node);  // ngl_scene.o

// ============================================================================
// nglSetRenderTarget / nglSetZTarget / nglSetClearStencil
// ============================================================================
void nglSetRenderTarget(nglTexture* Tex) { ngliSetRenderTarget(Tex); }
void nglSetZTarget(nglTexture* Tex) { ngliSetZTarget(Tex); }
void nglSetClearStencil(unsigned int Stencil) { ngliSetClearStencil(Stencil); }

// ============================================================================
// Scene-state setters
// ============================================================================
void nglSetClearFlags(unsigned int ClearFlags) { nglBuildScene->ClearFlags = ClearFlags; }
void nglSetClearColor(float r, float g, float b, float a) {
    nglBuildScene->ClearColor.v = _mm_setr_ps(r, g, b, a);
}
void nglSetClearZ(float Z) { nglBuildScene->ClearZ = Z; }
void nglSetFBWriteMask(unsigned int WriteMask) { nglBuildScene->FBWriteMask = WriteMask; }
void nglSetZWriteEnable(bool Enable) { nglBuildScene->ZWriteEnable = Enable; }
void nglSetZTestEnable(bool Enable) { nglBuildScene->ZTestEnable = Enable; }
void nglSetFBCopy(bool Upload, bool Download) {
    nglBuildScene->UploadFB = Upload;
    nglBuildScene->DownloadFB = Download;
}
void nglSetZCopy(bool Upload, bool Download) {
    nglBuildScene->UploadZ = Upload;
    nglBuildScene->DownloadZ = Download;
}
void nglSetRenderTiling(int Tiles) { nglBuildScene->Tiles = Tiles; }
void nglEnableFog(bool Enable) {
    nglBuildScene->FogEnabled = Enable;
    ngliEnableFog(Enable);
}
void nglSetFogColor(float r, float g, float b) {
    nglBuildScene->FogColor.v = _mm_setr_ps(r, g, b, 1.0f);
}
void nglSetFogRange(float Near, float Far, float Min, float Max) {
    if (Far < Near
        && _tlAssert("src/ngl_scene.cpp", 257, "Far >= Near",
                     "Fog Far must be greater or equal to Near."))
        __debugbreak();
    if (Max < Min
        && _tlAssert("src/ngl_scene.cpp", 258, "Max >= Min",
                     "Fog Max must be greater or equal to Min. "))
        __debugbreak();
    float NearZ = Near;
    if (Near <= nglBuildScene->NearZ)
        NearZ = nglBuildScene->NearZ;
    nglBuildScene->FogNear = NearZ;
    nglBuildScene->FogFar = Far;
    nglBuildScene->FogMin = Min;
    nglBuildScene->FogMax = Max;
}
void nglEnableDepthOfField(bool Enable) {
    nglBuildScene->DepthOfFieldEnabled = Enable;
    ngliEnableDepthOfField(Enable);
}
void nglSetFocusDepth(float Depth) { nglBuildScene->FocusDepth = Depth; }
void nglSetAnimTime(float Time) { nglBuildScene->AnimTime = Time; }

// ============================================================================
// Matrix helpers
// ============================================================================
math::Mat44* Perspective(math::Mat44* result, float hs, float vs, float zn, float zf) {
    float zc = zf / (zf - zn);
    float c = -zc * zn;
    result->x.v = _mm_setr_ps(hs, 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, vs, 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, zc, 1.0f);
    result->w.v = _mm_setr_ps(0.0f, 0.0f, c, 0.0f);
    return result;
}

math::Mat44* Ortho(math::Mat44* result, float ax, float ay, float zn, float zf) {
    float zc = 1.0f / (zf - zn);
    float c = -zc * zn;
    result->x.v = _mm_setr_ps(ax, 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, ay, 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, zc, 0.0f);
    result->w.v = _mm_setr_ps(0.0f, 0.0f, c, 1.0f);
    return result;
}

math::Mat44* Viewport(math::Mat44* result, float x1, float y1, float x2, float y2) {
    result->x.v = _mm_setr_ps((x2 - x1) * 0.5f, 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, (y1 - y2) * 0.5f, 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    result->w.v = _mm_setr_ps((x1 + x2) * 0.5f, (y1 + y2) * 0.5f, 1.0f, 1.0f);
    return result;
}

math::Mat44* InvScissor(math::Mat44* result, float sx1, float sy1, float sx2, float sy2) {
    result->x.v = _mm_setr_ps(2.0f / (sx2 - sx1), 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, 2.0f / (sy2 - sy1), 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    result->w.v = _mm_setr_ps(-(sx1 + sx2) / (sx2 - sx1),
                              -(sy1 + sy2) / (sy2 - sy1), 1.0f, 1.0f);
    return result;
}

static math::Mat44* ViewportToWorldImpl(math::Mat44* result, nglScene* Scene);
math::Mat44* ViewportToWorld(math::Mat44* result, nglScene* Scene) {
    return ViewportToWorldImpl(result, Scene);
}

// ============================================================================
// UI - ea: 0x83AC40
// ============================================================================
math::Mat44* UI(math::Mat44* result, nglScene* Scene) {
    float v3, v4;
    nglTexture* RenderTarget = Scene->RenderTarget;
    if (RenderTarget == NULL) {
        if (Scene->ZTarget == NULL) {
            v3 = 1.0f;
            v4 = 1.0f;
        } else {
            v3 = (float)Scene->ZTarget->Width * 0.5f;
            v4 = (float)Scene->ZTarget->Height * 0.5f;
        }
    } else if ((RenderTarget->Flags & 0x2000) == 0) {
        v3 = (float)Scene->RenderTarget->Width * 0.5f;
        v4 = (float)Scene->RenderTarget->Height * 0.5f;
    } else {
        v3 = 320.0f;
        v4 = 240.0f;
    }
    result->x.v = _mm_setr_ps(1.0f / v3, 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, 1.0f / v4, 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    result->w.v = _mm_setr_ps(-1.0f, -1.0f, 0.0f, 1.0f);
    return result;
}

// ============================================================================
// View setters
// ============================================================================
void nglSetView(float x1, float y1, float x2, float y2) {
    nglBuildScene->vx1 = x1;
    nglBuildScene->vy1 = y1;
    nglBuildScene->vx2 = x2;
    nglBuildScene->vy2 = y2;
    nglBuildScene->MatricesDirty = true;
}
void nglSetAspectRatio(float a) {
    nglBuildScene->AspectRatio = a;
    nglBuildScene->MatricesDirty = true;
}
void nglSetPerspectiveMatrix(float fov, float nearz, float farz) {
    if ((nearz <= 0.0f || nearz >= farz)
        && _tlAssert("src/ngl_scene.cpp", 674, "nearz > 0.0f && farz > nearz",
                     "Invalid projection parameters."))
        __debugbreak();
    nglBuildScene->ProjType = NGLPROJ_PERSPECTIVE;
    nglBuildScene->FOV = fov;
    nglBuildScene->NearZ = nearz;
    nglBuildScene->FarZ = farz;
    nglBuildScene->MatricesDirty = true;
}
void nglSetOrthoMatrix(float nearz, float farz) {
    if (farz <= nearz
        && _tlAssert("src/ngl_scene.cpp", 689, "farz > nearz",
                     "Invalid projection parameters."))
        __debugbreak();
    nglBuildScene->ProjType = NGLPROJ_ORTHOGRAPHIC;
    nglBuildScene->FOV = 0.0f;
    nglBuildScene->NearZ = nearz;
    nglBuildScene->FarZ = farz;
    nglBuildScene->MatricesDirty = true;
}
void nglSetWorldToViewMatrix(const math::Mat43* WorldToView) {
    __m128 v2 = _mm_mul_ps(WorldToView->x.v, WorldToView->x.v);
    float v5 = v2.m128_f32[0] + (v2.m128_f32[1] + v2.m128_f32[2]);
    __m128 v3 = _mm_mul_ps(WorldToView->y.v, WorldToView->y.v);
    float v6 = v3.m128_f32[0] + (v3.m128_f32[1] + v3.m128_f32[2]);
    __m128 v4 = _mm_mul_ps(WorldToView->z.v, WorldToView->z.v);
    if (fabsf((v4.m128_f32[0] + (v4.m128_f32[1] + v4.m128_f32[2])) - 1.0f)
            + fabsf(v6 - 1.0f) + fabsf(v5 - 1.0f) >= 0.0099999998f
        && _tlAssert(
            "src/ngl_scene.cpp", 705,
            "fabsf(AbsSquared(WorldToView.GetX())-1.0f) + fabsf(AbsSquared(WorldToView.GetY())-1.0f) + fabsf(AbsSquared(WorldToView.GetZ())-1.0f) < 0.01f",
            "Invalid scale detected in camera transform.\n"))
        __debugbreak();
    nglBuildScene->WorldToView = *WorldToView;
    nglBuildScene->MatricesDirty = true;
    if (nglSyncDebug.DumpSceneFile != 0)
        nglSceneDumpCamera(WorldToView);
}
void nglSetCameraMatrix(const math::Mat43* CameraToWorld) {
    // WorldToView = transpose(CameraToWorld) with negated translation.
    math::Mat43 v9;
    __m128 y = CameraToWorld->y.v;
    __m128 z = CameraToWorld->z.v;
    __m128 x = CameraToWorld->x.v;
    __m128 w = CameraToWorld->w.v;
    __m128 v5 = _mm_shuffle_ps(x, y, 0x44);
    v9.x.v = _mm_shuffle_ps(v5, z, 0x88);
    v9.y.v = _mm_shuffle_ps(v5, z, 0xDD);
    v9.z.v = _mm_shuffle_ps(_mm_shuffle_ps(x, y, 0xEE), z, 0xA8);
    v9.w.v = _mm_xor_ps(
        _mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
        _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(w, w, 0), v9.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(w, w, 85), v9.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(w, w, 170), v9.z.v)));
    nglSetWorldToViewMatrix(&v9);
}

// ============================================================================
// Callbacks
// ============================================================================
void nglSetSceneCallBack(nglSceneCallbackType Type, void (*Fn)(void*), void* Data) {
    nglSceneCallback* cb = NULL;
    const char* expr = NULL;
    const char* desc = NULL;
    switch (Type) {
    case NGLSCENE_PRE: cb = &nglBuildScene->Pre; expr = "Fn == NULL || nglBuildScene->Pre.Fn == NULL"; desc = "Only one PRE callback allowed per scene."; break;
    case NGLSCENE_MID: cb = &nglBuildScene->Mid; expr = "Fn == NULL || nglBuildScene->Mid.Fn == NULL"; desc = "Only one MID callback allowed per scene."; break;
    case NGLSCENE_POST: cb = &nglBuildScene->Post; expr = "Fn == NULL || nglBuildScene->Post.Fn == NULL"; desc = "Only one POST callback allowed per scene."; break;
    case NGLSCENE_STARTSCENE: cb = &nglBuildScene->StartScene; expr = "Fn == NULL || nglBuildScene->StartScene.Fn == NULL"; desc = "Only one STARTSCENE callback allowed per scene."; break;
    case NGLSCENE_SETUPSCENE: cb = &nglBuildScene->SetupScene; expr = "Fn == NULL || nglBuildScene->SetupScene.Fn == NULL"; desc = "Only one SETUPSCENE callback allowed per scene."; break;
    default:
        tlFatal("Unknown type for scene callback.");
        return;
    }
    if (Fn != NULL && cb->Fn != NULL)
        _tlAssert("src/ngl_scene.cpp", 859, expr, desc);
    cb->Fn = Fn;
    cb->Data = Data;
}

void nglSetEndOfRenderCallback(void (*Fn)(void*), void* Data) {
    nglEndOfRenderCallback = Fn;
    nglEndOfRenderData = Data;
}
void nglSetEndOfFrameCallback(void (*Fn)(void*), void* Data) {
    nglEndOfFrameCallback = Fn;
    nglEndOfFrameData = Data;
}
void nglSetEndOfVBlankCallback(void (*Fn)(void*), void* Data) {
    nglEndOfVBlankCallback = Fn;
    nglEndOfVBlankData = Data;
}

float nglGetRemainingFrameTime() {
    float LastFlip = (float)nglFlipCycle * 0.0000013636364f;
    float NextFlip = nglGetVBlankMS() * (float)nglFrameLock + LastFlip;
    return NextFlip - (float)__rdtsc() * 0.0000013636364f;
}

nglScene* nglListEndScene() {
    nglScene* result = nglBuildScene;
    if (nglBuildScene == nglRootBuildScene)
        tlFatal("Scene stack underflow (too many nglListEndScene calls!).\n");
    nglBuildScene = nglBuildScene->Parent;
    return result;
}

nglScene* nglListSelectScene(nglScene* scene) {
    nglScene* result = nglBuildScene;
    nglBuildScene = scene;
    return result;
}

bool nglHiresScreenShotInProgress() {
    return nglHiresScreenShot::ScreenshotInProgress;
}
unsigned int nglHiresScreenShotNumColumns() { return nglHiresScreenShot::NColumns; }
unsigned int nglHiresScreenShotNumRows() { return nglHiresScreenShot::NRows; }

void nglBeginHiresScreenShot(unsigned int Width, unsigned int Height) {
    nglHiresScreenShot::ScreenshotInProgress = true;
    nglHiresScreenShot::CurTilesCount = 0;
    unsigned int ScreenWidth = nglGetScreenWidth();
    unsigned int ScreenHeight = nglGetScreenHeight();
    nglHiresScreenShot::NColumns = ScreenWidth * (Width / ScreenWidth) / ScreenWidth;
    nglHiresScreenShot::NRows = ScreenHeight * (Height / ScreenHeight) / ScreenHeight;
    nglHiresScreenShot::TotalTilesCount =
        nglHiresScreenShot::NColumns * nglHiresScreenShot::NRows;
    unsigned int Count = nglHiresScreenShot::TotalTilesCount;
    nglHiresScreenShot::xx1 = (float*)tlMemAlloc(4 * Count, 8, 0x1000000);
    nglHiresScreenShot::yy1 = (float*)tlMemAlloc(4 * Count, 8, 0x1000000);
    nglHiresScreenShot::xx2 = (float*)tlMemAlloc(4 * Count, 8, 0x1000000);
    nglHiresScreenShot::yy2 = (float*)tlMemAlloc(4 * Count, 8, 0x1000000);
    for (unsigned int r = 0; r < nglHiresScreenShot::NRows; ++r) {
        for (unsigned int c = 0; c < nglHiresScreenShot::NColumns; ++c) {
            nglHiresScreenShot::xx1[c + r * nglHiresScreenShot::NColumns] = -(float)(2 * c + 1);
            nglHiresScreenShot::yy1[c + r * nglHiresScreenShot::NColumns] = -(float)(2 * r + 1);
            nglHiresScreenShot::xx2[c + r * nglHiresScreenShot::NColumns] =
                (float)(2 * (int)(nglHiresScreenShot::NColumns - c) - 1);
            nglHiresScreenShot::yy2[c + r * nglHiresScreenShot::NColumns] =
                (float)(2 * (int)(nglHiresScreenShot::NRows - r) - 1);
        }
    }
}

bool nglSaveHiresScreenshot() {
    char buf[64];
    sprintf(buf, "BigScreenShot%4.4dw%2.2dh%2.2dr%2.2dc%2.2d",
            nglHiresScreenShot::ShotCount, nglHiresScreenShot::NColumns,
            nglHiresScreenShot::NRows,
            nglHiresScreenShot::CurTilesCount / nglHiresScreenShot::NColumns,
            nglHiresScreenShot::CurTilesCount % nglHiresScreenShot::NColumns);
    nglScreenShot(buf);
    if (++nglHiresScreenShot::CurTilesCount == nglHiresScreenShot::TotalTilesCount) {
        ++nglHiresScreenShot::ShotCount;
        tlMemFree(nglHiresScreenShot::xx1);
        tlMemFree(nglHiresScreenShot::yy1);
        tlMemFree(nglHiresScreenShot::xx2);
        tlMemFree(nglHiresScreenShot::yy2);
        return false;
    }
    return true;
}

nglScene* nglAdjustViewForHiresScreenshot() {
    nglBuildScene->vx1 = nglHiresScreenShot::xx1[nglHiresScreenShot::CurTilesCount];
    nglBuildScene->vy1 = nglHiresScreenShot::yy1[nglHiresScreenShot::CurTilesCount];
    nglBuildScene->vx2 = nglHiresScreenShot::xx2[nglHiresScreenShot::CurTilesCount];
    nglBuildScene->vy2 = nglHiresScreenShot::yy2[nglHiresScreenShot::CurTilesCount];
    nglBuildScene->MatricesDirty = true;
    return nglBuildScene;
}

void nglSyncFrameBuffers() {
    ngliListInit();
    nglBuildScene->ClearFlags = 0;
    nglBuildScene->ZTestEnable = false;
    nglBuildScene->ZWriteEnable = false;
    nglQuad q;
    nglInitQuad(&q);
    nglSetQuadTex(&q, nglGetFrontBufferTex());
    nglSetQuadRect(&q, 0.0f, 0.0f, (float)nglGetScreenWidth(), (float)nglGetScreenHeight());
    nglListAddQuad(&q);
    ngliListSend();
}

void nglLockTexture(nglTexture* Tex) {
    if ((Tex->Flags & 1) != 0) {
        nglLockedTextureNode* v1 = (nglLockedTextureNode*)nglListAlloc(0x0C, 0x10);
        if (v1 != NULL) {
            v1->Tex = Tex;
            v1->Next = nglBuildScene->LockedTextures;
            nglBuildScene->LockedTextures = v1;
        }
    }
}

// ============================================================================
// Scissor / viewport
// ============================================================================
static float ClampCoord(float v) {
    return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v);
}

void nglSetScissorWH(float x1, float y1, float x2, float y2, float w, float h) {
    nglBuildScene->sx1 = ClampCoord(x1);
    nglBuildScene->sy1 = ClampCoord(y1);
    nglBuildScene->sx2 = ClampCoord(x2);
    nglBuildScene->sy2 = ClampCoord(y2);
    nglBuildScene->ViewX1 = (int)(((x1 + 1.0f) * 0.5f) * w + 0.5f);
    nglBuildScene->ViewX2 = (int)(((x2 + 1.0f) * 0.5f) * w + 0.5f);
    nglBuildScene->ViewY1 = (int)(((y1 + 1.0f) * 0.5f) * h + 0.5f);
    nglBuildScene->ViewY2 = (int)(((y2 + 1.0f) * 0.5f) * h + 0.5f);
    nglBuildScene->MatricesDirty = true;
}

void nglSetScissor(float x1, float y1, float x2, float y2) {
    nglBuildScene->sx1 = ClampCoord(x1);
    nglBuildScene->sy1 = ClampCoord(y1);
    nglBuildScene->sx2 = ClampCoord(x2);
    nglBuildScene->sy2 = ClampCoord(y2);
    nglTexture* RenderTarget = nglBuildScene->RenderTarget;
    float Width, Height;
    if (RenderTarget == NULL || (RenderTarget->Flags & 0x2000) != 0) {
        Width = (float)nglGetScreenWidth();
        Height = (float)nglGetScreenHeight();
    } else {
        Width = (float)RenderTarget->Width;
        Height = (float)RenderTarget->Height;
    }
    nglBuildScene->ViewX1 = (int)(((x1 + 1.0f) * 0.5f) * Width + 0.5f);
    nglBuildScene->ViewX2 = (int)(((x2 + 1.0f) * 0.5f) * Width + 0.5f);
    nglBuildScene->ViewY1 = (int)(((y1 + 1.0f) * 0.5f) * Height + 0.5f);
    nglBuildScene->ViewY2 = (int)(((y2 + 1.0f) * 0.5f) * Height + 0.5f);
    nglBuildScene->MatricesDirty = true;
}

void nglSetViewport(float x1, float y1, float x2, float y2) {
    nglTexture* RenderTarget = nglBuildScene->RenderTarget;
    if (RenderTarget != NULL) {
        float ScreenWidth, ScreenHeight;
        if ((RenderTarget->Flags & 0x2000) != 0) {
            ScreenWidth = (float)nglGetScreenWidth();
            ScreenHeight = (float)nglGetScreenHeight();
        } else {
            ScreenWidth = (float)RenderTarget->Width;
            ScreenHeight = (float)RenderTarget->Height;
        }
        float invW = 1.0f / ScreenWidth;
        float invH = 1.0f / ScreenHeight;
        float vx1 = (invW * x1) * 2.0f - 1.0f;
        float vy1 = (invH * y1) * 2.0f - 1.0f;
        float vx2 = ((x2 + 1.0f) * invW) * 2.0f - 1.0f;
        float vy2 = ((y2 + 1.0f) * invH) * 2.0f - 1.0f;
        nglBuildScene->vx1 = vx1;
        nglBuildScene->vy1 = vy1;
        nglBuildScene->vx2 = vx2;
        nglBuildScene->vy2 = vy2;
        nglBuildScene->MatricesDirty = true;
        nglSetScissor(vx1, vy1, vx2, vy2);
    } else {
        float w = x2 - x1;
        float h = y2 - y1;
        float invW = 1.0f / w;
        float invH = 1.0f / h;
        float vx1 = (invW * x1) * 2.0f - 1.0f;
        float vy1 = (invH * y1) * 2.0f - 1.0f;
        float vx2 = ((x2 + 1.0f) * invW) * 2.0f - 1.0f;
        float vy2 = ((y2 + 1.0f) * invH) * 2.0f - 1.0f;
        nglBuildScene->vx1 = vx1;
        nglBuildScene->vy1 = vy1;
        nglBuildScene->vx2 = vx2;
        nglBuildScene->vy2 = vy2;
        nglBuildScene->MatricesDirty = true;
        nglBuildScene->sx1 = ClampCoord(vx1);
        nglBuildScene->sy1 = ClampCoord(vy1);
        nglBuildScene->sx2 = ClampCoord(vx2);
        nglBuildScene->sy2 = ClampCoord(vy2);
        nglBuildScene->ViewX1 = (int)(((vx1 + 1.0f) * 0.5f) * w + 0.5f);
        nglBuildScene->ViewX2 = (int)(((vx2 + 1.0f) * 0.5f) * w + 0.5f);
        nglBuildScene->ViewY1 = (int)(((vy1 + 1.0f) * 0.5f) * h + 0.5f);
        nglBuildScene->ViewY2 = (int)(((vy2 + 1.0f) * 0.5f) * h + 0.5f);
        nglBuildScene->MatricesDirty = true;
    }
}

// ============================================================================
// Matrix getters / project / unproject
// ============================================================================
math::Mat44* nglGetMatrix(math::Mat44* result, nglMatrixType ID, nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    switch (ID) {
    case NGLMTX_VIEW_TO_SCREEN: *result = nglBuildScene->ViewToScreen; return result;
    case NGLMTX_WORLD_TO_SCREEN: *result = nglBuildScene->WorldToScreen; return result;
    case NGLMTX_PROJECTION: *result = nglBuildScene->Projection; return result;
    case NGLMTX_VIEW_TO_WORLD:
    case NGLMTX_WORLD_TO_VIEW: {
        const math::Mat43* m = ID == NGLMTX_VIEW_TO_WORLD
            ? &nglBuildScene->ViewToWorld : &nglBuildScene->WorldToView;
        result->x.v = _mm_shuffle_ps(m->x.v, _mm_setzero_ps(), 0xE4);
        result->y.v = _mm_shuffle_ps(m->y.v, _mm_setzero_ps(), 0xE4);
        result->z.v = _mm_shuffle_ps(m->z.v, _mm_setzero_ps(), 0xE4);
        result->w.v = _mm_shuffle_ps(m->w.v, _mm_set1_ps(1.0f), 0xE4);
        return result;
    }
    case NGLMTX_UI:
        UI(result, Scene);
        return result;
    default:
        _tlAssert("src/ngl_scene.cpp", 754, "false", "Invalid matrix ID.");
        result->x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        result->y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        result->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        result->w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
        return result;
    }
}

const math::Mat43* nglGetMatrix_ViewToWorld(nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    return &Scene->ViewToWorld;
}
const math::Mat43* nglGetMatrix_WorldToView(nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    return &Scene->WorldToView;
}
const math::Mat44* nglGetMatrix_ViewToScreen(nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    return &Scene->ViewToScreen;
}
const math::Mat44* nglGetMatrix_WorldToScreen(nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    return &Scene->WorldToScreen;
}

math::Position3* nglProjectPoint(math::Position3* result, const math::Position3* In,
                                 nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    __m128 v3 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(In->v, In->v, 0), Scene->WorldToView.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(In->v, In->v, 85), Scene->WorldToView.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(In->v, In->v, 170), Scene->WorldToView.z.v),
                   Scene->WorldToView.w.v));
    __m128 v4 = _mm_shuffle_ps(v3, v3, 170);
    __m128 v5 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v3, v3, 0), Scene->Projection.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v3, v3, 85), Scene->Projection.y.v)),
        _mm_add_ps(_mm_mul_ps(v4, Scene->Projection.z.v), Scene->Projection.w.v));
    __m128 v7 = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v5, v5, 0), Scene->View.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v5, v5, 85), Scene->View.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v5, v5, 170), Scene->View.z.v),
                   _mm_mul_ps(_mm_shuffle_ps(v5, v5, 255), Scene->View.w.v)));
    float w = _mm_shuffle_ps(v7, v7, 255).m128_f32[0];
    __m128 v8 = _mm_div_ps(v7, _mm_set1_ps(w));
    nglTexture* RenderTarget = Scene->RenderTarget;
    float ScreenWidth, ScreenHeight;
    if ((RenderTarget->Flags & 0x2000) != 0) {
        ScreenWidth = (float)nglGetScreenWidth();
        ScreenHeight = (float)nglGetScreenHeight();
    } else {
        ScreenWidth = (float)RenderTarget->Width;
        ScreenHeight = (float)RenderTarget->Height;
    }
    __m128 v10 = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(v8, _mm_set1_ps(0.5f)), _mm_set1_ps(0.5f)),
                            _mm_setr_ps(ScreenWidth, ScreenHeight, 0.0f, 0.0f));
    result->v = _mm_shuffle_ps(v10, _mm_shuffle_ps(v4, v10, 240), 196);
    return result;
}

// IDA 0x83D410 by-value overload (the compiler lowers this to the same
// hidden-result projection routine above).
math::Position3 nglProjectPoint(const math::Position3& In, nglScene* Scene) {
    math::Position3 result;
    nglProjectPoint(&result, &In, Scene);
    return result;
}

math::Position3* nglUnprojectPoint(math::Position3* result, const math::Position3* In,
                                   nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
    __m128 v = In->v;
    __m128 Point2 = _mm_setr_ps(v.m128_f32[0] * 2.0f, v.m128_f32[1] * 2.0f, 0.0f, 0.0f);
    __m128 v5;
    if (Scene->ProjType == NGLPROJ_PERSPECTIVE) {
        float z = v.m128_f32[2];
        v5 = _mm_setr_ps(z, z,
                         -(Scene->Projection.z.v.m128_f32[2] * z)
                         - Scene->Projection.w.v.m128_f32[2],
                         -z);
    } else {
        v5 = _mm_setr_ps(1.0f, 1.0f,
                         -(Scene->Projection.z.v.m128_f32[2] * v.m128_f32[2])
                         - Scene->Projection.w.v.m128_f32[2],
                         -1.0f);
    }
    nglTexture* RenderTarget = Scene->RenderTarget;
    __m128 v7;
    if ((RenderTarget->Flags & 0x2000) != 0)
        v7 = _mm_setr_ps((float)nglGetScreenWidth(), (float)nglGetScreenHeight(), 1.0f, 1.0f);
    else
        v7 = _mm_setr_ps((float)RenderTarget->Width, (float)RenderTarget->Height, 1.0f, 1.0f);
    __m128 v8 = _mm_mul_ps(v5, Point2);
    __m128 v10 = _mm_rcp_ps(v7);
    __m128 v11 = _mm_sub_ps(
        _mm_mul_ps(v8, _mm_mul_ps(_mm_sub_ps(_mm_set1_ps(2.0f), _mm_mul_ps(v10, v7)), v10)),
        v5);
    result->v = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v11, v11, 0), Scene->ViewportToWorld.x.v),
                   _mm_mul_ps(_mm_shuffle_ps(v11, v11, 85), Scene->ViewportToWorld.y.v)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v11, v11, 170), Scene->ViewportToWorld.z.v),
                   _mm_mul_ps(_mm_shuffle_ps(v11, v11, 255), Scene->ViewportToWorld.w.v)));
    return result;
}

void nglSetDefaultSceneParams() {
    ngliSetDefaultSceneParams();
    ngliSetRenderTarget(nglGetBackBufferTex());
    nglSetViewport(0.0f, 0.0f, (float)(nglGetScreenWidth() - 1),
                   (float)(nglGetScreenHeight() - 1));
    nglBuildScene->AspectRatio = nglIsDisplayWidescreen() ? 1.6161616f : 1.2121212f;
    nglBuildScene->MatricesDirty = true;
    nglSetPerspectiveMatrix(50.0f, 0.1f, 10000.0f);
    math::Mat43 v11;
    v11.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    v11.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    v11.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    v11.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    nglSetCameraMatrix(&v11);
    nglBuildScene->ClearFlags = 3;
    nglBuildScene->ClearColor.v = _mm_setzero_ps();
    nglBuildScene->ClearZ = 1.0f;
    nglBuildScene->FBWriteMask = 0x1010101;
    nglBuildScene->ZWriteEnable = true;
    nglBuildScene->ZTestEnable = true;
    nglBuildScene->FogEnabled = false;
    ngliEnableFog(false);
    nglSetFogRange(0.0f, 10000.0f, 0.0f, 1.0f);
    nglBuildScene->FogColor.v = _mm_set1_ps(1.0f);
    nglBuildScene->DepthOfFieldEnabled = false;
    ngliEnableDepthOfField(false);
    nglBuildScene->FocusDepth = 0.0f;
    nglBuildScene->AnimTime = 0.0f;
    nglSetLightContext(nglDefaultLightContext, nglBuildScene);
    nglBuildScene->MatricesDirty = true;
}

// ============================================================================
// Scene begin/end
// ============================================================================
void nglSetupBeginScene(nglScene* Scene, nglSceneParamType ParamSource) {
    nglScene* Parent = nglBuildScene;
    nglBuildScene = Scene;
    Scene->ZTarget = NULL;
    Scene->RenderTarget = NULL;
    switch (ParamSource) {
    case NGLSCENE_DEFAULTS:
        ngliSetupBeginSceneDefaults(Scene);
        break;
    case NGLSCENE_PARENT:
        *Scene = *Parent;
        break;
    case NGLSCENE_ROOT:
        *Scene = *nglRootBuildScene;
        break;
    default:
        break;
    }
    unsigned int* v5 = (unsigned int*)nglListAlloc(4 * nglSceneParamSet::NumParams + 8, 8);
    v5[0] = 0;
    v5[1] = 0;
    Scene->SceneParams.Array = v5;
    if (ParamSource == NGLSCENE_PARENT) {
        memcpy(Scene->SceneParams.Array, Parent->SceneParams.Array,
               4 * nglSceneParamSet::NumParams + 8);
    } else if (ParamSource == NGLSCENE_ROOT) {
        memcpy(Scene->SceneParams.Array, nglRootBuildScene->SceneParams.Array,
               4 * nglSceneParamSet::NumParams + 8);
    }
    ngliSetupBeginScene(Scene);
    Scene->NextSibling = NULL;
    Scene->FirstChild = NULL;
    Scene->LastChild = NULL;
    Scene->Pre.Fn = NULL;
    Scene->Pre.Data = NULL;
    Scene->Mid.Fn = NULL;
    Scene->Mid.Data = NULL;
    Scene->Post.Fn = NULL;
    Scene->Post.Data = NULL;
    Scene->StartScene.Fn = NULL;
    Scene->StartScene.Data = NULL;
    Scene->OpaqueRenderList = NULL;
    Scene->TransRenderList = NULL;
    Scene->OpaqueListCount = 0;
    Scene->TransListCount = 0;
    Scene->Parent = Parent;
}

nglScene* nglListBeginScene(nglSceneParamType ParamSource) {
    nglScene* v1 = (nglScene*)nglListAlloc(0x400, 0x10);
    if (v1 == NULL)
        return NULL;
    unsigned int* v2 = (unsigned int*)nglListAlloc(4 * nglSceneParamSet::NumParams + 8, 8);
    v1->SceneParams.Array = v2;
    v2[0] = 0;
    v2[1] = 0;
    if (nglBuildScene != NULL) {
        nglScene* LastChild = nglBuildScene->LastChild;
        if (LastChild != NULL)
            LastChild->NextSibling = v1;
        else
            nglBuildScene->FirstChild = v1;
        nglBuildScene->LastChild = v1;
        nglSetupBeginScene(v1, ParamSource);
        return v1;
    } else {
        nglRootBuildScene = v1;
        nglSetupBeginScene(v1, ParamSource);
        return v1;
    }
}

void nglListAddCustomNode(void (*CustomNodeFn)(void*), void* Data, const nglSortInfo* SortInfo) {
    if (nglBuildScene->MatricesDirty) {
        nglBuildScene->MatricesDirty = false;
        nglCalculateMatrices(nglBuildScene);
    }
    nglRenderCallbackNode* v3 = (nglRenderCallbackNode*)nglListAlloc(0x20, 0x10);
    if (v3 != NULL) {
        v3->SortInfo = *SortInfo;
        v3->Type = 1;
        v3->Fn = CustomNodeFn;
        v3->Data = Data;
        nglListAddNode(v3);
    }
}

nglScene* nglListBeginSceneNode(nglSceneParamType ParamSource, nglSortInfo* SortInfo) {
    nglScene* v2 = (nglScene*)nglListAlloc(0x400, 0x10);
    if (v2 == NULL)
        return NULL;
    unsigned int* v3 = (unsigned int*)nglListAlloc(4 * nglSceneParamSet::NumParams + 8, 8);
    v2->SceneParams.Array = v3;
    v3[0] = 0;
    v3[1] = 0;
    nglRenderCallbackNode* v4 = (nglRenderCallbackNode*)nglListAlloc(0x20, 0x10);
    if (v4 != NULL) {
        v4->SortInfo = *SortInfo;
        v4->Type = 0;
        v4->Fn = ngliRenderSceneNode;
        v4->Data = v2;
        nglListAddNode(v4);
    }
    nglSetupBeginScene(v2, ParamSource);
    return v2;
}

void nglSortScene(nglScene* Scene) {
    for (nglScene* i = Scene->FirstChild; i != NULL; i = i->NextSibling)
        nglSortScene(i);
    nglSortList_Impl(&Scene->OpaqueRenderList, (int)Scene->OpaqueListCount);
    nglSortList_Impl(&Scene->TransRenderList, (int)Scene->TransListCount);
}

void nglPresent() {
    if (nglBuildScene != nglRootBuildScene)
        tlFatal("nglPresent called while one or more scenes were still active (need to call nglListEndScene).");
    nglSortScene(nglBuildScene);
    ngliListSend();
    ngliListInit();
}

// ============================================================================
// nglCalculateMatrices - ea: 0x83B900
// ============================================================================

static math::Mat44* ViewportToWorldImpl(math::Mat44* result, nglScene* Scene) {
    float vx = Scene->View.x.v.m128_f32[0];
    float vy = Scene->View.y.v.m128_f32[1];
    __m128 v4 = _mm_setr_ps(-Scene->View.w.v.m128_f32[0] / vx,
                            (-1.0f / vy) * Scene->View.w.v.m128_f32[1],
                            1.0f, 1.0f);
    __m128 invV = _mm_setr_ps(1.0f / vx, 1.0f / vy, 1.0f, 0.0f);
    __m128 InvProjection_0, InvProjection_16, InvProjection_32;
    if (Scene->ProjType == NGLPROJ_PERSPECTIVE) {
        float pz = Scene->Projection.z.v.m128_f32[2];
        float pw = Scene->Projection.w.v.m128_f32[2];
        InvProjection_32 = _mm_setr_ps(0.0f, 0.0f, 1.0f, -pw / pz);
        InvProjection_16 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f / pz);
        InvProjection_0 = _mm_setr_ps(1.0f / Scene->Projection.x.v.m128_f32[0],
                                      1.0f / Scene->Projection.y.v.m128_f32[1],
                                      0.0f, 0.0f);
    } else {
        float pz = Scene->Projection.z.v.m128_f32[2];
        float pw = Scene->Projection.w.v.m128_f32[2];
        InvProjection_32 = _mm_setr_ps(0.0f, 0.0f, -pw / pz, 1.0f);
        InvProjection_16 = _mm_setr_ps(0.0f, 0.0f, 1.0f / pz, 0.0f);
        InvProjection_0 = _mm_setr_ps(1.0f / Scene->Projection.x.v.m128_f32[0],
                                      1.0f / Scene->Projection.y.v.m128_f32[1],
                                      0.0f, 0.0f);
    }
    __m128 rows[4];
    rows[0] = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(invV, invV, 0), InvProjection_0),
                   _mm_mul_ps(_mm_shuffle_ps(invV, invV, 85), InvProjection_16)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(invV, invV, 170), InvProjection_32),
                   _mm_mul_ps(_mm_shuffle_ps(invV, invV, 255), _mm_setzero_ps())));
    rows[1] = _mm_add_ps(
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(invV, invV, 0), InvProjection_0),
                   _mm_mul_ps(_mm_shuffle_ps(invV, invV, 85), InvProjection_16)),
        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(invV, invV, 170), InvProjection_32),
                   _mm_mul_ps(_mm_shuffle_ps(invV, invV, 255), _mm_setzero_ps())));
    // The ViewToWorld matrix maps clip -> world.
    __m128 v13 = _mm_shuffle_ps(Scene->ViewToWorld.x.v, _mm_setzero_ps(), 0xE4);
    __m128 v16 = _mm_shuffle_ps(Scene->ViewToWorld.y.v, _mm_setzero_ps(), 0xE4);
    __m128 v17 = _mm_shuffle_ps(Scene->ViewToWorld.z.v, _mm_setzero_ps(), 0xE4);
    __m128 v18 = _mm_shuffle_ps(Scene->ViewToWorld.w.v, _mm_set1_ps(1.0f), 0xE4);
    __m128 outs[4];
    for (int i = 0; i < 4; ++i) {
        outs[i] = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(rows[i], rows[i], 0), v13),
                       _mm_mul_ps(_mm_shuffle_ps(rows[i], rows[i], 85), v16)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(rows[i], rows[i], 170), v17),
                       _mm_mul_ps(_mm_shuffle_ps(rows[i], rows[i], 255), v18)));
    }
    result->x.v = outs[0];
    result->y.v = outs[1];
    result->z.v = outs[2];
    result->w.v = outs[3];
    return result;
}

void nglValidateMatrices(nglScene* Scene) {
    if (Scene->MatricesDirty) {
        Scene->MatricesDirty = false;
        nglCalculateMatrices(Scene);
    }
}

void nglCalculateMatrices(nglScene* Scene) {
    if ((Scene->OpaqueListCount != 0 || Scene->TransListCount != 0)
        && _tlAssert("src/ngl_scene.cpp", 416,
                     "!Scene->OpaqueListCount && !Scene->TransListCount",
                     "Illegal projection change after nodes have been added to the scene."))
        __debugbreak();
    if ((Scene->sx2 - Scene->sx1) <= 0.0f || (Scene->sy2 - Scene->sy1) <= 0.0f)
        _tlAssert("src/ngl_scene.cpp", 426,
                  "Scene->sx2 - Scene->sx1 > 0 && Scene->sy2 - Scene->sy1 > 0",
                  "Degenerate scissor!");
    if ((Scene->vx2 - Scene->vx1) <= 0.0f || (Scene->vy2 - Scene->vy1) <= 0.0f)
        _tlAssert("src/ngl_scene.cpp", 427,
                  "Scene->vx2 - Scene->vx1 > 0 && Scene->vy2 - Scene->vy1 > 0",
                  "Degenerate viewport!");
    float vx1 = Scene->vx1;
    float vx2 = Scene->vx2;
    float vy1 = Scene->vy1;
    float vy2 = Scene->vy2;
    float invW = 1.0f / (vx2 - vx1);
    float invH = 1.0f / (vy2 - vy1);
    float sx1p = (Scene->sx1 * (invW * 2.0f)) - ((vx2 + vx1) * invW);
    float sx2p = (Scene->sx2 * (invW * 2.0f)) - ((vx2 + vx1) * invW);
    float sy1p = (Scene->sy1 * (invH * 2.0f)) - ((vy2 + vy1) * invH);
    float sy2p = (Scene->sy2 * (invH * 2.0f)) - ((vy2 + vy1) * invH);
    Scene->sx1p = sx1p;
    Scene->sx2p = sx2p;
    Scene->sy1p = sy1p;
    Scene->sy2p = sy2p;
    if (Scene->ProjType == NGLPROJ_PERSPECTIVE) {
        float fovr = Scene->FOV * 0.0087266462f;
        math::Vector4 SinCos =
            math::SinCos<3, 0, 3, 0>(
                *(const math::Vector4*)&_mm_set1_ps(fovr));
        float tanv = SinCos.v.m128_f32[0] / SinCos.v.m128_f32[1];
        float asp = Scene->AspectRatio;
        float h2 = asp * tanv;
        float v2v = tanv;
        float ax1 = fabsf(sx1p), ax2 = fabsf(sx2p), ay1 = fabsf(sy1p), ay2 = fabsf(sy2p);
        if (ax2 <= ax1) ax2 = ax1;
        if (ay2 <= ay1) ay2 = ay1;
        float vx1p = ((vx1 - (vx2 + vx1) * 0.5f) * ax2) + (vx2 + vx1) * 0.5f;
        float vx2p = ((vx2 - (vx2 + vx1) * 0.5f) * ax2) + (vx2 + vx1) * 0.5f;
        float vy1p = ((vy1 - (vy2 + vy1) * 0.5f) * ay2) + (vy2 + vy1) * 0.5f;
        float vy2p = ((vy2 - (vy2 + vy1) * 0.5f) * ay2) + (vy2 + vy1) * 0.5f;
        Scene->h2 = ax2 * h2;
        Scene->v2 = ay2 * v2v;
        math::Mat44 P;
        Perspective(&P, 1.0f / (ax2 * h2), 1.0f / (ay2 * v2v), Scene->NearZ, Scene->FarZ);
        Scene->Projection = P;
        // Build clip planes (near/far + 4 side planes from the frustum).
        math::Vector4 planes[6];
        planes[4].v = _mm_setr_ps(0.0f, 0.0f, 1.0f, Scene->NearZ);
        planes[5].v = _mm_setr_ps(0.0f, 0.0f, -1.0f, -Scene->FarZ);
        float sy2pw = h2 * sy2p;
        float sx2pw = h2 * sx2p;
        float s = 1.0f / sqrtf(sy2pw * sy2pw + 1.0f);
        planes[0].v = _mm_setr_ps(s, 0.0f, -s * sy2pw, 0.0f);
        s = 1.0f / sqrtf(sx2pw * sx2pw + 1.0f);
        planes[1].v = _mm_setr_ps(-s, 0.0f, s * sx2pw, 0.0f);
        float sy1pw = v2v * sy1p;
        s = 1.0f / sqrtf(sy1pw * sy1pw + 1.0f);
        planes[2].v = _mm_setr_ps(0.0f, -s, -s * sy1pw, 0.0f);
        float sy2pv = v2v * sy2p;
        s = 1.0f / sqrtf(sy2pv * sy2pv + 1.0f);
        planes[3].v = _mm_setr_ps(0.0f, s, s * sy2pv, 0.0f);
        for (int i = 0; i < 6; ++i)
            Scene->ClipPlanes[i] = planes[i];
        // View matrix from the adjusted viewport + scissor.
        math::Mat44 V;
        Viewport(&V, vx1p, vy1p, vx2p, vy2p);
        Scene->View = V;
    } else {
        math::Mat44 P;
        Ortho(&P, 1.0f / Scene->AspectRatio, 1.0f, Scene->NearZ, Scene->FarZ);
        Scene->Projection = P;
        math::Vector4 planes[6];
        planes[4].v = _mm_setr_ps(0.0f, 0.0f, 1.0f, Scene->NearZ);
        planes[5].v = _mm_setr_ps(0.0f, 0.0f, -1.0f, -Scene->FarZ);
        planes[0].v = _mm_setr_ps(1.0f, 0.0f, 0.0f, Scene->AspectRatio * Scene->sx1p);
        planes[1].v = _mm_setr_ps(-1.0f, 0.0f, 0.0f, -Scene->AspectRatio * Scene->sx2p);
        planes[2].v = _mm_setr_ps(0.0f, -1.0f, 0.0f, Scene->sy1p);
        planes[3].v = _mm_setr_ps(0.0f, 1.0f, 0.0f, -Scene->sy2p);
        for (int i = 0; i < 6; ++i)
            Scene->ClipPlanes[i] = planes[i];
        math::Mat44 V;
        Viewport(&V, Scene->sx1p, Scene->sy1p, Scene->sx2p, Scene->sy2p);
        Scene->View = V;
    }
    math::Mat44 D;
    ngliGetDeviceMatrix(&D, Scene->RenderTarget);
    Scene->Device = D;
    // ViewToScreen = Device * View * Projection (row-vector).
    math::Mat44 VP;
    for (int r = 0; r < 4; ++r) {
        __m128 row = r == 0 ? Scene->Projection.x.v
                    : r == 1 ? Scene->Projection.y.v
                    : r == 2 ? Scene->Projection.z.v
                    : Scene->Projection.w.v;
        __m128 out = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 0), Scene->View.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 85), Scene->View.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 170), Scene->View.z.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 255), Scene->View.w.v)));
        if (r == 0) VP.x.v = out;
        else if (r == 1) VP.y.v = out;
        else if (r == 2) VP.z.v = out;
        else VP.w.v = out;
    }
    for (int r = 0; r < 4; ++r) {
        __m128 row = r == 0 ? VP.x.v : r == 1 ? VP.y.v : r == 2 ? VP.z.v : VP.w.v;
        __m128 out = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 0), Scene->Device.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 85), Scene->Device.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 170), Scene->Device.z.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 255), Scene->Device.w.v)));
        if (r == 0) Scene->ViewToScreen.x.v = out;
        else if (r == 1) Scene->ViewToScreen.y.v = out;
        else if (r == 2) Scene->ViewToScreen.z.v = out;
        else Scene->ViewToScreen.w.v = out;
    }
    // ViewToWorld = inverse transpose of WorldToView.
    __m128 v61 = _mm_shuffle_ps(Scene->WorldToView.x.v, Scene->WorldToView.y.v, 0x44);
    Scene->ViewToWorld.x.v = _mm_shuffle_ps(v61, Scene->WorldToView.z.v, 0x88);
    Scene->ViewToWorld.y.v = _mm_shuffle_ps(v61, Scene->WorldToView.z.v, 0xDD);
    Scene->ViewToWorld.z.v = _mm_shuffle_ps(
        _mm_shuffle_ps(Scene->WorldToView.x.v, Scene->WorldToView.y.v, 0xEE),
        Scene->WorldToView.z.v, 0xA8);
    Scene->ViewToWorld.w.v = _mm_xor_ps(
        _mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Scene->WorldToView.w.v,
                                          Scene->WorldToView.w.v, 0), Scene->ViewToWorld.x.v),
                _mm_mul_ps(_mm_shuffle_ps(Scene->WorldToView.w.v,
                                          Scene->WorldToView.w.v, 85), Scene->ViewToWorld.y.v)),
            _mm_mul_ps(_mm_shuffle_ps(Scene->WorldToView.w.v,
                                      Scene->WorldToView.w.v, 170), Scene->ViewToWorld.z.v)));
    // WorldToScreen = ViewToScreen * WorldToView (row-vector).
    __m128 w2vx = _mm_shuffle_ps(Scene->WorldToView.x.v, _mm_setzero_ps(), 0xE4);
    __m128 w2vy = _mm_shuffle_ps(Scene->WorldToView.y.v, _mm_setzero_ps(), 0xE4);
    __m128 w2vz = _mm_shuffle_ps(Scene->WorldToView.z.v, _mm_setzero_ps(), 0xE4);
    __m128 w2vw = _mm_shuffle_ps(Scene->WorldToView.w.v, _mm_set1_ps(1.0f), 0xE4);
    for (int r = 0; r < 4; ++r) {
        __m128 row = r == 0 ? Scene->ViewToScreen.x.v
                    : r == 1 ? Scene->ViewToScreen.y.v
                    : r == 2 ? Scene->ViewToScreen.z.v
                    : Scene->ViewToScreen.w.v;
        __m128 out = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 0), w2vx),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 85), w2vy)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 170), w2vz),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 255), w2vw)));
        if (r == 0) Scene->WorldToScreen.x.v = out;
        else if (r == 1) Scene->WorldToScreen.y.v = out;
        else if (r == 2) Scene->WorldToScreen.z.v = out;
        else Scene->WorldToScreen.w.v = out;
    }
    ViewportToWorldImpl(&Scene->ViewportToWorld, Scene);
    Scene->ViewPos.v = Scene->ViewToWorld.w.v;
    Scene->ViewDir.v = Scene->ViewToWorld.z.v;
    math::Mat44 ui;
    UI(&ui, Scene);
    for (int r = 0; r < 4; ++r) {
        __m128 row = r == 0 ? ui.x.v : r == 1 ? ui.y.v : r == 2 ? ui.z.v : ui.w.v;
        __m128 out = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 0), Scene->Device.x.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 85), Scene->Device.y.v)),
            _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(row, row, 170), Scene->Device.z.v),
                       _mm_mul_ps(_mm_shuffle_ps(row, row, 255), Scene->Device.w.v)));
        if (r == 0) Scene->UIToDevice.x.v = out;
        else if (r == 1) Scene->UIToDevice.y.v = out;
        else if (r == 2) Scene->UIToDevice.z.v = out;
        else Scene->UIToDevice.w.v = out;
    }
}
