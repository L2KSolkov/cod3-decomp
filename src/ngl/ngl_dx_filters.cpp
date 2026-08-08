// ============================================================================
// ngl_dx_filters.cpp - full-screen filter passes (18 funcs, verified vs IDA).
// Source: src/dx/ngl_dx_filters.cpp (ngl_xboxr)
// Data: CurZBufferLUT/ZBufferTex/FiltersTexAllocated/ZBufferLUT/WorkTex/
// SrcTex + BoxFilter/YFilter/XFilter/NPasses/Inc/PrevVals/PrevVals_0/
// NPasses_0/Inc_0/D3DZTex.
// ============================================================================

#include "ngl/ngl_dx_filters.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_fsaa.h"
#include "ngl/ngl_xb_internal.h"
#include "d3d8.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglScene* nglBuildScene;                       // ngl_scene.o
extern nglDisplayModeType nglDisplayMode;             // ngl_internal.o
extern nglTexture* nglDepthBufferTex;                 // ngl_dx_tex_create.o
extern nglTexture* nglGetBackBufferTex();             // ngl_dx_texture.o
extern nglDxRenderState nglDxState;                   // ngl_dx_state.o
extern nglDxTexCacheClass nglDxTexCache;              // ngl_dx_texture.o
extern void nglDxSetRenderTarget(const nglTexture* RenderTarget,
                                 const nglTexture* DepthTarget,
                                 unsigned int MipLevel, int CubeMapFace);  // ngl_dx_draw.o
extern void nglDxSetTexture(unsigned int Stage, nglTexture* Tex,
                            unsigned int FilterFlags, unsigned int MaxAnisotropy); // ngl_dx_texture.o
extern void nglDxInitShaders(bool RegisterShaders);   // ngl_dx_shader.o
extern void nglDxRenderState_SetBlendMode(unsigned int BM);  // (inline alias)
extern nglTexture* nglCreateTexture(unsigned int Flags, unsigned int Format, int Width,
                                    int Height, int Depth, int Levels);  // ngl_gpu_texture.o
extern void nglDestroyTexture(nglTexture* Tex);       // ngl_dx_tex_create.o
extern gpuVertexFormat nglGpuPUVVertexFmt;            // ngl_gpu.o
extern gpuVertexFormat nglGpuPUV4VertexFmt;           // ngl_gpu.o
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;  // cdGlowShader.o
extern unsigned int gpuHashVertexShader;
extern unsigned int gpuHashPixelShader;
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;
extern unsigned int gpuHashIndexBuffer;
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ngl pushbuffer method-encode table (.textbss, filled at D3D init).
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_40350;
extern unsigned int dword_40358;
extern unsigned int dword_4035C;
extern unsigned int dword_417FC;
extern unsigned int dword_40260;
extern unsigned int dword_40264;
extern unsigned int dword_40A60;
extern unsigned int dword_40A64;
extern unsigned int dword_40A80;
extern unsigned int dword_40A84;
extern unsigned int dword_40AC0;
extern unsigned int dword_40AC4;

// XDK D3D8 state cache (shim externs).
extern unsigned int dword_BC2D00;
extern unsigned int dword_BC2CFC;
extern unsigned int dword_BC2D38;
extern unsigned int dword_BC2D08;
extern unsigned int dword_BC2D0C;
extern unsigned int dword_BC2D04;
extern unsigned int dword_BC2CF8;

// XDK pixel-shader input encodings.
extern unsigned int byte_C800C9;
extern unsigned int loc_CA00CB;

// ============================================================================
// D3DXCOLOR::operator unsigned long (d3dx8d) - replicated inline: clamp each
// channel to [0,1], *255, +0.5 round, pack 0xAARRGGBB.
// ============================================================================
static unsigned int D3DXColorToARGB(float r, float g, float b, float a) {
    unsigned int R, G, B, A;
    if (r >= 1.0f) R = 0xFF;
    else if (r <= 0.0f) R = 0;
    else R = (unsigned int)(r * 255.0f + 0.5f);
    if (g >= 1.0f) G = 0xFF;
    else if (g <= 0.0f) G = 0;
    else G = (unsigned int)(g * 255.0f + 0.5f);
    if (b >= 1.0f) B = 0xFF;
    else if (b <= 0.0f) B = 0;
    else B = (unsigned int)(b * 255.0f + 0.5f);
    if (a >= 1.0f) A = 0xFF;
    else if (a <= 0.0f) A = 0;
    else A = (unsigned int)(a * 255.0f + 0.5f);
    return (A << 24) | (R << 16) | (G << 8) | B;
}

