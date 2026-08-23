// ============================================================================
// ngl_dx_texture.cpp - texture stage cache + front/back buffer wrappers (10).
// Source: src/dx/ngl_dx_texture.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_texture.o).
// Data: nglDxTexCache (0x14d2598) + static nglWhiteTex_FileName live here.
// The D3D__TextureState writes bypass the D3D8 library's internal state cache
// (XDK segment globals - declared in the shim, never implemented here).
// ============================================================================

#include "ngl/nglTexture.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/nglDebug.h"
#include "filesystem/apk.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void nglDestroyTexture(nglTexture* Tex);                 // ngl_dx_tex_create.o
extern nglTexture* nglCreateTexture(unsigned int Flags, _D3DFORMAT Format, int Width,
                                    int Height, int Depth, int Levels);  // ngl_gpu_texture.o
extern void nglDxUnbindPalettes(void);                          // ngl_xb_palette.o (ported)
extern void nglDxSetRenderTarget(const nglTexture* RenderTarget, const nglTexture* DepthTarget,
                                 unsigned int MipLevel, int CubeMapFace);  // ngl_dx_draw.o
extern void nglDxInitShaders(bool RegisterShaders);             // ngl_dx_shader.o
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern int nglFrame;
extern int nglTextureAnimFrame;
extern math::Vector4 nglFSAAParams;                             // ngl_dx_fsaa.o (ported)
extern gpuVertexFormat nglGpuPCUVVertexFmt;                     // ngl_gpu.o
extern unsigned int gpuHashVertexShader;                        // ngl_gpu.o
extern unsigned int gpuHashPixelShader;                         // ngl_gpu.o
extern unsigned int gpuHashVertexBuffer;                        // ngl_gpu.o
extern unsigned int gpuHashVertexFormat;                        // ngl_gpu.o
extern nglDxRenderState nglDxState;                             // ngl_dx_state.o
extern nglTexture* nglWhiteTex;                                 // ngl_texture.o (ported)
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;      // cdGlowShader.o

// ============================================================================
// Globals (data)
// ============================================================================
nglDxTexCacheClass nglDxTexCache;
static tlFixedString nglWhiteTex_FileName("nglwhite");

// ============================================================================
// ngliUnloadTexture - ea: 0x841860
// ============================================================================
void ngliUnloadTexture(apk::apkFile* File, apk::apkFileEntry* Entry) {
    tlFixedString name("image");
    int SectionIndex = File->GetSectionIndex(name);
    nglTexture* Data = (nglTexture*)Entry->GetData(File, SectionIndex, true);
    nglDestroyTexture(Data);
}

// ============================================================================
// nglDxUnbindTexStages - ea: 0x8418A0
// ============================================================================
void nglDxUnbindTexStages() {
    nglDxUnbindPalettes();
    for (unsigned int i = 0; i < 4; ++i)
        D3DDevice_SetTexture(i, NULL);
    for (int s = 0; s < 4; ++s) {
        nglDxTexCache.Prev[s].Tex = NULL;
        nglDxTexCache.Prev[s].TexHash = -1;
        nglDxTexCache.Prev[s].WrapU = -1;
        nglDxTexCache.Prev[s].WrapV = -1;
        nglDxTexCache.Prev[s].WrapW = -1;
        nglDxTexCache.Prev[s].FilterFlags = -1;
    }
}

// ============================================================================
// nglDxSetTexture - ea: 0x841940
// ============================================================================
void nglDxSetTexture(unsigned int Stage, nglTexture* Tex, unsigned int FilterFlags,
                     unsigned int MaxAnisotropy) {
    nglTexture* v4 = Tex;
    if ((Tex->Flags & 4) != 0) {
        v4 = Tex->Frames[nglTextureAnimFrame % Tex->NFrames];
        if (v4 == NULL && _tlAssert("src/dx/ngl_dx_texture.cpp", 269, "Tex",
                                    "nglDxSetTexture() encountered invalid IFL data.\n"))
            __debugbreak();
    }
    if (nglDxTexCache.Prev[Stage].Tex != v4) {
        nglDxTexCache.Prev[Stage].Tex = v4;
        v4->LastFrameRef = nglFrame;
        if (v4->File != NULL)
            v4->File->LastFrameRef = nglFrame;
        unsigned int v7 = v4->Flags & 0x4300;
        if ((v4->Flags & 0x4000) != 0 && v7 == nglDxTexCache.Prev[Stage].TexHash)
            D3DDevice::SwitchTexture(Stage, v4->Texture);
        else
            D3DDevice_SetTexture(Stage, v4->Texture);
        nglDxTexCache.Prev[Stage].TexHash = v7;
        nglDxTexCache.SetFilter(Stage, FilterFlags, MaxAnisotropy);
    }
}

