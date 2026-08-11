// ============================================================================
// ngl_dx_scene.cpp - D3D scene setup/render (17 funcs, verified vs IDA).
// Source: src/dx/ngl_dx_scene.cpp (ngl_xboxr)
// ============================================================================

#include "ngl/ngl_scene.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_fsaa.h"
#include "ngl/ngl_dx_filters.h"
#include "ngl/nglDebug.h"
#include "ngl/nglTexture.h"
#include "d3d8.h"

#include <intrin.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglDebugStruct nglSyncDebug;                     // ngl_debug.o
extern nglDxRenderState nglDxState;                     // ngl_dx_state.o
extern unsigned int nglFrameVBlankCount;                // ngl_internal.o
extern float nglGetVBlankMS();                          // ngl_internal.o
extern int nglGetScreenWidth();                         // ngl_internal.o
extern int nglGetScreenHeight();                        // ngl_internal.o
extern float nglIFLSpeed;                               // ngl_texture.o
extern void nglSetClearStencil(unsigned int Stencil);   // ngl_scene.o
extern void nglSetDefaultSceneParams();                 // ngl_scene.o
extern void nglSetSceneCallBack(nglSceneCallbackType Type, void (*Fn)(void*), void* Data);
extern void nglDxSetRenderTarget(const nglTexture* RenderTarget,
                                 const nglTexture* DepthTarget,
                                 unsigned int MipLevel, int CubeMapFace);  // ngl_dx_draw.o
extern void nglValidateMatrices(nglScene* Scene);       // ngl_scene.o
extern void ngliGenMipmaps(nglTexture* Tex);            // ngl_dx_texture.o
extern void nglDepthOfFieldCallBack();                  // ngl_dx_filters.o
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void tlPrintf(const char* fmt, ...);

// Render-list iteration (uses the apsRenderNode.h nglRenderNode from
// ngl_dx_quad.h; the binary dispatches Render via vtable slot 1).
class nglRenderNode;
extern void nglBeginRenderNode(nglRenderNode* Head);
extern void nglAdvanceRenderNode();
nglRenderNode* nglCurRenderNode = nullptr;      // ?nglCurRenderNode@@3PAVnglRenderNode@@A (ngl.o)
nglRenderNode* nglRenderListEndNode = nullptr;  // ?nglRenderListEndNode@@3PAVnglRenderNode@@A (ngl.o)
extern int nglSceneRecursion;

// vtable slot 1 = Render (slot 0 = dtor).
typedef void (*RenderFn)(void* self);
static void RenderNode_Render(nglRenderNode* node) {
    RenderFn fn = *(RenderFn*)((char*)*(void**)node + 4);
    fn(node);
}

// ngl pushbuffer method-encode table.
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int dword_40354;
extern unsigned int dword_40358;
extern unsigned int dword_4035C;
extern unsigned int dword_40364;
extern unsigned int dword_4036C;
extern unsigned int dword_40378;
extern unsigned int dword_40344;
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D10;
extern unsigned int dword_BC2D1C;
extern unsigned int dword_BC2D24;
extern unsigned int dword_BC2D28;
extern unsigned int dword_BC2D30;
extern unsigned int dword_BC2CF4;
extern unsigned int dword_BC2E50;

// ============================================================================
// Data (ngl_dx_scene.o)
// ============================================================================
static math::Vector4 FSAAParams4RegularTex;
static unsigned char nglGetFSAAParams_InitFlag = 0;
static const unsigned int Blend[8] = { 0, 1, 0x300, 0x301, 0x302, 0x303, 0x304, 0x305 };
static const unsigned int CullMode[6] = { 0, 1, 2, 3, 4, 5 };
static unsigned int nglDxSetGarbageStates_i = 0;