// ============================================================================
// Data (ngl_dx_filters.o)
// ============================================================================
namespace nglDxFilters {

t_ZBufferLUT CurZBufferLUT = ZLUT_MAX;
nglTexture ZBufferTex;
bool FiltersTexAllocated = false;
nglTexture* ZBufferLUT = NULL;
nglTexture* WorkTex[2] = { NULL, NULL };
nglTexture*& SrcTex = WorkTex[1];
FilterSample BoxFilter[4] = {
    { 0.25f, -0.5f, -0.5f },
    { 0.25f, 1.0f, -0.5f },
    { 0.25f, -0.5f, 1.0f },
    { 0.25f, 1.0f, 1.0f },
};
FilterSample YFilter[4] = {
    { 0.16666667f, 0.0f, -1.5f },
    { 0.33333334f, 0.0f, -0.5f },
    { 0.33333334f, 0.0f, 1.0f },
    { 0.16666667f, 0.0f, 1.5f },
};
FilterSample XFilter[4] = {
    { 0.16666667f, -1.5f, 0.0f },
    { 0.33333334f, -0.5f, 0.0f },
    { 0.33333334f, 1.0f, 0.0f },
    { 0.16666667f, 1.5f, 0.0f },
};
int NPasses = 4;
float Inc = 0.006f;
float PrevVals[3] = { -1000.0f, -1000.0f, -1000.0f };
float PrevVals_0[6] = { -1000.0f, -1000.0f, -1000.0f, -1000.0f, -1000.0f, -1000.0f };
int NPasses_0 = 3;
float Inc_0 = 0.003f;
D3DBaseTexture D3DZTex;

} // namespace nglDxFilters

// ============================================================================
// AllocFiltersTex / FreeFiltersTex - ea: 0x849280 / 0x849290
// ============================================================================
void nglDxFilters::AllocFiltersTex() {
}

void nglDxFilters::FreeFiltersTex() {
}

// ============================================================================
// nglSetFiltersTexSizes - ea: 0x8492A0
// ============================================================================
void nglSetFiltersTexSizes() {
}

// ============================================================================
// nglUpdateFilterTextures - ea: 0x8492B0
// ============================================================================
void nglUpdateFilterTextures() {
    XGSetTextureHeader(nglDisplayMode.Width, nglDisplayMode.Height,
                       1, 0, D3DFMT_LIN_A8R8G8B8, 0, &nglDxFilters::D3DZTex,
                       (void*)((D3DResource*)nglDepthBufferTex->RenderTarget)->Data,
                       4 * nglDisplayMode.Width);
    nglDxFilters::ZBufferTex.Texture = &nglDxFilters::D3DZTex;
}

// ============================================================================
// nglCreateFilterTextures - ea: 0x8492F0
// ============================================================================
void nglCreateFilterTextures(unsigned int FilterTexWidth, unsigned int FilterTexHeight) {
    if (FilterTexWidth == 0
        && _tlAssert("src/dx/ngl_dx_filters.cpp", 854, "FilterTexWidth >= 1",
                     "FilterTexWidth has to be >= 1 !"))
        __debugbreak();
    if (FilterTexHeight == 0
        && _tlAssert("src/dx/ngl_dx_filters.cpp", 855, "FilterTexHeight >= 1",
                     "FilterTexHeight has to be >= 1 !"))
        __debugbreak();
    nglDxFilters::ZBufferLUT = nglCreateTexture(0, 0, 256, 256, 0, 1);
    for (int i = 0; i < 2; ++i)
        nglDxFilters::WorkTex[i] = nglCreateTexture(0x11, 0x12, (int)FilterTexWidth,
                                                    (int)FilterTexHeight, 0, 1);
    XGSetTextureHeader(nglDisplayMode.Width, nglDisplayMode.Height, 1, 0,
                       D3DFMT_LIN_A8R8G8B8, 0, &nglDxFilters::D3DZTex,
                       (void*)((D3DResource*)nglDepthBufferTex->RenderTarget)->Data,
                       4 * nglDisplayMode.Width);
    nglDxFilters::ZBufferTex.Texture = &nglDxFilters::D3DZTex;
}

// ============================================================================
// nglDestroyFilterTextures - ea: 0x8493D0
// ============================================================================
void nglDestroyFilterTextures() {
    for (int i = 0; i < 2; ++i)
        nglDestroyTexture(nglDxFilters::WorkTex[i]);
    nglDestroyTexture(nglDxFilters::ZBufferLUT);
}

// ============================================================================
// nglPostProcessFiltersTex - ea: 0x849600
// ============================================================================
void nglPostProcessFiltersTex() {
}

// ============================================================================
// FillDepthPalette - ea: 0x8490A0
// ============================================================================
void nglDxFilters::FillDepthPalette() {
    if (PrevVals[0] == nglBuildScene->FocusDepth) {
        if (PrevVals[1] == nglBuildScene->NearZ) {
            if (PrevVals[2] == nglBuildScene->FarZ) {
                if (CurZBufferLUT != ZLUT_DOF)
                    CurZBufferLUT = ZLUT_DOF;
            } else {
                PrevVals[2] = nglBuildScene->FarZ;
                CurZBufferLUT = ZLUT_DOF;
            }
        } else {
            PrevVals[1] = nglBuildScene->NearZ;
            CurZBufferLUT = ZLUT_DOF;
        }
    } else {
        PrevVals[0] = nglBuildScene->FocusDepth;
        CurZBufferLUT = ZLUT_DOF;
    }
}