// ============================================================================
// nglGetFrontBufferTex - ea: 0x841A10
// ============================================================================
nglTexture* nglGetFrontBufferTex() {
    D3DResource_Release((D3DResource*)nglFrontBufferTex.Texture);
    nglFrontBufferTex.Texture = D3DDevice_GetBackBuffer2(-1);
    return &nglFrontBufferTex;
}

// ============================================================================
// nglGetBackBufferTex - ea: 0x841A30
// ============================================================================
nglTexture* nglGetBackBufferTex() {
    D3DResource_Release((D3DResource*)nglBackBufferTex.Texture);
    nglBackBufferTex.Texture = D3DDevice_GetBackBuffer2(0);
    return &nglBackBufferTex;
}

// ============================================================================
// ngliLockSceneTextures / ngliUnlockSceneTextures / ngliInitIdentityPalette
// ea: 0x841A50 / 0x841A60 / 0x841A70 (empty in this build)
// ============================================================================
void ngliLockSceneTextures() {}
void ngliUnlockSceneTextures() {}
void ngliInitIdentityPalette() {}

// ============================================================================
// ngliGenMipmaps - ea: 0x841A80
// Generates mip levels by re-rendering the base level into each mip surface
// with a full-screen quad (PCUV format).
// ============================================================================
void ngliGenMipmaps(nglTexture* Tex) {
    if (Tex == NULL)
        return;
    unsigned int NMipmaps = D3DBaseTexture_GetLevelCount(Tex->Texture);
    if (NMipmaps <= 1)
        return;

    if ((Tex->Flags & 0x10) == 0 && _tlAssert("src/dx/ngl_dx_texture.cpp", 416,
            "Tex->Flags & NGLTEX_RENDER_TARGET",
            "NGL: Cannot generate mipmap for non-render-target texture."))
        __debugbreak();
    if ((Tex->Width == 0 || ((Tex->Width - 1) & Tex->Width) != 0 ||
         Tex->Height == 0 || ((Tex->Height - 1) & Tex->Height) != 0) &&
        _tlAssert("src/dx/ngl_dx_texture.cpp", 417,
                  "tlIsPow2(w) && tlIsPow2(h)",
                  "NGL: Cannot generate mipmaps for a non power of 2 texture !"))
        __debugbreak();
    if ((Tex->Flags & 0x4000) == 0 && _tlAssert("src/dx/ngl_dx_texture.cpp", 420,
            "Tex->Flags & NGLTEX_SWIZZLED", "NGL: Linear textures don't support mipmaps."))
        __debugbreak();

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    nglDxState.SetBlendMode(0);
    nglDxSetTexture(0, Tex, 1u, 3u);
    if (nglDxTexCache.Prev[0].WrapU != 3) {
        nglDxTexCache.Prev[0].WrapU = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSU, 3) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSU] = 3;
        }
    }
    if (nglDxTexCache.Prev[0].WrapV != 3) {
        nglDxTexCache.Prev[0].WrapV = 3;
        if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_ADDRESSV, 3) == 0) {
            D3D__DirtyFlags |= 1u;
            D3D__TextureState[0][D3DTSS_ADDRESSV] = 3;
        }
    }

    nglDxInitShaders(false);
    if (gpuHashVertexShader != nglGpuQuadPUVVertexShader::Shader) {
        gpuHashVertexShader = nglGpuQuadPUVVertexShader::Shader;
        D3DDevice_LoadVertexShaderProgram(&nglGpuQuadPUVVertexShader::Shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
    if (gpuHashPixelShader != (unsigned int)nglGpuTexPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuTexPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuTexPixelShader::Shader);
    }

    // IDA's nglGpuPCUVVertexFmt is a 0x18-byte stream: XYZ float3,
    // packed color dword, then UV float2.
    struct PCUVVert {
        float x;
        float y;
        float z;
        unsigned int color;
        float u;
        float v;
    };
    static_assert(sizeof(PCUVVert) == 0x18, "ngl mipmap PCUV layout");
    PCUVVert Verts[4];
    memset(Verts, 0, sizeof(Verts));
    unsigned int w = Tex->Width;
    unsigned int h = Tex->Height;

    if (D3DDevice_SetTextureState_ParameterCheck(0, D3DTSS_MIPFILTER, 0) == 0) {
        D3D__DirtyFlags |= 1u;
        D3D__TextureState[0][D3DTSS_MIPFILTER] = 0;
    }

    for (unsigned int i = 1; i < NMipmaps; ++i) {
        nglDxSetRenderTarget(Tex, NULL, i, 0);
        float fw = (float)(w >> i);
        float fh = (float)(h >> i);
        gpuHashVertexBuffer = 0;
        gpuHashVertexFormat = 0;
        Verts[0].u = 0.0f;
        Verts[0].v = 0.0f;
        Verts[1].x = fw;
        Verts[1].u = nglFSAAParams.v.m128_f32[0];
        Verts[1].v = 0.0f;
        Verts[2].x = fw;
        Verts[2].y = fh;
        Verts[2].u = nglFSAAParams.v.m128_f32[0];
        Verts[2].v = nglFSAAParams.v.m128_f32[1];
        Verts[3].y = fh;
        Verts[3].u = 0.0f;
        Verts[3].v = nglFSAAParams.v.m128_f32[1];
        D3DDevice_SelectVertexShaderDirect(nglGpuPCUVVertexFmt.VertexDeclaration, 0);
        D3DDevice_DrawVerticesUP(D3DPT_TRIANGLESTRIP, 4, Verts,
                                 (unsigned int)nglGpuPCUVVertexFmt.VertexSize);
    }

    for (int s = 0; s < 4; ++s) {
        nglDxTexCache.Prev[s].Tex = NULL;
        nglDxTexCache.Prev[s].TexHash = -1;
        nglDxTexCache.Prev[s].WrapU = -1;
        nglDxTexCache.Prev[s].WrapV = -1;
        nglDxTexCache.Prev[s].WrapW = -1;
        nglDxTexCache.Prev[s].FilterFlags = -1;
    }
}