// ============================================================================
// ngliSetRenderTarget - ea: 0x851A30
// ============================================================================
void ngliSetRenderTarget(nglTexture* Tex) {
    if (Tex == NULL) {
        nglBuildScene->RenderTarget = Tex;
        nglBuildScene->CubeMapFace = 0;
        return;
    }
    if ((Tex->Flags & 0x10) == 0
        && _tlAssert("src/dx/ngl_dx_scene.cpp", 91, "Tex->Flags & NGLTEX_RENDER_TARGET",
                     "Texture missing NGLTEX_RENDER_TARGET flag."))
        __debugbreak();
    if ((Tex->Flags & 0x300) != 0) {
        if (_tlAssert("src/dx/ngl_dx_scene.cpp", 92,
                      "!(Tex->Flags & (NGLTEX_CUBE | NGLTEX_VOLUME))",
                      "Texture has NGLTEX_CUBE flag."))
            __debugbreak();
    }
    nglBuildScene->RenderTarget = Tex;
    nglBuildScene->CubeMapFace = 0;
}

// ============================================================================
// nglSetCubeMapRenderTarget - ea: 0x851AE0
// ============================================================================
void nglSetCubeMapRenderTarget(nglTexture* Tex, int CubeMapFace) {
    if (Tex == NULL) {
        nglBuildScene->RenderTarget = Tex;
        nglBuildScene->CubeMapFace = CubeMapFace;
        return;
    }
    if ((Tex->Flags & 0x10) == 0
        && _tlAssert("src/dx/ngl_dx_scene.cpp", 112, "Tex->Flags & NGLTEX_RENDER_TARGET",
                     "Texture missing NGLTEX_RENDER_TARGET flag."))
        __debugbreak();
    if ((Tex->Flags & 0x100) != 0) {
        nglBuildScene->RenderTarget = Tex;
        nglBuildScene->CubeMapFace = CubeMapFace;
    } else {
        _tlAssert("src/dx/ngl_dx_scene.cpp", 113, "Tex->Flags & NGLTEX_CUBE",
                  "Texture missing NGLTEX_CUBE flag.");
        nglBuildScene->RenderTarget = Tex;
        nglBuildScene->CubeMapFace = CubeMapFace;
    }
}

// ============================================================================
// ngliSetZTarget - ea: 0x851B70
// ============================================================================
void ngliSetZTarget(nglTexture* Tex) {
    if (Tex == NULL) {
        nglBuildScene->ZTarget = Tex;
        return;
    }
    if ((Tex->Flags & 0x20) != 0) {
        nglBuildScene->ZTarget = Tex;
    } else {
        if (_tlAssert("src/dx/ngl_dx_scene.cpp", 133, "Tex->Flags & NGLTEX_ZTARGET",
                      "Texture missing NGLTEX_ZTARGET flag."))
            __debugbreak();
        nglBuildScene->ZTarget = Tex;
    }
}

// ============================================================================
// nglGetFSAAParams - ea: 0x851BD0
// ============================================================================
math::Vector4* nglGetFSAAParams(math::Vector4* result, nglTexture* RenderTarget) {
    if ((nglGetFSAAParams_InitFlag & 1) == 0) {
        FSAAParams4RegularTex.v = _mm_setr_ps(1.0f, 1.0f, 0.53125f, 0.0f);
        nglGetFSAAParams_InitFlag |= 1u;
    }
    if ((RenderTarget->Flags & 0x2000) != 0)
        *result = nglFSAAParams;
    else
        *result = FSAAParams4RegularTex;
    return result;
}

// ============================================================================
// DeviceXBox - ea: 0x851CA0
// ============================================================================
math::Mat44* DeviceXBox(math::Mat44* result, nglTexture* Target,
                        const math::Vector4* FSAAParams) {
    float ScreenWidth, ScreenHeight;
    if ((Target->Flags & 0x2000) != 0) {
        ScreenWidth = (float)nglGetScreenWidth();
        ScreenHeight = (float)nglGetScreenHeight();
    } else {
        ScreenWidth = (float)Target->Width;
        ScreenHeight = (float)Target->Height;
    }
    float v5 = (FSAAParams->v.m128_f32[1] * ScreenHeight) * 0.5f;
    result->x.v = _mm_setr_ps((FSAAParams->v.m128_f32[0] * ScreenWidth) * 0.5f, 0.0f, 0.0f, 0.0f);
    result->y.v = _mm_setr_ps(0.0f, v5, 0.0f, 0.0f);
    result->z.v = _mm_setr_ps(0.0f, 0.0f, 0.998046875f, 0.0f);
    result->w.v = _mm_setr_ps(FSAAParams->v.m128_f32[2] + (FSAAParams->v.m128_f32[0] * ScreenWidth) * 0.5f,
                              FSAAParams->v.m128_f32[2] + v5, 0.0f, 1.0f);
    return result;
}