// ============================================================================
// FillFogPalette - ea: 0x849140
// ============================================================================
nglDxFilters::t_ZBufferLUT nglDxFilters::FillFogPalette() {
    if (PrevVals_0[0] == nglBuildScene->FogNear) {
        if (PrevVals_0[1] == nglBuildScene->FogFar) {
            if (PrevVals_0[2] == nglBuildScene->FogMin) {
                if (PrevVals_0[3] == nglBuildScene->FogMax) {
                    if (PrevVals_0[4] == nglBuildScene->NearZ) {
                        if (PrevVals_0[5] == nglBuildScene->FarZ) {
                            if (CurZBufferLUT != ZLUT_ZFOG)
                                CurZBufferLUT = ZLUT_ZFOG;
                        } else {
                            PrevVals_0[5] = nglBuildScene->FarZ;
                            CurZBufferLUT = ZLUT_ZFOG;
                        }
                    } else {
                        PrevVals_0[4] = nglBuildScene->NearZ;
                        CurZBufferLUT = ZLUT_ZFOG;
                    }
                } else {
                    PrevVals_0[3] = nglBuildScene->FogMax;
                    CurZBufferLUT = ZLUT_ZFOG;
                }
            } else {
                PrevVals_0[2] = nglBuildScene->FogMin;
                CurZBufferLUT = ZLUT_ZFOG;
            }
        } else {
            PrevVals_0[1] = nglBuildScene->FogFar;
            CurZBufferLUT = ZLUT_ZFOG;
        }
    } else {
        PrevVals_0[0] = nglBuildScene->FogNear;
        CurZBufferLUT = ZLUT_ZFOG;
    }
    return CurZBufferLUT;
}

// ============================================================================
// CopyImage - ea: 0x847EF0
// ============================================================================
void nglDxFilters::CopyImage(nglTexture* SrcTex, nglTexture* DstTex) {
    if (SrcTex == DstTex)
        return;
    float x2 = (float)DstTex->Width;
    float y2 = (float)DstTex->Height;
    float u2 = 1.0f;
    float v2 = 1.0f;
    unsigned int Flags = SrcTex->Flags;
    if ((Flags & 0x4000) == 0) {
        u2 = (float)SrcTex->Width;
        v2 = (float)SrcTex->Height;
        if ((Flags & 0x2000) != 0) {
            u2 = nglFSAAScaleX * u2;
            v2 = nglFSAAScaleY * v2;
        }
    }
    unsigned char v11 = (unsigned char)dword_BC2D10;
    unsigned int ColorWrite = dword_BC2D1C;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, 0x1010101) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, 0x1010101);
        dword_BC2D1C = 0x1010101;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 1);
        dword_BC2D10 = 1;
    }
    if (nglDxTexCache.Prev[0].WrapU != 3) {
        nglDxTexCache.Prev[0].WrapU = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 3) == 0) {
            D3D__DirtyFlags |= 1;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 3;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 3) {
        nglDxTexCache.Prev[0].WrapV = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 3) == 0) {
            D3D__DirtyFlags |= 1;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 3;
        }
    }
    nglDxSetRenderTarget(DstTex, NULL, 0, 0);
    if (gpuHashPixelShader != (unsigned int)nglGpuTexPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuTexPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuTexPixelShader::Shader);
    }
    float vq[20];
    memset(vq, 0, 20);
    vq[5] = x2; vq[6] = 0; vq[7] = 0; vq[8] = u2; vq[9] = 0; vq[10] = 0;
    vq[11] = y2; vq[12] = 0; vq[13] = 0; vq[14] = v2;
    vq[15] = x2; vq[16] = y2; vq[17] = 0; vq[18] = u2; vq[19] = v2;
    nglDxInitShaders(false);
    if (nglGpuQuadPUVVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    nglDxSetTexture(0, SrcTex, 0, 3);
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPUVVertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, vq, nglGpuPUVVertexFmt.VertexSize);
    unsigned int v4 = ColorWrite;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_COLORWRITEENABLE, ColorWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40358, v4);
        dword_BC2D1C = v4;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, v11) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v11);
        dword_BC2D10 = v11;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
}