// ============================================================================
// ngliInitWhiteTexture - ea: 0x841DA0
// ============================================================================
void ngliInitWhiteTexture() {
    nglGpuAcquireDevice();
    nglWhiteTex = nglCreateTexture(0, (_D3DFORMAT)6u, 1, 1, 0, 1);
    D3DLOCKED_RECT Rect;
    D3DTexture_LockRect((D3DTexture*)nglWhiteTex->Texture, 0, &Rect, NULL, 0);
    *(unsigned int*)Rect.pBits = 0xFFFFFFFF;
    nglWhiteTex->FileName = &nglWhiteTex_FileName;
    nglTextureDirectory.Add(nglWhiteTex);
    nglGpuReleaseDevice();
}

// ============================================================================
// nglDxTexCacheClass::SetFilter - ea: 0x841EC0
// ============================================================================
void nglDxTexCacheClass::SetFilter(unsigned int Stage, unsigned int FilterFlags,
                                   unsigned int MaxAnisotropy) {
    StageCache* v4 = &Prev[Stage];
    if (FilterFlags == v4->FilterFlags && MaxAnisotropy == v4->MaxAnisotropy)
        return;
    v4->FilterFlags = FilterFlags;
    v4->MaxAnisotropy = MaxAnisotropy;

    switch (FilterFlags & 3) {
    case 0:
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MAGFILTER, 1u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MAGFILTER] = 1;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MINFILTER, 1u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MINFILTER] = 1;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MIPFILTER, 1u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MIPFILTER] = 1;
        }
        break;
    case 1:
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MAGFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MAGFILTER] = 2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MINFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MINFILTER] = 2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MIPFILTER, 1u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MIPFILTER] = 1;
        }
        break;
    case 2:
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MAGFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MAGFILTER] = 2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MINFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MINFILTER] = 2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MIPFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MIPFILTER] = 2;
        }
        break;
    case 3:
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MAGFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MAGFILTER] = 2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MINFILTER, 3u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MINFILTER] = 3;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(Stage, D3DTSS_MIPFILTER, 2u) == 0) {
            D3D__DirtyFlags |= 1 << Stage;
            D3D__TextureState[Stage][D3DTSS_MIPFILTER] = 2;
        }
        nglDxState.SetMaxAnisotropy((int)Stage, (int)MaxAnisotropy);
        break;
    default:
        if (_tlAssert("c:\\cod\\code\\tl\\ngl\\include\\dx/ngl_dx_texture.h", 132,
                      "false", "Unknown filter type."))
            __debugbreak();
        break;
    }
}