// ============================================================================
// ngliGetDeviceMatrix - ea: 0x851E10
// ============================================================================
math::Mat44* ngliGetDeviceMatrix(math::Mat44* result, nglTexture* RenderTarget) {
    math::Vector4 Params;
    nglGetFSAAParams(&Params, RenderTarget);
    DeviceXBox(result, RenderTarget, &Params);
    return result;
}

// ============================================================================
// ngliCalculateMatrices - ea: 0x851E50
// ============================================================================
void ngliCalculateMatrices() {
}

// ============================================================================
// ngliSetClearStencil - ea: 0x851E60
// ============================================================================
void ngliSetClearStencil(unsigned int Stencil) {
    nglBuildScene->ClearStencil = Stencil;
}

// ============================================================================
// nglDxSetGarbageStates - ea: 0x851E70
// ============================================================================
unsigned int nglDxSetGarbageStates() {
    unsigned int v0 = CullMode[nglDxSetGarbageStates_i & 1];
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, v0) == 0)
        D3DDevice_SetRenderState_CullMode(v0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1);
        dword_BC2CFC = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x8006);
        dword_BC2D38 = 0x8006;
    }
    unsigned int v1 = Blend[nglDxSetGarbageStates_i & 7];
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, v1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, v1);
        dword_BC2D08 = v1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0);
        dword_BC2D0C = 0;
    }
    unsigned int result = nglDxSetGarbageStates_i + 1;
    nglDxState.PrevBM = -1;
    ++nglDxSetGarbageStates_i;
    if (((result) & 0x7F) == 0) {
        result = rand();
        nglDxSetGarbageStates_i = result;
    }
    return result;
}