// ============================================================================
// FilterCopy - ea: 0x8481D0
// ============================================================================
void nglDxFilters::FilterCopy(nglTexture* SrcTex, nglTexture* DstTex,
                              unsigned int dwNumSamples, FilterSample* rSample,
                              unsigned int dwSuperSampleX, unsigned int dwSuperSampleY) {
    nglTexture* DepthTarget = nglBuildScene->ZTarget;
    nglTexture* RenderTarget = nglBuildScene->RenderTarget;
    nglDxSetRenderTarget(DstTex, NULL, 0, 0);
    _D3DSURFACE_DESC descSrc;
    D3DTexture_GetLevelDesc(SrcTex->Texture, 0, &descSrc);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    unsigned int ZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_STENCILENABLE, 0) == 0)
        D3DDevice_SetRenderState_StencilEnable(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x8006);
        dword_BC2D38 = 0x8006;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 1);
        dword_BC2D08 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 1);
        dword_BC2D0C = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        nglDxSetTexture(Stage, SrcTex, 0, 3);
        if (nglDxTexCache.Prev[Stage].WrapU != 3) {
            nglDxTexCache.Prev[Stage].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[Stage].WrapV != 3) {
            nglDxTexCache.Prev[Stage].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSV] = 3;
            }
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MAXMIPLEVEL, 0) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MAXMIPLEVEL] = 0;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_COLORKEYOP, 0) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_COLORKEYOP] = 0;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_COLORSIGN, 0) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_COLORSIGN] = 0;
        }
    }
    nglDxInitShaders(false);
    if (nglGpuQuadPUV4VertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUV4VertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUV4VertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGpuFilterPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuFilterPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuFilterPixelShader::Shader);
    }
    float Width = (float)DstTex->Width;
    float Height = (float)DstTex->Height;
    float Quad[4][11];
    memset(Quad, 0, sizeof(Quad));
    Quad[1][0] = Width;
    Quad[2][1] = Height;
    Quad[3][0] = Width;
    Quad[3][1] = Height;
    float fOffsetScaleX, fWidthScale, u1, v1;
    if (XGIsSwizzledFormat(descSrc.Format) != 0) {
        fOffsetScaleX = dwSuperSampleX * (1.0f / (float)descSrc.Width);
        u1 = (float)descSrc.Width * (1.0f / (float)descSrc.Width);
        fWidthScale = dwSuperSampleY * (1.0f / (float)descSrc.Height);
        v1 = (float)descSrc.Height * (1.0f / (float)descSrc.Height);
    } else {
        fOffsetScaleX = (float)dwSuperSampleX;
        fWidthScale = (float)dwSuperSampleY;
        u1 = (float)descSrc.Width;
        v1 = (float)descSrc.Height;
    }
    unsigned int dwSample = 0;
    unsigned int v18 = 0;
    unsigned int rPSInput[4];
    unsigned int rColor[4];
    if (dwNumSamples != 0) {
        unsigned int SampleIdx = 0;
        const FilterSample* pSample = rSample;
        for (;;) {
            float v24 = pSample->fScale;
            if (v24 >= 0.0f) {
                rColor[v18] = D3DXColorToARGB(v24, v24, v24, v24);
                rPSInput[v18] = (((v18 & 1) != 0) + 1) | 0xC0;
            } else {
                rColor[v18] = D3DXColorToARGB(-v24, -v24, -v24, -v24);
                rPSInput[v18] = (((v18 & 1) != 0) + 1) | 0xE0;
            }
            float v26 = pSample->fOffsetY * fWidthScale;
            float v27 = pSample->fOffsetX * fOffsetScaleX;
            float v28 = v27 + u1;
            float v29 = v26 + v1;
            Quad[0][3 + 2 * v18] = v27;
            Quad[0][4 + 2 * v18] = v26;
            Quad[1][3 + 2 * v18] = v28;
            Quad[1][4 + 2 * v18] = v26;
            Quad[2][3 + 2 * v18] = v27;
            Quad[2][4 + 2 * v18] = v29;
            Quad[3][3 + 2 * v18] = v28;
            Quad[3][4 + 2 * v18] = v29;
            ++v18;
            if (v18 == 4)
                goto draw_batch;
            if (SampleIdx == dwNumSamples - 1)
                break;
advance:
            ++SampleIdx;
            ++pSample;
            dwSample = SampleIdx;
            if (SampleIdx >= dwNumSamples)
                goto done;
        }
        if (v18 < 4) {
            for (unsigned int i = v18; i < 4; ++i) {
                rPSInput[i] = 0;
                rColor[i] = 0;
            }
        }
draw_batch:
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSCONSTANT0_0, rColor[0]) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40A60, rColor[0]);
            dword_BC2C38 = rColor[0];
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSCONSTANT1_0, rColor[1]) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40A80, rColor[1]);
            dword_BC2C58 = rColor[1];
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSCONSTANT0_1, rColor[2]) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40A64, rColor[2]);
            dword_BC2C3C = rColor[2];
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSCONSTANT1_1, rColor[3]) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40A84, rColor[3]);
            dword_BC2C5C = rColor[3];
        }
        unsigned int v34 = (rPSInput[1] | (rPSInput[0] << 16)) << 8;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSRGBINPUTS0, byte_C800C9 | v34) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40AC0, byte_C800C9 | v34);
            dword_BC2C98 = byte_C800C9 | v34;
        }
        unsigned int v35 = v34 | 0x10D810D9;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PS_MIN, v35) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40260, v35);
            D3D__RenderState[0] = v35;
        }
        unsigned int v36 = (rPSInput[3] | (rPSInput[2] << 16)) << 8;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSRGBINPUTS1, loc_CA00CB | v36) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40AC4, loc_CA00CB | v36);
            dword_BC2C9C = loc_CA00CB | v36;
        }
        unsigned int v37 = v36 | 0x10DA10DB;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_PSALPHAINPUTS1, v37) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40264, v37);
            dword_BC2C14 = v37;
        }
        gpuHashVertexBuffer = 0;
        gpuHashVertexFormat = 0;
        D3DDevice_SelectVertexShaderDirect(nglGpuPUV4VertexFmt.VertexDeclaration, 0);
        D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Quad, nglGpuPUV4VertexFmt.VertexSize);
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 1);
            dword_BC2CFC = 1;
        }
        v18 = 0;
        SampleIdx = dwSample;
        goto advance;
    }
done:
    unsigned int v38 = ZWrite;
    nglDxState.PrevBM = -1;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, ZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v38);
        dword_BC2D10 = v38;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
    nglDxSetRenderTarget(RenderTarget, DepthTarget, 0, 0);
}

// ============================================================================
// RenderGlow - ea: 0x848A10
// ============================================================================
void nglDxFilters::RenderGlow(float GlowIntensity) {
    unsigned int ZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    nglTexture* BackBuffer = nglGetBackBufferTex();
    float GlowTexSize = (float)WorkTex[0]->Width;
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        if (nglDxTexCache.Prev[Stage].WrapU != 3) {
            nglDxTexCache.Prev[Stage].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[Stage].WrapV != 3) {
            nglDxTexCache.Prev[Stage].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSV] = 3;
            }
        }
    }
    nglTexture* LastRenderTarget = NULL;
    nglDxState.SetBlendMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
    nglDxInitShaders(false);
    if (nglGpuQuadPUVVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGlowShaderPixelPreFX::Shader) {
        gpuHashPixelShader = (unsigned int)nglGlowShaderPixelPreFX::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGlowShaderPixelPreFX::Shader);
    }
    nglDxSetTexture(0, BackBuffer, 1, 3);
    nglDxSetRenderTarget(WorkTex[0], NULL, 0, 0);
    float Width = (float)BackBuffer->Width;
    float Height = (float)BackBuffer->Height;
    float Quad[4][5];
    Quad[0][0] = 0; Quad[0][1] = 0; Quad[0][2] = 0; Quad[0][3] = 0; Quad[0][4] = 0;
    Quad[1][0] = GlowTexSize; Quad[1][1] = 0; Quad[1][2] = 0; Quad[1][3] = Width; Quad[1][4] = 0;
    Quad[2][0] = 0; Quad[2][1] = GlowTexSize; Quad[2][2] = 0; Quad[2][3] = 0; Quad[2][4] = Height;
    Quad[3][0] = GlowTexSize; Quad[3][1] = GlowTexSize; Quad[3][2] = 0; Quad[3][3] = Width; Quad[3][4] = Height;
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPUVVertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Quad, nglGpuPUVVertexFmt.VertexSize);
    nglDxInitShaders(false);
    if (nglGpuQuadPUV4VertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUV4VertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUV4VertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGlowShaderPixelFX::Shader) {
        gpuHashPixelShader = (unsigned int)nglGlowShaderPixelFX::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGlowShaderPixelFX::Shader);
    }
    float OffsetX[4], OffsetY[4];
    OffsetX[0] = -Inc; OffsetX[1] = Inc; OffsetX[2] = -Inc; OffsetX[3] = Inc;
    OffsetY[0] = Inc; OffsetY[1] = Inc; OffsetY[2] = -Inc; OffsetY[3] = -Inc;
    float Quad4[4][11];
    memset(Quad4, 0, sizeof(Quad4));
    for (int i = 0; i < 4; ++i) {
        Quad4[i][0] = (i & 1) ? GlowTexSize : 0.0f;
        Quad4[i][1] = (i & 2) ? GlowTexSize : 0.0f;
        Quad4[i][2] = 0.0f;
        Quad4[i][3] = (i & 1) ? GlowTexSize : 0.0f;
        Quad4[i][4] = (i & 2) ? GlowTexSize : 0.0f;
        Quad4[i][5] = GlowTexSize;
        Quad4[i][6] = GlowTexSize;
        Quad4[i][7] = 0.0f;
        Quad4[i][8] = Width;
        Quad4[i][9] = Height;
    }
    int v9 = 0;
    if (NPasses > 0) {
        do {
            LastRenderTarget = WorkTex[(v9 - 1) & 1];
            nglDxSetRenderTarget(LastRenderTarget, NULL, 0, 0);
            nglTexture* v12 = WorkTex[v9 & 1];
            for (unsigned int v10 = 0; v10 < 4; ++v10) {
                nglDxSetTexture(v10, v12, 1, 3);
                float v14 = 1.0f;
                if ((v12->Flags & 0x4000) == 0)
                    v14 = GlowTexSize;
                float v15 = OffsetX[v10];
                float v16 = OffsetY[v10];
                float v17 = v16 * v14;
                float v18 = (v15 + 1.0f) * v14;
                float v19 = (OffsetY[v10] + 1.0f) * v14;
                Quad4[v10][0] = v15 * v14;
                Quad4[v10][1] = v17;
                Quad4[v10][2] = 0.0f;
                Quad4[v10][3] = v18;
                Quad4[v10][4] = v17;
                Quad4[v10][5] = v19;
                Quad4[v10][6] = v18;
                Quad4[v10][7] = v19;
            }
            gpuHashVertexBuffer = 0;
            gpuHashVertexFormat = 0;
            D3DDevice_SelectVertexShaderDirect(nglGpuPUV4VertexFmt.VertexDeclaration, 0);
            D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Quad4, nglGpuPUV4VertexFmt.VertexSize);
            ++v9;
        } while (v9 < NPasses);
    }
    nglDxState.SetBlendMode((unsigned int)(GlowIntensity * 255.0f) | 0x86068600);
    nglDxInitShaders(false);
    if (nglGpuQuadPUVVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGlowShaderPixelPostFX::Shader) {
        gpuHashPixelShader = (unsigned int)nglGlowShaderPixelPostFX::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGlowShaderPixelPostFX::Shader);
    }
    nglDxSetTexture(0, LastRenderTarget, 1, 3);
    nglTexture* v20 = BackBuffer;
    nglDxSetRenderTarget(BackBuffer, NULL, 0, 0);
    float v21 = (float)v20->Width;
    float v22 = (float)v20->Height;
    float v23 = 1.0f;
    if ((LastRenderTarget->Flags & 0x4000) == 0)
        v23 = GlowTexSize;
    float QuadP[4][5];
    QuadP[0][0] = 0; QuadP[0][1] = 0; QuadP[0][2] = 0; QuadP[0][3] = 0; QuadP[0][4] = 0;
    QuadP[1][0] = v21; QuadP[1][1] = 0; QuadP[1][2] = 0; QuadP[1][3] = v23; QuadP[1][4] = 0;
    QuadP[2][0] = v21; QuadP[2][1] = v22; QuadP[2][2] = 0; QuadP[2][3] = v23; QuadP[2][4] = v23;
    QuadP[3][0] = 0; QuadP[3][1] = v22; QuadP[3][2] = 0; QuadP[3][3] = 0; QuadP[3][4] = v23;
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPUVVertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_QUADLIST, 4, QuadP, nglGpuPUVVertexFmt.VertexSize);
    unsigned int v24 = ZWrite;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, ZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v24);
        dword_BC2D10 = v24;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
}