// ============================================================================
// nglDxSetupScene - ea: 0x851F80
// ============================================================================
void nglDxSetupScene(nglScene* Scene) {
    if (Scene->AnimTime == 0.0f)
        Scene->CurAnimTime = nglGetVBlankMS() * (float)nglFrameVBlankCount * 0.001f;
    else
        Scene->CurAnimTime = Scene->AnimTime;
    if (Scene->ZWriteEnable || Scene->ZTestEnable) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
            D3DDevice_SetRenderState_ZEnable(1);
        if (Scene->ZTestEnable) {
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
                D3DDevice_SetRenderState_ZEnable(1);
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PS_MAX, 0x203) == 0) {
                D3DDevice_SetRenderState_Simple(dword_40354, 0x203);
                dword_BC2CF4 = 515;
            }
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ROPZCMPALWAYSREAD, 0) == 0)
                D3DDevice_SetRenderState_RopZCmpAlwaysRead(0);
        } else {
            if (dword_BC2E50 != 0) {
                if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILENABLE, 0) == 0)
                    D3DDevice_SetRenderState_StencilEnable(0);
            } else {
                if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILENABLE, 1) == 0)
                    D3DDevice_SetRenderState_StencilEnable(1);
                if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILFUNC, 0x207) == 0) {
                    D3DDevice_SetRenderState_Simple(dword_40364, 0x207);
                    dword_BC2D28 = 519;
                }
                if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILMASK, 0xFF) == 0) {
                    D3DDevice_SetRenderState_Simple(dword_4036C, 0xFF);
                    dword_BC2D30 = 255;
                }
                if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILPASS, 0) == 0) {
                    D3DDevice_SetRenderState_Simple(dword_40378, 0);
                    dword_BC2D24 = 0;
                }
            }
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PS_MAX, 0x207) == 0) {
                D3DDevice_SetRenderState_Simple(dword_40354, 0x207);
                dword_BC2CF4 = 519;
            }
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ROPZCMPALWAYSREAD, 1) == 0)
                D3DDevice_SetRenderState_RopZCmpAlwaysRead(1);
        }
    } else if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0) {
        D3DDevice_SetRenderState_ZEnable(0);
    }
    int CubeMapFace = Scene->CubeMapFace;
    nglTexture* ZTarget = Scene->ZTarget;
    nglTexture* v18 = Scene->RenderTarget;
    float ClearZ = *(float*)&ZTarget;
    nglDxSetRenderTarget(v18, ZTarget, 0, CubeMapFace);
    _D3DSURFACE_DESC desc;
    float Width = 1.0f;
    float Height = 1.0f;
    if (v18 != NULL) {
        D3DTexture_GetLevelDesc(v18->Texture, 0, &desc);
        Width = (float)desc.Width;
        Height = (float)desc.Height;
    } else if (ClearZ == 0.0f || *(D3DBaseTexture**)((char*)&ClearZ + 20) == NULL) {
        Width = 1.0f;
        Height = 1.0f;
    } else {
        D3DTexture_GetLevelDesc(*(D3DBaseTexture**)((char*)&ClearZ + 20), 0, &desc);
        Width = (float)desc.Width;
        Height = (float)desc.Height;
        if (Height < 0.0f)
            Height += 4294967300.0f;
    }
    int v19 = (int)(((Scene->sx1 + 1.0f) * 0.5f * Width + 0.5f));
    int v17 = (int)(((Scene->sy1 + 1.0f) * 0.5f * Height + 0.5f));
    int v7 = (int)(((Scene->sx2 + 1.0f) * 0.5f * Width + 0.5f));
    int v8 = (int)(((Scene->sy2 + 1.0f) * 0.5f * Height + 0.5f));
    int vp[10];
    if (v18 != NULL && (v18->Flags & 0x4000) != 0) {
        vp[1] = 0;
        vp[0] = 0;
        vp[2] = (int)Width;
        v8 = (int)Height;
    } else {
        vp[0] = v19;
        vp[1] = v17;
        vp[2] = v7 - v19;
        v8 = v8 - v17;
    }
    vp[3] = 0;
    vp[4] = v8;
    vp[5] = 1;
    D3DDevice_SetViewport(vp);
    void (*Fn)(void*) = Scene->SetupScene.Fn;
    float v10 = Scene->ClearZ;
    bool HasClear = Scene->ClearFlags != 0;
    if (Fn != NULL)
        Fn(Scene->SetupScene.Data);
    if (HasClear) {
        unsigned int ClearStencil = Scene->ClearStencil;
        unsigned int clearColor = 0;
        __m128 cc = _mm_mul_ps(Scene->ClearColor.v, _mm_set1_ps(255.0f));
        clearColor = ((unsigned int)cc.m128_f32[0] & 0xFF)
                   | (((unsigned int)cc.m128_f32[1] & 0xFF) << 8)
                   | (((unsigned int)cc.m128_f32[2] & 0xFF) << 16)
                   | (((unsigned int)cc.m128_f32[3] & 0xFF) << 24);
        D3DDevice_Clear(0, 0, Scene->ClearFlags, clearColor, v10, ClearStencil);
    }
    unsigned int FBWriteMask = Scene->FBWriteMask;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, FBWriteMask) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, FBWriteMask);
        dword_BC2D1C = FBWriteMask;
    }
    Scene->IFLFrame = (unsigned int)(nglIFLSpeed * Scene->CurAnimTime);
}

// ============================================================================
// nglRenderScene - ea: 0x8523F0
// ============================================================================
extern void nglRenderScene();  // recursion
int nglRenderScene_impl();