// ============================================================================
// RenderBlur - ea: 0x849610
// ============================================================================
void nglDxFilters::RenderBlur(nglTexture* SrcTex, nglTexture* DstTex) {
    unsigned int ZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    nglDxState.SetBlendMode(0);
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        if (nglDxTexCache.Prev[Stage].WrapU != 3) {
            nglDxTexCache.Prev[Stage].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[Stage].WrapV != 3) {
            nglDxTexCache.Prev[Stage].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSV] = 3;
            }
        }
    }
    nglDxInitShaders(false);
    if (nglGpuQuadPUVVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGpuTexPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuTexPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuTexPixelShader::Shader);
    }
    nglDxSetTexture(0, SrcTex, 1, 3);
    nglDxSetRenderTarget(WorkTex[0], NULL, 0, 0);
    D3DDevice_SelectVertexShaderDirect(nglGpuPUVVertexFmt.VertexDeclaration, 0);
    float Width = (float)WorkTex[0]->Width;
    float Height = (float)WorkTex[0]->Height;
    float u = 1.0f, v = 1.0f;
    if ((SrcTex->Flags & 0x4000) == 0) {
        u = (float)SrcTex->Width;
        v = (float)SrcTex->Height;
    }
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
    gpuHashIndexBuffer = 0;
    D3DDevice_SetIndices(NULL, 0);
    nglXbPushQuad.PB = D3DDevice_BeginPush(0x19);
    *nglXbPushQuad.PB = dword_417FC;
    nglXbPushQuad.PB[1] = 8;
    nglXbPushQuad.PB[2] = 1078990872;
    nglXbPushQuad.PB += 3;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&Width;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&u;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&Width;
    *++nglXbPushQuad.PB = *(unsigned int*)&Height;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&u;
    ++nglXbPushQuad.PB;
    *nglXbPushQuad.PB++ = *(unsigned int*)&v;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = *(unsigned int*)&Height;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = *(unsigned int*)&v;
    *nglXbPushQuad.PB = dword_417FC;
    nglXbPushQuad.PB[1] = 0;
    nglXbPushQuad.PB += 2;
    D3DDevice_EndPush(nglXbPushQuad.PB);
    nglDxInitShaders(false);
    if (nglGpuQuadPUV4VertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUV4VertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUV4VertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGpuFilterPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuFilterPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuFilterPixelShader::Shader);
    }
    float OffsetX[4], OffsetY[4];
    OffsetX[0] = -Inc_0; OffsetX[1] = Inc_0; OffsetX[2] = -Inc_0; OffsetX[3] = Inc_0;
    OffsetY[0] = Inc_0; OffsetY[1] = Inc_0; OffsetY[2] = -Inc_0; OffsetY[3] = -Inc_0;
    float v9 = (float)WorkTex[0]->Height;
    float v10 = (float)WorkTex[0]->Width;
    float Quad4[4][11];
    memset(Quad4, 0, sizeof(Quad4));
    for (int i = 0; i < 4; ++i) {
        Quad4[i][0] = (i & 1) ? v10 : 0.0f;
        Quad4[i][1] = (i & 2) ? v9 : 0.0f;
        Quad4[i][2] = 0.0f;
        Quad4[i][3] = (i & 1) ? v10 : 0.0f;
        Quad4[i][4] = (i & 2) ? v9 : 0.0f;
        Quad4[i][5] = v10;
        Quad4[i][6] = v9;
        Quad4[i][7] = 0.0f;
        Quad4[i][8] = 0.0f;
        Quad4[i][9] = 0.0f;
    }
    nglTexture* LastRenderTarget = NULL;
    int Pass = 0;
    if (NPasses_0 > 0) {
        while (1) {
            LastRenderTarget = WorkTex[(Pass - 1) & 1];
            nglDxSetRenderTarget(LastRenderTarget, NULL, 0, 0);
            nglTexture* v12 = WorkTex[Pass & 1];
            for (unsigned int v13 = 0; v13 < 4; ++v13) {
                nglDxSetTexture(v13, v12, 1, 3);
                float v15 = (float)WorkTex[0]->Width;
                float v16 = (float)WorkTex[0]->Height;
                float v17 = OffsetX[v13];
                float v18 = OffsetY[v13];
                float v19 = v18 * v16;
                float v20 = (v17 + 1.0f) * v15;
                float v21 = (OffsetY[v13] + 1.0f) * v16;
                Quad4[v13][0] = v17 * v15;
                Quad4[v13][1] = v19;
                Quad4[v13][2] = 0.0f;
                Quad4[v13][3] = v20;
                Quad4[v13][4] = v19;
                Quad4[v13][5] = v21;
                Quad4[v13][6] = v20;
                Quad4[v13][7] = v21;
            }
            gpuHashVertexBuffer = 0;
            gpuHashVertexFormat = 0;
            D3DDevice_SelectVertexShaderDirect(nglGpuPUV4VertexFmt.VertexDeclaration, 0);
            D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Quad4, nglGpuPUV4VertexFmt.VertexSize);
            if (++Pass >= NPasses_0)
                break;
        }
    }
    nglDxInitShaders(false);
    if (nglGpuQuadPUVVertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGpuTexPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuTexPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuTexPixelShader::Shader);
    }
    nglDxSetTexture(0, LastRenderTarget, 1, 3);
    nglDxSetRenderTarget(DstTex, NULL, 0, 0);
    D3DDevice_SelectVertexShaderDirect(nglGpuPUVVertexFmt.VertexDeclaration, 0);
    unsigned int v22 = LastRenderTarget->Flags;
    Width = (float)DstTex->Width;
    Height = (float)DstTex->Height;
    if ((v22 & 0x4000) == 0) {
        float v23 = (float)WorkTex[0]->Width;
        float v24 = (float)WorkTex[0]->Height;
        u = v23;
        v = v24;
        if ((v22 & 0x2000) != 0) {
            u = nglFSAAScaleX * v23;
            v = nglFSAAScaleY * v24;
        }
    }
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
    gpuHashIndexBuffer = 0;
    D3DDevice_SetIndices(NULL, 0);
    nglXbPushQuad.PB = D3DDevice_BeginPush(0x19);
    *nglXbPushQuad.PB = dword_417FC;
    nglXbPushQuad.PB[1] = 8;
    nglXbPushQuad.PB[2] = 1078990872;
    nglXbPushQuad.PB += 3;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&Width;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&u;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&Width;
    *++nglXbPushQuad.PB = *(unsigned int*)&Height;
    *++nglXbPushQuad.PB = 0;
    *++nglXbPushQuad.PB = *(unsigned int*)&u;
    ++nglXbPushQuad.PB;
    *nglXbPushQuad.PB++ = *(unsigned int*)&v;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = *(unsigned int*)&Height;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = 0;
    *nglXbPushQuad.PB++ = *(unsigned int*)&v;
    *nglXbPushQuad.PB = dword_417FC;
    nglXbPushQuad.PB[1] = 0;
    nglXbPushQuad.PB += 2;
    D3DDevice_EndPush(nglXbPushQuad.PB);
    unsigned int v28 = ZWrite;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, ZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v28);
        dword_BC2D10 = v28;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
    nglDxSetRenderTarget(nglBuildScene->RenderTarget, nglBuildScene->ZTarget, 0, 0);
}

// ============================================================================
// Callbacks
// ============================================================================
void nglEdgeDetectionCallBack(void* Data) {
    nglDxFilters::FilterCopy(nglBuildScene->RenderTarget, nglDxFilters::WorkTex[0], 4,
                             nglDxFilters::BoxFilter, 2, 2);
    nglDxFilters::FilterCopy(nglDxFilters::WorkTex[0], nglDxFilters::SrcTex, 3,
                             nglDxFilters::XFilter, 2, 2);
    nglDxFilters::FilterCopy(nglDxFilters::SrcTex, nglDxFilters::WorkTex[0], 3,
                             nglDxFilters::YFilter, 2, 2);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1);
        dword_BC2CFC = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x800B) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x800B);
        dword_BC2D38 = 0x800B;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 0x300) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 0x300);
        dword_BC2D08 = 0x300;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x306) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0x306);
        dword_BC2D0C = 0x306;
    }
    nglDxFilters::CopyImage(nglDxFilters::WorkTex[0], nglBuildScene->RenderTarget);
    nglDxState.PrevBM = -1;
}

void nglFogCallBack(void* Data) {
}

void nglGlowCallBack(float* Data) {
    float GlowIntensity = 1.0f;
    if (Data != NULL)
        GlowIntensity = *Data;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    unsigned char v1 = (unsigned char)dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    nglDxFilters::RenderGlow(GlowIntensity);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, v1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v1);
        dword_BC2D10 = v1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
    nglDxState.PrevBM = -1;
}