int nglRenderScene_impl() {
    nglScene* v0 = nglBuildScene;
    ++nglSceneRecursion;
    if (nglBuildScene->StartScene.Fn != NULL)
        nglBuildScene->StartScene.Fn(nglBuildScene->StartScene.Data);
    for (nglScene* i = v0->FirstChild; i != NULL; i = i->NextSibling) {
        nglBuildScene = i;
        if (v0->RenderTarget != NULL || v0->ZTarget != NULL)
            nglRenderScene();
    }
    nglBuildScene = v0;
    if (v0->RenderTarget != NULL || v0->ZTarget != NULL) {
        if (nglSyncDebug.DumpFrameLog != 0) {
            tlPrintf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            const char* str = v0->ZTarget != NULL ? v0->ZTarget->FileName->str : "null";
            const char* v7 = v0->RenderTarget != NULL ? v0->RenderTarget->FileName->str : "null";
            tlPrintf("+ NGL Scene Recursion=%d RenderTarget=%s DepthTarget=%s\n",
                     nglSceneRecursion, v7, str);
            tlPrintf("  OpaqueListCount=%d TransListCount=%d\n",
                     v0->OpaqueListCount, v0->TransListCount);
        }
        nglValidateMatrices(v0);
        nglDxSetupScene(v0);
        if (v0->Pre.Fn != NULL)
            v0->Pre.Fn(v0->Pre.Data);
        unsigned int ZWriteEnable = v0->ZWriteEnable;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, ZWriteEnable) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4035C, ZWriteEnable);
            dword_BC2D10 = ZWriteEnable;
        }
        nglBeginRenderNode(v0->OpaqueRenderList);
        while (nglCurRenderNode != nglRenderListEndNode) {
            RenderNode_Render(nglCurRenderNode);
            nglAdvanceRenderNode();
        }
        if (v0->Mid.Fn != NULL)
            v0->Mid.Fn(v0->Mid.Data);
        if (v0->TransListCount != 0) {
            if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
                D3DDevice_SetRenderState_Simple(dword_4035C, 0);
                dword_BC2D10 = 0;
            }
            nglBeginRenderNode(v0->TransRenderList);
            while (nglCurRenderNode != nglRenderListEndNode) {
                RenderNode_Render(nglCurRenderNode);
                nglAdvanceRenderNode();
            }
        }
        if (v0->Post.Fn != NULL)
            v0->Post.Fn(v0->Post.Data);
        if (v0->RenderTarget != NULL)
            ngliGenMipmaps(v0->RenderTarget);
        return --nglSceneRecursion;
    }
    return --nglSceneRecursion;
}

void nglRenderScene() {
    nglRenderScene_impl();
}

// ============================================================================
// ngliRenderSceneNode - ea: 0x852640
// ============================================================================
void ngliRenderSceneNode(void* Param) {
    nglScene* v1 = nglBuildScene;
    nglRenderNode* v2 = nglCurRenderNode;
    nglBuildScene = (nglScene*)Param;
    nglRenderScene();
    nglBuildScene = v1;
    nglCurRenderNode = v2;
    v1->ClearFlags = 0;
    nglDxSetupScene(nglBuildScene);
}

// ============================================================================
// ngliSetDefaultSceneParams / ngliSetupBeginSceneDefaults / ngliSetupBeginScene
// ============================================================================
void ngliSetDefaultSceneParams() {
    nglSetClearStencil(0);
}

void ngliSetupBeginSceneDefaults(nglScene* Scene) {
    memset(Scene, 0, sizeof(nglScene));
    nglSetDefaultSceneParams();
}

void ngliSetupBeginScene(nglScene* Scene) {
    Scene->LockedTextures = NULL;
    Scene->UploadFB = false;
    Scene->DownloadFB = false;
    Scene->UploadZ = false;
    Scene->DownloadZ = false;
}

// ============================================================================
// ngliEnableFog / ngliEnableDepthOfField
// ============================================================================
static void nglFogCallBackShim(void* Data) {
}

void ngliEnableFog(bool Enable) {
    if (Enable)
        nglSetSceneCallBack(NGLSCENE_POST, nglFogCallBackShim, NULL);
    else
        nglSetSceneCallBack(NGLSCENE_POST, NULL, NULL);
}

void ngliEnableDepthOfField(bool Enable) {
    if (Enable)
        nglSetSceneCallBack(NGLSCENE_POST,
                            (void (*)(void*))nglDepthOfFieldCallBack, NULL);
    else
        nglSetSceneCallBack(NGLSCENE_POST, NULL, NULL);
}