void nglBlurCallBack(void* Data) {
    nglDxFilters::RenderBlur(nglBuildScene->RenderTarget, nglBuildScene->RenderTarget);
}

void nglDepthOfFieldCallBack() {
    nglDxFilters::FillDepthPalette();
    nglDxFilters::RenderBlur(nglBuildScene->RenderTarget, nglDxFilters::WorkTex[0]);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 0) == 0)
        D3DDevice_SetRenderState_ZEnable(0);
    unsigned int ZWrite = dword_BC2D10;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, 0);
        dword_BC2D10 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 1);
        dword_BC2D00 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40340, 0);
        dword_BC2D04 = 0;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4033C, 0x204);
        dword_BC2CF8 = 0x204;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 1);
        dword_BC2CFC = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, 0x8006) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40350, 0x8006);
        dword_BC2D38 = 0x8006;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, 1) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40344, 1);
        dword_BC2D08 = 1;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, 0x303) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40348, 0x303);
        dword_BC2D0C = 0x303;
    }
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    nglDxSetTexture(0, &nglDxFilters::ZBufferTex, 0, 3);
    nglDxSetTexture(1, nglDxFilters::ZBufferLUT, 0, 3);
    nglDxSetTexture(2, nglDxFilters::WorkTex[0], 1, 3);
    nglDxSetTexture(3, nglDxFilters::WorkTex[0], 1, 3);
    for (unsigned int Stage = 0; Stage < 4; ++Stage) {
        if (nglDxTexCache.Prev[Stage].WrapU != 3) {
            nglDxTexCache.Prev[Stage].WrapU = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSU, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSU] = 3;
            }
        }
        if (nglDxTexCache.Prev[Stage].WrapV != 3) {
            nglDxTexCache.Prev[Stage].WrapV = 3;
            if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_ADDRESSV, 3) == 0) {
                D3D__DirtyFlags |= 1 << Stage;
                D3D__TextureState[Stage][D3DTSS_ADDRESSV] = 3;
            }
        }
    }
    nglDxInitShaders(false);
    if (nglGpuQuadPUV4VertexShader::Shader != gpuHashVertexShader) {
        gpuHashVertexShader = nglGpuQuadPUV4VertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUV4VertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglDOFPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglDOFPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglDOFPixelShader::Shader);
    }
    float Width = (float)nglDxFilters::ZBufferTex.Width;
    float Height = (float)nglDxFilters::ZBufferTex.Height;
    float Quad4[4][11];
    memset(Quad4, 0, sizeof(Quad4));
    for (int i = 0; i < 4; ++i) {
        Quad4[i][0] = (i & 1) ? Width * nglFSAAScaleX : 0.0f;
        Quad4[i][1] = (i & 2) ? Height * nglFSAAScaleY : 0.0f;
        Quad4[i][2] = 0.0f;
        Quad4[i][3] = (i & 1) ? Width : 0.0f;
        Quad4[i][4] = (i & 2) ? Height : 0.0f;
        Quad4[i][5] = (float)nglDxFilters::WorkTex[0]->Width;
        Quad4[i][6] = (float)nglDxFilters::WorkTex[0]->Height;
        Quad4[i][7] = 0.0f;
        Quad4[i][8] = (float)nglDxFilters::WorkTex[0]->Width;
        Quad4[i][9] = (float)nglDxFilters::WorkTex[0]->Height;
    }
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPUV4VertexFmt.VertexDeclaration, 0);
    D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Quad4, nglGpuPUV4VertexFmt.VertexSize);
    unsigned int v3 = ZWrite;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZWRITEENABLE, ZWrite) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4035C, v3);
        dword_BC2D10 = v3;
    }
    nglDxState.PrevBM = -1;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ZENABLE, 1) == 0)
        D3DDevice_SetRenderState_ZEnable(1);
}
